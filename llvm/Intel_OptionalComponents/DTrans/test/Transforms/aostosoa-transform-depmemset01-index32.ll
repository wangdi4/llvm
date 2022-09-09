; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies the parameters to memset calls for
; dependent types are updated when the size of the structure
; is modified.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64 }
%struct.test01dep = type { i32, %struct.test01*, i32 }

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

@g_test01dep = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01();
  call void @test02(i64 5);

  ret i32 0
}

; Verify the size argument for the memset gets changed to the new size of
; the structure.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  call void @llvm.memset.p0i8.i64(i8* align 8 bitcast (%struct.test01dep* @g_test01dep to i8*), i8 0, i64 24, i1 false)
; CHECK: call void @llvm.memset.p0i8.i64(i8* align 8 bitcast (%__SOADT_struct.test01dep* @g_test01dep to i8*), i8 0, i64 12, i1 false)

  ret void
}

; Verify the size argument for the memset gets changed to the new size of
; the structure when dealing with a non-constant size. (This test also verifies
; that when an size calculation needs to be cloned due to multiple uses, the
; cloning is done as expected.)
define internal void @test02(i64 %n) {
; CHECK-LABEL: define internal void @test02

  %multiple1 = mul i64 24, %n
; CHECK:  %multiple1.dt = mul i64 12, %n
; CHECK:  %multiple1 = mul i64 12, %n

  %alloc1 = call i8* @malloc(i64 %multiple1)
; CHECK: %alloc1 = call i8* @malloc(i64 %multiple1.dt)

  %dep01_mem = bitcast i8* %alloc1 to %struct.test01dep*
  call void @llvm.memset.p0i8.i64(i8* %alloc1, i8 0, i64 %multiple1, i1 false)
; CHECK: call void @llvm.memset.p0i8.i64(i8* %alloc1, i8 0, i64 %multiple1, i1 false)

  ret void
}

declare i8* @calloc(i64, i64)
declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)
