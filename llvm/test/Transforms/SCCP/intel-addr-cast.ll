; This test case checks that the interprocedural sparse conditional constant
; propagation doesn't remove the pointer cast in the ArrayType since the types
; between the variable and the constant matches.


; RUN: opt -passes=ipsccp -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@globArray = internal addrspace(3) global [5 x i32] zeroinitializer

define ptr addrspace(4) @foo(i32 %a, i32 %b) {
entry:
  %tmp1 = icmp eq i32 %a, 0
  br i1 %tmp1, label %true_bb, label %false_bb

true_bb:
  br label %merge_bb

false_bb:
  %tmp2 = icmp eq i32 %b, 0
  br i1 %tmp2, label %sub_true_bb, label %sub_false_bb

sub_true_bb:
  br label %sub_merge_bb

sub_false_bb:
  br label %sub_merge_bb

sub_merge_bb:
  %tmp3 = phi ptr addrspace(4) [ addrspacecast (ptr addrspace(3) @globArray to ptr addrspace(4)), %sub_true_bb ], [ undef, %sub_false_bb ]
  br label %merge_bb

merge_bb:
  %tmp4 = phi ptr addrspace(4) [ undef, %true_bb ], [ %tmp3, %sub_merge_bb ]
  ret ptr addrspace(4) %tmp4
}

define ptr addrspace(4) @bar() {
  %tmp1 = call ptr addrspace(4) @foo(i32 1, i32 0)
  ret ptr addrspace(4) %tmp1
}

; Check that @globArray was propagated in @foo
; CHECK: define ptr addrspace(4) @foo(i32 %a, i32 %b) {
; CHECK: ret ptr addrspace(4) addrspacecast (ptr addrspace(3) @globArray to ptr addrspace(4))

; Check that @globArray was propagated in @bar
; CHECK: define ptr addrspace(4) @bar() {
; CHECK: ret ptr addrspace(4) addrspacecast (ptr addrspace(3) @globArray to ptr addrspace(4))
