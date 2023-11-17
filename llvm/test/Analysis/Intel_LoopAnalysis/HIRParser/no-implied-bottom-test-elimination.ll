; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that we do not eliminate the bottom test of the loop which is implied
; true by identical dominating condition %cmp29.

; This allows us to successfully form the loop.


; CHECK: + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %storemerge12.out = 5 * i1 + 1;
; CHECK: |   %indvars.iv.out = 5 * i1 + 10;
; CHECK: |   if (-1 * i1 + 1 == 0)
; CHECK: |   {
; CHECK: |      goto for.end7;
; CHECK: |   }
; CHECK: |   %1 = (@nz)[0][5 * i1 + 1];
; CHECK: |   (@nz)[0][5 * i1 + 1] = %1 + %0;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@s = dso_local local_unnamed_addr global i64 0, align 8
@o = dso_local local_unnamed_addr global i64 0, align 8
@nz = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

define dso_local i32 @main() {
entry:
  %0 = load i32, ptr @nz, align 16
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %indvars.iv = phi i64 [ 10, %entry ], [ %indvars.iv.next, %for.inc5 ]
  %cmp29 = phi i1 [ true, %entry ], [ false, %for.inc5 ]
  %storemerge12 = phi i64 [ 1, %entry ], [ 6, %for.inc5 ]
  %arrayidx = getelementptr inbounds [20 x i32], ptr @nz, i64 0, i64 %storemerge12
  br i1 %cmp29, label %for.inc5, label %for.end7

for.inc5:                                         ; preds = %for.cond1.preheader
  %1 = load i32, ptr %arrayidx, align 4
  %add = add i32 %1, %0
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 5
  br i1 %cmp29, label %for.cond1.preheader, label %for.end7

for.end7:                                         ; preds = %for.cond1.preheader, %for.inc5
  %storemerge8.lcssa18 = phi i64 [ %indvars.iv, %for.inc5 ], [ %storemerge12, %for.cond1.preheader ]
  store i64 11, ptr @o, align 8
  store i64 %storemerge8.lcssa18, ptr @s, align 8
  ret i32 0
}

