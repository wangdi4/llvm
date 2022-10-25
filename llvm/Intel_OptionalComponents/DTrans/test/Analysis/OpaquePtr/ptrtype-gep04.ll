; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on getelementptr instructions.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
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
  call void %fptr(i8* %cptr), !intel_dtrans_type !1

 ret void
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE:  %null_offset = getelementptr %struct.listentry, %struct.listentry* null, i64 %offset
; CHECK-OPAQUE:  %null_offset = getelementptr %struct.listentry, ptr null, i64 %offset
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
