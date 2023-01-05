// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

/////////
// Prefixes

#ifdef PREFIX
#define COMMA ,
PREFIX(prefix_0, {nullptr})
PREFIX(prefix_1, {"-" COMMA nullptr})
#undef COMMA
#endif // PREFIX

#ifdef OPTION

//////////
// Options

OPTION(prefix_0, "<input>", INPUT, Input, INVALID, INVALID, nullptr, 0, 0,
       nullptr, nullptr, nullptr)
OPTION(prefix_0, "<unknown>", UNKNOWN, Unknown, INVALID, INVALID, nullptr, 0, 0,
       nullptr, nullptr, nullptr)
OPTION(prefix_1, "cl-denorms-are-zero", cl_denorms_are_zero, Flag, INVALID,
       INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "cl-fast-relaxed-math", cl_fast_relaxed_math, Flag, INVALID,
       INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "cl-finite-math-only", cl_finite_math_only, Flag, INVALID,
       INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "cl-no-signed-zeroes", cl_no_signed_zeroes, Flag, INVALID,
       INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "cl-unsafe-math-optimizations", cl_unsafe_math_optimizations,
       Flag, INVALID, INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "create-library", create_library, Flag, INVALID, INVALID,
       nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_1, "enable-link-options", enable_link_options, Flag, INVALID,
       INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
#endif // OPTION
