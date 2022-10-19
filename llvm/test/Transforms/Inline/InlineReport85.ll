; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

target triple = "x86_64-unknown-linux-gnu"

; Check that foo is inlined even though it has an alloca in a block which is
; not an entry block, but will be after OpenMP outlining.

; CHECK-CL-NOT: call{{.*}}@foo()
; CHECK-CL: DEAD STATIC FUNC: foo
; CHECK: COMPILE FUNC: main
; CHECK: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK-MD: DEAD STATIC FUNC: foo
; CHECK-MD-NOT: call{{.*}}@foo()

@a = dso_local global [100 x i32] zeroinitializer, align 16
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = call fastcc i32 @foo()
  ret i32 %call
}

define internal fastcc i32 @foo() unnamed_addr {
DIR.OMP.PARALLEL.LOOP.31:
  %a.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 0), i32** %a.addr, align 8
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  %i.addr = alloca i32*, align 8
  %.omp.lb.addr = alloca i32*, align 8
  %a.addr.addr = alloca i32**, align 8
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2)
  store i32 0, i32* %.omp.lb, align 4
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  store volatile i32 99, i32* %.omp.ub, align 4
  store i32* %i, i32** %i.addr, align 8
  store i32* %.omp.lb, i32** %.omp.lb.addr, align 8
  store i32** %a.addr, i32*** %a.addr.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32** %a.addr), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %a.addr, i32*** %a.addr.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.31
  %i8 = load volatile i32*, i32** %i.addr, align 8
  %.omp.lb9 = load volatile i32*, i32** %.omp.lb.addr, align 8
  %a.addr10 = load volatile i32**, i32*** %a.addr.addr, align 8
  %5 = load i32, i32* %.omp.lb9, align 4
  store volatile i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.3
  %6 = load volatile i32, i32* %.omp.iv, align 4
  %7 = load volatile i32, i32* %.omp.ub, align 4
  %cmp3.not = icmp sgt i32 %6, %7
  br i1 %cmp3.not, label %omp.precond.end, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, i32* %.omp.iv, align 4
  store i32 %8, i32* %i8, align 4
  %9 = load i32*, i32** %a.addr10, align 8
  %idxprom = sext i32 %8 to i64
  %ptridx = getelementptr inbounds i32, i32* %9, i64 %idxprom
  store i32 15, i32* %ptridx, align 4
  %10 = load volatile i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %10, 1
  store volatile i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.precond.end:                                  ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.31
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %11 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %11)
  %12 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12)
  %13 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13)
  %14 = load i32*, i32** %a.addr, align 8
  %15 = load i32, i32* %14, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret i32 %15
}
; end INTEL_FEATURE_SW_ADVANCED
