//===------ LoopOptReportBuilder.h ----------------------------*- C++ -*---===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines LoopOptReportBuilder class. That is the main interface of
// loop optimization reports for transformations.
//
// For more details on how to use loop transformation reports, please look at:
//     docs/Intel/OptReport.rst
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
#define LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H

namespace llvm {

/// The verbosity level of loop optimization reports.
/// For detailed description, please look at:
///     docs/Intel/OptReport.rst
namespace OptReportVerbosity {
enum Level { None = 0, Low = 1, Medium = 2, High = 3 };
}

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_MDOPTREPORTBUILDER_H
