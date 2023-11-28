; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,hir-vec-dir-insert" -print-before=hir-loop-distribute-memrec -print-after=hir-vec-dir-insert -disable-output %s 2>&1 | FileCheck %s

; Verify we distribute the loop with high number of scalar-expanded temps due to
; subtantial computation in the vectorizable chunk.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %t2703 + -1, 1   <DO_LOOP>
; CHECK: |   %t2723 = (%t286)[2 * i1]  >>  14;
; CHECK: |   %t2726 = (%t236)[i1];
; CHECK: |   %t2729 = (%t285)[i1];
; CHECK: |   %t2733 = %t2729  -  (%t2704)[sext.i32.i64(%t38) + sext.i32.i64(%t2723)];
; CHECK: |   %t2738 = (%t180)[%t2726].2;
; CHECK: |   %t2744 = fpext.float.double((%t180)[%t2726].1);
; CHECK: |   %t2745 = %t2313  *  %t2744;
; CHECK: |   %t2748 = %t1138  -  (%t180)[%t2726].0.0;
; CHECK: |   %t2751 = %t1141  -  (%t180)[%t2726].0.1;
; CHECK: |   %t2754 = %t1144  -  (%t180)[%t2726].0.2;
; CHECK: |   %t2757 = (%t2322)[2 * sext.i16.i32(%t2738)].0  *  %t34;
; CHECK: |   %t2760 = (%t2322)[2 * sext.i16.i32(%t2738)].1  *  %t34;
; CHECK: |   %t2761 = %t2729  -  %t36;
; CHECK: |   %t2707 = (%t3)[0]  +  %t2707;
; CHECK: |   %t2709 = (%t5)[0]  +  %t2709;
; CHECK: |   %t2770 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 9]  *  %t2745;
; CHECK: |   %t2773 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 10]  *  %t2745;
; CHECK: |   %t2776 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 11]  *  %t2745;
; CHECK: |   %t2777 = %t2745  *  %t2733;
; CHECK: |   %t2778 = %t2777  *  (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 8];
; CHECK: |   %t2779 = %t2778  *  0x3FC5555555555555;
; CHECK: |   %t2780 = %t2770  *  2.500000e-01;
; CHECK: |   %t2781 = %t2779  +  %t2780;
; CHECK: |   %t2782 = %t2781  *  %t2733;
; CHECK: |   %t2783 = %t2773  *  5.000000e-01;
; CHECK: |   %t2784 = %t2782  +  %t2783;
; CHECK: |   %t2785 = %t2784  *  %t2733;
; CHECK: |   %t2786 = %t2785  +  %t2776;
; CHECK: |   %t2787 = %t2786  *  %t56;
; CHECK: |   %t2708 = %t2708  -  %t2787;
; CHECK: |   %t2789 = %t2786  *  %t67;
; CHECK: |   %t2710 = %t2710  -  %t2789;
; CHECK: |   %t2791 = %t2778  +  %t2770;
; CHECK: |   %t2792 = %t2791  *  %t2733;
; CHECK: |   %t2793 = %t2792  +  %t2773;
; CHECK: |   %t2794 = %t2793  *  %t56;
; CHECK: |   %t2796 = %t2794  +  (%t4)[0];
; CHECK: |   %t2797 = %t2796  *  %t2748;
; CHECK: |   %t2798 = %t2797  *  %t2748;
; CHECK: |   %t2711 = %t2798  +  %t2711;
; CHECK: |   %t2800 = %t2797  *  %t2751;
; CHECK: |   %t2712 = %t2800  +  %t2712;
; CHECK: |   %t2802 = %t2797  *  %t2754;
; CHECK: |   %t2713 = %t2802  +  %t2713;
; CHECK: |   %t2717 = %t2797  +  %t2717;
; CHECK: |   %t2807 = (%t1072)[%t2726].0  -  %t2797;
; CHECK: |   (%t1072)[%t2726].0 = %t2807;
; CHECK: |   %t2808 = %t2796  *  %t2751;
; CHECK: |   %t2809 = %t2808  *  %t2751;
; CHECK: |   %t2714 = %t2809  +  %t2714;
; CHECK: |   %t2811 = %t2808  *  %t2754;
; CHECK: |   %t2715 = %t2811  +  %t2715;
; CHECK: |   %t2718 = %t2808  +  %t2718;
; CHECK: |   %t2816 = (%t1072)[%t2726].1  -  %t2808;
; CHECK: |   (%t1072)[%t2726].1 = %t2816;
; CHECK: |   %t2817 = %t2796  *  %t2754;
; CHECK: |   %t2818 = %t2817  *  %t2754;
; CHECK: |   %t2716 = %t2818  +  %t2716;
; CHECK: |   %t2719 = %t2817  +  %t2719;
; CHECK: |   %t2823 = (%t1072)[%t2726].2  -  %t2817;
; CHECK: |   (%t1072)[%t2726].2 = %t2823;
; CHECK: + END LOOP

