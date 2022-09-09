; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies special cases where GetElementPointer
; instructions need to be transformed during the AOS to SOA
; transformation for cases where the GEP indices are not
; the same size as a pointer when using a 32-bit peeling
; index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }

; This structure will be used for verifying accesses within dependent data
; structures get handled.
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*
  call void @test01(i32 1)
  ret i32 0
}

; These test transformations of GEP instructions that require
; conversions prior to using the GEP index.
define internal void @test01(i32 %idx1) {
; CHECK-LABEL: define internal void @test01(i32 %idx1)

  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %base = load i32, i32* @g_test01ptr

  ; Test with GEP index type that matches the peeling type, so does
  ; not require truncation.
  %array_elem1 = getelementptr inbounds %struct.test01, %struct.test01* %base, i32 %idx1
; CHECK:  %array_elem1 = add i32 %base, %idx1

  ; Test with GEP that accesses a field element where a GEP index matches
  ; the peeling type, so does not require truncation.
  %y_addr = getelementptr inbounds %struct.test01, %struct.test01* %base, i32 %idx1, i32 1
  %y_val = load i32, i32* %y_addr
; CHECK:  [[GLOB_ADDR2:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[ARRAY_ADDR2:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR2]]
; CHECK:  [[ELEM_ADDR2:%[0-9]+]] = add i32 %base, %idx1
; CHECK:  [[ZELEM_ADDR2:%[0-9]+]] = zext i32 [[ELEM_ADDR2]] to i64
; CHECK:  %y_addr = getelementptr i32, i32* [[ARRAY_ADDR2]], i64 [[ZELEM_ADDR2]]
; CHECK:  %y_val = load i32, i32* %y_addr

  ret void
}

declare i8* @calloc(i64, i64)
