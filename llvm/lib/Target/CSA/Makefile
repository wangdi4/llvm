##===- lib/Target/LPU/Makefile -----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMLPUCodeGen
TARGET = LPU

BUILT_SOURCES = LPUGenRegisterInfo.inc \
                LPUGenInstrInfo.inc \
                LPUGenAsmWriter.inc \
                LPUGenDAGISel.inc \
                LPUGenCallingConv.inc \
                LPUGenSubtargetInfo.inc

DIRS = InstPrinter TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common
