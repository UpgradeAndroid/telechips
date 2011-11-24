#
# Copyright (C) 2010-2011 ARM Limited. All rights reserved.
# 
# This program is free software and is provided to you under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
# 
# A copy of the licence is included with the program, and can also be obtained from Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

OSKOS=linux
FILES_PREFIX=

MALI_RELEASE_NAME = r2p3-00dev1

include $(src)/Makefile.common

ifeq ($(CONFIG_MALI_UMP),y)
USING_UMP = 1
else
USING_UMP = 0
endif

ifeq ($(CONFIG_MALI_OS_MEMORY),y)
USING_OS_MEMORY = 1
else
USING_OS_MEMORY = 0
endif

# set up defaults if not defined by the user
ARCH = arm
USING_MMU ?= 1
USING_UMP ?= 1
USING_MALI_PMU ?= 0
USING_PMM ?= 1
USING_OS_MEMORY ?= 1

USING_GPU_UTILIZATION ?= 0
USING_MALI_RUN_TIME_PM ?= 0
USING_MALI_PMM_TESTSUITE ?= 0
OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB ?= 6
USING_PROFILING ?= 0
TIMESTAMP ?= default

BUILD = release

# XXX: Remove platform specific definitions
ifeq ($(CONFIG_ARCH_TCC),y)
  ifeq ($(CONFIG_ARCH_TCC892X),y)
  TARGET_PLATFORM = tcc-m400
  else
  TARGET_PLATFORM ?= tcc-m200
  endif
  CONFIG ?= $(TARGET_PLATFORM)
endif

# Check if a Mali Core sub module should be enabled, true or false returned
ifneq ($(CONFIG),)
submodule_enabled = $(shell gcc $(DEFINES) -E $(1)/arch-$(CONFIG)/config.h | grep type | grep -c $(2))
endif

# This conditional makefile exports the global definition ARM_INTERNAL_BUILD. Customer releases will not include arm_internal.mak
-include ../../../arm_internal.mak

# Set up our defines, which will be passed to gcc
DEFINES += -DUSING_OS_MEMORY=$(USING_OS_MEMORY)
DEFINES += -DUSING_MMU=$(USING_MMU)
DEFINES += -DUSING_UMP=$(USING_UMP)
DEFINES += -D_MALI_OSK_SPECIFIC_INDIRECT_MMAP
DEFINES += -DMALI_TIMELINE_PROFILING_ENABLED=$(USING_PROFILING)
DEFINES += -DMALI_POWER_MGMT_TEST_SUITE=$(USING_MALI_PMM_TESTSUITE)
DEFINES += -DMALI_PMM_RUNTIME_JOB_CONTROL_ON=$(USING_MALI_RUN_TIME_PM)
ifeq ($(shell test $(SUBLEVEL) -gt 32 -a $(PATCHLEVEL) = 6 -a $(VERSION) = 2 -o $(VERSION) -gt 2 && echo "OK"),OK)
# MALI_STATE_TRACKING is only supported on Linux kernels from version 2.6.32.
DEFINES += -DMALI_STATE_TRACKING=1
else
DEFINES += -DMALI_STATE_TRACKING=0
endif
DEFINES += -DMALI_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB=$(OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB)
ifneq ($(call submodule_enabled, $(src), PMU),0)
        MALI_PLATFORM_FILE = platform/$(TARGET_PLATFORM)/mali_platform.c
else
        MALI_PLATFORM_FILE = platform/default/mali_platform.c
endif

DEFINES += -DUSING_MALI_PMM=$(USING_PMM)
DEFINES += -DMALI_GPU_UTILIZATION=$(USING_GPU_UTILIZATION)

DEFINES += -DUSING_MALI_PMU=$(USING_MALI_PMU)

ifeq ($(BUILD), debug)
DEFINES += -DDEBUG
endif
DEFINES += -DSVN_REV=$(SVN_REV)
DEFINES += -DSVN_REV_STRING=\"$(SVN_REV)\"

# Linux has its own mmap cleanup handlers (see mali_kernel_mem_mmu.c)
DEFINES += -DMALI_UKK_HAS_IMPLICIT_MMAP_CLEANUP

ifeq ($(USING_UMP),1)
	DEFINES += -DMALI_USE_UNIFIED_MEMORY_PROVIDER=1
	EXTRA_CFLAGS += -I$(src)/../ump/include/ump
else
	DEFINES += -DMALI_USE_UNIFIED_MEMORY_PROVIDER=0
