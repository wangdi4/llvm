; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -debug-only=hir-general-unroll -S < %s 2>&1 | FileCheck %s

; Verify that we bail out for multi-exit loops with too many liveouts as
; the liveouts increase register pressure.

; CHECK: Skipping unroll of multi-exit loop with 8 liveouts to not increase register pressure!

; Here's the loop dump but it is not very interesting-

; + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
; |   %t2680.out = %t2680;
; |   %t2679.out = %t2679;
; |   %t2678.out1 = %t2678;
; |   %t2677.out = -1 * i1 + %n;
; |   %t2676.out = %t2676;
; |   %t2699 = %t2679;
; |   %t2700 = %t2680;
; |   if (%t2679 <u 16777216)
; |   {
; |      if (%t2678.out1 == %t4)
; |      {
; |         goto early.exit;
; |      }
; |      %t2678 = %t2678  +  1;
; |      %t2694 = (%t2)[%t2678.out1];
; |      %t2696 = 256 * %t2680  |  %t2694;
; |      %t2699 = 256 * %t2679;
; |      %t2700 = %t2696;
; |   }
; |   %t2678.out = %t2678;
; |   %t2701 = %t2699  >>  1;
; |   %t2703 = -1 * (%t2699 /u 2) + %t2700  >>  31;
; |   %t2704 = %t2703  &  (%t2699)/u2;
; |   %t2705 = %t2704  +  -1 * (%t2699 /u 2) + %t2700;
; |   %t2708 = 2 * %t2676.out + 1  +  %t2703;
; |   %t2676 = 2 * %t2676.out + %t2703 + 1;
; |   %t2679 = (%t2699)/u2;
; |   %t2680 = %t2704 + -1 * (%t2699 /u 2) + %t2700;
; + END LOOP

define void @foo(ptr %t2, i32 %t.in1, i32 %n, i64 %t.in3, i32 %t.in4, i32 %t.in5, i64 %t4) {
entry:
  br label %loop

loop:                                             ; preds = %latch, %entry
  %t2676 = phi i32 [ %t.in1, %entry ], [ %t2708, %latch ]
  %t2677 = phi i32 [ %n, %entry ], [ %t2709, %latch ]
  %t2678 = phi i64 [ %t.in3, %entry ], [ %t2698, %latch ]
  %t2679 = phi i32 [ %t.in4, %entry ], [ %t2701, %latch ]
  %t2680 = phi i32 [ %t.in5, %entry ], [ %t2705, %latch ]
  %t2681 = icmp ult i32 %t2679, 16777216
  br i1 %t2681, label %t2682, label %latch

t2682:                                             ; preds = %loop
  %t2683 = icmp eq i64 %t2678, %t4
  br i1 %t2683, label %early.exit, label %t2689

early.exit:                                             ; preds = %t2682
  %t2685 = phi i32 [ %t2676, %t2682 ]
  %t2686 = phi i32 [ %t2677, %t2682 ]
  %t2687 = phi i32 [ %t2679, %t2682 ]
  %t2688 = phi i32 [ %t2680, %t2682 ]
  br label %ret

t2689:                                             ; preds = %t2682
  %t2690 = shl nuw i32 %t2679, 8
  %t2691 = shl i32 %t2680, 8
  %t2692 = add i64 %t2678, 1
  %t2693 = getelementptr inbounds i8, ptr %t2, i64 %t2678
  %t2694 = load i8, ptr %t2693, align 1
  %t2695 = zext i8 %t2694 to i32
  %t2696 = or i32 %t2691, %t2695
  br label %latch

latch:                                             ; preds = %t2689, %loop
  %t2698 = phi i64 [ %t2692, %t2689 ], [ %t2678, %loop ]
  %t2699 = phi i32 [ %t2690, %t2689 ], [ %t2679, %loop ]
  %t2700 = phi i32 [ %t2696, %t2689 ], [ %t2680, %loop ]
  %t2701 = lshr i32 %t2699, 1
  %t2702 = sub i32 %t2700, %t2701
  %t2703 = ashr i32 %t2702, 31
  %t2704 = and i32 %t2703, %t2701
  %t2705 = add i32 %t2704, %t2702
  %t2706 = shl i32 %t2676, 1
  %t2707 = or i32 %t2706, 1
  %t2708 = add nsw i32 %t2707, %t2703
  %t2709 = add i32 %t2677, -1
  %t2710 = icmp eq i32 %t2709, 0
  br i1 %t2710, label %exit, label %loop

exit:                                             ; preds = %latch
  %t2712 = phi i64 [ %t2698, %latch ]
  %t2713 = phi i32 [ %t2701, %latch ]
  %t2714 = phi i32 [ %t2705, %latch ]
  %t2715 = phi i32 [ %t2708, %latch ]
  br label %ret

ret:
  ret void
}
