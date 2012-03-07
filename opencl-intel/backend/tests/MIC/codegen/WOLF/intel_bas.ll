; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

@const_vector_msb = private constant [16 x i32] [i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648], align 64

declare void @__basScalar_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float, float, i32) nounwind

declare i64 @get_global_id(i32)

define float @_Z4sqrtf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_sqrtf1(float %x) nounwind readnone
  ret float %call
}

define float @_Z3logf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_logf1(float %x) nounwind readnone
  ret float %call
}

define float @_Z4fabsf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_fabsf1(float %x) nounwind readnone
  ret float %call
}

define float @_Z3expf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_expf1(float %x) nounwind readnone
  ret float %call
}

declare void @__basVectorized_original(<4 x float> addrspace(1)* noalias nocapture, <4 x float> addrspace(1)* noalias nocapture, <4 x float> addrspace(1)* noalias nocapture, <4 x float> addrspace(1)* noalias nocapture, <4 x float> addrspace(1)* noalias nocapture, float, float, i32) nounwind

define <4 x float> @_Z4sqrtDv4_f(<4 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_sqrtf4(<4 x float> %x) nounwind readnone
  ret <4 x float> %call
}

define <4 x float> @_Z3logDv4_f(<4 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_logf4(<4 x float> %x) nounwind readnone
  ret <4 x float> %call
}

define <4 x float> @_Z4fabsDv4_f(<4 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_fabsf4(<4 x float> %x) nounwind readnone
  ret <4 x float> %call
}

define <4 x float> @_Z3expDv4_f(<4 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %x) nounwind readnone
  ret <4 x float> %call
}

define <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float> %a, <4 x float> %b, <4 x i32> %c) nounwind readnone {
entry:
  %0 = bitcast <4 x float> %a to <4 x i32>
  %1 = bitcast <4 x float> %b to <4 x i32>
  %tmp2.i = shufflevector <4 x i32> %0, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i = shufflevector <4 x i32> %1, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i = shufflevector <4 x i32> %c, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i = bitcast <8 x i64> %tmp1.i.i to <16 x i32>
  %2 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i, <16 x i32> %conv2.i.i.i) nounwind
  %3 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %2, <16 x i32> %conv2.i.i.i) nounwind
  %4 = tail call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i, i16 %3, <16 x i32> %tmp2.i, <16 x i32> %tmp2.i) nounwind
  %tmp16.i = shufflevector <16 x i32> %4, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %5 = bitcast <4 x i32> %tmp16.i to <4 x float>
  ret <4 x float> %5
}

declare void @____Vectorized_.basScalar_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, float, float, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

define <16 x float> @_Z4sqrtDv16_f(<16 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_sqrtf16(<16 x float> %x) nounwind readnone
  ret <16 x float> %call
}

define <16 x float> @_Z3logDv16_f(<16 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_logf16(<16 x float> %x) nounwind readnone
  ret <16 x float> %call
}

define <16 x float> @_Z4fabsDv16_f(<16 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_fabsf16(<16 x float> %x) nounwind readnone
  ret <16 x float> %call
}

define <16 x float> @_Z3expDv16_f(<16 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %x) nounwind readnone
  ret <16 x float> %call
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__basScalar_separated_args(float addrspace(1)* noalias nocapture %S, float addrspace(1)* noalias nocapture %K, float addrspace(1)* noalias nocapture %T, float addrspace(1)* noalias nocapture %callOutput, float addrspace(1)* noalias nocapture %putOutput, float %r, float %v, i32 %optionsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = fmul float %v, %v
  %1 = fmul float %0, 5.000000e-01
  %2 = fadd float %1, %r
  %3 = fsub float -0.000000e+00, %r
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %9 = getelementptr inbounds float addrspace(1)* %S, i64 %8
  %10 = load float addrspace(1)* %9, align 4
  %11 = getelementptr inbounds float addrspace(1)* %K, i64 %8
  %12 = load float addrspace(1)* %11, align 4
  %13 = getelementptr inbounds float addrspace(1)* %T, i64 %8
  %14 = load float addrspace(1)* %13, align 4
  %call.i = tail call x86_svmlcc float @__ocl_svml_b1_sqrtf1(float %14) nounwind readnone
  %15 = fmul float %call.i, %v
  %16 = fdiv float %10, %12
  %call.i1 = tail call x86_svmlcc float @__ocl_svml_b1_logf1(float %16) nounwind readnone
  %17 = fmul float %2, %14
  %18 = fadd float %call.i1, %17
  %19 = fdiv float %18, %15
  %20 = fsub float %19, %15
  %call.i2 = tail call x86_svmlcc float @__ocl_svml_b1_fabsf1(float %19) nounwind readnone
  %21 = fmul float %call.i2, 0x3FCDA67120000000
  %22 = fadd float %21, 1.000000e+00
  %23 = fdiv float 1.000000e+00, %22
  %24 = fmul float %call.i2, -5.000000e-01
  %25 = fmul float %24, %call.i2
  %call.i3 = tail call x86_svmlcc float @__ocl_svml_b1_expf1(float %25) nounwind readnone
  %26 = fmul float %call.i3, 0x3FD9884540000000
  %27 = fmul float %26, %23
  %28 = fmul float %23, 0x3FF548CDE0000000
  %29 = fadd float %28, 0xBFFD23DD40000000
  %30 = fmul float %23, %29
  %31 = fadd float %30, 0x3FFC80EF00000000
  %32 = fmul float %23, %31
  %33 = fadd float %32, 0xBFD6D1F0E0000000
  %34 = fmul float %23, %33
  %35 = fadd float %34, 0x3FD470BF40000000
  %36 = fmul float %27, %35
  %37 = fcmp olt float %19, 0.000000e+00
  %38 = fsub float 1.000000e+00, %36
  %. = select i1 %37, float %36, float %38
  %.1 = select i1 %37, float %38, float %36
  %call.i4 = tail call x86_svmlcc float @__ocl_svml_b1_fabsf1(float %20) nounwind readnone
  %39 = fmul float %call.i4, 0x3FCDA67120000000
  %40 = fadd float %39, 1.000000e+00
  %41 = fdiv float 1.000000e+00, %40
  %42 = fmul float %call.i4, -5.000000e-01
  %43 = fmul float %42, %call.i4
  %call.i5 = tail call x86_svmlcc float @__ocl_svml_b1_expf1(float %43) nounwind readnone
  %44 = fmul float %call.i5, 0x3FD9884540000000
  %45 = fmul float %44, %41
  %46 = fmul float %41, 0x3FF548CDE0000000
  %47 = fadd float %46, 0xBFFD23DD40000000
  %48 = fmul float %41, %47
  %49 = fadd float %48, 0x3FFC80EF00000000
  %50 = fmul float %41, %49
  %51 = fadd float %50, 0xBFD6D1F0E0000000
  %52 = fmul float %41, %51
  %53 = fadd float %52, 0x3FD470BF40000000
  %54 = fmul float %45, %53
  %55 = fcmp olt float %20, 0.000000e+00
  %56 = fsub float 1.000000e+00, %54
  %.2 = select i1 %55, float %54, float %56
  %.3 = select i1 %55, float %56, float %54
  %57 = fmul float %14, %3
  %call.i6 = tail call x86_svmlcc float @__ocl_svml_b1_expf1(float %57) nounwind readnone
  %58 = fmul float %12, %call.i6
  %59 = fmul float %10, %.
  %60 = fmul float %58, %.2
  %61 = fsub float %59, %60
  %62 = fmul float %58, %.3
  %63 = fmul float %10, %.1
  %64 = fsub float %62, %63
  %65 = getelementptr inbounds float addrspace(1)* %callOutput, i64 %8
  store float %61, float addrspace(1)* %65, align 4
  %66 = getelementptr inbounds float addrspace(1)* %putOutput, i64 %8
  store float %64, float addrspace(1)* %66, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB4

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB4:                                          ; preds = %SyncBB
  ret void
}

