/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
