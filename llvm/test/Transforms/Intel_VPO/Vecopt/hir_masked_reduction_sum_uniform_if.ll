; Test vector code generation for masked reduction under uniform if.

; Incoming HIR into vectorizer
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;       + DO i1 = 0, 999, 1   <DO_LOOP>
;       |   if (%N2 > 0)
;       |   {
;       |      %add = (@B)[0][i1]  +  (@C)[0][i1];
;       |      %tsum.015 = %tsum.015  +  %add; <Safe Reduction>
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; Checks for VPValue based code generation.
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       %red.init = 0.000000e+00;
; CHECK-NEXT:       %phi.temp = %red.init;
; CHECK:            + DO i1 = 0, 999, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:       |   %phi.temp1 = %phi.temp;
; CHECK-NEXT:       |   if (%N2 > 0)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %.vec = (<4 x float>*)(@B)[0][i1];
; CHECK-NEXT:       |      %.vec3 = (<4 x float>*)(@C)[0][i1];
; CHECK-NEXT:       |      %.vec4 = %.vec  +  %.vec3;
; CHECK-NEXT:       |      %.vec5 = %phi.temp  +  %.vec4;
; CHECK-NEXT:       |      %phi.temp1 = %.vec5;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       |   %phi.temp = %phi.temp1;
; CHECK:            + END LOOP
; CHECK:            %tsum.015 = @llvm.vector.reduce.fadd.v4f32(%tsum.015,  %phi.temp1);
; CHECK:      END REGION
 
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @ifsum1(i32 %N, i32 %N2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %tsum.015 = phi float [ 0.000000e+00, %entry ], [ %tsum.1, %for.inc ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %N2, 0
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
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %tsum.1.lcssa = phi float [ %tsum.1, %for.inc ]
  ret float %tsum.1.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
