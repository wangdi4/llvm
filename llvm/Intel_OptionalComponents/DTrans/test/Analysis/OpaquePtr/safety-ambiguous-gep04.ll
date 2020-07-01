; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer.

; "Ambiguous GEP" case where type is used with a single GEP index, but using
; a type that does not match the expected type.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i16, i16 }
define void @test01(%struct.test01a* %pStruct.a) !dtrans_type !3 {
  %pStruct.a.as.b = bitcast %struct.test01a* %pStruct.a to %struct.test01b*
  %ambig_gep = getelementptr %struct.test01b, %struct.test01b* %pStruct.a.as.b, i64 4
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Ambiguous GEP{{ *$}}


; "Ambiguous GEP" case where the type is used with nested structure access, using
; the incorrect type.
%struct.test02a0 = type { i32, %struct.test02a1 }
%struct.test02a1 = type { i64, %struct.test02a2 }
%struct.test02a2 = type { i32, i32 }
%struct.test02b0 = type { i32, %struct.test02b1 }
%struct.test02b1 = type { i64, %struct.test02b2 }
%struct.test02b2 = type { i32, i16, i16 }
define void @test02(%struct.test02a0* %pStruct.a) !dtrans_type !14 {
  %pStruct.a.as.b = bitcast %struct.test02a0* %pStruct.a to %struct.test02b0*
  %ambig_gep = getelementptr %struct.test02b0, %struct.test02b0* %pStruct.a.as.b, i64 0, i32 1, i32 1, i32 2
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a0
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a1
; CHECK: Safety data: Ambiguous GEP | Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a2
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b0
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b1
; CHECK: Safety data: Ambiguous GEP | Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b2
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!8, i32 1}  ; %struct.test01b*
!8 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!9 = !{!"R", %struct.test02a1 zeroinitializer, i32 0}  ; %struct.test02a1
!10 = !{i64 0, i32 0}  ; i64
!11 = !{!"R", %struct.test02a2 zeroinitializer, i32 0}  ; %struct.test02a2
!12 = !{!"R", %struct.test02b1 zeroinitializer, i32 0}  ; %struct.test02b1
!13 = !{!"R", %struct.test02b2 zeroinitializer, i32 0}  ; %struct.test02b2
!14 = !{!"F", i1 false, i32 1, !4, !15}  ; void (%struct.test02a0*)
!15 = !{!16, i32 1}  ; %struct.test02a0*
!16 = !{!"R", %struct.test02a0 zeroinitializer, i32 0}  ; %struct.test02a0
!17 = !{!18, i32 1}  ; %struct.test02b0*
!18 = !{!"R", %struct.test02b0 zeroinitializer, i32 0}  ; %struct.test02b0
!19 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!20 = !{!"S", %struct.test01b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }
!21 = !{!"S", %struct.test02a0 zeroinitializer, i32 2, !1, !9} ; { i32, %struct.test02a1 }
!22 = !{!"S", %struct.test02a1 zeroinitializer, i32 2, !10, !11} ; { i64, %struct.test02a2}
!23 = !{!"S", %struct.test02a2 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!24 = !{!"S", %struct.test02b0 zeroinitializer, i32 2, !1, !12} ; { i32, %struct.test02b1 }
!25 = !{!"S", %struct.test02b1 zeroinitializer, i32 2, !10, !13} ; { i64, %struct.test02b2}
!26 = !{!"S", %struct.test02b2 zeroinitializer, i32 3, !1, !2, !2} ; { i32, i16, i16 }

!dtrans_types = !{!19, !20, !21, !22, !23, !24, !25, !26}
