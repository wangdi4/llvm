// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#pragma once

#ifdef _WIN32

    #define DECLARE_ALIAS(FUNC_NAME)
    #define SET_ALIAS(FUNC_NAME) 
    #define GET_ALIAS(FUNC_NAME) FUNC_NAME

#else

    #define DECLARE_ALIAS(FUNC_NAME) extern void local_intel_private_1234_##FUNC_NAME()
    #define SET_ALIAS(FUNC_NAME) void Intel::OpenCL::Framework::local_intel_private_1234_##FUNC_NAME() __attribute__ ((alias (#FUNC_NAME)))
    #define GET_ALIAS(FUNC_NAME) Intel::OpenCL::Framework::local_intel_private_1234_##FUNC_NAME

#endif