define void @__basVectorized_separated_args(<4 x float> addrspace(1)* noalias nocapture %S, <4 x float> addrspace(1)* noalias nocapture %K, <4 x float> addrspace(1)* noalias nocapture %T, <4 x float> addrspace(1)* noalias nocapture %callOutput, <4 x float> addrspace(1)* noalias nocapture %putOutput, float %r, float %v, i32 %optionsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = zext i32 %optionsPerItem to i64
  %1 = insertelement <4 x float> undef, float %v, i32 0
  %2 = shufflevector <4 x float> %1, <4 x float> undef, <4 x i32> zeroinitializer
  %3 = fmul float %v, %v
  %4 = fmul float %3, 5.000000e-01
  %5 = fadd float %4, %r
  %6 = insertelement <4 x float> undef, float %5, i32 0
  %7 = shufflevector <4 x float> %6, <4 x float> undef, <4 x i32> zeroinitializer
  %8 = fsub float -0.000000e+00, %r
  %9 = insertelement <4 x float> undef, float %8, i32 0
  %10 = shufflevector <4 x float> %9, <4 x float> undef, <4 x i32> zeroinitializer
  br label %SyncBB8

SyncBB8:                                          ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %11 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = add i64 %12, %14
  %16 = mul i64 %15, %0
  %17 = add i64 %16, %0
  %18 = icmp ult i64 %16, %17
  br i1 %18, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB8, %bb.nph
  %indvar = phi i64 [ %indvar.next, %bb.nph ], [ 0, %SyncBB8 ]
  %tmp3 = add i64 %16, %indvar
  %scevgep = getelementptr <4 x float> addrspace(1)* %S, i64 %tmp3
  %scevgep4 = getelementptr <4 x float> addrspace(1)* %K, i64 %tmp3
  %scevgep5 = getelementptr <4 x float> addrspace(1)* %T, i64 %tmp3
  %scevgep6 = getelementptr <4 x float> addrspace(1)* %callOutput, i64 %tmp3
  %scevgep7 = getelementptr <4 x float> addrspace(1)* %putOutput, i64 %tmp3
  %19 = load <4 x float> addrspace(1)* %scevgep, align 16
  %20 = load <4 x float> addrspace(1)* %scevgep4, align 16
  %21 = load <4 x float> addrspace(1)* %scevgep5, align 16
  %call.i = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_sqrtf4(<4 x float> %21) nounwind readnone
  %22 = fmul <4 x float> %2, %call.i
  %23 = fdiv <4 x float> %19, %20
  %call.i1 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_logf4(<4 x float> %23) nounwind readnone
  %24 = fmul <4 x float> %7, %21
  %25 = fadd <4 x float> %call.i1, %24
  %26 = fdiv <4 x float> %25, %22
  %27 = fsub <4 x float> %26, %22
  %call.i2 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_fabsf4(<4 x float> %26) nounwind readnone
  %28 = fmul <4 x float> %call.i2, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %29 = fadd <4 x float> %28, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %30 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %29
  %31 = fmul <4 x float> %call.i2, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %32 = fmul <4 x float> %31, %call.i2
  %call.i3 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %32) nounwind readnone
  %33 = fmul <4 x float> %call.i3, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %34 = fmul <4 x float> %33, %30
  %35 = fmul <4 x float> %30, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %36 = fadd <4 x float> %35, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %37 = fmul <4 x float> %30, %36
  %38 = fadd <4 x float> %37, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %39 = fmul <4 x float> %30, %38
  %40 = fadd <4 x float> %39, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %41 = fmul <4 x float> %30, %40
  %42 = fadd <4 x float> %41, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %43 = fmul <4 x float> %34, %42
  %44 = fsub <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %43
  %45 = fcmp olt <4 x float> %26, zeroinitializer
  %46 = sext <4 x i1> %45 to <4 x i32>
  %47 = bitcast <4 x float> %43 to <4 x i32>
  %48 = bitcast <4 x float> %44 to <4 x i32>
  %tmp2.i.i = shufflevector <4 x i32> %47, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i = shufflevector <4 x i32> %48, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i = shufflevector <4 x i32> %46, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i = bitcast <8 x i64> %tmp1.i.i.i to <16 x i32>
  %49 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i, <16 x i32> %conv2.i.i.i.i) nounwind
  %50 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %49, <16 x i32> %conv2.i.i.i.i) nounwind
  %51 = tail call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i, i16 %50, <16 x i32> %tmp2.i.i, <16 x i32> %tmp2.i.i) nounwind
  %tmp16.i.i = shufflevector <16 x i32> %51, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %52 = bitcast <4 x i32> %tmp16.i.i to <4 x float>
  %53 = bitcast <4 x float> %44 to <4 x i32>
  %54 = bitcast <4 x float> %43 to <4 x i32>
  %tmp2.i.i4 = shufflevector <4 x i32> %53, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i5 = shufflevector <4 x i32> %54, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i6 = shufflevector <4 x i32> %46, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i7 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i8 = bitcast <8 x i64> %tmp1.i.i.i7 to <16 x i32>
  %55 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i6, <16 x i32> %conv2.i.i.i.i8) nounwind
  %56 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %55, <16 x i32> %conv2.i.i.i.i8) nounwind
  %57 = tail call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i5, i16 %56, <16 x i32> %tmp2.i.i4, <16 x i32> %tmp2.i.i4) nounwind
  %tmp16.i.i9 = shufflevector <16 x i32> %57, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %58 = bitcast <4 x i32> %tmp16.i.i9 to <4 x float>
  %call.i10 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_fabsf4(<4 x float> %27) nounwind readnone
  %59 = fmul <4 x float> %call.i10, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %60 = fadd <4 x float> %59, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %61 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %60
  %62 = fmul <4 x float> %call.i10, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %63 = fmul <4 x float> %62, %call.i10
  %call.i11 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %63) nounwind readnone
  %64 = fmul <4 x float> %call.i11, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %65 = fmul <4 x float> %64, %61
  %66 = fmul <4 x float> %61, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %67 = fadd <4 x float> %66, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %68 = fmul <4 x float> %61, %67
  %69 = fadd <4 x float> %68, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %70 = fmul <4 x float> %61, %69
  %71 = fadd <4 x float> %70, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %72 = fmul <4 x float> %61, %71
  %73 = fadd <4 x float> %72, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %74 = fmul <4 x float> %65, %73
  %75 = fsub <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %74
  %76 = fcmp olt <4 x float> %27, zeroinitializer
  %77 = sext <4 x i1> %76 to <4 x i32>
  %78 = bitcast <4 x float> %74 to <4 x i32>
  %79 = bitcast <4 x float> %75 to <4 x i32>
  %tmp2.i.i12 = shufflevector <4 x i32> %78, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i13 = shufflevector <4 x i32> %79, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i14 = shufflevector <4 x i32> %77, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i15 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i16 = bitcast <8 x i64> %tmp1.i.i.i15 to <16 x i32>
  %80 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i14, <16 x i32> %conv2.i.i.i.i16) nounwind
  %81 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %80, <16 x i32> %conv2.i.i.i.i16) nounwind
  %82 = tail call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i13, i16 %81, <16 x i32> %tmp2.i.i12, <16 x i32> %tmp2.i.i12) nounwind
  %tmp16.i.i17 = shufflevector <16 x i32> %82, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %83 = bitcast <4 x i32> %tmp16.i.i17 to <4 x float>
  %84 = bitcast <4 x float> %75 to <4 x i32>
  %85 = bitcast <4 x float> %74 to <4 x i32>
  %tmp2.i.i18 = shufflevector <4 x i32> %84, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i19 = shufflevector <4 x i32> %85, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i20 = shufflevector <4 x i32> %77, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i21 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i22 = bitcast <8 x i64> %tmp1.i.i.i21 to <16 x i32>
  %86 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i20, <16 x i32> %conv2.i.i.i.i22) nounwind
  %87 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %86, <16 x i32> %conv2.i.i.i.i22) nounwind
  %88 = tail call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i19, i16 %87, <16 x i32> %tmp2.i.i18, <16 x i32> %tmp2.i.i18) nounwind
  %tmp16.i.i23 = shufflevector <16 x i32> %88, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %89 = bitcast <4 x i32> %tmp16.i.i23 to <4 x float>
  %90 = fmul <4 x float> %10, %21
  %call.i24 = tail call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %90) nounwind readnone
  %91 = fmul <4 x float> %20, %call.i24
  %92 = fmul <4 x float> %19, %58
  %93 = fmul <4 x float> %91, %89
  %94 = fsub <4 x float> %92, %93
  %95 = fmul <4 x float> %91, %83
  %96 = fmul <4 x float> %19, %52
  %97 = fsub <4 x float> %95, %96
  store <4 x float> %94, <4 x float> addrspace(1)* %scevgep6, align 16
  store <4 x float> %97, <4 x float> addrspace(1)* %scevgep7, align 16
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %0
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB8

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.basScalar_separated_args(float addrspace(1)* noalias nocapture %S, float addrspace(1)* noalias nocapture %K, float addrspace(1)* noalias nocapture %T, float addrspace(1)* noalias nocapture %callOutput, float addrspace(1)* noalias nocapture %putOutput, float %r, float %v, i32 %optionsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %temp = insertelement <16 x float> undef, float %v, i32 0
  %vector = shufflevector <16 x float> %temp, <16 x float> undef, <16 x i32> zeroinitializer
  %0 = fmul float %v, %v
  %1 = fmul float %0, 5.000000e-01
  %2 = fadd float %1, %r
  %temp18 = insertelement <16 x float> undef, float %2, i32 0
  %vector19 = shufflevector <16 x float> %temp18, <16 x float> undef, <16 x i32> zeroinitializer
  %3 = fsub float -0.000000e+00, %r
  %temp24 = insertelement <16 x float> undef, float %3, i32 0
  %vector25 = shufflevector <16 x float> %temp24, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %9 = getelementptr inbounds float addrspace(1)* %S, i64 %8
  %ptrTypeCast = bitcast float addrspace(1)* %9 to <16 x float> addrspace(1)*
  %10 = load <16 x float> addrspace(1)* %ptrTypeCast, align 4
  %11 = getelementptr inbounds float addrspace(1)* %K, i64 %8
  %ptrTypeCast16 = bitcast float addrspace(1)* %11 to <16 x float> addrspace(1)*
  %12 = load <16 x float> addrspace(1)* %ptrTypeCast16, align 4
  %13 = getelementptr inbounds float addrspace(1)* %T, i64 %8
  %ptrTypeCast17 = bitcast float addrspace(1)* %13 to <16 x float> addrspace(1)*
  %14 = load <16 x float> addrspace(1)* %ptrTypeCast17, align 4
  %call.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_sqrtf16(<16 x float> %14) nounwind readnone
  %15 = fmul <16 x float> %call.i, %vector
  %16 = fdiv <16 x float> %10, %12
  %call.i1 = call x86_svmlcc <16 x float> @__ocl_svml_b1_logf16(<16 x float> %16) nounwind readnone
  %17 = fmul <16 x float> %vector19, %14
  %18 = fadd <16 x float> %call.i1, %17
  %19 = fdiv <16 x float> %18, %15
  %20 = fsub <16 x float> %19, %15
  %call.i2 = call x86_svmlcc <16 x float> @__ocl_svml_b1_fabsf16(<16 x float> %19) nounwind readnone
  %21 = fmul <16 x float> %call.i2, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %22 = fadd <16 x float> %21, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %23 = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %22
  %24 = fmul <16 x float> %call.i2, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %25 = fmul <16 x float> %24, %call.i2
  %call.i3 = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %25) nounwind readnone
  %26 = fmul <16 x float> %call.i3, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %27 = fmul <16 x float> %26, %23
  %28 = fmul <16 x float> %23, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %29 = fadd <16 x float> %28, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %30 = fmul <16 x float> %23, %29
  %31 = fadd <16 x float> %30, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %32 = fmul <16 x float> %23, %31
  %33 = fadd <16 x float> %32, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %34 = fmul <16 x float> %23, %33
  %35 = fadd <16 x float> %34, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %36 = fmul <16 x float> %27, %35
  %37 = fcmp olt <16 x float> %19, zeroinitializer
  %38 = fsub <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %36
  %.20 = select <16 x i1> %37, <16 x float> %36, <16 x float> %38
  %.121 = select <16 x i1> %37, <16 x float> %38, <16 x float> %36
  %call.i4 = call x86_svmlcc <16 x float> @__ocl_svml_b1_fabsf16(<16 x float> %20) nounwind readnone
  %39 = fmul <16 x float> %call.i4, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %40 = fadd <16 x float> %39, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %41 = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %40
  %42 = fmul <16 x float> %call.i4, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %43 = fmul <16 x float> %42, %call.i4
  %call.i5 = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %43) nounwind readnone
  %44 = fmul <16 x float> %call.i5, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %45 = fmul <16 x float> %44, %41
  %46 = fmul <16 x float> %41, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %47 = fadd <16 x float> %46, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %48 = fmul <16 x float> %41, %47
  %49 = fadd <16 x float> %48, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %50 = fmul <16 x float> %41, %49
  %51 = fadd <16 x float> %50, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %52 = fmul <16 x float> %41, %51
  %53 = fadd <16 x float> %52, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %54 = fmul <16 x float> %45, %53
  %55 = fcmp olt <16 x float> %20, zeroinitializer
  %56 = fsub <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %54
  %.222 = select <16 x i1> %55, <16 x float> %54, <16 x float> %56
  %.323 = select <16 x i1> %55, <16 x float> %56, <16 x float> %54
  %57 = fmul <16 x float> %14, %vector25
  %call.i6 = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %57) nounwind readnone
  %58 = fmul <16 x float> %12, %call.i6
  %59 = fmul <16 x float> %10, %.20
  %60 = fmul <16 x float> %58, %.222
  %61 = fsub <16 x float> %59, %60
  %62 = fmul <16 x float> %58, %.323
  %63 = fmul <16 x float> %10, %.121
  %64 = fsub <16 x float> %62, %63
  %65 = getelementptr inbounds float addrspace(1)* %callOutput, i64 %8
  %ptrTypeCast26 = bitcast float addrspace(1)* %65 to <16 x float> addrspace(1)*
  store <16 x float> %61, <16 x float> addrspace(1)* %ptrTypeCast26, align 4
  %66 = getelementptr inbounds float addrspace(1)* %putOutput, i64 %8
  %ptrTypeCast27 = bitcast float addrspace(1)* %66 to <16 x float> addrspace(1)*
  store <16 x float> %64, <16 x float> addrspace(1)* %ptrTypeCast27, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB28

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB28:                                         ; preds = %SyncBB
  ret void
}

