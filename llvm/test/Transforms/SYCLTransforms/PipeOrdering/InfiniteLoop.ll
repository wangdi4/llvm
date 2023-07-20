; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; kernel void foo() {
;   // Loop only in for header.
;   while (true)
;     write_channel_intel(ch, 42);
; }

; CHECK-LABEL: while.body:
; CHECK: call i32 @__write_pipe_2_bl_fpga(
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: br label %while.body

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

define dso_local void @foo() #0 !kernel_arg_base_type !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !arg_type_null_val !2 {
entry:
  %write.src = alloca i32, align 4
  br label %while.body

while.body:                                       ; preds = %while.body, %entry
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !5
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  br label %while.body
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch.pipe.bs, ptr addrspace(1) @ch.pipe, align 8
  ret void
}

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #0

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
