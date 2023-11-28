; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-loop-blocking,print<hir>" 2>&1 < %s -disable-output | FileCheck %s

; Verify that the innermost loop's blocking is prevented by vector pragma (e.g. #pragma vector always)
; Verify futher that the outermost loop is not blocked when it is the only loop to be stripmined.
; Blocking only the outermost loop is the same as stripmining of the outermost loop only without interchange.
; Thus, no real effect of transformation as execution order did not change, but only may hinder
; loop vectorization after unit-strided loop is unroll and jammed.

; Before loop blocking after sinking for perfect loopnest
;
; CHECK: Function: ludcmp
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, zext.i32.i64(%n) + -1 * %indvars.iv74, 1   <DO_LOOP>  <MAX_TC_EST = 103>
; CHECK:               |   + DO i2 = 0, %indvars.iv74 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 102>  <LEGAL_MAX_TC = 102> <vectorize>
; CHECK:               |   |   %w.065 = (@a)[0][%indvars.iv80 + 1][i1 + %indvars.iv74];
; CHECK:               |   |   %mul = (@a)[0][i2][i1 + %indvars.iv74]  *  (@a)[0][%indvars.iv80 + 1][i2];
; CHECK:               |   |   %w.065 = %w.065  -  %mul;
; CHECK:               |   |   (@a)[0][%indvars.iv80 + 1][i1 + %indvars.iv74] = %w.065;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

; After loop blocking
;
; CHECK: Function: ludcmp
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, zext.i32.i64(%n) + -1 * %indvars.iv74, 1   <DO_LOOP>  <MAX_TC_EST = 103>
; CHECK:               |   + DO i2 = 0, %indvars.iv74 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 102>  <LEGAL_MAX_TC = 102> <vectorize>
; CHECK:               |   |   %w.065 = (@a)[0][%indvars.iv80 + 1][i1 + %indvars.iv74];
; CHECK:               |   |   %mul = (@a)[0][i2][i1 + %indvars.iv74]  *  (@a)[0][%indvars.iv80 + 1][i2];
; CHECK:               |   |   %w.065 = %w.065  -  %mul;
; CHECK:               |   |   (@a)[0][%indvars.iv80 + 1][i1 + %indvars.iv74] = %w.065;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

;Module Before HIR
; ModuleID = 'shorter.c'
source_filename = "shorter.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [103 x [103 x float]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [103 x float] zeroinitializer, align 16
@x = dso_local local_unnamed_addr global [103 x float] zeroinitializer, align 16

; Function Attrs: nofree nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @ludcmp(i32 noundef %n, double noundef nofpclass(nan inf) %eps) local_unnamed_addr #0 {
entry:
  %cmp1 = fcmp fast ugt double %eps, 0.000000e+00
  %0 = add i32 %n, -1
  %1 = icmp ult i32 %0, 102
  %or.cond70 = select i1 %1, i1 %cmp1, i1 false
  br i1 %or.cond70, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %2 = add nuw nsw i32 %n, 1
  %3 = zext i32 %n to i64
  %wide.trip.count78 = zext i32 %2 to i64
  br label %for.body

for.cond.loopexit.loopexit:                       ; preds = %for.end
  br label %for.cond.loopexit

for.cond.loopexit:                                ; preds = %for.cond.loopexit.loopexit, %if.end8
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond83.not = icmp eq i64 %indvars.iv.next81, %3
  br i1 %exitcond83.not, label %cleanup.loopexit, label %for.body, !llvm.loop !3

for.body:                                         ; preds = %for.body.preheader, %for.cond.loopexit
  %indvars.iv80 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next81, %for.cond.loopexit ]
  %indvars.iv74 = phi i64 [ 1, %for.body.preheader ], [ %indvars.iv.next75, %for.cond.loopexit ]
  %arrayidx4 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %indvars.iv80, i64 %indvars.iv80, !intel-tbaa !5
  %4 = load float, ptr %arrayidx4, align 4, !tbaa !5
  %5 = tail call fast float @llvm.fabs.f32(float %4)
  %6 = fpext float %5 to double
  %cmp5 = fcmp fast ugt double %6, %eps
  br i1 %cmp5, label %if.end8, label %cleanup.loopexit

if.end8:                                          ; preds = %for.body
  %indvars.iv.next81 = add nuw nsw i64 %indvars.iv80, 1
  %cmp10.not66.not = icmp ult i64 %indvars.iv80, %3
  br i1 %cmp10.not66.not, label %for.body12.preheader, label %for.cond.loopexit

for.body12.preheader:                             ; preds = %if.end8
  br label %for.body12

for.body12:                                       ; preds = %for.body12.preheader, %for.end
  %indvars.iv76 = phi i64 [ %indvars.iv.next77, %for.end ], [ %indvars.iv74, %for.body12.preheader ]
  %arrayidx17 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %indvars.iv.next81, i64 %indvars.iv76, !intel-tbaa !5
  %7 = load float, ptr %arrayidx17, align 4, !tbaa !5
  br label %for.body21

for.body21:                                       ; preds = %for.body12, %for.body21
  %indvars.iv = phi i64 [ 0, %for.body12 ], [ %indvars.iv.next, %for.body21 ]
  %w.065 = phi float [ %7, %for.body12 ], [ %sub, %for.body21 ]
  %arrayidx26 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %indvars.iv.next81, i64 %indvars.iv, !intel-tbaa !5
  %8 = load float, ptr %arrayidx26, align 4, !tbaa !5
  %arrayidx30 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %indvars.iv, i64 %indvars.iv76, !intel-tbaa !5
  %9 = load float, ptr %arrayidx30, align 4, !tbaa !5
  %mul = fmul fast float %9, %8
  %sub = fsub fast float %w.065, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %indvars.iv74
  br i1 %exitcond, label %for.end, label %for.body21, !llvm.loop !11

for.end:                                          ; preds = %for.body21
  %sub.lcssa = phi float [ %sub, %for.body21 ]
  store float %sub.lcssa, ptr %arrayidx17, align 4, !tbaa !5
  %indvars.iv.next77 = add nuw nsw i64 %indvars.iv76, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next77, %wide.trip.count78
  br i1 %exitcond79, label %for.cond.loopexit.loopexit, label %for.body12, !llvm.loop !15

cleanup.loopexit:                                 ; preds = %for.cond.loopexit, %for.body
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fabs.f32(float) #1

attributes #0 = { nofree nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA103_A103_f", !7, i64 0}
!7 = !{!"array@_ZTSA103_f", !8, i64 0}
!8 = !{!"float", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = distinct !{!11, !4, !12, !13, !14}
!12 = !{!"llvm.loop.vectorize.ignore_profitability"}
!13 = !{!"llvm.loop.vectorize.enable", i1 true}
!14 = !{!"llvm.loop.intel.vector.assert"}
!15 = distinct !{!15, !4}
