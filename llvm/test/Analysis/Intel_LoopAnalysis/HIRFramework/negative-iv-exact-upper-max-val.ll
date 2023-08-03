; RUN: opt < %s -passes="hir-ssa-deconstruction,print,print<hir>" 2>&1 | FileCheck %s

; Verify that the MAX_TC_EST of i2 loop is set to 102 instead of incorrectly
; refining it as 1. 1 was being derived by analyzing the subscript [-1 * i1 + i2 + 102].

; The trip count estimator has trouble handling cases where replacing the IV
; term in the subscript with the upper cancels out another term in the subscript.

; CHECK: + DO i1 = 0, 101, 1   <DO_LOOP>
; CHECK: |   %i132 = (%i)[0][-1 * i1 + 101];
; CHECK: |   %i135 = %i132;
; CHECK: |
; CHECK: |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 102>  <LEGAL_MAX_TC = 102>
; CHECK: |   |   %i137 = (@a)[0][-1 * i1 + 101][-1 * i1 + i2 + 102];
; CHECK: |   |   %i139 = (@x)[0][-1 * i1 + i2 + 102];
; CHECK: |   |   %i140 = %i139  *  %i137;
; CHECK: |   |   %i135 = %i135  -  %i140;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %i147 = (@a)[0][-1 * i1 + 101][-1 * i1 + 101];
; CHECK: |   %i148 = %i135  /  %i147;
; CHECK: |   (@x)[0][-1 * i1 + 101] = %i148;
; CHECK: + END LOOP


@a = external hidden unnamed_addr global [103 x [103 x float]], align 16
@x = external hidden unnamed_addr global [103 x float], align 16

define void @foo(ptr %i) {
entry:
  br label %loop.outer

loop.outer:
  %i129 = phi i64 [ 102, %entry ], [ %i130, %outer.latch ]
  %i130 = add nsw i64 %i129, -1
  %i131 = getelementptr inbounds [103 x float], ptr %i, i64 0, i64 %i130
  %i132 = load float, ptr %i131, align 4
  br label %loop.inner

loop.inner:                                            ; preds = %loop.inner, %loop.outer
  %i134 = phi i64 [ %i142, %loop.inner ], [ %i129, %loop.outer ]
  %i135 = phi float [ %i141, %loop.inner ], [ %i132, %loop.outer ]
  %i136 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %i130, i64 %i134
  %i137 = load float, ptr %i136, align 4
  %i138 = getelementptr inbounds [103 x float], ptr @x, i64 0, i64 %i134
  %i139 = load float, ptr %i138, align 4
  %i140 = fmul fast float %i139, %i137
  %i141 = fsub fast float %i135, %i140
  %i142 = add nuw nsw i64 %i134, 1
  %i143 = icmp eq i64 %i142, 103
  br i1 %i143, label %outer.latch, label %loop.inner

outer.latch:                                            ; preds = %loop.inner
  %i145 = phi float [ %i141, %loop.inner ]
  %i146 = getelementptr inbounds [103 x [103 x float]], ptr @a, i64 0, i64 %i130, i64 %i130
  %i147 = load float, ptr %i146, align 4
  %i148 = fdiv fast float %i145, %i147
  %i149 = getelementptr inbounds [103 x float], ptr @x, i64 0, i64 %i130
  store float %i148, ptr %i149, align 4
  %i150 = icmp ugt i64 %i129, 1
  br i1 %i150, label %loop.outer, label %bb151

bb151:
  ret void
}


