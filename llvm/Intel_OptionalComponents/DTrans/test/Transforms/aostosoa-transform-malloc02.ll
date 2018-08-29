; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a malloc call with a non-constant size
; that is a multiple of the structure size, when using a structure that has
; no padding between elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

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
define void @test01(i64 %num) {
  ; Allocate 3 times the number of incoming parameter. (structure size = 12)
  %size = mul nsw i64 %num, 36
  %mem = call i8* @malloc(i64 %size)

; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:  %1 = sdiv i64 %size, 12
; CHECK:  [[NUM_ALLOC:%[0-9]+]] = add i64 %1, 1
; CHECK:  %3 = mul i64 %2, 12
; CHECK:  %mem = call i8* @malloc(i64 %3)

; Verify the array elements of the peeled structure are initialized
; to point to the expected addresses within the allocated block.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:  store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)

; The next array should begin at: NumAllocated * sizeof(i32)
; CHECK:  [[OFFSET1:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 4
; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[OFFSET1]]
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:  store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)

; The next array should increment past the previous array
; CHECK:  [[SIZE2:%[0-9]+]] = mul i64 [[NUM_ALLOC]], 4
; CHECK:  [[OFFSET2:%[0-9]+]] = add i64 [[OFFSET1]], [[SIZE2]]
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 [[OFFSET2]]
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  store i8* %mem, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)  to i8**)
  ret void
}

declare i8* @malloc(i64)
