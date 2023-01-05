//==--- common_utils.h - Common utility functions for unittests -*- C++ -*--==//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__
#include <cstdio>
#include <string>
#include <vector>

bool GetEnv(std::string &result, const std::string &name);

std::string get_exe_dir(unsigned int pid = 0);

void readBinary(std::string filename, std::vector<unsigned char> &binary);

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#define SETENV(NAME, VALUE) (SetEnvironmentVariableA(NAME, VALUE) != 0)
#define UNSETENV(NAME) (SetEnvironmentVariableA(NAME, NULL) != 0)
#else
#include <cstdlib>
#define SETENV(NAME, VALUE) (setenv(NAME, VALUE, 1) == 0)
#define UNSETENV(NAME) (unsetenv(NAME) == 0)
#endif

#endif /*__COMMON_UTILS_H__*/
