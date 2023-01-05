; RUN: opt -passes="gvn" -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @llvm.masked.scatter.v2i32(<2 x i32>, <2 x i32*>, i32, <2 x i1>)

define i32 @test1() {
; CHECK-LABEL: @test1(
; CHECK: ret i32 %0
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 0
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %x, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 true, i1 true>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}

define i32 @test2() {
; CHECK-LABEL: @test2(
; CHECK: ret i32 undef
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 0
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %x, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 false, i1 false>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}

define i32 @test3() {
; CHECK-LABEL: @test3(
; CHECK: ret i32 %0
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %y = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 0
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %y, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 true, i1 false>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}

define i32 @test4() {
; CHECK-LABEL: @test4(
; CHECK: ret i32 undef
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %y = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 0
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %y, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 false, i1 true>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}

define i32 @test5() {
; CHECK-LABEL: @test5(
; CHECK: ret i32 undef
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %y = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 2
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %y, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 true, i1 true>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}

define i32 @test6(i32 %index) {
; CHECK-LABEL: @test6(
; CHECK: ret i32 %0
entry:
  %x = alloca [100 x [100 x i32]], align 16
  %y = alloca [100 x [100 x i32]], align 16
  %.splatinsert23 = insertelement <2 x [100 x [100 x i32]]*> undef, [100 x [100 x i32]]* %x, i32 %index
  %.splat24 = insertelement <2 x [100 x [100 x i32]]*> %.splatinsert23, [100 x [100 x i32]]* %y, i32 1
  %arrayIdx = getelementptr inbounds [100 x [100 x i32]], <2 x [100 x [100 x i32]]*> %.splat24, <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 2>, <2 x i64> <i64 1, i64 1>
  call void @llvm.masked.scatter.v2i32(<2 x i32> <i32 1, i32 1>, <2 x i32*> %arrayIdx, i32 0, <2 x i1> <i1 true, i1 true>)
  %arrayidx50 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %x, i64 0, i64 1, i64 1
  %0 = load i32, i32* %arrayidx50, align 16
  ret i32 %0
}
