# Makefile for TCCxxx camera driver

obj-$(CONFIG_VIDEO_TCCXX_CAMERA) += camera_core.o tcc_cam.o cam.o tdd_cif.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR) += sensor_if.o tcc_cam_i2c.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MT9D112) += mt9d112_2mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_OV3640) += ov3640_3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MV9317) += mv9317_3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_S5K4BAFB) += s5k4bafb_2mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111) += mt9p111_5mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) += venus_ait848_5mp.o venusv3_api.o venus_api.o venus_i2c_ctl.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_ISX006) += isx006_5mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_OV7690) += ov7690_vga.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_HI704) += hi704_vga.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_SIV100B) += siv100b_vga.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_GT2005) += gt2005_2mp.o gt2005_drv.o
obj-$(CONFIG_VIDEO_CAMERA_NEXTCHIP_TEST) += nextchip_test.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113) += mt9m113_1.3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111) += mt9t111_3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_MT9T113) += mt9t113_3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_S5K5CAGA) += s5k5caga_3mp.o
obj-$(CONFIG_VIDEO_CAMERA_SENSOR_SR130PC10) += sr130pc10_1.3mp.o
obj-$(CONFIG_USE_ISP) += tccisp/
obj-$(CONFIG_VIDEO_TCCXX_ATV) += atv/

EXTRA_CFLAGS = -I$(src)/..
