; RUN: SATest -BUILD -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2
; CHECK-NOT: Intel OpenCL Vectorizer
; CHECK-NOT: Dead Code Elimination
; CHECK-NOT: Simplify the CFG
; CHECK-NOT: WGLoopBoundariesLegacy
