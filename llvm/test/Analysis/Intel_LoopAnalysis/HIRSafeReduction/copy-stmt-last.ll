; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,hir-lmm,print<hir-safe-reduction-analysis>" -disable-output < %s 2>&1 | FileCheck %s

; After complete unroll of i2 loop and loop memory motion on i1 loop,
; the resulting HIR reveals a new pattern for safe reduction chain:
;    %t1 = ... + %t3
;    %t3 = %t1
;
; Test checks that 2 safe reduction chains were recognized involving
; %limm and %limm6.

; Original HIR:
;     BEGIN REGION { }
;        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        |   %0 = (@A)[0][i1][1];
;        |
;        |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll>
;        |   |   %add = (@B)[0][i2]  +  %0;
;        |   |   (@B)[0][i2] = %add;
;        |   + END LOOP
;        + END LOOP
;     END REGION


; HIR after unroll and lmm:
;     BEGIN REGION { modified }
;           %limm = (@B)[0][0];
;           %limm6 = (@B)[0][1];
;        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        |   %0 = (@A)[0][i1][1];
;        |   %add = %limm  +  %0;
;        |   %limm = %add;
;        |   %add = %limm6  +  %0;
;        |   %limm6 = %add;
;        + END LOOP
;           (@B)[0][1] = %limm6;
;           (@B)[0][0] = %limm;
;     END REGION


; CHECK:        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   <Safe Reduction> Red Op: fadd <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:        |   %add = %limm  +  %0; <Safe Reduction>
; CHECK:        |   %limm = %add; <Safe Reduction>
; CHECK:        |   <Safe Reduction> Red Op: fadd <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:        |   %add = %limm6  +  %0; <Safe Reduction>
; CHECK:        |   %limm6 = %add; <Safe Reduction>
; CHECK:        + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [2 x double]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [2 x double] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local double @foo(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.body.preheader, label %entry.for.end11_crit_edge

entry.for.end11_crit_edge:                        ; preds = %entry
  %.pre = sext i32 %n to i64
  br label %for.end11

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc9
  %indvars.iv27 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next28, %for.inc9 ]
  %arrayidx1 = getelementptr inbounds [100 x [2 x double]], ptr @A, i64 0, i64 %indvars.iv27, i64 1, !intel-tbaa !3
  %0 = load double, ptr %arrayidx1, align 8, !tbaa !3
  br label %for.body4

for.body4:                                        ; preds = %for.body, %for.body4
  %cmp3 = phi i1 [ true, %for.body ], [ false, %for.body4 ]
  %indvars.iv = phi i64 [ 0, %for.body ], [ 1, %for.body4 ]
  %arrayidx6 = getelementptr inbounds [2 x double], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !9
  %1 = load double, ptr %arrayidx6, align 8, !tbaa !9
  %add = fadd fast double %1, %0
  store double %add, ptr %arrayidx6, align 8, !tbaa !9
  br i1 %cmp3, label %for.body4, label %for.inc9, !llvm.loop !10

for.inc9:                                         ; preds = %for.body4
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next28, %wide.trip.count
  br i1 %exitcond.not, label %for.end11.loopexit, label %for.body, !llvm.loop !13

for.end11.loopexit:                               ; preds = %for.inc9
  br label %for.end11

for.end11:                                        ; preds = %for.end11.loopexit, %entry.for.end11_crit_edge
  %idxprom12.pre-phi = phi i64 [ %.pre, %entry.for.end11_crit_edge ], [ %wide.trip.count, %for.end11.loopexit ]
  %arrayidx13 = getelementptr inbounds [2 x double], ptr @B, i64 0, i64 %idxprom12.pre-phi, !intel-tbaa !9
  %2 = load double, ptr %arrayidx13, align 8, !tbaa !9
  ret double %2
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA100_A2_d", !5, i64 0}
!5 = !{!"array@_ZTSA2_d", !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!5, !6, i64 0}
!10 = distinct !{!10, !11, !12}
!11 = !{!"llvm.loop.mustprogress"}
!12 = !{!"llvm.loop.unroll.enable"}
!13 = distinct !{!13, !11}
