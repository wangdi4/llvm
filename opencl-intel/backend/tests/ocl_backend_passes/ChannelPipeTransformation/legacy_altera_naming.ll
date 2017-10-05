; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
;
; #define __IHC_USE_DEPRECATED_NAMES
;
; channel int ich;
; channel long lch;
;
; __kernel void k1() {
;   int i = read_channel_altera(ich);
;
;   bool ok = false;
;   long l = read_channel_nb_altera(lch, &ok);
; }
;
; __kernel void k() {
;   int i = 42;
;   write_channel_altera(ich, i);
;
;   long l = 43;
;   bool ok = write_channel_nb_altera(lch, l);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ich = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@lch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8

; CHECK-NOT: call {{.*}} read_channel
; CHECK-NOT: call {{.*}} write_channel

; Function Attrs: nounwind
define spir_kernel void @k1() #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 {
entry:
  %i = alloca i32, align 4
  %ok = alloca i8, align 1
  %l = alloca i64, align 8
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ich, align 4, !tbaa !10
  %call = call i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)* %1)
  store i32 %call, i32* %i, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %ok) #3
  store i8 0, i8* %ok, align 1, !tbaa !15
  %2 = bitcast i64* %l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @lch, align 8, !tbaa !10
  %4 = addrspacecast i8* %ok to i8 addrspace(4)*
  %call1 = call i64 @_Z22read_channel_nb_altera11ocl_channellPU3AS4b(%opencl.channel_t addrspace(1)* %3, i8 addrspace(4)* %4)
  store i64 %call1, i64* %l, align 8, !tbaa !17
  %5 = bitcast i64* %l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %5) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %ok) #3
  %6 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare i64 @_Z22read_channel_nb_altera11ocl_channellPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
define spir_kernel void @k() #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 {
entry:
  %i = alloca i32, align 4
  %l = alloca i64, align 8
  %ok = alloca i8, align 1
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !13
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ich, align 4, !tbaa !10
  %2 = load i32, i32* %i, align 4, !tbaa !13
  call void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %1, i32 %2)
  %3 = bitcast i64* %l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #3
  store i64 43, i64* %l, align 8, !tbaa !17
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %ok) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @lch, align 8, !tbaa !10
  %5 = load i64, i64* %l, align 8, !tbaa !17
  %call = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelll(%opencl.channel_t addrspace(1)* %4, i64 %5)
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %ok, align 1, !tbaa !15
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %ok) #3
  %6 = bitcast i64* %l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %6) #3
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #3
  ret void
}

declare void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelll(%opencl.channel_t addrspace(1)*, i64) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !3}
!llvm.module.flags = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!7}
!opencl.spir.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ich, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @lch, !4, !5}
!4 = !{!"packet_size", i32 8}
!5 = !{!"packet_align", i32 8}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 5.0.0 "}
!10 = !{!11, !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"bool", !11, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"long", !11, i64 0}
