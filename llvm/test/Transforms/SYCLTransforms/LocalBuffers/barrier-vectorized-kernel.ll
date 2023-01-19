; RUN: opt -passes=dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-local-buffers -S %s | FileCheck %s

; Check local variable used in scalar kernel and vector kernel has the same
; offset in both kernels. Local buffer size is also the same for both kernels.
; Note that barrier loops are removed for the sake of simplicity.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32

; CHECK-NOT: @test.i =

define dso_local void @test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle) !vectorized_kernel !1 !vectorized_width !2 {
entry:
; CHECK-LABEL: define dso_local void @test(
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE:![0-9]+]]
; CHECK: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[BC:%[0-9]+]] = bitcast i8 addrspace(3)* [[GEP]] to i32 addrspace(3)*
; CHECK: store i32 {{.*}}, i32 addrspace(3)* [[BC]], align 4
;
  store i32 0, i32 addrspace(3)* @test.i, align 4
  ret void
}

define dso_local void @_ZGVeN16u_test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle) !scalar_kernel !0 !vectorized_width !3 {
entry:
; CHECK-LABEL: define dso_local void @_ZGVeN16u_test(
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE]]
; CHECK: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[BC:%[0-9]+]] = bitcast i8 addrspace(3)* [[GEP:%[0-9]+]] to i32 addrspace(3)*
; CHECK: store i32 {{.*}}, i32 addrspace(3)* [[BC:%[0-9]+]], align 4
;
  store i32 0, i32 addrspace(3)* @test.i, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!1 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @_ZGVeN16u_test}
!2 = !{i32 1}
!3 = !{i32 16}

; CHECK: [[LOCAL_SIZE]] = !{i32 4}

; DEBUGIFY-NOT: WARNING
