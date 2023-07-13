; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on getelementptr instructions.



; This is a special case of a GEP that uses a null value for the pointer
; operand.
%struct.listentry = type { ptr, ptr }
define internal void @test01(i64 %offset) {
  %null_offset = getelementptr %struct.listentry, ptr null, i64 %offset
  %fptr_addr = getelementptr inbounds %struct.listentry, ptr %null_offset, i64 0, i32 0
  %fptr = load ptr, ptr %fptr_addr

  %cptr_addr = getelementptr %struct.listentry, ptr %null_offset, i64 0, i32 1
  %cptr = load ptr, ptr %cptr_addr
  call void %fptr(ptr %cptr), !intel_dtrans_type !1

 ret void
}
; CHECK-LABEL: void @test01
; CHECK:  %null_offset = getelementptr %struct.listentry, ptr null, i64 %offset
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:        %struct.listentry*{{ *$}}
; CHECK-NEXT:      No element pointees.


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (i8*)
!2 = !{!"void", i32 0}  ; void
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{!1, i32 1}  ; void (i8*)*
!5 = !{!"S", %struct.listentry zeroinitializer, i32 2, !4, !3} ; { void (i8*)*, i8* }

!intel.dtrans.types = !{!5}
