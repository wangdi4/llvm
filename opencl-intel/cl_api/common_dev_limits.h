// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __COMMON_DEV_LIMITES_H__
#define __COMMON_DEV_LIMITES_H__

#include "llvm/Transforms/SYCLTransforms/DevLimits.h"

// List of supported devices that differs from the a given one in cl.h
enum DeviceMode { CPU_DEVICE = 0, FPGA_EMU_DEVICE = 1 };

// List of supported vectorizers
enum VectorizerType { VPO_VECTORIZER, DEFAULT_VECTORIZER };

#endif // __COMMON_DEV_LIMITES_H__
