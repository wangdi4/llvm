// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  cpu_logger.h
//  Implementation of Helper functions for logger
///////////////////////////////////////////////////////////

#pragma once

enum CpuELogLevel {
  CPU_LL_DEBUG = 100,
  CPU_LL_INFO = 200,
  CPU_LL_ERROR = 300,
  CPU_LL_CRITICAL = 400,
  CPU_LL_STATISTIC = 500,
  CPU_LL_OFF = 1000
};

#ifndef INTEL_PRODUCT_RELEASE
#define CpuInfoLog(CLIENT, CLIENT_ID, ...)                                     \
  if (CLIENT && CLIENT_ID)                                                     \
    CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_INFO), __FILE__,             \
                         __FUNCTION__, __LINE__, __VA_ARGS__);
#define CpuDbgLog(CLIENT, CLIENT_ID, ...)                                      \
  if (CLIENT && CLIENT_ID)                                                     \
    CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_DEBUG), __FILE__,            \
                         __FUNCTION__, __LINE__, __VA_ARGS__);
#define CpuErrLog(CLIENT, CLIENT_ID, ...)                                      \
  if (CLIENT && CLIENT_ID)                                                     \
    CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_ERROR), __FILE__,            \
                         __FUNCTION__, __LINE__, __VA_ARGS__);
#define CpuCriticLog(CLIENT, CLIENT_ID, ...)                                   \
  if (CLIENT && CLIENT_ID)                                                     \
    CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_CRITICAL), __FILE__,         \
                         __FUNCTION__, __LINE__, __VA_ARGS__);
#else
#define CpuInfoLog(CLIENT, CLIENT_ID, ...)
#define CpuDbgLog(CLIENT, CLIENT_ID, ...)
#define CpuErrLog(CLIENT, CLIENT_ID, ...)
#define CpuCriticLog(CLIENT, CLIENT_ID, ...)
#endif
