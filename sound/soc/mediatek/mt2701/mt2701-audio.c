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

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt2701-dai.h"
#include "mt2701-dai-private.h"
#include "mt2701-afe.h"
#include "mt2701-private.h"
#include "mt2701-lp-audio.h"
#include "mt2701-aud-global.h"

struct snd_card *snd_card_test;

#ifndef AUD_K318_MIGRATION
#include "mt2701-audio-controls.c"
#endif
#include "mt2701-memif.c"
#include "mt2701-spdifout.c"
#include "mt2701-hdmi-pcm.c"
#include "mt2701-hdmi-raw.c"

static void link_stream_and_irq(struct mt_private *priv,
				enum mt_stream_id stream_id,
				enum audio_irq_id irq_id, void (*isr)(struct mt_stream *));

static int mt2701_pcm_open(struct snd_pcm_substream *substream)
{
	struct mt_stream *s;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt_private *priv = snd_soc_platform_get_drvdata(rtd->platform);

	pr_debug("%s() cpu_dai id %d, stream direction %d\n",
		 __func__, rtd->cpu_dai->id, substream->stream);
	s = priv->dais[rtd->cpu_dai->id][substream->stream].s;
	substream->runtime->private_data = s;
	if (!s) {
		pr_err("%s() error: no mt stream for this dai\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s() %s\n", __func__, s->name);
	if (s->occupied) {
		pr_warn("%s() warning: can't open %s because it has been occupied\n", __func__, s->name);
		return -EINVAL;
	}
	if (s->id >= MT_STREAM_DL1 && s->id <= MT_STREAM_DL5) {
		if (priv->streams[MT_STREAM_DLM].occupied) {
			pr_warn("%s() warning: can't open %s because MT_STREAM_DLM has been occupied\n",
				   __func__, s->name);
			return -EINVAL;
		}
	}
	if (s->id == MT_STREAM_DLM) {
		enum mt_stream_id i;

		for (i = MT_STREAM_DL1; i <= MT_STREAM_DL5; ++i) {
			if (priv->streams[i].occupied) {
				pr_warn("%s() warning: can't open MT_STREAM_DLM because %s has been occupied\n",
					   __func__, priv->streams[i].name);
				return -EINVAL;
			}
		}
	}
	s->substream = substream;
	s->occupied = 1;
	if (s->irq == NULL) {
		enum audio_irq_id irq_id = asys_irq_acquire();

		if (irq_id != IRQ_NUM) {
			/* link */
			link_stream_and_irq(priv, s->id, irq_id, memif_isr);
		} else
			pr_err("%s() error: no more asys irq\n", __func__);
	}
	if (s->ops && s->ops->open)
		return s->ops->open(substream);
	return 0;
}

static int mt2701_pcm_close(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt_private *priv = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt_stream *s = substream->runtime->private_data;

	pr_debug("%s() cpu_dai id %d, stream direction %d\n",
		 __func__, rtd->cpu_dai->id, substream->stream);
	if (!s) {
		pr_err("%s() error: no mt stream for this dai\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s() %s\n", __func__, s->name);
	if (s->ops && s->ops->close)
		ret = s->ops->close(substream);
	if (s->irq) {
		enum audio_irq_id irq_id = s->irq->id;

		if (irq_id >= IRQ_ASYS_IRQ1 && irq_id <= IRQ_ASYS_IRQ16) {
			/* delink */
			link_stream_and_irq(priv, s->id, IRQ_NUM, NULL);
			asys_irq_release(irq_id);
		}
	}
	s->occupied = 0;
	s->substream = NULL;
	substream->runtime->private_data = NULL;
	return ret;
}

static int mt2701_pcm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct mt_stream *s = substream->runtime->private_data;

	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->hw_params)
		return s->ops->hw_params(substream, params);
	return 0;
}

static int mt2701_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;

	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->hw_free)
		return s->ops->hw_free(substream);
	return 0;
}

static int mt2701_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;

	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->prepare)
		return s->ops->prepare(substream);
	return 0;
}

static int mt2701_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct mt_stream *s = substream->runtime->private_data;

	pr_debug("%s()\n", __func__);
	if (s && s->ops && s->ops->trigger)
		return s->ops->trigger(substream, cmd);
	return 0;
}

static snd_pcm_uframes_t mt2701_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct mt_stream *s = substream->runtime->private_data;

	if (s && s->ops && s->ops->pointer)
		return s->ops->pointer(substream);
	return 0;
}

static struct snd_pcm_ops mt2701_pcm_ops = {
	.open = mt2701_pcm_open,
	.close = mt2701_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = mt2701_pcm_hw_params,
	.hw_free = mt2701_pcm_hw_free,
	.prepare = mt2701_pcm_prepare,
	.trigger = mt2701_pcm_trigger,
	.pointer = mt2701_pcm_pointer,
};

