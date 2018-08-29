; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test access of struct.test element zero via a GEP with an i8 element zero.

%struct.test = type { i8, i32}

define void @test(%struct.test* %p) {
  %p0 = getelementptr %struct.test, %struct.test* %p, i64 0, i32 0
  store i8 0, i8* %p0
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test = type { i8, i32 }
; CHECK: Safety data: No issues found
