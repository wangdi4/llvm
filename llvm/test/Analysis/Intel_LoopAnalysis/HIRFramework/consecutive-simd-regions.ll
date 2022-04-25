; RUN: opt < %s -hir-ssa-deconstruction -print-after=hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print,print<hir>" 2>&1 | FileCheck %s

; There are two consecutive simd loops with header omp.inner.for.body6 and
; omp.inner.for.body23 where the exit intrinsic of first loop jumps straight
; to entry intrinsic of second loop. The basic block %omp.inner.for.body23.lr.ph
; is split in between the intrinsics such that the region exit of first region
; jumps to region entry of second region.

; This jump (goto) from first region was incorrectly tied to the label in the
; second region (cross-region jump) resulting in an assertion.

; To avoid direct jumps, we split the exiting edge after splitting the basic
; block.

; CHECK: omp.inner.for.body23.lr.ph.split.split:
; CHECK: omp.inner.for.body23.lr.ph.split:

; CHECK: BEGIN REGION { }
; CHECK:    %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK:    %mul9 = %conv  *  0x3F40624DE0000000;

; CHECK:    + DO i1 = 0, 4095, 1   <DO_LOOP> <simd>
; CHECK:    |   %conv10 = sitofp.i32.float(i1);
; CHECK:    |   %mul11 = %mul9  *  %conv10;
; CHECK:    |   (%a)[0][%init][i1] = %mul11;
; CHECK:    + END LOOP

; CHECK:    @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK:    %t7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK:    %mul33 = %conv  *  8.000000e+00;

; CHECK:    + DO i1 = 0, 4095, 1   <DO_LOOP> <simd>
; CHECK:    |   %t8 = (%a)[0][%init][i1 + 1];
; CHECK:    |   %call = @expf(%t8);
; CHECK:    |   %conv34 = sitofp.i32.float(i1);
; CHECK:    |   %mul35 = %mul33  *  %conv34;
; CHECK:    |   %call36 = @sinf(%mul35);
; CHECK:    |   %add37 = %call  +  %call36;
; CHECK:    |   (%a)[0][%init][i1] = %add37;
; CHECK:    + END LOOP

; CHECK:    @llvm.directive.region.exit(%t7); [ DIR.OMP.END.SIMD() ]
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define hidden void @main.DIR.OMP.PARALLEL.LOOP.3([16384 x [4097 x float]]* nocapture %a, i64 %init) {
entry:
  %t5 = trunc i64 %init to i32
  %conv = sitofp i32 %t5 to float
  br label %omp.inner.for.body6.lr.ph

omp.inner.for.body6.lr.ph:                        ; preds = %entry
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %mul9 = fmul float %conv, 0x3F40624DE0000000
  br label %omp.inner.for.body6

omp.inner.for.body6:                              ; preds = %omp.inner.for.body6, %omp.inner.for.body6.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body6 ], [ 0, %omp.inner.for.body6.lr.ph ]
  %t6 = trunc i64 %indvars.iv to i32
  %conv10 = sitofp i32 %t6 to float
  %mul11 = fmul float %mul9, %conv10
  %arrayidx13 = getelementptr inbounds [16384 x [4097 x float]], [16384 x [4097 x float]]* %a, i64 0, i64 %init, i64 %indvars.iv
  store float %mul11, float* %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %omp.inner.for.body23.lr.ph, label %omp.inner.for.body6

omp.inner.for.body23.lr.ph:                       ; preds = %omp.inner.for.body6
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  %t7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %mul33 = fmul float %conv, 8.000000e+00
  br label %omp.inner.for.body23

omp.inner.for.body23:                             ; preds = %omp.inner.for.body23, %omp.inner.for.body23.lr.ph
  %indvars.iv6 = phi i64 [ %indvars.iv.next7, %omp.inner.for.body23 ], [ 0, %omp.inner.for.body23.lr.ph ]
  %indvars.iv.next7 = add nuw nsw i64 %indvars.iv6, 1
  %arrayidx31 = getelementptr inbounds [16384 x [4097 x float]], [16384 x [4097 x float]]* %a, i64 0, i64 %init, i64 %indvars.iv.next7
  %t8 = load float, float* %arrayidx31, align 4
  %call = call float @expf(float %t8)
  %t9 = trunc i64 %indvars.iv6 to i32
  %conv34 = sitofp i32 %t9 to float
  %mul35 = fmul float %mul33, %conv34
  %call36 = call float @sinf(float %mul35)
  %add37 = fadd float %call, %call36
  %arrayidx41 = getelementptr inbounds [16384 x [4097 x float]], [16384 x [4097 x float]]* %a, i64 0, i64 %init, i64 %indvars.iv6
  store float %add37, float* %arrayidx41, align 4
  %exitcond8 = icmp eq i64 %indvars.iv.next7, 4096
  br i1 %exitcond8, label %DIR.OMP.END.SIMD.6, label %omp.inner.for.body23

DIR.OMP.END.SIMD.6:                               ; preds = %omp.inner.for.body23
  call void @llvm.directive.region.exit(token %t7) [ "DIR.OMP.END.SIMD"() ]
  br label %loop.region.exit.loopexit

loop.region.exit.loopexit:                        ; preds = %DIR.OMP.END.SIMD.6
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
declare dso_local float @expf(float) local_unnamed_addr

; Function Attrs: nounwind
declare dso_local float @sinf(float) local_unnamed_addr

