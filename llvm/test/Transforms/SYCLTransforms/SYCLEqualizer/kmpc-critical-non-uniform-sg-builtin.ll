; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; Check that intel_vec_len_hint metadata isn't generated for the kernel although
; it contains __kmpc_critical calls. The kernel calls non-uniform subgroup
; reduction builtin and not all workitems in the subgroup is active. This is
; unsupported by subgroup emulation, therefore the kernel must be vectorized.

; CHECK: define void @__omp_offloading_811_2373a41__ZL5test4iPdS_S__l19(
; CHECK-NOT: !intel_vec_len_hint

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

declare spir_func double @_Z20sub_group_reduce_addd(double)

define spir_kernel void @__omp_offloading_811_2373a41__ZL5test4iPdS_S__l19(ptr addrspace(1) noalias %sum.ascast, i64 %k.ascast.val.zext, i64 %.omp.lb.ascast.val93.zext, i64 %.omp.ub.ascast.val.zext, i64 %.capture_expr.0.ascast.val.zext) {
DIR.OMP.TEAMS.6:
  %0 = addrspacecast ptr addrspace(1) null to ptr addrspace(4)
  br label %DIR.OMP.END.PARALLEL.LOOP.10.loopexit

omp.inner.for.body.preheader:                     ; No predecessors!
  br label %DIR.OMP.END.PARALLEL.LOOP.10.loopexit

DIR.OMP.END.PARALLEL.LOOP.10.loopexit:            ; preds = %omp.inner.for.body.preheader, %DIR.OMP.TEAMS.6
  %sum.ascast.red.0 = phi double [ 0.000000e+00, %DIR.OMP.TEAMS.6 ], [ 0.000000e+00, %omp.inner.for.body.preheader ]
  call spir_func void @__kmpc_critical(ptr addrspace(4) %0)
  %1 = call spir_func double @_Z20sub_group_reduce_addd(double %sum.ascast.red.0)
  call spir_func void @__kmpc_end_critical(ptr addrspace(4) %0)
  ret void
}

declare spir_func void @__kmpc_critical(ptr addrspace(4))

declare spir_func void @__kmpc_end_critical(ptr addrspace(4))

; DEBUGIFY-NOT: WARNING
