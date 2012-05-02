;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: SATest -OCL -BUILD -config=%s.cfg -dump-IR-before=target_data -dump-IR-dir=.
;RUN: FileCheck %s --input-file=dump.target_data_before.ll
;CHECK: read_image{{.*}}, i32 {{[0-9]+}},
