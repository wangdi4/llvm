; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Verify that we do not create SCC (%add80376 -> %add47 -> %add80375) which has live-range overlap because both %add80376 is used in %add80375 after being killed by %add47 in the same bblock.

; CHECK: SCC1 
; CHECK-SAME: %0
; CHECK-NOT: SCC

;Module Before HIR; ModuleID = 't3408.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main(i32 %sub, i32 %.pre455, i32 %add31.lcssa, i32 %addop, i32 %addop1) {
entry:
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %n = alloca i32, align 4
  %b3 = alloca i32, align 4
  %n9 = alloca i32, align 4
  %jg = alloca i32, align 4
  %ny = alloca i32, align 4
  %a = alloca [100 x i32], align 16
  %on = alloca [100 x i32], align 16
  %x = alloca [100 x i32], align 16
  %nx = alloca [100 x [100 x i32]], align 16
  %c = alloca [100 x i32], align 16
  %ul = alloca [100 x i32], align 16
  %i2 = alloca [100 x i32], align 16
  store i32 39, i32* %j, align 4
  store i32 0, i32* %k, align 4
  store i32 43, i32* %n, align 4
  store i32 19, i32* %b3, align 4
  store i32 34, i32* %n9, align 4
  store i32 11, i32* %jg, align 4
  store i32 3, i32* %ny, align 4
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %i2, i64 0, i64 6
  br label %for.body38

for.body38:                                       ; preds = %for.cond36.backedge, %entry
  %0 = phi i32 [ %sub, %entry ], [ %10, %for.cond36.backedge ]
  %1 = phi i32 [ %.pre455, %entry ], [ %3, %for.cond36.backedge ]
  %indvars.iv419 = phi i64 [ 1, %entry ], [ %indvars.iv.next420, %for.cond36.backedge ]
  %add80376 = phi i32 [ %add31.lcssa, %entry ], [ %add80375, %for.cond36.backedge ]
  %arrayidx41 = getelementptr inbounds [100 x i32], [100 x i32]* %on, i64 0, i64 %indvars.iv419
  %2 = load i32, i32* %arrayidx41, align 4
  %sum = add i32 %add80376, %addop1
  %sub42 = sub i32 %2, %sum
  store i32 %sub42, i32* %arrayidx41, align 4
  %indvars.iv.next420 = add nuw nsw i64 %indvars.iv419, 1
  %arrayidx45 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv.next420
  %3 = load i32, i32* %arrayidx45, align 4
  %add47 = sub i32 %add80376, %3
  %cmp48 = icmp eq i64 %indvars.iv419, 1
  %4 = add nsw i64 %indvars.iv419, -1
  br i1 %cmp48, label %if.then49, label %if.else58

if.then49:                                        ; preds = %for.body38
  %5 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv419
  %sub50138 = add i32 %1, -21
  %add54 = add i32 %sub50138, %addop
  store i32 %add54, i32* %5, align 4
  %arrayidx57 = getelementptr inbounds [100 x i32], [100 x i32]* %on, i64 0, i64 %4
  %6 = load i32, i32* %arrayidx57, align 4
  store i32 %6, i32* %arrayidx, align 8
  br label %if.end

if.else58:                                        ; preds = %for.body38
  %arrayidx63 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv.next420
  %7 = load i32, i32* %arrayidx63, align 4
  %sub64 = sub i32 %7, %1
  store i32 %sub64, i32* %arrayidx63, align 4
  %8 = load i32, i32* %ny, align 4
  %inc65 = add i32 %8, 1
  store i32 %inc65, i32* %ny, align 4
  %arrayidx71 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %nx, i64 0, i64 %indvars.iv.next420, i64 %4
  %9 = load i32, i32* %arrayidx71, align 4
  %mul72 = mul i32 %9, %8
  store i32 %mul72, i32* %arrayidx71, align 4
  br label %if.end

if.end:                                           ; preds = %if.else58, %if.then49
  %10 = phi i32 [ %0, %if.else58 ], [ %6, %if.then49 ]
  %arrayidx74 = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 %indvars.iv419
  %11 = load i32, i32* %arrayidx74, align 4
  %cmp75 = icmp ugt i32 %11, %10
  br i1 %cmp75, label %if.else81, label %for.cond36.backedge

for.cond36.backedge:                              ; preds = %if.end, %if.else81
  %add80375 = phi i32 [ %add47, %if.else81 ], [ %add80376, %if.end ]
  %exitcond422 = icmp eq i64 %indvars.iv.next420, 18
  br i1 %exitcond422, label %for.inc95.loopexit, label %for.body38

if.else81:                                        ; preds = %if.end
  %arrayidx84 = getelementptr inbounds [100 x i32], [100 x i32]* %i2, i64 0, i64 %indvars.iv.next420
  %12 = load i32, i32* %arrayidx84, align 4
  %arrayidx87 = getelementptr inbounds [100 x i32], [100 x i32]* %ul, i64 0, i64 %indvars.iv.next420
  %13 = load i32, i32* %arrayidx87, align 4
  %sub88 = sub i32 %13, %12
  store i32 %sub88, i32* %arrayidx87, align 4
  br label %for.cond36.backedge

for.inc95.loopexit:                               ; preds = %for.cond36.backedge
  %add80375.lcssa = phi i32 [ %add80375, %for.cond36.backedge ]
  store i32 %add80375.lcssa, i32* %b3, align 4
  store i32 18, i32* %jg, align 4
  ret i32 0
}

