/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include "mt2701-aud-global.h"
#ifdef CONFIG_MTK_LEGACY_CLOCK
#include <mach/mt_clkmgr.h>
#else
#include "mt2701-afe-clk.h"
#endif
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/device.h>
/*#include <mach/mt_gpio.h>*/
#include "mt2701-afe.h"
#include "mt2701-afe-reg.h"
#include "mt2701-aud-gpio.h"


#ifdef AUDIO_MEM_IOREMAP
/* address for ioremap audio hardware register */
void *afe_base_address;
void *afe_sram_address;
void *topckgen_base_address;
void *cmsys_base_address;
void *infracfg_base_address;
void *pctrl_base_address;
void *afe_sram_phy_address;

#endif
#ifdef CONFIG_OF
static unsigned int afe_irq_id;
static unsigned int asys_irq_id;
#endif

void mt_afe_reg_unmap(void)
{
#ifdef AUDIO_MEM_IOREMAP

	if (afe_base_address) {
		iounmap(afe_base_address);
		afe_base_address = NULL;
	}
	if (afe_sram_address) {
		iounmap(afe_sram_address);
		afe_sram_address = NULL;
	}
	if (topckgen_base_address) {
		iounmap(topckgen_base_address);
		topckgen_base_address = NULL;
	}
	if (cmsys_base_address) {
		iounmap(cmsys_base_address);
		cmsys_base_address = NULL;
	}
#endif

}

int mt_afe_reg_remap(void *dev)
{
	int ret = 0;
#ifdef AUDIO_IOREMAP_FROM_DT
	struct device *pdev = dev;
	struct resource res;
	struct device_node *node;

	/* AFE register base */
	ret = of_address_to_resource(pdev->of_node, 0, &res);
	if (ret) {
		pr_err("%s of_address_to_resource#0 fail %d\n", __func__, ret);
		goto exit;
	}

	afe_base_address = ioremap_nocache(res.start, resource_size(&res));
	if (!afe_base_address) {
		pr_err("%s ioremap_nocache#0 addr:0x%llx size:0x%llx fail\n",
		       __func__, (unsigned long long)res.start,
		       (unsigned long long)resource_size(&res));
		ret = -ENXIO;
		goto exit;
	}
	pr_debug("afe_base_address = 0x%x from 0x%x\n", afe_base_address, res.start);

	/* audio SRAM base */
	ret = of_address_to_resource(pdev->of_node, 1, &res);
	if (ret) {
		pr_err("%s of_address_to_resource#1 fail %d\n", __func__, ret);
		goto exit;
	}

	afe_sram_address = ioremap_nocache(res.start, resource_size(&res));
	if (!afe_sram_address) {
		pr_err("%s ioremap_nocache#1 addr:0x%llx size:0x%llx fail\n",
		       __func__, (unsigned long long)res.start,
		       (unsigned long long)resource_size(&res));
		ret = -ENXIO;
		goto exit;
	}
	afe_sram_phy_address = res.start;

	/* TOPCKGEN register base */
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt2701-topckgen"); /* need to check */
	if (!node) {
		pr_warn("%s of_find_compatible_node(mediatek,mt2701-topckgen) fail\n", __func__);
		topckgen_base_address = ioremap_nocache(TOPCKGEN_BASE_ADDR, 0x1000);
	} else {
		topckgen_base_address = of_iomap(node, 0);
	}

	if (!topckgen_base_address) {
		pr_err("%s ioremap topckgen_base_address fail\n", __func__);
		ret = -ENODEV;
		goto exit;
	}
	pr_debug("topckgen_base_address = 0x%x\n", topckgen_base_address);

	/* todo for cmsys: need to find the device tree node for CMSYS*/
	cmsys_base_address = ioremap_nocache(CMSYS_BASE_ADDR, 0x1000);
	pr_debug("cmsys_base_address = 0x%x\n", cmsys_base_address);

	infracfg_base_address = ioremap_nocache(INFRACFG_BASE_ADDR, 0x1000);
	pr_debug("infracfg_base_address = 0x%x\n", infracfg_base_address);

	pctrl_base_address = ioremap_nocache(PCTRL_BASE_ADDR, 0x2000);
	pr_debug("pctrl_base_address = 0x%x\n", pctrl_base_address);
	pr_debug("afe_sram_address = 0x%x from 0x%x\n", afe_sram_address, afe_sram_phy_address);

exit:
	if (ret)
		mt_afe_reg_unmap();
#else
	afe_sram_address = ioremap_nocache(AFE_INTERNAL_SRAM_PHYS_BASE, AFE_INTERNAL_SRAM_SIZE);
	pr_warn("afe_sram_address = 0x%x\n", afe_sram_address);
	afe_base_address = ioremap_nocache(AUDIO_HW_PHYS_BASE_ADDR, 0x2000);
	pr_warn("afe_base_address = 0x%x\n", afe_base_address);
	/*pr_warn("afe_base_address = 0x%x\n", IO_PHYS_TO_VIRT((u32)(AUDIO_HW_PHYS_BASE_ADDR)));*/
	/*spm_base_address = ioremap_nocache(SPM_BASE, 0x1000);*/
	topckgen_base_address = ioremap_nocache(TOPCKGEN_BASE_ADDR, 0x1000);
	pr_warn("topckgen_base_address = 0x%x\n", topckgen_base_address);
	/*apmixedsys_base_address = ioremap_nocache(APMIXEDSYS_BASE, 0x1000);*/
	cmsys_base_address = ioremap_nocache(CMSYS_BASE_ADDR, 0x1000);
	pr_warn("cmsys_base_address = 0x%x\n", cmsys_base_address);
#endif
	return ret;
}



int mt_afe_platform_init(void *dev)
{
	struct device *pdev = dev;
	int ret = 0;

#ifdef CONFIG_OF
	unsigned int irq_id;

	if (!pdev->of_node) {
		pr_warn("%s invalid of_node\n", __func__);
		return -ENODEV;
	}

	irq_id = irq_of_parse_and_map(pdev->of_node, 0);
	if (irq_id)
		afe_irq_id = irq_id;
	else
		pr_warn("%s irq_of_parse_and_map invalid irq\n", __func__);

	irq_id = irq_of_parse_and_map(pdev->of_node, 1);
	if (irq_id)
		asys_irq_id = irq_id;
	else
		pr_warn("%s irq_of_parse_and_map invalid irq\n", __func__);


#endif
	ret = mt_afe_reg_remap(dev);
	if (ret)
		return ret;

	ret = mt_afe_init_clock(dev);
	if (ret)
		return ret;

	return ret;

}

void mt_afe_platform_deinit(void *dev)
{
	mt_afe_reg_unmap();
	mt_afe_deinit_clock(dev);
}

#ifdef CONFIG_OF
unsigned int mt_afe_get_afe_irq_id(void)
{
	return afe_irq_id;
}

unsigned int mt_afe_get_asys_irq_id(void)
{
	return asys_irq_id;
}
#endif

static inline void afe_set_bit(u32 addr, int bit)
{
	afe_msk_write(addr, 0x1 << bit, 0x1 << bit);
}

static inline void afe_clear_bit(u32 addr, int bit)
{
	afe_msk_write(addr, 0x0, 0x1 << bit);
}

static inline void afe_write_bits(u32 addr, u32 value, int bits, int len)
{
	u32 u4TargetBitField = ((0x1 << len) - 1) << bits;
	u32 u4TargetValue = (value << bits) & u4TargetBitField;
	u32 u4CurrValue;

	u4CurrValue = afe_read(addr);
	afe_write(addr, ((u4CurrValue & (~u4TargetBitField)) | u4TargetValue));
}

static inline u32 afe_read_bits(u32 addr, int bits, int len)
{
	u32 u4TargetBitField = ((0x1 << len) - 1) << bits;
	u32 u4CurrValue = afe_read(addr);

	return (u4CurrValue & u4TargetBitField) >> bits;
}

void afe_enable(int en)
{
	int i;

	#ifdef CONFIG_MTK_LEGACY_CLOCK
	enum cg_clk_id clks[] = {
		MT_CG_INFRA_AUDIO,
		MT_CG_AUDIO_AFE,		/*AUDIO_TOP_CON0[2]*/
		MT_CG_AUDIO_APLL,		/*AUDIO_TOP_CON0[19]*/
		MT_CG_AUDIO_A1SYS,		/*AUDIO_TOP_CON4[21]*/
		MT_CG_AUDIO_A2SYS,		/*AUDIO_TOP_CON4[22]*/
		MT_CG_AUDIO_AFE_CONN,	/*AUDIO_TOP_CON4[23]*/
	};

	for (i = 0; i < 6; ++i)
		afe_i2s_power_on_mclk(i, 0);
	#else
	for (i = 0; i < 6; ++i)
		mt_i2s_power_on_mclk(i, 0);
	#endif

	if (en) {
		#ifdef CONFIG_MTK_LEGACY_CLOCK
		enable_pll(AUD1PLL, "AUDIO");
		enable_pll(AUD2PLL, "AUDIO");
		enable_pll(HADDS2PLL, "AUDIO");
		for (i = 0; i < ARRAY_SIZE(clks); ++i)
			enable_clock(clks[i], "AUDIO");
		#else
		mt_afe_a1sys_hp_ck_on();
		mt_afe_a2sys_hp_ck_on();
		mt_afe_main_clk_on();
		#endif
		afe_msk_write(ASYS_TOP_CON, A1SYS_TIMING_ON, A1SYS_TIMING_ON_MASK);
		/* i2s-out1/2/3/4/5/6 don't select sine-wave gen as source */
		afe_msk_write(AFE_SGEN_CON0, (0x1F << 27), (0x1F << 27));
	#ifdef AUDIO_MEM_IOREMAP
		#ifdef CONFIG_MTK_LEGACY_CLOCK  /* Controlled by CCF*/
		/* 0:26M, 1:APLL1(98.304M), 2:APLL2(90.3168M), 3:HADDS, 4:EXT1, 5:EXT2 */
		topckgen_msk_write(CLK_AUDDIV_3, 0x1 << 0, AUD_MUX1_CLK_MASK);	/* 48K domain */
		topckgen_msk_write(CLK_AUDDIV_3, 0x2 << 3, AUD_MUX2_CLK_MASK);	/* 44.1K domain */
		/* topckgen_msk_write(CLK_AUDDIV_3, 0x3<<6, AUD_HADDS_CLK_MASK);//Hadds */
		topckgen_msk_write(CLK_AUDDIV_0, 0x1 << 16, AUD_A1SYS_K1_MASK);	/* APLL1 DIV Fix:APLL1/2 */
		topckgen_msk_write(CLK_AUDDIV_0, 0x1 << 24, AUD_A2SYS_K1_MASK);	/* APLL2 DIV Fix:APLL2/2 */
		/* APLL1 CK ENABLE  current apll1 & apll2 all use bit21, ECO will modify apll2 to bit22 */
		topckgen_msk_write(CLK_AUDDIV_3, AUD_A1SYS_K1_ON, AUD_A1SYS_K1_ON_MASK);
		#endif
	#else
		/* 0:26M, 1:APLL1(98.304M), 2:APLL2(90.3168M), 3:HADDS, 4:EXT1, 5:EXT2 */
		afe_msk_write(CLK_AUDDIV_3, 0x1 << 0, AUD_MUX1_CLK_MASK);	/* 48K domain */
		afe_msk_write(CLK_AUDDIV_3, 0x2 << 3, AUD_MUX2_CLK_MASK);	/* 44.1K domain */
		/* afe_msk_write(CLK_AUDDIV_3, 0x3<<6, AUD_HADDS_CLK_MASK); */	/* Hadds */
		afe_msk_write(CLK_AUDDIV_0, 0x1 << 16, AUD_A1SYS_K1_MASK);	/* APLL1 DIV Fix:APLL1/2 */
		afe_msk_write(CLK_AUDDIV_0, 0x1 << 24, AUD_A2SYS_K1_MASK);	/* APLL2 DIV Fix:APLL2/2 */
		/* APLL1 CK ENABLE  current apll1 & apll2 all use bit21, ECO will modify apll2 to bit22 */
		afe_msk_write(CLK_AUDDIV_3, AUD_A1SYS_K1_ON, AUD_A1SYS_K1_ON_MASK);
	#endif
		afe_write(PWR1_ASM_CON1, 0x492);	/* i2s asrc clk from 208M */
		afe_write(PWR2_ASM_CON1, 0x492492);	/* i2s asrc clk from 208M */

	#ifndef AUD_PINCTRL_SUPPORTING
		#ifdef AUDIO_MEM_IOREMAP
		/* boost i2s0   mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		topckgen_write(BOOST_DRIVING_I2S0, 0x0040);
		/* boost i2s1/2 mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		topckgen_write(BOOST_DRIVING_I2S12, 0x0044);
		/* boost i2s3/4 mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		topckgen_write(BOOST_DRIVING_I2S34, 0x4400);
		/* boost i2s5   mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		topckgen_write(BOOST_DRIVING_I2S5, 0x0004);
		/* boost spdif out's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		topckgen_write(BOOST_DRIVING_SPDIF, 0x0004);
		#else
		/* boost i2s0   mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		afe_write(BOOST_DRIVING_I2S0, 0x0040);
		/* boost i2s1/2 mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		afe_write(BOOST_DRIVING_I2S12, 0x0044);
		/* boost i2s3/4 mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		afe_write(BOOST_DRIVING_I2S34, 0x4400);
		/* boost i2s5   mclk,bck,lrck's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		afe_write(BOOST_DRIVING_I2S5, 0x0004);
		/* boost            spdif out's driving to 0(4mA),2(8mA),4(12mA),6(16mA) */
		afe_write(BOOST_DRIVING_SPDIF, 0x0004);
		#endif
	#endif
		/* for hdmi-tx */
		afe_msk_write(AUDIO_TOP_CON0, APB3_SEL, APB3_SEL | APB_R2T | APB_W2T);	/* set bus */
		afe_msk_write(AFE_DAC_CON0, AFE_ON, AFE_ON_MASK);
		afe_hwgain_init(AFE_HWGAIN_1);
	} else {
		afe_msk_write(AFE_DAC_CON0, AFE_OFF, AFE_ON_MASK);
		#ifdef CONFIG_MTK_LEGACY_CLOCK
		for (i = 0; i < ARRAY_SIZE(clks); ++i)
			disable_clock(clks[i], "AUDIO");
		disable_pll(HADDS2PLL, "AUDIO");
		disable_pll(AUD2PLL, "AUDIO");
		disable_pll(AUD1PLL, "AUDIO");
		#else
		mt_afe_main_clk_off();
		mt_afe_a1sys_hp_ck_off();
		mt_afe_a2sys_hp_ck_off();
		#endif
	}
}

void afe_power_mode(enum power_mode mode)
{
	u32 val = (mode == LOW_POWER_MODE)
		  ? CLK_AUD_INTBUS_SEL_CLK26M : CLK_AUD_INTBUS_SEL_SYSPLL1D4;

	#ifdef AUDIO_MEM_IOREMAP
		#ifdef CONFIG_MTK_LEGACY_CLOCK
		topckgen_msk_write(CLK_CFG_3, val, CLK_AUD_INTBUS_SEL_MASK);
		#else
		/* TODO */
		#endif
	#else
	afe_msk_write(CLK_CFG_3, val, CLK_AUD_INTBUS_SEL_MASK);
	#endif
}

static void trigger_cm4_soft0_irq(void)
{
	/* trigger soft0_irq_b to CM4 */
	#ifdef AUDIO_MEM_IOREMAP
	cmsys_msk_write(0x10, 0x0 << 2, 0x1 << 2);
	#else
	afe_msk_write(CMSYS_BASE + 0x10, 0x0 << 2, 0x1 << 2);
	#endif
}

static int is_cm4_soft0_irq_cleared(void)
{
	#ifdef AUDIO_MEM_IOREMAP
	return (cmsys_read(0x10) & (0x1 << 2)) >> 2;
	#else
	return (afe_read(CMSYS_BASE + 0x10) & (0x1 << 2)) >> 2;
	#endif
}

unsigned char *afe_sram_virt(void)
{
	#ifdef AUDIO_MEM_IOREMAP
	return afe_sram_address;
	#else
	return (unsigned char *)IO_PHYS_TO_VIRT(AFE_INTERNAL_SRAM_PHYS_BASE);
	#endif
}

u32 afe_sram_phys(void)
{
	return AFE_INTERNAL_SRAM_PHYS_BASE;
}

size_t afe_sram_size(void)
{
	return AFE_INTERNAL_SRAM_SIZE;
}

void afe_spdif_out2_source_sel(enum spdif_out2_source s)
{
	switch (s) {
	case SPDIF_OUT2_SOURCE_IEC2:
		afe_msk_write(AFE_SPDIFIN_CFG1, (0x0 << 11), (0x1 << 11));
		break;
	case SPDIF_OUT2_SOURCE_OPTICAL_IN:
		afe_msk_write(AFE_SPDIFIN_BR, (0x1 << 25), (0x1 << 25));
		afe_msk_write(AFE_SPDIFIN_CFG1, (0x1 << 11), (0x1 << 11));
		break;
	case SPDIF_OUT2_SOURCE_COAXIAL_IN:
		afe_msk_write(AFE_SPDIFIN_BR, (0x0 << 25), (0x0 << 25));
		afe_msk_write(AFE_SPDIFIN_CFG1, (0x1 << 11), (0x1 << 11));
		break;
	default:
		break;
	}
}

/******************** interconnection ********************/

int itrcon_connect(enum itrcon_in in, enum itrcon_out out, int connect)
{
	u32 addr;
	unsigned int bit_pos;

	connect = !!connect;
	if (out > O33) {
		pr_err("%s() error: bad output port %d\n", __func__, out);
		return -EINVAL;
	}
	if (in <= I31) {
		addr = AFE_CONN0 + out * 4;
		bit_pos = in;
	} else if (I32 <= in && in <= I36) {
		/* AFE_CONN35: O04 O03 O02 O01 O00
		   AFE_CONN36: O09 O08 O07 O06 O05
		   ...
		   AFE_CONN41:     O33 O32 O31 O30 */
		addr = AFE_CONN35 + (out / 5) * 4;
		bit_pos = (out % 5) * 6 + (in - I32);
	} else {
		pr_err("%s() error: bad input port %d\n", __func__, in);
		return -EINVAL;
	}
	pr_debug("%s() I_%d -> O_%d\n", connect ? "connect" : "disconnect", in, out);
	afe_msk_write(addr, connect << bit_pos, 0x1 << bit_pos);
	return 0;
}

void itrcon_disconnectall(void)
{
	u32 addr;

	pr_debug("%s()\n", __func__);
	for (addr = AFE_CONN0; addr <= AFE_CONN33; addr += 4)
		afe_write(addr, 0x0);
	for (addr = AFE_CONN35; addr <= AFE_CONN40; addr += 4)
		/* clear BIT29 ~ BIT0 */
		afe_msk_write(addr, 0x0, 0x3fffffff);
	/* clear BIT23 ~ BIT0 */
	afe_msk_write(AFE_CONN41, 0x0, 0xffffff);
}

int itrcon_rightshift1bit(enum itrcon_out out, int shift)
{
	u32 addr;
	unsigned int bit_pos;

	shift = !!shift;
	pr_debug("%s() %s right shift 1 bit for O_%d\n", __func__, shift ? "enable" : "disable",
		 out);
	if (out <= O31) {
		addr = AFE_CONN34;
		bit_pos = out;
	} else if (O32 <= out && out <= O33) {
		addr = AFE_CONN41;
		bit_pos = out - O32 + 24;
	} else {
		pr_err("%s() bad output port: %d\n", __func__, out);
		return -EINVAL;
	}
	afe_msk_write(addr, shift << bit_pos, 0x1 << bit_pos);
	return 0;
}

void itrcon_noshiftall(void)
{
	pr_debug("%s()\n", __func__);
	afe_write(AFE_CONN34, 0x0);
	/* clear BIT25 BIT24 */
	afe_msk_write(AFE_CONN41, 0x0, 0x3000000);
}


/******************** memory interface ********************/

int afe_memif_enable(enum afe_mem_interface memif, int en)
{
	u32 val, msk;

	en = !!en;
	val = (en << memif);
	msk = (0x1 << memif);
	if (memif == AFE_MEM_NONE || memif == AFE_MEM_RESERVED) {
		pr_err("%s() invalid memif\n", __func__);
		return -EINVAL;
	}
	if (memif == AFE_MEM_DLMCH) {	/* jiechao.wei said if you want to use DLMCH, DL1 should be enabled too */
		val |= (en << AFE_MEM_DL1);
		msk |= (0x1 << AFE_MEM_DL1);
	}
	afe_msk_write(AFE_DAC_CON0, val, msk);
	if (AFE_MEM_ULMCH == memif)
		afe_multilinein_enable(en);
	return 0;
}

