; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s


; Verify that parser is able to reverse engineer UB of i3 loop successfully by sign extending an existing value.


; CHECK: + DO i1 = 0, 34, 1   <DO_LOOP>
; CHECK: |   %hir.de.ssa.copy0.out = %1;
; CHECK: |   %2 = (@a1_sur)[0][i1 + 1];
; CHECK: |   (@a1_sur)[0][i1 + 1] = 64 * %2;
; CHECK: |   %v_pl.0157 = 0;
; CHECK: |
; CHECK: |   + DO i2 = 0, 17, 1   <DO_LOOP>
; CHECK: |   |   %4 = (@g_t)[0];
; CHECK: |   |   if (%4 != 0)
; CHECK: |   |   {
; CHECK: |   |      %5 = (@g_b)[0];
; CHECK: |   |      @_Z6printbj(%5);
; CHECK: |   |      %v_qf.0160 = i1 + 1;
; CHECK: |   |   }
; CHECK: |   |   %.pre-phi = i1 + 1;
; CHECK: |   |   if (2 * i2 <= i1 + 1)
; CHECK: |   |   {
; CHECK: |   |      %g_t.promoted = (@g_t)[0];
; CHECK: |   |      %mul27156 = %g_t.promoted;
; CHECK: |   |
; CHECK: |   |      + DO i3 = 0, trunc.i64.i32((((-1 * sext.i32.i64(%v_pl.0157)) + %indvars.iv167) /u 6)), 1   <DO_LOOP>
; CHECK: |   |      |   %mul27156 = %mul27156  *  i1 + 1;
; CHECK: |   |      + END LOOP
; CHECK: |   |
; CHECK: |   |      (@a1_sur)[0][i1 + 1] = 21;
; CHECK: |   |      (@g_t)[0] = %mul27156;
; CHECK: |   |      %.pre-phi = i1 + 1;
; CHECK: |   |   }
; CHECK: |   |   @_Z6printbj(%.pre-phi);
; CHECK: |   |   %v_pl.0157 = 2 * i2 + 2;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @_Z4swapRjS_(&((@a1_sur)[0][2 * i1 + 2]),  &((@g_b)[0]));
; CHECK: |   %9 = (@g_b)[0];
; CHECK: |   %10 = (@a1_sur)[0][i1 + 1];
; CHECK: |   %or47 = %10  ||  %9;
; CHECK: |   (@a1_sur)[0][i1 + 1] = %or47;
; CHECK: |   %1 = %9;
; CHECK: |   %indvars.iv167 = i1 + 2;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 't.cpp'
source_filename = "t.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_b = global i32 8, align 4
@g_t = local_unnamed_addr global i32 9, align 4
@a1_sur = global [192 x i32] zeroinitializer, align 16
@a1_cb = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a2_nf = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @g_b, align 4
  %or = or i32 %0, 71
  %add4 = add i32 %or, 113
  %add4.tr = trunc i32 %add4 to i16
  %conv6 = and i16 %add4.tr, 64
  br label %cond.end

cond.end:                                         ; preds = %for.end40, %entry
  %1 = phi i32 [ %0, %entry ], [ %9, %for.end40 ]
  %indvars.iv167 = phi i64 [ 1, %entry ], [ %indvars.iv.next168, %for.end40 ]
  %v_qf.0160 = phi i16 [ %conv6, %entry ], [ %v_qf.2.lcssa, %for.end40 ]
  %v_n.0159 = phi i32 [ 1, %entry ], [ %inc, %for.end40 ]
  %arrayidx = getelementptr inbounds [192 x i32], [192 x i32]* @a1_sur, i64 0, i64 %indvars.iv167
  %2 = load i32, i32* %arrayidx, align 4
  %mul = shl i32 %2, 6
  store i32 %mul, i32* %arrayidx, align 4
  %conv13 = trunc i32 %v_n.0159 to i16
  %.pre = trunc i64 %indvars.iv167 to i32
  %3 = trunc i64 %indvars.iv167 to i32
  br label %for.body11

for.body11:                                       ; preds = %cond.end, %for.end
  %v_qf.1158 = phi i16 [ %v_qf.0160, %cond.end ], [ %v_qf.2, %for.end ]
  %v_pl.0157 = phi i32 [ 0, %cond.end ], [ %add39, %for.end ]
  %4 = load i32, i32* @g_t, align 4
  %tobool12 = icmp eq i32 %4, 0
  br i1 %tobool12, label %if.end, label %if.then

if.then:                                          ; preds = %for.body11
  %5 = load i32, i32* @g_b, align 4
  tail call void @_Z6printbj(i32 %5)
  br label %if.end

if.end:                                           ; preds = %for.body11, %if.then
  %v_qf.2 = phi i16 [ %conv13, %if.then ], [ %v_qf.1158, %for.body11 ]
  %6 = zext i32 %v_pl.0157 to i64
  %cmp15154 = icmp ugt i64 %6, %indvars.iv167
  br i1 %cmp15154, label %for.end, label %for.body16.lr.ph

for.body16.lr.ph:                                 ; preds = %if.end
  %g_t.promoted = load i32, i32* @g_t, align 4
  br label %for.body16

for.body16:                                       ; preds = %for.body16.lr.ph, %for.body16
  %mul27156 = phi i32 [ %g_t.promoted, %for.body16.lr.ph ], [ %mul27, %for.body16 ]
  %v_z.0155 = phi i32 [ %v_pl.0157, %for.body16.lr.ph ], [ %add28, %for.body16 ]
  %mul27 = mul i32 %mul27156, %3
  %add28 = add nuw nsw i32 %v_z.0155, 6
  %7 = zext i32 %add28 to i64
  %cmp15 = icmp ugt i64 %7, %indvars.iv167
  br i1 %cmp15, label %for.cond14.for.end_crit_edge, label %for.body16

