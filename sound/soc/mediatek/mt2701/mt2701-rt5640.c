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

#include <linux/module.h>
#include <sound/soc.h>
#include "mt2701-aud-gpio.h"
#include "mt2701-afe-debug.h"
#include "mt2701-afe.h"
#include "mt2701-dai.h"

struct demo_factory_mode {
	int i2s_passthrough;
	int spdif_passthrough;
	int dmic_passthrough;
};

struct demo_private {
	struct demo_factory_mode factory_mode;
};

static const struct snd_soc_dapm_widget mt2701_rt5640_widgets[] = {
	SND_SOC_DAPM_HP("Headphone", NULL),
	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_MIC("Internal Mic", NULL),
	SND_SOC_DAPM_SPK("Speaker", NULL),
};

static const struct snd_soc_dapm_route mt2701_rt5640_audio_map[] = {
	{"Headset Mic", NULL, "MICBIAS1"},
	{"IN2P", NULL, "Headset Mic"},
	{"Headphone", NULL, "HPOL"},
	{"Headphone", NULL, "HPOR"},
	{"Speaker", NULL, "SPOLP"},
	{"Speaker", NULL, "SPOLN"},
	{"Speaker", NULL, "SPORP"},
	{"Speaker", NULL, "SPORN"},
};

static const struct snd_kcontrol_new mt2701_rt5640_controls[] = {
	SOC_DAPM_PIN_SWITCH("Headphone"),
	SOC_DAPM_PIN_SWITCH("Headset Mic"),
	SOC_DAPM_PIN_SWITCH("Internal Mic"),
	SOC_DAPM_PIN_SWITCH("Speaker"),
};


static int pcm_master_data_rate_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	/* codec slave, mt2701 master */
	unsigned int fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_CONT;
	unsigned int mclk_rate;
	unsigned int rate = params_rate(params);	/* data rate */
	unsigned int div_mclk_to_bck = rate > 192000 ? 2 : 4;
	unsigned int div_bck_to_lrck = 64;

	pr_debug("%s() rate = %d\n", __func__, rate);
	mclk_rate = rate * div_bck_to_lrck * div_mclk_to_bck;
	/* codec mclk */
	snd_soc_dai_set_sysclk(codec_dai, 0, mclk_rate, SND_SOC_CLOCK_IN);
	/* codec slave */
	snd_soc_dai_set_fmt(codec_dai, fmt);
	/* mt2701 mclk */
	snd_soc_dai_set_sysclk(cpu_dai, 0, mclk_rate, SND_SOC_CLOCK_OUT);
	/* mt2701 bck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_MCLK_TO_BCK, div_mclk_to_bck);
	/* mt2701 lrck */
	snd_soc_dai_set_clkdiv(cpu_dai, DIV_ID_BCK_TO_LRCK, div_bck_to_lrck);
	/* mt2701 master */
	snd_soc_dai_set_fmt(cpu_dai, fmt);
	return 0;
}

static struct snd_soc_ops stream_pcm_master_data_rate_ops = {
	.hw_params = pcm_master_data_rate_hw_params
};

/*#include "mt2701-machine-links.c" */
#include "mt2701-machine-controls.c"
static struct snd_soc_dai_link mt2701_rt5640_dai_links[] = {
	{
		.name = "demo-pcm-out0",
		.stream_name = "pcm-out0",
		.platform_name = "mt2701-audio",
		.cpu_dai_name = "mt2701-i2s1",
		/*.codec_dai_name = "pcm5102a-i2s",
		.codec_name = "pcm5102a",*/
		.codec_dai_name = "rt5640-aif1",
		.codec_name = "rt5640.0-001c",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	},
	{
		.name = "demo-pcm-in1",
		.stream_name = "pcm-in1",
		.platform_name = "mt2701-audio",
		.cpu_dai_name = "mt2701-i2s1",
		.codec_dai_name = "pcm1861-i2s",
		.codec_name = "pcm1861",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_GATED,
		.ops = &stream_pcm_master_data_rate_ops
	},

	{
		.name = "demo-hdmi-pcm-out",
		.stream_name = "hdmi-pcm-out",
		.platform_name = "mt2701-audio",
		.cpu_dai_name = "mt2701-hdmi-pcm-out",
		/* dummy codec is temporary, please change to real codec */
		.codec_dai_name = "dummy-codec-i2s",
		.codec_name = "dummy-codec",
	},
};

static struct snd_soc_card mt2701_rt5640_soc_card = {
	.name = "mt2701-rt5640-card",
	.dai_link = mt2701_rt5640_dai_links,
	.num_links = ARRAY_SIZE(mt2701_rt5640_dai_links),
	.controls = mt2701_rt5640_controls,
	.num_controls = ARRAY_SIZE(mt2701_rt5640_controls),
	.dapm_widgets = mt2701_rt5640_widgets,
	.num_dapm_widgets = ARRAY_SIZE(mt2701_rt5640_widgets),
	.dapm_routes = mt2701_rt5640_audio_map,
	.num_dapm_routes = ARRAY_SIZE(mt2701_rt5640_audio_map),
};

static int demo_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &mt2701_rt5640_soc_card;
	struct device *dev = &pdev->dev;
	int ret;
	struct demo_private *priv =
		devm_kzalloc(&pdev->dev, sizeof(struct demo_private), GFP_KERNEL);

	pr_debug("%s()\n", __func__);
	#ifdef CONFIG_OF
	if (dev->of_node) {
		dev_set_name(dev, "%s", "mt2701-soc-machine");
		pr_notice("%s set dev name %s\n", __func__, dev_name(dev));
	}
	#endif

	if (priv == NULL)
		return -ENOMEM;
	#if 0
	ret = snd_soc_add_card_controls(card, mt2701_rt5640_controls,
					ARRAY_SIZE(mt2701_rt5640_controls));
	if (ret) {
		dev_err(card->dev, "unable to add card controls\n");
		return ret;
	}
	#endif

	ret = mt_afe_platform_init(dev);
	if (ret) {
		pr_err("%s mt_afe_platform_init fail %d\n", __func__, ret);
		return ret;
	}
	mt_afe_debug_init();

	#ifdef AUD_PINCTRL_SUPPORTING
	mt2701_GPIO_probe(dev);
	#endif

	card->dev = &pdev->dev;
	snd_soc_card_set_drvdata(card, priv);
	return snd_soc_register_card(card);
}

static int demo_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	mt_afe_platform_deinit(&pdev->dev);
	devm_kfree(&pdev->dev, snd_soc_card_get_drvdata(card));
	return snd_soc_unregister_card(card);
}

#ifdef CONFIG_OF
static const struct of_device_id demo_machine_dt_match[] = {
	{.compatible = "mediatek,mt2701-soc-machine",},
	{}
};
#endif

static struct platform_driver mt2701_rt5640_machine = {
	.driver = {
		.name = "mt2701-rt5640",
		.owner = THIS_MODULE,
		   #ifdef CONFIG_OF
		   .of_match_table = demo_machine_dt_match,
		   #endif
	},
	.probe = demo_machine_probe,
	.remove = demo_machine_remove
};

module_platform_driver(mt2701_rt5640_machine);

/* Module information */
MODULE_DESCRIPTION("mt2701 rt5640 machine driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mt2701 5640 soc card");
