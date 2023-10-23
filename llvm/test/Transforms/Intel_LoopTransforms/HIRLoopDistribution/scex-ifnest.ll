; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -aa-pipeline="basic-aa" -print-after=hir-loop-distribute-memrec -disable-output -hir-loop-distribute-skip-vectorization-profitability-check=true -hir-loop-distribute-scex-cost=3 < %s 2>&1 | FileCheck %s


; Verify that we correctly handle distribution and scex for def/use refs when both
; are under an if and the use ref is under a deeper if nest. Previously %x.addr.026
; would be loaded outside the if, which is incorrect since it is liveout.


; *** IR Dump Before HIR Loop Distribution
;          BEGIN REGION { }
;                + DO i1 = 0, %u + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;                |   if (i1 > %cmp)
;                |   {
;                |      %x.addr.026 = %x.addr.026  +  1;
;                |      if (i1 < %v)
;                |      {
;                |         %y.addr.023 = %y.addr.023  +  1;
;                |         %0 = (@A1)[0][%x.addr.026];
;                |         %1 = (@A)[0][%x.addr.026];
;                |         %2 = (@B)[0][%y.addr.023];
;                |         (@B1)[0][%x.addr.026] = %0 + (%1 * %2);
;                |      }
;                |   }
;                + END LOOP
;          END REGION

; *** IR Dump After HIR Loop Distribution MemRec (hir-loop-distribute-memrec) ***
; CHECK:   BEGIN REGION { modified }
; CHECK:         + DO i1 = 0, (%u + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;                |   %min = (-64 * i1 + %u + -1 <= 63) ? -64 * i1 + %u + -1 : 63;
;                |
; CHECK:         |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; CHECK:         |   |   if (64 * i1 + i2 > %cmp)
;                |   |   {
; CHECK:         |   |      %x.addr.026 = %x.addr.026  +  1;
; CHECK:         |   |      (%.TempArray)[0][i2] = %x.addr.026;
;                |   |      if (64 * i1 + i2 < %v)
;                |   |      {
;                |   |         %y.addr.023 = %y.addr.023  +  1;
;                |   |         %0 = (@A1)[0][%x.addr.026];
;                |   |         (%.TempArray2)[0][i2] = %0;
;                |   |         %1 = (@A)[0][%x.addr.026];
;                |   |         (%.TempArray4)[0][i2] = %1;
;                |   |         %2 = (@B)[0][%y.addr.023];
;                |   |         (%.TempArray6)[0][i2] = %2;
;                |   |      }
;                |   |   }
; CHECK:         |   + END LOOP
;                |
;                |
; CHECK:         |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;                |   |   %0 = (@A1)[0][%x.addr.026];
;                |   |   %1 = (@A)[0][%x.addr.026];
;                |   |   %2 = (%.TempArray2)[0][i2];
; CHECK:         |   |   if (64 * i1 + i2 > %cmp)
;                |   |   {
; CHECK:         |   |      if (64 * i1 + i2 < %v)
;                |   |      {
; CHECK:         |   |         %x.addr.026 = (%.TempArray)[0][i2];
; CHECK-NEXT     |   |         (@B1)[0][%x.addr.026] = %0 + (%1 * %2);
;                |   |      }
;                |   |   }
;                |   + END LOOP
;                + END LOOP
;          END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A1 = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B1 = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 %u, i32 %v, i32 %x, i32 %y, i32 %z, i32 %cmp) local_unnamed_addr #0 {
entry:
  %cmp122 = icmp sgt i32 %u, 0
  br i1 %cmp122, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %x.addr.026 = phi i32 [ %x.addr.1, %for.inc ], [ %x, %for.body.preheader ]
  %z.addr.024 = phi i32 [ %inc13, %for.inc ], [ 0, %for.body.preheader ]
  %y.addr.023 = phi i32 [ %y.addr.1, %for.inc ], [ %y, %for.body.preheader ]
  %cmp2 = icmp sgt i32 %z.addr.024, %cmp
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %x.addr.026, 1
  %cmp3 = icmp slt i32 %z.addr.024, %v
  br i1 %cmp3, label %if.then4, label %for.inc

if.then4:                                         ; preds = %if.then
  %inc5 = add nsw i32 %y.addr.023, 1
  %idxprom = sext i32 %inc to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A1, i64 0, i64 %idxprom, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx7, align 4, !tbaa !3
  %idxprom8 = sext i32 %inc5 to i64
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %idxprom8, !intel-tbaa !3
  %2 = load i32, ptr %arrayidx9, align 4, !tbaa !3
  %mul = mul nsw i32 %2, %1
  %add = add nsw i32 %mul, %0
  %arrayidx11 = getelementptr inbounds [100 x i32], ptr @B1, i64 0, i64 %idxprom, !intel-tbaa !3
  store i32 %add, ptr %arrayidx11, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then4, %if.then
  %y.addr.1 = phi i32 [ %inc5, %if.then4 ], [ %y.addr.023, %if.then ], [ %y.addr.023, %for.body ]
  %x.addr.1 = phi i32 [ %inc, %if.then4 ], [ %inc, %if.then ], [ %x.addr.026, %for.body ]
  %inc13 = add nuw nsw i32 %z.addr.024, 1
  %exitcond.not = icmp eq i32 %inc13, %u
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.inc
  %x.addr.1.lcssa = phi i32 [ %x.addr.1, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %x.addr.0.lcssa = phi i32 [ %x, %entry ], [ %x.addr.1.lcssa, %for.end.loopexit ]
  ret i32 %x.addr.0.lcssa
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
