; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type that the structure begins with.
; These cases are casting a pointer to the structure to be a type that does not
; match the field type that starts the structure.


%struct.test01 = type { ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %mem = call ptr @malloc(i64 128)
  store ptr %mem, ptr %pStruct
  ret void
}
; This case gets treated as safe by DTrans because the i8* will be compatible
; with the element zero type of the structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { ptr }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %local = alloca i16
  store ptr %local, ptr %pStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { ptr, ptr }
define internal void @test03(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %local = alloca i64
  store ptr %local, ptr %pStruct
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test03

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32* }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 1, !1} ; { i32* }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }

!intel.dtrans.types = !{!10, !11, !12}
