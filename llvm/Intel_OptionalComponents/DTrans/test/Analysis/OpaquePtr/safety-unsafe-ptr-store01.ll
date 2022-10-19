; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type of the structure begins with.
; These cases are for using a scalar type that does not match the scalar that
; starts the structure.

; These cases could be considered to be a "Mismatched element access" or an
; "Unsafe pointer store". These are using "Unsafe pointer store" to be
; compatible with the implementation that was done in the LocalPointerAnalyzer.

%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i8 %value) !intel.dtrans.func.type !3 {
  %pStruct.as.p8 = bitcast %struct.test01* %pStruct to i8*
  store i8 %value, i8* %pStruct.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct, i16 %value) !intel.dtrans.func.type !5 {
  %pStruct.as.p16 = bitcast %struct.test02* %pStruct to i16*
  store i16 %value, i16* %pStruct.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct, i64 %value) !intel.dtrans.func.type !7 {
  %pStruct.as.p64 = bitcast %struct.test03* %pStruct to i64*
  store i64 %value, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test03


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9, !10}
