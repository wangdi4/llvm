//==--- pretty_printers.h -                                    -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Collection of utils to print OpenCL-specific types through gtest
//
// ===--------------------------------------------------------------------=== //

#ifndef __PRETTY_PRINTERS_H__
#define __PRETTY_PRINTERS_H__

#include "CL/cl.h"
#include <string>

std::string ErrToStr(cl_int error);

#endif // __PRETTY_PRINTERS_H__
