; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s


; Verify that Loops with denominator in UB can now be collapsed

; *** Source Code ***
; int A[10][10];
; void foo(unsigned long n) {
;
;   unsigned long i, j;
;
;   for(i=0; i<(n-1)/2 + 1; i++)
;     for(j=0; j<10; j++)
;       A[i][j] = 10;
;  }

; *** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
; Function: foo
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, (%n + -1)/u2, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   (@A)[0][i1][i2] = 10;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION


; *** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
; Function: foo

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 10 * ((1 + %n) /u 2) + -1, 1   <DO_LOOP>
; CHECK:           |   (@A)[0][0][i1] = 10;
; CHECK:           + END LOOP
; CHECK:     END REGION



;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i64 %n) local_unnamed_addr #0 {
entry:
  %sub = add i64 %n, -1
  %div = lshr i64 %sub, 1
  %0 = add nuw i64 %div, 1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %i.014 = phi i64 [ 0, %entry ], [ %inc6, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.013 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %arrayidx4 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %i.014, i64 %j.013, !intel-tbaa !2
  store i32 10, ptr %arrayidx4, align 4, !tbaa !2
  %inc = add nuw nsw i64 %j.013, 1
  %exitcond = icmp eq i64 %inc, 10
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %inc6 = add nuw i64 %i.014, 1
  %exitcond15 = icmp eq i64 %inc6, %0
  br i1 %exitcond15, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
