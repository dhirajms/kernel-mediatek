ifeq ($(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT), y)
obj-y += \
    disp_ovl_engine_api.o \
    disp_ovl_engine_core.o \
    disp_ovl_engine_dev.o \
    disp_ovl_engine_sw.o \
    disp_ovl_engine_hw.o

ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys   \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox   \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/ovl_engine   \
             -I$(srctree)/drivers/misc/mediatek/sync/      \
             -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)       \
             -I$(srctree)/drivers/misc/mediatek/cmdq/$(MTK_PLATFORM)      \
             -I$(srctree)/drivers/misc/mediatek/cmdq/      \
             -I$(srctree)/drivers/misc/mediatek/externaldisplay/$(MTK_PLATFORM) \
             -I$(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/include/mach/ \
             -I$(srctree)/drivers/staging/android/ion/ \
             -I$(srctree)/drivers/misc/mediatek/base/power/ \
             -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/ \
             -I$(srctree)/drivers/misc/mediatek/leds/ \
             -I$(srctree)/drivers/misc/mediatek/mmp/ \
             -I$(srctree)/drivers/staging/android/ion/ \
             -I$(srctree)/drivers/staging/android/ion/mtk
ccflags-$(CONFIG_MTK_LCM) += -I$(srctree)/drivers/misc/mediatek/lcm/inc/


ccflags-y += -DMTK_OVERLAY_ENGINE_SUPPORT

ifeq ($(strip $(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)),y)
ccflags-y += -I$(srctree)/include -I$(srctree)/include/trustzone
ccflags-y += -DMTK_SEC_VIDEO_PATH_SUPPORT
endif
endif
