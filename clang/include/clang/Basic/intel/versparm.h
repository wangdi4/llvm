//==--- versparm.h - Xmain-specific version information --------*- C++ -*---==//
//
// Copyright (C) 2019-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#define VERSTR(x)  #x
#define XSTR(x)    VERSTR(x)

// From version 2021.2.0 onwards, xmain version is numerically encoded as
// YYYYMMPP where:
// YYYY - major version or year
// MM   - minor version
// PP   - patch version
// E.g. 2021.2.0 is 20210200.
// Only version 2021.1.0 was encoded using format YYYYMP or 202110
// TODO: Auto-populate __INTEL_LLVM_COMPILER and __INTEL_CLANG_COMPILER
//       macros using build settings and remove XMAIN_VERSION_NUMBER
#if INTEL_DEPLOY_UNIFIED_LAYOUT
#define XMAIN_VERSION_NUMBER "20240000"
#else
#define XMAIN_VERSION_NUMBER "20230100"
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT

// XMAIN_VERSION_STRING is updated to the proper update string when the
// release branch is taken.  This allows us to differentiate via version
// output if the compiler is from the developent or release branch.  For
// example, the 2021.3.0 update will have "3.0" as the string.
#if INTEL_DEPLOY_UNIFIED_LAYOUT
#define XMAIN_VERSION_STRING "0.0"
#else
#define XMAIN_VERSION_STRING "x.0"
#endif // INTEL_DEPLOY_UNIFIED_LAYOUT

#ifndef BUILD_DATE_STAMP
#define BUILD_DATE_STAMP        YYYYMMDD
#endif
#define XMAIN_BUILD_DATE_STAMP_STR XSTR(BUILD_DATE_STAMP)
