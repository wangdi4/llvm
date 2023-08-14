; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-undo-sinking-for-perfect-loopnest,print<hir>" -hir-details 2>&1 < %s | FileCheck %s
;
; This test checks updating the live-out for the Lval ref: (%a)[i1 + %x.019.root]
; TODO: %a (sb:13) is no longer livein after undo sinking but we don't know if it is being used elsewhere in the loop

;*** IR Dump Before HIR Undo Sinking For Perfect Loopnest ***

;          + DO i1 = 0, 2, 1   <DO_LOOP>
;          |   %x.019.root = 1;
;          |
;          |   + DO i2 = 0, 3, 1   <DO_LOOP>
;          |   |   %spec.select = (i2 == %b) ? 3 : 2;
;          |   |   %x.019.root = %spec.select  *  %x.019.root;
;          |   |   (%a)[i1 + %x.019.root] = i1;
;          |   + END LOOP
;          + END LOOP


; CHECK:   + DO i32 i1 = 0, 2, 1   <DO_LOOP>
;          |   %x.019.root = 1;
;          |   <LVAL-REG> i32 1 {sb:3}
;          |
;          |
; CHECK:   |   + LiveOut symbases: [[LIVEOUTSB:[0-9]+]]
; CHECK:   |   + DO i32 i2 = 0, 3, 1   <DO_LOOP>
;          |   |   %spec.select = (i2 == %b) ? 3 : 2;
;          |   |   %x.019.root = %spec.select  *  %x.019.root;
; CHECK:   |   + END LOOP
; CHECK:   |      (%a)[i1 + %x.019.root] = i1;
;          |      <LVAL-REG> {al:4}(LINEAR ptr %a)[NON-LINEAR sext.i32.i64(i1 + %x.019.root)] inbounds {sb:17}
; CHECK:   |         <BLOB> NON-LINEAR i32 %x.019.root {sb:[[LIVEOUTSB]]}
;          |         <BLOB> LINEAR ptr %a {sb:13}
;          |      <RVAL-REG> LINEAR i32 i1 {sb:2}
;          |
;          + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %a, i32 noundef %b) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %i.021 = phi i32 [ 0, %entry ], [ %inc8, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 2
  %0 = load i32, ptr %arrayidx10, align 4
  ret i32 %0

for.cond.cleanup3:                                ; preds = %for.body4
  %mul.lcssa = phi i32 [ %mul, %for.body4 ]
  %add6 = add nuw nsw i32 %mul.lcssa, %i.021
  %idxprom = sext i32 %add6 to i64
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  store i32 %i.021, ptr %arrayidx, align 4
  %inc8 = add nuw nsw i32 %i.021, 1
  %exitcond22.not = icmp eq i32 %inc8, 3
  br i1 %exitcond22.not, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %j.020 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body4 ]
  %x.019 = phi i32 [ 1, %for.cond1.preheader ], [ %mul, %for.body4 ]
  %cmp5 = icmp eq i32 %j.020, %b
  %spec.select = select i1 %cmp5, i32 3, i32 2
  %mul = mul nsw i32 %spec.select, %x.019
  %inc = add nuw nsw i32 %j.020, 1
  %exitcond.not = icmp eq i32 %inc, 4
  br i1 %exitcond.not, label %for.cond.cleanup3, label %for.body4
}