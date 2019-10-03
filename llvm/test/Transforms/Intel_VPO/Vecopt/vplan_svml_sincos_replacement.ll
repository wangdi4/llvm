; Test to check LLVM-IR vector CG support for OpenCL sincos function. LIT test is reduced from below SYCL program.
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

; RUN: opt -vector-library=SVML -VPlanDriver -vplan-force-vf=8 -enable-vp-value-codegen=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
; RUN: opt -vector-library=SVML -VPlanDriver -vplan-force-vf=8 -enable-vp-value-codegen=true -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPVAL

; CHECK-LABEL: entry
; CHECK:              [[COSPTR_VEC:%.*]] = alloca <8 x float>
; CHECK-NEXT:         [[SINPTR_VEC:%.*]] = alloca <8 x float>
; CHECK-VPVAL-NEXT:   [[COSPTR_BC:%.*]] = bitcast <8 x float>* [[COSPTR_VEC]] to float*
; CHECK-VPVAL-NEXT:   [[COSPTR_GEP:%.*]] = getelementptr float, float* [[COSPTR_BC]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

; CHECK-LABEL: vector.body
; CHECK:       {{.*}} = call <8 x float> @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float> {{.*}}, <8 x float>* [[COSPTR_VEC]], <8 x float>* [[SINPTR_VEC]])
; CHECK:       {{.*}} = load <8 x float>, <8 x float>* [[SINPTR_VEC]]
; CHECK-LLVM:  {{.*}} = load <8 x float>, <8 x float>* [[COSPTR_VEC]], align 4
; CHECK-VPVAL: {{.*}} = call <8 x float> @llvm.masked.gather.v8f32.v8p0f32(<8 x float*> [[COSPTR_GEP]], i32 4, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x float> undef)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
declare float @_Z6sincosfPf(float, float*)

; Function Attrs: nounwind
define void @foo(float addrspace(1)* %uni1, float addrspace(1)* %uni2) {
entry:
  %cosPtr = alloca float
  br label %simd.begin.region

simd.begin.region:                                ; preds = %6
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(float addrspace(1)* %uni1, float addrspace(1)* %uni2), "QUAL.OMP.PRIVATE"(float* %cosPtr), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %index64 = sext i32 %index to i64
  %gep1 = getelementptr inbounds float, float addrspace(1)* %uni1, i64 %index64
  %load1 = load float, float addrspace(1)* %gep1, align 4
  %sinPtr = call float @_Z6sincosfPf(float %load1, float* %cosPtr)
  %cosVal = load float, float* %cosPtr
  %add = fadd float %cosVal, %sinPtr
  %gep2 = getelementptr inbounds float, float addrspace(1)* %uni2, i64 %index64
  store float %add, float addrspace(1)* %gep2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

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
