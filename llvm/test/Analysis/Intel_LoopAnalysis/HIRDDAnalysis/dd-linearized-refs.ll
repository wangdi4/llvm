; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that DD analysis can refine linearized DD edges if refs are the same.

;      BEGIN REGION { }
;          + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   |   |   + DO i4 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   |   |   |   %1 = (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216];
;          |   |   |   |   %add18 = %1  +  1.000000e+00;
;          |   |   |   |   (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] = %add18;
;          |   |   |   + END LOOP
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;      END REGION

; CHECK: DD graph for function _Z3subPfi:
; CHECK-DAG: 20:22 (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] --> (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] ANTI (* * * =) (? ? ? 0)
; CHECK-DAG: 22:20 (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] --> (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] FLOW (* * * =) (? ? ? 0)
; CHECK-DAG: 22:22 (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] --> (%A)[5308416 * i1 + 2304 * i2 + 2304 * i3 + i4 + -9216] OUTPUT (* * * =) (? ? ? 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3subPfi(ptr nocapture noundef %A, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp49 = icmp sgt i32 %N, 0
  br i1 %cmp49, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.cond5.preheader.lr.ph

for.cond5.preheader.lr.ph:                        ; preds = %for.cond.cleanup3, %for.cond1.preheader.lr.ph
  %i1.050 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc26, %for.cond.cleanup3 ]
  %mul = mul nuw nsw i32 %i1.050, 5308416
  %add15 = add nsw i32 %mul, -9216
  br label %for.cond9.preheader.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 0

for.cond9.preheader.preheader:                    ; preds = %for.cond.cleanup7, %for.cond5.preheader.lr.ph
  %i2.048 = phi i32 [ 0, %for.cond5.preheader.lr.ph ], [ %inc23, %for.cond.cleanup7 ]
  br label %for.body12.lr.ph

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc26 = add nuw nsw i32 %i1.050, 1
  %exitcond53.not = icmp eq i32 %inc26, %N
  br i1 %exitcond53.not, label %for.cond.cleanup.loopexit, label %for.cond5.preheader.lr.ph, !llvm.loop !3

for.body12.lr.ph:                                 ; preds = %for.cond.cleanup11, %for.cond9.preheader.preheader
  %i3.046 = phi i32 [ %inc20, %for.cond.cleanup11 ], [ 0, %for.cond9.preheader.preheader ]
  %reass.add = add nuw i32 %i3.046, %i2.048
  %reass.mul = mul i32 %reass.add, 2304
  %add16 = add i32 %add15, %reass.mul
  br label %for.body12

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %inc23 = add nuw nsw i32 %i2.048, 1
  %exitcond52.not = icmp eq i32 %inc23, %N
  br i1 %exitcond52.not, label %for.cond.cleanup3, label %for.cond9.preheader.preheader, !llvm.loop !5

for.cond.cleanup11:                               ; preds = %for.body12
  %inc20 = add nuw nsw i32 %i3.046, 1
  %exitcond51.not = icmp eq i32 %inc20, %N
  br i1 %exitcond51.not, label %for.cond.cleanup7, label %for.body12.lr.ph, !llvm.loop !6

for.body12:                                       ; preds = %for.body12.lr.ph, %for.body12
  %indvars.iv = phi i64 [ 0, %for.body12.lr.ph ], [ %indvars.iv.next, %for.body12 ]
  %0 = trunc i64 %indvars.iv to i32
  %add17 = add i32 %add16, %0
  %idxprom = sext i32 %add17 to i64
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %idxprom
  %1 = load float, ptr %arrayidx, align 4, !tbaa !7
  %add18 = fadd fast float %1, 1.000000e+00
  store float %add18, ptr %arrayidx, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup11, label %for.body12, !llvm.loop !11
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C++ TBAA"}
!11 = distinct !{!11, !4}
