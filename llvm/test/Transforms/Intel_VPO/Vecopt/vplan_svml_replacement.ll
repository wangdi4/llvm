; DPC++ Source-code used to generate the IR,
;
; <sample_input.cpp>
; Compilation command: clang++ --gcc-toolchain=${ICS_GCCBIN}/.. -fsycl sample_input.cpp -fsycl -lOpenCL -lsycl
; #include <CL/sycl.hpp>
; #include <array>
; #include <iostream>
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
;             int ix = global_id[0];
;             int iy = global_id[1];
;             out_accessor[ix] = cl::sycl::exp(in_accessor[ix]);
;             out_accessor[ix] = cl::sycl::sqrt(in_accessor[ix]);
;             out_accessor[ix] = cl::sycl::log(in_accessor[ix]);
;             out_accessor[ix] =
;                 cl::sycl::fmax(in_accessor[ix], out_accessor[ix]);
;             out_accessor[ix] = cl::sycl::mad(
;                 in_accessor[ix], out_accessor[ix], in_accessor[ix]);
;           });
;     });
;   }
; }

;RUN: opt -vector-library=SVML -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring  -vplan-vec -vplan-force-vf=8 -S %s | FileCheck %s
;RUN: opt -vector-library=SVML -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" -vplan-force-vf=8 -S %s | FileCheck %s

;CHECK: call <8 x float> @_Z3expDv8_f(<8 x float> {{.*}})
;CHECK: call <8 x float> @_Z4sqrtDv8_f(<8 x float> {{.*}})
;CHECK: call <8 x float> @_Z3logDv8_f(<8 x float> {{.*}})
;CHECK: call <8 x float> @_Z4fmaxDv8_fS_(<8 x float> {{.*}}, <8 x float> {{.*}})
;CHECK: call <8 x float> @_Z3madDv8_fS_S_(<8 x float> {{.*}}, <8 x float> {{.*}}, <8 x float> {{.*}})


; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: nounwind
declare float @_Z3expf(float) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z4sqrtf(float) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z3logf(float) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z4fmaxff(float, float) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z3madfff(float, float, float) local_unnamed_addr #0

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
define void @"_ZGVdN8uuuuuu_TSZZ4mainENK3$_0clERN2cl4sycl7handlerEE10VecScalMul"(float addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), float addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range")) {
  %7 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %6
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(float addrspace(1)* %0, %"class.cl::sycl::range"* %1, %"class.cl::sycl::range"* %2, float addrspace(1)* %3, %"class.cl::sycl::range"* %4, %"class.cl::sycl::range"* %5) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %8 = sext i32 %index to i64
  %add = add nuw i64 %8, %7
  %sext.i = shl i64 %add, 32
  %9 = ashr exact i64 %sext.i, 32
  %10 = getelementptr inbounds float, float addrspace(1)* %3, i64 %9
  %11 = load float, float addrspace(1)* %10, align 4
  %12 = call float @_Z3expf(float %11)
  %13 = getelementptr inbounds float, float addrspace(1)* %0, i64 %9
  store float %12, float addrspace(1)* %13, align 4
  %14 = load float, float addrspace(1)* %10, align 4
  %15 = call float @_Z4sqrtf(float %14)
  store float %15, float addrspace(1)* %13, align 4
  %16 = load float, float addrspace(1)* %10, align 4
  %17 = call float @_Z3logf(float %16)
  store float %17, float addrspace(1)* %13, align 4
  %18 = load float, float addrspace(1)* %10, align 4
  %19 = call float @_Z4fmaxff(float %18, float %17)
  store float %19, float addrspace(1)* %13, align 4
  %20 = load float, float addrspace(1)* %10, align 4
  %21 = call float @_Z3madfff(float %20, float %19, float %20)
  store float %21, float addrspace(1)* %13, align 4
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
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { readnone }

!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.unroll.disable"}
