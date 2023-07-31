; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-details < %s 2>&1 | FileCheck %s

; Check that liveouts in are updated due to peeled loops.
; %add23.lcssa46 should become live-out from a common loop because it will be used in a peeled iteration.

; BEGIN REGION { }
;       + DO i1 = 0, (-1 * %.pr + -2)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>
;       |   + DO i2 = 0, 2, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 4294967294, 1   <DO_LOOP>
;       |   |   |   %bf.set.lcssa.lcssa48.out1 = %bf.set.lcssa.lcssa48;
;       |   |   |   %3 = (@g)[0][i3 + 1];
;       |   |   |   %bf.set.lcssa.lcssa48 = 16 * (%bf.set.lcssa.lcssa48.out1 /u 16)  ||  trunc.i32.i4(%3);
;       |   |   + END LOOP
;       |   |
;       |   |   (@l)[0][2 * i2] = trunc.i32.i4(%3);
;       |   + END LOOP
;       |
;       |   %bf.set.lcssa.lcssa48.out = %bf.set.lcssa.lcssa48;
;       |   %add23.lcssa46 = %add23.lcssa.lcssa49;
;       |
;       |   + DO i2 = 0, 3, 1   <DO_LOOP>
;       |   |   %or = i2 + 1  ||  10002188366;
;       |   |   %add2343 = %add23.lcssa46;
;       |   |
;       |   |   + DO i3 = 0, 4294967294, 1   <DO_LOOP>
;       |   |   |   %add2343.out = %add2343;
;       |   |   |   %5 = (@g)[0][i3 + 1];
;       |   |   |   %add23 = %or  +  %5;
;       |   |   |   %add2343 = %or + sext.i32.i64(%5);
;       |   |   + END LOOP
;       |   |
;       |   |   %add23.lcssa46 = %add23;
;       |   + END LOOP
;       |
;       |   %add23.lcssa.lcssa49 = %add23;
;       + END LOOP
; END REGION

; CHECK-LABEL: BEGIN REGION {  }
; CHECK:       <BLOB> NON-LINEAR i64 %add23.lcssa46 {sb:[[SB:[0-9]+]]}
; CHECK:       END REGION

; CHECK-LABEL: BEGIN REGION { modified }
; CHECK:         DO i32 i1
; CHECK:         + LiveOut symbases:
; CHECK-SAME:      [[SB]]
; CHECK:         DO i64 i2 = 0, 2, 1
; CHECK:         END LOOP

; CHECK:         DO i64 i2 = 0, 0, 1
; CHECK:         END LOOP
; CHECK:       END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.anon = type { i8, [7 x i8] }

@k = common dso_local local_unnamed_addr global i32 0, align 4
@i = common dso_local local_unnamed_addr global i32 0, align 4
@j = common dso_local local_unnamed_addr global i32 0, align 4
@g = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@f = common dso_local local_unnamed_addr global %struct.anon zeroinitializer, align 8
@l = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@e = common dso_local local_unnamed_addr global i32 0, align 4
@h = common dso_local local_unnamed_addr global i64 0, align 8
@d = common dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local void @m() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @k, align 4
  %tobool47 = icmp eq i32 %.pr, 0
  br i1 %tobool47, label %for.end32, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %.promoted = load i8, ptr @f, align 8
  %h.promoted = load i64, ptr @h, align 8
  %0 = sub i32 -2, %.pr
  %1 = and i32 %0, -2
  %2 = add i32 %.pr, %1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc30
  %add3150 = phi i32 [ %.pr, %for.cond1.preheader.lr.ph ], [ %add31, %for.inc30 ]
  %add23.lcssa.lcssa49 = phi i64 [ %h.promoted, %for.cond1.preheader.lr.ph ], [ %add23.lcssa.lcssa, %for.inc30 ]
  %bf.set.lcssa.lcssa48 = phi i8 [ %.promoted, %for.cond1.preheader.lr.ph ], [ %bf.set.lcssa.lcssa, %for.inc30 ]
  br label %for.cond3.preheader

for.cond11.preheader:                             ; preds = %for.inc9
  %bf.set.lcssa.lcssa = phi i8 [ %bf.set.lcssa, %for.inc9 ]
  br label %for.cond15.preheader

for.cond3.preheader:                              ; preds = %for.cond1.preheader, %for.inc9
  %indvars.iv51 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next52, %for.inc9 ]
  %bf.set.lcssa41 = phi i8 [ %bf.set.lcssa.lcssa48, %for.cond1.preheader ], [ %bf.set.lcssa, %for.inc9 ]
  br label %for.body5

