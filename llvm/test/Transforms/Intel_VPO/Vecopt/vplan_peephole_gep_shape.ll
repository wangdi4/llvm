;
; RUN: opt -disable-output -passes=hir-vplan-vec -vplan-dump-da-shapes -vplan-print-after-early-peephole %s 2>&1 | FileCheck %s
;
; Check that stride is maintained after trunc/zext ->and peephole optimization
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i32:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: [DA: [Shape: Strided, Stride: i64 8]] ptr [[VP0:%.*]] = subscript

define void @_foo(ptr %in0, i64 noundef %N) local_unnamed_addr {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %c1 = trunc i64 %indvars.iv to i32
  %c2 = zext i32 %c1 to i64
  %arrayidx6 = getelementptr inbounds i64, ptr %in0, i64 %c2
  store i64 %c2, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond.not, label %omp.loopexit, label %omp.inner.for.body

omp.loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

