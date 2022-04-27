; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Conflicting loads: store is volatile.
define void @example(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store volatile <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Conflicting loads: load in same BB.
define void @example2(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example2(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %val = load <8 x i64>, <8 x i64>* %addr, align 16
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Conflicting loads: load in different BB.
define void @example3(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example3(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop.next ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  br label %loop.next

loop.next:
  %val = load <8 x i64>, <8 x i64>* %addr, align 16
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Conflicting loads: array ref in same range, but not same addr.
define void @example4(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example4(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %val = load <8 x i64>, <8 x i64>* %dest, align 16
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Conflicting stores: store in same range of loop.
define void @example5(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example5(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  store <8 x i64> zeroinitializer, <8 x i64>* %dest, align 16
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = !{i32 1}
