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

// A collection of common codes defined via "defvar" (local variables).

include "GENERIC/code_conversion.td"
include "GENERIC/code_conversion_sat_common.td"
include "GENERIC/code_expand_promote_common.td"
include "GENERIC/code_float_common.td"
include "GENERIC/code_integer_common.td"
include "GENERIC/code_math.td"
include "GENERIC/code_relational.td"

defvar ret_zero_code = [{
  return 0;
}];

defvar ret_void_code = [{
  return;
}];

defvar ret_arg0_code = [{
  return $Arg0VarName;
}];
