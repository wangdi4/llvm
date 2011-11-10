; RUN: SATest -OCL -BUILD -dump-IR-before=vectorizer -dump-IR-dir=%p/test-IR -config=%s.cfg
; RUN: FileCheck %s --input-file=%p/test-IR/dump.vectorizer_before.ll
; CHECK: define void @wlSimpleBoxBlur_GPU
