#pragma once

#include <icd_dispatch.h>

namespace CRT_ICD_DISPATCH
{

    /// This needs to be integrated into the ICD for the Common Runtime USE
    typedef cl_uint             cl_kernel_arg_info;
    #define CL_API_SUFFIX__VERSION_1_2
    #define CL_EXT_SUFFIX__VERSION_1_2
    #define CL_KERNEL_ARG_TYPE_NAME                     0x1302


    typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetKernelArgInfo)(
        cl_kernel            kernel,
        cl_uint              arg_indx,
        cl_kernel_arg_info   param_name,
        size_t               param_value_size,
        void *               param_value,
        size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

   


    struct CrtKHRicdVendorDispatch: public KHRicdVendorDispatch
    {
       KHRpfn_clGetKernelArgInfo             clGetKernelArgInfo;
    };
}