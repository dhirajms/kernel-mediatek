/*
* Copyright (c) 2014 MediaTek Inc.
* Author: Chiawen Lee <chiawen.lee@mediatek.com>
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
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/version.h>
#include <linux/pm_runtime.h>

#include "mt8173_mfgsys.h"
#include "mt8173_mfgdvfs.h"

#ifndef MTK_MFG_DVFS
struct regulator *g_vgpu;
struct clk *g_mmpll;
#endif


static char *top_mfg_clk_name[] ={
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 11, 0))
	"mfg_mem_in_sel",
	"mfg_axi_in_sel",
	"top_axi",
	"top_mem",
	"top_mfg",
#else
	"MT_CG_MFG_POWER",
	"MT_CG_MFG_AXI",
	"MT_CG_MFG_MEM",
	"MT_CG_MFG_G3D",
	"MT_CG_MFG_26M",
#endif
};
#define MAX_TOP_MFG_CLK ARRAY_SIZE(top_mfg_clk_name)
static struct clk *g_top_clk[MAX_TOP_MFG_CLK];


static void __iomem *gbRegBase;
struct platform_device *mtkBackupPVRLDMDev = NULL;

static bool bMfgInit;


#define DRV_WriteReg32(ptr, data) \
	writel((u32)data, ptr)

#define DRV_Reg32(ptr) \
	(readl(ptr))

#define REG_MFG_AXI (1 << 0)
#define	REG_MFG_MEM (1 << 1)
#define	REG_MFG_G3D (1 << 2)
#define	REG_MFG_26M (1 << 3)
#define	REG_MFG_ALL (REG_MFG_AXI | REG_MFG_MEM | REG_MFG_G3D | REG_MFG_26M)

#define REG_MFG_CG_STA 0x00
#define REG_MFG_CG_SET 0x04
#define REG_MFG_CG_CLR 0x08

static int MtkEnableMfgClock(void)
{
	int i;

	mtk_mfg_debug("MtkEnableMfgClock Begin\n");

	pm_runtime_get_sync(&mtkBackupPVRLDMDev->dev);
	for (i = 0; i < MAX_TOP_MFG_CLK; i++)
		clk_prepare_enable(g_top_clk[i]);
	DRV_WriteReg32(gbRegBase + REG_MFG_CG_CLR, REG_MFG_ALL);

	mtk_mfg_debug("MtkEnableMfgClock  end\n");
	return PVRSRV_OK;
}

static int MtkDisableMfgClock(void)
{
	int i;

	mtk_mfg_debug("MtkDisableMfgClock Begin\n");

	DRV_WriteReg32(gbRegBase + REG_MFG_CG_SET, REG_MFG_ALL);
	for (i = MAX_TOP_MFG_CLK - 1; i >= 0; i--)
		clk_disable_unprepare(g_top_clk[i]);
	pm_runtime_put_sync(&mtkBackupPVRLDMDev->dev);

	mtk_mfg_debug("MtkDisableMfgClock end\n");
	return PVRSRV_OK;
}

static int MTKEnableHWAPM(void)
{
	mtk_mfg_debug("MTKEnableHWAPM...\n");
	DRV_WriteReg32(gbRegBase + 0x24, 0x003c3d4d);
	DRV_WriteReg32(gbRegBase + 0x28, 0x4d45440b);
	DRV_WriteReg32(gbRegBase + 0xe0, 0x7a710184);
	DRV_WriteReg32(gbRegBase + 0xe4, 0x835f6856);
	DRV_WriteReg32(gbRegBase + 0xe8, 0x002b0234);
	DRV_WriteReg32(gbRegBase + 0xec, 0x80000000);
	DRV_WriteReg32(gbRegBase + 0xa0, 0x08000000);
	return PVRSRV_OK;
}


static int MTKDisableHWAPM(void)
{
	mtk_mfg_debug("MTKDisableHWAPM...\n");
	DRV_WriteReg32(gbRegBase + 0x24, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0x28, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0xe0, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0xe4, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0xe8, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0xec, 0x00000000);
	DRV_WriteReg32(gbRegBase + 0xa0, 0x00000000);

	return PVRSRV_OK;
}

int MTKMFGSystemInit(void)
{
	mtk_mfg_debug("[MFG]MTKMFGSystemInit Begin\n");

	bMfgInit = true;

#ifndef MTK_MFG_DVFS
	MTKInitFreqInfo(mtkBackupPVRLDMDev);
#endif

	/* MTKEnableHWAPM(); */

	mtk_mfg_debug("MTKMFGSystemInit End\n");
	return PVRSRV_OK;
}

int MTKMFGSystemDeInit(void)
{
#ifndef MTK_MFG_DVFS
	MTKDeInitFreqInfo(mtkBackupPVRLDMDev);
#endif
	return PVRSRV_OK;
}

static DEFINE_MUTEX(g_DevPreMutex);
static DEFINE_MUTEX(g_DevPostMutex);