for.cond14.for.end_crit_edge:                     ; preds = %for.body16
  %mul27.lcssa = phi i32 [ %mul27, %for.body16 ]
  store i32 21, i32* %arrayidx, align 4
  store i32 %mul27.lcssa, i32* @g_t, align 4
  br label %for.end

for.end:                                          ; preds = %if.end, %for.cond14.for.end_crit_edge
  %.pre-phi = phi i32 [ %3, %for.cond14.for.end_crit_edge ], [ %.pre, %if.end ]
  tail call void @_Z6printbj(i32 %.pre-phi)
  %add39 = add nuw nsw i32 %v_pl.0157, 2
  %cmp10 = icmp ult i32 %add39, 35
  br i1 %cmp10, label %for.body11, label %for.end40

for.end40:                                        ; preds = %for.end
  %v_qf.2.lcssa = phi i16 [ %v_qf.2, %for.end ]
  %8 = shl i64 %indvars.iv167, 1
  %arrayidx44 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_sur, i64 0, i64 %8
  tail call void @_Z4swapRjS_(i32* dereferenceable(4) %arrayidx44, i32* nonnull dereferenceable(4) @g_b)
  %9 = load i32, i32* @g_b, align 4
  %10 = load i32, i32* %arrayidx, align 4
  %or47 = or i32 %10, %9
  store i32 %or47, i32* %arrayidx, align 4
  %indvars.iv.next168 = add nuw nsw i64 %indvars.iv167, 1
  %inc = add nuw nsw i32 %v_n.0159, 1
  %exitcond = icmp eq i64 %indvars.iv.next168, 36
  br i1 %exitcond, label %for.end49, label %cond.end

for.end49:                                        ; preds = %for.end40
  %.lcssa = phi i32 [ %1, %for.end40 ]
  %v_qf.2.lcssa.lcssa = phi i16 [ %v_qf.2.lcssa, %for.end40 ]
  %add8.le = add i32 %.lcssa, 98
  %add51 = add i16 %v_qf.2.lcssa.lcssa, 62
  %xor53 = xor i16 %add51, %v_qf.2.lcssa.lcssa
  tail call void @_Z6printbt(i16 zeroext %xor53)
  tail call void @_Z6printbj(i32 %add8.le)
  tail call void @_Z6printbj(i32 %or)
  tail call void @_Z6printbj(i32 36)
  %11 = load i32, i32* @g_b, align 4
  tail call void @_Z6printbj(i32 %11)
  %12 = load i32, i32* @g_t, align 4
  tail call void @_Z6printbj(i32 %12)
  br label %for.body57

for.body57:                                       ; preds = %for.end49, %for.body57
  %indvars.iv165 = phi i64 [ 0, %for.end49 ], [ %indvars.iv.next166, %for.body57 ]
  %arrayidx59 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_sur, i64 0, i64 %indvars.iv165
  %13 = load i32, i32* %arrayidx59, align 4
  tail call void @_Z6printbj(i32 %13)
  %indvars.iv.next166 = add nuw nsw i64 %indvars.iv165, 1
  %cmp56 = icmp eq i64 %indvars.iv.next166, 192
  br i1 %cmp56, label %for.body67.preheader, label %for.body57

for.body67.preheader:                             ; preds = %for.body57
  br label %for.body67

for.body67:                                       ; preds = %for.body67.preheader, %for.body67
  %indvars.iv163 = phi i64 [ %indvars.iv.next164, %for.body67 ], [ 0, %for.body67.preheader ]
  %arrayidx69 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_cb, i64 0, i64 %indvars.iv163
  %14 = load i32, i32* %arrayidx69, align 4
  tail call void @_Z6printbj(i32 %14)
  %indvars.iv.next164 = add nuw nsw i64 %indvars.iv163, 1
  %cmp65 = icmp eq i64 %indvars.iv.next164, 192
  br i1 %cmp65, label %for.cond78.preheader.preheader, label %for.body67

for.cond78.preheader.preheader:                   ; preds = %for.body67
  br label %for.cond78.preheader

for.cond78.preheader:                             ; preds = %for.cond78.preheader.preheader, %for.cond.cleanup80
  %indvars.iv161 = phi i64 [ %indvars.iv.next162, %for.cond.cleanup80 ], [ 0, %for.cond78.preheader.preheader ]
  br label %for.body81

for.cond.cleanup75:                               ; preds = %for.cond.cleanup80
  tail call void @_Z11flushprintbv()
  ret i32 0

for.cond.cleanup80:                               ; preds = %for.body81
  %indvars.iv.next162 = add nuw nsw i64 %indvars.iv161, 1
  %cmp74 = icmp eq i64 %indvars.iv.next162, 192
  br i1 %cmp74, label %for.cond.cleanup75, label %for.cond78.preheader

for.body81:                                       ; preds = %for.cond78.preheader, %for.body81
  %indvars.iv = phi i64 [ 0, %for.cond78.preheader ], [ %indvars.iv.next, %for.body81 ]
  %arrayidx85 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_nf, i64 0, i64 %indvars.iv161, i64 %indvars.iv
  %15 = load i32, i32* %arrayidx85, align 4
  tail call void @_Z6printbj(i32 %15)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp79 = icmp eq i64 %indvars.iv.next, 192
  br i1 %cmp79, label %for.cond.cleanup80, label %for.body81
}

declare void @_Z6printbj(i32) local_unnamed_addr #2

declare void @_Z4swapRjS_(i32* dereferenceable(4), i32* dereferenceable(4)) local_unnamed_addr #2

declare void @_Z6printbt(i16 zeroext) local_unnamed_addr #2

declare void @_Z11flushprintbv() local_unnamed_addr #2


