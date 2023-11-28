// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __TYPE_CONVERSION_H__
#define __TYPE_CONVERSION_H__

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"

namespace intel {
llvm::Type *reflectionToLLVM(llvm::LLVMContext &,
                             const llvm::reflection::RefParamType &);
}

#endif // __TYPE_CONVERSION_H__
