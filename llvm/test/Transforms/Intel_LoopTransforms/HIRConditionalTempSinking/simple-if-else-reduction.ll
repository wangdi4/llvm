; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-conditional-temp-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-conditional-temp-sinking" -print-changed -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED

; Verify that the simple if-else reduction is converted into unconditional reduction after sinking.

; Before Change-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      %t.02 = %t.02  +  %0;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %t.02 = %t.02  +  (@B)[0][i1];
; CHECK: |   }
; CHECK: + END LOOP


; After change-

; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT: |   %0 = (@A)[0][i1];
; CHECK-NEXT: |   if (%0 > 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %tmp = %0;
; CHECK-NEXT: |   }
; CHECK-NEXT: |   else
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %tmp = (@B)[0][i1];
; CHECK-NEXT: |   }
; CHECK-NEXT: |   %t.02 = %t.02  +  %tmp;
; CHECK-NEXT: + END LOOP

; Verify that pass is dumped with print-changed when it triggers.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRConditionalTempSinking


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %t.02 = phi i32 [ 0, %entry ], [ %t.1, %for.inc ]
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %t.02, %0
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %idxprom
  %1 = load i32, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %t.02, %1
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %t.1 = phi i32 [ %add, %if.then ], [ %add6, %if.else ]
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %t.0.lcssa = phi i32 [ %t.1, %for.inc ]
  ret i32 %t.0.lcssa
}
