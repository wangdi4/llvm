// Copyright (C) 2013 Intel Corporation
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

// vim:ts=2:sw=2:et:
// NOTE
// Purporse of the file GENERIC_common.td is to share code between CPU and MIC
// during CPU built ins transition to tblgen

include "GENERIC/conversion_not_shared.td"
include "GENERIC/conversion_sat_not_shared.td"
include "GENERIC/integer_not_shared.td"
include "GENERIC/math_not_shared.td"
include "GENERIC/misc.td"
include "GENERIC/common_not_shared.td"
include "GENERIC/relational_not_shared.td"
include "GENERIC/soa_relational.td"
include "GENERIC/group_builtin.td"
include "GENERIC/sub_group_builtin_fake_impl.td"
