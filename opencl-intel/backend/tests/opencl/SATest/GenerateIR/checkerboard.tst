; RUN: SATest -OCL -BUILD -config=%s.cfg
; RUN: clang -cc1 -x cl -S -emit-llvm-bc -target-cpu auto -I %p/../../../../cl_api -I %p/../../../../clang_headers -I %p/../../../../build/Win32/bin/Debug/fe_include -include opencl_.h -D __OPENCL_VERSION__=110 -D CL_VERSION_1_0=100 -D CL_VERSION_1_1=110 \
; RUN: -D __ENDIAN_LITTLE__=1 -D __ROUNDING_MODE__=rte -D __IMAGE_SUPPORT__=1 -D cl_khr_fp64 -target-feature +cl_khr_fp64 -D cl_khr_global_int32_base_atomics -target-feature +cl_khr_global_int32_base_atomics\
; RUN: -D cl_khr_global_int32_extended_atomics -target-feature +cl_khr_global_int32_extended_atomics -D cl_khr_local_int32_base_atomics -target-feature +cl_khr_local_int32_base_atomics -D cl_khr_local_int32_extended_atomics\
; RUN: -target-feature +cl_khr_local_int32_extended_atomics -D cl_khr_gl_sharing -target-feature +cl_khr_gl_sharing -D cl_khr_byte_addressable_store -target-feature +cl_khr_byte_addressable_store -D cl_intel_printf \
: RUN: -target-feature +cl_intel_printf -D cl_intel_overloading -target-feature +cl_intel_overloading -O0 %p/checkerboard.cl -o %t1.bin
; RUN: diff checkerboard.tst.llvm_ir %t1.bin
