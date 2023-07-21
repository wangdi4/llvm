; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
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
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@bar = addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star = addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@bar_arr = addrspace(1) global [5 x target("spirv.Channel")] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x target("spirv.Channel")]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star_arr = addrspace(1) global [5 x [4 x [3 x target("spirv.Channel")]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK:      @llvm.global_ctors = {{.*}} @__pipe_global_ctor
; CHECK:      @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 16, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 16, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_STAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_STAR]].bs, ptr addrspace(1) @[[PIPE_STAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_BAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_BAR]].bs, ptr addrspace(1) @[[PIPE_BAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_FAR]].bs, i32 4, i32 3
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_FAR]].bs, ptr addrspace(1) @[[PIPE_FAR]]
;
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_STAR_ARR]].bs, ptr addrspace(1) @[[PIPE_STAR_ARR]]

; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 0, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 1, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 2, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 1, i32 3, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 0, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 1, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 2, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 2, i32 3, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 0, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 1, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 2, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 3, i32 3, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 0, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 1, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 2, i32 2)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 0)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 1)
; CHECK-DAG: store {{.*}}@[[PIPE_STAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_STAR_ARR]], i32 0, i32 4, i32 3, i32 2)

; CHECK-DAG: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_STAR_ARR]]{{.*}}, i32 60, i32 4, i32 0
;
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, {{.*}} @[[PIPE_BAR_ARR]]
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 4)
; CHECK-DAG: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_BAR_ARR]]{{.*}}, i32 5, i32 4, i32 0
;
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, {{.*}} @[[PIPE_FAR_ARR]]
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 0, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 0)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 1, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 0)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 2, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 0)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 3, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 0)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_FAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_FAR_ARR]], i32 0, i32 4, i32 3)
; CHECK-DAG: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_FAR_ARR]]{{.*}}, i32 20, i32 4, i32 3

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !4 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }

!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 3}
!3 = !{i32 2, i32 0}
!4 = !{}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!6 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING: Missing line
