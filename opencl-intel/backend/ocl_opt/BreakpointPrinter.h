// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

//===- BreakpointPrinter.h - Breakpoint location printer ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Breakpoint location printer.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_TOOLS_OPT_BREAKPOINTPRINTER_H
#define LLVM_TOOLS_OPT_BREAKPOINTPRINTER_H

namespace llvm {

class ModulePass;
class raw_ostream;

ModulePass *createBreakpointPrinter(raw_ostream &out);
} // namespace llvm

#endif // LLVM_TOOLS_OPT_BREAKPOINTPRINTER_H
