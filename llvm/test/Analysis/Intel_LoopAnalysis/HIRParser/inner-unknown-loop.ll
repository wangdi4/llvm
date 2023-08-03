; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that in the default cost model we form inner unknown loop (with header %for.body3) by default but disable the parent loop (%for.cond1.preheader).

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body3:
; CHECK: |   %j.013.out = %j.013.root;
; CHECK: |   %add14 = %add14  +  %j.013.out;
; CHECK: |   %j.013.root = %j.013.root  <<  1;
; CHECK: |   if (%j.013.root < %n)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body3;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp212 = icmp sgt i32 %n, 1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc4, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc4 ]
  br i1 %cmp212, label %for.body3.lr.ph, label %for.inc4

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %arrayidx.promoted = load i32, ptr %arrayidx, align 4
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %add14 = phi i32 [ %arrayidx.promoted, %for.body3.lr.ph ], [ %add, %for.body3 ]
  %j.013 = phi i32 [ 1, %for.body3.lr.ph ], [ %mul, %for.body3 ]
  %add = add nsw i32 %add14, %j.013
  %mul = shl nsw i32 %j.013, 1
  %cmp2 = icmp slt i32 %mul, %n
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc4_crit_edge

for.cond1.for.inc4_crit_edge:                     ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  store i32 %add.lcssa, ptr %arrayidx, align 4
  br label %for.inc4

for.inc4:                                         ; preds = %for.cond1.for.inc4_crit_edge, %for.cond1.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end5, label %for.cond1.preheader

for.end5:                                         ; preds = %for.inc4
  ret void
}

