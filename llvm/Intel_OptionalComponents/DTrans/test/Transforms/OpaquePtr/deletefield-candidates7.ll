; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks the delete fields candidate selection for nested
; structures.
;
; In this case, the fields 0 and 2 in %struct.B can be deleted even though it
; is enclosed in %struct.A. Also, field 0 and 1 of %struct.A can be deleted.

%struct.A = type { i16, i32, %struct.B }
%struct.B = type { i8, i16, i32 }

define i16 @foo(%struct.A* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !6 {
entry:
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2, i32 1
  %z = load i16, i16* %y, align 4
  ret i16 %z
}

define i16 @bar(%struct.B* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !8 {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %z = load i16, i16* %y, align 4
  ret i16 %z
}

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK-DAG: Selected for deletion: %struct.A
; CHECK-DAG: Selected for deletion: %struct.B

!1 = !{i16 0, i32 0}  ; i16
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 0}  ; i8
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = distinct !{!5}
!7 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!8 = distinct !{!7}
!9 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !3, !2} ; { i16, %struct.B, i32 }
!10 = !{!"S", %struct.B zeroinitializer, i32 3, !4, !1, !3} ; { i8, i16, i32 }

!intel.dtrans.types = !{!9, !10}
