; RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring  -VPlanDriver -vplan-force-vf=2 < %s 2>&1 | FileCheck %s


; CHECK-LABEL:@"_ZGVdN8uuuuuuuuuuuuuuuuu_TSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"
; CHECK:[[COSVEC1:%.*]] = alloca <2 x float>
; CHECK:[[VEC1:%.*]] = alloca [2 x [624 x i32]], align 4
; CHECK-LABEL:vector.body
; CHECK: [[PRIV:%.*]] = bitcast [2 x [624 x i32]]* [[VEC1]] to [624 x i32]*
; CHECK: [[GEP:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV]], <2 x i32> <i32 0, i32 1>
; CHECK: [[BC:%.*]] = bitcast <2 x [624 x i32]*>  [[GEP]] to <2 x i8*>
; CHECK: [[E2:%.*]] = extractelement <2 x i8*> [[BC]], i32 1
; CHECK: [[E1:%.*]] = extractelement <2 x i8*> [[BC]], i32 0
; CHECK-NEXT: call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull [[E1]]) #2
; CHECK-NEXT: call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull [[E2]]) #2

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: nounwind
define void @"_ZTSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"(float %_arg_, float %_arg_1, float addrspace(1)* %_arg_2, %"class.cl::sycl::range"* byval %_arg_Range, %"class.cl::sycl::range"* byval %_arg_Offset, float addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval %_arg_Range4, %"class.cl::sycl::range"* byval %_arg_Offset5, float addrspace(1)* %_arg_6, %"class.cl::sycl::range"* byval %_arg_Range7, %"class.cl::sycl::range"* byval %_arg_Offset8, float addrspace(1)* %_arg_9, %"class.cl::sycl::range"* byval %_arg_Range10, %"class.cl::sycl::range"* byval %_arg_Offset11, float addrspace(1)* %_arg_12, %"class.cl::sycl::range"* byval %_arg_Range13, %"class.cl::sycl::range"* byval %_arg_Offset14) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_type_qual !10 !kernel_arg_base_type !9 !vectorized_kernel !11 !no_barrier_path !12 !vectorized_width !13 !scalarized_kernel !14 {
  %cosPtr = alloca float
  %1 = alloca [624 x i32], align 4
  %2 = call i64 @_Z13get_global_idj(i32 0) #3
  %3 = getelementptr inbounds float, float addrspace(1)* %_arg_2, i64 %2
  %4 = addrspacecast float addrspace(1)* %3 to float*
  %5 = load float, float* %4, align 4
  %6 = getelementptr inbounds float, float addrspace(1)* %_arg_3, i64 %2
  %7 = addrspacecast float addrspace(1)* %6 to float*
  %8 = load float, float* %7, align 4
  %9 = getelementptr inbounds float, float addrspace(1)* %_arg_6, i64 %2
  %10 = addrspacecast float addrspace(1)* %9 to float*
  %11 = load float, float* %10, align 4
  %12 = fmul float %_arg_1, %_arg_1
  %13 = fmul float %12, 5.000000e-01
  %14 = fsub float %_arg_, %13
  %15 = call float @_Z4sqrtf(float %5) #2
  %16 = bitcast [624 x i32]* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %16) #2
  %17 = trunc i64 %2 to i32
  %18 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 0
  store i32 %17, i32* %18, align 4
  br label %19

; <label>:19:                                     ; preds = %19, %0
  %20 = phi i32 [ %17, %0 ], [ %25, %19 ]
  %indvars.iv.i.i = phi i64 [ 1, %0 ], [ %indvars.iv.next.i.i, %19 ]
  %21 = ashr i32 %20, 30
  %22 = xor i32 %21, %20
  %23 = mul i32 %22, 1812433253
  %24 = trunc i64 %indvars.iv.i.i to i32
  %25 = add i32 %23, %24
  %26 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %indvars.iv.i.i
  store i32 %25, i32* %26, align 4
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i.i = icmp eq i64 %indvars.iv.next.i.i, 624
  br i1 %exitcond.i.i, label %27, label %19

