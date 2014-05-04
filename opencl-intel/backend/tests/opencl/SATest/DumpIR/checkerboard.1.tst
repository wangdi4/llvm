; RUN: SATest -BUILD -dump-IR-before=target_data -dump-IR-dir=%T -config=%s.cfg
; RUN: FileCheck %s --input-file=%T/dump.target_data_before.ll
; CHECK: define void @checkerboard2D
