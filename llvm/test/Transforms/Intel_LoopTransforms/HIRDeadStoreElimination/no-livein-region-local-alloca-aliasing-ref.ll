; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that load/store of (%A)[0][0]/(%A)[0][%idx] are not eliminated even
; though %A is identified as a region local alloca. The two groups alias with
; each other preventing elimination. 

; TODO: Eliminate loads/stores of (%A)[0][0] and (%A)[0][%idx] by ignoring
; dependencies to aliasing loads which are dominated by identical stores.

; Incoming HIR-
; + DO i1 = 0, 0, 1   <DO_LOOP> <unroll>
; |   (%A)[0][%idx] = 5;
; |   %ld = (%A)[0][%idx];
; |   (%A)[0][i1] = %ld + %t;
; |   %ld1 = (%A)[0][i1];
; + END LOOP


; CHECK: Dump Before

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT: (%A)[0][%idx] = 5;
; CHECK-NEXT: %ld = (%A)[0][%idx];
; CHECK-NEXT: (%A)[0][0] = %ld + %t;
; CHECK-NEXT: %ld1 = (%A)[0][0];
; CHECK-NEXT: END REGION


; CHECK: Dump After

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT: (%A)[0][%idx] = 5;
; CHECK-NEXT: %ld = (%A)[0][%idx];
; CHECK-NEXT: (%A)[0][0] = %ld + %t;
; CHECK-NEXT: %ld1 = (%A)[0][0];
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t, i64 %idx, ptr %p) {
entry:
  %A = alloca [10 x i32], align 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 %A, i8 0, i64 40, i1 false)
  br label %outer.loop

outer.loop:
  %iv.outer = phi i64 [ 0, %entry], [ %iv.outer.inc, %latch ]
  br label %loop

loop:
  %iv = phi i64 [ 0, %outer.loop], [ %iv.inc, %loop]
  %gepidx = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %idx
  store i32 5, ptr %gepidx, align 4
  %ld = load i32, ptr %gepidx, align 4
  %add = add i32 %ld, %t
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  store i32 %add, ptr %gep, align 4
  %ld1 = load i32, ptr %gep, align 4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 1
  br i1 %cmp, label %latch, label %loop, !llvm.loop !0

latch:
  %iv.outer.inc = add i64 %iv.outer, 1
  %pld = load i64, ptr %p
  %cmp1 = icmp eq i64 %iv.outer.inc, %pld
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi i32 [ %ld1, %latch ]
  ret i32 %ld.lcssa
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) 

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}

