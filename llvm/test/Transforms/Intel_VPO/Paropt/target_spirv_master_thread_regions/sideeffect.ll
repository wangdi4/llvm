; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;#pragma omp target map(to:f[0:100]) map(from:r[0:100])
;  for (i = 0; i < 100; i++) {
;    const float sin_fi = sinf(f[i]);
;    const float cos_fi = cosf(f[i]);
;#pragma omp parallel
;    r[i] = sin_fi + cos_fi;
;  }

; Check that calls to sinf and cosf are guarded by is_master check, and the
; return value is broadcast to all other "threads" if needed for use in a
; parallel region.


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z3fooPfS_(float addrspace(4)* noalias %f, float addrspace(4)* noalias %r) {
entry:
  %f.addr = alloca float addrspace(4)*, align 8
  %f.addr.ascast = addrspacecast float addrspace(4)** %f.addr to float addrspace(4)* addrspace(4)*
  %r.addr = alloca float addrspace(4)*, align 8
  %r.addr.ascast = addrspacecast float addrspace(4)** %r.addr to float addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %sin_fi = alloca float, align 4
  %cos_fi = alloca float, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sin_fi.ascast = addrspacecast float* %sin_fi to float addrspace(4)*
  %cos_fi.ascast = addrspacecast float* %cos_fi to float addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(float addrspace(4)* %f, float addrspace(4)* %f, i64 400, i64 33, i8* null, i8* null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.FROM"(float addrspace(4)* %r, float addrspace(4)* %r, i64 400, i64 34, i8* null, i8* null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(float addrspace(4)* %sin_fi.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(float addrspace(4)* %cos_fi.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(float addrspace(4)* addrspace(4)* %f.addr.ascast, float addrspace(4)* null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(float addrspace(4)* addrspace(4)* %r.addr.ascast, float addrspace(4)* null, i32 1) ]
  store float addrspace(4)* %f, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  store float addrspace(4)* %r, float addrspace(4)* addrspace(4)* %r.addr.ascast, align 8
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %cmp = icmp slt i32 %1, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx2 = getelementptr inbounds float, float addrspace(4)* %2, i64 %idxprom
  %4 = load float, float addrspace(4)* %arrayidx2, align 4


  %call = call spir_func float @sinf(float %4)
; Check that %call is stored to a global, loaded in other threads, and stored to
; a thread-local value in each thread
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %call = call spir_func float @sinf(float %{{.*}})
; CHECK: store float %call, float addrspace(3)* @call.broadcast.ptr.__local
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %call.new = load float, float addrspace(3)* @call.broadcast.ptr.__local
; CHECK: store float %call.new, float* %sin_fi.ascast.priv, align 4

  store float %call, float addrspace(4)* %sin_fi.ascast, align 4
  %5 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %f.addr.ascast, align 8
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom3 = sext i32 %6 to i64
  %arrayidx4 = getelementptr inbounds float, float addrspace(4)* %5, i64 %idxprom3
  %7 = load float, float addrspace(4)* %arrayidx4, align 4

  %call5 = call spir_func float @cosf(float %7)
; Check that %call5 is captured and then broadcast
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %call5 = call spir_func float @cosf(float %{{.*}})
; CHECK: store float %call5, float addrspace(3)* @call5.broadcast.ptr.__local
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %call5.new = load float, float addrspace(3)* @call5.broadcast.ptr.__local
; CHECK: store float %call5.new, float* %cos_fi.ascast.priv, align 4

  store float %call5, float addrspace(4)* %cos_fi.ascast, align 4
  br label %DIR.OMP.PARALLEL

DIR.OMP.PARALLEL:
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(float addrspace(4)* addrspace(4)* %r.addr.ascast, float addrspace(4)* null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(float addrspace(4)* %sin_fi.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(float addrspace(4)* %cos_fi.ascast, float 0.000000e+00, i32 1) ]

  %9 = load float, float addrspace(4)* %sin_fi.ascast, align 4
  %10 = load float, float addrspace(4)* %cos_fi.ascast, align 4
  %add = fadd float %9, %10
; CHECK:  [[SIN_FI:%.*]] = load float, float* %sin_fi.ascast.priv, align 4
; CHECK:  [[COS_FI:%.*]] = load float, float* %cos_fi.ascast.priv, align 4
; CHECK: %add = fadd float [[SIN_FI]], [[COS_FI]]

  %11 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %r.addr.ascast, align 8
  %12 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom6 = sext i32 %12 to i64
  %arrayidx7 = getelementptr inbounds float, float addrspace(4)* %11, i64 %idxprom6
  store float %add, float addrspace(4)* %arrayidx7, align 4
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local spir_func float @sinf(float)
declare dso_local spir_func float @cosf(float)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64770, i32 1077853693, !"_Z3fooPfS_", i32 6, i32 0, i32 0}
