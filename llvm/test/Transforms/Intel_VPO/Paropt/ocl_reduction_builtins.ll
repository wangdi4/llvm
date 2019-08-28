; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload  -S | FileCheck %s

; Original code (see at the end of the file).

; Signed add:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_addi(i32
; CHECK-DAG: void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_addl(i64

; Unsigned add:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_addi(i32
; CHECK-DAG: void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_addl(i64

; FP add:
; CHECK-DAG: call void @__kmpc_atomic_float4_add(float addrspace(4)* {{.*}}, float
; CHECK-DAG: call double @_Z20sub_group_reduce_addd(double

; Signed min:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_mini(i32
; CHECK-DAG: call i32 @_Z20sub_group_reduce_mini(i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_minl(i64

; Unsigned min:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_minj(i32
; CHECK-DAG: call i32 @_Z20sub_group_reduce_minj(i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_minm(i64

; FP min:
; CHECK-DAG: call float @_Z20sub_group_reduce_minf(float
; CHECK-DAG: call double @_Z20sub_group_reduce_mind(double

; Signed max:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_maxi(i32
; CHECK-DAG: call i32 @_Z20sub_group_reduce_maxi(i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_maxl(i64

; Unsigned max:
; CHECK-DAG: call i32 @_Z20sub_group_reduce_maxj(i32
; CHECK-DAG: call i32 @_Z20sub_group_reduce_maxj(i32
; CHECK-DAG: call i64 @_Z20sub_group_reduce_maxm(i64

; FP max:
; CHECK-DAG: call float @_Z20sub_group_reduce_maxf(float
; CHECK-DAG: call double @_Z20sub_group_reduce_maxd(double

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo(i32 addrspace(4)* %x) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %0 = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %1 = addrspacecast i32* %i to i32 addrspace(4)*
  %sadd16 = alloca i16, align 2
  %2 = addrspacecast i16* %sadd16 to i16 addrspace(4)*
  %smin16 = alloca i16, align 2
  %3 = addrspacecast i16* %smin16 to i16 addrspace(4)*
  %smax16 = alloca i16, align 2
  %4 = addrspacecast i16* %smax16 to i16 addrspace(4)*
  %sadd32 = alloca i32, align 4
  %5 = addrspacecast i32* %sadd32 to i32 addrspace(4)*
  %smin32 = alloca i32, align 4
  %6 = addrspacecast i32* %smin32 to i32 addrspace(4)*
  %smax32 = alloca i32, align 4
  %7 = addrspacecast i32* %smax32 to i32 addrspace(4)*
  %sadd64 = alloca i64, align 8
  %8 = addrspacecast i64* %sadd64 to i64 addrspace(4)*
  %smin64 = alloca i64, align 8
  %9 = addrspacecast i64* %smin64 to i64 addrspace(4)*
  %smax64 = alloca i64, align 8
  %10 = addrspacecast i64* %smax64 to i64 addrspace(4)*
  %uadd16 = alloca i16, align 2
  %11 = addrspacecast i16* %uadd16 to i16 addrspace(4)*
  %umin16 = alloca i16, align 2
  %12 = addrspacecast i16* %umin16 to i16 addrspace(4)*
  %umax16 = alloca i16, align 2
  %13 = addrspacecast i16* %umax16 to i16 addrspace(4)*
  %uadd32 = alloca i32, align 4
  %14 = addrspacecast i32* %uadd32 to i32 addrspace(4)*
  %umin32 = alloca i32, align 4
  %15 = addrspacecast i32* %umin32 to i32 addrspace(4)*
  %umax32 = alloca i32, align 4
  %16 = addrspacecast i32* %umax32 to i32 addrspace(4)*
  %uadd64 = alloca i64, align 8
  %17 = addrspacecast i64* %uadd64 to i64 addrspace(4)*
  %umin64 = alloca i64, align 8
  %18 = addrspacecast i64* %umin64 to i64 addrspace(4)*
  %umax64 = alloca i64, align 8
  %19 = addrspacecast i64* %umax64 to i64 addrspace(4)*
  %fadd32 = alloca float, align 4
  %20 = addrspacecast float* %fadd32 to float addrspace(4)*
  %fmin32 = alloca float, align 4
  %21 = addrspacecast float* %fmin32 to float addrspace(4)*
  %fmax32 = alloca float, align 4
  %22 = addrspacecast float* %fmax32 to float addrspace(4)*
  %fadd64 = alloca double, align 8
  %23 = addrspacecast double* %fadd64 to double addrspace(4)*
  %fmin64 = alloca double, align 8
  %24 = addrspacecast double* %fmin64 to double addrspace(4)*
  %fmax64 = alloca double, align 8
  %25 = addrspacecast double* %fmax64 to double addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %26 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %27 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %28 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %29 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %0, align 8
  store i16 0, i16 addrspace(4)* %2, align 2
  store i16 200, i16 addrspace(4)* %3, align 2
  store i16 0, i16 addrspace(4)* %4, align 2
  store i32 0, i32 addrspace(4)* %5, align 4
  store i32 200, i32 addrspace(4)* %6, align 4
  store i32 0, i32 addrspace(4)* %7, align 4
  store i64 0, i64 addrspace(4)* %8, align 8
  store i64 200, i64 addrspace(4)* %9, align 8
  store i64 0, i64 addrspace(4)* %10, align 8
  store i16 0, i16 addrspace(4)* %11, align 2
  store i16 200, i16 addrspace(4)* %12, align 2
  store i16 0, i16 addrspace(4)* %13, align 2
  store i32 0, i32 addrspace(4)* %14, align 4
  store i32 200, i32 addrspace(4)* %15, align 4
  store i32 0, i32 addrspace(4)* %16, align 4
  store i64 0, i64 addrspace(4)* %17, align 8
  store i64 200, i64 addrspace(4)* %18, align 8
  store i64 0, i64 addrspace(4)* %19, align 8
  store float 0.000000e+00, float addrspace(4)* %20, align 4
  store float 2.000000e+02, float addrspace(4)* %21, align 4
  store float 0.000000e+00, float addrspace(4)* %22, align 4
  store double 0.000000e+00, double addrspace(4)* %23, align 8
  store double 2.000000e+02, double addrspace(4)* %24, align 8
  store double 0.000000e+00, double addrspace(4)* %25, align 8
  %30 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %30, i64 0
  %31 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %2), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %5), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %8), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %11), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %14), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %17), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %20), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %23), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %3), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %6), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %9), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %12), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %15), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %18), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %21), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %24), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %4), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %7), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %10), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %13), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %16), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %19), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %22), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %25), "QUAL.OMP.MAP.TO:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %0, i64 8), "QUAL.OMP.MAP.TO:AGGR"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* %arrayidx, i64 400), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %28), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %29), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %26), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %27) ]
  store i32 0, i32 addrspace(4)* %28, align 4
  store i32 99, i32 addrspace(4)* %29, align 4
  %32 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i16 addrspace(4)* %2), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %5), "QUAL.OMP.REDUCTION.ADD"(i64 addrspace(4)* %8), "QUAL.OMP.REDUCTION.ADD"(i16 addrspace(4)* %11), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %14), "QUAL.OMP.REDUCTION.ADD"(i64 addrspace(4)* %17), "QUAL.OMP.REDUCTION.ADD"(float addrspace(4)* %20), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %23), "QUAL.OMP.REDUCTION.MIN"(i16 addrspace(4)* %3), "QUAL.OMP.REDUCTION.MIN"(i32 addrspace(4)* %6), "QUAL.OMP.REDUCTION.MIN"(i64 addrspace(4)* %9), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i16 addrspace(4)* %12), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i32 addrspace(4)* %15), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i64 addrspace(4)* %18), "QUAL.OMP.REDUCTION.MIN"(float addrspace(4)* %21), "QUAL.OMP.REDUCTION.MIN"(double addrspace(4)* %24), "QUAL.OMP.REDUCTION.MAX"(i16 addrspace(4)* %4), "QUAL.OMP.REDUCTION.MAX"(i32 addrspace(4)* %7), "QUAL.OMP.REDUCTION.MAX"(i64 addrspace(4)* %10), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i16 addrspace(4)* %13), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i32 addrspace(4)* %16), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i64 addrspace(4)* %19), "QUAL.OMP.REDUCTION.MAX"(float addrspace(4)* %22), "QUAL.OMP.REDUCTION.MAX"(double addrspace(4)* %25), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %28), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %26), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %29), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %0) ]
  %33 = load i32, i32 addrspace(4)* %28, align 4
  store i32 %33, i32 addrspace(4)* %26, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %34 = load i32, i32 addrspace(4)* %26, align 4
  %35 = load i32, i32 addrspace(4)* %29, align 4
  %cmp = icmp sle i32 %34, %35
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %36 = load i32, i32 addrspace(4)* %26, align 4
  %mul = mul nsw i32 %36, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %1, align 4
  %37 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %38 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom = sext i32 %38 to i64
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %37, i64 %idxprom
  %39 = load i32, i32 addrspace(4)* %arrayidx1, align 4
  %40 = load i16, i16 addrspace(4)* %2, align 2
  %conv = sext i16 %40 to i32
  %add2 = add nsw i32 %conv, %39
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, i16 addrspace(4)* %2, align 2
  %41 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %42 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom4 = sext i32 %42 to i64
  %arrayidx5 = getelementptr inbounds i32, i32 addrspace(4)* %41, i64 %idxprom4
  %43 = load i32, i32 addrspace(4)* %arrayidx5, align 4
  %44 = load i32, i32 addrspace(4)* %5, align 4
  %add6 = add nsw i32 %44, %43
  store i32 %add6, i32 addrspace(4)* %5, align 4
  %45 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %46 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom7 = sext i32 %46 to i64
  %arrayidx8 = getelementptr inbounds i32, i32 addrspace(4)* %45, i64 %idxprom7
  %47 = load i32, i32 addrspace(4)* %arrayidx8, align 4
  %conv9 = sext i32 %47 to i64
  %48 = load i64, i64 addrspace(4)* %8, align 8
  %add10 = add nsw i64 %48, %conv9
  store i64 %add10, i64 addrspace(4)* %8, align 8
  %49 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %50 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom11 = sext i32 %50 to i64
  %arrayidx12 = getelementptr inbounds i32, i32 addrspace(4)* %49, i64 %idxprom11
  %51 = load i32, i32 addrspace(4)* %arrayidx12, align 4
  %52 = load i16, i16 addrspace(4)* %11, align 2
  %conv13 = zext i16 %52 to i32
  %add14 = add nsw i32 %conv13, %51
  %conv15 = trunc i32 %add14 to i16
  store i16 %conv15, i16 addrspace(4)* %11, align 2
  %53 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %54 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom16 = sext i32 %54 to i64
  %arrayidx17 = getelementptr inbounds i32, i32 addrspace(4)* %53, i64 %idxprom16
  %55 = load i32, i32 addrspace(4)* %arrayidx17, align 4
  %56 = load i32, i32 addrspace(4)* %14, align 4
  %add18 = add i32 %56, %55
  store i32 %add18, i32 addrspace(4)* %14, align 4
  %57 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %58 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom19 = sext i32 %58 to i64
  %arrayidx20 = getelementptr inbounds i32, i32 addrspace(4)* %57, i64 %idxprom19
  %59 = load i32, i32 addrspace(4)* %arrayidx20, align 4
  %conv21 = sext i32 %59 to i64
  %60 = load i64, i64 addrspace(4)* %17, align 8
  %add22 = add i64 %60, %conv21
  store i64 %add22, i64 addrspace(4)* %17, align 8
  %61 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %62 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom23 = sext i32 %62 to i64
  %arrayidx24 = getelementptr inbounds i32, i32 addrspace(4)* %61, i64 %idxprom23
  %63 = load i32, i32 addrspace(4)* %arrayidx24, align 4
  %conv25 = sitofp i32 %63 to float
  %64 = load float, float addrspace(4)* %20, align 4
  %add26 = fadd float %64, %conv25
  store float %add26, float addrspace(4)* %20, align 4
  %65 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %66 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom27 = sext i32 %66 to i64
  %arrayidx28 = getelementptr inbounds i32, i32 addrspace(4)* %65, i64 %idxprom27
  %67 = load i32, i32 addrspace(4)* %arrayidx28, align 4
  %conv29 = sitofp i32 %67 to double
  %68 = load double, double addrspace(4)* %23, align 8
  %add30 = fadd double %68, %conv29
  store double %add30, double addrspace(4)* %23, align 8
  %69 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %70 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom31 = sext i32 %70 to i64
  %arrayidx32 = getelementptr inbounds i32, i32 addrspace(4)* %69, i64 %idxprom31
  %71 = load i32, i32 addrspace(4)* %arrayidx32, align 4
  %72 = load i16, i16 addrspace(4)* %3, align 2
  %conv33 = sext i16 %72 to i32
  %cmp34 = icmp slt i32 %71, %conv33
  br i1 %cmp34, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  %73 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %74 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom36 = sext i32 %74 to i64
  %arrayidx37 = getelementptr inbounds i32, i32 addrspace(4)* %73, i64 %idxprom36
  %75 = load i32, i32 addrspace(4)* %arrayidx37, align 4
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  %76 = load i16, i16 addrspace(4)* %3, align 2
  %conv38 = sext i16 %76 to i32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %75, %cond.true ], [ %conv38, %cond.false ]
  %conv39 = trunc i32 %cond to i16
  store i16 %conv39, i16 addrspace(4)* %3, align 2
  %77 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %78 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom40 = sext i32 %78 to i64
  %arrayidx41 = getelementptr inbounds i32, i32 addrspace(4)* %77, i64 %idxprom40
  %79 = load i32, i32 addrspace(4)* %arrayidx41, align 4
  %80 = load i32, i32 addrspace(4)* %6, align 4
  %cmp42 = icmp slt i32 %79, %80
  br i1 %cmp42, label %cond.true44, label %cond.false47

