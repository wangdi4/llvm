=====================================
cl_khr_fp16 support for FPGA emulator
=====================================

Introduction
------------
FPGA Emulator supports cl_khr_fp16 only for a subset of
operations. Features described in this document are fully supported
and tested, everything else is not supported.

Requirements
------------

Current implementation requires `f16c` instruction set to be able to
codegen operations on a half type.

Extension
---------
* "#pragma OPENCL EXTENSION cl_khr_fp16 : enable" is supported and
  required to be enabled in order to use fp16 feature in OpenCL C
  kernel language.

* cl_khr_fp16 is *not* in the list of device supported extensions
  (clGetDeviceInfo, clGetPlatformInfo).

* clGetDeviceInfo(CL_DEVICE_HALF_FP_CONFIG) returns
  CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN.

Data types
----------
The following data types are supported:
half, half2, half3, half4, half8 and half16.

Built-in functions
------------------
* for half, half2, half3, half4, half8 and half16:

  * vload_halfn
  * vstore_halfn
  * operator +
  * operator -
  * operator *

* Implicit conversions between half and float/double:

  * float/double literal -> half variable
  * float/double variable -> half variable
  * half variable -> float/double variable

OpenCL headers
--------------
Only the typedef for cl_half is added into CL/cl_platform.h.
Other typedefs (for cl_halfn) are not supported.

Windows:
  typedef unsigned __int16        cl_half;

Linux:
  typedef uint16_t  cl_half     __attribute__((aligned(2)));
