; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Basic test of the AOS-to-SOA conversion for an allocation of a
; variable number of elements. Also, the field initialization needs
; to take into account element alignment when initializing the structure
; of arrays because the original structure contained padding between
; elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i16, ptr, i8, ptr }
%struct.test01dep = type { i32, ptr }
@glob = internal global %struct.test01dep zeroinitializer

define i32 @main() {
  call void @test01(i64 3)
  ret i32 0
}

; Test conversion of calloc call.
define void @test01(i64 %some_val) {
; CHECK-LABEL: define void @test01
  %mul = mul i64 %some_val, 32
  %mem = call ptr @calloc(i64 5, i64 %mul)

; Verify the allocation size is increased by the size of 1 structure element.

; CHECK: %mul = mul i64 %some_val, 32
; CHECK: %[[MUL:[0-9]+]] = mul i64 5, %mul
; CHECK: %[[SDIV:[0-9]+]] = sdiv i64 %[[MUL]], 32
; CHECK: %[[ALLOC_COUNT:[0-9]+]] = add i64 %[[SDIV]], 1
; CHECK: %mem = call ptr @calloc(i64 %[[ALLOC_COUNT]], i64 32)

; Verify the fields of the SOA structure are initialized with pointers to the
; address offsets for the memory block, and get adjusted for alignment.

; CHECK: %[[FIELD0_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 0
; CHECK: store ptr %[[FIELD0_ADDR]], ptr @__soa_struct.test01
; CHECK: %[[FIELD0_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 2
; CHECK: %[[FIELD1_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD0_SIZE]], 7
; CHECK: %[[FIELD1_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD1_ALIGN_STEP1]], 8
; CHECK: %[[FIELD1_OFFSET:[0-9]+]] = mul i64 %[[FIELD1_ALIGN_STEP2]], 8
; CHECK: %[[FIELD1_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD1_OFFSET]]
; CHECK: store ptr %[[FIELD1_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1)
; CHECK: %[[FIELD1_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 8
; CHECK: %[[FIELD2_OFFSET:[0-9]+]] = add i64 %[[FIELD1_OFFSET]], %[[FIELD1_SIZE]]
; CHECK: %[[FIELD2_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD2_OFFSET]]
; CHECK: store ptr %[[FIELD2_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2)
; CHECK: %[[FIELD2_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 1
; CHECK: %[[FIELD3_BASE_OFFSET:[0-9]+]] = add i64 %[[FIELD2_OFFSET]], %[[FIELD2_SIZE]]
; CHECK: %[[FIELD3_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD3_BASE_OFFSET]], 7
; CHECK: %[[FIELD3_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD3_ALIGN_STEP1]], 8
; CHECK: %[[FIELD3_OFFSET:[0-9]+]] = mul i64 %[[FIELD3_ALIGN_STEP2]], 8
; CHECK: %[[FIELD3_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD3_OFFSET]]
; CHECK: store ptr %[[FIELD3_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 3)

  ; Allocated pointer is stored to memory to establish the object
  ; as the structure type.
  store ptr %mem, ptr getelementptr (%struct.test01dep, ptr @glob, i64 0, i32 1)
  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i16 0, i32 0}  ; i16
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i32 0, i32 0}  ; i32
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !2, !3, !4} ; { i16, %struct.test01*, i8, i64* }
!9 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !5, !2} ; { i32, %struct.test01* }

!intel.dtrans.types = !{!8, !9}

