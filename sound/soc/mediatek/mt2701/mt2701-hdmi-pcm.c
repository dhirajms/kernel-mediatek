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
#include "mt2701-private.h"
#include "mt2701-afe-reg.h"
#include "mt2701-hdmi-control.h"

static void mtk2701_hdmi_set_interconnection(unsigned int connection_state, unsigned int channels);

static struct snd_pcm_hardware hdmi_pcm_hardware = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_RESUME |
	SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.rates = (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |
	SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
	SNDRV_PCM_RATE_192000),
	.rate_min = 32000,
	.rate_max = 192000,
	.channels_min = 2,
	.channels_max = 8,
	.buffer_bytes_max = (1024 * 1024 * 16),
	.period_bytes_max = (64 * 1024),
	.period_bytes_min = (2 * 1024),
	.periods_min = 2,
	.periods_max = 256,
	.fifo_size = 0,
};

static void hdmi_pcm_isr(struct mt_stream *s)
{
	unsigned int Afe_consumed_bytes = 0;
	unsigned int HW_memory_index = 0;
	unsigned int HW_Cur_ReadIdx = 0;

	HW_Cur_ReadIdx = afe_read(AFE_HDMI_OUT_CUR);
	if (HW_Cur_ReadIdx == 0) {
		pr_debug("%s() HW_Cur_ReadIdx = 0\n", __func__);
		HW_Cur_ReadIdx = s->substream->runtime->dma_addr;
	}
	HW_memory_index = (HW_Cur_ReadIdx - s->substream->runtime->dma_addr);
	/*
	pr_debug("%s() HW_Cur_ReadIdx = 0x%x HW_memory_index = 0x%x pucPhysBufAddr = 0x%x\n",
		 __func__, HW_Cur_ReadIdx, HW_memory_index, s->substream->runtime->dma_addr);
	*/
	/* get hw consume bytes */
	if (HW_memory_index > s->pointer)
		Afe_consumed_bytes = HW_memory_index - s->pointer;
	else {
		Afe_consumed_bytes =
			s->substream->runtime->dma_bytes + HW_memory_index - s->pointer;
	}
	if ((Afe_consumed_bytes & 0x1f) != 0)
		pr_err("%s() DMA address is not aligned 32 bytes\n", __func__);
	s->pointer += Afe_consumed_bytes;
	s->pointer %= s->substream->runtime->dma_bytes;
	snd_pcm_period_elapsed(s->substream);
}

static void mtk2701_hdmi_set_interconnection(HDMI_INTERCON_STATUS connection_state,
		unsigned int channels)
{
	/* O20~O27: L/R/LS/RS/C/LFE/CH7/CH8 */
	int Sdate = 0;

	pr_debug("%s() channels = %d\n", __func__, channels);
	switch (channels) {
	case 8:
		SetHdmiInterConnection(connection_state, HDMI_IN_I26, HDMI_OUT_O26);
		SetHdmiInterConnection(connection_state, HDMI_IN_I27, HDMI_OUT_O27);
		/* fallthrough */
	case 6:
		SetHdmiInterConnection(connection_state, HDMI_IN_I24, HDMI_OUT_O24);
		SetHdmiInterConnection(connection_state, HDMI_IN_I25, HDMI_OUT_O25);
		/* fallthrough */
	case 4:
		SetHdmiInterConnection(connection_state, HDMI_IN_I22, HDMI_OUT_O22);
		SetHdmiInterConnection(connection_state, HDMI_IN_I23, HDMI_OUT_O23);
		/* fallthrough */
	case 2:
		SetHdmiInterConnection(connection_state, HDMI_IN_I20, HDMI_OUT_O20);
		SetHdmiInterConnection(connection_state, HDMI_IN_I21, HDMI_OUT_O21);
		for (Sdate = 0; Sdate < 4; Sdate++)
			afe_msk_write(AUDIO_TOP_CON3, Sdate << 6, SPEAKER_OUT_HDMI_SEL_MASK);
		break;
	case 1:
		SetHdmiInterConnection(connection_state, HDMI_IN_I20, HDMI_OUT_O20);
		afe_msk_write(AUDIO_TOP_CON3, 0 << 6, SPEAKER_OUT_HDMI_SEL_MASK);
		break;
	default:
		pr_err("%s() unsupported channels %u\n", __func__, channels);
		break;
	}
}

static int mtk2701_hdmi_pcm_open(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;

	snd_soc_set_runtime_hwparams(substream, &hdmi_pcm_hardware);
	/* Ensure that buffer size is a multiple of period size */
	ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		pr_err("%s() snd_pcm_hw_constraint_integer fail %d\n", __func__, ret);
	pr_debug("%s() substream->pcm->device = %d\n", __func__, substream->pcm->device);
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	enable_clock(MT_CG_AUDIO_HDMI_CK, "AUDIO"); /*AUDIO_TOP_CON0[20]*/
	#else
	afe_msk_write(AUDIO_TOP_CON0, 0, PDN_HDMI_CK);
	#endif
	return ret;
}

static int mtk2701_hdmi_pcm_close(struct snd_pcm_substream *substream)
{
	#ifdef CONFIG_MTK_LEGACY_CLOCK
	disable_clock(MT_CG_AUDIO_HDMI_CK, "AUDIO");
	#else
	afe_msk_write(AUDIO_TOP_CON0, PDN_HDMI_CK, PDN_HDMI_CK);
	#endif
	return 0;
}

