; RUN: opt  < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,print<hir>,hir-cg" -force-hir-cg -xmain-opt-level=3 -hir-framework-details 2>&1 | FileCheck %s

; Verify that we can successfully generate code after multi-exit loop reroll.
; We were compfailing in CG because multi-exit reroll removed %i757 as region
; liveout even though it was liveout from normal region exit (defined in postexit).


; Incoming HIR to multi-exit reroll-
; + DO i1 = 0, ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4), 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1023>
; |   %i743 = (@log_name)[0][4 * i1];
; |   (%i231)[4 * i1] = %i743;
; |   if (%i743 == 0)
; |   {
; |      %i742 = &((@log_name)[0][4 * i1 + 1]);
; |      goto bb798;
; |   }
; |   %i748 = (@log_name)[0][4 * i1 + 1];
; |   (%i231)[4 * i1 + 1] = %i748;
; |   if (%i748 == 0)
; |   {
; |      %i747 = &((@log_name)[0][4 * i1 + 2]);
; |      goto bb798;
; |   }
; |   %i753 = (@log_name)[0][4 * i1 + 2];
; |   (%i231)[4 * i1 + 2] = %i753;
; |   if (%i753 == 0)
; |   {
; |      %i752 = &((@log_name)[0][4 * i1 + 3]);
; |      goto bb798;
; |   }
; |   %i758 = (@log_name)[0][4 * i1 + 3];
; |   (%i231)[4 * i1 + 3] = %i758;
; |   if (%i758 == 0)
; |   {
; |      %i757 = &((@log_name)[0][4 * i1 + 4]);
; |      goto bb798;
; |   }
; + END LOOP
;   %i757 = &((@log_name)[0][4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4) + 4]);
;   %i761 = &((%i231)[4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4) + 4]);
;   %i762 = %i232 + -4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4)  +  -4;

; CHECK BEGIN REGION
; CHECK: LiveOuts:
; CHECK-SAME: %i757

; CHECK: + DO i1 = 0, 4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4) + 3, 1   <DO_MULTI_EXIT_LOOP> <MAX_TC_EST = 4092>
; CHECK: |   %i743 = (@log_name)[0][i1];
; CHECK: |   (%i231)[i1] = %i743;
; CHECK: |   if (%i743 == 0)
; CHECK: |   {
; CHECK: |      %i742 = &((@log_name)[0][i1 + 1]);
; CHECK: |      goto bb798;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK:   %i757 = &((@log_name)[0][4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4) + 4]);
; CHECK:   %i761 = &((%i231)[4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4) + 4]);
; CHECK:   %i762 = %i232 + -4 * ((-1 + (-1 * umin(4, (-4 + %i232))) + %i232) /u 4)  +  -4;


@log_name = external hidden global [4096 x i8], align 16

define void @foo(i64 %i232, ptr %i231) {
entry:
 br label %loop

loop:                                            ; preds = %latch, %entry
  %i739 = phi i64 [ %i762, %latch ], [ %i232, %entry ]
  %i740 = phi ptr [ %i757, %latch ], [ getelementptr inbounds ([4096 x i8], ptr @log_name, i64 0, i64 0), %entry ]
  %i741 = phi ptr [ %i761, %latch ], [ %i231, %entry ]
  %i742 = getelementptr inbounds i8, ptr %i740, i64 1
  %i743 = load i8, ptr %i740, align 1
  store i8 %i743, ptr %i741, align 1
  %i744 = icmp eq i8 %i743, 0
  br i1 %i744, label %bb798, label %bb745

bb745:                                            ; preds = %loop
  %i746 = getelementptr inbounds i8, ptr %i741, i64 1
  %i747 = getelementptr inbounds i8, ptr %i740, i64 2
  %i748 = load i8, ptr %i742, align 1
  store i8 %i748, ptr %i746, align 1
  %i749 = icmp eq i8 %i748, 0
  br i1 %i749, label %bb798, label %bb750

bb750:                                            ; preds = %bb745
  %i751 = getelementptr inbounds i8, ptr %i741, i64 2
  %i752 = getelementptr inbounds i8, ptr %i740, i64 3
  %i753 = load i8, ptr %i747, align 1
  store i8 %i753, ptr %i751, align 1
  %i754 = icmp eq i8 %i753, 0
  br i1 %i754, label %bb798, label %bb755

bb755:                                            ; preds = %bb750
  %i756 = getelementptr inbounds i8, ptr %i741, i64 3
  %i757 = getelementptr inbounds i8, ptr %i740, i64 4
  %i758 = load i8, ptr %i752, align 1
  store i8 %i758, ptr %i756, align 1
  %i759 = icmp eq i8 %i758, 0
  br i1 %i759, label %bb798, label %latch

latch:                                            ; preds = %bb755
  %i761 = getelementptr inbounds i8, ptr %i741, i64 4
  %i762 = add i64 %i739, -4
  %i763 = icmp ugt i64 %i762, 4
  br i1 %i763, label %loop, label %bb764

bb764:                                            ; preds = %latch
  %i765 = phi ptr [ %i761, %latch ]
  %i766 = phi i64 [ %i762, %latch ]
  %i767 = phi ptr [ %i757, %latch ]
  ret void

bb798:                                            ; preds = %bb755, %bb750, %bb745, %loop
  %i799 = phi ptr [ %i742, %loop ], [ %i747, %bb745 ], [ %i752, %bb750 ], [ %i757, %bb755 ]
  ret void
}
