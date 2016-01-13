; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-before=hir-complete-unroll -print-after=hir-complete-unroll -hir-details 2>&1 < %s | FileCheck %s

; Source code-
; for(int i=0; i<6; i++) {
;   A[i] = k*i;
; }


; Check that the blob DDRefs are updated correctly after complete unroll


; CHECK: Dump Before HIR Complete Unroll

; CHECK: <RVAL-REG> LINEAR i32 %k * i1

; Capture symbase assigned to %k.
; CHECK: <BLOB> LINEAR i32 %k {sb:[[SYM:[0-9]+]]}


; CHECK: Dump After HIR Complete Unroll

; CHECK: (%A)[0] = 0
; Check for absence of any blob DDRefs for %k for the above statement.
; CHECK-NOT: <BLOB> LINEAR i32 %k 

; CHECK: (%A)[1] = %k

; Check that the symbase of the self-blob regular DDRef has been updated to %k's symbase.
; CHECK: <RVAL-REG> LINEAR i32 %k {sb:[[SYM]]}

; Check that the blob DDRef for %k has been removed for the self-blob regular DDRef.
; CHECK-NOT: <BLOB> 

; CHECK: (%A)[2] = 2 * %k


; ModuleID = 'blob-update.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %A, i32 %k) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %mul = mul nsw i32 %i.01, %k
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 6
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

