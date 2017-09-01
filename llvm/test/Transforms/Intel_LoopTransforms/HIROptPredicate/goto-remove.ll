; RUN: opt -hir-cost-model-throttling=0 -hir-ssa-deconstruction -disable-output -hir-opt-predicate -print-after=hir-opt-predicate -debug-only=hir-opt-predicate < %s 2>&1 | FileCheck %s

; Incoming HIR:
;  BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;       |   if (%n < 50)
;       |   {
;       |      (%q)[i1] = 0;
;       |   }
;       |   else
;       |   {
;       |      if (%n > 100)
;       |      {
;       |         %arrayidx8.pre-phi = &((%p)[i1]);
;       |         goto L;
;       |      }
;       |   }
;       |   (%p)[i1] = 0;
;       |   %arrayidx8.pre-phi = &((%p)[i1]);
;       |   L:
;       |   %0 = (%arrayidx8.pre-phi)[0];
;       |   (%arrayidx8.pre-phi)[0] = %0 + 1;
;       + END LOOP
;  END REGION

; Expected HIR after OptPredicate:
;  BEGIN REGION { modified }
;        if (%n < 50)
;        {
;          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;          |   (%q)[i1] = 0;
;          |   (%p)[i1] = 0;
;          |   %arrayidx8.pre-phi = &((%p)[i1]);
;          |   %0 = (%arrayidx8.pre-phi)[0];
;          |   (%arrayidx8.pre-phi)[0] = %0 + 1;
;          + END LOOP
;        }
;        else
;        {
;           if (%n > 100)
;           {
;             + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;             |   %arrayidx8.pre-phi = &((%p)[i1]);
;             |   %0 = (%arrayidx8.pre-phi)[0];
;             |   (%arrayidx8.pre-phi)[0] = %0 + 1;
;             + END LOOP
;           }
;           else
;           {
;             + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;             |   (%p)[i1] = 0;
;             |   %arrayidx8.pre-phi = &((%p)[i1]);
;             |   %0 = (%arrayidx8.pre-phi)[0];
;             |   (%arrayidx8.pre-phi)[0] = %0 + 1;
;             + END LOOP
;           }
;        }
;  END REGION

; REQUIRES: asserts

; Verify that there were goto and labels

; CHECK: Hoisting:
; CHECK: Hoisting:
; CHECK-SAME: if (%n > 100)
; CHECK: While
; CHECK: goto L[[X:.*]];
; CHECK-NOT: }
; CHECK-NOT: END
; CHECK: L[[X]]

; Verify that there are no goto or labels

; CHECK: After
; CHECK: BEGIN REGION { modified }
; CHECK-NOT: goto
; CHECK-NOT: :
; CHECK: END REGION

;Module Before HIR; ModuleID = 'module.ll'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture %q, i32* nocapture readnone %r, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp slt i32 %n, 50
  %cmp2 = icmp sgt i32 %n, 100
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %L
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %L, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %L ]
  br i1 %cmp1, label %L2, label %if.end

if.end:                                           ; preds = %for.body
  br i1 %cmp2, label %if.end.L_crit_edge, label %if.end4

if.end.L_crit_edge:                               ; preds = %if.end
  %.pre = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  br label %L

L2:                                               ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  store i32 0, i32* %arrayidx, align 4
  br label %if.end4

if.end4:                                          ; preds = %L2, %if.end
  %arrayidx6 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 0, i32* %arrayidx6, align 4
  br label %L

L:                                                ; preds = %if.end4, %if.end.L_crit_edge
  %arrayidx8.pre-phi = phi i32* [ %.pre, %if.end.L_crit_edge ], [ %arrayidx6, %if.end4 ]
  %0 = load i32, i32* %arrayidx8.pre-phi, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx8.pre-phi, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


