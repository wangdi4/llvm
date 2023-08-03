; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-io-transform %s -S | FileCheck %s
;
; This test checks that io pipe is replaced with builtin correctly when
; user-defined functions are not inlined.
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer,sycl-kernel-channel-pipe-transformation -verify %s -S
;
; #pragma OPENCL EXTENSION cl_intel_channels:enable
;
; __attribute__((noinline))
; int bar(read_only pipe int pipe0, read_only pipe int pipe1) {
;   int result;
;   while (!read_pipe(pipe0, &result)) {};
;   while (!read_pipe(pipe1, &result)) {};
;   return result;
; }
;
; __attribute__((noinline))
; int foo(read_only pipe int pipe0, read_only pipe int pipe1) {
;   return bar(pipe0, pipe1);
; }
;
; __attribute__((noinline))
; void dummy(write_only pipe int pipe0, read_only pipe int pipe1) {
;   int result;
;   read_pipe(pipe1, &result);
;   write_pipe(pipe0, &result);
; }
;
; __attribute__((noinline))
; int readPipeHelper(read_only pipe int pipe0, read_only pipe int pipe1) {
;   return foo(pipe0, pipe1) + foo(pipe1, pipe0);
; }
;
; __kernel void readFromFileViaPipe1(
;     read_only pipe int pipe0 __attribute__((io("pipe-io-0"))),
;     read_only pipe int pipe1 __attribute__((io("pipe-io-1"))),
;     read_only pipe int pipe2,
;     __global int *data) {
;   data[0] = readPipeHelper(pipe0, pipe2);
;   data[1] = readPipeHelper(pipe2, pipe1);
; }
;
; __kernel void readFromFileViaPipe2(
;     read_only pipe int pipe0 __attribute__((io("pipe-io-0"))),
;     read_only pipe int pipe1 __attribute__((io("pipe-io-1"))),
;     read_only pipe int pipe2,
;     __global int *data) {
;   data[0] = readPipeHelper(pipe0, pipe2);
;   data[1] = readPipeHelper(pipe2, pipe1);
; }
;
; __kernel void readFromFileViaPipe3(
;     read_only pipe int pipe0 __attribute__((io("pipe-io-0"))),
;     read_only pipe int pipe1 __attribute__((io("pipe-io-2"))),
;     read_only pipe int pipe2,
;     write_only pipe int pipe3, __attribute__((io("pipe-io-3"))),
;     __global int *data) {
;   data[0] = readPipeHelper(pipe0, pipe2);
;   data[1] = readPipeHelper(pipe2, pipe1);
;   data[2] = readPipeHelper(pipe0, pipe1);
;   dummy(pipe3, pipe0);
;   dummy(pipe3, pipe2);
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

; CHECK-LABEL: @bar
; CHECK: call i32 @__read_pipe_2_fpga
; CHECK: call i32 @__read_pipe_2_fpga
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @bar(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1) #1 !arg_type_null_val !4 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  %result = alloca i32, align 4
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr %result) #4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = addrspacecast ptr %result to ptr addrspace(4)
  %2 = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4)
  %tobool = icmp ne i32 %2, 0
  %lnot = xor i1 %tobool, true
  br i1 %lnot, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  br label %while.cond

while.end:                                        ; preds = %while.cond
  br label %while.cond1

while.cond1:                                      ; preds = %while.body4, %while.end
  %3 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %4 = addrspacecast ptr %result to ptr addrspace(4)
  %5 = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %3, ptr addrspace(4) %4, i32 4, i32 4)
  %tobool2 = icmp ne i32 %5, 0
  %lnot3 = xor i1 %tobool2, true
  br i1 %lnot3, label %while.body4, label %while.end5

while.body4:                                      ; preds = %while.cond1
  br label %while.cond1

while.end5:                                       ; preds = %while.cond1
  %6 = load i32, ptr %result, align 4, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 4, ptr %result) #4
  ret i32 %6
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; CHECK-LABEL: @foo
; CHECK: call i32 @bar
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @foo(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1) #1 !arg_type_null_val !4 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call = call i32 @bar(ptr addrspace(1) %0, ptr addrspace(1) %1) #5
  ret i32 %call
}

; CHECK-LABEL: @dummy
; CHECK: call i32 @__read_pipe_2_fpga
; CHECK: call i32 @__write_pipe_2_fpga
; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @dummy(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1) #1 !arg_type_null_val !10 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  %result = alloca i32, align 4
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr %result) #4
  %0 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %1 = addrspacecast ptr %result to ptr addrspace(4)
  %2 = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4)
  %3 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %4 = addrspacecast ptr %result to ptr addrspace(4)
  %5 = call i32 @__write_pipe_2_fpga(ptr addrspace(1) %3, ptr addrspace(4) %4, i32 4, i32 4)
  call void @llvm.lifetime.end.p0(i64 4, ptr %result) #4
  ret void
}

