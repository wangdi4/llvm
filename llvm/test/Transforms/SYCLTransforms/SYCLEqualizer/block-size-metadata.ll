; Source:
; kernel void device_side_enqueue(global int *a, global int *b, int i, char c0) {
;   queue_t default_queue;
;   unsigned flags = 0;
;   ndrange_t ndrange;
;
;   // Emits block literal on stack and block kernel.
;   enqueue_kernel(default_queue, flags, ndrange,
;                  ^(void) {
;                    a[i] = c0;
;                  });
;
;   // Emits global block literal and block kernel.
;   enqueue_kernel(default_queue, flags, ndrange,
;                  ^(local void *p) {
;                    return;
;                  },
;                  256);
;
;   void (^const block_A)(void) = ^{
;     return;
;   };
;
;   void (^const block_B)(local void *) = ^(local void *a) {
;     return;
;   };
;
;   unsigned size = get_kernel_work_group_size(block_A);
;   size = get_kernel_preferred_work_group_size_multiple(block_B);
; }

; Compilation command:
; clang -cc1 -x cl -cl-std=CL2.0 -triple spir64 -emit-llvm -disable-llvm-passes -finclude-default-header set_block_size_metadata.cl -o set_block_size_metadata.ll

; RUN: opt -passes=sycl-kernel-equalizer %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-equalizer %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