int afe_memif_configurate(enum afe_mem_interface memif, const struct afe_memif_config *config)
{
	u32 base, end;
	static const char *name[AFE_MEM_NUM] = {
		"AFE_MEM_NONE"	/* 0 */
		, "AFE_MEM_DL1"	/* 1 */
		, "AFE_MEM_DL2"	/* 2 */
		, "AFE_MEM_DL3"	/* 3 */
		, "AFE_MEM_DL4"	/* 4 */
		, "AFE_MEM_DL5"	/* 5 */
		, "AFE_MEM_DL6"	/* 6 */
		, "AFE_MEM_DLMCH"	/* 7 */
		, "AFE_MEM_ARB1"	/* 8 */
		, "AFE_MEM_DSDR"	/* 9 */
		, "AFE_MEM_UL1"	/* 10 */
		, "AFE_MEM_UL2"	/* 11 */
		, "AFE_MEM_UL3"	/* 12 */
		, "AFE_MEM_UL4"	/* 13 */
		, "AFE_MEM_UL5"	/* 14 */
		, "AFE_MEM_UL6"	/* 15 */
		, "AFE_MEM_ULMCH"	/* 16 */
		, "AFE_MEM_DAI"	/* 17 */
		, "AFE_MEM_MOD_PCM"	/* 18 */
		, "reserved"	/* 19 */
		, "AFE_MEM_AWB"	/* 20 */
		, "AFE_MEM_AWB2"	/* 21 */
		, "AFE_MEM_DSDW"	/* 22 */
	};

	if (config == NULL)
		return -ENOMEM;
	base = config->buffer.base;
	end = base + config->buffer.size - 1;
	if (memif > AFE_MEM_DSDW || memif == AFE_MEM_NONE || memif == AFE_MEM_RESERVED) {
		pr_err("%s() invalid memif %d\n", __func__, memif);
		return -EINVAL;
	}
	pr_debug("%s() memif=%s, base=0x%x, size=0x%x, fs=%u\n", __func__, name[memif], base,
		 config->buffer.size, (memif == AFE_MEM_DAI || memif == AFE_MEM_MOD_PCM)
		 ? (config->daimod_fs + 1) * 8000 : fs_integer(config->fs));
	if (config->buffer.size == 0 || (base & 0xF) != 0x0 || (end & 0xF) != 0xF) {
		pr_err("%s() invalid buffer: base=0x%08x, end=0x%08x\n", __func__, base, end);
		return -EINVAL;
	}
	switch (memif) {
	case AFE_MEM_DL1:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL1_HD_AUDIO_ON, DL1_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL1_HD_AUDIO_OFF, DL1_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 0), DL1_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 24) | (config->channel << 16),
			      DL1_MSB_OR_LSB_FIRST_MASK | DL1_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 0), DL1_DSD_WIDTH_MASK);
		afe_write(AFE_DL1_BASE, base);
		afe_write(AFE_DL1_END, end);
		break;
	case AFE_MEM_DL2:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL2_HD_AUDIO_ON, DL2_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL2_HD_AUDIO_OFF, DL2_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 5), DL2_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 25) | (config->channel << 17),
			      DL2_MSB_OR_LSB_FIRST_MASK | DL2_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 2), DL2_DSD_WIDTH_MASK);
		afe_write(AFE_DL2_BASE, base);
		afe_write(AFE_DL2_END, end);
		break;
	case AFE_MEM_DL3:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL3_HD_AUDIO_ON, DL3_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL3_HD_AUDIO_OFF, DL3_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 10), DL3_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 26) | (config->channel << 18),
			      DL3_MSB_OR_LSB_FIRST_MASK | DL3_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 4), DL3_DSD_WIDTH_MASK);
		afe_write(AFE_DL3_BASE, base);
		afe_write(AFE_DL3_END, end);
		break;
	case AFE_MEM_DL4:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL4_HD_AUDIO_ON, DL4_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL4_HD_AUDIO_OFF, DL4_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 15), DL4_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 27) | (config->channel << 19),
			      DL4_MSB_OR_LSB_FIRST_MASK | DL4_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 6), DL4_DSD_WIDTH_MASK);
		afe_write(AFE_DL4_BASE, base);
		afe_write(AFE_DL4_END, end);
		break;
	case AFE_MEM_DL5:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL5_HD_AUDIO_ON, DL5_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL5_HD_AUDIO_OFF, DL5_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 20), DL5_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 28) | (config->channel << 20),
			      DL5_MSB_OR_LSB_FIRST_MASK | DL5_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 8), DL5_DSD_WIDTH_MASK);
		afe_write(AFE_DL5_BASE, base);
		afe_write(AFE_DL5_END, end);
		break;
	case AFE_MEM_DL6:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_PAIR_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DL6_HD_AUDIO_ON, DL6_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DL6_HD_AUDIO_OFF, DL6_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON1, (config->fs << 25), DL6_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 29) | (config->channel << 21),
			      DL6_MSB_OR_LSB_FIRST_MASK | DL6_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 10), DL6_DSD_WIDTH_MASK);
		afe_write(AFE_DL6_BASE, base);
		afe_write(AFE_DL6_END, end);
		break;
	case AFE_MEM_DLMCH:
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_OUT_SEL_FULL_INTERLEAVE,
			      DLMCH_OUT_SEL_MASK);
		/* jiechao.wei solves zero data problem */
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, DLMCH_PBUF_SIZE_32BYTES, DLMCH_PBUF_SIZE_MASK);
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, (config->hd_audio << 28), DLMCH_BIT_WIDTH_MASK);
		/* jiechao.wei said DLMCH uses DL1's mode */
		afe_msk_write(AFE_DAC_CON1, (config->fs << 0), DL1_MODE_MASK);
		afe_msk_write(AFE_MEMIF_PBUF_SIZE, (config->dlmch_ch_num << 24), DLMCH_CH_NUM_MASK);
		afe_write(AFE_DLMCH_BASE, base);
		afe_write(AFE_DLMCH_END, end);
		break;
	case AFE_MEM_ARB1:
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, ARB1_HD_AUDIO_ON, ARB1_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, ARB1_HD_AUDIO_OFF, ARB1_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->fs << 10), ARB1_MODE_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->channel << 22), ARB1_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 12), ARB1_DSD_WIDTH_MASK);
		afe_write(AFE_ARB1_BASE, base);
		afe_write(AFE_ARB1_END, end);
		break;
	case AFE_MEM_DSDR:
		if (config->hd_audio)
			afe_msk_write(AFE_MEMIF_HD_CON0, DSDR_HD_AUDIO_ON, DSDR_HD_AUDIO_MASK);
		else
			afe_msk_write(AFE_MEMIF_HD_CON0, DSDR_HD_AUDIO_OFF, DSDR_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 30) | (config->channel << 23),
			      DSDR_MSB_OR_LSB_FIRST_MASK | DSDR_DATA_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 14), DSDR_DSD_WIDTH_MASK);
		afe_write(AFE_DSDR_BASE, base);
		afe_write(AFE_DSDR_END, end);
		break;
	case AFE_MEM_UL1:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 0), VUL_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 24) | (config->channel << 0),
			      VUL_MSB_OR_LSB_FIRST_MASK | VUL_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 1), VUL_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 0), VUL_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 16), VUL_DSD_WIDTH_MASK);
		afe_write(AFE_VUL_BASE, base);
		afe_write(AFE_VUL_END, end);
		break;
	case AFE_MEM_UL2:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 5), UL2_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 25) | (config->channel << 2),
			      UL2_MSB_OR_LSB_FIRST_MASK | UL2_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 3), UL2_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 2), UL2_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 18), UL2_DSD_WIDTH_MASK);
		afe_write(AFE_UL2_BASE, base);
		afe_write(AFE_UL2_END, end);
		break;
	case AFE_MEM_UL3:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 10), UL3_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 26) | (config->channel << 4),
			      UL3_MSB_OR_LSB_FIRST_MASK | UL3_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 5), UL3_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 4), UL3_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 20), UL3_DSD_WIDTH_MASK);
		afe_write(AFE_UL3_BASE, base);
		afe_write(AFE_UL3_END, end);
		break;
	case AFE_MEM_UL4:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 15), UL4_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 27) | (config->channel << 6),
			      UL4_MSB_OR_LSB_FIRST_MASK | UL4_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 7), UL4_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 6), UL4_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 22), UL4_DSD_WIDTH_MASK);
		afe_write(AFE_UL4_BASE, base);
		afe_write(AFE_UL4_END, end);
		break;
	case AFE_MEM_UL5:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 20), UL5_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 28) | (config->channel << 8),
			      UL5_MSB_OR_LSB_FIRST_MASK | UL5_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 9), UL5_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 8), UL5_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 24), UL5_DSD_WIDTH_MASK);
		afe_write(AFE_UL5_BASE, base);
		afe_write(AFE_UL5_END, end);
		break;
	case AFE_MEM_UL6:
		afe_msk_write(AFE_DAC_CON2, (config->fs << 25), UL6_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 29) | (config->channel << 10),
			      UL6_MSB_OR_LSB_FIRST_MASK | UL6_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 11), UL6_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 10), UL6_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 26), UL6_DSD_WIDTH_MASK);
		afe_write(AFE_UL6_BASE, base);
		afe_write(AFE_UL6_END, end);
		break;
	case AFE_MEM_ULMCH:
		afe_msk_write(AFE_DAC_CON4, (config->channel << 12), ULMCH_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 13), ULMCH_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 12), ULMCH_HD_AUDIO_MASK);
		afe_write(AFE_ULMCH_BASE, base);
		afe_write(AFE_ULMCH_END, end);
		break;
	case AFE_MEM_AWB:
		afe_msk_write(AFE_DAC_CON3, (config->fs << 0), AWB_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 30) | (config->channel << 16),
			      AWB_MSB_OR_LSB_FIRST_MASK | AWB_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 17), AWB_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 14), AWB_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 28), AWB_DSD_WIDTH_MASK);
		afe_write(AFE_AWB_BASE, base);
		afe_write(AFE_AWB_END, end);
		break;
	case AFE_MEM_AWB2:
		afe_msk_write(AFE_DAC_CON3, (config->fs << 5), AWB2_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->first_bit << 31) | (config->channel << 18),
			      AWB2_MSB_OR_LSB_FIRST_MASK | AWB2_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 19), AWB2_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 16), AWB2_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON5, (config->dsd_width << 30), AWB2_DSD_WIDTH_MASK);
		afe_write(AFE_AWB2_BASE, base);
		afe_write(AFE_AWB2_END, end);
		break;
	case AFE_MEM_DAI:
		afe_msk_write(AFE_DAC_CON2, (config->daimod_fs << 30), DAI_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->dup_write << 14), DAI_DUP_WR_MASK);
		afe_write(AFE_DAI_BASE, base);
		afe_write(AFE_DAI_END, end);
		break;
	case AFE_MEM_MOD_PCM:
		afe_msk_write(AFE_DAC_CON2, (config->daimod_fs << 31), MOD_PCM_MODE_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->dup_write << 15), MOD_PCM_DUP_WR_MASK);
		afe_write(AFE_MOD_PCM_BASE, base);
		afe_write(AFE_MOD_PCM_END, end);
		break;
	case AFE_MEM_DSDW:
		afe_msk_write(AFE_DAC_CON3, (config->first_bit << 31), DSDW_MSB_OR_LSB_FIRST_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->channel << 20), DSDW_DATA_MASK);
		if (config->channel == MONO)
			afe_msk_write(AFE_DAC_CON4, (config->mono_sel << 21), DSDW_R_MONO_MASK);
		afe_msk_write(AFE_MEMIF_HD_CON1, (config->hd_audio << 18), DSDW_HD_AUDIO_MASK);
		afe_msk_write(AFE_DAC_CON4, (config->dsd_width << 22), DSDW_DSD_WIDTH_MASK);
		afe_write(AFE_DSDW_BASE, base);
		afe_write(AFE_DSDW_END, end);
		break;
	default:
		pr_err("%s() error: wrong memif %d\n", __func__, memif);
		return -EINVAL;
	}
	return 0;
}

int afe_memif_pointer(enum afe_mem_interface memif, u32 *cur_ptr)
{
	static const u32 regs[] = {
		0,		/* AFE_MEM_NONE    =  0, */
		AFE_DL1_CUR,	/* AFE_MEM_DL1     =  1, */
		AFE_DL2_CUR,	/* AFE_MEM_DL2     =  2, */
		AFE_DL3_CUR,	/* AFE_MEM_DL3     =  3, */
		AFE_DL4_CUR,	/* AFE_MEM_DL4     =  4, */
		AFE_DL5_CUR,	/* AFE_MEM_DL5     =  5, */
		AFE_DL6_CUR,	/* AFE_MEM_DL6     =  6, */
		AFE_DLMCH_CUR,	/* AFE_MEM_DLMCH   =  7, */
		AFE_ARB1_CUR,	/* AFE_MEM_ARB1  =  8, */
		AFE_DSDR_CUR,	/* AFE_MEM_DSDR    =  9, */
		AFE_VUL_CUR,	/* AFE_MEM_UL1     = 10, */
		AFE_UL2_CUR,	/* AFE_MEM_UL2     = 11, */
		AFE_UL3_CUR,	/* AFE_MEM_UL3     = 12, */
		AFE_UL4_CUR,	/* AFE_MEM_UL4     = 13, */
		AFE_UL5_CUR,	/* AFE_MEM_UL5     = 14, */
		AFE_UL6_CUR,	/* AFE_MEM_UL6     = 15, */
		AFE_ULMCH_CUR,	/* AFE_MEM_ULMCH   = 16, */
		AFE_DAI_CUR,	/* AFE_MEM_DAI     = 17, */
		AFE_MOD_PCM_CUR,	/* AFE_MEM_MOD_PCM = 18, */
		0,		/* AFE_MEM_RESERVED      = 19, */
		AFE_AWB_CUR,	/* AFE_MEM_AWB     = 20, */
		AFE_AWB2_CUR,	/* AFE_MEM_AWB2    = 21, */
		AFE_DSDW_CUR,	/* AFE_MEM_DSDW    = 22 */
	};

	if (memif >= AFE_MEM_NUM || regs[memif] == 0) {
		pr_err("%s() error: invalid memif %u\n", __func__, memif);
		return -EINVAL;
	}
	if (cur_ptr == NULL)
		return -ENOMEM;
	*cur_ptr = afe_read(regs[memif]);
	return 0;
}

int memif_enable_clk(int memif_id , int on)
{
	int off = !on;

	if (memif_id == AFE_MEM_NONE) {
		pr_err("%s() error: memif_id %d\n", __func__, memif_id);
		return -EINVAL;
	}

	if (memif_id <= AFE_MEM_ARB1) /* AFE_MEM_DL1~AFE_MEM_ARB1 */
		afe_msk_write(AUDIO_TOP_CON5, off << (memif_id+5), 1 << (memif_id+5));
	else if (memif_id >= AFE_MEM_UL1 && memif_id <= AFE_MEM_UL6)
		afe_msk_write(AUDIO_TOP_CON5, off << (memif_id - 10), 1 << (memif_id - 10));
	else if (memif_id == AFE_MEM_DAI)
		afe_msk_write(AUDIO_TOP_CON5, off << 16, 1 << 16);
	else if (memif_id == AFE_MEM_AWB)
		afe_msk_write(AUDIO_TOP_CON5, off << 14, 1 << 14);
	else if (memif_id == AFE_MEM_AWB2)
		afe_msk_write(AUDIO_TOP_CON5, off << 15, 1 << 15);
	else
		pr_err("%s() error: memif_id %d\n", __func__, memif_id);

	if (memif_id == AFE_MEM_DLMCH) {
		if (on) {
			afe_msk_write(AUDIO_TOP_CON5, 0, PDN_MEMIF_DL1);
			afe_msk_write(AUDIO_TOP_CON5, 0, PDN_MEMIF_DL2);
			afe_msk_write(AUDIO_TOP_CON5, 0, PDN_MEMIF_DL3);
			afe_msk_write(AUDIO_TOP_CON5, 0, PDN_MEMIF_DL4);
			afe_msk_write(AUDIO_TOP_CON5, 0, PDN_MEMIF_DL5);
		} else {
			afe_msk_write(AUDIO_TOP_CON5, PDN_MEMIF_DL1, PDN_MEMIF_DL1);
			afe_msk_write(AUDIO_TOP_CON5, PDN_MEMIF_DL2, PDN_MEMIF_DL2);
			afe_msk_write(AUDIO_TOP_CON5, PDN_MEMIF_DL3, PDN_MEMIF_DL3);
			afe_msk_write(AUDIO_TOP_CON5, PDN_MEMIF_DL4, PDN_MEMIF_DL4);
			afe_msk_write(AUDIO_TOP_CON5, PDN_MEMIF_DL5, PDN_MEMIF_DL5);
		}
	}
	return 0;
}

int afe_memif_base(enum afe_mem_interface memif, u32 *base)
{
	static const u32 regs[] = {
		0,		/* AFE_MEM_NONE    =  0, */
		AFE_DL1_BASE,	/* AFE_MEM_DL1     =  1, */
		AFE_DL2_BASE,	/* AFE_MEM_DL2     =  2, */
		AFE_DL3_BASE,	/* AFE_MEM_DL3     =  3, */
		AFE_DL4_BASE,	/* AFE_MEM_DL4     =  4, */
		AFE_DL5_BASE,	/* AFE_MEM_DL5     =  5, */
		AFE_DL6_BASE,	/* AFE_MEM_DL6     =  6, */
		AFE_DLMCH_BASE,	/* AFE_MEM_DLMCH   =  7, */
		AFE_ARB1_BASE,	/* AFE_MEM_ARB1  =  8, */
		AFE_DSDR_BASE,	/* AFE_MEM_DSDR    =  9, */
		AFE_VUL_BASE,	/* AFE_MEM_UL1     = 10, */
		AFE_UL2_BASE,	/* AFE_MEM_UL2     = 11, */
		AFE_UL3_BASE,	/* AFE_MEM_UL3     = 12, */
		AFE_UL4_BASE,	/* AFE_MEM_UL4     = 13, */
		AFE_UL5_BASE,	/* AFE_MEM_UL5     = 14, */
		AFE_UL6_BASE,	/* AFE_MEM_UL6     = 15, */
		AFE_ULMCH_BASE,	/* AFE_MEM_ULMCH   = 16, */
		AFE_DAI_BASE,	/* AFE_MEM_DAI     = 17, */
		AFE_MOD_PCM_BASE,	/* AFE_MEM_MOD_PCM = 18, */
		0,		/* AFE_MEM_RESERVED      = 19, */
		AFE_AWB_BASE,	/* AFE_MEM_AWB     = 20, */
		AFE_AWB2_BASE,	/* AFE_MEM_AWB2    = 21, */
		AFE_DSDW_BASE,	/* AFE_MEM_DSDW    = 22 */
	};

	if (memif >= AFE_MEM_NUM || regs[memif] == 0) {
		pr_err("%s() error: invalid memif %u\n", __func__, memif);
		return -EINVAL;
	}
	if (base == NULL)
		return -ENOMEM;
	*base = afe_read(regs[memif]);
	return 0;
}


/******************** irq ********************/

u32 afe_irq_status(void)
{
	return afe_read(AFE_IRQ_STATUS);
}

void afe_irq_clear(u32 status)
{
	afe_msk_write(AFE_IRQ_CLR, status, IRQ_CLR_ALL);
}

u32 asys_irq_status(void)
{
	return afe_read(ASYS_IRQ_STATUS);
}

void asys_irq_clear(u32 status)
{
	afe_msk_write(ASYS_IRQ_CLR, status, ASYS_IRQ_CLR_ALL);
}

int audio_irq_configurate(enum audio_irq_id id, const struct audio_irq_config *config)
{
	u32 fs_mode;

	switch (config->mode) {
	case FS_8000HZ:
		fs_mode = 0;
		break;
	case FS_12000HZ:
		fs_mode = 1;
		break;
	case FS_16000HZ:
		fs_mode = 2;
		break;
	case FS_24000HZ:
		fs_mode = 3;
		break;
	case FS_32000HZ:
		fs_mode = 4;
		break;
	case FS_48000HZ:
		fs_mode = 5;
		break;
	case FS_96000HZ:
		fs_mode = 6;
		break;
	case FS_192000HZ:
		fs_mode = 7;
		break;
	case FS_11025HZ:
		fs_mode = 9;
		break;
	case FS_22050HZ:
		fs_mode = 0xb;
		break;
	case FS_44100HZ:
		fs_mode = 0xd;
		break;
	case FS_88200HZ:
		fs_mode = 0xe;
		break;
	case FS_176400HZ:
		fs_mode = 0xf;
		break;
	default:
		fs_mode = 5;
		break;
	}
	pr_debug("%s() id=%d, init_val=%u\n", __func__, id, config->init_val);
	if (id >= IRQ_ASYS_IRQ1 && id <= IRQ_ASYS_IRQ16) {
		u32 addrCON;

		addrCON = ASYS_IRQ1_CON + (id - IRQ_ASYS_IRQ1) * 4;
		afe_msk_write(addrCON, (config->mode << 24) | (config->init_val << 0)
			      , (0x1f << 24) | (0xffffff << 0));
	}
	/* hdmi-i2s-hdmi-tx irq counter */
	else if (id == IRQ_AFE_HDMI)
		afe_msk_write(AFE_IRQ_CNT5, config->init_val << 0, 0xffffffff);
	else if (id == IRQ_AFE_IRQ1) {
		afe_msk_write(AFE_IRQ_CON, fs_mode << 4, IRQ1_FS_MASK);
		afe_msk_write(AFE_IRQ_CNT1, config->init_val, IRQ_CNT1_MASK);
	} else if (id == IRQ_AFE_IRQ2) {
		afe_msk_write(AFE_IRQ_CON, fs_mode << 8, IRQ2_FS_MASK);
		afe_msk_write(AFE_IRQ_CNT2, config->init_val, IRQ_CNT2_MASK);
	}
	return 0;
}

int audio_irq_enable(enum audio_irq_id id, int en)
{
	en = !!en;
	pr_debug("%s() %s for id=%d\n", __func__, en ? "enable" : "disable", id);
	if (id >= IRQ_ASYS_IRQ1 && id <= IRQ_ASYS_IRQ16) {
		u32 addrCON;

		addrCON = ASYS_IRQ1_CON + (id - IRQ_ASYS_IRQ1) * 4;
		afe_msk_write(addrCON, en << 31, 0x1 << 31);
	} else if (id == IRQ_AFE_SPDIF2)
		afe_msk_write(AFE_IRQ_CON, (en << 3), IRQ4_ON);
	/* hdmi-i2s-hdmi-tx */
	else if (id == IRQ_AFE_HDMI)
		afe_msk_write(AFE_IRQ_CON, (en << 12), IRQ5_ON);
	/* hdmi-iec-hdmi-tx */
	else if (id == IRQ_AFE_SPDIF)
		afe_msk_write(AFE_IRQ_CON, (en << 13), IRQ6_ON);
	else if (id == IRQ_AFE_IRQ1)
		afe_msk_write(AFE_IRQ_CON, (en << 0), IRQ1_ON);
	else if (id == IRQ_AFE_IRQ2)
		afe_msk_write(AFE_IRQ_CON, (en << 1), IRQ2_ON);
	else if (id == IRQ_AFE_MULTIIN)
		afe_msk_write(AFE_IRQ_CON, (en << 2), IRQ_MULTI_ON);
	else {
		pr_warn("%s() %s for id=%d has not been implemented\n",
			__func__, en ? "enable" : "disable", id);
		return -EINVAL;
	}
	return 0;
}

/*
 * audio_irq_cnt_mon()
 * Return value: 0            - invalid
 *               1 ~ init_val - how many samples remained before triggering interrupt
 */
u32 audio_irq_cnt_mon(enum audio_irq_id id)
{
	u32 addr = 0, mask;

	if (id >= IRQ_ASYS_IRQ1 && id <= IRQ_ASYS_IRQ16) {
		u32 asys_irq_mon_sel = (id - IRQ_ASYS_IRQ1) << 0;

		afe_msk_write(ASYS_IRQ_CONFIG, asys_irq_mon_sel, ASYS_IRQ_MON_SEL_MASK);
		addr = ASYS_IRQ_MON2;
		mask = IRQ_CNT_STATUS_MASK;
	} else if (id >= IRQ_AFE_IRQ1 && id <= IRQ_AFE_IRQ2) {
		addr = AFE_IRQ1_MCU_CNT_MON + (id - IRQ_AFE_IRQ1) * 4;
		mask = AFE_IRQ_MCU_CNT_MON_MASK;
	}
	if (addr) {
		while (1) {
			u32 tmp1, tmp2;

			/*
			 * hailang.deng said the monitor value should be read twice,
			 * it is valid only if they are equal
			 */
			tmp1 = afe_read(addr) & mask;
			tmp2 = afe_read(addr) & mask;
			if (tmp1 == tmp2)
				return tmp1;
		}
	} else
		return 0;
}

static DEFINE_MUTEX(asys_irqs_using_lock);
#define ASYS_IRQ_NUM (16)
static int asys_irqs_using[ASYS_IRQ_NUM];

enum audio_irq_id asys_irq_acquire(void)
{
	int i;

	mutex_lock(&asys_irqs_using_lock);
	for (i = 0; i < ASYS_IRQ_NUM; ++i) {
		if (!asys_irqs_using[i]) {
			asys_irqs_using[i] = 1;
			mutex_unlock(&asys_irqs_using_lock);
			pr_debug("%s() %d is acquired\n", __func__, i);
			return IRQ_ASYS_IRQ1 + i;
		}
	}
	mutex_unlock(&asys_irqs_using_lock);
	pr_debug("%s() nothing is acquired\n", __func__);
	return IRQ_NUM;
}

