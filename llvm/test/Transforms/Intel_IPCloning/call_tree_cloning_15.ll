; RUN: opt < %s -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-do-mv=false -S | FileCheck %s
; RUN: opt < %s -call-tree-clone -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-do-mv=false -S | FileCheck %s

; Check that Call Tree Cloning transformation keeps !noalias and !alias_scope metadata on call instruction(s)

; CHECK: call fastcc void @"foo|_.10"(float* getelementptr inbounds ([100 x float], [100 x float]* @a, i64 0, i64 0)), !noalias !0
; CHECK: call fastcc void @"foo|_.20"(float* getelementptr inbounds ([100 x float], [100 x float]* @b, i64 0, i64 0)), !alias.scope !0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.1 = private unnamed_addr constant [9 x i8] c"foo(%p)\0A\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"loop:%d\0A\00", align 1
@a = internal global [100 x float] zeroinitializer, align 16
@b = internal global [100 x float] zeroinitializer, align 16

define internal fastcc void @foo(float* %0, i32 %1) {
  %3 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i64 0, i64 0), float* %0)
  br label %4

4:
  %5 = phi i32 [ %7, %4 ], [ 0, %2 ]
  %6 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 %5)
  %7 = add nuw nsw i32 %5, 1
  %8 = icmp eq i32 %7, %1
  br i1 %8, label %9, label %4

9:
  ret void
}

define i32 @main() {
  tail call fastcc void @foo(float* getelementptr inbounds ([100 x float], [100 x float]* @a, i64 0, i64 0), i32 10), !noalias !0
  tail call fastcc void @foo(float* getelementptr inbounds ([100 x float], [100 x float]* @b, i64 0, i64 0), i32 20), !alias.scope !0
  ret i32 0
}

declare i32 @printf(i8* nocapture readonly, ...)

!0 = !{!1}
!1 = distinct !{!1, !2, !"hello: %a"}
!2 = distinct !{!2, !"hello"}
