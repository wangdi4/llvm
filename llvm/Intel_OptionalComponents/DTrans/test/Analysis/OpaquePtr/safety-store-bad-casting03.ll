; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store a pointer to a structure, which has multiple type aliases that do not
; match the expected type for the pointer-to-pointer location used in the store
; instruction. This should result in the types being marked with "Unsafe
; pointer store"

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i32 }
%struct.test01c = type { i16, i16, i16, i16 }

@varTest01 = internal global ptr null, !intel_dtrans_type !3
define void @test01() {
  %var1 = call ptr @test01b()
  %var2 = call ptr @test01c()
  ; Keep the bitcasts here to trigger the type inference for the uses.
  %val1.as.i8 = bitcast ptr %var1 to ptr
  %val2.as.i8 = bitcast ptr %var2 to ptr
  %valueToStore.as.i8 = select i1 undef, ptr %val1.as.i8, ptr %val2.as.i8
  %valueToStore = bitcast ptr %valueToStore.as.i8 to ptr
  store ptr %valueToStore, ptr @varTest01
  ret void
}

define "intel_dtrans_func_index"="1" ptr @test01b() !intel.dtrans.func.type !5 {
  ret ptr null
}

define "intel_dtrans_func_index"="1" ptr @test01c() !intel.dtrans.func.type !7 {
  ret ptr null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Global pointer | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01c


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = distinct !{!4}
!6 = !{%struct.test01c zeroinitializer, i32 1}  ; %struct.test01c*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !1} ; { i16, i16, i32 }
!10 = !{!"S", %struct.test01c zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }

!intel.dtrans.types = !{!8, !9, !10}
