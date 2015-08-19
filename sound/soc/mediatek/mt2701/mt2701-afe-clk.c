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

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include "mt2701-afe-reg.h"
#include "mt2701-aud-global.h"

enum audio_system_clock_type {
	AUDCLK_INFRA_SYS_AUDIO, /*INFRA_PDN[5]*/
	AUDCLK_TOP_AUD_MUX1_SEL,
	AUDCLK_TOP_AUD_MUX2_SEL,
	AUDCLK_TOP_AUDPLL_MUX_SEL,
	AUDCLK_TOP_APLL_SEL,
	AUDCLK_TOP_AUD1PLL_98M,
	AUDCLK_TOP_AUD2PLL_90M,
	AUDCLK_TOP_HADDS2PLL_98M,
	AUDCLK_TOP_HADDS2PLL_294M,
	AUDCLK_TOP_AUDPLL,
	AUDCLK_TOP_AUDPLL_D4,
	AUDCLK_TOP_AUDPLL_D8,
	AUDCLK_TOP_AUDPLL_D16,
	AUDCLK_TOP_AUDPLL_D24,
	AUDCLK_TOP_AUDINTBUS,
	AUDCLK_CLK_26M,
	AUDCLK_TOP_SYSPLL1_D4,
	AUDCLK_TOP_AUD_K1_SRC_SEL,
	AUDCLK_TOP_AUD_K2_SRC_SEL,
	AUDCLK_TOP_AUD_K3_SRC_SEL,
	AUDCLK_TOP_AUD_K4_SRC_SEL,
	AUDCLK_TOP_AUD_K1_SRC_DIV,
	AUDCLK_TOP_AUD_K2_SRC_DIV,
	AUDCLK_TOP_AUD_K3_SRC_DIV,
	AUDCLK_TOP_AUD_K4_SRC_DIV,
	CLOCK_NUM
};


struct audio_clock_attr {
	const char *name;
	const bool prepare_once;
	bool is_prepared;
	struct clk *clock;
};

