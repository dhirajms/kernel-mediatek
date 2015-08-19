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
#else
#include "mt2701-afe-clk.h"
#endif
#include "mt2701-dai.h"
#include "mt2701-afe.h"
#include "mt2701-dai-private.h"

static struct audio_i2s *get_i2s_out(struct mt_i2s_all *i2s_all, int dai_id)
{
	int i;

	for (i = 0; i < AFE_I2S_OUT_NUM; ++i) {
		if (i2s_all->i2s_out[i].dai_id == dai_id)
			return &i2s_all->i2s_out[i];

	}
	if (i2s_all->i2s_out_mch.dai_id == dai_id)
		return &i2s_all->i2s_out_mch;

	return NULL;
}

static struct audio_i2s *get_i2s_in(struct mt_i2s_all *i2s_all, int dai_id)
{
	int i;

	for (i = 0; i < AFE_I2S_IN_NUM; ++i) {
		if (i2s_all->i2s_in[i].dai_id == dai_id)
			return &i2s_all->i2s_in[i];

	}
	return NULL;
}

static inline enum afe_sample_asrc_tx_id get_sample_asrc_tx_id(int dai_id)
{
	switch (dai_id) {
	case MT2701_DAI_I2S1_ID:
		return SAMPLE_ASRC_O1;
	case MT2701_DAI_I2S2_ID:
		return SAMPLE_ASRC_O2;
	case MT2701_DAI_I2S3_ID:
		return SAMPLE_ASRC_O3;
	case MT2701_DAI_I2S4_ID:
		return SAMPLE_ASRC_O4;
	case MT2701_DAI_I2S5_ID:
		return SAMPLE_ASRC_O5;
	case MT2701_DAI_I2S6_ID:
		return SAMPLE_ASRC_O6;
	case MT2701_DAI_BTPCM_ID:
		return SAMPLE_ASRC_PCM_OUT;
	default:
		return SAMPLE_ASRC_OUT_NUM;
	}
}

static inline enum afe_sample_asrc_rx_id get_sample_asrc_rx_id(int dai_id)
{
	switch (dai_id) {
	case MT2701_DAI_I2S1_ID:
		return SAMPLE_ASRC_I1;
	case MT2701_DAI_I2S2_ID:
		return SAMPLE_ASRC_I2;
	case MT2701_DAI_I2S3_ID:
		return SAMPLE_ASRC_I3;
	case MT2701_DAI_I2S4_ID:
		return SAMPLE_ASRC_I4;
	case MT2701_DAI_I2S5_ID:
		return SAMPLE_ASRC_I5;
	case MT2701_DAI_I2S6_ID:
		return SAMPLE_ASRC_I6;
	case MT2701_DAI_BTPCM_ID:
		return SAMPLE_ASRC_PCM_IN;
	default:
		return SAMPLE_ASRC_IN_NUM;
	}
}

static inline enum afe_i2s_out_id get_i2s_out_id(int dai_id)
{
	switch (dai_id) {
	case MT2701_DAI_I2S1_ID:
		return AFE_I2S_OUT_1;
	case MT2701_DAI_I2S2_ID:
		return AFE_I2S_OUT_2;
	case MT2701_DAI_I2S3_ID:
		return AFE_I2S_OUT_3;
	case MT2701_DAI_I2S4_ID:
		return AFE_I2S_OUT_4;
	case MT2701_DAI_I2S5_ID:
		return AFE_I2S_OUT_5;
	case MT2701_DAI_I2S6_ID:
		return AFE_I2S_OUT_6;
	case MT2701_DAI_I2SM_ID:
		return AFE_I2S_OUT_1;
	default:
		return AFE_I2S_OUT_NUM;
	}
}

