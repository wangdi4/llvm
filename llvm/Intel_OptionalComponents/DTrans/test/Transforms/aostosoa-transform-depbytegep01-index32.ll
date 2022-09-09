; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies that the offsets used in byte-flattened GEPs
; are updated on dependent types when the size of the structure
; is modified.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64 }
%struct.test01dep = type { i32, %struct.test01*, i32 }

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01();
  ret i32 0
}

; Verify the offsets in the byte-gep instructions get updated.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  ; Allocate a structure.
  %p = call i8* @malloc(i64 24)
  %p_test = bitcast i8* %p to %struct.test01dep*

  ; Verify the offsets get updated for the byte-flattened GEPs
  %p8_B = getelementptr i8, i8* %p, i64 8
  %p8_C = getelementptr i8, i8* %p, i64 16
; CHECK:  %p8_B = getelementptr i8, i8* %p, i64 4
; CHECK:  %p8_C = getelementptr i8, i8* %p, i64 8

  %p_test_A = bitcast i8* %p to i32*
  %p_test_B = bitcast i8* %p8_B to %struct.test01**
  %p_test_C = bitcast i8* %p8_C to i32*

  ; Write to each field
  store i32 1, i32* %p_test_A
  store %struct.test01* null, %struct.test01** %p_test_B
  store i32 2, i32* %p_test_C

  ret void
}

declare i8* @calloc(i64, i64)
declare i8* @malloc(i64)
