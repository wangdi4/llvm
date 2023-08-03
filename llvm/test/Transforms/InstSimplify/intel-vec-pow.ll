; RUN: opt -S -passes=instsimplify %s | FileCheck %s

; InstSimplify was expecting "powi" to have the same vector/scalar attribute
; on base and exponent.

; CHECK: call{{.*}}powi.v8f32.i32{{.*}}%arg2, i32 3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.powi.f32.i32(float, i32) #5

define fastcc <8 x i1> @module_bl_mynn_mp_mynn_bl_driver_(<8 x float> %arg, <8 x float> %arg2) #6 {
entry:
  %0 = call reassoc ninf nsz arcp contract afn <8 x float> @llvm.powi.v8f32.i32(<8 x float> %arg2, i32 3) #12
  %1 = fmul reassoc ninf nsz arcp contract afn <8 x float> %arg, %0
  %2 = fcmp oge <8 x float> %1, zeroinitializer
  ret <8 x i1> %2
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <8 x float> @llvm.powi.v8f32.i32(<8 x float>, i32) #5

attributes #5 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { "unsafe-fp-math"="true" }
