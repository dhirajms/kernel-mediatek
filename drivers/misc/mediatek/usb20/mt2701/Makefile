#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
ccflags-$(CONFIG_USB_MTK_HDRC) += -I$(srctree)/drivers/misc/mediatek/usb20


ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DP1_PROJECT
//ccflags-y += -DMTK_USB_NO_BATTERY

ccflags-y += -DMUSB_QMU_SUPPORT
#ccflags-y += -DMUSB_QMU_SUPPORT_HOST

#include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -Werror

obj-$(CONFIG_USB_MTK_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_MTK_HDRC)  += usb20_host.o
obj-$(CONFIG_USBIF_COMPLIANCE)   += usb20_otg_if.o
obj-$(CONFIG_USB_MTK_HDRC)	+= usb20_phy_debugfs.o
