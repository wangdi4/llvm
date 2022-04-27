; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "i386-unknown-linux-gnu"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"

define void @example(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = !{i32 1}
