; RUN: SATest -OCL -BUILD -dump-IR-after=all -dump-IR-dir=%T -config=%s.cfg
; RUN: FileCheck %s --input-file=%T/dump.vectorizer_after.ll
; CHECK: define void @__Vectorized_.intel_median
