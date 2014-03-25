#
# Makefile for the linux kernel.
#
# Common support
obj-$(CONFIG_ARCH_TCC93XX) += io.o irq.o devices.o time.o tca_ckc.o idle.o clock.o gpio.o tcc_display.o pm.o
obj-$(CONFIG_ARCH_TCC93XX)	+= tca_adconv.o tcc_adc.o
obj-$(CONFIG_ARCH_TCC93XX)	+= tcc_asm.o
obj-$(CONFIG_ARCH_TCC93XX)      += tca_serial.o
obj-$(CONFIG_ARCH_TCC93XX)	+= tca_i2c.o

obj-$(CONFIG_ARCH_TCC93XX)	+= tcc_power.o
obj-$(CONFIG_DRAM_DDR2)		+= tcc_ddr.o
obj-$(CONFIG_DRAM_DDR3)		+= tcc_ddr.o

obj-$(CONFIG_CPU_FREQ)	+= cpufreq.o

#obj-$(CONFIG_ARCH_TCC) += tcc_ckc_ctrl.o
obj-$(CONFIG_ARCH_TCC) += arm_ioctlutil.o

#obj-$(CONFIG_TCC_PM) += pm.o pm_asm.o

# Specific board support

obj-$(CONFIG_MACH_TCC9300) += board-tcc9300.o board-tcc9300-gpio.o board-tcc9300-panel.o board-tcc9300-hdmi.o board-tcc9300-power.o
obj-$(CONFIG_MACH_TCC9300) += board-tcc9300-keypad.o
obj-$(CONFIG_MACH_TCC9300) += board-tcc9300-mmc.o
obj-$(CONFIG_MACH_TCC9300) += board-tcc9300-dxb.o

obj-$(CONFIG_MACH_TCC9300ST) += board-tcc9300st.o board-tcc9300st-gpio.o board-tcc9300st-panel.o board-tcc9300st-hdmi.o
#obj-$(CONFIG_MACH_TCC9300ST) += board-tcc9300st-keypad.o
obj-$(CONFIG_MACH_TCC9300ST) += board-tcc9300st-mmc.o
obj-$(CONFIG_MACH_TCC9300ST) += board-tcc9300st-dxb.o

obj-$(CONFIG_MACH_TCC9300CM) += board-tcc9300cm.o board-tcc9300cm-gpio.o board-tcc9300cm-panel.o board-tcc9300cm-hdmi.o
obj-$(CONFIG_MACH_TCC9300CM) += board-tcc9300cm-keypad.o
obj-$(CONFIG_MACH_TCC9300CM) += board-tcc9300cm-mmc.o
obj-$(CONFIG_MACH_TCC9300CM) += board-tcc9300cm-dxb.o

obj-$(CONFIG_INPUT_TCC_REMOTE) += board-tcc9300-remocon.o
