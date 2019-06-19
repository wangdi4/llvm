; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload  -S | FileCheck %s

; Original code (see at the end of the file).

; Signed add:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupIAdd_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupIAdd_i32_i32_i64(i32 3, i32 0, i64

; Unsigned add:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupIAdd_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupIAdd_i32_i32_i64(i32 3, i32 0, i64

; FP add:
; CHECK-DAG: call void @__kmpc_atomic_float4_add(float addrspace(4)* {{.*}}, float
; CHECK-DAG: call double @__builtin_spirv_OpGroupFAdd_i32_i32_f64(i32 3, i32 0, double

; Signed min:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupSMin_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: call i32 @__builtin_spirv_OpGroupSMin_i32_i32_i32(i32 3, i32 0, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupSMin_i32_i32_i64(i32 3, i32 0, i64

; Unsigned min:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupUMin_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: call i32 @__builtin_spirv_OpGroupUMin_i32_i32_i32(i32 3, i32 0, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupUMin_i32_i32_i64(i32 3, i32 0, i64

; FP min:
; CHECK-DAG: call float @__builtin_spirv_OpGroupFMin_i32_i32_f32(i32 3, i32 0, float
; CHECK-DAG: call double @__builtin_spirv_OpGroupFMin_i32_i32_f64(i32 3, i32 0, double

; Signed max:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupSMax_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: call i32 @__builtin_spirv_OpGroupSMax_i32_i32_i32(i32 3, i32 0, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupSMax_i32_i32_i64(i32 3, i32 0, i64

; Unsigned max:
; CHECK-DAG: call i16 @__builtin_spirv_OpGroupUMax_i32_i32_i16(i32 3, i32 0, i16
; CHECK-DAG: call i32 @__builtin_spirv_OpGroupUMax_i32_i32_i32(i32 3, i32 0, i32
; CHECK-DAG: call i64 @__builtin_spirv_OpGroupUMax_i32_i32_i64(i32 3, i32 0, i64

; FP max:
; CHECK-DAG: call float @__builtin_spirv_OpGroupFMax_i32_i32_f32(i32 3, i32 0, float
; CHECK-DAG: call double @__builtin_spirv_OpGroupFMax_i32_i32_f64(i32 3, i32 0, double

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo(i32* %x) #0 {
entry:
  %x.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  %sadd16 = alloca i16, align 2
  %smin16 = alloca i16, align 2
  %smax16 = alloca i16, align 2
  %sadd32 = alloca i32, align 4
  %smin32 = alloca i32, align 4
  %smax32 = alloca i32, align 4
  %sadd64 = alloca i64, align 8
  %smin64 = alloca i64, align 8
  %smax64 = alloca i64, align 8
  %uadd16 = alloca i16, align 2
  %umin16 = alloca i16, align 2
  %umax16 = alloca i16, align 2
  %uadd32 = alloca i32, align 4
  %umin32 = alloca i32, align 4
  %umax32 = alloca i32, align 4
  %uadd64 = alloca i64, align 8
  %umin64 = alloca i64, align 8
  %umax64 = alloca i64, align 8
  %fadd32 = alloca float, align 4
  %fmin32 = alloca float, align 4
  %fmax32 = alloca float, align 4
  %fadd64 = alloca double, align 8
  %fmin64 = alloca double, align 8
  %fmax64 = alloca double, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %x, i32** %x.addr, align 8
  store i16 0, i16* %sadd16, align 2
  store i16 200, i16* %smin16, align 2
  store i16 0, i16* %smax16, align 2
  store i32 0, i32* %sadd32, align 4
  store i32 200, i32* %smin32, align 4
  store i32 0, i32* %smax32, align 4
  store i64 0, i64* %sadd64, align 8
  store i64 200, i64* %smin64, align 8
  store i64 0, i64* %smax64, align 8
  store i16 0, i16* %uadd16, align 2
  store i16 200, i16* %umin16, align 2
  store i16 0, i16* %umax16, align 2
  store i32 0, i32* %uadd32, align 4
  store i32 200, i32* %umin32, align 4
  store i32 0, i32* %umax32, align 4
  store i64 0, i64* %uadd64, align 8
  store i64 200, i64* %umin64, align 8
  store i64 0, i64* %umax64, align 8
  store float 0.000000e+00, float* %fadd32, align 4
  store float 2.000000e+02, float* %fmin32, align 4
  store float 0.000000e+00, float* %fmax32, align 4
  store double 0.000000e+00, double* %fadd64, align 8
  store double 2.000000e+02, double* %fmin64, align 8
  store double 0.000000e+00, double* %fmax64, align 8
  %0 = load i32*, i32** %x.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i16* %sadd16), "QUAL.OMP.MAP.TOFROM"(i32* %sadd32), "QUAL.OMP.MAP.TOFROM"(i64* %sadd64), "QUAL.OMP.MAP.TOFROM"(i16* %uadd16), "QUAL.OMP.MAP.TOFROM"(i32* %uadd32), "QUAL.OMP.MAP.TOFROM"(i64* %uadd64), "QUAL.OMP.MAP.TOFROM"(float* %fadd32), "QUAL.OMP.MAP.TOFROM"(double* %fadd64), "QUAL.OMP.MAP.TOFROM"(i16* %smin16), "QUAL.OMP.MAP.TOFROM"(i32* %smin32), "QUAL.OMP.MAP.TOFROM"(i64* %smin64), "QUAL.OMP.MAP.TOFROM"(i16* %umin16), "QUAL.OMP.MAP.TOFROM"(i32* %umin32), "QUAL.OMP.MAP.TOFROM"(i64* %umin64), "QUAL.OMP.MAP.TOFROM"(float* %fmin32), "QUAL.OMP.MAP.TOFROM"(double* %fmin64), "QUAL.OMP.MAP.TOFROM"(i16* %smax16), "QUAL.OMP.MAP.TOFROM"(i32* %smax32), "QUAL.OMP.MAP.TOFROM"(i64* %smax64), "QUAL.OMP.MAP.TOFROM"(i16* %umax16), "QUAL.OMP.MAP.TOFROM"(i32* %umax32), "QUAL.OMP.MAP.TOFROM"(i64* %umax64), "QUAL.OMP.MAP.TOFROM"(float* %fmax32), "QUAL.OMP.MAP.TOFROM"(double* %fmax64), "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %x.addr, i32** %x.addr, i64 8), "QUAL.OMP.MAP.TO:AGGR"(i32** %x.addr, i32* %arrayidx, i64 400), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i16* %sadd16), "QUAL.OMP.REDUCTION.ADD"(i32* %sadd32), "QUAL.OMP.REDUCTION.ADD"(i64* %sadd64), "QUAL.OMP.REDUCTION.ADD"(i16* %uadd16), "QUAL.OMP.REDUCTION.ADD"(i32* %uadd32), "QUAL.OMP.REDUCTION.ADD"(i64* %uadd64), "QUAL.OMP.REDUCTION.ADD"(float* %fadd32), "QUAL.OMP.REDUCTION.ADD"(double* %fadd64), "QUAL.OMP.REDUCTION.MIN"(i16* %smin16), "QUAL.OMP.REDUCTION.MIN"(i32* %smin32), "QUAL.OMP.REDUCTION.MIN"(i64* %smin64), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i16* %umin16), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i32* %umin32), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i64* %umin64), "QUAL.OMP.REDUCTION.MIN"(float* %fmin32), "QUAL.OMP.REDUCTION.MIN"(double* %fmin64), "QUAL.OMP.REDUCTION.MAX"(i16* %smax16), "QUAL.OMP.REDUCTION.MAX"(i32* %smax32), "QUAL.OMP.REDUCTION.MAX"(i64* %smax64), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i16* %umax16), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i32* %umax32), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i64* %umax64), "QUAL.OMP.REDUCTION.MAX"(float* %fmax32), "QUAL.OMP.REDUCTION.MAX"(double* %fmax64), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** %x.addr) ]
  %3 = load i32, i32* %.omp.lb, align 4
  store i32 %3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32* %.omp.iv, align 4
  %5 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %7 = load i32*, i32** %x.addr, align 8
  %8 = load i32, i32* %i, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx1 = getelementptr inbounds i32, i32* %7, i64 %idxprom
  %9 = load i32, i32* %arrayidx1, align 4
  %10 = load i16, i16* %sadd16, align 2
  %conv = sext i16 %10 to i32
  %add2 = add nsw i32 %conv, %9
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, i16* %sadd16, align 2
  %11 = load i32*, i32** %x.addr, align 8
  %12 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %12 to i64
  %arrayidx5 = getelementptr inbounds i32, i32* %11, i64 %idxprom4
  %13 = load i32, i32* %arrayidx5, align 4
  %14 = load i32, i32* %sadd32, align 4
  %add6 = add nsw i32 %14, %13
  store i32 %add6, i32* %sadd32, align 4
  %15 = load i32*, i32** %x.addr, align 8
  %16 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %16 to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %15, i64 %idxprom7
  %17 = load i32, i32* %arrayidx8, align 4
  %conv9 = sext i32 %17 to i64
  %18 = load i64, i64* %sadd64, align 8
  %add10 = add nsw i64 %18, %conv9
  store i64 %add10, i64* %sadd64, align 8
  %19 = load i32*, i32** %x.addr, align 8
  %20 = load i32, i32* %i, align 4
  %idxprom11 = sext i32 %20 to i64
  %arrayidx12 = getelementptr inbounds i32, i32* %19, i64 %idxprom11
  %21 = load i32, i32* %arrayidx12, align 4
  %22 = load i16, i16* %uadd16, align 2
  %conv13 = zext i16 %22 to i32
  %add14 = add nsw i32 %conv13, %21
  %conv15 = trunc i32 %add14 to i16
  store i16 %conv15, i16* %uadd16, align 2
  %23 = load i32*, i32** %x.addr, align 8
  %24 = load i32, i32* %i, align 4
  %idxprom16 = sext i32 %24 to i64
  %arrayidx17 = getelementptr inbounds i32, i32* %23, i64 %idxprom16
  %25 = load i32, i32* %arrayidx17, align 4
  %26 = load i32, i32* %uadd32, align 4
  %add18 = add i32 %26, %25
  store i32 %add18, i32* %uadd32, align 4
  %27 = load i32*, i32** %x.addr, align 8
  %28 = load i32, i32* %i, align 4
  %idxprom19 = sext i32 %28 to i64
  %arrayidx20 = getelementptr inbounds i32, i32* %27, i64 %idxprom19
  %29 = load i32, i32* %arrayidx20, align 4
  %conv21 = sext i32 %29 to i64
  %30 = load i64, i64* %uadd64, align 8
  %add22 = add i64 %30, %conv21
  store i64 %add22, i64* %uadd64, align 8
  %31 = load i32*, i32** %x.addr, align 8
  %32 = load i32, i32* %i, align 4
  %idxprom23 = sext i32 %32 to i64
  %arrayidx24 = getelementptr inbounds i32, i32* %31, i64 %idxprom23
  %33 = load i32, i32* %arrayidx24, align 4
  %conv25 = sitofp i32 %33 to float
  %34 = load float, float* %fadd32, align 4
  %add26 = fadd float %34, %conv25
  store float %add26, float* %fadd32, align 4
  %35 = load i32*, i32** %x.addr, align 8
  %36 = load i32, i32* %i, align 4
  %idxprom27 = sext i32 %36 to i64
  %arrayidx28 = getelementptr inbounds i32, i32* %35, i64 %idxprom27
  %37 = load i32, i32* %arrayidx28, align 4
  %conv29 = sitofp i32 %37 to double
  %38 = load double, double* %fadd64, align 8
  %add30 = fadd double %38, %conv29
  store double %add30, double* %fadd64, align 8
  %39 = load i32*, i32** %x.addr, align 8
  %40 = load i32, i32* %i, align 4
  %idxprom31 = sext i32 %40 to i64
  %arrayidx32 = getelementptr inbounds i32, i32* %39, i64 %idxprom31
  %41 = load i32, i32* %arrayidx32, align 4
  %42 = load i16, i16* %smin16, align 2
  %conv33 = sext i16 %42 to i32
  %cmp34 = icmp slt i32 %41, %conv33
  br i1 %cmp34, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  %43 = load i32*, i32** %x.addr, align 8
  %44 = load i32, i32* %i, align 4
  %idxprom36 = sext i32 %44 to i64
  %arrayidx37 = getelementptr inbounds i32, i32* %43, i64 %idxprom36
  %45 = load i32, i32* %arrayidx37, align 4
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  %46 = load i16, i16* %smin16, align 2
  %conv38 = sext i16 %46 to i32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %45, %cond.true ], [ %conv38, %cond.false ]
  %conv39 = trunc i32 %cond to i16
  store i16 %conv39, i16* %smin16, align 2
  %47 = load i32*, i32** %x.addr, align 8
  %48 = load i32, i32* %i, align 4
  %idxprom40 = sext i32 %48 to i64
  %arrayidx41 = getelementptr inbounds i32, i32* %47, i64 %idxprom40
  %49 = load i32, i32* %arrayidx41, align 4
  %50 = load i32, i32* %smin32, align 4
  %cmp42 = icmp slt i32 %49, %50
  br i1 %cmp42, label %cond.true44, label %cond.false47

cond.true44:                                      ; preds = %cond.end
  %51 = load i32*, i32** %x.addr, align 8
  %52 = load i32, i32* %i, align 4
  %idxprom45 = sext i32 %52 to i64
  %arrayidx46 = getelementptr inbounds i32, i32* %51, i64 %idxprom45
  %53 = load i32, i32* %arrayidx46, align 4
  br label %cond.end48

cond.false47:                                     ; preds = %cond.end
  %54 = load i32, i32* %smin32, align 4
  br label %cond.end48

cond.end48:                                       ; preds = %cond.false47, %cond.true44
  %cond49 = phi i32 [ %53, %cond.true44 ], [ %54, %cond.false47 ]
  store i32 %cond49, i32* %smin32, align 4
  %55 = load i32*, i32** %x.addr, align 8
  %56 = load i32, i32* %i, align 4
  %idxprom50 = sext i32 %56 to i64
  %arrayidx51 = getelementptr inbounds i32, i32* %55, i64 %idxprom50
  %57 = load i32, i32* %arrayidx51, align 4
  %conv52 = sext i32 %57 to i64
  %58 = load i64, i64* %smin64, align 8
  %cmp53 = icmp slt i64 %conv52, %58
  br i1 %cmp53, label %cond.true55, label %cond.false59

cond.true55:                                      ; preds = %cond.end48
  %59 = load i32*, i32** %x.addr, align 8
  %60 = load i32, i32* %i, align 4
  %idxprom56 = sext i32 %60 to i64
  %arrayidx57 = getelementptr inbounds i32, i32* %59, i64 %idxprom56
  %61 = load i32, i32* %arrayidx57, align 4
  %conv58 = sext i32 %61 to i64
  br label %cond.end60

cond.false59:                                     ; preds = %cond.end48
  %62 = load i64, i64* %smin64, align 8
  br label %cond.end60

cond.end60:                                       ; preds = %cond.false59, %cond.true55
  %cond61 = phi i64 [ %conv58, %cond.true55 ], [ %62, %cond.false59 ]
  store i64 %cond61, i64* %smin64, align 8
  %63 = load i32*, i32** %x.addr, align 8
  %64 = load i32, i32* %i, align 4
  %idxprom62 = sext i32 %64 to i64
  %arrayidx63 = getelementptr inbounds i32, i32* %63, i64 %idxprom62
  %65 = load i32, i32* %arrayidx63, align 4
  %66 = load i16, i16* %umin16, align 2
  %conv64 = zext i16 %66 to i32
  %cmp65 = icmp slt i32 %65, %conv64
  br i1 %cmp65, label %cond.true67, label %cond.false70

cond.true67:                                      ; preds = %cond.end60
  %67 = load i32*, i32** %x.addr, align 8
  %68 = load i32, i32* %i, align 4
  %idxprom68 = sext i32 %68 to i64
  %arrayidx69 = getelementptr inbounds i32, i32* %67, i64 %idxprom68
  %69 = load i32, i32* %arrayidx69, align 4
  br label %cond.end72

cond.false70:                                     ; preds = %cond.end60
  %70 = load i16, i16* %umin16, align 2
  %conv71 = zext i16 %70 to i32
  br label %cond.end72

cond.end72:                                       ; preds = %cond.false70, %cond.true67
  %cond73 = phi i32 [ %69, %cond.true67 ], [ %conv71, %cond.false70 ]
  %conv74 = trunc i32 %cond73 to i16
  store i16 %conv74, i16* %umin16, align 2
  %71 = load i32*, i32** %x.addr, align 8
  %72 = load i32, i32* %i, align 4
  %idxprom75 = sext i32 %72 to i64
  %arrayidx76 = getelementptr inbounds i32, i32* %71, i64 %idxprom75
  %73 = load i32, i32* %arrayidx76, align 4
  %74 = load i32, i32* %umin32, align 4
  %cmp77 = icmp ult i32 %73, %74
  br i1 %cmp77, label %cond.true79, label %cond.false82

cond.true79:                                      ; preds = %cond.end72
  %75 = load i32*, i32** %x.addr, align 8
  %76 = load i32, i32* %i, align 4
  %idxprom80 = sext i32 %76 to i64
  %arrayidx81 = getelementptr inbounds i32, i32* %75, i64 %idxprom80
  %77 = load i32, i32* %arrayidx81, align 4
  br label %cond.end83

cond.false82:                                     ; preds = %cond.end72
  %78 = load i32, i32* %umin32, align 4
  br label %cond.end83

cond.end83:                                       ; preds = %cond.false82, %cond.true79
  %cond84 = phi i32 [ %77, %cond.true79 ], [ %78, %cond.false82 ]
  store i32 %cond84, i32* %umin32, align 4
  %79 = load i32*, i32** %x.addr, align 8
  %80 = load i32, i32* %i, align 4
  %idxprom85 = sext i32 %80 to i64
  %arrayidx86 = getelementptr inbounds i32, i32* %79, i64 %idxprom85
  %81 = load i32, i32* %arrayidx86, align 4
  %conv87 = sext i32 %81 to i64
  %82 = load i64, i64* %umin64, align 8
  %cmp88 = icmp ult i64 %conv87, %82
  br i1 %cmp88, label %cond.true90, label %cond.false94

cond.true90:                                      ; preds = %cond.end83
  %83 = load i32*, i32** %x.addr, align 8
  %84 = load i32, i32* %i, align 4
  %idxprom91 = sext i32 %84 to i64
  %arrayidx92 = getelementptr inbounds i32, i32* %83, i64 %idxprom91
  %85 = load i32, i32* %arrayidx92, align 4
  %conv93 = sext i32 %85 to i64
  br label %cond.end95

cond.false94:                                     ; preds = %cond.end83
  %86 = load i64, i64* %umin64, align 8
  br label %cond.end95

cond.end95:                                       ; preds = %cond.false94, %cond.true90
  %cond96 = phi i64 [ %conv93, %cond.true90 ], [ %86, %cond.false94 ]
  store i64 %cond96, i64* %umin64, align 8
  %87 = load i32*, i32** %x.addr, align 8
  %88 = load i32, i32* %i, align 4
  %idxprom97 = sext i32 %88 to i64
  %arrayidx98 = getelementptr inbounds i32, i32* %87, i64 %idxprom97
  %89 = load i32, i32* %arrayidx98, align 4
  %conv99 = sitofp i32 %89 to float
  %90 = load float, float* %fmin32, align 4
  %cmp100 = fcmp olt float %conv99, %90
  br i1 %cmp100, label %cond.true102, label %cond.false106

cond.true102:                                     ; preds = %cond.end95
  %91 = load i32*, i32** %x.addr, align 8
  %92 = load i32, i32* %i, align 4
  %idxprom103 = sext i32 %92 to i64
  %arrayidx104 = getelementptr inbounds i32, i32* %91, i64 %idxprom103
  %93 = load i32, i32* %arrayidx104, align 4
  %conv105 = sitofp i32 %93 to float
  br label %cond.end107

cond.false106:                                    ; preds = %cond.end95
  %94 = load float, float* %fmin32, align 4
  br label %cond.end107

cond.end107:                                      ; preds = %cond.false106, %cond.true102
  %cond108 = phi float [ %conv105, %cond.true102 ], [ %94, %cond.false106 ]
  store float %cond108, float* %fmin32, align 4
  %95 = load i32*, i32** %x.addr, align 8
  %96 = load i32, i32* %i, align 4
  %idxprom109 = sext i32 %96 to i64
  %arrayidx110 = getelementptr inbounds i32, i32* %95, i64 %idxprom109
  %97 = load i32, i32* %arrayidx110, align 4
  %conv111 = sitofp i32 %97 to double
  %98 = load double, double* %fmin64, align 8
  %cmp112 = fcmp olt double %conv111, %98
  br i1 %cmp112, label %cond.true114, label %cond.false118

cond.true114:                                     ; preds = %cond.end107
  %99 = load i32*, i32** %x.addr, align 8
  %100 = load i32, i32* %i, align 4
  %idxprom115 = sext i32 %100 to i64
  %arrayidx116 = getelementptr inbounds i32, i32* %99, i64 %idxprom115
  %101 = load i32, i32* %arrayidx116, align 4
  %conv117 = sitofp i32 %101 to double
  br label %cond.end119

cond.false118:                                    ; preds = %cond.end107
  %102 = load double, double* %fmin64, align 8
  br label %cond.end119

cond.end119:                                      ; preds = %cond.false118, %cond.true114
  %cond120 = phi double [ %conv117, %cond.true114 ], [ %102, %cond.false118 ]
  store double %cond120, double* %fmin64, align 8
  %103 = load i16, i16* %smax16, align 2
  %conv121 = sext i16 %103 to i32
  %104 = load i32*, i32** %x.addr, align 8
  %105 = load i32, i32* %i, align 4
  %idxprom122 = sext i32 %105 to i64
  %arrayidx123 = getelementptr inbounds i32, i32* %104, i64 %idxprom122
  %106 = load i32, i32* %arrayidx123, align 4
  %cmp124 = icmp slt i32 %conv121, %106
  br i1 %cmp124, label %cond.true126, label %cond.false129

cond.true126:                                     ; preds = %cond.end119
  %107 = load i32*, i32** %x.addr, align 8
  %108 = load i32, i32* %i, align 4
  %idxprom127 = sext i32 %108 to i64
  %arrayidx128 = getelementptr inbounds i32, i32* %107, i64 %idxprom127
  %109 = load i32, i32* %arrayidx128, align 4
  br label %cond.end131

cond.false129:                                    ; preds = %cond.end119
  %110 = load i16, i16* %smax16, align 2
  %conv130 = sext i16 %110 to i32
  br label %cond.end131

cond.end131:                                      ; preds = %cond.false129, %cond.true126
  %cond132 = phi i32 [ %109, %cond.true126 ], [ %conv130, %cond.false129 ]
  %conv133 = trunc i32 %cond132 to i16
  store i16 %conv133, i16* %smax16, align 2
  %111 = load i32, i32* %smax32, align 4
  %112 = load i32*, i32** %x.addr, align 8
  %113 = load i32, i32* %i, align 4
  %idxprom134 = sext i32 %113 to i64
  %arrayidx135 = getelementptr inbounds i32, i32* %112, i64 %idxprom134
  %114 = load i32, i32* %arrayidx135, align 4
  %cmp136 = icmp slt i32 %111, %114
  br i1 %cmp136, label %cond.true138, label %cond.false141

cond.true138:                                     ; preds = %cond.end131
  %115 = load i32*, i32** %x.addr, align 8
  %116 = load i32, i32* %i, align 4
  %idxprom139 = sext i32 %116 to i64
  %arrayidx140 = getelementptr inbounds i32, i32* %115, i64 %idxprom139
  %117 = load i32, i32* %arrayidx140, align 4
  br label %cond.end142

cond.false141:                                    ; preds = %cond.end131
  %118 = load i32, i32* %smax32, align 4
  br label %cond.end142

cond.end142:                                      ; preds = %cond.false141, %cond.true138
  %cond143 = phi i32 [ %117, %cond.true138 ], [ %118, %cond.false141 ]
  store i32 %cond143, i32* %smax32, align 4
  %119 = load i64, i64* %smax64, align 8
  %120 = load i32*, i32** %x.addr, align 8
  %121 = load i32, i32* %i, align 4
  %idxprom144 = sext i32 %121 to i64
  %arrayidx145 = getelementptr inbounds i32, i32* %120, i64 %idxprom144
  %122 = load i32, i32* %arrayidx145, align 4
  %conv146 = sext i32 %122 to i64
  %cmp147 = icmp slt i64 %119, %conv146
  br i1 %cmp147, label %cond.true149, label %cond.false153

cond.true149:                                     ; preds = %cond.end142
  %123 = load i32*, i32** %x.addr, align 8
  %124 = load i32, i32* %i, align 4
  %idxprom150 = sext i32 %124 to i64
  %arrayidx151 = getelementptr inbounds i32, i32* %123, i64 %idxprom150
  %125 = load i32, i32* %arrayidx151, align 4
  %conv152 = sext i32 %125 to i64
  br label %cond.end154

cond.false153:                                    ; preds = %cond.end142
  %126 = load i64, i64* %smax64, align 8
  br label %cond.end154

cond.end154:                                      ; preds = %cond.false153, %cond.true149
  %cond155 = phi i64 [ %conv152, %cond.true149 ], [ %126, %cond.false153 ]
  store i64 %cond155, i64* %smax64, align 8
  %127 = load i16, i16* %umax16, align 2
  %conv156 = zext i16 %127 to i32
  %128 = load i32*, i32** %x.addr, align 8
  %129 = load i32, i32* %i, align 4
  %idxprom157 = sext i32 %129 to i64
  %arrayidx158 = getelementptr inbounds i32, i32* %128, i64 %idxprom157
  %130 = load i32, i32* %arrayidx158, align 4
  %cmp159 = icmp slt i32 %conv156, %130
  br i1 %cmp159, label %cond.true161, label %cond.false164

cond.true161:                                     ; preds = %cond.end154
  %131 = load i32*, i32** %x.addr, align 8
  %132 = load i32, i32* %i, align 4
  %idxprom162 = sext i32 %132 to i64
  %arrayidx163 = getelementptr inbounds i32, i32* %131, i64 %idxprom162
  %133 = load i32, i32* %arrayidx163, align 4
  br label %cond.end166

cond.false164:                                    ; preds = %cond.end154
  %134 = load i16, i16* %umax16, align 2
  %conv165 = zext i16 %134 to i32
  br label %cond.end166

cond.end166:                                      ; preds = %cond.false164, %cond.true161
  %cond167 = phi i32 [ %133, %cond.true161 ], [ %conv165, %cond.false164 ]
  %conv168 = trunc i32 %cond167 to i16
  store i16 %conv168, i16* %umax16, align 2
  %135 = load i32, i32* %umax32, align 4
  %136 = load i32*, i32** %x.addr, align 8
  %137 = load i32, i32* %i, align 4
  %idxprom169 = sext i32 %137 to i64
  %arrayidx170 = getelementptr inbounds i32, i32* %136, i64 %idxprom169
  %138 = load i32, i32* %arrayidx170, align 4
  %cmp171 = icmp ult i32 %135, %138
  br i1 %cmp171, label %cond.true173, label %cond.false176

cond.true173:                                     ; preds = %cond.end166
  %139 = load i32*, i32** %x.addr, align 8
  %140 = load i32, i32* %i, align 4
  %idxprom174 = sext i32 %140 to i64
  %arrayidx175 = getelementptr inbounds i32, i32* %139, i64 %idxprom174
  %141 = load i32, i32* %arrayidx175, align 4
  br label %cond.end177

cond.false176:                                    ; preds = %cond.end166
  %142 = load i32, i32* %umax32, align 4
  br label %cond.end177

cond.end177:                                      ; preds = %cond.false176, %cond.true173
  %cond178 = phi i32 [ %141, %cond.true173 ], [ %142, %cond.false176 ]
  store i32 %cond178, i32* %umax32, align 4
  %143 = load i64, i64* %umax64, align 8
  %144 = load i32*, i32** %x.addr, align 8
  %145 = load i32, i32* %i, align 4
  %idxprom179 = sext i32 %145 to i64
  %arrayidx180 = getelementptr inbounds i32, i32* %144, i64 %idxprom179
  %146 = load i32, i32* %arrayidx180, align 4
  %conv181 = sext i32 %146 to i64
  %cmp182 = icmp ult i64 %143, %conv181
  br i1 %cmp182, label %cond.true184, label %cond.false188

cond.true184:                                     ; preds = %cond.end177
  %147 = load i32*, i32** %x.addr, align 8
  %148 = load i32, i32* %i, align 4
  %idxprom185 = sext i32 %148 to i64
  %arrayidx186 = getelementptr inbounds i32, i32* %147, i64 %idxprom185
  %149 = load i32, i32* %arrayidx186, align 4
  %conv187 = sext i32 %149 to i64
  br label %cond.end189

cond.false188:                                    ; preds = %cond.end177
  %150 = load i64, i64* %umax64, align 8
  br label %cond.end189

cond.end189:                                      ; preds = %cond.false188, %cond.true184
  %cond190 = phi i64 [ %conv187, %cond.true184 ], [ %150, %cond.false188 ]
  store i64 %cond190, i64* %umax64, align 8
  %151 = load float, float* %fmax32, align 4
  %152 = load i32*, i32** %x.addr, align 8
  %153 = load i32, i32* %i, align 4
  %idxprom191 = sext i32 %153 to i64
  %arrayidx192 = getelementptr inbounds i32, i32* %152, i64 %idxprom191
  %154 = load i32, i32* %arrayidx192, align 4
  %conv193 = sitofp i32 %154 to float
  %cmp194 = fcmp olt float %151, %conv193
  br i1 %cmp194, label %cond.true196, label %cond.false200

cond.true196:                                     ; preds = %cond.end189
  %155 = load i32*, i32** %x.addr, align 8
  %156 = load i32, i32* %i, align 4
  %idxprom197 = sext i32 %156 to i64
  %arrayidx198 = getelementptr inbounds i32, i32* %155, i64 %idxprom197
  %157 = load i32, i32* %arrayidx198, align 4
  %conv199 = sitofp i32 %157 to float
  br label %cond.end201

cond.false200:                                    ; preds = %cond.end189
  %158 = load float, float* %fmax32, align 4
  br label %cond.end201

cond.end201:                                      ; preds = %cond.false200, %cond.true196
  %cond202 = phi float [ %conv199, %cond.true196 ], [ %158, %cond.false200 ]
  store float %cond202, float* %fmax32, align 4
  %159 = load double, double* %fmax64, align 8
  %160 = load i32*, i32** %x.addr, align 8
  %161 = load i32, i32* %i, align 4
  %idxprom203 = sext i32 %161 to i64
  %arrayidx204 = getelementptr inbounds i32, i32* %160, i64 %idxprom203
  %162 = load i32, i32* %arrayidx204, align 4
  %conv205 = sitofp i32 %162 to double
  %cmp206 = fcmp olt double %159, %conv205
  br i1 %cmp206, label %cond.true208, label %cond.false212

cond.true208:                                     ; preds = %cond.end201
  %163 = load i32*, i32** %x.addr, align 8
  %164 = load i32, i32* %i, align 4
  %idxprom209 = sext i32 %164 to i64
  %arrayidx210 = getelementptr inbounds i32, i32* %163, i64 %idxprom209
  %165 = load i32, i32* %arrayidx210, align 4
  %conv211 = sitofp i32 %165 to double
  br label %cond.end213

cond.false212:                                    ; preds = %cond.end201
  %166 = load double, double* %fmax64, align 8
  br label %cond.end213

cond.end213:                                      ; preds = %cond.false212, %cond.true208
  %cond214 = phi double [ %conv211, %cond.true208 ], [ %166, %cond.false212 ]
  store double %cond214, double* %fmax64, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end213
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %167 = load i32, i32* %.omp.iv, align 4
  %add215 = add nsw i32 %167, 1
  store i32 %add215, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
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

!0 = !{i32 0, i32 2052, i32 85987072, !"foo", i32 22, i32 0, i32 0}
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
