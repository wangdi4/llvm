##===- lib/Target/CSA/Makefile -----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
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