cond.true44:                                      ; preds = %cond.end
  %81 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %82 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom45 = sext i32 %82 to i64
  %arrayidx46 = getelementptr inbounds i32, i32 addrspace(4)* %81, i64 %idxprom45
  %83 = load i32, i32 addrspace(4)* %arrayidx46, align 4
  br label %cond.end48

cond.false47:                                     ; preds = %cond.end
  %84 = load i32, i32 addrspace(4)* %6, align 4
  br label %cond.end48

cond.end48:                                       ; preds = %cond.false47, %cond.true44
  %cond49 = phi i32 [ %83, %cond.true44 ], [ %84, %cond.false47 ]
  store i32 %cond49, i32 addrspace(4)* %6, align 4
  %85 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %86 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom50 = sext i32 %86 to i64
  %arrayidx51 = getelementptr inbounds i32, i32 addrspace(4)* %85, i64 %idxprom50
  %87 = load i32, i32 addrspace(4)* %arrayidx51, align 4
  %conv52 = sext i32 %87 to i64
  %88 = load i64, i64 addrspace(4)* %9, align 8
  %cmp53 = icmp slt i64 %conv52, %88
  br i1 %cmp53, label %cond.true55, label %cond.false59

cond.true55:                                      ; preds = %cond.end48
  %89 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %90 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom56 = sext i32 %90 to i64
  %arrayidx57 = getelementptr inbounds i32, i32 addrspace(4)* %89, i64 %idxprom56
  %91 = load i32, i32 addrspace(4)* %arrayidx57, align 4
  %conv58 = sext i32 %91 to i64
  br label %cond.end60

