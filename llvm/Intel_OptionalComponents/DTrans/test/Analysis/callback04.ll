; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function with a specifier that identifies which parameters to
; forward to the target function.

; This is a case where the parameter type does not match the expected type, so
; should trigger a DTrans safety flag.
%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test01alt = type { i64, i32, i64, i32 }
define void @test01(%struct.test01* %img) {
  tail call void @broker(
    void (%struct.test01alt*)* @test01callee,
    %struct.test01* %img
  )
  ret void
}

define void @test01callee(%struct.test01alt* %in) {
  %use1 = getelementptr %struct.test01alt, %struct.test01alt* %in, i64 0, i32 1
  ret void
}

declare !callback !0 void @broker(void (%struct.test01alt*)*, %struct.test01*)

!0 = !{!1}
!1 = !{i64 0, i64 1, i1 false}


; These should get "Mismatched argument use" because the type passed to the
; broker function does not match the type of the function the broker function
; will call.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: Mismatched argument use

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01alt = type { i64, i32, i64, i32 }
; CHECK: Safety data: Mismatched argument use

