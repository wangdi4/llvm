; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" < %s -debug-only=hir-loop-interchange 2>&1 | FileCheck %s
;
; A perfect loopnest shouldn't be enabled. If a perfect loopnest is enabled the end result will be wrong.
;
; An anti-edge from <2> to <22> identifies <22> as a store for <2>, however, temps %0 and %c.030 does not match. Two anti-edges from <2> and <5> to <22>.
;
; *** IR Dump Before HIR Loop Interchange ***
; Function: foo
;
; <0>       BEGIN REGION { }
; <28>            + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; <2>             |   %0 = (@A)[0][1];
; <5>             |   %c.030 = (@A)[0][i1 + 1];
; <29>            |
; <29>            |   + DO i2 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>
; <14>            |   |   %c.030 = %0 + %c.030  +  (@B)[0][i2][i1];
; <29>            |   + END LOOP
; <29>            |
; <22>            |   (@A)[0][i1 + 1] = %c.030;
; <28>            + END LOOP
; <0>       END REGION
;
; *** IR Dump After HIR Loop Interchange ***
; Function: foo
;
; <0>       BEGIN REGION { }
; <28>            + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; <2>             |   %0 = (@A)[0][1];
; <5>             |   %c.030 = (@A)[0][i1 + 1];
; <29>            |
; <29>            |   + DO i2 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>
; <14>            |   |   %c.030 = %0 + %c.030  +  (@B)[0][i2][i1];
; <29>            |   + END LOOP
; <29>            |
; <22>            |   (@A)[0][i1 + 1] = %c.030;
; <28>            + END LOOP
; <0>       END REGION
;
;
; Source code
;
; int A[K];
; int B[K][K];
; int C[K];
;
; void foo (int M)
; {
;   for (int i = 0 ; i < M; i++) {
;     //int b = A[i]; This will be correct to sink
;     int b = A[1];
;     int c = A[i+1];
;     for (int j = 0 ; j < M; j++) {
;       c = c + b + B[j][i];
;     }
;     A[i+1] = c;
;   }
; }
;
; CHECK: Function
;
; CHECK:     BEGIN REGION { }
; CHECK:     DO i1 = 0, sext.i32.i64(%M) + -1, 1
;               %0 = (@A)[0][1];
;               %c.030 = (@A)[0][i1 + 1];
;
; CHECK:        DO i2 = 0, sext.i32.i64(%M) + -1, 1
;
; CHECK: Function
;
; CHECK:     BEGIN REGION { }
; CHECK:     DO i1 = 0, sext.i32.i64(%M) + -1, 1
;               %0 = (@A)[0][1];
;               %c.030 = (@A)[0][i1 + 1];
;
; CHECK:        DO i2 = 0, sext.i32.i64(%M) + -1, 1


;Module Before HIR; ModuleID = 'sink-should-be-blocked.c'
source_filename = "sink-should-be-blocked.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [5 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [5 x [5 x i32]] zeroinitializer, align 16
@C = common local_unnamed_addr global [5 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %M) local_unnamed_addr #0 {
entry:
  %cmp32 = icmp sgt i32 %M, 0
  br i1 %cmp32, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %M to i64
  br label %for.body4.lr.ph

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4.lr.ph:                                  ; preds = %for.body.lr.ph, %for.cond.cleanup3
  %indvars.iv34 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next35, %for.cond.cleanup3 ]
  %0 = load i32, ptr getelementptr inbounds ([5 x i32], ptr @A, i64 0, i64 1), align 4, !tbaa !2
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %arrayidx = getelementptr inbounds [5 x i32], ptr @A, i64 0, i64 %indvars.iv.next35
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add10.lcssa = phi i32 [ %add10, %for.body4 ]
  store i32 %add10.lcssa, ptr %arrayidx, align 4, !tbaa !2
  %exitcond37 = icmp eq i64 %indvars.iv.next35, %wide.trip.count
  br i1 %exitcond37, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %c.030 = phi i32 [ %1, %for.body4.lr.ph ], [ %add10, %for.body4 ]
  %add5 = add nsw i32 %c.030, %0
  %arrayidx9 = getelementptr inbounds [5 x [5 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv34
  %2 = load i32, ptr %arrayidx9, align 4, !tbaa !7
  %add10 = add nsw i32 %add5, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1d86f3083c0798d8869b537e1ba2529d135374a1)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA5_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA5_A5_i", !3, i64 0}
