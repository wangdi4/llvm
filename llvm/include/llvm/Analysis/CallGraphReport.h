//===- CallGraphReport.h Implement call graph report ---------*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file describes a class CallGraphReport which contains pure virtual
// functions that should be redefined for each subclass.  These functions
// indicate how the reports in each subclass should be modified when a
// major transformation to the call graph occurs.
//
//===----------------------------------------------------------------------===//

#ifdef INTEL_CUSTOMIZATION

#ifndef LLVM_ANALYSIS_CALLGRAPHREPORT_H
#define LLVM_ANALYSIS_CALLGRAPHREPORT_H

namespace llvm {

class Function;

// \brief Class describing a Report that describes major transformations
// on the call graph.
class CallGraphReport {
public:
  virtual ~CallGraphReport() {};
  virtual void replaceFunctionWithFunction(Function *OldFunction,
                                           Function *NewFunction) = 0;
};

} // namespace llvm

#endif

#endif // INTEL_CUSTOMIZATION
