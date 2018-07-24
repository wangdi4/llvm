//==--- cl_sys_info.cpp - Table of directives and clauses -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //


#include "cl_sys_info.h"
#include <buildversion.h>

////////////////////////////////////////////////////////////////////////////////
// return the product version:
// Arguments - major, minor, revision, build - output version numbers
////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::GetModuleProductVersion(int* major, int* minor,
                                                   int* revision, int* build)
{
    *major    = PRODUCTVERSION_MAJOR;
    *minor    = PRODUCTVERSION_MINOR;
    *revision = BUILDVERSION16BIT_MAJOR;
    *build    = BUILDVERSION16BIT_MINOR;
}