static struct audio_clock_attr aud_clks[CLOCK_NUM] = {
	[AUDCLK_INFRA_SYS_AUDIO] = {"infra_sys_audio_clk" , true , false , NULL},
	[AUDCLK_TOP_AUD_MUX1_SEL] = {"top_audio_mux1_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD_MUX2_SEL] = {"top_audio_mux2_sel" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL_MUX_SEL] = {"top_audpll_mux_sel" , false , false, NULL},
	[AUDCLK_TOP_APLL_SEL] = {"top_apll_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD1PLL_98M] = {"top_aud1_pll_98M" , false , false, NULL},
	[AUDCLK_TOP_AUD2PLL_90M] = {"top_aud2_pll_90M" , false , false, NULL},
	[AUDCLK_TOP_HADDS2PLL_98M] = {"top_hadds2_pll_98M" , false , false, NULL},
	[AUDCLK_TOP_HADDS2PLL_294M] = {"top_hadds2_pll_294M" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL] = {"top_audpll" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL_D4] = {"top_audpll_d4" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL_D8] = {"top_audpll_d8" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL_D16] = {"top_audpll_d16" , false , false, NULL},
	[AUDCLK_TOP_AUDPLL_D24] = {"top_audpll_d24" , false , false, NULL},
	[AUDCLK_TOP_AUDINTBUS] = {"top_audintbus_sel" , false , false, NULL},
	[AUDCLK_CLK_26M] = {"clk_26m" , false , false, NULL},
	[AUDCLK_TOP_SYSPLL1_D4] = {"top_syspll1_d4" , false , false, NULL},
	[AUDCLK_TOP_AUD_K1_SRC_SEL] = {"top_aud_k1_src_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD_K2_SRC_SEL] = {"top_aud_k2_src_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD_K3_SRC_SEL] = {"top_aud_k3_src_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD_K4_SRC_SEL] = {"top_aud_k4_src_sel" , false , false, NULL},
	[AUDCLK_TOP_AUD_K1_SRC_DIV] = {"top_aud_k1_src_div" , false , false, NULL},
	[AUDCLK_TOP_AUD_K2_SRC_DIV] = {"top_aud_k2_src_div" , false , false, NULL},
	[AUDCLK_TOP_AUD_K3_SRC_DIV] = {"top_aud_k3_src_div" , false , false, NULL},
	[AUDCLK_TOP_AUD_K4_SRC_DIV] = {"top_aud_k4_src_div" , false , false, NULL},
};

int aud_a1sys_hp_ck_cntr;
int aud_a2sys_hp_ck_cntr;
int aud_f_apll_clk_cntr;
int aud_unipll_clk_cntr;

int aud_afe_clk_cntr;
int aud_i2s_clk_cntr;


static DEFINE_SPINLOCK(afe_clk_lock);
static DEFINE_MUTEX(afe_clk_mutex);

int turn_on_i2sin_clock(int id, int on)
{
	int pdn = !on;

	if (id < 0 || id > 5) {
		pr_err("%s() error: i2s id %d\n", __func__, id);
		return -EINVAL;
	}
	/*MT_CG_AUDIO_I2SIN1,	AUDIO_TOP_CON4[0]*/
	/*MT_CG_AUDIO_I2SIN2,	AUDIO_TOP_CON4[1]*/
	/*MT_CG_AUDIO_I2SIN3,	AUDIO_TOP_CON4[2]*/
	/*MT_CG_AUDIO_I2SIN4,	AUDIO_TOP_CON4[3]*/
	/*MT_CG_AUDIO_I2SIN5,	AUDIO_TOP_CON4[4]*/
	/*MT_CG_AUDIO_I2SIN6,	AUDIO_TOP_CON4[5]*/


	 afe_msk_write(AUDIO_TOP_CON4, pdn<<id, 1<<id);

}

int turn_on_i2sout_clock(int id, int on)
{
	int pdn = !on;

	pr_debug("%s\n", __func__);
	if (id < 0 || id > 5) {
		pr_err("%s() error: i2s id %d\n", __func__, id);
		return -EINVAL;
	}
	/*MT_CG_AUDIO_I2SO1,	AUDIO_TOP_CON4[6]*/
	/*MT_CG_AUDIO_I2SO2,	AUDIO_TOP_CON4[7]*/
	/*MT_CG_AUDIO_I2SO3,	AUDIO_TOP_CON4[8]*/
	/*MT_CG_AUDIO_I2SO4,	AUDIO_TOP_CON4[9]*/
	/*MT_CG_AUDIO_I2SO5,	AUDIO_TOP_CON4[10]*/
	/*MT_CG_AUDIO_I2SO6,	AUDIO_TOP_CON4[11]*/

	 afe_msk_write(AUDIO_TOP_CON4, pdn<<(id+6), 1<<(id+6));
}

#if 0 /*duplicate with afe_power_on_sample_asrc_rx/afe_power_on_sample_asrc_tx*/
int turn_on_asrcin_clock(int id, int on)
{
	int pdn = !on;

	if (id < 0 || id > 5) {
		pr_err("%s() error: asrc id %d\n", __func__, id);
		return -EINVAL;
	}
	/*MT_CG_AUDIO_ASRCI1,	AUDIO_TOP_CON4[12]
	/*MT_CG_AUDIO_ASRCI2,	AUDIO_TOP_CON4[13]
	/*MT_CG_AUDIO_ASRCI3,	PWR2_TOP_CON[2]*/
	/*MT_CG_AUDIO_ASRCI4,	PWR2_TOP_CON[3]*/
	/*MT_CG_AUDIO_ASRCI5,	PWR2_TOP_CON[4]*/
	/*MT_CG_AUDIO_ASRCI6,	PWR2_TOP_CON[5]*/
	if (id < 2)  /* ASRCI1,ASRCI2 */
	    afe_msk_write(AUDIO_TOP_CON4, pdn<<(id+12), 1<<(id+12));
	else if (id < 6)  /* ASRCI3,ASRCI4,,ASRCI5,ASRCI6 */
	    afe_msk_write(PWR2_TOP_CON, pdn<<id, 1<<id);
}


int turn_on_asrcout_clock(int id, int on)
{
	int pdn = !on;

	if (id < 0 || id > 5) {
		pr_err("%s() error: asrc id %d\n", __func__, id);
		return -EINVAL;
	}

	/*MT_CG_AUDIO_ASRCO1,	AUDIO_TOP_CON4[14]
	/*MT_CG_AUDIO_ASRCO2,	AUDIO_TOP_CON4[15]
	/*MT_CG_AUDIO_ASRCO3,	PWR2_TOP_CON[6]*/
	/*MT_CG_AUDIO_ASRCO4,	PWR2_TOP_CON[7]*/
	/*MT_CG_AUDIO_ASRCO5,	PWR2_TOP_CON[8]*/
	/*MT_CG_AUDIO_ASRCO6,	PWR2_TOP_CON[9]*/

	if (id < 2)  /* ASRCOUT1,ASRCOUT2 */
	    afe_msk_write(AUDIO_TOP_CON4, pdn<<(id+14), 1<<(id+14));
	else if (id < 6)  /* ASRCOUT3,ASRCOUT4,,ASRCOUT5,ASRCOUT6 */
	    afe_msk_write(PWR2_TOP_CON, pdn<<(id+4), 1<<(id+4));
}
#endif

void turn_on_i2s_clock(void)
{
	int id;

	pr_debug("%s\n", __func__);
	for (id = 0; id < 6;  id++) {
		turn_on_i2sin_clock(id, 1);
		turn_on_i2sout_clock(id, 1);
		/*turn_on_asrcin_clock(id, 1);*/
		/*turn_on_asrcout_clock(id, 1);*/
	}
}

void turn_off_i2s_clock(void)
{
	int id;

	for (id = 0; id < 6;  id++) {
		turn_on_i2sin_clock(id, 0);
		turn_on_i2sout_clock(id, 0);
		/*turn_on_asrcin_clock(id, 0);*/
		/*turn_on_asrcout_clock(id, 0);*/
	}
}

void turn_on_afe_clock(void)
{

	/*MT_CG_INFRA_AUDIO, INFRA_PDN_STA[5]*/
	int ret = clk_enable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);

	ret = clk_prepare_enable(aud_clks[AUDCLK_TOP_AUDINTBUS].clock);
	if (ret)
		pr_err("%s clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[AUDCLK_TOP_AUDINTBUS].name, ret);

	ret = clk_set_parent(aud_clks[AUDCLK_TOP_AUDINTBUS].clock,
		aud_clks[AUDCLK_TOP_SYSPLL1_D4].clock);
	if (ret)
		pr_err("%s clk_set_parent %s-%s fail %d\n",
			__func__, aud_clks[AUDCLK_TOP_AUDINTBUS].name,
			aud_clks[AUDCLK_TOP_SYSPLL1_D4].name, ret);

	if (ret)
		pr_err("%s clk_enable %s fail %d\n",
		       __func__, aud_clks[AUDCLK_INFRA_SYS_AUDIO].name, ret);
	/*MT_CG_AUDIO_AFE,		AUDIO_TOP_CON0[2]*/
	afe_msk_write(AUDIO_TOP_CON0, 0, PDN_AFE);
	/*MT_CG_AUDIO_APLL,		AUDIO_TOP_CON0[23]*/
	afe_msk_write(AUDIO_TOP_CON0, 0, PDN_APLL_CK);
	/*MT_CG_AUDIO_A1SYS,		AUDIO_TOP_CON4[21]*/
	afe_msk_write(AUDIO_TOP_CON4, 0 , PDN_A1SYS);
	/*MT_CG_AUDIO_A2SYS,		AUDIO_TOP_CON4[22]*/
	afe_msk_write(AUDIO_TOP_CON4, 0 , PDN_A2SYS);
	/*MT_CG_AUDIO_AFE_CONN,	AUDIO_TOP_CON4[23]*/
	afe_msk_write(AUDIO_TOP_CON4, 0 , PDN_AFE_CONN);

}

void turn_off_afe_clock(void)
{

	/*MT_CG_INFRA_AUDIO,*/
	clk_disable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);

	clk_disable_unprepare(aud_clks[AUDCLK_TOP_AUDINTBUS].clock);

	/*MT_CG_AUDIO_AFE,		AUDIO_TOP_CON0[2]*/
	afe_msk_write(AUDIO_TOP_CON0, PDN_AFE, PDN_AFE);
	/*MT_CG_AUDIO_APLL,		AUDIO_TOP_CON0[23]*/
	afe_msk_write(AUDIO_TOP_CON0, PDN_APLL_CK, PDN_APLL_CK);
	/*MT_CG_AUDIO_A1SYS,		AUDIO_TOP_CON4[21]*/
	afe_msk_write(AUDIO_TOP_CON4, PDN_A1SYS , PDN_A1SYS);
	/*MT_CG_AUDIO_A2SYS,		AUDIO_TOP_CON4[22]*/
	afe_msk_write(AUDIO_TOP_CON4, PDN_A2SYS, PDN_A2SYS);
	/*MT_CG_AUDIO_AFE_CONN,	AUDIO_TOP_CON4[23]*/
	afe_msk_write(AUDIO_TOP_CON4, PDN_AFE_CONN , PDN_AFE_CONN);
}


void turn_on_a1sys_hp_ck(void)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
		/* TODO: divider setting from CCF */
	topckgen_msk_write(CLK_AUDDIV_0, 0x1 << 16, AUD_A1SYS_K1_MASK);	/* APLL1 DIV Fix:APLL1/2 */
	ret = clk_prepare_enable(aud_clks[AUDCLK_TOP_AUD_MUX1_SEL].clock);
	if (ret)
		pr_err("%s clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[AUDCLK_TOP_AUD_MUX1_SEL].name, ret);

	ret = clk_set_parent(aud_clks[AUDCLK_TOP_AUD_MUX1_SEL].clock,
		aud_clks[AUDCLK_TOP_AUD1PLL_98M].clock);
	if (ret)
		pr_err("%s clk_set_parent %s-%s fail %d\n",
			__func__, aud_clks[AUDCLK_TOP_AUD_MUX1_SEL].name,
			aud_clks[AUDCLK_TOP_AUD1PLL_98M].name, ret);

	ret = clk_enable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	if (ret)
		pr_err("%s clk_enable %s fail %d\n",
			__func__, aud_clks[AUDCLK_INFRA_SYS_AUDIO].name, ret);

#endif
	return ret;

}

void turn_off_a1sys_hp_ck(void)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	clk_disable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	clk_disable_unprepare(aud_clks[AUDCLK_TOP_AUD_MUX1_SEL].clock);
#endif
	return ret;

}

void turn_on_a2sys_hp_ck(void)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	/* TODO: divider setting from CCF*/
	topckgen_msk_write(CLK_AUDDIV_0, 0x1 << 24, AUD_A2SYS_K1_MASK);	/* APLL2 DIV Fix:APLL2/2 */


