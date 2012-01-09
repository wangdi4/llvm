SOURCES:= main.c

OPENCL_INSTALL_ROOT:=../../../../install/Linux64/Debug
OPENC_CL_PATH:= ${OPENCL_INSTALL_ROOT}/include
OPENCL_BIN_PATH:=${OPENCL_INSTALL_ROOT}/bin
IDB:= /opt/intel/composerxe_mic/bin/mic/idb
GDB:= gdb

CFLAGS:= -I${OPENC_CL_PATH} -O0 -g
LDFLAGS:= -lintelocl -L${OPENCL_BIN_PATH} -Wl,-rpath-link=${OPENCL_BIN_PATH}
RUN_SETUP:= export LD_LIBRARY_PATH=${OPENCL_BIN_PATH};export SINK_LD_LIBRARY_PATH=/opt/intel/mic/coi/device-linux-debug/lib:${OPENCL_BIN_PATH}
#DEBUG_SETUP:= ${RUN_SETUP};export LD_BIND_NOW=1;export MALLOC_CHECK_=3
DEBUG_SETUP:= ${RUN_SETUP};export MALLOC_CHECK_=3

OBJECTS:=$(patsubst %.c,%.o,${SOURCES})

all : run_test
clean :
	rm -f ${OBJECTS} test

test : ${OBJECTS}
	cc -o $@ $^ ${LDFLAGS}

run_test : test
	${RUN_SETUP};./$<

gdb : test
	${DEBUG_SETUP};${GDB} ./$<

idb : test
	${DEBUG_SETUP};${IDB} -gdb ./$<

emacs : test
	${DEBUG_SETUP};emacs -f gdb

ddd : test
	${DEBUG_SETUP}; ddd ./$<

