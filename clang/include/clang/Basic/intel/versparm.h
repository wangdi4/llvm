//==--- versparm.h - Xmain-specific version information --------*- C++ -*---==//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#define VERSTR(x)  #x
#define XSTR(x)    VERSTR(x)

#define XMAIN_VERSION_NUMBER   "202110"
#define XMAIN_VERSION_STRING   "7.0"
#define XMAIN_VERSION_MINOR     7
#define XMAIN_VERSION_UPDATE    0

#define DPCPP_VERSION_QUALITY   "beta08"
#define DPCPP_VERSION_MAJOR     2021
#define DPCPP_VERSION_MINOR     1
#define DPCPP_VERSION_MAJOR_STR XSTR(DPCPP_VERSION_MAJOR)
#define DPCPP_VERSION_MINOR_STR XSTR(DPCPP_VERSION_MINOR)
#define DPCPP_VERSION_STR       DPCPP_VERSION_MAJOR_STR "." \
                                DPCPP_VERSION_MINOR_STR
#define DPCPP_PRODUCT_NAME      "Intel(R) oneAPI DPC++ Compiler"

#ifndef BUILD_DATE_STAMP
#define BUILD_DATE_STAMP        YYYYMMDD
#endif
#define XMAIN_BUILD_DATE_STAMP_STR XSTR(BUILD_DATE_STAMP)