	ret = clk_prepare_enable(aud_clks[AUDCLK_TOP_AUD_MUX2_SEL].clock);
	if (ret)
		pr_err("%s clk_prepare_enable %s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_AUD_MUX2_SEL].name, ret);

	ret = clk_set_parent(aud_clks[AUDCLK_TOP_AUD_MUX2_SEL].clock,
	aud_clks[AUDCLK_TOP_AUD2PLL_90M].clock);
	if (ret)
		pr_err("%s clk_set_parent %s-%s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_AUD_MUX2_SEL].name,
		aud_clks[AUDCLK_TOP_AUD2PLL_90M].name, ret);

	ret = clk_enable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	if (ret)
		pr_err("%s clk_enable %s fail %d\n",
		 __func__, aud_clks[AUDCLK_INFRA_SYS_AUDIO].name, ret);
#endif
	return ret;

}
void turn_off_a2sys_hp_ck(void)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	clk_disable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	clk_disable_unprepare(aud_clks[AUDCLK_TOP_AUD_MUX2_SEL].clock);
#endif
	return ret;
}

void turn_on_f_apll_ck(unsigned int mux_select, unsigned int divider)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	/*
	Set AUDCLK_TOP_AUDPLL_MUX_SEL =
	CLK_TOP_AUD1PLL_98M /CLK_TOP_AUD2PLL_90M/CLK_TOP_HADDS2PLL_98M /
	CLK_TOP_HADDS2PLL_294M
	*/
	ret = clk_prepare_enable(aud_clks[AUDCLK_TOP_AUDPLL_MUX_SEL].clock);
	if (ret)
		pr_err("%s clk_prepare %s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_AUDPLL_MUX_SEL].name, ret);

	ret = clk_set_parent(aud_clks[AUDCLK_TOP_AUDPLL_MUX_SEL].clock,
	aud_clks[AUDCLK_TOP_AUD1PLL_98M+mux_select].clock);
	if (ret)
		pr_err("%s clk_set_parent %s-%s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_AUDPLL_MUX_SEL].name,
		aud_clks[AUDCLK_TOP_AUD1PLL_98M+mux_select].name, ret);

	/*
	Set AUDCLK_TOP_APLL_SEL =
	AUDCLK_TOP_AUDPLL /AUDCLK_TOP_AUDPLL_D4/AUDCLK_TOP_AUDPLL_D8 /
	AUDCLK_TOP_AUDPLL_D16/AUDCLK_TOP_AUDPLL_D24
	*/
	ret = clk_prepare_enable(aud_clks[AUDCLK_TOP_APLL_SEL].clock);
	if (ret)
		pr_err("%s clk_prepare %s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_APLL_SEL].name, ret);

	ret = clk_set_parent(aud_clks[AUDCLK_TOP_APLL_SEL].clock,
		aud_clks[AUDCLK_TOP_AUDPLL+divider].clock);
	if (ret)
		pr_err("%s clk_set_parent %s-%s fail %d\n",
		__func__, aud_clks[AUDCLK_TOP_APLL_SEL].name,
		aud_clks[AUDCLK_TOP_AUDPLL+divider].name, ret);

	ret = clk_enable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	if (ret)
		pr_err("%s clk_enable %s fail %d\n",
		 __func__, aud_clks[AUDCLK_INFRA_SYS_AUDIO].name, ret);
#endif

	return ret;

}
void turn_off_f_apll_clock(void)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	clk_disable(aud_clks[AUDCLK_INFRA_SYS_AUDIO].clock);
	clk_disable_unprepare(aud_clks[AUDCLK_TOP_AUDPLL_MUX_SEL].clock);
	clk_disable_unprepare(aud_clks[AUDCLK_TOP_APLL_SEL].clock);
#endif
	return ret;

}