static inline enum afe_i2s_in_id get_i2s_in_id(int dai_id)
{
	switch (dai_id) {
	case MT2701_DAI_I2S1_ID:
		return AFE_I2S_IN_1;
	case MT2701_DAI_I2S2_ID:
		return AFE_I2S_IN_2;
	case MT2701_DAI_I2S3_ID:
		return AFE_I2S_IN_3;
	case MT2701_DAI_I2S4_ID:
		return AFE_I2S_IN_4;
	case MT2701_DAI_I2S5_ID:
		return AFE_I2S_IN_5;
	case MT2701_DAI_I2S6_ID:
		return AFE_I2S_IN_6;
	default:
		return AFE_I2S_IN_NUM;
	}
}

static enum afe_i2s_format i2s_format(const struct audio_i2s *i2s)
{
	/* data format */
	enum afe_i2s_format format = FMT_64CYCLE_32BIT_I2S;

	switch (i2s->format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_LEFT_J:
		if (i2s->div_bck_to_lrck == 32) {
			format = FMT_32CYCLE_16BIT_LJ;
		} else {
			format = FMT_64CYCLE_32BIT_LJ;
			/* format = FMT_64CYCLE_16BIT_LJ; */
		}
		break;
	case SND_SOC_DAIFMT_I2S:
		if (i2s->div_bck_to_lrck == 32) {
			format = FMT_32CYCLE_16BIT_I2S;
		} else {
			format = FMT_64CYCLE_32BIT_I2S;
			/* format = FMT_64CYCLE_16BIT_I2S; */
		}
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		if (i2s->div_bck_to_lrck == 32) {
			format = FMT_32CYCLE_16BIT_LJ;
		} else {
			/* format = FMT_64CYCLE_16BIT_RJ; */
			format = FMT_64CYCLE_24BIT_RJ;
			/* format = FMT_64CYCLE_32BIT_LJ; */
		}
		break;
	default:
		break;
	}
	return format;
}

static inline int is_slave(const struct audio_i2s *i2s)
{
	return (i2s->format & SND_SOC_DAIFMT_MASTER_MASK)
	    == SND_SOC_DAIFMT_CBM_CFM /*mt2701 slave */ ? 1 : 0;
}

static inline int is_slave_use_asrc(const struct audio_i2s *i2s)
{
	return (i2s->format & SLAVE_USE_ASRC_MASK)
	    == SLAVE_USE_ASRC_YES ? 1 : 0;
}

static inline int is_enable(const struct audio_i2s *i2s)
{
	return (i2s->format & SND_SOC_DAIFMT_CLOCK_MASK)
	    == SND_SOC_DAIFMT_CONT ? 1 : 0;
}

static inline int is_one_heart(const struct audio_i2s *i2s)
{
	return (i2s->dai_id == MT2701_DAI_I2SM_ID) ? 1 : 0;
}

static inline int is_dsd_mode(const struct audio_i2s *i2s)
{
	return (i2s->format & SND_SOC_DAIFMT_FORMAT_MASK)
	    == SND_SOC_DAIFMT_PDM ? 1 : 0;
}

static inline enum afe_sampling_rate i2s_sampling_rate(const struct audio_i2s *i2s)
{
	if (is_dsd_mode(i2s)) {
		if (i2s->div_mclk_to_bck == 0) {
			pr_warn("%s() warning: div_mclk_to_bck is 0 in dsd mode\n", __func__);
			return FS_88200HZ;
		}
		if (i2s->mclk_rate / i2s->div_mclk_to_bck == 128 * 44100)
			return FS_176400HZ;
		else
			return FS_88200HZ;

	} else {
		if (i2s->div_mclk_to_bck == 0 || i2s->div_bck_to_lrck == 0) {
			pr_warning
			    ("%s() warning: div_mclk_to_bck is %d, div_bck_to_lrck is %d in pcm mode\n",
			     __func__, i2s->div_mclk_to_bck, i2s->div_bck_to_lrck);
			return FS_48000HZ;
		}
		return fs_enum(i2s->mclk_rate / i2s->div_mclk_to_bck / i2s->div_bck_to_lrck);
	}
}

