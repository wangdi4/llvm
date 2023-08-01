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
;      A[i][j] += A[i][j] + 10*i + j + 1;
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
; CHECK:        |   |   %2 = (@A)[0][i1][i2];
; CHECK:        |   |   (@A)[0][i1][i2] = 10 * i1 + i2 + 2 * %2 + 1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   %2 = (@A)[0][0][i1];
; CHECK:        |   (@A)[0][0][i1] = i1 + 2 * %2 + 1;
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

for.cond1.preheader:                              ; preds = %for.inc13, %entry
  %indvars.iv29 = phi i64 [ 0, %entry ], [ %indvars.iv.next30, %for.inc13 ]
  %0 = mul nuw nsw i64 %indvars.iv29, 10
  %1 = or i64 %0, 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx5, align 4, !tbaa !1
  %factor = shl i32 %2, 1
  %3 = add nuw nsw i64 %1, %indvars.iv
  %4 = trunc i64 %3 to i32
  %add12 = add i32 %factor, %4
  store i32 %add12, ptr %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc13, label %for.body3

for.inc13:                                        ; preds = %for.body3
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond33 = icmp eq i64 %indvars.iv.next30, 10
  br i1 %exitcond33, label %for.end15, label %for.cond1.preheader

for.end15:                                        ; preds = %for.inc13
  %5 = load i32, ptr @A, align 16, !tbaa !1
  %add16 = add nsw i32 %5, 1
  ret i32 %add16
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21316) (llvm/branches/loopopt 21322)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
