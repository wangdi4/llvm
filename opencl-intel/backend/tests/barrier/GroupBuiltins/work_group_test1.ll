; RUN: opt -runtimelib %p/WGBuiltins64.ll -B-GroupBuiltins -verify -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the GroupBuiltin pass
;;  - The case is a kernel with a WG function call as first instruction.
;;  - It checks that a call to uniform WG function (like work-group_all)
;;    leads to generation of per-WI function call encompassed by  
;;    'barrier' loop which starts after per-WI result accumulator initialization
;;    and ends immediately after the per-WI function call
;;*****************************************************************************
;;__kernel void build_hash_table()
;;{
;; int done = 0;
;; while(!work_group_all(done))
;; {
;;     done = 1;
;;     work_group_barrier(CLK_LOCAL_MEM_FENCE);
;; }
;;}

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; CHECK: @build_hash_table
; CHECK: entry:
; CHECK-NEXT: %[[AllocaWGResult1:[A-Za-z0-9]+]] = alloca i32
; CHECK-NEXT: store i32 1, i32* %[[AllocaWGResult1]]
; CHECK-NEXT: %[[AllocaWGResult2:[A-Za-z0-9]+]] = alloca i32
; CHECK-NEXT: store i32 1, i32* %[[AllocaWGResult2]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: call i32 @_Z14work_group_alli
; CHECK: %[[CallWGForItem1:[A-Za-z0-9]+]] = call i32 @_Z14work_group_alliPi(i32 0, i32* %[[AllocaWGResult1]])
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: store i32 1, i32* %[[AllocaWGResult1]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %[[lnot2:[A-Za-z0-9]+]] = icmp eq i32 %[[CallWGForItem1]], 0
;
; CHECK-NOT: call i32 @_Z14work_group_alli
; CHECK:  %[[CallWGForItem2:[A-Za-z0-9]+]] = call i32 @_Z14work_group_alliPi(i32 1, i32* %[[AllocaWGResult2]])
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: store i32 1, i32* %[[AllocaWGResult2]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %[[lnot:[A-Za-z0-9]+]] = icmp eq i32 %[[CallWGForItem2]], 0


; Function Attrs: noduplicate nounwind
define void @build_hash_table() #0 {
entry:
  %call1 = tail call i32 @_Z14work_group_alli(i32 0) #4
  %lnot2 = icmp eq i32 %call1, 0
  br i1 %lnot2, label %while.body, label %while.end

while.body:                                       ; preds = %entry, %while.body
  tail call void @_Z18work_group_barrierj(i32 1) #4
  %call = tail call i32 @_Z14work_group_alli(i32 1) #4
  %lnot = icmp eq i32 %call, 0
  br i1 %lnot, label %while.body, label %while.end

while.end:                                        ; preds = %while.body, %entry
  ret void
}

; Function Attrs: noduplicate
declare i32 @_Z14work_group_alli(i32) #1

; Function Attrs: noduplicate
declare void @_Z18work_group_barrierj(i32) #1

; CHECK: @__Vectorized_.build_hash_table
; CHECK: entry:
; CHECK-NEXT: %[[AllocaWGResult3:[A-Za-z0-9]+]] = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %[[AllocaWGResult3]]
; CHECK-NEXT: %[[AllocaWGResult4:[A-Za-z0-9]+]] = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %[[AllocaWGResult4]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: call <4 x i32> @_Z14work_group_allDv4_i
; CHECK:  %[[CallWGForItem3:[A-Za-z0-9]+]] = call <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> zeroinitializer, <4 x i32>* %[[AllocaWGResult3]])
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: %[[LoadWGFinalResult3:[A-Za-z0-9]+]] = load <4 x i32>* %[[AllocaWGResult3]]
; CHECK-NEXT: %[[CallFinalizeWG3:[A-Za-z0-9]+]] = call <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %[[LoadWGFinalResult3]])
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %[[AllocaWGResult3]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %[[lnot3:[A-Za-z0-9]+]] = icmp eq <4 x i32> %[[CallFinalizeWG3]], zeroinitializer
;
; CHECK-NOT: call <4 x i32> @_Z14work_group_allDv4_i
; CHECK:  %[[CallWGForItem4:[A-Za-z0-9]+]] = call <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %[[AllocaWGResult4]])
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: %[[LoadWGFinalResult4:[A-Za-z0-9]+]] = load <4 x i32>* %[[AllocaWGResult4]]
; CHECK-NEXT: %[[CallFinalizeWG4:[A-Za-z0-9]+]] = call <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %[[LoadWGFinalResult4]])
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %[[AllocaWGResult4]]
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %[[lnot4:[A-Za-z0-9]+]] = icmp eq <4 x i32> %[[CallFinalizeWG4]], zeroinitializer


