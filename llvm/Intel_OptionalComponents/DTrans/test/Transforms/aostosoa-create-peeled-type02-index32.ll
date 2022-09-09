; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01,struct.test02 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01,struct.test02 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies the basic functionality for creating a peeled type
; for the AOS to SOA transformation to verify that the transformation
; works when there are multiple types to be converted that are
; dependent on each other, when using 32-bit indexes for the peeling index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, %struct.test01*, %struct.test02* }
%struct.test02 = type { i32*, %struct.test02*, %struct.test01* }

; CHECK-DAG: %__SOA_struct.test01 = type { i64*, i32*, i32* }
; CHECK-DAG: %__SOA_struct.test02 = type { i32**, i32*, i32* }
; CHECK-DAG: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
; CHECK-DAG: @__soa_struct.test02 = internal global %__SOA_struct.test02 zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 24)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 10, i64 24)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test02*

  call void @test01();
  ret i32 0
}

define internal void @test01() {
; We need some use of the type so that opt will generate it in the output.
  %local1 = alloca %struct.test01*
  %local2 = alloca %struct.test02*
  ret void
}
; Verify that a scalar integer is now used for the local.
; CHECK-LABEL: define internal void @test01()
; CHECK:   %local1 = alloca i32
; CHECK:   %local2 = alloca i32

declare i8* @calloc(i64, i64)
