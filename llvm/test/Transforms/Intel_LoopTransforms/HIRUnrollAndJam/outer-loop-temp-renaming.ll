; REQUIRES: asserts

; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam"  -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam" -hir-unroll-and-jam-max-outer-loop-cost=2 -debug-only=hir-unroll-and-jam 2>&1 | FileCheck %s --check-prefix=CHECK-THRESHOLD

; HIR-
; + DO i1 = 0, -1 * %init1 + %n + -1, 1   <DO_LOOP>
; |   %tmp2 = 0.000000e+00;
; |
; |   + DO i2 = 0, %sub + -1, 1   <DO_LOOP>
; |   |   %tmp11 = (%in1)[i2]  *  (%in2)[i1 + %mul * i2 + %init1 + -1 * (%sub * %mul) + %add];
; |   |   %tmp2 = %tmp2  +  %tmp11;
; |   + END LOOP
; |
; |   %tmp29 = 0.000000e+00;
; |
; |   + DO i2 = 0, %init2 + smax((-1 + (-1 * %sub)), (-1 * %init2)), 1   <DO_LOOP>
; |   |   %tmp37 = (%in1)[-1 * i2 + %init2]  *  (%in2)[i1 + -1 * %mul * i2 + %init1 + %add + (((-1 * %sub) + %init2) * %mul)];
; |   |   %tmp29 = %tmp29  +  %tmp37;
; |   + END LOOP
; |
; |   %tmp43 = %tmp2  +  %tmp29;
; |   %tmp44 = %tmp43  +  0.000000e+00;
; |   (%out)[i1 + %init1 + %add] = %tmp44;
; + END LOOP


; Verify that we rename %tmp29.
; Only part of the output is checked as it is too big.


; CHECK: + DO i1 = 0, %tgu + -1

; CHECK: |   + DO i2 = 0, %sub + -1

; CHECK: |   + END LOOP

; CHECK: |   %temp9 = 0.000000e+00;
; CHECK: |   %temp10 = 0.000000e+00;
; CHECK: |   %temp11 = 0.000000e+00;
; CHECK: |   %temp12 = 0.000000e+00;
; CHECK: |   %temp13 = 0.000000e+00;
; CHECK: |   %temp14 = 0.000000e+00;
; CHECK: |   %temp15 = 0.000000e+00;
; CHECK: |   %tmp29 = 0.000000e+00;

; CHECK: |   + DO i2 = 0, %init2

; CHECK: |   + END LOOP

; CHECK: + END LOOP

; CHECK-THRESHOLD: Skipping unroll & jam of loop as the outer loop body cost exceeds threshold, cost = 9, threshold = 2


define void @foo(i64 %init1, i64 %init2, i64 %add, i64 %n, i64 %sub, i64 %mul, ptr noalias %in1, ptr noalias %in2, ptr noalias %out) {
bb:
  br label %bb9

bb9:                                              ; preds = %bb41, %bb
  %tmp = phi i64 [ %tmp46, %bb41 ], [ %init1, %bb ]
  %tmp1 = add nsw i64 %tmp, %add
  br label %bb11

bb11:                                             ; preds = %bb11, %bb9
  %tmp2 = phi float [ %tmp12, %bb11 ], [ 0.000000e+00, %bb9 ]
  %tmp3 = phi i64 [ %tmp13, %bb11 ], [ 0, %bb9 ]
  %tmp4 = getelementptr inbounds float, ptr %in1, i64 %tmp3
  %tmp5 = load float, ptr %tmp4, align 4
  %tmp6 = sub nsw i64 %tmp3, %sub
  %tmp7 = mul nsw i64 %tmp6, %mul
  %tmp8 = add nsw i64 %tmp7, %tmp1
  %tmp9 = getelementptr inbounds float, ptr %in2, i64 %tmp8
  %tmp10 = load float, ptr %tmp9, align 4
  %tmp11 = fmul float %tmp5, %tmp10
  %tmp12 = fadd float %tmp2, %tmp11
  %tmp13 = add nuw nsw i64 %tmp3, 1
  %tmp14 = icmp eq i64 %tmp13, %sub
  br i1 %tmp14, label %bb25, label %bb11

bb25:                                             ; preds = %bb11
  %tmp26 = phi float [ %tmp12, %bb11 ]
  br label %bb27

bb27:                                             ; preds = %bb27, %bb25
  %tmp28 = phi i64 [ %init2, %bb25 ], [ %tmp39, %bb27 ]
  %tmp29 = phi float [ 0.000000e+00, %bb25 ], [ %tmp38, %bb27 ]
  %tmp30 = getelementptr inbounds float, ptr %in1, i64 %tmp28
  %tmp31 = load float, ptr %tmp30, align 4
  %tmp32 = sub nsw i64 %tmp28, %sub
  %tmp33 = mul nsw i64 %tmp32, %mul
  %tmp34 = add nsw i64 %tmp33, %tmp1
  %tmp35 = getelementptr inbounds float, ptr %in2, i64 %tmp34
  %tmp36 = load float, ptr %tmp35, align 4
  %tmp37 = fmul float %tmp31, %tmp36
  %tmp38 = fadd float %tmp29, %tmp37
  %tmp39 = add nsw i64 %tmp28, -1
  %tmp40 = icmp sgt i64 %tmp39, %sub
  br i1 %tmp40, label %bb27, label %bb41

bb41:                                             ; preds = %bb27
  %tmp42 = phi float [ %tmp38, %bb27 ]
  %tmp43 = fadd float %tmp26, %tmp42
  %tmp44 = fadd float %tmp43, 0.000000e+00
  %tmp45 = getelementptr inbounds float, ptr %out, i64 %tmp1
  store float %tmp44, ptr %tmp45, align 4
  %tmp46 = add nsw i64 %tmp, 1
  %tmp47 = icmp eq i64 %tmp46, %n
  br i1 %tmp47, label %bb48, label %bb9

bb48:                                             ; preds = %bb41
  ret void
}
