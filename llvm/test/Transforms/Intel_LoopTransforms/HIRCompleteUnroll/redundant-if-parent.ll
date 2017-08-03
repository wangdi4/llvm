; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll  2>&1 < %s | FileCheck %s

; Verify that we correctly handle redundant if parent of the loop being unrolled.

; CHECK: Dump Before HIR PostVec Complete Unroll

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   if (undef true undef)
; CHECK: |   {
; CHECK: |      %arrayidx75.promoted = (@a1_bl)[0][i1];
; CHECK: |      %xor76450 = %arrayidx75.promoted;
; CHECK: |      %2 = %0;
; CHECK: |
; CHECK: |      + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |      |   %and65 = %2  &&  %in;
; CHECK: |      |   %3 = (@a2_p)[0][i1 + i2][i1];
; CHECK: |      |   %4 = (@a1_r)[0][-1 * i2 + 128];
; CHECK: |      |   (@a1_r)[0][-1 * i2 + 128] = %3 + %4;
; CHECK: |      |   %xor76450 = %xor76450  ^  %and65;
; CHECK: |      |   %5 = (@a3_wp)[0][-1 * i1 + 89][i1 + 20][i1 + i2];
; CHECK: |      |   (@a3_wp)[0][-1 * i1 + 89][i1 + 20][i1 + i2] = %5 + 1;
; CHECK: |      |   %2 = %and65;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      (@a1_bl)[0][i1] = %xor76450;
; CHECK: |   }
; CHECK: |   (@a1_bl)[0][i1 + 64] = 0;
; CHECK: |   %0 = 60;
; CHECK: + END LOOP

; CHECK: Dump After HIR PostVec Complete Unroll

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   if (undef true undef)
; CHECK: |   {
; CHECK: |     %arrayidx75.promoted = (@a1_bl)[0][i1];
; CHECK: |     %xor76450 = %arrayidx75.promoted;
; CHECK: |     %2 = %0;
; CHECK: |     %and65 = %2  &&  %in;
; CHECK: |     %3 = (@a2_p)[0][i1][i1];
; CHECK: |     %4 = (@a1_r)[0][128];
; CHECK: |     (@a1_r)[0][128] = %3 + %4;
; CHECK: |     %xor76450 = %xor76450  ^  %and65;
; CHECK: |     %5 = (@a3_wp)[0][-1 * i1 + 89][i1 + 20][i1];
; CHECK: |     (@a3_wp)[0][-1 * i1 + 89][i1 + 20][i1] = %5 + 1;
; CHECK: |     %2 = %and65;
; CHECK: |     (@a1_bl)[0][i1] = %xor76450;
; CHECK: |   }
; CHECK: |   (@a1_bl)[0][i1 + 64] = 0;
; CHECK: |   %0 = 60;
; CHECK: + END LOOP


@a1_bl = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_r = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a2_p = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16
@a3_wp = local_unnamed_addr global [192 x [192 x [192 x i32]]] zeroinitializer, align 16

define void @foo(i32 %xor55, i64 %in) {
entry:
  br label %for.body58

for.body58:                                       ; preds = %entry, %for.cond89.preheader
  %indvars.iv447 = phi i32 [ 0, %entry ], [ %indvars.iv.next448, %for.cond89.preheader ]
  %0 = phi i32 [ %xor55, %entry ], [ 60, %for.cond89.preheader ]
  %1 = zext i32 %indvars.iv447 to i64
  br i1 false, label %for.cond89.preheader, label %for.body63.lr.ph

for.body63.lr.ph:                                 ; preds = %for.body58
  %conv59 = zext i32 %indvars.iv447 to i64
  %add69 = add i32 %indvars.iv447, 128
  %conv70 = zext i32 %add69 to i64
  %arrayidx75 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_bl, i64 0, i64 %1
  %sub78 = sub i32 89, %indvars.iv447
  %idxprom79 = zext i32 %sub78 to i64
  %add81 = add i32 %indvars.iv447, 20
  %idxprom82 = zext i32 %add81 to i64
  %arrayidx75.promoted = load i32, i32* %arrayidx75, align 4
  br label %for.body63

for.body63:                                       ; preds = %for.body63.lr.ph, %for.body63
  %xor76450 = phi i32 [ %arrayidx75.promoted, %for.body63.lr.ph ], [ %xor76, %for.body63 ]
  %2 = phi i32 [ %0, %for.body63.lr.ph ], [ %conv66, %for.body63 ]
  %v_ekgo.0417 = phi i64 [ %conv59, %for.body63.lr.ph ], [ %inc87, %for.body63 ]
  %conv64 = zext i32 %2 to i64
  %and65 = and i64 %conv64, %in
  %conv66 = trunc i64 %and65 to i32
  %arrayidx68 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_p, i64 0, i64 %v_ekgo.0417, i64 %1
  %3 = load i32, i32* %arrayidx68, align 4
  %sub71 = sub i64 %conv70, %v_ekgo.0417
  %arrayidx72 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_r, i64 0, i64 %sub71
  %4 = load i32, i32* %arrayidx72, align 4
  %add73 = add i32 %4, %3
  store i32 %add73, i32* %arrayidx72, align 4
  %xor76 = xor i32 %xor76450, %conv66
  %arrayidx84 = getelementptr inbounds [192 x [192 x [192 x i32]]], [192 x [192 x [192 x i32]]]* @a3_wp, i64 0, i64 %idxprom79, i64 %idxprom82, i64 %v_ekgo.0417
  %5 = load i32, i32* %arrayidx84, align 4
  %inc85 = add i32 %5, 1
  store i32 %inc85, i32* %arrayidx84, align 4
  %inc87 = add nuw nsw i64 %v_ekgo.0417, 1
  %exitcond449 = icmp eq i64 %v_ekgo.0417, %1
  br i1 %exitcond449, label %for.cond89.preheader.loopexit, label %for.body63

for.cond89.preheader.loopexit:                    ; preds = %for.body63
  %xor76.lcssa = phi i32 [ %xor76, %for.body63 ]
  store i32 %xor76.lcssa, i32* %arrayidx75, align 4
  br label %for.cond89.preheader

for.cond89.preheader:                             ; preds = %for.cond89.preheader.loopexit, %for.body58
  %mul102 = shl i32 %indvars.iv447, 1
  %add103 = sub i32 64, %indvars.iv447
  %sub104 = add i32 %add103, %mul102
  %idxprom105 = zext i32 %sub104 to i64
  %arrayidx106 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_bl, i64 0, i64 %idxprom105
  store i32 0, i32* %arrayidx106, align 4
  %indvars.iv.next448 = add i32 %indvars.iv447, 1
  %cmp57 = icmp ult i32 %indvars.iv.next448, 64
  br i1 %cmp57, label %for.body58, label %for.exit

for.exit:
  ret void
}


