; REQUIRES: asserts
; RUN: opt -S -passes=vplan-vec -debug-only=VPlanHCFGBuilder -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @main() {
; CHECK:  The trip count for loop outer.loop.header is 2048
; CHECK-NEXT:  The trip count for loop inner.loop.header is 256
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.loop.header

outer.loop.header:
  %outer.iv = phi i32 [ 0, %preheader ], [ %outer.iv.next, %outer.loop.latch ]
  br label %inner.loop.header

inner.loop.header:
  %inner.iv = phi i32 [ 0, %outer.loop.header ], [ %inner.iv.next, %inner.loop.latch ]
  %val = add nsw i32 %inner.iv, 1
  %early.exit.chk = icmp eq i32 %val, 256
  br i1 %early.exit.chk, label %bb1, label %inner.loop.latch

inner.loop.latch:
  %inner.iv.next = add nsw i32 %inner.iv, 1
  %inner.bottom.test = icmp eq i32 %inner.iv.next, 2048
  br i1 %inner.bottom.test, label %bb2, label %inner.loop.header

bb2:
  br label %outer.loop.latch

bb1:
  br label %outer.loop.latch

outer.loop.latch:
  %outer.iv.next = add nsw i32 %outer.iv, 1
  %outer.bottom.test = icmp eq i32 %outer.iv.next, 2048
  br i1 %outer.bottom.test, label %loop.exit, label %outer.loop.header

loop.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