void asys_irq_release(enum audio_irq_id id)
{
	if (id >= IRQ_ASYS_IRQ1 && id < IRQ_ASYS_IRQ1 + ASYS_IRQ_NUM) {
		int i = id - IRQ_ASYS_IRQ1;

		mutex_lock(&asys_irqs_using_lock);
		asys_irqs_using[i] = 0;
		mutex_unlock(&asys_irqs_using_lock);
		pr_debug("%s() %d is released\n", __func__, i);
	}
}


/******************** i2s ********************/

int afe_i2s_power_on_mclk(int id, int on)
{
	u32 val, msk;

	if (id < 0 || id > 5) {
		pr_err("%s() error: i2s id %d\n", __func__, id);
		return -EINVAL;
	}
	val = (!on) << (23 + id);
	msk = 0x1 << (23 + id);
	#ifdef AUDIO_MEM_IOREMAP
	topckgen_msk_write(CLK_AUDDIV_3, val, msk);
	#else
	afe_msk_write(CLK_AUDDIV_3, val, msk);
	#endif
	return 0;
}

static void afe_i2s_mclk_configurate(int id, int mclk)
{
	u32 addr, pos;
	int div;
	u32 domain_pos;
	int domain;

	switch (id) {
	case 0:
		addr = CLK_AUDDIV_1;
		pos = 0;
		domain_pos = 15;
		break;
	case 1:
		addr = CLK_AUDDIV_1;
		pos = 8;
		domain_pos = 16;
		break;
	case 2:
		addr = CLK_AUDDIV_1;
		pos = 16;
		domain_pos = 17;
		break;
	case 3:
		addr = CLK_AUDDIV_1;
		pos = 24;
		domain_pos = 18;
		break;
	case 4:
		addr = CLK_AUDDIV_2;
		pos = 0;
		domain_pos = 19;
		break;
	case 5:
		addr = CLK_AUDDIV_2;
		pos = 8;
		domain_pos = 20;
		break;
	default:
		return;
	}
	if (mclk <= 0) {
		pr_err("%s() error: i2s id %d, bad mclk %d\n", __func__, id, mclk);
		return;
	}
	if ((98304000 % mclk) == 0) {
		div = 98304000 / mclk;
		domain = 0;
	} else if ((90316800 % mclk) == 0) {
		div = 90316800 / mclk;
		domain = 1;
	} else {
		div = 1;
		domain = 0;
		pr_err("%s() error: i2s id %d, bad mclk %d\n", __func__, id, mclk);
		return;
	}
	#ifdef AUDIO_MEM_IOREMAP
	topckgen_msk_write(addr, (div - 1) << pos, 0xff << pos);
	topckgen_msk_write(CLK_AUDDIV_3, domain << domain_pos, 0x1 << domain_pos);
	#else
	afe_msk_write(addr, (div - 1) << pos, 0xff << pos);
	afe_msk_write(CLK_AUDDIV_3, domain << domain_pos, 0x1 << domain_pos);
	#endif
}

int afe_i2s_in_configurate(enum afe_i2s_in_id id, const struct afe_i2s_in_config *config)
{
	u32 addr;
	u32 val, msk;
	enum afe_sampling_rate mode;

	if (id >= AFE_I2S_IN_NUM)
		return -EINVAL;
	if (!config->slave)
		afe_i2s_mclk_configurate(id, config->mclk);
	/* don't invert bck when i2s-in slave
	   don't invert bck when i2s-in master */
	val = (0x0 << id) << 8;
	msk = (0x1 << id) << 8;
	afe_msk_write(ASYS_TOP_CON, val, msk);
	/* hailang.deng said if slave mode,
	   fs should be set to one of 48k domain fs */
	mode = config->slave ? FS_8000HZ : config->mode;
	addr = ASYS_I2SIN1_CON + id * 4;
	val = (config->fpga_test_loop3 << 23)
	      | (config->fpga_test_loop << 21)
	      | (config->fpga_test_loop2 << 20)
	      | (config->use_asrc << 19)
	      | (config->dsd_mode << 18)
	      | (mode << 8)
	      | (config->slave << 2)
	      | (0x1 << 31)	/* enable phase-shift fix */
	      ;
	if (config->dsd_mode)
		val |= (0x0 << 1);	/* must be 32cycle */
	/* I2S,LJ,RJ are none of business */
	else {
		switch (config->fmt) {
		case FMT_32CYCLE_16BIT_I2S:
			val |= ((0x0 << 1)	/* 32cycle */
				| (0x1 << 3) /* I2S */);
			break;
		case FMT_32CYCLE_16BIT_LJ:
			val |= ((0x0 << 1)	/* 32cycle */
				| (0x0 << 3) | (0x0 << 14)	/* LJ */
				| (0x1 << 5) /* LR Invert */);
			break;
		case FMT_64CYCLE_16BIT_I2S:
		case FMT_64CYCLE_32BIT_I2S:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x1 << 3) /* I2S */);
			break;
		case FMT_64CYCLE_16BIT_LJ:
		case FMT_64CYCLE_32BIT_LJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x0 << 14)	/* LJ */
				| (0x1 << 5) /* LR Invert */);
			break;
		case FMT_64CYCLE_16BIT_RJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x1 << 14)	/* RJ */
				| (0x1 << 5)	/* LR Invert */
				| (0 << 13) /* 16bit */);
			break;
		case FMT_64CYCLE_24BIT_RJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x1 << 14)	/* RJ */
				| (0x1 << 5)	/* LR Invert */
				| (1 << 13) /* 24bit */);
			break;
		default:
			break;
		}
	}
	msk = (0x1 << 23)	/* fpga_test_loop3 */
	      | (0x1 << 21)	/* fpga_test_loop */
	      | (0x1 << 20)	/* fpga_test_loop2 */
	      | (0x1 << 19)	/* use_asrc */
	      | (0x1 << 18)	/* dsdMode */
	      | (0x1 << 13)	/* RJ 16bit/24bit */
	      | (0x1F << 8)	/* mode */
	      | (0x1 << 2)		/* slave */
	      | (0x1 << 1)		/* cycle */
	      | (0x1 << 3) | (0x1 << 14)	/* fmt */
	      | (0x1 << 5)		/* LR Invert */
	      | (0x1 << 31);	/* phase-shift fix */
	afe_msk_write(addr, val, msk);
	return 0;
}

int afe_i2s_in_enable(enum afe_i2s_in_id id, int en)
{
	u32 addr;

	if (id >= AFE_I2S_IN_NUM)
		return -EINVAL;
	en = !!en;
	addr = ASYS_I2SIN1_CON + id * 4;
	if (en) {
		afe_msk_write(addr, (0x1 << 30), (0x1 << 30));
		udelay(1);
		afe_msk_write(addr, (0x0 << 30), (0x1 << 30));
		udelay(1);
	}
	afe_msk_write(addr, (en << 0), (0x1 << 0));
	return 0;
}

int afe_i2s_out_configurate(enum afe_i2s_out_id id, const struct afe_i2s_out_config *config)
{
	u32 addr;
	u32 val, msk;
	enum afe_sampling_rate mode;
	enum afe_i2s_out_dsd_use dsd_use;

	if (id >= AFE_I2S_OUT_NUM)
		return -EINVAL;
	if (!config->slave)
		afe_i2s_mclk_configurate(id, config->mclk);
	/* invert bck when i2s-out slave
	   don't invert bck when i2s-out master */
	val = (config->slave << id) << 8;
	msk = (0x1 << id) << 8;
	afe_msk_write(ASYS_TOP_CON, val, msk);
	/* hailang.deng said if slave mode,
	   fs should be set to one of 48k domain fs */
	mode = config->slave ? FS_8000HZ : config->mode;
	addr = ASYS_I2SO1_CON + id * 4;
	val = (config->fpga_test_loop << 21)
	      | (config->data_from_sine << 20)
	      | (config->use_asrc << 19)
	      | (config->dsd_mode << 18)
	      | (0 /*config->couple_mode */  << 17)
	      | (config->one_heart_mode << 16)
	      | (mode << 8)
	      | (config->slave << 2);
	if (config->dsd_mode) {
		dsd_use = config->dsd_use;
		val |= (0x0 << 1);	/* must be 32cycle */
		/* I2S,LJ,RJ are none of business */
	} else {
		dsd_use = I2S_OUT_DSD_USE_NORMAL;
		switch (config->fmt) {
		case FMT_32CYCLE_16BIT_I2S:
			val |= ((0x0 << 1)	/* 32cycle */
				| (0x1 << 3) /* I2S */);
			break;
		case FMT_32CYCLE_16BIT_LJ:
			val |= ((0x0 << 1)	/* 32cycle */
				| (0x0 << 3) | (0x0 << 14)	/* LJ */
				| (0x1 << 5) /* LR Invert */);
			break;
		case FMT_64CYCLE_16BIT_I2S:
		case FMT_64CYCLE_32BIT_I2S:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x1 << 3) /* I2S */);
			break;
		case FMT_64CYCLE_16BIT_LJ:
		case FMT_64CYCLE_32BIT_LJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x0 << 14)	/* LJ */
				| (0x1 << 5) /* LR Invert */);
			break;
		case FMT_64CYCLE_16BIT_RJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x1 << 14)	/* RJ */
				| (0x1 << 5)	/* LR Invert */
				| (0 << 13) /* 16bit */);
			break;
		case FMT_64CYCLE_24BIT_RJ:
			val |= ((0x1 << 1)	/* 64cycle */
				| (0x0 << 3) | (0x1 << 14)	/* RJ */
				| (0x1 << 5)	/* LR Invert */
				| (1 << 13) /* 24bit */);
			break;
		default:
			break;
		}
	}
	msk = (0x1 << 21)	/* fpga_test_loop */
	      | (0x1 << 20)	/* data_from_sine */
	      | (0x1 << 19)	/* use_asrc */
	      | (0x1 << 18)	/* dsd_mode */
	      | (0x1 << 17)	/* couple_mode */
	      | (0x1 << 16)	/* one_heart_mode */
	      | (0x1F << 8)	/* mode */
	      | (0x1 << 5)		/* LR Invert */
	      | (0x1 << 2)		/* slave */
	      | (0x1 << 13)	/* RJ 16bit/24bit */
	      | (0x1 << 1)		/* cycle */
	      | (0x1 << 3) | (0x1 << 14);	/* fmt */
	afe_msk_write(addr, val, msk);
	if (config->dsd_mode) {
		if (id == AFE_I2S_OUT_1) {
			afe_msk_write(ASYS_TOP_CON, dsd_use << 3, I2S1_DSD_USE_MASK);
			/* hailang.deng said
			 * if bypass fader, bck needs invert
			 * if use fader, bck doesn't need invert
			 */
			afe_msk_write(DSD1_FADER_CON0, (0x1 << 15) | (0x1 << 14) | (0x0 << 10)
				      , (0x1 << 15) | (0x1 << 14) | (0x1 << 10));
		} else if (id == AFE_I2S_OUT_2) {
			afe_msk_write(ASYS_TOP_CON, dsd_use << 4, I2S2_DSD_USE_MASK);
			/* hailang.deng said
			 * if bypass fader, bck needs invert
			 * if use fader, bck doesn't need invert
			 */
			afe_msk_write(DSD2_FADER_CON0, (0x1 << 15) | (0x1 << 14) | (0x0 << 10)
				      , (0x1 << 15) | (0x1 << 14) | (0x1 << 10));
		}
	} else {
		if (id == AFE_I2S_OUT_1)
			afe_msk_write(ASYS_TOP_CON, I2S1_DSD_USE_NORMAL, I2S1_DSD_USE_MASK);
		else if (id == AFE_I2S_OUT_2)
			afe_msk_write(ASYS_TOP_CON, I2S2_DSD_USE_NORMAL, I2S2_DSD_USE_MASK);
	}
	return 0;
}

int afe_i2s_out_enable(enum afe_i2s_out_id id, int en)
{
	u32 addr;

	if (id >= AFE_I2S_OUT_NUM)
		return -EINVAL;
	en = !!en;
	addr = ASYS_I2SO1_CON + id * 4;
	if (en) {
		afe_msk_write(addr, (0x1 << 30), (0x1 << 30));
		udelay(1);
		afe_msk_write(addr, (0x0 << 30), (0x1 << 30));
		udelay(1);
	}
	afe_msk_write(addr, (en << 0), (0x1 << 0));
	return 0;
}

int afe_i2s_out_slave_pcm_configurate(enum afe_i2s_out_id id, enum afe_i2s_format fmt, int use_asrc)
{
	/* configurate i2s-in first, and then i2s-out */
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.slave = 1,	/* must be slave */
			.fmt = fmt,
			.mclk = 0,
		};

		afe_i2s_in_configurate((enum afe_i2s_in_id)id, &config);
	}
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = use_asrc,
			.dsd_mode = 0,
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 1,	/* slave */
			.fmt = fmt,
			.mclk = 0,
		};

		afe_i2s_out_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_out_slave_pcm_enable(enum afe_i2s_out_id id, int en)
{
	return afe_i2s_out_enable(id, en);
}

int afe_i2s_out_master_dsd_configurate(enum afe_i2s_out_id id, enum afe_sampling_rate fs, int mclk,
				       enum afe_i2s_out_dsd_use dsd_use)
{
	switch (id) {
	case AFE_I2S_OUT_1:
		afe_msk_write(ASMO_TIMING_CON1, (fs << 0), ASMO1_MODE_MASK);
		break;
	case AFE_I2S_OUT_2:
		afe_msk_write(ASMO_TIMING_CON1, (fs << 5), ASMO2_MODE_MASK);
		break;
	default:
		pr_err("%s() error: i2s-out id %d doesn't support DSD\n", __func__, id);
		return -EINVAL;
	}
	/* configurate i2s-in first, and then i2s-out */
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 0,	/* must be none-DSD */
			.slave = 0,
			.fmt = FMT_32CYCLE_16BIT_I2S,
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_in_configurate((enum afe_i2s_in_id)id, &config);
	}
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 1,	/* DSD */
			.dsd_use = dsd_use,
			.one_heart_mode = 0,
			.slave = 0,	/* master */
			/* .fmt = ,  -> no need in DSD mode */
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_out_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_out_master_dsd_enable(enum afe_i2s_out_id id, int en)
{
	return afe_i2s_out_enable(id, en);
}

int afe_i2s_out_slave_dsd_configurate(enum afe_i2s_out_id id)
{
	/* configurate i2s-in first, and then i2s-out */
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 1,
			.slave = 1,
			/* .fmt = FMT_32CYCLE_16BIT_I2S, */
			/* .mclk = mclk, */
			/* .mode = fs */
		};

		afe_i2s_in_configurate((enum afe_i2s_in_id)id, &config);
	}
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 1,	/* DSD */
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 1,
			/* .fmt = ,  -> no need in DSD mode */
			/* .mclk = mclk, */
			/* .mode = fs */
		};

		afe_i2s_out_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_out_slave_dsd_enable(enum afe_i2s_out_id id, int en)
{
	return afe_i2s_out_enable(id, en);
}

int afe_i2s_in_master_pcm_configurate(enum afe_i2s_in_id id, enum afe_i2s_format fmt, int mclk,
				      enum afe_sampling_rate fs, int use_asrc)
{
	/* configurate i2s-out first, and then i2s-in */
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 0,
			.fmt = fmt,
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_out_configurate((enum afe_i2s_out_id)id, &config);
	}
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = use_asrc,
			.dsd_mode = 0,
			.slave = 0,
			.fmt = fmt,
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_in_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_in_master_pcm_enable(enum afe_i2s_in_id id, int en)
{
	afe_i2s_in_enable(id, en);
	afe_i2s_out_enable((enum afe_i2s_out_id)id, en);
	return 0;
}

int afe_i2s_in_master_dsd_configurate(enum afe_i2s_in_id id, enum afe_sampling_rate fs, int mclk)
{
	switch (id) {
	case AFE_I2S_IN_1:
		afe_msk_write(ASMI_TIMING_CON1, (fs << 0), ASMI1_MODE_MASK);
		break;
	case AFE_I2S_IN_2:
		afe_msk_write(ASMI_TIMING_CON1, (fs << 5), ASMI2_MODE_MASK);
		break;
	default:
		pr_err("%s() error: i2s-in id %d doesn't support DSD\n", __func__, id);
		return -EINVAL;
	}
	/* configurate i2s-out first, and then i2s-in */
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,	/* must be none-DSD */
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 0,
			.fmt = FMT_32CYCLE_16BIT_I2S,
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_out_configurate((enum afe_i2s_out_id)id, &config);
	}
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 1,	/* DSD */
			.slave = 0,	/* master */
			/* .fmt = ,  -> no need in DSD mode */
			.mclk = mclk,
			.mode = fs
		};

		afe_i2s_in_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_in_master_dsd_enable(enum afe_i2s_in_id id, int en)
{
	afe_i2s_in_enable(id, en);
	afe_i2s_out_enable((enum afe_i2s_out_id)id, en);
	return 0;
}

int afe_i2s_in_slave_dsd_configurate(enum afe_i2s_in_id id)
{
	/* configurate i2s-out first, and then i2s-in */
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,	/* none-DSD */
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 1,
			/* .fmt = ,  -> no need in DSD mode */
			/* .mclk = mclk, */
			/* .mode = fs */
		};

		afe_i2s_out_configurate((enum afe_i2s_out_id)id, &config);
	}
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 1,	/* DSD */
			.slave = 1,
			/* .fmt = FMT_32CYCLE_16BIT_I2S, */
			/* .mclk = mclk, */
			/* .mode = fs */
		};

		afe_i2s_in_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_in_slave_dsd_enable(enum afe_i2s_in_id id, int en)
{
	return afe_i2s_in_enable(id, en);
}

int afe_i2s_in_slave_pcm_configurate(enum afe_i2s_in_id id, enum afe_i2s_format fmt, int use_asrc)
{
	/* configurate i2s-out first, and then i2s-in */
	{
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.one_heart_mode = 0,
			.slave = 1,
			.fmt = fmt,
			.mclk = 0,
			/* .mode = fs */
		};

		afe_i2s_out_configurate((enum afe_i2s_out_id)id, &config);
	}
	{
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = use_asrc,
			.dsd_mode = 0,
			.slave = 1,
			.fmt = fmt,
			.mclk = 0,
			/* .mode = fs */
		};

		afe_i2s_in_configurate(id, &config);
	}
	return 0;
}

int afe_i2s_in_slave_pcm_enable(enum afe_i2s_in_id id, int en)
{
	return afe_i2s_in_enable(id, en);
}

/******************** multilinein ********************/

void afe_multilinein_configurate(const struct afe_multilinein_config *config)
{
	u32 val1, msk1;
	u32 val0, msk0;

	val1 = MULTI_SYNC_ENABLE
	       /*| MULTI_INV_BCK */
	       /*| (0x1 << 21) */   /* dsd 8 bit mode ? ? ? */
	       | (config->dsd_mode << 20)
	       | (0x0 << 19)    /* always cmpt_mode */
	       /* if 2ch, use none-hbr_mode */
	       /* if 8ch, use hbr_mode */
	       | ((config->ch_num == AFE_MULTILINEIN_8CH ? 0x1 : 0x0) << 18)
	       | (config->ch_num << 0);
	val0 = (config->intr_period << 4);
	switch (config->fmt) {
	case FMT_32CYCLE_16BIT_I2S:
		val1 |= (AINACK_CFG_LRCK_SEL_16 | AINACK_CFG_BITNUM_16 | AINACK_CFG_CLK_I2S);
		val0 |= DATA_16BIT;
		break;
	case FMT_32CYCLE_16BIT_LJ:
		val1 |= (AINACK_CFG_LRCK_SEL_16
			 | AINACK_CFG_BITNUM_16 | AINACK_CFG_CLK_LJ | AINACK_CFG_INV_LRCK);
		val0 |= DATA_16BIT;
		break;
	case FMT_64CYCLE_16BIT_I2S:
		val1 |= (AINACK_CFG_LRCK_SEL_32 | AINACK_CFG_BITNUM_16 | AINACK_CFG_CLK_I2S);
		val0 |= DATA_16BIT;
		break;
	case FMT_64CYCLE_32BIT_I2S:
		val1 |= (AINACK_CFG_LRCK_SEL_32 | AINACK_CFG_BITNUM_24 | AINACK_CFG_CLK_I2S);
		val0 |= DATA_24BIT;
		break;
	case FMT_64CYCLE_16BIT_LJ:
		val1 |= (AINACK_CFG_LRCK_SEL_32
			 | AINACK_CFG_BITNUM_16 | AINACK_CFG_CLK_LJ | AINACK_CFG_INV_LRCK);
		val0 |= DATA_16BIT;
		break;
	case FMT_64CYCLE_32BIT_LJ:
		val1 |= (AINACK_CFG_LRCK_SEL_32
			 | AINACK_CFG_BITNUM_24 | AINACK_CFG_CLK_LJ | AINACK_CFG_INV_LRCK);
		val0 |= DATA_24BIT;
		break;
	case FMT_64CYCLE_16BIT_RJ:
		val1 |= (AINACK_CFG_LRCK_SEL_32
			 | AINACK_CFG_BITNUM_16 | AINACK_CFG_CLK_RJ | AINACK_CFG_INV_LRCK);
		val0 |= DATA_16BIT;
		break;
	case FMT_64CYCLE_24BIT_RJ:
		val1 |= (AINACK_CFG_LRCK_SEL_32
			 | AINACK_CFG_BITNUM_24 | AINACK_CFG_CLK_RJ | AINACK_CFG_INV_LRCK);
		val0 |= DATA_24BIT;
		break;
	default:
		break;
	}
	if (is_hd_audio(config->fmt)) {
		/* 24bit case endian */
		if (config->endian == AFE_MULTILINEIN_LITTILE_ENDIAN) {
			val1 |= (0x1 << 22);
			val0 |= DATA_16BIT_NON_SWAP;
		} else {
			val1 |= (0x0 << 22);
			val0 |= DATA_16BIT_NON_SWAP;
		}
	} else {
		/* 16bit case endian */
		if (config->endian == AFE_MULTILINEIN_LITTILE_ENDIAN) {
			val1 |= (0x0 << 22);
			val0 |= DATA_16BIT_SWAP;
		} else {
			val1 |= (0x0 << 22);
			val0 |= DATA_16BIT_NON_SWAP;
		}
	}
	if (config->dsd_mode) {
		if (config->dsdWidth == DSD_WIDTH_32BIT) { /*24bit*/
			val1 |= (0x0 << 21); /* 24bit*/
		} else {
			val1 |= (0x1 << 21); /*8bit*/
		}
	}
	if (config->mss == AFE_MULTILINE_FROM_RX) {
		val0 |= CLK_SEL_HDMI_RX;
		val0 |= SDATA_SEL_HDMI_RX;
	} else {
		val0 |= CLK_SEL_8CH_I2S;
		val0 |= SDATA_SEL_8CH_I2S;
	}
	msk1 = AINACK_MULTI_SYNC_MASK
	       | MULTI_DSD_MODE_MASK /*dsd 8bit or 24bit*/
	       /*| (0x1 << 21) */   /* dsd 8 bit mode ? ? ? */
	       | MULTI_INV_BCK
	       | AINACK_CFG_IN_MODE
	       | MULTI_24BIT_SWAP_MASK
	       | MULTI_NONE_COMPACT_MASK
	       | AINACK_HBRMOD_MASK
	       | AINACK_CFG_LRCK_MASK
	       | AINACK_CFG_INV_LRCK
	       | AINACK_CFG_CLK_MASK
	       | AINACK_CFG_BITNUM_MASK
	       | AINACK_CFG_CH_NUM_MASK;
	msk0 = INTR_PERIOD_MASK
	       | DATA_16BIT_SWAP_MASK
	       | DATA_BIT_MASK
	       | CLK_SEL_MASK
	       | SDATA_SEL_MASK;
	afe_msk_write(AFE_MPHONE_MULTI_CON1, val1, msk1);
	afe_msk_write(AFE_MPHONE_MULTI_CON0, val0, msk0);
	afe_msk_write(AFE_MPHONE_MULTI_CON0,
		      (0x0 << 14) | (0x1 << 17) | (0x2 << 20) | (0x3 << 23) | (0x4 << 26) | (0x5 << 29),
		      (0x7 << 14) | (0x7 << 17) | (0x7 << 20) | (0x7 << 23) | (0x7 << 26) | (0x7 << 29));
}

