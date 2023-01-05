; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for stores that are marked 'volatile' which store pointers
; involving aggregate types.

; This case is storing a pointer-to-pointer so does not result in
; 'Volatile data' on the structure type.
%struct.test01 = type { i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %ppStruct) !intel.dtrans.func.type !3 {
  store volatile ptr null, ptr %ppStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; This case is storing the entire structure with a volatile store, so
; will result in 'Volatile data' on the structure type.
%struct.test02 = type { i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %local = load %struct.test02, ptr %pStruct
  store volatile %struct.test02 %local, ptr %pStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Volatile data | Whole structure reference{{ *$}}
; CHECK: End LLVMType: %struct.test02


; This case is storing the first member of the structure using that address
; of the structure itself with a volatile store, so will result in the
; 'Volatile data' on the structure type.
%struct.test03 = type { i32, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  store volatile i32 0, ptr %pStruct
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
