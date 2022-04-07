==========================
Intel® CPU signature macro
==========================

Overview
--------

The goal of this macro is to allow programmers to fine-tune their code
using knowledge of specific details of CPU device.

New OpenCL C predefined macro __INTEL_OPENCL_CPU_<CPUSIGN>, where <CPUSIGN>
is the CPU signature of a device.  Used to fine-tune the kernel for specific
CPU device microarchitecture.

New macro can take one of the following values:

=============================== ========================================
            Macro                    Intel Microarchitectures
=============================== ========================================
__INTEL_OPENCL_CPU_SKL__          Skylake microarchitecture
__INTEL_OPENCL_CPU_SKX__          SkylakeX microarchitecture
__INTEL_OPENCL_CPU_BDW__          Broadwell microarchitecture
__INTEL_OPENCL_CPU_BDW_XEON__     Broadwell Xeon microarchitecture
__INTEL_OPENCL_CPU_HSW__          Haswell microarchitecture
__INTEL_OPENCL_CPU_HSW_XEON__     Haswell Xeon microarchitecture
__INTEL_OPENCL_CPU_IVB__          Ivy Bridge microarchitecture
__INTEL_OPENCL_CPU_IVB_XEON__     Ivy Bridge Xeon microarchitecture
__INTEL_OPENCL_CPU_SNB__          Sandy Bridge microarchitecture
__INTEL_OPENCL_CPU_SNB_XEON__     Sandy Bridge Xeon microarchitecture
__INTEL_OPENCL_CPU_WST__          Westmere microarchitecture
__INTEL_OPENCL_CPU_WST_XEON__     Westmere Xeon microarchitecture
__INTEL_OPENCL_CPU_UNKNOWN__      Unknown microarchitecture
=============================== ========================================


Examples
--------


Combining with cl_intel_vec_len_hint extension
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    #if defined(cl_intel_vec_len_hint)
    #pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
    #endif //cl_intel_vec_len_hint

    #ifdef __INTEL_OPENCL_CPU_BDW_XEON__
    #if defined(cl_intel_vec_len_hint)
        __attribute__((intel_vec_len_hint(8)))
    #endif //cl_intel_vec_len_hint
    #endif // BDW
    __kernel void kernel1(...)
    {
        ...
    }


Device specific code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    __kernel void kernel2(...) {
    #ifdef __INTEL_OPENCL_CPU_SNB_XEON__
        // Sandy Bridge specific code
    #elseif
        // Generic code
    #endif
    }

