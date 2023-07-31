
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion" -disable-output -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s

; The first loop have no vectorization preventing edges. The second loop has
; vectorization preventing edge due to Safe reduction statement. Test checks
; that safe reduction edge was ignored and loops were fused. 

;<0>          BEGIN REGION { }
;<27>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<4>                |   (@a)[0][i1] = i1;
;<27>               + END LOOP
;<27>
;<15>               %s.023 = 0;
;<28>
;<28>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<20>               |   %s.023 = (@a)[0][i1]  +  %s.023;
;<28>               + END LOOP
;<0>          END REGION


;*** IR Dump After HIRLoopFusionPass ***
; CHECK:     BEGIN REGION { modified }
; CHECK:           %s.023 = 0;
;       
; CHECK:           + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   (@a)[0][i1] = i1;
; CHECK:           |   %s.023 = (@a)[0][i1]  +  %s.023;
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.end8

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond1.preheader:                              ; preds = %for.body
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %i = trunc i64 %indvars.iv to i32
  store i32 %i, ptr %arrayidx, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond1.preheader, label %for.body, !llvm.loop !8

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv24 = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next25, %for.body3 ]
  %s.023 = phi i32 [ 0, %for.body3.preheader ], [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %indvars.iv24, !intel-tbaa !3
  %i1 = load i32, ptr %arrayidx5, align 4, !tbaa !3
  %add = add nsw i32 %i1, %s.023
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond27.not = icmp eq i64 %indvars.iv.next25, %wide.trip.count
  br i1 %exitcond27.not, label %for.end8.loopexit, label %for.body3, !llvm.loop !10

for.end8.loopexit:                                ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa, %for.end8.loopexit ]
  ret i32 %s.0.lcssa
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
