/*
 * Copyright (c) 2014 MediaTek Inc.
 * Author: YongWu <yong.wu@mediatek.com>
 *         Honghui Zhang <honghui.zhang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/iommu.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <soc/mediatek/mtk-smi.h>
#include "mtk_iommu_platform.h"
#include "mtk_iommu_reg_mt2701.h"

/* clear iommu interruption status. */
static void mt2701_iommu_clear_intr(void __iomem *m4u_base)
{
	u32 temp;

	temp = readl(m4u_base + REG_MMU_INT_CONTROL) | F_INT_CLR_BIT;
	writel(temp, m4u_base + REG_MMU_INT_CONTROL);
}

static int mt2701_iommu_config_port(struct mtk_iommu_info *piommu,
				    int portid, bool enable)
{
	return mtk_smi_config_port(NULL, portid, enable);
}

void mt2701_iommu_invalidate_tlb(const struct mtk_iommu_info *piommu,
				 unsigned int m4u_id, int isinvall,
				 unsigned int iova_start,
				 unsigned int iova_end)
{
	u32 regval = 0;
	unsigned int cnt = 0;
	void __iomem *m4u_base_g = piommu->base;
	void __iomem *m4u_base_l2 = piommu->l2_base;

	if (isinvall || (iova_end <= iova_start))
		isinvall = 1;

	regval = F_MMUG_CTRL_INV_EN0 | F_MMUG_CTRL_INV_EN2;
	writel(regval, m4u_base_g + REG_MMUG_CTRL);

	if (isinvall) {
invalid_all:
		writel(F_MMUG_INV_ALL, m4u_base_g + REG_MMUG_INVLD);
		regval = readl(m4u_base_l2 + REG_L2_GDC_STATE);
		regval &= ~F_L2_GDC_ST_EVENT_MSK;
		writel(regval, m4u_base_l2 + REG_L2_GDC_STATE);
		return;
	}

	writel(iova_start & F_MMU_VA_MSK, m4u_base_g + REG_MMUG_INVLD_SA);
	writel(iova_end & F_MMU_VA_MSK, m4u_base_g + REG_MMUG_INVLD_EA);
	writel(F_MMUG_INV_RANGE, m4u_base_g + REG_MMUG_INVLD);

	while (!(readl(m4u_base_l2 + REG_L2_GPE_STATUS)
	       & F_L2_GPE_ST_RANGE_INV_DONE)) {
		if (cnt++ >= 10000) {
			dev_err_ratelimited(piommu->dev,
					    "invalid all couldn't be done after 10K times trys\n");
			cnt = 0;
			goto invalid_all;
		}
	}
	regval = readl(m4u_base_l2 + REG_L2_GPE_STATUS);
	regval &= ~F_L2_GPE_ST_RANGE_INV_DONE;
	writel(regval, m4u_base_l2 + REG_L2_GPE_STATUS);
}

irqreturn_t mt2701_iommu_isr(int irq, void *dev_id)
{
	unsigned int port;
	u32 intrsrc, port_regval, fault_iova, fault_pa;
	unsigned int *ptestart;
	void __iomem *m4u_base;
	struct mtk_iommu_domain *mtk_domain = (struct mtk_iommu_domain *)dev_id;
	struct mtk_iommu_info *piommu = mtk_domain->piommuinfo;
	struct device *dev = piommu->dev;

	m4u_base = piommu->base;
	fault_iova = readl(m4u_base + REG_MMU_FAULT_VA);
	fault_iova &= F_MMU_FAULT_VA_MSK;

	fault_pa = readl(m4u_base + REG_MMU_INVLD_PA);
	fault_pa &= F_MMU_FAULT_VA_MSK;

	intrsrc = readl(m4u_base + REG_MMU_FAULT_ST);
	intrsrc &= F_MMU_FAULT_INTR_MSK;
	if (0 == intrsrc) {
		dev_err_ratelimited(dev, "REG_MMU_FAULT_ST = 0x0 fail\n");
		goto imufault;
	}

	port_regval = readl(m4u_base + REG_MMU_INT_ID);
	port = MTK_TF_TO_PORT(piommu, port_regval);
	ptestart = mtk_domain->pgtableva + (fault_iova >> PAGE_SHIFT);

	dev_err_ratelimited(dev, "iommu fault: fault type is %d,  port is %d, iova is 0x%x, pa is 0x%x, *pte is 0x%x\n",
			    intrsrc, port, fault_iova, fault_pa, *ptestart);

	mt2701_iommu_invalidate_tlb(piommu, M4U_ID_ALL, 1, 0, 0);

imufault:
	mt2701_iommu_clear_intr(m4u_base);

	return IRQ_HANDLED;
}

static int mt2701_iommu_parse_dt(struct platform_device *pdev,
				 struct mtk_iommu_info *piommu)
{
	struct device *piommudev = &pdev->dev;
	struct device_node *ofnode;
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	piommu->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(piommu->base)) {
		dev_err(piommudev, "iommu base err 0x%p\n",
			piommu->base);
		return -EINVAL;
	}

	piommu->bclk = devm_clk_get(piommudev, "bclk");
	if (IS_ERR(piommu->bclk)) {
		dev_err(piommudev, "clk err 0x%p\n",
			piommu->bclk);
		return -EINVAL;
	}

	piommu->l2_base = piommu->base + piommu->imucfg->l2_offset;

	ofnode = piommudev->of_node;
	piommu->irq = irq_of_parse_and_map(ofnode, 0);

	return 0;
}

