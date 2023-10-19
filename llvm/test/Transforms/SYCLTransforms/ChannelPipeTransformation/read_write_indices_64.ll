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

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.st = type { i32 }

@__const.foo.s = private unnamed_addr addrspace(2) constant %struct.st { i32 100 }, align 4
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !2
@star_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@lar_arr = addrspace(1) global [6 x [5 x [4 x [3 x ptr addrspace(1)]]]] zeroinitializer, align 8, !packet_size !3, !packet_align !3

; CHECK: %[[CINDEX0:.*]] = load {{.*}} %char_index
; CHECK: %[[BAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX0]]
; CHECK: %[[GEP_BAR_PIPE_ARR_W:.*]] = getelementptr {{.*}} @bar_arr, i64 0, i64 %[[BAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[LOAD_BAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_BAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[LOAD_BAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX1:.*]] = load {{.*}} %char_index
; CHECK: %[[FAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX1]]
; CHECK: %[[GEP_FAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @far_arr, i64 0, i64 %[[FAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX0:.*]] = load {{.*}} %short_index
; CHECK: %[[FAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX0]]
; CHECK: %[[GEP_FAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_FAR_PIPE_ARR_W0]], i64 0, i64 %[[FAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[LOAD_FAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_FAR_PIPE_ARR_W1]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[LOAD_FAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX2:.*]] = load {{.*}} %char_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX2]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @star_arr, i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX1:.*]] = load {{.*}} %short_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX1]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_W0]], i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[IINDEX0:.*]] = load {{.*}} %int_index
; CHECK: %[[STAR_PIPE_ARR_W_INDEX2:.*]] = sext {{.*}} %[[IINDEX0]]
; CHECK: %[[GEP_STAR_PIPE_ARR_W2:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_W1]], i64 0, i64 %[[STAR_PIPE_ARR_W_INDEX2]]
; CHECK: %[[LOAD_STAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_STAR_PIPE_ARR_W2]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[LOAD_STAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX3:.*]] = load {{.*}} %char_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX0:.*]] = sext {{.*}} %[[CINDEX3]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W0:.*]] = getelementptr {{.*}} @lar_arr, i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX0]]
; CHECK: %[[SINDEX2:.*]] = load {{.*}} %short_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX1:.*]] = sext {{.*}} %[[SINDEX2]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W1:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W0]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX1]]
; CHECK: %[[IINDEX1:.*]] = load {{.*}} %int_index
; CHECK: %[[LAR_PIPE_ARR_W_INDEX2:.*]] = sext {{.*}} %[[IINDEX1]]
; CHECK: %[[GEP_LAR_PIPE_ARR_W2:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W1]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX2]]
; CHECK: %[[LAR_PIPE_ARR_W_INDEX3:.*]] = load {{.*}} %long_index
; CHECK: %[[GEP_LAR_PIPE_ARR_W3:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_W2]], i64 0, i64 %[[LAR_PIPE_ARR_W_INDEX3]]
; CHECK: %[[LOAD_LAR_PIPE_ARR_W:.*]] = load {{.*}} %[[GEP_LAR_PIPE_ARR_W3]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[LOAD_LAR_PIPE_ARR_W]]
;
; CHECK: %[[CINDEX4:.*]] = load {{.*}} %char_index
; CHECK: %[[BAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX4]]
; CHECK: %[[GEP_BAR_PIPE_ARR_R:.*]] = getelementptr {{.*}} @bar_arr, i64 0, i64 %[[BAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[LOAD_BAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_BAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[LOAD_BAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX5:.*]] = load {{.*}} %char_index
; CHECK: %[[FAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX5]]
; CHECK: %[[GEP_FAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @far_arr, i64 0, i64 %[[FAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX3:.*]] = load {{.*}} %short_index
; CHECK: %[[FAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX3]]
; CHECK: %[[GEP_FAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_FAR_PIPE_ARR_R0]], i64 0, i64 %[[FAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[LOAD_FAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_FAR_PIPE_ARR_R1]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[LOAD_FAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX6:.*]] = load {{.*}} %char_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX6]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @star_arr, i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX4:.*]] = load {{.*}} %short_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX4]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_R0]], i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[IINDEX2:.*]] = load {{.*}} %int_index
; CHECK: %[[STAR_PIPE_ARR_R_INDEX2:.*]] = sext {{.*}} %[[IINDEX2]]
; CHECK: %[[GEP_STAR_PIPE_ARR_R2:.*]] = getelementptr {{.*}} %[[GEP_STAR_PIPE_ARR_R1]], i64 0, i64 %[[STAR_PIPE_ARR_R_INDEX2]]
; CHECK: %[[LOAD_STAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_STAR_PIPE_ARR_R2]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[LOAD_STAR_PIPE_ARR_R]]
;
; CHECK: %[[CINDEX7:.*]] = load {{.*}} %char_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX0:.*]] = sext {{.*}} %[[CINDEX7]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R0:.*]] = getelementptr {{.*}} @lar_arr, i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX0]]
; CHECK: %[[SINDEX5:.*]] = load {{.*}} %short_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX1:.*]] = sext {{.*}} %[[SINDEX5]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R1:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R0]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX1]]
; CHECK: %[[IINDEX3:.*]] = load {{.*}} %int_index
; CHECK: %[[LAR_PIPE_ARR_R_INDEX2:.*]] = sext {{.*}} %[[IINDEX3]]
; CHECK: %[[GEP_LAR_PIPE_ARR_R2:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R1]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX2]]
; CHECK: %[[LAR_PIPE_ARR_R_INDEX3:.*]] = load {{.*}} %long_index
; CHECK: %[[GEP_LAR_PIPE_ARR_R3:.*]] = getelementptr {{.*}} %[[GEP_LAR_PIPE_ARR_R2]], i64 0, i64 %[[LAR_PIPE_ARR_R_INDEX3]]
; CHECK: %[[LOAD_LAR_PIPE_ARR_R:.*]] = load {{.*}} %[[GEP_LAR_PIPE_ARR_R3]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[LOAD_LAR_PIPE_ARR_R]]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !arg_type_null_val !5 {
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
  call void @llvm.lifetime.start.p0(i64 1, ptr %char_index) #4
  store i8 4, ptr %char_index, align 1, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 2, ptr %short_index) #4
  store i16 3, ptr %short_index, align 2, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 4, ptr %int_index) #4
  store i32 2, ptr %int_index, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 8, ptr %long_index) #4
  store i64 1, ptr %long_index, align 8, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 42, ptr %i, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 4, ptr %f) #4
  store float 0x40091EB860000000, ptr %f, align 4, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 4, ptr %s) #4
  call void @llvm.memcpy.p0.p2.i64(ptr align 4 %s, ptr addrspace(2) align 4 @__const.foo.s, i64 4, i1 false)
  call void @llvm.lifetime.start.p0(i64 8, ptr %l) #4
  store i64 500, ptr %l, align 8, !tbaa !15
  %0 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom = sext i8 %0 to i64
  %arrayidx = getelementptr inbounds [5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 %idxprom
  %1 = load ptr addrspace(1), ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %2 = load i32, ptr %i, align 4, !tbaa !13
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %1, i32 noundef %2) #5
  %3 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom1 = sext i8 %3 to i64
  %arrayidx2 = getelementptr inbounds [5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @far_arr, i64 0, i64 %idxprom1
  %4 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom3 = sext i16 %4 to i64
  %arrayidx4 = getelementptr inbounds [4 x ptr addrspace(1)], ptr addrspace(1) %arrayidx2, i64 0, i64 %idxprom3
  %5 = load ptr addrspace(1), ptr addrspace(1) %arrayidx4, align 4, !tbaa !8
  %6 = load float, ptr %f, align 4, !tbaa !17
  call void @_Z19write_channel_intel11ocl_channelff(ptr addrspace(1) %5, float noundef %6) #5
  %7 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom5 = sext i8 %7 to i64
  %arrayidx6 = getelementptr inbounds [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @star_arr, i64 0, i64 %idxprom5
  %8 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom7 = sext i16 %8 to i64
  %arrayidx8 = getelementptr inbounds [4 x [3 x ptr addrspace(1)]], ptr addrspace(1) %arrayidx6, i64 0, i64 %idxprom7
  %9 = load i32, ptr %int_index, align 4, !tbaa !13
  %idxprom9 = sext i32 %9 to i64
  %arrayidx10 = getelementptr inbounds [3 x ptr addrspace(1)], ptr addrspace(1) %arrayidx8, i64 0, i64 %idxprom9
  %10 = load ptr addrspace(1), ptr addrspace(1) %arrayidx10, align 4, !tbaa !8
  call void @_Z19write_channel_intel11ocl_channel2stS_(ptr addrspace(1) %10, ptr noundef byval(%struct.st) align 4 %s) #5
  %11 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom11 = sext i8 %11 to i64
  %arrayidx12 = getelementptr inbounds [6 x [5 x [4 x [3 x ptr addrspace(1)]]]], ptr addrspace(1) @lar_arr, i64 0, i64 %idxprom11
  %12 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom13 = sext i16 %12 to i64
  %arrayidx14 = getelementptr inbounds [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) %arrayidx12, i64 0, i64 %idxprom13
  %13 = load i32, ptr %int_index, align 4, !tbaa !13
  %idxprom15 = sext i32 %13 to i64
  %arrayidx16 = getelementptr inbounds [4 x [3 x ptr addrspace(1)]], ptr addrspace(1) %arrayidx14, i64 0, i64 %idxprom15
  %14 = load i64, ptr %long_index, align 8, !tbaa !15
  %arrayidx17 = getelementptr inbounds [3 x ptr addrspace(1)], ptr addrspace(1) %arrayidx16, i64 0, i64 %14
  %15 = load ptr addrspace(1), ptr addrspace(1) %arrayidx17, align 8, !tbaa !8
  %16 = load i64, ptr %l, align 8, !tbaa !15
  call void @_Z19write_channel_intel11ocl_channelll(ptr addrspace(1) %15, i64 noundef %16) #5
  %17 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom18 = sext i8 %17 to i64
  %arrayidx19 = getelementptr inbounds [5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 %idxprom18
  %18 = load ptr addrspace(1), ptr addrspace(1) %arrayidx19, align 4, !tbaa !8
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %18) #5
  store i32 %call, ptr %i, align 4, !tbaa !13
  %19 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom20 = sext i8 %19 to i64
  %arrayidx21 = getelementptr inbounds [5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @far_arr, i64 0, i64 %idxprom20
  %20 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom22 = sext i16 %20 to i64
  %arrayidx23 = getelementptr inbounds [4 x ptr addrspace(1)], ptr addrspace(1) %arrayidx21, i64 0, i64 %idxprom22
  %21 = load ptr addrspace(1), ptr addrspace(1) %arrayidx23, align 4, !tbaa !8
  %call24 = call float @_Z18read_channel_intel11ocl_channelf(ptr addrspace(1) %21) #5
  store float %call24, ptr %f, align 4, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 4, ptr %tmp) #4
  %22 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom25 = sext i8 %22 to i64
  %arrayidx26 = getelementptr inbounds [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @star_arr, i64 0, i64 %idxprom25
  %23 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom27 = sext i16 %23 to i64
  %arrayidx28 = getelementptr inbounds [4 x [3 x ptr addrspace(1)]], ptr addrspace(1) %arrayidx26, i64 0, i64 %idxprom27
  %24 = load i32, ptr %int_index, align 4, !tbaa !13
  %idxprom29 = sext i32 %24 to i64
  %arrayidx30 = getelementptr inbounds [3 x ptr addrspace(1)], ptr addrspace(1) %arrayidx28, i64 0, i64 %idxprom29
  %25 = load ptr addrspace(1), ptr addrspace(1) %arrayidx30, align 4, !tbaa !8
  call void @_Z18read_channel_intel11ocl_channel2st(ptr sret(%struct.st) align 4 %tmp, ptr addrspace(1) %25) #5
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %s, ptr align 4 %tmp, i64 4, i1 false), !tbaa.struct !19
  call void @llvm.lifetime.end.p0(i64 4, ptr %tmp) #4
  %26 = load i8, ptr %char_index, align 1, !tbaa !8
  %idxprom31 = sext i8 %26 to i64
  %arrayidx32 = getelementptr inbounds [6 x [5 x [4 x [3 x ptr addrspace(1)]]]], ptr addrspace(1) @lar_arr, i64 0, i64 %idxprom31
  %27 = load i16, ptr %short_index, align 2, !tbaa !11
  %idxprom33 = sext i16 %27 to i64
  %arrayidx34 = getelementptr inbounds [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) %arrayidx32, i64 0, i64 %idxprom33
  %28 = load i32, ptr %int_index, align 4, !tbaa !13
  %idxprom35 = sext i32 %28 to i64
  %arrayidx36 = getelementptr inbounds [4 x [3 x ptr addrspace(1)]], ptr addrspace(1) %arrayidx34, i64 0, i64 %idxprom35
  %29 = load i64, ptr %long_index, align 8, !tbaa !15
  %arrayidx37 = getelementptr inbounds [3 x ptr addrspace(1)], ptr addrspace(1) %arrayidx36, i64 0, i64 %29
  %30 = load ptr addrspace(1), ptr addrspace(1) %arrayidx37, align 8, !tbaa !8
  %call38 = call i64 @_Z18read_channel_intel11ocl_channell(ptr addrspace(1) %30) #5
  store i64 %call38, ptr %l, align 8, !tbaa !15
  call void @llvm.lifetime.end.p0(i64 8, ptr %l) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %s) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %f) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %long_index) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %int_index) #4
  call void @llvm.lifetime.end.p0(i64 2, ptr %short_index) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %char_index) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p2.i64(ptr noalias nocapture writeonly, ptr addrspace(2) noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelff(ptr addrspace(1), float noundef) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channel2stS_(ptr addrspace(1), ptr noundef byval(%struct.st) align 4) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelll(ptr addrspace(1), i64 noundef) #3

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #3

; Function Attrs: convergent nounwind
declare float @_Z18read_channel_intel11ocl_channelf(ptr addrspace(1)) #3

; Function Attrs: convergent nounwind
declare void @_Z18read_channel_intel11ocl_channel2st(ptr sret(%struct.st) align 4, ptr addrspace(1)) #3

; Function Attrs: convergent nounwind
declare i64 @_Z18read_channel_intel11ocl_channell(ptr addrspace(1)) #3

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind }

!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!sycl.kernels = !{!7}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 3}
!3 = !{i32 8}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!7 = !{ptr @foo}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"short", !9, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !9, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !9, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"float", !9, i64 0}
!19 = !{i64 0, i64 4, !13}

; DEBUGIFY-NOT: WARNING: Missing line