; CHECK-LABEL: @readPipeHelper
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @readPipeHelper(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1) #1 !arg_type_null_val !4 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call = call i32 @foo(ptr addrspace(1) %0, ptr addrspace(1) %1) #5
  %2 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %3 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %call1 = call i32 @foo(ptr addrspace(1) %2, ptr addrspace(1) %3) #5
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

; CHECK-LABEL: @readFromFileViaPipe1
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_1_readPipeHelper
; Function Attrs: convergent norecurse nounwind
define dso_local void @readFromFileViaPipe1(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1, ptr addrspace(1) %pipe2, ptr addrspace(1) noundef align 4 %data) #2 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !17 !kernel_arg_buffer_location !18 !arg_type_null_val !19 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  %pipe2.addr = alloca ptr addrspace(1), align 8
  %data.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe2, ptr %pipe2.addr, align 8, !tbaa !5
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %call = call i32 @readPipeHelper(ptr addrspace(1) %0, ptr addrspace(1) %1) #5
  %2 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %2, i64 0
  store i32 %call, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %3 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %4 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call1 = call i32 @readPipeHelper(ptr addrspace(1) %3, ptr addrspace(1) %4) #5
  %5 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %5, i64 1
  store i32 %call1, ptr addrspace(1) %arrayidx2, align 4, !tbaa !8
  ret void
}

; CHECK-LABEL: @readFromFileViaPipe2
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_1_readPipeHelper
; Function Attrs: convergent norecurse nounwind
define dso_local void @readFromFileViaPipe2(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1, ptr addrspace(1) %pipe2, ptr addrspace(1) noundef align 4 %data) #2 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !17 !kernel_arg_buffer_location !18 !arg_type_null_val !19 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  %pipe2.addr = alloca ptr addrspace(1), align 8
  %data.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe2, ptr %pipe2.addr, align 8, !tbaa !5
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %call = call i32 @readPipeHelper(ptr addrspace(1) %0, ptr addrspace(1) %1) #5
  %2 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %2, i64 0
  store i32 %call, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %3 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %4 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call1 = call i32 @readPipeHelper(ptr addrspace(1) %3, ptr addrspace(1) %4) #5
  %5 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %5, i64 1
  store i32 %call1, ptr addrspace(1) %arrayidx2, align 4, !tbaa !8
  ret void
}

; CHECK-LABEL: @readFromFileViaPipe3
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_2_readPipeHelper
; CHECK: call void @__io_pipe_0_3_1_0_dummy
; CHECK: call void @__io_pipe_0_3_dummy
; Function Attrs: convergent norecurse nounwind
define dso_local void @readFromFileViaPipe3(ptr addrspace(1) %pipe0, ptr addrspace(1) %pipe1, ptr addrspace(1) %pipe2, ptr addrspace(1) %pipe3, ptr addrspace(1) noundef align 4 %data) #2 !kernel_arg_addr_space !22 !kernel_arg_access_qual !23 !kernel_arg_type !24 !kernel_arg_base_type !24 !kernel_arg_type_qual !25 !kernel_arg_host_accessible !26 !kernel_arg_pipe_depth !27 !kernel_arg_pipe_io !28 !kernel_arg_buffer_location !29 !arg_type_null_val !30 {
entry:
  %pipe0.addr = alloca ptr addrspace(1), align 8
  %pipe1.addr = alloca ptr addrspace(1), align 8
  %pipe2.addr = alloca ptr addrspace(1), align 8
  %pipe3.addr = alloca ptr addrspace(1), align 8
  %data.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %pipe0, ptr %pipe0.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe1, ptr %pipe1.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe2, ptr %pipe2.addr, align 8, !tbaa !5
  store ptr addrspace(1) %pipe3, ptr %pipe3.addr, align 8, !tbaa !5
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !20
  %0 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %call = call i32 @readPipeHelper(ptr addrspace(1) %0, ptr addrspace(1) %1) #5
  %2 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %2, i64 0
  store i32 %call, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %3 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  %4 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call1 = call i32 @readPipeHelper(ptr addrspace(1) %3, ptr addrspace(1) %4) #5
  %5 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %5, i64 1
  store i32 %call1, ptr addrspace(1) %arrayidx2, align 4, !tbaa !8
  %6 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  %7 = load ptr addrspace(1), ptr %pipe1.addr, align 8, !tbaa !5
  %call3 = call i32 @readPipeHelper(ptr addrspace(1) %6, ptr addrspace(1) %7) #5
  %8 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !20
  %arrayidx4 = getelementptr inbounds i32, ptr addrspace(1) %8, i64 2
  store i32 %call3, ptr addrspace(1) %arrayidx4, align 4, !tbaa !8
  %9 = load ptr addrspace(1), ptr %pipe3.addr, align 8, !tbaa !5
  %10 = load ptr addrspace(1), ptr %pipe0.addr, align 8, !tbaa !5
  call void @dummy(ptr addrspace(1) %9, ptr addrspace(1) %10) #5
  %11 = load ptr addrspace(1), ptr %pipe3.addr, align 8, !tbaa !5
  %12 = load ptr addrspace(1), ptr %pipe2.addr, align 8, !tbaa !5
  call void @dummy(ptr addrspace(1) %11, ptr addrspace(1) %12) #5
  ret void
}

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #3

