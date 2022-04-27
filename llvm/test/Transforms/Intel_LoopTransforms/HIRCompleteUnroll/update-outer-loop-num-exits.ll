; RUN: opt -hir-cost-model-throttling=0 -hir-details -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -hir-cost-model-throttling=0 -hir-details -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that complete unroller updates the number of exits of outer loop after
; unrolling inner multi-exit loop.

; Input HIR-
; + DO i1 = 0, %t4 + -1, 1   <DO_MULTI_EXIT_LOOP>
; |   + DO i2 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; |   |   %indvars.iv.out = i2;
; |   |   %t5 = (%a.fpriv)[0].2[i2];
; |   |   %sub.us.us = %t5  +  -3.330000e+01;
; |   |   %t6 = @llvm.fabs.f64(%sub.us.us);
; |   |   if (%t6 > 0x3E7AD7F29ABCAF48)
; |   |   {
; |   |      goto if.then28;
; |   |   }
; |   + END LOOP
; + END LOOP

; CHECK: NumExits: 5
; CHECK: DO i32 i1
; CHECK-NOT: i2

%struct.t_struct = type { i8, i32, [4 x double] }

define void @foo(%struct.t_struct* %a.fpriv, i32 %t4) {
entry:
  br label %omp.inner.for.body.us.us

omp.inner.for.body.us.us:                         ; preds = %omp.inner.for.inc.us.us, %entry
  %.omp.iv.local.081.us.us = phi i32 [ %add40.us.us, %omp.inner.for.inc.us.us ], [ 0, %entry ]
  br label %for.body22.us.us

for.body22.us.us:                                 ; preds = %for.inc37.us.us, %omp.inner.for.body.us.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc37.us.us ], [ 0, %omp.inner.for.body.us.us ]
  %arrayidx25.us.us = getelementptr inbounds %struct.t_struct, %struct.t_struct* %a.fpriv, i64 0, i32 2, i64 %indvars.iv
  %t5 = load double, double* %arrayidx25.us.us, align 8
  %sub.us.us = fadd reassoc double %t5, -3.330000e+01
  %t6 = tail call reassoc double @llvm.fabs.f64(double %sub.us.us)
  %cmp26.us.us = fcmp reassoc ogt double %t6, 0x3E7AD7F29ABCAF48
  br i1 %cmp26.us.us, label %if.then28, label %for.inc37.us.us

for.inc37.us.us:                                  ; preds = %for.body22.us.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %omp.inner.for.inc.us.us, label %for.body22.us.us

omp.inner.for.inc.us.us:                          ; preds = %for.inc37.us.us
  %add40.us.us = add nuw nsw i32 %.omp.iv.local.081.us.us, 1
  %exitcond11 = icmp eq i32 %add40.us.us, %t4
  br i1 %exitcond11, label %DIR.OMP.END.LOOP.7.loopexit.loopexit, label %omp.inner.for.body.us.us

DIR.OMP.END.LOOP.7.loopexit.loopexit:
  ret void

if.then28:                                        ; preds = %for.body22.us.us
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body22.us.us ]
  ret void
}

declare double @llvm.fabs.f64(double) #3

attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
