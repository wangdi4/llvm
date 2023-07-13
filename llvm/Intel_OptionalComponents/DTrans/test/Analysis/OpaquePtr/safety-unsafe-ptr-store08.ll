; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Tests a pointer-to-pointer to a structure type being store to a pointer
; location that does not match the expected pointer type, results in the "Unsafe
; pointer store" safety violation.

%struct.test01 = type { i32, i32 }
define void @test01() {
  %localStruct = alloca %struct.test01
  %localPtr = alloca ptr, !intel_dtrans_type !2
  store ptr %localStruct, ptr %localPtr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, i32 }
define void @test02() {
  %localStruct = alloca %struct.test02
  %localPtr = alloca ptr, !intel_dtrans_type !3
  store ptr %localStruct, ptr %localPtr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32, i32 }
define void @test03() {
  %localStruct = alloca %struct.test03
  %localPtr = alloca ptr, !intel_dtrans_type !4
  store ptr %localStruct, ptr %localPtr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04a = type { i32, i32 }
%struct.test04b = type { i64 }
define void @test04() {
  %localStruct = alloca %struct.test04a
  %localPtr = alloca ptr, !intel_dtrans_type !6
  store ptr %localStruct, ptr %localPtr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test04b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{i16 0, i32 1}  ; i16*
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i64 0, i32 0}  ; i64
!6 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test04b zeroinitializer, i32 1, !5} ; { i64 }

!intel.dtrans.types = !{!7, !8, !9, !10, !11}
