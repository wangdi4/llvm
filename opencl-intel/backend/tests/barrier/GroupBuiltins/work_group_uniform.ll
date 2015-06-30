; RUN: opt -runtimelib %p/WGBuiltins32.ll -B-GroupBuiltins -verify -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the GroupBuiltin pass
;;  - It checks that a call to uniform WG function (like work-group_all)
;;    leads to generation of per-WI function call encompassed by  
;;    'barrier' loop which starts after per-WI result accumulator initialization
;;    and ends immediately after the per-WI function call
;;  - For vectorized version of uniform WG function call we yet add a finalization
;;    function call immediately after the 'barrier' loop (and before the next 
;;    'barrier' loop.
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

; CHECK: @wg_test_uniform
; CHECK: entry:
; CHECK-NEXT: %AllocaWGResult = alloca i32
; CHECK-NEXT: store i32 1, i32* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: call2 = tail call i32 @_Z14work_group_alli(i32 %conv)
; CHECK:  %CallWGForItem = call i32 @_Z14work_group_alliPi(i32 %conv, i32* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: store i32 1, i32* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %tobool = icmp eq i32 %CallWGForItem, 0


define void @wg_test_uniform(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %a, i32 %call
  %0 = load i32 addrspace(1)* %arrayidx, align 1
  %arrayidx1 = getelementptr inbounds i32 addrspace(1)* %b, i32 %call
  %1 = load i32 addrspace(1)* %arrayidx1, align 1
  %cmp = icmp sgt i32 %0, %1
  %conv = zext i1 %cmp to i32
  %call2 = tail call i32 @_Z14work_group_alli(i32 %conv) nounwind readnone
  %tobool = icmp eq i32 %call2, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  store i32 %1, i32 addrspace(1)* %arrayidx, align 1
  br label %if.end

if.else:                                          ; preds = %entry
  store i32 %0, i32 addrspace(1)* %arrayidx1, align 1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z14work_group_alli(i32) nounwind readnone

; CHECK: @__Vectorized_.wg_test_uniform
; CHECK: entry:
; CHECK-NEXT: %AllocaWGResult = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> %conv10)
; CHECK:  %CallWGForItem = call <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> %conv10, <4 x i32>* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: %LoadWGFinalResult = load <4 x i32>* %AllocaWGResult
; CHECK-NEXT: %CallFinalizeWG = call <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %LoadWGFinalResult)
; CHECK-NEXT: store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: %tobool = icmp eq <4 x i32> %CallFinalizeWG, zeroinitializer


define void @__Vectorized_.wg_test_uniform(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32 addrspace(1)* %a, i32 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <4 x i32> addrspace(1)*
  %1 = load <4 x i32> addrspace(1)* %ptrTypeCast, align 1
  %2 = getelementptr inbounds i32 addrspace(1)* %b, i32 %call
  %ptrTypeCast9 = bitcast i32 addrspace(1)* %2 to <4 x i32> addrspace(1)*
  %3 = load <4 x i32> addrspace(1)* %ptrTypeCast9, align 1
  %cmp = icmp sgt <4 x i32> %1, %3
  %conv10 = zext <4 x i1> %cmp to <4 x i32>
  %4 = call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> %conv10)
  %tobool = icmp eq <4 x i32> %4, zeroinitializer
  %Mneg11 = xor <4 x i1> %tobool, <i1 true, i1 true, i1 true, i1 true>
  %5 = call i1 @__ocl_allZero_v4(<4 x i1> %tobool)
  br i1 %5, label %header3, label %if.else

header3:                                          ; preds = %if.else, %entry
  %6 = call i1 @__ocl_allZero_v4(<4 x i1> %Mneg11)
  br i1 %6, label %if.end, label %if.then

if.then:                                          ; preds = %header3
  %extmask = sext <4 x i1> %Mneg11 to <4 x i32>
  %PtrCast = addrspacecast <4 x i32> addrspace(1)* %ptrTypeCast to <4 x i32>*
  call void @__ocl_masked_store_int4(<4 x i32>* %PtrCast, <4 x i32> %3, <4 x i32> %extmask) nounwind
  br label %if.end

if.else:                                          ; preds = %entry
  %extmask14 = sext <4 x i1> %tobool to <4 x i32>
  %PtrCast15 = addrspacecast <4 x i32> addrspace(1)* %ptrTypeCast9 to <4 x i32>*
  call void @__ocl_masked_store_int4(<4 x i32>* %PtrCast15, <4 x i32> %1, <4 x i32> %extmask14) nounwind
  br label %header3

if.end:                                           ; preds = %if.then, %header3
  ret void
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

declare void @masked_store_align4_0(i1, i32, i32 addrspace(1)*)

declare void @masked_store_align4_1(i1, i32, i32 addrspace(1)*)

declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32>) nounwind readnone

declare i1 @__ocl_allZero_v4(<4 x i1>) nounwind readnone

declare void @masked_store_align4_2(<4 x i1>, <4 x i32>, <4 x i32> addrspace(1)*)

declare void @masked_store_align4_3(<4 x i1>, <4 x i32>, <4 x i32> addrspace(1)*)

declare void @__ocl_masked_store_int4(<4 x i32>*, <4 x i32>, <4 x i32>)

; CHECK: declare i32 @_Z14work_group_alliPi(i32, i32*)
; CHECK: declare <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32>, <4 x i32>*)
; CHECK: declare <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32>)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!9}
!opencl.kernel_info = !{!10, !21}
!opencl.module_info_list = !{!28}
!llvm.functions_info = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_uniform, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_name", !"a", !"b"}
!6 = !{i32 1, i32 0}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"-cl-std=CL2.0"}
!10 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_uniform, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", i32 15}
!15 = !{!"kernel_has_barrier", i1 true}
!16 = !{!"no_barrier_path", i1 false}
!17 = !{!"vectorized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_uniform}
!18 = !{!"vectorized_width", i32 1}
!19 = !{!"kernel_wrapper", null}
!20 = !{!"scalarized_kernel", null}
!21 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_uniform, !22}
!22 = !{!12, !13, !23, !15, !24, !25, !26, !19, !27}
!23 = !{!"kernel_execution_length", i32 25}
!24 = !{!"no_barrier_path", null}
!25 = !{!"vectorized_kernel", null}
!26 = !{!"vectorized_width", i32 4}
!27 = !{!"scalarized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_uniform}
!28 = !{!29, !30}
!29 = !{!"gen_addr_space_pointer_counter", null}
!30 = !{!"gen_addr_space_pointer_warnings"}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__kernel void wg_test_uniform(__global int* a, __global int* b) {
;;;  size_t i = get_global_id(0);
;;;  if (work_group_all(a[i] > b[i])) {
;;;    a[i] = b[i];
;;;  } else {
;;;    b[i] = a[i];
;;;  }
;;;}
