/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#ifndef DEBUGINFO_UTILS_H
#define DEBUGINFO_UTILS_H

#include <llvm/IR/DebugInfo.h>

#include <string>

namespace debugservermessages {
    class VarTypeDescriptor;
}

namespace llvm {
    class DIType;
}

std::string DescribeVarType(const llvm::DIType* di_type);
std::string DescribeVarValue(const llvm::DIType* di_type, void* addr,
                             std::string type_name = "");


// Translate the internal llvm::DIType into a VarTypeDescriptor suitable for
// transmission to the client.
//
debugservermessages::VarTypeDescriptor GenerateVarTypeDescriptor(
                                       const llvm::DIType& di_type);


#endif // DEBUGINFO_UTILS_H
