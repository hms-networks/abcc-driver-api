# Makefile for the Anybus CompactCom Driver API
#
# How to use:
#  - Create ABCC_API_DIR containing the path to the
#    Anybus CompactCom Driver API directory.
#  - Create SRCS, add your source files to it (optional),
#    and create object files from the content of SRCS.
#  - Create INCLUDES, add your include paths to it
#    (optional), and append the content as compiler flags
#    when compiling.

# store the path to the Anybus CompactCom Driver directory
ABCC_DRIVER_DIR := $(ABCC_API_DIR)/abcc-driver

# include the Anybus CompactCom Driver
include $(ABCC_DRIVER_DIR)/abcc-driver.mk

# add the Anybus CompactCom Driver API source files
SRCS += $(ABCC_API_DIR)/src/abcc_api_select_firmware.c
SRCS += $(ABCC_API_DIR)/src/abcc_api_handler.c
SRCS += $(ABCC_API_DIR)/src/abcc_api_command_handler.c
SRCS += $(ABCC_API_DIR)/src/host_objects/application_data_object.c
SRCS += $(ABCC_API_DIR)/src/anybus_objects/anybus_file_system_interface_object.c

# add the Anybus CompactCom Driver API include directories
INCLUDES += -I$(ABCC_API_DIR)/inc
INCLUDES += -I$(ABCC_API_DIR)/inc/host_objects
INCLUDES += -I$(ABCC_API_DIR)/inc/anybus_objects