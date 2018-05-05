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
#define LCM_ID 0x8800
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
{0x11, 0,{}},
{REGFLAG_DELAY,120,{}}, 
{0xff, 5,{0x77,0x01,0x00,0x00,0x10}},
{0xc0, 2,{0x77,0x00}},
{0xc1, 2,{0x0e,0x02}},
{0xc2, 2,{0x31,0x02}},
{0xb0, 16,{0x00,0x03,0x0c,0x0c,0x14,0x09,0x04,0x09,0x08,0x1c,0x08,0x14,0x10,0x0d,0x11,0x18}},
{0xb1, 16,{0x00,0x01,0x0a,0x0a,0x12,0x07,0x04,0x09,0x08,0x1c,0x08,0x17,0x15,0x15,0x1b,0x18}},
{0xff, 5,{0x77,0x01,0x00,0x00,0x11}},
{0xb0, 1,{0x4d}},
{0xb1, 1,{0x4c}},
{0xb2, 1,{0x07}},
{0xb3, 1,{0x80}},
{0xb5, 1,{0x47}},
{0xb7, 1,{0x8a}},
{0xb8, 1,{0x21}},
{0xc0, 1,{0x07}},
{0xc1, 1,{0x78}},
{0xc2, 1,{0x78}},
{0xd0, 1,{0x88}},
{0xe0, 3,{0x00,0x00,0x02}},
{0xe1, 11,{0x07,0x8c,0x09,0x8c,0x06,0x8c,0x08,0x8c,0x00,0x44,0x44}},
{0xe2, 12,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xe3, 4,{0x00,0x00,0x33,0x33}},
{0xe4, 2,{0x44,0x44}},
{0xe5, 16,{0x0f,0xca,0x8c,0x8c,0x11,0xca,0x8c,0x8c,0x0b,0xca,0x8c,0x8c,0x0d,0xca,0x8c,0x8c}},
{0xe6, 4,{0x00,0x00,0x33,0x33}},
{0xe7, 2,{0x44,0x44}},
{0xe8, 16,{0x0e,0xca,0x8c,0x8c,0x10,0xca,0x8c,0x8c,0x0a,0xca,0x8c,0x8c,0x0c,0xca,0x8c,0x8c}},
{0xeb, 2,{0x02,0x00}},
{0xed, 16,{0xab,0x89,0x76,0x54,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0x10,0x45,0x67,0x98,0xba}},
{0x29, 0,{}},
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
  {0x28, 1, {0x00}},
  {REGFLAG_DELAY, 20, {}},
  // Sleep Mode On
  {0x10, 1, {0x00}},
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
  params->dsi.vertical_backporch                = 20;
  params->dsi.vertical_frontporch 				= 20;
  params->dsi.vertical_active_line				= FRAME_HEIGHT;

  params->dsi.horizontal_sync_active			= 20;
  params->dsi.horizontal_backporch				= 80;
  params->dsi.horizontal_frontporch				= 80;
  params->dsi.horizontal_active_pixel 		= FRAME_WIDTH;

  params->dsi.PLL_CLOCK = 212;
//  params->dsi.ssc_disable = 1;  // ssc disable control (1: disable, 0: enable, default: 0)
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
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static int adc_read_vol(void)
{
	int adc[1];
	int data[4] ={0,0,0,0};
	int sum = 0;
	int adc_vol=0;
	int num = 0;

	for(num=0;num<10;num++)
	{
		IMM_GetOneChannelValue(12, data, adc);
		sum+=(data[0]*100+data[1]);
	}
	adc_vol = sum/10;

#if defined(BUILD_LK)
	printf("wujie  adc_vol is %d\n",adc_vol);
#else
	printk("wujie  adc_vol is %d\n",adc_vol);
#endif

	return (adc_vol>80) ? 0 : 1;
}
static unsigned int lcm_compare_id(void)
{
	unsigned int array[4];
	unsigned short device_id;
	unsigned char buffer[4];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(20);	

    array[0]=0x00063902;
    array[1]=0x000177ff;
    array[2]=0x00001000;
    dsi_set_cmdq(array, 3, 1);
	MDELAY(10); 

	read_reg_v2(0xa1, buffer, 2);	
	device_id = (buffer[0]<<8) | (buffer[1]+adc_read_vol());

#if defined(BUILD_LK)
	printf("st7701_ivo50_hongzhan_fwvga  %s,line=%d, id = 0x%x, buffer[0]=0x%08x,buffer[1]=0x%08x\n",__func__,__LINE__, device_id, buffer[0],buffer[1]);
#else
	printk("st7701_ivo50_hongzhan_fwvga  %s,line=%d, id = 0x%x, buffer[0]=0x%08x,buffer[1]=0x%08x\n",__func__,__LINE__, device_id, buffer[0],buffer[1]);
#endif
  return (LCM_ID == device_id)?1:0;

}

LCM_DRIVER st7701s_hsd50_fsd_lfwvga_lcm_drv = 
{
  .name			  = "st7701s_hsd50_fsd_lfwvga",
  .set_util_funcs = lcm_set_util_funcs,
  .get_params     = lcm_get_params,
  .init           = lcm_init,
  .suspend        = lcm_suspend,
  .resume         = lcm_resume,
  .compare_id    = lcm_compare_id,
};
