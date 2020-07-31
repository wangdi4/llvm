; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -disable-output -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify constant array of structs is replaced with constant values

; CHECK: Function:
; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:       |   |   %0 = (@Glob)[0][i2].0;
; CHECK:       |   |   %2 = (%A)[2 * i2];
; CHECK:       |   |   (%A)[2 * i2] = %0 + %2;
; CHECK:       |   |   %3 = (@Glob)[0][i2].1;
; CHECK:       |   |   %5 = (%A)[2 * i2 + 1];
; CHECK:       |   |   (%A)[2 * i2 + 1] = %3 + %5;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; CHECK: Function:
; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   %0 = 1;
; CHECK:       |   %2 = (%A)[0];
; CHECK:       |   (%A)[0] = %0 + %2;
; CHECK:       |   %3 = 2;
; CHECK:       |   %5 = (%A)[1];
; CHECK:       |   (%A)[1] = %3 + %5;
; CHECK:       |   %0 = 3;
; CHECK:       |   %2 = (%A)[2];
; CHECK:       |   (%A)[2] = %0 + %2;
; CHECK:       |   %3 = 0;
; CHECK:       |   %5 = (%A)[3];
; CHECK:       |   (%A)[3] = %3 + %5;
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, i32 }

@Glob = dso_local local_unnamed_addr constant [2 x %struct.S] [%struct.S { i32 1, i32 2 }, %struct.S { i32 3, i32 0 }], align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %j.027 = phi i32 [ %inc14, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc14 = add nuw nsw i32 %j.027, 1
  %exitcond = icmp eq i32 %inc14, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %cmp2 = phi i1 [ true, %for.cond1.preheader ], [ false, %for.body4 ]
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ 1, %for.body4 ]
  %a = getelementptr inbounds [2 x %struct.S], [2 x %struct.S]* @Glob, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !2
  %0 = load i32, i32* %a, align 8, !tbaa !2
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %ptridx, align 4, !tbaa !8
  %add = add nsw i32 %2, %0
  store i32 %add, i32* %ptridx, align 4, !tbaa !8
  %b = getelementptr inbounds [2 x %struct.S], [2 x %struct.S]* @Glob, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = load i32, i32* %b, align 4, !tbaa !9
  %4 = or i64 %1, 1
  %ptridx11 = getelementptr inbounds i32, i32* %A, i64 %4
  %5 = load i32, i32* %ptridx11, align 4, !tbaa !8
  %add12 = add nsw i32 %5, %3
  store i32 %add12, i32* %ptridx11, align 4, !tbaa !8
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA2_1S", !4, i64 0}
!4 = !{!"struct@S", !5, i64 0, !5, i64 4}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!5, !5, i64 0}
!9 = !{!3, !5, i64 4}