int MTKMFGGetClocks(struct platform_device *pdev)
{
	int i, err;

	gbRegBase = of_iomap(pdev->dev.of_node, 1);
	if (!gbRegBase) {
		mtk_mfg_debug("Unable to ioremap registers pdev %p\n", pdev);
		return -ENOMEM;
	}

#ifndef MTK_MFG_DVFS
	g_mmpll = devm_clk_get(&pdev->dev, "mmpll_clk");
	if (IS_ERR(g_mmpll)) {
		mtk_mfg_debug("Failed to look up clock 'mmpll_clk'\n");
		err = PTR_ERR(g_mmpll);
		goto err_iounmap_reg_base;
	}
#endif

	for (i = 0; i < MAX_TOP_MFG_CLK; i++) {
		g_top_clk[i] = devm_clk_get(&pdev->dev, top_mfg_clk_name[i]);
		if (IS_ERR(g_top_clk[i])) {
			mtk_mfg_debug("Failed to look up clock '%s'\n", top_mfg_clk_name[i]);
			err = PTR_ERR(g_top_clk[i]);
			g_top_clk[i] = NULL;
			goto err_iounmap_reg_base;
		}
	}

	mtkBackupPVRLDMDev = pdev;

#ifndef MTK_MFG_DVFS
	g_vgpu = devm_regulator_get(&pdev->dev, "mfgsys-power");
	if (IS_ERR(g_vgpu)) {
		mtk_mfg_debug("Failed to look up regulator 'mfgsys-power'\n");
		err = PTR_ERR(g_vgpu);
		goto err_iounmap_reg_base;
	}

	{
		int enable = regulator_enable(g_vgpu);
		if (enable != 0)
			mtk_mfg_debug("failed to enable regulator vgpu\n");
	}
#endif

	pm_runtime_enable(&pdev->dev);

	if (!bMfgInit)
		MTKMFGSystemInit();

	return 0;

err_iounmap_reg_base:
	iounmap(gbRegBase);
	return err;
}

static PVRSRV_DEV_POWER_STATE g_eCurrPowerState = PVRSRV_DEV_POWER_STATE_ON;
static PVRSRV_DEV_POWER_STATE g_eNewPowerState = PVRSRV_DEV_POWER_STATE_DEFAULT;

void MTKSysSetInitialPowerState(void)
{
	mtk_mfg_debug("MTKSysSetInitialPowerState ---\n");
}

void MTKSysRestoreInitialPowerState(void)
{
	mtk_mfg_debug("MTKSysRestoreInitialPowerState ---\n");
}

PVRSRV_ERROR MTKSysDevPrePowerState(PVRSRV_DEV_POWER_STATE eNewPowerState,
				    PVRSRV_DEV_POWER_STATE eCurrentPowerState,
				    IMG_BOOL bForced)
{
	mtk_mfg_debug("MTKSysDevPrePowerState (%d->%d), bForced = %d\n",
		      eCurrentPowerState, eNewPowerState, bForced);

	mutex_lock(&g_DevPreMutex);

	if (PVRSRV_DEV_POWER_STATE_OFF == eNewPowerState
	    && PVRSRV_DEV_POWER_STATE_ON == eCurrentPowerState)	{
		MTKDisableHWAPM();
		MtkDisableMfgClock();
	} else if (PVRSRV_DEV_POWER_STATE_ON == eNewPowerState
		   && PVRSRV_DEV_POWER_STATE_OFF == eCurrentPowerState) {
		MtkEnableMfgClock();
		MTKEnableHWAPM();
	} else {
		mtk_mfg_debug("MTKSysDevPrePowerState do nothing!\n");
	}

	g_eCurrPowerState = eCurrentPowerState;
	g_eNewPowerState = eNewPowerState;

	mutex_unlock(&g_DevPreMutex);
	return PVRSRV_OK;
}

PVRSRV_ERROR MTKSysDevPostPowerState(PVRSRV_DEV_POWER_STATE eNewPowerState,
				     PVRSRV_DEV_POWER_STATE eCurrentPowerState,
				     IMG_BOOL bForced)
{
	/* Post power sequence move to PrePowerState */
	return PVRSRV_OK;

	mtk_mfg_debug("MTKSysDevPostPowerState (%d->%d)\n",
		      eCurrentPowerState, eNewPowerState);

	mutex_lock(&g_DevPostMutex);

	if (PVRSRV_DEV_POWER_STATE_ON == eNewPowerState
	   && PVRSRV_DEV_POWER_STATE_OFF == eCurrentPowerState) {
		MtkEnableMfgClock();
	} else if (PVRSRV_DEV_POWER_STATE_OFF == eNewPowerState
		   && PVRSRV_DEV_POWER_STATE_ON == eCurrentPowerState) {
		MtkDisableMfgClock();
	} else {
		mtk_mfg_debug("MTKSysDevPostPowerState do nothing!\n");
	}

	mutex_unlock(&g_DevPostMutex);

	return PVRSRV_OK;
}

PVRSRV_ERROR MTKSystemPrePowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	mtk_mfg_debug("MTKSystemPrePowerState eNewPowerState %d\n",
		      eNewPowerState);
	return PVRSRV_OK;
}

PVRSRV_ERROR MTKSystemPostPowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	mtk_mfg_debug("MTKSystemPostPowerState eNewPowerState %d\n",
		      eNewPowerState);
	return PVRSRV_OK;
}
