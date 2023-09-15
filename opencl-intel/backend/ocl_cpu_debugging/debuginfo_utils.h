// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#ifndef DEBUGINFO_UTILS_H
#define DEBUGINFO_UTILS_H

#include "llvm/IR/DebugInfo.h"
#include <string>

namespace debugservermessages {
class VarTypeDescriptor;
}

namespace llvm {
class DIType;
}

std::string DescribeVarType(const llvm::DIType *di_type);
std::string DescribeVarValue(const llvm::DIType *di_type, void *addr,
                             std::string type_name = "");

// Translate the internal llvm::DIType into a VarTypeDescriptor suitable for
// transmission to the client.
//
debugservermessages::VarTypeDescriptor
GenerateVarTypeDescriptor(const llvm::DIType &di_type);

#endif // DEBUGINFO_UTILS_H
