; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
;
;
; *** Source Code ***
;int A[10][10];
;int foo(void) {
;  int i, j;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      A[i][j] += A[i][j] + 1;
;    }
;  }
;  return A[0][0] + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %0 = (@A)[0][i1][i2];
; CHECK:        |   |   (@A)[0][i1][i2] = 2 * %0 + 1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %0 = (@A)[0][0][i1];
; CHECK:        |   (@A)[0][0][i1] = 2 * %0 + 1;
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

@A = common local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.inc11 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %indvars.iv24, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4, !tbaa !1
  %factor = shl i32 %0, 1
  %add10 = or i32 %factor, 1
  store i32 %add10, ptr %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 10
  br i1 %exitcond26, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
  %1 = load i32, ptr @A, align 16, !tbaa !1
  %add14 = add nsw i32 %1, 1
  ret i32 %add14
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21316) (llvm/branches/loopopt 21322)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
