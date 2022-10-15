; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks the delete fields candidate selection for nested
; structures.
;
; In this case, the fields 0 and 2 in %struct.B are candidates for deletion, but
; the structure gets disqualified because it is enclosed within a structure that
; does not pass the necessary safety checks (%struct.A has the 'Field address
; taken' safety flag).

%struct.A = type { i32, %struct.B, %struct.C }
%struct.B = type { i8, i16, i32 }
%struct.C = type { i32 }

define void @foo(%struct.A* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !7 {
entry:
  %0 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  call void @bas(%struct.C* %0)
  ret void
}

define i16 @bar(%struct.B* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !9 {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK: No candidates found.

declare !intel.dtrans.func.type !11 void @bas(%struct.C* "intel_dtrans_func_index"="1" %c)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{%struct.C zeroinitializer, i32 0}  ; %struct.C
!4 = !{i8 0, i32 0}  ; i8
!5 = !{i16 0, i32 0}  ; i16
!6 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!7 = distinct !{!6}
!8 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!9 = distinct !{!8}
!10 = !{%struct.C zeroinitializer, i32 1}  ; %struct.C*
!11 = distinct !{!10}
!12 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.B, %struct.C }
!13 = !{!"S", %struct.B zeroinitializer, i32 3, !4, !5, !1} ; { i8, i16, i32 }
!14 = !{!"S", %struct.C zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!12, !13, !14}
