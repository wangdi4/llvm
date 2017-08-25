; RUN: opt -O1 -ocl-asaa -S < %s | FileCheck %s
;;*****************************************************************************
;; Verify that "store float 0x3FB99999A0000000" isn't in loop
;; and there's no comparison of constant instruction
;; 0x3FB99999A0000000 and value
;;*****************************************************************************
; ModuleID = 'main'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

;CHECK-LABEL: scalar_kernel_entry
;CHECK: store float 0x3FB99999A0000000
;CHECK-NOT: fcmp {{.*}} 0x3FB99999A0000000

;CHECK-LABEL: dim_0_exit
;CHECK-NOT: store float 0x3FB99999A0000000

@testKernel.val = internal addrspace(1) global float 0.000000e+00, align 4

; Function Attrs: nounwind readnone
declare zeroext i1 @isFenceValid(i32) #0

; Function Attrs: nounwind readonly
declare zeroext i1 @helperFunction(float addrspace(4)*, float) #1

; Function Attrs: nounwind readnone
declare i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)*) #0

; Function Attrs: nounwind
define void @testKernel(i32 addrspace(1)* noalias nocapture %results) #2 {
  %ptr = alloca float addrspace(4)*, align 8
  %early_exit_call = call [7 x i64] @WG.boundaries.testKernel(i32 addrspace(1)* %results)
  %1 = extractvalue [7 x i64] %early_exit_call, 0
  %2 = trunc i64 %1 to i1
  br i1 %2, label %WGLoopsEntry, label %9

WGLoopsEntry:                                     ; preds = %0
  %3 = extractvalue [7 x i64] %early_exit_call, 1
  %4 = extractvalue [7 x i64] %early_exit_call, 2
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %WGLoopsEntry
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %helperFunction.exit, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %helperFunction.exit ]
  %dim_0_tid = phi i64 [ %3, %dim_0_pre_head ], [ %dim_0_inc_tid, %helperFunction.exit ]
  store float 0x3FB99999A0000000, float addrspace(1)* @testKernel.val, align 4
  store volatile float addrspace(4)* addrspacecast (float addrspace(1)* @testKernel.val to float addrspace(4)*), float addrspace(4)** %ptr, align 8
  %5 = load volatile float addrspace(4)*, float addrspace(4)** %ptr, align 8
  %6 = bitcast float addrspace(4)* %5 to i8 addrspace(4)*
  %switch.i.i = icmp ult i32 2, 4
  br i1 %switch.i.i, label %if.end.i, label %helperFunction.exit

if.end.i:                                         ; preds = %scalar_kernel_entry
  %7 = load float, float addrspace(1)* @testKernel.val, align 4
  %8 = load float, float addrspace(4)* %5, align 4
  %not.cmp.i = fcmp oeq float %8, %7
  br label %helperFunction.exit

helperFunction.exit:                              ; preds = %scalar_kernel_entry, %if.end.i
  %retval.0.i = phi i1 [ false, %scalar_kernel_entry ], [ %not.cmp.i, %if.end.i ]
  %conv2 = zext i1 %retval.0.i to i32
  %idxprom = and i64 %dim_0_tid, 4294967295
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom
  store i32 %conv2, i32 addrspace(1)* %arrayidx, align 4
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %4
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %helperFunction.exit
  br label %9

; <label>:9                                       ; preds = %0, %dim_0_exit
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #0

define [7 x i64] @WG.boundaries.testKernel(i32 addrspace(1)*) {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}
!opencl.kernel_info = !{!12}
!opencl.module_info_list = !{!29}
!llvm.functions_info = !{}
!opencl.functions_stats = !{}
!opencl.stat_descriptions = !{}
!opencl.module_stat_info = !{}

!0 = !{void (i32 addrspace(1)*)* @testKernel, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"uint*"}
!4 = !{!"kernel_arg_base_type", !"uint*"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{!"kernel_arg_name", !"results"}
!7 = !{i32 1, i32 2}
!8 = !{i32 2, i32 0}
!9 = !{}
!10 = !{!"-cl-std=CL2.0"}
!11 = !{!"clang version 3.6.2"}
!12 = !{void (i32 addrspace(1)*)* @testKernel, !13}
!13 = !{!14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28}
!14 = !{!"local_buffer_size", null}
!15 = !{!"barrier_buffer_size", i32 0}
!16 = !{!"kernel_execution_length", i32 18}
!17 = !{!"max_wg_dimensions", i32 1}
!18 = !{!"kernel_has_barrier", i1 false}
!19 = !{!"kernel_has_global_sync", i1 false}
!20 = !{!"no_barrier_path", i1 true}
!21 = !{!"vectorized_kernel", null}
!22 = !{!"vectorized_width", null}
!23 = !{!"kernel_wrapper", null}
!24 = !{!"scalarized_kernel", null}
!25 = !{!"block_literal_size", null}
!26 = !{!"private_memory_size", i32 8}
!27 = !{!"vectorization_dimension", null}
!28 = !{!"can_unite_workgroups", null}
!29 = !{!30, !31, !32}
!30 = !{!"global_variable_total_size", i64 4}
!31 = !{!"gen_addr_space_pointer_counter", null}
!32 = !{!"gen_addr_space_pointer_warnings"}

;;;bool helperFunction(float *floatp, float val) {
;;;  if(!isFenceValid(get_fence(floatp)))
;;;    return false;
;;;  if (*floatp != val)
;;;    return false; return true;
;;;}
;;;_kernel void testKernel(_global uint *results) {
;;;  uint tid = get_global_id(0);
;;;  static __global float val;
;;;  val = 0.1f;
;;;  float * volatile ptr = &val;
;;;  results[tid] = helperFunction(ptr, val);
;;;}
