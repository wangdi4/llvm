; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test currently fails because we don't recognize the element reads.
; CMPLRS-51700
; XFAIL: *

; Test access of struct.test.b element zero via a bitcast and load of
; a pointer to pointer to struct.test.b.

; This should identify test.struct.b element zero as having been read.

%struct.test.a = type { i32, i32, %struct.test.b* }
%struct.test.b = type { i32, i32, i32, i32 }

define void @test(%struct.test.a* %p) {
  %tmp1 = getelementptr %struct.test.a, %struct.test.a* %p, i64 0, i32 2
  %tmp2 = bitcast %struct.test.b** %tmp1 to i32**
  %tmp3 = load i32*, i32** %tmp2
  %tmp4 = load i32, i32* %tmp3
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test.a = type { i32, i32, %struct.test.b* }
; CHECK: Field info:
; CHECK: Field info:
; CHECK: Field info: Read
; CHECK: Safety data: No issues found

; CHECK-LABEL: LLVMType: %struct.test.b = type { i32, i32, i32, i32 }
; CHECK: Field info: Read
; CHECK: Field info:
; CHECK: Field info:
; CHECK: Field info:
; CHECK: Safety data: No issues found
