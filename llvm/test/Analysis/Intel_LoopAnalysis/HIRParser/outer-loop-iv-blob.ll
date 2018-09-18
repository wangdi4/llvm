; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check parsing output for the loop verifying that the IV of i1 and i2 loops are
; reverse engineered successfully in the i3 upper. They were failing due to
; conservative wrap flag propagation by ScalarEvolution so parsing ignores wrap
; flags for now.

; CHECK: + DO i1 = 0, 78, 1   <DO_LOOP>
; CHECK: |   %9 = (%qu)[0][-1 * i1 + 80];
; CHECK: |   %10 = (%qu)[0][74];
; CHECK: |   if (%9 != %10)
; CHECK: |   {
; CHECK: |      (%ka)[0] = -1 * i1 + 80;
; CHECK: |
; CHECK: |      + DO i2 = 0, i1 + -42, 1   <DO_LOOP>  <MAX_TC_EST = 37>
; CHECK: |      |   %13 = zext.i32.i64(i2 + -1);
; CHECK: |      |   %inc.lcssa80 = -1 * i1 + 80;
; CHECK: |      |
; CHECK: |      |   + DO i3 = 0, %13, 1   <DO_LOOP>
; CHECK: |      |   |   (%qu)[0][-1 * i1 + i3 + 81] = -1 * i1 + i2 + 80;
; CHECK: |      |   |   %16 = (%qu)[0][-1 * i1 + i3 + 80];
; CHECK: |      |   |   %17 = (%qu)[0][-1 * i1 + i3 + 79];
; CHECK: |      |   |   (%qu)[0][-1 * i1 + i3 + 79] = -1 * %16 + %17;
; CHECK: |      |   + END LOOP
; CHECK: |      |      %inc.lcssa80 = -1 * i1 + %13 + 81;
; CHECK: |      |
; CHECK: |      |   %indvars.iv.next93 = -1 * i1 + i2 + 80  +  1;
; CHECK: |      + END LOOP
; CHECK: |         (%j8)[0] = %inc.lcssa80;
; CHECK: |         (%ka)[0] = %indvars.iv.next93;
; CHECK: |   }
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 't2077140.c'
source_filename = "t2077140.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [9 x i8] c"%u %u %u\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture)

declare i32 @srand(...)

declare i32 @rand(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture)

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  %i5 = alloca i32, align 4
  %ka = alloca i32, align 4
  %j8 = alloca i32, align 4
  %x = alloca [100 x i32], align 16
  %qu = alloca [100 x i32], align 16
  %0 = bitcast i32* %i5 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) 
  store i32 42, i32* %i5, align 4
  %1 = bitcast i32* %ka to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) 
  store i32 78, i32* %ka, align 4
  %2 = bitcast i32* %j8 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) 
  store i32 92, i32* %j8, align 4
  %3 = bitcast [100 x i32]* %x to i8*
  call void @llvm.lifetime.start(i64 400, i8* %3) 
  %4 = bitcast [100 x i32]* %qu to i8*
  call void @llvm.lifetime.start(i64 400, i8* %4) 
  %call.i = tail call i32 (i32, ...) bitcast (i32 (...)* @srand to i32 (i32, ...)*)(i32 33) 
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %entry ]
  %call1.i = tail call i32 (...) @rand() 
  %rem.i = srem i32 %call1.i, 101
  %arrayidx.i = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv.i
  store i32 %rem.i, i32* %arrayidx.i, align 4
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond100 = icmp eq i64 %indvars.iv.next.i, 100
  br i1 %exitcond100, label %init.exit, label %for.body.i

init.exit:                                        ; preds = %for.body.i
  %arrayidx1 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 74
  %call.i46 = tail call i32 (i32, ...) bitcast (i32 (...)* @srand to i32 (i32, ...)*)(i32 91) 
  br label %for.body.i54

for.body.i54:                                     ; preds = %for.body.i54, %init.exit
  %indvars.iv.i47 = phi i64 [ %indvars.iv.next.i51, %for.body.i54 ], [ 0, %init.exit ]
  %call1.i48 = tail call i32 (...) @rand() 
  %rem.i49 = srem i32 %call1.i48, 101
  %arrayidx.i50 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 %indvars.iv.i47
  store i32 %rem.i49, i32* %arrayidx.i50, align 4
  %indvars.iv.next.i51 = add nuw nsw i64 %indvars.iv.i47, 1
  %exitcond99 = icmp eq i64 %indvars.iv.next.i51, 100
  br i1 %exitcond99, label %init.exit55, label %for.body.i54

init.exit55:                                      ; preds = %for.body.i54
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32* nonnull %i5, i32* nonnull %ka, i32* nonnull %j8)
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 15
  %arrayidx4 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 31
  %5 = load i32, i32* %arrayidx4, align 4
  %6 = load i32, i32* %arrayidx3, align 4
  %add = add i32 %6, %5
  store i32 %add, i32* %arrayidx3, align 4
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 25
  %7 = load i32, i32* %arrayidx5, align 4
  %sub = add i32 %7, -6880
  store i32 %sub, i32* %arrayidx5, align 4
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 98
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 11
  %8 = load i32, i32* %arrayidx7, align 4
  store i32 %8, i32* %arrayidx6, align 8
  store i32 80, i32* %i5, align 4
  br label %for.body

