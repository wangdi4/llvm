// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef VALIDATION_ERROR_CODE_H
#define VALIDATION_ERROR_CODE_H

namespace Validation {
/*! \enum VALIDATION_ERROR_CODE
 * Defines device return values that are used by the OCL framework.
 */
enum VALIDATION_ERROR_CODE {
  VALIDATION_SUCCESS = 0,             //!< Function call or query call succeeded
  VALIDATION_ERROR_FAIL = 0x80000001, //!< Internal unspecified error
  VALIDATION_INVALID_OPERATION_MODE   //!< Invalid cpu/mic architecture was set.
  // VALIDATION_INVALID_VALUE,        //!< Invalid value was passed to the
  //                                       function.
  // VALIDATION_INVALID_PROPERTIES,   //!< Properties might be valid but not
  //                                       supported
  // VALIDATION_OUT_OF_MEMORY,        //!< Resource allocation failure
  // VALIDATION_INVALID_COMMAND_LIST, //!< Invalid command list handle
  // VALIDATION_INVALID_COMMAND_TYPE, //!< Invalid command type
  // VALIDATION_INVALID_COMMAND_PARAM,//!< Invalid command parameter
  // VALIDATION_INVALID_MEM_OBJECT,   //!< Invalid memory object
  // VALIDATION_INVALID_KERNEL,       //!< Invalid kernel identifier
  // VALIDATION_INVALID_OPERATION,    //!< Device cannot perform requested
  //                                       operation
  // VALIDATION_INVALID_WRK_DIM,      //!< Invalid work dimension (i.e. a value
  //                                       between 1 and 3)
  // VALIDATION_INVALID_WG_SIZE,      //!< Invalid work-group size
  // VALIDATION_INVALID_GLB_OFFSET,   //!< Invalid global offset, only (0, 0, 0)
  //                                       is supported
  // VALIDATION_INVALID_WRK_ITEM_SIZE,//!< Invalid work-item size
  // VALIDATION_INVALID_IMG_FORMAT,   //!< Invalid image format
  // VALIDATION_INVALID_IMG_SIZE,     //!< Width or height of the image are 0 or
  //                                       exceed maximum possible values
  // VALIDATION_OBJECT_ALLOC_FAIL,    //!< Failed to allocate resources for
  //                                       memory object
  // VALIDATION_INVALID_BINARY,       //!< The binary is not supported by the
  //                                       device or program container content
  //                                       is invalid
  // VALIDATION_INVALID_PROGRAM,      //!< Invalid program object handle
  // VALIDATION_BUILD_IN_PROGRESS,    //!< Back-end compiler is still in
  //                                       operation
  // VALIDATION_BUILD_ALREADY_COMPLETE,//!< Back-end compiler previously
  //                                        compiled this program
  // VALIDATION_BUILD_ERROR,          //!< Error occurred during back-end build
  //                                       process
  // VALIDATION_INVALID_KERNEL_NAME,  //!< Kernel name is not found in the
  //                                       program
  // VALIDATION_OBJECT_ALREADY_LOCKED,//!< Memory object is already locked
  // VALIDATION_NOT_SUPPORTED         //!< The operation is not supported by the
  //                                       device
};

} // namespace Validation

#endif // VALIDATION_ERROR_CODE_H
