; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s 

@b.ascast.priv.__local = internal unnamed_addr addrspace(3) global [16 x float] zeroinitializer, align 32

@a.ascast.priv.__local = internal unnamed_addr addrspace(3) global [16 x i64] zeroinitializer ; INTEL

define dso_local spir_kernel void @test_slm_store() {
  store <8 x float> <float 1.160000e+02, float 1.170000e+02, float 1.180000e+02, float 1.190000e+02, float 1.200000e+02, float 1.210000e+02, float 1.220000e+02, float 1.230000e+02>, <8 x float> addrspace(3)* bitcast (float addrspace(3)* getelementptr inbounds ([16 x float], [16 x float] addrspace(3)* @b.ascast.priv.__local, i64 0, i64 8) to <8 x float> addrspace(3)*), align 32
; CHECK: [[ADD:%.*]] = add i32 0, 32
; CHECK-NEXT: call void @llvm.genx.scatter.scaled.v8i1.v8i32.v8f32(<8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 2, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 4, i32 8, i32 12, i32 16, i32 20, i32 24, i32 28>, <8 x float> <float 1.160000e+02, float 1.170000e+02, float 1.180000e+02, float 1.190000e+02, float 1.200000e+02, float 1.210000e+02, float 1.220000e+02, float 1.230000e+02>)

  ret void
}

