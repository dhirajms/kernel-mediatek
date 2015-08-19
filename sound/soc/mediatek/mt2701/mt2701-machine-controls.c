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
#include <linux/delay.h>
/*#include <mach/mt_gpio.h>*/
#include "mt2701-afe.h"
#include "mt2701-aud-global.h"
#include "mt2701-aud-gpio.h"


static int i2s_passthrough_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

static int i2s_passthrough_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	ucontrol->value.integer.value[0] = priv->factory_mode.i2s_passthrough;
	return 0;
}

static int i2s_passthrough_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int passthrough = ucontrol->value.integer.value[0];
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	if (passthrough < 0 || passthrough > 1) {
		pr_warn("%s() warning: invalid i2s passthrough path\n", __func__);
		return -EINVAL;
	}
	{
		enum afe_i2s_in_id id;
		struct afe_i2s_in_config config = {
			.fpga_test_loop3 = 0,
			.fpga_test_loop = 0,
			.fpga_test_loop2 = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.slave = 0,
			.fmt = FMT_64CYCLE_32BIT_I2S,
			.mode = FS_48000HZ
		};
		for (id = AFE_I2S_IN_1; id <= AFE_I2S_IN_6; ++id) {
			afe_i2s_in_configurate(id, &config);
			afe_i2s_in_enable(id, passthrough);
		}
	}
	{
		enum afe_i2s_out_id id;
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.couple_mode = 1,
			.one_heart_mode = 0,
			.slave = 0,
			.fmt = FMT_64CYCLE_32BIT_I2S,
			.mode = FS_48000HZ
		};
		for (id = AFE_I2S_OUT_1; id <= AFE_I2S_OUT_6; ++id) {
			afe_i2s_out_configurate(id, &config);
			afe_i2s_out_enable(id, passthrough);
		}
	}
	itrcon_connect(I00, O15, passthrough);
	itrcon_connect(I01, O16, passthrough);
	itrcon_connect(I02, O17, passthrough);
	itrcon_connect(I03, O18, passthrough);
	itrcon_connect(I04, O19, passthrough);
	itrcon_connect(I05, O20, passthrough);
	itrcon_connect(I06, O21, passthrough);
	itrcon_connect(I07, O22, passthrough);
	itrcon_connect(I08, O23, passthrough);
	itrcon_connect(I09, O24, passthrough);
	itrcon_connect(I10, O25, passthrough);
	itrcon_connect(I11, O26, passthrough);
	priv->factory_mode.i2s_passthrough = passthrough;
	return 0;
}

static int spdif_passthrough_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 2;
	return 0;
}

static int spdif_passthrough_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	ucontrol->value.integer.value[0] = priv->factory_mode.spdif_passthrough;
	return 0;
}

static int spdif_passthrough_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int passthrough = ucontrol->value.integer.value[0];
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	if (passthrough < SPDIF_OUT2_SOURCE_IEC2 || passthrough > SPDIF_OUT2_SOURCE_COAXIAL_IN) {
		pr_warn("%s() warning: invalid spdif passthrough path\n", __func__);
		return -EINVAL;
	}
	afe_spdif_out2_source_sel((enum spdif_out2_source)passthrough);
	priv->factory_mode.spdif_passthrough = passthrough;
	return 0;
}

static int dmic_passthrough_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

static int dmic_passthrough_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	ucontrol->value.integer.value[0] = priv->factory_mode.dmic_passthrough;
	return 0;
}

static int dmic_passthrough_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int passthrough = ucontrol->value.integer.value[0];
	struct demo_private *priv = snd_soc_card_get_drvdata(snd_kcontrol_chip(kcontrol));

	if (!priv)
		return -EINVAL;
	if (passthrough < 0 || passthrough > 1) {
		pr_warn("%s() warning: invalid dmic passthrough path\n", __func__);
		return -EINVAL;
	}
	{
		enum afe_dmic_id id;
		struct afe_dmic_config config = {
			.one_wire_mode = 1,
			.iir_on = 0,
			.iir_mode = 0,
			.voice_mode = FS_48000HZ
		};
		for (id = AFE_DMIC_1; id <= AFE_DMIC_2; ++id) {
			afe_power_on_dmic(id, passthrough);
			afe_dmic_configurate(id, &config);
			afe_dmic_enable(id, passthrough);
		}
	}
	{
		enum afe_i2s_out_id id;
		struct afe_i2s_out_config config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 0,
			.couple_mode = 1,
			.one_heart_mode = 0,
			.slave = 0,
			.fmt = FMT_64CYCLE_32BIT_I2S,
			.mode = FS_48000HZ
		};
		for (id = AFE_I2S_OUT_1; id <= AFE_I2S_OUT_2; ++id) {
			afe_i2s_out_configurate(id, &config);
			afe_i2s_out_enable(id, passthrough);
		}
	}
	itrcon_connect(I31, O15, passthrough);
	itrcon_connect(I32, O16, passthrough);
	itrcon_connect(I33, O17, passthrough);
	itrcon_connect(I34, O18, passthrough);
	priv->factory_mode.dmic_passthrough = passthrough;
	return 0;
}

