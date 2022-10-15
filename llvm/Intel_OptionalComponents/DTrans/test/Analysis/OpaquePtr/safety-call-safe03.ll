; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calling a function using an alias to the function using a properly typed
; pointer to the structure. This is safe because we have the function's
; definition and the types match for the caller and callee.

%struct.test01 = type { i32, i32 }
@f01_alias = internal alias void (%struct.test01*), void (%struct.test01*)* @f01

define internal void @f01(%struct.test01* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !3 {
  %p = getelementptr %struct.test01, %struct.test01* %s, i64 0, i32 0
  store i32 0, i32* %p
  ret void
}

define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !4 {
  call void @f01_alias(%struct.test01* %s)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5}
