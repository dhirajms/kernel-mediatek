ifeq ($(CONFIG_MTK_COMBO_WIFI),y)
    subdir-ccflags-y += -D MTK_WCN_BUILT_IN_DRIVER
endif

ifneq ($(filter "MT6630",$(CONFIG_MTK_COMBO_CHIP)),)
	obj-y   +=  gen3/
endif
ifneq ($(filter "CONSYS_6797",$(CONFIG_MTK_COMBO_CHIP)),)
	obj-y   +=  gen3/
endif
