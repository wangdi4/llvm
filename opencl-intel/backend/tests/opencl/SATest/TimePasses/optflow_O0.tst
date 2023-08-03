; RUN: SATest -BUILD -dump-time-passes=- -config=%s.cfg 2>&1 | FileCheck %s
; CHECK-NOT: Intel OpenCL Vectorizer
; CHECK-NOT: Dead Code Elimination
; CHECK-NOT: Simplify the CFG
; CHECK-NOT: WGLoopBoundariesLegacy
