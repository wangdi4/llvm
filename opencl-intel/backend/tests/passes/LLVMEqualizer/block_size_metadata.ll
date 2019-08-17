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

; RUN: %oclopt -llvm-equalizer -verify -S %s | FileCheck %s

; ModuleID = 'set_block_size_metadata.cl'
source_filename = "set_block_size_metadata.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"

%opencl.queue_t = type opaque
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

@__block_literal_global = internal addrspace(1) constant { i32, i32 } { i32 8, i32 4 }, align 4
@__block_literal_global.1 = internal addrspace(1) constant { i32, i32 } { i32 8, i32 4 }, align 4
@__block_literal_global.2 = internal addrspace(1) constant { i32, i32 } { i32 8, i32 4 }, align 4

; Function Attrs: convergent nounwind
define spir_kernel void @device_side_enqueue(i32 addrspace(1)* %a, i32 addrspace(1)* %b, i32 %i, i8 signext %c0) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
  %default_queue = alloca %opencl.queue_t*, align 8
  %flags = alloca i32, align 4
  %ndrange = alloca %struct.ndrange_t, align 8
  %block = alloca <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, align 8
  %tmp = alloca %struct.ndrange_t, align 8
  %block_A = alloca void () addrspace(4)*, align 8
  %block_B = alloca void (i8 addrspace(3)*) addrspace(4)*, align 8
  %size = alloca i32, align 4
  store i32 0, i32* %flags, align 4, !tbaa !14
  %0 = load %opencl.queue_t*, %opencl.queue_t** %default_queue, align 8, !tbaa !17
  %1 = load i32, i32* %flags, align 4, !tbaa !14
  %block.size = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block, i32 0, i32 0
  store i32 21, i32* %block.size, align 8
  %block.align = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block, i32 0, i32 1
  store i32 8, i32* %block.align, align 4
  %block.captured = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block, i32 0, i32 2
  store i32 addrspace(1)* %a, i32 addrspace(1)** %block.captured, align 8, !tbaa !10
  %block.captured1 = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block, i32 0, i32 3
  store i32 %i, i32* %block.captured1, align 8, !tbaa !14
  %block.captured2 = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block, i32 0, i32 4
  store i8 %c0, i8* %block.captured2, align 4, !tbaa !16
  %2 = bitcast <{ i32, i32, i32 addrspace(1)*, i32, i8 }>* %block to void ()*
  %3 = addrspacecast void ()* %2 to i8 addrspace(4)*
  %call1 = call i32 @__enqueue_kernel_basic(%opencl.queue_t* %0, i32 %1, %struct.ndrange_t* byval %tmp, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__device_side_enqueue_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* %3)
  %4 = alloca [1 x i64]
  %5 = getelementptr [1 x i64], [1 x i64]* %4, i32 0, i32 0
  store i64 256, i64* %5, align 8
  %call2 = call i32 @__enqueue_kernel_varargs(%opencl.queue_t* %0, i32 %1, %struct.ndrange_t* %tmp, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*)* @__device_side_enqueue_block_invoke_2_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32 } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), i32 1, i64* %5)
  %call3 = call i32 @__get_kernel_work_group_size_impl(i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__device_side_enqueue_block_invoke_3_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32 } addrspace(1)* @__block_literal_global.1 to i8 addrspace(1)*) to i8 addrspace(4)*))
  store i32 %call3, i32* %size, align 4, !tbaa !14
  %call4 = call i32 @__get_kernel_preferred_work_group_size_multiple_impl(i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*)* @__device_side_enqueue_block_invoke_4_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32 } addrspace(1)* @__block_literal_global.2 to i8 addrspace(1)*) to i8 addrspace(4)*))
  store i32 %call4, i32* %size, align 4, !tbaa !14
  ret void
}

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke(i8 addrspace(4)* %.block_descriptor) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32, i32 addrspace(1)*, i32, i8 }> addrspace(4)*
  %block.capture.addr = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }> addrspace(4)* %block, i32 0, i32 4
  %0 = load i8, i8 addrspace(4)* %block.capture.addr, align 4, !tbaa !16
  %conv = sext i8 %0 to i32
  %block.capture.addr1 = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }> addrspace(4)* %block, i32 0, i32 2
  %1 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %block.capture.addr1, align 8, !tbaa !10
  %block.capture.addr2 = getelementptr inbounds <{ i32, i32, i32 addrspace(1)*, i32, i8 }>, <{ i32, i32, i32 addrspace(1)*, i32, i8 }> addrspace(4)* %block, i32 0, i32 3
  %2 = load i32, i32 addrspace(4)* %block.capture.addr2, align 8, !tbaa !14
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 %idxprom
  store i32 %conv, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_kernel(i8 addrspace(4)* %0) #2 !block_literal_size [[KER0:![0-9]+]]

