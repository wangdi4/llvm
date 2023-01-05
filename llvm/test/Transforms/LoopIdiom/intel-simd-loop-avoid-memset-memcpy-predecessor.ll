; Avoid memset/memcpy recognition in OMP SIMD loops in cases when SIMD directive is in one of loop predecessors.
; RUN: opt -passes="loop(loop-idiom)" -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_memset(float* nocapture %a, i32 %N) {
; CHECK-NOT: @llvm.memset.p0i8.i64
entry:
  %i.linear.iv = alloca i32, align 4
  br label %dir.simd

dir.simd:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %loop.ph

loop.ph:
  %cmp312 = icmp slt i32 %N, 1
  %1 = bitcast i32* %i.linear.iv to i8*
  %wide.trip.count = sext i32 %N to i64
  br label %loop

loop:                                             ; preds = %loop.ph, %loop
  %indvars.iv = phi i64 [ 0, %loop.ph ], [ %indvars.iv.next, %loop ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %i.linear.iv, align 4
  %ptridx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float 0.000000e+00, float* %ptridx, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %loop, label %omp.inner.for.cond.omp.loop.exit.split.loopexit_crit_edge

omp.inner.for.cond.omp.loop.exit.split.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit.split.loopexit_crit_edge
  ret void
}

define void @test_memcpy(i64 %Size) {
; CHECK-NOT: @llvm.memcpy.p0i8.i64
bb.nph:
  %Base = alloca i8, i32 10000
  %Dest = alloca i8, i32 10000
  br label %dir.simd

dir.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body.ph

for.body.ph:
  br label %for.body

for.body:
  %indvar = phi i64 [ 0, %for.body.ph ], [ %indvar.next, %for.body ]
  %I.0.014 = getelementptr i8, i8* %Base, i64 %indvar
  %DestI = getelementptr i8, i8* %Dest, i64 %indvar
  %V = load i8, i8* %I.0.014, align 1
  store i8 %V, i8* %DestI, align 1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %Size
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.end


for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