void turn_on_unipll_clock(void)
{
	int ret = 0;

#ifdef COMMON_CLOCK_FRAMEWORK_API
	/* TODO*/
#endif
	return ret;

}
void turn_off_unipll_clock(void)
{
	int ret = 0;

#ifdef COMMON_CLOCK_FRAMEWORK_API
	/* TODO*/
#endif
	return ret;

}


int mt_afe_init_clock(void *dev)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	size_t i;

	for (i = 0; i < ARRAY_SIZE(aud_clks); i++) {
		aud_clks[i].clock = devm_clk_get(dev, aud_clks[i].name);
		if (IS_ERR(aud_clks[i].clock)) {
			ret = PTR_ERR(aud_clks[i].clock);
			pr_err("%s devm_clk_get %s fail %d\n", __func__, aud_clks[i].name, ret);
			break;
		}
	}

	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(aud_clks); i++) {
		if (aud_clks[i].prepare_once) {
			ret = clk_prepare(aud_clks[i].clock);
			if (ret) {
				pr_err("%s clk_prepare %s fail %d\n",
				       __func__, aud_clks[i].name, ret);
				break;
			}
			aud_clks[i].is_prepared = true;
		}
	}
#endif
	return ret;
}


void mt_afe_deinit_clock(void *dev)
{
#ifdef COMMON_CLOCK_FRAMEWORK_API
	size_t i;

	pr_debug("%s\n", __func__);
	for (i = 0; i < ARRAY_SIZE(aud_clks); i++) {
		if (aud_clks[i].clock && !IS_ERR(aud_clks[i].clock) && aud_clks[i].is_prepared) {
			clk_unprepare(aud_clks[i].clock);
			aud_clks[i].is_prepared = false;
		}
	}
#endif
}