@__block_literal_global = internal addrspace(1) constant { i32, i32, ptr addrspace(4) } { i32 16, i32 8, ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_2 to ptr addrspace(4)) }, align 8 #0
@__block_literal_global.1 = internal addrspace(1) constant { i32, i32, ptr addrspace(4) } { i32 16, i32 8, ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_3 to ptr addrspace(4)) }, align 8 #0
@__block_literal_global.2 = internal addrspace(1) constant { i32, i32, ptr addrspace(4) } { i32 16, i32 8, ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_4 to ptr addrspace(4)) }, align 8 #0

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @device_side_enqueue(ptr addrspace(1) noundef align 4 %a, ptr addrspace(1) noundef align 4 %b, i32 noundef %i, i8 noundef signext %c0) #1 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !arg_type_null_val !9 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  %i.addr = alloca i32, align 4
  %c0.addr = alloca i8, align 1
  %default_queue = alloca ptr, align 8
  %flags = alloca i32, align 4
  %ndrange = alloca %struct.ndrange_t, align 8
  %tmp = alloca %struct.ndrange_t, align 8
  %block = alloca <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, align 8
  %tmp3 = alloca %struct.ndrange_t, align 8
  %block_sizes = alloca [1 x i64], align 8
  %block_A = alloca ptr addrspace(4), align 8
  %block_B = alloca ptr addrspace(4), align 8
  %size = alloca i32, align 4
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !10
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !10
  store i32 %i, ptr %i.addr, align 4, !tbaa !14
  store i8 %c0, ptr %c0.addr, align 1, !tbaa !16
  call void @llvm.lifetime.start.p0(i64 8, ptr %default_queue) #5
  call void @llvm.lifetime.start.p0(i64 4, ptr %flags) #5
  store i32 0, ptr %flags, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 80, ptr %ndrange) #5
  %0 = load ptr, ptr %default_queue, align 8, !tbaa !17
  %1 = load i32, ptr %flags, align 4, !tbaa !14
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp, ptr align 8 %ndrange, i64 80, i1 false), !tbaa.struct !19
  %block.size = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 0
  store i32 29, ptr %block.size, align 8
  %block.align = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 1
  store i32 8, ptr %block.align, align 4
  %block.invoke = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 2
  store ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke to ptr addrspace(4)), ptr %block.invoke, align 8
  %block.captured = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 3
  %2 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !10
  store ptr addrspace(1) %2, ptr %block.captured, align 8, !tbaa !10
  %block.captured1 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 4
  %3 = load i32, ptr %i.addr, align 4, !tbaa !14
  store i32 %3, ptr %block.captured1, align 8, !tbaa !14
  %block.captured2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr %block, i32 0, i32 5
  %4 = load i8, ptr %c0.addr, align 1, !tbaa !16
  store i8 %4, ptr %block.captured2, align 4, !tbaa !16
  %5 = addrspacecast ptr %block to ptr addrspace(4)
  %6 = call spir_func i32 @__enqueue_kernel_basic(ptr %0, i32 %1, ptr byval(%struct.ndrange_t) %tmp, ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) %5)
  %7 = load ptr, ptr %default_queue, align 8, !tbaa !17
  %8 = load i32, ptr %flags, align 4, !tbaa !14
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp3, ptr align 8 %ndrange, i64 80, i1 false), !tbaa.struct !19
  call void @llvm.lifetime.start.p0(i64 8, ptr %block_sizes) #5
  %9 = getelementptr [1 x i64], ptr %block_sizes, i32 0, i32 0
  store i64 256, ptr %9, align 8
  %10 = call spir_func i32 @__enqueue_kernel_varargs(ptr %7, i32 %8, ptr %tmp3, ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_2_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), i32 1, ptr %9)
  call void @llvm.lifetime.end.p0(i64 8, ptr %block_sizes) #5
  call void @llvm.lifetime.start.p0(i64 8, ptr %block_A) #5
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global.1 to ptr addrspace(4)), ptr %block_A, align 8, !tbaa !16
  call void @llvm.lifetime.start.p0(i64 8, ptr %block_B) #5
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global.2 to ptr addrspace(4)), ptr %block_B, align 8, !tbaa !16
  call void @llvm.lifetime.start.p0(i64 4, ptr %size) #5
  %11 = call spir_func i32 @__get_kernel_work_group_size_impl(ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_3_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global.1 to ptr addrspace(4)))
  store i32 %11, ptr %size, align 4, !tbaa !14
  %12 = call spir_func i32 @__get_kernel_preferred_work_group_size_multiple_impl(ptr addrspace(4) addrspacecast (ptr @__device_side_enqueue_block_invoke_4_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global.2 to ptr addrspace(4)))
  store i32 %12, ptr %size, align 4, !tbaa !14
  call void @llvm.lifetime.end.p0(i64 4, ptr %size) #5
  call void @llvm.lifetime.end.p0(i64 8, ptr %block_B) #5
  call void @llvm.lifetime.end.p0(i64 8, ptr %block_A) #5
  call void @llvm.lifetime.end.p0(i64 80, ptr %ndrange) #5
  call void @llvm.lifetime.end.p0(i64 4, ptr %flags) #5
  call void @llvm.lifetime.end.p0(i64 8, ptr %default_queue) #5
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke(ptr addrspace(4) noundef %.block_descriptor) #4 {
entry:
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  %block.capture.addr = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 5
  %0 = load i8, ptr addrspace(4) %block.capture.addr, align 4, !tbaa !16
  %conv = sext i8 %0 to i32
  %block.capture.addr1 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 3
  %1 = load ptr addrspace(1), ptr addrspace(4) %block.capture.addr1, align 8, !tbaa !10
  %block.capture.addr2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 4
  %2 = load i32, ptr addrspace(4) %block.capture.addr2, align 8, !tbaa !14
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 %idxprom
  store i32 %conv, ptr addrspace(1) %arrayidx, align 4, !tbaa !14
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_kernel(ptr addrspace(4) %0)
; CHECK-SAME: !block_literal_size ![[BLS1:[0-9]+]]
; Function Attrs: convergent nounwind
define spir_kernel void @__device_side_enqueue_block_invoke_kernel(ptr addrspace(4) %0) #4 !arg_type_null_val !20 {
entry:
  call spir_func void @__device_side_enqueue_block_invoke(ptr addrspace(4) %0)
  ret void
}

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_2(ptr addrspace(4) noundef %.block_descriptor, ptr addrspace(3) noundef %p) #4 {
entry:
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  %p.addr = alloca ptr addrspace(3), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  store ptr addrspace(3) %p, ptr %p.addr, align 8, !tbaa !10
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_2_kernel(ptr addrspace(4) %0, ptr addrspace(3) %1)
; CHECK-SAME: !block_literal_size ![[BLS2:[0-9]+]]
; Function Attrs: convergent nounwind
define spir_kernel void @__device_side_enqueue_block_invoke_2_kernel(ptr addrspace(4) %0, ptr addrspace(3) %1) #4 !arg_type_null_val !21 {
entry:
  call spir_func void @__device_side_enqueue_block_invoke_2(ptr addrspace(4) %0, ptr addrspace(3) %1)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_3(ptr addrspace(4) noundef %.block_descriptor) #4 {
