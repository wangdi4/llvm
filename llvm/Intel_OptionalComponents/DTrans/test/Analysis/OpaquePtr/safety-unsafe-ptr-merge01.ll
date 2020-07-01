; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Unsafe pointer merge" safety condition when using
; 'select' instructions.

; In this case, there is a dominant type determined for the 'select'
; instruction. However, because the type is also detected as being
; used as a i32*, which is not a generic equivalent, this needs to be
; marked as 'Unsafe pointer merge'
%struct.test01 = type { i32*, %struct.test01* }
define internal void @test01(%struct.test01* %pStruct, i32* %p32) !dtrans_type !4 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pValue = load %struct.test01*, %struct.test01** %pField
  %badCast = bitcast %struct.test01* %pValue to i32*
  %badMerge = select i1 undef, i32* %p32, i32* %badCast
  %badLoad = load i32, i32* %badMerge
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Unsafe pointer merge{{ *$}}


; In this case, there is a dominant type, and an i8* associated with the
; 'select' instruction. However, i8* is treated as compatible with the
; the pointer-to struct type because it is a generic equivalent, so this
; case will not trigger a safety condition.
%struct.test02 = type { i32, i32 }
@global_test02 = internal global %struct.test02* zeroinitializer, !dtrans_type !7
define internal void @test02() {
  %pStruct1.p8 = call i8* @malloc(i64 8)
  %pStruct1 = bitcast i8* %pStruct1.p8 to %struct.test02*

  %pStruct2.p8 = call i8* @malloc(i64 16)
  %pStruct2 = bitcast i8* %pStruct2.p8 to %struct.test02*

  %chosen = select i1 undef, %struct.test02* %pStruct1, %struct.test02* %pStruct2
  store %struct.test02* %chosen, %struct.test02** @global_test02

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global pointer{{ *$}}

declare i8* @malloc(i64)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"F", i1 false, i32 2, !5, !2, !1}  ; void (%struct.test01*, i32*)
!5 = !{!"void", i32 0}  ; void
!6 = !{i32 0, i32 0}  ; i32
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32*, %struct.test01* }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !6, !6} ; { i32, i32 }

!dtrans_types = !{!9, !10}