static int mtk2701_hdmi_pcm_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *hw_params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;

	dma_buf->dev.type = SNDRV_DMA_TYPE_DEV;
	dma_buf->dev.dev = substream->pcm->card->dev;
	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	if (ret < 0)
		pr_err("%s() snd_pcm_lib_malloc_pages fail %d\n", __func__, ret);
	pr_debug("%s() dma_bytes = %zu dma_area = %p dma_addr = 0x%llx\n", __func__,
		 runtime->dma_bytes, runtime->dma_area, (unsigned long long)runtime->dma_addr);
	pr_debug("%s() runtime_periods = %d period_size = %d buffer_size = %d\n", __func__,
		 runtime->periods, runtime->period_size, runtime->buffer_size);
	return ret;
}

static int mtk2701_hdmi_pcm_free(struct snd_pcm_substream *substream)
{
	pr_debug("%s()\n", __func__);
	return snd_pcm_lib_free_pages(substream);
}

static int mtk2701_hdmi_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	pr_debug("%s() rate = %u  channels = %u period_size = %lu,periods = %lu,buffer_size = %lu\n",
		__func__, runtime->rate, runtime->channels, runtime->period_size,
		runtime->periods, runtime->buffer_size);
	if (frames_to_bytes(runtime, runtime->buffer_size) % 16 != 0) {
		pr_err("%s()  buffer-size not multiple 16 bytes\n", __func__);
		return -1;
	}
	return 0;
}

static int mtk2701_hdmi_pcm_start(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	enum afe_sampling_rate sample_rate_idx = fs_enum(runtime->rate);

	pr_debug("%s() channel = %d,samplerate = 0x%x,bit = %d,runtime_format = %d\n",
		 __func__, runtime->channels, sample_rate_idx, runtime->sample_bits, runtime->format);
	pr_debug("%s() period_size = %lu,periods = %d,buffer_size = %d\n",
		 __func__, runtime->periods, runtime->buffer_size);
	pr_debug("%s() dma_bytes = %d,dma_addr = 0x%llx\n", __func__,
		 runtime->dma_bytes, (unsigned long long)runtime->dma_addr);
	vAudioClockSetting(sample_rate_idx, 128, APLL_HDMI_CK, PCM_OUTPUT_32BIT, 0, 0);
	init_hdmi_dma_buffer(MT_AFE_MEM_I2S, runtime, s);
	if (s->irq) {
		struct audio_irq_config config = {
			.mode = sample_rate_idx,
			.init_val = runtime->period_size
		};
		audio_irq_configurate(s->irq->id, &config);
	}
	audio_irq_enable(s->irq->id, 1);
	mtk2701_hdmi_set_interconnection(HDMI_Connection, runtime->channels);
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_LE:
		set_hdmi_out_control(runtime->channels, HDMI_OUT_BIT_WIDTH_16);
		set_hdmi_out_control_enable(true);
		set_hdmi_i2s();
		set_hdmi_i2s_enable(true);
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		set_hdmi_out_control(runtime->channels, HDMI_OUT_BIT_WIDTH_32);
		set_hdmi_out_control_enable(true);
		set_hdmi_i2s();
		set_hdmi_i2s_enable(true);
		break;
	case SNDRV_PCM_FORMAT_DSD_U8:
		set_hdmi_out_dsd_control(runtime->channels, HDMI_OUT_DSD_8BIT);
		set_hdmi_out_control_enable(true);
		set_hdmi_i2s_dsd();
		set_hdmi_i2s_enable(true);
		break;
	case SNDRV_PCM_FORMAT_DSD_U16_LE:
		set_hdmi_out_dsd_control(runtime->channels, HDMI_OUT_DSD_16BIT);
		set_hdmi_out_control_enable(true);
		set_hdmi_i2s_dsd();
		set_hdmi_i2s_enable(true);
		break;
	default:
		pr_err("%s() invaled format\n", __func__);
		break;
	}
	/* test loopback I2S5 */
	set_hdmi_i2s_to_I2S5();
	return 0;
}

static int mtk2701_hdmi_pcm_stop(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_stream *s = runtime->private_data;
	enum afe_sampling_rate sample_rate_idx = fs_enum(runtime->rate);

	pr_debug("%s()\n", __func__);
	mtk2701_hdmi_set_interconnection(HDMI_DisConnect, runtime->channels);
	set_hdmi_i2s_enable(false);
	set_hdmi_out_control_enable(false);
	audio_irq_enable(s->irq->id, 0);
	/* clean audio hardware buffer */
	reset_hdmi_dma_buffer(MT_AFE_MEM_I2S, runtime, s);
	vAudioClockSetting(sample_rate_idx, 128, APLL_HDMI_CK, PCM_OUTPUT_16BIT, 0, 1);
	return 0;
}

static int mtk2701_hdmi_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	pr_debug("%s() cmd = %d\n", __func__, cmd);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		return mtk2701_hdmi_pcm_start(substream);
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		return mtk2701_hdmi_pcm_stop(substream);
	default:
		pr_err("%s() command %d not handled\n", __func__, cmd);
		break;
	}
	return -EINVAL;
}

static snd_pcm_uframes_t mtk2701_hdmi_pcm_pointer(struct snd_pcm_substream *substream)
{
	int offset = 0;
	struct mt_stream *s = substream->runtime->private_data;

	offset = bytes_to_frames(substream->runtime, s->pointer);
	if (unlikely(offset >= substream->runtime->buffer_size))
		offset = 0;
	return offset;
}

static struct snd_pcm_ops hdmi_pcm_ops = {
	.open = mtk2701_hdmi_pcm_open,
	.close = mtk2701_hdmi_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = mtk2701_hdmi_pcm_params,
	.hw_free = mtk2701_hdmi_pcm_free,
	.prepare = mtk2701_hdmi_pcm_prepare,
	.trigger = mtk2701_hdmi_pcm_trigger,
	.pointer = mtk2701_hdmi_pcm_pointer,
};
