/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __TYPE_CONVERSION_H__
#define __TYPE_CONVERSION_H__

#include "Type.h"
#include "llvm/Type.h"
#include "llvm/LLVMContext.h"

namespace intel{
  llvm::Type* reflectionToLLVM(llvm::LLVMContext&, const reflection::Type*);
}

#endif // __TYPE_CONVERSION_H__