; RUN: SATest -OCL -BUILD -dump-IR-before=target_data -dump-IR-dir=%p/test-IR -config=%s.cfg
; RUN: FileCheck %s --input-file=%p/test-IR/dump.target_data_before.ll
; CHECK: define void @checkerboard2D
