; RUN: SATest -BUILD -vectorizer-type=volcano -dump-time-passes=- -config=%s.cfg | FileCheck %s
; CHECK-DAG: Intel OpenCL Vectorizer
; CHECK-DAG: Dead Code Elimination
; CHECK-DAG: Simplify the CFG
; CHECK-DAG: WGLoopBoundariesLegacy
