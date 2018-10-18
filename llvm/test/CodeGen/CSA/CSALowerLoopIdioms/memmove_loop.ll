; RUN: opt -S -csa-lower-loop-idioms < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_i8_move(i8* %dst, i8* %src, i64 %len) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 %len, i1 0)
  ret void
; CHECK-LABEL: test_i8_move
; CHECK: memmoveloop:
; CHECK: load i8
; CHECK: store i8
}

define void @test_i16_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 1
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 2 %dst, i8* align 2 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i16_move
; CHECK: memmoveloop:
; CHECK: load i16
; CHECK: store i16
}

define void @test_i32_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 2
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 4 %dst, i8* align 4 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i32_move
; CHECK: memmoveloop:
; CHECK: load i32
; CHECK: store i32
}

define void @test_i64_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_i64_move
; CHECK: memmoveloop:
; CHECK: load i64
; CHECK: store i64
}

define void @test_dot32_move(i8* %dst, i8* %src, i32 %len) {
  %shlen = shl nuw i32 %len, 3
  call void @llvm.memmove.p0i8.p0i8.i32(i8* align 8 %dst, i8* align 8 %src, i32 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_dot32_move
; CHECK: memmoveloop:
; CHECK: load i64
; CHECK: store i64
}

define void @test_volatile_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 %shlen, i1 1)
  ret void
; CHECK-LABEL: test_volatile_move
; CHECK: memmoveloop:
; CHECK: load volatile i64
; CHECK: store volatile i64
}

define void @test_lessmult_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 1
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_lessmult_move
; CHECK: memmoveloop:
; CHECK: load i16
; CHECK: store i16
}

define void @test_lessaligned_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 3
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 2 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_lessaligned_move
; CHECK: memmoveloop:
; CHECK: load i16
; CHECK: store i16
}

define void @test_overaligned_move(i8* %dst, i8* %src, i64 %len) {
  %shlen = shl nuw i64 %len, 4
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 16 %dst, i8* align 16 %src, i64 %shlen, i1 0)
  ret void
; CHECK-LABEL: test_overaligned_move
; CHECK: memmoveloop:
; CHECK: load i64
; CHECK: store i64
}

define void @test_mul_move(i8* %dst, i8* %src, i64 %len) {
  %mulen = mul nuw i64 %len, 6
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 %mulen, i1 0)
  ret void
; CHECK-LABEL: test_mul_move
; CHECK: memmoveloop:
; CHECK: load i16
; CHECK: store i16
}

define void @test_literal_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 100, i1 0)
  ret void
; CHECK-LABEL: test_literal_move
; CHECK: memmoveloop:
; CHECK: load i32
; CHECK: store i32
}

define void @test_giant_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 4503599627370496, i1 0)
  ret void
; CHECK-LABEL: test_giant_move
; CHECK: memmoveloop:
; CHECK: load i64
; CHECK: store i64
}

declare void @llvm.memmove.p0i8.p0i8.i32(i8*, i8*, i32, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)
