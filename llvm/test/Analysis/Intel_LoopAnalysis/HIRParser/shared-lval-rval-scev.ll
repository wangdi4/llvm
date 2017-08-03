; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the parser is able to distinguish between lval %conv6 and rval %dec.sink.lcssa which share the same SCEV.

; CHECK: + DO i1 = 0, 45, 1   <DO_LOOP>
; CHECK: |   %dec.sink.lcssa = -1 * i1 + 46;
; CHECK: |
; CHECK: |   + DO i2 = 0, zext.i8.i64((-25 + trunc.i64.i8(%indvars.iv118))), 1   <DO_LOOP>  <MAX_TC_EST = 232>
; CHECK: |   |   %3 = (@a1_ahj)[0][-1 * i1 + -1 * i2 + 46];
; CHECK: |   |   (@a1_ahj)[0][-1 * i1 + -1 * i2 + 46] = %3 + 1;
; CHECK: |   |   (@a1_ob)[0][-1 * i1 + -1 * i2 + 46] = 48;
; CHECK: |   + END LOOP
; CHECK: |      %dec.sink.lcssa = 24;
; CHECK: |
; CHECK: |   %conv6 = %dec.sink.lcssa  &&  255;
; CHECK: |   (@a1_ob)[0][-1 * i1 + 46] = %conv6;
; CHECK: |   %indvars.iv118 = -1 * i1 + 45;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_jnv = local_unnamed_addr global i8 30, align 1
@a1_ob = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_ahj = local_unnamed_addr global [192 x i64] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define void @foo() local_unnamed_addr {
  %1 = load i8, i8* @g_jnv, align 1
  %conv = zext i8 %1 to i32
  %add = add nuw nsw i32 %conv, 100
  %add2 = add nuw nsw i32 %conv, 32
  %mul = mul nuw nsw i32 %add2, %add
  %sub = add nsw i32 %mul, -77
  br label %2

; <label>:2:                                      ; preds = %0, %._crit_edge
  %indvars.iv118 = phi i64 [ 46, %0 ], [ %indvars.iv.next119, %._crit_edge ]
  %v_i.0101 = phi i32 [ 46, %0 ], [ %dec18, %._crit_edge ]
  %conv5 = trunc i32 %v_i.0101 to i8
  %cmp799 = icmp ugt i8 %conv5, 24
  br i1 %cmp799, label %.lr.ph.preheader, label %._crit_edge

.lr.ph.preheader:                                 ; preds = %2
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.preheader, %.lr.ph
  %indvars.iv116 = phi i64 [ %indvars.iv.next117, %.lr.ph ], [ %indvars.iv118, %.lr.ph.preheader ]
  %arrayidx = getelementptr inbounds [192 x i64], [192 x i64]* @a1_ahj, i64 0, i64 %indvars.iv116
  %3 = load i64, i64* %arrayidx, align 8
  %inc = add i64 %3, 1
  store i64 %inc, i64* %arrayidx, align 8
  %arrayidx12 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_ob, i64 0, i64 %indvars.iv116
  store i32 48, i32* %arrayidx12, align 4
  %4 = trunc i64 %indvars.iv116 to i8
  %dec = add i8 %4, -1
  %cmp7 = icmp ugt i8 %dec, 24
  %indvars.iv.next117 = add nsw i64 %indvars.iv116, -1
  br i1 %cmp7, label %.lr.ph, label %._crit_edge.loopexit

._crit_edge.loopexit:                             ; preds = %.lr.ph
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %2
  %dec.sink.lcssa = phi i32 [ %v_i.0101, %2 ], [ 24, %._crit_edge.loopexit ]
  %conv6 = and i32 %dec.sink.lcssa, 255
  %arrayidx16 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_ob, i64 0, i64 %indvars.iv118
  store i32 %conv6, i32* %arrayidx16, align 4
  %indvars.iv.next119 = add nsw i64 %indvars.iv118, -1
  %dec18 = add nsw i32 %v_i.0101, -1
  %cmp = icmp eq i64 %indvars.iv.next119, 0
  br i1 %cmp, label %5, label %2

; <label>:5:                                      ; preds = %._crit_edge
  ret void
}

