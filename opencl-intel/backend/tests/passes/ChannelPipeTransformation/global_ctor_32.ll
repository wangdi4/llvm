; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
; channel int bar __attribute__((depth(0)));
; channel float far __attribute__((depth(3)));
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
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
; CHECK: %opencl.pipe_t{{.*}} = type opaque
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

; CHECK: @llvm.global_ctors = {{.*}} @__global_pipes_ctor
;
; CHECK: declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32)
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
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_STAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 2)

; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_STAR_ARR]] {{.*}}, i32 60, i32 4, i32 1)
;
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_BAR_ARR]], i32 0, i32 4)
; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_BAR_ARR]] {{.*}}, i32 5, i32 4, i32 1)
;
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 3)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 0)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 1)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 2)
; CHECK-DAG: store {{.*}}* @[[PIPE_FAR_ARR]].bs, i32 0, i32 {{[0-9]+}}) {{.*}}, {{.*}}* @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 3)
; CHECK-DAG: call void @__pipe_init_array_intel({{.*}} @[[PIPE_FAR_ARR]] {{.*}}, i32 20, i32 4, i32 3)

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !12 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !12 {
entry:
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.channels = !{!0, !4, !6, !7, !8, !9}
!llvm.module.flags = !{!10}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!11}
!opencl.spir.version = !{!11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}
!llvm.ident = !{!13}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !1, !2, !3}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{!"depth", i32 0}
!4 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !1, !2, !5}
!5 = !{!"depth", i32 3}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !1, !2}
!7 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !1, !2, !3}
!8 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !1, !2, !5}
!9 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !1, !2}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{i32 2, i32 0}
!12 = !{}
!13 = !{!"clang version 5.0.0 "}
