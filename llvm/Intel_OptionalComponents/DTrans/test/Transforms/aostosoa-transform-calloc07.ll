; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a calloc call using 32 bit types
; to verify allocation count computations and offset calculations
; support 32-bit targets.

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i16, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i32 9, i32 3)
  ret i32 0
}

define void @test01(i32 %count, i32 %some_val) {
  ; Allocate %count * %some_val * sizeof(struct) elements.
  %mul = mul i32 %some_val, 12
  %mem = call i8* @calloc(i32 %count, i32 %mul)

; Verify the calculation for the current allocation size and number of
; elements allocated; followed by an increment of 1 more structure element.
; CHECK:  %1 = mul i32 %count, %mul
; CHECK:  %2 = sdiv i32 %1, 12
; CHECK:  [[NUM_ALLOC:%[0-9]+]] = add i32 %2, 1
; CHECK:  %mem = call i8* @calloc(i32 [[NUM_ALLOC]], i32 12)

; Verify the new allocation count is being used for computing the addresses
; to store in the peeled structure.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i32 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i16*
; CHECK:  store i16* [[CAST1]], i16** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)

; Padding calculation to find aligned offset is:
;    ((offset + align_req - 1) / align_req) * align_req
; CHECK:  [[SIZE2:%[0-9]+]] = mul i32 [[NUM_ALLOC]], 2
; CHECK:  [[PAD2_TMP1:%[0-9]+]] = add i32 [[SIZE2]], 3
; CHECK:  [[PAD2_TMP2:%[0-9]+]] = sdiv i32 [[PAD2_TMP1]], 4
; CHECK:  [[PAD_OFFSET2:%[0-9]+]] = mul i32 [[PAD2_TMP2]], 4
; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i32 [[PAD_OFFSET2]]
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:  store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)

; CHECK:  [[SIZE3:%[0-9]+]] = mul i32 [[NUM_ALLOC]], 4
; CHECK:  [[OFFSET3:%[0-9]+]] = add i32 [[PAD_OFFSET2]], [[SIZE3]]
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i32 [[OFFSET3]]
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test01*
  ret void
}

declare i8* @calloc(i32, i32)