cond.false59:                                     ; preds = %cond.end48
  %92 = load i64, i64 addrspace(4)* %9, align 8
  br label %cond.end60

cond.end60:                                       ; preds = %cond.false59, %cond.true55
  %cond61 = phi i64 [ %conv58, %cond.true55 ], [ %92, %cond.false59 ]
  store i64 %cond61, i64 addrspace(4)* %9, align 8
  %93 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %94 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom62 = sext i32 %94 to i64
  %arrayidx63 = getelementptr inbounds i32, i32 addrspace(4)* %93, i64 %idxprom62
  %95 = load i32, i32 addrspace(4)* %arrayidx63, align 4
  %96 = load i16, i16 addrspace(4)* %12, align 2
  %conv64 = zext i16 %96 to i32
  %cmp65 = icmp slt i32 %95, %conv64
  br i1 %cmp65, label %cond.true67, label %cond.false70

cond.true67:                                      ; preds = %cond.end60
  %97 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %98 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom68 = sext i32 %98 to i64
  %arrayidx69 = getelementptr inbounds i32, i32 addrspace(4)* %97, i64 %idxprom68
  %99 = load i32, i32 addrspace(4)* %arrayidx69, align 4
  br label %cond.end72

cond.false70:                                     ; preds = %cond.end60
  %100 = load i16, i16 addrspace(4)* %12, align 2
  %conv71 = zext i16 %100 to i32
  br label %cond.end72

