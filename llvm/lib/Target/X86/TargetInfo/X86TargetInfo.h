//===-- X86TargetInfo.h - X86 Target Implementation -------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_TARGETINFO_X86TARGETINFO_H
#define LLVM_LIB_TARGET_X86_TARGETINFO_X86TARGETINFO_H

namespace llvm {

class Target;

Target &getTheX86_32Target();
Target &getTheX86_64Target();
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_XUCC
Target &getTheX86_XuCCTarget();
#endif // INTEL_FEATURE_XUCC
#endif // INTEL_CUSTOMIZATION

}

#endif // LLVM_LIB_TARGET_X86_TARGETINFO_X86TARGETINFO_H
