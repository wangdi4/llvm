; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a scalar field type using a pointer type.


%struct.test01 = type { i32, i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  ; In this case, we create the i8* element with an alloca instruction, because
  ; an i8* parameter would have its type inferred based on the uses.
  %value = alloca i8
  %pField = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test02, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32, i32 , i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test03, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32 }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !12 {
  %pField = getelementptr %struct.test04a, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test04b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = !{i32 0, i32 1}  ; i32*
!6 = distinct !{!4, !5}
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = !{i64 0, i32 1}  ; i64*
!9 = distinct !{!7, !8}
!10 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!11 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!12 = distinct !{!10, !11}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32 , i32 }
!16 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!13, !14, !15, !16, !17}