static int dsdenc_test_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

static int dsdenc_test_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = 0;
	return 0;
}

static size_t mem_cmp(const u8 *mem1, const u8 *mem2, size_t size)
{
	size_t i;

	for (i = 0; i < size; ++i) {
		if (mem1[i] != mem2[i])
			return i;
	}
	return 0xFFFFFFFF;
}

static int dsdenc_test_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
#include "dsdenc_test_data.c"
	int fun = ucontrol->value.integer.value[0];

	/* fun[ 3: 0] - function id
	 * fun[ 7: 4] - function parameter 1
	 * fun[11: 8] - function parameter 2
	 */
	pr_debug("%s() start test function 0x%08x\n", __func__, fun);
	switch (fun & 0xF) {
	case 0: {
		/* fun=0x0 */
		enum afe_dsdenc_mode mode;

		afe_power_on_dsdenc(1);
		for (mode = DSD128_TO_DSD128; mode <= PCM8_TO_DSD64; ++mode) {
			const u8 *src, *dst;
			#ifdef AUD_K318_MIGRATION
			const u8 *dst_vitrual;
			#endif
			size_t srcSize, dstSize;

			switch (mode) {
			case DSD128_TO_DSD128:
			case DSD128_TO_DSD128_LONGEST:
			case DSD64_TO_DSD64:
				src = (const u8 *)dsd_1_ini;
				srcSize = sizeof(dsd_1_ini);
				break;
			case DSD64_TO_DSD128:
				src = (const u8 *)dsd_2_ini;
				srcSize = sizeof(dsd_2_ini);
				break;
			case PCM8_TO_DSD128:
			case PCM8_TO_DSD64:
				src = (const u8 *)pcm_ini;
				srcSize = sizeof(pcm_ini);
				break;
			default:
				return -EINVAL;
			}
			memcpy((void *)afe_sram_virt(), src, srcSize);
			src = (u8 *) afe_sram_phys();
			dst = src + srcSize;
			#ifdef AUD_K318_MIGRATION
			dst_vitrual = (u8 *)afe_sram_virt() + srcSize;
			#endif
			switch (mode) {
			case DSD128_TO_DSD128:
			case DSD128_TO_DSD128_LONGEST:
			case DSD64_TO_DSD64:
				dstSize = srcSize;
				break;
			case DSD64_TO_DSD128:
				dstSize = srcSize * 2;
				break;
			case PCM8_TO_DSD128:
				/* one second PCM8 data size is   8 * 44.1k(sample/s) * chNum * 4(byte/sample),
				   one second DSD128 data size is   128 * 44.1k(bit/s) * chNum / 8(bit/byte),
				   so PCM8.dataSize : DSD128.dataSize = 2 : 1 */
				dstSize = srcSize / 2;
				break;
			case PCM8_TO_DSD64:
				/* one second PCM8 data size is   8 * 44.1k(sample/s) * chNum * 4(byte/sample),
				   one second DSD64 data size is   64 * 44.1k(bit/s) * chNum / 8(bit/byte),
				   so PCM8.dataSize : DSD64.dataSize = 4 : 1 */
				dstSize = srcSize / 4;
				break;
			default:
				return -EINVAL;
			}
			{
				struct afe_memif_config dsdr_config = {
					.hd_audio = 1,
					.dsd_width = DSD_WIDTH_32BIT,
					.first_bit = MSB_FIRST,
					.channel = STEREO,
					.buffer = {
						.base = (u32) src,
						.size = srcSize
					}
				};
				afe_memif_configurate(AFE_MEM_DSDR, &dsdr_config);
				afe_memif_enable(AFE_MEM_DSDR, 1);
			}
			{
				struct afe_memif_config dsdw_config = {
					.hd_audio = 1,
					.dsd_width = DSD_WIDTH_32BIT,
					.first_bit = MSB_FIRST,
					.channel = STEREO,
					.buffer = {
						.base = (u32) dst,
						.size = dstSize
					}
				};
				afe_memif_configurate(AFE_MEM_DSDW, &dsdw_config);
				afe_memif_enable(AFE_MEM_DSDW, 1);
			}
			afe_dsdenc_configurate(mode);
			afe_dsdenc_enable(1);
			{
				u32 p;

				do {
					afe_memif_pointer(AFE_MEM_DSDW, &p);
				} while (p - (u32) dst < dstSize / 2);	/* wait data filled in */
				afe_memif_enable(AFE_MEM_DSDW, 0);
			}
			{
				static const char *const mode_string[] = {
					"DSD128_TO_DSD128"
					, "DSD128_TO_DSD128_LONGEST"
					, "DSD64_TO_DSD128"
					, "DSD64_TO_DSD64"
					, "PCM8_TO_DSD128"
					, "PCM8_TO_DSD64"
				};
				const u32 *golden_captured[] = {
					golden_DSD128_TO_DSD128
					, golden_DSD128_TO_DSD128_LONGEST
					, golden_DSD64_TO_DSD128
					, golden_DSD64_TO_DSD64
					, golden_PCM8_TO_DSD128
					, golden_PCM8_TO_DSD64
				};
			#ifdef AUD_K318_MIGRATION
			size_t diff_offset =
				mem_cmp((const u8 *)golden_captured[mode] , dst_vitrual,
					dstSize / 2);
			#else
				size_t diff_offset =
					mem_cmp((const u8 *)golden_captured[mode],
						(const u8 *)IO_PHYS_TO_VIRT((u32) dst),
						dstSize / 2);
			#endif
				if (diff_offset != 0xFFFFFFFF) {
					pr_debug("%s() %s FAIL at offset 0x%x\n", __func__,
						 mode_string[mode], diff_offset);
				} else {
					pr_debug("%s() %s PASS\n", __func__,
						 mode_string[mode]);
				}
			}
			/* turn off */
			afe_memif_enable(AFE_MEM_DSDR, 0);
			afe_dsdenc_enable(0);
			msleep(100);
		}
		afe_power_on_dsdenc(0);
		break;
	}
	case 1: {
		afe_power_on_dsdenc(1);
		{
			/* PCM8_TO_DSD64 */
			const u8 *src, *dst;
			size_t srcSize, dstSize;

			src = (const u8 *)pcm_ini;
			srcSize = sizeof(pcm_ini);
			memcpy((void *)afe_sram_virt(), src, srcSize);
			src = (u8 *) afe_sram_phys();
			dst = src + srcSize;
			dstSize = srcSize / 2;
			{
				struct afe_memif_config dsdr_config = {
					.hd_audio = 1,
					.dsd_width = DSD_WIDTH_32BIT,
					.first_bit = MSB_FIRST,
					.channel = STEREO,
					.buffer = {
						.base = (u32) src,
						.size = srcSize
					}
				};
				afe_memif_configurate(AFE_MEM_DSDR, &dsdr_config);
				afe_memif_enable(AFE_MEM_DSDR, 1);
			}
			{
				struct afe_memif_config dsdw_config = {
					.hd_audio = 1,
					.dsd_width = DSD_WIDTH_32BIT,
					.first_bit = MSB_FIRST,
					.channel = STEREO,
					.buffer = {
						.base = (u32) dst,
						.size = dstSize
					}
				};
				afe_memif_configurate(AFE_MEM_DSDW, &dsdw_config);
				afe_memif_enable(AFE_MEM_DSDW, 1);
			}
			afe_dsdenc_configurate(PCM8_TO_DSD64);
			afe_dsdenc_enable(1);
		}
		{
			/* FS_88200HZ  means  64*44100 dsd bck */
			/* FS_176400HZ means 128*44100 dsd bck */
			enum afe_sampling_rate fs =
				(fun & 0xF0) == 0 ? FS_88200HZ : FS_176400HZ;
			int mclk = (((fun & 0xF00) >> 8) + 1) * 128 * 44100;
			/* e.g. fun=0x001(1)   - 2.8224M bck,  5.6448M mclk
			 *      fun=0x011(17)  - 5.6448M bck,  5.6448M mclk
			 *      fun=0x111(273) - 5.6448M bck, 11.2896M mclk
			 *      fun=0x101(257) - 2.8224M bck, 11.2896M mclk
			 *      fun=0x201(513) - 2.8224M bck, 16.9344M mclk
			 *      ...
			 */
			/* note: I2S_OUT_DSD_USE_SONY_IP has no mclk output */
			afe_i2s_out_master_dsd_configurate(AFE_I2S_OUT_1, fs, mclk,
							   I2S_OUT_DSD_USE_SONY_IP);
			afe_i2s_out_master_dsd_configurate(AFE_I2S_OUT_2, fs, mclk,
							   I2S_OUT_DSD_USE_SONY_IP);
			afe_i2s_out_master_dsd_enable(AFE_I2S_OUT_1, 1);
			afe_i2s_out_master_dsd_enable(AFE_I2S_OUT_2, 1);
		}

		#ifdef AUD_PINCTRL_SUPPORTING
		/* use I2S0_LRCK as GPIO to reset PCM1795 */
		mt2701_GPIO_I2S0_LRCK_Select(I2S0_MODE0_GPIO);
		if (gpio_is_valid(aud_i2s0_lrck_gpio)) {
			gpio_direction_output(aud_i2s0_lrck_gpio, 0);
			usleep_range(1000, 1100);
			gpio_set_value(aud_i2s0_lrck_gpio, 1);
			usleep_range(1000, 1100);
		} else
			pr_warn("%s() aud_i2s0_lrck_gpio invalid\n", __func__);
		#else
		/* use I2S0_LRCK as GPIO to reset PCM1795 */
		mt_set_gpio_mode(GPIO73, GPIO_MODE_GPIO);
		mt_set_gpio_dir(GPIO73, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO73, GPIO_OUT_ZERO);
		usleep_range(1000, 1100);
		mt_set_gpio_out(GPIO73, GPIO_OUT_ONE);
		usleep_range(1000, 1100);
		#endif
		/* add i2c configuration of PCM1795 here (todo) */
		break;
	}
	case 2:
		/* fun=0x2 */
		afe_i2s_out_master_dsd_enable(AFE_I2S_OUT_1, 0);
		afe_i2s_out_master_dsd_enable(AFE_I2S_OUT_2, 0);
		afe_dsdenc_enable(0);
		afe_memif_enable(AFE_MEM_DSDW, 0);
		afe_memif_enable(AFE_MEM_DSDR, 0);
		afe_power_on_dsdenc(0);
		/* restore I2S0_LRCK function */
		#ifdef AUD_PINCTRL_SUPPORTING
		mt2701_GPIO_I2S0_LRCK_Select(I2S0_MODE1_I2S0);
		#else
		mt_set_gpio_mode(GPIO73, GPIO_MODE_01);
		#endif

		break;
	default:
		pr_warn("%s() error: invalid DSD-ENC test function id 0x%08x\n", __func__, fun);
		break;
	}
	return 0;
}

