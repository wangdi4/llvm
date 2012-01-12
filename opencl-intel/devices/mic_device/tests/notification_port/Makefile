#############################################################################
#              INTEL CORPORATION PROPRIETARY INFORMATION
#    This software is supplied under the terms of a license agreement or 
#    nondisclosure agreement with Intel Corporation and may not be copied or 
#    disclosed except in accordance with the terms of that agreement.  
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#############################################################################

MIC_DIR ?= /opt/intel/mic
COI_DIR ?= $(MIC_DIR)/coi
COMMONINC = -I $(COI_DIR)/include -I../../../../utils/cl_sys_utils/export -I../../../../cl_api -I../../../../externals/tbb/include
MIC_FLAGS = $(COMMONINC) -lcoi_device -Wa,-mtune=l1om -rdynamic -Wl,--enable-new-dtags -Wl,-rpath=/lib64:/lib
HOST_FLAGS = $(COMMONINC) -lcoi_host
DEBUG_FLAGS = -g -O0 -D_DEBUG
RELEASE_FLAGS = -DNDEBUG -O3
DEBUG_DIR = debug
RELEASE_DIR = release

HOST_CC ?= /opt/intel/Compiler/11.1/137/bin/icc
MIC_CC ?= /opt/intel/mic/Compiler/bin/icc

all: $(DEBUG_DIR)/notification_port_test_source_host $(DEBUG_DIR)/notification_port_test_sink_mic \
     $(RELEASE_DIR)/notification_port_test_source_host $(RELEASE_DIR)/notification_port_test_sink_mic
	 
release: $(RELEASE_DIR)/notification_port_test_source_host $(RELEASE_DIR)/notification_port_test_sink_mic	 

debug: $(DEBUG_DIR)/notification_port_test_source_host $(DEBUG_DIR)/notification_port_test_sink_mic

$(DEBUG_DIR)/notification_port_test_source_host: notification_port_test_source.cpp ../../notification_port.cpp cl_synch_objects_linux.cpp
	mkdir -p $(DEBUG_DIR)
	$(HOST_CC) -L$(COI_DIR)/host-linux-debug/lib $(HOST_FLAGS) $(DEBUG_FLAGS) -o $@ $< ../../notification_port.cpp cl_synch_objects_linux.cpp

$(RELEASE_DIR)/notification_port_test_source_host: notification_port_test_source.cpp ../../notification_port.cpp cl_synch_objects_linux.cpp
	mkdir -p $(RELEASE_DIR)
	$(HOST_CC) -L$(COI_DIR)/host-linux-release/lib $(HOST_FLAGS) $(RELEASE_FLAGS) -o $@ $< ../../notification_port.cpp cl_synch_objects_linux.cpp

$(DEBUG_DIR)/notification_port_test_sink_mic: notification_port_test_sink.cpp
	mkdir -p $(DEBUG_DIR)
	$(MIC_CC) -L$(COI_DIR)/device-linux-debug/lib $(MIC_FLAGS) $(DEBUG_FLAGS) -o $@ $<

$(RELEASE_DIR)/notification_port_test_sink_mic: notification_port_test_sink.cpp
	mkdir -p $(RELEASE_DIR)
	$(MIC_CC) -L$(COI_DIR)/device-linux-release/lib $(MIC_FLAGS) $(RELEASE_FLAGS) -o $@ $<

clean:
	rm -rf debug
	rm -rf release
