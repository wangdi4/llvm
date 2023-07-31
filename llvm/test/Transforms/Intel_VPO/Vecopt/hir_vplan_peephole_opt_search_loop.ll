; Test to verify that peephole optimization (trunc + zext -> and) is
; disabled on search loops.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -xmain-opt-level=3 -vplan-print-after-initial-transforms -vplan-print-after-predicator -debug-only=vplan-idioms -disable-output < %s 2>&1 | FileCheck %s

; REQUIRES: asserts

; HIR before VPlan-
;  BEGIN REGION { modified }
;        + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
;        |   if ((%t4)[i1] == &((%t1)[0]))
;        |   {
;        |      %gep = &((%t4)[i1]);
;        |      goto found;
;        |   }
;        + END LOOP
;  END REGION

; CHECK: VPlan after initial VPlan transforms:
; CHECK:        i64 [[IV:%.*]] = phi  [ i64 0, {{.*}} ],  [ i64 [[IV_NEXT:%.*]], {{.*}} ]
; CHECK-NEXT:   i32 [[IV_TRUNC:%.*]] = trunc i64 [[IV]] to i32
; CHECK-NEXT:   i64 [[IV_ZEXT:%.*]] = zext i32 [[IV_TRUNC]] to i64

; CHECK: VPlan after predicator:
; CHECK:        i64 [[IV:%.*]] = phi  [ i64 0, {{.*}} ],  [ i64 [[IV_NEXT:%.*]], {{.*}} ]
; CHECK-NEXT:   i32 [[IV_TRUNC:%.*]] = trunc i64 [[IV]] to i32
; CHECK-NEXT:   i64 [[IV_ZEXT:%.*]] = zext i32 [[IV_TRUNC]] to i64

; CHECK: PtrEq loop has PeelArray:(%t4)[i1]
; CHECK: Search loop was recognized.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct2 = type <{ i32, [4 x i8] }>

; Function Attrs: uwtable
define ptr @foo(ptr %t4, ptr %t1, i64 %n) {
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %latch]
  %iv.prom = and i64 %iv, 4294967295
  %gep = getelementptr ptr, ptr %t4, i64 %iv.prom
  %ptr = load ptr, ptr %gep
  %cmp.found = icmp eq ptr %ptr, %t1
  br i1 %cmp.found, label %found, label %latch

latch:
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %n
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  br label %exit

found:
  %lcssa = phi ptr [ %gep , %header ]
  br label %exit

exit:
  %val = phi ptr [ %lcssa, %found ], [ zeroinitializer, %loop.exit ]
  ret ptr %val
}
