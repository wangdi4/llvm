##===- clang/lib/Sema/Makefile -----------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
#
#  This implements the semantic analyzer and AST builder library for the 
#  C-Language front-end.
#
##===----------------------------------------------------------------------===##

CLANG_LEVEL := ../..
LIBRARYNAME := clangSema

# CQ#366833 - fix for iclang makefile - based build.
ifeq ($(INTEL_CUSTOMIZATION),1)
SOURCES = $(notdir $(wildcard $(PROJ_SRC_DIR)/*.cpp \
	      $(PROJ_SRC_DIR)/*.cc $(PROJ_SRC_DIR)/*.c)) \
	  intel/SemaCilk.cpp
endif

include $(CLANG_LEVEL)/Makefile