; CHECK: Dump After
; CHECK: modified

; CHECK: + DO i1 = 0, (%t2703 + -1)/u64, 1   <DO_LOOP>
; CHECK: |   %min = (-64 * i1 + %t2703 + -1 <= 63) ? -64 * i1 + %t2703 + -1 : 63;
; CHECK: |
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %t2723 = (%t286)[128 * i1 + 2 * i2]  >>  14;
; CHECK: |   |   %t2726 = (%t236)[64 * i1 + i2];
; CHECK: |   |   %t2729 = (%t285)[64 * i1 + i2];
; CHECK: |   |   %t2733 = %t2729  -  (%t2704)[sext.i32.i64(%t38) + sext.i32.i64(%t2723)];
; CHECK: |   |   (%.TempArray)[0][i2] = %t2733;
; CHECK: |   |   %t2738 = (%t180)[%t2726].2;
; CHECK: |   |   %t2744 = fpext.float.double((%t180)[%t2726].1);
; CHECK: |   |   %t2745 = %t2313  *  %t2744;
; CHECK: |   |   %t2748 = %t1138  -  (%t180)[%t2726].0.0;
; CHECK: |   |   (%.TempArray3)[0][i2] = %t2748;
; CHECK: |   |   %t2751 = %t1141  -  (%t180)[%t2726].0.1;
; CHECK: |   |   (%.TempArray5)[0][i2] = %t2751;
; CHECK: |   |   %t2754 = %t1144  -  (%t180)[%t2726].0.2;
; CHECK: |   |   (%.TempArray7)[0][i2] = %t2754;
; CHECK: |   |   %t2757 = (%t2322)[2 * sext.i16.i32(%t2738)].0  *  %t34;
; CHECK: |   |   %t2760 = (%t2322)[2 * sext.i16.i32(%t2738)].1  *  %t34;
; CHECK: |   |   %t2707 = (%t3)[0]  +  %t2707;
; CHECK: |   |   %t2709 = (%t5)[0]  +  %t2709;
; CHECK: |   |   %t2770 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 9]  *  %t2745;
; CHECK: |   |   (%.TempArray9)[0][i2] = %t2770;
; CHECK: |   |   %t2773 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 10]  *  %t2745;
; CHECK: |   |   (%.TempArray11)[0][i2] = %t2773;
; CHECK: |   |   %t2776 = (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 11]  *  %t2745;
; CHECK: |   |   (%.TempArray13)[0][i2] = %t2776;
; CHECK: |   |   %t2777 = %t2745  *  %t2733;
; CHECK: |   |   %t2778 = %t2777  *  (%t33)[sext.i32.i64((16 * (%t38 + %t2723))) + 8];
; CHECK: |   |   (%.TempArray15)[0][i2] = %t2778;
; CHECK: |   |   %t2791 = %t2778  +  %t2770;
; CHECK: |   |   %t2792 = %t2791  *  %t2733;
; CHECK: |   |   %t2793 = %t2792  +  %t2773;
; CHECK: |   |   %t2794 = %t2793  *  %t56;
; CHECK: |   |   %t2796 = %t2794  +  (%t4)[0];
; CHECK: |   |   %t2797 = %t2796  *  %t2748;
; CHECK: |   |   (%.TempArray17)[0][i2] = %t2797;
; CHECK: |   |   %t2807 = (%t1072)[%t2726].0  -  %t2797;
; CHECK: |   |   (%t1072)[%t2726].0 = %t2807;
; CHECK: |   |   %t2808 = %t2796  *  %t2751;
; CHECK: |   |   (%.TempArray19)[0][i2] = %t2808;
; CHECK: |   |   %t2816 = (%t1072)[%t2726].1  -  %t2808;
; CHECK: |   |   (%t1072)[%t2726].1 = %t2816;
; CHECK: |   |   %t2817 = %t2796  *  %t2754;
; CHECK: |   |   (%.TempArray21)[0][i2] = %t2817;
; CHECK: |   |   %t2823 = (%t1072)[%t2726].2  -  %t2817;
; CHECK: |   |   (%t1072)[%t2726].2 = %t2823;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: | %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK: |
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %t2729 = (%t285)[64 * i1 + i2];
; CHECK: |   |   %t2733 = (%.TempArray)[0][i2];
; CHECK: |   |   %t2748 = (%.TempArray3)[0][i2];
; CHECK: |   |   %t2751 = (%.TempArray5)[0][i2];
; CHECK: |   |   %t2754 = (%.TempArray7)[0][i2];
; CHECK: |   |   %t2770 = (%.TempArray9)[0][i2];
; CHECK: |   |   %t2773 = (%.TempArray11)[0][i2];
; CHECK: |   |   %t2776 = (%.TempArray13)[0][i2];
; CHECK: |   |   %t2778 = (%.TempArray15)[0][i2];
; CHECK: |   |   %t2797 = (%.TempArray17)[0][i2];
; CHECK: |   |   %t2808 = (%.TempArray19)[0][i2];
; CHECK: |   |   %t2817 = (%.TempArray21)[0][i2];
; CHECK: |   |   %t2761 = %t2729  -  %t36;
; CHECK: |   |   %t2779 = %t2778  *  0x3FC5555555555555;
; CHECK: |   |   %t2780 = %t2770  *  2.500000e-01;
; CHECK: |   |   %t2781 = %t2779  +  %t2780;
; CHECK: |   |   %t2782 = %t2781  *  %t2733;
; CHECK: |   |   %t2783 = %t2773  *  5.000000e-01;
; CHECK: |   |   %t2784 = %t2782  +  %t2783;
; CHECK: |   |   %t2785 = %t2784  *  %t2733;
; CHECK: |   |   %t2786 = %t2785  +  %t2776;
; CHECK: |   |   %t2787 = %t2786  *  %t56;
; CHECK: |   |   %t2708 = %t2708  -  %t2787;
; CHECK: |   |   %t2789 = %t2786  *  %t67;
; CHECK: |   |   %t2710 = %t2710  -  %t2789;
; CHECK: |   |   %t2798 = %t2797  *  %t2748;
; CHECK: |   |   %t2711 = %t2798  +  %t2711;
; CHECK: |   |   %t2800 = %t2797  *  %t2751;
; CHECK: |   |   %t2712 = %t2800  +  %t2712;
; CHECK: |   |   %t2802 = %t2797  *  %t2754;
; CHECK: |   |   %t2713 = %t2802  +  %t2713;
; CHECK: |   |   %t2717 = %t2797  +  %t2717;
; CHECK: |   |   %t2809 = %t2808  *  %t2751;
; CHECK: |   |   %t2714 = %t2809  +  %t2714;
; CHECK: |   |   %t2811 = %t2808  *  %t2754;
; CHECK: |   |   %t2715 = %t2811  +  %t2715;
; CHECK: |   |   %t2718 = %t2808  +  %t2718;
; CHECK: |   |   %t2818 = %t2817  *  %t2754;
; CHECK: |   |   %t2716 = %t2818  +  %t2716;
; CHECK: |   |   %t2719 = %t2817  +  %t2719;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK: + END LOOP

