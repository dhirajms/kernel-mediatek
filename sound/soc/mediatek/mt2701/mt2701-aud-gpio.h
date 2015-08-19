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

#ifndef _MT2701_AUD_GPIO_H
#define _MT2701_AUD_GPIO_H

#include "mt2701-aud-global.h"
#ifdef AUD_PINCTRL_SUPPORTING
#include <linux/gpio.h>

extern int aud_i2s0_lrck_gpio;

enum I2S0_LRCK_PIN_SEL {
	I2S0_MODE0_GPIO,
	I2S0_MODE1_I2S0
};

enum I2S2_PIN_SEL {
	I2S2_MODE1_I2S2,
	I2S2_MODE4_DMIC
};

enum PCM_PIN_SEL {
	PCM_MODE1_PCM,
	PCM_MODE2_MRG
};

enum SPDIF_PIN_SEL {
	SPDIF_MODE0_GPIO,
	SPDIF_MODE1_SPDIF
};

extern void mt2701_GPIO_probe(void *dev);
extern int mt2701_GPIO_I2S0_LRCK_Select(enum I2S0_LRCK_PIN_SEL pin_select);
extern int mt2701_GPIO_I2S2_Select(enum I2S2_PIN_SEL pin_select);
extern int mt2701_GPIO_PCM_Select(enum PCM_PIN_SEL pin_select);
extern int mt2701_GPIO_SPDIF_IN0_Select(enum SPDIF_PIN_SEL pin_select);
extern int mt2701_GPIO_SPDIF_IN1_Select(enum SPDIF_PIN_SEL pin_select);


#endif
#endif
