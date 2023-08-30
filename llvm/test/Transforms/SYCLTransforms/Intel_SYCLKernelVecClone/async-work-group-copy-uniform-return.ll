; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone,vplan-vec -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

; CHECK-NOT: opencl-vec-uniform-return
; CHECK: declare ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, i64, ptr) [[ATTR:#[0-9]*]]
; CHECK: declare ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, ptr) [[ATTR]]
; CHECK-NOT: opencl-vec-uniform-return
; CHECK: attributes [[ATTR]] = { {{.*}}opencl-vec-uniform-return{{.*}} }
; CHECK-NOT: opencl-vec-uniform-return

; Function Attrs: convergent
define void @test_fn(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7) !recommended_vector_length !1 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %call8 = tail call ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7, ptr null) #0
  %call6 = tail call ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, ptr null) #0
  ret void
}
; Function Attrs: convergent
declare ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, i64, ptr) #0
declare ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, ptr) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}

!0 = !{ptr @test_fn}
!1 = !{i32 4}
!2 = !{!"char*", !"char*", !"long", !"long"}
!3 = !{ptr addrspace(3) null, ptr addrspace(1) null, i64 0, i64 0}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_test_fn {{.*}} br
; DEBUGIFY-NOT: WARNING