define dso_local spir_kernel void @test_slm_storevi64() {
  store <8 x i64> <i64 116002, i64 117002, i64 118002, i64 119002, i64 120002, i64 121002, i64 122002, i64 123002>, <8 x i64> addrspace(3)* bitcast (i64 addrspace(3)* getelementptr inbounds ([16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 8) to <8 x i64> addrspace(3)*), align 32
; CHECK: [[ADD:%.*]] = add i32 64, 64
; CHECK-NEXT: [[DATA:%.*]] = bitcast <8 x i64> <i64 116002, i64 117002, i64 118002, i64 119002, i64 120002, i64 121002, i64 122002, i64 123002> to <16 x i32>
; CHECK-NEXT: [[LOW:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[DATA]], i32 0, i32 8, i32 2, i32 0, i32 undef)
; CHECK-NEXT: [[HIGH:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[DATA]], i32 0, i32 8, i32 2, i32 4, i32 undef)
; CHECK-NEXT: [[WL:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> undef, <8 x i32> [[LOW]], i32 0, i32 8, i32 1, i32 0, i32 undef, i1 true)
; CHECK-NEXT: [[WH:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> [[WL]], <8 x i32> [[HIGH]], i32 0, i32 8, i32 1, i32 32, i32 undef, i1 true)
; CHECK-NEXT: call void @llvm.genx.scatter4.scaled.v8i1.v8i32.v16i32(<8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 3, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>, <16 x i32> [[WH]])
  ret void
}

define dso_local spir_kernel void @test_slm_store2(i32 %rem) {
  %idxprom18 = sext i32 %rem to i64
  %arrayidx19 = getelementptr inbounds [16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 %idxprom18
  store i64 1, i64 addrspace(3)* %arrayidx19, align 4
; CHECK: [[TRUNC:%.*]] = trunc i64 %idxprom18 to i32
; CHECK-NEXT: [[MUL:%.*]] = mul i32 [[TRUNC]], 8
; CHECK-NEXT: [[ADD:%.*]] = add i32 64, [[MUL]]
; CHECK-NEXT: [[ONE:%.*]] = bitcast i64 1 to <2 x i32>
; CHECK-NEXT: [[CAST:%.*]] = bitcast i32 [[ADD]] to <1 x i32>
; CHECK-NEXT: [[LOW:%.*]] = extractelement <2 x i32> [[ONE]], i32 0
; CHECK-NEXT: [[HIGH:%.*]] = extractelement <2 x i32> [[ONE]], i32 1
; CHECK-NEXT: [[LOWV:%.*]] = bitcast i32 [[LOW]] to <1 x i32>
; CHECK-NEXT: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 0, <1 x i32> [[CAST]], <1 x i32> [[LOWV]])
; CHECK-NEXT: [[HIGHV:%.*]] = bitcast i32 [[HIGH]] to <1 x i32>
; CHECK-NEXT: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 4, <1 x i32> [[CAST]], <1 x i32> [[HIGHV]])
  ret void
}

define dso_local spir_kernel void @test_slm_load(i32 %rem) {
  %idxprom18 = sext i32 %rem to i64
  %arrayidx19 = getelementptr inbounds [16 x float], [16 x float] addrspace(3)* @b.ascast.priv.__local, i64 0, i64 %idxprom18
  %1 = load float, float addrspace(3)* %arrayidx19, align 4
; CHECK: [[TRUNC:%.*]] = trunc i64 %idxprom18 to i32
; CHECK-NEXT: [[MUL:%.*]] = mul i32 [[TRUNC]], 4
; CHECK-NEXT: [[ADD:%.*]] = add i32 0, [[MUL]]
; CHECK-NEXT: [[BCAS:%.*]] = bitcast i32 [[ADD]] to <1 x i32>
; CHECK-NEXT: [[GATHER:%.*]] = call <1 x float> @llvm.genx.gather.scaled.v1f32.v1i1.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 0, <1 x i32> [[BCAS]], <1 x float> undef)
; CHECK-NEXT: [[EXT:%.*]] = extractelement <1 x float> [[GATHER]], i32 0
  ret void
}

define dso_local spir_kernel void @test_slm_load2(i32 %rem) {
  %idxprom18 = sext i32 %rem to i64
  %arrayidx19 = getelementptr inbounds [16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 %idxprom18
  %1 = load i64, i64 addrspace(3)* %arrayidx19, align 4
; CHECK: [[TRUNC:%.*]] = trunc i64 %idxprom18 to i32
; CHECK-NEXT: [[MUL:%.*]] = mul i32 [[TRUNC]], 8
; CHECK-NEXT: [[ADD:%.*]] = add i32 64, [[MUL]]
; CHECK-NEXT: [[BCAS:%.*]] = bitcast i32 [[ADD]] to <1 x i32>
; CHECK-NEXT: [[G1:%.*]] = call <1 x i32> @llvm.genx.gather.scaled.v1i32.v1i1.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 0, <1 x i32> [[BCAS]], <1 x i32> undef)
; CHECK-NEXT: [[G2:%.*]] = call <1 x i32> @llvm.genx.gather.scaled.v1i32.v1i1.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 4, <1 x i32> [[BCAS]], <1 x i32> undef)
; CHECK-NEXT: [[LOW:%.*]] = extractelement <1 x i32> [[G1]], i32 0
; CHECK-NEXT: [[HIGH:%.*]] = extractelement <1 x i32> [[G2]], i32 0
; CHECK-NEXT: [[INS1:%.*]] = insertelement <2 x i32> undef, i32 [[LOW]], i32 0
; CHECK-NEXT: [[INS2:%.*]] = insertelement <2 x i32> [[INS1]], i32 [[HIGH]], i32 1
; CHECK-NEXT: [[RES:%.*]] = bitcast <2 x i32> [[INS2]] to i64

  ret void
}

