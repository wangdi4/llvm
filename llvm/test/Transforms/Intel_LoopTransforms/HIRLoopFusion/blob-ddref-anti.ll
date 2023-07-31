; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, (-1 * %.pr + -2)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   (@b)[0][i2 + 1] = %conv10.lcssa24;
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   %3 = (@b)[0][i2 + 1];
;       |   |   %conv10.lcssa24 = %conv10.lcssa24  &&  %3;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK: DO i1

; CHECK:   DO i2
; CHECK:   END LOOP

; CHECK:   DO i2
; CHECK:   END LOOP

; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = common dso_local local_unnamed_addr global i32 0, align 4
@c = common dso_local local_unnamed_addr global i32 0, align 4
@e = common dso_local local_unnamed_addr global i32 0, align 4
@b = external dso_local local_unnamed_addr global [0 x i64], align 8

; Function Attrs: norecurse nounwind uwtable
define dso_local void @f() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @d, align 4
  %tobool22 = icmp eq i32 %.pr, 0
  br i1 %tobool22, label %for.end15, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %e.promoted23 = load i32, ptr @e, align 4
  %0 = sub i32 -2, %.pr
  %1 = and i32 %0, -2
  %2 = add i32 %.pr, %1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc14
  %add25 = phi i32 [ %.pr, %for.cond1.preheader.lr.ph ], [ %add, %for.inc14 ]
  %conv10.lcssa24 = phi i32 [ %e.promoted23, %for.cond1.preheader.lr.ph ], [ %conv10.lcssa, %for.inc14 ]
  %conv = sext i32 %conv10.lcssa24 to i64
  br label %for.body2

for.body2:                                        ; preds = %for.body2, %for.cond1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body2 ]
  %arrayidx = getelementptr inbounds [0 x i64], ptr @b, i64 0, i64 %indvars.iv
  store i64 %conv, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.body6.preheader, label %for.body2

for.body6.preheader:                              ; preds = %for.body2
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv26 = phi i64 [ %indvars.iv.next27, %for.body6 ], [ 1, %for.body6.preheader ]
  %conv1021 = phi i32 [ %conv10, %for.body6 ], [ %conv10.lcssa24, %for.body6.preheader ]
  %arrayidx8 = getelementptr inbounds [0 x i64], ptr @b, i64 0, i64 %indvars.iv26
  %3 = load i64, ptr %arrayidx8, align 8
  %4 = trunc i64 %3 to i32
  %conv10 = and i32 %conv1021, %4
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next27, 11
  br i1 %exitcond28, label %for.inc14, label %for.body6

for.inc14:                                        ; preds = %for.body6
  %conv10.lcssa = phi i32 [ %conv10, %for.body6 ]
  %add = add nsw i32 %add25, 2
  %tobool = icmp eq i32 %add, 0
  br i1 %tobool, label %for.cond.for.end15_crit_edge, label %for.cond1.preheader

for.cond.for.end15_crit_edge:                     ; preds = %for.inc14
  %conv10.lcssa.lcssa = phi i32 [ %conv10.lcssa, %for.inc14 ]
  %5 = add i32 %2, 2
  store i32 %conv10.lcssa.lcssa, ptr @e, align 4
  store i32 11, ptr @c, align 4
  store i32 %5, ptr @d, align 4
  br label %for.end15

for.end15:                                        ; preds = %for.cond.for.end15_crit_edge, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


