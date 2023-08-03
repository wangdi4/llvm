; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compile from OpenCL kernel:
; typedef struct T_ {
;   clk_event_t block_evt[1000][64][32][7];
;   queue_t q[4][2][8];
; } T;
;
; kernel void enqueue_block_multi_queue(global int *res, queue_t q0, queue_t q1) {
;   T t;
;   queue_t *queue = &t.q[0][0][0];
;   ndrange_t ndrange = ndrange_1D(1);
;   res[0] = enqueue_kernel(queue[0], CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 1, NULL,
;                           &t.block_evt[0][0][0][0],
;                           ^{
;                           });
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: [[StructT:%struct.T_.*]] = type { [1000 x [64 x [32 x [7 x ptr]]]], [4 x [2 x [8 x ptr]]] }

%struct.T_ = type { [1000 x [64 x [32 x [7 x target("spirv.DeviceEvent")]]]], [4 x [2 x [8 x target("spirv.Queue")]]] }
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

@__block_literal_global = internal addrspace(1) constant { i32, i32, ptr addrspace(4) } { i32 16, i32 8, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_multi_queue_block_invoke to ptr addrspace(4)) }, align 8 #0

define dso_local spir_kernel void @enqueue_block_multi_queue(ptr addrspace(1) noundef align 4 %res, target("spirv.Queue") %q0, target("spirv.Queue") %q1) #1 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @enqueue_block_multi_queue(ptr addrspace(1) noundef align 4 %res, ptr %q0, ptr %q1)
; CHECK-SAME: !arg_type_null_val [[MD_ARG_TY_NULL:![0-9]+]]
; CHECK: %q0.addr = alloca ptr, align 8
; CHECK: %q1.addr = alloca ptr, align 8
; CHECK: %t = alloca %struct.T_.0, align 8

  %res.addr = alloca ptr addrspace(1), align 8
  %q0.addr = alloca target("spirv.Queue"), align 8
  %q1.addr = alloca target("spirv.Queue"), align 8
  %t = alloca %struct.T_, align 8
  %queue = alloca ptr addrspace(4), align 8
  %ndrange = alloca %struct.ndrange_t, align 8
  %tmp = alloca %struct.ndrange_t, align 8
  store ptr addrspace(1) %res, ptr %res.addr, align 8, !tbaa !7

; CHECK: store ptr %q0, ptr %q0.addr, align 8, !tbaa
; CHECK: store ptr %q1, ptr %q1.addr, align 8, !tbaa
; CHECK: %q = getelementptr inbounds [[StructT]], ptr %t, i32 0, i32 1
; CHECK: %arrayidx = getelementptr inbounds [4 x [2 x [8 x ptr]]], ptr %q, i64 0, i64 0
; CHECK: %arrayidx1 = getelementptr inbounds [2 x [8 x ptr]], ptr %arrayidx, i64 0, i64 0
; CHECK: %arrayidx2 = getelementptr inbounds [8 x ptr], ptr %arrayidx1, i64 0, i64 0

  store target("spirv.Queue") %q0, ptr %q0.addr, align 8, !tbaa !11
  store target("spirv.Queue") %q1, ptr %q1.addr, align 8, !tbaa !11
  %q = getelementptr inbounds %struct.T_, ptr %t, i32 0, i32 1
  %arrayidx = getelementptr inbounds [4 x [2 x [8 x target("spirv.Queue")]]], ptr %q, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [2 x [8 x target("spirv.Queue")]], ptr %arrayidx, i64 0, i64 0
  %arrayidx2 = getelementptr inbounds [8 x target("spirv.Queue")], ptr %arrayidx1, i64 0, i64 0
  %arrayidx2.ascast = addrspacecast ptr %arrayidx2 to ptr addrspace(4)
  store ptr addrspace(4) %arrayidx2.ascast, ptr %queue, align 8, !tbaa !7
  call spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8 %ndrange, i64 noundef 1) #3
  %0 = load ptr addrspace(4), ptr %queue, align 8, !tbaa !7

