; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation for a malloc call with a constant size that is
; a multiple of the structure size, when using a structure that has no padding
; between elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate 10 elements.
  %mem = call i8* @malloc(i64 120)

; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:   %mem = call i8* @malloc(i64 132)

; Verify that the array elements of the peeled structure are initialized
; to point to the expected addresses within the allocated block.
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:  [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:  store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)
; CHECK:  [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 44
; CHECK:  [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:  store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)
; CHECK:  [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 88
; CHECK:  [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test01*
  ret i32 0
}

declare i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
