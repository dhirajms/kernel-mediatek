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
/*#include <mach/mt_gpio.h>*/
#include "mt2701-aud-global.h"
#ifdef CONFIG_MTK_LEGACY_CLOCK
#include <mach/mt_clkmgr.h>
#endif
#include "mt2701-dai.h"
#include "mt2701-afe.h"
#include "mt2701-dai-private.h"
#include "mt2701-aud-gpio.h"

static struct audio_dmic *get_dmic_in(struct mt_dmic_all *dmic_all, int dai_id)
{
	int i;

	for (i = 0; i < AFE_DMIC_NUM; ++i) {
		if (dmic_all->dmic_in[i].dai_id == dai_id)
			return &dmic_all->dmic_in[i];
	}
	return NULL;
}

static inline enum afe_dmic_id get_dmic_id(int dai_id)
{
	switch (dai_id) {
	case MT2701_DAI_DMIC1_ID:
			return AFE_DMIC_1;
	case MT2701_DAI_DMIC2_ID:
		return AFE_DMIC_2;
	default:
		return AFE_DMIC_NUM;
	}
}

static int dmic_configurate(const struct audio_dmic *in)
{
	enum afe_sampling_rate stream_fs;

	if (!in)
		return -EINVAL;
	stream_fs = in->stream_fs;
	/* configure dmic */
	{
		struct afe_dmic_config dmic_config = {
			.one_wire_mode = 1,
			.iir_on = 0,
			.iir_mode = 0,
			.voice_mode = stream_fs
		};
		enum afe_dmic_id dmic_id;

		dmic_id = get_dmic_id(in->dai_id);
		afe_dmic_configurate(dmic_id, &dmic_config);
		afe_dmic_enable(dmic_id, 1);
	}
	return 0;
}

static int mt2701_dmic_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	int ret;
	struct mt_dai_private *priv;
	struct mt_dmic_all *dmic_all;
	enum afe_sampling_rate stream_fs;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	dmic_all = &priv->dmic_all;
	stream_fs = fs_enum(params_rate(params));
	/* UNNECESSARY_ELSE: else is not generally useful after a break or return
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return -EINVAL;
	else {
		struct audio_dmic *in;

		in = get_dmic_in(dmic_all, dai->id);
		if (!in)
			return -EINVAL;
		in->stream_fs = stream_fs;
		ret = dmic_configurate(in);
		if (ret < 0)
			return ret;
	}
	*/
	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK) {
		struct audio_dmic *in;

		in = get_dmic_in(dmic_all, dai->id);
		if (!in)
			return -EINVAL;
		in->stream_fs = stream_fs;
		ret = dmic_configurate(in);
		if (ret < 0)
			return ret;
	}
	return -EINVAL;
}
#ifdef CONFIG_MTK_LEGACY_CLOCK
static const enum cg_clk_id dmic_clks[AFE_DMIC_NUM] = {
	MT_CG_AUDIO_DMIC1, /* PWR2_TOP_CON[0] */
	MT_CG_AUDIO_DMIC2, /* PWR2_TOP_CON[1] */
};
#endif
static int mt2701_dmic_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;
	struct mt_dmic_all *dmic_all;
	struct audio_dmic *in;
	enum afe_dmic_id id;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	dmic_all = &priv->dmic_all;
	in = get_dmic_in(dmic_all, dai->id);
	if (!in)
		return -EINVAL;
	if (in->occupied)
		return -EINVAL;
	id = get_dmic_id(in->dai_id);
	dev_dbg(dai->dev, "%s() set gpio for dmic%d\n", __func__, id + 1);
	#ifdef AUD_PINCTRL_SUPPORTING
	mt2701_GPIO_I2S2_Select(I2S2_MODE4_DMIC);
	#else
	if (id == AFE_DMIC_1) {
		mt_set_gpio_mode(GPIO38, GPIO_MODE_04);
		mt_set_gpio_mode(GPIO51, GPIO_MODE_04);
	} else if (id == AFE_DMIC_2) {
		mt_set_gpio_mode(GPIO50, GPIO_MODE_04);
		mt_set_gpio_mode(GPIO52, GPIO_MODE_04);
	} else {
		dev_err(dai->dev, "%s() invalid dmic id %u\n", __func__, id);
		return -EINVAL;
	}
	#endif
	in->occupied = 1;
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	enable_clock(dmic_clks[id], "AUDIO");
	#endif
	afe_power_on_dmic(id, 1);
	return 0;
}

static void mt2701_dmic_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;
	struct mt_dmic_all *dmic_all;
	struct audio_dmic *in;
	enum afe_dmic_id id;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	dmic_all = &priv->dmic_all;
	in = get_dmic_in(dmic_all, dai->id);
	if (!in)
		return;
	in->occupied = 0;
	id = get_dmic_id(in->dai_id);
	afe_power_on_dmic(id, 0);
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	disable_clock(dmic_clks[id], "AUDIO");
	#endif
}

static void init_mt_dmic_all(struct mt_dmic_all *dmic_all)
{
	if (!dmic_all->inited) {
		int dai_id;

		for (dai_id = MT2701_DAI_DMIC1_ID; dai_id <= MT2701_DAI_DMIC2_ID; ++dai_id)
			dmic_all->dmic_in[get_dmic_id(dai_id)].dai_id = dai_id;
		dmic_all->inited = 1;
	}
}

static int mt2701_dmic_probe(struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	init_mt_dmic_all(&priv->dmic_all);
	return 0;
}

static struct snd_soc_dai_ops mt2701_dmic_ops = {
	.startup = mt2701_dmic_startup,
	.shutdown = mt2701_dmic_shutdown,
	.hw_params = mt2701_dmic_hw_params,
};

