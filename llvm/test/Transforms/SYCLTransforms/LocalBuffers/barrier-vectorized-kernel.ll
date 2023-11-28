; RUN: opt -passes=sycl-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-local-buffers -S %s | FileCheck %s

; Check local variable used in scalar kernel and vector kernel has the same
; offset in both kernels. Local buffer size is also the same for both kernels.
; Note that barrier loops are removed for the sake of simplicity.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32

; CHECK: @test.i = external addrspace(3) global i32

define dso_local void @test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) !vectorized_kernel !1 !vectorized_width !2 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
; CHECK-LABEL: define dso_local void @test(
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE:![0-9]+]]
; CHECK: %0 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: store i32 {{.*}}, ptr addrspace(3) %0, align 4
;
  store i32 0, ptr addrspace(3) @test.i, align 4
  ret void
}

define dso_local void @_ZGVeN16u_test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) !scalar_kernel !0 !vectorized_width !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
; CHECK-LABEL: define dso_local void @_ZGVeN16u_test(
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE]]
; CHECK: %0 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: store i32 {{.*}}, ptr addrspace(3) %0, align 4
;
  store i32 0, ptr addrspace(3) @test.i, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{ptr @_ZGVeN16u_test}
!2 = !{i32 1}
!3 = !{i32 16}
!4 = !{!"int*"}
!5 = !{ptr addrspace(1) null}

; CHECK: [[LOCAL_SIZE]] = !{i32 4}

; DEBUGIFY-NOT: WARNING