entry:
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  ret void
}

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_4(ptr addrspace(4) noundef %.block_descriptor, ptr addrspace(3) noundef %a) #4 {
entry:
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  %a.addr = alloca ptr addrspace(3), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  store ptr addrspace(3) %a, ptr %a.addr, align 8, !tbaa !10
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_3_kernel(ptr addrspace(4) %0)
; CHECK-SAME: !block_literal_size ![[BLS2]]
; Function Attrs: convergent nounwind
define spir_kernel void @__device_side_enqueue_block_invoke_3_kernel(ptr addrspace(4) %0) #4 !arg_type_null_val !20 {
entry:
  call spir_func void @__device_side_enqueue_block_invoke_3(ptr addrspace(4) %0)
  ret void
}

declare spir_func i32 @__get_kernel_work_group_size_impl(ptr addrspace(4), ptr addrspace(4))

; CHECK: void @__device_side_enqueue_block_invoke_4_kernel(ptr addrspace(4) %0, ptr addrspace(3) %1)
; CHECK-SAME: !block_literal_size ![[BLS2]]
; Function Attrs: convergent nounwind
define spir_kernel void @__device_side_enqueue_block_invoke_4_kernel(ptr addrspace(4) %0, ptr addrspace(3) %1) #4 !arg_type_null_val !21 {
entry:
  call spir_func void @__device_side_enqueue_block_invoke_4(ptr addrspace(4) %0, ptr addrspace(3) %1)
  ret void
}
; %block = alloca <{ i32, i32, ptr addrspace(4), ptr addrspace(1), i32, i8 }>, size = 29
; CHECK: ![[BLS1]] = !{i32 29}
; First element of __block_literal_global, __block_literal_global.1 and __block_literal_global.1 are 16
; CHECK: ![[BLS2]] = !{i32 16}

declare spir_func i32 @__get_kernel_preferred_work_group_size_multiple_impl(ptr addrspace(4), ptr addrspace(4))

declare spir_func i32 @__enqueue_kernel_basic(ptr, i32, ptr, ptr addrspace(4), ptr addrspace(4))

declare spir_func i32 @__enqueue_kernel_varargs(ptr, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr)

attributes #0 = { "objc_arc_inert" }
attributes #1 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #5 = { nounwind }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{i32 1, i32 1, i32 0, i32 0}
!4 = !{!"none", !"none", !"none", !"none"}
!5 = !{!"int*", !"int*", !"int", !"char"}
!6 = !{!"", !"", !"", !""}
!7 = !{i1 false, i1 false, i1 false, i1 false}
!8 = !{i32 0, i32 0, i32 0, i32 0}
!9 = !{ptr addrspace(1) null, ptr addrspace(1) null, i32 0, i8 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"any pointer", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}
!16 = !{!12, !12, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"queue_t", !12, i64 0}
!19 = !{i64 0, i64 4, !14, i64 8, i64 24, !16, i64 32, i64 24, !16, i64 56, i64 24, !16}
!20 = !{ptr addrspace(4) null}
!21 = !{ptr addrspace(4) null, ptr addrspace(3) null}

; DEBUGIFY-NOT: WARNING
