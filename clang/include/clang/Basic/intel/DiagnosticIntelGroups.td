//===--- DiagnosticIntelGroups.td - Intel Diagnostic Group Definitions ----===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

//if INTEL_SPECIFIC_CILKPLUS
// Cilk Plus warnings.
def SourceUsesCilkPlus : DiagGroup<"source-uses-cilk-plus">;
def CilkPlusLoopControlVarModification : DiagGroup<"cilk-loop-control-var-modification">;
def CilkPlusCEAN : DiagGroup<"extended-array-notation">;
def IntelSpecificCilkplus : DiagGroup <"intel-specific-cilkplus", [
    CilkPlusLoopControlVarModification,
    CilkPlusCEAN
  ]>;
//endif  // INTEL_SPECIFIC_CILKPLUS

// Intel pragma warnings.
def IntelPragma : DiagGroup<"intel-pragma">;
// CQ#364717
// A warning group for warnings about Intel compatibility extensions.
def IntelCompat : DiagGroup<"intel-compat">;

// A warning group for warnings about Intel MS compatibility extensions that are
// not planned for clang/llvm trunk. CQ#367312.
def IntelMSCompat : DiagGroup<"intel-ms-compat">;

// Warning group for err_access_dtor_vbase error that we converted to a warning.
// CQ#376362.
def PrivateVBaseDtor : DiagGroup<"private-vbase-dtor">;

// Put here any group if you'd like to add it to "all"
def IntelSpecific : DiagGroup <"intel-specific", [IntelSpecificCilkplus]>;
