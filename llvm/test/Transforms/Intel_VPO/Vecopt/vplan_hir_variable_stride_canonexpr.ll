; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec -vplan-print-after-vpentity-instrs -disable-output -S < %s  | FileCheck %s

; Check that a VPExternalDef is created for a stride canon expr containing conversions.

; CHECK: {(sext i32 (trunc i64 %call.i to i32) to i64)}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #1 {
entry:
  %i9.linear.iv = alloca i64, align 8
  %b = alloca [128 x i64], align 16
  %arrayidx3 = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx3, align 8
  %call.i = tail call i64 @strtol(ptr nocapture noundef nonnull %0, ptr noundef null, i32 noundef 10) #3
  %sext = shl i64 %call.i, 32
  %conv4 = ashr exact i64 %sext, 32
  %conv.i = trunc i64 %call.i to i32
  br label %DIR.OMP.SIMD.157

DIR.OMP.SIMD.157:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i9.linear.iv, i64 0, i32 1, i32 %conv.i) ]
  br label %DIR.OMP.SIMD.256

DIR.OMP.SIMD.256:                                 ; preds = %DIR.OMP.SIMD.157
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.256, %omp.inner.for.body
  %.omp.iv.local.048 = phi i64 [ 0, %DIR.OMP.SIMD.256 ], [ %add14, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.048, %conv4
  %add.i = add nsw i64 %mul, 1
  %arrayidx13 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %mul
  store i64 %add.i, ptr %arrayidx13, align 8
  %add14 = add nuw nsw i64 %.omp.iv.local.048, 1
  %exitcond.not = icmp eq i64 %add14, 64
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.446

DIR.OMP.END.SIMD.446:                             ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  br label %for.cond.cleanup18

for.cond.cleanup18:                               ; preds = %DIR.OMP.END.SIMD.446
  ret i32 0
}

declare dso_local i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
