#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# UGFX_COMPONENT_PATH ?= $(abspath $(shell pwd)/../)

UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components
UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components/general
UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components/spi_devices
UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components/i2c_devices
UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components/i2c_devices/sensor
UGFX_COMPONENT_DIRS += $(UGFX_COMPONENT_PATH)/components/i2c_devices/others

EXTRA_COMPONENT_DIRS += $(UGFX_COMPONENT_DIRS)
