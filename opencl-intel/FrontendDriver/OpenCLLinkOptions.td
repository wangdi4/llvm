//
// Copyright (C) 2023 Intel Corporation
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

include "llvm/Option/OptParser.td"

def cl_denorms_are_zero : Flag<["-"], "cl-denorms-are-zero">;
def cl_fast_relaxed_math : Flag<["-"], "cl-fast-relaxed-math">;
def cl_finite_math_only: Flag<["-"], "cl-finite-math-only">;
def cl_no_signed_zeroes : Flag<["-"], "cl-no-signed-zeroes">;
def cl_unsafe_math_optimizations: Flag<["-"], "cl-unsafe-math-optimizations">;
def create_library : Flag<["-"], "create-library">;
def enable_link_options : Flag<["-"], "enable-link-options">;