static int mt2701_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	snd_pcm_lib_preallocate_pages_for_all(
		rtd->pcm, SNDRV_DMA_TYPE_DEV,
		NULL, 1024 * 1024 * 16, 1024 * 1024 * 16
	);
	return 0;
}

static void mt2701_pcm_free(struct snd_pcm *pcm)
{
	snd_pcm_lib_preallocate_free_for_all(pcm);
}

static inline void call_isr(struct mt_irq *irq)
{
	if (irq->isr)
		irq->isr(irq->s);
}

static irqreturn_t afe_isr(int irq, void *dev)
{
	enum audio_irq_id id;
	struct mt_private *priv = (struct mt_private *)dev;
	u32 status = afe_irq_status();

	afe_irq_clear(status);
	if (lp_substream) {
		/* lp-audio high frequency timer */
		if (status & (0x1 << (IRQ_AFE_IRQ1 - IRQ_AFE_IRQ1)))
			lp_audio_isr();
	} else {
		for (id = IRQ_AFE_IRQ1; id <= IRQ_AFE_DMA; ++id) {
			/* AFE_IRQ1_IRQ to AFE_DMA_IRQ */
			if (status & (0x1 << (id - IRQ_AFE_IRQ1)))
				call_isr(&(priv->irqs[id]));
		}
	}
	return IRQ_HANDLED;
}

static irqreturn_t asys_isr(int irq, void *dev)
{
	enum audio_irq_id id;
	struct mt_private *priv;
	u32 status;

	status = asys_irq_status();
	asys_irq_clear(status);
	priv = dev;
	for (id = IRQ_ASYS_IRQ1; id <= IRQ_ASYS_IRQ16; ++id) {
		/* ASYS_IRQ1_IRQ to ASYS_IRQ16_IRQ */
		if (status & (0x1 << (id - IRQ_ASYS_IRQ1)))
			call_isr(&(priv->irqs[id]));
	}
	return IRQ_HANDLED;
}

static void link_dai_and_stream(struct mt_private *priv,
				int dai_id, int dir, enum mt_stream_id stream_id)
{
	priv->dais[dai_id][dir].s = &(priv->streams[stream_id]);
}

static void link_stream_and_irq(struct mt_private *priv,
				enum mt_stream_id stream_id,
				enum audio_irq_id irq_id, void (*isr)(struct mt_stream *))
{
	if (stream_id < MT_STREAM_NUM) {
		if (irq_id < IRQ_NUM) {
			priv->streams[stream_id].irq = &(priv->irqs[irq_id]);
			priv->irqs[irq_id].s = &(priv->streams[stream_id]);
		} else
			priv->streams[stream_id].irq = NULL;
	} else {
		if (irq_id < IRQ_NUM)
			priv->irqs[irq_id].s = NULL;
	}
	if (irq_id < IRQ_NUM)
		priv->irqs[irq_id].isr = isr;
}

static void link_stream_and_ops(struct mt_private *priv,
				enum mt_stream_id stream_id, struct snd_pcm_ops *ops)
{
	priv->streams[stream_id].ops = ops;
}

static void spdifrx_isr(struct mt_stream *s)
{
	afe_spdifrx_isr();
}

