; REQUIRES: asserts
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Basic test of the AOS-to-SOA conversion for an allocation of a
; variable number of elements. Also, the field initialization needs
; to take into account element alignment when initializing the structure
; of arrays because the original structure contained padding between
; elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i16, %struct.test01*, i8, i64* }
%struct.test01dep = type { i32, %struct.test01* }
@glob = internal global %struct.test01dep zeroinitializer

define i32 @main() {
  call void @test01(i64 3)
  ret i32 0
}

; Test conversion of calloc call.
define void @test01(i64 %some_val) {
; CHECK-LABEL: define void @test01
  %mul = mul i64 %some_val, 32
  %mem = call i8* @calloc(i64 5, i64 %mul)

; Verify the allocation size is increased by the size of 1 structure element.
; CHECK-NONOPAQUE: %mul = mul i64 %some_val, 32
; CHECK-NONOPAQUE: %[[MUL:[0-9]+]] = mul i64 5, %mul
; CHECK-NONOPAQUE: %[[SDIV:[0-9]+]] = sdiv i64 %[[MUL]], 32
; CHECK-NONOPAQUE: %[[ALLOC_COUNT:[0-9]+]] = add i64 %[[SDIV]], 1
; CHECK-NONOPAQUE: %mem = call i8* @calloc(i64 %[[ALLOC_COUNT]], i64 32)

; CHECK-OPAQUE: %mul = mul i64 %some_val, 32
; CHECK-OPAQUE: %[[MUL:[0-9]+]] = mul i64 5, %mul
; CHECK-OPAQUE: %[[SDIV:[0-9]+]] = sdiv i64 %[[MUL]], 32
; CHECK-OPAQUE: %[[ALLOC_COUNT:[0-9]+]] = add i64 %[[SDIV]], 1
; CHECK-OPAQUE: %mem = call ptr @calloc(i64 %[[ALLOC_COUNT]], i64 32)

; Verify the fields of the SOA structure are initialized with pointers to the
; address offsets for the memory block, and get adjusted for alignment.
; CHECK-NONOPAQUE: %[[FIELD0_ADDR:[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK-NONOPAQUE: %[[FIELD0_ADDR_CAST:[0-9]+]] = bitcast i8* %[[FIELD0_ADDR]] to i16*
; CHECK-NONOPAQUE: store i16* %[[FIELD0_ADDR_CAST]], i16** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0), align 8
; CHECK-NONOPAQUE: %[[FIELD0_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 2
; CHECK-NONOPAQUE: %[[FIELD1_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD0_SIZE]], 7
; CHECK-NONOPAQUE: %[[FIELD1_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD1_ALIGN_STEP1]], 8
; CHECK-NONOPAQUE: %[[FIELD1_OFFSET:[0-9]+]] = mul i64 %[[FIELD1_ALIGN_STEP2]], 8
; CHECK-NONOPAQUE: %[[FIELD1_ADDR:[0-9]+]] = getelementptr i8, i8* %mem, i64 %[[FIELD1_OFFSET]]
; CHECK-NONOPAQUE: %[[FIELD1_ADDR_CAST:[0-9]+]] = bitcast i8* %[[FIELD1_ADDR]] to i64*
; CHECK-NONOPAQUE: store i64* %[[FIELD1_ADDR_CAST]], i64** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1), align 8
; CHECK-NONOPAQUE: %[[FIELD1_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 8
; CHECK-NONOPAQUE: %[[FIELD2_OFFSET:[0-9]+]] = add i64 %[[FIELD1_OFFSET]], %[[FIELD1_SIZE]]
; CHECK-NONOPAQUE: %[[FIELD2_ADDR:[0-9]+]] = getelementptr i8, i8* %mem, i64 %[[FIELD2_OFFSET]]
; CHECK-NONOPAQUE: store i8* %[[FIELD2_ADDR]], i8** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2), align 8
; CHECK-NONOPAQUE: %[[FIELD2_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 1
; CHECK-NONOPAQUE: %[[FIELD3_BASE_OFFSET:[0-9]+]] = add i64 %[[FIELD2_OFFSET]], %[[FIELD2_SIZE]]
; CHECK-NONOPAQUE: %[[FIELD3_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD3_BASE_OFFSET]], 7
; CHECK-NONOPAQUE: %[[FIELD3_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD3_ALIGN_STEP1]], 8
; CHECK-NONOPAQUE: %[[FIELD3_OFFSET:[0-9]+]] = mul i64 %[[FIELD3_ALIGN_STEP2]], 8
; CHECK-NONOPAQUE: %[[FIELD3_ADDR:[0-9]+]] = getelementptr i8, i8* %mem, i64 %[[FIELD3_OFFSET]]
; CHECK-NONOPAQUE: %[[FIELD3_ADDR_CAST:[0-9]+]] = bitcast i8* %[[FIELD3_ADDR]] to i64**
; CHECK-NONOPAQUE: store i64** %[[FIELD3_ADDR_CAST:[0-9]+]], i64*** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 3), align 8

; CHECK-OPAQUE: %[[FIELD0_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 0
; CHECK-OPAQUE: store ptr %[[FIELD0_ADDR]], ptr @__soa_struct.test01
; CHECK-OPAQUE: %[[FIELD0_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 2
; CHECK-OPAQUE: %[[FIELD1_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD0_SIZE]], 7
; CHECK-OPAQUE: %[[FIELD1_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD1_ALIGN_STEP1]], 8
; CHECK-OPAQUE: %[[FIELD1_OFFSET:[0-9]+]] = mul i64 %[[FIELD1_ALIGN_STEP2]], 8
; CHECK-OPAQUE: %[[FIELD1_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD1_OFFSET]]
; CHECK-OPAQUE: store ptr %[[FIELD1_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1)
; CHECK-OPAQUE: %[[FIELD1_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 8
; CHECK-OPAQUE: %[[FIELD2_OFFSET:[0-9]+]] = add i64 %[[FIELD1_OFFSET]], %[[FIELD1_SIZE]]
; CHECK-OPAQUE: %[[FIELD2_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD2_OFFSET]]
; CHECK-OPAQUE: store ptr %[[FIELD2_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2)
; CHECK-OPAQUE: %[[FIELD2_SIZE:[0-9]+]] = mul i64 %[[ALLOC_COUNT]], 1
; CHECK-OPAQUE: %[[FIELD3_BASE_OFFSET:[0-9]+]] = add i64 %[[FIELD2_OFFSET]], %[[FIELD2_SIZE]]
; CHECK-OPAQUE: %[[FIELD3_ALIGN_STEP1:[0-9]+]] = add i64 %[[FIELD3_BASE_OFFSET]], 7
; CHECK-OPAQUE: %[[FIELD3_ALIGN_STEP2:[0-9]+]] = sdiv i64 %[[FIELD3_ALIGN_STEP1]], 8
; CHECK-OPAQUE: %[[FIELD3_OFFSET:[0-9]+]] = mul i64 %[[FIELD3_ALIGN_STEP2]], 8
; CHECK-OPAQUE: %[[FIELD3_ADDR:[0-9]+]] = getelementptr i8, ptr %mem, i64 %[[FIELD3_OFFSET]]
; CHECK-OPAQUE: store ptr %[[FIELD3_ADDR]], ptr getelementptr inbounds (%__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 3)

  ; Allocated pointer is stored to memory to establish the object
  ; as the structure type.
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @glob, i64 0, i32 1)
  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

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

