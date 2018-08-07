; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a malloc call using 32 bit types
; to verify allocation count computations and offset calculations
; support 32-bit targets.

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i16, i32, i32 }

; This structure is where the pointer to the allocated memory is
; going to be stored.
%struct.test01dep = type { i16, %struct.test01* }

; Container that holds a pointer to the type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i32 15)
  ret i32 0
}

; Test malloc with non-const size that is some multiple of the structure
; size.
define void @test01(i32 %num) {
  ; Allocate %num elements. (structure size = 12)
  %size = mul nsw i32 %num, 12
  %mem = call i8* @malloc(i32 %size)
; Verify the number of allocated elements is increased by 1 structure size.
; CHECK:  %1 = sdiv i32 %size, 12
; CHECK:  [[NUM_ALLOC:%[0-9]+]] = add i32 %1, 1
; CHECK:  %3 = mul i32 %2, 12
; CHECK:  %mem = call i8* @malloc(i32 %3)

; Verify the new allocation count is being used for computing the addresses
; to store in the peeled structure.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i32 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i16*
; CHECK:  store i16* [[CAST1]], i16** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)

; This field needs padding:
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

declare i8* @malloc(i32)
