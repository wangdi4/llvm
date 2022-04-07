; RUN: SATest -BUILD -vectorizer-type=volcano -dump-time-passes=%t2 -config=%s.cfg
; RUN: FileCheck %s --input-file=%t2
; CHECK-DAG: Intel OpenCL Vectorizer
; CHECK-DAG: Dead Code Elimination
; CHECK-DAG: Simplify the CFG
; CHECK-DAG: WGLoopBoundariesLegacy
