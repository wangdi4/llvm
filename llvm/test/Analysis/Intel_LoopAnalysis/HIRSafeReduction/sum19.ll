; int  s = 0; 
; for (int i=0; i < n1; i++) {
;     s = a[i]  - s;
; }
; float  s1 = 0; 
; for (int i=0; i< n2; i++) {
;     s1 = d[i]  - s1;
; }
; float  s2 = 0; 
; for (int i=0; i< n3; i++) {
;     s2 = d[i]  - s2;
;     s2 = e[i]  - s2;
; }
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction -hir-temp-cleanup -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; CHECK: No Safe Reduction

; ModuleID = 'sum19.cpp'
source_filename = "sum19.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@d = local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@e = local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @_Z3subPiiii(i32* nocapture readonly %a, i32 %n1, i32 %n2, i32 %n3) local_unnamed_addr #0 {
entry:
  %cmp52 = icmp sgt i32 %n1, 0
  br i1 %cmp52, label %for.body.preheader, label %for.cond2.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count62 = zext i32 %n1 to i64
  br label %for.body

for.cond2.preheader.loopexit:                     ; preds = %for.body
  %phitmp = sitofp i32 %sub to float
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.loopexit, %entry
  %s.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %phitmp, %for.cond2.preheader.loopexit ]
  %cmp348 = icmp sgt i32 %n2, 0
  br i1 %cmp348, label %for.body5.preheader, label %for.cond13.preheader

for.body5.preheader:                              ; preds = %for.cond2.preheader
  %wide.trip.count58 = zext i32 %n2 to i64
  br label %for.body5

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv60 = phi i64 [ %indvars.iv.next61, %for.body ], [ 0, %for.body.preheader ]
  %s.053 = phi i32 [ %sub, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv60
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub = sub nsw i32 %0, %s.053
  %indvars.iv.next61 = add nuw nsw i64 %indvars.iv60, 1
  %exitcond63 = icmp eq i64 %indvars.iv.next61, %wide.trip.count62
  br i1 %exitcond63, label %for.cond2.preheader.loopexit, label %for.body

for.cond13.preheader:                             ; preds = %for.body5, %for.cond2.preheader
  %s1.0.lcssa = phi float [ 0.000000e+00, %for.cond2.preheader ], [ %sub8, %for.body5 ]
  %cmp1445 = icmp sgt i32 %n3, 0
  br i1 %cmp1445, label %for.body16.preheader, label %for.cond.cleanup15

for.body16.preheader:                             ; preds = %for.cond13.preheader
  %wide.trip.count = zext i32 %n3 to i64
  br label %for.body16

for.body5:                                        ; preds = %for.body5.preheader, %for.body5
  %indvars.iv56 = phi i64 [ %indvars.iv.next57, %for.body5 ], [ 0, %for.body5.preheader ]
  %s1.049 = phi float [ %sub8, %for.body5 ], [ 0.000000e+00, %for.body5.preheader ]
  %arrayidx7 = getelementptr inbounds [1000 x float], [1000 x float]* @d, i64 0, i64 %indvars.iv56
  %1 = load float, float* %arrayidx7, align 4, !tbaa !5
  %sub8 = fsub float %1, %s1.049
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond59 = icmp eq i64 %indvars.iv.next57, %wide.trip.count58
  br i1 %exitcond59, label %for.cond13.preheader, label %for.body5

for.cond.cleanup15:                               ; preds = %for.body16, %for.cond13.preheader
  %s2.0.lcssa = phi float [ 0.000000e+00, %for.cond13.preheader ], [ %sub22, %for.body16 ]
  %add = fadd float %s.0.lcssa, %s1.0.lcssa
  %add26 = fadd float %add, %s2.0.lcssa
  %conv27 = fptosi float %add26 to i32
  ret i32 %conv27

for.body16:                                       ; preds = %for.body16.preheader, %for.body16
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body16 ], [ 0, %for.body16.preheader ]
  %s2.046 = phi float [ %sub22, %for.body16 ], [ 0.000000e+00, %for.body16.preheader ]
  %arrayidx18 = getelementptr inbounds [1000 x float], [1000 x float]* @d, i64 0, i64 %indvars.iv
  %2 = load float, float* %arrayidx18, align 4, !tbaa !5
  %sub19 = fsub float %2, %s2.046
  %arrayidx21 = getelementptr inbounds [1000 x float], [1000 x float]* @e, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx21, align 4, !tbaa !5
  %sub22 = fsub float %3, %sub19
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup15, label %for.body16
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
