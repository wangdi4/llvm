; RUN opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that we are able to link bblocks in the correct lexical order.

; CHECK: DO i1 = 0, %n + -1, 1

; Verify that jump to label does not occur after it.

; CHECK:   poly_basis_dx.exit2828.thread:
; CHECK-NOT: goto poly_basis_dx.exit2828.thread

; CHECK: + END LOOP


define void @foo(i64 %t_sw, i64 %n) {
entry:
  br label %for.body1559

for.body1559:                                     ; preds = %poly_basis_dy.exit2836, %entry
  %storemerge2547284 = phi i64 [ 0, %entry ], [ %inc1612, %poly_basis_dy.exit2836 ]
  switch i64 %t_sw, label %poly_basis_dx.exit [
    i64 19, label %poly_basis_dx.exit.thread
    i64 1, label %poly_basis_dx.exit.thread207
    i64 18, label %sw.bb37.i
    i64 3, label %poly_basis_dx.exit.thread211
    i64 4, label %sw.bb4.i2755
    i64 17, label %sw.bb33.i
    i64 6, label %sw.bb6.i2757
    i64 7, label %sw.bb7.i2759
    i64 8, label %sw.bb9.i2761
    i64 16, label %sw.bb29.i
    i64 10, label %sw.bb12.i2764
    i64 11, label %sw.bb15.i2767
    i64 12, label %sw.bb18.i2768
    i64 13, label %sw.bb21.i
    i64 15, label %sw.bb25.i
  ]

poly_basis_dx.exit.thread:                        ; preds = %sw.bb25.i, %sw.bb21.i, %sw.bb18.i2768, %sw.bb15.i2767, %sw.bb12.i2764, %sw.bb29.i, %sw.bb9.i2761, %sw.bb7.i2759, %sw.bb6.i2757, %sw.bb33.i, %sw.bb37.i, %for.body1559
  br label %sw.default.i

poly_basis_dx.exit.thread207:                     ; preds = %sw.bb4.i2755, %for.body1559
  br label %poly_basis_dy.exit

poly_basis_dx.exit:                               ; preds = %for.body1559
  switch i64 %t_sw, label %sw.default.i [
    i64 0, label %poly_basis_dy.exit.thread308
    i64 5, label %sw.bb5.i2782
    i64 2, label %poly_basis_dy.exit.thread
  ]

sw.bb37.i:                                        ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

poly_basis_dx.exit.thread211:                     ; preds = %for.body1559
  br label %poly_basis_dx.exit2828.thread256

sw.bb4.i2755:                                     ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread207

sw.bb33.i:                                        ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb6.i2757:                                     ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb7.i2759:                                     ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb9.i2761:                                     ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb29.i:                                        ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb12.i2764:                                    ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb15.i2767:                                    ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb18.i2768:                                    ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb21.i:                                        ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.bb25.i:                                        ; preds = %for.body1559
  br label %poly_basis_dx.exit.thread

sw.default.i:                                     ; preds = %poly_basis_dx.exit, %poly_basis_dx.exit.thread
  switch i64 %t_sw, label %poly_basis_dy.exit [
    i64 20, label %poly_basis_dy.exit.thread308
    i64 16, label %poly_basis_dy.exit.thread392
    i64 19, label %poly_basis_dy.exit.thread317
    i64 14, label %sw.bb21.i2930
    i64 5, label %sw.bb4.i2912
    i64 18, label %poly_basis_dy.exit.thread329
    i64 7, label %poly_basis_dy.exit.thread368
    i64 8, label %poly_basis_dy.exit.thread380
    i64 9, label %sw.bb9.i2918
    i64 17, label %poly_basis_dy.exit.thread356
    i64 11, label %poly_basis_dy.exit.thread404
    i64 12, label %poly_basis_dy.exit.thread416
    i64 13, label %poly_basis_dy.exit.thread428
  ]

poly_basis_dy.exit.thread:                        ; preds = %sw.bb5.i2782, %poly_basis_dx.exit
  br label %poly_basis_dx.exit2828

poly_basis_dy.exit.thread308:                     ; preds = %sw.bb21.i2930, %sw.bb9.i2918, %sw.bb4.i2912, %sw.default.i, %poly_basis_dx.exit
  br label %poly_basis_dx.exit2828

poly_basis_dy.exit:                               ; preds = %sw.default.i, %poly_basis_dx.exit.thread207
  switch i64 %t_sw, label %poly_basis_dx.exit2828 [
    i64 19, label %poly_basis_dx.exit2828.thread
    i64 1, label %poly_basis_dx.exit2828.thread250
    i64 18, label %sw.bb37.i2822
    i64 3, label %poly_basis_dx.exit2828.thread256
    i64 4, label %sw.bb4.i2788
    i64 17, label %sw.bb33.i2818
    i64 6, label %sw.bb6.i2790
    i64 7, label %sw.bb7.i2792
    i64 8, label %sw.bb9.i2794
    i64 16, label %sw.bb29.i2814
    i64 10, label %sw.bb12.i2797
    i64 11, label %sw.bb15.i2800
    i64 12, label %sw.bb18.i2803
    i64 13, label %sw.bb21.i2806
    i64 15, label %sw.bb25.i2810
  ]

sw.bb5.i2782:                                     ; preds = %poly_basis_dx.exit
  br label %poly_basis_dy.exit.thread

poly_basis_dy.exit.thread317:                     ; preds = %sw.default.i
  br label %poly_basis_dx.exit2828.thread

sw.bb4.i2912:                                     ; preds = %sw.default.i
  br label %poly_basis_dy.exit.thread308

poly_basis_dy.exit.thread329:                     ; preds = %sw.default.i
  br label %sw.bb37.i2822

poly_basis_dy.exit.thread368:                     ; preds = %sw.default.i
  br label %sw.bb7.i2792

poly_basis_dy.exit.thread380:                     ; preds = %sw.default.i
  br label %sw.bb9.i2794

sw.bb9.i2918:                                     ; preds = %sw.default.i
  br label %poly_basis_dy.exit.thread308

poly_basis_dy.exit.thread356:                     ; preds = %sw.default.i
  br label %sw.bb33.i2818

poly_basis_dy.exit.thread404:                     ; preds = %sw.default.i
  br label %sw.bb15.i2800

poly_basis_dy.exit.thread416:                     ; preds = %sw.default.i
  br label %sw.bb18.i2803

poly_basis_dy.exit.thread428:                     ; preds = %sw.default.i
  br label %sw.bb21.i2806

sw.bb21.i2930:                                    ; preds = %sw.default.i
  br label %poly_basis_dy.exit.thread308

poly_basis_dy.exit.thread392:                     ; preds = %sw.default.i
  br label %sw.bb29.i2814

poly_basis_dx.exit2828.thread:                    ; preds = %sw.bb25.i2810, %sw.bb21.i2806, %sw.bb18.i2803, %sw.bb15.i2800, %sw.bb12.i2797, %sw.bb29.i2814, %sw.bb9.i2794, %sw.bb7.i2792, %sw.bb6.i2790, %sw.bb33.i2818, %sw.bb37.i2822, %poly_basis_dy.exit.thread317, %poly_basis_dy.exit
  br label %sw.default.i2834

poly_basis_dx.exit2828.thread250:                 ; preds = %sw.bb4.i2788, %poly_basis_dy.exit
  br label %poly_basis_dy.exit2836

poly_basis_dx.exit2828:                           ; preds = %poly_basis_dy.exit, %poly_basis_dy.exit.thread308, %poly_basis_dy.exit.thread
  switch i64 %t_sw, label %sw.default.i2834 [
    i64 0, label %poly_basis_dy.exit2836
    i64 1, label %poly_basis_dy.exit2836
    i64 2, label %sw.bb2.i2829
    i64 3, label %sw.bb3.i2830
    i64 4, label %poly_basis_dy.exit2836
    i64 5, label %sw.bb5.i2831
  ]

sw.bb37.i2822:                                    ; preds = %poly_basis_dy.exit.thread329, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

poly_basis_dx.exit2828.thread256:                 ; preds = %poly_basis_dy.exit, %poly_basis_dx.exit.thread211
  br label %sw.bb3.i2830

sw.bb4.i2788:                                     ; preds = %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread250

sw.bb33.i2818:                                    ; preds = %poly_basis_dy.exit.thread356, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb6.i2790:                                     ; preds = %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb7.i2792:                                     ; preds = %poly_basis_dy.exit.thread368, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb9.i2794:                                     ; preds = %poly_basis_dy.exit.thread380, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb29.i2814:                                    ; preds = %poly_basis_dy.exit.thread392, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb12.i2797:                                    ; preds = %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb15.i2800:                                    ; preds = %poly_basis_dy.exit.thread404, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb18.i2803:                                    ; preds = %poly_basis_dy.exit.thread416, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb21.i2806:                                    ; preds = %poly_basis_dy.exit.thread428, %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.bb25.i2810:                                    ; preds = %poly_basis_dy.exit
  br label %poly_basis_dx.exit2828.thread

sw.default.i2834:                                 ; preds = %poly_basis_dx.exit2828, %poly_basis_dx.exit2828.thread
  switch i64 %t_sw, label %poly_basis_dy.exit2836 [
    i64 20, label %sw.bb41.i2993
    i64 16, label %sw.bb25.i2977
    i64 19, label %sw.bb37.i2989
    i64 14, label %sw.bb21.i2973
    i64 13, label %sw.bb18.i2970
    i64 18, label %sw.bb33.i2985
    i64 7, label %sw.bb6.i2957
    i64 8, label %sw.bb7.i2959
    i64 9, label %sw.bb9.i2961
    i64 17, label %sw.bb29.i2981
    i64 11, label %sw.bb12.i2964
    i64 12, label %sw.bb15.i2967
  ]

sw.bb2.i2829:                                     ; preds = %poly_basis_dx.exit2828
  br label %poly_basis_dy.exit2836

sw.bb3.i2830:                                     ; preds = %poly_basis_dx.exit2828.thread256, %poly_basis_dx.exit2828
  br label %poly_basis_dy.exit2836

sw.bb5.i2831:                                     ; preds = %poly_basis_dx.exit2828
  br label %poly_basis_dy.exit2836

sw.bb41.i2993:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb37.i2989:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb33.i2985:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb6.i2957:                                     ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb7.i2959:                                     ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb9.i2961:                                     ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb29.i2981:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb12.i2964:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb15.i2967:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb18.i2970:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb21.i2973:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

sw.bb25.i2977:                                    ; preds = %sw.default.i2834
  br label %poly_basis_dy.exit2836

poly_basis_dy.exit2836:                           ; preds = %sw.bb25.i2977, %sw.bb21.i2973, %sw.bb18.i2970, %sw.bb15.i2967, %sw.bb12.i2964, %sw.bb29.i2981, %sw.bb9.i2961, %sw.bb7.i2959, %sw.bb6.i2957, %sw.bb33.i2985, %sw.bb37.i2989, %sw.bb41.i2993, %sw.bb5.i2831, %sw.bb3.i2830, %sw.bb2.i2829, %sw.default.i2834, %poly_basis_dx.exit2828, %poly_basis_dx.exit2828, %poly_basis_dx.exit2828, %poly_basis_dx.exit2828.thread250
  %inc1612 = add nuw nsw i64 %storemerge2547284, 1
  %exitcond = icmp eq i64 %inc1612, %n
  br i1 %exitcond, label %for.end1613.loopexit, label %for.body1559

for.end1613.loopexit:                             ; preds = %poly_basis_dy.exit2836
 ret void
}

