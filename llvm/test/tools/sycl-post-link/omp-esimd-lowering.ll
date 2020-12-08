; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s 

define dso_local spir_kernel void @test_masked_store(<8 x i32>* nonnull %ptr) {
; CHECK: [[LOAD:%.*]] = load <8 x i32>, <8 x i32>* %ptr, align 32
; CHECK-NEXT: [[SEL:%.*]] = select <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, <8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32> [[LOAD]]
; CHECK-NEXT: store <8 x i32> [[SEL]], <8 x i32>* %ptr, align 32
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32>* nonnull %ptr, i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
  ret void
}

define dso_local spir_kernel i32 @test_svm_gather(i32 addrspace(1)* %ptr) {
; CHECK: [[PTR_CAST:%.*]] = bitcast i32 addrspace(1)* %ptr to <1 x i32 addrspace(1)*>
; CHECK-NEXT: [[GATHER:%.*]] = call <1 x i32> @llvm.genx.svm.gather.v1i32.v1i1.v1p1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)*> [[PTR_CAST]], <1 x i32> undef)
; CHECK-NEXT: [[EXTRACT:%.*]] = extractelement <1 x i32> [[GATHER]], i32 0
  %l = load i32, i32 addrspace(1)* %ptr
  ret i32 %l
}

define dso_local spir_kernel void @test_svm_scatter(i32 %val, i32 addrspace(1)* %ptr) {
; CHECK: [[PTR_CAST:%.*]] = bitcast i32 addrspace(1)* %ptr to <1 x i32 addrspace(1)*>
; CHECK-NEXT: [[INS:%.*]] = insertelement <1 x i32> undef, i32 %val, i32 0
; CHECK-NEXT: call void @llvm.genx.svm.scatter.v1i1.v1p1i32.v1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)*> [[PTR_CAST]], <1 x i32> [[INS]])
  store i32 %val, i32 addrspace(1)* %ptr
  ret void
}

define dso_local spir_kernel <8 x float>  @test_generic_as_infer(float addrspace(1)* %ptr) {
; CHECK: [[CAST:%.*]]  = bitcast float addrspace(1)* %ptr to <8 x float> addrspace(1)*
; CHECK-NEXT: [[LOAD:%.*]] = call <8 x float> @llvm.genx.svm.block.ld.unaligned.v8f32.p1v8f32(<8 x float> addrspace(1)* [[CAST]])
  %cast = bitcast float addrspace(1)* %ptr to <8 x float> addrspace(1)*
  %as_cast = addrspacecast <8 x float> addrspace(1)* %cast to <8 x float> addrspace(4)*
  %load = load <8 x float>, <8 x float> addrspace(4)* %as_cast
  ret <8 x float> %load
}


declare void @llvm.masked.store.v8i32.p0v8i32(<8 x i32>, <8 x i32>*, i32 immarg, <8 x i1>) #1
