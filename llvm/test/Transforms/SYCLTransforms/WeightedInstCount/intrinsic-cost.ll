; RUN: opt -mcpu=skx -passes="require<sycl-kernel-weighted-inst-count-analysis>" -debug-only=sycl-kernel-weighted-inst-count-analysis -disable-output %s 2>&1 | FileCheck %s

; Check costs of intrinsic calls.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: [   1] {{.*}} = call float @llvm.fmuladd.f32(float 0.000000e+00, float 0.000000e+00, float 0.000000e+00)

; CHECK: [   1] {{.*}} = call <16 x float> @llvm.fmuladd.v16f32(<16 x float> zeroinitializer, <16 x float> zeroinitializer, <16 x float> zeroinitializer)
; CHECK: [   1] {{.*}} = call <16 x float> @llvm.masked.load.v16f32.p1(ptr addrspace(1) null, i32 1, <16 x i1> zeroinitializer, <16 x float> poison)
; CHECK: [   1]   call void @llvm.masked.store.v16f32.p1(<16 x float> zeroinitializer, ptr addrspace(1) null, i32 1, <16 x i1> zeroinitializer)
; CHECK: [   8] {{.*}} = tail call <16 x i32> @llvm.masked.gather.v16i32.v16p1(<16 x ptr addrspace(1)> zeroinitializer, i32 2, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> poison)
; CHECK: [  22]   call void @llvm.masked.scatter.v16i32.v16p1(<16 x i32> zeroinitializer, <16 x ptr addrspace(1)> zeroinitializer, i32 4, <16 x i1> zeroinitializer)

define void @scalar() {
  %fmuladd = call float @llvm.fmuladd.f32(float 0.000000e+00, float 0.000000e+00, float 0.000000e+00)
  ret void
}

define void @vector() {
entry:
  %fmuladd = call <16 x float> @llvm.fmuladd.v16f32(<16 x float> zeroinitializer, <16 x float> zeroinitializer, <16 x float> zeroinitializer)
  %masked.load = call <16 x float> @llvm.masked.load.v16f32.p1(ptr addrspace(1) null, i32 1, <16 x i1> zeroinitializer, <16 x float> poison)
  call void @llvm.masked.store.v16f32.p1(<16 x float> zeroinitializer, ptr addrspace(1) null, i32 1, <16 x i1> zeroinitializer)
  %masked.gather = tail call <16 x i32> @llvm.masked.gather.v16i32.v16p1(<16 x ptr addrspace(1)> zeroinitializer, i32 2, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> poison)
  call void @llvm.masked.scatter.v16i32.v16p1(<16 x i32> zeroinitializer, <16 x ptr addrspace(1)> zeroinitializer, i32 4, <16 x i1> zeroinitializer)
  ret void
}

declare float @llvm.fmuladd.f32(float, float, float) #0

declare <16 x i32> @llvm.masked.gather.v16i32.v16p1(<16 x ptr addrspace(1)>, i32 immarg, <16 x i1>, <16 x i32>) #1

declare void @llvm.masked.scatter.v16i32.v16p1(<16 x i32>, <16 x ptr addrspace(1)>, i32 immarg, <16 x i1>) #2

declare <16 x float> @llvm.masked.load.v16f32.p1(ptr addrspace(1) nocapture, i32 immarg, <16 x i1>, <16 x float>) #3

declare <16 x float> @llvm.fmuladd.v16f32(<16 x float>, <16 x float>, <16 x float>) #0

declare void @llvm.masked.store.v16f32.p1(<16 x float>, ptr addrspace(1) nocapture, i32 immarg, <16 x i1>) #4

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(read) }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(write) }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #4 = { nocallback nofree nosync nounwind willreturn memory(argmem: write) }
