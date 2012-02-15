;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: SATest -OCL -BUILD -config=%s.cfg
;RUN: llvm-dis -o %t2 test_const_inside_kernel.tst.llvm_ir
;RUN: FileCheck %s --input-file=%t2
;CHECK: read_image{{.*}}, i32 {{[0-9]+}},