; <label>:27:                                     ; preds = %19
  %28 = fmul float %14, %5
  %29 = fmul float %15, %_arg_1
  %30 = load i32, i32* %18, align 4
  br label %31

; <label>:31:                                     ; preds = %31, %27
  %.021.i.i = phi i32 [ 0, %27 ], [ %121, %31 ]
  %.0120.i.i = phi i32 [ %30, %27 ], [ %72, %31 ]
  %.0219.i.i = phi float [ 0.000000e+00, %27 ], [ %120, %31 ]
  %.118.i.i = phi i32 [ 0, %27 ], [ %67, %31 ]
  %32 = icmp eq i32 %.118.i.i, 623
  %33 = add nsw i32 %.118.i.i, 1
  %34 = select i1 %32, i32 0, i32 %33
  %35 = icmp sgt i32 %.118.i.i, 226
  %.v.i.i = select i1 %35, i32 -227, i32 397
  %36 = add nsw i32 %.v.i.i, %.118.i.i
  %37 = sext i32 %.118.i.i to i64
  %38 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %37
  store i32 %.0120.i.i, i32* %38, align 4
  %39 = sext i32 %34 to i64
  %40 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %39
  %41 = load i32, i32* %40, align 4
  %42 = and i32 %.0120.i.i, -2147483648
  %43 = and i32 %41, 2147483646
  %44 = or i32 %43, %42
  %45 = sext i32 %36 to i64
  %46 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %45
  %47 = load i32, i32* %46, align 4
  %48 = lshr exact i32 %44, 1
  %49 = and i32 %41, 1
  %50 = sub nsw i32 0, %49
  %51 = and i32 %50, -1727483681
  %52 = xor i32 %51, %47
  %53 = xor i32 %52, %48
  store i32 %53, i32* %38, align 4
  %54 = lshr i32 %53, 11
  %55 = xor i32 %54, %53
  %56 = shl i32 %55, 7
  %57 = and i32 %56, -1658038656
  %58 = xor i32 %57, %55
  %59 = shl i32 %58, 15
  %60 = and i32 %59, -272236544
  %61 = xor i32 %60, %58
  %62 = lshr i32 %61, 18
  %63 = xor i32 %62, %61
  %64 = uitofp i32 %63 to float
  %65 = icmp eq i32 %34, 623
  %66 = add nsw i32 %34, 1
  %67 = select i1 %65, i32 0, i32 %66
  %68 = icmp sgt i32 %34, 226
  %.v1.i.i = select i1 %68, i32 -227, i32 397
  %69 = add nsw i32 %.v1.i.i, %34
  store i32 %41, i32* %40, align 4
  %70 = sext i32 %67 to i64
  %71 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %70
  %72 = load i32, i32* %71, align 4
  %73 = and i32 %41, -2147483648
  %74 = and i32 %72, 2147483646
  %75 = or i32 %74, %73
  %76 = sext i32 %69 to i64
  %77 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %76
  %78 = load i32, i32* %77, align 4
  %79 = lshr exact i32 %75, 1
  %80 = and i32 %72, 1
  %81 = sub nsw i32 0, %80
  %82 = and i32 %81, -1727483681
  %83 = xor i32 %82, %78
  %84 = xor i32 %83, %79
  store i32 %84, i32* %40, align 4
  %85 = lshr i32 %84, 11
  %86 = xor i32 %85, %84
  %87 = shl i32 %86, 7
  %88 = and i32 %87, -1658038656
  %89 = xor i32 %88, %86
  %90 = shl i32 %89, 15
  %91 = and i32 %90, -272236544
  %92 = xor i32 %91, %89
  %93 = lshr i32 %92, 18
  %94 = xor i32 %93, %92
  %95 = uitofp i32 %94 to float
  %96 = fadd float %64, 1.000000e+00
  %97 = fmul float %96, 0x3DF0000000000000
  %98 = fadd float %95, 1.000000e+00
  %99 = fmul float %98, 0x3DF0000000000000
  %100 = call float @_Z3logf(float %97) #2
  %101 = fmul float %100, -2.000000e+00
  %102 = call float @_Z4fmaxff(float %101, float 0.000000e+00) #2
  %103 = call float @_Z4sqrtf(float %102) #2
  %104 = fmul float %99, 0x401921FB60000000
  %sinPtr = call float @_Z6sincosfPf(float %104, float* %cosPtr)
  %cosVal = load float, float* %cosPtr
  %105 = fmul float %103, %cosVal
  %106 = fmul float %103, %sinPtr
  %107 = fmul float %29, %105
  %108 = fadd float %28, %107
  %109 = call float @_Z3expf(float %108) #2
  %110 = fmul float %8, %109
  %111 = fsub float %110, %11
  %112 = call float @_Z4fmaxff(float %111, float 0.000000e+00) #2
  %113 = fadd float %.0219.i.i, %112
  %114 = fmul float %29, %106
  %115 = fadd float %28, %114
  %116 = call float @_Z3expf(float %115) #2
  %117 = fmul float %8, %116
  %118 = fsub float %117, %11
  %119 = call float @_Z4fmaxff(float %118, float 0.000000e+00) #2
  %120 = fadd float %113, %119
  %121 = add nuw nsw i32 %.021.i.i, 2
  %122 = icmp ult i32 %121, 262144
  br i1 %122, label %31, label %"_ZZZ2mciENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit"

