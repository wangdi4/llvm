; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; Original code (see at the end of the file).

; Signed add:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_addi(i32
; CHECK-DAG: spir_func void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: spir_func void @__kmpc_atomic_fixed8_add(i64 addrspace(4)* {{.*}}, i64

; Unsigned add:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_addi(i32
; CHECK-DAG: spir_func void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: spir_func void @__kmpc_atomic_fixed8_add(i64 addrspace(4)* {{.*}}, i64

; FP add:
; CHECK-DAG: call spir_func void @__kmpc_atomic_float4_add(float addrspace(4)* {{.*}}, float
; CHECK-DAG: call spir_func void @__kmpc_atomic_float8_add(double addrspace(4)* {{.*}}, double

; Signed min:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_mini(i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_min(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed8_min(i64 addrspace(4)* {{.*}}, i64

; Unsigned min:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_minj(i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4u_min(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed8u_min(i64 addrspace(4)* {{.*}}, i64

; FP min:
; CHECK-DAG: call spir_func void @__kmpc_atomic_float4_min(float addrspace(4)* {{.*}}, float
; CHECK-DAG: call spir_func void @__kmpc_atomic_float8_min(double addrspace(4)* {{.*}}, double

; Signed max:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_maxi(i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_max(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed8_max(i64 addrspace(4)* {{.*}}, i64

; Unsigned max:
; CHECK-DAG: call spir_func i32 @_Z20sub_group_reduce_maxj(i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4u_max(i32 addrspace(4)* {{.*}}, i32
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed8u_max(i64 addrspace(4)* {{.*}}, i64

; FP max:
; CHECK-DAG: call spir_func void @__kmpc_atomic_float4_max(float addrspace(4)* {{.*}}, float
; CHECK-DAG: call spir_func void @__kmpc_atomic_float8_max(double addrspace(4)* {{.*}}, double

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define hidden spir_func void @foo(i32 addrspace(4)* %x) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sadd16 = alloca i16, align 2
  %sadd16.ascast = addrspacecast i16* %sadd16 to i16 addrspace(4)*
  %smin16 = alloca i16, align 2
  %smin16.ascast = addrspacecast i16* %smin16 to i16 addrspace(4)*
  %smax16 = alloca i16, align 2
  %smax16.ascast = addrspacecast i16* %smax16 to i16 addrspace(4)*
  %sadd32 = alloca i32, align 4
  %sadd32.ascast = addrspacecast i32* %sadd32 to i32 addrspace(4)*
  %smin32 = alloca i32, align 4
  %smin32.ascast = addrspacecast i32* %smin32 to i32 addrspace(4)*
  %smax32 = alloca i32, align 4
  %smax32.ascast = addrspacecast i32* %smax32 to i32 addrspace(4)*
  %sadd64 = alloca i64, align 8
  %sadd64.ascast = addrspacecast i64* %sadd64 to i64 addrspace(4)*
  %smin64 = alloca i64, align 8
  %smin64.ascast = addrspacecast i64* %smin64 to i64 addrspace(4)*
  %smax64 = alloca i64, align 8
  %smax64.ascast = addrspacecast i64* %smax64 to i64 addrspace(4)*
  %uadd16 = alloca i16, align 2
  %uadd16.ascast = addrspacecast i16* %uadd16 to i16 addrspace(4)*
  %umin16 = alloca i16, align 2
  %umin16.ascast = addrspacecast i16* %umin16 to i16 addrspace(4)*
  %umax16 = alloca i16, align 2
  %umax16.ascast = addrspacecast i16* %umax16 to i16 addrspace(4)*
  %uadd32 = alloca i32, align 4
  %uadd32.ascast = addrspacecast i32* %uadd32 to i32 addrspace(4)*
  %umin32 = alloca i32, align 4
  %umin32.ascast = addrspacecast i32* %umin32 to i32 addrspace(4)*
  %umax32 = alloca i32, align 4
  %umax32.ascast = addrspacecast i32* %umax32 to i32 addrspace(4)*
  %uadd64 = alloca i64, align 8
  %uadd64.ascast = addrspacecast i64* %uadd64 to i64 addrspace(4)*
  %umin64 = alloca i64, align 8
  %umin64.ascast = addrspacecast i64* %umin64 to i64 addrspace(4)*
  %umax64 = alloca i64, align 8
  %umax64.ascast = addrspacecast i64* %umax64 to i64 addrspace(4)*
  %fadd32 = alloca float, align 4
  %fadd32.ascast = addrspacecast float* %fadd32 to float addrspace(4)*
  %fmin32 = alloca float, align 4
  %fmin32.ascast = addrspacecast float* %fmin32 to float addrspace(4)*
  %fmax32 = alloca float, align 4
  %fmax32.ascast = addrspacecast float* %fmax32 to float addrspace(4)*
  %fadd64 = alloca double, align 8
  %fadd64.ascast = addrspacecast double* %fadd64 to double addrspace(4)*
  %fmin64 = alloca double, align 8
  %fmin64.ascast = addrspacecast double* %fmin64 to double addrspace(4)*
  %fmax64 = alloca double, align 8
  %fmax64.ascast = addrspacecast double* %fmax64 to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %x.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %x.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %x.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8, !tbaa !9
  store i16 0, i16 addrspace(4)* %sadd16.ascast, align 2, !tbaa !13
  store i16 200, i16 addrspace(4)* %smin16.ascast, align 2, !tbaa !13
  store i16 0, i16 addrspace(4)* %smax16.ascast, align 2, !tbaa !13
  store i32 0, i32 addrspace(4)* %sadd32.ascast, align 4, !tbaa !15
  store i32 200, i32 addrspace(4)* %smin32.ascast, align 4, !tbaa !15
  store i32 0, i32 addrspace(4)* %smax32.ascast, align 4, !tbaa !15
  store i64 0, i64 addrspace(4)* %sadd64.ascast, align 8, !tbaa !17
  store i64 200, i64 addrspace(4)* %smin64.ascast, align 8, !tbaa !17
  store i64 0, i64 addrspace(4)* %smax64.ascast, align 8, !tbaa !17
  store i16 0, i16 addrspace(4)* %uadd16.ascast, align 2, !tbaa !13
  store i16 200, i16 addrspace(4)* %umin16.ascast, align 2, !tbaa !13
  store i16 0, i16 addrspace(4)* %umax16.ascast, align 2, !tbaa !13
  store i32 0, i32 addrspace(4)* %uadd32.ascast, align 4, !tbaa !15
  store i32 200, i32 addrspace(4)* %umin32.ascast, align 4, !tbaa !15
  store i32 0, i32 addrspace(4)* %umax32.ascast, align 4, !tbaa !15
  store i64 0, i64 addrspace(4)* %uadd64.ascast, align 8, !tbaa !17
  store i64 200, i64 addrspace(4)* %umin64.ascast, align 8, !tbaa !17
  store i64 0, i64 addrspace(4)* %umax64.ascast, align 8, !tbaa !17
  store float 0.000000e+00, float addrspace(4)* %fadd32.ascast, align 4, !tbaa !19
  store float 2.000000e+02, float addrspace(4)* %fmin32.ascast, align 4, !tbaa !19
  store float 0.000000e+00, float addrspace(4)* %fmax32.ascast, align 4, !tbaa !19
  store double 0.000000e+00, double addrspace(4)* %fadd64.ascast, align 8, !tbaa !21
  store double 2.000000e+02, double addrspace(4)* %fmin64.ascast, align 8, !tbaa !21
  store double 0.000000e+00, double addrspace(4)* %fmax64.ascast, align 8, !tbaa !21
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !15
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !15
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8, !tbaa !9
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8, !tbaa !9
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %2, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %sadd16.ascast, i16 addrspace(4)* %sadd16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sadd32.ascast, i32 addrspace(4)* %sadd32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %sadd64.ascast, i64 addrspace(4)* %sadd64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %uadd16.ascast, i16 addrspace(4)* %uadd16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %uadd32.ascast, i32 addrspace(4)* %uadd32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %uadd64.ascast, i64 addrspace(4)* %uadd64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %fadd32.ascast, float addrspace(4)* %fadd32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %fadd64.ascast, double addrspace(4)* %fadd64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %smin16.ascast, i16 addrspace(4)* %smin16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %smin32.ascast, i32 addrspace(4)* %smin32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %smin64.ascast, i64 addrspace(4)* %smin64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %umin16.ascast, i16 addrspace(4)* %umin16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %umin32.ascast, i32 addrspace(4)* %umin32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %umin64.ascast, i64 addrspace(4)* %umin64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %fmin32.ascast, float addrspace(4)* %fmin32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %fmin64.ascast, double addrspace(4)* %fmin64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %smax16.ascast, i16 addrspace(4)* %smax16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %smax32.ascast, i32 addrspace(4)* %smax32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %smax64.ascast, i64 addrspace(4)* %smax64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i16 addrspace(4)* %umax16.ascast, i16 addrspace(4)* %umax16.ascast, i64 2, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %umax32.ascast, i32 addrspace(4)* %umax32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i64 addrspace(4)* %umax64.ascast, i64 addrspace(4)* %umax64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %fmax32.ascast, float addrspace(4)* %fmax32.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %fmax64.ascast, double addrspace(4)* %fmax64.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %1, i32 addrspace(4)* %arrayidx, i64 400, i64 33, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  store i32 addrspace(4)* %1, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i16 addrspace(4)* %sadd16.ascast), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sadd32.ascast), "QUAL.OMP.REDUCTION.ADD"(i64 addrspace(4)* %sadd64.ascast), "QUAL.OMP.REDUCTION.ADD"(i16 addrspace(4)* %uadd16.ascast), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %uadd32.ascast), "QUAL.OMP.REDUCTION.ADD"(i64 addrspace(4)* %uadd64.ascast), "QUAL.OMP.REDUCTION.ADD"(float addrspace(4)* %fadd32.ascast), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %fadd64.ascast), "QUAL.OMP.REDUCTION.MIN"(i16 addrspace(4)* %smin16.ascast), "QUAL.OMP.REDUCTION.MIN"(i32 addrspace(4)* %smin32.ascast), "QUAL.OMP.REDUCTION.MIN"(i64 addrspace(4)* %smin64.ascast), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i16 addrspace(4)* %umin16.ascast), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i32 addrspace(4)* %umin32.ascast), "QUAL.OMP.REDUCTION.MIN:UNSIGNED"(i64 addrspace(4)* %umin64.ascast), "QUAL.OMP.REDUCTION.MIN"(float addrspace(4)* %fmin32.ascast), "QUAL.OMP.REDUCTION.MIN"(double addrspace(4)* %fmin64.ascast), "QUAL.OMP.REDUCTION.MAX"(i16 addrspace(4)* %smax16.ascast), "QUAL.OMP.REDUCTION.MAX"(i32 addrspace(4)* %smax32.ascast), "QUAL.OMP.REDUCTION.MAX"(i64 addrspace(4)* %smax64.ascast), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i16 addrspace(4)* %umax16.ascast), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i32 addrspace(4)* %umax32.ascast), "QUAL.OMP.REDUCTION.MAX:UNSIGNED"(i64 addrspace(4)* %umax64.ascast), "QUAL.OMP.REDUCTION.MAX"(float addrspace(4)* %fmax32.ascast), "QUAL.OMP.REDUCTION.MAX"(double addrspace(4)* %fmax64.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]
  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !15
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !15
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !15
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !15
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !15
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %9 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %10 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom = sext i32 %10 to i64
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %9, i64 %idxprom
  %11 = load i32, i32 addrspace(4)* %arrayidx1, align 4, !tbaa !15
  %12 = load i16, i16 addrspace(4)* %sadd16.ascast, align 2, !tbaa !13
  %conv = sext i16 %12 to i32
  %add2 = add nsw i32 %conv, %11
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, i16 addrspace(4)* %sadd16.ascast, align 2, !tbaa !13
  %13 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom4 = sext i32 %14 to i64
  %arrayidx5 = getelementptr inbounds i32, i32 addrspace(4)* %13, i64 %idxprom4
  %15 = load i32, i32 addrspace(4)* %arrayidx5, align 4, !tbaa !15
  %16 = load i32, i32 addrspace(4)* %sadd32.ascast, align 4, !tbaa !15
  %add6 = add nsw i32 %16, %15
  store i32 %add6, i32 addrspace(4)* %sadd32.ascast, align 4, !tbaa !15
  %17 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %18 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom7 = sext i32 %18 to i64
  %arrayidx8 = getelementptr inbounds i32, i32 addrspace(4)* %17, i64 %idxprom7
  %19 = load i32, i32 addrspace(4)* %arrayidx8, align 4, !tbaa !15
  %conv9 = sext i32 %19 to i64
  %20 = load i64, i64 addrspace(4)* %sadd64.ascast, align 8, !tbaa !17
  %add10 = add nsw i64 %20, %conv9
  store i64 %add10, i64 addrspace(4)* %sadd64.ascast, align 8, !tbaa !17
  %21 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %22 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom11 = sext i32 %22 to i64
  %arrayidx12 = getelementptr inbounds i32, i32 addrspace(4)* %21, i64 %idxprom11
  %23 = load i32, i32 addrspace(4)* %arrayidx12, align 4, !tbaa !15
  %24 = load i16, i16 addrspace(4)* %uadd16.ascast, align 2, !tbaa !13
  %conv13 = zext i16 %24 to i32
  %add14 = add nsw i32 %conv13, %23
  %conv15 = trunc i32 %add14 to i16
  store i16 %conv15, i16 addrspace(4)* %uadd16.ascast, align 2, !tbaa !13
  %25 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %26 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom16 = sext i32 %26 to i64
  %arrayidx17 = getelementptr inbounds i32, i32 addrspace(4)* %25, i64 %idxprom16
  %27 = load i32, i32 addrspace(4)* %arrayidx17, align 4, !tbaa !15
  %28 = load i32, i32 addrspace(4)* %uadd32.ascast, align 4, !tbaa !15
  %add18 = add i32 %28, %27
  store i32 %add18, i32 addrspace(4)* %uadd32.ascast, align 4, !tbaa !15
  %29 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %30 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom19 = sext i32 %30 to i64
  %arrayidx20 = getelementptr inbounds i32, i32 addrspace(4)* %29, i64 %idxprom19
  %31 = load i32, i32 addrspace(4)* %arrayidx20, align 4, !tbaa !15
  %conv21 = sext i32 %31 to i64
  %32 = load i64, i64 addrspace(4)* %uadd64.ascast, align 8, !tbaa !17
  %add22 = add i64 %32, %conv21
  store i64 %add22, i64 addrspace(4)* %uadd64.ascast, align 8, !tbaa !17
  %33 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %34 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom23 = sext i32 %34 to i64
  %arrayidx24 = getelementptr inbounds i32, i32 addrspace(4)* %33, i64 %idxprom23
  %35 = load i32, i32 addrspace(4)* %arrayidx24, align 4, !tbaa !15
  %conv25 = sitofp i32 %35 to float
  %36 = load float, float addrspace(4)* %fadd32.ascast, align 4, !tbaa !19
  %add26 = fadd fast float %36, %conv25
  store float %add26, float addrspace(4)* %fadd32.ascast, align 4, !tbaa !19
  %37 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %38 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom27 = sext i32 %38 to i64
  %arrayidx28 = getelementptr inbounds i32, i32 addrspace(4)* %37, i64 %idxprom27
  %39 = load i32, i32 addrspace(4)* %arrayidx28, align 4, !tbaa !15
  %conv29 = sitofp i32 %39 to double
  %40 = load double, double addrspace(4)* %fadd64.ascast, align 8, !tbaa !21
  %add30 = fadd fast double %40, %conv29
  store double %add30, double addrspace(4)* %fadd64.ascast, align 8, !tbaa !21
  %41 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %42 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom31 = sext i32 %42 to i64
  %arrayidx32 = getelementptr inbounds i32, i32 addrspace(4)* %41, i64 %idxprom31
  %43 = load i32, i32 addrspace(4)* %arrayidx32, align 4, !tbaa !15
  %44 = load i16, i16 addrspace(4)* %smin16.ascast, align 2, !tbaa !13
  %conv33 = sext i16 %44 to i32
  %cmp34 = icmp slt i32 %43, %conv33
  br i1 %cmp34, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  %45 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %46 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom36 = sext i32 %46 to i64
  %arrayidx37 = getelementptr inbounds i32, i32 addrspace(4)* %45, i64 %idxprom36
  %47 = load i32, i32 addrspace(4)* %arrayidx37, align 4, !tbaa !15
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  %48 = load i16, i16 addrspace(4)* %smin16.ascast, align 2, !tbaa !13
  %conv38 = sext i16 %48 to i32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %47, %cond.true ], [ %conv38, %cond.false ]
  %conv39 = trunc i32 %cond to i16
  store i16 %conv39, i16 addrspace(4)* %smin16.ascast, align 2, !tbaa !13
  %49 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %50 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom40 = sext i32 %50 to i64
  %arrayidx41 = getelementptr inbounds i32, i32 addrspace(4)* %49, i64 %idxprom40
  %51 = load i32, i32 addrspace(4)* %arrayidx41, align 4, !tbaa !15
  %52 = load i32, i32 addrspace(4)* %smin32.ascast, align 4, !tbaa !15
  %cmp42 = icmp slt i32 %51, %52
  br i1 %cmp42, label %cond.true44, label %cond.false47

cond.true44:                                      ; preds = %cond.end
  %53 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %54 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom45 = sext i32 %54 to i64
  %arrayidx46 = getelementptr inbounds i32, i32 addrspace(4)* %53, i64 %idxprom45
  %55 = load i32, i32 addrspace(4)* %arrayidx46, align 4, !tbaa !15
  br label %cond.end48

cond.false47:                                     ; preds = %cond.end
  %56 = load i32, i32 addrspace(4)* %smin32.ascast, align 4, !tbaa !15
  br label %cond.end48

cond.end48:                                       ; preds = %cond.false47, %cond.true44
  %cond49 = phi i32 [ %55, %cond.true44 ], [ %56, %cond.false47 ]
  store i32 %cond49, i32 addrspace(4)* %smin32.ascast, align 4, !tbaa !15
  %57 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %58 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom50 = sext i32 %58 to i64
  %arrayidx51 = getelementptr inbounds i32, i32 addrspace(4)* %57, i64 %idxprom50
  %59 = load i32, i32 addrspace(4)* %arrayidx51, align 4, !tbaa !15
  %conv52 = sext i32 %59 to i64
  %60 = load i64, i64 addrspace(4)* %smin64.ascast, align 8, !tbaa !17
  %cmp53 = icmp slt i64 %conv52, %60
  br i1 %cmp53, label %cond.true55, label %cond.false59

cond.true55:                                      ; preds = %cond.end48
  %61 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %62 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom56 = sext i32 %62 to i64
  %arrayidx57 = getelementptr inbounds i32, i32 addrspace(4)* %61, i64 %idxprom56
  %63 = load i32, i32 addrspace(4)* %arrayidx57, align 4, !tbaa !15
  %conv58 = sext i32 %63 to i64
  br label %cond.end60

cond.false59:                                     ; preds = %cond.end48
  %64 = load i64, i64 addrspace(4)* %smin64.ascast, align 8, !tbaa !17
  br label %cond.end60

cond.end60:                                       ; preds = %cond.false59, %cond.true55
  %cond61 = phi i64 [ %conv58, %cond.true55 ], [ %64, %cond.false59 ]
  store i64 %cond61, i64 addrspace(4)* %smin64.ascast, align 8, !tbaa !17
  %65 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %66 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom62 = sext i32 %66 to i64
  %arrayidx63 = getelementptr inbounds i32, i32 addrspace(4)* %65, i64 %idxprom62
  %67 = load i32, i32 addrspace(4)* %arrayidx63, align 4, !tbaa !15
  %68 = load i16, i16 addrspace(4)* %umin16.ascast, align 2, !tbaa !13
  %conv64 = zext i16 %68 to i32
  %cmp65 = icmp slt i32 %67, %conv64
  br i1 %cmp65, label %cond.true67, label %cond.false70

cond.true67:                                      ; preds = %cond.end60
  %69 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %70 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom68 = sext i32 %70 to i64
  %arrayidx69 = getelementptr inbounds i32, i32 addrspace(4)* %69, i64 %idxprom68
  %71 = load i32, i32 addrspace(4)* %arrayidx69, align 4, !tbaa !15
  br label %cond.end72

cond.false70:                                     ; preds = %cond.end60
  %72 = load i16, i16 addrspace(4)* %umin16.ascast, align 2, !tbaa !13
  %conv71 = zext i16 %72 to i32
  br label %cond.end72

cond.end72:                                       ; preds = %cond.false70, %cond.true67
  %cond73 = phi i32 [ %71, %cond.true67 ], [ %conv71, %cond.false70 ]
  %conv74 = trunc i32 %cond73 to i16
  store i16 %conv74, i16 addrspace(4)* %umin16.ascast, align 2, !tbaa !13
  %73 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %74 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom75 = sext i32 %74 to i64
  %arrayidx76 = getelementptr inbounds i32, i32 addrspace(4)* %73, i64 %idxprom75
  %75 = load i32, i32 addrspace(4)* %arrayidx76, align 4, !tbaa !15
  %76 = load i32, i32 addrspace(4)* %umin32.ascast, align 4, !tbaa !15
  %cmp77 = icmp ult i32 %75, %76
  br i1 %cmp77, label %cond.true79, label %cond.false82

cond.true79:                                      ; preds = %cond.end72
  %77 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %78 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom80 = sext i32 %78 to i64
  %arrayidx81 = getelementptr inbounds i32, i32 addrspace(4)* %77, i64 %idxprom80
  %79 = load i32, i32 addrspace(4)* %arrayidx81, align 4, !tbaa !15
  br label %cond.end83

cond.false82:                                     ; preds = %cond.end72
  %80 = load i32, i32 addrspace(4)* %umin32.ascast, align 4, !tbaa !15
  br label %cond.end83

cond.end83:                                       ; preds = %cond.false82, %cond.true79
  %cond84 = phi i32 [ %79, %cond.true79 ], [ %80, %cond.false82 ]
  store i32 %cond84, i32 addrspace(4)* %umin32.ascast, align 4, !tbaa !15
  %81 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %82 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom85 = sext i32 %82 to i64
  %arrayidx86 = getelementptr inbounds i32, i32 addrspace(4)* %81, i64 %idxprom85
  %83 = load i32, i32 addrspace(4)* %arrayidx86, align 4, !tbaa !15
  %conv87 = sext i32 %83 to i64
  %84 = load i64, i64 addrspace(4)* %umin64.ascast, align 8, !tbaa !17
  %cmp88 = icmp ult i64 %conv87, %84
  br i1 %cmp88, label %cond.true90, label %cond.false94

cond.true90:                                      ; preds = %cond.end83
  %85 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %86 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom91 = sext i32 %86 to i64
  %arrayidx92 = getelementptr inbounds i32, i32 addrspace(4)* %85, i64 %idxprom91
  %87 = load i32, i32 addrspace(4)* %arrayidx92, align 4, !tbaa !15
  %conv93 = sext i32 %87 to i64
  br label %cond.end95

cond.false94:                                     ; preds = %cond.end83
  %88 = load i64, i64 addrspace(4)* %umin64.ascast, align 8, !tbaa !17
  br label %cond.end95

cond.end95:                                       ; preds = %cond.false94, %cond.true90
  %cond96 = phi i64 [ %conv93, %cond.true90 ], [ %88, %cond.false94 ]
  store i64 %cond96, i64 addrspace(4)* %umin64.ascast, align 8, !tbaa !17
  %89 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %90 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom97 = sext i32 %90 to i64
  %arrayidx98 = getelementptr inbounds i32, i32 addrspace(4)* %89, i64 %idxprom97
  %91 = load i32, i32 addrspace(4)* %arrayidx98, align 4, !tbaa !15
  %conv99 = sitofp i32 %91 to float
  %92 = load float, float addrspace(4)* %fmin32.ascast, align 4, !tbaa !19
  %cmp100 = fcmp fast olt float %conv99, %92
  br i1 %cmp100, label %cond.true102, label %cond.false106

cond.true102:                                     ; preds = %cond.end95
  %93 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %94 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom103 = sext i32 %94 to i64
  %arrayidx104 = getelementptr inbounds i32, i32 addrspace(4)* %93, i64 %idxprom103
  %95 = load i32, i32 addrspace(4)* %arrayidx104, align 4, !tbaa !15
  %conv105 = sitofp i32 %95 to float
  br label %cond.end107

cond.false106:                                    ; preds = %cond.end95
  %96 = load float, float addrspace(4)* %fmin32.ascast, align 4, !tbaa !19
  br label %cond.end107

cond.end107:                                      ; preds = %cond.false106, %cond.true102
  %cond108 = phi fast float [ %conv105, %cond.true102 ], [ %96, %cond.false106 ]
  store float %cond108, float addrspace(4)* %fmin32.ascast, align 4, !tbaa !19
  %97 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %98 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom109 = sext i32 %98 to i64
  %arrayidx110 = getelementptr inbounds i32, i32 addrspace(4)* %97, i64 %idxprom109
  %99 = load i32, i32 addrspace(4)* %arrayidx110, align 4, !tbaa !15
  %conv111 = sitofp i32 %99 to double
  %100 = load double, double addrspace(4)* %fmin64.ascast, align 8, !tbaa !21
  %cmp112 = fcmp fast olt double %conv111, %100
  br i1 %cmp112, label %cond.true114, label %cond.false118

cond.true114:                                     ; preds = %cond.end107
  %101 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %102 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom115 = sext i32 %102 to i64
  %arrayidx116 = getelementptr inbounds i32, i32 addrspace(4)* %101, i64 %idxprom115
  %103 = load i32, i32 addrspace(4)* %arrayidx116, align 4, !tbaa !15
  %conv117 = sitofp i32 %103 to double
  br label %cond.end119

cond.false118:                                    ; preds = %cond.end107
  %104 = load double, double addrspace(4)* %fmin64.ascast, align 8, !tbaa !21
  br label %cond.end119

cond.end119:                                      ; preds = %cond.false118, %cond.true114
  %cond120 = phi fast double [ %conv117, %cond.true114 ], [ %104, %cond.false118 ]
  store double %cond120, double addrspace(4)* %fmin64.ascast, align 8, !tbaa !21
  %105 = load i16, i16 addrspace(4)* %smax16.ascast, align 2, !tbaa !13
  %conv121 = sext i16 %105 to i32
  %106 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %107 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom122 = sext i32 %107 to i64
  %arrayidx123 = getelementptr inbounds i32, i32 addrspace(4)* %106, i64 %idxprom122
  %108 = load i32, i32 addrspace(4)* %arrayidx123, align 4, !tbaa !15
  %cmp124 = icmp slt i32 %conv121, %108
  br i1 %cmp124, label %cond.true126, label %cond.false129

cond.true126:                                     ; preds = %cond.end119
  %109 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %110 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom127 = sext i32 %110 to i64
  %arrayidx128 = getelementptr inbounds i32, i32 addrspace(4)* %109, i64 %idxprom127
  %111 = load i32, i32 addrspace(4)* %arrayidx128, align 4, !tbaa !15
  br label %cond.end131

cond.false129:                                    ; preds = %cond.end119
  %112 = load i16, i16 addrspace(4)* %smax16.ascast, align 2, !tbaa !13
  %conv130 = sext i16 %112 to i32
  br label %cond.end131

cond.end131:                                      ; preds = %cond.false129, %cond.true126
  %cond132 = phi i32 [ %111, %cond.true126 ], [ %conv130, %cond.false129 ]
  %conv133 = trunc i32 %cond132 to i16
  store i16 %conv133, i16 addrspace(4)* %smax16.ascast, align 2, !tbaa !13
  %113 = load i32, i32 addrspace(4)* %smax32.ascast, align 4, !tbaa !15
  %114 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %115 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom134 = sext i32 %115 to i64
  %arrayidx135 = getelementptr inbounds i32, i32 addrspace(4)* %114, i64 %idxprom134
  %116 = load i32, i32 addrspace(4)* %arrayidx135, align 4, !tbaa !15
  %cmp136 = icmp slt i32 %113, %116
  br i1 %cmp136, label %cond.true138, label %cond.false141

cond.true138:                                     ; preds = %cond.end131
  %117 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %118 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom139 = sext i32 %118 to i64
  %arrayidx140 = getelementptr inbounds i32, i32 addrspace(4)* %117, i64 %idxprom139
  %119 = load i32, i32 addrspace(4)* %arrayidx140, align 4, !tbaa !15
  br label %cond.end142

cond.false141:                                    ; preds = %cond.end131
  %120 = load i32, i32 addrspace(4)* %smax32.ascast, align 4, !tbaa !15
  br label %cond.end142

cond.end142:                                      ; preds = %cond.false141, %cond.true138
  %cond143 = phi i32 [ %119, %cond.true138 ], [ %120, %cond.false141 ]
  store i32 %cond143, i32 addrspace(4)* %smax32.ascast, align 4, !tbaa !15
  %121 = load i64, i64 addrspace(4)* %smax64.ascast, align 8, !tbaa !17
  %122 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %123 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom144 = sext i32 %123 to i64
  %arrayidx145 = getelementptr inbounds i32, i32 addrspace(4)* %122, i64 %idxprom144
  %124 = load i32, i32 addrspace(4)* %arrayidx145, align 4, !tbaa !15
  %conv146 = sext i32 %124 to i64
  %cmp147 = icmp slt i64 %121, %conv146
  br i1 %cmp147, label %cond.true149, label %cond.false153

cond.true149:                                     ; preds = %cond.end142
  %125 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %126 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom150 = sext i32 %126 to i64
  %arrayidx151 = getelementptr inbounds i32, i32 addrspace(4)* %125, i64 %idxprom150
  %127 = load i32, i32 addrspace(4)* %arrayidx151, align 4, !tbaa !15
  %conv152 = sext i32 %127 to i64
  br label %cond.end154

cond.false153:                                    ; preds = %cond.end142
  %128 = load i64, i64 addrspace(4)* %smax64.ascast, align 8, !tbaa !17
  br label %cond.end154

cond.end154:                                      ; preds = %cond.false153, %cond.true149
  %cond155 = phi i64 [ %conv152, %cond.true149 ], [ %128, %cond.false153 ]
  store i64 %cond155, i64 addrspace(4)* %smax64.ascast, align 8, !tbaa !17
  %129 = load i16, i16 addrspace(4)* %umax16.ascast, align 2, !tbaa !13
  %conv156 = zext i16 %129 to i32
  %130 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %131 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom157 = sext i32 %131 to i64
  %arrayidx158 = getelementptr inbounds i32, i32 addrspace(4)* %130, i64 %idxprom157
  %132 = load i32, i32 addrspace(4)* %arrayidx158, align 4, !tbaa !15
  %cmp159 = icmp slt i32 %conv156, %132
  br i1 %cmp159, label %cond.true161, label %cond.false164

cond.true161:                                     ; preds = %cond.end154
  %133 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %134 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom162 = sext i32 %134 to i64
  %arrayidx163 = getelementptr inbounds i32, i32 addrspace(4)* %133, i64 %idxprom162
  %135 = load i32, i32 addrspace(4)* %arrayidx163, align 4, !tbaa !15
  br label %cond.end166

cond.false164:                                    ; preds = %cond.end154
  %136 = load i16, i16 addrspace(4)* %umax16.ascast, align 2, !tbaa !13
  %conv165 = zext i16 %136 to i32
  br label %cond.end166

cond.end166:                                      ; preds = %cond.false164, %cond.true161
  %cond167 = phi i32 [ %135, %cond.true161 ], [ %conv165, %cond.false164 ]
  %conv168 = trunc i32 %cond167 to i16
  store i16 %conv168, i16 addrspace(4)* %umax16.ascast, align 2, !tbaa !13
  %137 = load i32, i32 addrspace(4)* %umax32.ascast, align 4, !tbaa !15
  %138 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %139 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom169 = sext i32 %139 to i64
  %arrayidx170 = getelementptr inbounds i32, i32 addrspace(4)* %138, i64 %idxprom169
  %140 = load i32, i32 addrspace(4)* %arrayidx170, align 4, !tbaa !15
  %cmp171 = icmp ult i32 %137, %140
  br i1 %cmp171, label %cond.true173, label %cond.false176

cond.true173:                                     ; preds = %cond.end166
  %141 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %142 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom174 = sext i32 %142 to i64
  %arrayidx175 = getelementptr inbounds i32, i32 addrspace(4)* %141, i64 %idxprom174
  %143 = load i32, i32 addrspace(4)* %arrayidx175, align 4, !tbaa !15
  br label %cond.end177

cond.false176:                                    ; preds = %cond.end166
  %144 = load i32, i32 addrspace(4)* %umax32.ascast, align 4, !tbaa !15
  br label %cond.end177

cond.end177:                                      ; preds = %cond.false176, %cond.true173
  %cond178 = phi i32 [ %143, %cond.true173 ], [ %144, %cond.false176 ]
  store i32 %cond178, i32 addrspace(4)* %umax32.ascast, align 4, !tbaa !15
  %145 = load i64, i64 addrspace(4)* %umax64.ascast, align 8, !tbaa !17
  %146 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %147 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom179 = sext i32 %147 to i64
  %arrayidx180 = getelementptr inbounds i32, i32 addrspace(4)* %146, i64 %idxprom179
  %148 = load i32, i32 addrspace(4)* %arrayidx180, align 4, !tbaa !15
  %conv181 = sext i32 %148 to i64
  %cmp182 = icmp ult i64 %145, %conv181
  br i1 %cmp182, label %cond.true184, label %cond.false188

cond.true184:                                     ; preds = %cond.end177
  %149 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %150 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom185 = sext i32 %150 to i64
  %arrayidx186 = getelementptr inbounds i32, i32 addrspace(4)* %149, i64 %idxprom185
  %151 = load i32, i32 addrspace(4)* %arrayidx186, align 4, !tbaa !15
  %conv187 = sext i32 %151 to i64
  br label %cond.end189

cond.false188:                                    ; preds = %cond.end177
  %152 = load i64, i64 addrspace(4)* %umax64.ascast, align 8, !tbaa !17
  br label %cond.end189

cond.end189:                                      ; preds = %cond.false188, %cond.true184
  %cond190 = phi i64 [ %conv187, %cond.true184 ], [ %152, %cond.false188 ]
  store i64 %cond190, i64 addrspace(4)* %umax64.ascast, align 8, !tbaa !17
  %153 = load float, float addrspace(4)* %fmax32.ascast, align 4, !tbaa !19
  %154 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %155 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom191 = sext i32 %155 to i64
  %arrayidx192 = getelementptr inbounds i32, i32 addrspace(4)* %154, i64 %idxprom191
  %156 = load i32, i32 addrspace(4)* %arrayidx192, align 4, !tbaa !15
  %conv193 = sitofp i32 %156 to float
  %cmp194 = fcmp fast olt float %153, %conv193
  br i1 %cmp194, label %cond.true196, label %cond.false200

cond.true196:                                     ; preds = %cond.end189
  %157 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %158 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom197 = sext i32 %158 to i64
  %arrayidx198 = getelementptr inbounds i32, i32 addrspace(4)* %157, i64 %idxprom197
  %159 = load i32, i32 addrspace(4)* %arrayidx198, align 4, !tbaa !15
  %conv199 = sitofp i32 %159 to float
  br label %cond.end201

cond.false200:                                    ; preds = %cond.end189
  %160 = load float, float addrspace(4)* %fmax32.ascast, align 4, !tbaa !19
  br label %cond.end201

cond.end201:                                      ; preds = %cond.false200, %cond.true196
  %cond202 = phi fast float [ %conv199, %cond.true196 ], [ %160, %cond.false200 ]
  store float %cond202, float addrspace(4)* %fmax32.ascast, align 4, !tbaa !19
  %161 = load double, double addrspace(4)* %fmax64.ascast, align 8, !tbaa !21
  %162 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %163 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom203 = sext i32 %163 to i64
  %arrayidx204 = getelementptr inbounds i32, i32 addrspace(4)* %162, i64 %idxprom203
  %164 = load i32, i32 addrspace(4)* %arrayidx204, align 4, !tbaa !15
  %conv205 = sitofp i32 %164 to double
  %cmp206 = fcmp fast olt double %161, %conv205
  br i1 %cmp206, label %cond.true208, label %cond.false212

cond.true208:                                     ; preds = %cond.end201
  %165 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8, !tbaa !9
  %166 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !15
  %idxprom209 = sext i32 %166 to i64
  %arrayidx210 = getelementptr inbounds i32, i32 addrspace(4)* %165, i64 %idxprom209
  %167 = load i32, i32 addrspace(4)* %arrayidx210, align 4, !tbaa !15
  %conv211 = sitofp i32 %167 to double
  br label %cond.end213

cond.false212:                                    ; preds = %cond.end201
  %168 = load double, double addrspace(4)* %fmax64.ascast, align 8, !tbaa !21
  br label %cond.end213

cond.end213:                                      ; preds = %cond.false212, %cond.true208
  %cond214 = phi fast double [ %conv211, %cond.true208 ], [ %168, %cond.false212 ]
  store double %cond214, double addrspace(4)* %fmax64.ascast, align 8, !tbaa !21
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end213
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %169 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !15
  %add215 = add nsw i32 %169, 1
  store i32 %add215, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !15
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder i32 addrspace(4)* %i.ascast, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 1, 0 }
  uselistorder i16 addrspace(4)* %sadd16.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i16 addrspace(4)* %smin16.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i16 addrspace(4)* %smax16.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i32 addrspace(4)* %sadd32.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i32 addrspace(4)* %smin32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i32 addrspace(4)* %smax32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i64 addrspace(4)* %sadd64.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i64 addrspace(4)* %smin64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i64 addrspace(4)* %smax64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i16 addrspace(4)* %uadd16.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i16 addrspace(4)* %umin16.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i16 addrspace(4)* %umax16.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i32 addrspace(4)* %uadd32.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i32 addrspace(4)* %umin32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i32 addrspace(4)* %umax32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i64 addrspace(4)* %uadd64.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder i64 addrspace(4)* %umin64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i64 addrspace(4)* %umax64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder float addrspace(4)* %fadd32.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder float addrspace(4)* %fmin32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder float addrspace(4)* %fmax32.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder double addrspace(4)* %fadd64.ascast, { 3, 4, 2, 0, 1, 5 }
  uselistorder double addrspace(4)* %fmin64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder double addrspace(4)* %fmax64.ascast, { 3, 4, 5, 2, 0, 1, 6 }
  uselistorder i32 addrspace(4)* %.omp.lb.ascast, { 2, 1, 0, 3 }
  uselistorder i32 addrspace(4)* %.omp.ub.ascast, { 2, 1, 0, 3 }
  uselistorder i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 1, 42, 0 }
  uselistorder i32 addrspace(4)* %.omp.iv.ascast, { 2, 3, 4, 5, 6, 1, 0 }
  uselistorder i32 addrspace(4)* %1, { 1, 0 }
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; uselistorder directives
uselistorder i32 0, { 1, 0, 2, 3, 4, 5, 6 }
uselistorder void (token)* @llvm.directive.region.exit, { 1, 0 }

attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!8}

!0 = !{i32 0, i32 2053, i32 12460681, !"_Z3foo", i32 22, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
!8 = !{!"clang version 12.0.0"}
!9 = !{!10, !10, i64 0}
!10 = !{!"pointer@_ZTSPi", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"short", !11, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !11, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"long long", !11, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"float", !11, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"double", !11, i64 0}

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