cond.end72:                                       ; preds = %cond.false70, %cond.true67
  %cond73 = phi i32 [ %99, %cond.true67 ], [ %conv71, %cond.false70 ]
  %conv74 = trunc i32 %cond73 to i16
  store i16 %conv74, i16 addrspace(4)* %12, align 2
  %101 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %102 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom75 = sext i32 %102 to i64
  %arrayidx76 = getelementptr inbounds i32, i32 addrspace(4)* %101, i64 %idxprom75
  %103 = load i32, i32 addrspace(4)* %arrayidx76, align 4
  %104 = load i32, i32 addrspace(4)* %15, align 4
  %cmp77 = icmp ult i32 %103, %104
  br i1 %cmp77, label %cond.true79, label %cond.false82

cond.true79:                                      ; preds = %cond.end72
  %105 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %106 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom80 = sext i32 %106 to i64
  %arrayidx81 = getelementptr inbounds i32, i32 addrspace(4)* %105, i64 %idxprom80
  %107 = load i32, i32 addrspace(4)* %arrayidx81, align 4
  br label %cond.end83

cond.false82:                                     ; preds = %cond.end72
  %108 = load i32, i32 addrspace(4)* %15, align 4
  br label %cond.end83

cond.end83:                                       ; preds = %cond.false82, %cond.true79
  %cond84 = phi i32 [ %107, %cond.true79 ], [ %108, %cond.false82 ]
  store i32 %cond84, i32 addrspace(4)* %15, align 4
  %109 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %110 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom85 = sext i32 %110 to i64
  %arrayidx86 = getelementptr inbounds i32, i32 addrspace(4)* %109, i64 %idxprom85
  %111 = load i32, i32 addrspace(4)* %arrayidx86, align 4
  %conv87 = sext i32 %111 to i64
  %112 = load i64, i64 addrspace(4)* %18, align 8
  %cmp88 = icmp ult i64 %conv87, %112
  br i1 %cmp88, label %cond.true90, label %cond.false94

