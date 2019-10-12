; Test that VPlan is able to import masked sum reduction into VPEntities framework, and generate correct
; masked vector code with mixed CG approach.

; Incoming HIR into vectorizer
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;       + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;       |   %0 = (@B)[0][i1];
;       |   if (%0 > 0.000000e+00)
;       |   {
;       |      %add = %0  +  (@C)[0][i1];
;       |      %tsum.015 = %tsum.015  +  %add; <Safe Reduction>
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-entities-dump -vplan-print-after-hcfg -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; Check that reduction is imported as VPEntity.
; CHECK-LABEL: Reduction list
; CHECK:  (+) Start: float %tsum.015 Exit: float [[EXIT_BLEND_PHI:%vp.*]]

; Checks for generated HIR code
; CHECK-LABEL: Function: ifsum1
; CHECK:      %tgu = (sext.i32.i64(%N))/u4;
; CHECK-NEXT: if (0 <u 4 * %tgu)
; CHECK-NEXT: {
; CHECK-NEXT:    %result.vector = insertelement zeroinitializer,  %tsum.015,  0;

; CHECK:         + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 250> <nounroll> <novectorize>
; CHECK-NEXT:    |   %.vec = (<4 x float>*)(@B)[0][i1];
; CHECK-NEXT:    |   %wide.cmp. = %.vec > 0.000000e+00;
; CHECK-NEXT:    |   %add.vec = %.vec  +  (<4 x float>*)(@C)[0][i1]; Mask = @{%wide.cmp.}
; CHECK-NEXT:    |   %result.vector = %result.vector  +  %add.vec; Mask = @{%wide.cmp.}
; CHECK-NEXT:    + END LOOP

; CHECK:         %rdx.shuf = shufflevector %result.vector,  %result.vector,  <i32 2, i32 3, i32 undef, i32 undef>;
; CHECK-NEXT:    %bin.rdx = %result.vector  +  %rdx.shuf;
; CHECK-NEXT:    %rdx.shuf1 = shufflevector %bin.rdx,  %bin.rdx,  <i32 1, i32 undef, i32 undef, i32 undef>;
; CHECK-NEXT:    %bin.rdx2 = %bin.rdx  +  %rdx.shuf1;
; CHECK-NEXT:    %tsum.015 = extractelement %bin.rdx2,  0;
; CHECK-NEXT: }


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @ifsum1(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %N, 0
  br i1 %cmp14, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %tsum.1.lcssa = phi float [ %tsum.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %tsum.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %tsum.1.lcssa, %for.cond.cleanup.loopexit ]
  ret float %tsum.0.lcssa

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %tsum.015 = phi float [ 0.000000e+00, %for.body.preheader ], [ %tsum.1, %for.inc ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %0, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [1000 x float], [1000 x float]* @C, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx5, align 4, !tbaa !2
  %add = fadd fast float %0, %1
  %add6 = fadd fast float %tsum.015, %add
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %tsum.1 = phi float [ %add6, %if.then ], [ %tsum.015, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

