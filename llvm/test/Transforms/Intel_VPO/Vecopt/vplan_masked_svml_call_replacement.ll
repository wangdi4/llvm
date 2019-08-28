; RUN: opt -VPlanDriver -vplan-force-vf=4 -vector-library=SVML -S < %s | FileCheck %s

;CHECK-LABEL: vector.body
;CHECK: [[SQRT:%.*]] = call <4 x float> @_Z4sqrtDv4_f(<4 x float> {{.*}})
;CHECK: [[PRED_SQRT:%.*]] = call <4 x float> @_Z4sqrtDv4_f(<4 x float> {{.*}})
;CHECK: [[PRED_EXPF:%.*]] = call <4 x float> @_Z3expDv4_f(<4 x float> {{.*}})
;CHECK: [[PRED_NATIVE_EXPF:%.*]] = call <4 x float> @_Z10native_expDv4_f(<4 x float> {{.*}})


; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }


; Function Attrs: nounwind
declare float @_Z4sqrtf(float) local_unnamed_addr

; Function Attrs: nounwind
declare float @_Z3expf(float) local_unnamed_addr

; Function Attrs: nounwind
declare float @_Z10native_expf(float) local_unnamed_addr

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
define void @"_ZGVdN8uuuuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE10VecScalMul"(float addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), float addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range")) local_unnamed_addr  {
  %9 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %8
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(float addrspace(1)* %0, %"class.cl::sycl::range"* %1, %"class.cl::sycl::range"* %2, %"class.cl::sycl::range"* %3, float addrspace(1)* %4, %"class.cl::sycl::range"* %5, %"class.cl::sycl::range"* %6, %"class.cl::sycl::range"* %7) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %10 = sext i32 %index to i64
  %add = add nuw i64 %10, %9
  %11 = call i64 @_Z13get_global_idj(i32 1)
  %12 = getelementptr inbounds float, float addrspace(1)* %0, i64 %add
  %13 = load float, float addrspace(1)* %12, align 4
  %14 = call float @_Z4sqrtf(float %13)
  %15 = getelementptr inbounds float, float addrspace(1)* %4, i64 %add
  store float %14, float addrspace(1)* %15, align 4
  %16 = urem i64 600, %11
  %17 = icmp eq i64 %16, 1
  br i1 %17, label %18, label %"LP.exit"

18:                                               ; preds = %simd.loop
  %19 = load float, float addrspace(1)* %12, align 4
  %20 = fpext float %19 to double
  %21 = fmul double %20, 1.040000e+01
  %22 = fptrunc double %21 to float
  %23 = call float @_Z4sqrtf(float %22)
  %expResult = call float @_Z3expf(float %22)
  %native_expResult = call float @_Z10native_expf(float %22)
  %fadd = fadd float %23, %expResult
  %fadd_native = fadd float %fadd, %expResult
  store float %fadd_native, float addrspace(1)* %15, align 4
  br label %"LP.exit"

"LP.exit": ; preds = %18, %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %"LP.exit"
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !19

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

!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.disable"}
