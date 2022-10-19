; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that storing the address of an element of an array member within a
; structure type is also treated as "Field address taken memory" for the structure if it
; is element 0 of the array.

; Element 0 of the array is also the address of the structure field. This should
; be "Field address taken memory".
%struct.test01a = type { i64, float, %struct.test01b }
%struct.test01b = type { i64, [10 x i8] }
@var01a = internal global %struct.test01a zeroinitializer
@var01charptr = internal global i8* zeroinitializer, !intel_dtrans_type !6
define void @test01()  {
  %array_elem_addr = getelementptr %struct.test01a, %struct.test01a* @var01a, i64 0, i32 2, i32 1, i64 0
  store i8* %array_elem_addr, i8** @var01charptr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Field address taken memory | Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; Element 3 of the array is not the same address as the structure field. This is not
; "Field address taken"
%struct.test02a = type { i64, float, %struct.test02b }
%struct.test02b = type { i64, [10 x i8] }
@var02a = internal global %struct.test02a zeroinitializer
@var02charptr = internal global i8* zeroinitializer, !intel_dtrans_type !6
define void @test02()  {
  %array_elem_addr = getelementptr %struct.test02a, %struct.test02a* @var02a, i64 0, i32 2, i32 1, i64 3
  store i8* %array_elem_addr, i8** @var02charptr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


!1 = !{i64 0, i32 0}  ; i64
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{!"A", i32 10, !5}  ; [10 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!8 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i64, float, %struct.test01b }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }
!10 = !{!"S", %struct.test02a zeroinitializer, i32 3, !1, !2, !7} ; { i64, float, %struct.test02b }
!11 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }

!intel.dtrans.types = !{!8, !9, !10, !11}
