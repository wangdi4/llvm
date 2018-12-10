; RUN: opt -S -csa-lower-loop-idioms < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_i8_set(i8* %dst, i8 %src, i64 %len) {
  call void @llvm.memset.p0i8.i64(i8* %dst, i8 %src, i64 %len, i1 0)
  ret void
; CHECK-LABEL: test_i8_set
; CHECK: memsetloop:
; CHECK: store i8
}

define void @test_i16_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 1
  call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i16_set
; CHECK: memsetloop:
; CHECK: store i16
}

define void @test_i32_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 2
  call void @llvm.memset.p0i8.i64(i8* align 4 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i32_set
; CHECK: memsetloop:
; CHECK: store i32
}

define void @test_i64_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i64_set
; CHECK: memsetloop:
; CHECK: store i64
}

define void @test_dot32_set(i8* %dst, i8 %src, i32 %len) {
  %shlen = shl nuw i32 %len, 3
  call void @llvm.memset.p0i8.i32(i8* align 8 %dst, i8 %src, i32 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_dot32_set
; CHECK: memsetloop:
; CHECK: store i64
}

define void @test_volatile_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 %shlen, i1 1)
  ret void
; CHECK-LABEL: test_volatile_set
; CHECK: memsetloop:
; CHECK: store volatile i64
}

define void @test_lessmult_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 1
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_lessmult_set
; CHECK: memsetloop:
; CHECK: store i16
}

define void @test_lessaligned_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_lessaligned_set
; CHECK: memsetloop:
; CHECK: store i16
}

define void @test_overaligned_set(i8* %dst, i8 %src, i64 %len) {
  %shlen = shl nuw i64 %len, 4
  call void @llvm.memset.p0i8.i64(i8* align 16 %dst, i8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_overaligned_set
; CHECK: memsetloop:
; CHECK: store i64
}

define void @test_mul_set(i8* %dst, i8 %src, i64 %len) {
  %mulen = mul nuw i64 %len, 6
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 %mulen, i1 0)
  ret void
; CHECK-LABEL: test_mul_set
; CHECK: memsetloop:
; CHECK: store i16
}

define void @test_liti8_set(i8* %dst, i64 %len) {
  call void @llvm.memset.p0i8.i64(i8* %dst, i8 175, i64 %len, i1 0)
  ret void
; CHECK-LABEL: test_liti8_set
; CHECK: memsetloop:
; CHECK: store i8 -81
}

define void @test_liti16_set(i8* %dst, i64 %len) {
  %shlen = shl nuw i64 %len, 1
  call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 175, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_liti16_set
; CHECK: memsetloop:
; CHECK: store i16 -20561
}

define void @test_liti32_set(i8* %dst, i64 %len) {
  %shlen = shl nuw i64 %len, 2
  call void @llvm.memset.p0i8.i64(i8* align 4 %dst, i8 175, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_liti32_set
; CHECK: memsetloop:
; CHECK: store i32 -1347440721
}

define void @test_liti64_set(i8* %dst, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 175, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_liti64_set
; CHECK: memsetloop:
; CHECK: store i64 -5787213827046133841
}

define void @test_litlen_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 100, i1 0)
  ret void
; CHECK-LABEL: test_litlen_set
; CHECK: memsetloop:
; CHECK: store i32
}

define void @test_giant_set(i8* %dst, i8 %src) {
  call void @llvm.memset.p0i8.i64(i8* align 8 %dst, i8 %src, i64 4503599627370496, i1 0)
  ret void
; CHECK-LABEL: test_giant_set
; CHECK: memsetloop:
; CHECK: store i64
}

declare void @llvm.memset.p0i8.i32(i8*, i8, i32, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
