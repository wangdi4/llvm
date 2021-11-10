; RUN: SATest -BUILD -build-iterations=1 -dump-kernel-property -serialize-work-groups -config=%s.cfg 2>&1 | FileCheck %s

; This test checks that kernel property needSerializeWGs is false when runtime
; config serialize-work-groups is specified.

; CHECK-DAG: Test program was successfully built.
; CHECK-DAG: needSerializeWGs: 0
