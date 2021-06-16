; RUN: llvm-as %p/WGBuiltins32.ll -o %t.WGBuiltins32.ll.bc
; RUN: %oclopt -runtimelib=%t.WGBuiltins32.ll.bc -B-GroupBuiltins -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -runtimelib=%t.WGBuiltins32.ll.bc -B-GroupBuiltins -verify -S < %s | FileCheck %s

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

; CHECK: @wg_test_broadcast
; CHECK: entry
; CHECK-NEXT: %AllocaWGResult = alloca i32
; CHECK-NEXT: store i32 0, i32* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: WIcall = call i32 @_Z12get_local_idj(i32 0)
; CHECK-NOT: %call1 = tail call i32 @_Z20work_group_broadcastij(i32 %0, i32 2)
; CHECK-NEXT: %CallWGForItem = call i32 @_Z20work_group_broadcastijjPi(i32 %0, i32 2, i32 %WIcall, i32* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK: store i32 %CallWGForItem, i32 addrspace(1)* %arrayidx2, align 1

define void @wg_test_broadcast(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i32 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 1
  %call1 = tail call i32 @_Z20work_group_broadcastij(i32 %0, i32 2) nounwind
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %b, i32 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx2, align 1
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z20work_group_broadcastij(i32, i32)

; CHECK: @__Vectorized_.wg_test_broadcast
; CHECK: entry
; CHECK-NEXT: %AllocaWGResult = alloca <4 x i32>
; CHECK-NEXT: store <4 x i32> zeroinitializer, <4 x i32>* %AllocaWGResult
; CHECK-NEXT: call void @dummybarrier.()
; CHECK: WIcall = call i32 @_Z12get_local_idj(i32 0)
; CHECK-NOT: call <4 x i32> @_Z20work_group_broadcastDv4_ij(<4 x i32> %1, i32 2)
; CHECK-NEXT: CallWGForItem = call <4 x i32> @_Z20work_group_broadcastDv4_ijjPS_(<4 x i32> %1, i32 2, i32 %WIcall, <4 x i32>* %AllocaWGResult)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK: store <4 x i32> %CallWGForItem, <4 x i32> addrspace(1)* %ptrTypeCast4, align 1

define void @__Vectorized_.wg_test_broadcast(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = getelementptr inbounds i32, i32 addrspace(1)* %a, i32 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <4 x i32> addrspace(1)*
  %1 = load <4 x i32>, <4 x i32> addrspace(1)* %ptrTypeCast, align 1
  %2 = call <4 x i32> @_Z20work_group_broadcastDv4_ij(<4 x i32> %1, i32 2)
  %3 = getelementptr inbounds i32, i32 addrspace(1)* %b, i32 %call
  %ptrTypeCast4 = bitcast i32 addrspace(1)* %3 to <4 x i32> addrspace(1)*
  store <4 x i32> %2, <4 x i32> addrspace(1)* %ptrTypeCast4, align 1
  ret void
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

declare <4 x i32> @_Z20work_group_broadcastDv4_ij(<4 x i32>, i32) nounwind readnone

; CHECK: declare i32 @_Z12get_local_idj(i32)
; CHECK: declare i32 @_Z20work_group_broadcastijjPi(i32, i32, i32, i32*)
; CHECK: declare <4 x i32> @_Z20work_group_broadcastDv4_ijjPS_(<4 x i32>, i32, i32, <4 x i32>*)


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

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_broadcast, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_name", !"a", !"b"}
!6 = !{i32 1, i32 0}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"-cl-std=CL2.0"}
!10 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_broadcast, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", i32 7}
!15 = !{!"kernel_has_barrier", i1 true}
!16 = !{!"no_barrier_path", i1 false}
!17 = !{!"vectorized_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_broadcast}
!18 = !{!"vectorized_width", i32 1}
!19 = !{!"kernel_wrapper", null}
!20 = !{!"scalar_kernel", null}
!21 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @__Vectorized_.wg_test_broadcast, !22}
!22 = !{!12, !13, !23, !15, !24, !25, !26, !19, !27}
!23 = !{!"kernel_execution_length", i32 9}
!24 = !{!"no_barrier_path", null}
!25 = !{!"vectorized_kernel", null}
!26 = !{!"vectorized_width", i32 4}
!27 = !{!"scalar_kernel", void (i32 addrspace(1)*, i32 addrspace(1)*)* @wg_test_broadcast}
!28 = !{!29, !30}
!29 = !{!"gen_addr_space_pointer_counter", null}
!30 = !{!"gen_addr_space_pointer_warnings"}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__kernel void wg_test_broadcast(__global int* a, __global int* b) {
;;;  size_t i = get_global_id(0);
;;;  b[i] = work_group_broadcast(a[i], 2);
;;;}

;; These are inserted by GroupBuiltin pass, should not have debug info
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- %AllocaWGResult = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- store i32 0, i32* %AllocaWGResult, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- %WIcall = call i32 @_Z12get_local_idj(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- store i32 0, i32* %AllocaWGResult, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function wg_test_broadcast -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- %AllocaWGResult = alloca <4 x i32>, align 16
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- store <4 x i32> zeroinitializer, <4 x i32>* %AllocaWGResult, align 16
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- call void @dummybarrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- %WIcall = call i32 @_Z12get_local_idj(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- store <4 x i32> zeroinitializer, <4 x i32>* %AllocaWGResult, align 16
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __Vectorized_.wg_test_broadcast -- call void @dummybarrier.()

; DEBUGIFY-NOT: WARNING