"_ZZZ2mciENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit": ; preds = %31
  %.lcssa = phi float [ %120, %31 ]
  %123 = fmul float %.lcssa, 0x3ED0000000000000
  %124 = fmul float %5, %_arg_
  %125 = fsub float -0.000000e+00, %124
  %126 = call float @_Z3expf(float %125) #2
  %127 = fmul float %123, %126
  %128 = fsub float %127, %8
  %129 = call float @_Z3expf(float %125) #2
  %130 = fmul float %11, %129
  %131 = fadd float %128, %130
  %132 = getelementptr inbounds float, float addrspace(1)* %_arg_9, i64 %2
  %133 = addrspacecast float addrspace(1)* %132 to float*
  store float %127, float* %133, align 4
  %134 = getelementptr inbounds float, float addrspace(1)* %_arg_12, i64 %2
  %135 = addrspacecast float addrspace(1)* %134 to float*
  store float %131, float* %135, align 4
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %16) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare float @_Z4sqrtf(float) local_unnamed_addr #2

; Function Attrs: nounwind
declare float @_Z3logf(float) local_unnamed_addr #2

; Function Attrs: nounwind
declare float @_Z4fmaxff(float, float) local_unnamed_addr #2

; Function Attrs: nounwind
declare float @_Z3cosf(float) local_unnamed_addr #2

; Function Attrs: nounwind
declare float @_Z3sinf(float) local_unnamed_addr #2

; Function Attrs: nounwind
declare float @_Z3expf(float) local_unnamed_addr #2

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #3

