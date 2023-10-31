; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -aa-pipeline="default" -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

; Check that we correctly identify that we can't delay the stores in the
; following loop:
; for (i = 0; i < 1000; i++)
;   A[i + 1] = A[i];

define void @example(ptr %src) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  %dest = getelementptr inbounds <8 x i64>, ptr %src, i32 1
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  %addr.src = getelementptr inbounds <8 x i64>, ptr %src, i64 %index
  %val = load <8 x i64>, ptr %addr.src, align 16
  store <8 x i64> %val, ptr %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = !{i32 1}
