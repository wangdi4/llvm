; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; Test src:

; int x = 0;
; int a[10];
; int p[10];
;
; void foo() {
; #pragma omp simd reduction(inscan,+: x)
;   for (int i = 0; i < 10; i++) {
;     p[i] = x;
; #pragma omp scan exclusive(x)
;     x += a[i];
;   }
; }

; The IR is a hand-modified version of the above test without scan/incan.

; CHECK: BEGIN SIMD ID=1 {
; ...
; CHECK:   REDUCTION-INSCAN maps: (1: EXCLUSIVE)
; CHECK:   REDUCTION clause (size=1): (ADD: i32* @x, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1) INSCAN<1>)
; CHECK:   LINEAR clause (size=1): IV(i32* %i, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 1)
; ...
; CHECK:   BEGIN GUARD.MEM.MOTION ID=2 {
; ...
; CHECK:     BEGIN SCAN ID=3 {
; ...
; CHECK:      INSCAN-REDUCTION maps: (1: ADD)
; CHECK:      INCLUSIVE clause: UNSPECIFIED
; CHECK:      EXCLUSIVE clause (size=1): (i32* @x INSCAN<1>)
; ...
; CHECK:     } END SCAN ID=3
;
; CHECK:  } END GUARD.MEM.MOTION ID=2
;
; CHECK: } END SIMD ID=1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [10 x i32] zeroinitializer, align 16
@p = dso_local global [10 x i32] zeroinitializer, align 16
@x = dso_local global i32 0, align 4

define void @foo() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  store i32 9, i32* %.omp.ub, align 4

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
  "QUAL.OMP.REDUCTION.ADD:TYPED.INSCAN"(i32* @x, i32 0, i32 1, i64 1),
  "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0),
  "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0),
  "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i, i32 0, i32 1, i32 1) ]

  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
  "QUAL.OMP.LIVEIN"(i32* @x) ]

  %5 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5)
  %6 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %7 = load i32, i32* @x, align 4
  %8 = load i32, i32* %i, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @p, i64 0, i64 %idxprom
  store i32 %7, i32* %arrayidx, align 4

  %scan = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(i32* @x, i64 1) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %scan) [ "DIR.OMP.END.SCAN"() ]

  %9 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %9 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @a, i64 0, i64 %idxprom1
  %10 = load i32, i32* %arrayidx2, align 4
  %11 = load i32, i32* @x, align 4
  %add3 = add nsw i32 %11, %10
  store i32 %add3, i32* @x, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32* %.omp.iv, align 4

  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]

  %add4 = add nsw i32 %13, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !0

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  %14 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14)
  %15 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.enable", i1 true}