void afe_multilinein_enable(int en)
{
	en = !!en;
	if (en) {
		#ifdef CONFIG_MTK_LEGACY_CLOCK
		enable_clock(MT_CG_AUDIO_HDMIRX, "AUDIO"); /* AUDIO_TOP_CON4[19]:pdn_multi_in */
		enable_clock(MT_CG_AUDIO_INTDIR, "AUDIO"); /* AUDIO_TOP_CON4[20]:pdn_intdir */
		#else
		afe_msk_write(AUDIO_TOP_CON4, 0, PND_MULTI_IN);
		afe_msk_write(AUDIO_TOP_CON4, 0, PND_INTDIR);
		#endif
		afe_msk_write(AFE_MPHONE_MULTI_CON0, 1 << 0, HW_EN_MASK);
	} else {
		afe_msk_write(AFE_MPHONE_MULTI_CON0, 0 << 0, HW_EN_MASK);
		#ifdef CONFIG_MTK_LEGACY_CLOCK
		disable_clock(MT_CG_AUDIO_HDMIRX, "AUDIO");
		disable_clock(MT_CG_AUDIO_INTDIR, "AUDIO");
		#else
		afe_msk_write(AUDIO_TOP_CON4, PND_MULTI_IN, PND_MULTI_IN);
		afe_msk_write(AUDIO_TOP_CON4, PND_INTDIR, PND_INTDIR);
		#endif
	}
}

int afe_asmi_timing_set(enum afe_i2s_in_id id, enum afe_sampling_rate rate)
{
	u32 addr = ASMI_TIMING_CON1;
	u32 val = (rate << (id * 5));
	u32 msk = (0x1F << (id * 5));

	afe_msk_write(addr, val, msk);
	return 0;
}

int afe_asmo_timing_set(enum afe_i2s_out_id id, enum afe_sampling_rate rate)
{
	u32 addr = ASMO_TIMING_CON1;
	u32 val = (rate << (id * 5));
	u32 msk = (0x1F << (id * 5));

	afe_msk_write(addr, val, msk);
	return 0;
}

/******************** sample-based asrc ********************/

int afe_power_on_sample_asrc_rx(enum afe_sample_asrc_rx_id id, int on)
{
	const int off = !on;

	switch (id) {
	case SAMPLE_ASRC_I1:
		afe_msk_write(AUDIO_TOP_CON4, off << 12, PDN_ASRCI1);
		break;
	case SAMPLE_ASRC_I2:
		afe_msk_write(AUDIO_TOP_CON4, off << 13, PDN_ASRCI2);
		break;
	case SAMPLE_ASRC_I3:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCI3_POS, PDN_ASRCI3_MASK);
		break;
	case SAMPLE_ASRC_I4:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCI4_POS, PDN_ASRCI4_MASK);
		break;
	case SAMPLE_ASRC_I5:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCI5_POS, PDN_ASRCI5_MASK);
		break;
	case SAMPLE_ASRC_I6:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCI6_POS, PDN_ASRCI6_MASK);
		break;
	case SAMPLE_ASRC_PCM_IN:
		afe_msk_write(AUDIO_TOP_CON4, off << 16, PDN_ASRC11);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int afe_power_on_sample_asrc_tx(enum afe_sample_asrc_tx_id id, int on)
{
	const int off = !on;

	switch (id) {
	case SAMPLE_ASRC_O1:
		afe_msk_write(AUDIO_TOP_CON4, off << 14, PDN_ASRCO1);
		break;
	case SAMPLE_ASRC_O2:
		afe_msk_write(AUDIO_TOP_CON4, off << 15, PDN_ASRCO2);
		break;
	case SAMPLE_ASRC_O3:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCO3_POS, PDN_ASRCO3_MASK);
		break;
	case SAMPLE_ASRC_O4:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCO4_POS, PDN_ASRCO4_MASK);
		break;
	case SAMPLE_ASRC_O5:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCO5_POS, PDN_ASRCO5_MASK);
		break;
	case SAMPLE_ASRC_O6:
		afe_msk_write(PWR2_TOP_CON, off << PDN_ASRCO6_POS, PDN_ASRCO6_MASK);
		break;
	case SAMPLE_ASRC_PCM_OUT:
		afe_msk_write(AUDIO_TOP_CON4, off << 17, PDN_ASRC12);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static inline u32 AutoRstThLo(enum afe_sampling_rate fs)
{
	switch (fs) {
	case FS_8000HZ:
		return 0x05A000;
	case FS_12000HZ:
		return 0x03c000;
	case FS_16000HZ:
		return 0x02d000;
	case FS_24000HZ:
		return 0x01e000;
	case FS_32000HZ:
		return 0x016000;
	case FS_48000HZ:
		return 0x00f000;
	case FS_96000HZ:
		return 0x007800;
	case FS_192000HZ:
		return 0x003c00;
	case FS_384000HZ:
		return 0x001e00;
	case FS_7350HZ:
		return 0x066000;
	case FS_11025HZ:
		return 0x042000;
	case FS_14700HZ:
		return 0x033000;
	case FS_22050HZ:
		return 0x021000;
	case FS_29400HZ:
		return 0x019800;
	case FS_44100HZ:
		return 0x011000;
	case FS_88200HZ:
		return 0x008300;
	case FS_176400HZ:
		return 0x004100;
	case FS_352800HZ:
		return 0x002100;
	default:
		return 0x0;
	}
}

static inline u32 AutoRstThHi(enum afe_sampling_rate fs)
{
	switch (fs) {
	case FS_8000HZ:
		return 0x066000;
	case FS_12000HZ:
		return 0x044000;
	case FS_16000HZ:
		return 0x033000;
	case FS_24000HZ:
		return 0x022000;
	case FS_32000HZ:
		return 0x01a000;
	case FS_48000HZ:
		return 0x011000;
	case FS_96000HZ:
		return 0x008800;
	case FS_192000HZ:
		return 0x004400;
	case FS_384000HZ:
		return 0x002200;
	case FS_7350HZ:
		return 0x06F000;
	case FS_11025HZ:
		return 0x04a000;
	case FS_14700HZ:
		return 0x037800;
	case FS_22050HZ:
		return 0x025000;
	case FS_29400HZ:
		return 0x01BC00;
	case FS_44100HZ:
		return 0x012800;
	case FS_88200HZ:
		return 0x009400;
	case FS_176400HZ:
		return 0x004a00;
	case FS_352800HZ:
		return 0x002500;
	default:
		return 0x0;
	}
}

static inline u32 FrequencyPalette(enum afe_sampling_rate fs)
{
	switch (fs) {
	case FS_8000HZ:
		return 0x050000;
	case FS_12000HZ:
		return 0x078000;
	case FS_16000HZ:
		return 0x0A0000;
	case FS_24000HZ:
		return 0x0F0000;
	case FS_32000HZ:
		return 0x140000;
	case FS_48000HZ:
		return 0x1E0000;
	case FS_96000HZ:
		return 0x3C0000;
	case FS_192000HZ:
		return 0x780000;
	case FS_384000HZ:
		return 0xF00000;
	case FS_7350HZ:
		return 0x049800;
	case FS_11025HZ:
		return 0x06E400;
	case FS_14700HZ:
		return 0x093000;
	case FS_22050HZ:
		return 0x0DC800;
	case FS_29400HZ:
		return 0x126000;
	case FS_44100HZ:
		return 0x1B9000;
	case FS_88200HZ:
		return 0x372000;
	case FS_176400HZ:
		return 0x6E4000;
	case FS_352800HZ:
		return 0xDC8000;
	default:
		return 0x0;
	}
}

static u32 PeriodPalette(enum afe_sampling_rate fs)
{
	switch (fs) {
	case FS_8000HZ:
		return 0x060000;
	case FS_12000HZ:
		return 0x040000;
	case FS_16000HZ:
		return 0x030000;
	case FS_24000HZ:
		return 0x020000;
	case FS_32000HZ:
		return 0x018000;
	case FS_48000HZ:
		return 0x010000;
	case FS_96000HZ:
		return 0x008000;
	case FS_192000HZ:
		return 0x004000;
	case FS_384000HZ:
		return 0x002000;
	case FS_7350HZ:
		return 0x0687D8;
	case FS_11025HZ:
		return 0x045A90;
	case FS_14700HZ:
		return 0x0343EC;
	case FS_22050HZ:
		return 0x022D48;
	case FS_29400HZ:
		return 0x01A1F6;
	case FS_44100HZ:
		return 0x0116A4;
	case FS_88200HZ:
		return 0x008B52;
	case FS_176400HZ:
		return 0x0045A9;
	case FS_352800HZ:
		return 0x0022D4;	/* ??? */
	default:
		return 0x0;
	}
}

#define TBL_SZ_MEMASRC_IIR_COEF (48)

static const u32 IIR_COEF_384_TO_352[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x0c1e4c4d, 0x17660d8a, 0x0c1e4c4d, 0xe2946bc1, 0xf02fc7da, 0x00000003, 0x0c1e543f,
	0x1778413f, 0x0c1e543f, 0xe32da798, 0xf09e4014, 0x00000003, 0x0c1e543f, 0x179b90a7,
	0x0c1e543f, 0xe4459cf5,	0xf14576d1, 0x00000003, 0x0c1e543f, 0x17cc5176, 0x0c1e543f,
	0xe66ec095, 0xf278ca27,	0x00000003, 0x0c1e543f, 0x18017831, 0x0c1e543f, 0xeb20a845,
	0xf504ab81, 0x00000003, 0x0c1e543f, 0x182c2146,	0x0c1e543f, 0xf567b454, 0xfa8dff93,
	0x00000003, 0x00000000, 0x307950fa, 0x307950fa,	0xff0b0b35, 0x00000000, 0x00000001,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

static const u32 IIR_COEF_256_TO_192[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x10b9cde0, 0x180a83a1, 0x10b9cde0, 0xdb9d18b9, 0xe142d9f9, 0x00000002, 0x10b9d8d6,
	0x18d03bcc, 0x10b9d8d6, 0xde3f32cd, 0xe26dbd09, 0x00000002, 0x10b9d8d6, 0x1a0d5c8c,
	0x10b9d8d6, 0xe2d391e2,	0xe4326d09, 0x00000002, 0x10b9d8d6, 0x1bc844f4, 0x10b9d8d6,
	0xea90b7c9, 0xe7079cc2,	0x00000002, 0x10b9d8d6, 0x1de17e15, 0x10b9d8d6, 0xf734d869,
	0xeb90a9cf, 0x00000002, 0x10b9d8d6, 0x1ff06552,	0x10b9d8d6, 0x0977d5bc, 0xf211d78c,
	0x00000002, 0x10b9d8d6, 0x21449ad6, 0x10b9d8d6,	0x1aa484d3, 0xf82c15a1, 0x00000002,
	0x10b9d8d6, 0x17ace769, 0x10b9d8d6, 0xda3bed4c, 0xe0646921, 0x00000002
};

static const u32 IIR_COEF_352_TO_256[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x0fceedfc, 0x1527bb6c, 0x0fceedfc, 0xdfdb3347, 0xe153db23, 0x00000002, 0x0fcef858,
	0x15fdafc1,
	0x0fcef858, 0xe2a471e0, 0xe28bd5b5, 0x00000002, 0x0fcef858, 0x17570091, 0x0fcef858,
	0xe7760dbc,
	0xe45e890b, 0x00000002, 0x0fcef858, 0x193db76d, 0x0fcef858, 0xef77cb57, 0xe73b8741,
	0x00000002,
	0x0fcef858, 0x1b9320a7, 0x0fcef858, 0xfc28d0cb, 0xebacd099, 0x00000002, 0x0fcef858,
	0x1de4b630,
	0x0fcef858, 0x0dad93bd, 0xf1c25a4c, 0x00000002, 0x0fcef858, 0x1f6811b0, 0x0fcef858,
	0x1d548bd7,
	0xf72e2e06, 0x00000002, 0x0fcef858, 0x14c2c52e, 0x0fcef858, 0xde6a0892, 0xe069ec41,
	0x00000002
};

static const u32 IIR_COEF_384_TO_256[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x0d965de6, 0x0e1b6929, 0x0d965de6, 0xebeea45e, 0xe1762778, 0x00000002, 0x0d965de6,
	0x0f10583c,
	0x0d965de6, 0xeef33d08, 0xe2c5fdeb, 0x00000002, 0x0d965de6, 0x10a3525f, 0x0d965de6,
	0xf4183f9d,
	0xe4ac2fce, 0x00000002, 0x0d965de6, 0x12ebbe05, 0x0d965de6, 0xfc4848d7, 0xe781e025,
	0x00000002,
	0x0d965de6, 0x15d4b2e8, 0x0d965de6, 0x08572f40, 0xeb942a9a, 0x00000002, 0x0d965de6,
	0x18d8dd11,
	0x0d965de6, 0x175519fb, 0xf097946e, 0x00000002, 0x0d965de6, 0x1ae2f1e2, 0x0d965de6,
	0x2355287c,
	0xf4974161, 0x00000002, 0x0d9654fe, 0x0da8ec0b, 0x0d9654fe, 0xea6be2ff, 0xe0753fab,
	0x00000002
};

static const u32 IIR_COEF_352_TO_192[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x142eeb51, 0x06b34def, 0x142eeb51, 0x0958fc7f, 0xc312ece3, 0x00000001, 0x142ef88b,
	0x08a00c0a,
	0x142ef88b, 0x0ecb0808, 0xc5b979c2, 0x00000001, 0x0a177c46, 0x05f875e0, 0x0a177c46,
	0x0bf28b91,
	0xe4ac05ad, 0x00000002, 0x0a177c46, 0x088e313b, 0x0a177c46, 0x12b40cf3, 0xe72657ed,
	0x00000002,
	0x0a177c46, 0x0c34071a, 0x0a177c46, 0x1b989387, 0xea4e1ee9, 0x00000002, 0x0a177c46,
	0x107a1c9d,
	0x0a177c46, 0x252a40af, 0xeda78dab, 0x00000002, 0x0a177c46, 0x13b3f410, 0x0a177c46,
	0x2bd942ea,
	0xeffb3451, 0x00000002, 0x142ef88b, 0x05d2c87f, 0x142ef88b, 0x06d93bba, 0xc0f8d762,
	0x00000001
};

static const u32 IIR_COEF_384_TO_192[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x1224c87e, 0x00fa2c27, 0x1224c87e, 0x1a888270, 0xc30385a5, 0x00000001, 0x0912643f,
	0x01623909,
	0x0912643f, 0x0fb0a678, 0xe2ca55f1, 0x00000002, 0x0912643f, 0x02f47311, 0x0912643f,
	0x13bbee09,
	0xe483016b, 0x00000002, 0x0912643f, 0x057b2062, 0x0912643f, 0x19a055e9, 0xe6cf26c5,
	0x00000002,
	0x0912643f, 0x0936b62c, 0x0912643f, 0x2126e0d8, 0xe9a388b4, 0x00000002, 0x0912643f,
	0x0ddb8e02,
	0x0912643f, 0x28f5ec28, 0xec87cb7f, 0x00000002, 0x0912643f, 0x1192da43, 0x0912643f,
	0x2e3e61f7,
	0xee798a33, 0x00000002, 0x1224bc9a, 0x002b6673, 0x1224bc9a, 0x18695df9, 0xc0f4978c,
	0x00000001
};

static const u32 IIR_COEF_384_TO_176[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x083cca40, 0xfe542e8b, 0x083cca40, 0x14b89292, 0xe174c45f, 0x00000002, 0x083cca40,
	0xff239101,
	0x083cca40, 0x16d16909, 0xe2af239e, 0x00000002, 0x083cca40, 0x00961458, 0x083cca40,
	0x1a53640e,
	0xe44e9e05, 0x00000002, 0x083cca40, 0x02fbaed5, 0x083cca40, 0x1f5e0902, 0xe66de1cc,
	0x00000002,
	0x083cca40, 0x06b13899, 0x083cca40, 0x25a90d8a, 0xe8f9b5dc, 0x00000002, 0x083cca40,
	0x0b9bedac,
	0x083cca40, 0x2c03ac03, 0xeb80dd55, 0x00000002, 0x083cca40, 0x0fcef360, 0x083cca40,
	0x3036b73a,
	0xed29a138, 0x00000002, 0x083cc4da, 0xfdf7793d, 0x083cc4da, 0x13daaff5, 0xe07667f7,
	0x00000002
};

static const u32 IIR_COEF_256_TO_96[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x07104bfb, 0xfae83473, 0x07104bfb, 0x21d1e3df, 0xe14227d2, 0x00000002, 0x0710509c,
	0xfb817e04,
	0x0710509c, 0x232bcab6, 0xe25192a8, 0x00000002, 0x0710509c, 0xfc9f0cac, 0x0710509c,
	0x257f1f7b,
	0xe3b54771, 0x00000002, 0x0710509c, 0xfe9aa99d, 0x0710509c, 0x28d1ff5c, 0xe57e6839,
	0x00000002,
	0x0710509c, 0x020c4272, 0x0710509c, 0x2ce15dca, 0xe794bb2e, 0x00000002, 0x0710509c,
	0x0779bea6,
	0x0710509c, 0x30dcd07b, 0xe9970e65, 0x00000002, 0x0710509c, 0x0d1feb14, 0x0710509c,
	0x336d8a1f,
	0xeadfd229, 0x00000002, 0x0710509c, 0xfaa52732, 0x0710509c, 0x215c8e5f, 0xe0663754,
	0x00000002
};

static const u32 IIR_COEF_352_TO_128[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x069be373, 0xfad488a8, 0x069be373, 0x24199bda, 0xe144180b, 0x00000002, 0x069be7c7,
	0xfb64ffbc,
	0x069be7c7, 0x255e407c, 0xe2508f4a, 0x00000002, 0x069be7c7, 0xfc71e899, 0x069be7c7,
	0x278c85cd,
	0xe3a9d407, 0x00000002, 0x069be7c7, 0xfe506785, 0x069be7c7, 0x2aa16777, 0xe55cb163,
	0x00000002,
	0x069be7c7, 0x019335ca, 0x069be7c7, 0x2e561d8d, 0xe74ed201, 0x00000002, 0x069be7c7,
	0x06c3c931,
	0x069be7c7, 0x31e9ccc0, 0xe925888b, 0x00000002, 0x069be7c7, 0x0c3d077b, 0x069be7c7,
	0x34306a9f,
	0xea4e6164, 0x00000002, 0x069be7c7, 0xfa954442, 0x069be7c7, 0x23b140c5, 0xe067431a,
	0x00000002
};

static const u32 IIR_COEF_384_TO_128[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x0628c376, 0xfa188dfd, 0x0628c376, 0x286fd8b7, 0xe12fa1d2, 0x00000002, 0x0628c376,
	0xfa938c5b,
	0x0628c376, 0x2971d0c8, 0xe22a1377, 0x00000002, 0x0628c376, 0xfb7b6b03, 0x0628c376,
	0x2b36837d,
	0xe369fbe6, 0x00000002, 0x0628c376, 0xfd21c239, 0x0628c376, 0x2db639f7, 0xe4f93218,
	0x00000002,
	0x0628c376, 0x002207c6, 0x0628c376, 0x30b1fa28, 0xe6bd643a, 0x00000002, 0x0628c376,
	0x054286b4,
	0x0628c376, 0x338bf21e, 0xe863c493, 0x00000002, 0x0628c376, 0x0b31848a, 0x0628c376,
	0x35588810,
	0xe96bb3c3, 0x00000002, 0x0628bf6d, 0xf9e310e4, 0x0628bf6d, 0x282a01bb, 0xe060cc64,
	0x00000002
};

static const u32 IIR_COEF_352_TO_96[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x0606a21a, 0xf845fd43, 0x0606a21a, 0x2ef2ecf7, 0xe0f21f16, 0x00000002, 0x0606a60d,
	0xf89b07c7,
	0x0606a60d, 0x2f7df3f3, 0xe1c0b0b6, 0x00000002, 0x0606a60d, 0xf9410702, 0x0606a60d,
	0x30876e31,
	0xe2d1225b, 0x00000002, 0x0606a60d, 0xfa8191ef, 0x0606a60d, 0x320cb36e, 0xe42ef98b,
	0x00000002,
	0x0606a60d, 0xfd07dfe1, 0x0606a60d, 0x33e87567, 0xe5c473e2, 0x00000002, 0x0606a60d,
	0x023bba76,
	0x0606a60d, 0x35b66128, 0xe7457d7e, 0x00000002, 0x0606a60d, 0x0a36eafd, 0x0606a60d,
	0x36dc8544,
	0xe838742e, 0x00000002, 0x0606a60d, 0xf821a9d8, 0x0606a60d, 0x2ee2831b, 0xe04c7274,
	0x00000002
};

static const u32 IIR_COEF_384_TO_96[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x056c02c1, 0xf87e144d, 0x056c02c1, 0x31e3dd71, 0xe0e93bfb, 0x00000002, 0x056c02c1,
	0xf8c50add,
	0x056c02c1, 0x324b7faa, 0xe1abdcfa, 0x00000002, 0x056c02c1, 0xf94fc78f, 0x056c02c1,
	0x331aefc2,
	0xe2a65112, 0x00000002, 0x056c02c1, 0xfa5da6a8, 0x056c02c1, 0x344aabed, 0xe3dfb554,
	0x00000002,
	0x056c02c1, 0xfc88550b, 0x056c02c1, 0x35b89513, 0xe541dddc, 0x00000002, 0x056c02c1,
	0x0132c6bf,
	0x056c02c1, 0x3715c904, 0xe68ab98f, 0x00000002, 0x056c02c1, 0x08f2afd4, 0x056c02c1,
	0x37f125fa,
	0xe756f5a0, 0x00000002, 0x056bff33, 0xf85fcd96, 0x056bff33, 0x31e59af6, 0xe04a08f0,
	0x00000002
};

static const u32 IIR_COEF_384_TO_88[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x052ee2f7, 0xf8588cce, 0x052ee2f7, 0x34042565, 0xe0d85468, 0x00000002, 0x052ee2f7,
	0xf893f103,
	0x052ee2f7, 0x344cd93d, 0xe18ca6c7, 0x00000002, 0x052ee2f7, 0xf908d5fb, 0x052ee2f7,
	0x34e9dd90,
	0xe2742d5c, 0x00000002, 0x052ee2f7, 0xf9ef3955, 0x052ee2f7, 0x35d3a92e, 0xe394fca9,
	0x00000002,
	0x052ee2f7, 0xfbd59c97, 0x052ee2f7, 0x36ee5164, 0xe4da2527, 0x00000002, 0x052ee2f7,
	0x002bcc50,
	0x052ee2f7, 0x37fbda27, 0xe606ea72, 0x00000002, 0x052ee2f7, 0x08375772, 0x052ee2f7,
	0x38a4e03d,
	0xe6c125ab, 0x00000002, 0x052edf91, 0xf83f4bc8, 0x052edf91, 0x3413133f, 0xe044aa61,
	0x00000002
};

static const u32 IIR_COEF_256_TO_48[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05b132e2, 0xf69d3a3c, 0x05b132e2, 0x372186aa, 0xe0a0e18a, 0x00000002, 0x05b1369d,
	0xf6c4f36e,
	0x05b1369d, 0x3735a71c, 0xe12ff803, 0x00000002, 0x05b1369d, 0xf71564a4, 0x05b1369d,
	0x3777af4d,
	0xe1f446a4, 0x00000002, 0x05b1369d, 0xf7bacb5d, 0x05b1369d, 0x37e581ba, 0xe2f9a198,
	0x00000002,
	0x05b1369d, 0xf932a591, 0x05b1369d, 0x387385b2, 0xe431eb77, 0x00000002, 0x05b1369d,
	0xfd210e04,
	0x05b1369d, 0x3901de4f, 0xe56168cd, 0x00000002, 0x05b1369d, 0x078b8369, 0x05b1369d,
	0x395e1bb1,
	0xe623b33c, 0x00000002, 0x05b1369d, 0xf68c9373, 0x05b1369d, 0x373e17ba, 0xe0322f66,
	0x00000002
};

static const u32 IIR_COEF_352_TO_64[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x051b341f, 0xf780a0ab, 0x051b341f, 0x37e8be91, 0xe0a7c886, 0x00000002, 0x051b3778,
	0xf7a5dedf,
	0x051b3778, 0x37f8aa89, 0xe13799f1, 0x00000002, 0x051b3778, 0xf7f0b52f, 0x051b3778,
	0x38356bf9,
	0xe1f56c0f, 0x00000002, 0x051b3778, 0xf8896965, 0x051b3778, 0x389a6da2, 0xe2e88141,
	0x00000002,
	0x051b3778, 0xf9e21266, 0x051b3778, 0x391a74dc, 0xe4008a1b, 0x00000002, 0x051b3778,
	0xfd7616b8,
	0x051b3778, 0x3997d2e6, 0xe5085351, 0x00000002, 0x051b3778, 0x06cf9520, 0x051b3778,
	0x39e79cd2,
	0xe5ad988a, 0x00000002, 0x051b3778, 0xf770eca7, 0x051b3778, 0x380a2bdb, 0xe034d6da,
	0x00000002
};

static const u32 IIR_COEF_384_TO_64[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x04f22f8f, 0xf782a4f0, 0x04f22f8f, 0x3923e6ba, 0xe09ab828, 0x00000002, 0x04f22f8f,
	0xf7a196e2,
	0x04f22f8f, 0x3925c100, 0xe11f535a, 0x00000002, 0x04f22f8f, 0xf7dff667, 0x04f22f8f,
	0x3949fb66,
	0xe1ce3bd2, 0x00000002, 0x04f22f8f, 0xf8602f0c, 0x04f22f8f, 0x398c0d92, 0xe2adfff0,
	0x00000002,
	0x04f22f8f, 0xf9865c6c, 0x04f22f8f, 0x39e1fd22, 0xe3af7786, 0x00000002, 0x04f22f8f,
	0xfcb5ef73,
	0x04f22f8f, 0x3a36e276, 0xe4a1af89, 0x00000002, 0x04f22f8f, 0x060fd7e2, 0x04f22f8f,
	0x3a6d1360,
	0xe5394a6c, 0x00000002, 0x04f22c51, 0xf775aa9f, 0x04f22c51, 0x3949a8da, 0xe030b721,
	0x00000002
};

static const u32 IIR_COEF_352_TO_48[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05247bf6, 0xf6afaafc, 0x05247bf6, 0x3b23c3de, 0xe07784e2, 0x00000002, 0x05247f55,
	0xf6c3e399,
	0x05247f55, 0x3b106a6d, 0xe0e1d4ce, 0x00000002, 0x05247f55, 0xf6ed4b44, 0x05247f55,
	0x3b0b1955,
	0xe1737be5, 0x00000002, 0x05247f55, 0xf7444b41, 0x05247f55, 0x3b101ecb, 0xe234fa1b,
	0x00000002,
	0x05247f55, 0xf813db16, 0x05247f55, 0x3b1c0d05, 0xe31b8751, 0x00000002, 0x05247f55,
	0xfa8db75e,
	0x05247f55, 0x3b29fecf, 0xe3fae7d7, 0x00000002, 0x05247f55, 0x04791078, 0x05247f55,
	0x3b338316,
	0xe4898f1a, 0x00000002, 0x05247f55, 0xf6a73769, 0x05247f55, 0x3b4b13ff, 0xe025418c,
	0x00000002
};

static const u32 IIR_COEF_384_TO_48[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05085c7f, 0xf6bd7040, 0x05085c7f, 0x3bda2ee1, 0xe06e44ce, 0x00000002, 0x05085c7f,
	0xf6ce5cbe,
	0x05085c7f, 0x3bc18ba5, 0xe0d0608f, 0x00000002, 0x05085c7f, 0xf6f10a32, 0x05085c7f,
	0x3bb1d491,
	0xe156cac7, 0x00000002, 0x05085c7f, 0xf73a30b6, 0x05085c7f, 0x3ba73e28, 0xe209595c,
	0x00000002,
	0x05085c7f, 0xf7ea4add, 0x05085c7f, 0x3b9fb145, 0xe2de171f, 0x00000002, 0x05085c7f,
	0xfa12486c,
	0x05085c7f, 0x3b9a689d, 0xe3ac324b, 0x00000002, 0x05085c7f, 0x03a4f796, 0x05085c7f,
	0x3b978e1f,
	0xe42fd002, 0x00000002, 0x05085933, 0xf6b66c60, 0x05085933, 0x3c01f4c8, 0xe0225d89,
	0x00000002
};

static const u32 IIR_COEF_384_TO_44[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05441baf, 0xf62cc594, 0x05441baf, 0x3c65d2a5, 0xe060a83e, 0x00000002, 0x05441baf,
	0xf63ae1c9,
	0x05441baf, 0x3c49b8c6, 0xe0b8dc65, 0x00000002, 0x05441baf, 0xf65801f9, 0x05441baf,
	0x3c30913a,
	0xe134e31d, 0x00000002, 0x05441baf, 0xf6960334, 0x05441baf, 0x3c1654d4, 0xe1ddfef3,
	0x00000002,
	0x05441baf, 0xf72d4b4a, 0x05441baf, 0x3bfa3480, 0xe2ac852f, 0x00000002, 0x05441baf,
	0xf9168769,
	0x05441baf, 0x3bdfeb89, 0xe378ea8f, 0x00000002, 0x05441baf, 0x02be4d2c, 0x05441baf,
	0x3bcf46e4,
	0xe3fd6575, 0x00000002, 0x0544183c, 0xf626f559, 0x0544183c, 0x3c8b729e, 0xe01deb6c,
	0x00000002
};

static const u32 IIR_COEF_352_TO_32[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x04e9a693, 0xf69886fa, 0x04e9a693, 0x3da9ea6e, 0xe04f1333, 0x00000002, 0x04e9a9cb,
	0xf6a13b64,
	0x04e9a9cb, 0x3d88e4d7, 0xe096916f, 0x00000002, 0x04e9a9cb, 0xf6b34334, 0x04e9a9cb,
	0x3d6464bd,
	0xe0fa1bcc, 0x00000002, 0x04e9a9cb, 0xf6d9ce55, 0x04e9a9cb, 0x3d3877fe, 0xe18082ba,
	0x00000002,
	0x04e9a9cb, 0xf739150f, 0x04e9a9cb, 0x3d05ed76, 0xe2232b81, 0x00000002, 0x04e9a9cb,
	0xf87aecb7,
	0x04e9a9cb, 0x3cd554b3, 0xe2c2ee11, 0x00000002, 0x04e9a9cb, 0x0070f81a, 0x04e9a9cb,
	0x3cb6432b,
	0xe329f0e3, 0x00000002, 0x04e9a9cb, 0xf694e3cf, 0x04e9a9cb, 0x3dcdec50, 0xe01887b8,
	0x00000002
};

static const u32 IIR_COEF_384_TO_32[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x04d7f376, 0xf6a8c048, 0x04d7f376, 0x3e071e5b, 0xe0484db1, 0x00000002, 0x04d7f376,
	0xf6aff4af,
	0x04d7f376, 0x3de5b609, 0xe089b344, 0x00000002, 0x04d7f376, 0xf6bed9fd, 0x04d7f376,
	0x3dbf1b46,
	0xe0e4c852, 0x00000002, 0x04d7f376, 0xf6dec133, 0x04d7f376, 0x3d8f714c, 0xe15fd0db,
	0x00000002,
	0x04d7f376, 0xf72df987, 0x04d7f376, 0x3d57eb14, 0xe1f4cb80, 0x00000002, 0x04d7f376,
	0xf83d6862,
	0x04d7f376, 0x3d2239d9, 0xe28733cf, 0x00000002, 0x04d7f376, 0xff89d13a, 0x04d7f376,
	0x3cffcffd,
	0xe2e5a5dd, 0x00000002, 0x04d7f049, 0xf6a5cb3c, 0x04d7f049, 0x3e29a2ce, 0xe0166cd1,
	0x00000002
};

static const u32 IIR_COEF_352_TO_24[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05cc92a3, 0xf4acf514, 0x05cc92a3, 0x3e942e5e, 0xe0319ffe, 0x00000002, 0x05cc9670,
	0xf4b19b8b,
	0x05cc9670, 0x3e764514, 0xe06271a8, 0x00000002, 0x05cc9670, 0xf4bb7edf, 0x05cc9670,
	0x3e4d1ecb,
	0xe0ac7f21, 0x00000002, 0x05cc9670, 0xf4d126e4, 0x05cc9670, 0x3e12cb32, 0xe1198b2f,
	0x00000002,
	0x05cc9670, 0xf50827bd, 0x05cc9670, 0x3dc7100d, 0xe1a91d5f, 0x00000002, 0x05cc9670,
	0xf5cbe170,
	0x05cc9670, 0x3d774db2, 0xe2411846, 0x00000002, 0x05cc9670, 0xfc4f37fd, 0x05cc9670,
	0x3d41247c,
	0xe2a87d37, 0x00000002, 0x05cc9670, 0xf4ab0633, 0x05cc9670, 0x3eae1284, 0xe00f0b95,
	0x00000002
};

static const u32 IIR_COEF_384_TO_24[TBL_SZ_MEMASRC_IIR_COEF] = {
	0x05bfdee5, 0xf4b9698c, 0x05bfdee5, 0x3ed09e53, 0xe02d0b1e, 0x00000002, 0x05bfdee5,
	0xf4bd3c5d,
	0x05bfdee5, 0x3eb3cb47, 0xe0595f05, 0x00000002, 0x05bfdee5, 0xf4c551ae, 0x05bfdee5,
	0x3e8b8881,
	0xe09ca331, 0x00000002, 0x05bfdee5, 0xf4d70a24, 0x05bfdee5, 0x3e521532, 0xe0ffbf93,
	0x00000002,
	0x05bfdee5, 0xf50426c9, 0x05bfdee5, 0x3e07425e, 0xe1825747, 0x00000002, 0x05bfdee5,
	0xf5a5e88b,
	0x05bfdee5, 0x3db850df, 0xe20cb5f6, 0x00000002, 0x05bfdee5, 0xfb5a7f7e, 0x05bfdee5,
	0x3d82a63c,
	0xe26aebe0, 0x00000002, 0x05bfdb20, 0xf4b7e2b6, 0x05bfdb20, 0x3ee8e76d, 0xe00da776,
	0x00000002
};

static u32 freq_new_index[25] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 99, 99, 99, 99, 99, 99, 99,
	9, 10, 11, 12, 13, 14, 15, 16,
	17
};

