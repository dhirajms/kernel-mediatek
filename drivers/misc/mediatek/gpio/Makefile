# Vanzo:chentianxiao on: Fri, 24 Mar 2017 10:03:16 +0800
# Modified for vanzo customization
project_name:=$(shell echo $(VANZO_INNER_PROJECT_NAME))
ifneq ($(project_name),)
ifeq ($(strip $(VANZO_MTK_PLATFORM)),MT6753)
    ccflags-y += -DVANZO_MTK_PLATFORM_MT6753
endif
ifeq ($(strip $(VANZO_MTK_PLATFORM)),MT6735)
    ccflags-y += -DVANZO_MTK_PLATFORM_MT6735
endif
endif
# End of Vanzo:chentianxiao

obj-$(CONFIG_MTK_GPIO)	+= mt_gpio_core.o
obj-$(CONFIG_MTK_GPIO)  += $(subst ",,$(CONFIG_MTK_PLATFORM))/
