; RUN: opt -O2 -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s

; Verify that loopopt marker pass runs at the very beginning of the pipeline just after module verification.

; REQUIRES: asserts

; CHECK:   FunctionPass Manager
; CHECK-NEXT:     Module Verifier
; CHECK-NEXT:     LoopOpt Marker

define void @f() "loopopt-pipeline"="full" {
  ret void
}