static const u32 *iir_coef_tbl_list[23] = {
	IIR_COEF_384_TO_352,	/* 0 */
	IIR_COEF_256_TO_192,	/* 1 */
	IIR_COEF_352_TO_256,	/* 2 */
	IIR_COEF_384_TO_256,	/* 3 */
	IIR_COEF_352_TO_192,	/* 4 */
	IIR_COEF_384_TO_192,	/* 5 */
	IIR_COEF_384_TO_176,	/* 6 */
	IIR_COEF_256_TO_96,	/* 7 */
	IIR_COEF_352_TO_128,	/* 8 */
	IIR_COEF_384_TO_128,	/* 9 */
	IIR_COEF_352_TO_96,	/* 10 */
	IIR_COEF_384_TO_96,	/* 11 */
	IIR_COEF_384_TO_88,	/* 12 */
	IIR_COEF_256_TO_48,	/* 13 */
	IIR_COEF_352_TO_64,	/* 14 */
	IIR_COEF_384_TO_64,	/* 15 */
	IIR_COEF_352_TO_48,	/* 16 */
	IIR_COEF_384_TO_48,	/* 17 */
	IIR_COEF_384_TO_44,	/* 18 */
	IIR_COEF_352_TO_32,	/* 19 */
	IIR_COEF_384_TO_32,	/* 20 */
	IIR_COEF_352_TO_24,	/* 21 */
	IIR_COEF_384_TO_24	/* 22 */
};

#define RATIOVER 23
#define INV_COEF 24
#define NO_NEED 25

static u32 iir_coef_tbl_matrix[18][18] = {
	{
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 0,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		3, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, INV_COEF, 0,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		5, 1, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 6, INV_COEF, 0,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		9, 5, 3, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, INV_COEF, 6, INV_COEF, 0,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		11, 7, 5, 1, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 12, INV_COEF, 6, INV_COEF, 0,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		15, 11, 9, 5, 3, NO_NEED, NO_NEED, NO_NEED, NO_NEED, INV_COEF, 12, INV_COEF, 6, INV_COEF,
		0, NO_NEED, NO_NEED, NO_NEED
	},
	{
		20, 17, 15, 11, 9, 5, NO_NEED, NO_NEED, NO_NEED, INV_COEF, 18, INV_COEF, 12, INV_COEF, 6,
		0, NO_NEED, NO_NEED
	},
	{
		RATIOVER, 22, 20, 17, 15, 11, 5, NO_NEED, NO_NEED, RATIOVER, RATIOVER, INV_COEF, 18,
		INV_COEF, 12, 6, 0, NO_NEED
	},
	{
		RATIOVER, RATIOVER, RATIOVER, 22, 20, 17, 11, 5, NO_NEED, RATIOVER, RATIOVER, RATIOVER,
		RATIOVER, INV_COEF, 18, 12, 6, 0
	},
	{
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		2, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 3, NO_NEED,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		4, INV_COEF, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 5, 1, NO_NEED,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		8, 4, 2, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 9, 5, 3, NO_NEED, NO_NEED,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		10, INV_COEF, 4, INV_COEF, NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 11, 7, 5, 1,
		NO_NEED, NO_NEED, NO_NEED, NO_NEED, NO_NEED
	},
	{
		14, 10, 8, 4, 2, NO_NEED, NO_NEED, NO_NEED, NO_NEED, 15, 11, 9, 5, 3, NO_NEED, NO_NEED,
		NO_NEED, NO_NEED
	},
	{
		19, 16, 14, 10, 8, 4, NO_NEED, NO_NEED, NO_NEED, 20, 17, 15, 11, 9, 5, NO_NEED, NO_NEED,
		NO_NEED
	},
	{
		RATIOVER, 21, 19, 16, 14, 10, 4, NO_NEED, NO_NEED, RATIOVER, 22, 20, 17, 15, 11, 5,
		NO_NEED, NO_NEED
	},
	{
		RATIOVER, RATIOVER, RATIOVER, 21, 19, 16, 10, 4, NO_NEED, RATIOVER, RATIOVER, RATIOVER, 22,
		20, 17, 11, 5, NO_NEED
	}
};

static const u32 *get_iir_coef(enum afe_sampling_rate input_fs, enum afe_sampling_rate output_fs)
{
	const u32 *coef = NULL;
	u32 i = freq_new_index[input_fs];
	u32 j = freq_new_index[output_fs];

	if ((17 < i) || (17 < j)) {
		pr_debug
		("%s() error: freq_new_index[] invalid, Input[0x%x] = %d, Output[0x%x] = %d\n",
		 __func__, input_fs, freq_new_index[input_fs], output_fs,
		 freq_new_index[output_fs]);
	}
	if (25 < iir_coef_tbl_matrix[i][j]) {
		pr_debug
		("%s() error: iir_coef_tbl_matrix invalid, Input[0x%x] = %d, Output[0x%x] = %d\n",
		 __func__, input_fs, fs_integer(input_fs), output_fs, fs_integer(output_fs));
	} else if (RATIOVER == iir_coef_tbl_matrix[i][j]) {
		pr_debug
		("%s() warning: Up-sampling ratio exceeds the 16, Input[0x%x] = %d, Output[0x%x] = %d\n",
		 __func__, input_fs, fs_integer(input_fs), output_fs, fs_integer(output_fs));
	} else if (INV_COEF == iir_coef_tbl_matrix[i][j]) {
		pr_debug
		("%s() warning: Up-sampling ratio is rare, need gen coeff, Input[0x%x] = %d, Output[0x%x] = %d\n",
		 __func__, input_fs, fs_integer(input_fs), output_fs, fs_integer(output_fs));
	} else if (NO_NEED == iir_coef_tbl_matrix[i][j])
		coef = NULL;
	else
		coef = iir_coef_tbl_list[iir_coef_tbl_matrix[i][j]];
	return coef;
}

int afe_sample_asrc_tx_configurate(enum afe_sample_asrc_tx_id id,
				   const struct afe_sample_asrc_config *config)
{
	u32 addrCON0, addrCON1, addrCON4, addrCON6, addrCON7, addrCON10, addrCON11, addrCON13,
	    addrCON14;
	u32 val, msk;
	const u32 *coef;

	switch (id) {
	case SAMPLE_ASRC_O1:
		addrCON0 = AFE_ASRCO1_NEW_CON0;
		addrCON1 = AFE_ASRCO1_NEW_CON1;
		addrCON4 = AFE_ASRCO1_NEW_CON4;
		addrCON6 = AFE_ASRCO1_NEW_CON6;
		addrCON7 = AFE_ASRCO1_NEW_CON7;
		addrCON10 = AFE_ASRCO1_NEW_CON10;
		addrCON11 = AFE_ASRCO1_NEW_CON11;
		addrCON13 = AFE_ASRCO1_NEW_CON13;
		addrCON14 = AFE_ASRCO1_NEW_CON14;
		break;
	case SAMPLE_ASRC_O2:
		addrCON0 = AFE_ASRCO2_NEW_CON0;
		addrCON1 = AFE_ASRCO2_NEW_CON1;
		addrCON4 = AFE_ASRCO2_NEW_CON4;
		addrCON6 = AFE_ASRCO2_NEW_CON6;
		addrCON7 = AFE_ASRCO2_NEW_CON7;
		addrCON10 = AFE_ASRCO2_NEW_CON10;
		addrCON11 = AFE_ASRCO2_NEW_CON11;
		addrCON13 = AFE_ASRCO2_NEW_CON13;
		addrCON14 = AFE_ASRCO2_NEW_CON14;
		break;
	case SAMPLE_ASRC_O3:
		addrCON0 = AFE_ASRCO3_NEW_CON0;
		addrCON1 = AFE_ASRCO3_NEW_CON1;
		addrCON4 = AFE_ASRCO3_NEW_CON4;
		addrCON6 = AFE_ASRCO3_NEW_CON6;
		addrCON7 = AFE_ASRCO3_NEW_CON7;
		addrCON10 = AFE_ASRCO3_NEW_CON10;
		addrCON11 = AFE_ASRCO3_NEW_CON11;
		addrCON13 = AFE_ASRCO3_NEW_CON13;
		addrCON14 = AFE_ASRCO3_NEW_CON14;
		break;
	case SAMPLE_ASRC_O4:
		addrCON0 = AFE_ASRCO4_NEW_CON0;
		addrCON1 = AFE_ASRCO4_NEW_CON1;
		addrCON4 = AFE_ASRCO4_NEW_CON4;
		addrCON6 = AFE_ASRCO4_NEW_CON6;
		addrCON7 = AFE_ASRCO4_NEW_CON7;
		addrCON10 = AFE_ASRCO4_NEW_CON10;
		addrCON11 = AFE_ASRCO4_NEW_CON11;
		addrCON13 = AFE_ASRCO4_NEW_CON13;
		addrCON14 = AFE_ASRCO4_NEW_CON14;
		break;
	case SAMPLE_ASRC_O5:
		addrCON0 = AFE_ASRCO5_NEW_CON0;
		addrCON1 = AFE_ASRCO5_NEW_CON1;
		addrCON4 = AFE_ASRCO5_NEW_CON4;
		addrCON6 = AFE_ASRCO5_NEW_CON6;
		addrCON7 = AFE_ASRCO5_NEW_CON7;
		addrCON10 = AFE_ASRCO5_NEW_CON10;
		addrCON11 = AFE_ASRCO5_NEW_CON11;
		addrCON13 = AFE_ASRCO5_NEW_CON13;
		addrCON14 = AFE_ASRCO5_NEW_CON14;
		break;
	case SAMPLE_ASRC_O6:
		addrCON0 = AFE_ASRCO6_NEW_CON0;
		addrCON1 = AFE_ASRCO6_NEW_CON1;
		addrCON4 = AFE_ASRCO6_NEW_CON4;
		addrCON6 = AFE_ASRCO6_NEW_CON6;
		addrCON7 = AFE_ASRCO6_NEW_CON7;
		addrCON10 = AFE_ASRCO6_NEW_CON10;
		addrCON11 = AFE_ASRCO6_NEW_CON11;
		addrCON13 = AFE_ASRCO6_NEW_CON13;
		addrCON14 = AFE_ASRCO6_NEW_CON14;
		break;
	case SAMPLE_ASRC_PCM_OUT:
		addrCON0 = AFE_ASRCPCMO_NEW_CON0;
		addrCON1 = AFE_ASRCPCMO_NEW_CON1;
		addrCON4 = AFE_ASRCPCMO_NEW_CON4;
		addrCON6 = AFE_ASRCPCMO_NEW_CON6;
		addrCON7 = AFE_ASRCPCMO_NEW_CON7;
		addrCON10 = AFE_ASRCPCMO_NEW_CON10;
		addrCON11 = AFE_ASRCPCMO_NEW_CON11;
		addrCON13 = AFE_ASRCPCMO_NEW_CON13;
		addrCON14 = AFE_ASRCPCMO_NEW_CON14;
		break;
	default:
		pr_err("error: invalid afe_sample_asrc_tx_id\n");
		return -EINVAL;
	}
	/* CON0 setting */
	val = (config->o16bit << 19)
	      | (config->mono << 16)
	      | (0x0 << 14)
	      | (0x3 << 12);
	msk = O16BIT_MASK | IS_MONO_MASK | (0x3 << 14)
	      | (0x3 << 12);
	afe_msk_write(addrCON0, val, msk);
	coef = get_iir_coef(config->input_fs, config->output_fs);
	if (coef) {
		size_t i;
		u32 iir_stage;

		afe_msk_write(addrCON0, 0x1 << 1, 0x1 << 1);	/* CPU control IIR coeff SRAM */
		afe_write(addrCON11, 0x0);	/* set to 0, IIR coeff SRAM addr */
		for (i = 0; i < TBL_SZ_MEMASRC_IIR_COEF; ++i)
			afe_write(addrCON10, coef[i]);
		afe_msk_write(addrCON0, 0x0 << 1, 0x1 << 1);	/* disable IIR coeff SRAM access */
		iir_stage = (coef == IIR_COEF_384_TO_352) ? IIR_STAGE_7 : IIR_STAGE_8;
		afe_msk_write(addrCON0, CLR_IIR_HISTORY | IIR_EN | iir_stage,
			      CLR_IIR_HISTORY_MASK | IIR_EN_MASK | IIR_STAGE_MASK);
	} else
		afe_msk_write(addrCON0, IIR_DIS, IIR_EN_MASK);
	/* CON4 setting (output period) (controlled by HW if tracking) */
	val = PeriodPalette(config->output_fs);
	afe_msk_write(addrCON4, val, 0x00FFFFFF);
	/* CON1 setting (input period) (fixed) */
	val = PeriodPalette(config->input_fs);
	afe_msk_write(addrCON1, val, 0x00FFFFFF);
	/* CON6 setting */
	if (config->tracking)
		afe_write(addrCON6, 0x003F988F);
	else {
		afe_msk_write(addrCON6, (0x0 << 3) | (0x0 << 12) | (0x1 << 1) | (0x0 << 0)
			      , (0x1 << 3) | (0x1 << 12) | (0x1 << 1) | (0x1 << 0));
	}
	/* CON7 setting */
	afe_write(addrCON7, 0x3C00);
	/* CON13 setting */
	val = AutoRstThHi(config->output_fs);
	afe_write(addrCON13, val);
	/* CON14 setting */
	val = AutoRstThLo(config->output_fs);
	afe_write(addrCON14, val);
	return 0;
}

