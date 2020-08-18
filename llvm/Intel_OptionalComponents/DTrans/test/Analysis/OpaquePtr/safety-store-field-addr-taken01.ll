; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-outofboundsok=true -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -whole-program-assume -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -whole-program-assume -dtrans-outofboundsok=false -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F
; RUN: opt -whole-program-assume -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F


; Test that storing the address of a structure field results in the "Field
; address taken" safety bit.

%struct.test01a = type { i64, %struct.test01b* }
%struct.test01b = type { i32, i32 }
@gTest01b = internal global %struct.test01b** zeroinitializer, !dtrans_type !5
define void @test01(%struct.test01a* %pStruct) !dtrans_type !6 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  store %struct.test01b** %pField, %struct.test01b*** @gTest01b
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01a
; CHECK_ALWAYS: Field address taken{{ *$}}

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01b
; CHECK_OOB_T: Field address taken | Global pointer{{ *$}}
; CHECK_OOB_F: Global pointer{{ *$}}


; This case is to verify that storing the address of field into another field
; (ie. the pointer location is an element pointee) results in the "Field
; address taken" safety bit.
%struct.test02a = type { i64*, i64, %struct.test02b* }
%struct.test02b = type { i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !13 {
  %pField0 = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField1 = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  store i64* %pField1, i64** %pField0
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test02a
; CHECK_ALWAYS: Field address taken{{ *$}}

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test02b
; CHECK_OOB_T: Field address taken{{ *$}}
; CHECK_OOB_F: No issues found


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01b*
!3 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!3, i32 2}  ; %struct.test01b**
!6 = !{!"F", i1 false, i32 1, !7, !8}  ; void (%struct.test01a*)
!7 = !{!"void", i32 0}  ; void
!8 = !{!9, i32 1}  ; %struct.test01a*
!9 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!10 = !{i64 0, i32 1}  ; i64*
!11 = !{!12, i32 1}  ; %struct.test02b*
!12 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!13 = !{!"F", i1 false, i32 1, !7, !14}  ; void (%struct.test02a*)
!14 = !{!15, i32 1}  ; %struct.test02a*
!15 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!16 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01b* }
!17 = !{!"S", %struct.test01b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!18 = !{!"S", %struct.test02a zeroinitializer, i32 3, !10, !1, !11} ; { i64*, i64, %struct.test02b* }
!19 = !{!"S", %struct.test02b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!dtrans_types = !{!16, !17, !18, !19}
