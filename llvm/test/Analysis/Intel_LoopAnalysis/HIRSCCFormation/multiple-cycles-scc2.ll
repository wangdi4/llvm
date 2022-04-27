; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck %s

; Verify that we do not construct the SCC (%v_fnaob.1254 -> %v_fnaob.0261 -> 
; %xor105.lcssa -> %xor105 -> %xor97) which has multiple cycles. There are 3
; other legitimate SCCs which are formed including the inner loop SCC (%xor105
; -> %v_fnaob.1254).

; CHECK: SCC1
; CHECK-SAME: %.lcssa318320

; CHECK: SCC2
; CHECK-SAME: %xor111259

; CHECK: SCC3
; CHECK-NOT: %v_fnaob.0261
; CHECK-SAME: %v_fnaob.1254


;Module Before HIR; ModuleID = 'bug6.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@g_djqb = local_unnamed_addr global i32 67, align 4
@g_n = local_unnamed_addr global i32 -43, align 4
@g_oy = global i32 96, align 4
@g_i = local_unnamed_addr global i32 84, align 4
@g_ksoh = local_unnamed_addr global i16 83, align 2
@g_t = local_unnamed_addr global i32 2604, align 4
@a1_ritun = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_pxx = local_unnamed_addr global [192 x i64] zeroinitializer, align 16
@a2_t = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16
@a2_e = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define i32 @foo(i32 %inc77) local_unnamed_addr #0 {
entry:
  %v_fqrhgjc.i = alloca i8, align 1
  %v_s = alloca i64, align 8
  %0 = load i32, i32* @g_djqb, align 4
  %1 = load i32, i32* @g_t, align 4
  %or = or i32 %1, 13
  store i32 %or, i32* @g_t, align 4
  store i32 0, i32* @g_n, align 4
  %2 = bitcast i64* %v_s to i8*
  %g_i.promoted268 = load i32, i32* @g_i, align 4
  %g_ksoh.promoted = load i16, i16* @g_ksoh, align 2
  %g_t.promoted = load i32, i32* @g_t, align 4
  br label %for.body81

for.body81:                                       ; preds = %for.end120, %entry
  %.lcssa318320 = phi i32 [ %g_t.promoted, %entry ], [ %.lcssa, %for.end120 ]
  %conv98.lcssa.lcssa319 = phi i16 [ %g_ksoh.promoted, %entry ], [ %conv98.lcssa.lcssa, %for.end120 ]
  %dec119.lcssa269 = phi i32 [ %g_i.promoted268, %entry ], [ 21, %for.end120 ]
  %sub117.lcssa267 = phi i32 [ %inc77, %entry ], [ %sub117.lcssa, %for.end120 ]
  %v_wfebnjn.5266 = phi i32 [ 0, %entry ], [ %inc122, %for.end120 ]
  br label %for.body87.lr.ph

for.body87.lr.ph:                                 ; preds = %for.body81, %for.end114
  %3 = phi i32 [ %.lcssa318320, %for.body81 ], [ %5, %for.end114 ]
  %conv98.lcssa316 = phi i16 [ %conv98.lcssa.lcssa319, %for.body81 ], [ %conv98.lcssa, %for.end114 ]
  %indvars.iv299 = phi i32 [ 10, %for.body81 ], [ %indvars.iv.next300, %for.end114 ]
  %4 = phi i32 [ 53, %for.body81 ], [ %dec119, %for.end114 ]
  %xor111.lcssa263 = phi i32 [ %sub117.lcssa267, %for.body81 ], [ %sub117, %for.end114 ]
  %v_fnaob.0261 = phi i32 [ %dec119.lcssa269, %for.body81 ], [ %xor105.lcssa, %for.end114 ]
  br label %for.body87

for.body87:                                       ; preds = %for.body87.lr.ph, %for.body87
  %dec113260 = phi i32 [ 63, %for.body87.lr.ph ], [ %dec113, %for.body87 ]
  %xor111259 = phi i32 [ %xor111.lcssa263, %for.body87.lr.ph ], [ %xor111, %for.body87 ]
  %conv98257 = phi i16 [ %conv98.lcssa316, %for.body87.lr.ph ], [ %conv98, %for.body87 ]
  %v_fnaob.1254 = phi i32 [ %v_fnaob.0261, %for.body87.lr.ph ], [ %xor105, %for.body87 ]
  %conv96 = zext i16 %conv98257 to i32
  %xor97 = xor i32 %conv96, %v_fnaob.0261
  %conv98 = trunc i32 %xor97 to i16
  %conv104 = and i32 %xor97, 65535
  %xor105 = xor i32 %conv104, %v_fnaob.1254
  %inc107 = add i32 %xor111259, 1
  %and = and i32 %4, %inc107
  %xor111 = xor i32 %dec113260, %and
  %dec113 = add nsw i32 %dec113260, -1
  %cmp86 = icmp ugt i32 %dec113, %4
  br i1 %cmp86, label %for.body87, label %for.end114

for.end114:                                       ; preds = %for.body87
  %conv98.lcssa = phi i16 [ %conv98, %for.body87 ]
  %xor105.lcssa = phi i32 [ %xor105, %for.body87 ]
  %xor111.lcssa = phi i32 [ %xor111, %for.body87 ]
  %5 = add i32 %3, %indvars.iv299
  %sub117 = add i32 %xor111.lcssa, -33
  %dec119 = add nsw i32 %4, -1
  %indvars.iv.next300 = add nuw nsw i32 %indvars.iv299, 1
  %exitcond = icmp eq i32 %indvars.iv.next300, 42
  br i1 %exitcond, label %for.end120, label %for.body87.lr.ph

for.end120:                                       ; preds = %for.end114
  %.lcssa = phi i32 [ %5, %for.end114 ]
  %sub117.lcssa = phi i32 [ %sub117, %for.end114 ]
  %conv98.lcssa.lcssa = phi i16 [ %conv98.lcssa, %for.end114 ]
  %inc122 = add nuw nsw i32 %v_wfebnjn.5266, 1
  %exitcond301 = icmp eq i32 %inc122, 64
  br i1 %exitcond301, label %for.end123, label %for.body81

for.end123:                                       ; preds = %for.end120
  ret i32 0
}