static void init_mt_private(struct mt_private *priv)
{
	enum mt_stream_id stream_id;
	enum audio_irq_id irq_id;
	static const char *names[MT_STREAM_NUM] = {
		/* playback streams */
		"MT_STREAM_DL1",
		"MT_STREAM_DL2",
		"MT_STREAM_DL3",
		"MT_STREAM_DL4",
		"MT_STREAM_DL5",
		"MT_STREAM_DL6",
		"MT_STREAM_DLM",
		"MT_STREAM_ARB1",
		"MT_STREAM_DSDR",
		"MT_STREAM_8CH_I2S_OUT",
		"MT_STREAM_IEC1",
		"MT_STREAM_IEC2",
		/* capture streams */
		"MT_STREAM_UL1",
		"MT_STREAM_UL2",
		"MT_STREAM_UL3",
		"MT_STREAM_UL4",
		"MT_STREAM_UL5",
		"MT_STREAM_UL6",
		"MT_STREAM_DAI",
		"MT_STREAM_MOD_PCM",
		"MT_STREAM_AWB",
		"MT_STREAM_AWB2",
		"MT_STREAM_DSDW",
		"MT_STREAM_MULTIIN",
	};
	for (stream_id = MT_STREAM_DL1; stream_id < MT_STREAM_NUM; ++stream_id) {
		priv->streams[stream_id].id = stream_id;
		priv->streams[stream_id].name = names[stream_id];
	}
	for (irq_id = IRQ_AFE_IRQ1; irq_id < IRQ_NUM; ++irq_id)
		priv->irqs[irq_id].id = irq_id;
	/* 1. stream <-> ops */
	link_stream_and_ops(priv, MT_STREAM_DL1,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL2,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL3,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL4,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL5,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DL6,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DLM,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_ARB1,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DSDR,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_8CH_I2S_OUT, &hdmi_pcm_ops);
	link_stream_and_ops(priv, MT_STREAM_IEC1,    &hdmi_raw_ops);
	link_stream_and_ops(priv, MT_STREAM_IEC2,    &spdif_iec2_ops);
	link_stream_and_ops(priv, MT_STREAM_UL1,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL2,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL3,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL4,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL5,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_UL6,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DAI,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_MOD_PCM, &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_AWB,     &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_AWB2,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_DSDW,    &memif_ops);
	link_stream_and_ops(priv, MT_STREAM_MULTIIN, &memif_ops);
	/* 2. dai <-> stream */
	itrcon_connect(I12, O15, 1);
	itrcon_connect(I13, O16, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S1_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL1);
	itrcon_connect(I14, O17, 1);
	itrcon_connect(I15, O18, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S2_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL2);
	itrcon_connect(I16, O19, 1);
	itrcon_connect(I17, O20, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S3_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL3);
	itrcon_connect(I18, O27, 1);
	itrcon_connect(I19, O28, 1);
	itrcon_connect(I27, O21, 1);
	itrcon_connect(I28, O22, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S4_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL4);
	itrcon_connect(I20, O23, 1);
	itrcon_connect(I21, O24, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S5_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL5);
	itrcon_connect(I22, O25, 1);
	itrcon_connect(I23, O26, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S6_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL6);
	link_dai_and_stream(priv, MT2701_DAI_I2SM_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DLM);
	itrcon_connect(I00, O00, 1);
	itrcon_connect(I01, O01, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S1_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL1);
	itrcon_connect(I02, O02, 1);
	itrcon_connect(I03, O03, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S2_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL2);
	itrcon_connect(I04, O04, 1);
	itrcon_connect(I05, O05, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S3_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL3);
	itrcon_connect(I06, O06, 1);
	itrcon_connect(I07, O07, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S4_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL4);
	itrcon_connect(I08, O08, 1);
	itrcon_connect(I09, O09, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S5_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL5);
	itrcon_connect(I10, O10, 1);
	itrcon_connect(I11, O11, 1);
	link_dai_and_stream(priv, MT2701_DAI_I2S6_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL6);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_SPDIF_OUT_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_IEC2);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_MULTI_IN_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_MULTIIN);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_HDMI_OUT_I2S_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_8CH_I2S_OUT);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_HDMI_OUT_IEC_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_IEC1);
	/* no itrcon */
	/*link_dai_and_stream(priv, MT2701_DAI_HDMI_IN_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_MULTIIN);*/
	itrcon_connect(I35, O31, 1);
	link_dai_and_stream(priv, MT2701_DAI_BTPCM_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_ARB1);
	link_dai_and_stream(priv, MT2701_DAI_MRGIF_BT_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_ARB1);
	itrcon_connect(I22, O25, 1);
	itrcon_connect(I23, O26, 1);
	link_dai_and_stream(priv, MT2701_DAI_MRGIF_I2S_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DL6);
	itrcon_connect(I26, O14, 1);
	link_dai_and_stream(priv, MT2701_DAI_BTPCM_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DAI);
	link_dai_and_stream(priv, MT2701_DAI_MRGIF_BT_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DAI);
	itrcon_connect(I24, O00, 1);
	itrcon_connect(I25, O01, 1);
	link_dai_and_stream(priv, MT2701_DAI_MRGIF_I2S_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_UL1);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_DSDENC_ID, SNDRV_PCM_STREAM_PLAYBACK, MT_STREAM_DSDR);
	/* no itrcon */
	link_dai_and_stream(priv, MT2701_DAI_DSDENC_RECORD_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_DSDW);
	itrcon_connect(I31, O12, 1);
	itrcon_connect(I32, O13, 1);
	link_dai_and_stream(priv, MT2701_DAI_DMIC1_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_AWB);
	itrcon_connect(I33, O32, 1);
	itrcon_connect(I34, O33, 1);
	link_dai_and_stream(priv, MT2701_DAI_DMIC2_ID, SNDRV_PCM_STREAM_CAPTURE, MT_STREAM_AWB2);
	/* 3. stream <-> irq */
	/*
	 * The streams below statically link to the specified irq.
	 */
	link_stream_and_irq(priv, MT_STREAM_AWB, IRQ_AFE_IRQ1, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_AWB2, IRQ_AFE_IRQ2, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_8CH_I2S_OUT, IRQ_AFE_HDMI, hdmi_pcm_isr);
	link_stream_and_irq(priv, MT_STREAM_IEC1, IRQ_AFE_SPDIF, hdmi_raw_isr);
	link_stream_and_irq(priv, MT_STREAM_IEC2, IRQ_AFE_SPDIF2, spdifout_isr);
	link_stream_and_irq(priv, MT_STREAM_MULTIIN, IRQ_AFE_MULTIIN, memif_isr);
	link_stream_and_irq(priv, MT_STREAM_NUM, IRQ_AFE_SPDIFIN, spdifrx_isr);
	/*
	 * The streams that don't link to any irq here
	 * will link to the asys irq at OPEN stage,
	 * and delink at CLOSE stage.
	 * The dynamic link/delink can save the irq resource.
	 */
}

