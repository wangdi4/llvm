; RUN: SATest -BUILD -dump-IR-before=vectorizer -dump-IR-dir=%T -config=%s.cfg
; RUN: FileCheck %s --input-file=%T/dump.vectorizer_before.ll
; CHECK: define void @wlSimpleBoxBlur_GPU
