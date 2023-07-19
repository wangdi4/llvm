; for (int i=0; i < n1; i++) {
;     s = a[i]  - s;
;     s1 = d[i]  - s1;
;     s2 = d[i]  - s2;
;     s2 = e[i]  - s2; }
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; CHECK:   No Safe Reduction
; ModuleID = 'sum20.cpp'
source_filename = "sum20.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@d = local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@e = local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @_Z3subPiiii(ptr nocapture readonly %a, i32 %n1, i32 %n2, i32 %n3) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %n1, 0
  br i1 %cmp25, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n1 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %phitmp = sitofp i32 %sub to float
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %s.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %phitmp, %for.cond.cleanup.loopexit ]
  %s1.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %sub3, %for.cond.cleanup.loopexit ]
  %s2.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %sub9, %for.cond.cleanup.loopexit ]
  %add = fadd float %s.0.lcssa, %s1.0.lcssa
  %add10 = fadd float %add, %s2.0.lcssa
  %conv11 = fptosi float %add10 to i32
  ret i32 %conv11

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %s2.028 = phi float [ %sub9, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %s1.027 = phi float [ %sub3, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %s.026 = phi i32 [ %sub, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %sub = sub nsw i32 %0, %s.026
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @d, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !5
  %sub3 = fsub float %1, %s1.027
  %sub6 = fsub float %1, %s2.028
  %arrayidx8 = getelementptr inbounds [1000 x float], ptr @e, i64 0, i64 %indvars.iv
  %2 = load float, ptr %arrayidx8, align 4, !tbaa !5
  %sub9 = fsub float %2, %sub6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20668)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !3, i64 0}
