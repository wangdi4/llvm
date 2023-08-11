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

; RUN: opt -vector-library=SVML -passes=vplan-vec -vplan-force-vf=8 -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare float @_Z6sincosfPf(float, ptr)

; OCL sincos call with simple scalar private cos pointer operand.
define void @foo(ptr addrspace(1) %uni1, ptr addrspace(1) %uni2) {
; CHECK-LABEL: define void @foo
; CHECK:       entry
; CHECK:         [[COSPTR_VEC:%.*]] = alloca <8 x float>
; CHECK-NEXT:    [[COSPTR_GEP:%.*]] = getelementptr float, ptr [[COSPTR_VEC]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

; CHECK:       vector.body
; CHECK:         {{.*}} = call afn <8 x float> @_Z6sincosDv8_fPS_(<8 x float> {{.*}}, ptr [[COSPTR_VEC]])
; CHECK:         {{.*}} = load <8 x float>, ptr [[COSPTR_VEC]], align 4
;
entry:
  %cosPtr = alloca float
  br label %simd.begin.region

simd.begin.region:                                ; preds = %6
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %uni1, float zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %uni2, float zeroinitializer, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %cosPtr, float 0.000000e+00, i32 1), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %index64 = sext i32 %index to i64
  %gep1 = getelementptr inbounds float, ptr addrspace(1) %uni1, i64 %index64
  %load1 = load float, ptr addrspace(1) %gep1, align 4
  %sinPtr = call afn float @_Z6sincosfPf(float %load1, ptr %cosPtr)
  %cosVal = load float, ptr %cosPtr
  %add = fadd float %cosVal, %sinPtr
  %gep2 = getelementptr inbounds float, ptr addrspace(1) %uni2, i64 %index64
  store float %add, ptr addrspace(1) %gep2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; OCL sincos call with ArrayType private cos pointer operand.
define void @foo_array(ptr addrspace(1) %uni1, ptr addrspace(1) %uni2) {
; CHECK-LABEL: define void @foo_array
; CHECK:       entry
; CHECK:         [[COSPTR_VEC:%.*]] = alloca [8 x [2 x float]], align 4
; CHECK:         [[COSPTR_VEC_BASE_ADDR:%.*]] = getelementptr [2 x float], ptr [[COSPTR_VEC]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

; CHECK:       vector.body
; CHECK:         [[COSPTR_GEP:%.*]] = getelementptr inbounds [2 x float], <8 x ptr> [[COSPTR_VEC_BASE_ADDR]], <8 x i64> zeroinitializer, <8 x i64> zeroinitializer
; CHECK-8:       {{.*}} = extractelement <8 x ptr> [[COSPTR_GEP]], i32 {{.*}}
; CHECK-8:       {{.*}} = call float @_Z6sincosfPf(float {{.*}}, ptr {{.*}})
;
entry:
  %cosPtr = alloca [2 x float], align 4
  br label %simd.begin.region

simd.begin.region:                                ; preds = %6
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %uni1, float zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %uni2, float zeroinitializer, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %cosPtr, float zeroinitializer, i32 2), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %index64 = sext i32 %index to i64
  %gep1 = getelementptr inbounds float, ptr addrspace(1) %uni1, i64 %index64
  %load1 = load float, ptr addrspace(1) %gep1, align 4
  %cosPtr.gep = getelementptr inbounds [2 x float], ptr %cosPtr, i64 0, i64 0
  %sinPtr = call float @_Z6sincosfPf(float %load1, ptr nonnull %cosPtr.gep)
  %cosVal = load float, ptr %cosPtr.gep
  %add = fadd float %cosVal, %sinPtr
  %gep2 = getelementptr inbounds float, ptr addrspace(1) %uni2, i64 %index64
  store float %add, ptr addrspace(1) %gep2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
