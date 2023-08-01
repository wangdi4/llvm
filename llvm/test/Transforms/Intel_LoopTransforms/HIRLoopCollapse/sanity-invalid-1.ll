; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Analysis]
; Applicable: YES
; Legal:      NO (CollapseLevel for B[i][j][0] is 0, not legal)
; Profitable: N/A
; Suitable:   NO
;
;
; *** Source Code ***
;int A[10][10];
;int B[10][10][10];
;
;int foo(void) {
;  int sum = 0;
;  int i, j;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      sum += A[i][j] + B[i][j][0] + 1;
;    }
;  }
;  return A[0][0] + B[0][0][0] + sum + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %sum.030.out = %sum.030;
; CHECK:        |   |   %0 = (@A)[0][i1][i2];
; CHECK:        |   |   %1 = (@B)[0][i1][i2][0];
; CHECK:        |   |   %sum.030 = %0 + %sum.030.out + 1  +  %1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   %sum.030.out = %sum.030;
; CHECK:        |   |   %0 = (@A)[0][i1][i2];
; CHECK:        |   |   %1 = (@B)[0][i1][i2][0];
; CHECK:        |   |   %sum.030 = %0 + %sum.030.out + 1  +  %1;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
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
@B = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %entry
  %indvars.iv32 = phi i64 [ 0, %entry ], [ %indvars.iv.next33, %for.inc13 ]
  %sum.030 = phi i32 [ 0, %entry ], [ %add12, %for.inc13 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %sum.128 = phi i32 [ %sum.030, %for.cond1.preheader ], [ %add12, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %indvars.iv32, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4, !tbaa !1
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @B, i64 0, i64 %indvars.iv32, i64 %indvars.iv, i64 0
  %1 = load i32, ptr %arrayidx10, align 8, !tbaa !1
  %add = add i32 %sum.128, 1
  %add11 = add i32 %add, %0
  %add12 = add i32 %add11, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc13, label %for.body3

for.inc13:                                        ; preds = %for.body3
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 10
  br i1 %exitcond34, label %for.end15, label %for.cond1.preheader

for.end15:                                        ; preds = %for.inc13
  %2 = load i32, ptr @A, align 16, !tbaa !1
  %3 = load i32, ptr @B, align 16, !tbaa !1
  %add16 = add i32 %add12, 1
  %add17 = add i32 %add16, %2
  %add18 = add i32 %add17, %3
  ret i32 %add18
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21179)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
