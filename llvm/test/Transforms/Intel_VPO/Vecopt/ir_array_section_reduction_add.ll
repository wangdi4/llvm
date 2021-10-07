; Test to verify that VPlan vectorizer bails out for array reduction idioms
; (using array sections) identified in incoming IR.

; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vplan-vec < %s 2>&1 | FileCheck %s

; CHECK: VPlan LLVM-IR Driver for Function: foo
; CHECK: VD: Not vectorizing: Cannot handle array reductions.

; CHECK: define i32 @foo
; CHECK: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([100 x i32]* %a.red, i64 1, i64 0, i64 100, i64 1) ]

define i32 @foo([100 x [100 x i32]]* %b) #0 {
entry:
  %a.red = alloca [100 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %a.red58 = bitcast [100 x i32]* %a.red to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(400) %a.red58, i8 0, i64 400, i1 false)
  br label %DIR.OMP.SIMD.170

DIR.OMP.SIMD.170:                                 ; preds = %DIR.OMP.SIMD.1
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([100 x i32]* %a.red, i64 1, i64 0, i64 100, i64 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.170, %omp.inner.for.inc
  %indvars.iv59 = phi i64 [ 0, %DIR.OMP.SIMD.170 ], [ %indvars.iv.next60, %omp.inner.for.inc ]
  br label %for.body15

for.body15:                                       ; preds = %omp.inner.for.body, %for.body15
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body ], [ %indvars.iv.next, %for.body15 ]
  %arrayidx19 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %b, i64 0, i64 %indvars.iv59, i64 %indvars.iv
  %b.ld = load i32, i32* %arrayidx19, align 4
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* %a.red, i64 0, i64 %indvars.iv
  %a.ld = load i32, i32* %arrayidx21, align 4
  %add22 = add nsw i32 %a.ld, %b.ld
  store i32 %add22, i32* %arrayidx21, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %omp.inner.for.inc, label %for.body15

omp.inner.for.inc:                                ; preds = %for.body15
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next60, 100
  br i1 %exitcond61.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  br label %DIR.OMP.END.SIMD.271

DIR.OMP.END.SIMD.271:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.271
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
