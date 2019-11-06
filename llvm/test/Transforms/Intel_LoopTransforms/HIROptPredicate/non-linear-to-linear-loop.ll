; RUN: opt -hir-details -disable-output -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -disable-output -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; Check that %p.addr.022 def level is updated after opt-predicate.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   if (i1 == %n)
;       |   |   {
;       |   |      %0 = (%q)[i2];
;       |   |      %p.addr.022 = &((%0)[0]);
;       |   |   }
;       |   |   (%p.addr.022)[i2] = i2;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i32 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   if (i1 == %n)
; CHECK:       |   {
; CHECK:       |      + DO i64 i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |      |   %0 = (%q)[i2];
; CHECK:       |      |   %p.addr.022 = &((%0)[0]);
; CHECK:       |      |   <LVAL-REG> NON-LINEAR i32* %p.addr.022
; CHECK:       |      |
; CHECK:       |      |   (%p.addr.022)[i2] = i2;
; CHECK:       |      |   <LVAL-REG> {al:4}(NON-LINEAR i32* %p.addr.022)[LINEAR i64 i2]
; CHECK:       |      |      <BLOB> NON-LINEAR i32* %p.addr.022
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      + DO i64 i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |      |   (%p.addr.022)[i2] = i2;
; CHECK:       |      |   <LVAL-REG> {al:4}(LINEAR i32* %p.addr.022{def@1})[LINEAR i64 i2]
; CHECK:       |      |   <BLOB> LINEAR i32* %p.addr.022{def@1}
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %p, i32** nocapture readonly %q, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %j.023 = phi i32 [ 0, %entry ], [ %inc9, %for.cond.cleanup3 ]
  %p.addr.022 = phi i32* [ %p, %entry ], [ %p.addr.2.lcssa, %for.cond.cleanup3 ]
  %cmp5 = icmp eq i32 %j.023, %n
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %if.end
  %p.addr.2.lcssa = phi i32* [ %p.addr.2, %if.end ]
  %inc9 = add nuw nsw i32 %j.023, 1
  %exitcond24 = icmp eq i32 %inc9, 100
  br i1 %exitcond24, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %if.end, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %p.addr.119 = phi i32* [ %p.addr.022, %for.cond1.preheader ], [ %p.addr.2, %if.end ]
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %arrayidx = getelementptr inbounds i32*, i32** %q, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body4
  %p.addr.2 = phi i32* [ %0, %if.then ], [ %p.addr.119, %for.body4 ]
  %arrayidx7 = getelementptr inbounds i32, i32* %p.addr.2, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

