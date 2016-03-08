; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the innermost loop verifying that loop upper composed of non-generable outer loop IVs is parsed correctly.
; CHECK: DO i1 = 0, -1 * %m.047 + -1 * %l.043 + %n + -3
; CHECK-NEXT: (%A)[i1 + %m.047 + %l.043 + 2] = i1 + %m.047 + %l.043 + zext.i32.i64(%i.045) + %j.041 + 2
; CHECK-NEXT: END LOOP

; ModuleID = 'outer-loop-iv-upper1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A, i64 %n) {
entry:
  %sub = add i64 %n, 4294967295
  %conv = trunc i64 %sub to i32
  %cmp.44 = icmp sgt i32 %conv, 1
  br i1 %cmp.44, label %for.body.lr.ph, label %for.end.20

for.body.lr.ph:                                   ; preds = %entry
  %cmp3.40 = icmp sgt i64 %n, 1
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.18
  %m.047 = phi i64 [ 5, %for.body.lr.ph ], [ %inc, %for.inc.18 ]
  %i.045 = phi i32 [ %conv, %for.body.lr.ph ], [ %div19, %for.inc.18 ]
  %inc = add nuw nsw i64 %m.047, 1
  br i1 %cmp3.40, label %for.body.5.lr.ph, label %for.inc.18

for.body.5.lr.ph:                                 ; preds = %for.body
  %conv11.37 = zext i32 %i.045 to i64
  br label %for.body.5

for.body.5:                                       ; preds = %for.body.5.lr.ph, %for.inc.16
  %l.043 = phi i64 [ 2, %for.body.5.lr.ph ], [ %inc6, %for.inc.16 ]
  %j.041 = phi i64 [ %n, %for.body.5.lr.ph ], [ %div, %for.inc.16 ]
  %inc6 = add nuw nsw i64 %l.043, 1
  %add = add nsw i64 %inc6, %inc
  %cmp8.38 = icmp slt i64 %add, %n
  br i1 %cmp8.38, label %for.body.10.lr.ph, label %for.inc.16

for.body.10.lr.ph:                                ; preds = %for.body.5
  %add12 = add i64 %j.041, %conv11.37
  br label %for.body.10

for.body.10:                                      ; preds = %for.body.10, %for.body.10.lr.ph
  %k.039 = phi i64 [ %add, %for.body.10.lr.ph ], [ %inc15, %for.body.10 ]
  %add13 = add i64 %add12, %k.039
  %conv14 = trunc i64 %add13 to i32
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %k.039
  store i32 %conv14, i32* %arrayidx, align 4
  %inc15 = add nsw i64 %k.039, 1
  %exitcond = icmp eq i64 %inc15, %n
  br i1 %exitcond, label %for.inc.16, label %for.body.10

for.inc.16:                                       ; preds = %for.body.10, %for.body.5
  %div = sdiv i64 %j.041, 2
  %cmp3 = icmp sgt i64 %j.041, 3
  br i1 %cmp3, label %for.body.5, label %for.inc.18

for.inc.18:                                       ; preds = %for.inc.16, %for.body
  %div19 = sdiv i32 %i.045, 2
  %cmp = icmp sgt i32 %i.045, 3
  br i1 %cmp, label %for.body, label %for.end.20

for.end.20:                                       ; preds = %for.inc.18, %entry
  ret void
}

