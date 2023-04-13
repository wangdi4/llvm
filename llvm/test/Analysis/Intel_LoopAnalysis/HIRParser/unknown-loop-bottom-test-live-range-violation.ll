; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>" -disable-output  2>&1 | FileCheck %s

; Verify that the bottom test of i3 loop is not reverse engineered in terms of
; %indvars.iv747 which is the IV of the i3 loop since that will cause live-range
; violation as the IV would be redefined at the end of the loop using livein
; copy.
; Before the change, loop was parsed as follows-
;
; |      |      %indvars.iv747 = %smin;
; |      |
; |      |      + UNKNOWN LOOP i3
; |      |      |   <i3 = 0>
; |      |      |   bb642:
; |      |      |   %indvars.iv747 = -1 * i3 + %smin + -1;
; |      |      |   if (%indvars.iv747 > %"(i32)func_result2$")
; |      |      |   {
; |      |      |      <i3 = i3 + 1>
; |      |      |      goto bb642;
; |      |      |   }
; |      |      + END LOOP

; Notice that %indvars.iv747 has been decremented by 1 at the end of the loop
; before being used in the bottom test.

; CHECK: |      |      %smin = @llvm.smin.i64(i1 + -1 * i2 + %t0,  i2);
; CHECK: |      |
; CHECK: |      |      + UNKNOWN LOOP i3
; CHECK: |      |      |   <i3 = 0>
; CHECK: |      |      |   bb642:
; CHECK: |      |      |   if (-1 * i3 + %smin > %func_result2si)
; CHECK: |      |      |   {
; CHECK: |      |      |      <i3 = i3 + 1>
; CHECK: |      |      |      goto bb642;
; CHECK: |      |      |   }
; CHECK: |      |      + END LOOP


