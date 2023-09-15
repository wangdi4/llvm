##===- lib/Target/Intel_CSA/Makefile -----------------------*- Makefile -*-===##
#
# Copyright (C) 2017 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of Intel Corporation and may not be disclosed, examined
# or reproduced in whole or in part without explicit written authorization
# from the company.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMCSACodeGen
TARGET = CSA

BUILT_SOURCES = CSAGenRegisterInfo.inc \
                CSAGenInstrInfo.inc \
                CSAGenAsmWriter.inc \
                CSAGenDAGISel.inc \
                CSAGenCallingConv.inc \
                CSAGenSubtargetInfo.inc

DIRS = InstPrinter TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common
