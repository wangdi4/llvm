; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Load a pointer to a structure with a different type than expected
; for the structure. This should result in the 'Bad casting' safety
; condition. When pointers are opaque, the bitcast will not exist,
; but the types are mismatched so it will still be treated as if it
; were cast.

%struct.test01a = type { i32 }
%struct.test01b = type { i16, i16 }
define void @test01(%struct.test01a** %pStruct) !dtrans_type !3 {
  %bc = bitcast %struct.test01a** %pStruct to %struct.test01b**
  %pB = load %struct.test01b*, %struct.test01b** %bc
  %pField = getelementptr %struct.test01b, %struct.test01b* %pB, i64 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a**)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 2}  ; %struct.test01a**
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i16, i16 }

!dtrans_types = !{!7, !8}
