; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store a pointer to a structure to a location that expects a pointer to a
; different type of structure.

; This case will not be able to resolve a dominant aggregate type for the value
; operand of the store because it is declared as one type, but used as another.
%struct.test01a = type { ptr, ptr, ptr }
%struct.test01b = type { i64, i64, i64 }
define i64 @test01(ptr "intel_dtrans_func_index"="1" %ppStructA) !intel.dtrans.func.type !4 {
  %pStructB = alloca %struct.test01b
  store ptr %pStructB, ptr %ppStructA
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; This case will not be able to resolve a dominant type for the pointer operand
; of the store because it is declared at one level of indirection, but used at a
; different level of indirection.
%struct.test02a = type { ptr, ptr, ptr }
define i64 @test02(ptr "intel_dtrans_func_index"="1" %pppStructA) !intel.dtrans.func.type !6 {
  %pStructA = alloca %struct.test02a
  store ptr %pStructA, ptr %pppStructA
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02a


; This case stores an arbitrary i64 value to a location that should hold a
; pointer to an aggregate type.
%struct.test03a = type { i64, i64, i64 }
define i64 @test03(ptr "intel_dtrans_func_index"="1" %ppStructA, i64 %value) !intel.dtrans.func.type !8 {
  store i64 %value, ptr %ppStructA
  ret i64 0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test03a


!1 = !{i64 0, i32 1}  ; i64*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = distinct !{!3}
!5 = !{%struct.test02a zeroinitializer, i32 3}  ; %struct.test02a***
!6 = distinct !{!5}
!7 = !{%struct.test03a zeroinitializer, i32 2}  ; %struct.test03a**
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i64*, i64*, i64* }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i64, i64, i64 }
!11 = !{!"S", %struct.test02a zeroinitializer, i32 3, !1, !1, !1} ; { i64*, i64*, i64* }
!12 = !{!"S", %struct.test03a zeroinitializer, i32 3, !2, !2, !2} ; { i64, i64, i64 }

!intel.dtrans.types = !{!9, !10, !11, !12}