; Function Attrs: noduplicate nounwind
define void @__Vectorized_.build_hash_table() #0 {
entry:
  %0 = call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> zeroinitializer)
  %lnot2 = icmp eq <4 x i32> %0, zeroinitializer
  %1 = call i1 @__ocl_allZero_v4(<4 x i1> %lnot2)
  br i1 %1, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %postload
  %vectorPHI = phi <4 x i1> [ %local_edge5, %postload ], [ %lnot2, %entry ]
  %extract = extractelement <4 x i1> %vectorPHI, i32 0
  br i1 %extract, label %preload, label %postload

preload:                                          ; preds = %while.body
  call void @_Z18work_group_barrierj(i32 1) #4
  br label %postload

postload:                                         ; preds = %while.body, %preload
  %2 = call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> <i32 1, i32 1, i32 1, i32 1>)
  %lnot = icmp eq <4 x i32> %2, zeroinitializer
  %local_edge5 = and <4 x i1> %vectorPHI, %lnot
  %3 = call i1 @__ocl_allZero_v4(<4 x i1> %local_edge5)
  br i1 %3, label %while.end, label %while.body

while.end:                                        ; preds = %entry, %postload
  ret void
}

; CHECK: declare i32 @_Z14work_group_alliPi(i32, i32*)
; CHECK: declare <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32>, <4 x i32>*)
; CHECK: declare <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32>)

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

declare void @maskedf_0__Z18work_group_barrierj(i1, i32)

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32>) #2

; Function Attrs: nounwind readnone
declare i1 @__ocl_allZero_v4(<4 x i1>) #3

attributes #0 = { noduplicate nounwind }
attributes #1 = { noduplicate }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}
!opencl.kernel_info = !{!11, !26}
!opencl.module_info_list = !{!33}
!llvm.functions_info = !{}

!0 = !{void ()* @build_hash_table, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_type_qual"}
!5 = !{!"kernel_arg_base_type"}
!6 = !{!"kernel_arg_name"}
!7 = !{i32 1, i32 2}
!8 = !{i32 2, i32 0}
!9 = !{}
!10 = !{!"-cl-std=CL2.0"}
!11 = !{void ()* @build_hash_table, !12}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25}
!13 = !{!"local_buffer_size", null}
!14 = !{!"barrier_buffer_size", null}
!15 = !{!"kernel_execution_length", i32 44}
!16 = !{!"max_wg_dimensions", i32 0}
!17 = !{!"kernel_has_barrier", i1 true}
!18 = !{!"kernel_has_global_sync", i1 false}
!19 = !{!"no_barrier_path", i1 false}
!20 = !{!"vectorized_kernel", void ()* @__Vectorized_.build_hash_table}
!21 = !{!"vectorized_width", i32 1}
!22 = !{!"kernel_wrapper", null}
!23 = !{!"scalarized_kernel", null}
!24 = !{!"block_literal_size", null}
!25 = !{!"private_memory_size", null}
!26 = !{void ()* @__Vectorized_.build_hash_table, !27}
!27 = !{!13, !14, !28, !16, !17, !18, !29, !30, !31, !22, !32, !24, !25}
!28 = !{!"kernel_execution_length", i32 105}
!29 = !{!"no_barrier_path", null}
!30 = !{!"vectorized_kernel", null}
!31 = !{!"vectorized_width", i32 4}
!32 = !{!"scalarized_kernel", void ()* @build_hash_table}
!33 = !{!34, !35, !36}
!34 = !{!"global_variable_total_size", i64 0}
!35 = !{!"gen_addr_space_pointer_counter", null}
!36 = !{!"gen_addr_space_pointer_warnings"}
