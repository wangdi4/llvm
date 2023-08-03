; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; Check that intel_vec_len_hint metadata isn't generated for the kernel although
; it contains __kmpc_critical calls. The kernel calls non-uniform subgroup
; reduction builtin and not all workitems in the subgroup is active. This is
; unsupported by subgroup emulation, therefore the kernel must be vectorized.

; CHECK: define void @__omp_offloading_804_2620649__ZL5test4iPdS_S__l19(
; CHECK-NOT: !intel_vec_len_hint

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__tgt_offload_entry.0 = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }

@__omp_offloading_entries_table = external addrspace(1) constant [1 x %struct.__tgt_offload_entry.0]

declare spir_func i32 @_Z18get_sub_group_sizev()

declare spir_func i32 @_Z22get_sub_group_local_idv()

declare spir_func double @_Z32sub_group_non_uniform_reduce_addd(double)

define spir_kernel void @__omp_offloading_804_2620649__ZL5test4iPdS_S__l19(ptr addrspace(1) noalias %sum.ascast, i64 %k.ascast.val.zext, i64 %.omp.lb.ascast.val101.zext, i64 %.omp.ub.ascast.val.zext, i64 %.capture_expr.0.ascast.val.zext) {
DIR.OMP.TEAMS.6:
  %0 = addrspacecast ptr addrspace(1) null to ptr addrspace(4)
  br i1 false, label %DIR.OMP.END.PARALLEL.LOOP.10.loopexit, label %DIR.OMP.END.TEAMS.11

omp.inner.for.body.preheader:
  br label %DIR.OMP.END.PARALLEL.LOOP.10.loopexit

DIR.OMP.END.PARALLEL.LOOP.10.loopexit:            ; preds = %omp.inner.for.body.preheader, %DIR.OMP.TEAMS.6
  %sum.ascast.red.0 = phi double [ 0.000000e+00, %omp.inner.for.body.preheader ], [ 0.000000e+00, %DIR.OMP.TEAMS.6 ]
  br label %DIR.OMP.END.PARALLEL.LOOP.1075.split88.split

DIR.OMP.END.PARALLEL.LOOP.1075.split88.split: ; preds = %DIR.OMP.END.PARALLEL.LOOP.10.loopexit
  %1 = call spir_func i32 @_Z18get_sub_group_sizev() #0
  %exit.pred.not5.not = icmp eq i32 %1, 0
  br i1 %exit.pred.not5.not, label %DIR.OMP.END.PARALLEL.LOOP.1075.split.split.split90, label %.lr.ph7

.lr.ph7:                                          ; preds = %DIR.OMP.END.PARALLEL.LOOP.1075.split88.split
  br label %2

2:                                                ; preds = %DIR.OMP.END.PARALLEL.LOOP.1075.split.split, %.lr.ph7
  %3 = call spir_func i32 @_Z22get_sub_group_local_idv()
  %skip.pred.not = icmp eq i32 0, %3
  br i1 %skip.pred.not, label %red.update.body.preheader, label %DIR.OMP.END.PARALLEL.LOOP.1075.split.split

DIR.OMP.END.PARALLEL.LOOP.1075.split.split: ; preds = %red.update.body.preheader, %2
  br label %2

DIR.OMP.END.PARALLEL.LOOP.1075.split.split.split90: ; preds = %DIR.OMP.END.PARALLEL.LOOP.1075.split88.split
  call spir_func void @__kmpc_critical(ptr addrspace(4) %0) #0
  call spir_func void @__kmpc_end_critical(ptr addrspace(4) %0)
  br label %DIR.OMP.END.TEAMS.11

red.update.body.preheader:                        ; preds = %1
  %4 = call spir_func double @_Z32sub_group_non_uniform_reduce_addd(double %sum.ascast.red.0)
  br label %DIR.OMP.END.PARALLEL.LOOP.1075.split.split

DIR.OMP.END.TEAMS.11:                             ; preds = %DIR.OMP.END.PARALLEL.LOOP.1075.split.split.split90, %DIR.OMP.TEAMS.6
  ret void
}

declare spir_func void @__kmpc_critical(ptr addrspace(4))

declare spir_func void @__kmpc_end_critical(ptr addrspace(4))

attributes #0 = { nounwind }

!spirv.Source = !{!0}

!0 = !{i32 4, i32 200000}

; DEBUGIFY-NOT: WARNING
