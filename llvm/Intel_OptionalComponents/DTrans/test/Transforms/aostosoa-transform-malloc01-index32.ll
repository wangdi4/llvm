; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a malloc call with a non-constant size
; that is a multiple of the structure size, when using a structure that has
; padding between elements, and contains remapped types, including a pointer
; to the type being converted to verify when 32-bit indexing is enabled
; the index is handled as a 32-bit index, instead of a 64-bit index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i16, %struct.test01*, %struct.test01dep* }

; This structure is where the pointer to the allocated memory is
; going to be stored.
%struct.test01dep = type { i16, %struct.test01* }

; Container that holds a pointer to the type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15)
  ret i32 0
}

; Test malloc with non-const size that is some multiple of the structure
; size.
define internal void @test01(i64 %num) {
; CHECK-LABEL: define internal void @test01

  ; Allocate %num elements. (structure size = 24)
  %size = mul nsw i64 %num, 24
  %mem = call i8* @malloc(i64 %size)

; Verify the number of allocated elements is increased by 1 structure size.
; CHECK:  %1 = sdiv i64 %size, 24
; CHECK:  [[NUM_ALLOC:%[0-9]+]] = add i64 %1, 1
; CHECK:  %3 = mul i64 %2, 24
; CHECK:  %mem = call i8* @malloc(i64 %3)

; Verify the array elements of the peeled structure are initialized
; to point to the expected addresses within the allocated block.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i16*
; CHECK:  store i16* [[CAST1]], i16** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)

; The next array should begin at: NumAllocated * sizeof(i16) + padding
; Padding calculation to find aligned offset is:
;    ((offset + align_req - 1) / align_req) * align_req
; CHECK:  [[SIZE2:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 2
; CHECK:  [[PAD2_TMP1:%[0-9]+]] = add i64 [[SIZE2]], 3
; CHECK:  [[PAD2_TMP2:%[0-9]+]] = sdiv i64 [[PAD2_TMP1]], 4
; CHECK:  [[OFFSET2:%[0-9]+]] = mul i64 [[PAD2_TMP2]], 4
; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[OFFSET2]]
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:  store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)

; Increment the offset by: NumAllocated * sizeof(i32*). No padding required.
; CHECK:  [[SIZE3:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 4
; CHECK:  [[OFFSET3:%[0-9]+]] = add i64 [[OFFSET2]], [[SIZE3]]
; CHECK:  [[PAD3_TMP1:%[0-9]+]] = add i64 [[OFFSET3]], 7
; CHECK:  [[PAD3_TMP2:%[0-9]+]] = sdiv i64 [[PAD3_TMP1]], 8
; CHECK:  [[PAD_OFFSET3:%[0-9]+]] = mul i64 [[PAD3_TMP2]], 8
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[PAD_OFFSET3]]
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to %__SOADT_struct.test01dep**
; CHECK:  store %__SOADT_struct.test01dep** [[CAST3]], %__SOADT_struct.test01dep*** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %field_addr = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1
  %struct_ptr = bitcast i8* %mem to %struct.test01*
  store  %struct.test01* %struct_ptr,  %struct.test01** %field_addr
; CHECK: store i32 1, i32* %field_addr

  ret void
}

declare i8* @malloc(i64)
