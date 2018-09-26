; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; __kernel void k1(read_only pipe int p1 __attribute__((io("test1"))),
;                  write_only pipe int p2 __attribute__((io("test2"))),
;                  int i1, global float * f1) {
;   int data;
;   int i = read_pipe(p1, &data);
;   if (i > 0)
;     write_pipe(p2, &data);
; }
;
; channel int ichIn __attribute__((io("test_ch_in")));
; channel int ichOut __attribute__((io("test_ch_out")));
;
; __kernel void in_kernel() {
;   long data;
;   data = read_channel_intel(ichIn);
;   if (data > 0)
;     write_channel_intel(ichOut, data);
; }
; ----------------------------------------------------
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

; CHECK: @llvm.global_dtors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_dtor, i8* null }]
; CHECK: @test_ch_in.str = private unnamed_addr constant [11 x i8] c"test_ch_in\00", align 1
; CHECK: @test1.str = private unnamed_addr constant [6 x i8] c"test1\00", align 1
; CHECK: @test2.str = private unnamed_addr constant [6 x i8] c"test2\00", align 1
;
; CHECK: define void @k1
; CHECK: %[[PIPERO:[0-9]+]] = load %opencl.pipe_ro_t {{.*}} %p1.addr
; CHECK: %[[TEST1STR:[0-9]+]] = addrspacecast [6 x i8]* @test1.str to i8 addrspace(4)*
; CHECK: call i32 @__read_pipe_2_io_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPERO]], i8 addrspace(4)* %{{[0-9]+}}, i8 addrspace(4)* %[[TEST1STR]], i32 4, i32 4)
; CHECK: %[[PIPEWO:[0-9]+]] = load %opencl.pipe_wo_t {{.*}} %p2.addr
; CHECK: %[[TEST2STR:[0-9]+]] = addrspacecast [6 x i8]* @test2.str to i8 addrspace(4)*
; CHECK: call i32 @__write_pipe_2_io_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPEWO]], i8 addrspace(4)* %{{[0-9]+}}, i8 addrspace(4)* %[[TEST2STR]], i32 4, i32 4)
;
; CHECK: define void @in_kernel
; CHECK: %[[PIPE1RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @ichIn.pipe
; CHECK: %[[PIPE1RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE1RW]] to %opencl.pipe_ro_t
; CHECK: %[[TESTCHINSTR:[0-9]+]] = addrspacecast [11 x i8]* @test_ch_in.str to i8 addrspace(4)*
; CHECK: call i32 @__read_pipe_2_bl_io_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPE1RO]], i8 addrspace(4)* %{{[0-9]+}}, i8 addrspace(4)* %[[TESTCHINSTR]], i32 4, i32 4)
; CHECK: %[[PIPE2RW:[0-9]+]] = load %opencl.pipe_rw_t {{.*}} @ichOut.pipe
; CHECK: %[[PIPE2WO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPE2RW]] to %opencl.pipe_wo_t
; CHECK: %[[TESTCHOUTSTR:[0-9]+]] = addrspacecast [12 x i8]* @test_ch_out.str to i8 addrspace(4)*
; CHECK: call i32 @__write_pipe_2_bl_io_fpga(%opencl.pipe_wo_t addrspace(1)* %[[PIPE2WO]], i8 addrspace(4)* %{{[0-9]+}}, i8 addrspace(4)* %[[TESTCHOUTSTR]], i32 4, i32 4)
;
; CHECK: define void @__pipe_global_dtor()
; CHECK: call void @__pipe_release_fpga(%struct.__pipe_t addrspace(1)* bitcast (%opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichIn.pipe to %struct.__pipe_t addrspace(1)*))
; CHECK: call void @__pipe_release_fpga(%struct.__pipe_t addrspace(1)* bitcast (%opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichOut.pipe to %struct.__pipe_t addrspace(1)*))
; CHECK: declare void @__pipe_release_fpga(%struct.__pipe_t addrspace(1)* nocapture readonly)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque
%opencl.pipe_wo_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ichIn = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !1
@ichOut = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !2
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ichIn.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !1
@ichIn.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ichOut.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0, !io !2
@ichOut.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @k1(%opencl.pipe_ro_t addrspace(1)* %p1, %opencl.pipe_wo_t addrspace(1)* %p2, i32 %i1, float addrspace(1)* %f1) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !15 {
entry:
  %p1.addr = alloca %opencl.pipe_ro_t addrspace(1)*, align 8
  %p2.addr = alloca %opencl.pipe_wo_t addrspace(1)*, align 8
  %i1.addr = alloca i32, align 4
  %f1.addr = alloca float addrspace(1)*, align 8
  %data = alloca i32, align 4
  %i = alloca i32, align 4
  store %opencl.pipe_ro_t addrspace(1)* %p1, %opencl.pipe_ro_t addrspace(1)** %p1.addr, align 8, !tbaa !16
  store %opencl.pipe_wo_t addrspace(1)* %p2, %opencl.pipe_wo_t addrspace(1)** %p2.addr, align 8, !tbaa !16
  store i32 %i1, i32* %i1.addr, align 4, !tbaa !19
  store float addrspace(1)* %f1, float addrspace(1)** %f1.addr, align 8, !tbaa !21
  %0 = bitcast i32* %data to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  %2 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p1.addr, align 8, !tbaa !16
  %3 = bitcast i32* %data to i8*
  %4 = addrspacecast i8* %3 to i8 addrspace(4)*
  %5 = call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)* %2, i8 addrspace(4)* %4, i32 4, i32 4)
  store i32 %5, i32* %i, align 4, !tbaa !19
  %6 = load i32, i32* %i, align 4, !tbaa !19
  %cmp = icmp sgt i32 %6, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %7 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p2.addr, align 8, !tbaa !16
  %8 = bitcast i32* %data to i8*
  %9 = addrspacecast i8* %8 to i8 addrspace(4)*
  %10 = call i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)* %7, i8 addrspace(4)* %9, i32 4, i32 4)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  %12 = bitcast i32* %data to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @__read_pipe_2_AS0(%opencl.pipe_ro_t addrspace(1)*, i8*, i32, i32)

