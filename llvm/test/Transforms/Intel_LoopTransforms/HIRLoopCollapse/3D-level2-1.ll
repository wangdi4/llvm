; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: basic test for a valid case
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
; Note: collapse happens over i2-i3 level.
;
; *** Source Code ***
;int A[10][10][10];
;int B[10][10][10];
;
;int foo(void) {
;  int i, j, k;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (k = 0; k <= 9; ++k) {
;        A[i][j][k] = B[2][j][k] + 1;
;      }
;    }
;  }
;  return A[0][0][0] + B[1][2][3] + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   %0 = (@B)[0][2][i2][i3];
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = %0 + 1;
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
; CHECK:        |   |   %0 = (@B)[0][2][0][i2];
; CHECK:        |   |   (@A)[0][i1][0][i2] = %0 + 1;
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

@B = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16
@A = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc18, %entry
  %indvars.iv39 = phi i64 [ 0, %entry ], [ %indvars.iv.next40, %for.inc18 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc15, %for.cond1.preheader
  %indvars.iv36 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next37, %for.inc15 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @B, i64 0, i64 2, i64 %indvars.iv36, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx8, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %arrayidx14 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv39, i64 %indvars.iv36, i64 %indvars.iv
  store i32 %add, ptr %arrayidx14, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc15, label %for.body6

for.inc15:                                        ; preds = %for.body6
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next37, 10
  br i1 %exitcond38, label %for.inc18, label %for.cond4.preheader

for.inc18:                                        ; preds = %for.inc15
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next40, 10
  br i1 %exitcond41, label %for.end20, label %for.cond1.preheader

for.end20:                                        ; preds = %for.inc18
  %1 = load i32, ptr @A, align 16, !tbaa !1
  %2 = load i32, ptr getelementptr inbounds ([10 x [10 x [10 x i32]]], ptr @B, i64 0, i64 1, i64 2, i64 3), align 4, !tbaa !1
  %add21 = add i32 %1, 1
  %add22 = add i32 %add21, %2
  ret i32 %add22
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21316) (llvm/branches/loopopt 21326)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
