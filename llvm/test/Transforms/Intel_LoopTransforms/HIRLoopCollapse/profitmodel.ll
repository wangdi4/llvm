; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: tests for advanced profit models
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES (1 loop is good, 2 other loops are bad)
; Suitable:   YES (1 loop is good, 2 other loops are bad)
;
;
; *** Source Code ***
;
;int A[10][10][10];
;int foo(void) {
;  int i, j, k;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (k = 0; k <= 9; ++k) {
;        A[i][j][k] = i; //can collapse on j-k level, not i-j-k
;      }
;    }
;  }
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (k = 0; k <= 9; ++k) {
;        A[i][j][k] = j; //can't collapse, gap between i and k
;      }
;    }
;  }
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (k = 0; k <= 9; ++k) {
;        A[i][j][k] = k; //may extend to allow collapse over i-j level
;      }
;    }
;  }
;
;  return A[0][0][0] + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = i1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
;
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = i2;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
;
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = i3;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   |   (@A)[0][i1][0][i2] = i1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
;
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = i2;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
;
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = i3;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %entry
  %indvars.iv124 = phi i64 [ 0, %entry ], [ %indvars.iv.next125, %for.inc14 ]
  %0 = trunc i64 %indvars.iv124 to i32
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc11, %for.cond1.preheader
  %indvars.iv121 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next122, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv118 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next119, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv124, i64 %indvars.iv121, i64 %indvars.iv118
  store i32 %0, ptr %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next119 = add nuw nsw i64 %indvars.iv118, 1
  %exitcond120 = icmp eq i64 %indvars.iv.next119, 10
  br i1 %exitcond120, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next122 = add nuw nsw i64 %indvars.iv121, 1
  %exitcond123 = icmp eq i64 %indvars.iv.next122, 10
  br i1 %exitcond123, label %for.inc14, label %for.cond4.preheader

for.inc14:                                        ; preds = %for.inc11
  %indvars.iv.next125 = add nuw nsw i64 %indvars.iv124, 1
  %exitcond126 = icmp eq i64 %indvars.iv.next125, 10
  br i1 %exitcond126, label %for.cond20.preheader, label %for.cond1.preheader

for.cond20.preheader:                             ; preds = %for.inc14, %for.inc38
  %indvars.iv115 = phi i64 [ %indvars.iv.next116, %for.inc38 ], [ 0, %for.inc14 ]
  br label %for.cond23.preheader

for.cond23.preheader:                             ; preds = %for.inc35, %for.cond20.preheader
  %indvars.iv112 = phi i64 [ 0, %for.cond20.preheader ], [ %indvars.iv.next113, %for.inc35 ]
  %1 = trunc i64 %indvars.iv112 to i32
  br label %for.body25

for.body25:                                       ; preds = %for.body25, %for.cond23.preheader
  %indvars.iv109 = phi i64 [ 0, %for.cond23.preheader ], [ %indvars.iv.next110, %for.body25 ]
  %arrayidx31 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv115, i64 %indvars.iv112, i64 %indvars.iv109
  store i32 %1, ptr %arrayidx31, align 4, !tbaa !1
  %indvars.iv.next110 = add nuw nsw i64 %indvars.iv109, 1
  %exitcond111 = icmp eq i64 %indvars.iv.next110, 10
  br i1 %exitcond111, label %for.inc35, label %for.body25

for.inc35:                                        ; preds = %for.body25
  %indvars.iv.next113 = add nuw nsw i64 %indvars.iv112, 1
  %exitcond114 = icmp eq i64 %indvars.iv.next113, 10
  br i1 %exitcond114, label %for.inc38, label %for.cond23.preheader

for.inc38:                                        ; preds = %for.inc35
  %indvars.iv.next116 = add nuw nsw i64 %indvars.iv115, 1
  %exitcond117 = icmp eq i64 %indvars.iv.next116, 10
  br i1 %exitcond117, label %for.cond44.preheader, label %for.cond20.preheader

for.cond44.preheader:                             ; preds = %for.inc38, %for.inc62
  %indvars.iv106 = phi i64 [ %indvars.iv.next107, %for.inc62 ], [ 0, %for.inc38 ]
  br label %for.cond47.preheader

for.cond47.preheader:                             ; preds = %for.inc59, %for.cond44.preheader
  %indvars.iv103 = phi i64 [ 0, %for.cond44.preheader ], [ %indvars.iv.next104, %for.inc59 ]
  br label %for.body49

for.body49:                                       ; preds = %for.body49, %for.cond47.preheader
  %indvars.iv = phi i64 [ 0, %for.cond47.preheader ], [ %indvars.iv.next, %for.body49 ]
  %arrayidx55 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv106, i64 %indvars.iv103, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %arrayidx55, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc59, label %for.body49

for.inc59:                                        ; preds = %for.body49
  %indvars.iv.next104 = add nuw nsw i64 %indvars.iv103, 1
  %exitcond105 = icmp eq i64 %indvars.iv.next104, 10
  br i1 %exitcond105, label %for.inc62, label %for.cond47.preheader

for.inc62:                                        ; preds = %for.inc59
  %indvars.iv.next107 = add nuw nsw i64 %indvars.iv106, 1
  %exitcond108 = icmp eq i64 %indvars.iv.next107, 10
  br i1 %exitcond108, label %for.end64, label %for.cond44.preheader

for.end64:                                        ; preds = %for.inc62
  %3 = load i32, ptr @A, align 16, !tbaa !1
  %add = add nsw i32 %3, 1
  ret i32 %add
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21423)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
