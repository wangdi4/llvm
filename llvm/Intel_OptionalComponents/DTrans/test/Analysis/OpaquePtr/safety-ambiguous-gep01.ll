; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer.

; The input parameter is declared as one type, but gets loaded as a
; different type, resulting in an "Ambiguous GEP" on the GEP pointer
; operand.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i16, i16 }
define void @test01(%struct.test01a** %ppStruct.a) !dtrans_type !3 {
  %pStruct.a.as.ppb = bitcast %struct.test01a** %ppStruct.a to %struct.test01b**
  %pStruct.b = load %struct.test01b*, %struct.test01b** %pStruct.a.as.ppb
  %pField = getelementptr %struct.test01b, %struct.test01b* %pStruct.b, i64 0, i32 3
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


; Merging pointers of two different structures together, and then using
; the result in a GEP prevents DTrans from knowing which structure type is
; being accessed.
%struct.test02a = type { i32 }
%struct.test02b = type { i32 }
define void @test02(%struct.test02a* %pStruct1a, %struct.test02b* %pStruct1b) !dtrans_type !7 {
  %pStruct1a.as.p8 = bitcast %struct.test02a* %pStruct1a to i8*
  %pStruct1b.as.p8 = bitcast %struct.test02b* %pStruct1b to i8*
  %ambig_ptr = select i1 undef, i8* %pStruct1a.as.p8, i8* %pStruct1b.as.p8
  %ambig_gep = getelementptr i8, i8* %ambig_ptr, i64 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Ambiguous GEP | Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Ambiguous GEP | Unsafe pointer merge{{ *$}}


; Mixing structure pointer and array with an alternative representation of the
; first field is not supported, and should be marked as "Ambiguous GEP"
%struct.test03a = type { %struct.test03b, i32 }
%struct.test03b = type { [8 x i64] }
define void @test03(%struct.test03a* %pStruct.a) !dtrans_type !15 {
  %pStruct.a.as.ar = bitcast %struct.test03a* %pStruct.a to [64 x i8]*
  %first = getelementptr inbounds [64 x i8], [64 x i8]* %pStruct.a.as.ar, i64 0, i64 0
  %value = load i8, i8* %first
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a**)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 2}  ; %struct.test01a**
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"F", i1 false, i32 2, !4, !8, !10}  ; void (%struct.test02a*, %struct.test02b*)
!8 = !{!9, i32 1}  ; %struct.test02a*
!9 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!10 = !{!11, i32 1}  ; %struct.test02b*
!11 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!12 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!13 = !{!"A", i32 8, !14}  ; [8 x i64]
!14 = !{i64 0, i32 0}  ; i64
!15 = !{!"F", i1 false, i32 1, !4, !16}  ; void (%struct.test03a*)
!16 = !{!17, i32 1}  ; %struct.test03a*
!17 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!18 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test01b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }
!20 = !{!"S", %struct.test02a zeroinitializer, i32 1, !1} ; { i32 }
!21 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!22 = !{!"S", %struct.test03a zeroinitializer, i32 2, !12, !1} ; { %struct.test03b, i32 }
!23 = !{!"S", %struct.test03b zeroinitializer, i32 1, !13} ; { [8 x i64] }

!dtrans_types = !{!18, !19, !20, !21, !22, !23}
