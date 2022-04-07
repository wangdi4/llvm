;XFAIL: i686-pc-win32
;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: python %S/../test_deploy.py ../../lib/libImathLibd.so ../../bin
;RUN: SATest -BUILD -config=%s.cfg -llvm-option=-print-before=dpcpp-kernel-equalizer 2>&1 | FileCheck %s
;CHECK: read_image{{.*}}, %opencl.sampler_t
