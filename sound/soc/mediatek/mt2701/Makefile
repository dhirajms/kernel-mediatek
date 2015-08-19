ccflags-y += -DDEBUG

# platform driver
# platform
obj-$(CONFIG_MT_SND_SOC_2701) += mt2701-lp-audio.o
# platform
obj-$(CONFIG_MT_SND_SOC_2701) += mt2701-audio.o mt2701-hdmi-control.o
# cpu dai
obj-$(CONFIG_MT_SND_SOC_2701) += mt2701-dai.o
# hardware
obj-$(CONFIG_MT_SND_SOC_2701) += mt2701-afe.o mt2701-afe-clk.o mt2701-aud-gpio.o

# codec driver
obj-$(CONFIG_MT_SND_SOC_2701) += dummy-codec.o pcm1795.o pcm4202.o pcm1861.o pcm5102a.o

# others
obj-$(CONFIG_MT_SND_SOC_2701) += mt2701-asrc.o mt2701-audrtc.o mt2701-register-debug.o mt2701-afe-debug.o

# machine driver
obj-$(CONFIG_SND_SOC_MT2701_EVB) += mt2701-machine.o
obj-$(CONFIG_SND_SOC_MT2701_RT5640) += mt2701-rt5640.o
