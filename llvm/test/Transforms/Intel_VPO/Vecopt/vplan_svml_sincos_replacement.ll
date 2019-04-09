; Compilation command - clang++ --gcc-toolchain=${ICS_GCCBIN}/.. -fsycl input.cpp -fsycl -lOpenCL -lsycl
; #include <CL/sycl.hpp>
; #include <array>
; #include <numeric>
; #include <cmath>
;
; int main() {
;   const size_t array_size = 1024 * 512;
;   const size_t imageW = 800;
;   const size_t imageH = 600;
;   std::array<cl::sycl::cl_float, array_size> in, out;
;   std::iota(begin(in), end(in), 0.2);
;
;   {
;     cl::sycl::queue device_queue;
;     cl::sycl::range<1> n_items{array_size};
;     cl::sycl::buffer<cl::sycl::cl_float, 1> in_buffer(in.data(), n_items);
;     cl::sycl::buffer<cl::sycl::cl_float, 1> out_buffer(out.data(), n_items);
;
;     device_queue.submit([&](cl::sycl::handler &cgh) {
;       constexpr auto sycl_read = cl::sycl::access::mode::read;
;       constexpr auto sycl_write = cl::sycl::access::mode::write;
;
;       auto in_accessor = in_buffer.get_access<sycl_read>(cgh);
;       auto out_accessor = out_buffer.get_access<sycl_write>(cgh);
;
;       cgh.parallel_for<class VecScalMul>(
;           cl::sycl::range<2>{(size_t)imageW, (size_t)imageH},
;           [=](cl::sycl::id<2> global_id) {
;             size_t ix = global_id[0];
;             size_t iy = global_id[1];
;             auto inVal = in_accessor[ix];
;             out_accessor[ix] = cl::sycl::cos(inVal)+cl::sycl::sin(inVal);
;           });
;     });
;   }
; }

;RUN: opt -vector-library=SVML -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring  -VPlanDriver -vplan-force-vf=8 -S %s | FileCheck %s

;CHECK: [[SINPTR_VEC:%.*]] = alloca <8 x float>
;CHECK: {{.*}} = call <8 x float> @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float> {{.*}}, <8 x float>* [[COSPTR_VEC:%.*]], <8 x float>* [[SINPTR_VEC:%.*]])
;CHECK: {{.*}} = load <8 x float>, <8 x float>* [[SINPTR_VEC]]
;CHECK  {{.*}} = load <8 x float>, <8 x float>* [[COSPTR_VEC]], align 4

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
declare float @_Z6sincosfPf(float, float*)

; Function Attrs: nounwind
define void @"_ZGVdN8uuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE10VecScalMul"(float addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, float addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_type_qual !10 !kernel_arg_base_type !9 !vectorized_kernel !14 !no_barrier_path !12 !ocl_recommended_vector_length !15 !vectorized_width !15 !vectorization_dimension !6 !can_unite_workgroups !16 {
  %cosPtr = alloca float
  %7 = call i64 @_Z13get_global_idj(i32 0) #2
  br label %simd.begin.region

simd.begin.region:                                ; preds = %6
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(float addrspace(1)* %0, %"class.cl::sycl::range"* %1, %"class.cl::sycl::range"* %2, float addrspace(1)* %3, %"class.cl::sycl::range"* %4, %"class.cl::sycl::range"* %5), "QUAL.OMP.PRIVATE"(float* %cosPtr), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %8 = sext i32 %index to i64
  %add = add nuw i64 %8, %7
  %9 = getelementptr inbounds float, float addrspace(1)* %0, i64 %add
  %10 = load float, float addrspace(1)* %9, align 4
  %sinPtr = call float @_Z6sincosfPf(float %10, float* %cosPtr)
  %cosVal = load float, float* %cosPtr
  %11 = fadd float %cosVal, %sinPtr
  %12 = getelementptr inbounds float, float addrspace(1)* %3, i64 %add
  store float %11, float addrspace(1)* %12, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
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
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind "vector-variants"="_ZGVdN8uuuuuu__ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE10VecScalMul" }
attributes #1 = { nounwind }

!6 = !{i32 0}
!7 = !{i32 1, i32 0, i32 0, i32 1, i32 0, i32 0}
!8 = !{!"none", !"none", !"none", !"none", !"none", !"none"}
!9 = !{!"float*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"float*", !"class.cl::sycl::range", !"class.cl::sycl::range"}
!10 = !{!"", !"", !"", !"", !"", !""}
!11 = !{void (float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, float addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*)* @"_ZGVdN8uuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE10VecScalMul"}
!12 = !{i1 true}
!13 = !{i32 1}
!14 = !{null}
!15 = !{i32 8}
!16 = !{i1 false}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.unroll.disable"}
