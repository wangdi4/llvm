;    for (j=0; j <  n; j++) {
;        B[j] =  1;
;    }
;    B[1] = 3;
;    C[1] = B[m]; 
;   for (j=0; j <  2*n; j++) {
;        B[j] =  2;
;   }
;
; RUN:  opt < %s -hir-ssa-deconstruction -hir-create-function-level-region | opt -hir-create-function-level-region  -hir-dd-analysis  -hir-dd-analysis-verify=Region -analyze  | FileCheck %s 
;
; (*) built for refs with no common loop nests and both of them are inside loops
; CHECK-DAG:   (%B)[i1] --> (%B)[i1] OUTPUT (*) 
; (=) built for refs with no common loop nests and one of them is outside loop
; CHECK-DAG:   (%B)[1] --> (%B)[i1] OUTPUT (=) 
; CHECK-DAG:   (%B)[1] --> (%B)[%m] FLOW (=) 
; CHECK-DAG:   (%B)[%m] --> (%B)[i1] ANTI (=) 


; ModuleID = 'oneregion.c'
source_filename = "oneregion.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @sub1(float* nocapture %B, i32* nocapture %C, i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv28 = phi i64 [ %indvars.iv.next29, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv28
  store float 1.000000e+00, float* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond = icmp eq i64 %indvars.iv.next29, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %arrayidx1 = getelementptr inbounds float, float* %B, i64 1
  store float 3.000000e+00, float* %arrayidx1, align 4, !tbaa !1
  %idxprom2 = sext i32 %m to i64
  %arrayidx3 = getelementptr inbounds float, float* %B, i64 %idxprom2
  %0 = load float, float* %arrayidx3, align 4, !tbaa !1
  %conv = fptosi float %0 to i32
  %arrayidx4 = getelementptr inbounds i32, i32* %C, i64 1
  store i32 %conv, i32* %arrayidx4, align 4, !tbaa !5
  %cmp624 = icmp sgt i32 %n, 0
  br i1 %cmp624, label %for.body8.preheader, label %for.end13

for.body8.preheader:                              ; preds = %for.end
  %mul = shl nsw i32 %n, 1
  %1 = sext i32 %mul to i64
  br label %for.body8

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %indvars.iv = phi i64 [ 0, %for.body8.preheader ], [ %indvars.iv.next, %for.body8 ]
  %arrayidx10 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  store float 2.000000e+00, float* %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp6 = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp6, label %for.body8, label %for.end13.loopexit

for.end13.loopexit:                               ; preds = %for.body8
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %for.end
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
