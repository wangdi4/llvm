; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure where the callee does not use the
; argument. This does not require marking it as "Address taken"

; Test as a pointer-sized int type
%struct.test01a = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01a
  %pStruct.as.i64 = ptrtoint %struct.test01a* %pStruct to i64
  call void @test01callee(i64 %pStruct.as.i64)
  ret void
}
define void @test01callee(i64 %in) {
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Local instance{{ *$}}


; Test with bitcast function call
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define void @test02(%struct.test02a** %pp) !dtrans_type !3 {
  %pStruct = load %struct.test02a*, %struct.test02a** %pp
  call void bitcast (void (%struct.test02b*)* @test02callee
                       to void (%struct.test02a*)*)(%struct.test02a* %pStruct)
  ret void
}
define void @test02callee(%struct.test02b* %in) !dtrans_type !7 {
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test02a**)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 2}  ; %struct.test02a**
!6 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!7 = !{!"F", i1 false, i32 1, !4, !8}  ; void (%struct.test02b*)
!8 = !{!9, i32 1}  ; %struct.test02b*
!9 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!10 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test02b zeroinitializer, i32 1, !2} ; { i64 }

!dtrans_types = !{!10, !11, !12}
