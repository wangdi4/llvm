; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a pointer type that does not match the field's
; pointer type. In these cases, the pointer operand of the store instruction
; is cast to a different type.


%struct.test01 = type { ptr, ptr, ptr }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %value = call ptr @malloc(i64 4)
  %pField = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; Memory allocation gets inferred based on the type it is used as.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found


%struct.test01a = type { ptr, ptr, ptr }
define void @test01a(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %value = call ptr @malloc(i64 4)
  %pField1 = getelementptr %struct.test01a, ptr %pStruct, i64 0, i32 1
  %pField2 = getelementptr %struct.test01a, ptr %pStruct, i64 0, i32 2
  store ptr %value, ptr %pField1
  store ptr %value, ptr %pField2
  ret void
}
; Here the memory allocation gets inferred as two different types, resulting in
; a safety condition.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01

%struct.test02 = type { ptr, ptr, ptr }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test02, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { ptr, ptr, ptr }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !11 {
  %pField = getelementptr %struct.test03, ptr %pStruct, i64 0, i32 1
  store ptr %value, ptr %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04a = type { ptr, ptr, ptr }
%struct.test04b = type { ptr }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStruct, ptr "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !14 {
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

declare !intel.dtrans.func.type !16 "intel_dtrans_func_index"="1" ptr @malloc(i64)


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!8 = !{i16 0, i32 1}  ; i16*
!9 = distinct !{!7, !8}
!10 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!11 = distinct !{!10, !4}
!12 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!13 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!14 = distinct !{!12, !13}
!15 = !{i8 0, i32 1}  ; i8*
!16 = distinct !{!15}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!18 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !4} ; { i32*, i32*, i64* }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!20 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!22 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }

!intel.dtrans.types = !{!17, !18, !19, !20, !21, !22}
