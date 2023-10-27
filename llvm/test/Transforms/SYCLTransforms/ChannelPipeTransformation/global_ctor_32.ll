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
; channel int bar_arr[4] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir-unknown-unknown-intelfpga"

@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !2
@star = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
@star_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK:      @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK:      @llvm.global_ctors = {{.*}} @__pipe_global_ctor
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_BAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_BAR]].bs, ptr addrspace(1) @[[PIPE_BAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_FAR]].bs, i32 4, i32 3
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_FAR]].bs, ptr addrspace(1) @[[PIPE_FAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_STAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_STAR]].bs, ptr addrspace(1) @[[PIPE_STAR]]

; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, {{.*}} @[[PIPE_BAR_ARR]],
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}), {{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 1)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}), {{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 2)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}), {{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 3)
; CHECK-DAG: store {{.*}} @[[PIPE_BAR_ARR]].bs, i32 0, i32 {{[0-9]+}}), {{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 4)
; CHECK-DAG: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_BAR_ARR]]{{.*}}, i32 5, i32 4, i32 0, i32 0)

; CHECK-LABEL: body{{.*}}:
; CHECK: [[ElemInd:%elem.ind[0-9]+]] = mul nuw nsw i32 %j{{.*}}, 456
; CHECK: [[GEP0:%[0-9]+]] = getelementptr [27360 x i8], ptr addrspace(1) @[[PIPE_STAR_ARR]].bs, i32 0, i32 [[ElemInd]]
; CHECK: [[GEP1:%[0-9]+]] = getelementptr [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @[[PIPE_STAR_ARR]], i32 0, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}
; CHECK: store ptr addrspace(1) [[GEP0]], ptr addrspace(1) [[GEP1]], align 4
; CHECK-LABEL: exit{{.*}}:
; CHECK: call void @__pipe_init_array_fpga(ptr addrspace(1) @[[PIPE_STAR_ARR]], i32 60, i32 4, i32 0, i32 0)

; CHECK-LABEL: body:
; CHECK-NEXT: [[I1:%.*]] = phi i32 [ 0, %exit8 ], [ [[I1]], %body ], [ [[I1INC:%.*]], %latch1 ]
; CHECK-NEXT: [[I0:%.*]] = phi i32 [ 0, %exit8 ], [ [[I0INC:%.*]], %body ], [ 0, %latch1 ]
; CHECK-NEXT: [[J:%.*]] = phi i32 [ 0, %exit8 ], [ [[JINC:%.*]], %body ], [ [[JINC]], %latch1 ]
; CHECK-NEXT: [[JINC]] = add i32 [[J]], 1
; CHECK-NEXT: [[ElemInd1:%elem.ind[0-9]*]] = mul nuw nsw i32 [[J]], 464
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr [9280 x i8], ptr addrspace(1) @[[PIPE_FAR_ARR]].bs, i32 0, i32 [[ElemInd1]]
; CHECK-NEXT: [[GEP3:%[0-9]+]] = getelementptr [5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @[[PIPE_FAR_ARR]], i32 0, i32 [[I1]], i32 [[I0]]
; CHECK-NEXT: store ptr addrspace(1) [[GEP2]], ptr addrspace(1) [[GEP3]], align 4
; CHECK-NEXT: [[I0INC]] = add i32 [[I0]], 1
; CHECK-NEXT: [[CMP:%.*]] = icmp slt i32 [[I0INC]], 4
; CHECK-NEXT: br i1 [[CMP]], label %body, label %latch1

; CHECK-LABEL: latch1:
; CHECK-NEXT: [[I1INC]] = add i32 [[I1]], 1
; CHECK-NEXT: [[CMP1:%.*]] = icmp slt i32 [[I1INC]], 5
; CHECK-NEXT: br i1 [[CMP1]], label %body, label %exit

; CHECK-LABEL: exit:
; CHECK-NEXT: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_FAR_ARR]]{{.*}}, i32 20, i32 4, i32 3, i32 0)
; CHECK-NEXT: ret void

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !4 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }

!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 3}
!3 = !{i32 2, i32 0}
!4 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING: Missing line
