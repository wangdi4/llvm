//===--- DiagnosticIntelCommonKinds.td - Intel common diagnostics ---------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Common Helpers
//===----------------------------------------------------------------------===//

let Component = "Common" in {

let CategoryName = "Parse Issue" in {
def err_pragma_simd_expected_for_loop : Error<
  "expected for statement following '#pragma simd'">;
def err_pragma_simd_invalid_linear_var : Error<
  "invalid linear variable">;
}

}