; CHECK: %arrayidx3 = getelementptr inbounds ptr, ptr addrspace(4)
; CHECK: load ptr, ptr addrspace(4) %arrayidx3, align 8, !tbaa

  %arrayidx3 = getelementptr inbounds target("spirv.Queue"), ptr addrspace(4) %0, i64 0
  %1 = load target("spirv.Queue"), ptr addrspace(4) %arrayidx3, align 8, !tbaa !11
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp, ptr align 8 %ndrange, i64 80, i1 false), !tbaa.struct !13

; CHECK: %block_evt = getelementptr inbounds [[StructT]], ptr %t, i32 0, i32 0
; CHECK: %arrayidx4 = getelementptr inbounds [1000 x [64 x [32 x [7 x ptr]]]], ptr %block_evt, i64 0, i64 0
; CHECK: %arrayidx5 = getelementptr inbounds [64 x [32 x [7 x ptr]]], ptr %arrayidx4, i64 0, i64 0
; CHECK: %arrayidx6 = getelementptr inbounds [32 x [7 x ptr]], ptr %arrayidx5, i64 0, i64 0
; CHECK: %arrayidx7 = getelementptr inbounds [7 x ptr], ptr %arrayidx6, i64 0, i64 0
; CHECK: call spir_func i32 @__enqueue_kernel_basic_events(ptr

  %block_evt = getelementptr inbounds %struct.T_, ptr %t, i32 0, i32 0
  %arrayidx4 = getelementptr inbounds [1000 x [64 x [32 x [7 x target("spirv.DeviceEvent")]]]], ptr %block_evt, i64 0, i64 0
  %arrayidx5 = getelementptr inbounds [64 x [32 x [7 x target("spirv.DeviceEvent")]]], ptr %arrayidx4, i64 0, i64 0
  %arrayidx6 = getelementptr inbounds [32 x [7 x target("spirv.DeviceEvent")]], ptr %arrayidx5, i64 0, i64 0
  %arrayidx7 = getelementptr inbounds [7 x target("spirv.DeviceEvent")], ptr %arrayidx6, i64 0, i64 0
  %2 = addrspacecast ptr %arrayidx7 to ptr addrspace(4)
  %3 = call spir_func i32 @__enqueue_kernel_basic_events(target("spirv.Queue") %1, i32 0, ptr %tmp, i32 1, ptr addrspace(4) null, ptr addrspace(4) %2, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_multi_queue_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)))
  %4 = load ptr addrspace(1), ptr %res.addr, align 8, !tbaa !7
  %arrayidx8 = getelementptr inbounds i32, ptr addrspace(1) %4, i64 0
  store i32 %3, ptr addrspace(1) %arrayidx8, align 4, !tbaa !14
  ret void
}

declare spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8, i64 noundef) #3

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

define internal spir_func void @__enqueue_block_multi_queue_block_invoke(ptr addrspace(4) noundef %.block_descriptor) #3 {
entry:
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  ret void
}

define spir_kernel void @__enqueue_block_multi_queue_block_invoke_kernel(ptr addrspace(4) %0) #3 {
entry:
  call spir_func void @__enqueue_block_multi_queue_block_invoke(ptr addrspace(4) %0)
  ret void
}

declare spir_func i32 @__enqueue_kernel_basic_events(target("spirv.Queue"), i32, ptr, i32, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4))

attributes #0 = { "objc_arc_inert" }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { convergent nounwind }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nounwind }

; CHECK: [[MD_ARG_TY_NULL]] = !{ptr addrspace(1) null, target("spirv.Queue") zeroinitializer, target("spirv.Queue") zeroinitializer}

!0 = !{i32 1, i32 0, i32 0}
!1 = !{!"none", !"none", !"none"}
!2 = !{!"int*", !"queue_t", !"queue_t"}
!3 = !{!"", !"", !""}
!4 = !{!"res", !"q0", !"q1"}
!5 = !{i1 false, i1 false, i1 false}
!6 = !{i32 0, i32 0, i32 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"queue_t", !9, i64 0}
!13 = !{i64 0, i64 4, !14, i64 8, i64 24, !16, i64 32, i64 24, !16, i64 56, i64 24, !16}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !9, i64 0}
!16 = !{!9, !9, i64 0}

; DEBUGIFY-NOT: WARNING