for.body:                                         ; preds = %for.inc29, %init.exit55
  %indvars.iv96 = phi i64 [ 80, %init.exit55 ], [ %indvars.iv.next97, %for.inc29 ]
  %indvars.iv88 = phi i32 [ -81, %init.exit55 ], [ %indvars.iv.next89, %for.inc29 ]
  %indvars.iv86 = phi i64 [ 81, %init.exit55 ], [ %indvars.iv.next87, %for.inc29 ]
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 %indvars.iv96
  %9 = load i32, i32* %arrayidx8, align 4
  %10 = load i32, i32* %arrayidx1, align 8
  %cmp9 = icmp eq i32 %9, %10
  br i1 %cmp9, label %for.inc29, label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.body
  %11 = trunc i64 %indvars.iv96 to i32
  store i32 %11, i32* %ka, align 4
  %cmp1178 = icmp ult i64 %indvars.iv96, 39
  br i1 %cmp1178, label %for.body12.lr.ph, label %for.inc29

for.body12.lr.ph:                                 ; preds = %for.cond10.preheader
  %12 = add i32 %11, %indvars.iv88
  br label %for.body12

for.body12:                                       ; preds = %for.body12.lr.ph, %for.inc26
  %indvars.iv92 = phi i32 [ %11, %for.body12.lr.ph ], [ %indvars.iv.next93, %for.inc26 ]
  %indvars.iv90 = phi i32 [ %12, %for.body12.lr.ph ], [ %indvars.iv.next91, %for.inc26 ]
  %13 = zext i32 %indvars.iv90 to i64
  %14 = add i64 %indvars.iv86, %13
  %15 = zext i32 %indvars.iv92 to i64
  %cmp1475 = icmp ult i64 %indvars.iv96, %15
  br i1 %cmp1475, label %for.body15.preheader, label %for.inc26

for.body15.preheader:                             ; preds = %for.body12
  br label %for.body15

for.body15:                                       ; preds = %for.body15.preheader, %for.body15
  %indvars.iv84 = phi i64 [ %indvars.iv.next85, %for.body15 ], [ %indvars.iv96, %for.body15.preheader ]
  %indvars.iv.next85 = add nuw nsw i64 %indvars.iv84, 1
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 %indvars.iv.next85
  store i32 %indvars.iv92, i32* %arrayidx18, align 4
  %arrayidx20 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 %indvars.iv84
  %16 = load i32, i32* %arrayidx20, align 4
  %sub22 = add i64 %indvars.iv84, 4294967295
  %idxprom23 = and i64 %sub22, 4294967295
  %arrayidx24 = getelementptr inbounds [100 x i32], [100 x i32]* %qu, i64 0, i64 %idxprom23
  %17 = load i32, i32* %arrayidx24, align 4
  %add25 = sub i32 %17, %16
  store i32 %add25, i32* %arrayidx24, align 4
  %lftr.wideiv = trunc i64 %indvars.iv.next85 to i32
  %exitcond94 = icmp eq i32 %lftr.wideiv, %indvars.iv92
  br i1 %exitcond94, label %for.inc26.loopexit, label %for.body15

for.inc26.loopexit:                               ; preds = %for.body15
  %18 = trunc i64 %14 to i32
  br label %for.inc26

for.inc26:                                        ; preds = %for.inc26.loopexit, %for.body12
  %inc.lcssa80 = phi i32 [ %11, %for.body12 ], [ %18, %for.inc26.loopexit ]
  %indvars.iv.next93 = add i32 %indvars.iv92, 1
  %cmp11 = icmp ult i32 %indvars.iv.next93, 39
  %indvars.iv.next91 = add i32 %indvars.iv90, 1
  br i1 %cmp11, label %for.body12, label %for.cond10.for.inc29.loopexit_crit_edge

for.cond10.for.inc29.loopexit_crit_edge:          ; preds = %for.inc26
  %indvars.iv.next93.lcssa = phi i32 [ %indvars.iv.next93, %for.inc26 ]
  %inc.lcssa80.lcssa = phi i32 [ %inc.lcssa80, %for.inc26 ]
  store i32 %inc.lcssa80.lcssa, i32* %j8, align 4
  store i32 %indvars.iv.next93.lcssa, i32* %ka, align 4
  br label %for.inc29

for.inc29:                                        ; preds = %for.cond10.preheader, %for.cond10.for.inc29.loopexit_crit_edge, %for.body
  %indvars.iv.next97 = add nsw i64 %indvars.iv96, -1
  %indvars.iv.next87 = add nsw i64 %indvars.iv86, -1
  %indvars.iv.next89 = add nuw nsw i32 %indvars.iv88, 1
  %exitcond98 = icmp eq i32 %indvars.iv.next89, -2
  br i1 %exitcond98, label %for.body.i62, label %for.body

for.body.i62:                                     ; preds = %for.inc29
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @__isoc99_scanf(i8* nocapture readonly, ...) 

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 

