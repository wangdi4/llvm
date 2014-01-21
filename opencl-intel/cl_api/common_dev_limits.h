/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __COMMON_DEV_LIMITES_H__
#define __COMMON_DEV_LIMITES_H__

/*! \def MAX_WORK_DIM
    \brief The maximum working dimension size.

    Memory Objects and Kernels should use this define to go over working dimensions.
*/
#define MAX_WORK_DIM        3
// Assuming MAX_WORK_DIM == 3
#define MAX_WI_DIM_POW_OF_2	(MAX_WORK_DIM+1)

#define DEV_MAXIMUM_ALIGN			128
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)		( ((X)+DEV_MAXIMUM_ALIGN-1) & (~(DEV_MAXIMUM_ALIGN-1)))

// OpenCL 2.0 introduced the non-uniform work-group size.
// In our implementation RT calculates two arrays of local WG sizes and passes them to a kernel.
// One array for region of work-groups with sizes equal to the spefied in clEnqueueNDRangeKernel while
// another is for region with sizes eqaul to get_global_size(dim) % get_enqueued_local_size(dim).
enum {
  UNIFORM_WG_SIZE_INDEX    = 0, // Index of WG sizes returned by get_enqueued_local_size.
  NONUNIFORM_WG_SIZE_INDEX = 1, // Index of WG sizes returned by get_local_size for tail WGs.
  WG_SIZE_NUM                   // Number of WG size arrays.
};

#endif // __COMMON_DEV_LIMITES_H__
