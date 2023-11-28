; RUN: SATest -BUILD -build-log --config=%s.cfg 2>&1 | FileCheck %s

; CHECK-DAG: warning: VLA has been detected
; CHECK-DAG: Test program was successfully built.