declare float @__ocl_svml_b1_sqrtf1(float) readnone

declare float @__ocl_svml_b1_logf1(float) readnone

declare float @__ocl_svml_b1_fabsf1(float) readnone

declare float @__ocl_svml_b1_expf1(float) readnone

declare <4 x float> @__ocl_svml_b1_sqrtf4(<4 x float>) readnone

declare <4 x float> @__ocl_svml_b1_logf4(<4 x float>) readnone

declare <4 x float> @__ocl_svml_b1_fabsf4(<4 x float>) readnone

declare <4 x float> @__ocl_svml_b1_expf4(<4 x float>) readnone

declare i16 @llvm.x86.mic.cmpeq.pi(<16 x i32>, <16 x i32>) nounwind readnone

declare <16 x i32> @llvm.x86.mic.and.pi(<16 x i32>, <16 x i32>) nounwind readnone

declare <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32>, i16, <16 x i32>, <16 x i32>) nounwind readnone

declare <16 x float> @__ocl_svml_b1_sqrtf16(<16 x float>) readnone

declare <16 x float> @__ocl_svml_b1_logf16(<16 x float>) readnone

declare <16 x float> @__ocl_svml_b1_fabsf16(<16 x float>) readnone

declare <16 x float> @__ocl_svml_b1_expf16(<16 x float>) readnone

