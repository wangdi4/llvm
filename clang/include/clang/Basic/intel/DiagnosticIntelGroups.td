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

// Intel pragma warnings.
def IntelPragma : DiagGroup<"intel-pragma">;
// CQ#364717
// A warning group for warnings about Intel compatibility extensions.
def IntelCompat : DiagGroup<"intel-compat">;

// A warning group for warnings if '#pragma ivdep' is used for FPGA emulator.
def IvdepFPGAUsage : DiagGroup<"ivdep-usage">;

// Warning group for err_access_dtor_vbase error that we converted to a warning.
// CQ#376362.
def PrivateVBaseDtor : DiagGroup<"private-vbase-dtor">;

// Error/warning messages about inappropriate usage of OpenCL FPGA Channels
def FPGAChannelsUsage : DiagGroup<"analyze-channels-usage">;

// Openmp: a warning group for unsupported simd clauses.
def SimdUnsupported : DiagGroup<"simd-unsupported">;
