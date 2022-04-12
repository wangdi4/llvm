; RUN: opt < %s -analyze -xmain-opt-level=3 -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -xmain-opt-level=3 2>&1 | FileCheck %s

; Check that we successfull generate HIR for this loop. It was previously
; compfailing because the inner loop for (; e > a; e--) was optimized away
; but the framework could not find it when setting livein/liveouts

; Source
;int a, b, c, d, e;
;unsigned int f;
;int main() {
;  int g = scanf;
;  for (; a; a++)
;    if (a > 16) {
;      for (; c; --c)
;        for (; d; --d)
;          ;
;      e = 8;
;      for (; e > a; e--) {
;        f = 1;
;        for (; f; ++f)
;          b *= g;
;      }
;    }
;  return 0;
;}

; CHECK: BEGIN REGION { }
;          + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
;          |   %d.promoted5464.out = %d.promoted5464;
;          |   %dec8.lcssa4865.out = %dec8.lcssa4865;
;          |   if (i1 + %.pr > 16)
;          |   {
;          |      %storemerge.lcssa = 8;
;          |
;          |         %dec.lcssa3135 = %d.promoted5464.out;
;          |      + DO i2 = 0, %dec8.lcssa4865.out + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;          |      |   if (%dec.lcssa3135 != 0)
;          |      |   {
;          |      |      (@d)[0] = 0;
;          |      |      %d.promoted5464 = 0;
;          |      |   }
;          |      |   %dec.lcssa3135 = 0;
;          |      + END LOOP
;          |         (@c)[0] = 0;
;          |         %storemerge.lcssa = 8;
;          |
;          |      (@e)[0] = %storemerge.lcssa;
;          |      %dec8.lcssa4865 = 0;
;          |   }
;          + END LOOP
;        END REGION

@a = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global i32 0, align 4
@d = dso_local local_unnamed_addr global i32 0, align 4
@e = dso_local local_unnamed_addr global i32 0, align 4
@f = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %.pr = load i32, i32* @a, align 4, !tbaa !3
  %tobool.not59 = icmp eq i32 %.pr, 0
  br i1 %tobool.not59, label %for.end23, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %b.promoted = load i32, i32* @b, align 4, !tbaa !3
  %d.promoted49 = load i32, i32* @d, align 4, !tbaa !3
  %c.promoted45 = load i32, i32* @c, align 4, !tbaa !3
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc21
  %dec8.lcssa4865 = phi i32 [ %dec8.lcssa46, %for.inc21 ], [ %c.promoted45, %for.body.preheader ]
  %d.promoted5464 = phi i32 [ %d.promoted50, %for.inc21 ], [ %d.promoted49, %for.body.preheader ]
  %inc225561 = phi i32 [ %inc22, %for.inc21 ], [ %.pr, %for.body.preheader ]
  %mul37.lcssa.lcssa5860 = phi i32 [ %mul37.lcssa.lcssa56, %for.inc21 ], [ %b.promoted, %for.body.preheader ]
  %cmp = icmp sgt i32 %inc225561, 16
  br i1 %cmp, label %for.cond1thread-pre-split, label %for.inc21

for.cond1thread-pre-split:                        ; preds = %for.body
  %tobool2.not33 = icmp eq i32 %dec8.lcssa4865, 0
  br i1 %tobool2.not33, label %for.inc21.loopexit, label %for.cond4thread-pre-split.preheader

for.cond4thread-pre-split.preheader:              ; preds = %for.cond1thread-pre-split
  br label %for.cond4thread-pre-split

for.cond10.preheader:                             ; preds = %for.inc7
  %d.promoted51.lcssa = phi i32 [ %d.promoted51, %for.inc7 ]
  store i32 0, i32* @c, align 4, !tbaa !3
  %cmp1142 = icmp slt i32 %inc225561, 8
  br i1 %cmp1142, label %for.cond13.preheader.preheader, label %for.inc21.loopexit

for.cond13.preheader.preheader:                   ; preds = %for.cond10.preheader
  br label %for.cond13.preheader

for.cond4thread-pre-split:                        ; preds = %for.cond4thread-pre-split.preheader, %for.inc7
  %d.promoted52 = phi i32 [ %d.promoted51, %for.inc7 ], [ %d.promoted5464, %for.cond4thread-pre-split.preheader ]
  %dec.lcssa3135 = phi i32 [ 0, %for.inc7 ], [ %d.promoted5464, %for.cond4thread-pre-split.preheader ]
  %dec83234 = phi i32 [ %dec8, %for.inc7 ], [ %dec8.lcssa4865, %for.cond4thread-pre-split.preheader ]
  %tobool5.not29 = icmp eq i32 %dec.lcssa3135, 0
  br i1 %tobool5.not29, label %for.inc7, label %for.inc.preheader

