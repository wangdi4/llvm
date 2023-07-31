; REQUIRES: asserts
;RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -aa-pipeline="basic-aa" -debug-only=hir-loop-interchange -disable-output  < %s 2>&1 | FileCheck %s
;
;CHECK-NOT: Interchanged
;
; Loop is not interchanged because a perfect loopnest is not enabled.
; An anti-edge from <4> to <21> with DV Equal is required for sinking.
;
; <0>  BEGIN REGION { }
; <27>  + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
; <4>   |   %c.027 = (@A)[0][i1 + 1];
; <28>  |
; <28>  |   + DO i2 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; <11>  |   |   %c.027 = (@B)[0][i2][i1]  +  %c.027;
; <28>  |   + END LOOP
; <28>  |
; <21>  |   (@A)[0][i1 + -1] = %c.027;
; <27>  + END LOOP
; <0>  END REGION
;
;
;Module Before HIR; ModuleID = 't1.c'

source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %M) local_unnamed_addr #0 {
entry:
  %cmp29 = icmp sgt i32 %M, 0
  br i1 %cmp29, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %M to i64
  br label %for.body4.lr.ph

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4.lr.ph:                                  ; preds = %for.body.lr.ph, %for.cond.cleanup3
  %indvars.iv31 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv.next32
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add9.lcssa = phi i32 [ %add9, %for.body4 ]
  %1 = add nsw i64 %indvars.iv31, -1
  %arrayidx11 = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %1
  store i32 %add9.lcssa, ptr %arrayidx11, align 4, !tbaa !2
  %exitcond35 = icmp eq i64 %indvars.iv.next32, %wide.trip.count
  br i1 %exitcond35, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %c.027 = phi i32 [ %0, %for.body4.lr.ph ], [ %add9, %for.body4 ]
  %arrayidx8 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv31
  %2 = load i32, ptr %arrayidx8, align 4, !tbaa !7
  %add9 = add nsw i32 %2, %c.027
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 2dae522329424d1c3266a6a2b321b0b493dec2b0)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_A1000_i", !3, i64 0}
