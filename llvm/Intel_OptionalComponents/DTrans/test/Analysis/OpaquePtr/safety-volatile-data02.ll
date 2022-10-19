; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases for loads that are marked 'volatile' which load pointers
; involving aggregate types.

; This case is loading a pointer-to-pointer so does not result in
; 'Volatile data' on the structure type.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01** "intel_dtrans_func_index"="1" %ppStruct) !intel.dtrans.func.type !3 {
  %pStruct = load volatile %struct.test01*, %struct.test01** %ppStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; This case is loading the entire structure with a volatile load, so
; will result in 'Volatile data' on the structure type.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %Struct = load volatile %struct.test02, %struct.test02* %pStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Volatile data | Whole structure reference{{ *$}}
; CHECK: End LLVMType: %struct.test02


; This case is loading the first member of the structure using the address
; of the structure itself with a volatile load, so will result in the
; 'Volatile data' on the structure type.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pStruct.as.p32 = bitcast %struct.test03* %pStruct to i32*
  %val = load volatile i32, i32* %pStruct.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Volatile data{{ *$}}
; CHECK: End LLVMType: %struct.test03


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9, !10}
