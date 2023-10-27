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

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@bar = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !2
@star = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !2
@star_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0

; CHECK:      @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !1
; CHECK-NEXT: @[[PIPE_BAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !2
; CHECK-NEXT: @[[PIPE_FAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0
; CHECK-NEXT: @[[PIPE_STAR_ARR]].bs = addrspace(1) global [{{[0-9]+}} x i8] zeroinitializer, align 4
; CHECK:      @llvm.global_ctors = {{.*}} @__pipe_global_ctor
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_STAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_STAR]].bs, ptr addrspace(1) @[[PIPE_STAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_BAR]].bs, i32 4, i32 0
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_BAR]].bs, ptr addrspace(1) @[[PIPE_BAR]]
;
; CHECK-DAG: call void @__pipe_init_fpga(ptr addrspace(1) @[[PIPE_FAR]].bs, i32 4, i32 3
; CHECK-DAG: store ptr addrspace(1) @[[PIPE_FAR]].bs, ptr addrspace(1) @[[PIPE_FAR]]

; CHECK: store {{.*}} @[[PIPE_BAR_ARR]].bs, {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 1)
; CHECK: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 2)
; CHECK: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 3)
; CHECK: store {{.*}} @[[PIPE_BAR_ARR]].bs, i64 0, i64 {{[0-9]+}}){{.*}} @[[PIPE_BAR_ARR]], i32 0, i32 4)
; CHECK: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_BAR_ARR]]{{.*}}, i32 5, i32 4, i32 0, i32 0)

; CHECK-LABEL: body{{.*}}:
; CHECK-NEXT: [[I2:%.*]] = phi i64 [ 0, %preheader2 ], [ [[I2]], %[[BODY1:.*]] ], [ [[I2]], %[[LATCH1:.*]] ], [ [[I2INC:%.*]], %[[LATCH2:.*]] ]
; CHECK-NEXT: [[I1:%.*]] = phi i64 [ 0, %preheader2 ], [ [[I1]], %[[BODY1]] ], [ [[I1INC:%.*]], %[[LATCH1]] ], [ 0, %[[LATCH2]] ]
; CHECK-NEXT: [[I0:%.*]] = phi i64 [ 0, %preheader2 ], [ [[I0INC:%.*]], %[[BODY1]] ], [ 0, %[[LATCH1]] ], [ 0, %[[LATCH2]] ]
; CHECK-NEXT: [[J:%.*]] = phi i64 [ 0, %preheader2 ], [ [[JINC:%.*]], %[[BODY1]] ], [ [[JINC]], %[[LATCH1]] ], [ [[JINC]], %[[LATCH2]] ]
; CHECK-NEXT: [[JINC]] = add i64 [[J]], 1
; CHECK-NEXT: [[ElemInd:%elem.ind[0-9]+]] = mul nuw nsw i64 [[J]], 456
; CHECK-NEXT: [[GEP0:%[0-9]+]] = getelementptr [27360 x i8], ptr addrspace(1) @[[PIPE_STAR_ARR]].bs, i64 0, i64 [[ElemInd]]
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr [5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @[[PIPE_STAR_ARR]], i64 0, i64 [[I2]], i64 [[I1]], i64 [[I0]]
; CHECK-NEXT: store ptr addrspace(1) [[GEP0]], ptr addrspace(1) [[GEP1]], align 8
; CHECK-NEXT: [[I0INC]] = add i64 [[I0]], 1
; CHECK-NEXT: [[CMP0:%.*]] = icmp slt i64 [[I0INC]], 3
; CHECK-NEXT: br i1 [[CMP0]], label %[[BODY1]], label %[[LATCH1]]

; CHECK-LABEL: latch{{.*}}:
; CHECK-NEXT: [[I1INC]] = add i64 [[I1]], 1
; CHECK-NEXT: [[CMP1:%.*]] = icmp slt i64 [[I1INC]], 4
; CHECK-NEXT: br i1 [[CMP1]], label %[[BODY1]], label %[[LATCH2]]

; CHECK-LABEL: latch{{.*}}:
; CHECK-NEXT: [[I2INC]] = add i64 [[I2]], 1
; CHECK-NEXT: [[CMP2:%.*]] = icmp slt i64 [[I2INC]], 5
; CHECK-NEXT: br i1 [[CMP2]], label %[[BODY1]], label %[[EXIT:.*]]

; CHECK: exit{{.*}}:
; CHECK-NEXT: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_STAR_ARR]]{{.*}}, i32 60, i32 4, i32 0, i32 0)

; CHECK-LABEL: body{{.*}}:
; CHECK: [[ElemInd1:%elem.ind[0-9]*]] = mul nuw nsw i64 %j{{.*}}, 464
; CHECK: [[GEP2:%[0-9]+]] = getelementptr [9280 x i8], ptr addrspace(1) @[[PIPE_FAR_ARR]].bs, i64 0, i64 [[ElemInd1]]
; CHECK: [[GEP3:%[0-9]+]] = getelementptr [5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @[[PIPE_FAR_ARR]], i64 0, i64 {{.*}}, i64 {{.*}}
; CHECK: store ptr addrspace(1) [[GEP2]], ptr addrspace(1) [[GEP3]], align 8
; CHECK-LABEL: exit{{.*}}:
; CHECK: call void @__pipe_init_array_fpga({{.*}} @[[PIPE_FAR_ARR]]{{.*}}, i32 20, i32 4, i32 3, i32 0)

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
