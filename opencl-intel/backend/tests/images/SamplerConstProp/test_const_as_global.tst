;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: mkdir %T_const_as_global
;RUN: SATest -OCL -BUILD -config=%s.cfg -dump-IR-before=target_data -dump-IR-dir=%T_const_as_global
;RUN: FileCheck %s --input-file=%T_const_as_global/dump.target_data_before.ll
;RUN: rm -rf %T_const_as_global
;CHECK: read_image{{.*}}, i32 {{[0-9]+}},
