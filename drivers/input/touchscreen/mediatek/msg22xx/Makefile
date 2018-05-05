# Linux driver folder
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/msg22xx/
ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/msg22xx/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/msg22xx/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hwmon/include/

project_name:=$(shell echo $(VANZO_INNER_PROJECT_NAME))
ifneq ($(strip $(project_name)),)
    ifeq ($(strip $(CUSTOM_KERNEL_ALSPS)),touch_ps)
        ccflags-y += -DCONFIG_ENABLE_PROXIMITY_DETECTION
    endif
endif

obj-y   +=  mstar_drv_common.o
obj-y   +=  mstar_drv_hotknot.o
obj-y   +=  mstar_drv_hotknot_queue.o
obj-y   +=  mstar_drv_ic_fw_porting_layer.o
obj-y   +=  mstar_drv_jni_interface.o
obj-y   +=  mstar_drv_main.o
obj-y   +=  mstar_drv_mtk.o
obj-y   +=  mstar_drv_mutual_fw_control.o
obj-y   +=  mstar_drv_mutual_mp_test.o
obj-y   +=  mstar_drv_platform_interface.o
obj-y   +=  mstar_drv_platform_porting_layer.o
obj-y   +=  mstar_drv_self_fw_control.o
obj-y   +=  mstar_drv_self_mp_test.o
obj-y   +=  mstar_drv_utility_adaption.o
