; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function with a specifier that identifies which parameters to
; forward to the target function, and does not forward all parameters

; This is a safe case where the parameter types match the expected types.
%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(%struct.test01* %img, %struct.test02* %s2) {
  tail call void @broker(
    void (%struct.test01*)* @test01callee,
    %struct.test01* %img,
    %struct.test02* %s2
  )

  ret void
}

define void @test01callee(%struct.test01* %in) {
  %use1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  ret void
}

declare !callback !0 void @broker(void (%struct.test01*)*, %struct.test01*, %struct.test02*)

!0 = !{!1}
!1 = !{i64 0, i64 1, i1 false}


; This structure should not get marked "Address taken" because it is passed to
; the external broker function to be forwarded to the callback function, and the
; broker function is not permitted to inspect or modify those parameters.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: No issues found

; This structure is not forwarded by the broker function, so should be marked as
; "Address taken".

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Address taken
