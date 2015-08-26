
ccflags-y += -I$(srctree)/drivers/misc/mediatek/imgsensor/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/mach

obj-y += kd_sensorlist.o
obj-y += mt_cam.o

##
## sensor driver should change folder path, and correct the following make path
##
define FILTER_PROJECT_DRV
ifeq ($(wildcard $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/imgsensor/$(1)),)
platform_drv += $(1)
endif
endef

$(foreach drv,$(subst $\",,$(CONFIG_CUSTOM_KERNEL_IMGSENSOR)),$(eval $(call FILTER_PROJECT_DRV,$(drv))))
custom_kernel_imgsensor := $(addsuffix /, $(shell echo $(platform_drv)))
obj-y += $(custom_kernel_imgsensor)
$(info imgsensor drv by platform $(platform_drv))
