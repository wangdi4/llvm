/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef _CL_KERNEL_ARG_TYPE_H__
#define _CL_KERNEL_ARG_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \def MAX_WORK_DIM
    \brief The maximum working dimension size.

    Memory Objects and Kernels should use this define to go over working dimensions.
*/
#define MAX_WORK_DIM        3
// Assuming MAX_WORK_DIM == 3
#define CPU_MAX_WI_DIM_POW_OF_2	(MAX_WORK_DIM+1)

#define CPU_DEV_MAXIMUM_ALIGN			128
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)		( ((X)+CPU_DEV_MAXIMUM_ALIGN-1) & (~(CPU_DEV_MAXIMUM_ALIGN-1)))

/*! \enum cl_kernel_arg_type
 * Defines possible values for kernel argument types
 */
enum cl_kernel_arg_type
{
    CL_KRNL_ARG_INT     = 0,    //!< Argument is a signed integer.
    CL_KRNL_ARG_UINT,           //!< Argument is an unsigned integer.
    CL_KRNL_ARG_FLOAT,          //!< Argument is a float.
    CL_KRNL_ARG_DOUBLE,         //!< Argument is a double.
    CL_KRNL_ARG_VECTOR,         //!< Argument is a vector of basic types, like int8, float4, etc.
    CL_KRNL_ARG_SAMPLER,        //!< Argument is a sampler object
    CL_KRNL_ARG_COMPOSITE,      //!< Argument is a user defined struct
    CL_KRNL_ARG_PTR_LOCAL,      //!< Argument is a pointer to array declared in local memory
                                //!< Memory object types bellow this line
    CL_KRNL_ARG_PTR_GLOBAL,     //!< Argument is a pointer to array in global memory of various types
                                //!< The array type could be char, short, int, float or double
                                //!< User must pass a handle to a memory buffer for this argument type
    CL_KRNL_ARG_PTR_CONST,      //!< Argument is a pointer to buffer declared in constant(global) memory
    CL_KRNL_ARG_PTR_IMG_2D,     //!< Argument is a pointer to 2D image
    CL_KRNL_ARG_PTR_IMG_3D,     //!< Argument is a pointer to 3D image
    CL_KRNL_ARG_PTR_IMG_2D_ARR, //!< Argument is a pointer to a 2D image array
    CL_KRNL_ARG_PTR_IMG_1D,     //!< Argument is a pointer to 1D image
    CL_KRNL_ARG_PTR_IMG_1D_ARR, //!< Argument is a pointer to 1D image array
    CL_KRNL_ARG_PTR_IMG_1D_BUF  //!< Argument is a pointer to 1D image buffer
};

/*! \struct cl_kernel_argument
 *  \brief Defines possible values for kernel argument types.
 */
struct cl_kernel_argument
{
    cl_kernel_arg_type      type;               //!< Type of the argument.
    unsigned int            size_in_bytes;      //!< Size of the argument in bytes
                                                //!< for pointer types the size is 0
};

#ifdef __cplusplus
}
#endif

#endif // _CL_KERNEL_ARG_TYPE_H__