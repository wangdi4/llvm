; Checks if instructions in preheader and postexit are sinked into their parent loop to enable perfect loopnests.
; Also, make sure loop interchange happens.

; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -aa-pipeline="basic-aa" < %s -debug-only=hir-loop-interchange 2>&1 | FileCheck %s
;
;
; CHECK: Interchanged: ( 1 2 3 ) --> ( 2 3 1 )
;
;
; *** IR Dump After HIR Loop Interchange ***
;
; IR Dump Before HIR Loop Interchange ***
; Function: _Z16gemm_blockedPdS_S_iii
;
; <0>       BEGIN REGION { }
; <56>         + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>
; <57>         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; <58>         |   |   + DO i3 = 0, sext.i32.i64(%K) + -1, 1   <DO_LOOP>
; <17>         |   |   |   %add1850 = (%matC)[i1 + sext.i32.i64(%M) * i2];
; <29>         |   |   |   %mul17 = (%matA)[i1 + sext.i32.i64(%M) * i3]  *  (%matB)[sext.i32.i64(%K) * i2 + i3];
; <30>         |   |   |   %add1850 = %add1850  +  %mul17;
; <38>         |   |   |   (%matC)[i1 + sext.i32.i64(%M) * i2] = %add1850;
; <58>         |   |   + END LOOP
; <57>         |   + END LOOP
; <56>         + END LOOP
; <0>       END REGION
;
;
; *** IR Dump After HIR Loop Interchange ***
; Function: _Z16gemm_blockedPdS_S_iii
;
; <0>       BEGIN REGION { modified }
; <56>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; <57>            |   + DO i2 = 0, sext.i32.i64(%K) + -1, 1   <DO_LOOP>
; <58>            |   |   + DO i3 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>
; <17>            |   |   |   %add1850 = (%matC)[sext.i32.i64(%M) * i1 + i3];
; <29>            |   |   |   %mul17 = (%matA)[sext.i32.i64(%M) * i2 + i3]  *  (%matB)[sext.i32.i64(%K) * i1 + i2];
; <30>            |   |   |   %add1850 = %add1850  +  %mul17;
; <38>            |   |   |   (%matC)[sext.i32.i64(%M) * i1 + i3] = %add1850;
; <58>            |   |   + END LOOP
; <57>            |   + END LOOP
; <56>            + END LOOP
; <0>       END REGION


;Module Before HIR; ModuleID = 'gemm-jira.cpp'
source_filename = "gemm-jira.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @_Z16gemm_blockedPdS_S_iii(ptr noalias nocapture readonly %matA, ptr noalias nocapture readonly %matB, ptr noalias nocapture %matC, i32 %M, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %cmp53 = icmp sgt i32 %M, 0
  br i1 %cmp53, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp251 = icmp sgt i32 %N, 0
  %cmp648 = icmp sgt i32 %K, 0
  %0 = sext i32 %M to i64
  %1 = sext i32 %K to i64
  %wide.trip.count64 = sext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph
  %indvars.iv66 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next67, %for.cond.cleanup3 ]
  br i1 %cmp251, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.body
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup7
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond69 = icmp eq i64 %indvars.iv.next67, %0
  br i1 %exitcond69, label %for.cond.cleanup.loopexit, label %for.body

for.body4:                                        ; preds = %for.cond.cleanup7, %for.body4.lr.ph
  %indvars.iv59 = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next60, %for.cond.cleanup7 ]
  br i1 %cmp648, label %for.body8.lr.ph, label %for.cond.cleanup7

for.body8.lr.ph:                                  ; preds = %for.body4
  %2 = mul nsw i64 %indvars.iv59, %0
  %3 = add nsw i64 %2, %indvars.iv66
  %arrayidx = getelementptr inbounds double, ptr %matC, i64 %3
  %4 = mul nsw i64 %indvars.iv59, %1
  %arrayidx.promoted = load double, ptr %arrayidx, align 8, !tbaa !2
  br label %for.body8

for.cond5.for.cond.cleanup7_crit_edge:            ; preds = %for.body8
  %add18.lcssa = phi double [ %add18, %for.body8 ]
  store double %add18.lcssa, ptr %arrayidx, align 8, !tbaa !2
  br label %for.cond.cleanup7

for.cond.cleanup7:                                ; preds = %for.cond5.for.cond.cleanup7_crit_edge, %for.body4
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond65 = icmp eq i64 %indvars.iv.next60, %wide.trip.count64
  br i1 %exitcond65, label %for.cond.cleanup3.loopexit, label %for.body4

for.body8:                                        ; preds = %for.body8, %for.body8.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body8.lr.ph ], [ %indvars.iv.next, %for.body8 ]
  %add1850 = phi double [ %arrayidx.promoted, %for.body8.lr.ph ], [ %add18, %for.body8 ]
  %5 = mul nsw i64 %indvars.iv, %0
  %6 = add nsw i64 %5, %indvars.iv66
  %arrayidx12 = getelementptr inbounds double, ptr %matA, i64 %6
  %7 = load double, ptr %arrayidx12, align 8, !tbaa !2
  %8 = add nsw i64 %indvars.iv, %4
  %arrayidx16 = getelementptr inbounds double, ptr %matB, i64 %8
  %9 = load double, ptr %arrayidx16, align 8, !tbaa !2
  %mul17 = fmul double %7, %9
  %add18 = fadd double %add1850, %mul17
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond, label %for.cond5.for.cond.cleanup7_crit_edge, label %for.body8
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ff84e38780f57f13ba087f157adf85b0d141dfea)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
