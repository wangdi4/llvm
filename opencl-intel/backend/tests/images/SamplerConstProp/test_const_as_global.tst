;XFAIL: i686-pc-win32
;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: SATest -BUILD -config=%s.cfg -llvm-option=-print-before=sycl-kernel-equalizer 2>&1 | FileCheck %s
;CHECK: call spir_func <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(
