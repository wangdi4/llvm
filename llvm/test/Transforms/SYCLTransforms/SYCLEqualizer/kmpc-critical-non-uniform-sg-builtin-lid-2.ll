; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; Check that intel_vec_len_hint metadata isn't generated for the kernel although
; it contains __kmpc_critical calls. The kernel calls non-uniform subgroup
; reduction builtin and not all workitems in the subgroup is active. This is
; unsupported by subgroup emulation, therefore the kernel must be vectorized.

; CHECK: define void @__omp_offloading_801_663ae85_MAIN___l18(
; CHECK-NOT: !intel_vec_len_hint

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__tgt_offload_entry = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }
%"QNCA_a0$ptr$rank1$.0" = type { ptr addrspace(4), i64, i64, i64, i64, i64, [1 x %structtype.3] }
%structtype.3 = type { i64, i64, i64 }

@__omp_offloading_entries_table = addrspace(1) constant [3 x %struct.__tgt_offload_entry] [%struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr addrspace(1) null to ptr addrspace(4)), ptr addrspace(2) null, i64 3888, i32 0, i32 0, i64 53 }, %struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr addrspace(1) null to ptr addrspace(4)), ptr addrspace(2) null, i64 8, i32 0, i32 0, i64 45 }, %struct.__tgt_offload_entry { ptr addrspace(4) null, ptr addrspace(2) null, i64 0, i32 0, i32 0, i64 40 }]

declare spir_func i32 @_Z18get_sub_group_sizev()

declare spir_func i32 @_Z22get_sub_group_local_idv()

declare spir_func double @_Z32sub_group_non_uniform_reduce_addd(double)

define spir_kernel void @__omp_offloading_801_663ae85_MAIN___l18(ptr addrspace(1) noalias %"ascastB$target_reduction_array_dp_1d_$ARRAY", ptr addrspace(1) noalias %"ascastB$target_reduction_array_dp_1d_$ASUM", i64 %omp.pdo.norm.lb.val389.zext, i64 %omp.pdo.norm.ub.val.zext, i64 %"ascast$target_reduction_array_dp_1d_$I.val.zext") {
newFuncRoot:
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split379.split.clone

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split379.split.clone: ; preds = %newFuncRoot
  call spir_func void @__kmpc_critical(ptr addrspace(4) null)
  %0 = call spir_func i32 @_Z18get_sub_group_sizev()
  %exit.pred383.not17.not = icmp eq i32 %0, 0
  br i1 %exit.pred383.not17.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split, label %.lr.ph

.lr.ph:                                           ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split379.split.clone
  br label %1

1:                                                ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone, %.lr.ph
  %simdlane.id38218 = phi i32 [ 0, %.lr.ph ], [ %simdlane.id.inc385, %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone ]
  %2 = call spir_func i32 @_Z22get_sub_group_local_idv()
  %skip.pred384.not = icmp eq i32 %simdlane.id38218, %2
  br i1 %skip.pred384.not, label %red.update.body.clone.preheader, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone: ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.loopexit, %1
  %simdlane.id.inc385 = add nuw i32 %simdlane.id38218, 1
  %exitcond23.not = icmp eq i32 %simdlane.id.inc385, %0
  br i1 %exitcond23.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split.loopexit, label %1

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split.loopexit: ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split: ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split.loopexit, %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split379.split.clone
  call spir_func void @__kmpc_end_critical(ptr addrspace(4) null)
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.split380

red.update.body.clone.preheader:                  ; preds = %1
  br label %red.update.body.clone

red.update.body.clone:                            ; preds = %red.update.body.clone, %red.update.body.clone.preheader
  %red.cpy.dest.ptr375.clone = phi ptr addrspace(4) [ %red.cpy.dest.inc376.clone, %red.update.body.clone ], [ null, %red.update.body.clone.preheader ]
  %red.cpy.src.ptr.clone = phi ptr [ %red.cpy.src.inc.clone, %red.update.body.clone ], [ null, %red.update.body.clone.preheader ]
  %3 = call spir_func double @_Z32sub_group_non_uniform_reduce_addd(double 0.000000e+00)
  %red.cpy.dest.inc376.clone = getelementptr double, ptr addrspace(4) %red.cpy.dest.ptr375.clone, i64 1
  %red.cpy.src.inc.clone = getelementptr double, ptr %red.cpy.src.ptr.clone, i64 1
  %red.cpy.done377.clone = icmp eq i64 0, 0
  br i1 %red.cpy.done377.clone, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.loopexit, label %red.update.body.clone

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.loopexit: ; preds = %red.update.body.clone
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone

DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.split380: ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4363.split.split.split.clone.split
  ret void
}

declare spir_func void @__kmpc_critical(ptr addrspace(4))

declare spir_func void @__kmpc_end_critical(ptr addrspace(4))

; DEBUGIFY-NOT: WARNING
