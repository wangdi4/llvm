; Verify that the test does not fail after iteration bug in DDUtils was fixed. 
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIRSinkingForPerfectLoopnestPass ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<30>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<3>                |   %t2.048 = 1;
;<4>                |   %cmp6.not47 = -1;
;<31>               |   
;<31>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<10>               |   |   %cmp5 = (@A)[0][0][i2 + 1] <= i1;
;<11>               |   |   %cmp8.not = (%cmp6.not47 != 0) ? %cmp5 : 0;
;<13>               |   |   %t2.048 = (%cmp8.not != 0) ? %t2.048 : i2 + 1;
;<15>               |   |   %cmp6.not47 = 0;
;<31>               |   + END LOOP
;<31>               |   
;<24>               |   (@A)[0][0][%t2.048] = i1 + 1;
;<30>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIRSinkingForPerfectLoopnestPass ***
;Function: foo
;
; CHECK:         BEGIN REGION { }
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end
  %i.050 = phi i32 [ 0, %entry ], [ %add24, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t2.048 = phi i32 [ 1, %for.cond1.preheader ], [ %2, %for.body3 ]
  %cmp6.not47 = phi i1 [ true, %for.cond1.preheader ], [ false, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 0, i64 %indvars.iv.next
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %cmp5 = icmp sle i32 %0, %i.050
  %cmp8.not = select i1 %cmp6.not47, i1 %cmp5, i1 false
  %1 = trunc i64 %indvars.iv.next to i32
  %2 = select i1 %cmp8.not, i32 %t2.048, i32 %1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body3, !llvm.loop !9

for.end:                                          ; preds = %for.body3
  %.lcssa = phi i32 [ %2, %for.body3 ]
  %add24 = add nuw nsw i32 %i.050, 1
  %idxprom25 = sext i32 %.lcssa to i64
  %arrayidx26 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 0, i64 %idxprom25
  store i32 %add24, ptr %arrayidx26, align 4, !tbaa !3
  %exitcond51.not = icmp eq i32 %add24, 100
  br i1 %exitcond51.not, label %for.end29, label %for.cond1.preheader, !llvm.loop !11

for.end29:                                        ; preds = %for.end
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA100_A100_i", !5, i64 0}
!5 = !{!"array@_ZTSA100_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
