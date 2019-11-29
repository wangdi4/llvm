;
; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -vplan-print-after-linearization -disable-output | FileCheck %s

;source code:
;void foo(float *a, float *b, int* n) {
;  #pragma omp simd 
;  for (int k = 0; k < 8; k++)
;  for (int i = 0; i < n[k]; i++) {
;    for (int j  = i; j < n[k]; j++) {
;      if (b[j] > 0)
;       a[i] += b[j];
;    }
;    a[i] += b[i];
;  }
;}
; Expected 2 all-zero-check VPInstructions created proving the recursivness
; of loopCFU calls.

; CHECK-LABEL: After predication and linearization
; CHECK:        i1 [[ALLZERO_1:%vp.*]] = all-zero-check
; CHECK-NEXT: SUCCESSORS(1)
; CHECK:      Condition({{BB[0-9]+}}): [DA: Uniform] i1  [[ALLZERO_1]] = all-zero-check
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[ALLZERO_1]]), {{BB[0-9]+}}(!i1 [[ALLZERO_1]])
; CHECK:        i1 [[ALLZERO_2:%vp.*]] = all-zero-check
; CHECK-NEXT:   i1 [[NOTCOND:%vp.*]] = not i1 [[ALLZERO_2]]
; CHECK-NEXT: SUCCESSORS(1)
; CHECK:      Condition({{BB[0-9]+}}): [DA: Uniform] i1  [[NOTCOND]] = not i1 [[ALLZERO_2]]
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[NOTCOND]]), {{BB[0-9]+}}(!i1 [[NOTCOND]])
;

; ModuleID = 'two_inner_llops.cpp'
source_filename = "two_inner_llops.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32
@"@bid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooPfS_Pi(float* nocapture %a, float* nocapture readonly %b, i32* nocapture readonly %n) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %cleanup.dest.slot.priv = alloca i32, align 4
  %j.priv = alloca i32, align 4
  %i.priv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(i32* %i.priv), "QUAL.OMP.PRIVATE"(i32* %j.priv), "QUAL.OMP.PRIVATE"(i32* %cleanup.dest.slot.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %i.priv to i8*
  %2 = bitcast i32* %j.priv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.cond.cleanup, %DIR.OMP.SIMD.2
  %indvars.iv43 = phi i64 [ %indvars.iv.next44, %for.cond.cleanup ], [ 0, %DIR.OMP.SIMD.2 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 0, i32* %i.priv, align 4, !tbaa !2
  %arrayidx = getelementptr inbounds i32, i32* %n, i64 %indvars.iv43
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp138 = icmp sgt i32 %3, 0
  br i1 %cmp138, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %omp.inner.for.body
  %4 = sext i32 %3 to i64
  br label %for.body

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.cond.cleanup6
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.cond.cleanup6 ]
  %5 = trunc i64 %indvars.iv.next.lcssa to i32
  store i32 %5, i32* %i.priv, align 4, !tbaa !2
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %omp.inner.for.body
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, 8
  br i1 %exitcond45, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.cond.cleanup6
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.cond.cleanup6 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  %6 = trunc i64 %indvars.iv to i32
  store i32 %6, i32* %j.priv, align 4, !tbaa !2
  %cmp536 = icmp slt i64 %indvars.iv, %4
  br i1 %cmp536, label %for.body7.lr.ph, label %for.cond.cleanup6

for.body7.lr.ph:                                  ; preds = %for.body
  %arrayidx14 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  br label %for.body7

for.cond2.for.cond.cleanup6_crit_edge:            ; preds = %for.inc
  store i32 %3, i32* %j.priv, align 4, !tbaa !2
  br label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond2.for.cond.cleanup6_crit_edge, %for.body
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  %arrayidx17 = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %7 = load float, float* %arrayidx17, align 4, !tbaa !6
  %arrayidx19 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %8 = load float, float* %arrayidx19, align 4, !tbaa !6
  %add20 = fadd float %7, %8
  store float %add20, float* %arrayidx19, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp1 = icmp slt i64 %indvars.iv.next, %4
  br i1 %cmp1, label %for.body, label %for.cond.for.cond.cleanup_crit_edge

for.body7:                                        ; preds = %for.inc, %for.body7.lr.ph
  %indvars.iv41 = phi i64 [ %indvars.iv, %for.body7.lr.ph ], [ %indvars.iv.next42, %for.inc ]
  %arrayidx9 = getelementptr inbounds float, float* %b, i64 %indvars.iv41
  %9 = load float, float* %arrayidx9, align 4, !tbaa !6
  %cmp10 = fcmp ogt float %9, 0.000000e+00
  br i1 %cmp10, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body7
  %10 = load float, float* %arrayidx14, align 4, !tbaa !6
  %add15 = fadd float %9, %10
  store float %add15, float* %arrayidx14, align 4, !tbaa !6
  br label %for.inc

for.inc:                                          ; preds = %for.body7, %if.then
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond = icmp eq i64 %indvars.iv.next42, %4
  br i1 %exitcond, label %for.cond2.for.cond.cleanup6_crit_edge, label %for.body7

DIR.OMP.END.SIMD.2:                               ; preds = %for.cond.cleanup
  store i32 6, i32* %cleanup.dest.slot.priv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  ret void
}
; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang bdad00d3d5ca8719ad7e63e7828962319c5ae7b9) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c089897100fc1ea50c09f25f5a2f0955f0f44d84)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}

