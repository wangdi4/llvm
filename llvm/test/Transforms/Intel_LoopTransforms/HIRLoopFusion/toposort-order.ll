; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify nodes are placed in topological order after fusion of the 2nd and 3rd
; i2 loops.

; BEGIN REGION { }
;    + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
;    |   %2 = %1;
;    |
;    |   + DO i2 = 0, 4, 1   <DO_LOOP>
;    |   |   %hir.de.ssa.copy1.out = %2;
;    |   |   %2 = 0;
;    |   + END LOOP
;    |
;    |   %conv.le = %hir.de.ssa.copy1.out  *  -8;
;    |
;    |   + DO i2 = 0, 2, 1   <DO_LOOP>
;    |   |   %4 = (@b)[0][i2];
;    |   |   (@b)[0][i2] = -1 * %4;
;    |   + END LOOP
;    |
;    |   %and43 = %and43  &&  -2135488334;
;    |   %and22 = %and43  &&  %0;
;    |
;    |   + DO i2 = 0, 2, 1   <DO_LOOP>
;    |   |   (@b)[0][i2] = (zext.i8.i16((-8 * trunc.i32.i8(%hir.de.ssa.copy1.out))) * (1 + (560 * trunc.i32.i16(%and22))));
;    |   + END LOOP
;    |
;    |   %1 = 0;
;    + END LOOP
; END REGION

; CHECK: modified
; CHECK: %and43 =
; CHECK: %and22 = %and43
; CHECK: DO i2
; CHECK: (@b)[0][i2] = -1
; CHECK-NEXT: (@b)[0][i2] =
; CHECK-SAME: %and22


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g = common dso_local local_unnamed_addr global i32 0, align 4
@f = common dso_local local_unnamed_addr global i32 0, align 4
@i = common dso_local local_unnamed_addr global i32 0, align 4
@h = common dso_local local_unnamed_addr global i8 0, align 1
@c = common dso_local local_unnamed_addr global i32 0, align 4
@e = common dso_local local_unnamed_addr global i32 0, align 4
@b = common dso_local local_unnamed_addr global [5 x i16] zeroinitializer, align 2
@d = common dso_local local_unnamed_addr global i8 0, align 1

define dso_local void @j() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @g, align 4
  %tobool41 = icmp eq i32 %.pr, 0
  br i1 %tobool41, label %for.end34, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = load i8, ptr @d, align 1
  %conv21 = sext i8 %0 to i32
  %i.promoted42 = load i32, ptr @i, align 4
  %c.promoted = load i32, ptr @c, align 4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc32
  %inc3344 = phi i32 [ %.pr, %for.cond1.preheader.lr.ph ], [ %inc33, %for.inc32 ]
  %and43 = phi i32 [ %c.promoted, %for.cond1.preheader.lr.ph ], [ %and, %for.inc32 ]
  %1 = phi i32 [ %i.promoted42, %for.cond1.preheader.lr.ph ], [ 0, %for.inc32 ]
  %hir.de.ssa.copy1.in = bitcast i32 %1 to i32
  %storemerge38.in = bitcast i32 0 to i32
  br label %for.body2

for.body2:                                        ; preds = %for.body2, %for.cond1.preheader
  %2 = phi i32 [ %1, %for.cond1.preheader ], [ 0, %for.body2 ]
  %storemerge38 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body2 ]
  %hir.de.ssa.copy1.out = bitcast i32 %2 to i32
  %inc = add nuw nsw i32 %storemerge38, 1
  %exitcond = icmp eq i32 %inc, 5
  %hir.de.ssa.copy1.in49 = bitcast i32 0 to i32
  %storemerge38.in50 = bitcast i32 %inc to i32
  br i1 %exitcond, label %for.end, label %for.body2

for.end:                                          ; preds = %for.body2
  %.lcssa = phi i32 [ %hir.de.ssa.copy1.out, %for.body2 ]
  %3 = trunc i32 %.lcssa to i8
  %conv.le = mul i8 %3, -8
  %indvars.iv.in = bitcast i64 0 to i64
  br label %for.body8

for.cond16.preheader:                             ; preds = %for.body8
  %and = and i32 %and43, -2135488334
  %conv20 = zext i8 %conv.le to i32
  %and22 = and i32 %and, %conv21
  %mul23 = mul nuw nsw i32 %conv20, 560
  %mul24 = mul i32 %mul23, %and22
  %add = or i32 %mul24, %conv20
  %conv26 = trunc i32 %add to i16
  %indvars.iv46.in = bitcast i64 0 to i64
  br label %for.body19

for.body8:                                        ; preds = %for.body8, %for.end
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %for.body8 ]
  %arrayidx = getelementptr inbounds [5 x i16], ptr @b, i64 0, i64 %indvars.iv
  %4 = load i16, ptr %arrayidx, align 2
  %sub = sub i16 0, %4
  store i16 %sub, ptr %arrayidx, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next, 3
  %indvars.iv.in51 = bitcast i64 %indvars.iv.next to i64
  br i1 %exitcond45, label %for.cond16.preheader, label %for.body8

for.body19:                                       ; preds = %for.cond16.preheader, %for.body19
  %indvars.iv46 = phi i64 [ 0, %for.cond16.preheader ], [ %indvars.iv.next47, %for.body19 ]
  %arrayidx28 = getelementptr inbounds [5 x i16], ptr @b, i64 0, i64 %indvars.iv46
  store i16 %conv26, ptr %arrayidx28, align 2
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %cmp17 = icmp ult i64 %indvars.iv.next47, 3
  %indvars.iv46.in52 = bitcast i64 %indvars.iv.next47 to i64
  br i1 %cmp17, label %for.body19, label %for.inc32

for.inc32:                                        ; preds = %for.body19
  %inc33 = add nsw i32 %inc3344, 1
  %tobool = icmp eq i32 %inc33, 0
  %hir.de.ssa.copy0.in = bitcast i32 0 to i32
  %inc3344.in = bitcast i32 %inc33 to i32
  br i1 %tobool, label %for.cond.for.end34_crit_edge, label %for.cond1.preheader

for.cond.for.end34_crit_edge:                     ; preds = %for.inc32
  %and.lcssa = phi i32 [ %and, %for.inc32 ]
  %conv.le.lcssa = phi i8 [ %conv.le, %for.inc32 ]
  store i32 0, ptr @i, align 4
  store i8 %conv.le.lcssa, ptr @h, align 1
  store i32 5, ptr @f, align 4
  store i32 6, ptr @e, align 4
  store i32 %and.lcssa, ptr @c, align 4
  store i32 0, ptr @g, align 4
  br label %for.end34

for.end34:                                        ; preds = %for.cond.for.end34_crit_edge, %entry
  ret void
}