#define AFE_MCU_IRQ_ID  (104+32)		/* (136) */
#define ASYS_MCU_IRQ_ID (132+32)		/*(164)	*/

static unsigned int audio_afe_irq_id = AFE_MCU_IRQ_ID;
static unsigned int audio_asys_irq_id = ASYS_MCU_IRQ_ID;

static int mt2701_pcm_probe(struct snd_soc_platform *platform)
{
	struct mt_private *priv;
	int ret;

	dev_dbg(platform->dev, "%s()\n", __func__);
	priv = devm_kzalloc(platform->dev, sizeof(struct mt_private), GFP_KERNEL);
	if (!priv) {
		dev_err(platform->dev, "%s() can't allocate memory\n", __func__);
		return -ENOMEM;
	}
	afe_enable(1);
	init_mt_private(priv);
	#ifdef AUD_K318_MIGRATION
	snd_card_test =  platform->component.card->snd_card;
	#else
	snd_card_test =  platform->card->snd_card;
	#endif
	snd_soc_platform_set_drvdata(platform, priv);

	#ifdef CONFIG_OF
	audio_afe_irq_id = mt_afe_get_afe_irq_id();
	audio_asys_irq_id = mt_afe_get_asys_irq_id();
	#endif
	ret = request_irq(audio_afe_irq_id, afe_isr, IRQF_TRIGGER_LOW, "afe-isr", priv);
	if (ret) {
		dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
			__func__, audio_afe_irq_id, ret);
	}
	ret = request_irq(audio_asys_irq_id, asys_isr, IRQF_TRIGGER_LOW, "asys-isr", priv);
	if (ret) {
		dev_err(platform->dev, "%s() can't register ISR for IRQ %u (ret=%i)\n",
			__func__, audio_asys_irq_id, ret);
	}
	return 0;
}

static int mt2701_pcm_remove(struct snd_soc_platform *platform)
{
	struct mt_private *priv;

	priv = snd_soc_platform_get_drvdata(platform);
	free_irq(AFE_MCU_IRQ_ID, priv);
	free_irq(ASYS_MCU_IRQ_ID, priv);
	devm_kfree(platform->dev, priv);
	itrcon_disconnectall();
	afe_enable(0);
	return 0;
}

static struct snd_soc_platform_driver mt2701_soc_platform_driver = {
	.probe = mt2701_pcm_probe,
	.remove = mt2701_pcm_remove,
	.pcm_new = mt2701_pcm_new,
	.pcm_free = mt2701_pcm_free,
	.ops = &mt2701_pcm_ops,
	#ifndef AUD_K318_MIGRATION
	.controls = mt2701_soc_controls,
	.num_controls = ARRAY_SIZE(mt2701_soc_controls),
	#endif
};

static int mt2701_audio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	dev_dbg(&pdev->dev, "%s()\n", __func__);
	#ifdef CONFIG_OF
		if (dev->of_node) {
			dev_set_name(dev, "%s", "mt2701-audio");
			pr_notice("%s set dev name %s\n", __func__, dev_name(dev));
		}
	#endif
	return snd_soc_register_platform(&pdev->dev, &mt2701_soc_platform_driver);
}

static int mt2701_audio_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mt2701_audio_dt_match[] = {
	{.compatible = "mediatek,mt2701-audio",},
	{}
};
#endif

#ifdef CONFIG_PM

static int mt2701_audio_suspend(struct platform_device *pdev, pm_message_t state)
{
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	afe_enable(0);
	return 0;
}

static int mt2701_audio_resume(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	afe_enable(1);
	return 0;
}

#else

#define mt2701_audio_suspend (NULL)
#define mt2701_audio_resume (NULL)

#endif

static struct platform_driver mt2701_audio = {
	.driver = {
		.name = "mt2701-audio",
		.owner = THIS_MODULE,
		    #ifdef CONFIG_OF
		   .of_match_table = mt2701_audio_dt_match,
		   #endif
	},
	.probe = mt2701_audio_probe,
	.remove = mt2701_audio_remove,
	.suspend = mt2701_audio_suspend,
	.resume = mt2701_audio_resume,
};

module_platform_driver(mt2701_audio);

MODULE_DESCRIPTION("mt2701 audio driver");
MODULE_LICENSE("GPL");
