; UNSUPPORTED: enable-opaque-pointers
; RUN: opt <%s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt <%s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies that load instructions used for the address of the
; array pointers in the structure of arrays are marked as invariant.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i64, %struct.test01* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

; Test with accesses to each field to verify the array address is marked as
; invariant. They should all refer to a metadata node with no elements.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %base = load i32, i32* @g_test01ptr

  %x_addr = getelementptr %struct.test01, %struct.test01* %base, i64 0, i32 0
  store i32 20, i32* %x_addr
; CHECK:  [[GLOB_ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[ARRAY_ADDR1:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR1]], align 8, !invariant.load !0
; CHECK:  [[ZELEM_ZDDR1:%[0-9]+]] = zext i32 %base to i64
; CHECK:  %x_addr = getelementptr i32, i32* [[ARRAY_ADDR1]], i64 [[ZELEM_ZDDR1]]
; CHECK:  store i32 20, i32* %x_addr

  %z_addr = getelementptr %struct.test01, %struct.test01* %base, i64 0, i32 1
  store i64 40, i64* %z_addr
; CHECK:  [[GLOB_ADDR2:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[ARRAY_ADDR2:%[0-9]+]] = load i64*, i64** [[GLOB_ADDR2]], align 8, !invariant.load !0
; CHECK:  [[ZELEM_ZDDR2:%[0-9]+]] = zext i32 %base to i64
; CHECK:  %z_addr = getelementptr i64, i64* [[ARRAY_ADDR2]], i64 [[ZELEM_ZDDR2]]
; CHECK:  i64 40, i64* %z_addr

  ; Test with peeling index which was converted to be an i32.
  %ptr_addr = getelementptr %struct.test01, %struct.test01* %base, i64 0, i32 2
  %ptr = load %struct.test01*, %struct.test01** %ptr_addr
; CHECK:  [[GLOB_ADDR3:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[ARRAY_ADDR3:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR3]], align 8, !invariant.load !0
; CHECK:  [[ZELEM_ZDDR3:%[0-9]+]] = zext i32 %base to i64
; CHECK:  %ptr_addr = getelementptr i32, i32* [[ARRAY_ADDR3]], i64 [[ZELEM_ZDDR3]]
; CHECK:  %ptr = load i32, i32* %ptr_addr

  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 24)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01()
  ret i32 0
}

; CHECK: !0 = !{}

declare i8* @calloc(i64, i64)