static int mt2701_i2s_set_sysclk(struct snd_soc_dai *dai, int clk_id, unsigned int freq, int dir)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;
	struct audio_i2s *out, *in;

	dev_dbg(dai->dev, "%s() cpu_dai id %d, freq %d, dir %d\n", __func__, dai->id, freq, dir);
	/* mclk */
	if (dir == SND_SOC_CLOCK_IN) {
		dev_warn(dai->dev, "%s() warning: mt2701 doesn't support mclk input\n", __func__);
		return -EINVAL;
	}
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;
	out = get_i2s_out(i2s_all, dai->id);
	in = get_i2s_in(i2s_all, dai->id);
	if (out)
		out->mclk_rate = freq;
	if (in)
		in->mclk_rate = freq;
	return 0;
}

static int mt2701_i2s_set_clkdiv(struct snd_soc_dai *dai, int div_id, int div)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;
	struct audio_i2s *out, *in;

	dev_dbg(dai->dev, "%s() cpu_dai id %d, div_id %d, div %d\n", __func__, dai->id, div_id,
		div);
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;
	out = get_i2s_out(i2s_all, dai->id);
	in = get_i2s_in(i2s_all, dai->id);
	switch (div_id) {
	case DIV_ID_MCLK_TO_BCK:
		if (out)
			out->div_mclk_to_bck = div;
		if (in)
			in->div_mclk_to_bck = div;
		break;
	case DIV_ID_BCK_TO_LRCK:
		if (out)
			out->div_bck_to_lrck = div;
		if (in)
			in->div_bck_to_lrck = div;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int mt2701_i2s_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;
	struct audio_i2s *out, *in;

	dev_dbg(dai->dev, "%s() cpu_dai id %d, fmt 0x%x\n", __func__, dai->id, fmt);
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;
	out = get_i2s_out(i2s_all, dai->id);
	in = get_i2s_in(i2s_all, dai->id);
	if (out)
		out->format = fmt;
	if (in)
		in->format = fmt;
	return 0;
}

