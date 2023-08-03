; INTEL_FEATURE_CSA
; RUN: opt -passes="hir-ssa-deconstruction,hir-recognize-par-loop,hir-cg" -S < %s | FileCheck %s
;
; REQUIRES: csa-registered-target
;
; Verify that CG generates CSA parallel region/section entry/exit calls for
; a loop annotated by "OMP.PARALLEL.LOOP" directive.

target triple = "csa"

; CHECK-LABEL: @foo
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])

define void @foo(i64 %n, ptr %ip) {
entry:
  %ip.addr = alloca ptr, align 8
  %.omp.lb = alloca i32, align 4
  %i = alloca i32, align 4
  %n.addr.sroa.0.0.extract.trunc = trunc i64 %n to i32
  store ptr %ip, ptr %ip.addr, align 8
  %cmp = icmp sgt i32 %n.addr.sroa.0.0.extract.trunc, 0
  %0 = bitcast ptr %.omp.lb to ptr
  br i1 %cmp, label %DIR.OMP.PARALLEL.LOOP.112, label %omp.precond.end

DIR.OMP.PARALLEL.LOOP.112:
  store i32 0, ptr %.omp.lb, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"() ]
  %2 = load i32, ptr %.omp.lb, align 4
  %cmp413 = icmp slt i32 %2, %n.addr.sroa.0.0.extract.trunc
  br i1 %cmp413, label %omp.inner.for.body.preheader, label %omp.loop.exit

omp.inner.for.body.preheader:
  %3 = bitcast ptr %i to ptr
  %4 = load ptr, ptr %ip.addr, align 8
  %5 = sext i32 %2 to i64
  %sub2 = shl i64 %n, 32
  %sext = add i64 %sub2, -4294967296
  %6 = ashr exact i64 %sext, 32
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ %5, %omp.inner.for.body.preheader ], [ %indvars.iv.next, %omp.inner.for.body ]
  %7 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds i32, ptr %4, i64 %indvars.iv
  store i32 %7, ptr %arrayidx, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp4 = icmp slt i64 %indvars.iv, %6
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit.loopexit

omp.loop.exit.loopexit:
  br label %omp.loop.exit

omp.loop.exit:
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_FEATURE_CSA