define void @basVectorized(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to <4 x float> addrspace(1)**
  %7 = load <4 x float> addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to <4 x float> addrspace(1)**
  %10 = load <4 x float> addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to <4 x float> addrspace(1)**
  %13 = load <4 x float> addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 44
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 104
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = zext i32 %22 to i64
  %33 = insertelement <4 x float> undef, float %19, i32 0
  %34 = shufflevector <4 x float> %33, <4 x float> undef, <4 x i32> zeroinitializer
  %35 = fmul float %19, %19
  %36 = fmul float %35, 5.000000e-01
  %37 = fadd float %36, %16
  %38 = insertelement <4 x float> undef, float %37, i32 0
  %39 = shufflevector <4 x float> %38, <4 x float> undef, <4 x i32> zeroinitializer
  %40 = fsub float -0.000000e+00, %16
  %41 = insertelement <4 x float> undef, float %40, i32 0
  %42 = shufflevector <4 x float> %41, <4 x float> undef, <4 x i32> zeroinitializer
  br label %SyncBB8.i

SyncBB8.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %43 = getelementptr %struct.PaddedDimId* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %25, i64 0, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = add i64 %44, %46
  %48 = mul i64 %47, %32
  %49 = add i64 %48, %32
  %50 = icmp ult i64 %48, %49
  br i1 %50, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB8.i
  %indvar.i = phi i64 [ %indvar.next.i, %bb.nph.i ], [ 0, %SyncBB8.i ]
  %tmp3.i = add i64 %48, %indvar.i
  %scevgep.i = getelementptr <4 x float> addrspace(1)* %1, i64 %tmp3.i
  %scevgep4.i = getelementptr <4 x float> addrspace(1)* %4, i64 %tmp3.i
  %scevgep5.i = getelementptr <4 x float> addrspace(1)* %7, i64 %tmp3.i
  %scevgep6.i = getelementptr <4 x float> addrspace(1)* %10, i64 %tmp3.i
  %scevgep7.i = getelementptr <4 x float> addrspace(1)* %13, i64 %tmp3.i
  %51 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %52 = load <4 x float> addrspace(1)* %scevgep4.i, align 16
  %53 = load <4 x float> addrspace(1)* %scevgep5.i, align 16
  %call.i.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_sqrtf4(<4 x float> %53) nounwind readnone
  %54 = fmul <4 x float> %34, %call.i.i
  %55 = fdiv <4 x float> %51, %52
  %call.i1.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_logf4(<4 x float> %55) nounwind readnone
  %56 = fmul <4 x float> %39, %53
  %57 = fadd <4 x float> %call.i1.i, %56
  %58 = fdiv <4 x float> %57, %54
  %59 = fsub <4 x float> %58, %54
  %call.i2.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_fabsf4(<4 x float> %58) nounwind readnone
  %60 = fmul <4 x float> %call.i2.i, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %61 = fadd <4 x float> %60, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %62 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %61
  %63 = fmul <4 x float> %call.i2.i, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %64 = fmul <4 x float> %63, %call.i2.i
  %call.i3.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %64) nounwind readnone
  %65 = fmul <4 x float> %call.i3.i, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %66 = fmul <4 x float> %65, %62
  %67 = fmul <4 x float> %62, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %68 = fadd <4 x float> %67, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %69 = fmul <4 x float> %62, %68
  %70 = fadd <4 x float> %69, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %71 = fmul <4 x float> %62, %70
  %72 = fadd <4 x float> %71, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %73 = fmul <4 x float> %62, %72
  %74 = fadd <4 x float> %73, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %75 = fmul <4 x float> %66, %74
  %76 = fsub <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %75
  %77 = fcmp olt <4 x float> %58, zeroinitializer
  %78 = sext <4 x i1> %77 to <4 x i32>
  %79 = bitcast <4 x float> %75 to <4 x i32>
  %80 = bitcast <4 x float> %76 to <4 x i32>
  %tmp2.i.i.i = shufflevector <4 x i32> %79, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i.i = shufflevector <4 x i32> %80, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i.i = shufflevector <4 x i32> %78, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i.i = bitcast <8 x i64> %tmp1.i.i.i.i to <16 x i32>
  %81 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i.i, <16 x i32> %conv2.i.i.i.i.i) nounwind
  %82 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %81, <16 x i32> %conv2.i.i.i.i.i) nounwind
  %83 = call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i.i, i16 %82, <16 x i32> %tmp2.i.i.i, <16 x i32> %tmp2.i.i.i) nounwind
  %tmp16.i.i.i = shufflevector <16 x i32> %83, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %84 = bitcast <4 x i32> %tmp16.i.i.i to <4 x float>
  %85 = bitcast <4 x float> %76 to <4 x i32>
  %86 = bitcast <4 x float> %75 to <4 x i32>
  %tmp2.i.i4.i = shufflevector <4 x i32> %85, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i5.i = shufflevector <4 x i32> %86, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i6.i = shufflevector <4 x i32> %78, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i7.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i8.i = bitcast <8 x i64> %tmp1.i.i.i7.i to <16 x i32>
  %87 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i6.i, <16 x i32> %conv2.i.i.i.i8.i) nounwind
  %88 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %87, <16 x i32> %conv2.i.i.i.i8.i) nounwind
  %89 = call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i5.i, i16 %88, <16 x i32> %tmp2.i.i4.i, <16 x i32> %tmp2.i.i4.i) nounwind
  %tmp16.i.i9.i = shufflevector <16 x i32> %89, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %90 = bitcast <4 x i32> %tmp16.i.i9.i to <4 x float>
  %call.i10.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_fabsf4(<4 x float> %59) nounwind readnone
  %91 = fmul <4 x float> %call.i10.i, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %92 = fadd <4 x float> %91, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %93 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %92
  %94 = fmul <4 x float> %call.i10.i, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %95 = fmul <4 x float> %94, %call.i10.i
  %call.i11.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %95) nounwind readnone
  %96 = fmul <4 x float> %call.i11.i, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %97 = fmul <4 x float> %96, %93
  %98 = fmul <4 x float> %93, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %99 = fadd <4 x float> %98, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %100 = fmul <4 x float> %93, %99
  %101 = fadd <4 x float> %100, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %102 = fmul <4 x float> %93, %101
  %103 = fadd <4 x float> %102, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %104 = fmul <4 x float> %93, %103
  %105 = fadd <4 x float> %104, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %106 = fmul <4 x float> %97, %105
  %107 = fsub <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %106
  %108 = fcmp olt <4 x float> %59, zeroinitializer
  %109 = sext <4 x i1> %108 to <4 x i32>
  %110 = bitcast <4 x float> %106 to <4 x i32>
  %111 = bitcast <4 x float> %107 to <4 x i32>
  %tmp2.i.i12.i = shufflevector <4 x i32> %110, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i13.i = shufflevector <4 x i32> %111, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i14.i = shufflevector <4 x i32> %109, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i15.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i16.i = bitcast <8 x i64> %tmp1.i.i.i15.i to <16 x i32>
  %112 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i14.i, <16 x i32> %conv2.i.i.i.i16.i) nounwind
  %113 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %112, <16 x i32> %conv2.i.i.i.i16.i) nounwind
  %114 = call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i13.i, i16 %113, <16 x i32> %tmp2.i.i12.i, <16 x i32> %tmp2.i.i12.i) nounwind
  %tmp16.i.i17.i = shufflevector <16 x i32> %114, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %115 = bitcast <4 x i32> %tmp16.i.i17.i to <4 x float>
  %116 = bitcast <4 x float> %107 to <4 x i32>
  %117 = bitcast <4 x float> %106 to <4 x i32>
  %tmp2.i.i18.i = shufflevector <4 x i32> %116, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp6.i.i19.i = shufflevector <4 x i32> %117, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp10.i.i20.i = shufflevector <4 x i32> %109, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp1.i.i.i21.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv2.i.i.i.i22.i = bitcast <8 x i64> %tmp1.i.i.i21.i to <16 x i32>
  %118 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %tmp10.i.i20.i, <16 x i32> %conv2.i.i.i.i22.i) nounwind
  %119 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %118, <16 x i32> %conv2.i.i.i.i22.i) nounwind
  %120 = call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %tmp6.i.i19.i, i16 %119, <16 x i32> %tmp2.i.i18.i, <16 x i32> %tmp2.i.i18.i) nounwind
  %tmp16.i.i23.i = shufflevector <16 x i32> %120, <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %121 = bitcast <4 x i32> %tmp16.i.i23.i to <4 x float>
  %122 = fmul <4 x float> %42, %53
  %call.i24.i = call x86_svmlcc <4 x float> @__ocl_svml_b1_expf4(<4 x float> %122) nounwind readnone
  %123 = fmul <4 x float> %52, %call.i24.i
  %124 = fmul <4 x float> %51, %90
  %125 = fmul <4 x float> %123, %121
  %126 = fsub <4 x float> %124, %125
  %127 = fmul <4 x float> %123, %115
  %128 = fmul <4 x float> %51, %84
  %129 = fsub <4 x float> %127, %128
  store <4 x float> %126, <4 x float> addrspace(1)* %scevgep6.i, align 16
  store <4 x float> %129, <4 x float> addrspace(1)* %scevgep7.i, align 16
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %32
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB8.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %__basVectorized_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB8.i