define [7 x i64] @"WG.boundaries._ZTSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"(float, float, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*) !ocl_recommended_vector_length !15 {
entry:
  %17 = call i64 @_Z14get_local_sizej(i32 0)
  %18 = call i64 @get_base_global_id.(i32 0)
  %19 = call i64 @_Z14get_local_sizej(i32 1)
  %20 = call i64 @get_base_global_id.(i32 1)
  %21 = call i64 @_Z14get_local_sizej(i32 2)
  %22 = call i64 @get_base_global_id.(i32 2)
  %23 = insertvalue [7 x i64] undef, i64 %17, 2
  %24 = insertvalue [7 x i64] %23, i64 %18, 1
  %25 = insertvalue [7 x i64] %24, i64 %19, 4
  %26 = insertvalue [7 x i64] %25, i64 %20, 3
  %27 = insertvalue [7 x i64] %26, i64 %21, 6
  %28 = insertvalue [7 x i64] %27, i64 %22, 5
  %29 = insertvalue [7 x i64] %28, i64 1, 0
  ret [7 x i64] %29
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
declare float @_Z6sincosfPf(float, float*) #4

; Function Attrs: nounwind
define void @"_ZGVdN8uuuuuuuuuuuuuuuuu_TSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"(float %_arg_, float %_arg_1, float addrspace(1)* %_arg_2, %"class.cl::sycl::range"* byval %_arg_Range, %"class.cl::sycl::range"* byval %_arg_Offset, float addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval %_arg_Range4, %"class.cl::sycl::range"* byval %_arg_Offset5, float addrspace(1)* %_arg_6, %"class.cl::sycl::range"* byval %_arg_Range7, %"class.cl::sycl::range"* byval %_arg_Offset8, float addrspace(1)* %_arg_9, %"class.cl::sycl::range"* byval %_arg_Range10, %"class.cl::sycl::range"* byval %_arg_Offset11, float addrspace(1)* %_arg_12, %"class.cl::sycl::range"* byval %_arg_Range13, %"class.cl::sycl::range"* byval %_arg_Offset14) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_type_qual !10 !kernel_arg_base_type !9 !vectorized_kernel !14 !no_barrier_path !12 !ocl_recommended_vector_length !15 !vectorized_width !15 !vectorization_dimension !6 !scalarized_kernel !5 !can_unite_workgroups !16 {
  %cosPtr = alloca float
  %1 = alloca [624 x i32], align 4
  %2 = call i64 @_Z13get_global_idj(i32 0) #3
  br label %simd.begin.region

simd.begin.region:                                ; preds = %0
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(float %_arg_, float %_arg_1, float addrspace(1)* %_arg_2, %"class.cl::sycl::range"* %_arg_Range, %"class.cl::sycl::range"* %_arg_Offset, float addrspace(1)* %_arg_3, %"class.cl::sycl::range"* %_arg_Range4, %"class.cl::sycl::range"* %_arg_Offset5, float addrspace(1)* %_arg_6, %"class.cl::sycl::range"* %_arg_Range7, %"class.cl::sycl::range"* %_arg_Offset8, float addrspace(1)* %_arg_9, %"class.cl::sycl::range"* %_arg_Range10, %"class.cl::sycl::range"* %_arg_Offset11, float addrspace(1)* %_arg_12, %"class.cl::sycl::range"* %_arg_Range13, %"class.cl::sycl::range"* %_arg_Offset14), "QUAL.OMP.PRIVATE"(float* %cosPtr, [624 x i32]* %1), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %3 = sext i32 %index to i64
  %add = add nuw i64 %3, %2
  %4 = getelementptr inbounds float, float addrspace(1)* %_arg_2, i64 %add
  %5 = addrspacecast float addrspace(1)* %4 to float*
  %6 = load float, float* %5, align 4
  %7 = getelementptr inbounds float, float addrspace(1)* %_arg_3, i64 %add
  %8 = addrspacecast float addrspace(1)* %7 to float*
  %9 = load float, float* %8, align 4
  %10 = getelementptr inbounds float, float addrspace(1)* %_arg_6, i64 %add
  %11 = addrspacecast float addrspace(1)* %10 to float*
  %12 = load float, float* %11, align 4
  %13 = fmul float %_arg_1, %_arg_1
  %14 = fmul float %13, 5.000000e-01
  %15 = fsub float %_arg_, %14
  %16 = call float @_Z4sqrtf(float %6) #2
  %17 = bitcast [624 x i32]* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %17) #2
  %18 = trunc i64 %add to i32
  %19 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 0
  store i32 %18, i32* %19, align 4
  br label %20

; <label>:20:                                     ; preds = %20, %simd.loop
  %21 = phi i32 [ %18, %simd.loop ], [ %26, %20 ]
  %indvars.iv.i.i = phi i64 [ 1, %simd.loop ], [ %indvars.iv.next.i.i, %20 ]
  %22 = ashr i32 %21, 30
  %23 = xor i32 %22, %21
  %24 = mul i32 %23, 1812433253
  %25 = trunc i64 %indvars.iv.i.i to i32
  %26 = add i32 %24, %25
  %27 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %indvars.iv.i.i
  store i32 %26, i32* %27, align 4
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i.i = icmp eq i64 %indvars.iv.next.i.i, 624
  br i1 %exitcond.i.i, label %28, label %20

; <label>:28:                                     ; preds = %20
  %29 = fmul float %15, %6
  %30 = fmul float %16, %_arg_1
  %31 = load i32, i32* %19, align 4
  br label %32

; <label>:32:                                     ; preds = %32, %28
  %.021.i.i = phi i32 [ 0, %28 ], [ %122, %32 ]
  %.0120.i.i = phi i32 [ %31, %28 ], [ %73, %32 ]
  %.0219.i.i = phi float [ 0.000000e+00, %28 ], [ %121, %32 ]
  %.118.i.i = phi i32 [ 0, %28 ], [ %68, %32 ]
  %33 = icmp eq i32 %.118.i.i, 623
  %34 = add nsw i32 %.118.i.i, 1
  %35 = select i1 %33, i32 0, i32 %34
  %36 = icmp sgt i32 %.118.i.i, 226
  %.v.i.i = select i1 %36, i32 -227, i32 397
  %37 = add nsw i32 %.v.i.i, %.118.i.i
  %38 = sext i32 %.118.i.i to i64
  %39 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %38
  store i32 %.0120.i.i, i32* %39, align 4
  %40 = sext i32 %35 to i64
  %41 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %40
  %42 = load i32, i32* %41, align 4
  %43 = and i32 %.0120.i.i, -2147483648
  %44 = and i32 %42, 2147483646
  %45 = or i32 %44, %43
  %46 = sext i32 %37 to i64
  %47 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %46
  %48 = load i32, i32* %47, align 4
  %49 = lshr exact i32 %45, 1
  %50 = and i32 %42, 1
  %51 = sub nsw i32 0, %50
  %52 = and i32 %51, -1727483681
  %53 = xor i32 %52, %48
  %54 = xor i32 %53, %49
  store i32 %54, i32* %39, align 4
  %55 = lshr i32 %54, 11
  %56 = xor i32 %55, %54
  %57 = shl i32 %56, 7
  %58 = and i32 %57, -1658038656
  %59 = xor i32 %58, %56
  %60 = shl i32 %59, 15
  %61 = and i32 %60, -272236544
  %62 = xor i32 %61, %59
  %63 = lshr i32 %62, 18
  %64 = xor i32 %63, %62
  %65 = uitofp i32 %64 to float
  %66 = icmp eq i32 %35, 623
  %67 = add nsw i32 %35, 1
  %68 = select i1 %66, i32 0, i32 %67
  %69 = icmp sgt i32 %35, 226
  %.v1.i.i = select i1 %69, i32 -227, i32 397
  %70 = add nsw i32 %.v1.i.i, %35
  store i32 %42, i32* %41, align 4
  %71 = sext i32 %68 to i64
  %72 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %71
  %73 = load i32, i32* %72, align 4
  %74 = and i32 %42, -2147483648
  %75 = and i32 %73, 2147483646
  %76 = or i32 %75, %74
  %77 = sext i32 %70 to i64
  %78 = getelementptr inbounds [624 x i32], [624 x i32]* %1, i64 0, i64 %77
  %79 = load i32, i32* %78, align 4
  %80 = lshr exact i32 %76, 1
  %81 = and i32 %73, 1
  %82 = sub nsw i32 0, %81
  %83 = and i32 %82, -1727483681
  %84 = xor i32 %83, %79
  %85 = xor i32 %84, %80
  store i32 %85, i32* %41, align 4
  %86 = lshr i32 %85, 11
  %87 = xor i32 %86, %85
  %88 = shl i32 %87, 7
  %89 = and i32 %88, -1658038656
  %90 = xor i32 %89, %87
  %91 = shl i32 %90, 15
  %92 = and i32 %91, -272236544
  %93 = xor i32 %92, %90
  %94 = lshr i32 %93, 18
  %95 = xor i32 %94, %93
  %96 = uitofp i32 %95 to float
  %97 = fadd float %65, 1.000000e+00
  %98 = fmul float %97, 0x3DF0000000000000
  %99 = fadd float %96, 1.000000e+00
  %100 = fmul float %99, 0x3DF0000000000000
  %101 = call float @_Z3logf(float %98) #2
  %102 = fmul float %101, -2.000000e+00
  %103 = call float @_Z4fmaxff(float %102, float 0.000000e+00) #2
  %104 = call float @_Z4sqrtf(float %103) #2
  %105 = fmul float %100, 0x401921FB60000000
  %sinPtr = call float @_Z6sincosfPf(float %105, float* %cosPtr)
  %cosVal = load float, float* %cosPtr
  %106 = fmul float %104, %cosVal
  %107 = fmul float %104, %sinPtr
  %108 = fmul float %30, %106
  %109 = fadd float %29, %108
  %110 = call float @_Z3expf(float %109) #2
  %111 = fmul float %9, %110
  %112 = fsub float %111, %12
  %113 = call float @_Z4fmaxff(float %112, float 0.000000e+00) #2
  %114 = fadd float %.0219.i.i, %113
  %115 = fmul float %30, %107
  %116 = fadd float %29, %115
  %117 = call float @_Z3expf(float %116) #2
  %118 = fmul float %9, %117
  %119 = fsub float %118, %12
  %120 = call float @_Z4fmaxff(float %119, float 0.000000e+00) #2
  %121 = fadd float %114, %120
  %122 = add nuw nsw i32 %.021.i.i, 2
  %123 = icmp ult i32 %122, 262144
  br i1 %123, label %32, label %"_ZZZ2mciENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit"

"_ZZZ2mciENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit": ; preds = %32
  %124 = fmul float %121, 0x3ED0000000000000
  %125 = fmul float %6, %_arg_
  %126 = fsub float -0.000000e+00, %125
  %127 = call float @_Z3expf(float %126) #2
  %128 = fmul float %124, %127
  %129 = fsub float %128, %9
  %130 = call float @_Z3expf(float %126) #2
  %131 = fmul float %12, %130
  %132 = fadd float %129, %131
  %133 = getelementptr inbounds float, float addrspace(1)* %_arg_9, i64 %add
  %134 = addrspacecast float addrspace(1)* %133 to float*
  store float %128, float* %134, align 4
  %135 = getelementptr inbounds float, float addrspace(1)* %_arg_12, i64 %add
  %136 = addrspacecast float addrspace(1)* %135 to float*
  store float %132, float* %136, align 4
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %17) #2
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %"_ZZZ2mciENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit"
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !17

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind "vector-variants"="_ZGVdN8uuuuuuuuuuuuuuuuu__ZTSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}
!opencl.compiler.options = !{!3}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{i16 6, i16 14}
!5 = !{void (float, float, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @"_ZTSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"}
!6 = !{i32 0}
!7 = !{i32 0, i32 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0, i32 1, i32 0, i32 0}
!8 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!9 = !{!"float", !"float", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range"}
!10 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!11 = !{void (float, float, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @"_ZGVdN8uuuuuuuuuuuuuuuuu_TSZZ2mciENK3$_0clERN2cl4sycl7handlerEE20monteCarloSimulation"}
!12 = !{i1 true}
!13 = !{i32 1}
!14 = !{null}
!15 = !{i32 8}
!16 = !{i1 false}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.unroll.disable"}
