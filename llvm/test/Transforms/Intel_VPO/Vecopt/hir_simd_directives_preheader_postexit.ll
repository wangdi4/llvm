; Check that VPlan HIR vectorizer is able to handle SIMD loops where entry/exit directives
; are in HLLoop's preheader and postexit blocks.

; Incoming HIR
; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;       |   %sum.sroa.0.1 = 0;
;       |
;       |      %sum.red = alloca 1;
;       |      %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(32),  QUAL.OMP.REDUCTION.ADD(&((%sum.red)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;       |      (%sum.red)[0] = 0.000000e+00;
;       |      %1 = 0.000000e+00;
;       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <simd>
;       |   |   %2 = (%A)[i2];
;       |   |   %3 = (%B)[i2];
;       |   |   %mul9 = %2  *  %3;
;       |   |   %1 = %1  +  %mul9;
;       |   + END LOOP
;       |      (%sum.red)[0] = %1;
;       |      %4 = %1  +  0.000000e+00;
;       |      %5 = bitcast.float.i32(%4);
;       |      @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       |      %sum.sroa.0.1 = %5;
;       |
;       |   (i32*)(%C)[i1] = %sum.sroa.0.1;
;       + END LOOP
; END REGION

; Note: -hir-vec-dir-insert is not explicitly used below to ensure that SIMD loop is recognized
; and vectorized by VPlan.
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -print-after=VPlanDriverHIR -disable-output -enable-vp-value-codegen-hir=0 < %s  2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -print-after=VPlanDriverHIR -disable-output -enable-vp-value-codegen-hir < %s  2>&1 | FileCheck %s

; CHECK:                 %red.var = 0.000000e+00;
; CHECK-NEXT:         + DO i2 = 0, 32 * %tgu + -1, 32   <DO_LOOP>  <MAX_TC_EST = 134217727> <nounroll> <novectorize>
; CHECK-NEXT:         |   %.vec = (<32 x float>*)(%A)[i2];
; CHECK-NEXT:         |   %.vec3 = (<32 x float>*)(%B)[i2];
; CHECK-NEXT:         |   [[MUL:%.*]] = %.vec  *  %.vec3;
; CHECK-NEXT:         |   %red.var = %red.var  +  [[MUL]];
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:            %1 = @llvm.vector.reduce.fadd.v32f32(%1,  %red.var);


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooiiPfS_S_(i32 %n, i32 %m, float* nocapture readonly %A, float* nocapture readonly %B, float* nocapture %C) local_unnamed_addr #0 {
entry:
  %cmp31 = icmp sgt i32 %m, 0
  br i1 %cmp31, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp4 = icmp sgt i32 %n, 0
  %wide.trip.count3638 = zext i32 %m to i64
  %wide.trip.count39 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %omp.precond.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %omp.precond.end, %for.body.lr.ph
  %indvars.iv34 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next35, %omp.precond.end ]
  br i1 %cmp4, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %for.body
  %sum.red = alloca float, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 32), "QUAL.OMP.REDUCTION.ADD"(float* %sum.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  store float 0.000000e+00, float* %sum.red, align 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = phi float [ 0.000000e+00, %DIR.OMP.SIMD.2 ], [ %add10, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %arrayidx8 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %3 = load float, float* %arrayidx8, align 4
  %mul9 = fmul fast float %2, %3
  %add10 = fadd fast float %1, %mul9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count39
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split: ; preds = %omp.inner.for.body
  %add10.lcssa = phi float [ %add10, %omp.inner.for.body ]
  store float %add10.lcssa, float* %sum.red, align 4
  %4 = fadd float %add10.lcssa, 0.000000e+00
  %5 = bitcast float %4 to i32
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split, %for.body
  %sum.sroa.0.1 = phi i32 [ %5, %omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split ], [ 0, %for.body ]
  %arrayidx13 = getelementptr inbounds float, float* %C, i64 %indvars.iv34
  %6 = bitcast float* %arrayidx13 to i32*
  store i32 %sum.sroa.0.1, i32* %6, align 4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next35, %wide.trip.count3638
  br i1 %exitcond37, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