enum afe_dsdenc_mode dsdenc_mode = PCM8_TO_DSD64;

static int dsdenc_change_mode_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 5;
	return 0;
}

static int dsdenc_change_mode_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = dsdenc_mode;
	return 0;
}

static int dsdenc_change_mode_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int mode = ucontrol->value.integer.value[0];

	pr_debug("%s() mode: %d\n", __func__, mode);
	switch (mode) {
	case 0:
		dsdenc_mode = DSD128_TO_DSD128;
		break;
	case 1:
		dsdenc_mode = DSD128_TO_DSD128_LONGEST;
		break;
	case 2:
		dsdenc_mode = DSD64_TO_DSD128;
		break;
	case 3:
		dsdenc_mode = DSD64_TO_DSD64;
		break;
	case 4:
		dsdenc_mode = PCM8_TO_DSD128;
		break;
	case 5:
		dsdenc_mode = PCM8_TO_DSD64;
		break;
	default:
		pr_warn("%s() error: invalid DSD-ENC mode: %d\n", __func__, mode);
		break;
	}
	return 0;
}

static const struct snd_kcontrol_new demo_controls[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "I2S In/Out Pass-through",
		.info = i2s_passthrough_info,
		.get = i2s_passthrough_get,
		.put = i2s_passthrough_put
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "SPDIF In/Out Pass-through",
		.info = spdif_passthrough_info,
		.get = spdif_passthrough_get,
		.put = spdif_passthrough_put
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "DMIC/I2S-Out Pass-through",
		.info = dmic_passthrough_info,
		.get = dmic_passthrough_get,
		.put = dmic_passthrough_put
	},
	{
		/* test only */
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "DSD-ENC Test",
		.info = dsdenc_test_info,
		.get = dsdenc_test_get,
		.put = dsdenc_test_put
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "DSD-ENC-CHANGE-MODE",
		.info = dsdenc_change_mode_info,
		.get = dsdenc_change_mode_get,
		.put = dsdenc_change_mode_put
	},
};
