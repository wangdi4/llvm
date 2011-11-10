; RUN: SATest -OCL -BUILD -dump-IR-after=all -dump-IR-dir=%p/test-IR -config=%s.cfg
; RUN: FileCheck %s --input-file=%p/test-IR/dump.vectorizer_after.ll
; CHECK: define void @__Vectorized_.intel_median
