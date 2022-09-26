; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"

; This test checks that FieldAddressTakenCall is NOT a pointer
; carried safety condition when -dtrans-outofboundsok is false.

%struct.test01.a = type { i32, i32, %struct.test01.b* }
%struct.test01.b = type { i64 }
declare void @test01ext(i32 *)
define void @test01(%struct.test01.a* %in) {
  %fa = getelementptr %struct.test01.a, %struct.test01.a* %in, i64 0, i32 1
  call void @test01ext(i32* %fa)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01.a = type { i32, i32, %struct.test01.b* }
; CHECK: Safety data:
; CHECK-SAME: Field address taken call
; CHECK-LABEL: LLVMType: %struct.test01.b = type { i64 }
; CHECK: Safety data:
; CHECK-NOT: Field address taken call
