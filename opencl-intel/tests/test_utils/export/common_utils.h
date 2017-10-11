//==--- common_utils.h - Common utility functions for unittests -*- C++ -*--==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include <string>
#include <cstdio>

bool GetEnv(std::string& result, const std::string& name);

std::string get_exe_dir(unsigned int pid = 0);

#if defined(_WIN32)
#include <windows.h>
#define SETENV(NAME,VALUE)      (_putenv_s(NAME,VALUE) == 0)
#define UNSETENV(NAME)          (_putenv_s(NAME,"") == 0)
#else
#include <cstdlib>
#define SETENV(NAME,VALUE)      (setenv(NAME,VALUE,1) == 0)
#define UNSETENV(NAME)          (unsetenv(NAME) == 0)
#endif
