; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

; Check that pipe type isn't demangled from _ZTS9FetchTask, which is extracted
; from test sycl_fpga_benchmarks/fft1d_offchip.
; Check that type is demangled from the second function name, which is extracted
; from TCE_FPGA_samples/hostpipes.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@_ZN4sycl3_V13ext5intel4pipeI7mwt_in08float2x4Li64EE9m_StorageE.syclpipe = external addrspace(1) global ptr addrspace(1), !packet_size !0, !packet_align !0

; Function _ZTS9FetchTask is extracted from test sycl_fpga_benchmarks/fft1d_offchip

define void @_ZTS9FetchTask(ptr addrspace(1) %_arg_src, ptr %_arg_src1, ptr %_arg_src2, ptr %_arg_src3, i32 %_arg_mangle_int, i32 %_arg_twidle_int, i32 %_arg_log_rows_arg, i32 %_arg_log_columns_arg, i32 %_arg_rows_arg, i32 %_arg_columns_arg) !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !arg_type_null_val !4 {
entry:
; CHECK-LABEL: define void @_ZTS9FetchTask(
; CHECK: call void @__store_write_pipe_use(
; CHECK: call i32 @__write_pipe_2_fpga(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(

  %call.i = call ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZN4sycl3_V13ext5intel4pipeI7mwt_in08float2x4Li64EE9m_StorageE.syclpipe to ptr addrspace(4)))
  %0 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %call.i, ptr addrspace(4) null, i32 32, i32 4)
  ret void
}

define void @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI9H2DPipeIDEEiLi8ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE33__latency_control_bl_read_wrapperIiEEv8ocl_pipePT_iiii(ptr addrspace(1) %Pipe, ptr addrspace(4) %Data, i32 %AnchorID, i32 %TargetAnchor, i32 %Type, i32 %Cycle) {
entry:
; CHECK-LABEL: define void @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI9H2DPipeIDEEiLi8ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE33__latency_control_bl_read_wrapperIiEEv8ocl_pipePT_iiii(
; CHECK: call void @__store_read_pipe_use(
; CHECK: call i32 @__read_pipe_2_fpga(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(

  %Pipe.addr = alloca ptr addrspace(1), align 8
  %Data.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(1) %Pipe, ptr %Pipe.addr, align 8
  %0 = bitcast ptr %Data.addr to ptr
  store ptr addrspace(4) %Data, ptr %0, align 8
  %1 = load ptr addrspace(1), ptr %Pipe.addr, align 8
  %2 = bitcast ptr %Data.addr to ptr
  %3 = load ptr addrspace(4), ptr %2, align 8
  %4 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %1, ptr addrspace(4) %3, i32 4, i32 4)
  ret void
}

declare ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4))

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

!0 = !{i32 4}
!1 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!2 = !{!"struct float2*", !"class.sycl::_V1::range", !"class.sycl::_V1::range", !"class.sycl::_V1::range", !"int", !"int", !"int", !"int", !"int", !"int"}
!3 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!4 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}

; DEBUGIFY-NOT: WARNING
