; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test access of struct.test.a element zero via a bitcast to i8**.

; This should identify test.struct.b element zero as having been read.

%struct.test.a = type { %struct.test.b*, i32}
%struct.test.b = type { i32, i32, i32, i32 }

define void @test(%struct.test.a* %p) {
  %tmp = bitcast %struct.test.a* %p to i8**
  %pb8 = load i8*, i8** %tmp
  %pb = bitcast i8* %pb8 to %struct.test.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test.a = type { %struct.test.b*, i32 }
; CHECK: Field info: Read
; CHECK: Field info:
; CHECK: Safety data: No issues found

; CHECK-LABEL: LLVMType: %struct.test.b = type { i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