define dso_local spir_kernel void @test_slm_loadvi64() {
  %1 = load <8 x i64>, <8 x i64> addrspace(3)* bitcast (i64 addrspace(3)* getelementptr inbounds ([16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 8) to <8 x i64> addrspace(3)*), align 32
; CHECK: [[ADD:%.*]] = add i32 64, 64
; CHECK-NEXT: [[GATHER:%.*]] = call <16 x i32> @llvm.genx.gather4.scaled.v16i32.v8i1.v8i32(<8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 3, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>, <16 x i32> undef)
; CHECK-NEXT: [[LOW:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[GATHER]], i32 0, i32 8, i32 1, i32 0, i32 undef)
; CHECK-NEXT: [[HIGH:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[GATHER]], i32 0, i32 8, i32 1, i32 32, i32 undef)
; CHECK-NEXT: [[WL:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> undef, <8 x i32> [[LOW]], i32 0, i32 8, i32 2, i32 0, i32 undef, i1 true)
; CHECK-NEXT: [[WH:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> [[WL]], <8 x i32> [[HIGH]], i32 0, i32 8, i32 2, i32 4, i32 undef, i1 true)
; CHECK-NEXT: [[CAST:%.*]] = bitcast <16 x i32> [[WH]] to <8 x i64>
  ret void
}

define dso_local spir_kernel void @test_slm_masked_load64(<8 x i64>* nonnull %ptr) {
  %1 = call <8 x i64> @llvm.masked.load.v8i64.p3v8i64(<8 x i64> addrspace(3)* bitcast (i64 addrspace(3)* getelementptr inbounds ([16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 8) to <8 x i64> addrspace(3)*), i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, <8 x i64> <i64 undef, i64 1, i64 2, i64 3, i64 undef, i64 5, i64 6, i64 7>)
; CHECK: [[ADD:%.*]] = add i32 64, 64
; CHECK-NEXT: [[GATHER:%.*]] = call <16 x i32> @llvm.genx.gather4.scaled.v16i32.v8i1.v8i32(<8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, i32 3, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>, <16 x i32> undef)
; CHECK-NEXT: [[CAST:%.*]] = bitcast <8 x i64> <i64 undef, i64 1, i64 2, i64 3, i64 undef, i64 5, i64 6, i64 7> to <16 x i32>
; CHECK-NEXT: [[LOW:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[GATHER]], i32 0, i32 8, i32 1, i32 0, i32 undef)
; CHECK-NEXT: [[HIGH:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[GATHER]], i32 0, i32 8, i32 1, i32 32, i32 undef)
; CHECK-NEXT: [[WL:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.v8i1(<16 x i32> [[CAST]], <8 x i32> [[LOW]], i32 0, i32 8, i32 2, i32 0, i32 undef, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
; CHECK-NEXT: [[WH:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.v8i1(<16 x i32> [[WL]], <8 x i32> [[HIGH]], i32 0, i32 8, i32 2, i32 4, i32 undef, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
; CHECK-NEXT  [[RES:%.*]] = bitcast <16 x i32> [[WH]] to <8 x i64>
  ret void
}

define dso_local spir_kernel void @test_slm_masked_store64(<8 x i64>* nonnull %ptr) {
  call void @llvm.masked.store.v8i64.p3v8i64(<8 x i64> <i64 undef, i64 1, i64 2, i64 3, i64 undef, i64 5, i64 6, i64 7>, <8 x i64> addrspace(3)* bitcast (i64 addrspace(3)* getelementptr inbounds ([16 x i64], [16 x i64] addrspace(3)* @a.ascast.priv.__local, i64 0, i64 8) to <8 x i64> addrspace(3)*), i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
; CHECK: [[ADD:%.*]] = add i32 64, 64
; CHECK-NEXT: [[CAST:%.*]] = bitcast <8 x i64> <i64 undef, i64 1, i64 2, i64 3, i64 undef, i64 5, i64 6, i64 7> to <16 x i32>
; CHECK-NEXT: [[LOW:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[CAST]], i32 0, i32 8, i32 2, i32 0, i32 undef)
; CHECK-NEXT: [[HIGH:%.*]] = call <8 x i32> @llvm.genx.rdregioni.v8i32.v16i32.i32(<16 x i32> [[CAST]], i32 0, i32 8, i32 2, i32 4, i32 undef)
; CHECK-NEXT: [[WL:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> undef, <8 x i32> [[LOW]], i32 0, i32 8, i32 1, i32 0, i32 undef, i1 true)
; CHECK-NEXT: [[WH:%.*]] = call <16 x i32> @llvm.genx.wrregioni.v16i32.v8i32.i32.i1(<16 x i32> [[WL]], <8 x i32> [[HIGH]], i32 0, i32 8, i32 1, i32 32, i32 undef, i1 true)
; CHECK-NEXT: call void @llvm.genx.scatter4.scaled.v8i1.v8i32.v16i32(<8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, i32 3, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>, <16 x i32> [[WH]])
  ret void
}

define dso_local spir_kernel void @test_slm_masked_store(<8 x i32>* nonnull %ptr) {
  call void @llvm.masked.store.v8i32.p3v8i32(<8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32> addrspace(3)* bitcast (float addrspace(3)* getelementptr inbounds ([16 x float], [16 x float] addrspace(3)* @b.ascast.priv.__local, i64 0, i64 8) to <8 x i32> addrspace(3)*), i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
; CHECK: [[ADD:%.*]] = add i32 0, 32
; CHECK-NEXT: call void @llvm.genx.scatter.scaled.v8i1.v8i32.v8i32(<8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, i32 2, i16 0, i32 254, i32 [[ADD]], <8 x i32> <i32 0, i32 4, i32 8, i32 12, i32 16, i32 20, i32 24, i32 28>, <8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>)
  ret void
}

define dso_local spir_kernel void @test_masked_store(<8 x i32>* nonnull %ptr) {
; CHECK: [[LOAD:%.*]] = load <8 x i32>, <8 x i32>* %ptr, align 32
; CHECK-NEXT: [[SEL:%.*]] = select <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, <8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32> [[LOAD]]
; CHECK-NEXT: store <8 x i32> [[SEL]], <8 x i32>* %ptr, align 32
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32>* nonnull %ptr, i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
  ret void
}

define dso_local spir_kernel void @test_masked_scatter(<8 x i32 addrspace(1)*> %ptr) {
; CHECK: call void @llvm.genx.svm.scatter.v8i1.v8p1i32.v8i32(<8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 0, <8 x i32 addrspace(1)*> %ptr, <8 x i32> zeroinitializer)
  call void @llvm.masked.scatter.v8i32.v8p1i32(<8 x i32> zeroinitializer, <8 x i32 addrspace(1)*> %ptr, i32 4, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  ret void
}

define dso_local spir_kernel void @test_generic_cast(<8 x i32 addrspace(1)*> %ptr) {
; CHECK: call void @llvm.genx.svm.scatter.v8i1.v8p1i32.v8i32(<8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 0, <8 x i32 addrspace(1)*> %ptr, <8 x i32> zeroinitializer)
  %ptr4 = addrspacecast <8 x i32 addrspace(1)*> %ptr to <8 x i32 addrspace(4)*>
  call void @llvm.masked.scatter.v8i32.v8p4i32(<8 x i32> zeroinitializer, <8 x i32 addrspace(4)*> %ptr4, i32 4, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
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

define dso_local spir_kernel i32 addrspace(1)* @test_svm_gather_ptrv(i32 addrspace(1)* addrspace(1)* %ptr) {
; CHECK: [[PTR_CAST:%.*]] = bitcast i32 addrspace(1)* addrspace(1)* %ptr to <1 x i32 addrspace(1)* addrspace(1)*>
; CHECK-NEXT: [[GATHER:%.*]] = call <1 x i32 addrspace(1)*> @llvm.genx.svm.gather.v1p1i32.v1i1.v1p1p1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)* addrspace(1)*> [[PTR_CAST]], <1 x i32 addrspace(1)*> undef)
; CHECK-NEXT: [[EXTRACT:%.*]] = extractelement <1 x i32 addrspace(1)*> [[GATHER]], i32 0
  %l = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(1)* %ptr
  ret i32 addrspace(1)* %l
}

define dso_local spir_kernel void @test_svm_scatter_ptrv(i32 addrspace(1)* %val, i32 addrspace(1)* addrspace(1)* %ptr) {
; CHECK: [[PTR_CAST:%.*]] = bitcast i32 addrspace(1)* addrspace(1)* %ptr to <1 x i32 addrspace(1)* addrspace(1)*>
; CHECK-NEXT: [[INS:%.*]] = insertelement <1 x i32 addrspace(1)*> undef, i32 addrspace(1)* %val, i32 0
; CHECK-NEXT: call void @llvm.genx.svm.scatter.v1i1.v1p1p1i32.v1p1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)* addrspace(1)*> [[PTR_CAST]], <1 x i32 addrspace(1)*> [[INS]])
  store i32 addrspace(1)* %val, i32 addrspace(1)* addrspace(1)* %ptr
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

define dso_local spir_kernel <8 x float>  @test_math_lowering(<8 x float> %a0) {
  %b0 = call fast <8 x float> @llvm.log2.v8f32(<8 x float> %a0)
; CHECK: [[SQRT:%.*]] =  call <8 x float> @llvm.genx.log.v8f32(<8 x float> %a0)
  ret <8 x float> %b0
}

define dso_local spir_kernel <8 x float>  @test_math_no_lowering(<8 x float> %a0) {
  %b0 = call fast <8 x float> @llvm.round.v8f32(<8 x float> %a0)
; CHECK: [[RND:%.*]] =  call fast <8 x float> @llvm.round.v8f32(<8 x float> %a0)
  ret <8 x float> %b0
}

define dso_local spir_kernel <8 x double> @test_sqrt_double(<8 x double> %a0) {
; CHECK:  [[IEEE_SQRT_DOUBLE:%.*]] = call <8 x double> @llvm.genx.ieee.sqrt.v8f64(<8 x double> %a0)
; CHECK-NEXT: ret <8 x double> [[IEEE_SQRT_DOUBLE]]
  %1 = call <8 x double> @llvm.sqrt.v8f64(<8 x double> %a0)
  ret <8 x double> %1
}

define dso_local spir_kernel <8 x float> @test_sqrt_not_fast(<8 x float> %a0) {
; CHECK:  [[IEEE_SQRT_FLOAT:%.*]] = call <8 x float> @llvm.genx.ieee.sqrt.v8f32(<8 x float> %a0)
; CHECK-NEXT:  ret <8 x float> [[IEEE_SQRT_FLOAT]]
  %1 = call <8 x float> @llvm.sqrt.v8f32(<8 x float> %a0)
  ret <8 x float> %1
}

define dso_local spir_kernel <8 x float> @test_sqrt_fast(<8 x float> %a0) {
; CHECK:  [[NATIVE_SQRT:%.*]] = call <8 x float> @llvm.genx.sqrt.v8f32(<8 x float> %a0)
; CHECK-NEXT: ret <8 x float> [[NATIVE_SQRT]]
  %1 = call fast <8 x float> @llvm.sqrt.v8f32(<8 x float> %a0)
  ret <8 x float> %1
}

define dso_local spir_kernel void @test_wg_barrier() {
; CHECK: call void @llvm.genx.fence(i8 33)
; CHECK-NEXT  call void @llvm.genx.barrier()
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

define dso_local spir_kernel void @test_sg_barrier() {
; CHECK: call void @llvm.genx.fence(i8 33)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 3, i32 2, i32 784)
  ret void
}

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)

declare <8 x double> @llvm.sqrt.v8f64(<8 x double>)
declare <8 x float> @llvm.sqrt.v8f32(<8 x float>)

declare <8 x float> @llvm.log2.v8f32(<8 x float>)
declare <8 x float> @llvm.round.v8f32(<8 x float>)

declare void @llvm.masked.store.v8i32.p0v8i32(<8 x i32>, <8 x i32>*, i32 immarg, <8 x i1>) #1
declare void @llvm.masked.store.v8i32.p3v8i32(<8 x i32>, <8 x i32> addrspace(3)*, i32 immarg, <8 x i1>) #1
declare void @llvm.masked.store.v8i64.p3v8i64(<8 x i64>, <8 x i64> addrspace(3)*, i32 immarg, <8 x i1>) #1
declare void @llvm.masked.scatter.v8i32.v8p1i32(<8 x i32>, <8 x i32 addrspace(1)*>, i32 immarg, <8 x i1>)
declare void @llvm.masked.scatter.v8i32.v8p4i32(<8 x i32>, <8 x i32 addrspace(4)*>, i32 immarg, <8 x i1>)
declare <8 x i64> @llvm.masked.load.v8i64.p3v8i64(<8 x i64> addrspace(3)*, i32 immarg, <8 x i1>, <8 x i64>)