; Function Attrs: nounwind
define internal spir_kernel void @__device_side_enqueue_block_invoke_kernel(i8 addrspace(4)*) #3 {
entry:
  call void @__device_side_enqueue_block_invoke(i8 addrspace(4)* %0)
  ret void
}

declare i32 @__enqueue_kernel_basic(%opencl.queue_t*, i32, %struct.ndrange_t*, i8 addrspace(4)*, i8 addrspace(4)*)

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_2(i8 addrspace(4)* %.block_descriptor, i8 addrspace(3)* %p) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  %p.addr = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32 }> addrspace(4)*
  store i8 addrspace(3)* %p, i8 addrspace(3)** %p.addr, align 8, !tbaa !10
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_2_kernel(i8 addrspace(4)* %0, i8 addrspace(3)* %1) #2 !block_literal_size [[KER1:![0-9]+]]

; Function Attrs: nounwind
define internal spir_kernel void @__device_side_enqueue_block_invoke_2_kernel(i8 addrspace(4)*, i8 addrspace(3)*) #3 {
entry:
  call void @__device_side_enqueue_block_invoke_2(i8 addrspace(4)* %0, i8 addrspace(3)* %1)
  ret void
}

declare i32 @__enqueue_kernel_varargs(%opencl.queue_t*, i32, %struct.ndrange_t*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i64*)

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_3(i8 addrspace(4)* %.block_descriptor) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32 }> addrspace(4)*
  ret void
}

; Function Attrs: convergent nounwind
define internal spir_func void @__device_side_enqueue_block_invoke_4(i8 addrspace(4)* %.block_descriptor, i8 addrspace(3)* %a) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  %a.addr = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32 }> addrspace(4)*
  store i8 addrspace(3)* %a, i8 addrspace(3)** %a.addr, align 8, !tbaa !10
  ret void
}

; CHECK: void @__device_side_enqueue_block_invoke_3_kernel(i8 addrspace(4)* %0) #2 !block_literal_size [[KER1]]

; Function Attrs: nounwind
define internal spir_kernel void @__device_side_enqueue_block_invoke_3_kernel(i8 addrspace(4)*) #3 {
entry:
  call void @__device_side_enqueue_block_invoke_3(i8 addrspace(4)* %0)
  ret void
}

declare i32 @__get_kernel_work_group_size_impl(i8 addrspace(4)*, i8 addrspace(4)*)

; CHECK: void @__device_side_enqueue_block_invoke_4_kernel(i8 addrspace(4)* %0, i8 addrspace(3)* %1) #2 !block_literal_size [[KER1]]

; Function Attrs: nounwind
define internal spir_kernel void @__device_side_enqueue_block_invoke_4_kernel(i8 addrspace(4)*, i8 addrspace(3)*) #3 {
entry:
  call void @__device_side_enqueue_block_invoke_4(i8 addrspace(4)* %0, i8 addrspace(3)* %1)
  ret void
}

declare i32 @__get_kernel_preferred_work_group_size_multiple_impl(i8 addrspace(4)*, i8 addrspace(4)*)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"clang version 7.0.0"}
!4 = !{i32 1, i32 1, i32 0, i32 0}
!5 = !{!"none", !"none", !"none", !"none"}
!6 = !{!"int*", !"int*", !"int", !"char"}
!7 = !{!"", !"", !"", !""}
!8 = !{i1 false, i1 false, i1 false, i1 false}
!9 = !{i32 0, i32 0, i32 0, i32 0}
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

; CHECK: [[KER0]] = !{i32 21}
; CHECK: [[KER1]] = !{i32 8}