for.inc.preheader:                                ; preds = %for.cond4thread-pre-split
  store i32 0, i32* @d, align 4, !tbaa !3
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc.preheader, %for.cond4thread-pre-split
  %d.promoted51 = phi i32 [ 0, %for.inc.preheader ], [ %d.promoted52, %for.cond4thread-pre-split ]
  %dec8 = add nsw i32 %dec83234, -1
  %tobool2.not = icmp eq i32 %dec8, 0
  br i1 %tobool2.not, label %for.cond10.preheader, label %for.cond4thread-pre-split, !llvm.loop !7

for.cond13.preheader:                             ; preds = %for.cond13.preheader.preheader, %for.inc18
  %storemerge44 = phi i32 [ %dec19, %for.inc18 ], [ 8, %for.cond13.preheader.preheader ]
  %mul37.lcssa4143 = phi i32 [ %mul.lcssa, %for.inc18 ], [ %mul37.lcssa.lcssa5860, %for.cond13.preheader.preheader ]
  br label %for.body15

for.body15:                                       ; preds = %for.cond13.preheader, %for.body15
  %storemerge2639 = phi i32 [ 1, %for.cond13.preheader ], [ %inc, %for.body15 ]
  %mul3738 = phi i32 [ %mul37.lcssa4143, %for.cond13.preheader ], [ %mul, %for.body15 ]
  %mul = mul nsw i32 %mul3738, ptrtoint (i32 (i8*, ...)* @__isoc99_scanf to i32)
  %inc = add i32 %storemerge2639, 1
  %tobool14.not = icmp eq i32 %inc, 0
  br i1 %tobool14.not, label %for.inc18, label %for.body15, !llvm.loop !9

for.inc18:                                        ; preds = %for.body15
  %mul.lcssa = phi i32 [ %mul, %for.body15 ]
  %dec19 = add nsw i32 %storemerge44, -1
  %cmp11 = icmp sgt i32 %dec19, %inc225561
  br i1 %cmp11, label %for.cond13.preheader, label %for.cond10.for.inc21.loopexit_crit_edge, !llvm.loop !10

for.cond10.for.inc21.loopexit_crit_edge:          ; preds = %for.inc18
  %mul.lcssa.lcssa = phi i32 [ %mul.lcssa, %for.inc18 ]
  store i32 0, i32* @f, align 4, !tbaa !3
  store i32 %mul.lcssa.lcssa, i32* @b, align 4, !tbaa !3
  br label %for.inc21.loopexit

for.inc21.loopexit:                               ; preds = %for.cond1thread-pre-split, %for.cond10.for.inc21.loopexit_crit_edge, %for.cond10.preheader
  %d.promoted5369 = phi i32 [ %d.promoted51.lcssa, %for.cond10.for.inc21.loopexit_crit_edge ], [ %d.promoted51.lcssa, %for.cond10.preheader ], [ %d.promoted5464, %for.cond1thread-pre-split ]
  %mul37.lcssa.lcssa57 = phi i32 [ %mul.lcssa.lcssa, %for.cond10.for.inc21.loopexit_crit_edge ], [ %mul37.lcssa.lcssa5860, %for.cond10.preheader ], [ %mul37.lcssa.lcssa5860, %for.cond1thread-pre-split ]
  %storemerge.lcssa = phi i32 [ %inc225561, %for.cond10.for.inc21.loopexit_crit_edge ], [ 8, %for.cond10.preheader ], [ 8, %for.cond1thread-pre-split ]
  store i32 %storemerge.lcssa, i32* @e, align 4, !tbaa !3
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21.loopexit, %for.body
  %mul37.lcssa.lcssa56 = phi i32 [ %mul37.lcssa.lcssa57, %for.inc21.loopexit ], [ %mul37.lcssa.lcssa5860, %for.body ]
  %d.promoted50 = phi i32 [ %d.promoted5369, %for.inc21.loopexit ], [ %d.promoted5464, %for.body ]
  %dec8.lcssa46 = phi i32 [ 0, %for.inc21.loopexit ], [ %dec8.lcssa4865, %for.body ]
  %inc22 = add nsw i32 %inc225561, 1
  %tobool.not = icmp eq i32 %inc22, 0
  br i1 %tobool.not, label %for.cond.for.end23_crit_edge, label %for.body, !llvm.loop !11

for.cond.for.end23_crit_edge:                     ; preds = %for.inc21
  store i32 0, i32* @a, align 4, !tbaa !3
  br label %for.end23

for.end23:                                        ; preds = %for.cond.for.end23_crit_edge, %entry
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(i8* nocapture noundef readonly, ...) #1

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
!10 = distinct !{!10, !8}
!11 = distinct !{!11, !8}
