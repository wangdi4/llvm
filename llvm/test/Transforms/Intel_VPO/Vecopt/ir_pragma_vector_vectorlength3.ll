; RUN: opt %s -S -passes=vplan-vec -vplan-force-vf=2 --debug-only=LoopVectorizationPlanner_vec_lengths 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; Checks if the code with #pragma vector vectorlength(4) (!{!"llvm.loop.vector.vectorlength", i64 4} metadata),
; -vplan-force-vf=2 and #pragma omp simd is being vectorized correctly with LLVM-IR.
;
; void foo(int *arr){
;   #pragma omp simd
;   #pragma vector vectorlength(4)
;   for (int i=0; i<1024; i++)
;     arr[i] = i;
; }
;
; CHECK: LVP: Specified vectorlengths: 2{{[[:space:]]}}
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPi(ptr %arr) {
DIR.OMP.SIMD.113:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.113
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %ptridx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  store i32 %1, ptr %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body, !llvm.loop !7

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!7 = distinct !{!7, !8, !9, !10}
!8 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!9 = !{!"llvm.loop.parallel_accesses"}
!10 = !{!"llvm.loop.intel.vector.vectorlength", i64 4}
