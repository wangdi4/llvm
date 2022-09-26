; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function which can invoke multiple functions with a specifier
; that identifies which parameters are forwarded to each target function.

; Test case where a parameter is forwarded to more than one function,
; and the type of the target function does not match the expected type.
%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test01alt = type { i64, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(%struct.test01* %img, %struct.test02* %s2) {
  tail call void @broker(
    void (%struct.test01*)* @test01callee,
	void (%struct.test02*, %struct.test01alt*)* @test01callee2,
    %struct.test01* %img,
    %struct.test02* %s2
  )

  ret void
}

define void @test01callee(%struct.test01* %in) {
  %use1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  ret void
}

define void @test01callee2(%struct.test02* %in, %struct.test01alt* %alt) {
  %use1 = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 1
  %use2 = getelementptr %struct.test01alt, %struct.test01alt* %alt, i64 0, i32 1
  ret void
}

declare !callback !0 void @broker(void (%struct.test01*)*, void (%struct.test02*, %struct.test01alt*)*, %struct.test01*, %struct.test02*)

!0 = !{!1, !2}
!1 = !{i64 0, i64 2, i1 false}
!2 = !{i64 1, i64 3, i64 2, i1 false}


; These should get "Mismatched argument use" because the type passed to the
; broker function does not match the type of the function the broker function
; will call.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: Mismatched argument use

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01alt = type { i64, i32, i64, i32 }
; CHECK: Safety data: Mismatched argument use

; This structure is passed with the expected type, and should not get a safety
; flag.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found
