//=- PassPrinters.h - Utilities to print analysis info for passes -*- C++ -*-=//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Utilities to print analysis info for various kinds of passes.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_OPT_PASSPRINTERS_H
#define LLVM_TOOLS_OPT_PASSPRINTERS_H

namespace llvm {

class CallGraphSCCPass;
class FunctionPass;
class ModulePass;
class LoopPass;
class PassInfo;
class raw_ostream;
class RegionPass;

FunctionPass *createFunctionPassPrinter(const PassInfo *PI, raw_ostream &out);

CallGraphSCCPass *createCallGraphPassPrinter(const PassInfo *PI,
                                             raw_ostream &out);

ModulePass *createModulePassPrinter(const PassInfo *PI, raw_ostream &out);

LoopPass *createLoopPassPrinter(const PassInfo *PI, raw_ostream &out);

RegionPass *createRegionPassPrinter(const PassInfo *PI, raw_ostream &out);

} // end namespace llvm

#endif // LLVM_TOOLS_OPT_PASSPRINTERS_H