static int i2s_out_configurate(const struct audio_i2s *out)
{
	int en;
	int use_asrc = 0;
	int dsd_mode;
	int slave;
	int one_heart_mode;
	enum afe_sampling_rate stream_fs, i2s_fs;
	enum afe_i2s_format i2s_fmt;

	if (!out)
		return -EINVAL;
	en = is_enable(out);
	i2s_fs = i2s_sampling_rate(out);
	slave = is_slave(out);
	one_heart_mode = is_one_heart(out);
	dsd_mode = is_dsd_mode(out);
	i2s_fmt = i2s_format(out);
	stream_fs = out->stream_fs;

	if (dsd_mode) {
		/* DSD */
		if (out->dai_id == MT2701_DAI_I2SM_ID) {
			pr_warn("%s() warning: can't output multi channel in dsd mode\n",
				   __func__);
			return -EINVAL;
		}
		use_asrc = 0;
	} else {
		/* PCM or DoP */
		if (slave) {
			/* slave */
			use_asrc = is_slave_use_asrc(out);
		} else {
			/* master */
			use_asrc = (i2s_fs != stream_fs) ? 1 : 0;
		}
	}

	/* configurate i2s-out */
	{
		struct afe_i2s_out_config i2s_config = {
			.fpga_test_loop = 0,
			.data_from_sine = 0,
			.use_asrc = use_asrc,
			.dsd_mode = dsd_mode,
			.dsd_use = I2S_OUT_DSD_USE_NORMAL,
			.couple_mode = 1,
			.one_heart_mode = one_heart_mode,
			.slave = slave,
			.fmt = i2s_fmt,
			.mclk = out->mclk_rate,
			.mode = i2s_fs
		};

		if (out->dai_id == MT2701_DAI_I2SM_ID) {
			afe_i2s_out_configurate(AFE_I2S_OUT_1, &i2s_config);
			afe_i2s_out_configurate(AFE_I2S_OUT_2, &i2s_config);
			afe_i2s_out_configurate(AFE_I2S_OUT_3, &i2s_config);
			afe_i2s_out_configurate(AFE_I2S_OUT_4, &i2s_config);
			afe_i2s_out_configurate(AFE_I2S_OUT_5, &i2s_config);
			afe_i2s_out_enable(AFE_I2S_OUT_1, en);

			afe_asmo_timing_set(AFE_I2S_OUT_1,
					    (slave && !use_asrc) ? FS_I2S1 : stream_fs);
			afe_asmo_timing_set(AFE_I2S_OUT_2,
					    (slave && !use_asrc) ? FS_I2S2 : stream_fs);
			afe_asmo_timing_set(AFE_I2S_OUT_3,
					    (slave && !use_asrc) ? FS_I2S3 : stream_fs);
			afe_asmo_timing_set(AFE_I2S_OUT_4,
					    (slave && !use_asrc) ? FS_I2S4 : stream_fs);
			afe_asmo_timing_set(AFE_I2S_OUT_5,
					    (slave && !use_asrc) ? FS_I2S5 : stream_fs);
		} else {
			enum afe_i2s_out_id i2s_id;

			i2s_id = get_i2s_out_id(out->dai_id);
			if (slave && !dsd_mode) {
				afe_i2s_out_slave_pcm_configurate(i2s_id, i2s_fmt, use_asrc);
				afe_i2s_out_slave_pcm_enable(i2s_id, en);
			} else if (!slave && dsd_mode) {
				afe_i2s_out_master_dsd_configurate(i2s_id, i2s_fs, out->mclk_rate,
								   I2S_OUT_DSD_USE_NORMAL);
				afe_i2s_out_master_dsd_enable(i2s_id, en);
			} else if (slave && dsd_mode) {
				afe_i2s_out_slave_dsd_configurate(i2s_id);
				afe_i2s_out_slave_dsd_enable(i2s_id, en);
			} else {
				afe_i2s_out_configurate(i2s_id, &i2s_config);
				afe_i2s_out_enable(i2s_id, en);
			}

			afe_asmo_timing_set(i2s_id,
					    (slave && !use_asrc) ? (FS_I2S1 + i2s_id) : stream_fs);
		}
	}

	/* configurate asrc */
	{
		enum afe_sample_asrc_tx_id asrc_id = get_sample_asrc_tx_id(out->dai_id);

		if (use_asrc && en) {
			struct afe_sample_asrc_config asrc_config = {
				.o16bit = 0,
				.mono = 0,
				.input_fs = stream_fs,
				.output_fs = i2s_fs,
				.tracking = slave
			};
			afe_sample_asrc_tx_configurate(asrc_id, &asrc_config);
			afe_sample_asrc_tx_enable(asrc_id, 1);
		} else {
			afe_sample_asrc_tx_enable(asrc_id, 0);
		}
	}

	return 0;
}