static int mt2701_iommu_hw_init(const struct mtk_iommu_info *piommu)
{
	int ret;
	u32 regval, protectpa;

	ret = clk_prepare_enable(piommu->bclk);
	if (ret) {
		dev_err(piommu->dev, "m4u clk enable error\n");
		return -ENODEV;
	}

	/* pgt base physical address. */
	writel(piommu->pgt_basepa, piommu->base + REG_MMUG_PT_BASE);
	/* clock will be gated when idle. */
	writel(F_MMUG_DCM_ON, piommu->base + REG_MMUG_DCM);

	regval = F_L2_GDC_LOCK_TH(3);
	writel(regval, piommu->l2_base + REG_L2_GDC_OP);

	protectpa = piommu->protect_base;
	protectpa = ALIGN(protectpa, MTK_PROTECT_PA_ALIGN);

	regval = F_MMU_CTRL_TF_PROT_VAL(2)
		 | F_MMU_CTRL_COHERE_EN;
	writel(regval,  piommu->base + REG_MMU_CTRL_REG);

	/* enable interrupt control except "Same VA-to-PA test" */
	regval = F_INT_TRANSLATION_FAULT |  F_INT_TRANSLATION_FAULT
		 | F_INT_TLB_MULTI_HIT_FAULT
		 | F_INT_INVALID_PHYSICAL_ADDRESS_FAULT
		 | F_INT_ENTRY_REPLACEMENT_FAULT | F_INT_TABLE_WALK_FAULT
		 | F_INT_TLB_MISS_FAULT | F_INT_PFH_DMA_FIFO_OVERFLOW
		 | F_INT_MISS_DMA_FIFO_OVERFLOW;
	writel(regval,  piommu->base + REG_MMU_INT_CONTROL);

	/* protect memory,hw will write here while translation fault */
	writel(piommu->protect_base,  piommu->base + REG_MMU_IVRP_PADDR);

	return 0;
}

void mt2701_iommu_hw_deinit(const struct mtk_iommu_info *piommu)
{
	writel(0, piommu->base + REG_MMUG_PT_BASE);
	clk_disable_unprepare(piommu->bclk);
}

static int mt2701_iommu_map(struct mtk_iommu_domain *mtkdomain,
			    unsigned int iova, phys_addr_t paddr, size_t size)
{
	unsigned int i;
	unsigned int page_num = DIV_ROUND_UP(size, PAGE_SIZE);
	u32 *pgt_base_iova;
	u32 pabase = (u32)paddr;

	/*spinlock in upper function*/
	pgt_base_iova = mtkdomain->pgtableva + (iova >> PAGE_SHIFT);
	for (i = 0; i < page_num; i++) {
		pgt_base_iova[i] = pabase
				   | F_DESC_VALID
				   | F_DESC_NONSEC(1)
				   | F_DESC_SHARE(0);
		pabase += PAGE_SIZE;
	}
	mt2701_iommu_invalidate_tlb(mtkdomain->piommuinfo, M4U_ID_ALL, 0,
				    iova, iova + size - 1);
	return 0;
}

static size_t mt2701_iommu_unmap(struct mtk_iommu_domain *mtkdomain,
				 unsigned int iova, size_t size)
{
	unsigned int page_num = DIV_ROUND_UP(size, PAGE_SIZE);
	unsigned int *pgt_base_iova;

	/*spinlock in upper function*/
	pgt_base_iova = mtkdomain->pgtableva + (iova >> PAGE_SHIFT);
	memset(pgt_base_iova, 0, page_num * sizeof(int));

	mt2701_iommu_invalidate_tlb(mtkdomain->piommuinfo, M4U_ID_ALL, 0,
				    iova, iova + size - 1);
	return size;
}

static phys_addr_t mt2701_iommu_iova_to_phys(struct mtk_iommu_domain *mtkdomain,
					     unsigned int iova)
{
	u32 phys;

	phys = *(mtkdomain->pgtableva + (iova >> PAGE_SHIFT));
	return (phys_addr_t)(phys & PAGE_MASK);
}

const struct mtk_iommu_cfg mtk_iommu_mt2701_cfg = {
	.l2_offset		= REG_L2_OFFSET,
	.m4u_port_in_larbx	= {0, 11, 21, 44},
	.m4u_port_nr		= 44,
	.dt_parse		= mt2701_iommu_parse_dt,
	.hw_init		= mt2701_iommu_hw_init,
	.hw_deinit		= mt2701_iommu_hw_deinit,
	.map			= mt2701_iommu_map,
	.unmap			= mt2701_iommu_unmap,
	.iova_to_phys		= mt2701_iommu_iova_to_phys,
	.config_port		= mt2701_iommu_config_port,
	.iommu_isr		= mt2701_iommu_isr,
	.invalid_tlb		= mt2701_iommu_invalidate_tlb,
	.clear_intr		= mt2701_iommu_clear_intr,
};
