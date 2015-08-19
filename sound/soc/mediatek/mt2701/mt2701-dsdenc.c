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

#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt2701-aud-global.h"
#ifdef CONFIG_MTK_LEGACY_CLOCK
#include <mach/mt_clkmgr.h>
#endif
#include "mt2701-dai.h"
#include "mt2701-afe.h"
#include "mt2701-dai-private.h"

static int mt2701_dsdenc_hw_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	/* snd_pcm_format_t stream_fmt = params_format(params); */
	dev_dbg(dai->dev, "%s() cpu_dai id %d, dsdenc_mode %d\n", __func__, dai->id, dsdenc_mode);
	{
		struct afe_i2s_out_config i2s_config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = 0,
			.dsd_mode = 1,
			.dsd_use = I2S_OUT_DSD_USE_SONY_IP,
			.couple_mode = 1,
			.one_heart_mode = 0,
			.slave = 0,
			.mode = FS_88200HZ
		};
		afe_i2s_out_configurate(AFE_I2S_OUT_1, &i2s_config);
	}
	{
		afe_dsdenc_configurate(dsdenc_mode);
		afe_dsdenc_enable(1);
	}
	return 0;
}

static int mt2701_dsdenc_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;

	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;
	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	if (priv->dsdenc.occupied) {
		dev_warn(dai->dev,
			 "%s() warning: can't open MT2701_DAI_DSDENC_ID because it has been occupied\n",
			 __func__);
		return -EINVAL;
	}
	if (i2s_all->i2s_out[AFE_I2S_OUT_1].occupied
	    || i2s_all->i2s_out_mch.occupied || i2s_all->i2s_in[AFE_I2S_IN_1].occupied) {
		dev_warn(dai->dev,
			 "%s() warning: can't open MT2701_DAI_DSDENC_ID because i2s1 has been occupied\n",
			 __func__);
		return -EINVAL;
	}
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	enable_clock(MT_CG_AUDIO_DSD_ENC, "AUDIO"); /*PWR2_TOP_CON[15]*/
	#endif
	afe_power_on_dsdenc(1);
	priv->dsdenc.occupied = 1;
	return 0;
}

static void mt2701_dsdenc_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;

	priv = dev_get_drvdata(dai->dev);
	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv->dsdenc.occupied = 0;
	afe_power_on_dsdenc(0);
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	disable_clock(MT_CG_AUDIO_DSD_ENC, "AUDIO");
	#endif
}

static struct snd_soc_dai_ops mt2701_dsdenc_ops = {
	.startup = mt2701_dsdenc_startup,
	.shutdown = mt2701_dsdenc_shutdown,
	.hw_params = mt2701_dsdenc_hw_params,
};