cond.true90:                                      ; preds = %cond.end83
  %113 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %114 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom91 = sext i32 %114 to i64
  %arrayidx92 = getelementptr inbounds i32, i32 addrspace(4)* %113, i64 %idxprom91
  %115 = load i32, i32 addrspace(4)* %arrayidx92, align 4
  %conv93 = sext i32 %115 to i64
  br label %cond.end95

cond.false94:                                     ; preds = %cond.end83
  %116 = load i64, i64 addrspace(4)* %18, align 8
  br label %cond.end95

cond.end95:                                       ; preds = %cond.false94, %cond.true90
  %cond96 = phi i64 [ %conv93, %cond.true90 ], [ %116, %cond.false94 ]
  store i64 %cond96, i64 addrspace(4)* %18, align 8
  %117 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %118 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom97 = sext i32 %118 to i64
  %arrayidx98 = getelementptr inbounds i32, i32 addrspace(4)* %117, i64 %idxprom97
  %119 = load i32, i32 addrspace(4)* %arrayidx98, align 4
  %conv99 = sitofp i32 %119 to float
  %120 = load float, float addrspace(4)* %21, align 4
  %cmp100 = fcmp olt float %conv99, %120
  br i1 %cmp100, label %cond.true102, label %cond.false106

cond.true102:                                     ; preds = %cond.end95
  %121 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %122 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom103 = sext i32 %122 to i64
  %arrayidx104 = getelementptr inbounds i32, i32 addrspace(4)* %121, i64 %idxprom103
  %123 = load i32, i32 addrspace(4)* %arrayidx104, align 4
  %conv105 = sitofp i32 %123 to float
  br label %cond.end107

cond.false106:                                    ; preds = %cond.end95
  %124 = load float, float addrspace(4)* %21, align 4
  br label %cond.end107

cond.end107:                                      ; preds = %cond.false106, %cond.true102
  %cond108 = phi float [ %conv105, %cond.true102 ], [ %124, %cond.false106 ]
  store float %cond108, float addrspace(4)* %21, align 4
  %125 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %126 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom109 = sext i32 %126 to i64
  %arrayidx110 = getelementptr inbounds i32, i32 addrspace(4)* %125, i64 %idxprom109
  %127 = load i32, i32 addrspace(4)* %arrayidx110, align 4
  %conv111 = sitofp i32 %127 to double
  %128 = load double, double addrspace(4)* %24, align 8
  %cmp112 = fcmp olt double %conv111, %128
  br i1 %cmp112, label %cond.true114, label %cond.false118

cond.true114:                                     ; preds = %cond.end107
  %129 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %130 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom115 = sext i32 %130 to i64
  %arrayidx116 = getelementptr inbounds i32, i32 addrspace(4)* %129, i64 %idxprom115
  %131 = load i32, i32 addrspace(4)* %arrayidx116, align 4
  %conv117 = sitofp i32 %131 to double
  br label %cond.end119

cond.false118:                                    ; preds = %cond.end107
  %132 = load double, double addrspace(4)* %24, align 8
  br label %cond.end119

cond.end119:                                      ; preds = %cond.false118, %cond.true114
  %cond120 = phi double [ %conv117, %cond.true114 ], [ %132, %cond.false118 ]
  store double %cond120, double addrspace(4)* %24, align 8
  %133 = load i16, i16 addrspace(4)* %4, align 2
  %conv121 = sext i16 %133 to i32
  %134 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %135 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom122 = sext i32 %135 to i64
  %arrayidx123 = getelementptr inbounds i32, i32 addrspace(4)* %134, i64 %idxprom122
  %136 = load i32, i32 addrspace(4)* %arrayidx123, align 4
  %cmp124 = icmp slt i32 %conv121, %136
  br i1 %cmp124, label %cond.true126, label %cond.false129

cond.true126:                                     ; preds = %cond.end119
  %137 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %138 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom127 = sext i32 %138 to i64
  %arrayidx128 = getelementptr inbounds i32, i32 addrspace(4)* %137, i64 %idxprom127
  %139 = load i32, i32 addrspace(4)* %arrayidx128, align 4
  br label %cond.end131

