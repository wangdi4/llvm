; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Should not convert: store is in an if statement.
define void @example(<8 x i64>* %dest) "target-features"="+avx512f" {
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
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  br label %loop.latch

loop.latch:
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Should convert: store is not in an if statement.
define void @example2(<8 x i64>* %dest) "target-features"="+avx512f" {
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
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Should not convert: exiting block is in an if statement.
define void @example3(<8 x i64>* %dest) "target-features"="+avx512f" {
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
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  br label %loop

exit:
  ret void
}

; Should convert: exiting block not in if statement (dominates latch).
define void @example4(<8 x i64>* %dest) "target-features"="+avx512f" {
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
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  br label %loop

exit:
  ret void
}

!0 = !{i32 1}