; Function Attrs: nounwind memory(none)
declare i32 @__write_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture readonly, i32, i32) #3

; CHECK: define dso_local i32 @__io_pipe_0_0_readPipeHelper
; CHECK: define dso_local i32 @__io_pipe_1_1_readPipeHelper
; CHECK: define dso_local i32 @__io_pipe_1_0_foo
; CHECK: define dso_local i32 @__io_pipe_0_0_foo
; CHECK: define dso_local i32 @__io_pipe_0_0_bar
; CHECK: define dso_local i32 @__io_pipe_1_0_bar
; CHECK: define dso_local i32 @__io_pipe_0_1_foo
; CHECK: define dso_local i32 @__io_pipe_1_1_foo
; CHECK: define dso_local i32 @__io_pipe_1_1_bar
; CHECK: define dso_local i32 @__io_pipe_0_1_bar
; CHECK: define dso_local void @__io_pipe_0_3_1_0_dummy
; CHECK: define dso_local i32 @__io_pipe_0_0_1_2_readPipeHelper
; CHECK: define dso_local i32 @__io_pipe_1_2_readPipeHelper
; CHECK: define dso_local void @__io_pipe_0_3_dummy
; CHECK: define dso_local i32 @__io_pipe_0_2_foo
; CHECK: define dso_local i32 @__io_pipe_1_2_foo
; CHECK: define dso_local i32 @__io_pipe_1_2_bar
; CHECK: define dso_local i32 @__io_pipe_0_2_bar
; CHECK: define dso_local i32 @__io_pipe_0_2_1_0_foo
; CHECK: define dso_local i32 @__io_pipe_0_0_1_2_foo
; CHECK: define dso_local i32 @__io_pipe_0_0_1_2_bar
; CHECK: define dso_local i32 @__io_pipe_0_2_1_0_bar

; CHECK: declare i32 @__read_pipe_2_io_fpga

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { convergent noinline norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #3 = { nounwind memory(none) }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{ptr @readFromFileViaPipe1, ptr @readFromFileViaPipe2, ptr @readFromFileViaPipe3}
!4 = !{target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
!10 = !{target("spirv.Pipe", 1) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer}
!11 = !{i32 1, i32 1, i32 1, i32 1}
!12 = !{!"read_only", !"read_only", !"read_only", !"none"}
!13 = !{!"int", !"int", !"int", !"int*"}
!14 = !{!"pipe", !"pipe", !"pipe", !""}
!15 = !{i1 false, i1 false, i1 false, i1 false}
!16 = !{i32 0, i32 0, i32 0, i32 0}
!17 = !{!"pipe-io-0", !"pipe-io-1", !"", !""}
!18 = !{!"", !"", !"", !""}
!19 = !{target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer, ptr addrspace(1) null}
!20 = !{!21, !21, i64 0}
!21 = !{!"any pointer", !6, i64 0}
!22 = !{i32 1, i32 1, i32 1, i32 1, i32 1}
!23 = !{!"read_only", !"read_only", !"read_only", !"write_only", !"none"}
!24 = !{!"int", !"int", !"int", !"int", !"int*"}
!25 = !{!"pipe", !"pipe", !"pipe", !"pipe", !""}
!26 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!27 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!28 = !{!"pipe-io-0", !"pipe-io-2", !"", !"pipe-io-3", !""}
!29 = !{!"", !"", !"", !"", !""}
!30 = !{target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer, target("spirv.Pipe", 1) zeroinitializer, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING: Missing line