cond.false129:                                    ; preds = %cond.end119
  %140 = load i16, i16 addrspace(4)* %4, align 2
  %conv130 = sext i16 %140 to i32
  br label %cond.end131

cond.end131:                                      ; preds = %cond.false129, %cond.true126
  %cond132 = phi i32 [ %139, %cond.true126 ], [ %conv130, %cond.false129 ]
  %conv133 = trunc i32 %cond132 to i16
  store i16 %conv133, i16 addrspace(4)* %4, align 2
  %141 = load i32, i32 addrspace(4)* %7, align 4
  %142 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %143 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom134 = sext i32 %143 to i64
  %arrayidx135 = getelementptr inbounds i32, i32 addrspace(4)* %142, i64 %idxprom134
  %144 = load i32, i32 addrspace(4)* %arrayidx135, align 4
  %cmp136 = icmp slt i32 %141, %144
  br i1 %cmp136, label %cond.true138, label %cond.false141

cond.true138:                                     ; preds = %cond.end131
  %145 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %146 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom139 = sext i32 %146 to i64
  %arrayidx140 = getelementptr inbounds i32, i32 addrspace(4)* %145, i64 %idxprom139
  %147 = load i32, i32 addrspace(4)* %arrayidx140, align 4
  br label %cond.end142

cond.false141:                                    ; preds = %cond.end131
  %148 = load i32, i32 addrspace(4)* %7, align 4
  br label %cond.end142

cond.end142:                                      ; preds = %cond.false141, %cond.true138
  %cond143 = phi i32 [ %147, %cond.true138 ], [ %148, %cond.false141 ]
  store i32 %cond143, i32 addrspace(4)* %7, align 4
  %149 = load i64, i64 addrspace(4)* %10, align 8
  %150 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %151 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom144 = sext i32 %151 to i64
  %arrayidx145 = getelementptr inbounds i32, i32 addrspace(4)* %150, i64 %idxprom144
  %152 = load i32, i32 addrspace(4)* %arrayidx145, align 4
  %conv146 = sext i32 %152 to i64
  %cmp147 = icmp slt i64 %149, %conv146
  br i1 %cmp147, label %cond.true149, label %cond.false153

cond.true149:                                     ; preds = %cond.end142
  %153 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %154 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom150 = sext i32 %154 to i64
  %arrayidx151 = getelementptr inbounds i32, i32 addrspace(4)* %153, i64 %idxprom150
  %155 = load i32, i32 addrspace(4)* %arrayidx151, align 4
  %conv152 = sext i32 %155 to i64
  br label %cond.end154

cond.false153:                                    ; preds = %cond.end142
  %156 = load i64, i64 addrspace(4)* %10, align 8
  br label %cond.end154

cond.end154:                                      ; preds = %cond.false153, %cond.true149
  %cond155 = phi i64 [ %conv152, %cond.true149 ], [ %156, %cond.false153 ]
  store i64 %cond155, i64 addrspace(4)* %10, align 8
  %157 = load i16, i16 addrspace(4)* %13, align 2
  %conv156 = zext i16 %157 to i32
  %158 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %159 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom157 = sext i32 %159 to i64
  %arrayidx158 = getelementptr inbounds i32, i32 addrspace(4)* %158, i64 %idxprom157
  %160 = load i32, i32 addrspace(4)* %arrayidx158, align 4
  %cmp159 = icmp slt i32 %conv156, %160
  br i1 %cmp159, label %cond.true161, label %cond.false164

cond.true161:                                     ; preds = %cond.end154
  %161 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %162 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom162 = sext i32 %162 to i64
  %arrayidx163 = getelementptr inbounds i32, i32 addrspace(4)* %161, i64 %idxprom162
  %163 = load i32, i32 addrspace(4)* %arrayidx163, align 4
  br label %cond.end166

cond.false164:                                    ; preds = %cond.end154
  %164 = load i16, i16 addrspace(4)* %13, align 2
  %conv165 = zext i16 %164 to i32
  br label %cond.end166

cond.end166:                                      ; preds = %cond.false164, %cond.true161
  %cond167 = phi i32 [ %163, %cond.true161 ], [ %conv165, %cond.false164 ]
  %conv168 = trunc i32 %cond167 to i16
  store i16 %conv168, i16 addrspace(4)* %13, align 2
  %165 = load i32, i32 addrspace(4)* %16, align 4
  %166 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %167 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom169 = sext i32 %167 to i64
  %arrayidx170 = getelementptr inbounds i32, i32 addrspace(4)* %166, i64 %idxprom169
  %168 = load i32, i32 addrspace(4)* %arrayidx170, align 4
  %cmp171 = icmp ult i32 %165, %168
  br i1 %cmp171, label %cond.true173, label %cond.false176

