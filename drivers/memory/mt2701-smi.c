/*
 * Copyright (c) 2014-2015 MediaTek Inc.
 * Author:	Yong Wu <yong.wu@mediatek.com>
 *	 :	Honghui Zhang <honghui.zhang@mediatek.com>
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
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <soc/mediatek/mtk-smi.h>
#include <linux/module.h>

#define F_SMI_MMU_EN(port)			(1 << (port))
#define REG_SMI_SECUR_CON(x)			(0x5C0 + ((x) << 2))

/* every register controls 8 ports */
#define REG_SMI_SECUR_CON_OF_PORT(port)		REG_SMI_SECUR_CON(port >> 3)

/* bit[port] is controls secure bit*/
#define F_SMI_SECUR_CON_SECURE(port)		(1 << ((port & 0x7) << 2))

/* for mt2701, domain will always be 3, means multi-media domain */
/* bit[port + 2]:[port + 1] */
#define F_SMI_SECUR_CON_DOMAIN(port, val)	(((val) & 0x3)\
		<< (((port & 0x7) << 2) + 1))

/* bit[port + 3] controls the virtual physical */
#define F_SMI_SECUR_CON_VIRTUAL(port)		(1 << (((port & 0x7) << 2) + 3))
#define F_SMI_SECUR_CON_MASK(port)		(0xF << ((port & 0x7) << 2))

struct mtk_smi_common {
	void __iomem	*base;
	struct clk	*clk_apb;
	struct clk	*clk_smi;
};

struct mtk_smi_larb {
	void __iomem	*base;
	struct clk	*clk_smi;
	struct device	*smi;
};

struct device *smi_device;

static int mtk_smi_common_get(struct device *smidev)
{
	struct mtk_smi_common *smipriv = dev_get_drvdata(smidev);
	int ret;

	ret = clk_enable(smipriv->clk_smi);
	if (ret)
		dev_err(smidev, "Failed to enable the smi clock\n");

	return ret;
}

static void mtk_smi_common_put(struct device *smidev)
{
	struct mtk_smi_common *smipriv = dev_get_drvdata(smidev);

	clk_disable(smipriv->clk_smi);
}

int mtk_smi_larb_get(struct device *larbdev)
{
	int ret;
	struct mtk_smi_larb *larbpriv = dev_get_drvdata(larbdev);

	ret = mtk_smi_common_get(larbpriv->smi);
	if (ret)
		return ret;

	ret = clk_enable(larbpriv->clk_smi);
	if (ret) {
		dev_err(larbdev, "Failed to enable the larb smi clock\n");
		goto error;
	}

	return ret;

error:
	mtk_smi_common_put(larbpriv->smi);
	return ret;
}

void mtk_smi_larb_put(struct device *larbdev)
{
	struct mtk_smi_larb *larbpriv = dev_get_drvdata(larbdev);

	clk_disable(larbpriv->clk_smi);
	mtk_smi_common_put(larbpriv->smi);
}

int mtk_smi_config_port(struct device *larbdev, unsigned int larbportid,
			bool iommuen)
{
	int ret;
	u32 regval, sec_con_val;
	struct mtk_smi_common *smipriv = dev_get_drvdata(smi_device);

	dev_dbg(larbdev, "smi-config port id %d\n", larbportid);

	ret = mtk_smi_common_get(smi_device);
	if (ret)
		return ret;

	if (iommuen)
		sec_con_val = F_SMI_SECUR_CON_VIRTUAL(larbportid);
	else
		sec_con_val = 0;

	regval = readl(smipriv->base + REG_SMI_SECUR_CON_OF_PORT(larbportid));
	regval &= (~F_SMI_SECUR_CON_MASK(larbportid));
	regval |= sec_con_val;

	writel(regval,
	       smipriv->base + REG_SMI_SECUR_CON_OF_PORT(larbportid));

	mtk_smi_common_put(smi_device);

	return 0;
}

