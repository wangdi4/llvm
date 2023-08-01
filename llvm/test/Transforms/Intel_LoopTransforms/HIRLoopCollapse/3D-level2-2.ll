; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: test collecting perfect-loopnest(s) starting from an outermost lp
;
; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
; Note: collapse happens over i2-i3 level.
;
; *** Source Code ***
;
;float A[10][10][10];
;float B[10][10][10];
;int C;
;
;int foo(void) {
;  unsigned i, j, k;
;
;  for (i = 0; i < 10; ++i) {
;    if (C >= 4) {
;      for (j = 0; j < 10; ++j) {
;        for (k = 0; k < 10; ++k) {
;          A[i][j][k] = A[i][j][k] + B[i][j][k] + 1;
;          ++C;
;        }
;      }
;    }
;  }
;
;  return A[0][0][0] + B[1][1][1] + 1;
;}
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   if (%C.promoted52 > 3)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |      |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |      |   |   %0 = (@A)[0][i1][i2][i3];
; CHECK:        |      |   |   %1 = (@B)[0][i1][i2][i3];
; CHECK:        |      |   |   %add = %0  +  %1;
; CHECK:        |      |   |   %add18 = %add  +  1.000000e+00;
; CHECK:        |      |   |   (@A)[0][i1][i2][i3] = %add18;
; CHECK:        |      |   + END LOOP
; CHECK:        |      + END LOOP
; CHECK:        |
; CHECK:        |      %C.promoted52 = %C.promoted52  +  100;
; CHECK:        |      (@C)[0] = %C.promoted52;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   if (%C.promoted52 > 3)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        |      |   %0 = (@A)[0][i1][0][i2];
; CHECK:        |      |   %1 = (@B)[0][i1][0][i2];
; CHECK:        |      |   %add = %0  +  %1;
; CHECK:        |      |   %add18 = %add  +  1.000000e+00;
; CHECK:        |      |   (@A)[0][i1][0][i2] = %add18;
; CHECK:        |      + END LOOP
; CHECK:        |
; CHECK:        |      %C.promoted52 = %C.promoted52  +  100;
; CHECK:        |      (@C)[0] = %C.promoted52;
; CHECK:        |   }
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

@C = common local_unnamed_addr global i32 0, align 4
@A = common local_unnamed_addr global [10 x [10 x [10 x float]]] zeroinitializer, align 16
@B = common local_unnamed_addr global [10 x [10 x [10 x float]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  %.pre = load i32, ptr @C, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.inc29, %entry
  %C.promoted52 = phi i32 [ %.pre, %entry ], [ %3, %for.inc29 ]
  %indvars.iv58 = phi i64 [ 0, %entry ], [ %indvars.iv.next59, %for.inc29 ]
  %cmp1 = icmp sgt i32 %C.promoted52, 3
  br i1 %cmp1, label %for.cond5.preheader.preheader, label %for.inc29

for.cond5.preheader.preheader:                    ; preds = %for.body
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond5.preheader.preheader, %for.inc26
  %indvars.iv55 = phi i64 [ %indvars.iv.next56, %for.inc26 ], [ 0, %for.cond5.preheader.preheader ]
  br label %for.body7

for.body7:                                        ; preds = %for.body7, %for.cond5.preheader
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body7 ]
  %arrayidx11 = getelementptr inbounds [10 x [10 x [10 x float]]], ptr @A, i64 0, i64 %indvars.iv58, i64 %indvars.iv55, i64 %indvars.iv
  %0 = load float, ptr %arrayidx11, align 4, !tbaa !5
  %arrayidx17 = getelementptr inbounds [10 x [10 x [10 x float]]], ptr @B, i64 0, i64 %indvars.iv58, i64 %indvars.iv55, i64 %indvars.iv
  %1 = load float, ptr %arrayidx17, align 4, !tbaa !5
  %add = fadd float %0, %1
  %add18 = fadd float %add, 1.000000e+00
  store float %add18, ptr %arrayidx11, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc26, label %for.body7

for.inc26:                                        ; preds = %for.body7
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 10
  br i1 %exitcond57, label %for.inc29.loopexit, label %for.cond5.preheader

for.inc29.loopexit:                               ; preds = %for.inc26
  %2 = add i32 %C.promoted52, 100
  store i32 %2, ptr @C, align 4, !tbaa !1
  br label %for.inc29

for.inc29:                                        ; preds = %for.inc29.loopexit, %for.body
  %3 = phi i32 [ %2, %for.inc29.loopexit ], [ %C.promoted52, %for.body ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond60 = icmp eq i64 %indvars.iv.next59, 10
  br i1 %exitcond60, label %for.end31, label %for.body

for.end31:                                        ; preds = %for.inc29
  %4 = load float, ptr @A, align 16, !tbaa !5
  %5 = load float, ptr getelementptr inbounds ([10 x [10 x [10 x float]]], ptr @B, i64 0, i64 1, i64 1, i64 1), align 4, !tbaa !5
  %add32 = fadd float %4, %5
  %add33 = fadd float %add32, 1.000000e+00
  %conv = fptosi float %add33 to i32
  ret i32 %conv
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21413)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !3, i64 0}
