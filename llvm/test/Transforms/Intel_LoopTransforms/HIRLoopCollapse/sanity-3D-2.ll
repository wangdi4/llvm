; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
;
; Note: loop collapse triggered over i1-i2-i3 level
;
; *** Source Code ***
;int A[10][10][10];
;int foo(void) {
;  int i, j;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (j = 0; j <= 9; ++j) {
;        A[i][j][k] += A[i][j][k] + 100*i+10*j+1;
;      }
;    }
;  }
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
; CHECK:        |   |   |   %4 = (@A)[0][i1][i2][i3];
; CHECK:        |   |   |   (@A)[0][i1][i2][i3] = 100 * i1 + 10 * i2 + i3 + 2 * %4 + 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:        |   %4 = (@A)[0][0][0][i1];
; CHECK:        |   (@A)[0][0][0][i1] = i1 + 2 * %4 + 1
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

for.cond1.preheader:                              ; preds = %for.inc25, %entry
  %indvars.iv53 = phi i64 [ 0, %entry ], [ %indvars.iv.next54, %for.inc25 ]
  %0 = mul nuw nsw i64 %indvars.iv53, 100
  %1 = or i64 %0, 1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc22, %for.cond1.preheader
  %indvars.iv48 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next49, %for.inc22 ]
  %2 = mul nuw nsw i64 %indvars.iv48, 10
  %3 = add nuw nsw i64 %1, %2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv53, i64 %indvars.iv48, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx10, align 4, !tbaa !1
  %factor = shl i32 %4, 1
  %5 = add nuw nsw i64 %3, %indvars.iv
  %6 = trunc i64 %5 to i32
  %add21 = add i32 %factor, %6
  store i32 %add21, ptr %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc22, label %for.body6

for.inc22:                                        ; preds = %for.body6
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next49, 10
  br i1 %exitcond52, label %for.inc25, label %for.cond4.preheader

for.inc25:                                        ; preds = %for.inc22
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next54, 10
  br i1 %exitcond57, label %for.end27, label %for.cond1.preheader

for.end27:                                        ; preds = %for.inc25
  %7 = load i32, ptr @A, align 16, !tbaa !1
  %add28 = add nsw i32 %7, 1
  ret i32 %add28
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21316) (llvm/branches/loopopt 21393)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