__basVectorized_separated_args.exit:              ; preds = %._crit_edge.i
  ret void
}

define void @basScalar(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(1)**
  %10 = load float addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float addrspace(1)**
  %13 = load float addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 44
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 104
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = fmul float %19, %19
  %30 = fmul float %29, 5.000000e-01
  %31 = fadd float %30, %16
  %32 = fsub float -0.000000e+00, %16
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %33 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %37
  %39 = load float addrspace(1)* %38, align 4
  %40 = getelementptr inbounds float addrspace(1)* %4, i64 %37
  %41 = load float addrspace(1)* %40, align 4
  %42 = getelementptr inbounds float addrspace(1)* %7, i64 %37
  %43 = load float addrspace(1)* %42, align 4
  %call.i.i = call x86_svmlcc float @__ocl_svml_b1_sqrtf1(float %43) nounwind readnone
  %44 = fmul float %call.i.i, %19
  %45 = fdiv float %39, %41
  %call.i1.i = call x86_svmlcc float @__ocl_svml_b1_logf1(float %45) nounwind readnone
  %46 = fmul float %31, %43
  %47 = fadd float %call.i1.i, %46
  %48 = fdiv float %47, %44
  %49 = fsub float %48, %44
  %call.i2.i = call x86_svmlcc float @__ocl_svml_b1_fabsf1(float %48) nounwind readnone
  %50 = fmul float %call.i2.i, 0x3FCDA67120000000
  %51 = fadd float %50, 1.000000e+00
  %52 = fdiv float 1.000000e+00, %51
  %53 = fmul float %call.i2.i, -5.000000e-01
  %54 = fmul float %53, %call.i2.i
  %call.i3.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %54) nounwind readnone
  %55 = fmul float %call.i3.i, 0x3FD9884540000000
  %56 = fmul float %55, %52
  %57 = fmul float %52, 0x3FF548CDE0000000
  %58 = fadd float %57, 0xBFFD23DD40000000
  %59 = fmul float %52, %58
  %60 = fadd float %59, 0x3FFC80EF00000000
  %61 = fmul float %52, %60
  %62 = fadd float %61, 0xBFD6D1F0E0000000
  %63 = fmul float %52, %62
  %64 = fadd float %63, 0x3FD470BF40000000
  %65 = fmul float %56, %64
  %66 = fcmp olt float %48, 0.000000e+00
  %67 = fsub float 1.000000e+00, %65
  %..i = select i1 %66, float %65, float %67
  %.1.i = select i1 %66, float %67, float %65
  %call.i4.i = call x86_svmlcc float @__ocl_svml_b1_fabsf1(float %49) nounwind readnone
  %68 = fmul float %call.i4.i, 0x3FCDA67120000000
  %69 = fadd float %68, 1.000000e+00
  %70 = fdiv float 1.000000e+00, %69
  %71 = fmul float %call.i4.i, -5.000000e-01
  %72 = fmul float %71, %call.i4.i
  %call.i5.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %72) nounwind readnone
  %73 = fmul float %call.i5.i, 0x3FD9884540000000
  %74 = fmul float %73, %70
  %75 = fmul float %70, 0x3FF548CDE0000000
  %76 = fadd float %75, 0xBFFD23DD40000000
  %77 = fmul float %70, %76
  %78 = fadd float %77, 0x3FFC80EF00000000
  %79 = fmul float %70, %78
  %80 = fadd float %79, 0xBFD6D1F0E0000000
  %81 = fmul float %70, %80
  %82 = fadd float %81, 0x3FD470BF40000000
  %83 = fmul float %74, %82
  %84 = fcmp olt float %49, 0.000000e+00
  %85 = fsub float 1.000000e+00, %83
  %.2.i = select i1 %84, float %83, float %85
  %.3.i = select i1 %84, float %85, float %83
  %86 = fmul float %43, %32
  %call.i6.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %86) nounwind readnone
  %87 = fmul float %41, %call.i6.i
  %88 = fmul float %39, %..i
  %89 = fmul float %87, %.2.i
  %90 = fsub float %88, %89
  %91 = fmul float %87, %.3.i
  %92 = fmul float %39, %.1.i
  %93 = fsub float %91, %92
  %94 = getelementptr inbounds float addrspace(1)* %10, i64 %37
  store float %90, float addrspace(1)* %94, align 4
  %95 = getelementptr inbounds float addrspace(1)* %13, i64 %37
  store float %93, float addrspace(1)* %95, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %__basScalar_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__basScalar_separated_args.exit:                  ; preds = %SyncBB.i
  ret void
}

