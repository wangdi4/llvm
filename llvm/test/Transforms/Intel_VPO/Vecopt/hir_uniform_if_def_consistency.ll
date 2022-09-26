; Test to check that uniform/external-if conditions hoisted out of vectorized
; loops have consistent def@level for attached nesting level.

; Input HIR
; BEGIN REGION { }
;       + DO i1 = 0, 199, 1   <DO_LOOP>
;       |   %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;       |   %control.fetch = (@control)[0];
;       |
;       |   + DO i2 = 0, 99, 1   <DO_LOOP> <simd>
;       |   |   if (trunc.i32.i1(%control.fetch) == 0)
;       |   |   <RVAL-REG> LINEAR zext.i1.i32(trunc.i32.i1(%control.fetch)){def@1} {sb:2}
;       |   |      <BLOB> LINEAR i32 %control.fetch{def@1} {sb:6}
;       |   |   {
;       |   |      (%arr1)[i2] = i2;
;       |   |   }
;       |   |   else
;       |   |   {
;       |   |      (%arr2)[i2] = i2;
;       |   |   }
;       |   + END LOOP
;       |
;       |   @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
;       + END LOOP
; END REGION

; When the CanonExpr "zext.i1.i32(trunc.i32.i1(%control.fetch))" is hoisted out of
; i2 loop and attached to i1 loop, it will not be LINEAR anymore.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -print-after=hir-vplan-vec -hir-details -vplan-force-vf=4 -vplan-force-linearization-hir < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -hir-details -vplan-force-vf=4 -vplan-force-linearization-hir < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@control = dso_local local_unnamed_addr global i32 0, align 8

define dso_local void @foo(i64* nocapture readnone %arr1, i64* nocapture readnone %arr2) local_unnamed_addr #0 {
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i64 i1 = 0, 199, 1   <DO_LOOP>
; CHECK:          |   %control.fetch = (@control)[0];
; CHECK:          |   %cmp = trunc.i32.i1(%control.fetch) == 0;
; CHECK:          |   <LVAL-REG> NON-LINEAR i1 %cmp {sb:19}
; CHECK:          |   <RVAL-REG> NON-LINEAR zext.i1.i32(trunc.i32.i1(%control.fetch)) {sb:2}
; CHECK:          |      <BLOB> NON-LINEAR i32 %control.fetch {sb:6}
; CHECK:          |   %.vec = %cmp  ^  -1;
; CHECK:          |
; CHECK:          |   + DO i64 i2 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK:          |   |   (<4 x i64>*)(%arr2)[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{%.vec};
; CHECK:          |   |   (<4 x i64>*)(%arr1)[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{%cmp};
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:    END REGION

entry:
  br label %outer.header

outer.header:
  %outer.iv = phi i64 [ 0, %entry ], [ %outer.iv.next, %outer.latch ]
  br label %simd.start

simd.start:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %inner.ph

inner.ph:
  %control.fetch = load i32, i32* @control, align 8
  %and = and i32 %control.fetch, 1
  %uni.cond = icmp eq i32 %and, 0
  br label %inner.header

inner.header:
  %inner.iv = phi i64 [ 0, %inner.ph ], [ %inner.iv.next, %inner.latch ]
  br i1 %uni.cond, label %if.then, label %if.else

if.then:
  %gep1 = getelementptr inbounds i64, i64* %arr1, i64 %inner.iv
  store i64 %inner.iv, i64* %gep1, align 8
  br label %inner.latch

if.else:
  %gep2 = getelementptr inbounds i64, i64* %arr2, i64 %inner.iv
  store i64 %inner.iv, i64* %gep2, align 8
  br label %inner.latch

inner.latch:
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  %inner.cond = icmp eq i64 %inner.iv.next, 100
  br i1 %inner.cond, label %outer.latch, label %inner.header

outer.latch:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %outer.cond = icmp eq i64 %outer.iv.next, 200
  br i1 %outer.cond, label %exit, label %outer.header

exit:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