%class._ZTS6Vector.Vector = type { double, double, double }
%struct._ZTS8CompAtom.CompAtom = type { %class._ZTS6Vector.Vector, float, i16, i8, i8 }
%"struct._ZTSN7LJTable10TableEntryE.LJTable::TableEntry" = type { double, double }

define void @foo(double %t2699, ptr %t286, ptr %t236, ptr %t285, ptr %t2704, ptr %t1072, double %t1138, double %t1141, double %t1144, ptr %t180, double %t2313, ptr %t2322, i64 %t2703, ptr %t3, ptr %t33, double %t34, double %t36, i32 %t38, ptr %t4, ptr %t5, double %t56, double %t67) {
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %t2706 = phi i64 [ 0, %entry ], [ %t2824, %loop ]
  %t2707 = phi double [ %t2699, %entry ], [ %t2763, %loop ]
  %t2708 = phi double [ %t2699, %entry ], [ %t2788, %loop ]
  %t2709 = phi double [ %t2699, %entry ], [ %t2765, %loop ]
  %t2710 = phi double [ %t2699, %entry ], [ %t2790, %loop ]
  %t2711 = phi double [ %t2699, %entry ], [ %t2799, %loop ]
  %t2712 = phi double [ %t2699, %entry ], [ %t2801, %loop ]
  %t2713 = phi double [ %t2699, %entry ], [ %t2803, %loop ]
  %t2714 = phi double [ %t2699, %entry ], [ %t2810, %loop ]
  %t2715 = phi double [ %t2699, %entry ], [ %t2812, %loop ]
  %t2716 = phi double [ %t2699, %entry ], [ %t2819, %loop ]
  %t2717 = phi double [ %t2699, %entry ], [ %t2804, %loop ]
  %t2718 = phi double [ %t2699, %entry ], [ %t2813, %loop ]
  %t2719 = phi double [ %t2699, %entry ], [ %t2820, %loop ]
  %t2720 = shl nuw nsw i64 %t2706, 1
  %t2721 = getelementptr inbounds i32, ptr %t286, i64 %t2720
  %t2722 = load i32, ptr %t2721, align 4
  %t2723 = ashr i32 %t2722, 14
  %t2724 = add nsw i32 %t2723, %t38
  %t2725 = getelementptr inbounds i16, ptr %t236, i64 %t2706
  %t2726 = load i16, ptr %t2725, align 2
  %t2727 = zext i16 %t2726 to i64
  %t2728 = getelementptr inbounds double, ptr %t285, i64 %t2706
  %t2729 = load double, ptr %t2728, align 8
  %t2730 = sext i32 %t2724 to i64
  %t2731 = getelementptr inbounds double, ptr %t2704, i64 %t2730
  %t2732 = load double, ptr %t2731, align 8
  %t2733 = fsub fast double %t2729, %t2732
  %t2734 = shl nsw i32 %t2724, 4
  %t2735 = sext i32 %t2734 to i64
  %t2736 = getelementptr inbounds double, ptr %t33, i64 %t2735
  %t2737 = getelementptr inbounds %struct._ZTS8CompAtom.CompAtom, ptr %t180, i64 %t2727, i32 2
  %t2738 = load i16, ptr %t2737, align 2
  %t2739 = sext i16 %t2738 to i32
  %t2740 = shl nsw i32 %t2739, 1
  %t2741 = sext i32 %t2740 to i64
  %t2742 = getelementptr inbounds %struct._ZTS8CompAtom.CompAtom, ptr %t180, i64 %t2727, i32 1
  %t2743 = load float, ptr %t2742, align 4
  %t2744 = fpext float %t2743 to double
  %t2745 = fmul fast double %t2313, %t2744
  %t2746 = getelementptr inbounds %struct._ZTS8CompAtom.CompAtom, ptr %t180, i64 %t2727, i32 0, i32 0
  %t2747 = load double, ptr %t2746, align 8
  %t2748 = fsub fast double %t1138, %t2747
  %t2749 = getelementptr inbounds %struct._ZTS8CompAtom.CompAtom, ptr %t180, i64 %t2727, i32 0, i32 1
  %t2750 = load double, ptr %t2749, align 8
  %t2751 = fsub fast double %t1141, %t2750
  %t2752 = getelementptr inbounds %struct._ZTS8CompAtom.CompAtom, ptr %t180, i64 %t2727, i32 0, i32 2
  %t2753 = load double, ptr %t2752, align 8
  %t2754 = fsub fast double %t1144, %t2753
  %t2755 = getelementptr inbounds %"struct._ZTSN7LJTable10TableEntryE.LJTable::TableEntry", ptr %t2322, i64 %t2741, i32 0
  %t2756 = load double, ptr %t2755, align 8
  %t2757 = fmul fast double %t2756, %t34
  %t2758 = getelementptr inbounds %"struct._ZTSN7LJTable10TableEntryE.LJTable::TableEntry", ptr %t2322, i64 %t2741, i32 1
  %t2759 = load double, ptr %t2758, align 8
  %t2760 = fmul fast double %t2759, %t34
  %t2761 = fsub fast double %t2729, %t36
  %t2762 = load double, ptr %t3, align 8
  %t2763 = fadd fast double %t2762, %t2707
  %t2764 = load double, ptr %t5, align 8
  %t2765 = fadd fast double %t2764, %t2709
  %t2766 = getelementptr inbounds double, ptr %t2736, i64 8
  %t2767 = load double, ptr %t2766, align 8
  %t2768 = getelementptr inbounds double, ptr %t2736, i64 9
  %t2769 = load double, ptr %t2768, align 8
  %t2770 = fmul fast double %t2769, %t2745
  %t2771 = getelementptr inbounds double, ptr %t2736, i64 10
  %t2772 = load double, ptr %t2771, align 8
  %t2773 = fmul fast double %t2772, %t2745
  %t2774 = getelementptr inbounds double, ptr %t2736, i64 11
  %t2775 = load double, ptr %t2774, align 8
  %t2776 = fmul fast double %t2775, %t2745
  %t2777 = fmul fast double %t2745, %t2733
  %t2778 = fmul fast double %t2777, %t2767
  %t2779 = fmul fast double %t2778, 0x3FC5555555555555
  %t2780 = fmul fast double %t2770, 2.500000e-01
  %t2781 = fadd fast double %t2779, %t2780
  %t2782 = fmul fast double %t2781, %t2733
  %t2783 = fmul fast double %t2773, 5.000000e-01
  %t2784 = fadd fast double %t2782, %t2783
  %t2785 = fmul fast double %t2784, %t2733
  %t2786 = fadd fast double %t2785, %t2776
  %t2787 = fmul fast double %t2786, %t56
  %t2788 = fsub fast double %t2708, %t2787
  %t2789 = fmul fast double %t2786, %t67
  %t2790 = fsub fast double %t2710, %t2789
  %t2791 = fadd fast double %t2778, %t2770
  %t2792 = fmul fast double %t2791, %t2733
  %t2793 = fadd fast double %t2792, %t2773
  %t2794 = fmul fast double %t2793, %t56
  %t2795 = load double, ptr %t4, align 8
  %t2796 = fadd fast double %t2794, %t2795
  %t2797 = fmul fast double %t2796, %t2748
  %t2798 = fmul fast double %t2797, %t2748
  %t2799 = fadd fast double %t2798, %t2711
  %t2800 = fmul fast double %t2797, %t2751
  %t2801 = fadd fast double %t2800, %t2712
  %t2802 = fmul fast double %t2797, %t2754
  %t2803 = fadd fast double %t2802, %t2713
  %t2804 = fadd fast double %t2797, %t2717
  %t2805 = getelementptr inbounds %class._ZTS6Vector.Vector, ptr %t1072, i64 %t2727, i32 0
  %t2806 = load double, ptr %t2805, align 8
  %t2807 = fsub fast double %t2806, %t2797
  store double %t2807, ptr %t2805, align 8
  %t2808 = fmul fast double %t2796, %t2751
  %t2809 = fmul fast double %t2808, %t2751
  %t2810 = fadd fast double %t2809, %t2714
  %t2811 = fmul fast double %t2808, %t2754
  %t2812 = fadd fast double %t2811, %t2715
  %t2813 = fadd fast double %t2808, %t2718
  %t2814 = getelementptr inbounds %class._ZTS6Vector.Vector, ptr %t1072, i64 %t2727, i32 1
  %t2815 = load double, ptr %t2814, align 8
  %t2816 = fsub fast double %t2815, %t2808
  store double %t2816, ptr %t2814, align 8
  %t2817 = fmul fast double %t2796, %t2754
  %t2818 = fmul fast double %t2817, %t2754
  %t2819 = fadd fast double %t2818, %t2716
  %t2820 = fadd fast double %t2817, %t2719
  %t2821 = getelementptr inbounds %class._ZTS6Vector.Vector, ptr %t1072, i64 %t2727, i32 2
  %t2822 = load double, ptr %t2821, align 8
  %t2823 = fsub fast double %t2822, %t2817
  store double %t2823, ptr %t2821, align 8
  %t2824 = add nuw nsw i64 %t2706, 1
  %t2825 = icmp eq i64 %t2824, %t2703
  br i1 %t2825, label %exit, label %loop

exit:                                             ; preds = %loop
  %t2827 = phi double [ %t2763, %loop ]
  %t2828 = phi double [ %t2765, %loop ]
  %t2829 = phi double [ %t2788, %loop ]
  %t2830 = phi double [ %t2790, %loop ]
  %t2831 = phi double [ %t2799, %loop ]
  %t2832 = phi double [ %t2801, %loop ]
  %t2833 = phi double [ %t2803, %loop ]
  %t2834 = phi double [ %t2804, %loop ]
  %t2835 = phi double [ %t2810, %loop ]
  %t2836 = phi double [ %t2812, %loop ]
  %t2837 = phi double [ %t2813, %loop ]
  %t2838 = phi double [ %t2819, %loop ]
  %t2839 = phi double [ %t2820, %loop ]
  ret void
}

