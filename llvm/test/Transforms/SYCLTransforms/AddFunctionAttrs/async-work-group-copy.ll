; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

; Function Attrs: convergent
define void @test_fn(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7) #0 !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
; CHECK: call ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event
; CHECK-SAME: #0
  %call8 = tail call ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7, ptr null) #0
; CHECK: call ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event
; CHECK-SAME: #0
  %call6 = tail call ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, ptr null) #0
  ret void
}
; Function Attrs: convergent
declare ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, i64, ptr) #0
; CHECK: declare ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event
; CHECK-SAME: #0
declare ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, ptr) #0
; CHECK: declare ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event
; CHECK-SAME: #0

;; now have a call to @test_fn. The call site has to be "kernel-convergent-call" "kernel-call-once"
define void @foo(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7) #0 !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
; CHECK: call void @test_fn
;; The following check does not hold, the pass does not assigns "kernel-convergent-call" "kernel-call-once"
;; This will be important when we'll get to function call vectorization.
; CHECK-SAMEx: #0
  tail call void @test_fn(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7) #0
  ret void
}

attributes #0 = { convergent }

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
;; Fails for the same reason as above
; CHECK-NOTx: #1

!0 = !{!"char*", !"char*", !"long", !"long"}
!1 = !{ptr addrspace(3) null, ptr addrspace(1) null, i64 0, i64 0}

; DEBUGIFY-NOT: WARNING
