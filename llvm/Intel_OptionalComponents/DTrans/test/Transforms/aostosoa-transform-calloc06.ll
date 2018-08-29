; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a calloc call when the parameters
; are variables, but one of them is a multiple of the structure size.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 9, i64 3)
  ret i32 0
}

define void @test01(i64 %count, i64 %some_val) {
  ; Allocate %count * %some_val * sizeof(struct) elements.
  %mul = mul i64 %some_val, 12
  %mem = call i8* @calloc(i64 %count, i64 %mul)

; Verify the calculation for the current allocation size and number of
; elements allocated; followed by an increment of 1 more structure element.
; CHECK:  %1 = mul i64 %count, %mul
; CHECK:  %2 = sdiv i64 %1, 12
; CHECK:  [[NUM_ALLOC:%[0-9]+]] = add i64 %2, 1
; CHECK:  %mem = call i8* @calloc(i64 [[NUM_ALLOC]], i64 12)

; Verify the new allocation count is being used for computing the addresses
; to store in the peeled structure.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:  store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)
; CHECK:  [[OFFSET2:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 4
; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[OFFSET2]]
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:  store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)
; CHECK:  [[SIZE3:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 4
; CHECK:  [[OFFSET3:%[0-9]+]] = add i64 [[OFFSET2]], [[SIZE3]]
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[OFFSET3]]
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test01*
  ret void
}

declare i8* @calloc(i64, i64)
