; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that storing a pointer to a function type that contains pointers to
; structure type in the function signature to a 'i64' field does NOT generate
; the 'Address taken' safety bit on the structure types.

%struct.test01 = type { i64, void (i32, %struct.test02*)* }
%struct.test02 = type { i32, i32 }
%struct.test03 = type { i64 }

define void @test(%struct.test01 *%a1, %struct.test03* "intel_dtrans_func_index"="1" %a2) !intel.dtrans.func.type !8 {
  %f1 = getelementptr %struct.test01, %struct.test01* %a1, i64 0, i32 1
  %fptr = load void (i32, %struct.test02*)*, void (i32, %struct.test02*)** %f1
  store void (i32, %struct.test02*)* @foo, void (i32, %struct.test02*)** %f1
  %pti = ptrtoint void (i32, %struct.test02*)* %fptr to i64


  %f2 = getelementptr %struct.test03, %struct.test03* %a2, i64 0, i32 0
  store i64 %pti, i64* %f2

  ret void
}
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Has function ptr{{ *$}}
; CHECK: End LLVMType: %struct.test01

; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02

; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03

define void @foo(i32 %a1, %struct.test02* "intel_dtrans_func_index"="1" %a2) !intel.dtrans.func.type !9 {
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"F", i1 false, i32 2, !3, !4, !5}  ; void (i32, %struct.test02*)
!3 = !{!"void", i32 0}  ; void
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = !{!2, i32 1}  ; void (i32, %struct.test02*)*
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = distinct !{!7}
!9 = distinct !{!5}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !6} ; { i64, void (i32, %struct.test02*)* }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 1, !1} ; { i64 }

!intel.dtrans.types = !{!10, !11, !12}
