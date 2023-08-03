; negative test: check that mm+index idiom analyzer bailouts in non-idiom cases.
; In this test we have mm-index converted from a float which is partially calculated
; from a load.
; REQUIRES: asserts
; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert' -enable-mmindex=1 -disable-nonlinear-mmindex=1 -debug-only=parvec-analysis -S < %s 2>&1 | FileCheck %s
;
;CHECK: [MinMax+Index] Looking at candidate
;CHECK-NEXT: [MinMax+Index] Depends on
;CHECK: [MinMax+Index] Skipped: dependency on non-select node
;
;<0>          BEGIN REGION { }
;<28>               + DO i1 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>
;<4>                |   %0 = (%ordering)[i1];
;<6>                |   if (%0 > %best.029)
;<6>                |   {
;<15>               |      %cond = (%o2)[i1];
;<6>                |   }
;<6>                |   else
;<6>                |   {
;<10>               |      %conv = sitofp.i32.float(%tmp.030);
;<11>               |      %cond = %conv;
;<6>                |   }
;<19>               |   %conv4 = fptosi.float.i32(%cond);
;<20>               |   %best.029 = (%0 > %best.029) ? %0 : %best.029;
;<23>               |   %tmp.030 = %conv4;
;<28>               + END LOOP
;<0>          END REGION
;
; Function Attrs: norecurse nounwind readonly uwtable

define dso_local i32 @_Z6maxlociPiPf(i32 %m, ptr nocapture readonly %ordering, ptr nocapture readonly %o2) local_unnamed_addr #0 {
entry:
  %cmp28 = icmp sgt i32 %m, 0
  br i1 %cmp28, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %cond.end
  %conv4.lcssa = phi i32 [ %conv4, %cond.end ]
  %.best.0.lcssa = phi i32 [ %.best.0, %cond.end ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %best.0.lcssa = phi i32 [ -111111111, %entry ], [ %.best.0.lcssa, %for.cond.cleanup.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %conv4.lcssa, %for.cond.cleanup.loopexit ]
  %add = add nsw i32 %tmp.0.lcssa, %best.0.lcssa
  ret i32 %add

for.body:                                         ; preds = %cond.end, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %cond.end ]
  %tmp.030 = phi i32 [ 0, %for.body.preheader ], [ %conv4, %cond.end ]
  %best.029 = phi i32 [ -111111111, %for.body.preheader ], [ %.best.0, %cond.end ]
  %arrayidx = getelementptr inbounds i32, ptr %ordering, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, %best.029
  br i1 %cmp1, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body
  %arrayidx3 = getelementptr inbounds float, ptr %o2, i64 %indvars.iv
  %1 = load float, ptr %arrayidx3, align 4, !tbaa !6
  br label %cond.end

cond.false:                                       ; preds = %for.body
  %conv = sitofp i32 %tmp.030 to float
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi float [ %1, %cond.true ], [ %conv, %cond.false ]
  %conv4 = fptosi float %cond to i32
  %.best.0 = select i1 %cmp1, i32 %0, i32 %best.029
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}

