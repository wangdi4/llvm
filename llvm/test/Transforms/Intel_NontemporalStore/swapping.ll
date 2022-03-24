; CMPLRLLVM-24857
; XFAIL: *

; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -aa-pipeline="default" -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check that we correctly identify contiguous arrays if they are swapping.
define void @example(<8 x i64>* noalias %arr1, <8 x i64>* noalias %arr2, i64 %N) "target-features"="+avx512f" {
; CHECK-LABEL: @example
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %swap_loop

swap_loop:
  %swap.idx = phi i64 [ 0, %entry ], [ %swap.idx.next, %swap_loop.check ]
  %dest = phi <8 x i64>* [ %arr1, %entry ], [ %src, %swap_loop.check ]
  %src = phi <8 x i64>* [ %arr2, %entry ], [ %dest, %swap_loop.check ]
  br label %loop

loop:
  %index = phi i64 [ 0, %swap_loop ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %trash = load <8 x i64>, <8 x i64>* %src, align 16
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %swap_loop.check, label %loop

swap_loop.check:
  %swap.idx.next = add i64 %swap.idx, 1
  %swap_loop.cond = icmp eq i64 %swap.idx.next, %N
  br i1 %swap_loop.cond, label %exit, label %swap_loop

exit:
  ret void
}

!0 = !{i32 1}