static int i2s_in_configurate(const struct audio_i2s *in)
{
	int en;
	int slave;
	int use_asrc = 0;
	int dsd_mode;
	enum afe_i2s_format i2s_fmt;
	enum afe_sampling_rate i2s_fs, stream_fs;

	if (!in)
		return -EINVAL;
	en = is_enable(in);
	i2s_fs = i2s_sampling_rate(in);
	stream_fs = in->stream_fs;
	i2s_fmt = i2s_format(in);
	slave = is_slave(in);
	dsd_mode = is_dsd_mode(in);

	if (dsd_mode) {
		/* DSD */
		use_asrc = 0;
	} else {
		/* PCM or DoP */
		if (slave) {
			/* slave */
			use_asrc = is_slave_use_asrc(in);
		} else {
			/* master */
			use_asrc = (i2s_fs != stream_fs) ? 1 : 0;
		}
	}

	/* configurate i2s-in */
	{
		enum afe_i2s_in_id i2s_id;

		i2s_id = get_i2s_in_id(in->dai_id);
		if (!slave && !dsd_mode) {
			afe_i2s_in_master_pcm_configurate(i2s_id, i2s_fmt, in->mclk_rate, i2s_fs,
							  use_asrc);
			afe_i2s_in_master_pcm_enable(i2s_id, en);
		} else if (!slave && dsd_mode) {
			afe_i2s_in_master_dsd_configurate(i2s_id, i2s_fs, in->mclk_rate);
			afe_i2s_in_master_dsd_enable(i2s_id, en);
		} else if (slave && dsd_mode) {
			afe_i2s_in_slave_dsd_configurate(i2s_id);
			afe_i2s_in_slave_dsd_enable(i2s_id, en);
		} else if (slave && !dsd_mode) {
			afe_i2s_in_slave_pcm_configurate(i2s_id, i2s_fmt, use_asrc);
			afe_i2s_in_slave_pcm_enable(i2s_id, en);
		} else {
			struct afe_i2s_in_config i2s_config = {
				.fpga_test_loop3 = 0,
				.fpga_test_loop = 0,
				.fpga_test_loop2 = 0,
				.use_asrc = use_asrc,
				.dsd_mode = dsd_mode,
				.slave = slave,
				.fmt = i2s_fmt,
				.mclk = in->mclk_rate,
				.mode = i2s_fs
			};
			afe_i2s_in_configurate(i2s_id, &i2s_config);
			afe_i2s_in_enable(i2s_id, en);
		}

		afe_asmi_timing_set(i2s_id, (slave && !use_asrc) ? (FS_I2S1 + i2s_id) : stream_fs);
	}

	/* configurate asrc */
	{
		enum afe_sample_asrc_rx_id asrc_id = get_sample_asrc_rx_id(in->dai_id);

		if (use_asrc && en) {
			struct afe_sample_asrc_config asrc_config = {
				.o16bit = 0,
				.mono = 0,
				.input_fs = i2s_fs,
				.output_fs = stream_fs,
				.tracking = slave
			};
			afe_sample_asrc_rx_configurate(asrc_id, &asrc_config);
			afe_sample_asrc_rx_enable(asrc_id, 1);
		} else {
			afe_sample_asrc_rx_enable(asrc_id, 0);
		}
	}

	return 0;
}

static int mt2701_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	int ret;
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;
	snd_pcm_format_t stream_fmt;
	enum afe_sampling_rate stream_fs;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;
	stream_fmt = params_format(params);
	stream_fs = fs_enum(params_rate(params));

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		struct audio_i2s *out;

		out = get_i2s_out(i2s_all, dai->id);
		if (!out)
			return -EINVAL;
		out->stream_fmt = stream_fmt;
		out->stream_fs = stream_fs;
		ret = i2s_out_configurate(out);
		if (ret < 0)
			return ret;
	} else {
		struct audio_i2s *in;

		in = get_i2s_in(i2s_all, dai->id);
		if (!in)
			return -EINVAL;
		in->stream_fmt = stream_fmt;
		in->stream_fs = stream_fs;
		ret = i2s_in_configurate(in);
		if (ret < 0)
			return ret;
	}
	return 0;
}

