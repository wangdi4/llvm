; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts
; RUN: opt  -passes=tight-loop-emitter -tight-loop-emitter-run=remark -debug-only=tight-loop-emitter  -disable-output < %s 2>&1 | FileCheck %s

; CHECK: Intel Tight Loop Emitter : dotProduct
; CHECK: Tight cycle found for Loop:
; CHECK-NEXT:   phi
; CHECK-NEXT:   fadd


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local float @dotProduct(float* noundef readonly %p1, float* nocapture noundef readonly %p2, i64 noundef %count) local_unnamed_addr {
entry:
  %cmp10 = icmp sgt i64 %count, 0
  br i1 %cmp10, label %region.0, label %for.end

for.end:                                          ; preds = %loop.64, %afterloop.17, %entry
  %result.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %14, %afterloop.17 ], [ %18, %loop.64 ]
  ret float %result.0.lcssa

region.0:                                         ; preds = %entry
  %p161 = ptrtoint float* %p1 to i64
  %0 = shl i64 %count, 2
  %1 = add i64 %0, %p161
  %2 = add i64 %p161, 4
  %umax = tail call i64 @llvm.umax.i64(i64 %1, i64 %2)
  %3 = xor i64 %p161, -1
  %4 = add i64 %umax, %3
  %5 = lshr i64 %4, 2
  %6 = add nuw nsw i64 %5, 1
  %7 = and i64 %6, 9223372036854775800
  %extract.0.42 = icmp eq i64 %7, 0
  br i1 %extract.0.42, label %loop.64.preheader, label %loop.17

loop.17:                                          ; preds = %region.0, %loop.17
  %t29.0 = phi <8 x float> [ %13, %loop.17 ], [ zeroinitializer, %region.0 ]
  %i1.i64.0 = phi i64 [ %nextivloop.17, %loop.17 ], [ 0, %region.0 ]
  %8 = getelementptr inbounds float, float* %p2, i64 %i1.i64.0
  %9 = bitcast float* %8 to <8 x float>*
  %gepload = load <8 x float>, <8 x float>* %9, align 4, !tbaa !3
  %10 = getelementptr inbounds float, float* %p1, i64 %i1.i64.0
  %11 = bitcast float* %10 to <8 x float>*
  %gepload44 = load <8 x float>, <8 x float>* %11, align 4, !tbaa !3
  %12 = fmul fast <8 x float> %gepload, %gepload44
  %13 = fadd fast <8 x float> %12, %t29.0
  %nextivloop.17 = add nuw nsw i64 %i1.i64.0, 8
  %condloop.17.not.not = icmp ult i64 %nextivloop.17, %7
  br i1 %condloop.17.not.not, label %loop.17, label %afterloop.17, !llvm.loop !7

afterloop.17:                                     ; preds = %loop.17
  %14 = tail call fast float @llvm.vector.reduce.fadd.v8f32(float 0.000000e+00, <8 x float> %13)
  %extract.0.3354 = icmp eq i64 %6, %7
  br i1 %extract.0.3354, label %for.end, label %loop.64.preheader

loop.64.preheader:                                ; preds = %afterloop.17, %region.0
  %i1.i64.1.ph = phi i64 [ %7, %afterloop.17 ], [ 0, %region.0 ]
  %t3.1.ph = phi float [ %14, %afterloop.17 ], [ 0.000000e+00, %region.0 ]
  br label %loop.64

loop.64:                                          ; preds = %loop.64.preheader, %loop.64
  %i1.i64.1 = phi i64 [ %nextivloop.64, %loop.64 ], [ %i1.i64.1.ph, %loop.64.preheader ]
  %t3.1 = phi float [ %18, %loop.64 ], [ %t3.1.ph, %loop.64.preheader ]
  %15 = getelementptr inbounds float, float* %p2, i64 %i1.i64.1
  %gepload57 = load float, float* %15, align 4, !tbaa !3
  %16 = getelementptr inbounds float, float* %p1, i64 %i1.i64.1
  %gepload58 = load float, float* %16, align 4, !tbaa !3
  %17 = fmul fast float %gepload57, %gepload58
  %18 = fadd fast float %17, %t3.1
  %nextivloop.64 = add nuw nsw i64 %i1.i64.1, 1
  %condloop.64.not = icmp eq i64 %i1.i64.1, %5
  br i1 %condloop.64.not, label %for.end, label %loop.64, !llvm.loop !12
}

declare float @llvm.vector.reduce.fadd.v8f32(float, <8 x float>)

declare i64 @llvm.umax.i64(i64, i64)

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9, !10, !11}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!"llvm.loop.vectorize.width", i32 1}
!10 = !{!"llvm.loop.interleave.count", i32 1}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!12, !8, !13, !11, !9, !10}
!13 = !{!"llvm.loop.intel.loopcount_maximum", i32 7}
; end INTEL_FEATURE_SW_ADVANCED
