; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; struct st {
;   int i;
; };
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct st star_arr[5][4][3];
; channel long lar_arr[6][5][4][3];
;
; __kernel void foo() {
;   char char_index = 4;
;   short short_index = 3;
;   int int_index = 2;
;   long long_index = 1;
;
;   int i = 42;
;   float f = 3.14;
;   struct st s = { 100 };
;   long l = 500;
;
;   write_channel_intel(bar_arr[char_index], i);
;   write_channel_intel(far_arr[char_index][short_index], f);
;   write_channel_intel(star_arr[char_index][short_index][int_index], s);
;   write_channel_intel(lar_arr[char_index][short_index][int_index][long_index], l);
;
;   i = read_channel_intel(bar_arr[char_index]);
;   f = read_channel_intel(far_arr[char_index][short_index]);
;   s = read_channel_intel(star_arr[char_index][short_index][int_index]);
;   l = read_channel_intel(lar_arr[char_index][short_index][int_index][long_index]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.st = type { i32 }
%opencl.channel_t = type opaque

@foo.s = private unnamed_addr addrspace(2) constant %struct.st { i32 100 }, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@lar_arr = common addrspace(1) global [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] zeroinitializer, align 8, !packet_size !3, !packet_align !3

; CHECK: %[[CINDEX0:.*]] = load {{.*}} %char_index
; CHECK: %[[BAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX0]]
; CHECK: %[[GEP_BAR_PIPE_ARR_W:.*]] = getelementptr {{.*}} @bar_arr.pipe, i64 0, i64 %[[BAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[LOAD_BAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_BAR_PIPE_ARR_W]]
; CHECK: %[[CAST_BAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX1:.*]] = load {{.*}} %char_index
; CHECK: %[[FAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX1]]
; CHECK: %[[GEP_FAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @far_arr.pipe, i64 0, i64 %[[FAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX0:.*]] = load {{.*}} %short_index
; CHECK: %[[FAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX0]]
; CHECK: %[[GEP_FAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_FAR_PIPE_ARR_W0]], i64 0, i64 %[[FAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[LOAD_FAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_FAR_PIPE_ARR_W1]]
; CHECK: %[[CAST_FAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX2:.*]] = load {{.*}} %char_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX2]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @star_arr.pipe, i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX1:.*]] = load {{.*}} %short_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX1]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_W0]], i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[IINDEX0:.*]] = load {{.*}} %int_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX2:.*]] = sext {{.*}} %[[IINDEX0]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W2:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_W1]], i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX2]]
; CHECK: %[[LOAD_STAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_STAR_PIPE_ARR_W2]]
; CHECK: %[[CAST_STAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX3:.*]] = load {{.*}} %char_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX3]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @lar_arr.pipe, i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX2:.*]] = load {{.*}} %short_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX2]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W0]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[IINDEX1:.*]] = load {{.*}} %int_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX2:.*]] = sext {{.*}} %[[IINDEX1]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W2:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W1]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX2]]
; CHECK: %[[LAR_PIPE_ARR_W_INDEX3:.*]] = load {{.*}} %long_index
; CHECK: %[[GEP_LAR_PIPE_ARR_W3:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W2]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX3]]
; CHECK: %[[LOAD_LAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_LAR_PIPE_ARR_W3]]
; CHECK: %[[CAST_LAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_LAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_LAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX4:.*]] = load {{.*}} %char_index
; CHECK: %[[BAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX4]]
; CHECK: %[[GEP_BAR_PIPE_ARR_R:.*]] = getelementptr {{.*}} @bar_arr.pipe, i64 0, i64 %[[BAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[LOAD_BAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_BAR_PIPE_ARR_R]]
; CHECK: %[[CAST_BAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX5:.*]] = load {{.*}} %char_index
; CHECK: %[[FAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX5]]
; CHECK: %[[GEP_FAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @far_arr.pipe, i64 0, i64 %[[FAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX3:.*]] = load {{.*}} %short_index
; CHECK: %[[FAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX3]]
; CHECK: %[[GEP_FAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_FAR_PIPE_ARR_R0]], i64 0, i64 %[[FAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[LOAD_FAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_FAR_PIPE_ARR_R1]]
; CHECK: %[[CAST_FAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX6:.*]] = load {{.*}} %char_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX6]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @star_arr.pipe, i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX4:.*]] = load {{.*}} %short_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX4]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_R0]], i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[IINDEX2:.*]] = load {{.*}} %int_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX2:.*]] = sext {{.*}} %[[IINDEX2]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R2:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_R1]], i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX2]]
; CHECK: %[[LOAD_STAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_STAR_PIPE_ARR_R2]]
; CHECK: %[[CAST_STAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_STAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX7:.*]] = load {{.*}} %char_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX7]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @lar_arr.pipe, i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX5:.*]] = load {{.*}} %short_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX5]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R0]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[IINDEX3:.*]] = load {{.*}} %int_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX2:.*]] = sext {{.*}} %[[IINDEX3]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R2:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R1]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX2]]
; CHECK: %[[LAR_PIPE_ARR_R_INDEX3:.*]] = load {{.*}} %long_index
; CHECK: %[[GEP_LAR_PIPE_ARR_R3:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R2]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX3]]
; CHECK: %[[LOAD_LAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_LAR_PIPE_ARR_R3]]
; CHECK: %[[CAST_LAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_LAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_LAR_PIPE_ARR_R]]

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !12 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !12 {
entry:
  %char_index = alloca i8, align 1
  %short_index = alloca i16, align 2
  %int_index = alloca i32, align 4
  %long_index = alloca i64, align 8
  %i = alloca i32, align 4
  %f = alloca float, align 4
  %s = alloca %struct.st, align 4
  %l = alloca i64, align 8
  %tmp = alloca %struct.st, align 4
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %char_index) #3
  store i8 4, i8* %char_index, align 1, !tbaa !14
  %0 = bitcast i16* %short_index to i8*
  call void @llvm.lifetime.start.p0i8(i64 2, i8* %0) #3
  store i16 3, i16* %short_index, align 2, !tbaa !17
  %1 = bitcast i32* %int_index to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store i32 2, i32* %int_index, align 4, !tbaa !19
  %2 = bitcast i64* %long_index to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #3
  store i64 1, i64* %long_index, align 8, !tbaa !21
  %3 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #3
  store i32 42, i32* %i, align 4, !tbaa !19
  %4 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  store float 0x40091EB860000000, float* %f, align 4, !tbaa !23
  %5 = bitcast %struct.st* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  %6 = bitcast %struct.st* %s to i8*
  call void @llvm.memcpy.p0i8.p2i8.i64(i8* %6, i8 addrspace(2)* bitcast (%struct.st addrspace(2)* @foo.s to i8 addrspace(2)*), i64 4, i32 4, i1 false)
  %7 = bitcast i64* %l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %7) #3
  store i64 500, i64* %l, align 8, !tbaa !21
  %8 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom = sext i8 %8 to i64
  %arrayidx = getelementptr inbounds [5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 %idxprom
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx, align 4, !tbaa !14
  %10 = load i32, i32* %i, align 4, !tbaa !19
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %9, i32 %10)
  %11 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom1 = sext i8 %11 to i64
  %arrayidx2 = getelementptr inbounds [5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 %idxprom1
  %12 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom3 = sext i16 %12 to i64
  %arrayidx4 = getelementptr inbounds [4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx2, i64 0, i64 %idxprom3
  %13 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx4, align 4, !tbaa !14
  %14 = load float, float* %f, align 4, !tbaa !23
  call void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %13, float %14)
  %15 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom5 = sext i8 %15 to i64
  %arrayidx6 = getelementptr inbounds [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 %idxprom5
  %16 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom7 = sext i16 %16 to i64
  %arrayidx8 = getelementptr inbounds [4 x [3 x %opencl.channel_t addrspace(1)*]], [4 x [3 x %opencl.channel_t addrspace(1)*]] addrspace(1)* %arrayidx6, i64 0, i64 %idxprom7
  %17 = load i32, i32* %int_index, align 4, !tbaa !19
  %idxprom9 = sext i32 %17 to i64
  %arrayidx10 = getelementptr inbounds [3 x %opencl.channel_t addrspace(1)*], [3 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx8, i64 0, i64 %idxprom9
  %18 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx10, align 4, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channel2stS_(%opencl.channel_t addrspace(1)* %18, %struct.st* byval align 4 %s)
  %19 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom11 = sext i8 %19 to i64
  %arrayidx12 = getelementptr inbounds [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]], [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] addrspace(1)* @lar_arr, i64 0, i64 %idxprom11
  %20 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom13 = sext i16 %20 to i64
  %arrayidx14 = getelementptr inbounds [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* %arrayidx12, i64 0, i64 %idxprom13
  %21 = load i32, i32* %int_index, align 4, !tbaa !19
  %idxprom15 = sext i32 %21 to i64
  %arrayidx16 = getelementptr inbounds [4 x [3 x %opencl.channel_t addrspace(1)*]], [4 x [3 x %opencl.channel_t addrspace(1)*]] addrspace(1)* %arrayidx14, i64 0, i64 %idxprom15
  %22 = load i64, i64* %long_index, align 8, !tbaa !21
  %arrayidx17 = getelementptr inbounds [3 x %opencl.channel_t addrspace(1)*], [3 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx16, i64 0, i64 %22
  %23 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx17, align 8, !tbaa !14
  %24 = load i64, i64* %l, align 8, !tbaa !21
  call void @_Z19write_channel_intel11ocl_channelll(%opencl.channel_t addrspace(1)* %23, i64 %24)
  %25 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom18 = sext i8 %25 to i64
  %arrayidx19 = getelementptr inbounds [5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 %idxprom18
  %26 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx19, align 4, !tbaa !14
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %26)
  store i32 %call, i32* %i, align 4, !tbaa !19
  %27 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom20 = sext i8 %27 to i64
  %arrayidx21 = getelementptr inbounds [5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 %idxprom20
  %28 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom22 = sext i16 %28 to i64
  %arrayidx23 = getelementptr inbounds [4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx21, i64 0, i64 %idxprom22
  %29 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx23, align 4, !tbaa !14
  %call24 = call float @_Z18read_channel_intel11ocl_channelf(%opencl.channel_t addrspace(1)* %29)
  store float %call24, float* %f, align 4, !tbaa !23
  %30 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom25 = sext i8 %30 to i64
  %arrayidx26 = getelementptr inbounds [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 %idxprom25
  %31 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom27 = sext i16 %31 to i64
  %arrayidx28 = getelementptr inbounds [4 x [3 x %opencl.channel_t addrspace(1)*]], [4 x [3 x %opencl.channel_t addrspace(1)*]] addrspace(1)* %arrayidx26, i64 0, i64 %idxprom27
  %32 = load i32, i32* %int_index, align 4, !tbaa !19
  %idxprom29 = sext i32 %32 to i64
  %arrayidx30 = getelementptr inbounds [3 x %opencl.channel_t addrspace(1)*], [3 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx28, i64 0, i64 %idxprom29
  %33 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx30, align 4, !tbaa !14
  call void @_Z18read_channel_intel11ocl_channel2st(%struct.st* sret %tmp, %opencl.channel_t addrspace(1)* %33)
  %34 = bitcast %struct.st* %s to i8*
  %35 = bitcast %struct.st* %tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %34, i8* %35, i64 4, i32 4, i1 false), !tbaa.struct !25
  %36 = load i8, i8* %char_index, align 1, !tbaa !14
  %idxprom31 = sext i8 %36 to i64
  %arrayidx32 = getelementptr inbounds [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]], [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] addrspace(1)* @lar_arr, i64 0, i64 %idxprom31
  %37 = load i16, i16* %short_index, align 2, !tbaa !17
  %idxprom33 = sext i16 %37 to i64
  %arrayidx34 = getelementptr inbounds [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* %arrayidx32, i64 0, i64 %idxprom33
  %38 = load i32, i32* %int_index, align 4, !tbaa !19
  %idxprom35 = sext i32 %38 to i64
  %arrayidx36 = getelementptr inbounds [4 x [3 x %opencl.channel_t addrspace(1)*]], [4 x [3 x %opencl.channel_t addrspace(1)*]] addrspace(1)* %arrayidx34, i64 0, i64 %idxprom35
  %39 = load i64, i64* %long_index, align 8, !tbaa !21
  %arrayidx37 = getelementptr inbounds [3 x %opencl.channel_t addrspace(1)*], [3 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx36, i64 0, i64 %39
  %40 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx37, align 8, !tbaa !14
  %call38 = call i64 @_Z18read_channel_intel11ocl_channell(%opencl.channel_t addrspace(1)* %40)
  store i64 %call38, i64* %l, align 8, !tbaa !21
  %41 = bitcast i64* %l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %41) #3
  %42 = bitcast %struct.st* %s to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #3
  %43 = bitcast float* %f to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %43) #3
  %44 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %44) #3
  %45 = bitcast i64* %long_index to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %45) #3
  %46 = bitcast i32* %int_index to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %46) #3
  %47 = bitcast i16* %short_index to i8*
  call void @llvm.lifetime.end.p0i8(i64 2, i8* %47) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %char_index) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p2i8.i64(i8* nocapture writeonly, i8 addrspace(2)* nocapture readonly, i64, i32, i1) #1

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare void @_Z19write_channel_intel11ocl_channel2stS_(%opencl.channel_t addrspace(1)*, %struct.st* byval align 4) #2

declare void @_Z19write_channel_intel11ocl_channelll(%opencl.channel_t addrspace(1)*, i64) #2

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare float @_Z18read_channel_intel11ocl_channelf(%opencl.channel_t addrspace(1)*) #2

declare void @_Z18read_channel_intel11ocl_channel2st(%struct.st* sret, %opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

declare i64 @_Z18read_channel_intel11ocl_channell(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!10}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!11}
!opencl.spir.version = !{!11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}
!llvm.ident = !{!13}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 3}
!3 = !{i32 8}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{i32 2, i32 0}
!12 = !{}
!13 = !{!"clang version 5.0.0 "}
!14 = !{!15, !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"short", !15, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !15, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"long", !15, i64 0}
!23 = !{!24, !24, i64 0}
!24 = !{!"float", !15, i64 0}
!25 = !{i64 0, i64 4, !19}
