; RUN: opt -passes=dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-group-builtin -S < %s | FileCheck %s
; RUN: opt -dpcpp-kernel-group-builtin -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the GroupBuiltins pass
;; The case: kernel "main" with async copy instruction,
;;    which is calling function "foo" that contains async copy strided instruction
;; The expected result:
;;      1. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) just before calling the function "async_copy" in main
;;      2. A call to @barrier_dummy() just after calling the function  "async_copy" in main
;;      3. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) just before calling the function "async_copy_strided" in foo
;;      4. A call to @barrier_dummy() just after calling the function  "async_copy_strided" in foo
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl.event_t = type opaque

; CHECK: @main
define void @main(i32 %x, i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count) nounwind {
  %pEvent = alloca %opencl.event_t*, align 8
  %event = call %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count, %opencl.event_t* null) nounwind
  store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
  %y = xor i32 %x, %x
  call void @foo(i32 %x, i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count)
  call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent) nounwind
  ret void

; CHECK: %pEvent = alloca %opencl.event_t*, align 8
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %event = call %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count, %opencl.event_t* null)
; CHECK: call void @barrier_dummy
; CHECK: store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
; CHECK: %y = xor i32 %x, %x
; CHECK: call void @foo(i32 %x, i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count)
; CHECK: call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent)
; CHECK: ret void
}

; CHECK: @foo
define void @foo(i32 %x, i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %globalBuffer, i64 %count) nounwind {
  %pEvent = alloca %opencl.event_t*, align 8
  %event = call %opencl.event_t* @_Z29async_work_group_strided_copyPU3AS1cPU3AS3Kcmm9ocl_event(i8 addrspace(1)* %globalBuffer, i8 addrspace(3)* %localBuffer, i64 %count, i64 18, %opencl.event_t* null) nounwind
  store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
  %y = xor i32 %x, %x
  call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent) nounwind
  ret void

; CHECK: %pEvent = alloca %opencl.event_t*, align 8
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %event = call %opencl.event_t* @_Z29async_work_group_strided_copyPU3AS1cPU3AS3Kcmm9ocl_event(i8 addrspace(1)* %globalBuffer, i8 addrspace(3)* %localBuffer, i64 %count, i64 18, %opencl.event_t* null)
; CHECK: call void @barrier_dummy
; CHECK: store %opencl.event_t* %event, %opencl.event_t** %pEvent, align 8
; CHECK: %y = xor i32 %x, %x
; CHECK: call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** %pEvent)
; CHECK: ret void
}

declare %opencl.event_t* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, %opencl.event_t*)
declare %opencl.event_t* @_Z29async_work_group_strided_copyPU3AS1cPU3AS3Kcmm9ocl_event(i8 addrspace(1)*, i8 addrspace(3)*, i64, i64, %opencl.event_t*)
declare void @_Z17wait_group_eventsiP9ocl_event(i32, %opencl.event_t**)

; CHECK: declare void @_Z18work_group_barrierj(i32)
; CHECK: declare void @barrier_dummy()

!sycl.kernels = !{!0}
!opencl.build.options = !{}
!opencl.kernel_info = !{!6}
!opencl.disabled.FP_CONTRACT = !{}

!0 = !{void (i32, i8 addrspace(3)*, i8 addrspace(1)*, i64)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
!6 = !{void (i32, i8 addrspace(3)*, i8 addrspace(1)*, i64)* @main, !7}
!7 = !{!8, !9, !10, !11, !12, !13, !14, !15, !16}
!8 = !{!"local_buffer_size", null}
!9 = !{!"barrier_buffer_size", null}
!10 = !{!"kernel_execution_length", null}
!11 = !{!"kernel_has_barrier", null}
!12 = !{!"no_barrier_path", i1 false}
!13 = !{!"vectorized_kernel", null}
!14 = !{!"vectorized_width", null}
!15 = !{!"kernel_wrapper", null}
!16 = !{!"scalar_kernel", null}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- call void @barrier_dummy()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- call void @barrier_dummy()
; DEBUGIFY-NOT: WARNING
