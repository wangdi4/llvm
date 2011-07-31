#############################################################################
#              INTEL CORPORATION PROPRIETARY INFORMATION
#    This software is supplied under the terms of a license agreement or 
#    nondisclosure agreement with Intel Corporation and may not be copied or 
#    disclosed except in accordance with the terms of that agreement.  
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#############################################################################

#AdirD - make file for gcc COI version 304
MIC_DIR ?= /opt/intel/mic
COI_DIR ?= ../COI_SDK
COMMONINC = -I $(COI_DIR)/include
MIC_FLAGS = $(COMMONINC) -lcoi_device -march=lrb -Wa,-mtune=l1om -rdynamic -Wl,--enable-new-dtags -Wl,-rpath=/lib64:/lib
HOST_FLAGS = $(COMMONINC) -lcoi_host -D__STDC_LIMIT_MACROS
DEBUG_FLAGS = -g -O0 -D_DEBUG
RELEASE_FLAGS = -DNDEBUG -O3
DEBUG_DIR = debug
RELEASE_DIR = release

HOST_CC ?= gcc
MIC_CC ?= $(MIC_DIR)/lrb/bin/gcc

all: $(DEBUG_DIR)/mic_server $(RELEASE_DIR)/mic_server

$(DEBUG_DIR)/mic_server: mic_native_main.cpp
	mkdir -p $(DEBUG_DIR)
	$(MIC_CC) -L$(COI_DIR)/device-linux-debug/lib $(MIC_FLAGS) $(DEBUG_FLAGS) -o $@ $<

$(RELEASE_DIR)/mic_server: mic_native_main.cpp
	mkdir -p $(RELEASE_DIR)
	$(MIC_CC) -L$(COI_DIR)/device-linux-release/lib $(MIC_FLAGS) $(RELEASE_FLAGS) -o $@ $<

clean:
	rm -rf debug
	rm -rf release
