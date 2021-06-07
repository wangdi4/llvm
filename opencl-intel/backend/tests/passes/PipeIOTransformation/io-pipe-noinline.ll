; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation %s -S | FileCheck %s
;
; This test checks that io pipe is replaced with builtin correctly when
; user-defined functions are not inlined.
; IR is dumped from VOLCANO_LLVM_OPTIONS=-print-before=pipe-io-transformation
; from following cl source:
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
; int dummy(write_only pipe int pipe0, read_only pipe int pipe1) {
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
;     __global int *data) {
;   data[0] = readPipeHelper(pipe0, pipe2);
;   data[1] = readPipeHelper(pipe2, pipe1);
;   data[2] = readPipeHelper(pipe0, pipe1);
;   dummy(pipe3, pipe0);
;   dummy(pipe3, pipe2);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.pipe_ro_t.5 = type opaque
%opencl.pipe_wo_t.6 = type opaque

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @bar(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #0 {
; CHECK-LABEL: @bar
; CHECK: call i32 @__read_pipe_2_fpga
; CHECK: call i32 @__read_pipe_2_fpga
entry:
  %result = alloca i32, align 4
  %0 = bitcast i32* %result to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #4
  br label %while.cond

while.cond:                                       ; preds = %while.cond, %entry
  %1 = bitcast i32* %result to i8*
  %2 = addrspacecast i8* %1 to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, i8 addrspace(4)* %2, i32 4, i32 4)
  %tobool = icmp eq i32 %3, 0
  br i1 %tobool, label %while.cond, label %while.cond1

while.cond1:                                      ; preds = %while.cond1, %while.cond
  %4 = bitcast i32* %result to i8*
  %5 = addrspacecast i8* %4 to i8 addrspace(4)*
  %6 = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t.5 addrspace(1)* %pipe1, i8 addrspace(4)* %5, i32 4, i32 4)
  %tobool2 = icmp eq i32 %6, 0
  br i1 %tobool2, label %while.cond1, label %while.end5

while.end5:                                       ; preds = %while.cond1
  %7 = load i32, i32* %result, align 4, !tbaa !5
  %8 = bitcast i32* %result to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %8) #4
  ret i32 %7
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @foo(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #0 {
; CHECK-LABEL: @foo
; CHECK: call i32 @bar
entry:
  %call = call i32 @bar(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  ret i32 %call
}

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @dummy(%opencl.pipe_wo_t.6 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #0 {
; CHECK-LABEL: @dummy
; CHECK: call i32 @__read_pipe_2_fpga
; CHECK: call i32 @__write_pipe_2_fpga
entry:
  %result = alloca i32, align 4
  %0 = bitcast i32* %result to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #4
  %1 = bitcast i32* %result to i8*
  %2 = addrspacecast i8* %1 to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t.5 addrspace(1)* %pipe1, i8 addrspace(4)* %2, i32 4, i32 4)
  %4 = bitcast i32* %result to i8*
  %5 = addrspacecast i8* %4 to i8 addrspace(4)*
  %6 = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t.6 addrspace(1)* %pipe0, i8 addrspace(4)* %5, i32 4, i32 4)
  %7 = bitcast i32* %result to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %7) #4
  ret i32 undef
}

; Function Attrs: convergent noinline norecurse nounwind
define internal i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #0 {
; CHECK-LABEL: @readPipeHelper
; CHECK: call i32 @foo
; CHECK: call i32 @foo
entry:
  %call = call i32 @foo(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  %call1 = call i32 @foo(%opencl.pipe_ro_t.5 addrspace(1)* %pipe1, %opencl.pipe_ro_t.5 addrspace(1)* %pipe0) #5
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

; Function Attrs: convergent norecurse nounwind
define void @readFromFileViaPipe1(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2, i32 addrspace(1)* %data) #2 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_name !13 !kernel_arg_host_accessible !14 !kernel_arg_pipe_depth !15 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !17 {
; CHECK-LABEL: @readFromFileViaPipe1
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_1_readPipeHelper
entry:
  %call = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2) #5
  store i32 %call, i32 addrspace(1)* %data, align 4, !tbaa !5
  %call1 = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe2, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 1
  store i32 %call1, i32 addrspace(1)* %ptridx2, align 4, !tbaa !5
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @readFromFileViaPipe2(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2, i32 addrspace(1)* %data) #2 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_name !13 !kernel_arg_host_accessible !14 !kernel_arg_pipe_depth !15 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !17 {
; CHECK-LABEL: @readFromFileViaPipe2
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_1_readPipeHelper
entry:
  %call = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2) #5
  store i32 %call, i32 addrspace(1)* %data, align 4, !tbaa !5
  %call1 = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe2, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 1
  store i32 %call1, i32 addrspace(1)* %ptridx2, align 4, !tbaa !5
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @readFromFileViaPipe3(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2, %opencl.pipe_wo_t.6 addrspace(1)* %pipe3, i32 addrspace(1)* %data) #2 !kernel_arg_addr_space !18 !kernel_arg_access_qual !19 !kernel_arg_type !20 !kernel_arg_base_type !20 !kernel_arg_type_qual !21 !kernel_arg_name !22 !kernel_arg_host_accessible !23 !kernel_arg_pipe_depth !24 !kernel_arg_pipe_io !25 !kernel_arg_buffer_location !26 {
; CHECK-LABEL: @readFromFileViaPipe3
; CHECK: call i32 @__io_pipe_0_0_readPipeHelper
; CHECK: call i32 @__io_pipe_1_2_readPipeHelper
; CHECK: call i32 @__io_pipe_0_3_1_0_dummy
; CHECK: call i32 @__io_pipe_0_3_dummy
entry:
  %call = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2) #5
  store i32 %call, i32 addrspace(1)* %data, align 4, !tbaa !5
  %call1 = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe2, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 1
  store i32 %call1, i32 addrspace(1)* %ptridx2, align 4, !tbaa !5
  %call3 = call i32 @readPipeHelper(%opencl.pipe_ro_t.5 addrspace(1)* %pipe0, %opencl.pipe_ro_t.5 addrspace(1)* %pipe1) #5
  %ptridx4 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 2
  store i32 %call3, i32 addrspace(1)* %ptridx4, align 4, !tbaa !5
  %call5 = call i32 @dummy(%opencl.pipe_wo_t.6 addrspace(1)* %pipe3, %opencl.pipe_ro_t.5 addrspace(1)* %pipe0) #5
  %call6 = call i32 @dummy(%opencl.pipe_wo_t.6 addrspace(1)* %pipe3, %opencl.pipe_ro_t.5 addrspace(1)* %pipe2) #5
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t.5 addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #3

