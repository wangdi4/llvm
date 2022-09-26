; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies the parameters to malloc memory allocations
; for dependent types when ptr-to-ptr is used for types. In these
; cases the allocation sizes should NOT be changed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64 }
%struct.test01dep = type { i32, %struct.test01*, i32 }
%struct.test02dep = type { i32, %struct.test01** }

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01();
  call void @test02();

  ret i32 0
}

; Verify the size argument for the allocation of the dependent type
; does NOT get updated for the malloc.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  %alloc = call i8* @malloc(i64 8)
; CHECK: %alloc = call i8* @malloc(i64 8)

  %dep01_mem = bitcast i8* %alloc to %struct.test01dep**
  ret void
}

; Verify the size argument for the allocation of the dependent type
; does NOT get updated for the malloc.
define internal void @test02() {
; CHECK-LABEL: define internal void @test02

  %alloc = call i8* @malloc(i64 16)
; CHECK: %alloc = call i8* @malloc(i64 16)

  %dep02_mem = bitcast i8* %alloc to %struct.test02dep*
  ret void
}

declare i8* @calloc(i64, i64) #0
declare i8* @malloc(i64) #1

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
