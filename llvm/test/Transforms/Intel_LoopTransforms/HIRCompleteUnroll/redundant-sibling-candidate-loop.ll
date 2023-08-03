; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination,hir-post-vec-complete-unroll,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; The first store to (@c)[0][i3 + 1] is eliminated by dead store elimination
; due to post-dominating store. This makes the second i2 loop redundant.
; During unroll, the second i2 loop is removed by redundant node removal
; utility after we unroll first i2 loop.
; Complete unroll was trying to unroll this loop which resulted in failure.

; Input HIR-
; + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   (@b)[0][2 * i2 + 2] = 0;
; |   + END LOOP
; |
; |
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   %or = 2 * i2  ||  109;
; |   |
; |   |   + DO i3 = 0, 4294967294, 1   <DO_LOOP>
; |   |   |   (@c)[0][i3 + 1] = %or;
; |   |   + END LOOP
; |   + END LOOP
; |
; |
; |   + DO i2 = 0, 2147483644, 1   <DO_LOOP>
; |   |   %and = 2 * i2 + 6  &&  70;
; |   |
; |   |   + DO i3 = 0, 4294967294, 1   <DO_LOOP>
; |   |   |   (@c)[0][i3 + 1] = %and;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: modified
; CHECK: (@c)[0][i3 + 1]
; CHECK-NOT: (@c)[0][i3 + 1]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@f = common dso_local local_unnamed_addr global i32 0, align 4
@d = common dso_local local_unnamed_addr global i32 0, align 4
@b = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@e = common dso_local local_unnamed_addr global i32 0, align 4
@c = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @g() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @f, align 4
  %tobool47 = icmp eq i32 %.pr, 0
  br i1 %tobool47, label %for.end32, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc30
  %inc3149 = phi i32 [ %inc31, %for.inc30 ], [ %.pr, %for.cond1.preheader.preheader ]
  br label %for.body2

for.body2:                                        ; preds = %for.cond1.preheader, %for.body2
  %indvars.iv = phi i64 [ 2, %for.cond1.preheader ], [ %indvars.iv.next, %for.body2 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @b, i64 0, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 7
  br i1 %cmp, label %for.body2, label %for.cond6.preheader.preheader

for.cond6.preheader.preheader:                    ; preds = %for.body2
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.cond6.preheader.preheader, %for.inc13
  %h.143 = phi i32 [ %add14, %for.inc13 ], [ 0, %for.cond6.preheader.preheader ]
  %or = or i32 %h.143, 109
  br label %for.body8

for.body8:                                        ; preds = %for.cond6.preheader, %for.body8
  %indvars.iv50 = phi i64 [ 1, %for.cond6.preheader ], [ %indvars.iv.next51, %for.body8 ]
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr @c, i64 0, i64 %indvars.iv50
  store i32 %or, ptr %arrayidx10, align 4
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond = icmp eq i64 %indvars.iv.next51, 4294967296
  br i1 %exitcond, label %for.inc13, label %for.body8

for.inc13:                                        ; preds = %for.body8
  %add14 = add nuw nsw i32 %h.143, 2
  %cmp4 = icmp ult i32 %add14, 5
  br i1 %cmp4, label %for.cond6.preheader, label %for.cond19.preheader.preheader

for.cond19.preheader.preheader:                   ; preds = %for.inc13
  br label %for.cond19.preheader

for.cond19.preheader:                             ; preds = %for.cond19.preheader.preheader, %for.inc27
  %h.246 = phi i32 [ %add28, %for.inc27 ], [ 6, %for.cond19.preheader.preheader ]
  %and = and i32 %h.246, 70
  br label %for.body21

for.body21:                                       ; preds = %for.cond19.preheader, %for.body21
  %indvars.iv52 = phi i64 [ 1, %for.cond19.preheader ], [ %indvars.iv.next53, %for.body21 ]
  %arrayidx23 = getelementptr inbounds [100 x i32], ptr @c, i64 0, i64 %indvars.iv52
  store i32 %and, ptr %arrayidx23, align 4
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next53, 4294967296
  br i1 %exitcond54, label %for.inc27, label %for.body21

for.inc27:                                        ; preds = %for.body21
  %add28 = add i32 %h.246, 2
  %tobool17 = icmp eq i32 %add28, 0
  br i1 %tobool17, label %for.inc30, label %for.cond19.preheader

for.inc30:                                        ; preds = %for.inc27
  %inc31 = add nsw i32 %inc3149, 1
  %tobool = icmp eq i32 %inc31, 0
  br i1 %tobool, label %for.cond.for.end32_crit_edge, label %for.cond1.preheader

for.cond.for.end32_crit_edge:                     ; preds = %for.inc30
  store i32 8, ptr @d, align 4
  store i32 0, ptr @e, align 4
  store i32 0, ptr @f, align 4
  br label %for.end32

for.end32:                                        ; preds = %for.cond.for.end32_crit_edge, %entry
  ret void
}