; Function Attrs: convergent norecurse nounwind
declare i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t.6 addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #3

; CHECK: define internal i32 @__io_pipe_0_0_readPipeHelper
; CHECK: define internal i32 @__io_pipe_1_1_readPipeHelper
; CHECK: define internal i32 @__io_pipe_1_0_foo
; CHECK: define internal i32 @__io_pipe_0_0_foo
; CHECK: define internal i32 @__io_pipe_0_0_bar
; CHECK: define internal i32 @__io_pipe_1_0_bar
; CHECK: define internal i32 @__io_pipe_0_1_foo
; CHECK: define internal i32 @__io_pipe_1_1_foo
; CHECK: define internal i32 @__io_pipe_1_1_bar
; CHECK: define internal i32 @__io_pipe_0_1_bar
; CHECK: define internal i32 @__io_pipe_0_3_1_0_dummy
; CHECK: define internal i32 @__io_pipe_0_0_1_2_readPipeHelper
; CHECK: define internal i32 @__io_pipe_1_2_readPipeHelper
; CHECK: define internal i32 @__io_pipe_0_3_dummy
; CHECK: define internal i32 @__io_pipe_0_2_foo
; CHECK: define internal i32 @__io_pipe_1_2_foo
; CHECK: define internal i32 @__io_pipe_1_2_bar
; CHECK: define internal i32 @__io_pipe_0_2_bar
; CHECK: define internal i32 @__io_pipe_0_2_1_0_foo
; CHECK: define internal i32 @__io_pipe_0_0_1_2_foo
; CHECK: define internal i32 @__io_pipe_0_0_1_2_bar
; CHECK: define internal i32 @__io_pipe_0_2_1_0_bar

; CHECK: declare i32 @__read_pipe_2_io_fpga


attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void (%opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, i32 addrspace(1)*)* @readFromFileViaPipe1, void (%opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, i32 addrspace(1)*)* @readFromFileViaPipe2, void (%opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_ro_t.5 addrspace(1)*, %opencl.pipe_wo_t.6 addrspace(1)*, i32 addrspace(1)*)* @readFromFileViaPipe3}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{i32 1, i32 1, i32 1, i32 1}
!10 = !{!"read_only", !"read_only", !"read_only", !"none"}
!11 = !{!"int", !"int", !"int", !"int*"}
!12 = !{!"pipe", !"pipe", !"pipe", !""}
!13 = !{!"pipe0", !"pipe1", !"pipe2", !"data"}
!14 = !{i1 false, i1 false, i1 false, i1 false}
!15 = !{i32 0, i32 0, i32 0, i32 0}
!16 = !{!"pipe-io-0", !"pipe-io-1", !"", !""}
!17 = !{!"", !"", !"", !""}
!18 = !{i32 1, i32 1, i32 1, i32 1, i32 1}
!19 = !{!"read_only", !"read_only", !"read_only", !"write_only", !"none"}
!20 = !{!"int", !"int", !"int", !"int", !"int*"}
!21 = !{!"pipe", !"pipe", !"pipe", !"pipe", !""}
!22 = !{!"pipe0", !"pipe1", !"pipe2", !"pipe3", !"data"}
!23 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!24 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!25 = !{!"pipe-io-0", !"pipe-io-2", !"", !"pipe-io-3", !""}
!26 = !{!"", !"", !"", !"", !""}

; DEBUGIFY-NOT: WARNING: Missing line
