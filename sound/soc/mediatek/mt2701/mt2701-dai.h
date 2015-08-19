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


#ifndef _MT2701_DAI_H_
#define _MT2701_DAI_H_

#define MT2701_DAI_NONE             (0)
#define MT2701_DAI_I2S1_ID          (1)
#define MT2701_DAI_I2S2_ID          (2)
#define MT2701_DAI_I2S3_ID          (3)
#define MT2701_DAI_I2S4_ID          (4)
#define MT2701_DAI_I2S5_ID          (5)
#define MT2701_DAI_I2S6_ID          (6)
#define MT2701_DAI_I2SM_ID          (7)
#define MT2701_DAI_SPDIF_OUT_ID     (8)
#define MT2701_DAI_MULTI_IN_ID      (9)
#define MT2701_DAI_HDMI_OUT_I2S_ID (10)
#define MT2701_DAI_HDMI_OUT_IEC_ID (11)
/*#define MT2701_DAI_HDMI_IN_ID      (12)*/
#define MT2701_DAI_BTPCM_ID        (12)
#define MT2701_DAI_DSDENC_ID       (13)
#define MT2701_DAI_DMIC1_ID        (14)
#define MT2701_DAI_DMIC2_ID        (15)
#define MT2701_DAI_MRGIF_I2S_ID    (16)
#define MT2701_DAI_MRGIF_BT_ID     (17)
#define MT2701_DAI_DSDENC_RECORD_ID (18)
#define MT2701_DAI_NUM              (19)

#define DIV_ID_MCLK_TO_BCK  (0)
#define DIV_ID_BCK_TO_LRCK  (1)

#define SLAVE_USE_ASRC_MASK  (1U<<31)
#define SLAVE_USE_ASRC_YES   (1U<<31)
#define SLAVE_USE_ASRC_NO    (0U<<31)

#endif