int afe_sample_asrc_tx_enable(enum afe_sample_asrc_tx_id id, int en)
{
	u32 addrCON0;
	static u32 addrCON0s[] = {
		AFE_ASRCO1_NEW_CON0, AFE_ASRCO2_NEW_CON0, AFE_ASRCO3_NEW_CON0, AFE_ASRCO4_NEW_CON0,
		AFE_ASRCO5_NEW_CON0, AFE_ASRCO6_NEW_CON0, AFE_ASRCPCMO_NEW_CON0
	};

	if (id >= SAMPLE_ASRC_OUT_NUM)
		return -EINVAL;
	addrCON0 = addrCON0s[id];
	if (en) {
		afe_msk_write(addrCON0, (0x1 << 4), (0x1 << 4));	/* clear */
		afe_msk_write(addrCON0, (0x1 << 4) | (0x1 << 0), (0x1 << 4) | ASM_ON_MASK);	/* clear and ON */
	} else {
		afe_msk_write(addrCON0, (0x0 << 0), ASM_ON_MASK);	/* OFF */
	}
	return 0;
}

int afe_sample_asrc_rx_configurate(enum afe_sample_asrc_rx_id id,
				   const struct afe_sample_asrc_config *config)
{
	u32 addrCON0, addrCON2, addrCON3, addrCON6, addrCON7, addrCON10, addrCON11, addrCON13,
	    addrCON14;
	u32 val, msk;
	const u32 *coef;

	switch (id) {
	case SAMPLE_ASRC_I1:
		addrCON0 = AFE_ASRC_NEW_CON0;
		addrCON2 = AFE_ASRC_NEW_CON2;
		addrCON3 = AFE_ASRC_NEW_CON3;
		addrCON6 = AFE_ASRC_NEW_CON6;
		addrCON7 = AFE_ASRC_NEW_CON7;
		addrCON10 = AFE_ASRC_NEW_CON10;
		addrCON11 = AFE_ASRC_NEW_CON11;
		addrCON13 = AFE_ASRC_NEW_CON13;
		addrCON14 = AFE_ASRC_NEW_CON14;
		break;
	case SAMPLE_ASRC_I2:
		addrCON0 = AFE_ASRCI2_NEW_CON0;
		addrCON2 = AFE_ASRCI2_NEW_CON2;
		addrCON3 = AFE_ASRCI2_NEW_CON3;
		addrCON6 = AFE_ASRCI2_NEW_CON6;
		addrCON7 = AFE_ASRCI2_NEW_CON7;
		addrCON10 = AFE_ASRCI2_NEW_CON10;
		addrCON11 = AFE_ASRCI2_NEW_CON11;
		addrCON13 = AFE_ASRCI2_NEW_CON13;
		addrCON14 = AFE_ASRCI2_NEW_CON14;
		break;
	case SAMPLE_ASRC_I3:
		addrCON0 = AFE_ASRCI3_NEW_CON0;
		addrCON2 = AFE_ASRCI3_NEW_CON2;
		addrCON3 = AFE_ASRCI3_NEW_CON3;
		addrCON6 = AFE_ASRCI3_NEW_CON6;
		addrCON7 = AFE_ASRCI3_NEW_CON7;
		addrCON10 = AFE_ASRCI3_NEW_CON10;
		addrCON11 = AFE_ASRCI3_NEW_CON11;
		addrCON13 = AFE_ASRCI3_NEW_CON13;
		addrCON14 = AFE_ASRCI3_NEW_CON14;
		break;
	case SAMPLE_ASRC_I4:
		addrCON0 = AFE_ASRCI4_NEW_CON0;
		addrCON2 = AFE_ASRCI4_NEW_CON2;
		addrCON3 = AFE_ASRCI4_NEW_CON3;
		addrCON6 = AFE_ASRCI4_NEW_CON6;
		addrCON7 = AFE_ASRCI4_NEW_CON7;
		addrCON10 = AFE_ASRCI4_NEW_CON10;
		addrCON11 = AFE_ASRCI4_NEW_CON11;
		addrCON13 = AFE_ASRCI4_NEW_CON13;
		addrCON14 = AFE_ASRCI4_NEW_CON14;
		break;
	case SAMPLE_ASRC_I5:
		addrCON0 = AFE_ASRCI5_NEW_CON0;
		addrCON2 = AFE_ASRCI5_NEW_CON2;
		addrCON3 = AFE_ASRCI5_NEW_CON3;
		addrCON6 = AFE_ASRCI5_NEW_CON6;
		addrCON7 = AFE_ASRCI5_NEW_CON7;
		addrCON10 = AFE_ASRCI5_NEW_CON10;
		addrCON11 = AFE_ASRCI5_NEW_CON11;
		addrCON13 = AFE_ASRCI5_NEW_CON13;
		addrCON14 = AFE_ASRCI5_NEW_CON14;
		break;
	case SAMPLE_ASRC_I6:
		addrCON0 = AFE_ASRCI6_NEW_CON0;
		addrCON2 = AFE_ASRCI6_NEW_CON2;
		addrCON3 = AFE_ASRCI6_NEW_CON3;
		addrCON6 = AFE_ASRCI6_NEW_CON6;
		addrCON7 = AFE_ASRCI6_NEW_CON7;
		addrCON10 = AFE_ASRCI6_NEW_CON10;
		addrCON11 = AFE_ASRCI6_NEW_CON11;
		addrCON13 = AFE_ASRCI6_NEW_CON13;
		addrCON14 = AFE_ASRCI6_NEW_CON14;
		break;
	case SAMPLE_ASRC_PCM_IN:
		addrCON0 = AFE_ASRCPCMI_NEW_CON0;
		addrCON2 = AFE_ASRCPCMI_NEW_CON2;
		addrCON3 = AFE_ASRCPCMI_NEW_CON3;
		addrCON6 = AFE_ASRCPCMI_NEW_CON6;
		addrCON7 = AFE_ASRCPCMI_NEW_CON7;
		addrCON10 = AFE_ASRCPCMI_NEW_CON10;
		addrCON11 = AFE_ASRCPCMI_NEW_CON11;
		addrCON13 = AFE_ASRCPCMI_NEW_CON13;
		addrCON14 = AFE_ASRCPCMI_NEW_CON14;
		break;
	default:
		pr_err("error: invalid SAMPLE_ASRC_RX_ID\n");
		return -EINVAL;
	}
	/* CON0 setting */
	val = (config->o16bit << 19)
	      | (config->mono << 16)
	      | (0x1 << 14)
	      | (0x2 << 12);
	msk = O16BIT_MASK | IS_MONO_MASK | (0x3 << 14)
	      | (0x3 << 12);
	afe_msk_write(addrCON0, val, msk);
	coef = get_iir_coef(config->input_fs, config->output_fs);
	if (coef) {
		size_t i;
		u32 iir_stage;

		afe_msk_write(addrCON0, 0x1 << 1, 0x1 << 1);	/* CPU control IIR coeff SRAM */
		afe_write(addrCON11, 0x0);	/* set to 0, IIR coeff SRAM addr */
		for (i = 0; i < TBL_SZ_MEMASRC_IIR_COEF; ++i)
			afe_write(addrCON10, coef[i]);
		afe_msk_write(addrCON0, 0x0 << 1, 0x1 << 1);	/* disable IIR coeff SRAM access */
		iir_stage = (coef == IIR_COEF_384_TO_352) ? IIR_STAGE_7 : IIR_STAGE_8;
		afe_msk_write(addrCON0, CLR_IIR_HISTORY | IIR_EN | iir_stage,
			      CLR_IIR_HISTORY_MASK | IIR_EN_MASK | IIR_STAGE_MASK);
	} else
		afe_msk_write(addrCON0, IIR_DIS, IIR_EN_MASK);
	/* CON3 setting (input fs) (controlled by HW if tracking) */
	val = FrequencyPalette(config->input_fs);
	afe_msk_write(addrCON3, val, 0x00FFFFFF);
	/* CON2 setting (output fs) (fixed) */
	val = FrequencyPalette(config->output_fs);
	afe_msk_write(addrCON2, val, 0x00FFFFFF);
	/* CON6 setting */
	if (config->tracking)
		afe_write(addrCON6, 0x003F988F);
	else {
		afe_msk_write(addrCON6, (0x0 << 3) | (0x0 << 12) | (0x1 << 1) | (0x0 << 0)
			      , (0x1 << 3) | (0x1 << 12) | (0x1 << 1) | (0x1 << 0));
	}
	/* CON7 setting */
	afe_write(addrCON7, 0x3C00);
	/* CON13 setting */
	val = AutoRstThHi(config->input_fs);
	afe_write(addrCON13, val);
	/* CON14 setting */
	val = AutoRstThLo(config->input_fs);
	afe_write(addrCON14, val);
	return 0;
}

int afe_sample_asrc_rx_enable(enum afe_sample_asrc_rx_id id, int en)
{
	u32 addrCON0;
	static u32 addrCON0s[] = {
		AFE_ASRC_NEW_CON0, AFE_ASRCI2_NEW_CON0, AFE_ASRCI3_NEW_CON0, AFE_ASRCI4_NEW_CON0,
		AFE_ASRCI5_NEW_CON0, AFE_ASRCI6_NEW_CON0, AFE_ASRCPCMI_NEW_CON0
	};

	if (id >= SAMPLE_ASRC_IN_NUM)
		return -EINVAL;
	addrCON0 = addrCON0s[id];
	if (en) {
		afe_msk_write(addrCON0, (0x1 << 4), (0x1 << 4));	/* clear */
		afe_msk_write(addrCON0, (0x1 << 4) | (0x1 << 0), (0x1 << 4) | ASM_ON_MASK);	/* clear and ON */
	} else {
		afe_msk_write(addrCON0, (0x0 << 0), ASM_ON_MASK);	/* OFF */
	}
	return 0;
}


/******************** dsdenc ********************/

int afe_power_on_dsdenc(int on)
{
	const int off = !on;

	afe_msk_write(PWR2_TOP_CON, off << PDN_DSD_ENC_POS, PDN_DSD_ENC_MASK);
	return 0;
}

#define DSD_44K_DOMAIN 1
int afe_dsdenc_configurate(enum afe_dsdenc_mode mode)
{
	switch (mode) {
	case DSD128_TO_DSD128:
		afe_write(DSD_ENC_CON1, 0x8000);
		afe_write(DSD_ENC_CON2, 0x0);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x30A00);
#else
		afe_write(DSD_ENC_CON0, 0x10a00);
#endif
		break;
	case DSD128_TO_DSD128_LONGEST:
		afe_write(DSD_ENC_CON1, 0x1000);
		afe_write(DSD_ENC_CON2, 0x4000);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x30A00);
#else
		afe_write(DSD_ENC_CON0, 0x10a00);
#endif
		break;
	case DSD64_TO_DSD128:
		afe_write(DSD_ENC_CON1, 0x8000);
		afe_write(DSD_ENC_CON2, 0x0);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x30800);
#else
		afe_write(DSD_ENC_CON0, 0x10800);
#endif
		break;
	case DSD64_TO_DSD64:
		afe_write(DSD_ENC_CON1, 0x0);
		afe_write(DSD_ENC_CON2, 0x8000);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x70A0C);
#else
		afe_write(DSD_ENC_CON0, 0x50a0c);
#endif
		break;
	case PCM8_TO_DSD128:
		afe_write(DSD_ENC_CON1, 0x8000);
		afe_write(DSD_ENC_CON2, 0x0);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x20000);
#else
		afe_write(DSD_ENC_CON0, 0x0);
#endif
		break;
	case PCM8_TO_DSD64:
		afe_write(DSD_ENC_CON1, 0x0);
		afe_write(DSD_ENC_CON2, 0x8000);
#if DSD_44K_DOMAIN
		afe_write(DSD_ENC_CON0, 0x6000C);
#else
		afe_write(DSD_ENC_CON0, 0x4000c);
#endif
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int afe_dsdenc_enable(int en)
{
	en = !!en;
	afe_msk_write(DSD_ENC_CON0, en << 31, 0x1 << 31);
	return 0;
}


/******************** merge interface ********************/

int afe_daibt_configurate(struct afe_daibt_config *config)
{
	u32 val, msk;

	val = (config->daibt_c << C_POS)
	      | (config->daibt_ready << DATA_RDY_POS)
	      | (config->daibt_mode << DAIBT_MODE_POS)
	      | (config->afe_daibt_input << USE_MRGIF_INPUT_POS);
	msk = C_MASK | DATA_RDY_MASK | DAIBT_MODE_MASK | USE_MRGIF_INPUT_MASK;
	afe_msk_write(AFE_DAIBT_CON0, val, msk);
	afe_write(AFE_BT_SECURITY0, 0x235000);
	afe_write(AFE_BT_SECURITY1, 0x5);
	return 0;
}

int afe_daibt_set_output_fs(enum afe_daibt_output_fs outputfs)
{
	if ((DAIBT_OUTPUT_FS_8K != outputfs) && (DAIBT_OUTPUT_FS_16K != outputfs))
		return -EINVAL;
	afe_msk_write(AFE_DAIBT_CON0, outputfs << DAIBT_MODE_POS, DAIBT_MODE_MASK);
	return 0;
}

void afe_daibt_set_enable(int en)
{
	en = !!en;
	afe_msk_write(AFE_DAIBT_CON0, en << DAIBT_ON_POS, DAIBT_ON_MASK);
}

void afe_merge_set_sync_dly(unsigned int  mrgsyncdly)
{
	mrgsyncdly &= 0xF;
	afe_msk_write(AFE_MRGIF_CON, mrgsyncdly << MRG_SYNC_DLY_POS, MRG_SYNC_DLY_MASK);
}

void afe_merge_set_clk_edge_dly(unsigned int  clkedgedly)
{
	clkedgedly &= 0x3;
	afe_msk_write(AFE_MRGIF_CON, clkedgedly << MRG_CLK_EDGE_DLY_POS, MRG_CLK_EDGE_DLY_MASK);
}

void afe_merge_set_clk_dly(unsigned int  clkdly)
{
	clkdly &= 0x3;
	afe_msk_write(AFE_MRGIF_CON, clkdly << MRG_CLK_DLY_POS, MRG_CLK_DLY_MASK);
}

void afe_merge_i2s_set_mode(enum afe_mrg_i2s_mode i2smode)
{
	afe_msk_write(AFE_MRGIF_CON, i2smode << MRGIF_I2S_MODE_POS, MRGIF_I2S_MODE_MASK);
}

void afe_merge_i2s_enable(int on)
{
	on = !!on;
	afe_msk_write(AFE_MRGIF_CON, on << MRGIF_I2S_EN_POS, MRGIF_I2S_EN_MASK);
}

void afe_merge_i2s_clk_invert(int on)
{
	/* 0:invert,1:non-invert */
	on = !!on;
	afe_msk_write(AFE_MRGIF_CON, on << MRG_CLK_NO_INV_POS, MRG_CLK_NO_INV_MASK);
}

void afe_merge_set_enable(int on)
{
	on = !!on;
	afe_msk_write(AFE_MRGIF_CON, (on << MRGIF_EN_POS), MRGIF_EN_MASK);
}

void afe_i26_pcm_rx_sel_pcmrx(int on)
{
	on = !!on;
	afe_msk_write(AFE_CONN35, on << 31, I26_PCM_RX_SEL_MASK);
}

void afe_o31_pcm_tx_sel_pcmtx(int on)
{
	on = !!on;
	afe_msk_write(AFE_CONN35, on << 30, O31_PCM_TX_SEL_MASK);
}

/******************** pcm interface ********************/

int afe_power_on_btpcm(int on)
{
	const int off = !on;

	afe_msk_write(AUDIO_TOP_CON4, off << 24, PDN_PCMIF);
	return 0;
}

int afe_power_on_mrg(int on)
{
	const int off = !on;

	afe_msk_write(AUDIO_TOP_CON4, off << 25, PDN_MRGIF);
	return 0;
}

int afe_power_on_intdir(int on)
{
	const int off = !on;

	afe_msk_write(AUDIO_TOP_CON4, off << 20, PND_INTDIR);
	return 0;
}

int afe_btpcm_configurate(const struct afe_btpcm_config *config)
{
	u32 val, msk;

	val = (config->fmt << PCM_FMT_POS)
	      | (config->mode << PCM_MODE_POS)
	      | (config->slave << PCM_SLAVE_POS)
	      | (0x0 << BYP_ASRC_POS)
	      | (0x1 << SYNC_TYPE_POS)
	      | (0x1F << SYNC_LENGTH_POS)
	      | (config->extloopback << EXT_MODEM_POS)
	      | (config->wlen << PCM_WLEN_POS);
	msk = PCM_FMT_MASK
	      | PCM_MODE_MASK
	      | PCM_SLAVE_MASK
	      | BYP_ASRC_MASK | SYNC_TYPE_MASK | SYNC_LENGTH_MASK | EXT_MODEM_MASK | PCM_WLEN_MASK;
	afe_msk_write(PCM_INTF_CON1, val, msk);
	return 0;
}

int afe_btpcm_enable(int en)
{
	en = !!en;
	afe_msk_write(PCM_INTF_CON1, en << PCM_EN_POS, PCM_EN_MASK);
	return 0;
}


/******************** dmic ********************/

int afe_power_on_dmic(enum afe_dmic_id id, int on)
{
	const int off = !on;

	pr_debug("%s() for dmic%d\n", __func__, (id + 1));
	afe_msk_write(PWR2_TOP_CON, off << (PDN_DMIC1_POS + id), (PDN_DMIC1_MASK << id));
	return 0;
}

int afe_dmic_configurate(enum afe_dmic_id id, const struct afe_dmic_config *config)
{
	u32 addr;
	u32 val, msk;
	enum afe_sampling_rate mode;

	if (id >= AFE_DMIC_NUM)
		return -EINVAL;
	mode = config->voice_mode;
	addr = DMIC_TOP_CON - (id * 0x150);
	val = (1 << DMIC_TIMING_ON_POS)
	      | (config->iir_on << DMIC_IIR_ON_POS)
	      | (config->iir_mode << DMIC_IIR_MODE_POS);
	switch (mode) {
	case FS_8000HZ:
		break;
	case FS_16000HZ:
		val |= (1 << DMIC_VOICE_MODE_POS);
		break;
	case FS_32000HZ:
		val |= (2 << DMIC_VOICE_MODE_POS);
		break;
	case FS_48000HZ:
		val |= (3 << DMIC_VOICE_MODE_POS);
		break;
	case FS_44100HZ:
		val |= ((3 << DMIC_VOICE_MODE_POS) | (1 << DMIC_DMSEL_POS));
		break;
	default:
		return -EINVAL;
	}
	msk = DMIC_TIMING_ON_MASK
	      | DMIC_IIR_ON_MASK | DMIC_IIR_MODE_MASK | DMIC_VOICE_MODE_MASK | DMIC_DMSEL_MASK;
	afe_msk_write(addr, val, msk);
	return 0;
}

int afe_dmic_enable(enum afe_dmic_id id, int en)
{
	pr_debug("%s() for dmic%d\n", __func__, (id + 1));
	en = !!en;
	afe_msk_write(DMIC_TOP_CON - (id * 0x150), en << DMIC_CIC_ON_POS, 0x1 << DMIC_CIC_ON_POS);
	pr_debug("%s() reg value after: 0x%08x\n", __func__, afe_read(DMIC_TOP_CON - (id * 0x150)));
	return 0;
}


/********************* memory asrc *********************/

int afe_power_on_mem_asrc(enum afe_mem_asrc_id id, int on)
{
	int pos;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return -EINVAL;
	}
	if (on) {
		pos = PDN_MEM_ASRC1_POS + id;
		afe_clear_bit(PWR2_TOP_CON, pos);
		afe_clear_bit(PWR2_TOP_CON, 16);
		/* force reset */
		pos = MEM_ASRC_1_RESET_POS + id;
		afe_set_bit(PWR2_ASM_CON2, pos);
		afe_clear_bit(PWR2_ASM_CON2, pos);
		/* asrc_ck select asm_h_ck(208M) */
		pos = id * 3;
		afe_msk_write(PWR2_ASM_CON2, 0x2 << pos, 0x3 << pos);
	} else {
		pos = PDN_MEM_ASRC1_POS + id;
		afe_set_bit(PWR2_TOP_CON, pos);
	}
	return 0;
}

