; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-GroupBuiltins -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the GroupBuiltin pass
;;  - It checks that a call to uniform WG function (like work-group_all)
;;    leads to generation of per-WI function call encompassed by  
;;    'barrier' loop which starts after per-WI result accumulator initialization
;;    and ends immediately after the per-WI function call
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

; CHECK: @wg_test_scan
; CHECK: entry:
; CHECK-NEXT: %AllocaWGResult = alloca i32
; CHECK-NEXT: store i32 -2147483648, i32* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: %call1 = tail call i32 @_Z29work_group_scan_exclusive_maxi(i32 %0)
; CHECK: %CallWGForItem = call i32 @_Z29work_group_scan_exclusive_maxiPi(i32 %0, i32* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK: store i32 %CallWGForItem, i32 addrspace(1)* %arrayidx2, align 1

define void @wg_test_scan(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %a, i32 %call
  %0 = load i32 addrspace(1)* %arrayidx, align 1
  %call1 = tail call i32 @_Z29work_group_scan_exclusive_maxi(i32 %0) nounwind
  %arrayidx2 = getelementptr inbounds i32 addrspace(1)* %b, i32 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 1
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z29work_group_scan_exclusive_maxi(i32)

; CHECK: @__Vectorized_.wg_test_scan
; CHECK: entry:
; CHECK-NEXT: %AllocaWGResult = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>, <4 x i32>* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NOT: call <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_i(<4 x i32> %1)
; CHECK: %CallWGForItem = call <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_iPS_(<4 x i32> %1, <4 x i32>* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK: store <4 x i32> %CallWGForItem, <4 x i32> addrspace(1)* %ptrTypeCast4, align 1

define void @__Vectorized_.wg_test_scan(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32 addrspace(1)* %a, i32 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <4 x i32> addrspace(1)*
  %1 = load <4 x i32> addrspace(1)* %ptrTypeCast, align 1
  %2 = call <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_i(<4 x i32> %1)
  %3 = getelementptr inbounds i32 addrspace(1)* %b, i32 %call
  %ptrTypeCast4 = bitcast i32 addrspace(1)* %3 to <4 x i32> addrspace(1)*
  store <4 x i32> %2, <4 x i32> addrspace(1)* %ptrTypeCast4, align 1
  ret void
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_i(<4 x i32>) nounwind readnone

; CHECK: declare i32 @_Z29work_group_scan_exclusive_maxiPi(i32, i32*)
; CHECK: declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_iPS_(<4 x i32>, <4 x i32>*)

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

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_scan, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"a", metadata !"b"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 2, i32 0}
!8 = metadata !{}
!9 = metadata !{metadata !"-cl-std=CL2.0"}
!10 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_scan, metadata !11}
!11 = metadata !{metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19, metadata !20}
!12 = metadata !{metadata !"local_buffer_size", null}
!13 = metadata !{metadata !"barrier_buffer_size", null}
!14 = metadata !{metadata !"kernel_execution_length", i32 7}
!15 = metadata !{metadata !"kernel_has_barrier", i1 true}
!16 = metadata !{metadata !"no_barrier_path", i1 false}
!17 = metadata !{metadata !"vectorized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_scan}
!18 = metadata !{metadata !"vectorized_width", i32 1}
!19 = metadata !{metadata !"kernel_wrapper", null}
!20 = metadata !{metadata !"scalarized_kernel", null}
!21 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_scan, metadata !22}
!22 = metadata !{metadata !12, metadata !13, metadata !23, metadata !15, metadata !24, metadata !25, metadata !26, metadata !19, metadata !27}
!23 = metadata !{metadata !"kernel_execution_length", i32 9}
!24 = metadata !{metadata !"no_barrier_path", null}
!25 = metadata !{metadata !"vectorized_kernel", null}
!26 = metadata !{metadata !"vectorized_width", i32 4}
!27 = metadata !{metadata !"scalarized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_scan}
!28 = metadata !{metadata !29, metadata !30}
!29 = metadata !{metadata !"gen_addr_space_pointer_counter", null}
!30 = metadata !{metadata !"gen_addr_space_pointer_warnings"}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__kernel void wg_test_scan(__global int* a, __global int* b) {
;;;  size_t i = get_global_id(0);
;;;  b[i] = work_group_scan_exclusive_max(a[i]);
;;;}
