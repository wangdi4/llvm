; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch2;
;
; __kernel void k(__global int *cond, __global int *res) {
;   *res = (cond) ? read_channel_intel(ch1) : read_channel_intel(ch2);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL1.2
; Opt options: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-kernel-channel-pipe-transformation -S
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

; CHECK:      call void @__store_read_pipe_use({{.*}} ptr addrspace(1) %[[PIPERO:[0-9]+]]
; CHECK:      %[[CALL:.+]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %[[PIPERO]]
; CHECK:      %[[ICMP:.+]] = icmp ne i32 %[[CALL]], 0
; CHECK-NEXT: br i1 %[[ICMP]], label %[[FLUSHBB:[0-9]+]]
; CHECK:      [[FLUSHBB]]:
; CHECK-NEXT: call void @__flush_pipe_read_array
; CHECK-NEXT: call void @__flush_pipe_write_array

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch1 = internal local_unnamed_addr addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@ch2 = internal local_unnamed_addr addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch2.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch2.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent norecurse nounwind
define dso_local void @k(ptr addrspace(1) noundef readnone align 4 %cond, ptr addrspace(1) nocapture noundef writeonly align 4 %res) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !11 {
entry:
  %read.dst = alloca i32, align 4
  %tobool.not = icmp eq ptr addrspace(1) %cond, null
  br i1 %tobool.not, label %cond.false, label %cond.true

cond.true:                                        ; preds = %entry
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch1.pipe, align 8, !tbaa !12
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !12
  %2 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call2 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %3 = load i32, ptr %read.dst, align 4
  br label %cond.end

cond.false:                                       ; preds = %entry
  %4 = load ptr addrspace(1), ptr addrspace(1) @ch2.pipe, align 8, !tbaa !12
  %5 = load ptr addrspace(1), ptr addrspace(1) @ch2, align 4, !tbaa !12
  %6 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call11 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %4, ptr addrspace(4) %6, i32 4, i32 4)
  %7 = load i32, ptr %read.dst, align 4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond2 = phi i32 [ %3, %cond.true ], [ %7, %cond.false ]
  store i32 %cond2, ptr addrspace(1) %res, align 4, !tbaa !15
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch1.pipe.bs, ptr addrspace(1) @ch1.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch2.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch2.pipe.bs, ptr addrspace(1) @ch2.pipe, align 8
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_fpga(ptr addrspace(1), i32, i32, i32) #1

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nounwind memory(none) }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!4 = !{ptr @k}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{ptr addrspace(1) null, ptr addrspace(1) null}
!12 = !{!13, !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !13, i64 0}

; DEBUGIFY-NOT: WARNING
