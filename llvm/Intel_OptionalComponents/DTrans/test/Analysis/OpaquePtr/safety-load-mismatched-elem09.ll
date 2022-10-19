; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a pointer type, and the load
; type is a scalar type.

%struct.test01 = type { i32*, i32* }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pStruct.as.p8 = bitcast %struct.test01* %pStruct to i8*
  %vField = load i8, i8* %pStruct.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pStruct.as.p16 = bitcast %struct.test02* %pStruct to i16*
  %vField = load i16, i16* %pStruct.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32*, i32* }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pStruct.as.p64 = bitcast %struct.test03* %pStruct to i64*
  %vField = load i64, i64* %pStruct.as.p64
  ret void
}
; This case is treated as safe because it is the equivalent of using a GEP to
; get the address of the first field, and then casting that from i32** to i64*,
; which is marked as safe. %vField is still being tracked as a pointer type by
; the pointer analyzer, so any unsafe uses of that could result in a safety flag
; getting set.
;
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }

!intel.dtrans.types = !{!8, !9, !10}
