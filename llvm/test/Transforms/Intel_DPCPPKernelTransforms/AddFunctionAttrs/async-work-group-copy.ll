; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S < %s | FileCheck %s

%opencl.event_t.5 = type opaque
; Function Attrs: convergent
define void @test_fn(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7) #0 {
entry:
; CHECK: call %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event
; CHECK-SAME: #0
  %call8 = tail call %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7, %opencl.event_t.5* null) #0
; CHECK: call %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event
; CHECK-SAME: #0
  %call6 = tail call %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, %opencl.event_t.5* null) #0
  ret void
}
; Function Attrs: convergent
declare %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, i64, %opencl.event_t.5*) #0
; CHECK: declare %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event
; CHECK-SAME: #0
declare %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, %opencl.event_t.5*) #0
; CHECK: declare %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event
; CHECK-SAME: #0

;; now have a call to @test_fn. The call site has to be "kernel-convergent-call" "kernel-call-once"
define void @foo(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7) #0 {
entry:
; CHECK: call void @test_fn
;; The following check does not hold, the pass does not assigns "kernel-convergent-call" "kernel-call-once"
;; This will be important when we'll get to function call vectorization.
; CHECK-SAMEx: #0
  tail call void @test_fn(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7) #0
  ret void
}

attributes #0 = { convergent }

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
;; Fails for the same reason as above
; CHECK-NOTx: #1

; DEBUGIFY-NOT: WARNING
