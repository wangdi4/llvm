; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that storing a pointer to a structure type to a 'i64' field generates
; the 'Address taken' safety bit on the type being stored.

%struct.test01 = type { i64, %struct.test02* }
%struct.test02 = type { i32, i32 }
%struct.test03 = type { i64 }

define void @test(%struct.test01 *%a1, %struct.test03* "intel_dtrans_func_index"="1" %a2) !intel.dtrans.func.type !5 {
  %f1 = getelementptr %struct.test01, %struct.test01* %a1, i64 0, i32 1
  %fptr = load %struct.test02*, %struct.test02** %f1

  ; Converting a pointer to a structure type, and storing it to an 'i64' field
  ; should require in the 'Address taken' safety flag on the type being stored.
  %pti = ptrtoint %struct.test02* %fptr to i64
  %f2 = getelementptr %struct.test03, %struct.test03* %a2, i64 0, i32 0
  store i64 %pti, i64* %f2

  ret void
}

; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Address taken{{ *$}}
; CHECK: End LLVMType: %struct.test02

; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test02* }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!8 = !{!"S", %struct.test03 zeroinitializer, i32 1, !1} ; { i64 }

!intel.dtrans.types = !{!6, !7, !8}