for.body5:                                        ; preds = %for.cond3.preheader, %for.body5
  %indvars.iv = phi i64 [ 1, %for.cond3.preheader ], [ %indvars.iv.next, %for.body5 ]
  %bf.set38 = phi i8 [ %bf.set.lcssa41, %for.cond3.preheader ], [ %bf.set, %for.body5 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @g, i64 0, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx, align 4
  %4 = trunc i32 %3 to i8
  %bf.value = and i8 %4, 15
  %bf.clear = and i8 %bf.set38, -16
  %bf.set = or i8 %bf.clear, %bf.value
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4294967296
  br i1 %exitcond, label %for.inc9, label %for.body5

for.inc9:                                         ; preds = %for.body5
  %.lcssa = phi i32 [ %3, %for.body5 ]
  %bf.set.lcssa = phi i8 [ %bf.set, %for.body5 ]
  %arrayidx8 = getelementptr inbounds [100 x i32], ptr @l, i64 0, i64 %indvars.iv51
  %conv636 = shl i32 %.lcssa, 28
  %conv6 = ashr exact i32 %conv636, 28
  store i32 %conv6, ptr %arrayidx8, align 8
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 2
  %cmp = icmp ult i64 %indvars.iv.next52, 6
  br i1 %cmp, label %for.cond3.preheader, label %for.cond11.preheader

for.cond15.preheader:                             ; preds = %for.inc27, %for.cond11.preheader
  %indvars.iv56 = phi i64 [ 1, %for.cond11.preheader ], [ %indvars.iv.next57, %for.inc27 ]
  %add23.lcssa46 = phi i64 [ %add23.lcssa.lcssa49, %for.cond11.preheader ], [ %add23.lcssa, %for.inc27 ]
  %or = or i64 %indvars.iv56, 10002188366
  br label %for.body17

for.body17:                                       ; preds = %for.cond15.preheader, %for.body17
  %indvars.iv53 = phi i64 [ 1, %for.cond15.preheader ], [ %indvars.iv.next54, %for.body17 ]
  %add2343 = phi i64 [ %add23.lcssa46, %for.cond15.preheader ], [ %add23, %for.body17 ]
  %arrayidx21 = getelementptr inbounds [100 x i32], ptr @g, i64 0, i64 %indvars.iv53
  %5 = load i32, ptr %arrayidx21, align 4
  %conv22 = sext i32 %5 to i64
  %add23 = add nsw i64 %or, %conv22
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next54, 4294967296
  br i1 %exitcond55, label %for.inc27, label %for.body17

for.inc27:                                        ; preds = %for.body17
  %add2343.lcssa = phi i64 [ %add2343, %for.body17 ]
  %add23.lcssa = phi i64 [ %add23, %for.body17 ]
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58 = icmp eq i64 %indvars.iv.next57, 5
  br i1 %exitcond58, label %for.inc30, label %for.cond15.preheader

for.inc30:                                        ; preds = %for.inc27
  %add2343.lcssa.lcssa = phi i64 [ %add2343.lcssa, %for.inc27 ]
  %add23.lcssa.lcssa = phi i64 [ %add23.lcssa, %for.inc27 ]
  %add31 = add nsw i32 %add3150, 2
  %tobool = icmp eq i32 %add31, 0
  br i1 %tobool, label %for.cond.for.end32_crit_edge, label %for.cond1.preheader

for.cond.for.end32_crit_edge:                     ; preds = %for.inc30
  %add2343.lcssa.lcssa.lcssa = phi i64 [ %add2343.lcssa.lcssa, %for.inc30 ]
  %add23.lcssa.lcssa.lcssa = phi i64 [ %add23.lcssa.lcssa, %for.inc30 ]
  %bf.set.lcssa.lcssa.lcssa = phi i8 [ %bf.set.lcssa.lcssa, %for.inc30 ]
  %6 = add i32 %2, 2
  %conv18 = trunc i64 %add2343.lcssa.lcssa.lcssa to i32
  store i8 %bf.set.lcssa.lcssa.lcssa, ptr @f, align 8
  store i32 0, ptr @j, align 4
  store i32 6, ptr @i, align 4
  store i64 %add23.lcssa.lcssa.lcssa, ptr @h, align 8
  store i32 %conv18, ptr @d, align 4
  store i32 5, ptr @e, align 4
  store i32 %6, ptr @k, align 4
  br label %for.end32

for.end32:                                        ; preds = %for.cond.for.end32_crit_edge, %entry
  ret void
}

