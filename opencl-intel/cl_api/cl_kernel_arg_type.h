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

#ifndef __CL_KERNEL_ARG_TYPE_H__
#define __CL_KERNEL_ARG_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \enum cl_kernel_arg_type
 * Defines possible values for kernel argument types
 */
enum cl_kernel_arg_type
{
    CL_KRNL_ARG_INT     = 0,          //!< Argument is a signed integer.
    CL_KRNL_ARG_UINT,                 //!< Argument is an unsigned integer.
    CL_KRNL_ARG_FLOAT,                //!< Argument is a float.
    CL_KRNL_ARG_DOUBLE,               //!< Argument is a double.
    CL_KRNL_ARG_VECTOR,               //!< Argument is a vector of basic types, like int8, float4, etc.
    CL_KRNL_ARG_VECTOR_BY_REF,        //!< Argument is a byval pointer to a vector of basic types, like int8, float4, etc.
    CL_KRNL_ARG_SAMPLER,              //!< Argument is a sampler object
    CL_KRNL_ARG_COMPOSITE,            //!< Argument is a user defined struct
    CL_KRNL_ARG_PTR_LOCAL,            //!< Argument is a pointer to array declared in local memory
                                      //!< Memory object types bellow this line
    CL_KRNL_ARG_PTR_GLOBAL,           //!< Argument is a pointer to array in global memory of various types
                                      //!< The array type could be char, short, int, float or double
                                      //!< User must pass a handle to a memory buffer for this argument type
    CL_KRNL_ARG_PTR_CONST,            //!< Argument is a pointer to buffer declared in constant(global) memory
    CL_KRNL_ARG_PTR_IMG_2D,           //!< Argument is a pointer to 2D image
    CL_KRNL_ARG_PTR_IMG_2D_DEPTH,     //!< Argument is a pointer to 2D image depth
    CL_KRNL_ARG_PTR_IMG_3D,           //!< Argument is a pointer to 3D image
    CL_KRNL_ARG_PTR_IMG_2D_ARR,       //!< Argument is a pointer to a 2D image array
    CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH, //!< Argument is a pointer to a 2D image depth array
    CL_KRNL_ARG_PTR_IMG_1D,           //!< Argument is a pointer to 1D image
    CL_KRNL_ARG_PTR_IMG_1D_ARR,       //!< Argument is a pointer to 1D image array
    CL_KRNL_ARG_PTR_IMG_1D_BUF,       //!< Argument is a pointer to 1D image buffer
    CL_KRNL_ARG_PTR_BLOCK_LITERAL,    //!< Argument is a pointer to Block Literal structure
    CL_KRNL_ARG_PTR_QUEUE_T,          //!< Argument is a pointer to device execution queue
    CL_KRNL_ARG_PTR_SAMPLER_T,        //!< Argument is a pointer to a sampler object
    CL_KRNL_ARG_PTR_PIPE_T,           //!< Argument is a pointer to a pipe
    CL_KRNL_ARG_PTR_CLK_EVENT_T,      //!< Argument is a pointer to a device side event
};

/*! \struct cl_kernel_argument
 *  \brief Defines possible values for kernel argument types.
 */
struct cl_kernel_argument
{
    cl_kernel_arg_type      type;               //!< Type of the argument.
    unsigned int            size_in_bytes;      //!< Size of the argument in bytes
    unsigned int            access;             //!< Access type for pointers
    unsigned int            offset_in_bytes;    //!< Offset of the argument in argument buffer
};

// Sampler type as defined in OpenCL kernels
typedef unsigned int _sampler_t;

#ifdef __cplusplus
}
#endif

#endif // __CL_KERNEL_ARG_TYPE_H__
