; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<20>            + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>
;<2>             |   %i.012.out = i1;
;<3>             |   %t.013.out = %t.013;
;<5>             |   %0 = (%A)[i1];
;<7>             |   if (%0 > 0)
;<7>             |   {
;<8>             |      goto for.end.loopexit;
;<7>             |   }
;<12>            |   (%A)[i1] = %0 + 1;
;<16>            |   %t.013 = i1;
;<20>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK:   BEGIN REGION { }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:        |   %t.013.out = %t.013;
; CHECK:        |   %0 = (%A)[i1];
; CHECK:        |   if (%0 > 0)
; CHECK:        |   {
; CHECK:        |      %i.012.out = i1;
; CHECK:        |      goto for.end.loopexit;
; CHECK:        |   }
; CHECK:        |   (%A)[i1] = %0 + 1;
; CHECK:        |   %t.013 = i1;
; CHECK:        + END LOOP
; CHECK:           %i.012.out = %N + -1;
; CHECK:  END REGION
;
define dso_local i64 @foo(i64 %N, i64* nocapture %A) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i64 %N, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %t.013 = phi i64 [ %i.012, %if.end ], [ 0, %for.body.preheader ]
  %i.012 = phi i64 [ %inc3, %if.end ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %i.012
  %0 = load i64, i64* %arrayidx, align 8
  %cmp1 = icmp sgt i64 %0, 0
  br i1 %cmp1, label %for.end.loopexit, label %if.end

if.end:                                           ; preds = %for.body
  %inc = add nsw i64 %0, 1
  store i64 %inc, i64* %arrayidx, align 8
  %inc3 = add nuw nsw i64 %i.012, 1
  %cmp = icmp slt i64 %inc3, %N
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body, %if.end
  %t.0.lcssa.ph = phi i64 [ %i.012, %if.end ], [ %t.013, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi i64 [ 0, %entry ], [ %t.0.lcssa.ph, %for.end.loopexit ]
  ret i64 %t.0.lcssa
}
