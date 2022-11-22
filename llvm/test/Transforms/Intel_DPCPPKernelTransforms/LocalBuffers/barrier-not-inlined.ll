; RUN: opt -dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-local-buffers -S %s | FileCheck %s

; Check local variable used in noinline function which is called by both scalar
; kernel and vector kernel has the same offset in both kernels.
; Local buffer size is also the same for both kernels.
; Note that barrier loops are removed for the sake of simplicity.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32

; CHECK-NOT: @test.i =

define internal fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle) {
entry:
; CHECK-LABEL: define internal fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-NOT: !local_buffer_size
; CHECK-SAME: {
; CHECK: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[BC:%[0-9]+]] = bitcast i8 addrspace(3)* [[GEP]] to i32 addrspace(3)*
; CHECK: store i32 1, i32 addrspace(3)* [[BC]],
;
  store i32 1, i32 addrspace(3)* @test.i, align 4, !tbaa !1
  ret void
}

define dso_local void @test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle) !vectorized_kernel !5 !vectorized_width !6 {
LoopEnd_0:
; CHECK-LABEL: define dso_local void @test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE:![0-9]+]]
; CHECK: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[BC:%[0-9]+]] = bitcast i8 addrspace(3)* [[GEP]] to i32 addrspace(3)*
; CHECK: store i32 {{.*}}, i32 addrspace(3)* [[BC]]
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: [[LOAD:%[0-9]+]] = load i32, i32 addrspace(3)* [[BC]]
; CHECK: store i32 [[LOAD]], i32 addrspace(1)* %dst
;
  store i32 0, i32 addrspace(3)* @test.i, align 4, !tbaa !1
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  %0 = load i32, i32 addrspace(3)* @test.i, align 4, !tbaa !1
  store i32 %0, i32 addrspace(1)* %dst, align 4
  ret void
}

define dso_local void @_ZGVeN16u_test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle) !vectorized_width !7 !vectorization_dimension !8 !scalar_kernel !0 {
LoopEnd_0:
; CHECK-LABEL: define dso_local void @_ZGVeN16u_test(i32 addrspace(1)* noundef align 4 %dst, i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE]]
; CHECK: [[GEP:%[0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[BC:%[0-9]+]] = bitcast i8 addrspace(3)* [[GEP]] to i32 addrspace(3)*
; CHECK: store i32 {{.*}}, i32 addrspace(3)* [[BC]],
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK: [[LOAD1:%[0-9]+]] = load i32, i32 addrspace(3)* [[BC]],
; CHECK: store i32 [[LOAD1]], i32 addrspace(1)* %dst,
;
  store i32 0, i32 addrspace(3)* @test.i, align 4
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  call fastcc void @foo(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* null, i64* null, [4 x i64] zeroinitializer, i8* null, {}* null)
  %0 = load i32, i32 addrspace(3)* @test.i, align 4
  store i32 %0, i32 addrspace(1)* %dst, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @test}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @_ZGVeN16u_test}
!6 = !{i32 1}
!7 = !{i32 16}
!8 = !{i32 0}

; CHECK: [[LOCAL_SIZE]] = !{i32 4}

; DEBUGIFY-NOT: WARNING
