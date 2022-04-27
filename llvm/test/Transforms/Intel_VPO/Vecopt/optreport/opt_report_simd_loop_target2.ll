; Test to check the functionality of target check of vectorization opt-report for LLVM-IR based vectorizer.
; Test should fail if remark #15569(Compiler has chosen to target XMM/YMM vector) is emitted for code with #pragma omp simd

; RUN: opt -S -vplan-vec -intel-ir-optreport-emitter -intel-opt-report=high -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -S -vplan-vec -intel-ir-optreport-emitter -intel-opt-report=high -disable-output -enable-intel-advanced-opts -mcpu=skylake-avx512  2>&1 < %s | FileCheck %s

; CHECK-NOT:   remark #15569

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo1(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c, i32 %N) local_unnamed_addr #0 {
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp3.not20 = icmp slt i32 %N, 1
  br i1 %cmp3.not20, label %omp.precond.end, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %2 = load float, float* %arrayidx6, align 4
  %add7 = fadd fast float %2, %1
  %arrayidx9 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %add7, float* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  store i32 %N, i32* %i.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.1, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @foo2(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c, i32 %N) local_unnamed_addr #0 {
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp3.not20 = icmp slt i32 %N, 1
  br i1 %cmp3.not20, label %omp.precond.end, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %2 = load float, float* %arrayidx6, align 4
  %add7 = fadd fast float %2, %1
  %arrayidx9 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %add7, float* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  store i32 %N, i32* %i.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.1, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable "tune-cpu"="generic" }
