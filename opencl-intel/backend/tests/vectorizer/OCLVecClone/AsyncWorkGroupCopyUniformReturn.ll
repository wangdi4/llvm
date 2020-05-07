; RUN: %oclopt --ocl-vecclone --ocl-vec-clone-isa-encoding-override=AVX512Core -S < %s | FileCheck %s

%opencl.event_t.5 = type opaque

; CHECK-NOT: opencl-vec-uniform-return
; CHECK: declare %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, i64, %opencl.event_t.5*) [[ATTR:#[0-9]*]]
; CHECK: declare %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, %opencl.event_t.5*) [[ATTR]]
; CHECK-NOT: opencl-vec-uniform-return
; CHECK: attributes [[ATTR]] = { {{.*}}opencl-vec-uniform-return{{.*}} }
; CHECK-NOT: opencl-vec-uniform-return

; Function Attrs: convergent
define void @test_fn(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7) !ocl_recommended_vector_length !1 {
entry:
  %call8 = tail call %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, i64 %conv7, %opencl.event_t.5* null) #0
  %call6 = tail call %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)* %localBuffer, i8 addrspace(1)* %add.ptr, i64 %conv6, %opencl.event_t.5* null) #0
  ret void
}
; Function Attrs: convergent
declare %opencl.event_t.5* @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, i64, %opencl.event_t.5*) #0
declare %opencl.event_t.5* @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(i8 addrspace(3)*, i8 addrspace(1)*, i64, %opencl.event_t.5*) #0

attributes #0 = { convergent }

!opencl.kernels = !{!0}

!0 = !{void (i8 addrspace(3)*, i8 addrspace(1)*, i64, i64)* @test_fn}
!1 = !{i32 4}
