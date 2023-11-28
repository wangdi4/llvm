; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-memory-reduction-sinking,print<hir>" -hir-cost-model-throttling=0  -disable-output  2>&1 | FileCheck %s

; This test case checks that the memory reductions inside the internal gotos
; are sinked properly and the conditions are created correctly. The reductions
; for (@A)[0][7] should be handled under one condition, and (@A)[0][5] in
; another condition.

; HIR before transformation

;   BEGIN REGION { }
;         + DO i1 = 0, 99, 1   <DO_LOOP>
;         |   (@B)[0][i1] = i1;
;         |   if (i1 != 0)
;         |   {
;         |      if (i1 == 1)
;         |      {
;         |         goto L2;
;         |      }
;         |      if (i1 == 2)
;         |      {
;         |         goto L2.63;
;         |      }
;         |   }
;         |   %0 = (@A)[0][5];
;         |   (@A)[0][5] = %0 + 1;
;         |   L2:
;         |   %1 = (@A)[0][7];
;         |   (@A)[0][7] = %1 + 1;
;         |   %2 = (@A)[0][7];
;         |   (@A)[0][7] = %2 + 1;
;         |   L2.63:
;         |   %3 = (@A)[0][3];
;         |   (@A)[0][3] = %3 + 1;
;         + END LOOP
;   END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0;
; CHECK:          %tmp8 = 0;
; CHECK:          %tmp10 = 0;
; CHECK:          %tmp14 = 0;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   (@B)[0][i1] = i1;
; CHECK:       |   if (i1 != 0)
; CHECK:       |   {
; CHECK:       |      if (i1 == 1)
; CHECK:       |      {
; CHECK:       |         goto L2;
; CHECK:       |      }
; CHECK:       |      if (i1 == 2)
; CHECK:       |      {
; CHECK:       |         goto L2.63;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   %tmp14 = %tmp14  +  1;
; CHECK:       |   L2:
; CHECK:       |   %tmp10 = %tmp10  +  1;
; CHECK:       |   %tmp8 = %tmp8  +  1;
; CHECK:       |   L2.63:
; CHECK:       |   %tmp = %tmp  +  1;
; CHECK:       + END LOOP
; CHECK:          %3 = (@A)[0][3];
; CHECK:          (@A)[0][3] = %3 + %tmp;
; CHECK:       if (%tmp14 != 0)
; CHECK:       {
; CHECK:          %0 = (@A)[0][5];
; CHECK:          (@A)[0][5] = %0 + %tmp14;
; CHECK:       }
; CHECK:       %cmp12 = %tmp8 != 0;
; CHECK:       %cmp13 = %tmp10 != 0;
; CHECK:       %or = %cmp12  |  %cmp13;
; CHECK:       if (%or != 0)
; CHECK:       {
; CHECK:          %1 = (@A)[0][7];
; CHECK:          (@A)[0][7] = %1 + %tmp10;
; CHECK:          %2 = (@A)[0][7];
; CHECK:          (@A)[0][7] = %2 + %tmp8;
; CHECK:       }
; CHECK: END REGION


; ModuleID = 'goto.c'
source_filename = "goto.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal global [100 x i32] zeroinitializer, align 16
@B = internal global [100 x i32] zeroinitializer, align 16

define i32 @main() {
entry:
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc15, %do.cond ]
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [5 x i32], ptr @B, i32 0, i64 %idxprom
  store i32 %i.0, ptr %arrayidx, align 4
  %cmp = icmp eq i32 %i.0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %do.body
  br label %L1

if.end:                                           ; preds = %do.body
  %cmp1 = icmp eq i32 %i.0, 1
  br i1 %cmp1, label %if.then.2, label %if.end.3

if.then.2:                                        ; preds = %if.end
  br label %L2

if.end.3:                                         ; preds = %if.end
  %cmp4 = icmp eq i32 %i.0, 2
  br i1 %cmp4, label %if.then.5, label %if.end.6

if.then.5:                                        ; preds = %if.end.3
  br label %L2.63

if.end.6:                                         ; preds = %if.end.3
  br label %L1

L1:                                               ; preds = %if.end.6, %if.then
  %arrayidx8 = getelementptr inbounds [100 x i32], ptr @A, i32 0, i64 5
  %0 = load i32, ptr %arrayidx8, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx8, align 4
  br label %L2

L2:                                               ; preds = %L1, %if.then.2
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr @A, i32 0, i64 7
  %1 = load i32, ptr %arrayidx10, align 4
  %inc11 = add nsw i32 %1, 1
  store i32 %inc11, ptr %arrayidx10, align 4

  %arrayidx11 = getelementptr inbounds [100 x i32], ptr @A, i32 0, i64 7
  %2 = load i32, ptr %arrayidx11, align 4
  %inc12 = add nsw i32 %2, 1
  store i32 %inc12, ptr %arrayidx11, align 4

  br label %L2.63

L2.63:                                            ; preds = %L2, %if.then.5
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr @A, i32 0, i64 3
  %3 = load i32, ptr %arrayidx13, align 4
  %inc14 = add nsw i32 %3, 1
  store i32 %inc14, ptr %arrayidx13, align 4
  %inc15 = add nsw i32 %i.0, 1
  br label %do.cond

do.cond:                                          ; preds = %L2.63
  %cmp16 = icmp ne i32 %inc15, 100
  br i1 %cmp16, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  %4 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i32 0, i64 0), align 4
  ret i32 %4
}
