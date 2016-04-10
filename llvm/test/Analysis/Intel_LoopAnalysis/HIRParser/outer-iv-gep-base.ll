; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that GEP's phi base which is inductive in the outer loop as well is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %1 = {al:2}(%0)[i1 + %storemerge.3205 + 1]
; CHECK-NEXT: END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @RemInt(i16* %p, i32 %n) #0 {
entry:
  br i1 undef, label %return, label %cond.end.175

cond.end.175:                                     ; preds = %entry
  br label %for.end.217

for.end.217:                                      ; preds = %for.cond.202
  br label %for.body.254

for.body.254:                                     ; preds = %for.cond.251.backedge, %for.end.217
  %0 = phi i16* [ %p, %for.end.217 ], [ %2, %for.cond.251.backedge ]
  %storemerge.3205 = phi i32 [ 0, %for.end.217 ], [ %storemerge.3, %for.cond.251.backedge ]
  br i1 undef, label %return, label %if.end.273

if.end.273:                                       ; preds = %for.body.254
  %add.ptr330 = getelementptr i16, i16* %0, i32 %storemerge.3205
  br i1 undef, label %for.end.351, label %for.inc.347.lr.ph

for.inc.347.lr.ph:                                ; preds = %if.end.273
  br label %for.inc.347

for.inc.347:                                      ; preds = %for.inc.347, %for.inc.347.lr.ph
  %m.sroa.0.1.in190 = phi i16* [ %add.ptr330, %for.inc.347.lr.ph ], [ %incdec.ptr349, %for.inc.347 ]
  %k.0189 = phi i32 [ 0, %for.inc.347.lr.ph ], [ %inc348, %for.inc.347 ]
  %inc348 = add nuw i32 %k.0189, 1
  %incdec.ptr349 = getelementptr i16, i16* %m.sroa.0.1.in190, i32 1
  %1 = load i16, i16* %incdec.ptr349, align 2
  %exitcond = icmp eq i32 %inc348, %n
  br i1 %exitcond, label %for.cond.332.for.end.351_crit_edge, label %for.inc.347

for.cond.332.for.end.351_crit_edge:               ; preds = %for.inc.347
  br label %for.end.351

for.end.351:                                      ; preds = %for.cond.332.for.end.351_crit_edge, %if.end.273
  br label %for.cond.251.backedge

for.cond.251.backedge:                            ; preds = %for.end.351, %for.body.254
  %storemerge.3 = add i32 %storemerge.3205, -1
  %2 = load i16*, i16** undef, align 4
  br label %for.body.254

return:                                           ; preds = %entry
  ret void
}
