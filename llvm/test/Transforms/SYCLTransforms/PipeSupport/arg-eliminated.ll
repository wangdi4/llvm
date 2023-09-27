; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

; Check the pass can handle the case that an unused argument is eliminated.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch_in0 = external addrspace(1) global target("spirv.Channel"), !packet_size !0, !packet_align !0

define fastcc i32 @__io_pipe_0_0_bar(ptr addrspace(1) %ch0) !arg_type_null_val !1 {
entry:
; CHECK: call void @__store_read_pipe_use(ptr {{.*}}, ptr {{.*}}, ptr addrspace(1) null)
; CHECK: {{.*}}all11 = call i32 @__read_pipe_2_io_fpga(ptr addrspace(1) null, ptr addrspace(4) null, ptr addrspace(4) null, i32 0, i32 0)
; CHECK: call void @__flush_pipe_read_array(ptr {{.*}}, ptr {{.*}})
; CHECK: call void @__flush_pipe_write_array(ptr {{.*}}, ptr {{.*}})
; CHECK: call void @__flush_pipe_read_array(ptr {{.*}}, ptr {{.*}})
; CHECK: call void @__flush_pipe_write_array(ptr {{.*}}, ptr {{.*}})

  %call11 = call i32 @__read_pipe_2_bl_io_fpga(ptr addrspace(1) null, ptr addrspace(4) null, ptr addrspace(4) null, i32 0, i32 0)
  ret i32 0
}

declare i32 @__read_pipe_2_bl_io_fpga(ptr addrspace(1), ptr addrspace(4), ptr addrspace(4), i32, i32)

!0 = !{i32 4}
!1 = !{i32 0, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}

; DEBUGIFY-NOT: WARNING
