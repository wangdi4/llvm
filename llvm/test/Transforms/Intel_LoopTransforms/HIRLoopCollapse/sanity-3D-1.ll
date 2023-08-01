; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
;
; *** Source Code ***
;int A[10][10][10];
;
;int foo(void) {
;  int sum = 0;
;  int i, j, k;
;
;  for (i = 0; i <= 9; ++i) {
;    for (j = 0; j <= 9; ++j) {
;      for (k = 0; k <= 9; ++k) {
;        sum += A[i][j][k] + 1;
;      }
;    }
;  }
;  return A[0][0][0] + sum + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   %sum.035.out = %sum.035;
; CHECK:        |   |   |   %0 = (@A)[0][i1][i2][i3];
; CHECK:        |   |   |   %sum.035 = %sum.035.out + 1  +  %0;
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
; CHECK:        |   %sum.035.out = %sum.035;
; CHECK:        |   %0 = (@A)[0][0][0][i1];
; CHECK:        |   %sum.035 = %sum.035.out + 1  +  %0;
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

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc15, %entry
  %indvars.iv40 = phi i64 [ 0, %entry ], [ %indvars.iv.next41, %for.inc15 ]
  %sum.035 = phi i32 [ 0, %entry ], [ %add11, %for.inc15 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc12, %for.cond1.preheader
  %indvars.iv37 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next38, %for.inc12 ]
  %sum.133 = phi i32 [ %sum.035, %for.cond1.preheader ], [ %add11, %for.inc12 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %sum.231 = phi i32 [ %sum.133, %for.cond4.preheader ], [ %add11, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr @A, i64 0, i64 %indvars.iv40, i64 %indvars.iv37, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx10, align 4, !tbaa !1
  %add = add i32 %sum.231, 1
  %add11 = add i32 %add, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc12, label %for.body6

for.inc12:                                        ; preds = %for.body6
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond39 = icmp eq i64 %indvars.iv.next38, 10
  br i1 %exitcond39, label %for.inc15, label %for.cond4.preheader

for.inc15:                                        ; preds = %for.inc12
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %exitcond42 = icmp eq i64 %indvars.iv.next41, 10
  br i1 %exitcond42, label %for.end17, label %for.cond1.preheader

for.end17:                                        ; preds = %for.inc15
  %1 = load i32, ptr @A, align 16, !tbaa !1
  %add18 = add i32 %add11, 1
  %add19 = add i32 %add18, %1
  ret i32 %add19
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21198) (llvm/branches/loopopt 21314)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
