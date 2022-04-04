; Test that this lit test can be passed with opaque pointer and get the correct value of the
; const aggregate variable to create the const ref in DDRefUtils::simplifyConstArray().
; 
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR PreVec Complete Unroll (hir-pre-vec-complete-unroll) ***
;Function: e
;
;<0>          BEGIN REGION { }
;<21>               + DO i1 = 0, sext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<22>               |   + DO i2 = 0, 7, 1   <DO_LOOP>
;<6>                |   |   %10 = (@g)[i2];
;<7>                |   |   %11 = @l(%10);
;<22>               |   + END LOOP
;<21>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR PreVec Complete Unroll (hir-pre-vec-complete-unroll) ***
;Function: e
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, sext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   %11 = @l(4);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           |   %11 = @l(0);
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.b = type { ptr }

@g = internal unnamed_addr constant <{ i32, [8 x i32] }> <{ i32 4, [8 x i32] zeroinitializer }>, align 16
@f = internal global %struct.b { ptr @e }, align 8

; Function Attrs: nounwind uwtable
define internal void @e(i32 noundef %0) #2 {
  %2 = sext i32 %0 to i64
  %3 = icmp sgt i32 %0, 0
  br i1 %3, label %4, label %18

4:                                                ; preds = %1
  br label %5

5:                                                ; preds = %4, %14
  %6 = phi i64 [ %15, %14 ], [ 0, %4 ]
  br label %7

7:                                                ; preds = %7, %5
  %8 = phi i64 [ 0, %5 ], [ %12, %7 ]
  %9 = getelementptr inbounds i32, ptr @g, i64 %8
  %10 = load i32, ptr %9, align 4, !tbaa !10
  %11 = tail call i32 (i32, ...) @l(i32 noundef %10) #4
  %12 = add nuw nsw i64 %8, 1
  %13 = icmp eq i64 %12, 8
  br i1 %13, label %14, label %7, !llvm.loop !12

14:                                               ; preds = %7
  %15 = add nuw nsw i64 %6, 1
  %16 = icmp eq i64 %15, %2
  br i1 %16, label %17, label %5, !llvm.loop !14

17:                                               ; preds = %14
  br label %18

18:                                               ; preds = %17, %1
  ret void
}

declare dso_local i32 @l(...) local_unnamed_addr #3

attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSP1b", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !8, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = distinct !{!14, !13}
