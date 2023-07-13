; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Unsafe pointer merge" safety condition when using
; 'select' instructions.

; In this case, there is a dominant type determined for the 'select'
; instruction. However, because the type is also detected as being
; used as a i32*, which is not a generic equivalent, this needs to be
; marked as 'Unsafe pointer merge'
%struct.test01 = type { ptr, ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %p32) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  %pValue = load ptr, ptr %pField
  %badMerge = select i1 undef, ptr %p32, ptr %pValue
  %badLoad = load i32, ptr %badMerge
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
@global_test02 = internal global ptr zeroinitializer, !intel_dtrans_type !5
define internal void @test02() {
  %pStruct1.p8 = call ptr @malloc(i64 8)

  %pStruct2.p8 = call ptr @malloc(i64 16)

  %chosen = select i1 undef, ptr %pStruct1.p8, ptr %pStruct2.p8
  store ptr %chosen, ptr @global_test02

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test02

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

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
