; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:1) and (B:1) should stay.

%struct.A = type { i16, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFT_struct.A = type { %__DFT_struct.B }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

@a = private global %struct.A zeroinitializer, align 8
@b = private global %struct.B zeroinitializer, align 8

define i16 @foo() {
entry:
  %tmp1 = load i16, i16* getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i32 1)
  ret i16 %tmp1
}
; CHECK-LABEL: define i16 @foo
; CHECK: %tmp1 = load i16, {{.*}} getelementptr inbounds (%__DFT_struct.A, {{.*}} @a, i64 0, i32 0, i32 0)

define i16 @bar() {
entry:
  %tmp1 = load i16, i16* getelementptr inbounds (%struct.B, %struct.B* @b, i64 0, i32 1)
  ret i16 %tmp1
}
; CHECK-LABEL: define i16 @bar
; CHECK: %tmp1 = load i16, {{.*}} getelementptr inbounds (%__DFT_struct.B, {{.*}} @b, i64 0, i32 0)

!1 = !{i16 0, i32 0}  ; i16
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 0}  ; i8
!5 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !3} ; { i16, %struct.B, i32 }
!6 = !{!"S", %struct.B zeroinitializer, i32 3, !4, !1, !3} ; { i8, i16, i32 }

!intel.dtrans.types = !{!5, !6}