endif

# Use our defines when compiling
EXTRA_CFLAGS += $(DEFINES) -I$(src) -I$(src)/common -I$(src)/linux -I$(src)/platform

# For customer releases the Linux Device Drivers will be provided as ARM proprietary and GPL releases:
# The ARM proprietary product will only include the license/proprietary directory
# The GPL product will only include the license/gpl directory

ifeq ($(wildcard $(src)/linux/license/gpl/*),)
EXTRA_CFLAGS += -I$(src)/linux/license/proprietary
else
EXTRA_CFLAGS += -I$(src)/linux/license/gpl
endif
EXTRA_CFLAGS += -I$(src)/common/pmm
EXTRA_CFLAGS += -I$(src)/arch-$(CONFIG)

# Source files which always are included in a build
SRC = \
	common/mali_kernel_core.c \
	linux/mali_kernel_linux.c \
	$(OSKOS)/mali_osk_indir_mmap.c \
	common/mali_kernel_rendercore.c \
	common/mali_kernel_descriptor_mapping.c \
	common/mali_kernel_vsync.c \
	linux/mali_ukk_vsync.c \
	linux/mali_kernel_sysfs.c \
	$(MALI_PLATFORM_FILE) \
	$(OSKFILES) \
	$(UKKFILES)

ifeq ($(USING_PROFILING),1)
SRC += \
	common/mali_kernel_profiling.c \
	timestamp-$(TIMESTAMP)/mali_timestamp.c
EXTRA_CFLAGS += -I$(src)/timestamp-$(TIMESTAMP)
endif

# Selecting files to compile by parsing the config file

ifeq ($(USING_PMM),1)
SRC += \
	common/pmm/mali_pmm.c \
	common/pmm/mali_pmm_policy.c \
	common/pmm/mali_pmm_policy_alwayson.c \
	common/pmm/mali_pmm_policy_jobcontrol.c \
	common/pmm/mali_pmm_state.c \
	linux/mali_kernel_pm.c \
	linux/mali_osk_pm.c \
	linux/mali_device_pause_resume.c
endif

ifeq ($(USING_GPU_UTILIZATION),1)
SRC += \
	common/mali_kernel_utilization.c
endif

ifneq ($(call submodule_enabled, $(src), MALI400PP),0)
	# Mali-400 PP in use
	EXTRA_CFLAGS += -DUSING_MALI400
	SRC += common/mali_kernel_MALI200.c
endif

ifneq ($(call submodule_enabled, $(src), MALI400GP),0)
	# Mali-400 GP in use
	SRC += common/mali_kernel_GP2.c
endif

ifneq ($(call submodule_enabled, $(src), MALI300PP),0)
	# Mali-400 PP in use
	EXTRA_CFLAGS += -DUSING_MALI400
	SRC += common/mali_kernel_MALI200.c
endif

ifneq ($(call submodule_enabled, $(src), MALI300GP),0)
	# Mali-400 GP in use
	SRC += common/mali_kernel_GP2.c
endif

ifneq ($(call submodule_enabled, $(src), MALI200),0)
	# Mali200 in use
	EXTRA_CFLAGS += -DUSING_MALI200
	SRC += common/mali_kernel_MALI200.c
endif

ifneq ($(call submodule_enabled, $(src), MALIGP2),0)
	# MaliGP2 in use
	SRC += common/mali_kernel_GP2.c
endif

ifneq ($(call submodule_enabled, $(src), MMU),0)
	# Mali MMU in use
	SRC += common/mali_kernel_mem_mmu.c common/mali_kernel_memory_engine.c common/mali_block_allocator.c common/mali_kernel_mem_os.c
else
	# No Mali MMU in use
	SRC += common/mali_kernel_mem_buddy.c
endif

ifneq ($(call submodule_enabled, $(src), MALI400L2)$(),0)
	# Mali Level2 cache in use
	EXTRA_CFLAGS += -DUSING_MALI400_L2_CACHE
	SRC += common/mali_kernel_l2_cache.c
endif

ifneq ($(call submodule_enabled, $(src), MALI300L2)$(),0)
	# Mali Level2 cache in use
	EXTRA_CFLAGS += -DUSING_MALI400_L2_CACHE
	SRC += common/mali_kernel_l2_cache.c
endif

obj-$(CONFIG_MALI_GPU) += mali.o

mali-y := $(SRC:.c=.o)