define void @__Vectorized_.basScalar(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(1)**
  %10 = load float addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float addrspace(1)**
  %13 = load float addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 44
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 104
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %temp.i = insertelement <16 x float> undef, float %19, i32 0
  %vector.i = shufflevector <16 x float> %temp.i, <16 x float> undef, <16 x i32> zeroinitializer
  %29 = fmul float %19, %19
  %30 = fmul float %29, 5.000000e-01
  %31 = fadd float %30, %16
  %temp18.i = insertelement <16 x float> undef, float %31, i32 0
  %vector19.i = shufflevector <16 x float> %temp18.i, <16 x float> undef, <16 x i32> zeroinitializer
  %32 = fsub float -0.000000e+00, %16
  %temp24.i = insertelement <16 x float> undef, float %32, i32 0
  %vector25.i = shufflevector <16 x float> %temp24.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %33 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %37
  %ptrTypeCast.i = bitcast float addrspace(1)* %38 to <16 x float> addrspace(1)*
  %39 = load <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %40 = getelementptr inbounds float addrspace(1)* %4, i64 %37
  %ptrTypeCast16.i = bitcast float addrspace(1)* %40 to <16 x float> addrspace(1)*
  %41 = load <16 x float> addrspace(1)* %ptrTypeCast16.i, align 4
  %42 = getelementptr inbounds float addrspace(1)* %7, i64 %37
  %ptrTypeCast17.i = bitcast float addrspace(1)* %42 to <16 x float> addrspace(1)*
  %43 = load <16 x float> addrspace(1)* %ptrTypeCast17.i, align 4
  %call.i.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_sqrtf16(<16 x float> %43) nounwind readnone
  %44 = fmul <16 x float> %call.i.i, %vector.i
  %45 = fdiv <16 x float> %39, %41
  %call.i1.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_logf16(<16 x float> %45) nounwind readnone
  %46 = fmul <16 x float> %vector19.i, %43
  %47 = fadd <16 x float> %call.i1.i, %46
  %48 = fdiv <16 x float> %47, %44
  %49 = fsub <16 x float> %48, %44
  %call.i2.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_fabsf16(<16 x float> %48) nounwind readnone
  %50 = fmul <16 x float> %call.i2.i, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %51 = fadd <16 x float> %50, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %52 = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %51
  %53 = fmul <16 x float> %call.i2.i, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %54 = fmul <16 x float> %53, %call.i2.i
  %call.i3.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %54) nounwind readnone
  %55 = fmul <16 x float> %call.i3.i, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %56 = fmul <16 x float> %55, %52
  %57 = fmul <16 x float> %52, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %58 = fadd <16 x float> %57, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %59 = fmul <16 x float> %52, %58
  %60 = fadd <16 x float> %59, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %61 = fmul <16 x float> %52, %60
  %62 = fadd <16 x float> %61, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %63 = fmul <16 x float> %52, %62
  %64 = fadd <16 x float> %63, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %65 = fmul <16 x float> %56, %64
  %66 = fcmp olt <16 x float> %48, zeroinitializer
  %67 = fsub <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %65
  %.20.i = select <16 x i1> %66, <16 x float> %65, <16 x float> %67
  %.121.i = select <16 x i1> %66, <16 x float> %67, <16 x float> %65
  %call.i4.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_fabsf16(<16 x float> %49) nounwind readnone
  %68 = fmul <16 x float> %call.i4.i, <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>
  %69 = fadd <16 x float> %68, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %70 = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %69
  %71 = fmul <16 x float> %call.i4.i, <float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01, float -5.000000e-01>
  %72 = fmul <16 x float> %71, %call.i4.i
  %call.i5.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %72) nounwind readnone
  %73 = fmul <16 x float> %call.i5.i, <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>
  %74 = fmul <16 x float> %73, %70
  %75 = fmul <16 x float> %70, <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>
  %76 = fadd <16 x float> %75, <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>
  %77 = fmul <16 x float> %70, %76
  %78 = fadd <16 x float> %77, <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>
  %79 = fmul <16 x float> %70, %78
  %80 = fadd <16 x float> %79, <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>
  %81 = fmul <16 x float> %70, %80
  %82 = fadd <16 x float> %81, <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>
  %83 = fmul <16 x float> %74, %82
  %84 = fcmp olt <16 x float> %49, zeroinitializer
  %85 = fsub <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %83
  %.222.i = select <16 x i1> %84, <16 x float> %83, <16 x float> %85
  %.323.i = select <16 x i1> %84, <16 x float> %85, <16 x float> %83
  %86 = fmul <16 x float> %43, %vector25.i
  %call.i6.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %86) nounwind readnone
  %87 = fmul <16 x float> %41, %call.i6.i
  %88 = fmul <16 x float> %39, %.20.i
  %89 = fmul <16 x float> %87, %.222.i
  %90 = fsub <16 x float> %88, %89
  %91 = fmul <16 x float> %87, %.323.i
  %92 = fmul <16 x float> %39, %.121.i
  %93 = fsub <16 x float> %91, %92
  %94 = getelementptr inbounds float addrspace(1)* %10, i64 %37
  %ptrTypeCast26.i = bitcast float addrspace(1)* %94 to <16 x float> addrspace(1)*
  store <16 x float> %90, <16 x float> addrspace(1)* %ptrTypeCast26.i, align 4
  %95 = getelementptr inbounds float addrspace(1)* %13, i64 %37
  %ptrTypeCast27.i = bitcast float addrspace(1)* %95 to <16 x float> addrspace(1)*
  store <16 x float> %93, <16 x float> addrspace(1)* %ptrTypeCast27.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.basScalar_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.basScalar_separated_args.exit:    ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, float, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__basScalar_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *restrict, float const __attribute__((address_space(1))) *restrict, float const __attribute__((address_space(1))) *restrict, float __attribute__((address_space(1))) *restrict, float __attribute__((address_space(1))) *restrict, float const, float const, uint const", metadata !"opencl_basScalar_locals_anchor", void (i8*)* @basScalar}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, float, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__basVectorized_separated_args, metadata !1, metadata !1, metadata !"float4", metadata !"float4 const __attribute__((address_space(1))) *restrict, float4 const __attribute__((address_space(1))) *restrict, float4 const __attribute__((address_space(1))) *restrict, float4 __attribute__((address_space(1))) *restrict, float4 __attribute__((address_space(1))) *restrict, float const, float const, uint const", metadata !"opencl_basVectorized_locals_anchor", void (i8*)* @basVectorized}
