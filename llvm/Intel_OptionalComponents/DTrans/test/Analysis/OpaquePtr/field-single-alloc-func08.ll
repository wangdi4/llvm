; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that an element zero access to a nested field being assigned
; something other than a nullptr or a malloc return value yields a Bottom
; Alloc Function without crashing.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %union.anon = type { i16, [8 x i8] }
; CHECK: Name: union.anon
; CHECK: Number of fields: 2
; CHECK: 0)Field LLVM Type: i16
; CHECK: DTrans Type: i16
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: [8 x i8]
; CHECK: DTrans Type: [8 x i8]
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Nested structure | Local instance
; CHECK: End LLVMType: %union.anon

%struct.Alloc_hider = type { ptr }
%union.anon = type { i16, [8 x i8] }
%class.basic_string = type { %struct.Alloc_hider, i64, %union.anon }

define void @test() {
  %i0 = alloca %class.basic_string, align 8
  %i1 = getelementptr inbounds %class.basic_string, ptr %i0, i64 0, i32 2
  store i16 19783, ptr %i1, align 2
  ret void
}

!intel.dtrans.types = !{!0, !2, !6}

!0 = !{!"S", %struct.Alloc_hider zeroinitializer, i32 1, !1}
!1 = !{i8 0, i32 1}
!2 = !{!"S", %union.anon zeroinitializer, i32 2, !3, !4}
!3 = !{i16 0, i32 0}
!4 = !{!"A", i32 8, !5}
!5 = !{i8 0, i32 0}
!6 = !{!"S", %class.basic_string zeroinitializer, i32 3, !7, !8, !9}
!7 = !{%struct.Alloc_hider zeroinitializer, i32 0}
!8 = !{i64 0, i32 0}
!9 = !{%union.anon zeroinitializer, i32 0}
