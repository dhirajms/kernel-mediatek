
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox \
             -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/ovl_engine/ \
             -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/ \
             -I$(srctree)/drivers/misc/mediatek/leds/ \
             -I$(srctree)/drivers/misc/mediatek/mmp/
ccflags-$(CONFIG_MTK_LCM) += -I$(srctree)/drivers/misc/mediatek/lcm/inc/

obj-y += ddp_drv.o
obj-y += ddp_path.o
obj-y += ddp_ovl.o
obj-y += ddp_bls.o
obj-y += ddp_color.o
obj-y += ddp_rdma.o
obj-y += ddp_wdma.o
obj-y += ddp_aal.o
obj-y += ddp_debug.o
obj-y += lcd_drv.o
obj-y += dpi_drv.o
obj-y += dsi_drv.o
#obj-y += ddp_cmdq.o
#obj-y += ddp_cmdq_debug.o
#obj-y += ddp_cmdq_sec.o
#obj-y += cmdq_ccf.o
#obj-y += ddp_dpfd.o

ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)
endif

ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/mhltx/$(CONFIG_MTK_PLATFORM)
endif
ccflags-y += -Werror

ifeq ($(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT), y)
ccflags-y += -DMTK_OVERLAY_ENGINE_SUPPORT
endif
