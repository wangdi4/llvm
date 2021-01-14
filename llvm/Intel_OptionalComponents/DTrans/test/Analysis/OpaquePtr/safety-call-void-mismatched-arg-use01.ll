; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where an void pointer is passed to a function that should be marked
; as "Mismatched argument use" by the safety analyzer.

; Test the simple case where the void pointer argument gets used as a different
; type in the callee than the caller.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define void @use_test01(i8* %p) !dtrans_type !3 {
  %p2 = bitcast i8* %p to %struct.test01b*
  ; This is needed to establish %struct.test01b* as an aliased type.
  %field = getelementptr %struct.test01b, %struct.test01b* %p2, i64 0, i32 0
  store i64 0, i64* %field
  ret void
}
define void @test01() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test01a*
  ; This is needed to establish %struct.test01a* as an aliased type.
  %field = getelementptr %struct.test01a, %struct.test01a* %tmp, i64 0, i32 1
  store i32 0, i32* %field

  ; This is the instruction we're actually interested in.
  call void @use_test01(i8* %p)
  call void @free(i8* %p)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Mismatched argument use{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Mismatched argument use{{ *$}}

declare i8* @malloc(i64)
declare void @free(i8*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (i8*)
!4 = !{!"void", i32 0}  ; void
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }

!dtrans_types = !{!6, !7}
