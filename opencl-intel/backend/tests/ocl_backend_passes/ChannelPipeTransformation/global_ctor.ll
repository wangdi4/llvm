; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
; channel int bar __attribute__((depth(0)));
; channel float far __attribtue__((depth(3)));
;
; struct Foo {
;   int i;
; };
; channel struct Foo star;
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
;
;   write_channel_altera(bar, 42);
;   write_channel_altera(far, f);
;   write_channel_altera(star, st);
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque
; CHECK: %opencl.pipe_t{{.*}} = type opaque
%struct.Foo = type { i32 }
; CHECK: %struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = common addrspace(1) global [5 x [4 x [3 x %opencl.pipe_t{{.*}} addrspace(1)*]]] zeroinitializer, align 4
;

; CHECK-DAG: @[[PIPE_BAR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_FAR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_STAR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_BAR_ARR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_FAR_ARR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_STAR_ARR]].bs = common addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
;

; CHECK: @llvm.global_ctors = {{.*}} @__global_pipes_ctor
;
; CHECK: declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32) #3
;
; CHECK: declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32)
;
; CHECK: define void @__global_pipes_ctor()
;
; CHECK-DAG: call void @__pipe_init_intel({{.*}}* @[[PIPE_STAR]].bs {{.*}}, i32 4, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR]].bs {{.*}}, {{.*}}* @[[PIPE_STAR]]
;
; CHECK-DAG: call void @__pipe_init_intel({{.*}}* @[[PIPE_BAR]].bs {{.*}}, i32 4, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR]].bs {{.*}}, {{.*}}* @[[PIPE_BAR]]
;
; CHECK-DAG: call void @__pipe_init_intel({{.*}}* @[[PIPE_FAR]].bs {{.*}}, i32 4, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR]].bs {{.*}}, {{.*}}* @[[PIPE_FAR]]
;
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 48) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 96) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 144) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 192) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 240) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 288) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 336) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 384) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 432) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 480) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 528) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 576) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 624) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 672) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 720) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 768) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 816) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 864) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 912) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 960) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1008) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1056) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1104) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1152) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1200) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1248) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1296) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1344) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1392) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1440) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1488) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1536) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1584) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1632) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1680) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1728) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1776) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1824) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1872) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1920) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 1968) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2016) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2064) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2112) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2160) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2208) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2256) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2304) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2352) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2400) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2448) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2496) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2544) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2592) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2640) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2688) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2736) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2784) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 2832) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 2)

; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_STAR_ARR]] {{.*}}, i32 60, i32 4, i32 1)
;
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 48) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 96) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 144) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 192) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 4)
; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_BAR_ARR]] {{.*}}, i32 5, i32 4, i32 1)
;
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 56) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 112) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 168) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 224) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 280) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 336) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 392) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 448) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 504) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 560) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 616) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 672) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 728) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 784) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 840) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 896) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 952) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 1008) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 1064) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 3)
; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_FAR_ARR]] {{.*}}, i32 20, i32 4, i32 3)

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  ret void
}

attributes #0 = { norecurse nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !10, !12, !13, !14, !15}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!16}
!opencl.spir.version = !{!16}
!opencl.used.extensions = !{!17}
!opencl.used.optional.core.features = !{!17}
!opencl.compiler.options = !{!17}
!llvm.ident = !{!18}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !7, !8, !9}
!7 = !{!"packet_size", i32 4}
!8 = !{!"packet_align", i32 4}
!9 = !{!"depth", i32 0}
!10 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !7, !8, !11}
!11 = !{!"depth", i32 3}
!12 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !7, !8}
!13 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !7, !8, !9}
!14 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !7, !8, !11}
!15 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !7, !8}
!16 = !{i32 2, i32 0}
!17 = !{}
!18 = !{!"clang version 3.8.1 "}
