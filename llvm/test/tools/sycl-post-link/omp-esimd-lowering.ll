; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s 

@b.ascast.priv.__local = internal unnamed_addr addrspace(3) global [16 x float] zeroinitializer, align 32

@a.ascast.priv.__local = internal unnamed_addr addrspace(3) global [16 x i64] zeroinitializer ; INTEL

define dso_local spir_kernel <8 x float>  @test_generic_as_infer(ptr addrspace(1) %ptr) !omp_simd_kernel !0 {
; CHECK: [[CAST:%.*]]  = bitcast ptr addrspace(1) %ptr to ptr addrspace(1)
; CHECK-NEXT: [[LOAD:%.*]] = load <8 x float>, ptr addrspace(1) [[CAST]]
  %cast = bitcast ptr addrspace(1) %ptr to ptr addrspace(1)
  %as_cast = addrspacecast ptr addrspace(1) %cast to ptr addrspace(4)
  %load = load <8 x float>, ptr addrspace(4) %as_cast
  ret <8 x float> %load
}

; TODO: extend math lowering checks coverage

define dso_local spir_kernel <8 x float>  @test_math_lowering(<8 x float> %a0) !omp_simd_kernel !0 {
  %b0 = call fast <8 x float> @llvm.log2.v8f32(<8 x float> %a0)
; CHECK: [[SQRT:%.*]] =  call <8 x float> @llvm.genx.log.v8f32(<8 x float> %a0)
  ret <8 x float> %b0
}

define dso_local spir_kernel <8 x float>  @test_math_no_lowering(<8 x float> %a0) !omp_simd_kernel !0 {
  %b0 = call fast <8 x float> @llvm.round.v8f32(<8 x float> %a0)
; CHECK: [[RND:%.*]] =  call fast <8 x float> @llvm.round.v8f32(<8 x float> %a0)
  %c0 = call fast <8 x float> @llvm.maxnum.v8f32(<8 x float> %a0, <8 x float> zeroinitializer)
; CHECK: call fast <8 x float> @llvm.maxnum.v8f32(<8 x float> %a0, <8 x float> zeroinitializer)
  ret <8 x float> %b0
}

define dso_local spir_kernel <8 x double> @test_sqrt_double(<8 x double> %a0) !omp_simd_kernel !0 {
; CHECK:  [[IEEE_SQRT_DOUBLE:%.*]] = call <8 x double> @llvm.genx.ieee.sqrt.v8f64(<8 x double> %a0)
; CHECK-NEXT: ret <8 x double> [[IEEE_SQRT_DOUBLE]]
  %1 = call <8 x double> @llvm.sqrt.v8f64(<8 x double> %a0)
  ret <8 x double> %1
}

define dso_local spir_kernel <8 x float> @test_sqrt_not_fast(<8 x float> %a0) !omp_simd_kernel !0 {
; CHECK:  [[IEEE_SQRT_FLOAT:%.*]] = call <8 x float> @llvm.genx.ieee.sqrt.v8f32(<8 x float> %a0)
; CHECK-NEXT:  ret <8 x float> [[IEEE_SQRT_FLOAT]]
  %1 = call <8 x float> @llvm.sqrt.v8f32(<8 x float> %a0)
  ret <8 x float> %1
}

define dso_local spir_kernel <8 x float> @test_sqrt_fast(<8 x float> %a0) !omp_simd_kernel !0 {
; CHECK:  [[NATIVE_SQRT:%.*]] = call <8 x float> @llvm.genx.sqrt.v8f32(<8 x float> %a0)
; CHECK-NEXT: ret <8 x float> [[NATIVE_SQRT]]
  %1 = call fast <8 x float> @llvm.sqrt.v8f32(<8 x float> %a0)
  ret <8 x float> %1
}

define dso_local spir_kernel void @test_wg_barrier() !omp_simd_kernel !0 {
; CHECK: call void @llvm.genx.fence(i8 33)
; CHECK-NEXT  call void @llvm.genx.barrier()
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

define dso_local spir_kernel void @test_sg_barrier() !omp_simd_kernel !0 {
; CHECK: call void @llvm.genx.fence(i8 33)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 3, i32 2, i32 784)
  ret void
}

define dso_local spir_kernel void @test_prefetch(ptr addrspace(4) %ptr) !omp_simd_kernel !0 {
; CHECK: [[PTR_CAST:%.*]] = ptrtoint ptr addrspace(4) %ptr to i64
; CHECK: [[PTR1_CAST:%.*]] = bitcast i64 %1 to <1 x i64>
; CHECK-NEXT: call void @llvm.genx.lsc.prefetch.stateless.v1i1.v1i64(<1 x i1> <i1 true>, i8 0, i8 0, i8 0, i16 1, i32 0, i8 3, i8 6, i8 2, i8 0, <1 x i64> [[PTR1_CAST]], i32 0)

  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  tail call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %ptr, i32 0, i32 0)
  ret void
}

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)
declare spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4), i32, i32)

declare <8 x double> @llvm.sqrt.v8f64(<8 x double>)
declare <8 x float> @llvm.sqrt.v8f32(<8 x float>)

declare <8 x float> @llvm.log2.v8f32(<8 x float>)
declare <8 x float> @llvm.round.v8f32(<8 x float>)
declare <8 x float> @llvm.maxnum.v8f32(<8 x float>, <8 x float>)

declare void @llvm.masked.store.v8i32.p0(<8 x i32>, ptr, i32 immarg, <8 x i1>) #1
declare void @llvm.masked.store.v8i32.p3(<8 x i32>, ptr addrspace(3), i32 immarg, <8 x i1>) #1
declare void @llvm.masked.store.v8i64.p3(<8 x i64>, ptr addrspace(3), i32 immarg, <8 x i1>) #1
declare void @llvm.masked.scatter.v8i32.v8p1(<8 x i32>, <8 x ptr addrspace(1)>, i32 immarg, <8 x i1>)
declare void @llvm.masked.scatter.v8i32.v8p4(<8 x i32>, <8 x ptr addrspace(4)>, i32 immarg, <8 x i1>)
declare <8 x i64> @llvm.masked.load.v8i64.p3(ptr addrspace(3), i32 immarg, <8 x i1>, <8 x i64>)

!0 = !{}
