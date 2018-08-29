// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __CUSTOM_VERSION_MAPPING_H__
#define __CUSTOM_VERSION_MAPPING_H__

#include "FunctionDescriptor.h"
#include "utils.h"



TableRow mappings[] = {
  // {{scalar_version, width2_version, ..., width16_version, width3_version}, isScalarizable, isPacketizable}
  { {"__ocl_allOne","__ocl_allOne_v2","__ocl_allOne_v4","__ocl_allOne_v8","__ocl_allOne_v16",INVALID_ENTRY}, false, true},
  { {"__ocl_allZero","__ocl_allZero_v2","__ocl_allZero_v4","__ocl_allZero_v8","__ocl_allZero_v16",INVALID_ENTRY}, false, true},
  { {"_Z13get_global_idj", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z15get_global_sizej", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
//this file is automatically gnerated by tblgen. It is strongly recmommended to use this mechanism, not to write string hardcoded
#include "CustomMappings.gen"
};
#endif//__CUSTOM_VERSION_MAPPING_H__
