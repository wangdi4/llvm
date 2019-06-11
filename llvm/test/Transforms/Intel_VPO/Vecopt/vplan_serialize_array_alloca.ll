; This test checks that we have serialized allocation and serialized load/stores
; from private array-types.

; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring \
; RUN: -VPlanDriver -vplan-force-vf=4 -vplan-serialize-alloca=true 2>&1 | FileCheck %s

; CHECK: [[ORIG_PRIV1:%.*]] = alloca [624 x i32], align 4
; CHECK: [[PRIV1:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV2:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV3:%.*]] = alloca [624 x i32], align 4
; CHECK-NEXT: [[PRIV4:%.*]] = alloca [624 x i32], align 4

; CHECK-LABEL: vector.body
; CHECK: [[PRIV_BASE1:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV1]], i64 0, i64 0

; CHECK: [[PRIV_BASE2:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV2]], i64 0, i64 0

; CHECK: [[PRIV_BASE3:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV3]], i64 0, i64 0

; CHECK: [[PRIV_BASE4:%.*]] = getelementptr [624 x i32], [624 x i32]* [[PRIV4]], i64 0, i64 0

; CHECK-NEXT: [[SV1:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 0
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE1]], align 4
; CHECK-NEXT: [[SV2:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 1
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE2]], align 4
; CHECK-NEXT: [[SV3:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 2
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE3]], align 4
; CHECK-NEXT: [[SV4:%.*]] = extractelement <4 x i32> [[TRUNC:%.*]], i64 3
; CHECK-NEXT: store i32 {{.*}}, i32* [[PRIV_BASE4]], align 4

; CHECK-LABEL: {{VPlannedBB[0-9]+}}:
; CHECK: [[V1:%.*]] = load i32, i32* [[PRIV_BASE1]], align 4
; CHECK-NEXT: [[VEC1:%.*]] = insertelement <4 x i32> undef, i32 [[V1]], i64 0

; CHECK-NEXT: [[V2:%.*]] = load i32, i32* [[PRIV_BASE2]], align 4
; CHECK-NEXT: [[VEC2:%.*]] = insertelement <4 x i32> [[VEC1]], i32 [[V2]], i64 1

; CHECK-NEXT: [[V3:%.*]] = load i32, i32* [[PRIV_BASE3]], align 4
; CHECK-NEXT: [[VEC3:%.*]] = insertelement <4 x i32> [[VEC2]], i32 [[V3]], i64 2

; CHECK-NEXT: [[V4:%.*]] = load i32, i32* [[PRIV_BASE4]], align 4
; CHECK-NEXT: [[VEC4:%.*]] = insertelement <4 x i32> [[VEC3]], i32 [[V4]], i64 3

; CHECK-NEXT: br label {{.*}}

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }


; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

; Function Attrs: nounwind
declare float @_Z4sqrtf(float)

; Function Attrs: nounwind
declare float @_Z3logf(float)

; Function Attrs: nounwind
declare float @_Z4fmaxff(float, float)

; Function Attrs: nounwind
declare float @_Z3cosf(float)

; Function Attrs: nounwind
declare float @_Z3sinf(float) 

; Function Attrs: nounwind
declare float @_Z3expf(float)

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
declare float @_Z6sincosfPf(float, float*)

; Function Attrs: nounwind
define void @"MonteCarloSimulation"(float %_arg_, float %_arg_1, float addrspace(1)* %_arg_2, %"class.cl::sycl::range"* byval %_arg_Range, %"class.cl::sycl::range"* byval %_arg_Offset, float addrspace(1)* %_arg_3, %"class.cl::sycl::range"* byval %_arg_Range4, %"class.cl::sycl::range"* byval %_arg_Offset5, float addrspace(1)* %_arg_6, %"class.cl::sycl::range"* byval %_arg_Range7, %"class.cl::sycl::range"* byval %_arg_Offset8, float addrspace(1)* %_arg_9, %"class.cl::sycl::range"* byval %_arg_Range10, %"class.cl::sycl::range"* byval %_arg_Offset11, float addrspace(1)* %_arg_12, %"class.cl::sycl::range"* byval %_arg_Range13, %"class.cl::sycl::range"* byval %_arg_Offset14) {
  %cosPtr = alloca float
  %1 = alloca [624 x i32], align 4
  %2 = call i64 @_Z13get_global_idj(i32 0)
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
  %16 = call float @_Z4sqrtf(float %6)
  %17 = bitcast [624 x i32]* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %17)
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
  %101 = call float @_Z3logf(float %98)
  %102 = fmul float %101, -2.000000e+00
  %103 = call float @_Z4fmaxff(float %102, float 0.000000e+00)
  %104 = call float @_Z4sqrtf(float %103)
  %105 = fmul float %100, 0x401921FB60000000
  %sinPtr = call float @_Z6sincosfPf(float %105, float* %cosPtr)
  %cosVal = load float, float* %cosPtr
  %106 = fmul float %104, %cosVal
  %107 = fmul float %104, %sinPtr
  %108 = fmul float %30, %106
  %109 = fadd float %29, %108
  %110 = call float @_Z3expf(float %109)
  %111 = fmul float %9, %110
  %112 = fsub float %111, %12
  %113 = call float @_Z4fmaxff(float %112, float 0.000000e+00)
  %114 = fadd float %.0219.i.i, %113
  %115 = fmul float %30, %107
  %116 = fadd float %29, %115
  %117 = call float @_Z3expf(float %116)
  %118 = fmul float %9, %117
  %119 = fsub float %118, %12
  %120 = call float @_Z4fmaxff(float %119, float 0.000000e+00)
  %121 = fadd float %114, %120
  %122 = add nuw nsw i32 %.021.i.i, 2
  %123 = icmp ult i32 %122, 262144
  br i1 %123, label %32, label %"_ZZZ2mciiENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit"

"_ZZZ2mciiENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit": ; preds = %32
  %124 = fmul float %121, 0x3ED0000000000000
  %125 = fmul float %6, %_arg_
  %126 = fsub float -0.000000e+00, %125
  %127 = call float @_Z3expf(float %126)
  %128 = fmul float %124, %127
  %129 = fsub float %128, %9
  %130 = call float @_Z3expf(float %126)
  %131 = fmul float %12, %130
  %132 = fadd float %129, %131
  %133 = getelementptr inbounds float, float addrspace(1)* %_arg_9, i64 %add
  %134 = addrspacecast float addrspace(1)* %133 to float*
  store float %128, float* %134, align 4
  %135 = getelementptr inbounds float, float addrspace(1)* %_arg_12, i64 %add
  %136 = addrspacecast float addrspace(1)* %135 to float*
  store float %132, float* %136, align 4
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %17)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %"_ZZZ2mciiENK3$_0clERN2cl4sycl7handlerEENKUlNS1_2idILi1EEEE_clES5_.exit"
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
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.unroll.disable"}
