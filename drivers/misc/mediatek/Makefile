#$(call all-subdir-src-or-makefile)
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat
subdir-ccflags-y += -Werror -I$(srctree)/drivers/mmc/host/mediatek/$(MTK_PLATFORM)

obj-$(CONFIG_MTK_BASE_POWER) += base/
obj-$(CONFIG_MEDIATEK_SOLUTION)	+= sched/
obj-$(CONFIG_MEDIATEK_SOLUTION) += mach/
obj-$(CONFIG_MTK_GPT_COMMON) += gpt/
obj-$(CONFIG_MTK_SERIAL)	+= uart/
obj-$(CONFIG_MTK_SYS_CIRQ) += cirq/
obj-$(CONFIG_MTK_GIC_EXT) += ext_gic/
obj-$(CONFIG_MTK_ICE_DEUBG) +=ice_debug/
obj-$(CONFIG_MTK_GPS)	    += gps/
obj-$(CONFIG_MTK_NFC) += nfc/

obj-$(CONFIG_MMPROFILE) += mmp/
obj-$(CONFIG_MTK_AUXADC)	+= auxadc/
obj-$(CONFIG_MTK_M4U)	+= m4u/
obj-$(CONFIG_MTK_PWM)	    += pwm/
ifeq ($(CONFIG_MTK_SENSOR_SUPPORT),y)
obj-$(CONFIG_MTK_HWMON)     += hwmon/
obj-$(CONFIG_CUSTOM_KERNEL_ALSPS) += alsps/
obj-$(CONFIG_CUSTOM_KERNEL_ACCELEROMETER) += accelerometer/
obj-$(CONFIG_CUSTOM_KERNEL_GYROSCOPE) += gyroscope/
obj-$(CONFIG_CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR)	+= step_counter/
endif
obj-$(CONFIG_MTPROF)	+= mtprof/
obj-$(CONFIG_MTK_LEDS)	+= leds/
obj-$(CONFIG_MTK_VIBRATOR)	+= vibrator/
obj-$(CONFIG_MTK_ACCDET)	+= accdet/
obj-$(CONFIG_MTK_VIDEOCODEC_DRIVER) += videocodec/
obj-$(CONFIG_MTK_JPEG)	+= jpeg/
obj-$(CONFIG_MTK_RTC)	+= rtc/
obj-$(CONFIG_MTK_PMIC)	+= power/
obj-$(CONFIG_MTK_PMIC_WRAP) += pmic_wrap/
obj-$(CONFIG_SSUSB_DRV) += ssusb/
obj-$(CONFIG_SECURITY_SELINUX)	+= selinux_warning/
obj-$(CONFIG_MTK_MEM) += mem/
obj-$(CONFIG_MTK_CMDQ) += cmdq/
obj-$(CONFIG_MTK_BOOT) += boot/
obj-$(CONFIG_MTK_BOOT_REASON) += boot_reason/
obj-$(CONFIG_MTK_CHIP) += chip/
obj-$(CONFIG_MTK_DEVINFO_DRIVER) += devinfo/
obj-$(CONFIG_MTK_LCM) += lcm/
obj-y += ext_disp/
obj-$(CONFIG_MTK_FB) += video/
obj-$(CONFIG_MTK_GPU_SUPPORT) += gpu/
ifneq ($(CONFIG_MTK_PLATFORM), "mt2701")
obj-$(CONFIG_MTK_SMI_EXT)	+= smi/
endif
obj-$(CONFIG_MTK_DRAMC) += dramc/
obj-$(CONFIG_MTK_SYNC) += sync/
obj-$(CONFIG_MTK_FREQ_HOPPING) += freqhopping/
obj-$(CONFIG_MTK_SECURITY_SW_SUPPORT) +=masp/
obj-$(CONFIG_MEDIATEK_SOLUTION)  += sched/
obj-y += cameraisp/src/
obj-$(CONFIG_MTK_IMGSENSOR) += imgsensor/src/
obj-$(CONFIG_MTK_LENS)   += lens/
obj-$(CONFIG_MTK_FLASHLIGHT)	+= flashlight/src/
ifneq ($(wildcard $(srctree)/drivers/misc/mediatek/met/),)
obj-$(CONFIG_MTK_MET) += met/
endif

obj-$(CONFIG_MTK_COMBO) += connectivity/

obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += gcpu/
obj-$(CONFIG_TRUSTONIC_TEE_SUPPORT) += gcpu/
obj-$(CONFIG_MTK_DEBUG_LATCH)	+= debug_latch/
obj-$(CONFIG_MTK_SYSTRACKER)   += systracker/
obj-$(CONFIG_MTK_RAM_CONSOLE)	+= ram_console/

# gator driver is platform-independent
ifneq ($(wildcard $(srctree)/$(src)/gator-driver/),)
obj-$(CONFIG_GATOR_DRIVER) += gator-driver/
endif # gator-driver folder exist

obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += trustzone/
ifeq ($(CONFIG_HAVE_MTK_IN_HOUSE_TEE_SUPPORT),y)
obj-$(CONFIG_TRUSTY) += trustzone/
endif

ifeq (,$(filter $(CONFIG_ARCH_MT8127), y))
obj-$(CONFIG_THERMAL)   += thermal/
endif

obj-$(CONFIG_MEDIATEK_SOLUTION)	+= aee/
obj-$(CONFIG_MTK_ATF_LOGGER)	+= atf_log/
obj-$(CONFIG_MTK_MLOG) += mlog/
obj-$(CONFIG_MTK_EMI_BWL)	+= emi_bwl/
obj-$(CONFIG_MTK_EMI_MPU)	+= emi_mpu/
obj-$(CONFIG_MTK_EMI_MBW) += emi_mbw/
ifneq ($(wildcard $(srctree)/drivers/misc/mediatek/tc1_interface/),)
obj-$(CONFIG_MTK_TC1_FEATURE)   += tc1_interface/
endif
obj-$(CONFIG_MTK_DYNAMIC_BOOST) += dynamic_boost/

