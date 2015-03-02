##===- clang/lib/CodeGen/Makefile --------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
#
# This implements the AST -> LLVM code generation library for the 
# C-Language front-end.
#
##===----------------------------------------------------------------------===##

CLANG_LEVEL := ../..
LIBRARYNAME := clangCodeGen

# CQ#366833 - fix for iclang makefile - based build.
ifeq ($(INTEL_CUSTOMIZATION),1)
SOURCES = $(notdir $(wildcard $(PROJ_SRC_DIR)/*.cpp \
	      $(PROJ_SRC_DIR)/*.cc $(PROJ_SRC_DIR)/*.c)) \
	  intel/CGCilkPlusRuntime.cpp \
	  intel/CGElementalFunction.cpp
endif

include $(CLANG_LEVEL)/Makefile