static int mtk_smi_larb_probe(struct platform_device *pdev)
{
	int ret;
	struct mtk_smi_larb *larbpriv;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct device_node *smi_node;
	struct platform_device *smi_pdev;

	larbpriv = devm_kzalloc(dev, sizeof(struct mtk_smi_larb), GFP_KERNEL);
	if (!larbpriv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	larbpriv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(larbpriv->base))
		return PTR_ERR(larbpriv->base);

	larbpriv->clk_smi = devm_clk_get(dev, "smi");
	if (IS_ERR(larbpriv->clk_smi))
		return PTR_ERR(larbpriv->clk_smi);

	ret = clk_prepare(larbpriv->clk_smi);
	if (ret) {
		dev_err(dev, "Failed to prepare smi clock 0x%x\n", ret);
		return ret;
	}

	smi_node = of_parse_phandle(dev->of_node, "smi", 0);
	if (!smi_node) {
		dev_err(dev, "Failed to get smi node\n");
		ret = -EINVAL;
		goto fail_clk_smi;
	}

	smi_pdev = of_find_device_by_node(smi_node);
	of_node_put(smi_node);
	if (smi_pdev) {
		larbpriv->smi = &smi_pdev->dev;
	} else {
		dev_err(dev, "Failed to get the smi_common device\n");
		ret = -EINVAL;
		goto fail_clk_smi;
	}

	dev_set_drvdata(dev, larbpriv);
	return 0;

fail_clk_smi:
	clk_unprepare(larbpriv->clk_smi);
	return ret;
}

static const struct of_device_id mtk_smi_larb_of_ids[] = {
	{ .compatible = "mediatek,mt2701-smi-larb",
	},
	{ .compatible = "mediatek,mt8127-smi-larb",
	},
	{}
};

static struct platform_driver mtk_smi_larb_driver = {
	.probe	= mtk_smi_larb_probe,
	.driver	= {
		.name = "mtk-smi-larb",
		.of_match_table = mtk_smi_larb_of_ids,
	}
};

static int mtk_smi_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct mtk_smi_common *smipriv;

	smipriv = devm_kzalloc(dev, sizeof(*smipriv), GFP_KERNEL);
	if (!smipriv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	smipriv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(smipriv->base))
		return PTR_ERR(smipriv->base);

	smipriv->clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(smipriv->clk_apb))
		return PTR_ERR(smipriv->clk_apb);

	ret = clk_prepare(smipriv->clk_smi);
	if (ret) {
		dev_err(dev, "Failed to prepare smi apb clock 0x%x\n", ret);
		return ret;
	}

	smipriv->clk_smi = devm_clk_get(dev, "smi");
	if (IS_ERR(smipriv->clk_smi)) {
		ret = PTR_ERR(smipriv->clk_smi);
		goto error_clk;
	}

	ret = clk_prepare(smipriv->clk_smi);
	if (ret) {
		dev_err(dev, "Failed to prepare smi clock 0x%x\n", ret);
		goto error_clk;
	}

	dev_set_drvdata(dev, smipriv);
	/* the apb clock of smi will be always on.*/
	clk_enable(smipriv->clk_apb);
	smi_device = dev;
	return ret;

error_clk:
	clk_unprepare(smipriv->clk_apb);
	return ret;
}

static const struct of_device_id mtk_smi_of_ids[] = {
	{ .compatible = "mediatek,mt2701-smi", },
	{}
};

static struct platform_driver mtk_smi_driver = {
	.probe	= mtk_smi_probe,
	.driver	= {
		.name = "mtk-smi",
		.of_match_table = mtk_smi_of_ids,
	}
};

static int __init mtk_smi_init(void)
{
	int ret;

	ret = platform_driver_register(&mtk_smi_driver);
	if (ret)
		return ret;

	ret = platform_driver_register(&mtk_smi_larb_driver);
	if (ret)
		goto fail_smi_larb;

	return ret;

fail_smi_larb:
	platform_driver_unregister(&mtk_smi_driver);
	return ret;
}

subsys_initcall(mtk_smi_init);

MODULE_AUTHOR("Yong Wu <yong.wu@mediatek.com>");
MODULE_AUTHOR("Honghui Zhang <honghui.zhang@mediatek.com>");
MODULE_DESCRIPTION("SMI (Smart Muti-media Interface) API for MTK architected implementations");
MODULE_LICENSE("GPL v2");
