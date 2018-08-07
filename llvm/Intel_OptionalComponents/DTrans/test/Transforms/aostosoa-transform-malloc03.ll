; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a malloc call with a constant size that
; is a multiple of the structure size, when using a structure that has padding
; between elements. In this case some padding may be needed for the computation
; of the middle array in the structure of arrays.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; These are the data structures the test is going to transform.
%struct.test01 = type { i32, i64, i32 }
%struct.test02 = type { i32, i64, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  call void @test02()
  ret i32 0
}

; Test a case where padding does not need to be inserted
define void @test01() {
; CHECK-LABEL: define internal void @test01

%mem = call i8* @malloc(i64 120)
; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:   %mem = call i8* @malloc(i64 144)

; Verify that the array elements of the peeled structure are initialized
; to point to the expected addresses within the allocated block.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:  store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)

; Padding is not inserted because there are six 4-byte elements for the
; first array.

; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 24
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i64*
; CHECK:  store i64* [[CAST2]], i64** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 72
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test01*
  ret void
}

; Test a case where padding does need to be inserted
define void @test02() {
; CHECK-LABEL: define internal void @test02

  %mem = call i8* @malloc(i64 96)
; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:   %mem = call i8* @malloc(i64 120)

; Verify that the array elements of the peeled structure are initialized
; to point to the expected addresses within the allocated block.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:  store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test02, %__SOA_struct.test02* @__soa_struct.test02, i64 0, i32 0)

; Padding is inserted here because there are five 4-byte elements for the
; first array.

; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 24
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i64*
; CHECK:  store i64* [[CAST2]], i64** getelementptr inbounds (%__SOA_struct.test02, %__SOA_struct.test02* @__soa_struct.test02, i64 0, i32 1)
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 64
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test02, %__SOA_struct.test02* @__soa_struct.test02, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test02*
  ret void
}

declare i8* @malloc(i64)
