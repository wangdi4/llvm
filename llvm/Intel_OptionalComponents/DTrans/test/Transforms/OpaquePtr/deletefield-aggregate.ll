; RUN: opt %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; nested structure element accesses.

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:1) and (B:1) should stay.

%struct.A = type { i16, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFT_struct.A = type { %__DFT_struct.B }
; CHECK-DAG: %__DFT_struct.B = type { i16 }


define i16 @foo(%struct.A* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !6 {
entry:
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1, i32 1
  %z = load i16, i16* %y, align 4
  ret i16 %z
}
; CHECK-LABEL: define internal i16 @foo
; CHECK: %y = getelementptr inbounds %__DFT_struct.A, {{.*}} %a, i64 0, i32 0, i32 0

define i16 @bar(%struct.B* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !8 {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %z = load i16, i16* %y, align 4
  ret i16 %z
}
; CHECK-LABEL: define internal i16 @bar
; CHECK: %y = getelementptr inbounds %__DFT_struct.B, {{.*}} %b, i64 0, i32 0

!1 = !{i16 0, i32 0}  ; i16
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 0}  ; i8
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = distinct !{!5}
!7 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!8 = distinct !{!7}
!9 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !3} ; { i16, %struct.B, i32 }
!10 = !{!"S", %struct.B zeroinitializer, i32 3, !4, !1, !3} ; { i8, i16, i32 }

!intel.dtrans.types = !{!9, !10}
