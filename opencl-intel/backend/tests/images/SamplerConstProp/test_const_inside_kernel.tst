;XFAIL: win32
;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: mkdir %T_const_inside_kernel
;RUN: SATest -BUILD -config=%s.cfg -dump-IR-before=target_data -dump-IR-dir=%T_const_inside_kernel
;RUN: FileCheck %s --input-file=%T_const_inside_kernel/dump.target_data_before.ll
;RUN: rm -rf %T_const_inside_kernel
;CHECK: read_image{{.*}}, %opencl.sampler_t