int mt_i2s_power_on_mclk(int id, int on)
{
	int ret = 0;

	pr_debug("%s\n", __func__);
#ifdef COMMON_CLOCK_FRAMEWORK_API
	/* TODO , wait for shunli*/
	afe_i2s_power_on_mclk(id, on);

#endif
	return ret;

}



void mt_afe_a1sys_hp_ck_on(void)
{

	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_a1sys_hp_ck_cntr:%d\n", __func__, aud_a1sys_hp_ck_cntr);

	if (aud_a1sys_hp_ck_cntr == 0)
		turn_on_a1sys_hp_ck();
	aud_a1sys_hp_ck_cntr++;
	mutex_unlock(&afe_clk_mutex);
}

void mt_afe_a1sys_hp_ck_off(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_a1sys_hp_ck_cntr(%d)\n", __func__, aud_a1sys_hp_ck_cntr);

	aud_a1sys_hp_ck_cntr--;
	if (aud_a1sys_hp_ck_cntr == 0)
		turn_off_a1sys_hp_ck();
	if (aud_a1sys_hp_ck_cntr < 0) {
		pr_err("%s aud_a1sys_hp_ck_cntr:%d<0\n", __func__, aud_a1sys_hp_ck_cntr);
		aud_a1sys_hp_ck_cntr = 0;
	}

	mutex_unlock(&afe_clk_mutex);

}

