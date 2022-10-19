; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test detection of "Unsafe pointer merge" safety condition when using
; 'select' instructions.

; In this case, there is a dominant type determined for the 'select'
; instruction. However, because the type is also detected as being
; used as a i32*, which is not a generic equivalent, this needs to be
; marked as 'Unsafe pointer merge'
%struct.test01 = type { i32*, %struct.test01* }
define internal void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i32* "intel_dtrans_func_index"="2" %p32) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pValue = load %struct.test01*, %struct.test01** %pField
  %badCast = bitcast %struct.test01* %pValue to i32*
  %badMerge = select i1 undef, i32* %p32, i32* %badCast
  %badLoad = load i32, i32* %badMerge
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01


; In this case, there is a dominant type, and an i8* associated with the
; 'select' instruction. However, i8* is treated as compatible with the
; the pointer-to struct type because it is a generic equivalent, so this
; case will not trigger a safety condition.
%struct.test02 = type { i32, i32 }
@global_test02 = internal global %struct.test02* zeroinitializer, !intel_dtrans_type !5
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
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test02

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !1}
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32*, %struct.test01* }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9}
