; RUN: opt -passes=sycl-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-local-buffers -S %s | FileCheck %s

; Check local variable used in noinline function which is called by both scalar
; kernel and vector kernel has the same offset in both kernels.
; Local buffer size is also the same for both kernels.
; Note that barrier loops are removed for the sake of simplicity.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32

; CHECK: @test.i = external addrspace(3) global i32

define internal fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) {
entry:
; CHECK-LABEL: define internal fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-NOT: !local_buffer_size
; CHECK-SAME: {
; CHECK: %0 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: store i32 1, ptr addrspace(3) %0,
;
  store i32 1, ptr addrspace(3) @test.i, align 4, !tbaa !1
  ret void
}

define dso_local void @test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) !vectorized_kernel !5 !vectorized_width !6 !kernel_arg_base_type !9 !arg_type_null_val !10 {
LoopEnd_0:
; CHECK-LABEL: define dso_local void @test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE:![0-9]+]]
; CHECK: %0 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: store i32 {{.*}}, ptr addrspace(3) %0
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: [[LOAD:%[0-9]+]] = load i32, ptr addrspace(3) %0
; CHECK: store i32 [[LOAD]], ptr addrspace(1) %dst
;
  store i32 0, ptr addrspace(3) @test.i, align 4, !tbaa !1
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  %0 = load i32, ptr addrspace(3) @test.i, align 4, !tbaa !1
  store i32 %0, ptr addrspace(1) %dst, align 4
  ret void
}

define dso_local void @_ZGVeN16u_test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) !vectorized_width !7 !vectorization_dimension !8 !scalar_kernel !0 !kernel_arg_base_type !9 !arg_type_null_val !10 {
LoopEnd_0:
; CHECK-LABEL: define dso_local void @_ZGVeN16u_test(ptr addrspace(1) noundef align 4 %dst, ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE]]
; CHECK: %0 = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: store i32 {{.*}}, ptr addrspace(3) %0,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK: [[LOAD1:%[0-9]+]] = load i32, ptr addrspace(3) %0,
; CHECK: store i32 [[LOAD1]], ptr addrspace(1) %dst,
;
  store i32 0, ptr addrspace(3) @test.i, align 4
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  call fastcc void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr null, ptr null, [4 x i64] zeroinitializer, ptr null, ptr null)
  %0 = load i32, ptr addrspace(3) @test.i, align 4
  store i32 %0, ptr addrspace(1) %dst, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{ptr @_ZGVeN16u_test}
!6 = !{i32 1}
!7 = !{i32 16}
!8 = !{i32 0}
!9 = !{!"int*"}
!10 = !{ptr addrspace(1) null}

; CHECK: [[LOCAL_SIZE]] = !{i32 4}

; DEBUGIFY-NOT: WARNING
