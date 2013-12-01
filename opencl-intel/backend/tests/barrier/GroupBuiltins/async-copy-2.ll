; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-GroupBuiltins -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the GroupBuiltins pass
;; The case: kernel "main" with async copy instruction inside a loop,
;; The expected result:
;;      1. A call to @_Z7barrierj(LOCAL_MEM_FENCE) just before calling the function "async_copy" in main
;;      2. A call to @dummybarrier.() just after calling the function  "async_copy" in main
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl.event_t = type opaque

; CHECK: @main
define void @main(i32 %x, i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count) nounwind {
  %pEvent = alloca %opencl.event_t*, align 8
  br label %BBLoop
BBLoop:
  %event = call %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPKU3AS1cm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count, %opencl.event_t* null) nounwind
  store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
  %y = xor i32 %x, %x
  %cond = icmp sgt i64 %count, 0 
  br i1 %cond, label %BBLoop, label %BBEndLoop
BBEndLoop:
  call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent) nounwind
  ret void

; CHECK: %pEvent = alloca %opencl.event_t*, align 8
; CHECK: br label %BBLoop
; CHECK: BBLoop:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: %event = call %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPKU3AS1cm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count, %opencl.event_t* null)
; CHECK: call void @dummybarrier.
; CHECK: store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
; CHECK: %y = xor i32 %x, %x
; CHECK: %cond = icmp sgt i64 %count, 0 
; CHECK: br i1 %cond, label %BBLoop, label %BBEndLoop
; CHECK: BBEndLoop:
; CHECK: call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent)
; CHECK: ret void
}


declare %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPKU3AS1cm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, %opencl.event_t*)
declare %opencl.event_t* @_Z29async_work_group_strided_copyPU3AS1cPKU3AS3cmm9ocl_event(i8 addrspace(1)*, i8 addrspace(3)*, i64, i64, %opencl.event_t*)
declare void @_Z17wait_group_eventsiP9ocl_event(i32, %opencl.event_t**)

; CHECK: declare void @_Z7barrierj(i32)
; CHECK: declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}
!opencl.kernel_info = !{!6}
!opencl.disabled.FP_CONTRACT = !{}

!0 = metadata !{void (i32, i8 addrspace(3)*, i8 addrspace(1)*, i64)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
!6 = metadata !{void (i32, i8 addrspace(3)*, i8 addrspace(1)*, i64)* @main, metadata !7}
!7 = metadata !{metadata !8, metadata !9, metadata !10, metadata !11, metadata !12, metadata !13, metadata !14, metadata !15, metadata !16}
!8 = metadata !{metadata !"local_buffer_size", null}
!9 = metadata !{metadata !"barrier_buffer_size", null}
!10 = metadata !{metadata !"kernel_execution_length", null}
!11 = metadata !{metadata !"kernel_has_barrier", null}
!12 = metadata !{metadata !"no_barrier_path", i1 false}
!13 = metadata !{metadata !"vectorized_kernel", null}
!14 = metadata !{metadata !"vectorized_width", null}
!15 = metadata !{metadata !"kernel_wrapper", null}
!16 = metadata !{metadata !"scalarized_kernel", null}
