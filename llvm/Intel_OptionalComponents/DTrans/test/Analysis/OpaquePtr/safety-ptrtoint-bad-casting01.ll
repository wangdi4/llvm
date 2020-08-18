; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety checks for 'ptrtoint' instructions when there are unsafe
; uses.

; 'ptrtoint' of a pointer that may alias multiple types is not permitted.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define void @test01(%struct.test01a* %pStruct1) !dtrans_type !3 {
  %pStruct1.as.pB = bitcast %struct.test01a* %pStruct1 to %struct.test01b*
  %use1 = getelementptr %struct.test01b, %struct.test01b* %pStruct1.as.pB, i64 0, i32 0
  %tmp1 = ptrtoint %struct.test01b* %pStruct1.as.pB to i64
  ret void
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


; 'ptrtoint' that does not go to the same size as a pointer is not permitted.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %pStruct1) !dtrans_type !7 {
  %tmp1 = ptrtoint %struct.test02* %pStruct1 to i32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"F", i1 false, i32 1, !4, !8}  ; void (%struct.test02*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!10, !11, !12}