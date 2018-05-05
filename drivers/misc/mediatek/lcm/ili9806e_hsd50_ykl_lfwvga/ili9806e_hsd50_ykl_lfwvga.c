/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef BUILD_LK
#include <linux/string.h>
#endif

#include "lcm_drv.h"

/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */

#define FRAME_WIDTH										(480)
#define FRAME_HEIGHT										(960)
#define LCM_DENSITY					(320)

/* physical size in um */
#define LCM_PHYSICAL_WIDTH    (59500)
#define LCM_PHYSICAL_HEIGHT   (104700)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      						0xFFF   // END OF REGISTERS MARKER

//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                      lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)       


struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
  {0xff, 5,{0xff,0x98,0x06,0x04,0x01}},
  {0x08, 1,{0x10}},
  {0x21, 1,{0x01}},
  {0x30, 1,{0x07}},
  {0x2e, 1,{0x77}},
  {0x31, 1,{0x02}},
  {0x40, 1,{0x10}},
  {0x41, 1,{0x44}},
  {0x42, 1,{0x03}},
  {0x43, 1,{0x09}},
  {0x44, 1,{0x05}},
  {0x50, 1,{0x78}},
  {0x51, 1,{0x78}},
  {0x52, 1,{0x00}},
  {0x53, 1,{0x37}},//3f
  {0x57, 1,{0x50}},
  {0x60, 1,{0x07}},
  {0x61, 1,{0x00}},
  {0x62, 1,{0x0f}},//0f
  {0x63, 1,{0x00}},
  {0xa0, 1,{0x00}},
  {0xa1, 1,{0x02}},
  {0xa2, 1,{0x08}},
  {0xa3, 1,{0x0b}},
  {0xa4, 1,{0x07}},
  {0xa5, 1,{0x18}},
  {0xa6, 1,{0x0a}},
  {0xa7, 1,{0x09}},
  {0xa8, 1,{0x03}},
  {0xa9, 1,{0x0a}},
  {0xaa, 1,{0x03}},
  {0xab, 1,{0x06}},
  {0xac, 1,{0x0d}},
  {0xad, 1,{0x3b}},
  {0xae, 1,{0x36}},
  {0xaf, 1,{0x00}},
  {0xc0, 1,{0x00}},
  {0xc1, 1,{0x02}},
  {0xc2, 1,{0x07}},
  {0xc3, 1,{0x10}},
  {0xc4, 1,{0x09}},
  {0xc5, 1,{0x18}},
  {0xc6, 1,{0x0a}},
  {0xc7, 1,{0x09}},
  {0xc8, 1,{0x03}},
  {0xc9, 1,{0x07}},
  {0xca, 1,{0x07}},
  {0xcb, 1,{0x05}},
  {0xcc, 1,{0x0b}},
  {0xcd, 1,{0x20}},
  {0xce, 1,{0x1c}},
  {0xcf, 1,{0x00}},
  {0xff, 5,{0xff, 0x98, 0x06,0x04,0x06}},
  {0x00, 1,{0x21}},
  {0x01, 1,{0x06}},
  {0x02, 1,{0x00}},
  {0x03, 1,{0x00}},
  {0x04, 1,{0x01}},
  {0x05, 1,{0x01}},
  {0x06, 1,{0x98}},//98
  {0x07, 1,{0x02}},
  {0x08, 1,{0x01}},//01
  {0x09, 1,{0x80}},
  {0x0a, 1,{0x00}},
  {0x0b, 1,{0x00}},
  {0x0c, 1,{0x08}},//08
  {0x0d, 1,{0x08}},//08
  {0x0e, 1,{0x00}},
  {0x0f, 1,{0x00}},
  {0x10, 1,{0xf0}},
  {0x11, 1,{0xf4}},
  {0x12, 1,{0x04}},//04
  {0x13, 1,{0x00}},
  {0x14, 1,{0x00}},
  {0x15, 1,{0xc0}},
  {0x16, 1,{0x00}},//00
  {0x17, 1,{0x00}},
  {0x18, 1,{0x00}},
  {0x19, 1,{0x00}},
  {0x1a, 1,{0x00}},
  {0x1b, 1,{0x00}},
  {0x1c, 1,{0x00}},
  {0x1d, 1,{0x00}},
  {0x20, 1,{0x01}},
  {0x21, 1,{0x23}},
  {0x22, 1,{0x45}},
  {0x23, 1,{0x67}},
  {0x24, 1,{0x01}},
  {0x25, 1,{0x23}},
  {0x26, 1,{0x45}},
  {0x27, 1,{0x67}},
  {0x30, 1,{0x01}},//01
  {0x31, 1,{0x22}},
  {0x32, 1,{0x22}},
  {0x33, 1,{0x22}},
  {0x34, 1,{0x22}},
  {0x35, 1,{0xbb}},
  {0x36, 1,{0xca}},
  {0x37, 1,{0xdd}},
  {0x38, 1,{0xac}},
  {0x39, 1,{0x76}},
  {0x3a, 1,{0x67}},
  {0x3b, 1,{0x22}},
  {0x3c, 1,{0x22}},
  {0x3d, 1,{0x22}},
  {0x3e, 1,{0x22}},
  {0x3f, 1,{0x22}},
  {0x40, 1,{0x22}},
  {0x52, 1,{0x10}},
  {0x53, 1,{0x10}},
  {0x54, 1,{0x13}},
  {0xff, 5,{0xff, 0x98, 0x06,0x04,0x07}},
  {0x17, 1,{0x22}},
  {0x02, 1,{0x77}},
  {0xe1, 1,{0x79}},
  {0x26, 1,{0xb2}},
  //{0xb3, 1,{0x10}},
  //{0x06, 1,{0x13}},
  {0xff, 5,{0xff, 0x98, 0x06,0x04,0x00}},

  {0x11,1,{0x00}},
  {REGFLAG_DELAY,120,{}},
  {0x29,1,{0x00}},                 // Display On
  {REGFLAG_DELAY,20,{}},
  //{0x21,1,{0x00}},                 // Display On
  {REGFLAG_END_OF_TABLE,0x00,{}}
};

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
  // Sleep Out
  {0x11, 0, {0x00}},
  {REGFLAG_DELAY, 120, {}},

  // Display ON
  {0x29, 0, {0x00}},
  {REGFLAG_DELAY, 20, {}},
  {REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
  // Display off sequence
  {0x28, 0, {0x00}},
  {REGFLAG_DELAY, 20, {}},
  // Sleep Mode On
  {0x10, 0, {0x00}},
  {REGFLAG_DELAY, 120, {}},
  {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
  unsigned int i;

  for(i = 0; i < count; i++) {

    unsigned cmd;
    cmd = table[i].cmd;

    switch (cmd) {

      case REGFLAG_DELAY :
        MDELAY(table[i].count);
        break;

      case REGFLAG_END_OF_TABLE :
        break;

      default:
        dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
    }
  }

}


/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->density = LCM_DENSITY;

	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;

	/* enable tearing-free */
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.mode = SYNC_PULSE_VDO_MODE;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_TWO_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	/* Not support in MT6573 */
	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

  params->dsi.vertical_sync_active              = 8;
  params->dsi.vertical_backporch                = 18;
  params->dsi.vertical_frontporch 				= 18;
  params->dsi.vertical_active_line				= FRAME_HEIGHT;

  params->dsi.horizontal_sync_active			= 6;
  params->dsi.horizontal_backporch				= 60;
  params->dsi.horizontal_frontporch				= 40;
  params->dsi.horizontal_active_pixel 		= FRAME_WIDTH;

  params->dsi.PLL_CLOCK = 210;
  params->dsi.ssc_disable = 1;  // ssc disable control (1: disable, 0: enable, default: 0)
}

static void lcm_init(void)
{

  SET_RESET_PIN(1);
  MDELAY(1);
  SET_RESET_PIN(0);
  MDELAY(10);//Must > 10ms
  SET_RESET_PIN(1);
  MDELAY(120);//Must > 120ms

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
  push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
  lcm_init();
}

LCM_DRIVER ili9806e_hsd50_ykl_lfwvga_lcm_drv = 
{
  .name			  = "ili9806e_hsd50_ykl_lfwvga",
  .set_util_funcs = lcm_set_util_funcs,
  .get_params     = lcm_get_params,
  .init           = lcm_init,
  .suspend        = lcm_suspend,
  .resume         = lcm_resume,
};
