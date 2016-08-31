;  Testing for temps 
;    for (i= 1 ; i < 1000; i++) {
;        for (k = 1; k < 1000 ; k++) {
;					sum = sum + y[k];
;					x[i-1] += sum;
; RUN:  opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK-DAG:  %sum.022 FLOW (<= *)
;
; ModuleID = 'temp2.c'
source_filename = "temp2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = common global [1000 x float] zeroinitializer, align 16
@x = common global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define float @sub() #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %sum.022 = phi float [ 0.000000e+00, %entry ], [ %add, %for.inc6 ]
  %i.021 = phi i64 [ 1, %entry ], [ %inc7, %for.inc6 ]
  %sub = add nsw i64 %i.021, -1
  %arrayidx4 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %sub
  %arrayidx4.promoted = load float, float* %arrayidx4, align 4, !tbaa !1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %add520 = phi float [ %arrayidx4.promoted, %for.cond1.preheader ], [ %add5, %for.body3 ]
  %sum.119 = phi float [ %sum.022, %for.cond1.preheader ], [ %add, %for.body3 ]
  %k.018 = phi i64 [ 1, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @y, i64 0, i64 %k.018
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %sum.119, %0
  %add5 = fadd float %add520, %add
  %inc = add nuw nsw i64 %k.018, 1
  %exitcond = icmp eq i64 %inc, 1000
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  store float %add5, float* %arrayidx4, align 4, !tbaa !1
  %inc7 = add nuw nsw i64 %i.021, 1
  %exitcond23 = icmp eq i64 %inc7, 1000
  br i1 %exitcond23, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  ret float %add
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15842) (llvm/branches/loopopt 15854)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
