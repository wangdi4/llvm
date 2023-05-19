; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; kernel void test_write(write_only pipe int p, int data) {
;   write_pipe(p, &data);
; }
; kernel void test_read(read_only pipe int p, int data) { read_pipe(p, &data); }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local spir_kernel void @test_write(target("spirv.Pipe", 1) %p, i32 noundef %data) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
; CHECK: define dso_local spir_kernel void @test_write(ptr addrspace(1) %p, i32 noundef %data) {{.*}} !arg_type_null_val [[MD_ARG_TY_W_NULL:![0-9]+]]
; CHECK: %p.addr = alloca ptr addrspace(1), align 8
; CHECK: store ptr addrspace(1) %p, ptr %p.addr, align 8
; CHECK: load ptr addrspace(1), ptr %p.addr, align 8
; CHECK: call spir_func i32 @__write_pipe_2(ptr addrspace(1) {{.*}}, ptr addrspace(4) {{.*}}, i32 4, i32 4)

  %p.addr = alloca target("spirv.Pipe", 1), align 8
  %data.addr = alloca i32, align 4
  store target("spirv.Pipe", 1) %p, ptr %p.addr, align 8, !tbaa !8
  store i32 %data, ptr %data.addr, align 4, !tbaa !11
  %0 = load target("spirv.Pipe", 1), ptr %p.addr, align 8, !tbaa !8
  %1 = addrspacecast ptr %data.addr to ptr addrspace(4)
  %2 = call spir_func i32 @__write_pipe_2(target("spirv.Pipe", 1) %0, ptr addrspace(4) %1, i32 4, i32 4)
  ret void
}

declare spir_func i32 @__write_pipe_2(target("spirv.Pipe", 1), ptr addrspace(4), i32, i32)

define dso_local spir_kernel void @test_read(target("spirv.Pipe", 0) %p, i32 noundef %data) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !13 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
; CHECK: define dso_local spir_kernel void @test_read(ptr addrspace(1) %p, i32 noundef %data) {{.*}} !arg_type_null_val [[MD_ARG_TY_R_NULL:![0-9]+]]
; CHECK: %p.addr = alloca ptr addrspace(1), align 8
; CHECK: store ptr addrspace(1) %p, ptr %p.addr, align 8
; CHECK: load ptr addrspace(1), ptr %p.addr, align 8
; CHECK: call spir_func i32 @__read_pipe_2(ptr addrspace(1) {{.*}}, ptr addrspace(4) {{.*}}, i32 4, i32 4)

  %p.addr = alloca target("spirv.Pipe", 0), align 8
  %data.addr = alloca i32, align 4
  store target("spirv.Pipe", 0) %p, ptr %p.addr, align 8, !tbaa !8
  store i32 %data, ptr %data.addr, align 4, !tbaa !11
  %0 = load target("spirv.Pipe", 0), ptr %p.addr, align 8, !tbaa !8
  %1 = addrspacecast ptr %data.addr to ptr addrspace(4)
  %2 = call spir_func i32 @__read_pipe_2(target("spirv.Pipe", 0) %0, ptr addrspace(4) %1, i32 4, i32 4)
  ret void
}

; CHECK: declare spir_func i32 @__read_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32)

declare spir_func i32 @__read_pipe_2(target("spirv.Pipe", 0), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }

; CHECK: [[MD_ARG_TY_W_NULL]] = !{target("spirv.Pipe", 1) zeroinitializer, i32 0}
; CHECK: [[MD_ARG_TY_R_NULL]] = !{target("spirv.Pipe", 0) zeroinitializer, i32 0}

!0 = !{i32 1, i32 0}
!1 = !{!"write_only", !"none"}
!2 = !{!"int", !"int"}
!3 = !{!"pipe", !""}
!4 = !{!"p", !"data"}
!5 = !{i1 false, i1 false}
!6 = !{i32 0, i32 0}
!7 = !{!"", !""}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !9, i64 0}
!13 = !{!"read_only", !"none"}

; DEBUGIFY-NOT: WARNING