cond.true173:                                     ; preds = %cond.end166
  %169 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %170 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom174 = sext i32 %170 to i64
  %arrayidx175 = getelementptr inbounds i32, i32 addrspace(4)* %169, i64 %idxprom174
  %171 = load i32, i32 addrspace(4)* %arrayidx175, align 4
  br label %cond.end177

cond.false176:                                    ; preds = %cond.end166
  %172 = load i32, i32 addrspace(4)* %16, align 4
  br label %cond.end177

cond.end177:                                      ; preds = %cond.false176, %cond.true173
  %cond178 = phi i32 [ %171, %cond.true173 ], [ %172, %cond.false176 ]
  store i32 %cond178, i32 addrspace(4)* %16, align 4
  %173 = load i64, i64 addrspace(4)* %19, align 8
  %174 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %175 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom179 = sext i32 %175 to i64
  %arrayidx180 = getelementptr inbounds i32, i32 addrspace(4)* %174, i64 %idxprom179
  %176 = load i32, i32 addrspace(4)* %arrayidx180, align 4
  %conv181 = sext i32 %176 to i64
  %cmp182 = icmp ult i64 %173, %conv181
  br i1 %cmp182, label %cond.true184, label %cond.false188

cond.true184:                                     ; preds = %cond.end177
  %177 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %178 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom185 = sext i32 %178 to i64
  %arrayidx186 = getelementptr inbounds i32, i32 addrspace(4)* %177, i64 %idxprom185
  %179 = load i32, i32 addrspace(4)* %arrayidx186, align 4
  %conv187 = sext i32 %179 to i64
  br label %cond.end189

cond.false188:                                    ; preds = %cond.end177
  %180 = load i64, i64 addrspace(4)* %19, align 8
  br label %cond.end189

cond.end189:                                      ; preds = %cond.false188, %cond.true184
  %cond190 = phi i64 [ %conv187, %cond.true184 ], [ %180, %cond.false188 ]
  store i64 %cond190, i64 addrspace(4)* %19, align 8
  %181 = load float, float addrspace(4)* %22, align 4
  %182 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %183 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom191 = sext i32 %183 to i64
  %arrayidx192 = getelementptr inbounds i32, i32 addrspace(4)* %182, i64 %idxprom191
  %184 = load i32, i32 addrspace(4)* %arrayidx192, align 4
  %conv193 = sitofp i32 %184 to float
  %cmp194 = fcmp olt float %181, %conv193
  br i1 %cmp194, label %cond.true196, label %cond.false200

cond.true196:                                     ; preds = %cond.end189
  %185 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %186 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom197 = sext i32 %186 to i64
  %arrayidx198 = getelementptr inbounds i32, i32 addrspace(4)* %185, i64 %idxprom197
  %187 = load i32, i32 addrspace(4)* %arrayidx198, align 4
  %conv199 = sitofp i32 %187 to float
  br label %cond.end201

cond.false200:                                    ; preds = %cond.end189
  %188 = load float, float addrspace(4)* %22, align 4
  br label %cond.end201

cond.end201:                                      ; preds = %cond.false200, %cond.true196
  %cond202 = phi float [ %conv199, %cond.true196 ], [ %188, %cond.false200 ]
  store float %cond202, float addrspace(4)* %22, align 4
  %189 = load double, double addrspace(4)* %25, align 8
  %190 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %191 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom203 = sext i32 %191 to i64
  %arrayidx204 = getelementptr inbounds i32, i32 addrspace(4)* %190, i64 %idxprom203
  %192 = load i32, i32 addrspace(4)* %arrayidx204, align 4
  %conv205 = sitofp i32 %192 to double
  %cmp206 = fcmp olt double %189, %conv205
  br i1 %cmp206, label %cond.true208, label %cond.false212

cond.true208:                                     ; preds = %cond.end201
  %193 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %194 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom209 = sext i32 %194 to i64
  %arrayidx210 = getelementptr inbounds i32, i32 addrspace(4)* %193, i64 %idxprom209
  %195 = load i32, i32 addrspace(4)* %arrayidx210, align 4
  %conv211 = sitofp i32 %195 to double
  br label %cond.end213

cond.false212:                                    ; preds = %cond.end201
  %196 = load double, double addrspace(4)* %25, align 8
  br label %cond.end213

