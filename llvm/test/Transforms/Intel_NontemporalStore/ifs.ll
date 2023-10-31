; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

; Should not convert: store is in an if statement.
define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop.latch ]
  %index.next = add i64 %index, 1
  %if.test = icmp eq i64 %index, 5
  br i1 %if.test, label %loop.latch, label %loop.if

loop.if:
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  br label %loop.latch

loop.latch:
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Should convert: store is not in an if statement.
define void @example2(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example2(
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop.latch ]
  %index.next = add i64 %index, 1
  %if.test = icmp eq i64 %index, 5
  br i1 %if.test, label %loop.latch, label %loop.if

loop.if:
  br label %loop.latch

loop.latch:
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Should not convert: exiting block is in an if statement.
define void @example3(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example3(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop.latch ]
  %index.next = add i64 %index, 1
  %if.test = icmp ne i64 %index, 5
  br i1 %if.test, label %loop.latch, label %loop.if

loop.if:
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop.latch

loop.latch:
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  br label %loop

exit:
  ret void
}

; Should convert: exiting block not in if statement (dominates latch).
define void @example4(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example4(
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop.latch ]
  %index.next = add i64 %index, 1
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop.latch

loop.latch:
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  br label %loop

exit:
  ret void
}

!0 = !{i32 1}
