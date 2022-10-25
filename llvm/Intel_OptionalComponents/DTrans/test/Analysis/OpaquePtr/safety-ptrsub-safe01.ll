; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of converting a pointer to an integer for use in a subtract
; instruction that are safe.

; Subtracting two pointers of the same type yields a safe offset when the
; result is used for dividing by the structure's size.
%struct.test01 = type { i64, i64 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct1, %struct.test01* "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !3 {
  %t1 = ptrtoint %struct.test01* %pStruct1 to i64
  %t2 = ptrtoint %struct.test01* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = sdiv i64 %offset, 16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

; udiv is also safe.
%struct.test02 = type { i64, i64 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct1, %struct.test02* "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !5 {
  %t1 = ptrtoint %struct.test02* %pStruct1 to i64
  %t2 = ptrtoint %struct.test02* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = udiv i64 %offset, 16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Subtracting ptr-to-ptr types does not require division by the aggregate type
; size.
%struct.test03 = type { i64, i64 }
define void @test03(%struct.test03** "intel_dtrans_func_index"="1" %ppStruct1, %struct.test03** "intel_dtrans_func_index"="2" %ppStruct2) !intel.dtrans.func.type !7 {
  %t1 = ptrtoint %struct.test03** %ppStruct1 to i64
  %t2 = ptrtoint %struct.test03** %ppStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by pointer size, not by structure size.
  %count = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{%struct.test03 zeroinitializer, i32 2}  ; %struct.test03**
!7 = distinct !{!6, !6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!8, !9, !10}