static const unsigned int asrc_irq[MEM_ASRC_NUM] = {
	165, 166, 167, 200, 201
};

int afe_mem_asrc_register_irq(enum afe_mem_asrc_id id, irq_handler_t isr, const char *name, void *dev)
{
	int ret;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid id %u\n", __func__, id);
		return -EINVAL;
	}
	ret = request_irq(asrc_irq[id], isr, IRQF_TRIGGER_LOW, name, dev);
	if (ret)
		pr_err("%s() can't register ISR for mem asrc[%u] (ret=%i)\n", __func__, id, ret);
	return ret;
}

int afe_mem_asrc_unregister_irq(enum afe_mem_asrc_id id, void *dev)
{
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid id %u\n", __func__, id);
		return -EINVAL;
	}
	free_irq(asrc_irq[id], dev);
	return 0;
}

u32 afe_mem_asrc_irq_status(enum afe_mem_asrc_id id)
{
	u32 addr = REG_ASRC_IFR + id * 0x100;

	return afe_read(addr);
}

void afe_mem_asrc_irq_clear(enum afe_mem_asrc_id id, u32 status)
{
	u32 addr = REG_ASRC_IFR + id * 0x100;

	afe_write(addr, status);
}

int afe_mem_asrc_irq_enable(enum afe_mem_asrc_id id, u32 interrupts, int en)
{
	u32 addr = REG_ASRC_IER + id * 0x100;
	u32 val = en ? interrupts : 0;

	afe_msk_write(addr, val, interrupts);
	return 0;
}

static enum afe_sampling_rate freq_to_fs(u32 freq)
{
	if (freq < (0x049800 + 0x050000) / 2)
		return FS_7350HZ;	/* 0x049800 */
	else if (freq < (0x050000 + 0x06E400) / 2)
		return FS_8000HZ;	/* 0x050000 */
	else if (freq < (0x06E400 + 0x078000) / 2)
		return FS_11025HZ;	/* 0x06E400 */
	else if (freq < (0x078000 + 0x093000) / 2)
		return FS_12000HZ;	/* 0x078000 */
	else if (freq < (0x093000 + 0x0A0000) / 2)
		return FS_14700HZ;	/* 0x093000 */
	else if (freq < (0x0A0000 + 0x0DC800) / 2)
		return FS_16000HZ;	/* 0x0A0000 */
	else if (freq < (0x0DC800 + 0x0F0000) / 2)
		return FS_22050HZ;	/* 0x0DC800 */
	else if (freq < (0x0F0000 + 0x126000) / 2)
		return FS_24000HZ;	/* 0x0F0000 */
	else if (freq < (0x126000 + 0x140000) / 2)
		return FS_29400HZ;	/* 0x126000 */
	else if (freq < (0x140000 + 0x1B9000) / 2)
		return FS_32000HZ;	/* 0x140000 */
	else if (freq < (0x1B9000 + 0x1E0000) / 2)
		return FS_44100HZ;	/* 0x1B9000 */
	else if (freq < (0x1E0000 + 0x372000) / 2)
		return FS_48000HZ;	/* 0x1E0000 */
	else if (freq < (0x372000 + 0x3C0000) / 2)
		return FS_88200HZ;	/* 0x372000 */
	else if (freq < (0x3C0000 + 0x6E4000) / 2)
		return FS_96000HZ;	/* 0x3C0000 */
	else if (freq < (0x6E4000 + 0x780000) / 2)
		return FS_176400HZ;	/* 0x6E4000 */
	else if (freq < (0x780000 + 0xDC8000) / 2)
		return FS_192000HZ;	/* 0x780000 */
	else if (freq < (0xDC8000 + 0xF00000) / 2)
		return FS_352800HZ;	/* 0xDC8000 */
	else
		return FS_384000HZ;	/* 0xF00000 */
}

int afe_mem_asrc_configurate(enum afe_mem_asrc_id id, const struct afe_mem_asrc_config *config)
{
	u32 addr_offset;
	const u32 *coef;
	enum afe_sampling_rate input_fs, output_fs;

	if (!config) {
		pr_err("%s() error: invalid config parameter\n", __func__);
		return -EINVAL;
	}
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return -EINVAL;
	}
	if (config->input_buffer.base & 0xF) {
		pr_err("%s() error: input_buffer.base(0x%08x) is not 16 byte align\n",
		       __func__, config->input_buffer.base);
		return -EINVAL;
	}
	if (config->input_buffer.size & 0xF) {
		pr_err("%s() error: input_buffer.size(0x%08x) is not 16 byte align\n",
		       __func__, config->input_buffer.base);
		return -EINVAL;
	}
	if (config->input_buffer.size < 64 || config->input_buffer.size > 0xffff0) {
		pr_err("%s() error: input_buffer.size(0x%08x) is too small or too large\n",
		       __func__, config->input_buffer.size);
		return -EINVAL;
	}
	if (config->output_buffer.base & 0xF) {
		pr_err("%s() error: output_buffer.base(0x%08x) is not 16 byte align\n",
		       __func__, config->output_buffer.base);
		return -EINVAL;
	}
	if (config->output_buffer.size & 0xF) {
		pr_err("%s() error: output_buffer.size(0x%08x) is not 16 byte align\n",
		       __func__, config->output_buffer.base);
		return -EINVAL;
	}
	if (config->output_buffer.size < 64 || config->output_buffer.size > 0xffff0) {
		pr_err("%s() error: output_buffer.size(0x%08x) is too small or too large\n",
		       __func__, config->output_buffer.size);
		return -EINVAL;
	}
	input_fs = freq_to_fs(config->input_buffer.freq);
	output_fs = freq_to_fs(config->output_buffer.freq);
	pr_debug("config->input_buffer.freq=0x%08x(%u)\n", config->input_buffer.freq, input_fs);
	pr_debug("config->output_buffer.freq=0x%08x(%u)\n", config->output_buffer.freq, output_fs);
	addr_offset = id * 0x100;
	/* check whether mem-asrc is running */
	if (afe_read_bits(REG_ASRC_GEN_CONF + addr_offset, POS_ASRC_BUSY, 1) == 1) {
		pr_err("%s() error: asrc[%d] is running\n", __func__, id);
		return -EBUSY;
	}
	/* when there is only 1 block data left in the input buffer, issue interrupt */
	/* times of 512bit. */
	afe_write_bits(REG_ASRC_IBUF_INTR_CNT0 + addr_offset, 0xFF, POS_CH01_IBUF_INTR_CNT, 8);
	/* when there is only 1 block space in the output buffer, issue interrupt */
	/* times of 512bit. 0xFF means if more than 16kB, send interrupt */
	afe_write_bits(REG_ASRC_OBUF_INTR_CNT0 + addr_offset, 0xFF, POS_CH01_OBUF_INTR_CNT, 8);
	/* enable interrupt */
	afe_write(REG_ASRC_IER + addr_offset, IBUF_EMPTY_INT | OBUF_OV_INT);
	/* clear all interrupt flag */
	afe_write(REG_ASRC_IFR + addr_offset,
		  IBUF_EMPTY_INT | OBUF_OV_INT | IBUF_AMOUNT_INT | OBUF_AMOUNT_INT);
	/* iir coeffient setting for down-sample */
	coef = get_iir_coef(input_fs, output_fs);
	if (coef) {
		int i;

		/* turn on IIR coef setting path */
		afe_set_bit(REG_ASRC_GEN_CONF + addr_offset, POS_DSP_CTRL_COEFF_SRAM);
		/* Load Coef */
		afe_write_bits(REG_ASRC_IIR_CRAM_ADDR + addr_offset, 0, POS_ASRC_IIR_CRAM_ADDR, 8);
		for (i = 0; i < TBL_SZ_MEMASRC_IIR_COEF; ++i)
			afe_write(REG_ASRC_IIR_CRAM_DATA + addr_offset, coef[i]);
		/* turn off IIR coe setting path */
		afe_clear_bit(REG_ASRC_GEN_CONF + addr_offset, POS_DSP_CTRL_COEFF_SRAM);
		afe_set_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_CLR_IIR_BUF);	/* clear IIR filter history */
		if (0 == iir_coef_tbl_matrix[freq_new_index[input_fs]][freq_new_index[output_fs]])
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 6, POS_IIR_STAGE, 3);	/* set IIR_stage-1 */
		else
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 7, POS_IIR_STAGE, 3);	/* set IIR_stage-1 */
		afe_set_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_IIR_EN);
		afe_set_bit(REG_ASRC_GEN_CONF + addr_offset, POS_CH_CLEAR);
		afe_set_bit(REG_ASRC_GEN_CONF + addr_offset, POS_CH_EN);
	} else
		afe_clear_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_IIR_EN);
	pr_debug("config->input_buffer.base=0x%08x\n", config->input_buffer.base);
	pr_debug("config->input_buffer.size=0x%08x\n", config->input_buffer.size);
	/* set input buffer's base and size */
	afe_write(REG_ASRC_IBUF_SADR + addr_offset, config->input_buffer.base);
	afe_write_bits(REG_ASRC_IBUF_SIZE + addr_offset, config->input_buffer.size,
		       POS_CH_IBUF_SIZE, 20);
	pr_debug("config->output_buffer.base=0x%08x\n", config->output_buffer.base);
	pr_debug("config->output_buffer.size=0x%08x\n", config->output_buffer.size);
	/* set input buffer's rp and wp */
	afe_write(REG_ASRC_CH01_IBUF_RDPNT + addr_offset, config->input_buffer.base);
	afe_write(REG_ASRC_CH01_IBUF_WRPNT + addr_offset, config->input_buffer.base);
	/* set output buffer's base and size */
	afe_write(REG_ASRC_OBUF_SADR + addr_offset, config->output_buffer.base);
	afe_write_bits(REG_ASRC_OBUF_SIZE + addr_offset, config->output_buffer.size,
		       POS_CH_OBUF_SIZE, 20);
	/* set output buffer's rp and wp */
	afe_write(REG_ASRC_CH01_OBUF_WRPNT + addr_offset, config->output_buffer.base);
	afe_write(REG_ASRC_CH01_OBUF_RDPNT + addr_offset,
		  config->output_buffer.base + config->output_buffer.size - 16);
	if (16 == config->input_buffer.bitwidth)
		afe_set_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_IBIT_WIDTH);	/* 16bit */
	else
		afe_clear_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_IBIT_WIDTH);	/* 32bit */
	if (16 == config->output_buffer.bitwidth)
		afe_set_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_OBIT_WIDTH);	/* 16bit */
	else
		afe_clear_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_OBIT_WIDTH);	/* 32bit */
	if (config->stereo)
		afe_clear_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_MONO);
	else
		afe_set_bit(REG_ASRC_CH01_CNFG + addr_offset, POS_MONO);
	afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 0x8, POS_CLAC_AMOUNT, 8);
	afe_write_bits(REG_ASRC_MAX_OUT_PER_IN0 + addr_offset, 0, POS_CH01_MAX_OUT_PER_IN0, 16);
	if (config->tracking_mode != MEM_ASRC_NO_TRACKING) {
#if 0				/* internal test only */
		u32 u4I2sTestFs =
			(MEM_ASRC_TRACKING_TX ==
			 config->tracking_mode) ? config->input_buffer.fs : config->output_buffer.fs;
		/* Check Freq Cali status */
		if (0 !=
		    afe_read_bits(REG_ASRC_FREQ_CALI_CTRL + addr_offset, POS_FREQ_CALC_BUSY, 1))
			pr_debug("[vAudMemAsrcConfig] Warning! Freq Calibration is busy!\n");
		if (0 != afe_read_bits(REG_ASRC_FREQ_CALI_CTRL + addr_offset, POS_CALI_EN, 1)) {
			pr_debug
			("[vAudMemAsrcConfig] Warning! Freq Calibration is already enabled!\n");
		}
		pr_debug("[vAudMemAsrcConfig] Tracing mode: %s!\n",
			 (MEM_ASRC_TRACKING_TX ==
			  config->tracking_mode) ? "Tracing Tx" : "Tracing Rx");
		/* select i2sin1 slave lrck */
		afe_write_bits(MASM_TRAC_CON1, 1, (POS_MASRC1_CALC_LRCK_SEL + 3 * id), 3);
		/* freq_mode = (denominator/period_mode)*0x800000 */
		afe_write(REG_ASRC_FREQ_CALI_CYC + addr_offset, 0x3F00);
		afe_write(REG_ASRC_CALI_DENOMINATOR + addr_offset, 0x3c00);
		afe_write(REG_ASRC_FREQ_CALI_CTRL + addr_offset, 0x18D00);
		if (MEM_ASRC_TRACKING_TX == config->tracking_mode) {
			afe_clear_bit(REG_ASRC_FREQ_CALI_CTRL + addr_offset, 9);	/* Tx ->Period Mode Bit9 = 0 */
			afe_write_bits(REG_ASRC_FREQUENCY_0 + addr_offset,
				       g_u4PeriodModeVal_Dm48[config->input_buffer.fs], 0, 24);
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 2, POS_IFS, 2);
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 0, POS_OFS, 2);
		} else {
			afe_set_bit(REG_ASRC_FREQ_CALI_CTRL + addr_offset, 9);	/* Rx -> FreqMode   Bit9 = 1 */
			afe_write_bits(REG_ASRC_FREQUENCY_0 + addr_offset,
				       freq_mode_val[config->output_buffer.fs], 0, 24);
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 2, POS_IFS, 2);
			afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 0, POS_OFS, 2);
		}
#else
		pr_err("%s() error: don't support tracking mode\n", __func__);
		return -EINVAL;
#endif
	} else {
		afe_write_bits(REG_ASRC_FREQUENCY_0 + addr_offset, config->input_buffer.freq, 0, 24);
		afe_write_bits(REG_ASRC_FREQUENCY_1 + addr_offset, config->output_buffer.freq, 0, 24);
		afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 0, POS_IFS, 2);
		afe_write_bits(REG_ASRC_CH01_CNFG + addr_offset, 1, POS_OFS, 2);
	}
	return 0;
}

#if 0
static void afe_mem_asrc_dump_registers(enum afe_mem_asrc_id id)
{
	u32 addr_offset = id * 0x100;

	pr_debug("PWR2_TOP_CON           [0x%08x]=0x%08x\n"
		 "PWR2_ASM_CON2          [0x%08x]=0x%08x\n"
		 "REG_ASRC_GEN_CONF      [0x%08x]=0x%08x\n"
		 "REG_ASRC_IBUF_INTR_CNT0[0x%08x]=0x%08x\n"
		 "REG_ASRC_OBUF_INTR_CNT0[0x%08x]=0x%08x\n"
		 "REG_ASRC_CH01_CNFG     [0x%08x]=0x%08x\n"
		 "REG_ASRC_IBUF_SADR     [0x%08x]=0x%08x\n"
		 "REG_ASRC_IBUF_SIZE     [0x%08x]=0x%08x\n"
		 "REG_ASRC_OBUF_SADR     [0x%08x]=0x%08x\n"
		 "REG_ASRC_OBUF_SIZE     [0x%08x]=0x%08x\n"
		 "REG_ASRC_FREQUENCY_0   [0x%08x]=0x%08x\n"
		 "REG_ASRC_FREQUENCY_1   [0x%08x]=0x%08x\n"
		 "REG_ASRC_FREQUENCY_2   [0x%08x]=0x%08x\n"
		 "REG_ASRC_FREQUENCY_3   [0x%08x]=0x%08x\n", PWR2_TOP_CON, afe_read(PWR2_TOP_CON)
		 , PWR2_ASM_CON2, afe_read(PWR2_ASM_CON2)
		 , REG_ASRC_GEN_CONF + addr_offset, afe_read(REG_ASRC_GEN_CONF + addr_offset)
		 , REG_ASRC_IBUF_INTR_CNT0 + addr_offset,
		 afe_read(REG_ASRC_IBUF_INTR_CNT0 + addr_offset)
		 , REG_ASRC_OBUF_INTR_CNT0 + addr_offset,
		 afe_read(REG_ASRC_OBUF_INTR_CNT0 + addr_offset)
		 , REG_ASRC_CH01_CNFG + addr_offset, afe_read(REG_ASRC_CH01_CNFG + addr_offset)
		 , REG_ASRC_IBUF_SADR + addr_offset, afe_read(REG_ASRC_IBUF_SADR + addr_offset)
		 , REG_ASRC_IBUF_SIZE + addr_offset, afe_read(REG_ASRC_IBUF_SIZE + addr_offset)
		 , REG_ASRC_OBUF_SADR + addr_offset, afe_read(REG_ASRC_OBUF_SADR + addr_offset)
		 , REG_ASRC_OBUF_SIZE + addr_offset, afe_read(REG_ASRC_OBUF_SIZE + addr_offset)
		 , REG_ASRC_FREQUENCY_0 + addr_offset, afe_read(REG_ASRC_FREQUENCY_0 + addr_offset)
		 , REG_ASRC_FREQUENCY_1 + addr_offset, afe_read(REG_ASRC_FREQUENCY_1 + addr_offset)
		 , REG_ASRC_FREQUENCY_2 + addr_offset, afe_read(REG_ASRC_FREQUENCY_2 + addr_offset)
		 , REG_ASRC_FREQUENCY_3 + addr_offset, afe_read(REG_ASRC_FREQUENCY_3 + addr_offset)
		);
}
#endif

int afe_mem_asrc_enable(enum afe_mem_asrc_id id, int en)
{
	u32 addr;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid id %u\n", __func__, id);
		return -EINVAL;
	}
	addr = REG_ASRC_GEN_CONF + id * 0x100;
	if (en) {
		afe_set_bit(addr, POS_CH_CLEAR);
		afe_set_bit(addr, POS_CH_EN);
		afe_set_bit(addr, POS_ASRC_EN);
#if 0
		afe_mem_asrc_dump_registers(id);
#endif
	} else {
		afe_clear_bit(addr, POS_CH_EN);
		afe_clear_bit(addr, POS_ASRC_EN);
	}
	return 0;
}

u32 afe_mem_asrc_get_ibuf_rp(enum afe_mem_asrc_id id)
{
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	return afe_read(REG_ASRC_CH01_IBUF_RDPNT + id * 0x100) & (~(u32) 0xF);
}

u32 afe_mem_asrc_get_ibuf_wp(enum afe_mem_asrc_id id)
{
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	return afe_read(REG_ASRC_CH01_IBUF_WRPNT + id * 0x100) & (~(u32) 0xF);
}

int afe_mem_asrc_set_ibuf_wp(enum afe_mem_asrc_id id, u32 p)
{
	u32 addr_offset;
	u32 base;
	u32 size;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return -EINVAL;
	}
	addr_offset = id * 0x100;
	base = afe_read(REG_ASRC_IBUF_SADR + addr_offset);
	size = afe_read(REG_ASRC_IBUF_SIZE + addr_offset);
	if (unlikely(p < base || p >= (base + size))) {
		pr_err
		("%s() error: can't update input buffer's wp:0x%08x (base:0x%08x, size:0x%08x)\n",
		 __func__, p, base, size);
		return -EINVAL;
	}
	afe_write(REG_ASRC_CH01_IBUF_WRPNT + addr_offset, p);
	return 0;
}

u32 afe_mem_asrc_get_obuf_rp(enum afe_mem_asrc_id id)
{
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	return afe_read(REG_ASRC_CH01_OBUF_RDPNT + id * 0x100) & (~(u32) 0xF);
}

u32 afe_mem_asrc_get_obuf_wp(enum afe_mem_asrc_id id)
{
	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	return afe_read(REG_ASRC_CH01_OBUF_WRPNT + id * 0x100) & (~(u32) 0xF);
}

int afe_mem_asrc_set_obuf_rp(enum afe_mem_asrc_id id, u32 p)
{
	u32 addr_offset;
	u32 base;
	u32 size;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return -EINVAL;
	}
	addr_offset = id * 0x100;
	base = afe_read(REG_ASRC_OBUF_SADR + addr_offset);
	size = afe_read(REG_ASRC_OBUF_SIZE + addr_offset);
	if (unlikely(p < base || p >= (base + size))) {
		pr_err
		("%s() error: can't update output buffer's rp:0x%08x (base:0x%08x, size:0x%08x)\n",
		 __func__, p, base, size);
		return -EINVAL;
	}
	afe_write(REG_ASRC_CH01_OBUF_RDPNT + addr_offset, p);
	return 0;
}

int afe_mem_asrc_set_ibuf_freq(enum afe_mem_asrc_id id, u32 freq)
{
	u32 addr = REG_ASRC_FREQUENCY_0 + id * 0x100;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	afe_write_bits(addr, freq, 0, 24);
	return 0;
}

int afe_mem_asrc_set_obuf_freq(enum afe_mem_asrc_id id, u32 freq)
{
	u32 addr = REG_ASRC_FREQUENCY_1 + id * 0x100;

	if (unlikely(id >= MEM_ASRC_NUM)) {
		pr_err("%s() error: invalid asrc[%d]\n", __func__, id);
		return 0;
	}
	afe_write_bits(addr, freq, 0, 24);
	return 0;
}


/********************* hw gain *********************/

int afe_hwgain_init(enum afe_hwgain_id id)
{
	afe_msk_write(AFE_GAIN1_CON1 + id * 0x18, 0x80000, MASK_GAIN_TARGET);	/* 0db */
	afe_msk_write(AFE_GAIN1_CON0 + id * 0x18, 0x5 << POS_GAIN_MODE, MASK_GAIN_MODE);
	afe_msk_write(AFE_GAIN1_CON0 + id * 0x18, 1 << POS_GAIN_ON, MASK_GAIN_ON);
	return 0;
}
int afe_hwgain_gainmode_set(enum afe_hwgain_id id, enum afe_sampling_rate fs)
{
	afe_msk_write(AFE_GAIN1_CON0 + id * 0x18, fs << POS_GAIN_MODE, MASK_GAIN_MODE);
	return 0;
}

void afe_hwgain_set(enum afe_hwgain_id id, struct afe_hw_gain_config *hgcfg)
{
	u32  hgupstep, hgdownstep;

	switch (hgcfg->hgstepdb) {
	case AFE_STEP_DB_1_8:	/* 0.125db */
		hgupstep = 0x81de6;
		hgdownstep = 0x7e2f3;
		break;
	case AFE_STEP_DB_1_4:	/* 0.25db */
		hgupstep = 0x83bcd;
		hgdownstep = 0x7c5e5;
		break;
	case AFE_STEP_DB_1_2:	/* 0.5db */
		hgupstep = 0x8779a;
		hgdownstep = 0x78bca;
		break;
	default:
		break;
	}
	afe_msk_write(AFE_GAIN1_CON0 + id * 0x18,
		      hgcfg->hwgainsteplen << POS_GAIN_SAMPLE_PER_STEP, MASK_GAIN_SAMPLE_PER_STEP);
	afe_msk_write(AFE_GAIN1_CON1 + id * 0x18, hgcfg->hwtargetgain, MASK_GAIN_TARGET);
	afe_msk_write(AFE_GAIN1_CON2 + id * 0x18, hgdownstep, MASK_DOWN_STEP);
	afe_msk_write(AFE_GAIN1_CON3 + id * 0x18, hgupstep, MASK_UP_STEP);
	pr_debug("%s() AFE_GAIN1_CON0 + %d * 0x18:0x%x\n", __func__, id, afe_read(AFE_GAIN1_CON0 + id * 0x18));
	pr_debug("%s() AFE_GAIN1_CON1 + %d * 0x18:0x%x\n", __func__, id, afe_read(AFE_GAIN1_CON1 + id * 0x18));
	pr_debug("%s() AFE_GAIN1_CON2 + %d * 0x18:0x%x\n", __func__, id, afe_read(AFE_GAIN1_CON2 + id * 0x18));
	pr_debug("%s() AFE_GAIN1_CON3 + %d * 0x18:0x%x\n", __func__, id, afe_read(AFE_GAIN1_CON3 + id * 0x18));
}