void mt_afe_a2sys_hp_ck_on(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_a2sys_hp_ck_cntr:%d\n", __func__, aud_a2sys_hp_ck_cntr);

	if (aud_a2sys_hp_ck_cntr == 0)
		turn_on_a2sys_hp_ck();

	aud_a2sys_hp_ck_cntr++;
	mutex_unlock(&afe_clk_mutex);
}

void mt_afe_a2sys_hp_ck_off(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_a1sys_hp_ck_cntr(%d)\n", __func__, aud_a2sys_hp_ck_cntr);

	aud_a2sys_hp_ck_cntr--;
	if (aud_a2sys_hp_ck_cntr == 0)
		turn_off_a2sys_hp_ck();

	if (aud_a2sys_hp_ck_cntr < 0) {
		pr_err("%s aud_a2sys_hp_ck_cntr:%d<0\n", __func__, aud_a2sys_hp_ck_cntr);
		aud_a2sys_hp_ck_cntr = 0;
	}
	mutex_unlock(&afe_clk_mutex);

}

void mt_afe_f_apll_ck_on(unsigned int mux_select, unsigned int divider)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_f_apll_clk_cntr:%d\n", __func__, aud_f_apll_clk_cntr);

	if (aud_f_apll_clk_cntr == 0)
		turn_on_f_apll_ck(mux_select, divider);

	aud_f_apll_clk_cntr++;
	mutex_unlock(&afe_clk_mutex);

}

