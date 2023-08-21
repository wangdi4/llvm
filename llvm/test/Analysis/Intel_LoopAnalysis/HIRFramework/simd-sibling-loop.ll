; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the SIMD loop is marked correctly, and its
; sibling loop is not marked as SIMD.

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:       |   |   if (i1 != 20)
; CHECK:       |   |   {
; CHECK:       |   |      (%a)[i2] = i2;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   if (%n != 20)
; CHECK:       |   |   {
; CHECK:       |   |      (%a)[i2] = i2;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %a, i32 %n) {
header:
  br label %outer.for.body

outer.for.body:
  %indvars.iv.out = phi i64 [ %indvars.iv.out.next, %outer.for.inc ], [ 0, %header ]
  br label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %cmp1 = icmp eq i64 %indvars.iv.out, 20
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  br i1 %cmp1, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %sibloop.header

sibloop.header:
  %cmp2 = icmp eq i32 %n, 20
  br label %inner.for.body

inner.for.body:                               ; preds = %inner.for.inc, %sibloop.header
  %indvars.iv.sib = phi i64 [ %indvars.iv.sib.next, %inner.for.inc ], [ 0, %sibloop.header ]
  br i1 %cmp2, label %inner.for.inc, label %if.sib.then

if.sib.then:                                          ; preds = %inner.for.body
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv.sib
  %2 = trunc i64 %indvars.iv.sib to i32
  store i32 %2, ptr %arrayidx2
  br label %inner.for.inc

inner.for.inc:                                ; preds = %inner.for.body, %if.sib.then
  %indvars.iv.sib.next = add nuw nsw i64 %indvars.iv.sib, 1
  %exitcond.sib = icmp eq i64 %indvars.iv.sib.next, 100
  br i1 %exitcond.sib, label %loop.end, label %inner.for.body

loop.end:                               ; preds = %inner.for.inc
  br label %outer.for.inc

outer.for.inc:
  %indvars.iv.out.next = add nuw nsw i64 %indvars.iv.out, 1
  %exitcond.out = icmp eq i64 %indvars.iv.out.next, 100
  br i1 %exitcond.out, label %exit, label %outer.for.body

exit:
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
