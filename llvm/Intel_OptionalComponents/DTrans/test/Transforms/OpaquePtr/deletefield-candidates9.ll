; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks the delete fields candidate selection for nested
; structures.
;
; In this case, the fields 0 and 2 in %struct.C are candidates for deletion, but
; the structure gets disqualified because it is enclosed within a structure that
; does not pass the necessary safety checks (%struct.A has the 'Field address
; taken' safety flag). This case is similar to deletefield-candidates8.ll,
; except it's designed to check that the dependency analysis for nested types
; also considers types are not the immediate parents within the nesting
; hierarchy.

%struct.A = type { i32, %struct.B, %struct.D }
%struct.B = type { i16, %struct.C }
%struct.C = type { i8, i16, i32 }
%struct.D = type { i32 }

define void @foo(%struct.A* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !8 {
entry:
  %0 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  call void @bas(%struct.D* %0)
  ret void
}

define i16 @bar(%struct.C* "intel_dtrans_func_index"="1" %c) !intel.dtrans.func.type !10 {
entry:
  %y = getelementptr inbounds %struct.C, %struct.C* %c, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK: No candidates found.

declare !intel.dtrans.func.type !12 void @bas(%struct.D* "intel_dtrans_func_index"="1" %d)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{%struct.D zeroinitializer, i32 0}  ; %struct.D
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.C zeroinitializer, i32 0}  ; %struct.C
!6 = !{i8 0, i32 0}  ; i8
!7 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!8 = distinct !{!7}
!9 = !{%struct.C zeroinitializer, i32 1}  ; %struct.C*
!10 = distinct !{!9}
!11 = !{%struct.D zeroinitializer, i32 1}  ; %struct.D*
!12 = distinct !{!11}
!13 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.B, %struct.D }
!14 = !{!"S", %struct.B zeroinitializer, i32 2, !4, !5} ; { i16, %struct.C }
!15 = !{!"S", %struct.C zeroinitializer, i32 3, !6, !4, !1} ; { i8, i16, i32 }
!16 = !{!"S", %struct.D zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!13, !14, !15, !16}