void mt_afe_f_apll_ck_off(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_f_apll_clk_cntr(%d)\n", __func__, aud_f_apll_clk_cntr);

	aud_f_apll_clk_cntr--;
	if (aud_f_apll_clk_cntr == 0)
		turn_off_f_apll_clock();
	if (aud_f_apll_clk_cntr < 0) {
		pr_err("%s aud_f_apll_clk_cntr:%d<0\n", __func__, aud_f_apll_clk_cntr);
		aud_f_apll_clk_cntr = 0;
	}
	mutex_unlock(&afe_clk_mutex);

}

void mt_afe_unipll_clk_on(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_unipll_clk_cntr:%d\n", __func__, aud_unipll_clk_cntr);

	if (aud_unipll_clk_cntr == 0)
		turn_on_unipll_clock();

	aud_unipll_clk_cntr++;
	mutex_unlock(&afe_clk_mutex);

}

void mt_afe_unipll_clk_off(void)
{
	mutex_lock(&afe_clk_mutex);
	pr_debug("%s aud_unipll_clk_cntr(%d)\n", __func__, aud_unipll_clk_cntr);

	aud_unipll_clk_cntr--;
	if (aud_unipll_clk_cntr == 0)
		turn_off_unipll_clock();

	if (aud_unipll_clk_cntr < 0) {
		pr_err("%s aud_unipll_clk_cntr:%d<0\n", __func__, aud_unipll_clk_cntr);
		aud_unipll_clk_cntr = 0;
	}
	mutex_unlock(&afe_clk_mutex);
}

void mt_afe_main_clk_on(void)
{
	unsigned long flags;

	spin_lock_irqsave(&afe_clk_lock, flags);
	pr_debug("%s aud_afe_clk_cntr:%d\n", __func__, aud_afe_clk_cntr);

	if (aud_afe_clk_cntr == 0)
		turn_on_afe_clock();

	aud_afe_clk_cntr++;

	spin_unlock_irqrestore(&afe_clk_lock, flags);
}

void mt_afe_main_clk_off(void)
{
	unsigned long flags;

	spin_lock_irqsave(&afe_clk_lock, flags);
	pr_debug("%s aud_afe_clk_cntr:%d\n", __func__, aud_afe_clk_cntr);

	aud_afe_clk_cntr--;
	if (aud_afe_clk_cntr == 0)
		turn_off_afe_clock();
	else if (aud_afe_clk_cntr < 0) {
		pr_err("%s aud_afe_clk_cntr:%d<0\n", __func__, aud_afe_clk_cntr);
		aud_afe_clk_cntr = 0;
	}

	spin_unlock_irqrestore(&afe_clk_lock, flags);
}

void mt_afe_i2s_clk_on(void)
{
	unsigned long flags;

	spin_lock_irqsave(&afe_clk_lock, flags);
	pr_debug("%s aud_i2s_clk_cntr:%d\n", __func__, aud_i2s_clk_cntr);

	if (aud_i2s_clk_cntr == 0)
		turn_on_i2s_clock();

	aud_i2s_clk_cntr++;

	spin_unlock_irqrestore(&afe_clk_lock, flags);
}

void mt_afe_i2s_clk_off(void)
{
	unsigned long flags;

	spin_lock_irqsave(&afe_clk_lock, flags);
	pr_debug("%s aud_i2s_clk_cntr:%d\n", __func__, aud_i2s_clk_cntr);

	aud_i2s_clk_cntr--;
	if (aud_i2s_clk_cntr == 0)
		turn_off_i2s_clock();
	else if (aud_i2s_clk_cntr < 0) {
		pr_err("%s aud_i2s_clk_cntr:%d<0\n", __func__, aud_i2s_clk_cntr);
		aud_i2s_clk_cntr = 0;
	}

	spin_unlock_irqrestore(&afe_clk_lock, flags);
}

void mt_turn_on_i2sout_clock(int id, int on)
{
	turn_on_i2sout_clock(id, on);
}

void mt_turn_on_i2sin_clock(int id, int on)
{
	turn_on_i2sin_clock(id, on);
}

