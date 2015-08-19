
obj-$(CONFIG_MTK_FB) += disp_drv.o                  \
		mtkfb_console.o                            	\
		disp_assert_layer.o                       	\
		mtkfb.o                                   	\
		mtkfb_vsync.o								\
		mtkfb_pattern.o                            	\
		debug.o                                    	\
		mtkfb_proc.o								\
		disp_drv_dbi.o								\
		disp_drv_dpi.o								\
		disp_drv_dsi.o								\
		disp_hal.o									\
		disp_intr.o

obj-$(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT) += ovl_engine/

ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys   \
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

ifeq ($(CONFIG_MTK_DITHERING_SUPPORT), y)
ccflags-y += -DDITHERING_SUPPORT
endif

ifeq ($(CONFIG_MTK_GPU_SUPPORT), y)
ccflags-y += -DHWGPU_SUPPORT
endif
