; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -dtrans-inline-heuristics -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -dtrans-inline-heuristics -inline-report=0xe886 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = call fastcc i32 @foo()
  ret i32 %call
}

define internal fastcc i32 @foo() unnamed_addr {
DIR.OMP.PARALLEL.LOOP.31:
  %a.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr @a, ptr %a.addr, align 8
  %i1 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i1)
  %i.addr = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  %a.addr.addr = alloca ptr, align 8
  %i2 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i2)
  %i3 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i3)
  store i32 0, ptr %.omp.lb, align 4
  %i4 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i4)
  store volatile i32 99, ptr %.omp.ub, align 4
  store ptr %i, ptr %i.addr, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr, align 8
  store ptr %a.addr, ptr %a.addr.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %i5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(ptr %a.addr), "QUAL.OMP.PRIVATE"(ptr %i), "QUAL.OMP.NORMALIZED.IV"(ptr %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(ptr %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(ptr %.omp.ub), "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a.addr, ptr %a.addr.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.31
  %i8 = load volatile ptr, ptr %i.addr, align 8
  %.omp.lb9 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %a.addr10 = load volatile ptr, ptr %a.addr.addr, align 8
  %i6 = load i32, ptr %.omp.lb9, align 4
  store volatile i32 %i6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.3
  %i7 = load volatile i32, ptr %.omp.iv, align 4
  %i9 = load volatile i32, ptr %.omp.ub, align 4
  %cmp3.not = icmp sgt i32 %i7, %i9
  br i1 %cmp3.not, label %omp.precond.end, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i10 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %i10, ptr %i8, align 4
  %i11 = load ptr, ptr %a.addr10, align 8
  %idxprom = sext i32 %i10 to i64
  %ptridx = getelementptr inbounds i32, ptr %i11, i64 %idxprom
  store i32 15, ptr %ptridx, align 4
  %i12 = load volatile i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %i12, 1
  store volatile i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.precond.end:                                  ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.31
  call void @llvm.directive.region.exit(token %i5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %i13 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i13)
  %i14 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i14)
  %i15 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i15)
  %i16 = load ptr, ptr %a.addr, align 8
  %i17 = load i32, ptr %i16, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i1)
  ret i32 %i17
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
; end INTEL_FEATURE_SW_ADVANCED
