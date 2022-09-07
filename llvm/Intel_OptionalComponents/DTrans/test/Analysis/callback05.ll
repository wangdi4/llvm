; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function which can invoke multiple functions with a specifier
; that identifies which parameters are forwarded to each target function.

; This is a safe case where the parameter types match the expected types.
%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(%struct.test01* %img, %struct.test02* %s2) {
  tail call void @broker(
    void (%struct.test01*)* @test01callee,
	void (%struct.test02*)* @test01callee2,
    %struct.test01* %img,
    %struct.test02* %s2
  )

  ret void
}

define void @test01callee(%struct.test01* %in) {
  %use1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  ret void
}

define void @test01callee2(%struct.test02* %in) {
  %use1 = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 1
  ret void
}

declare !callback !0 void @broker(void (%struct.test01*)*, void (%struct.test02*)*, %struct.test01*, %struct.test02*)

!0 = !{!1, !2}
!1 = !{i64 0, i64 2, i1 false}
!2 = !{i64 1, i64 3, i1 false}


; These structures should not get marked "Address taken" because they are passed
; to the external broker function to be forwarded to the callback function.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found
