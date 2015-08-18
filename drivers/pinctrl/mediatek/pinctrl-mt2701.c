/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Dandan He <dandan.he@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/regmap.h>
#include <dt-bindings/pinctrl/mt65xx.h>

#include "pinctrl-mtk-common.h"
#include "pinctrl-mtk-mt2701.h"

static const struct mtk_drv_group_desc mt2701_drv_grp[] =  {
	/* 0E4E8SR 4/8/12/16 */
	MTK_DRV_GRP(4, 16, 1, 2, 4),
	/* 0E2E4SR  2/4/6/8 */
	MTK_DRV_GRP(2, 8, 1, 2, 2),
	/* E8E4E2  2/4/6/8/10/12/14/16 */
	MTK_DRV_GRP(2, 16, 0, 2, 2)
};

static const struct mtk_pin_drv_grp mt2701_pin_drv[] = {
	MTK_PIN_DRV_GRP(0, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(1, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(2, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(3, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(4, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(5, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(6, 0xf50, 0, 1),
	MTK_PIN_DRV_GRP(7, 0xf50, 4, 1),
	MTK_PIN_DRV_GRP(8, 0xf50, 4, 1),
	MTK_PIN_DRV_GRP(9, 0xf50, 4, 1),
	MTK_PIN_DRV_GRP(10, 0xf50, 8, 1),
	MTK_PIN_DRV_GRP(11, 0xf50, 8, 1),
	MTK_PIN_DRV_GRP(12, 0xf50, 8, 1),
	MTK_PIN_DRV_GRP(13, 0xf50, 8, 1),
	MTK_PIN_DRV_GRP(14, 0xf50, 12, 0),
	MTK_PIN_DRV_GRP(15, 0xf50, 12, 0),
	MTK_PIN_DRV_GRP(16, 0xf60, 0, 0),
	MTK_PIN_DRV_GRP(17, 0xf60, 0, 0),
	MTK_PIN_DRV_GRP(18, 0xf60, 4, 0),
	MTK_PIN_DRV_GRP(19, 0xf60, 4, 0),
	MTK_PIN_DRV_GRP(20, 0xf60, 4, 0),
	MTK_PIN_DRV_GRP(21, 0xf60, 4, 0),
	MTK_PIN_DRV_GRP(22, 0xf60, 8, 0),
	MTK_PIN_DRV_GRP(23, 0xf60, 8, 0),
	MTK_PIN_DRV_GRP(24, 0xf60, 8, 0),
	MTK_PIN_DRV_GRP(25, 0xf60, 8, 0),
	MTK_PIN_DRV_GRP(26, 0xf60, 8, 0),
	MTK_PIN_DRV_GRP(27, 0xf60, 12, 0),
	MTK_PIN_DRV_GRP(28, 0xf60, 12, 0),
	MTK_PIN_DRV_GRP(29, 0xf60, 12, 0),
	MTK_PIN_DRV_GRP(30, 0xf60, 0, 0),
	MTK_PIN_DRV_GRP(31, 0xf60, 0, 0),
	MTK_PIN_DRV_GRP(32, 0xf60, 0, 0),
	MTK_PIN_DRV_GRP(33, 0xf70, 0, 0),
	MTK_PIN_DRV_GRP(34, 0xf70, 0, 0),
	MTK_PIN_DRV_GRP(35, 0xf70, 0, 0),
	MTK_PIN_DRV_GRP(36, 0xf70, 0, 0),
	MTK_PIN_DRV_GRP(37, 0xf70, 0, 0),
	MTK_PIN_DRV_GRP(38, 0xf70, 4, 0),
	MTK_PIN_DRV_GRP(39, 0xf70, 8, 1),
	MTK_PIN_DRV_GRP(40, 0xf70, 8, 1),
	MTK_PIN_DRV_GRP(41, 0xf70, 8, 1),
	MTK_PIN_DRV_GRP(42, 0xf70, 8, 1),
	MTK_PIN_DRV_GRP(43, 0xf70, 12, 0),
	MTK_PIN_DRV_GRP(44, 0xf70, 12, 0),
	MTK_PIN_DRV_GRP(45, 0xf70, 12, 0),
	MTK_PIN_DRV_GRP(47, 0xf80, 0, 0),
	MTK_PIN_DRV_GRP(48, 0xf80, 0, 0),
	MTK_PIN_DRV_GRP(49, 0xf80, 4, 0),
	MTK_PIN_DRV_GRP(50, 0xf70, 4, 0),
	MTK_PIN_DRV_GRP(51, 0xf70, 4, 0),
	MTK_PIN_DRV_GRP(52, 0xf70, 4, 0),
	MTK_PIN_DRV_GRP(53, 0xf80, 12, 0),
	MTK_PIN_DRV_GRP(54, 0xf80, 12, 0),
	MTK_PIN_DRV_GRP(55, 0xf80, 12, 0),
	MTK_PIN_DRV_GRP(56, 0xf80, 12, 0),
	MTK_PIN_DRV_GRP(60, 0xf90, 8, 1),
	MTK_PIN_DRV_GRP(61, 0xf90, 8, 1),
	MTK_PIN_DRV_GRP(62, 0xf90, 8, 1),
	MTK_PIN_DRV_GRP(63, 0xf90, 12, 1),
	MTK_PIN_DRV_GRP(64, 0xf90, 12, 1),
	MTK_PIN_DRV_GRP(65, 0xf90, 12, 1),
	MTK_PIN_DRV_GRP(66, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(67, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(68, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(69, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(70, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(71, 0xfA0, 0, 1),
	MTK_PIN_DRV_GRP(72, 0xf80, 4, 0),
	MTK_PIN_DRV_GRP(73, 0xf80, 4, 0),
	MTK_PIN_DRV_GRP(74, 0xf80, 4, 0),
	MTK_PIN_DRV_GRP(85, 0xDA0, 0, 2),
	MTK_PIN_DRV_GRP(86, 0xD90, 0, 2),
	MTK_PIN_DRV_GRP(87, 0xDB0, 0, 2),
	MTK_PIN_DRV_GRP(88, 0xDB0, 0, 2),
	MTK_PIN_DRV_GRP(89, 0xDB0, 0, 2),
	MTK_PIN_DRV_GRP(90, 0xDB0, 0, 2),
	MTK_PIN_DRV_GRP(105, 0xD40, 0, 2),
	MTK_PIN_DRV_GRP(106, 0xD30, 0, 2),
	MTK_PIN_DRV_GRP(107, 0xD50, 0, 2),
	MTK_PIN_DRV_GRP(108, 0xD50, 0, 2),
	MTK_PIN_DRV_GRP(109, 0xD50, 0, 2),
	MTK_PIN_DRV_GRP(110, 0xD50, 0, 2),
	MTK_PIN_DRV_GRP(111, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(112, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(113, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(114, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(115, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(116, 0xCD0, 0, 2),
	MTK_PIN_DRV_GRP(117, 0xCC0, 0, 2),
	MTK_PIN_DRV_GRP(118, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(119, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(120, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(121, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(126, 0xf80, 4, 0),
	MTK_PIN_DRV_GRP(188, 0xf70, 4, 0),
	MTK_PIN_DRV_GRP(189, 0xfE0, 8, 0),
	MTK_PIN_DRV_GRP(190, 0xfE0, 8, 0),
	MTK_PIN_DRV_GRP(191, 0xfE0, 8, 0),
	MTK_PIN_DRV_GRP(192, 0xfE0, 8, 0),
	MTK_PIN_DRV_GRP(193, 0xfE0, 8, 0),
	MTK_PIN_DRV_GRP(194, 0xfE0, 12, 0),
	MTK_PIN_DRV_GRP(195, 0xfE0, 12, 0),
	MTK_PIN_DRV_GRP(196, 0xfE0, 12, 0),
	MTK_PIN_DRV_GRP(197, 0xfE0, 12, 0),
	MTK_PIN_DRV_GRP(198, 0xfE0, 12, 0),
	MTK_PIN_DRV_GRP(199, 0xf50, 4, 1),
	MTK_PIN_DRV_GRP(200, 0xfD0, 0, 0),
	MTK_PIN_DRV_GRP(201, 0xfD0, 0, 0),
	MTK_PIN_DRV_GRP(202, 0xfD0, 0, 0),
	MTK_PIN_DRV_GRP(203, 0xfD0, 4, 0),
	MTK_PIN_DRV_GRP(204, 0xfD0, 4, 0),
	MTK_PIN_DRV_GRP(205, 0xfD0, 4, 0),
	MTK_PIN_DRV_GRP(206, 0xfD0, 4, 0),
	MTK_PIN_DRV_GRP(207, 0xfD0, 4, 0),
	MTK_PIN_DRV_GRP(208, 0xfD0, 8, 0),
	MTK_PIN_DRV_GRP(209, 0xfD0, 8, 0),
	MTK_PIN_DRV_GRP(210, 0xfD0, 12, 1),
	MTK_PIN_DRV_GRP(211, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(212, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(213, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(214, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(215, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(216, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(217, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(218, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(219, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(220, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(221, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(222, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(223, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(224, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(225, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(226, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(227, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(228, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(229, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(230, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(231, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(232, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(233, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(234, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(235, 0xfF0, 0, 1),
	MTK_PIN_DRV_GRP(236, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(237, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(238, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(239, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(240, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(241, 0xfF0, 4, 0),
	MTK_PIN_DRV_GRP(242, 0xfF0, 8, 0),
	MTK_PIN_DRV_GRP(243, 0xfF0, 8, 0),
	MTK_PIN_DRV_GRP(248, 0xf00, 0, 0),
	MTK_PIN_DRV_GRP(249, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(250, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(251, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(252, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(253, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(254, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(255, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(256, 0xfC0, 0, 2),
	MTK_PIN_DRV_GRP(257, 0xCE0, 0, 2),
	MTK_PIN_DRV_GRP(258, 0xCB0, 0, 2),
	MTK_PIN_DRV_GRP(259, 0xC90, 0, 2),
	MTK_PIN_DRV_GRP(260, 0x3A0, 0, 2),
	MTK_PIN_DRV_GRP(261, 0xD50, 0, 2),
	MTK_PIN_DRV_GRP(262, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(263, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(264, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(265, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(266, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(267, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(268, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(269, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(270, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(271, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(272, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(273, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(274, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(275, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(276, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(277, 0xf00, 8, 0),
	MTK_PIN_DRV_GRP(278, 0xf70, 8, 1),
};

static const struct mtk_pin_spec_pupd_set_samereg mt2701_spec_pupd[] = {
	MTK_PIN_PUPD_SPEC_SR(111, 0xd00, 12, 13, 14),   /* ms0 data7 */
	MTK_PIN_PUPD_SPEC_SR(112, 0xd00, 8, 9, 10),   /* ms0 data6 */
	MTK_PIN_PUPD_SPEC_SR(113, 0xd00, 4, 5, 6),   /* ms0 data5 */
	MTK_PIN_PUPD_SPEC_SR(114, 0xd00, 0, 1, 2),   /* ms0 data4 */
	MTK_PIN_PUPD_SPEC_SR(115, 0xd10, 0, 1, 2),   /* ms0 rstb */
	MTK_PIN_PUPD_SPEC_SR(116, 0xcd0, 8, 9, 10),   /* ms0 cmd */
	MTK_PIN_PUPD_SPEC_SR(117, 0xcc0, 8, 9, 10),   /* ms0 clk */
	MTK_PIN_PUPD_SPEC_SR(118, 0xcf0, 12, 13, 14),   /* ms0 data3 */
	MTK_PIN_PUPD_SPEC_SR(119, 0xcf0, 8, 9, 10),   /* ms0 data2 */
	MTK_PIN_PUPD_SPEC_SR(120, 0xcf0, 4, 5, 6),   /* ms0 data1 */
	MTK_PIN_PUPD_SPEC_SR(121, 0xcf0, 0, 1, 2),   /* ms0 data0 */

	MTK_PIN_PUPD_SPEC_SR(105, 0xd40, 8, 9, 10),    /* ms1 cmd */
	MTK_PIN_PUPD_SPEC_SR(106, 0xd30, 8, 9, 10),    /* ms1 clk */
	MTK_PIN_PUPD_SPEC_SR(107, 0xd60, 0, 1, 2),    /* ms1 dat0 */
	MTK_PIN_PUPD_SPEC_SR(108, 0xd60, 10, 9, 8),   /* ms1 dat1 */
	MTK_PIN_PUPD_SPEC_SR(109, 0xd60, 4, 5, 6), /* ms1 dat2 */
	MTK_PIN_PUPD_SPEC_SR(110, 0xc60, 12, 13, 14),    /* ms1 dat3 */

	MTK_PIN_PUPD_SPEC_SR(85, 0xda0, 8, 9, 10),	/* ms2 cmd */
	MTK_PIN_PUPD_SPEC_SR(86, 0xd90, 8, 9, 10),	/* ms2 clk */
	MTK_PIN_PUPD_SPEC_SR(87, 0xdc0, 0, 1, 2),    /* ms2 dat0 */
	MTK_PIN_PUPD_SPEC_SR(88, 0xdc0, 10, 9, 8),   /* ms2 dat1 */
	MTK_PIN_PUPD_SPEC_SR(89, 0xdc0, 4, 5, 6), /* ms2 dat2 */
	MTK_PIN_PUPD_SPEC_SR(90, 0xdc0, 12, 13, 14),	  /* ms2 dat3 */

	MTK_PIN_PUPD_SPEC_SR(85, 0xd40, 2, 1, 0),    /* ms2 dat0 */
	MTK_PIN_PUPD_SPEC_SR(86, 0xd40, 6, 5, 4),    /* ms2 dat1 */
	MTK_PIN_PUPD_SPEC_SR(87, 0xd40, 10, 9, 8),   /* ms2 dat2 */
	MTK_PIN_PUPD_SPEC_SR(88, 0xd40, 14, 13, 12), /* ms2 dat3 */
	MTK_PIN_PUPD_SPEC_SR(89, 0xc80, 2, 1, 0),    /* ms2 clk */
	MTK_PIN_PUPD_SPEC_SR(90, 0xc90, 2, 1, 0),    /* ms2 cmd */

	MTK_PIN_PUPD_SPEC_SR(249, 0x140, 0, 1, 2),    /* ms0e rstb */
	MTK_PIN_PUPD_SPEC_SR(250, 0x130, 12, 13, 14),    /* ms0e dat7 */
	MTK_PIN_PUPD_SPEC_SR(251, 0x130, 8, 9, 10),   /* ms0e dat6 */
	MTK_PIN_PUPD_SPEC_SR(252, 0x130, 4, 5, 6), /* ms0e dat5 */
	MTK_PIN_PUPD_SPEC_SR(253, 0x130, 0, 1, 2),    /* ms0e dat4 */
	MTK_PIN_PUPD_SPEC_SR(254, 0xf40, 12, 13, 14),     /* ms0e dat3 */
	MTK_PIN_PUPD_SPEC_SR(255, 0xf40, 8, 9, 10),   /* ms0e dat2 */
	MTK_PIN_PUPD_SPEC_SR(256, 0xf40, 4, 5, 6), /* ms0e dat1 */
	MTK_PIN_PUPD_SPEC_SR(257, 0xf40, 0, 1, 2),    /* ms0e dat0 */
	MTK_PIN_PUPD_SPEC_SR(258, 0xcb0, 8, 9, 10),	   /* ms0e cmd */
	MTK_PIN_PUPD_SPEC_SR(259, 0xc90, 8, 9, 10),	   /* ms0e clk */
	MTK_PIN_PUPD_SPEC_SR(261, 0x140, 8, 9, 10),	   /* ms1 ins */
};

static int mt2701_spec_pull_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, bool isup, unsigned int r1r0)
{
	return mtk_pctrl_spec_pull_set_samereg(regmap, mt2701_spec_pupd,
		ARRAY_SIZE(mt2701_spec_pupd), pin, align, isup, r1r0);
}

static const struct mtk_pin_ies_smt_set mt2701_ies_set[] = {
	MTK_PIN_IES_SMT_SPEC(0, 6, 0xB20, 0),
	MTK_PIN_IES_SMT_SPEC(7, 9, 0xB20, 1),
	MTK_PIN_IES_SMT_SPEC(10, 13, 0xB30, 3),
	MTK_PIN_IES_SMT_SPEC(14, 15, 0xB30, 13),
	MTK_PIN_IES_SMT_SPEC(16, 17, 0xB40, 7),
	MTK_PIN_IES_SMT_SPEC(18, 21, 0xB40, 13),
	MTK_PIN_IES_SMT_SPEC(22, 26, 0xB40, 13),
	MTK_PIN_IES_SMT_SPEC(27, 29, 0xB40, 13),
	MTK_PIN_IES_SMT_SPEC(30, 32, 0xB40, 7),
	MTK_PIN_IES_SMT_SPEC(33, 37, 0xB40, 13),
	MTK_PIN_IES_SMT_SPEC(38, 38, 0xB20, 13),
	MTK_PIN_IES_SMT_SPEC(39, 42, 0xB40, 13),
	MTK_PIN_IES_SMT_SPEC(43, 45, 0xB20, 10),
	MTK_PIN_IES_SMT_SPEC(47, 48, 0xB20, 11),
	MTK_PIN_IES_SMT_SPEC(49, 49, 0xB20, 12),
	MTK_PIN_IES_SMT_SPEC(50, 52, 0xB20, 13),
	MTK_PIN_IES_SMT_SPEC(53, 56, 0xB20, 14),
	MTK_PIN_IES_SMT_SPEC(57, 58, 0xB20, 15),
	MTK_PIN_IES_SMT_SPEC(59, 59, 0xB30, 10),
	MTK_PIN_IES_SMT_SPEC(60, 62, 0xB30, 0),
	MTK_PIN_IES_SMT_SPEC(63, 65, 0xB30, 1),
	MTK_PIN_IES_SMT_SPEC(66, 71, 0xB30, 2),
	MTK_PIN_IES_SMT_SPEC(72, 74, 0xB20, 12),
	MTK_PIN_IES_SMT_SPEC(75, 76, 0xB30, 3),
	MTK_PIN_IES_SMT_SPEC(77, 78, 0xB30, 4),
	MTK_PIN_IES_SMT_SPEC(79, 82, 0xB30, 5),
	MTK_PIN_IES_SMT_SPEC(83, 84, 0xB30, 2),
	MTK_PIN_IES_SMT_SPEC(85, 85, 0xDA0, 4),
	MTK_PIN_IES_SMT_SPEC(86, 86, 0xD90, 4),
	MTK_PIN_IES_SMT_SPEC(87, 90, 0xDB0, 4),
	MTK_PIN_IES_SMT_SPEC(101, 104, 0xB30, 6),
	MTK_PIN_IES_SMT_SPEC(105, 105, 0xD40, 4),
	MTK_PIN_IES_SMT_SPEC(106, 106, 0xD30, 4),
	MTK_PIN_IES_SMT_SPEC(107, 110, 0xD50, 4),
	MTK_PIN_IES_SMT_SPEC(111, 115, 0xCE0, 4),
	MTK_PIN_IES_SMT_SPEC(116, 116, 0xCD0, 4),
	MTK_PIN_IES_SMT_SPEC(117, 117, 0xCC0, 4),
	MTK_PIN_IES_SMT_SPEC(118, 121, 0xCE0, 4),
	MTK_PIN_IES_SMT_SPEC(122, 125, 0xB30, 7),
	MTK_PIN_IES_SMT_SPEC(126, 126, 0xB20, 12),
	MTK_PIN_IES_SMT_SPEC(127, 142, 0xB30, 9),
	MTK_PIN_IES_SMT_SPEC(143, 160, 0xB30, 10),
	MTK_PIN_IES_SMT_SPEC(161, 168, 0xB30, 12),
	MTK_PIN_IES_SMT_SPEC(169, 183, 0xB30, 10),
	MTK_PIN_IES_SMT_SPEC(184, 186, 0xB30, 9),
	MTK_PIN_IES_SMT_SPEC(187, 187, 0xB30, 14),
	MTK_PIN_IES_SMT_SPEC(188, 188, 0xB20, 13),
	MTK_PIN_IES_SMT_SPEC(189, 193, 0xB30, 15),
	MTK_PIN_IES_SMT_SPEC(194, 198, 0xB40, 0),
	MTK_PIN_IES_SMT_SPEC(199, 199, 0xB20, 1),
	MTK_PIN_IES_SMT_SPEC(200, 202, 0xB40, 1),
	MTK_PIN_IES_SMT_SPEC(203, 207, 0xB40, 2),
	MTK_PIN_IES_SMT_SPEC(208, 209, 0xB40, 3),
	MTK_PIN_IES_SMT_SPEC(210, 210, 0xB40, 4),
	MTK_PIN_IES_SMT_SPEC(211, 235, 0xB40, 5),
	MTK_PIN_IES_SMT_SPEC(236, 241, 0xB40, 6),
	MTK_PIN_IES_SMT_SPEC(242, 243, 0xB40, 7),
	MTK_PIN_IES_SMT_SPEC(244, 247, 0xB40, 8),
	MTK_PIN_IES_SMT_SPEC(248, 248, 0xB40, 9),
	MTK_PIN_IES_SMT_SPEC(249, 257, 0xFC0, 4),
	MTK_PIN_IES_SMT_SPEC(258, 258, 0xCB0, 4),
	MTK_PIN_IES_SMT_SPEC(259, 259, 0xC90, 4),
	MTK_PIN_IES_SMT_SPEC(260, 260, 0x3A0, 4),
	MTK_PIN_IES_SMT_SPEC(261, 261, 0xD50, 4),
	MTK_PIN_IES_SMT_SPEC(262, 277, 0xB40, 12),
	MTK_PIN_IES_SMT_SPEC(278, 278, 0xB40, 13),
};

static const struct mtk_pin_ies_smt_set mt2701_smt_set[] = {
	MTK_PIN_IES_SMT_SPEC(0, 6, 0xB50, 0),
	MTK_PIN_IES_SMT_SPEC(7, 9, 0xB50, 1),
	MTK_PIN_IES_SMT_SPEC(10, 13, 0xB60, 3),
	MTK_PIN_IES_SMT_SPEC(14, 15, 0xB60, 13),
	MTK_PIN_IES_SMT_SPEC(16, 17, 0xB70, 7),
	MTK_PIN_IES_SMT_SPEC(18, 21, 0xB70, 13),
	MTK_PIN_IES_SMT_SPEC(22, 26, 0xB70, 13),
	MTK_PIN_IES_SMT_SPEC(27, 29, 0xB70, 13),
	MTK_PIN_IES_SMT_SPEC(30, 32, 0xB70, 7),
	MTK_PIN_IES_SMT_SPEC(33, 37, 0xB70, 13),
	MTK_PIN_IES_SMT_SPEC(38, 38, 0xB50, 13),
	MTK_PIN_IES_SMT_SPEC(39, 42, 0xB70, 13),
	MTK_PIN_IES_SMT_SPEC(43, 45, 0xB50, 10),
	MTK_PIN_IES_SMT_SPEC(47, 48, 0xB50, 11),
	MTK_PIN_IES_SMT_SPEC(49, 49, 0xB50, 12),
	MTK_PIN_IES_SMT_SPEC(50, 52, 0xB50, 13),
	MTK_PIN_IES_SMT_SPEC(53, 56, 0xB50, 14),
	MTK_PIN_IES_SMT_SPEC(57, 58, 0xB50, 15),
	MTK_PIN_IES_SMT_SPEC(59, 59, 0xB60, 10),
	MTK_PIN_IES_SMT_SPEC(60, 62, 0xB60, 0),
	MTK_PIN_IES_SMT_SPEC(63, 65, 0xB60, 1),
	MTK_PIN_IES_SMT_SPEC(66, 71, 0xB60, 2),
	MTK_PIN_IES_SMT_SPEC(72, 74, 0xB50, 12),
	MTK_PIN_IES_SMT_SPEC(75, 76, 0xB60, 3),
	MTK_PIN_IES_SMT_SPEC(77, 78, 0xB60, 4),
	MTK_PIN_IES_SMT_SPEC(79, 82, 0xB60, 5),
	MTK_PIN_IES_SMT_SPEC(83, 84, 0xB60, 2),
	MTK_PIN_IES_SMT_SPEC(85, 85, 0xDA0, 11),
	MTK_PIN_IES_SMT_SPEC(86, 86, 0xD90, 11),
	MTK_PIN_IES_SMT_SPEC(87, 87, 0xDC0, 3),
	MTK_PIN_IES_SMT_SPEC(88, 88, 0xDC0, 7),
	MTK_PIN_IES_SMT_SPEC(89, 89, 0xDC0, 11),
	MTK_PIN_IES_SMT_SPEC(90, 90, 0xDC0, 15),
	MTK_PIN_IES_SMT_SPEC(101, 104, 0xB60, 6),
	MTK_PIN_IES_SMT_SPEC(105, 105, 0xD40, 11),
	MTK_PIN_IES_SMT_SPEC(106, 106, 0xD30, 11),
	MTK_PIN_IES_SMT_SPEC(107, 107, 0xD60, 3),
	MTK_PIN_IES_SMT_SPEC(108, 108, 0xD60, 7),
	MTK_PIN_IES_SMT_SPEC(109, 109, 0xD60, 11),
	MTK_PIN_IES_SMT_SPEC(110, 110, 0xD60, 15),
	MTK_PIN_IES_SMT_SPEC(111, 111, 0xD00, 15),
	MTK_PIN_IES_SMT_SPEC(112, 112, 0xD00, 11),
	MTK_PIN_IES_SMT_SPEC(113, 113, 0xD00, 7),
	MTK_PIN_IES_SMT_SPEC(114, 114, 0xD00, 3),
	MTK_PIN_IES_SMT_SPEC(115, 115, 0xD10, 3),
	MTK_PIN_IES_SMT_SPEC(116, 116, 0xCD0, 11),
	MTK_PIN_IES_SMT_SPEC(117, 117, 0xCC0, 11),
	MTK_PIN_IES_SMT_SPEC(118, 118, 0xCF0, 15),
	MTK_PIN_IES_SMT_SPEC(119, 119, 0xCF0, 11),
	MTK_PIN_IES_SMT_SPEC(120, 120, 0xCF0, 7),
	MTK_PIN_IES_SMT_SPEC(121, 121, 0xCF0, 3),
	MTK_PIN_IES_SMT_SPEC(122, 125, 0xB60, 7),
	MTK_PIN_IES_SMT_SPEC(126, 126, 0xB50, 12),
	MTK_PIN_IES_SMT_SPEC(127, 142, 0xB60, 9),
	MTK_PIN_IES_SMT_SPEC(143, 160, 0xB60, 10),
	MTK_PIN_IES_SMT_SPEC(161, 168, 0xB60, 12),
	MTK_PIN_IES_SMT_SPEC(169, 183, 0xB60, 10),
	MTK_PIN_IES_SMT_SPEC(184, 186, 0xB60, 9),
	MTK_PIN_IES_SMT_SPEC(187, 187, 0xB60, 14),
	MTK_PIN_IES_SMT_SPEC(188, 188, 0xB50, 13),
	MTK_PIN_IES_SMT_SPEC(189, 193, 0xB60, 15),
	MTK_PIN_IES_SMT_SPEC(194, 198, 0xB70, 0),
	MTK_PIN_IES_SMT_SPEC(199, 199, 0xB50, 1),
	MTK_PIN_IES_SMT_SPEC(200, 202, 0xB70, 1),
	MTK_PIN_IES_SMT_SPEC(203, 207, 0xB70, 2),
	MTK_PIN_IES_SMT_SPEC(208, 209, 0xB70, 3),
	MTK_PIN_IES_SMT_SPEC(210, 210, 0xB70, 4),
	MTK_PIN_IES_SMT_SPEC(211, 235, 0xB70, 5),
	MTK_PIN_IES_SMT_SPEC(236, 241, 0xB70, 6),
	MTK_PIN_IES_SMT_SPEC(242, 243, 0xB70, 7),
	MTK_PIN_IES_SMT_SPEC(244, 247, 0xB70, 8),
	MTK_PIN_IES_SMT_SPEC(248, 248, 0xB70, 9),
	MTK_PIN_IES_SMT_SPEC(249, 249, 0x140, 3),
	MTK_PIN_IES_SMT_SPEC(250, 250, 0x130, 15),
	MTK_PIN_IES_SMT_SPEC(251, 251, 0x130, 11),
	MTK_PIN_IES_SMT_SPEC(252, 252, 0x130, 7),
	MTK_PIN_IES_SMT_SPEC(253, 253, 0x130, 3),
	MTK_PIN_IES_SMT_SPEC(254, 254, 0xF40, 15),
	MTK_PIN_IES_SMT_SPEC(255, 255, 0xF40, 11),
	MTK_PIN_IES_SMT_SPEC(256, 256, 0xF40, 7),
	MTK_PIN_IES_SMT_SPEC(257, 257, 0xF40, 3),
	MTK_PIN_IES_SMT_SPEC(258, 258, 0xCB0, 11),
	MTK_PIN_IES_SMT_SPEC(259, 259, 0xC90, 11),
	MTK_PIN_IES_SMT_SPEC(260, 260, 0x3A0, 11),
	MTK_PIN_IES_SMT_SPEC(261, 261, 0x0B0, 3),
	MTK_PIN_IES_SMT_SPEC(262, 277, 0xB70, 12),
	MTK_PIN_IES_SMT_SPEC(278, 278, 0xB70, 13),
};

static int mt2701_ies_smt_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, int value, enum pin_config_param arg)
{
	if (arg == PIN_CONFIG_INPUT_ENABLE)
		return mtk_pconf_spec_set_ies_smt_range(regmap, mt2701_ies_set,
			ARRAY_SIZE(mt2701_ies_set), pin, align, value);
	else if (arg == PIN_CONFIG_INPUT_SCHMITT_ENABLE)
		return mtk_pconf_spec_set_ies_smt_range(regmap, mt2701_smt_set,
			ARRAY_SIZE(mt2701_smt_set), pin, align, value);
	return -EINVAL;
}

static const struct mtk_pinctrl_devdata mt2701_pinctrl_data = {
	.pins = mtk_pins_mt2701,
	.npins = ARRAY_SIZE(mtk_pins_mt2701),
	.grp_desc = mt2701_drv_grp,
	.n_grp_cls = ARRAY_SIZE(mt2701_drv_grp),
	.pin_drv_grp = mt2701_pin_drv,
	.n_pin_drv_grps = ARRAY_SIZE(mt2701_pin_drv),
	.spec_pull_set = mt2701_spec_pull_set,
	.spec_ies_smt_set = mt2701_ies_smt_set,
	.dir_offset = 0x0000,
	.pullen_offset = 0x0150,
	.pullsel_offset = 0x0280,
	.dout_offset = 0x0500,
	.din_offset = 0x0630,
	.pinmux_offset = 0x0760,
	.type1_start = 280,
	.type1_end = 280,
	.port_shf = 4,
	.port_mask = 0xf,
	.port_align = 4,
	.eint_offsets = {
		.name = "mt2701_eint",
		.stat      = 0x000,
		.ack       = 0x040,
		.mask      = 0x080,
		.mask_set  = 0x0c0,
		.mask_clr  = 0x100,
		.sens      = 0x140,
		.sens_set  = 0x180,
		.sens_clr  = 0x1c0,
		.soft      = 0x200,
		.soft_set  = 0x240,
		.soft_clr  = 0x280,
		.pol       = 0x300,
		.pol_set   = 0x340,
		.pol_clr   = 0x380,
		.dom_en    = 0x400,
		.dbnc_ctrl = 0x500,
		.dbnc_set  = 0x600,
		.dbnc_clr  = 0x700,
		.port_mask = 6,
		.ports     = 6,
	},
	.ap_num = 169,
	.db_cnt = 16,
};

static int mt2701_pinctrl_probe(struct platform_device *pdev)
{
	return mtk_pctrl_init(pdev, &mt2701_pinctrl_data, NULL);
}

static const struct of_device_id mt2701_pctrl_match[] = {
	{ .compatible = "mediatek,mt2701-pinctrl", },
	{}
};
MODULE_DEVICE_TABLE(of, mt2701_pctrl_match);

static struct platform_driver mtk_pinctrl_driver = {
	.probe = mt2701_pinctrl_probe,
	.driver = {
		.name = "mediatek-mt2701-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = mt2701_pctrl_match,
	},
};

static int __init mtk_pinctrl_init(void)
{
	return platform_driver_register(&mtk_pinctrl_driver);
}

module_init(mtk_pinctrl_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek Pinctrl Driver");
MODULE_AUTHOR("Dandan He <dandan.he@mediatek.com>");
