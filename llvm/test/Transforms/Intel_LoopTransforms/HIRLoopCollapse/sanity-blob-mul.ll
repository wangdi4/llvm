; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s


; Verify that Loops with Blob Coefficient is multiplied correctly

; [Analysis]
; Applicable: YES
; Legal:      YES
; Profitable: YES
; Suitable:   YES
;
;
; *** Source Code ***
; int A[10][10];
; int foo(unsigned long X) {
;   int i, j;
;   for (i = 0; i < 2*X; ++i) {
;     for (j = 0; j < 10; ++j) {
;       A[i][j]++;
;     }
;   }
;   return A[1][1];
; }

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 2 * %X + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK:           |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   %0 = (@A)[0][i1][i2];
; CHECK:           |   |   (@A)[0][i1][i2] = %0 + 1;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: foo

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 10 * (2 * %X) + -1, 1   <DO_LOOP>
; CHECK:           |   %0 = (@A)[0][0][i1];
; CHECK:           |   (@A)[0][0][i1] = %0 + 1;
; CHECK:           + END LOOP
; CHECK:     END REGION


;Module Before HIR
; ModuleID = '/nfs/sc/home/liuchen3/test/blocking-denominator.c'
source_filename = "/nfs/sc/home/liuchen3/test/blocking-denominator.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @foo(i64 %X) local_unnamed_addr #0 {
entry:
  %mul = shl i64 %X, 1
  %cmp18 = icmp eq i64 %mul, 0
  br i1 %cmp18, label %for.end11, label %for.cond2.preheader.preheader

for.cond2.preheader.preheader:                    ; preds = %entry
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.preheader, %for.inc9
  %indvars.iv21 = phi i64 [ %indvars.iv.next22, %for.inc9 ], [ 0, %for.cond2.preheader.preheader ]
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.cond2.preheader
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %indvars.iv21, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx7, align 4, !tbaa !2
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc9, label %for.body5

for.inc9:                                         ; preds = %for.body5
  %indvars.iv.next22 = add nuw i64 %indvars.iv21, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next22, %mul
  br i1 %exitcond23, label %for.end11.loopexit, label %for.cond2.preheader

for.end11.loopexit:                               ; preds = %for.inc9
  br label %for.end11

for.end11:                                        ; preds = %for.end11.loopexit, %entry
  %1 = load i32, ptr getelementptr inbounds ([10 x [10 x i32]], ptr @A, i64 0, i64 1, i64 1), align 4, !tbaa !2
  ret i32 %1
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA10_A10_i", !4, i64 0}
!4 = !{!"array@_ZTSA10_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

