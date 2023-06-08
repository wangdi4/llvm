; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int cin;
; channel int cout;
;
; __kernel void k1() {
;   int i = read_channel_intel(cin);
;   write_channel_intel(cout, i);
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -x cl -disable-llvm-passes -emit-llvm -cl-std=CL1.2
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -sycl-kernel-equalizer -sycl-kernel-channel-pipe-transformation %s -S
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@cin = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@cout = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@cin.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@cin.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@cout.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@cout.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent norecurse nounwind
define dso_local void @k1() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
  %write.src = alloca i32, align 4
  %i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @cin.pipe, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) @cin, align 4, !tbaa !5
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  ; CHECK:      call void @__store_read_pipe_use
  ; CHECK: br label %[[BR0:.*]]
  ; CHECK: [[BR0]]
  ; CHECK-NEXT: call i32 @__read_pipe_2_fpga
  ; CHECK:      call void @__flush_pipe_read_array
  ; CHECK-NEXT: call void @__flush_pipe_write_array
  ; CHECK-NEXT: br label %[[BR0]]

  %3 = load i32, ptr %write.src, align 4
  store i32 %3, ptr %i, align 4, !tbaa !8
  %4 = load ptr addrspace(1), ptr addrspace(1) @cout.pipe, align 8, !tbaa !5
  %5 = load ptr addrspace(1), ptr addrspace(1) @cout, align 4, !tbaa !5
  %6 = load i32, ptr %i, align 4, !tbaa !8
  store i32 %6, ptr %write.src, align 4
  %7 = addrspacecast ptr %write.src to ptr addrspace(4)
  %8 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %4, ptr addrspace(4) %7, i32 4, i32 4)
  ; CHECK:      call void @__store_write_pipe_use
  ; CHECK: br label %[[BR1:.*]]
  ; CHECK: [[BR1]]
  ; CHECK-NEXT: call i32 @__write_pipe_2_fpga
  ; CHECK:      call void @__flush_pipe_read_array
  ; CHECK-NEXT: call void @__flush_pipe_write_array
  ; CHECK-NEXT: br label %[[BR1]]

  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @cin.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @cin.pipe.bs, ptr addrspace(1) @cin.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @cout.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @cout.pipe.bs, ptr addrspace(1) @cout.pipe, align 8
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_fpga(ptr addrspace(1), i32, i32, i32) #2

; Function Attrs: nounwind memory(none)
declare i32 @__write_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture readonly, i32, i32) #2

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #2

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind memory(none) }
attributes #3 = { nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!4 = !{ptr @k1}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}

; DEBUGIFY-NOT: WARNING