define void @foo(i32 %in, i64 %t0, i32 %t1) {
entry:
  br label %bb634

bb634:                                            ; preds = %entry, %bb639
  %indvars.iv758.in = phi i32 [ %in, %entry ], [ %indvars.iv758, %bb639 ]
  %indvars.iv = phi i64 [ %t0, %entry ], [ %indvars.iv.next, %bb639 ]
  %powers.60 = phi i32 [ 1, %entry ], [ %powers.65, %bb639 ]
  %indvars.iv758 = add i32 %indvars.iv758.in, 1
  %t2 = trunc i64 %indvars.iv to i32
  %t3 = add i32 %t2, 2
  %add.14 = sitofp i32 %t3 to double
  %mul.350 = fmul reassoc ninf nsz arcp contract afn double %add.14, 0x3FD5555555555555
  %func_result = tail call reassoc ninf nsz arcp contract afn double @llvm.floor.f64(double %mul.350)
  %func_resultsi = fptosi double %func_result to i32
  %t4 = sext i32 %func_resultsi to i64
  %rel.626 = icmp slt i64 %indvars.iv, %t4
  br i1 %rel.626, label %bb639, label %bb638.preheader

bb638.preheader:                                  ; preds = %bb634
  %t5 = sub i32 %indvars.iv758, %func_resultsi
  br label %bb638

bb638:                                            ; preds = %bb638.preheader, %bb643
  %indvars.iv745 = phi i32 [ 0, %bb638.preheader ], [ %indvars.iv.next746, %bb643 ]
  %indvars.iv743 = phi i64 [ %indvars.iv, %bb638.preheader ], [ %indvars.iv.next744, %bb643 ]
  %powers.61 = phi i32 [ %powers.60, %bb638.preheader ], [ %powers.64, %bb643 ]
  %powers.6a = phi i32 [ %t2, %bb638.preheader ], [ %add.1583, %bb643 ]
  %t6 = sub nsw i64 %indvars.iv, %indvars.iv743
  %sub.272 = sub nsw i32 %t2, %powers.6a
  %t7 = trunc i64 %indvars.iv743 to i32
  %slct.286 = tail call i32 @llvm.smin.i32(i32 %sub.272, i32 %t7)
  %t8 = trunc i64 %t6 to i32
  %t9 = add i32 %t8, 1
  %add.1434 = sitofp i32 %t9 to double
  %mul.351 = fmul reassoc ninf nsz arcp contract afn double %add.1434, 5.000000e-01
  %func_result2 = tail call reassoc ninf nsz arcp contract afn double @llvm.floor.f64(double %mul.351)
  %func_result2si = fptosi double %func_result2 to i32
  %rel.628 = icmp slt i32 %slct.286, %func_result2si
  br i1 %rel.628, label %bb643, label %bb642.preheader

bb642.preheader:                                  ; preds = %bb638
  %t10 = sext i32 %indvars.iv745 to i64
  %smin = call i64 @llvm.smin.i64(i64 %indvars.iv743, i64 %t10)
  %t11 = add nsw i64 %indvars.iv743, 1
  %t12 = sext i32 %func_result2si to i64
  br label %bb642

bb642:                                            ; preds = %bb642.preheader, %bb713_endif
  %indvars.iv747 = phi i64 [ %smin, %bb642.preheader ], [ %indvars.iv.next748, %bb713_endif ]
  %powers.62 = phi i32 [ %powers.61, %bb642.preheader ], [ %powers.63, %bb713_endif ]
  %powers.6b0 = phi i32 [ %slct.286, %bb642.preheader ], [ %add.1582, %bb713_endif ]
  %sub.275 = sub nsw i32 %t8, %powers.6b0
  %t13 = trunc i64 %indvars.iv747 to i32
  %rel.629 = icmp eq i32 %t7, %t13
  %rel.630 = icmp eq i32 %sub.275, %t13
  %and.31 = and i1 %rel.629, %rel.630
  br i1 %and.31, label %bb_new1617_then, label %bb_new1625_else

loop_exit1622:                                    ; preds = %loop_body1621
  %add.1445 = add nsw i32 %powers.62, 1
  br label %bb713_endif

loop_exit1643:                                    ; preds = %loop_body1642
  %add.1479 = add nsw i32 %powers.62, 3
  br label %bb713_endif

loop_exit1666:                                    ; preds = %loop_body1665
  %add.1513 = add nsw i32 %powers.62, 3
  br label %bb713_endif

loop_exit1703:                                    ; preds = %loop_body1702
  %add.1581 = add nsw i32 %powers.62, 6
  br label %bb713_endif

bb_new1648_else:                                  ; preds = %bb_new1625_else
  %t16 = sext i32 %sub.275 to i64
  %rel.638 = icmp sgt i64 %indvars.iv747, %t16
  %and.33 = and i1 %rel.629, %rel.638
  br i1 %and.33, label %loop_exit1666, label %loop_exit1703

bb_new1617_then:                                  ; preds = %bb642
  %int_sext = sext i32 %powers.62 to i64
  br label %loop_exit1622

bb_new1625_else:                                  ; preds = %bb642
  %rel.632 = icmp sgt i64 %indvars.iv743, %indvars.iv747
  %and.32 = and i1 %rel.632, %rel.630
  br i1 %and.32, label %loop_exit1643, label %bb_new1648_else

bb713_endif:                                      ; preds = %loop_exit1643, %loop_exit1703, %loop_exit1666, %loop_exit1622
  %powers.63 = phi i32 [ %add.1445, %loop_exit1622 ], [ %add.1479, %loop_exit1643 ], [ %add.1513, %loop_exit1666 ], [ %add.1581, %loop_exit1703 ]
  %indvars.iv.next748 = add nsw i64 %indvars.iv747, -1
  %add.1582 = add nsw i32 %powers.6b0, -1
  %rel.648.not.not = icmp sgt i64 %indvars.iv747, %t12
  br i1 %rel.648.not.not, label %bb642, label %bb643.loopexit

bb643.loopexit:                                   ; preds = %bb713_endif
  %powers.63lcssa = phi i32 [ %powers.63, %bb713_endif ]
  br label %bb643

bb643:                                            ; preds = %bb643.loopexit, %bb638
  %powers.64 = phi i32 [ %powers.61, %bb638 ], [ %powers.63lcssa, %bb643.loopexit ]
  %indvars.iv.next744 = add nsw i64 %indvars.iv743, -1
  %add.1583 = add nsw i32 %powers.6a, -1
  %indvars.iv.next746 = add i32 %indvars.iv745, 1
  %exitcond760.not = icmp eq i32 %indvars.iv.next746, %t5
  br i1 %exitcond760.not, label %bb639.loopexit, label %bb638

bb639.loopexit:                                   ; preds = %bb643
  %powers.64lcssa = phi i32 [ %powers.64, %bb643 ]
  br label %bb639

bb639:                                            ; preds = %bb639.loopexit, %bb634
  %powers.65 = phi i32 [ %powers.60, %bb634 ], [ %powers.64lcssa, %bb639.loopexit ]
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond762 = icmp eq i32 %t1, %lftr.wideiv
  br i1 %exitcond762, label %bb635.loopexit, label %bb634

bb635.loopexit:
  ret void
}

declare double @llvm.floor.f64(double)
declare i32 @llvm.smin.i32(i32, i32)
declare i64 @llvm.smin.i64(i64, i64)
