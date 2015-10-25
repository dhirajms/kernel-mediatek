obj-$(CONFIG_POWERVR_MT8173) += mt8173.o
mt8173-y += mtk_module.o
mt8173-y += ion_support_mt8173.o
mt8173-y += mt8173_mfgsys.o
mt8173-y += mt8173_sysconfig.o
ccflags-y += -include $(srctree)/drivers/staging/imgtec/config_kernel.h \
 -I$(srctree)/$(src) \
 -I$(srctree)/drivers/staging/imgtec \
 -I$(srctree)/drivers/staging/imgtec/rogue \
 -I$(srctree)/drivers/staging/imgtec/rogue/hwdefs \
 -I$(srctree)/drivers/staging/imgtec/rogue/hwdefs/km \
 -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)
