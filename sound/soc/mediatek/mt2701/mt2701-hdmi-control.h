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



#ifndef _AUDIO_HDMI_CONTROL_H
#define _AUDIO_HDMI_CONTROL_H
#include <sound/soc.h>
#include "mt2701-aud-global.h"
#ifdef CONFIG_MTK_LEGACY_CLOCK
#include"mt_clkmgr.h"
#endif

#define APLL_DIV_CLK_MASK	(0x7 << 16)
#define HDMI_POWER_UP_MASK	(0x1 << 20)
#define SPDIF1_POWER_UP_MASK	(0x1 << 21)
#define SPDIF2_POWER_UP_MASK	(0x1 << 22)
#define HDMISPDIF_POWER_UP_MASK	(0x1 << 23)


enum mt_afe_mem_context {
	MT_AFE_MEM_I2S = 0,
	MT_AFE_MEM_IEC1,
	MT_AFE_MEM_IEC2,
	MT_AFE_MEM_COUNT
};

typedef enum {
	PCM_OUTPUT_8BIT = 0,
	PCM_OUTPUT_16BIT = 1,
	PCM_OUTPUT_24BIT = 2,
	PCM_OUTPUT_32BIT = 3
} SPDIF_PCM_BITWIDTH;

enum {
	APLL_SPDIF1_CK = 0,
	APLL_SPDIF2_CK = 1,
	APLL_HDMI_CK = 2,
};

typedef enum {
	HDMI_APLL1 = 0,		/* 48K  98.304M */
	HDMI_APLL2 = 1		/* 44.1K 90.3168M */
} APLL_DIV_IOMUX_SELECT_T;

enum {
	HDMI_IN_I20 = 20,
	HDMI_IN_I21,
	HDMI_IN_I22,
	HDMI_IN_I23,
	HDMI_IN_I24,
	HDMI_IN_I25,
	HDMI_IN_I26,
	HDMI_IN_I27,
	HDMI_IN_BASE = HDMI_IN_I20,
	HDMI_IN_MAX = HDMI_IN_I27,
	HDMI_NUM_INPUT = (HDMI_IN_MAX - HDMI_IN_BASE + 1)
};

enum {
	HDMI_OUT_O20 = 20,
	HDMI_OUT_O21,
	HDMI_OUT_O22,
	HDMI_OUT_O23,
	HDMI_OUT_O24,
	HDMI_OUT_O25,
	HDMI_OUT_O26,
	HDMI_OUT_O27,
	HDMI_OUT_O28,
	HDMI_OUT_O29,
	HDMI_OUT_BASE = HDMI_OUT_O20,
	HDMI_OUT_MAX = HDMI_OUT_O29,
	HDMI_NUM_OUTPUT = (HDMI_OUT_MAX - HDMI_OUT_BASE + 1)
};


typedef enum {
	HDMI_DisConnect = 0x0,
	HDMI_Connection = 0x1
} HDMI_INTERCON_STATUS;

enum {
	HDMI_I2S_NOT_DELAY = 0,
	HDMI_I2S_FIRST_BIT_1T_DELAY
};

enum {
	HDMI_I2S_LRCK_NOT_INVERSE = 0,
	HDMI_I2S_LRCK_INVERSE
};

enum {
	HDMI_I2S_BCLK_NOT_INVERSE = 0,
	HDMI_I2S_BCLK_INVERSE
};

void SetHdmiInterConnection(unsigned int ConnectionState, unsigned int Input, unsigned int Output);

void set_hdmi_out_control(unsigned int channels, unsigned int input_bit);

void set_hdmi_out_dsd_control(unsigned int channels, unsigned int dsd_bit);

void set_hdmi_out_control_enable(bool enable);

void set_hdmi_i2s(void);

void set_hdmi_i2s_dsd(void);

void set_hdmi_i2s_enable(bool enable);

void set_hdmi_i2s_to_I2S5(void);

void vAudioClockSetting(unsigned int sample_rate_idx, unsigned int mclk_fs, unsigned int switch_clk,
			SPDIF_PCM_BITWIDTH eBitWidth, unsigned int DSDBCK, unsigned int value);

void init_hdmi_dma_buffer(enum mt_afe_mem_context mem_context, struct snd_pcm_runtime *runtime,
			  struct mt_stream *s);

void reset_hdmi_dma_buffer(enum mt_afe_mem_context mem_context, struct snd_pcm_runtime *runtime,
			   struct mt_stream *s);

void set_data_output_from_iec_enable(unsigned int sample_rate_idx, struct snd_pcm_runtime *runtime);

void set_data_output_from_iec_disable(void);

void set_data_output_from_iec2_enable(unsigned int sample_rate_idx, struct snd_pcm_runtime *runtime);

void set_data_output_from_iec2_disable(void);

extern int spdif_chst_flag;
extern int hdmi_chst_flag;

#endif
