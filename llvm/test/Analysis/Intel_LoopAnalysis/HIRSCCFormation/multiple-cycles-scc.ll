; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Verify that we do not construct the SCC (%4 -> %inc.us -> %inc5.us -> %10) which contains two different cycles in it (%4 -> %inc.us -> %10) and (%4 -> %inc5.us -> %10). 
; CHECK-NOT: SCC1


; ModuleID = 'multiple-cycles-scc.ll'
source_filename = "bug.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_u = local_unnamed_addr global i32 0, align 4
@g_y = local_unnamed_addr global i32 0, align 4
@g_c = local_unnamed_addr global i32 1, align 4
@g_xau = local_unnamed_addr global i32 0, align 4
@a1_x_0 = local_unnamed_addr global i32 0, align 4
@main_v_wu = local_unnamed_addr global i32 0, align 4
@main_v_mqqmq = local_unnamed_addr global i32 1, align 4
@main_v_mjl = local_unnamed_addr global i32 0, align 4
@g_yf = global i32 0, align 4
@a2_gfnls = local_unnamed_addr global [0 x i32] zeroinitializer, align 4

define i32 @main() local_unnamed_addr {
entry:
  %.pr = load i32, i32* @main_v_wu, align 4
  %cmp20 = icmp slt i32 %.pr, 1
  br i1 %cmp20, label %for.body.lr.ph, label %for.end.loopexit

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32, i32* @main_v_mqqmq, align 4
  %cmp1 = icmp sgt i32 %0, 0
  %1 = load i32, i32* @g_c, align 4
  %tobool = icmp eq i32 %1, 0
  %2 = load i32, i32* @g_u, align 4
  %tobool6 = icmp eq i32 %2, 0
  br i1 %cmp1, label %for.body.us.preheader, label %for.end.loopexit

for.body.us.preheader:                            ; preds = %for.body.lr.ph
  %.pre = load i32, i32* @g_y, align 4
  br label %for.body.us

for.body.us:                                      ; preds = %for.inc.us, %for.body.us.preheader
  %3 = phi i32 [ %.pr, %for.body.us.preheader ], [ %inc11.us, %for.inc.us ]
  %4 = phi i32 [ %.pre, %for.body.us.preheader ], [ %10, %for.inc.us ]
  %inc.us = add nsw i32 %4, 1
  store i32 %inc.us, i32* @g_y, align 4
  br i1 %tobool, label %for.inc.us, label %if.then2.us

if.then2.us:                                      ; preds = %for.body.us
  %5 = load i32, i32* @g_xau, align 4
  %inc3.us = add nsw i32 %5, 1
  store i32 %inc3.us, i32* @g_xau, align 4
  %6 = load volatile i32, i32* @g_yf, align 4
  %inc4.us = add i32 %6, 1
  store volatile i32 %inc4.us, i32* @g_yf, align 4
  %inc5.us = add nsw i32 %4, 2
  store i32 %inc5.us, i32* @g_y, align 4
  br i1 %tobool6, label %if.else.us, label %if.then7.us

if.then7.us:                                      ; preds = %if.then2.us
  %7 = load i32, i32* @a1_x_0, align 4
  %xor.us = xor i32 %7, 1
  store i32 %xor.us, i32* @a1_x_0, align 4
  br label %if.end.us

if.else.us:                                       ; preds = %if.then2.us
  %idxprom.us = sext i32 %3 to i64
  %arrayidx.us = getelementptr inbounds [0 x i32], [0 x i32]* @a2_gfnls, i64 0, i64 %idxprom.us
  %8 = load i32, i32* %arrayidx.us, align 4
  %inc8.us = add nsw i32 %8, 1
  store i32 %inc8.us, i32* %arrayidx.us, align 4
  br label %if.end.us

if.end.us:                                        ; preds = %if.else.us, %if.then7.us
  %9 = load i32, i32* @main_v_mjl, align 4
  %dec.us = add nsw i32 %9, -1
  store i32 %dec.us, i32* @main_v_mjl, align 4
  br label %for.inc.us

for.inc.us:                                       ; preds = %if.end.us, %for.body.us
  %10 = phi i32 [ %inc5.us, %if.end.us ], [ %inc.us, %for.body.us ]
  %inc11.us = add nsw i32 %3, 1
  store i32 %inc11.us, i32* @main_v_wu, align 4
  %cmp.us = icmp slt i32 %3, 0
  br i1 %cmp.us, label %for.body.us, label %for.end.loopexit.loopexit

for.end.loopexit.loopexit:                        ; preds = %for.inc.us
  br label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.end.loopexit.loopexit, %for.body.lr.ph, %entry
  ret i32 0
}
