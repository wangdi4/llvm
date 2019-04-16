;XFAIL: i686-pc-win32
;RUN: python %S/../test_deploy.py %s.cl .
;RUN: python %S/../test_deploy.py ../../lib/libOclCpuBackEnd.so ../../bin
;RUN: python %S/../test_deploy.py ../../lib/libImathLibd.so ../../bin
;RUN: mkdir -p %t.dir
;RUN: SATest -BUILD -config=%s.cfg -dump-IR-before=target_data -dump-IR-dir=%t.dir
;RUN: FileCheck %s --input-file=%t.dir/dump.target_data_before.ll
;CHECK: read_image{{.*}}, %opencl.sampler_t
