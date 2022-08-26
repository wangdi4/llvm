; Original implementation of the feature required the pointer to point to a
; struct. That's unnecessary and impossible to check for with opaque pointers.
; This test verifies the latter scenario works.

; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:      -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -opaque-pointers \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       %arr.base.cast = ptrtoint.ptr.i64(&((%t4)[0]));
; CHECK-NEXT:       %alignment = %arr.base.cast  &  31;
; CHECK-NEXT:       %peel.factor = 32  -  %alignment;
; CHECK-NEXT:       %peel.factor1 = %peel.factor  >>  3;
; CHECK-NEXT:       %peel.factor1 = (77 <= %peel.factor1) ? 77 : %peel.factor1;
; CHECK-NEXT:       if (%peel.factor1 != 0)
; CHECK-NEXT:       {
; CHECK-NEXT:          + DO i1 = 0, %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 31>
; CHECK-NEXT:          |   if ((%t4)[i1] == &((%t1)[0]))
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      %gep = &((%t4)[i1]);
; CHECK-NEXT:          |      goto found;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:       }
; CHECK-NEXT:       if (%peel.factor1 <u 77)
; CHECK-NEXT:       {
; CHECK-NEXT:          %tgu = (-1 * %peel.factor1 + 77)/u4;
; CHECK-NEXT:          if (0 <u 4 * %tgu)
; CHECK-NEXT:          {
; CHECK-NEXT:             + DO i1 = 0, 4 * %tgu + -1, 4   <DO_MULTI_EXIT_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:             |   %wide.cmp. = (<4 x ptr>*)(%t4)[i1 + %peel.factor1] == &((<4 x ptr>)(%t1)[0]);
; CHECK-NEXT:             |   %intmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CHECK-NEXT:             |   if (%intmask != 0)
; CHECK-NEXT:             |   {
; CHECK-NEXT:             |      %.vec = %wide.cmp.  ^  -1;
; CHECK-NEXT:             |      %bsfintmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CHECK-NEXT:             |      %bsf = @llvm.cttz.i4(%bsfintmask,  1);
; CHECK-NEXT:             |      %cast = zext.i4.i64(%bsf);
; CHECK-NEXT:             |      %gep = &((%t4)[i1 + %peel.factor1 + %cast]);
; CHECK-NEXT:             |      goto found;
; CHECK-NEXT:             |   }
; CHECK-NEXT:             + END LOOP
; CHECK-NEXT:          }
; CHECK:               + DO i1 = 4 * %tgu, -1 * %peel.factor1 + 76, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3> <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:          |   if ((%t4)[i1 + %peel.factor1] == &((%t1)[0]))
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      %gep = &((%t4)[i1 + %peel.factor1]);
; CHECK-NEXT:          |      goto found;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:       }
; CHECK-NEXT: END REGION

define ptr @foo(ptr %t4, ptr %t1) {
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %latch]
  %gep = getelementptr ptr, ptr%t4, i64 %iv
  %ptr = load ptr, ptr %gep
  %cmp.found = icmp eq ptr %ptr, %t1
  br i1 %cmp.found, label %found, label %latch

latch:
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 77
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