#ifdef CONFIG_MTK_LEGACY_CLOCK
static const enum cg_clk_id clks[] = {
	MT_CG_AUDIO_I2SIN1, /*AUDIO_TOP_CON4[0]*/
	MT_CG_AUDIO_I2SIN2, /*AUDIO_TOP_CON4[1]*/
	MT_CG_AUDIO_I2SIN3, /*AUDIO_TOP_CON4[2]*/
	MT_CG_AUDIO_I2SIN4, /*AUDIO_TOP_CON4[3]*/
	MT_CG_AUDIO_I2SIN5, /*AUDIO_TOP_CON4[4]*/
	MT_CG_AUDIO_I2SIN6, /*AUDIO_TOP_CON4[5]*/
	MT_CG_AUDIO_I2SO1, /*AUDIO_TOP_CON4[6]*/
	MT_CG_AUDIO_I2SO2, /*AUDIO_TOP_CON4[7]*/
	MT_CG_AUDIO_I2SO3, /*AUDIO_TOP_CON4[8]*/
	MT_CG_AUDIO_I2SO4, /*AUDIO_TOP_CON4[9]*/
	MT_CG_AUDIO_I2SO5, /*AUDIO_TOP_CON4[10]*/
	MT_CG_AUDIO_I2SO6, /*AUDIO_TOP_CON4[11]*/
	MT_CG_AUDIO_ASRCI1,  /*AUDIO_TOP_CON4[12]*/
	MT_CG_AUDIO_ASRCI2,  /*AUDIO_TOP_CON4[13]*/
	MT_CG_AUDIO_ASRCI3,  /*PWR2_TOP_CON[2]*/
	MT_CG_AUDIO_ASRCI4,  /*PWR2_TOP_CON[3]*/
	MT_CG_AUDIO_ASRCI5,  /*PWR2_TOP_CON[4]*/
	MT_CG_AUDIO_ASRCI6,  /*PWR2_TOP_CON[5]*/
	MT_CG_AUDIO_ASRCO1, /*AUDIO_TOP_CON4[14]*/
	MT_CG_AUDIO_ASRCO2, /*AUDIO_TOP_CON4[15]*/
	MT_CG_AUDIO_ASRCO3, /*PWR2_TOP_CON[6]*/
	MT_CG_AUDIO_ASRCO4, /*PWR2_TOP_CON[7]*/
	MT_CG_AUDIO_ASRCO5, /*PWR2_TOP_CON[8]*/
	MT_CG_AUDIO_ASRCO6, /*PWR2_TOP_CON[9]*/
};
#endif
static int has_ocuppied(const struct mt_i2s_all *all)
{
	int i;

	for (i = 0; i < AFE_I2S_OUT_NUM; ++i) {
		if (all->i2s_out[i].occupied)
			return 1;
	}
	if (all->i2s_out_mch.occupied)
		return 1;
	for (i = 0; i < AFE_I2S_IN_NUM; ++i) {
		if (all->i2s_in[i].occupied)
			return 1;
	}
	return 0;
}

static int mt2701_i2s_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;

	if ((dai->id == MT2701_DAI_I2S1_ID || dai->id == MT2701_DAI_I2SM_ID)
	    && priv->dsdenc.occupied) {
		return -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		enum afe_sample_asrc_tx_id tx_id;
		struct audio_i2s *out;

		out = get_i2s_out(i2s_all, dai->id);
		if (!out)
			return -EINVAL;
		if (out->occupied)
			return -EINVAL;
		if (out->dai_id == MT2701_DAI_I2SM_ID) {
			int dai_id_tmp;

			for (dai_id_tmp = MT2701_DAI_I2S1_ID; dai_id_tmp <= MT2701_DAI_I2S5_ID;
			     ++dai_id_tmp) {
				struct audio_i2s *out_tmp;

				out_tmp = get_i2s_out(i2s_all, dai_id_tmp);
				if (out_tmp && out_tmp->occupied)
					return -EINVAL;

			}
		} else if (out->dai_id >= MT2701_DAI_I2S1_ID && out->dai_id <= MT2701_DAI_I2S5_ID) {
			struct audio_i2s *out_tmp;

			out_tmp = get_i2s_out(i2s_all, MT2701_DAI_I2SM_ID);
			if (out_tmp && out_tmp->occupied)
				return -EINVAL;

		}
		if (!has_ocuppied(i2s_all)) {
			int i;

			#ifdef CONFIG_MTK_LEGACY_CLOCK
			for (i = 0; i < ARRAY_SIZE(clks); ++i)
				enable_clock(clks[i], "AUDIO");
			for (i = 0; i < 6; ++i)
				afe_i2s_power_on_mclk(i, 1);
			#else
			mt_afe_i2s_clk_on();
			dev_dbg(dai->dev, "%s()  dbg0\n", __func__);
			for (i = 0; i < 6; ++i)
				mt_i2s_power_on_mclk(i, 1);
			#endif
		}
		dev_dbg(dai->dev, "%s()  dbg1\n", __func__);
		out->occupied = 1;
		tx_id = get_sample_asrc_tx_id(dai->id);
		dev_dbg(dai->dev, "%s()  dbg2\n", __func__);
		afe_power_on_sample_asrc_tx(tx_id, 1);
		dev_dbg(dai->dev, "%s()  dbg3\n", __func__);
	} else {
		enum afe_sample_asrc_rx_id rx_id;
		struct audio_i2s *in;

		in = get_i2s_in(i2s_all, dai->id);
		if (!in)
			return -EINVAL;
		if (in->occupied)
			return -EINVAL;
		if (!has_ocuppied(i2s_all)) {
			int i;

			#ifdef CONFIG_MTK_LEGACY_CLOCK
			for (i = 0; i < ARRAY_SIZE(clks); ++i)
				enable_clock(clks[i], "AUDIO");
			for (i = 0; i < 6; ++i)
				afe_i2s_power_on_mclk(i, 1);
			#else
			mt_afe_i2s_clk_on();
			for (i = 0; i < 6; ++i)
				mt_i2s_power_on_mclk(i, 1);
			#endif
		}
		in->occupied = 1;
		rx_id = get_sample_asrc_rx_id(dai->id);
		afe_power_on_sample_asrc_rx(rx_id, 1);
	}
	dev_dbg(dai->dev, "%s() end cpu_dai id %d\n", __func__, dai->id);
	return 0;
}