cond.end213:                                      ; preds = %cond.false212, %cond.true208
  %cond214 = phi double [ %conv211, %cond.true208 ], [ %196, %cond.false212 ]
  store double %cond214, double addrspace(4)* %25, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end213
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %197 = load i32, i32 addrspace(4)* %26, align 4
  %add215 = add nsw i32 %197, 1
  store i32 %add215, i32 addrspace(4)* %26, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %32) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %31) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 85985690, !"foo", i32 22, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 9.0.0"}

; Original code:
; #if LIT == 0
; #include <stdio.h>
; #include <stdlib.h>
; static int errors = 0;
; #define CHECK(x, expected) if ((int)(x) != (int)(expected)) { printf(#x " must be %d: %d\n", (int)expected, (int)(x)); ++errors; }
; #endif  // LIT == 0
; #define MIN(x, y) (x) = ((y) < (x)) ? y : x
; #define MAX(x, y) (x) = ((x) < (y)) ? y : x
; void foo(int x[]) {
;   int i;
;   short sadd16 = 0, smin16 = 200, smax16 = 0;
;   int sadd32 = 0, smin32 = 200, smax32 = 0;
;   long long int sadd64 = 0, smin64 = 200, smax64 = 0;
;
;   unsigned short uadd16 = 0, umin16 = 200, umax16 = 0;
;   unsigned int uadd32 = 0, umin32 = 200, umax32 = 0;
;   unsigned long long int uadd64 = 0, umin64 = 200, umax64 = 0;
;
;   float fadd32 = 0.0F, fmin32 = 200.0F, fmax32 = 0.0F;
;   double fadd64 = 0.0, fmin64 = 200.0, fmax64 = 0.0;
;
; #pragma omp target map(tofrom: sadd16,sadd32,sadd64,uadd16,uadd32,uadd64,fadd32,fadd64,smin16,smin32,smin64,umin16,umin32,umin64,fmin32,fmin64,smax16,smax32,smax64,umax16,umax32,umax64,fmax32,fmax64) map(to:x[0:100])
; #pragma omp parallel for reduction(+:sadd16,sadd32,sadd64,uadd16,uadd32,uadd64,fadd32,fadd64) reduction(min:smin16,smin32,smin64,umin16,umin32,umin64,fmin32,fmin64) reduction(max:smax16,smax32,smax64,umax16,umax32,umax64,fmax32,fmax64)
;   for (i = 0; i < 100; ++i) {
;     sadd16 += x[i];
;     sadd32 += x[i];
;     sadd64 += x[i];
;     uadd16 += x[i];
;     uadd32 += x[i];
;     uadd64 += x[i];
;     fadd32 += x[i];
;     fadd64 += x[i];
;
;     MIN(smin16, x[i]);
;     MIN(smin32, x[i]);
;     MIN(smin64, x[i]);
;     MIN(umin16, x[i]);
;     MIN(umin32, x[i]);
;     MIN(umin64, x[i]);
;     MIN(fmin32, x[i]);
;     MIN(fmin64, x[i]);
;
;     MAX(smax16, x[i]);
;     MAX(smax32, x[i]);
;     MAX(smax64, x[i]);
;     MAX(umax16, x[i]);
;     MAX(umax32, x[i]);
;     MAX(umax64, x[i]);
;     MAX(fmax32, x[i]);
;     MAX(fmax64, x[i]);
;   }
; #if LIT == 0
;   CHECK(sadd16, 297);
;   CHECK(sadd32, 297);
;   CHECK(sadd64, 297);
;   CHECK(uadd16, 297);
;   CHECK(uadd32, 297);
;   CHECK(uadd64, 297);
;   CHECK(smin16, 1);
;   CHECK(smin32, 1);
;   CHECK(smin64, 1);
;   CHECK(umin16, 1);
;   CHECK(umin32, 1);
;   CHECK(umin64, 1);
;   CHECK(fmin32, 1);
;   CHECK(fmin64, 1);
;   CHECK(smax16, 100);
;   CHECK(smax32, 100);
;   CHECK(smax64, 100);
;   CHECK(umax16, 100);
;   CHECK(umax32, 100);
;   CHECK(umax64, 100);
;   CHECK(fmax32, 100);
;   CHECK(fmax64, 100);
; #endif  // LIT == 0
; }
; #if LIT == 0
; int main() {
;   int i;
;   int *x = (int *)malloc(100 * sizeof(int));
;   for (i = 0; i < 100; ++i)
;     x[i] = 2;
;
;   x[37] = 100;
;   x[17] = 1;
;
;   foo(x);
;
;   if (errors != 0) {
;     printf("Test failed with %d errors.\n", errors);
;     return 1;
;   }
;
;   return 0;
; }
; #endif  // LIT == 0
