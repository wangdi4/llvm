; RUN: opt -S -csa-lower-loop-idioms -csa-max-stores-per-memintr=8 < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_byte_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_byte_move
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: load i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
; CHECK: store i8
}

define void @test_short_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 2 %dst, i8* align 2 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_short_move
; CHECK: load i16
; CHECK: load i16
; CHECK: load i16
; CHECK: load i16
; CHECK: store i16
; CHECK: store i16
; CHECK: store i16
; CHECK: store i16
}

define void @test_int_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 4 %dst, i8* align 4 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_int_move
; CHECK: load i32
; CHECK: load i32
; CHECK: store i32
; CHECK: store i32
}

define void @test_long_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 16, i1 0)
  ret void
; CHECK-LABEL: test_long_move
; CHECK: load i64
; CHECK: load i64
; CHECK: store i64
; CHECK: store i64
}

define void @test_diffalign_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 4 %src, i64 8, i1 0)
  ret void
; CHECK-LABEL: test_diffalign_move
; CHECK: load i32
; CHECK: load i32
; CHECK: store i32
; CHECK: store i32
}

define void @test_overalign_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 16 %dst, i8* align 16 %src, i64 16, i1 0)
  ret void
; CHECK-LABEL: test_overalign_move
; CHECK: load i64
; CHECK: load i64
; CHECK: store i64
; CHECK: store i64
}

define void @test_npot_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 15, i1 0)
  ret void
; CHECK-LABEL: test_npot_move
; CHECK: load i64
; CHECK: load i32
; CHECK: load i16
; CHECK: load i8
; CHECK: store i64
; CHECK: store i32
; CHECK: store i16
; CHECK: store i8
}

define void @test_small_npot_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 6, i1 0)
  ret void
; CHECK-LABEL: test_small_npot_move
; CHECK: load i32
; CHECK: load i16
; CHECK: store i32
; CHECK: store i16
}

define void @test_doti32_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i32(i8* align 8 %dst, i8* align 8 %src, i32 16, i1 0)
  ret void
; CHECK-LABEL: test_doti32_move
; CHECK: load i64
; CHECK: load i64
; CHECK: store i64
; CHECK: store i64
}

define void @test_volatile_move(i8* %dst, i8* %src) {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 16, i1 1)
  ret void
; CHECK-LABEL: test_volatile_move
; CHECK: load volatile i64
; CHECK: load volatile i64
; CHECK: store volatile i64
; CHECK: store volatile i64
}

declare void @llvm.memmove.p0i8.p0i8.i32(i8*, i8*, i32, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)