static void mt2701_i2s_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;
	struct mt_i2s_all *i2s_all;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	i2s_all = &priv->i2s_all;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		enum afe_sample_asrc_tx_id tx_id;
		struct audio_i2s *out;

		out = get_i2s_out(i2s_all, dai->id);
		if (out)
			out->occupied = 0;
		tx_id = get_sample_asrc_tx_id(dai->id);
		afe_power_on_sample_asrc_tx(tx_id, 0);
	} else {
		enum afe_sample_asrc_rx_id rx_id;
		struct audio_i2s *in;

		in = get_i2s_in(i2s_all, dai->id);
		if (in)
			in->occupied = 0;
		rx_id = get_sample_asrc_rx_id(dai->id);
		afe_power_on_sample_asrc_rx(rx_id, 0);
	}
	if (!has_ocuppied(i2s_all)) {
		int i;

		#ifdef CONFIG_MTK_LEGACY_CLOCK
		for (i = 0; i < 6; ++i)
			afe_i2s_power_on_mclk(i, 0);

		for (i = 0; i < ARRAY_SIZE(clks); ++i)
			disable_clock(clks[i], "AUDIO");
		#else
		for (i = 0; i < 6; ++i)
			mt_i2s_power_on_mclk(i, 0);
		mt_afe_i2s_clk_off();
		#endif
	}
}

static void init_mt_i2s_all(struct mt_i2s_all *i2s_all)
{
	if (!i2s_all->inited) {
		int dai_id;

		for (dai_id = MT2701_DAI_I2S1_ID; dai_id <= MT2701_DAI_I2S6_ID; ++dai_id) {
			i2s_all->i2s_out[get_i2s_out_id(dai_id)].dai_id = dai_id;
			i2s_all->i2s_in[get_i2s_in_id(dai_id)].dai_id = dai_id;
		}
		i2s_all->i2s_out_mch.dai_id = MT2701_DAI_I2SM_ID;
		i2s_all->inited = 1;
	}
}

static int mt2701_i2s_probe(struct snd_soc_dai *dai)
{
	struct mt_dai_private *priv;

	dev_dbg(dai->dev, "%s() cpu_dai id %d\n", __func__, dai->id);
	priv = dev_get_drvdata(dai->dev);
	init_mt_i2s_all(&priv->i2s_all);
	return 0;
}

static struct snd_soc_dai_ops mt2701_i2s_ops = {
	.set_sysclk = mt2701_i2s_set_sysclk,
	.set_clkdiv = mt2701_i2s_set_clkdiv,
	.set_fmt = mt2701_i2s_set_fmt,
	.startup = mt2701_i2s_startup,
	.shutdown = mt2701_i2s_shutdown,
	.hw_params = mt2701_i2s_hw_params,
};
