//===--- DiagnosticIntelDriverKinds.td - Intel libdriver diagnostics ------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

let Component = "Driver" in {
def err_drv_cilk_objc : Error<
  "Cilk Plus does not support Objective-C">;
def err_drv_cilk_unsupported: Error<
  "Cilk Plus not yet supported for this target">;
def warn_drv_argument_not_allowed_with : Warning<
  "invalid argument '%0' not allowed with '%1'">,
  InGroup<InvalidCommandLineArgument>;
}