declare i32 @__write_pipe_2_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define void @in_kernel() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 {
entry:
  %write.src = alloca i32
  %data = alloca i64, align 8
  %0 = bitcast i64* %data to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  %1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichIn.pipe
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ichIn, align 4, !tbaa !16
  %3 = bitcast %opencl.pipe_rw_t addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)*
  %4 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
  %5 = load i32, i32* %write.src
  %conv = sext i32 %5 to i64
  store i64 %conv, i64* %data, align 8, !tbaa !23
  %6 = load i64, i64* %data, align 8, !tbaa !23
  %cmp = icmp sgt i64 %6, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %7 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichOut.pipe
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ichOut, align 4, !tbaa !16
  %9 = load i64, i64* %data, align 8, !tbaa !23
  %conv2 = trunc i64 %9 to i32
  store i32 %conv2, i32* %write.src
  %10 = bitcast %opencl.pipe_rw_t addrspace(1)* %7 to %opencl.pipe_wo_t addrspace(1)*
  %11 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %12 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %10, i8 addrspace(4)* %11, i32 4, i32 4)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %13 = bitcast i64* %data to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %13) #3
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #2

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #2

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ichIn.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ichIn.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichIn.pipe
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ichOut.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ichOut.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ichOut.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #2

declare i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{i32 4}
!1 = !{!"test_ch_in"}
!2 = !{!"test_ch_out"}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 1, i32 2}
!5 = !{}
!6 = !{!"clang version 7.0.0 "}
!7 = !{void (%opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)*, i32, float addrspace(1)*)* @k1, void ()* @in_kernel}
!8 = !{i32 1, i32 1, i32 0, i32 1}
!9 = !{!"read_only", !"write_only", !"none", !"none"}
!10 = !{!"int", !"int", !"int", !"float*"}
!11 = !{!"pipe", !"pipe", !"", !""}
!12 = !{i1 false, i1 false, i1 false, i1 false}
!13 = !{i32 0, i32 0, i32 0, i32 0}
!14 = !{!"test1", !"test2", !"", !""}
!15 = !{!"", !"", !"", !""}
!16 = !{!17, !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !17, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"any pointer", !17, i64 0}
!23 = !{!24, !24, i64 0}
!24 = !{!"long", !17, i64 0}
