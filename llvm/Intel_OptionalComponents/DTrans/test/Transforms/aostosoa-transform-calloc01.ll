; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a calloc call with a constant size
; parameter equal to the structure size, and a constant number of elements
; being allocated.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate 10 elements. (structure size = 12)
  %mem = call i8* @calloc(i64 10, i64 12)

; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:   %mem = call i8* @calloc(i64 11, i64 12)

; CHECK:   [[ADDR1:%[0-9]+]] = getelementptr i8, i8* %mem, i64 0
; CHECK:   [[CAST1:%[0-9]+]] = bitcast i8* [[ADDR1]] to i32*
; CHECK:   store i32* [[CAST1]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0)
; CHECK:   [[ADDR2:%[0-9]+]] = getelementptr i8, i8* %mem, i64 44
; CHECK:   [[CAST2:%[0-9]+]] = bitcast i8* [[ADDR2]] to i32*
; CHECK:   store i32* [[CAST2]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1)
; CHECK:   [[ADDR3:%[0-9]+]] = getelementptr i8, i8* %mem, i64 88
; CHECK:   [[CAST3:%[0-9]+]] = bitcast i8* [[ADDR3]] to i32*
; CHECK:  store i32* [[CAST3]], i32** getelementptr inbounds (%__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2)

  %st = bitcast i8* %mem to %struct.test01*
  ret i32 0
}

declare i8* @calloc(i64, i64)
