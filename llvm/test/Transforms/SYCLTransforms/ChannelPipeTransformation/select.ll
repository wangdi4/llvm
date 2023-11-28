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
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL1.2 -finclude-default-header
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

; CHECK: @[[PIPE1:.*]] = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8,
; CHECK: @[[PIPE2:.*]] = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8,

; CHECK: %[[PIPE1VAL:.*]] = load ptr addrspace(1), ptr addrspace(1) @[[PIPE1]]
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %[[PIPE1VAL]]
; CHECK: %[[VAL1:.*]] = load i32, ptr %read.dst
; CHECK: %[[PIPE2VAL:.*]] = load ptr addrspace(1), ptr addrspace(1) @[[PIPE2]]
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %[[PIPE2VAL]]
; CHECK: %[[VAL2:.*]] = load i32, ptr %read.dst
; CHECK: phi i32 [ %[[VAL1]], %cond.true ], [ %[[VAL2]], %cond.false ]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch1 = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch2 = local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0

; Function Attrs: convergent norecurse nounwind
define dso_local void @k(ptr addrspace(1) noundef readnone align 4 %cond, ptr addrspace(1) nocapture noundef writeonly align 4 %res) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !11 {
entry:
  %tobool.not = icmp eq ptr addrspace(1) %cond, null
  br i1 %tobool.not, label %cond.false, label %cond.true

cond.true:                                        ; preds = %entry
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !12
  %call = tail call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #2
  br label %cond.end

cond.false:                                       ; preds = %entry
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch2, align 4, !tbaa !12
  %call1 = tail call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %1) #2
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond2 = phi i32 [ %call, %cond.true ], [ %call1, %cond.false ]
  store i32 %cond2, ptr addrspace(1) %res, align 4, !tbaa !15
  ret void
}

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #1

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
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

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function k -- %read.dst = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor -- call void @__pipe_init_fpga(ptr addrspace(1) @ch1.bs, i32 4, i32 0, i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor -- store ptr addrspace(1) @ch1.bs, ptr addrspace(1) @ch1, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor -- call void @__pipe_init_fpga(ptr addrspace(1) @ch2.bs, i32 4, i32 0, i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor -- store ptr addrspace(1) @ch2.bs, ptr addrspace(1) @ch2, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor -- ret void
; DEBUGIFY-NOT: WARNING