/*
    hgctl0:
      bit0:hwgainID 0:AFE_HWGAIN_1 ,1:AFE_HWGAIN_2
      bit1:hwgain: enable/disable
      bit2~bit9:hwgainsample per step (0~255)
      bit10~bit11:hwgainsetpdb:0:.125db,1:0.25db,2:0.5db

     hgctl1:
      hwgain :0x0[-inf.dB]~0x80000[0dB]
*/
int afe_hwgain_configurate(u32 hgctl0, u32 hgctl1)
{
	struct afe_hw_gain_config hwcfg;
	enum afe_hwgain_id id = hgctl0 & 0x1;	/* 0:hwgain1,1:hwgain2 */

	hwcfg.hwgainsteplen = (hgctl0 & 0x3fc) >> 2;	/* bit[9~2] */
	hwcfg.hgstepdb = (hgctl0 & 0xc00) >> 10;	/* bit[11~10] */
	hwcfg.hwtargetgain = hgctl1 & 0xfffff;	/* hwgain */
	if ((id >= AFE_HWGAIN_NUM) || (hwcfg.hgstepdb == 3)) {
		pr_err("%s() please check your hwgainid or stepdb setting\n", __func__);
		return -EINVAL;
	}
	if (hwcfg.hwtargetgain > 0x80000)
		hwcfg.hwtargetgain = 0x80000;
	afe_hwgain_set(id, &hwcfg);
	return 0;
}

int afe_hwgain_enable(enum afe_hwgain_id id, int en)
{
	en = !!en;
	afe_msk_write(AFE_GAIN1_CON0 + id * 0x18, (en << POS_GAIN_ON), MASK_GAIN_ON);
	return 0;
}


/**************** ultra low-power ****************/

int lp_configurate(volatile struct lp_info *lp, u32 base, u32 size,
		   unsigned int rate, unsigned int channels, unsigned int bitwidth)
{
	memset((void *)lp, 0x00, sizeof(struct lp_info));
	lp->m.mode = LP_MODE_NORMAL;
	lp->buf.base = lp->buf.hw_pointer = base;
	lp->buf.size = size;
	lp->rate = rate;
	lp->channels = channels;
	lp->bitwidth = bitwidth;
	return 0;
}

u32 lp_hw_offset(volatile struct lp_info *lp)
{
	return lp->buf.hw_pointer - lp->buf.base;
}

int lp_cmd_excute(volatile struct lp_info *lp, unsigned int cmd)
{
	/* lp hardware is busy */ /* todo: add timeout */
	while (lp->cmd != LP_CMD_NONE || !is_cm4_soft0_irq_cleared())
		;
	lp->cmd = cmd;
	trigger_cm4_soft0_irq();
	return 0;
}


/**************** spdif receiver ****************/

#define ISPDIF_FS_SUPPORT_RANGE 9

static volatile struct afe_dir_info spdifrx_state;
static bool spdifrx_inited;

static u32 spdifrx_fscnt[16][9] = {
	/*32k       44.1k        48k             64k             88.2k      96k          128k       176k         192k*/
	{6750 , 4898 , 4500 , 3375 , 2455 , 2250 , 1688 , 1227 , 1125 }, /* 1 subframe*/
	{13500 , 9796 , 9000 , 6750 , 4909 , 4500 , 3375 , 2455 , 2250 }, /* 2 subframe*/
	{27000 , 19592 , 18000 , 13500 , 9818 , 9000 , 6750 , 4909 , 4500 }, /* 4 subframe*/
	{54000 , 39184 , 36000 , 27000 , 19636 , 18000 , 13500 , 9818 , 9000 }, /* 8 subframe*/
	{108000 , 78367 , 72000 , 54000 , 39273 , 36000 , 27000 , 19636 , 18000 }, /* 16 subframe*/
	{216000 , 156735 , 144000 , 108000 , 78546 , 72000 , 54000 , 39273 , 36000 }, /* 32 subframe*/
	{432000 , 313469 , 288000 , 216000 , 157091 , 144000 , 108000 , 78546 , 72000 }, /* 64 subframe*/
	{864000 , 626939 , 576000 , 432000 , 314182 , 288000 , 216000 , 157091 , 144000 }, /* 128 subframe*/
	{1728027 , 1253897 , 1152018 , 864014 , 626949 , 576008 , 432000 , 313469 , 288000 }, /*256 subframe*/
	{3456000 , 2507755 , 2304000 , 1728000 , 1256727 , 1152000 , 864000 , 628364 , 576000 }, /* 512 subframe*/
	{6912000 , 5015510 , 4608000 , 3456000 , 2513455 , 2304000 , 1728000 , 1256727 , 1152000 }, /* 1024 subframe*/
	{13824000 , 10031020 , 9216000 , 6912000 , 5026909 , 4608000 , 3456000 , 2513455 , 2304000 }, /* 2048 subframe*/
	 /* 4096 subframe*/
	{27648000 , 20062041 , 18432000 , 13824000 , 10053818 , 9216000 , 6912000 , 5026909 , 4608000 },
	/* 8192 subframe*/
	{55296000 , 40124082 , 36864000 , 27648000 , 20107636 , 18432000 , 13824000 , 10053818 , 9216000 },
	/* 16384 subframe*/
	{110592000 , 80248163 , 73728000 , 55296000 , 40215272 , 36864000 , 27648000 , 20107636 , 18432000},
	/* 32768 subframe*/
	{221184000 , 160496327 , 147456000 , 110592000 , 80430546 , 73728000 , 55296000 , 40215273 , 36864000}
};

static u32 spdifrx_fsoft[16][9]  = {
	/*32k       44.1k        48k            64k         88.2k       96k         128k        176k        192k*/
	{78, 78, 78, 78, 78, 78, 78, 78, 78 }, /* 1 subframe*/
	{156, 156, 156, 156, 156, 156, 156, 156, 156 }, /* 2 subframe*/
	{312, 312, 312, 312, 312, 312, 312, 312, 312 }, /* 4 subframe*/
	{625, 625, 625, 625, 625, 625, 625, 625, 625 }, /* 8 subframe*/
	{1250, 1250, 1250, 1250, 1250, 1250, 1250, 1250, 1250 }, /* 16 subframe*/
	{2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500 }, /*32 subframe*/
	{5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000 }, /* 64 subframe*/
	{10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000 }, /* 128 subframe*/
	{200000, 45000, 45000, 27000, 20000, 18000, 14000, 10000, 9000 }, /* 256 subframe*/
	{60000, 45000, 45000, 20000, 20000, 20000, 20000, 20000, 20000 }, /* 512 subframe*/
	{80000, 80000, 80000, 80000, 80000, 80000, 80000, 80000, 80000 }, /* 1024 subframe*/
	{160000, 160000, 160000, 160000, 160000, 160000, 160000, 160000, 160000 }, /* 2048 subframe*/
	{320000, 320000, 320000, 320000, 320000, 320000, 320000, 320000, 320000 }, /* 4096 subframe*/
	{640000, 640000, 640000, 640000, 640000, 640000, 640000, 640000, 640000 }, /* 8192 subframe*/
	{1280000, 1280000, 1280000, 1280000, 1280000, 1280000, 1280000, 1280000, 1280000 }, /* 16384 subframe*/
	{2560000, 2560000, 2560000, 2560000, 2560000, 2560000, 2560000, 2560000, 2560000 }  /* 32768 subframe*/
};

const volatile struct afe_dir_info *afe_spdifrx_state(void)
{
	return &spdifrx_state;
}

static void spdifrx_select_port(enum afe_spdifrx_port port)
{
	if (port == SPDIFRX_PORT_OPT)
		afe_write(AFE_SPDIFIN_INT_EXT,
			(afe_read(AFE_SPDIFIN_INT_EXT) & (~MULTI_INPUT_SEL_MASK)) | MULTI_INPUT_SEL_ARC);
	else
		afe_write(AFE_SPDIFIN_INT_EXT,
			(afe_read(AFE_SPDIFIN_INT_EXT) & (~MULTI_INPUT_SEL_MASK)) | MULTI_INPUT_SEL_OPT);
	afe_write(AFE_SPDIFIN_CFG1,
			afe_read(AFE_SPDIFIN_CFG1) & (AFE_SPDIFIN_REAL_OPTICAL) & (AFE_SPDIFIN_SWITCH_REAL_OPTICAL));
	afe_write(AFE_SPDIFIN_CFG1, afe_read(AFE_SPDIFIN_CFG1) | AFE_SPDIFIN_SEL_SPDIFIN_EN |
						AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN | AFE_SPDIFIN_FIFOSTARTPOINT_5);
}

static void spdifrx_clear_vucp(void)
{
	memset(spdifrx_state.c_bit, 0xff, sizeof(spdifrx_state.c_bit));
	memset(spdifrx_state.u_bit, 0xff, sizeof(spdifrx_state.u_bit));
}

static u32 spdifrx_fs_interpreter(u32 fsval)
{
	u8 period, cnt;
	u32 fs = SPDIFIN_OUT_RANGE;
	u32 rangeplus, rangeminus;

	period = (afe_read(AFE_SPDIFIN_BR)&AFE_SPDIFIN_BR_SUBFRAME_MASK) >> 8;
	for (cnt = 0; cnt < ISPDIF_FS_SUPPORT_RANGE; cnt++) {
		rangeplus = (spdifrx_fscnt[period][cnt] + spdifrx_fsoft[period][cnt]);
		rangeminus = (spdifrx_fscnt[period][cnt] - spdifrx_fsoft[period][cnt]);
		rangeplus = (rangeplus * 624) / 432;
		rangeminus = (rangeminus * 624) / 432;
		if ((fsval > rangeminus) && (fsval < rangeplus)) {
			fs = cnt + SPDIFIN_32K; /*from 32k~192k*/
			break;
		}
	}
	if (cnt > ISPDIF_FS_SUPPORT_RANGE) {
		fs = SPDIFIN_OUT_RANGE;
		pr_err("%s()FS Out of Detected Range!\n", __func__);
	}
	return fs;
}

static void (*spdifrx_callback)(void);

void afe_spdifrx_isr(void)
{
	u32 regval1, regval2, regval3, regval4, fsval, chsintflag;
	int i, j;
	bool err;

	regval1 = afe_read(AFE_SPDIFIN_DEBUG3);
	regval2 = afe_read(AFE_SPDIFIN_INT_EXT2);
	regval3 = afe_read(AFE_SPDIFIN_DEBUG1);
	regval4 = afe_read(AFE_SPDIFIN_CFG1);
	chsintflag = afe_read(AFE_SPDIFIN_DEBUG2);
	err = (regval1 & SPDIFIN_ALL_ERR_ERR_STS) ||
		(regval2 & SPDIFIN_LRCK_CHG_INT_STS) || (regval3 & SPDIFIN_DATA_LATCH_ERR);
	if (err) {
		if (spdifrx_state.rate > 0) {
			pr_debug("%s Spdif Rx unlock!\n", __func__);
			if (regval1 & SPDIFIN_ALL_ERR_ERR_STS)
				pr_debug("%s Error is 0x%x\n", __func__, regval1 & SPDIFIN_ALL_ERR_ERR_STS);
			if (regval2 & SPDIFIN_LRCK_CHG_INT_STS)
				pr_debug("%s LRCK Change\n", __func__);
			if (regval3 & SPDIFIN_DATA_LATCH_ERR)
				pr_debug("%s Data Latch error!\n", __func__);
			spdifrx_state.rate = 0;
			spdifrx_clear_vucp();
			if (spdifrx_callback)
				spdifrx_callback();
		}
		/*Disable SpdifRx interrupt disable*/
		afe_msk_write(AFE_SPDIFIN_CFG0, SPDIFIN_INT_DIS | SPDIFIN_DIS, SPDIFIN_INT_EN_MASK | SPDIFIN_EN_MASK);
		/*disable auto lock for tune apll*/
		afe_msk_write(AFE_SPDIFIN_CLK_CFG, SPDIFIN_LOCKED_FLAG,
			      SPDIFIN_AUTOLOCK_EN_MASK | SPDIFIN_APLL_393_MASK | SPDIFIN_LOCKED_FLAG);
		/*Clear interrupt bits*/
		afe_msk_write(AFE_SPDIFIN_EC, SPDIFIN_INT_CLEAR_ALL, SPDIFIN_INT_ERR_CLEAR_MASK);
		/*Disable TimeOut Interrupt*/
		afe_msk_write(AFE_SPDIFIN_CFG1, 0, SPDIFIN_TIMEOUT_INT_EN);
	} else {
		/*Enable Timeout Interrupt*/
		afe_msk_write(AFE_SPDIFIN_CFG1, SPDIFIN_TIMEOUT_INT_EN, SPDIFIN_TIMEOUT_INT_EN);
		/*Set SpdifRx auto lock cfg*/
		afe_msk_write(AFE_SPDIFIN_CLK_CFG, 1,
			      SPDIFIN_AUTOLOCK_EN_MASK | SPDIFIN_LOCKED_FLAG); /*enable auto lock for tune apll*/
		fsval = afe_read(AFE_SPDIFIN_BR_DBG1);
		fsval = spdifrx_fs_interpreter(fsval);
		if (fsval != SPDIFIN_OUT_RANGE) {
			if (spdifrx_state.rate == 0) {
				pr_debug("%s Spdif Rx Lock!\n", __func__);
				/*Disable Spdif Sginal dected function*/
				afe_write(AFE_SPDIFIN_INT_EXT,
					(afe_read(AFE_SPDIFIN_INT_EXT) & (~MULTI_INPUT_DETECT_SEL_MASK)));
				spdifrx_state.rate = fsval;
				pr_debug("%s spdifrx_state.rate =0x%x.\n", __func__, spdifrx_state.rate);
				if (spdifrx_callback)
					spdifrx_callback();
			}
		}
		if (((chsintflag & SPDIFIN_CHANNEL_STATUS_INT_FLAG) != 0) &&
			((regval4 & SPDIFIN_CHSTS_COLLECTION_EN) != 0) && (fsval != SPDIFIN_OUT_RANGE)) {
			for (i = 0; i < 6; i++) {
				u32 temp = afe_read(AFE_SPDIFIN_CHSTS1 + i * 0x4);

				if (temp != spdifrx_state.c_bit[i]) {
					spdifrx_state.c_bit[i] =  temp;
					if (spdifrx_callback)
						spdifrx_callback();
				}
			}
			for (i = 0; i < 2; i++) {
				for (j = 0; j < 6; j++) {
					u32 temp  = afe_read(AFE_SPDIFIN_USERCODE_1 + (i * 6 + j) * 0x4);

					if (temp != spdifrx_state.u_bit[i][j]) {
						spdifrx_state.u_bit[i][j] = temp;
					if (spdifrx_callback)
						spdifrx_callback();
					}
				}
			}
		}
		/*Clear interrupt bits*/
		afe_msk_write(AFE_SPDIFIN_EC, SPDIFIN_INT_CLEAR_ALL, SPDIFIN_INT_ERR_CLEAR_MASK);
	}
	if (err) {
		/*Enable SpdifRx interrupt disable*/
		afe_msk_write(AFE_SPDIFIN_CFG0, SPDIFIN_INT_EN | SPDIFIN_EN, SPDIFIN_INT_EN_MASK | SPDIFIN_EN_MASK);
	}
}

static void spdifrx_irq_enable(int en)
{
	if (en) {
		afe_msk_write(AFE_SPDIFIN_CFG1,
			      SPDIFIN_ALL_ERR_INT_EN | SEL_BCK_SPDIFIN |
			      AFE_SPDIFIN_SEL_SPDIFIN_EN | AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN,
			      SPDIFIN_INT_ERR_EN_MASK | SEL_BCK_SPDIFIN |
			      AFE_SPDIFIN_SEL_SPDIFIN_EN | AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN);
		afe_msk_write(AFE_SPDIFIN_INT_EXT, SPDIFIN_DATALATCH_ERR_EN, SPDIFIN_DATALATCH_ERR_EN_MASK);
		afe_msk_write(AFE_SPDIFIN_INT_EXT2, SPDIFIN_LRCK_CHG_INT_EN, SPDIFIN_LRCK_CHG_INT_MASK);
		afe_msk_write(AFE_SPDIFIN_CFG0,
			      SPDIFIN_EN | SPDIFIN_INT_EN | SPDIFIN_FLIP_EN | 4 << 8 |
			      SPDIFIN_DE_SEL_DECNT | 0xED << 16,
			      SPDIFIN_EN_MASK | SPDIFIN_INT_EN_MASK | SPDIFIN_FLIP_EN_MASK |
			      SPDIFIN_DE_CNT_MASK | SPDIFIN_DE_SEL_MASK | MAX_LEN_NUM_MASK);
	} else {
		afe_msk_write(AFE_SPDIFIN_CFG0, SPDIFRX_INT_DIS | AFE_SPDIFIN_SEL_SPDIFIN_DIS,
			      SPDIFRX_INT_EN | AFE_SPDIFIN_SEL_SPDIFIN_EN);
		afe_msk_write(AFE_SPDIFIN_CFG1,
			      SPDIFIN_ALL_ERR_INT_DIS | SEL_BCK_SPDIFIN |
			      AFE_SPDIFIN_SEL_SPDIFIN_EN | AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN,
			      SPDIFIN_INT_ERR_EN_MASK | SEL_BCK_SPDIFIN |
			      AFE_SPDIFIN_SEL_SPDIFIN_EN | AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN);
		afe_msk_write(AFE_SPDIFIN_INT_EXT, SPDIFIN_DATALATCH_ERR_DIS, SPDIFIN_DATALATCH_ERR_EN_MASK);
	}
}

static void spdifrx_init(enum afe_spdifrx_port port)
{
	if (spdifrx_inited) {
		pr_err("%s() Dir has already inited.\n", __func__);
		return;
	}
	spdifrx_clear_vucp();
	/*Set spdifin clk cfg*/
	#ifdef AUDIO_MEM_IOREMAP
	topckgen_msk_write(CLK_INTDIR_SEL, UNIVPLL_D2, UNIVPLL_D2);
	#else
	afe_msk_write(CLK_INTDIR_SEL, UNIVPLL_D2, CLK_INTDIR_SEL_MASK);/*624M*/
	#endif
	afe_write(AFE_SPDIFIN_FREQ_INF, 0x877986);
	afe_write(AFE_SPDIFIN_FREQ_INF_2, 0x6596ED);
	afe_write(AFE_SPDIFIN_FREQ_INF_3, 0x5A4);
	/*Bitclk recovery enable and lowbound*/
	afe_msk_write(AFE_SPDIFIN_BR,
		      1 | AFE_SPDIFIN_BR_FS_256 | AFE_SPDIFIN_BR_SUBFRAME_256 | AFE_SPDIFIN_BR_TUNE_MODE1 | 0x19 << 12,
		      AFE_SPDIFIN_BRE_MASK | AFE_SPDIFIN_BR_FS_MASK | AFE_SPDIFIN_BR_SUBFRAME_MASK |
		      AFE_SPDIFIN_BR_TUNE_MODE_MASK | AFE_SPDIFIN_BR_LOWBOUND_MASK);
	afe_msk_write(AFE_SPDIFIN_INT_EXT2, 0x10E | SPDIFIN_594MODE_EN,
		      SPDIFIN_LRC_MASK | SPDIFIN_594MODE_EN); /*10E: 432M mode 186:624M Mode*/
	/*HW tune PLL*/
	afe_msk_write(AFE_SPDIFIN_CLK_CFG, SPDIFIN_REF_CALI_RATIO_NOM_624M | SPDIFIN_REF_CALI_RATIO_DENOM_624M ,
		      SPDIFIN_REF_CALI_RATIO_NOM | SPDIFIN_REF_CALI_RATIO_DENOM); /*624M*/
	/*Enable autolock*/
	afe_msk_write(AFE_SPDIFIN_CLK_CFG, SPDIFIN_AUTOLOCK_EN, SPDIFIN_AUTOLOCK_EN_MASK);
	/*Enable Multi-input detect enable*/
	afe_msk_write(AFE_SPDIFIN_INT_EXT,
		      MULTI_INPUT_DETECT_SEL_ARC | MULTI_INPUT_DETECT_SEL_OPT,
		      MULTI_INPUT_DETECT_SEL_MASK);
	afe_power_on_intdir(1);
	spdifrx_select_port(port);
	spdifrx_irq_enable(1);
	spdifrx_inited = 1;
}

static void spdifrx_uninit(void)
{
	if (!spdifrx_inited) {
		pr_err("%s() Dir has already uninited.\n", __func__);
		return;
	}
	afe_msk_write(AFE_SPDIFIN_CLK_CFG,
		      (SPDIFIN_AUTOLOCK_DIS | 0 | SPDIFIN_LOCKED_FLAG),
		      (SPDIFIN_AUTOLOCK_EN_MASK | SPDIFIN_APLL_393_MASK | SPDIFIN_LOCKED_FLAG));
	spdifrx_irq_enable(0);
	afe_power_on_intdir(0);
	spdifrx_inited = 0;
}

void afe_spdifrx_start(enum afe_spdifrx_port port, void (*callback)(void))
{
	switch (port) {
	case SPDIFRX_PORT_OPT:
		#ifdef AUD_PINCTRL_SUPPORTING
		mt2701_GPIO_SPDIF_IN1_Select(SPDIF_MODE1_SPDIF);
		#else
		mt_set_gpio_mode(GPIO201, GPIO_MODE_01); /* spdif in1*/
		#endif
		break;
	case SPDIFRX_PORT_ARC:
		#ifdef AUD_PINCTRL_SUPPORTING
		mt2701_GPIO_SPDIF_IN0_Select(SPDIF_MODE1_SPDIF);
		#else
		mt_set_gpio_mode(GPIO202, GPIO_MODE_01); /* spdif in0*/
		#endif
		break;
	default:
		pr_err("%s() invalid port: %d\n", __func__, port);
		return;
	}
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	enable_clock(MT_CG_AUDIO_INTDIR, "AUDIO"); /*AUDIO_TOP_CON4[10]:pdn_intdir */
	#endif
	spdifrx_callback = callback;
	spdifrx_init(port);
}

void afe_spdifrx_stop(void)
{
	spdifrx_uninit();
	spdifrx_callback = NULL;
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	disable_clock(MT_CG_AUDIO_INTDIR, "AUDIO");
	#endif
}

