; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that original (@e)[0][3 * i1 + i2 + 2] --> (@e)[0][3 * i1 + 2] OUTPUT (= <=) edge
; is split into these two edges by DD
; * Forward edge   (@e)[0][3 * i1 + 2] --> (@e)[0][3 * i1 + i2 + 2] OUTPUT (= =)
; * Backward edge  (@e)[0][3 * i1 + i2 + 2] --> (@e)[0][3 * i1 + 2] OUTPUT (= <)

; <0>          BEGIN REGION { }
; <39>               + DO i1 = 0, 1, 1   <DO_LOOP>
; <40>               |   + DO i2 = 0, -3 * i1 + 1, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; <12>               |   |   %5 = (@e)[0][3 * i1 + 2];
; <14>               |   |   (@e)[0][3 * i1 + 2] = -3 * i1 + %5 + -2;
; <16>               |   |   (@e)[0][3 * i1 + i2 + 2] = 3 * i1 + 2;
; <40>               |   + END LOOP
; <40>               |
; <30>               |   (@i)[0][3 * i1 + 2] = 6 * i1 + 6;
; <39>               + END LOOP
; <0>          END REGION

; CHECK-DAG: 14:16 (@e)[0][3 * i1 + 2] --> (@e)[0][3 * i1 + i2 + 2] OUTPUT (= =)
; CHECK-DAG: 16:12 (@e)[0][3 * i1 + i2 + 2] --> (@e)[0][3 * i1 + 2] FLOW (= <)
; CHECK-NOT: 16:12 (@e)[0][3 * i1 + i2 + 2] --> (@e)[0][3 * i1 + 2] OUTPUT (= <=)

@c = dso_local local_unnamed_addr global i32 2, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4
@d = dso_local local_unnamed_addr global i32 0, align 4
@e = dso_local local_unnamed_addr global [20 x i8] zeroinitializer, align 16
@i = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

define dso_local i32 @main() {
entry:
  %.pr = load i32, i32* @b, align 4
  %cmp29 = icmp slt i32 %.pr, 20
  br i1 %cmp29, label %for.body.lr.ph, label %for.cond1.preheader

for.body.lr.ph:                                   ; preds = %entry
  %c.promoted = load i32, i32* @c, align 4
  %0 = add i32 %c.promoted, 80
  %1 = shl i32 %.pr, 2
  %2 = sub i32 %0, %1
  store i32 %2, i32* @c, align 4
  store i32 20, i32* @b, align 4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.body.lr.ph, %entry
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.cond.cleanup
  %indvars.iv = phi i64 [ 2, %for.cond1.preheader ], [ %indvars.iv.next, %for.cond.cleanup ]
  %storemerge28 = phi i32 [ 2, %for.cond1.preheader ], [ %add21, %for.cond.cleanup ]
  %conv = trunc i32 %storemerge28 to i8
  %cmp626 = icmp slt i8 %conv, 4
  br i1 %cmp626, label %for.body8.lr.ph, label %for.cond.cleanup

for.body8.lr.ph:                                  ; preds = %for.body3
  %arrayidx = getelementptr inbounds [20 x i8], [20 x i8]* @e, i64 0, i64 %indvars.iv
  br label %for.body8

for.cond.cleanup.loopexit:                        ; preds = %for.body8
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %for.body3
  %arrayidx19 = getelementptr inbounds [20 x i32], [20 x i32]* @i, i64 0, i64 %indvars.iv
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %3 = shl i32 %indvars.iv.tr, 1
  %4 = add i32 %3, 2
  store i32 %4, i32* %arrayidx19, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %add21 = add nuw nsw i32 %storemerge28, 3
  %cmp2 = icmp ult i64 %indvars.iv, 4
  br i1 %cmp2, label %for.body3, label %for.end22

for.body8:                                        ; preds = %for.body8.lr.ph, %for.body8
  %indvars.iv32 = phi i64 [ %indvars.iv, %for.body8.lr.ph ], [ %indvars.iv.next33, %for.body8 ]
  %5 = load i8, i8* %arrayidx, align 1
  %conv10 = sub i8 %5, %conv
  store i8 %conv10, i8* %arrayidx, align 1
  %arrayidx13 = getelementptr inbounds [20 x i8], [20 x i8]* @e, i64 0, i64 %indvars.iv32
  store i8 %conv, i8* %arrayidx13, align 1
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next33, 4
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body8

for.end22:                                        ; preds = %for.cond.cleanup
  store i32 8, i32* @d, align 4
  ret i32 0
}
