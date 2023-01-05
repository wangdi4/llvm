; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test dominant type resolution when the pointer type information contains 3
; pointer types, where one type is considered the dominant type of the other
; two, but the other two do not directly dominate each other to verify that a
; dominant type is consistently determined. This is a regression test for
; CMPLRLLVM-32215 where the order of evaluation can affect the resolution of the
; dominant type.

%class.Block = type { %class.Base, i16, i16, [4 x i8] }
%class.Base = type { %class.Allocator, i16, i16, ptr }
%class.Allocator = type { ptr }
%class.Manager = type { ptr }
%class.Fragment = type { double }
%struct.Node = type { ptr, ptr, ptr }

define i1 @test01() {
  %i = getelementptr inbounds %struct.Node, ptr undef, i64 0, i32 0
  %i2 = getelementptr inbounds %struct.Node, ptr undef, i64 0, i32 1
  %i3 = load ptr, ptr %i2, align 8

  ; The inference of the usage type for this instruction will result
  ; in 3 pointer types. We want to check the that dominant type from
  ; them is deterministic.
  %i4 = bitcast ptr %i3 to ptr

  %i5 = load ptr, ptr %i4, align 8
  %i6 = getelementptr inbounds %class.Base, ptr %i5, i64 0, i32 3
  %i7 = bitcast ptr %i5 to ptr
  %i9 = load ptr, ptr %i, align 8
  %i10 = icmp eq ptr %i9, %i7
  ret i1 true
}

; CHECK: %i4 = bitcast ptr %i3 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %class.Base**{{ *}}
; CHECK-NEXT:        %class.Block**{{ *}}
; CHECK-NEXT:        %struct.Node*{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.Node @ 0
; CHECK-NEXT:      DomTy: %struct.Node*{{ *}}

!intel.dtrans.types = !{!0, !8, !1, !2, !11, !13}

!0 = !{!"S", %class.Block zeroinitializer, i32 4, !1, !4, !4, !6}
!1 = !{!"S", %class.Base zeroinitializer, i32 4, !2, !4, !4, !5}
!2 = !{!"S", %class.Allocator zeroinitializer, i32 1, !3}
!3 = !{%class.Manager zeroinitializer, i32 1}
!4 = !{i16 0, i32 0}
!5 = !{%class.Fragment zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{!"S", %struct.Node zeroinitializer, i32 3, !9, !10, !10}
!9 = !{%class.Block zeroinitializer, i32 1}
!10 = !{%struct.Node zeroinitializer, i32 1}
!11 = !{!"S", %class.Fragment zeroinitializer, i32 1, !12}
!12 = !{double 0.000000e+00, i32 0}
!13 = !{!"S", %class.Manager zeroinitializer, i32 1, !14}
!14 = !{!15, i32 2}
!15 = !{!"F", i1 true, i32 0, !16}
!16 = !{i32 0, i32 0}
