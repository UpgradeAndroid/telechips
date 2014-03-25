#
# Makefile for the linux kernel.
#
# Common support
obj-$(CONFIG_ARCH_TCC92XX)	+= io.o irq.o devices.o time.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tca_ckc.o pm.o gpio.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tcc_display.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tca_serial.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tca_i2c.o

obj-$(CONFIG_TCC_EHI)		+= IO_EHI.o IO_IPC.o ipc_tty.o

obj-$(CONFIG_TCC_ECID_SUPPORT)  += tcc_ecid.o

ifeq ($(CONFIG_PM),y)
obj-$(CONFIG_ARCH_TCC92XX)	+= tcc_asm.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tcc_power.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tcc_wakeup.o
obj-$(CONFIG_DRAM_MDDR)		+= tcc_lpddr.o
obj-$(CONFIG_DRAM_DDR2)		+= tcc_ddr2.o
obj-$(CONFIG_ARCH_TCC92XX)	+= clock.o
endif

# Specific board support

obj-$(CONFIG_ARCH_TCC92XX)	+= idle.o
obj-$(CONFIG_ARCH_TCC92XX)	+= tca_adconv.o tcc_adconv.o tcc_adc.o
obj-$(CONFIG_ARCH_TCC92XX)	+= board-tcc9200-power.o

obj-$(CONFIG_CPU_FREQ)		+= cpufreq.o

obj-$(CONFIG_MACH_TCC9200S)	+= board-tcc9200s.o board-tcc9200s-gpio.o
obj-$(CONFIG_MACH_TCC9200S)	+= board-tcc9200s-mmc.o
obj-$(CONFIG_MACH_TCC9200S)	+= board-tcc9200s-dxb.o
obj-$(CONFIG_MACH_TCC9200S)	+= board-tcc9200s-panel.o
obj-$(CONFIG_MACH_TCC9200S)	+= board-tcc9200s-keypad.o

obj-$(CONFIG_MACH_TCC9201)	+= board-tcc9201.o board-tcc9201-gpio.o
obj-$(CONFIG_MACH_TCC9201)	+= board-tcc9201-mmc.o
obj-$(CONFIG_MACH_TCC9201)	+= board-tcc8900-dxb.o
obj-$(CONFIG_MACH_TCC9201)	+= board-tcc9201-panel.o
obj-$(CONFIG_MACH_TCC9201)	+= board-tcc9201-keypad.o

obj-$(CONFIG_MACH_TCC8900)	+= board-tcc8900.o board-tcc8900-gpio.o
obj-$(CONFIG_MACH_TCC8900)	+= board-tcc8900-mmc.o
obj-$(CONFIG_MACH_TCC8900)	+= board-tcc8900-dxb.o
obj-$(CONFIG_MACH_TCC8900)	+= board-tcc8900-panel.o
obj-$(CONFIG_MACH_TCC8900)	+= board-tcc8900-keypad.o

obj-$(CONFIG_MACH_M57TE)	+= board-m57te.o board-m57te-gpio.o
obj-$(CONFIG_MACH_M57TE)	+= board-m57te-mmc.o
obj-$(CONFIG_MACH_M57TE)	+= board-m57te-panel.o
obj-$(CONFIG_MACH_M57TE)	+= board-m57te-keypad.o


obj-$(CONFIG_MACH_M801)		+= board-m801.o board-m801-gpio.o
obj-$(CONFIG_MACH_M801)		+= board-m801-mmc.o
obj-$(CONFIG_MACH_M801)		+= board-m801-panel.o
obj-$(CONFIG_MACH_M801)		+= board-m801-keypad.o

obj-$(CONFIG_TCC_HDMI_DRIVER)	+= board-tcc8900-hdmi.o

obj-$(CONFIG_INPUT_TCC_REMOTE) += board-tcc9200-remocon.o
