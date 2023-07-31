; negative test: check that mm+index idiom analyzer bailouts in non-idiom cases.
; In this test we have not-same-order select. Select for index has flipped operands
; comparing to the select for main value ("best" is last operand, while "tmp" is
; the first one).
; REQUIRES: asserts
; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert' -enable-mmindex=1 -disable-nonlinear-mmindex=0 -debug-only=parvec-analysis -S < %s 2>&1 | FileCheck %s
;
;CHECK: [MinMax+Index] Looking at candidate
;CHECK-NEXT: [MinMax+Index] Depends on
;CHECK-NEXT: [MinMax+Index] Skipped: dependency on not-same-order select
;
;<0>          BEGIN REGION { }
;<15>               + DO i1 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>
;<4>                |   %0 = (%ordering)[i1];
;<7>                |   %tmp.027 = (%0 > %best.026) ? %tmp.027 : i1;
;<8>                |   %best.026 = (%0 > %best.026) ? %0 : %best.026;
;<15>               + END LOOP
;<0>          END REGION
;
; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z6maxloc2iPi(i32 %m, ptr nocapture readonly %ordering) local_unnamed_addr {
entry:
  %cmp25 = icmp sgt i32 %m, 0
  br i1 %cmp25, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %.lcssa = phi i32 [ %2, %for.body ]
  %.best.0.lcssa = phi i32 [ %.best.0, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %best.0.lcssa = phi i32 [ -111111111, %entry ], [ %.best.0.lcssa, %for.cond.cleanup.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %.lcssa, %for.cond.cleanup.loopexit ]
  %add = add nsw i32 %tmp.0.lcssa, %best.0.lcssa
  ret i32 %add

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %tmp.027 = phi i32 [ 0, %for.body.preheader ], [ %2, %for.body ]
  %best.026 = phi i32 [ -111111111, %for.body.preheader ], [ %.best.0, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %ordering, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, %best.026
  %1 = trunc i64 %indvars.iv to i32
  %2 = select i1 %cmp1, i32 %tmp.027, i32 %1
  %.best.0 = select i1 %cmp1, i32 %0, i32 %best.026
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}


!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}

