#
# Makefile for Telechips TCC88xx processors
#

# Common support

obj-$(CONFIG_ARCH_TCC892X)      += cpu_early_init.o
obj-$(CONFIG_ARCH_TCC892X)      += time.o irq.o io.o clock.o tcc_ddr.o
obj-$(CONFIG_ARCH_TCC892X)      += pm.o gpio.o 
obj-$(CONFIG_ARCH_TCC892X)      += devices.o
obj-$(CONFIG_ARCH_TCC892X)      += tca_ckc.o
obj-$(CONFIG_ARCH_TCC892X)      += tca_adconv.o tcc_adc.o
obj-$(CONFIG_ARCH_TCC892X)      += tcc_asm.o
obj-$(CONFIG_ARCH_TCC892X)      += tca_serial.o
obj-$(CONFIG_ARCH_TCC892X)      += tca_i2c.o
obj-$(CONFIG_ARCH_TCC892X)      += vioc/vioc_disp.o vioc/vioc_outcfg.o vioc/vioc_rdma.o vioc/vioc_wdma.o vioc/vioc_wmix.o

obj-$(CONFIG_CPU_FREQ)          += cpufreq.o

obj-$(CONFIG_TCC_ECID_SUPPORT)  += tcc_ecid.o

# Specific board support
obj-$(CONFIG_MACH_TCC8920)      += board-tcc8920.o board-tcc8920-gpio.o board-tcc8920-keypad.o board-tcc8920-power.o
# obj-$(CONFIG_MACH_TCC8920)      += board-tcc8920-panel.o
# obj-$(CONFIG_MACH_TCC8920)      += board-tcc8920-mmc.o
# obj-$(CONFIG_MACH_TCC8920)      += board-tcc8920-dxb.o
# obj-$(CONFIG_MACH_TCC8920)      += board-tcc8920-hdmi.o

# obj-$(CONFIG_INPUT_TCC_REMOTE)  += board-tcc8800-remocon.o
