; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test case checks that a dead Bitcast instruction doesn't produce
; "Bad casting".

; CHECK-LABEL: %struct.test = type { i32, i32 }
; CHECK: Safety data: No issues found

%struct.test = type { i32, i32 }
define i32 @foo(%struct.test* %arg) {
  %tmp = getelementptr %struct.test, %struct.test* %arg, i64 0, i32 0
  %tmp2 = load i32, i32* %tmp
  %tmp3 = bitcast %struct.test* %arg to i64*
  ret i32 %tmp2
}