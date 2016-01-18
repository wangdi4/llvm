//==--- DiagnosticGroups.td - Diagnostic Group Definitions ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

// if INTEL_CUSTOMIZATION
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
// endif INTEL_CUSTOMIZATION

// Put here any group if you'd like to add it to "all"
def IntelSpecific : DiagGroup <"intel-specific", [IntelSpecificCilkplus]>;
