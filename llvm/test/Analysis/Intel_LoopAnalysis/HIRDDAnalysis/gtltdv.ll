;    int N = 1024;
;    for (int i = 0; i < N; i += 1) {
;        R[i] = S[(N - 1) - i] +1.0;
;        S[i] = 1.0; }
;  DD test results = (<>) 
;  Need to construct 2 edges 
;  
; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK-DAG: (@S)[0][i1] ANTI (<)	
; CHECK-DAG: (@S)[0][-1 * i1 + 1023] FLOW (<)

; ModuleID = 'gtltdv.c'
source_filename = "gtltdv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@S = common global [10000 x float] zeroinitializer, align 16
@R = common global [10000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @test() #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 1023, %indvars.iv
  %arrayidx = getelementptr inbounds [10000 x float], [10000 x float]* @S, i64 0, i64 %0
  %1 = load float, float* %arrayidx, align 4, !tbaa !1
  %conv2 = fadd float %1, 1.000000e+00
  %arrayidx4 = getelementptr inbounds [10000 x float], [10000 x float]* @R, i64 0, i64 %indvars.iv
  store float %conv2, float* %arrayidx4, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds [10000 x float], [10000 x float]* @S, i64 0, i64 %indvars.iv
  store float 1.000000e+00, float* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 9725) (llvm/branches/loopopt 9735)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
