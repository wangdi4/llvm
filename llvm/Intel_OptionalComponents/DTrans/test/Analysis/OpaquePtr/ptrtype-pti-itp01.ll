; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for "ptrtoint" and "inttoptr" instructions


; Simple test of type propagation tracking through "ptrtoint" and "inttoptr"
; instructions.
%struct.test01 = type { i32, i32 }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  %i = ptrtoint ptr %in to i64
  %p = inttoptr i64 %i to ptr
  ret void
}
; CHECK-LABEL: void @test01
; CHECK:  %i = ptrtoint ptr %in to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK:  %p = inttoptr i64 %i to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test of type propagation for a case that safety analysis will later
; need to detect as unsafe.
%struct.test02 = type { i32, i32 }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ; The Conversion results in truncation of value. For the purpose of type
  ; recovery, we will track it as the original pointer type, and leave it
  ; to the safety analysis to mark it as unsafe.
  %i = ptrtoint ptr %in to i8
  %p = inttoptr i8 %i to ptr
  ret void
}
; CHECK-LABEL: void @test02
; CHECK:  %i = ptrtoint ptr %in to i8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK:  %p = inttoptr i8 %i to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.


%struct.test03 = type { i32, i32 }
define internal void @test03(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %i = ptrtoint ptr %in to i64
  %r8 = ashr i64 %i, 8
  %l8 = shl i64 %r8, 8
  %a8 = add i64 %l8, 8

  %p = inttoptr i64 %a8 to ptr
  %gep = getelementptr %struct.test03, ptr %p, i64 0, i32 1
  ret void
}
; CHECK-LABEL: void @test03
; CHECK:  %i = ptrtoint ptr %in to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK:  %p = inttoptr i64 %a8 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03*{{ *$}}
; CHEKC-NEXT: No Element pointees


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32}
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32}
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32}

!intel.dtrans.types = !{!8, !9, !10}
