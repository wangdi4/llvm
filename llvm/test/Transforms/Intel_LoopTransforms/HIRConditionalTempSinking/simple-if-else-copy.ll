; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-conditional-temp-sinking -print-before=hir-conditional-temp-sinking -print-after=hir-conditional-temp-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-conditional-temp-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that the simple if-else reduction is converted into unconditional reduction after sinking.

; Before Change-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if ((@A)[0][i1] > 0)
; CHECK: |   {
; CHECK: |      %t.1 = 2;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %t.1 = 2;
; CHECK: |   }
; CHECK: + END LOOP


; After change-

; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT: |   %t.1 = 2;
; CHECK-NEXT: + END LOOP


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
  br label %for.inc

if.else:                                          ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %t.1 = phi i32 [ 2, %if.then ], [ 2, %if.else ]
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %t.0.lcssa = phi i32 [ %t.1, %for.inc ]
  ret i32 %t.0.lcssa
}
