; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a void pointer is passed to a function that should be marked
; as safe by the safety analyzer.

; Test the simple case where the void pointer argument gets used as the expected
; type.
%struct.test01 = type { i32, i32 }
define void @use_test01(i8* %p) !dtrans_type !2 {
  %p2 = bitcast i8* %p to %struct.test01*
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, %struct.test01* %p2, i64 0, i32 0
  store i32 0, i32* %field
  ret void
}
define void @test01() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test01*
  ; This is needed to establish %struct.test01* as an aliased type.
  %field = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 1
  store i32 0, i32* %field

  ; This is the instruction we're actually interested in.
  call void @use_test01(i8* %p)
  call void @free(i8* %p)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; TODO: When void pointers are analyzed in the callee this case should no
;       longer trigger any safety flags.
; TODO: Safety data: No issues found
; CHECK: Safety data: Mismatched argument use{{ *$}}

declare i8* @malloc(i64)
declare void @free(i8*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (i8*)
!3 = !{!"void", i32 0}  ; void
!4 = !{i8 0, i32 1}  ; i8*
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!5}
