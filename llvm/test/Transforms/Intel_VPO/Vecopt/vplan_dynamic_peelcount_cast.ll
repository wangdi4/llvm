; RUN: opt -passes="vplan-vec" -vplan-enable-peeling -S < %s | FileCheck %s

; This test checks to make sure appropriate casting is done for the instruction
; used as the dynamic peel count. The original upper bound for the loop can be
; of different int types and we must account for this when creating the dynamic
; peel since this is the type used when creating it.
; Note: urem/trunc should become scalar once SVA-based codegen is finished.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: peel.checkz{{.*}}
; CHECK: [[TMP0:%.*]] = urem i64
; CHECK-NEXT: [[TMP1:%.*]] = insertelement <4 x i64> poison, i64 [[TMP0]], i64 0
; CHECK-NEXT: [[TMP2:%.*]] = shufflevector <4 x i64> [[TMP1]], <4 x i64> poison, <4 x i32> zeroinitializer
; CHECK-NEXT: trunc <4 x i64> [[TMP2]] to <4 x i32>

define dso_local void @foo(i32 %n, i32 %x, ptr nocapture %A, ptr nocapture readonly %B) local_unnamed_addr #0 {
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp3.not18 = icmp slt i32 %n, 1
  br i1 %cmp3.not18, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.125

DIR.OMP.SIMD.125:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.125, %omp.inner.for.body
  %indvars.iv = phi i32 [ 0, %DIR.OMP.SIMD.125 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i32 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %sub5 = sub nsw i32 %1, %x
  %arrayidx7 = getelementptr inbounds i32, ptr %A, i32 %indvars.iv
  store i32 %sub5, ptr %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond.not = icmp eq i32 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body
  store i32 %n, ptr %i.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.2, %entry
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
