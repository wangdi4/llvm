;  Not a safe reduction
;   for (int j=0; j < N; j++) {
;    if (B[j] > 0){
;      tsum += B[j] + C[j];
;    }
;    else {
;      tsum = B[j];
;    }
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s
; CHECK:   No Safe Reduction
;
; ModuleID = 'ifsum2.c'
source_filename = "ifsum2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [64 x float] zeroinitializer, align 16
@winner = common dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @ifsum1(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp16 = icmp sgt i32 %N, 0
  br i1 %cmp16, label %for.body.preheader, label %for.cond.cleanup

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
  %tsum.017 = phi float [ 0.000000e+00, %for.body.preheader ], [ %tsum.1, %for.inc ]
  %arrayidx = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %0, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [64 x float], ptr @C, i64 0, i64 %indvars.iv, !intel-tbaa !7
  %1 = load float, ptr %arrayidx5, align 4, !tbaa !7
  %add = fadd float %0, %1
  %add6 = fadd float %tsum.017, %add
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %tsum.1 = phi float [ %add6, %if.then ], [ %0, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA64_f", !4, i64 0}

