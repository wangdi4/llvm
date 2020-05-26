; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on getelementptr instructions.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; This is a special case of a GEP that uses a null value for the pointer
; operand.
%struct.listentry = type { void (i8*)*, i8* }
define internal void @test01(i64 %offset) {
  %null_offset = getelementptr %struct.listentry, %struct.listentry* null, i64 %offset
  %fptr_addr = getelementptr inbounds %struct.listentry, %struct.listentry* %null_offset, i64 0, i32 0
  %fptr = load void (i8*)*, void (i8*)** %fptr_addr

  %cptr_addr = getelementptr %struct.listentry, %struct.listentry* %null_offset, i64 0, i32 1
  %cptr = load i8*, i8** %cptr_addr
  call void %fptr(i8* %cptr), !dtrans_type !1

 ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR:  %null_offset = getelementptr %struct.listentry, %struct.listentry* null, i64 %offset
; CHECK-FUT:  %null_offset = getelementptr %struct.listentry, p0 null, i64 %offset
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:        %struct.listentry*{{ *$}}
; CHECK-NEXT:      No element pointees.


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (i8*)
!2 = !{!"void", i32 0}  ; void
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{i8 0, i32 0}  ; i8
!5 = !{!1, i32 1}  ; void (i8*)*
!6 = !{!"S", %struct.listentry zeroinitializer, i32 2, !5, !3} ; { void (i8*)*, i8* }

!dtrans_types = !{!6}
