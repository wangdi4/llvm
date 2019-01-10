; RUN: opt -S -csa-lower-loop-idioms -csa-max-stores-per-memintr=8 < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_i8_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* %dst, i8 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_i8_set
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
}

define void @test_i16_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_i16_set
; CHECK: store i16
; CHECK: store i16
; CHECK: store i16
; CHECK: store i16
}

define void @test_i32_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 4 %dst, i8 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_i32_set
; CHECK: store i32
; CHECK: store i32
}

define void @test_i64_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 16, i1 0)
  ret void
; CHECK-LABEL: test_i64_set
; CHECK: store i64
; CHECK: store i64
}

define void @test_npot_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 15, i1 0)
  ret void
; CHECK-LABEL: test_npot_set
; CHECK: store i64
; CHECK: store i32
; CHECK: store i16
; CHECK: store i8
}

define void @test_small_npot_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 6, i1 0)
  ret void
; CHECK-LABEL: test_small_npot_set
; CHECK: store i32
; CHECK: store i16
}

define void @test_literal_set(i8* %dst) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 175, i64 15, i1 0)
  ret void
; CHECK-LABEL: test_literal_set
; CHECK: store i64 -5787213827046133841
; CHECK: store i32 -1347440721
; CHECK: store i16 -20561
; CHECK: store i8 -81
}

define void @test_doti32_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i32(i8* align 8 %dst, i8 %src, i32 16, i1 0)
  ret void
; CHECK-LABEL: test_doti32_set
; CHECK: store i64
; CHECK: store i64
}

define void @test_volatile_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 16, i1 1)
  ret void
; CHECK-LABEL: test_volatile_set
; CHECK: store volatile i64
; CHECK: store volatile i64
}

declare void @llvm.memset.p0i8.i32(i8*, i8, i32, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
