; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

@const_vector_zeros = private constant [16 x i32] zeroinitializer, align 64
@const_vector_msb = private constant [16 x i32] [i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648], align 64

declare <4 x float> @__evaluatePixel_original(double, double, <4 x float>, <4 x float>) nounwind

define double @_Z5floorDv2_f(double %x.coerce) nounwind readnone {
entry:
  %tmp11 = bitcast double %x.coerce to i64
  %tmp10 = bitcast i64 %tmp11 to <2 x float>
  %tmp8 = bitcast <2 x float> %tmp10 to <1 x double>
  %tmp7 = extractelement <1 x double> %tmp8, i32 0
  %call = tail call x86_svmlcc double @__ocl_svml_b1_floorf2(double %tmp7) nounwind readnone
  %tmp5 = bitcast double %call to i64
  %tmp4 = bitcast i64 %tmp5 to <2 x float>
  %tmp14 = bitcast <2 x float> %tmp4 to <1 x double>
  %tmp13 = extractelement <1 x double> %tmp14, i32 0
  ret double %tmp13
}

define double @_Z4fmodDv2_fS_(double %x.coerce, double %y.coerce) nounwind readnone {
entry:
  %tmp20 = bitcast double %x.coerce to i64
  %tmp19 = bitcast i64 %tmp20 to <2 x float>
  %tmp17 = bitcast double %y.coerce to i64
  %tmp16 = bitcast i64 %tmp17 to <2 x float>
  %tmp14 = bitcast <2 x float> %tmp19 to <1 x double>
  %tmp13 = extractelement <1 x double> %tmp14, i32 0
  %tmp11 = bitcast <2 x float> %tmp16 to <1 x double>
  %tmp10 = extractelement <1 x double> %tmp11, i32 0
  %call = tail call x86_svmlcc double @__ocl_svml_b1_fmodf2(double %tmp13, double %tmp10) nounwind readnone
  %tmp8 = bitcast double %call to i64
  %tmp7 = bitcast i64 %tmp8 to <2 x float>
  %tmp23 = bitcast <2 x float> %tmp7 to <1 x double>
  %tmp22 = extractelement <1 x double> %tmp23, i32 0
  ret double %tmp22
}

define i32 @_Z3allDv2_i(double %x.coerce) nounwind readnone {
entry:
  %tmp7 = bitcast double %x.coerce to i64
  %tmp6 = bitcast i64 %tmp7 to <2 x i32>
  %tmp2 = shufflevector <2 x i32> %tmp6, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %0 = bitcast <16 x i32> %tmp2 to <16 x float>
  %1 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %0, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i = bitcast <16 x float> %1 to <16 x i32>
  %conv2.i.i = bitcast <8 x i64> %tmp5.i to <16 x i32>
  %2 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i, <16 x i32> %conv2.i.i) nounwind
  %3 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %2, <16 x i32> %conv2.i.i) nounwind
  %4 = tail call i32 @llvm.x86.mic.kortestc(i16 %3, i16 %3) nounwind
  %tobool.i = icmp ne i32 %4, 0
  %cond.i = zext i1 %tobool.i to i32
  ret i32 %cond.i
}

define double @_Z7isequalDv2_fS_(double %x.coerce, double %y.coerce) nounwind readnone {
entry:
  %0 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %1 = bitcast <16 x float> %0 to <16 x i32>
  %tmp24 = shufflevector <16 x i32> %1, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32 = bitcast <2 x i32> %tmp24 to <1 x double>
  %tmp31 = extractelement <1 x double> %tmp32, i32 0
  ret double %tmp31
}

declare void @__checkerboard2D_original(<4 x float> addrspace(1)* nocapture, double, <4 x float>, <4 x float>) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.checkerboard2D_original(<4 x float> addrspace(1)* nocapture, double, <4 x float>, <4 x float>) nounwind

define float @_Z5floorf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_floorf1(float %x) nounwind readnone
  ret float %call
}

define float @_Z4fmodff(float %x, float %y) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_fmodf1(float %x, float %y) nounwind readnone
  ret float %call
}

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare double @maskedf_0__Z7isequalDv2_fS_(i1, double, double)

declare i32 @maskedf_1__Z3allDv2_i(i1, double)

define <16 x float> @_Z5floorDv16_f(<16 x float> %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_floorf16(<16 x float> %x) nounwind readnone
  ret <16 x float> %call
}

define <16 x float> @_Z4fmodDv16_fS_(<16 x float> %x, <16 x float> %y) nounwind readnone {
entry:
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_b1_fmodf16(<16 x float> %x, <16 x float> %y) nounwind readnone
  ret <16 x float> %call
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define <4 x float> @evaluatePixel(double %outCrd.coerce, double %checkerSize.coerce, <4 x float> %color1, <4 x float> %color2, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind {
  %1 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %2 = bitcast <16 x float> %1 to <16 x i32>
  %tmp24.i = shufflevector <16 x i32> %2, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i = bitcast <2 x i32> %tmp24.i to <1 x double>
  %tmp31.i = extractelement <1 x double> %tmp32.i, i32 0
  %tmp16 = bitcast double %tmp31.i to i64
  %tmp15 = bitcast i64 %tmp16 to <2 x i32>
  %tmp13 = bitcast <2 x i32> %tmp15 to <1 x double>
  %tmp12 = extractelement <1 x double> %tmp13, i32 0
  %tmp7.i6 = bitcast double %tmp12 to i64
  %tmp6.i = bitcast i64 %tmp7.i6 to <2 x i32>
  %tmp2.i = shufflevector <2 x i32> %tmp6.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %3 = bitcast <16 x i32> %tmp2.i to <16 x float>
  %4 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %3, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i = bitcast <16 x float> %4 to <16 x i32>
  %conv2.i.i.i = bitcast <8 x i64> %tmp5.i.i to <16 x i32>
  %5 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i, <16 x i32> %conv2.i.i.i) nounwind
  %6 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %5, <16 x i32> %conv2.i.i.i) nounwind
  %7 = tail call i32 @llvm.x86.mic.kortestc(i16 %6, i16 %6) nounwind
  %tobool.i.i = icmp eq i32 %7, 0
  br i1 %tobool.i.i, label %8, label %UnifiedReturnBlock

; <label>:8                                       ; preds = %0
  %9 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %10 = bitcast <16 x float> %9 to <16 x i32>
  %tmp24.i7 = shufflevector <16 x i32> %10, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i8 = bitcast <2 x i32> %tmp24.i7 to <1 x double>
  %tmp31.i9 = extractelement <1 x double> %tmp32.i8, i32 0
  %tmp5 = bitcast double %tmp31.i9 to i64
  %tmp4 = bitcast i64 %tmp5 to <2 x i32>
  %tmp2 = bitcast <2 x i32> %tmp4 to <1 x double>
  %tmp1 = extractelement <1 x double> %tmp2, i32 0
  %tmp7.i10 = bitcast double %tmp1 to i64
  %tmp6.i11 = bitcast i64 %tmp7.i10 to <2 x i32>
  %tmp2.i12 = shufflevector <2 x i32> %tmp6.i11, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %11 = bitcast <16 x i32> %tmp2.i12 to <16 x float>
  %12 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %11, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i13 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i14 = bitcast <16 x float> %12 to <16 x i32>
  %conv2.i.i.i15 = bitcast <8 x i64> %tmp5.i.i13 to <16 x i32>
  %13 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i14, <16 x i32> %conv2.i.i.i15) nounwind
  %14 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %13, <16 x i32> %conv2.i.i.i15) nounwind
  %15 = tail call i32 @llvm.x86.mic.kortestc(i16 %14, i16 %14) nounwind
  %tobool.i.i16 = icmp ne i32 %15, 0
  %phitmp = select i1 %tobool.i.i16, <4 x float> %color1, <4 x float> %color2
  ret <4 x float> %phitmp

UnifiedReturnBlock:                               ; preds = %0
  ret <4 x float> %color1
}

define void @__checkerboard2D_separated_args(<4 x float> addrspace(1)* nocapture %output, double %checkerSize.coerce, <4 x float> %color1, <4 x float> %color2, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB9

SyncBB9:                                          ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %7 = load i64* %6, align 8
  %8 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = add i64 %7, %9
  %11 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %12 = load i64* %11, align 8
  %13 = mul i64 %12, %10
  %14 = add i64 %13, %5
  %15 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %16 = bitcast <16 x float> %15 to <16 x i32>
  %tmp24.i11 = shufflevector <16 x i32> %16, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i12 = bitcast <2 x i32> %tmp24.i11 to <1 x double>
  %tmp31.i13 = extractelement <1 x double> %tmp32.i12, i32 0
  %tmp16.i = bitcast double %tmp31.i13 to i64
  %tmp15.i = bitcast i64 %tmp16.i to <2 x i32>
  %tmp13.i = bitcast <2 x i32> %tmp15.i to <1 x double>
  %tmp12.i = extractelement <1 x double> %tmp13.i, i32 0
  %tmp7.i14 = bitcast double %tmp12.i to i64
  %tmp6.i = bitcast i64 %tmp7.i14 to <2 x i32>
  %tmp2.i15 = shufflevector <2 x i32> %tmp6.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %17 = bitcast <16 x i32> %tmp2.i15 to <16 x float>
  %18 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %17, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i = bitcast <16 x float> %18 to <16 x i32>
  %conv2.i.i.i = bitcast <8 x i64> %tmp5.i.i to <16 x i32>
  %19 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i, <16 x i32> %conv2.i.i.i) nounwind
  %20 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %19, <16 x i32> %conv2.i.i.i) nounwind
  %21 = tail call i32 @llvm.x86.mic.kortestc(i16 %20, i16 %20) nounwind
  %tobool.i.i = icmp eq i32 %21, 0
  br i1 %tobool.i.i, label %22, label %evaluatePixel.exit

; <label>:22                                      ; preds = %SyncBB9
  %23 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %24 = bitcast <16 x float> %23 to <16 x i32>
  %tmp24.i16 = shufflevector <16 x i32> %24, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i17 = bitcast <2 x i32> %tmp24.i16 to <1 x double>
  %tmp31.i18 = extractelement <1 x double> %tmp32.i17, i32 0
  %tmp5.i = bitcast double %tmp31.i18 to i64
  %tmp4.i = bitcast i64 %tmp5.i to <2 x i32>
  %tmp2.i = bitcast <2 x i32> %tmp4.i to <1 x double>
  %tmp1.i = extractelement <1 x double> %tmp2.i, i32 0
  %tmp7.i19 = bitcast double %tmp1.i to i64
  %tmp6.i20 = bitcast i64 %tmp7.i19 to <2 x i32>
  %tmp2.i21 = shufflevector <2 x i32> %tmp6.i20, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %25 = bitcast <16 x i32> %tmp2.i21 to <16 x float>
  %26 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %25, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i22 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i23 = bitcast <16 x float> %26 to <16 x i32>
  %conv2.i.i.i24 = bitcast <8 x i64> %tmp5.i.i22 to <16 x i32>
  %27 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i23, <16 x i32> %conv2.i.i.i24) nounwind
  %28 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %27, <16 x i32> %conv2.i.i.i24) nounwind
  %29 = tail call i32 @llvm.x86.mic.kortestc(i16 %28, i16 %28) nounwind
  %tobool.i.i25 = icmp ne i32 %29, 0
  %phitmp.i = select i1 %tobool.i.i25, <4 x float> %color1, <4 x float> %color2
  br label %evaluatePixel.exit

evaluatePixel.exit:                               ; preds = %SyncBB9, %22
  %UnifiedRetVal.i = phi <4 x float> [ %phitmp.i, %22 ], [ %color1, %SyncBB9 ]
  %sext = shl i64 %14, 32
  %30 = ashr i64 %sext, 32
  %31 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %30
  store <4 x float> %UnifiedRetVal.i, <4 x float> addrspace(1)* %31, align 16
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %evaluatePixel.exit
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB9

SyncBB:                                           ; preds = %evaluatePixel.exit
  ret void
}

define void @____Vectorized_.checkerboard2D_separated_args(<4 x float> addrspace(1)* nocapture %output, double %checkerSize.coerce, <4 x float> %color1, <4 x float> %color2, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %scalar16 = extractelement <4 x float> %color2, i32 0
  %temp286 = insertelement <16 x float> undef, float %scalar16, i32 0
  %vector287 = shufflevector <16 x float> %temp286, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar17 = extractelement <4 x float> %color2, i32 1
  %temp291 = insertelement <16 x float> undef, float %scalar17, i32 0
  %vector292 = shufflevector <16 x float> %temp291, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar18 = extractelement <4 x float> %color2, i32 2
  %temp296 = insertelement <16 x float> undef, float %scalar18, i32 0
  %vector297 = shufflevector <16 x float> %temp296, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar19 = extractelement <4 x float> %color2, i32 3
  %temp301 = insertelement <16 x float> undef, float %scalar19, i32 0
  %vector302 = shufflevector <16 x float> %temp301, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar12 = extractelement <4 x float> %color1, i32 0
  %temp284 = insertelement <16 x float> undef, float %scalar12, i32 0
  %vector285 = shufflevector <16 x float> %temp284, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar13 = extractelement <4 x float> %color1, i32 1
  %temp289 = insertelement <16 x float> undef, float %scalar13, i32 0
  %vector290 = shufflevector <16 x float> %temp289, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar14 = extractelement <4 x float> %color1, i32 2
  %temp294 = insertelement <16 x float> undef, float %scalar14, i32 0
  %vector295 = shufflevector <16 x float> %temp294, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar15 = extractelement <4 x float> %color1, i32 3
  %temp299 = insertelement <16 x float> undef, float %scalar15, i32 0
  %vector300 = shufflevector <16 x float> %temp299, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %broadcast1 = insertelement <16 x i64> undef, i64 %4, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %5 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %6 = trunc <16 x i64> %5 to <16 x i32>
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %12 = trunc i64 %11 to i32
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %14 = load i64* %13, align 8
  %15 = trunc i64 %14 to i32
  %16 = mul nsw i32 %15, %12
  %temp = insertelement <16 x i32> undef, i32 %16, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %17 = add nsw <16 x i32> %vector, %6
  %18 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %19 = bitcast <16 x float> %18 to <16 x i32>
  %tmp24.i = shufflevector <16 x i32> %19, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i = bitcast <2 x i32> %tmp24.i to <1 x double>
  %tmp31.i = extractelement <1 x double> %tmp32.i, i32 0
  %20 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %21 = bitcast <16 x float> %20 to <16 x i32>
  %tmp24.i4 = shufflevector <16 x i32> %21, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i5 = bitcast <2 x i32> %tmp24.i4 to <1 x double>
  %tmp31.i6 = extractelement <1 x double> %tmp32.i5, i32 0
  %22 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %23 = bitcast <16 x float> %22 to <16 x i32>
  %tmp24.i7 = shufflevector <16 x i32> %23, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i8 = bitcast <2 x i32> %tmp24.i7 to <1 x double>
  %tmp31.i9 = extractelement <1 x double> %tmp32.i8, i32 0
  %24 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %25 = bitcast <16 x float> %24 to <16 x i32>
  %tmp24.i10 = shufflevector <16 x i32> %25, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i11 = bitcast <2 x i32> %tmp24.i10 to <1 x double>
  %tmp31.i12 = extractelement <1 x double> %tmp32.i11, i32 0
  %26 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %27 = bitcast <16 x float> %26 to <16 x i32>
  %tmp24.i13 = shufflevector <16 x i32> %27, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i14 = bitcast <2 x i32> %tmp24.i13 to <1 x double>
  %tmp31.i15 = extractelement <1 x double> %tmp32.i14, i32 0
  %28 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %29 = bitcast <16 x float> %28 to <16 x i32>
  %tmp24.i16 = shufflevector <16 x i32> %29, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i17 = bitcast <2 x i32> %tmp24.i16 to <1 x double>
  %tmp31.i18 = extractelement <1 x double> %tmp32.i17, i32 0
  %30 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %31 = bitcast <16 x float> %30 to <16 x i32>
  %tmp24.i19 = shufflevector <16 x i32> %31, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i20 = bitcast <2 x i32> %tmp24.i19 to <1 x double>
  %tmp31.i21 = extractelement <1 x double> %tmp32.i20, i32 0
  %32 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %33 = bitcast <16 x float> %32 to <16 x i32>
  %tmp24.i22 = shufflevector <16 x i32> %33, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i23 = bitcast <2 x i32> %tmp24.i22 to <1 x double>
  %tmp31.i24 = extractelement <1 x double> %tmp32.i23, i32 0
  %34 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %35 = bitcast <16 x float> %34 to <16 x i32>
  %tmp24.i25 = shufflevector <16 x i32> %35, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i26 = bitcast <2 x i32> %tmp24.i25 to <1 x double>
  %tmp31.i27 = extractelement <1 x double> %tmp32.i26, i32 0
  %36 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %37 = bitcast <16 x float> %36 to <16 x i32>
  %tmp24.i28 = shufflevector <16 x i32> %37, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i29 = bitcast <2 x i32> %tmp24.i28 to <1 x double>
  %tmp31.i30 = extractelement <1 x double> %tmp32.i29, i32 0
  %38 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %39 = bitcast <16 x float> %38 to <16 x i32>
  %tmp24.i31 = shufflevector <16 x i32> %39, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i32 = bitcast <2 x i32> %tmp24.i31 to <1 x double>
  %tmp31.i33 = extractelement <1 x double> %tmp32.i32, i32 0
  %40 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %41 = bitcast <16 x float> %40 to <16 x i32>
  %tmp24.i34 = shufflevector <16 x i32> %41, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i35 = bitcast <2 x i32> %tmp24.i34 to <1 x double>
  %tmp31.i36 = extractelement <1 x double> %tmp32.i35, i32 0
  %42 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %43 = bitcast <16 x float> %42 to <16 x i32>
  %tmp24.i37 = shufflevector <16 x i32> %43, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i38 = bitcast <2 x i32> %tmp24.i37 to <1 x double>
  %tmp31.i39 = extractelement <1 x double> %tmp32.i38, i32 0
  %44 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %45 = bitcast <16 x float> %44 to <16 x i32>
  %tmp24.i40 = shufflevector <16 x i32> %45, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i41 = bitcast <2 x i32> %tmp24.i40 to <1 x double>
  %tmp31.i42 = extractelement <1 x double> %tmp32.i41, i32 0
  %46 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %47 = bitcast <16 x float> %46 to <16 x i32>
  %tmp24.i43 = shufflevector <16 x i32> %47, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i44 = bitcast <2 x i32> %tmp24.i43 to <1 x double>
  %tmp31.i45 = extractelement <1 x double> %tmp32.i44, i32 0
  %48 = tail call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %49 = bitcast <16 x float> %48 to <16 x i32>
  %tmp24.i46 = shufflevector <16 x i32> %49, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i47 = bitcast <2 x i32> %tmp24.i46 to <1 x double>
  %tmp31.i48 = extractelement <1 x double> %tmp32.i47, i32 0
  %extract186 = bitcast double %tmp31.i to i64
  %extract187 = bitcast double %tmp31.i6 to i64
  %extract188 = bitcast double %tmp31.i9 to i64
  %extract189 = bitcast double %tmp31.i12 to i64
  %extract190 = bitcast double %tmp31.i15 to i64
  %extract191 = bitcast double %tmp31.i18 to i64
  %extract192 = bitcast double %tmp31.i21 to i64
  %extract193 = bitcast double %tmp31.i24 to i64
  %extract194 = bitcast double %tmp31.i27 to i64
  %extract195 = bitcast double %tmp31.i30 to i64
  %extract196 = bitcast double %tmp31.i33 to i64
  %extract197 = bitcast double %tmp31.i36 to i64
  %extract198 = bitcast double %tmp31.i39 to i64
  %extract199 = bitcast double %tmp31.i42 to i64
  %extract200 = bitcast double %tmp31.i45 to i64
  %extract201 = bitcast double %tmp31.i48 to i64
  %50 = bitcast i64 %extract186 to <2 x i32>
  %51 = bitcast i64 %extract187 to <2 x i32>
  %52 = bitcast i64 %extract188 to <2 x i32>
  %53 = bitcast i64 %extract189 to <2 x i32>
  %54 = bitcast i64 %extract190 to <2 x i32>
  %55 = bitcast i64 %extract191 to <2 x i32>
  %56 = bitcast i64 %extract192 to <2 x i32>
  %57 = bitcast i64 %extract193 to <2 x i32>
  %58 = bitcast i64 %extract194 to <2 x i32>
  %59 = bitcast i64 %extract195 to <2 x i32>
  %60 = bitcast i64 %extract196 to <2 x i32>
  %61 = bitcast i64 %extract197 to <2 x i32>
  %62 = bitcast i64 %extract198 to <2 x i32>
  %63 = bitcast i64 %extract199 to <2 x i32>
  %64 = bitcast i64 %extract200 to <2 x i32>
  %65 = bitcast i64 %extract201 to <2 x i32>
  %66 = bitcast <2 x i32> %50 to <1 x double>
  %67 = bitcast <2 x i32> %51 to <1 x double>
  %68 = bitcast <2 x i32> %52 to <1 x double>
  %69 = bitcast <2 x i32> %53 to <1 x double>
  %70 = bitcast <2 x i32> %54 to <1 x double>
  %71 = bitcast <2 x i32> %55 to <1 x double>
  %72 = bitcast <2 x i32> %56 to <1 x double>
  %73 = bitcast <2 x i32> %57 to <1 x double>
  %74 = bitcast <2 x i32> %58 to <1 x double>
  %75 = bitcast <2 x i32> %59 to <1 x double>
  %76 = bitcast <2 x i32> %60 to <1 x double>
  %77 = bitcast <2 x i32> %61 to <1 x double>
  %78 = bitcast <2 x i32> %62 to <1 x double>
  %79 = bitcast <2 x i32> %63 to <1 x double>
  %80 = bitcast <2 x i32> %64 to <1 x double>
  %81 = bitcast <2 x i32> %65 to <1 x double>
  %82 = extractelement <1 x double> %66, i32 0
  %83 = extractelement <1 x double> %67, i32 0
  %84 = extractelement <1 x double> %68, i32 0
  %85 = extractelement <1 x double> %69, i32 0
  %86 = extractelement <1 x double> %70, i32 0
  %87 = extractelement <1 x double> %71, i32 0
  %88 = extractelement <1 x double> %72, i32 0
  %89 = extractelement <1 x double> %73, i32 0
  %90 = extractelement <1 x double> %74, i32 0
  %91 = extractelement <1 x double> %75, i32 0
  %92 = extractelement <1 x double> %76, i32 0
  %93 = extractelement <1 x double> %77, i32 0
  %94 = extractelement <1 x double> %78, i32 0
  %95 = extractelement <1 x double> %79, i32 0
  %96 = extractelement <1 x double> %80, i32 0
  %97 = extractelement <1 x double> %81, i32 0
  %tmp7.i = bitcast double %82 to i64
  %tmp6.i = bitcast i64 %tmp7.i to <2 x i32>
  %tmp2.i = shufflevector <2 x i32> %tmp6.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %98 = bitcast <16 x i32> %tmp2.i to <16 x float>
  %99 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %98, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i = bitcast <16 x float> %99 to <16 x i32>
  %conv2.i.i.i = bitcast <8 x i64> %tmp5.i.i to <16 x i32>
  %100 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i, <16 x i32> %conv2.i.i.i) nounwind
  %101 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %100, <16 x i32> %conv2.i.i.i) nounwind
  %102 = tail call i32 @llvm.x86.mic.kortestc(i16 %101, i16 %101) nounwind
  %tobool.i.i = icmp ne i32 %102, 0
  %cond.i.i = zext i1 %tobool.i.i to i32
  %tmp7.i49 = bitcast double %83 to i64
  %tmp6.i50 = bitcast i64 %tmp7.i49 to <2 x i32>
  %tmp2.i51 = shufflevector <2 x i32> %tmp6.i50, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %103 = bitcast <16 x i32> %tmp2.i51 to <16 x float>
  %104 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %103, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i52 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i53 = bitcast <16 x float> %104 to <16 x i32>
  %conv2.i.i.i54 = bitcast <8 x i64> %tmp5.i.i52 to <16 x i32>
  %105 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i53, <16 x i32> %conv2.i.i.i54) nounwind
  %106 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %105, <16 x i32> %conv2.i.i.i54) nounwind
  %107 = tail call i32 @llvm.x86.mic.kortestc(i16 %106, i16 %106) nounwind
  %tobool.i.i55 = icmp ne i32 %107, 0
  %cond.i.i56 = zext i1 %tobool.i.i55 to i32
  %tmp7.i57 = bitcast double %84 to i64
  %tmp6.i58 = bitcast i64 %tmp7.i57 to <2 x i32>
  %tmp2.i59 = shufflevector <2 x i32> %tmp6.i58, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %108 = bitcast <16 x i32> %tmp2.i59 to <16 x float>
  %109 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %108, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i60 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i61 = bitcast <16 x float> %109 to <16 x i32>
  %conv2.i.i.i62 = bitcast <8 x i64> %tmp5.i.i60 to <16 x i32>
  %110 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i61, <16 x i32> %conv2.i.i.i62) nounwind
  %111 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %110, <16 x i32> %conv2.i.i.i62) nounwind
  %112 = tail call i32 @llvm.x86.mic.kortestc(i16 %111, i16 %111) nounwind
  %tobool.i.i63 = icmp ne i32 %112, 0
  %cond.i.i64 = zext i1 %tobool.i.i63 to i32
  %tmp7.i65 = bitcast double %85 to i64
  %tmp6.i66 = bitcast i64 %tmp7.i65 to <2 x i32>
  %tmp2.i67 = shufflevector <2 x i32> %tmp6.i66, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %113 = bitcast <16 x i32> %tmp2.i67 to <16 x float>
  %114 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %113, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i68 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i69 = bitcast <16 x float> %114 to <16 x i32>
  %conv2.i.i.i70 = bitcast <8 x i64> %tmp5.i.i68 to <16 x i32>
  %115 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i69, <16 x i32> %conv2.i.i.i70) nounwind
  %116 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %115, <16 x i32> %conv2.i.i.i70) nounwind
  %117 = tail call i32 @llvm.x86.mic.kortestc(i16 %116, i16 %116) nounwind
  %tobool.i.i71 = icmp ne i32 %117, 0
  %cond.i.i72 = zext i1 %tobool.i.i71 to i32
  %tmp7.i73 = bitcast double %86 to i64
  %tmp6.i74 = bitcast i64 %tmp7.i73 to <2 x i32>
  %tmp2.i75 = shufflevector <2 x i32> %tmp6.i74, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %118 = bitcast <16 x i32> %tmp2.i75 to <16 x float>
  %119 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %118, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i76 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i77 = bitcast <16 x float> %119 to <16 x i32>
  %conv2.i.i.i78 = bitcast <8 x i64> %tmp5.i.i76 to <16 x i32>
  %120 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i77, <16 x i32> %conv2.i.i.i78) nounwind
  %121 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %120, <16 x i32> %conv2.i.i.i78) nounwind
  %122 = tail call i32 @llvm.x86.mic.kortestc(i16 %121, i16 %121) nounwind
  %tobool.i.i79 = icmp ne i32 %122, 0
  %cond.i.i80 = zext i1 %tobool.i.i79 to i32
  %tmp7.i81 = bitcast double %87 to i64
  %tmp6.i82 = bitcast i64 %tmp7.i81 to <2 x i32>
  %tmp2.i83 = shufflevector <2 x i32> %tmp6.i82, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %123 = bitcast <16 x i32> %tmp2.i83 to <16 x float>
  %124 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %123, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i84 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i85 = bitcast <16 x float> %124 to <16 x i32>
  %conv2.i.i.i86 = bitcast <8 x i64> %tmp5.i.i84 to <16 x i32>
  %125 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i85, <16 x i32> %conv2.i.i.i86) nounwind
  %126 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %125, <16 x i32> %conv2.i.i.i86) nounwind
  %127 = tail call i32 @llvm.x86.mic.kortestc(i16 %126, i16 %126) nounwind
  %tobool.i.i87 = icmp ne i32 %127, 0
  %cond.i.i88 = zext i1 %tobool.i.i87 to i32
  %tmp7.i89 = bitcast double %88 to i64
  %tmp6.i90 = bitcast i64 %tmp7.i89 to <2 x i32>
  %tmp2.i91 = shufflevector <2 x i32> %tmp6.i90, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %128 = bitcast <16 x i32> %tmp2.i91 to <16 x float>
  %129 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %128, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i92 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i93 = bitcast <16 x float> %129 to <16 x i32>
  %conv2.i.i.i94 = bitcast <8 x i64> %tmp5.i.i92 to <16 x i32>
  %130 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i93, <16 x i32> %conv2.i.i.i94) nounwind
  %131 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %130, <16 x i32> %conv2.i.i.i94) nounwind
  %132 = tail call i32 @llvm.x86.mic.kortestc(i16 %131, i16 %131) nounwind
  %tobool.i.i95 = icmp ne i32 %132, 0
  %cond.i.i96 = zext i1 %tobool.i.i95 to i32
  %tmp7.i97 = bitcast double %89 to i64
  %tmp6.i98 = bitcast i64 %tmp7.i97 to <2 x i32>
  %tmp2.i99 = shufflevector <2 x i32> %tmp6.i98, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %133 = bitcast <16 x i32> %tmp2.i99 to <16 x float>
  %134 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %133, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i100 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i101 = bitcast <16 x float> %134 to <16 x i32>
  %conv2.i.i.i102 = bitcast <8 x i64> %tmp5.i.i100 to <16 x i32>
  %135 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i101, <16 x i32> %conv2.i.i.i102) nounwind
  %136 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %135, <16 x i32> %conv2.i.i.i102) nounwind
  %137 = tail call i32 @llvm.x86.mic.kortestc(i16 %136, i16 %136) nounwind
  %tobool.i.i103 = icmp ne i32 %137, 0
  %cond.i.i104 = zext i1 %tobool.i.i103 to i32
  %tmp7.i105 = bitcast double %90 to i64
  %tmp6.i106 = bitcast i64 %tmp7.i105 to <2 x i32>
  %tmp2.i107 = shufflevector <2 x i32> %tmp6.i106, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %138 = bitcast <16 x i32> %tmp2.i107 to <16 x float>
  %139 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %138, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i108 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i109 = bitcast <16 x float> %139 to <16 x i32>
  %conv2.i.i.i110 = bitcast <8 x i64> %tmp5.i.i108 to <16 x i32>
  %140 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i109, <16 x i32> %conv2.i.i.i110) nounwind
  %141 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %140, <16 x i32> %conv2.i.i.i110) nounwind
  %142 = tail call i32 @llvm.x86.mic.kortestc(i16 %141, i16 %141) nounwind
  %tobool.i.i111 = icmp ne i32 %142, 0
  %cond.i.i112 = zext i1 %tobool.i.i111 to i32
  %tmp7.i113 = bitcast double %91 to i64
  %tmp6.i114 = bitcast i64 %tmp7.i113 to <2 x i32>
  %tmp2.i115 = shufflevector <2 x i32> %tmp6.i114, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %143 = bitcast <16 x i32> %tmp2.i115 to <16 x float>
  %144 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %143, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i116 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i117 = bitcast <16 x float> %144 to <16 x i32>
  %conv2.i.i.i118 = bitcast <8 x i64> %tmp5.i.i116 to <16 x i32>
  %145 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i117, <16 x i32> %conv2.i.i.i118) nounwind
  %146 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %145, <16 x i32> %conv2.i.i.i118) nounwind
  %147 = tail call i32 @llvm.x86.mic.kortestc(i16 %146, i16 %146) nounwind
  %tobool.i.i119 = icmp ne i32 %147, 0
  %cond.i.i120 = zext i1 %tobool.i.i119 to i32
  %tmp7.i121 = bitcast double %92 to i64
  %tmp6.i122 = bitcast i64 %tmp7.i121 to <2 x i32>
  %tmp2.i123 = shufflevector <2 x i32> %tmp6.i122, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %148 = bitcast <16 x i32> %tmp2.i123 to <16 x float>
  %149 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %148, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i124 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i125 = bitcast <16 x float> %149 to <16 x i32>
  %conv2.i.i.i126 = bitcast <8 x i64> %tmp5.i.i124 to <16 x i32>
  %150 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i125, <16 x i32> %conv2.i.i.i126) nounwind
  %151 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %150, <16 x i32> %conv2.i.i.i126) nounwind
  %152 = tail call i32 @llvm.x86.mic.kortestc(i16 %151, i16 %151) nounwind
  %tobool.i.i127 = icmp ne i32 %152, 0
  %cond.i.i128 = zext i1 %tobool.i.i127 to i32
  %tmp7.i129 = bitcast double %93 to i64
  %tmp6.i130 = bitcast i64 %tmp7.i129 to <2 x i32>
  %tmp2.i131 = shufflevector <2 x i32> %tmp6.i130, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %153 = bitcast <16 x i32> %tmp2.i131 to <16 x float>
  %154 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %153, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i132 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i133 = bitcast <16 x float> %154 to <16 x i32>
  %conv2.i.i.i134 = bitcast <8 x i64> %tmp5.i.i132 to <16 x i32>
  %155 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i133, <16 x i32> %conv2.i.i.i134) nounwind
  %156 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %155, <16 x i32> %conv2.i.i.i134) nounwind
  %157 = tail call i32 @llvm.x86.mic.kortestc(i16 %156, i16 %156) nounwind
  %tobool.i.i135 = icmp ne i32 %157, 0
  %cond.i.i136 = zext i1 %tobool.i.i135 to i32
  %tmp7.i137 = bitcast double %94 to i64
  %tmp6.i138 = bitcast i64 %tmp7.i137 to <2 x i32>
  %tmp2.i139 = shufflevector <2 x i32> %tmp6.i138, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %158 = bitcast <16 x i32> %tmp2.i139 to <16 x float>
  %159 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %158, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i140 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i141 = bitcast <16 x float> %159 to <16 x i32>
  %conv2.i.i.i142 = bitcast <8 x i64> %tmp5.i.i140 to <16 x i32>
  %160 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i141, <16 x i32> %conv2.i.i.i142) nounwind
  %161 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %160, <16 x i32> %conv2.i.i.i142) nounwind
  %162 = tail call i32 @llvm.x86.mic.kortestc(i16 %161, i16 %161) nounwind
  %tobool.i.i143 = icmp ne i32 %162, 0
  %cond.i.i144 = zext i1 %tobool.i.i143 to i32
  %tmp7.i145 = bitcast double %95 to i64
  %tmp6.i146 = bitcast i64 %tmp7.i145 to <2 x i32>
  %tmp2.i147 = shufflevector <2 x i32> %tmp6.i146, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %163 = bitcast <16 x i32> %tmp2.i147 to <16 x float>
  %164 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %163, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i148 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i149 = bitcast <16 x float> %164 to <16 x i32>
  %conv2.i.i.i150 = bitcast <8 x i64> %tmp5.i.i148 to <16 x i32>
  %165 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i149, <16 x i32> %conv2.i.i.i150) nounwind
  %166 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %165, <16 x i32> %conv2.i.i.i150) nounwind
  %167 = tail call i32 @llvm.x86.mic.kortestc(i16 %166, i16 %166) nounwind
  %tobool.i.i151 = icmp ne i32 %167, 0
  %cond.i.i152 = zext i1 %tobool.i.i151 to i32
  %tmp7.i153 = bitcast double %96 to i64
  %tmp6.i154 = bitcast i64 %tmp7.i153 to <2 x i32>
  %tmp2.i155 = shufflevector <2 x i32> %tmp6.i154, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %168 = bitcast <16 x i32> %tmp2.i155 to <16 x float>
  %169 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %168, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i156 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i157 = bitcast <16 x float> %169 to <16 x i32>
  %conv2.i.i.i158 = bitcast <8 x i64> %tmp5.i.i156 to <16 x i32>
  %170 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i157, <16 x i32> %conv2.i.i.i158) nounwind
  %171 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %170, <16 x i32> %conv2.i.i.i158) nounwind
  %172 = tail call i32 @llvm.x86.mic.kortestc(i16 %171, i16 %171) nounwind
  %tobool.i.i159 = icmp ne i32 %172, 0
  %cond.i.i160 = zext i1 %tobool.i.i159 to i32
  %tmp7.i161 = bitcast double %97 to i64
  %tmp6.i162 = bitcast i64 %tmp7.i161 to <2 x i32>
  %tmp2.i163 = shufflevector <2 x i32> %tmp6.i162, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %173 = bitcast <16 x i32> %tmp2.i163 to <16 x float>
  %174 = tail call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %173, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i164 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i165 = bitcast <16 x float> %174 to <16 x i32>
  %conv2.i.i.i166 = bitcast <8 x i64> %tmp5.i.i164 to <16 x i32>
  %175 = tail call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i165, <16 x i32> %conv2.i.i.i166) nounwind
  %176 = tail call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %175, <16 x i32> %conv2.i.i.i166) nounwind
  %177 = tail call i32 @llvm.x86.mic.kortestc(i16 %176, i16 %176) nounwind
  %tobool.i.i167 = icmp ne i32 %177, 0
  %cond.i.i168 = zext i1 %tobool.i.i167 to i32
  %temp.vect202 = insertelement <16 x i32> undef, i32 %cond.i.i, i32 0
  %temp.vect203 = insertelement <16 x i32> %temp.vect202, i32 %cond.i.i56, i32 1
  %temp.vect204 = insertelement <16 x i32> %temp.vect203, i32 %cond.i.i64, i32 2
  %temp.vect205 = insertelement <16 x i32> %temp.vect204, i32 %cond.i.i72, i32 3
  %temp.vect206 = insertelement <16 x i32> %temp.vect205, i32 %cond.i.i80, i32 4
  %temp.vect207 = insertelement <16 x i32> %temp.vect206, i32 %cond.i.i88, i32 5
  %temp.vect208 = insertelement <16 x i32> %temp.vect207, i32 %cond.i.i96, i32 6
  %temp.vect209 = insertelement <16 x i32> %temp.vect208, i32 %cond.i.i104, i32 7
  %temp.vect210 = insertelement <16 x i32> %temp.vect209, i32 %cond.i.i112, i32 8
  %temp.vect211 = insertelement <16 x i32> %temp.vect210, i32 %cond.i.i120, i32 9
  %temp.vect212 = insertelement <16 x i32> %temp.vect211, i32 %cond.i.i128, i32 10
  %temp.vect213 = insertelement <16 x i32> %temp.vect212, i32 %cond.i.i136, i32 11
  %temp.vect214 = insertelement <16 x i32> %temp.vect213, i32 %cond.i.i144, i32 12
  %temp.vect215 = insertelement <16 x i32> %temp.vect214, i32 %cond.i.i152, i32 13
  %temp.vect216 = insertelement <16 x i32> %temp.vect215, i32 %cond.i.i160, i32 14
  %temp.vect217 = insertelement <16 x i32> %temp.vect216, i32 %cond.i.i168, i32 15
  %178 = icmp eq <16 x i32> %temp.vect217, zeroinitializer
  %extract219 = extractelement <16 x i1> %178, i32 0
  %extract220 = extractelement <16 x i1> %178, i32 1
  %extract221 = extractelement <16 x i1> %178, i32 2
  %extract222 = extractelement <16 x i1> %178, i32 3
  %extract223 = extractelement <16 x i1> %178, i32 4
  %extract224 = extractelement <16 x i1> %178, i32 5
  %extract225 = extractelement <16 x i1> %178, i32 6
  %extract226 = extractelement <16 x i1> %178, i32 7
  %extract227 = extractelement <16 x i1> %178, i32 8
  %extract228 = extractelement <16 x i1> %178, i32 9
  %extract229 = extractelement <16 x i1> %178, i32 10
  %extract230 = extractelement <16 x i1> %178, i32 11
  %extract231 = extractelement <16 x i1> %178, i32 12
  %extract232 = extractelement <16 x i1> %178, i32 13
  %extract233 = extractelement <16 x i1> %178, i32 14
  %extract234 = extractelement <16 x i1> %178, i32 15
  br i1 %extract219, label %preload, label %postload

preload:                                          ; preds = %SyncBB
  %179 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %180 = bitcast <16 x float> %179 to <16 x i32>
  %tmp24.i169 = shufflevector <16 x i32> %180, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i170 = bitcast <2 x i32> %tmp24.i169 to <1 x double>
  %tmp31.i171 = extractelement <1 x double> %tmp32.i170, i32 0
  %phitmp483 = bitcast double %tmp31.i171 to i64
  %phitmp498 = bitcast i64 %phitmp483 to <2 x i32>
  %phitmp513 = bitcast <2 x i32> %phitmp498 to <1 x double>
  br label %postload

postload:                                         ; preds = %preload, %SyncBB
  %phi = phi <1 x double> [ undef, %SyncBB ], [ %phitmp513, %preload ]
  br i1 %extract220, label %preload391, label %postload392

preload391:                                       ; preds = %postload
  %181 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %182 = bitcast <16 x float> %181 to <16 x i32>
  %tmp24.i172 = shufflevector <16 x i32> %182, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i173 = bitcast <2 x i32> %tmp24.i172 to <1 x double>
  %tmp31.i174 = extractelement <1 x double> %tmp32.i173, i32 0
  %phitmp484 = bitcast double %tmp31.i174 to i64
  %phitmp499 = bitcast i64 %phitmp484 to <2 x i32>
  %phitmp514 = bitcast <2 x i32> %phitmp499 to <1 x double>
  br label %postload392

postload392:                                      ; preds = %preload391, %postload
  %phi393 = phi <1 x double> [ undef, %postload ], [ %phitmp514, %preload391 ]
  br i1 %extract221, label %preload397, label %postload398

preload397:                                       ; preds = %postload392
  %183 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %184 = bitcast <16 x float> %183 to <16 x i32>
  %tmp24.i175 = shufflevector <16 x i32> %184, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i176 = bitcast <2 x i32> %tmp24.i175 to <1 x double>
  %tmp31.i177 = extractelement <1 x double> %tmp32.i176, i32 0
  %phitmp485 = bitcast double %tmp31.i177 to i64
  %phitmp500 = bitcast i64 %phitmp485 to <2 x i32>
  %phitmp515 = bitcast <2 x i32> %phitmp500 to <1 x double>
  br label %postload398

postload398:                                      ; preds = %preload397, %postload392
  %phi399 = phi <1 x double> [ undef, %postload392 ], [ %phitmp515, %preload397 ]
  br i1 %extract222, label %preload403, label %postload404

preload403:                                       ; preds = %postload398
  %185 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %186 = bitcast <16 x float> %185 to <16 x i32>
  %tmp24.i178 = shufflevector <16 x i32> %186, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i179 = bitcast <2 x i32> %tmp24.i178 to <1 x double>
  %tmp31.i180 = extractelement <1 x double> %tmp32.i179, i32 0
  %phitmp486 = bitcast double %tmp31.i180 to i64
  %phitmp501 = bitcast i64 %phitmp486 to <2 x i32>
  %phitmp516 = bitcast <2 x i32> %phitmp501 to <1 x double>
  br label %postload404

postload404:                                      ; preds = %preload403, %postload398
  %phi405 = phi <1 x double> [ undef, %postload398 ], [ %phitmp516, %preload403 ]
  br i1 %extract223, label %preload409, label %postload410

preload409:                                       ; preds = %postload404
  %187 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %188 = bitcast <16 x float> %187 to <16 x i32>
  %tmp24.i181 = shufflevector <16 x i32> %188, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i182 = bitcast <2 x i32> %tmp24.i181 to <1 x double>
  %tmp31.i183 = extractelement <1 x double> %tmp32.i182, i32 0
  %phitmp487 = bitcast double %tmp31.i183 to i64
  %phitmp502 = bitcast i64 %phitmp487 to <2 x i32>
  %phitmp517 = bitcast <2 x i32> %phitmp502 to <1 x double>
  br label %postload410

postload410:                                      ; preds = %preload409, %postload404
  %phi411 = phi <1 x double> [ undef, %postload404 ], [ %phitmp517, %preload409 ]
  br i1 %extract224, label %preload415, label %postload416

preload415:                                       ; preds = %postload410
  %189 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %190 = bitcast <16 x float> %189 to <16 x i32>
  %tmp24.i184 = shufflevector <16 x i32> %190, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i185 = bitcast <2 x i32> %tmp24.i184 to <1 x double>
  %tmp31.i186 = extractelement <1 x double> %tmp32.i185, i32 0
  %phitmp488 = bitcast double %tmp31.i186 to i64
  %phitmp503 = bitcast i64 %phitmp488 to <2 x i32>
  %phitmp518 = bitcast <2 x i32> %phitmp503 to <1 x double>
  br label %postload416

postload416:                                      ; preds = %preload415, %postload410
  %phi417 = phi <1 x double> [ undef, %postload410 ], [ %phitmp518, %preload415 ]
  br i1 %extract225, label %preload421, label %postload422

preload421:                                       ; preds = %postload416
  %191 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %192 = bitcast <16 x float> %191 to <16 x i32>
  %tmp24.i187 = shufflevector <16 x i32> %192, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i188 = bitcast <2 x i32> %tmp24.i187 to <1 x double>
  %tmp31.i189 = extractelement <1 x double> %tmp32.i188, i32 0
  %phitmp489 = bitcast double %tmp31.i189 to i64
  %phitmp504 = bitcast i64 %phitmp489 to <2 x i32>
  %phitmp519 = bitcast <2 x i32> %phitmp504 to <1 x double>
  br label %postload422

postload422:                                      ; preds = %preload421, %postload416
  %phi423 = phi <1 x double> [ undef, %postload416 ], [ %phitmp519, %preload421 ]
  br i1 %extract226, label %preload427, label %postload428

preload427:                                       ; preds = %postload422
  %193 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %194 = bitcast <16 x float> %193 to <16 x i32>
  %tmp24.i190 = shufflevector <16 x i32> %194, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i191 = bitcast <2 x i32> %tmp24.i190 to <1 x double>
  %tmp31.i192 = extractelement <1 x double> %tmp32.i191, i32 0
  %phitmp490 = bitcast double %tmp31.i192 to i64
  %phitmp505 = bitcast i64 %phitmp490 to <2 x i32>
  %phitmp520 = bitcast <2 x i32> %phitmp505 to <1 x double>
  br label %postload428

postload428:                                      ; preds = %preload427, %postload422
  %phi429 = phi <1 x double> [ undef, %postload422 ], [ %phitmp520, %preload427 ]
  br i1 %extract227, label %preload433, label %postload434

preload433:                                       ; preds = %postload428
  %195 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %196 = bitcast <16 x float> %195 to <16 x i32>
  %tmp24.i193 = shufflevector <16 x i32> %196, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i194 = bitcast <2 x i32> %tmp24.i193 to <1 x double>
  %tmp31.i195 = extractelement <1 x double> %tmp32.i194, i32 0
  %phitmp491 = bitcast double %tmp31.i195 to i64
  %phitmp506 = bitcast i64 %phitmp491 to <2 x i32>
  %phitmp521 = bitcast <2 x i32> %phitmp506 to <1 x double>
  br label %postload434

postload434:                                      ; preds = %preload433, %postload428
  %phi435 = phi <1 x double> [ undef, %postload428 ], [ %phitmp521, %preload433 ]
  br i1 %extract228, label %preload439, label %postload440

preload439:                                       ; preds = %postload434
  %197 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %198 = bitcast <16 x float> %197 to <16 x i32>
  %tmp24.i196 = shufflevector <16 x i32> %198, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i197 = bitcast <2 x i32> %tmp24.i196 to <1 x double>
  %tmp31.i198 = extractelement <1 x double> %tmp32.i197, i32 0
  %phitmp492 = bitcast double %tmp31.i198 to i64
  %phitmp507 = bitcast i64 %phitmp492 to <2 x i32>
  %phitmp522 = bitcast <2 x i32> %phitmp507 to <1 x double>
  br label %postload440

postload440:                                      ; preds = %preload439, %postload434
  %phi441 = phi <1 x double> [ undef, %postload434 ], [ %phitmp522, %preload439 ]
  br i1 %extract229, label %preload445, label %postload446

preload445:                                       ; preds = %postload440
  %199 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %200 = bitcast <16 x float> %199 to <16 x i32>
  %tmp24.i199 = shufflevector <16 x i32> %200, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i200 = bitcast <2 x i32> %tmp24.i199 to <1 x double>
  %tmp31.i201 = extractelement <1 x double> %tmp32.i200, i32 0
  %phitmp493 = bitcast double %tmp31.i201 to i64
  %phitmp508 = bitcast i64 %phitmp493 to <2 x i32>
  %phitmp523 = bitcast <2 x i32> %phitmp508 to <1 x double>
  br label %postload446

postload446:                                      ; preds = %preload445, %postload440
  %phi447 = phi <1 x double> [ undef, %postload440 ], [ %phitmp523, %preload445 ]
  br i1 %extract230, label %preload451, label %postload452

preload451:                                       ; preds = %postload446
  %201 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %202 = bitcast <16 x float> %201 to <16 x i32>
  %tmp24.i202 = shufflevector <16 x i32> %202, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i203 = bitcast <2 x i32> %tmp24.i202 to <1 x double>
  %tmp31.i204 = extractelement <1 x double> %tmp32.i203, i32 0
  %phitmp494 = bitcast double %tmp31.i204 to i64
  %phitmp509 = bitcast i64 %phitmp494 to <2 x i32>
  %phitmp524 = bitcast <2 x i32> %phitmp509 to <1 x double>
  br label %postload452

postload452:                                      ; preds = %preload451, %postload446
  %phi453 = phi <1 x double> [ undef, %postload446 ], [ %phitmp524, %preload451 ]
  br i1 %extract231, label %preload457, label %postload458

preload457:                                       ; preds = %postload452
  %203 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %204 = bitcast <16 x float> %203 to <16 x i32>
  %tmp24.i205 = shufflevector <16 x i32> %204, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i206 = bitcast <2 x i32> %tmp24.i205 to <1 x double>
  %tmp31.i207 = extractelement <1 x double> %tmp32.i206, i32 0
  %phitmp495 = bitcast double %tmp31.i207 to i64
  %phitmp510 = bitcast i64 %phitmp495 to <2 x i32>
  %phitmp525 = bitcast <2 x i32> %phitmp510 to <1 x double>
  br label %postload458

postload458:                                      ; preds = %preload457, %postload452
  %phi459 = phi <1 x double> [ undef, %postload452 ], [ %phitmp525, %preload457 ]
  br i1 %extract232, label %preload463, label %postload464

preload463:                                       ; preds = %postload458
  %205 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %206 = bitcast <16 x float> %205 to <16 x i32>
  %tmp24.i208 = shufflevector <16 x i32> %206, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i209 = bitcast <2 x i32> %tmp24.i208 to <1 x double>
  %tmp31.i210 = extractelement <1 x double> %tmp32.i209, i32 0
  %phitmp496 = bitcast double %tmp31.i210 to i64
  %phitmp511 = bitcast i64 %phitmp496 to <2 x i32>
  %phitmp526 = bitcast <2 x i32> %phitmp511 to <1 x double>
  br label %postload464

postload464:                                      ; preds = %preload463, %postload458
  %phi465 = phi <1 x double> [ undef, %postload458 ], [ %phitmp526, %preload463 ]
  br i1 %extract233, label %preload469, label %postload470

preload469:                                       ; preds = %postload464
  %207 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %208 = bitcast <16 x float> %207 to <16 x i32>
  %tmp24.i211 = shufflevector <16 x i32> %208, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i212 = bitcast <2 x i32> %tmp24.i211 to <1 x double>
  %tmp31.i213 = extractelement <1 x double> %tmp32.i212, i32 0
  %phitmp497 = bitcast double %tmp31.i213 to i64
  %phitmp512 = bitcast i64 %phitmp497 to <2 x i32>
  %phitmp527 = bitcast <2 x i32> %phitmp512 to <1 x double>
  br label %postload470

postload470:                                      ; preds = %preload469, %postload464
  %phi471 = phi <1 x double> [ undef, %postload464 ], [ %phitmp527, %preload469 ]
  br i1 %extract234, label %preload475, label %postload476

preload475:                                       ; preds = %postload470
  %209 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %210 = bitcast <16 x float> %209 to <16 x i32>
  %tmp24.i214 = shufflevector <16 x i32> %210, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i215 = bitcast <2 x i32> %tmp24.i214 to <1 x double>
  %tmp31.i216 = extractelement <1 x double> %tmp32.i215, i32 0
  %phitmp = bitcast double %tmp31.i216 to i64
  %phitmp481 = bitcast i64 %phitmp to <2 x i32>
  %phitmp482 = bitcast <2 x i32> %phitmp481 to <1 x double>
  br label %postload476

postload476:                                      ; preds = %preload475, %postload470
  %phi477 = phi <1 x double> [ undef, %postload470 ], [ %phitmp482, %preload475 ]
  %211 = extractelement <1 x double> %phi393, i32 0
  %212 = extractelement <1 x double> %phi399, i32 0
  %213 = extractelement <1 x double> %phi405, i32 0
  %214 = extractelement <1 x double> %phi411, i32 0
  %215 = extractelement <1 x double> %phi417, i32 0
  %216 = extractelement <1 x double> %phi423, i32 0
  %217 = extractelement <1 x double> %phi429, i32 0
  %218 = extractelement <1 x double> %phi435, i32 0
  %219 = extractelement <1 x double> %phi441, i32 0
  %220 = extractelement <1 x double> %phi447, i32 0
  %221 = extractelement <1 x double> %phi453, i32 0
  %222 = extractelement <1 x double> %phi459, i32 0
  %223 = extractelement <1 x double> %phi465, i32 0
  %224 = extractelement <1 x double> %phi471, i32 0
  %225 = extractelement <1 x double> %phi477, i32 0
  br i1 %extract219, label %preload388, label %postload389

preload388:                                       ; preds = %postload476
  %226 = extractelement <1 x double> %phi, i32 0
  %tmp7.i217 = bitcast double %226 to i64
  %tmp6.i218 = bitcast i64 %tmp7.i217 to <2 x i32>
  %tmp2.i219 = shufflevector <2 x i32> %tmp6.i218, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %227 = bitcast <16 x i32> %tmp2.i219 to <16 x float>
  %228 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %227, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i220 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i221 = bitcast <16 x float> %228 to <16 x i32>
  %conv2.i.i.i222 = bitcast <8 x i64> %tmp5.i.i220 to <16 x i32>
  %229 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i221, <16 x i32> %conv2.i.i.i222) nounwind
  %230 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %229, <16 x i32> %conv2.i.i.i222) nounwind
  %231 = call i32 @llvm.x86.mic.kortestc(i16 %230, i16 %230) nounwind
  %tobool.i.i223 = icmp ne i32 %231, 0
  %cond.i.i224 = zext i1 %tobool.i.i223 to i32
  br label %postload389

postload389:                                      ; preds = %preload388, %postload476
  %phi390 = phi i32 [ undef, %postload476 ], [ %cond.i.i224, %preload388 ]
  br i1 %extract220, label %preload394, label %postload395

preload394:                                       ; preds = %postload389
  %tmp7.i225 = bitcast double %211 to i64
  %tmp6.i226 = bitcast i64 %tmp7.i225 to <2 x i32>
  %tmp2.i227 = shufflevector <2 x i32> %tmp6.i226, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %232 = bitcast <16 x i32> %tmp2.i227 to <16 x float>
  %233 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %232, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i228 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i229 = bitcast <16 x float> %233 to <16 x i32>
  %conv2.i.i.i230 = bitcast <8 x i64> %tmp5.i.i228 to <16 x i32>
  %234 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i229, <16 x i32> %conv2.i.i.i230) nounwind
  %235 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %234, <16 x i32> %conv2.i.i.i230) nounwind
  %236 = call i32 @llvm.x86.mic.kortestc(i16 %235, i16 %235) nounwind
  %tobool.i.i231 = icmp ne i32 %236, 0
  %cond.i.i232 = zext i1 %tobool.i.i231 to i32
  br label %postload395

postload395:                                      ; preds = %preload394, %postload389
  %phi396 = phi i32 [ undef, %postload389 ], [ %cond.i.i232, %preload394 ]
  br i1 %extract221, label %preload400, label %postload401

preload400:                                       ; preds = %postload395
  %tmp7.i233 = bitcast double %212 to i64
  %tmp6.i234 = bitcast i64 %tmp7.i233 to <2 x i32>
  %tmp2.i235 = shufflevector <2 x i32> %tmp6.i234, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %237 = bitcast <16 x i32> %tmp2.i235 to <16 x float>
  %238 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %237, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i236 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i237 = bitcast <16 x float> %238 to <16 x i32>
  %conv2.i.i.i238 = bitcast <8 x i64> %tmp5.i.i236 to <16 x i32>
  %239 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i237, <16 x i32> %conv2.i.i.i238) nounwind
  %240 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %239, <16 x i32> %conv2.i.i.i238) nounwind
  %241 = call i32 @llvm.x86.mic.kortestc(i16 %240, i16 %240) nounwind
  %tobool.i.i239 = icmp ne i32 %241, 0
  %cond.i.i240 = zext i1 %tobool.i.i239 to i32
  br label %postload401

postload401:                                      ; preds = %preload400, %postload395
  %phi402 = phi i32 [ undef, %postload395 ], [ %cond.i.i240, %preload400 ]
  br i1 %extract222, label %preload406, label %postload407

preload406:                                       ; preds = %postload401
  %tmp7.i241 = bitcast double %213 to i64
  %tmp6.i242 = bitcast i64 %tmp7.i241 to <2 x i32>
  %tmp2.i243 = shufflevector <2 x i32> %tmp6.i242, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %242 = bitcast <16 x i32> %tmp2.i243 to <16 x float>
  %243 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %242, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i244 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i245 = bitcast <16 x float> %243 to <16 x i32>
  %conv2.i.i.i246 = bitcast <8 x i64> %tmp5.i.i244 to <16 x i32>
  %244 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i245, <16 x i32> %conv2.i.i.i246) nounwind
  %245 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %244, <16 x i32> %conv2.i.i.i246) nounwind
  %246 = call i32 @llvm.x86.mic.kortestc(i16 %245, i16 %245) nounwind
  %tobool.i.i247 = icmp ne i32 %246, 0
  %cond.i.i248 = zext i1 %tobool.i.i247 to i32
  br label %postload407

postload407:                                      ; preds = %preload406, %postload401
  %phi408 = phi i32 [ undef, %postload401 ], [ %cond.i.i248, %preload406 ]
  br i1 %extract223, label %preload412, label %postload413

preload412:                                       ; preds = %postload407
  %tmp7.i249 = bitcast double %214 to i64
  %tmp6.i250 = bitcast i64 %tmp7.i249 to <2 x i32>
  %tmp2.i251 = shufflevector <2 x i32> %tmp6.i250, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %247 = bitcast <16 x i32> %tmp2.i251 to <16 x float>
  %248 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %247, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i252 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i253 = bitcast <16 x float> %248 to <16 x i32>
  %conv2.i.i.i254 = bitcast <8 x i64> %tmp5.i.i252 to <16 x i32>
  %249 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i253, <16 x i32> %conv2.i.i.i254) nounwind
  %250 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %249, <16 x i32> %conv2.i.i.i254) nounwind
  %251 = call i32 @llvm.x86.mic.kortestc(i16 %250, i16 %250) nounwind
  %tobool.i.i255 = icmp ne i32 %251, 0
  %cond.i.i256 = zext i1 %tobool.i.i255 to i32
  br label %postload413

postload413:                                      ; preds = %preload412, %postload407
  %phi414 = phi i32 [ undef, %postload407 ], [ %cond.i.i256, %preload412 ]
  br i1 %extract224, label %preload418, label %postload419

preload418:                                       ; preds = %postload413
  %tmp7.i257 = bitcast double %215 to i64
  %tmp6.i258 = bitcast i64 %tmp7.i257 to <2 x i32>
  %tmp2.i259 = shufflevector <2 x i32> %tmp6.i258, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %252 = bitcast <16 x i32> %tmp2.i259 to <16 x float>
  %253 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %252, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i260 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i261 = bitcast <16 x float> %253 to <16 x i32>
  %conv2.i.i.i262 = bitcast <8 x i64> %tmp5.i.i260 to <16 x i32>
  %254 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i261, <16 x i32> %conv2.i.i.i262) nounwind
  %255 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %254, <16 x i32> %conv2.i.i.i262) nounwind
  %256 = call i32 @llvm.x86.mic.kortestc(i16 %255, i16 %255) nounwind
  %tobool.i.i263 = icmp ne i32 %256, 0
  %cond.i.i264 = zext i1 %tobool.i.i263 to i32
  br label %postload419

postload419:                                      ; preds = %preload418, %postload413
  %phi420 = phi i32 [ undef, %postload413 ], [ %cond.i.i264, %preload418 ]
  br i1 %extract225, label %preload424, label %postload425

preload424:                                       ; preds = %postload419
  %tmp7.i265 = bitcast double %216 to i64
  %tmp6.i266 = bitcast i64 %tmp7.i265 to <2 x i32>
  %tmp2.i267 = shufflevector <2 x i32> %tmp6.i266, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %257 = bitcast <16 x i32> %tmp2.i267 to <16 x float>
  %258 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %257, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i268 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i269 = bitcast <16 x float> %258 to <16 x i32>
  %conv2.i.i.i270 = bitcast <8 x i64> %tmp5.i.i268 to <16 x i32>
  %259 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i269, <16 x i32> %conv2.i.i.i270) nounwind
  %260 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %259, <16 x i32> %conv2.i.i.i270) nounwind
  %261 = call i32 @llvm.x86.mic.kortestc(i16 %260, i16 %260) nounwind
  %tobool.i.i271 = icmp ne i32 %261, 0
  %cond.i.i272 = zext i1 %tobool.i.i271 to i32
  br label %postload425

postload425:                                      ; preds = %preload424, %postload419
  %phi426 = phi i32 [ undef, %postload419 ], [ %cond.i.i272, %preload424 ]
  br i1 %extract226, label %preload430, label %postload431

preload430:                                       ; preds = %postload425
  %tmp7.i273 = bitcast double %217 to i64
  %tmp6.i274 = bitcast i64 %tmp7.i273 to <2 x i32>
  %tmp2.i275 = shufflevector <2 x i32> %tmp6.i274, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %262 = bitcast <16 x i32> %tmp2.i275 to <16 x float>
  %263 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %262, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i276 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i277 = bitcast <16 x float> %263 to <16 x i32>
  %conv2.i.i.i278 = bitcast <8 x i64> %tmp5.i.i276 to <16 x i32>
  %264 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i277, <16 x i32> %conv2.i.i.i278) nounwind
  %265 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %264, <16 x i32> %conv2.i.i.i278) nounwind
  %266 = call i32 @llvm.x86.mic.kortestc(i16 %265, i16 %265) nounwind
  %tobool.i.i279 = icmp ne i32 %266, 0
  %cond.i.i280 = zext i1 %tobool.i.i279 to i32
  br label %postload431

postload431:                                      ; preds = %preload430, %postload425
  %phi432 = phi i32 [ undef, %postload425 ], [ %cond.i.i280, %preload430 ]
  br i1 %extract227, label %preload436, label %postload437

preload436:                                       ; preds = %postload431
  %tmp7.i281 = bitcast double %218 to i64
  %tmp6.i282 = bitcast i64 %tmp7.i281 to <2 x i32>
  %tmp2.i283 = shufflevector <2 x i32> %tmp6.i282, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %267 = bitcast <16 x i32> %tmp2.i283 to <16 x float>
  %268 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %267, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i284 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i285 = bitcast <16 x float> %268 to <16 x i32>
  %conv2.i.i.i286 = bitcast <8 x i64> %tmp5.i.i284 to <16 x i32>
  %269 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i285, <16 x i32> %conv2.i.i.i286) nounwind
  %270 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %269, <16 x i32> %conv2.i.i.i286) nounwind
  %271 = call i32 @llvm.x86.mic.kortestc(i16 %270, i16 %270) nounwind
  %tobool.i.i287 = icmp ne i32 %271, 0
  %cond.i.i288 = zext i1 %tobool.i.i287 to i32
  br label %postload437

postload437:                                      ; preds = %preload436, %postload431
  %phi438 = phi i32 [ undef, %postload431 ], [ %cond.i.i288, %preload436 ]
  br i1 %extract228, label %preload442, label %postload443

preload442:                                       ; preds = %postload437
  %tmp7.i289 = bitcast double %219 to i64
  %tmp6.i290 = bitcast i64 %tmp7.i289 to <2 x i32>
  %tmp2.i291 = shufflevector <2 x i32> %tmp6.i290, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %272 = bitcast <16 x i32> %tmp2.i291 to <16 x float>
  %273 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %272, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i292 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i293 = bitcast <16 x float> %273 to <16 x i32>
  %conv2.i.i.i294 = bitcast <8 x i64> %tmp5.i.i292 to <16 x i32>
  %274 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i293, <16 x i32> %conv2.i.i.i294) nounwind
  %275 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %274, <16 x i32> %conv2.i.i.i294) nounwind
  %276 = call i32 @llvm.x86.mic.kortestc(i16 %275, i16 %275) nounwind
  %tobool.i.i295 = icmp ne i32 %276, 0
  %cond.i.i296 = zext i1 %tobool.i.i295 to i32
  br label %postload443

postload443:                                      ; preds = %preload442, %postload437
  %phi444 = phi i32 [ undef, %postload437 ], [ %cond.i.i296, %preload442 ]
  br i1 %extract229, label %preload448, label %postload449

preload448:                                       ; preds = %postload443
  %tmp7.i297 = bitcast double %220 to i64
  %tmp6.i298 = bitcast i64 %tmp7.i297 to <2 x i32>
  %tmp2.i299 = shufflevector <2 x i32> %tmp6.i298, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %277 = bitcast <16 x i32> %tmp2.i299 to <16 x float>
  %278 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %277, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i300 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i301 = bitcast <16 x float> %278 to <16 x i32>
  %conv2.i.i.i302 = bitcast <8 x i64> %tmp5.i.i300 to <16 x i32>
  %279 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i301, <16 x i32> %conv2.i.i.i302) nounwind
  %280 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %279, <16 x i32> %conv2.i.i.i302) nounwind
  %281 = call i32 @llvm.x86.mic.kortestc(i16 %280, i16 %280) nounwind
  %tobool.i.i303 = icmp ne i32 %281, 0
  %cond.i.i304 = zext i1 %tobool.i.i303 to i32
  br label %postload449

postload449:                                      ; preds = %preload448, %postload443
  %phi450 = phi i32 [ undef, %postload443 ], [ %cond.i.i304, %preload448 ]
  br i1 %extract230, label %preload454, label %postload455

preload454:                                       ; preds = %postload449
  %tmp7.i305 = bitcast double %221 to i64
  %tmp6.i306 = bitcast i64 %tmp7.i305 to <2 x i32>
  %tmp2.i307 = shufflevector <2 x i32> %tmp6.i306, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %282 = bitcast <16 x i32> %tmp2.i307 to <16 x float>
  %283 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %282, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i308 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i309 = bitcast <16 x float> %283 to <16 x i32>
  %conv2.i.i.i310 = bitcast <8 x i64> %tmp5.i.i308 to <16 x i32>
  %284 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i309, <16 x i32> %conv2.i.i.i310) nounwind
  %285 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %284, <16 x i32> %conv2.i.i.i310) nounwind
  %286 = call i32 @llvm.x86.mic.kortestc(i16 %285, i16 %285) nounwind
  %tobool.i.i311 = icmp ne i32 %286, 0
  %cond.i.i312 = zext i1 %tobool.i.i311 to i32
  br label %postload455

postload455:                                      ; preds = %preload454, %postload449
  %phi456 = phi i32 [ undef, %postload449 ], [ %cond.i.i312, %preload454 ]
  br i1 %extract231, label %preload460, label %postload461

preload460:                                       ; preds = %postload455
  %tmp7.i313 = bitcast double %222 to i64
  %tmp6.i314 = bitcast i64 %tmp7.i313 to <2 x i32>
  %tmp2.i315 = shufflevector <2 x i32> %tmp6.i314, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %287 = bitcast <16 x i32> %tmp2.i315 to <16 x float>
  %288 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %287, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i316 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i317 = bitcast <16 x float> %288 to <16 x i32>
  %conv2.i.i.i318 = bitcast <8 x i64> %tmp5.i.i316 to <16 x i32>
  %289 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i317, <16 x i32> %conv2.i.i.i318) nounwind
  %290 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %289, <16 x i32> %conv2.i.i.i318) nounwind
  %291 = call i32 @llvm.x86.mic.kortestc(i16 %290, i16 %290) nounwind
  %tobool.i.i319 = icmp ne i32 %291, 0
  %cond.i.i320 = zext i1 %tobool.i.i319 to i32
  br label %postload461

postload461:                                      ; preds = %preload460, %postload455
  %phi462 = phi i32 [ undef, %postload455 ], [ %cond.i.i320, %preload460 ]
  br i1 %extract232, label %preload466, label %postload467

preload466:                                       ; preds = %postload461
  %tmp7.i321 = bitcast double %223 to i64
  %tmp6.i322 = bitcast i64 %tmp7.i321 to <2 x i32>
  %tmp2.i323 = shufflevector <2 x i32> %tmp6.i322, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %292 = bitcast <16 x i32> %tmp2.i323 to <16 x float>
  %293 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %292, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i324 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i325 = bitcast <16 x float> %293 to <16 x i32>
  %conv2.i.i.i326 = bitcast <8 x i64> %tmp5.i.i324 to <16 x i32>
  %294 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i325, <16 x i32> %conv2.i.i.i326) nounwind
  %295 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %294, <16 x i32> %conv2.i.i.i326) nounwind
  %296 = call i32 @llvm.x86.mic.kortestc(i16 %295, i16 %295) nounwind
  %tobool.i.i327 = icmp ne i32 %296, 0
  %cond.i.i328 = zext i1 %tobool.i.i327 to i32
  br label %postload467

postload467:                                      ; preds = %preload466, %postload461
  %phi468 = phi i32 [ undef, %postload461 ], [ %cond.i.i328, %preload466 ]
  br i1 %extract233, label %preload472, label %postload473

preload472:                                       ; preds = %postload467
  %tmp7.i329 = bitcast double %224 to i64
  %tmp6.i330 = bitcast i64 %tmp7.i329 to <2 x i32>
  %tmp2.i331 = shufflevector <2 x i32> %tmp6.i330, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %297 = bitcast <16 x i32> %tmp2.i331 to <16 x float>
  %298 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %297, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i332 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i333 = bitcast <16 x float> %298 to <16 x i32>
  %conv2.i.i.i334 = bitcast <8 x i64> %tmp5.i.i332 to <16 x i32>
  %299 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i333, <16 x i32> %conv2.i.i.i334) nounwind
  %300 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %299, <16 x i32> %conv2.i.i.i334) nounwind
  %301 = call i32 @llvm.x86.mic.kortestc(i16 %300, i16 %300) nounwind
  %tobool.i.i335 = icmp ne i32 %301, 0
  %cond.i.i336 = zext i1 %tobool.i.i335 to i32
  br label %postload473

postload473:                                      ; preds = %preload472, %postload467
  %phi474 = phi i32 [ undef, %postload467 ], [ %cond.i.i336, %preload472 ]
  br i1 %extract234, label %preload478, label %postload479

preload478:                                       ; preds = %postload473
  %tmp7.i337 = bitcast double %225 to i64
  %tmp6.i338 = bitcast i64 %tmp7.i337 to <2 x i32>
  %tmp2.i339 = shufflevector <2 x i32> %tmp6.i338, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %302 = bitcast <16 x i32> %tmp2.i339 to <16 x float>
  %303 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %302, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i340 = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i341 = bitcast <16 x float> %303 to <16 x i32>
  %conv2.i.i.i342 = bitcast <8 x i64> %tmp5.i.i340 to <16 x i32>
  %304 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i341, <16 x i32> %conv2.i.i.i342) nounwind
  %305 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %304, <16 x i32> %conv2.i.i.i342) nounwind
  %306 = call i32 @llvm.x86.mic.kortestc(i16 %305, i16 %305) nounwind
  %tobool.i.i343 = icmp ne i32 %306, 0
  %cond.i.i344 = zext i1 %tobool.i.i343 to i32
  br label %postload479

postload479:                                      ; preds = %preload478, %postload473
  %phi480 = phi i32 [ undef, %postload473 ], [ %cond.i.i344, %preload478 ]
  %temp.vect268 = insertelement <16 x i32> undef, i32 %phi390, i32 0
  %temp.vect269 = insertelement <16 x i32> %temp.vect268, i32 %phi396, i32 1
  %temp.vect270 = insertelement <16 x i32> %temp.vect269, i32 %phi402, i32 2
  %temp.vect271 = insertelement <16 x i32> %temp.vect270, i32 %phi408, i32 3
  %temp.vect272 = insertelement <16 x i32> %temp.vect271, i32 %phi414, i32 4
  %temp.vect273 = insertelement <16 x i32> %temp.vect272, i32 %phi420, i32 5
  %temp.vect274 = insertelement <16 x i32> %temp.vect273, i32 %phi426, i32 6
  %temp.vect275 = insertelement <16 x i32> %temp.vect274, i32 %phi432, i32 7
  %temp.vect276 = insertelement <16 x i32> %temp.vect275, i32 %phi438, i32 8
  %temp.vect277 = insertelement <16 x i32> %temp.vect276, i32 %phi444, i32 9
  %temp.vect278 = insertelement <16 x i32> %temp.vect277, i32 %phi450, i32 10
  %temp.vect279 = insertelement <16 x i32> %temp.vect278, i32 %phi456, i32 11
  %temp.vect280 = insertelement <16 x i32> %temp.vect279, i32 %phi462, i32 12
  %temp.vect281 = insertelement <16 x i32> %temp.vect280, i32 %phi468, i32 13
  %temp.vect282 = insertelement <16 x i32> %temp.vect281, i32 %phi474, i32 14
  %temp.vect283 = insertelement <16 x i32> %temp.vect282, i32 %phi480, i32 15
  %307 = icmp ne <16 x i32> %temp.vect283, zeroinitializer
  %phitmp.i20288 = select <16 x i1> %307, <16 x float> %vector285, <16 x float> %vector287
  %phitmp.i21293 = select <16 x i1> %307, <16 x float> %vector290, <16 x float> %vector292
  %phitmp.i22298 = select <16 x i1> %307, <16 x float> %vector295, <16 x float> %vector297
  %phitmp.i23303 = select <16 x i1> %307, <16 x float> %vector300, <16 x float> %vector302
  %merge51304 = select <16 x i1> %178, <16 x float> %phitmp.i23303, <16 x float> %vector300
  %extract356 = extractelement <16 x float> %merge51304, i32 0
  %extract357 = extractelement <16 x float> %merge51304, i32 1
  %extract358 = extractelement <16 x float> %merge51304, i32 2
  %extract359 = extractelement <16 x float> %merge51304, i32 3
  %extract360 = extractelement <16 x float> %merge51304, i32 4
  %extract361 = extractelement <16 x float> %merge51304, i32 5
  %extract362 = extractelement <16 x float> %merge51304, i32 6
  %extract363 = extractelement <16 x float> %merge51304, i32 7
  %extract364 = extractelement <16 x float> %merge51304, i32 8
  %extract365 = extractelement <16 x float> %merge51304, i32 9
  %extract366 = extractelement <16 x float> %merge51304, i32 10
  %extract367 = extractelement <16 x float> %merge51304, i32 11
  %extract368 = extractelement <16 x float> %merge51304, i32 12
  %extract369 = extractelement <16 x float> %merge51304, i32 13
  %extract370 = extractelement <16 x float> %merge51304, i32 14
  %extract371 = extractelement <16 x float> %merge51304, i32 15
  %merge49305 = select <16 x i1> %178, <16 x float> %phitmp.i22298, <16 x float> %vector295
  %extract340 = extractelement <16 x float> %merge49305, i32 0
  %extract341 = extractelement <16 x float> %merge49305, i32 1
  %extract342 = extractelement <16 x float> %merge49305, i32 2
  %extract343 = extractelement <16 x float> %merge49305, i32 3
  %extract344 = extractelement <16 x float> %merge49305, i32 4
  %extract345 = extractelement <16 x float> %merge49305, i32 5
  %extract346 = extractelement <16 x float> %merge49305, i32 6
  %extract347 = extractelement <16 x float> %merge49305, i32 7
  %extract348 = extractelement <16 x float> %merge49305, i32 8
  %extract349 = extractelement <16 x float> %merge49305, i32 9
  %extract350 = extractelement <16 x float> %merge49305, i32 10
  %extract351 = extractelement <16 x float> %merge49305, i32 11
  %extract352 = extractelement <16 x float> %merge49305, i32 12
  %extract353 = extractelement <16 x float> %merge49305, i32 13
  %extract354 = extractelement <16 x float> %merge49305, i32 14
  %extract355 = extractelement <16 x float> %merge49305, i32 15
  %merge47306 = select <16 x i1> %178, <16 x float> %phitmp.i21293, <16 x float> %vector290
  %extract324 = extractelement <16 x float> %merge47306, i32 0
  %extract325 = extractelement <16 x float> %merge47306, i32 1
  %extract326 = extractelement <16 x float> %merge47306, i32 2
  %extract327 = extractelement <16 x float> %merge47306, i32 3
  %extract328 = extractelement <16 x float> %merge47306, i32 4
  %extract329 = extractelement <16 x float> %merge47306, i32 5
  %extract330 = extractelement <16 x float> %merge47306, i32 6
  %extract331 = extractelement <16 x float> %merge47306, i32 7
  %extract332 = extractelement <16 x float> %merge47306, i32 8
  %extract333 = extractelement <16 x float> %merge47306, i32 9
  %extract334 = extractelement <16 x float> %merge47306, i32 10
  %extract335 = extractelement <16 x float> %merge47306, i32 11
  %extract336 = extractelement <16 x float> %merge47306, i32 12
  %extract337 = extractelement <16 x float> %merge47306, i32 13
  %extract338 = extractelement <16 x float> %merge47306, i32 14
  %extract339 = extractelement <16 x float> %merge47306, i32 15
  %merge307 = select <16 x i1> %178, <16 x float> %phitmp.i20288, <16 x float> %vector285
  %extract308 = extractelement <16 x float> %merge307, i32 0
  %extract309 = extractelement <16 x float> %merge307, i32 1
  %extract310 = extractelement <16 x float> %merge307, i32 2
  %extract311 = extractelement <16 x float> %merge307, i32 3
  %extract312 = extractelement <16 x float> %merge307, i32 4
  %extract313 = extractelement <16 x float> %merge307, i32 5
  %extract314 = extractelement <16 x float> %merge307, i32 6
  %extract315 = extractelement <16 x float> %merge307, i32 7
  %extract316 = extractelement <16 x float> %merge307, i32 8
  %extract317 = extractelement <16 x float> %merge307, i32 9
  %extract318 = extractelement <16 x float> %merge307, i32 10
  %extract319 = extractelement <16 x float> %merge307, i32 11
  %extract320 = extractelement <16 x float> %merge307, i32 12
  %extract321 = extractelement <16 x float> %merge307, i32 13
  %extract322 = extractelement <16 x float> %merge307, i32 14
  %extract323 = extractelement <16 x float> %merge307, i32 15
  %308 = insertelement <4 x float> undef, float %extract308, i32 0
  %309 = insertelement <4 x float> undef, float %extract309, i32 0
  %310 = insertelement <4 x float> undef, float %extract310, i32 0
  %311 = insertelement <4 x float> undef, float %extract311, i32 0
  %312 = insertelement <4 x float> undef, float %extract312, i32 0
  %313 = insertelement <4 x float> undef, float %extract313, i32 0
  %314 = insertelement <4 x float> undef, float %extract314, i32 0
  %315 = insertelement <4 x float> undef, float %extract315, i32 0
  %316 = insertelement <4 x float> undef, float %extract316, i32 0
  %317 = insertelement <4 x float> undef, float %extract317, i32 0
  %318 = insertelement <4 x float> undef, float %extract318, i32 0
  %319 = insertelement <4 x float> undef, float %extract319, i32 0
  %320 = insertelement <4 x float> undef, float %extract320, i32 0
  %321 = insertelement <4 x float> undef, float %extract321, i32 0
  %322 = insertelement <4 x float> undef, float %extract322, i32 0
  %323 = insertelement <4 x float> undef, float %extract323, i32 0
  %324 = insertelement <4 x float> %308, float %extract324, i32 1
  %325 = insertelement <4 x float> %309, float %extract325, i32 1
  %326 = insertelement <4 x float> %310, float %extract326, i32 1
  %327 = insertelement <4 x float> %311, float %extract327, i32 1
  %328 = insertelement <4 x float> %312, float %extract328, i32 1
  %329 = insertelement <4 x float> %313, float %extract329, i32 1
  %330 = insertelement <4 x float> %314, float %extract330, i32 1
  %331 = insertelement <4 x float> %315, float %extract331, i32 1
  %332 = insertelement <4 x float> %316, float %extract332, i32 1
  %333 = insertelement <4 x float> %317, float %extract333, i32 1
  %334 = insertelement <4 x float> %318, float %extract334, i32 1
  %335 = insertelement <4 x float> %319, float %extract335, i32 1
  %336 = insertelement <4 x float> %320, float %extract336, i32 1
  %337 = insertelement <4 x float> %321, float %extract337, i32 1
  %338 = insertelement <4 x float> %322, float %extract338, i32 1
  %339 = insertelement <4 x float> %323, float %extract339, i32 1
  %340 = insertelement <4 x float> %324, float %extract340, i32 2
  %341 = insertelement <4 x float> %325, float %extract341, i32 2
  %342 = insertelement <4 x float> %326, float %extract342, i32 2
  %343 = insertelement <4 x float> %327, float %extract343, i32 2
  %344 = insertelement <4 x float> %328, float %extract344, i32 2
  %345 = insertelement <4 x float> %329, float %extract345, i32 2
  %346 = insertelement <4 x float> %330, float %extract346, i32 2
  %347 = insertelement <4 x float> %331, float %extract347, i32 2
  %348 = insertelement <4 x float> %332, float %extract348, i32 2
  %349 = insertelement <4 x float> %333, float %extract349, i32 2
  %350 = insertelement <4 x float> %334, float %extract350, i32 2
  %351 = insertelement <4 x float> %335, float %extract351, i32 2
  %352 = insertelement <4 x float> %336, float %extract352, i32 2
  %353 = insertelement <4 x float> %337, float %extract353, i32 2
  %354 = insertelement <4 x float> %338, float %extract354, i32 2
  %355 = insertelement <4 x float> %339, float %extract355, i32 2
  %356 = insertelement <4 x float> %340, float %extract356, i32 3
  %357 = insertelement <4 x float> %341, float %extract357, i32 3
  %358 = insertelement <4 x float> %342, float %extract358, i32 3
  %359 = insertelement <4 x float> %343, float %extract359, i32 3
  %360 = insertelement <4 x float> %344, float %extract360, i32 3
  %361 = insertelement <4 x float> %345, float %extract361, i32 3
  %362 = insertelement <4 x float> %346, float %extract362, i32 3
  %363 = insertelement <4 x float> %347, float %extract363, i32 3
  %364 = insertelement <4 x float> %348, float %extract364, i32 3
  %365 = insertelement <4 x float> %349, float %extract365, i32 3
  %366 = insertelement <4 x float> %350, float %extract366, i32 3
  %367 = insertelement <4 x float> %351, float %extract367, i32 3
  %368 = insertelement <4 x float> %352, float %extract368, i32 3
  %369 = insertelement <4 x float> %353, float %extract369, i32 3
  %370 = insertelement <4 x float> %354, float %extract370, i32 3
  %371 = insertelement <4 x float> %355, float %extract371, i32 3
  %372 = extractelement <16 x i32> %17, i32 0
  %373 = sext i32 %372 to i64
  %374 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %373
  %375 = extractelement <16 x i32> %17, i32 1
  %376 = sext i32 %375 to i64
  %377 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %376
  %378 = extractelement <16 x i32> %17, i32 2
  %379 = sext i32 %378 to i64
  %380 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %379
  %381 = extractelement <16 x i32> %17, i32 3
  %382 = sext i32 %381 to i64
  %383 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %382
  %384 = extractelement <16 x i32> %17, i32 4
  %385 = sext i32 %384 to i64
  %386 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %385
  %387 = extractelement <16 x i32> %17, i32 5
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %388
  %390 = extractelement <16 x i32> %17, i32 6
  %391 = sext i32 %390 to i64
  %392 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %391
  %393 = extractelement <16 x i32> %17, i32 7
  %394 = sext i32 %393 to i64
  %395 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %394
  %396 = extractelement <16 x i32> %17, i32 8
  %397 = sext i32 %396 to i64
  %398 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %397
  %399 = extractelement <16 x i32> %17, i32 9
  %400 = sext i32 %399 to i64
  %401 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %400
  %402 = extractelement <16 x i32> %17, i32 10
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %403
  %405 = extractelement <16 x i32> %17, i32 11
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %406
  %408 = extractelement <16 x i32> %17, i32 12
  %409 = sext i32 %408 to i64
  %410 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %409
  %411 = extractelement <16 x i32> %17, i32 13
  %412 = sext i32 %411 to i64
  %413 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %412
  %414 = extractelement <16 x i32> %17, i32 14
  %415 = sext i32 %414 to i64
  %416 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %415
  %417 = extractelement <16 x i32> %17, i32 15
  %418 = sext i32 %417 to i64
  %419 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %418
  store <4 x float> %356, <4 x float> addrspace(1)* %374, align 16
  store <4 x float> %357, <4 x float> addrspace(1)* %377, align 16
  store <4 x float> %358, <4 x float> addrspace(1)* %380, align 16
  store <4 x float> %359, <4 x float> addrspace(1)* %383, align 16
  store <4 x float> %360, <4 x float> addrspace(1)* %386, align 16
  store <4 x float> %361, <4 x float> addrspace(1)* %389, align 16
  store <4 x float> %362, <4 x float> addrspace(1)* %392, align 16
  store <4 x float> %363, <4 x float> addrspace(1)* %395, align 16
  store <4 x float> %364, <4 x float> addrspace(1)* %398, align 16
  store <4 x float> %365, <4 x float> addrspace(1)* %401, align 16
  store <4 x float> %366, <4 x float> addrspace(1)* %404, align 16
  store <4 x float> %367, <4 x float> addrspace(1)* %407, align 16
  store <4 x float> %368, <4 x float> addrspace(1)* %410, align 16
  store <4 x float> %369, <4 x float> addrspace(1)* %413, align 16
  store <4 x float> %370, <4 x float> addrspace(1)* %416, align 16
  store <4 x float> %371, <4 x float> addrspace(1)* %419, align 16
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB528

thenBB:                                           ; preds = %postload479
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB528:                                        ; preds = %postload479
  ret void
}

declare double @__ocl_svml_b1_floorf2(double) readnone

declare double @__ocl_svml_b1_fmodf2(double, double) readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i16 @llvm.x86.mic.cmpeq.pi(<16 x i32>, <16 x i32>) nounwind readnone

declare <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float>, i16, i8*, i32, i32, i32) nounwind readonly

declare <16 x i32> @llvm.x86.mic.and.pi(<16 x i32>, <16 x i32>) nounwind readnone

declare <16 x float> @llvm.x86.mic.zero.ps() nounwind readnone

declare float @__ocl_svml_b1_floorf1(float) readnone

declare float @__ocl_svml_b1_fmodf1(float, float) readnone

declare <16 x float> @__ocl_svml_b1_floorf16(<16 x float>) readnone

declare <16 x float> @__ocl_svml_b1_fmodf16(<16 x float>, <16 x float>) readnone

define void @checkerboard2D(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to <4 x float>*
  %4 = load <4 x float>* %3, align 16
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to <4 x float>*
  %7 = load <4 x float>* %6, align 16
  %8 = getelementptr i8* %pBuffer, i64 56
  %9 = bitcast i8* %8 to %struct.WorkDim**
  %10 = load %struct.WorkDim** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 72
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 80
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 96
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  br label %SyncBB9.i

SyncBB9.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %20 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = add i64 %21, %23
  %25 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %26 = load i64* %25, align 8
  %27 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %28 = load i64* %27, align 8
  %29 = add i64 %26, %28
  %30 = getelementptr %struct.WorkDim* %10, i64 0, i32 2, i64 0
  %31 = load i64* %30, align 8
  %32 = mul i64 %31, %29
  %33 = add i64 %32, %24
  %34 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %35 = bitcast <16 x float> %34 to <16 x i32>
  %tmp24.i11.i = shufflevector <16 x i32> %35, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i12.i = bitcast <2 x i32> %tmp24.i11.i to <1 x double>
  %tmp31.i13.i = extractelement <1 x double> %tmp32.i12.i, i32 0
  %tmp16.i.i = bitcast double %tmp31.i13.i to i64
  %tmp15.i.i = bitcast i64 %tmp16.i.i to <2 x i32>
  %tmp13.i.i = bitcast <2 x i32> %tmp15.i.i to <1 x double>
  %tmp12.i.i = extractelement <1 x double> %tmp13.i.i, i32 0
  %tmp7.i14.i = bitcast double %tmp12.i.i to i64
  %tmp6.i.i = bitcast i64 %tmp7.i14.i to <2 x i32>
  %tmp2.i15.i = shufflevector <2 x i32> %tmp6.i.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %36 = bitcast <16 x i32> %tmp2.i15.i to <16 x float>
  %37 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %36, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i.i = bitcast <16 x float> %37 to <16 x i32>
  %conv2.i.i.i.i = bitcast <8 x i64> %tmp5.i.i.i to <16 x i32>
  %38 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i.i, <16 x i32> %conv2.i.i.i.i) nounwind
  %39 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %38, <16 x i32> %conv2.i.i.i.i) nounwind
  %40 = call i32 @llvm.x86.mic.kortestc(i16 %39, i16 %39) nounwind
  %tobool.i.i.i = icmp eq i32 %40, 0
  br i1 %tobool.i.i.i, label %41, label %evaluatePixel.exit.i

; <label>:41                                      ; preds = %SyncBB9.i
  %42 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %43 = bitcast <16 x float> %42 to <16 x i32>
  %tmp24.i16.i = shufflevector <16 x i32> %43, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i17.i = bitcast <2 x i32> %tmp24.i16.i to <1 x double>
  %tmp31.i18.i = extractelement <1 x double> %tmp32.i17.i, i32 0
  %tmp5.i.i = bitcast double %tmp31.i18.i to i64
  %tmp4.i.i = bitcast i64 %tmp5.i.i to <2 x i32>
  %tmp2.i.i = bitcast <2 x i32> %tmp4.i.i to <1 x double>
  %tmp1.i.i = extractelement <1 x double> %tmp2.i.i, i32 0
  %tmp7.i19.i = bitcast double %tmp1.i.i to i64
  %tmp6.i20.i = bitcast i64 %tmp7.i19.i to <2 x i32>
  %tmp2.i21.i = shufflevector <2 x i32> %tmp6.i20.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %44 = bitcast <16 x i32> %tmp2.i21.i to <16 x float>
  %45 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %44, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i22.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i23.i = bitcast <16 x float> %45 to <16 x i32>
  %conv2.i.i.i24.i = bitcast <8 x i64> %tmp5.i.i22.i to <16 x i32>
  %46 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i23.i, <16 x i32> %conv2.i.i.i24.i) nounwind
  %47 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %46, <16 x i32> %conv2.i.i.i24.i) nounwind
  %48 = call i32 @llvm.x86.mic.kortestc(i16 %47, i16 %47) nounwind
  %tobool.i.i25.i = icmp ne i32 %48, 0
  %phitmp.i.i = select i1 %tobool.i.i25.i, <4 x float> %4, <4 x float> %7
  br label %evaluatePixel.exit.i

evaluatePixel.exit.i:                             ; preds = %41, %SyncBB9.i
  %UnifiedRetVal.i.i = phi <4 x float> [ %phitmp.i.i, %41 ], [ %4, %SyncBB9.i ]
  %sext.i = shl i64 %33, 32
  %49 = ashr i64 %sext.i, 32
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %49
  store <4 x float> %UnifiedRetVal.i.i, <4 x float> addrspace(1)* %50, align 16
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__checkerboard2D_separated_args.exit

thenBB.i:                                         ; preds = %evaluatePixel.exit.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB9.i

__checkerboard2D_separated_args.exit:             ; preds = %evaluatePixel.exit.i
  ret void
}

define void @__Vectorized_.checkerboard2D(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to <4 x float>*
  %4 = load <4 x float>* %3, align 16
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to <4 x float>*
  %7 = load <4 x float>* %6, align 16
  %8 = getelementptr i8* %pBuffer, i64 56
  %9 = bitcast i8* %8 to %struct.WorkDim**
  %10 = load %struct.WorkDim** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 72
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 80
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 96
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %scalar16.i = extractelement <4 x float> %7, i32 0
  %temp286.i = insertelement <16 x float> undef, float %scalar16.i, i32 0
  %vector287.i = shufflevector <16 x float> %temp286.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar17.i = extractelement <4 x float> %7, i32 1
  %temp291.i = insertelement <16 x float> undef, float %scalar17.i, i32 0
  %vector292.i = shufflevector <16 x float> %temp291.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar18.i = extractelement <4 x float> %7, i32 2
  %temp296.i = insertelement <16 x float> undef, float %scalar18.i, i32 0
  %vector297.i = shufflevector <16 x float> %temp296.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar19.i = extractelement <4 x float> %7, i32 3
  %temp301.i = insertelement <16 x float> undef, float %scalar19.i, i32 0
  %vector302.i = shufflevector <16 x float> %temp301.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar12.i = extractelement <4 x float> %4, i32 0
  %temp284.i = insertelement <16 x float> undef, float %scalar12.i, i32 0
  %vector285.i = shufflevector <16 x float> %temp284.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar13.i = extractelement <4 x float> %4, i32 1
  %temp289.i = insertelement <16 x float> undef, float %scalar13.i, i32 0
  %vector290.i = shufflevector <16 x float> %temp289.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar14.i = extractelement <4 x float> %4, i32 2
  %temp294.i = insertelement <16 x float> undef, float %scalar14.i, i32 0
  %vector295.i = shufflevector <16 x float> %temp294.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar15.i = extractelement <4 x float> %4, i32 3
  %temp299.i = insertelement <16 x float> undef, float %scalar15.i, i32 0
  %vector300.i = shufflevector <16 x float> %temp299.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %20 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = add i64 %21, %23
  %broadcast1.i = insertelement <16 x i64> undef, i64 %24, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %25 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %26 = trunc <16 x i64> %25 to <16 x i32>
  %27 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %28 = load i64* %27, align 8
  %29 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %30 = load i64* %29, align 8
  %31 = add i64 %28, %30
  %32 = trunc i64 %31 to i32
  %33 = getelementptr %struct.WorkDim* %10, i64 0, i32 2, i64 0
  %34 = load i64* %33, align 8
  %35 = trunc i64 %34 to i32
  %36 = mul nsw i32 %35, %32
  %temp.i = insertelement <16 x i32> undef, i32 %36, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %37 = add nsw <16 x i32> %vector.i, %26
  %38 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %39 = bitcast <16 x float> %38 to <16 x i32>
  %tmp24.i.i = shufflevector <16 x i32> %39, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i.i = bitcast <2 x i32> %tmp24.i.i to <1 x double>
  %tmp31.i.i = extractelement <1 x double> %tmp32.i.i, i32 0
  %40 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %41 = bitcast <16 x float> %40 to <16 x i32>
  %tmp24.i4.i = shufflevector <16 x i32> %41, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i5.i = bitcast <2 x i32> %tmp24.i4.i to <1 x double>
  %tmp31.i6.i = extractelement <1 x double> %tmp32.i5.i, i32 0
  %42 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %43 = bitcast <16 x float> %42 to <16 x i32>
  %tmp24.i7.i = shufflevector <16 x i32> %43, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i8.i = bitcast <2 x i32> %tmp24.i7.i to <1 x double>
  %tmp31.i9.i = extractelement <1 x double> %tmp32.i8.i, i32 0
  %44 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %45 = bitcast <16 x float> %44 to <16 x i32>
  %tmp24.i10.i = shufflevector <16 x i32> %45, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i11.i = bitcast <2 x i32> %tmp24.i10.i to <1 x double>
  %tmp31.i12.i = extractelement <1 x double> %tmp32.i11.i, i32 0
  %46 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %47 = bitcast <16 x float> %46 to <16 x i32>
  %tmp24.i13.i = shufflevector <16 x i32> %47, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i14.i = bitcast <2 x i32> %tmp24.i13.i to <1 x double>
  %tmp31.i15.i = extractelement <1 x double> %tmp32.i14.i, i32 0
  %48 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %49 = bitcast <16 x float> %48 to <16 x i32>
  %tmp24.i16.i = shufflevector <16 x i32> %49, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i17.i = bitcast <2 x i32> %tmp24.i16.i to <1 x double>
  %tmp31.i18.i = extractelement <1 x double> %tmp32.i17.i, i32 0
  %50 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %51 = bitcast <16 x float> %50 to <16 x i32>
  %tmp24.i19.i = shufflevector <16 x i32> %51, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i20.i = bitcast <2 x i32> %tmp24.i19.i to <1 x double>
  %tmp31.i21.i = extractelement <1 x double> %tmp32.i20.i, i32 0
  %52 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %53 = bitcast <16 x float> %52 to <16 x i32>
  %tmp24.i22.i = shufflevector <16 x i32> %53, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i23.i = bitcast <2 x i32> %tmp24.i22.i to <1 x double>
  %tmp31.i24.i = extractelement <1 x double> %tmp32.i23.i, i32 0
  %54 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %55 = bitcast <16 x float> %54 to <16 x i32>
  %tmp24.i25.i = shufflevector <16 x i32> %55, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i26.i = bitcast <2 x i32> %tmp24.i25.i to <1 x double>
  %tmp31.i27.i = extractelement <1 x double> %tmp32.i26.i, i32 0
  %56 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %57 = bitcast <16 x float> %56 to <16 x i32>
  %tmp24.i28.i = shufflevector <16 x i32> %57, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i29.i = bitcast <2 x i32> %tmp24.i28.i to <1 x double>
  %tmp31.i30.i = extractelement <1 x double> %tmp32.i29.i, i32 0
  %58 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %59 = bitcast <16 x float> %58 to <16 x i32>
  %tmp24.i31.i = shufflevector <16 x i32> %59, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i32.i = bitcast <2 x i32> %tmp24.i31.i to <1 x double>
  %tmp31.i33.i = extractelement <1 x double> %tmp32.i32.i, i32 0
  %60 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %61 = bitcast <16 x float> %60 to <16 x i32>
  %tmp24.i34.i = shufflevector <16 x i32> %61, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i35.i = bitcast <2 x i32> %tmp24.i34.i to <1 x double>
  %tmp31.i36.i = extractelement <1 x double> %tmp32.i35.i, i32 0
  %62 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %63 = bitcast <16 x float> %62 to <16 x i32>
  %tmp24.i37.i = shufflevector <16 x i32> %63, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i38.i = bitcast <2 x i32> %tmp24.i37.i to <1 x double>
  %tmp31.i39.i = extractelement <1 x double> %tmp32.i38.i, i32 0
  %64 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %65 = bitcast <16 x float> %64 to <16 x i32>
  %tmp24.i40.i = shufflevector <16 x i32> %65, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i41.i = bitcast <2 x i32> %tmp24.i40.i to <1 x double>
  %tmp31.i42.i = extractelement <1 x double> %tmp32.i41.i, i32 0
  %66 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %67 = bitcast <16 x float> %66 to <16 x i32>
  %tmp24.i43.i = shufflevector <16 x i32> %67, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i44.i = bitcast <2 x i32> %tmp24.i43.i to <1 x double>
  %tmp31.i45.i = extractelement <1 x double> %tmp32.i44.i, i32 0
  %68 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %69 = bitcast <16 x float> %68 to <16 x i32>
  %tmp24.i46.i = shufflevector <16 x i32> %69, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i47.i = bitcast <2 x i32> %tmp24.i46.i to <1 x double>
  %tmp31.i48.i = extractelement <1 x double> %tmp32.i47.i, i32 0
  %extract186.i = bitcast double %tmp31.i.i to i64
  %extract187.i = bitcast double %tmp31.i6.i to i64
  %extract188.i = bitcast double %tmp31.i9.i to i64
  %extract189.i = bitcast double %tmp31.i12.i to i64
  %extract190.i = bitcast double %tmp31.i15.i to i64
  %extract191.i = bitcast double %tmp31.i18.i to i64
  %extract192.i = bitcast double %tmp31.i21.i to i64
  %extract193.i = bitcast double %tmp31.i24.i to i64
  %extract194.i = bitcast double %tmp31.i27.i to i64
  %extract195.i = bitcast double %tmp31.i30.i to i64
  %extract196.i = bitcast double %tmp31.i33.i to i64
  %extract197.i = bitcast double %tmp31.i36.i to i64
  %extract198.i = bitcast double %tmp31.i39.i to i64
  %extract199.i = bitcast double %tmp31.i42.i to i64
  %extract200.i = bitcast double %tmp31.i45.i to i64
  %extract201.i = bitcast double %tmp31.i48.i to i64
  %70 = bitcast i64 %extract186.i to <2 x i32>
  %71 = bitcast i64 %extract187.i to <2 x i32>
  %72 = bitcast i64 %extract188.i to <2 x i32>
  %73 = bitcast i64 %extract189.i to <2 x i32>
  %74 = bitcast i64 %extract190.i to <2 x i32>
  %75 = bitcast i64 %extract191.i to <2 x i32>
  %76 = bitcast i64 %extract192.i to <2 x i32>
  %77 = bitcast i64 %extract193.i to <2 x i32>
  %78 = bitcast i64 %extract194.i to <2 x i32>
  %79 = bitcast i64 %extract195.i to <2 x i32>
  %80 = bitcast i64 %extract196.i to <2 x i32>
  %81 = bitcast i64 %extract197.i to <2 x i32>
  %82 = bitcast i64 %extract198.i to <2 x i32>
  %83 = bitcast i64 %extract199.i to <2 x i32>
  %84 = bitcast i64 %extract200.i to <2 x i32>
  %85 = bitcast i64 %extract201.i to <2 x i32>
  %86 = bitcast <2 x i32> %70 to <1 x double>
  %87 = bitcast <2 x i32> %71 to <1 x double>
  %88 = bitcast <2 x i32> %72 to <1 x double>
  %89 = bitcast <2 x i32> %73 to <1 x double>
  %90 = bitcast <2 x i32> %74 to <1 x double>
  %91 = bitcast <2 x i32> %75 to <1 x double>
  %92 = bitcast <2 x i32> %76 to <1 x double>
  %93 = bitcast <2 x i32> %77 to <1 x double>
  %94 = bitcast <2 x i32> %78 to <1 x double>
  %95 = bitcast <2 x i32> %79 to <1 x double>
  %96 = bitcast <2 x i32> %80 to <1 x double>
  %97 = bitcast <2 x i32> %81 to <1 x double>
  %98 = bitcast <2 x i32> %82 to <1 x double>
  %99 = bitcast <2 x i32> %83 to <1 x double>
  %100 = bitcast <2 x i32> %84 to <1 x double>
  %101 = bitcast <2 x i32> %85 to <1 x double>
  %102 = extractelement <1 x double> %86, i32 0
  %103 = extractelement <1 x double> %87, i32 0
  %104 = extractelement <1 x double> %88, i32 0
  %105 = extractelement <1 x double> %89, i32 0
  %106 = extractelement <1 x double> %90, i32 0
  %107 = extractelement <1 x double> %91, i32 0
  %108 = extractelement <1 x double> %92, i32 0
  %109 = extractelement <1 x double> %93, i32 0
  %110 = extractelement <1 x double> %94, i32 0
  %111 = extractelement <1 x double> %95, i32 0
  %112 = extractelement <1 x double> %96, i32 0
  %113 = extractelement <1 x double> %97, i32 0
  %114 = extractelement <1 x double> %98, i32 0
  %115 = extractelement <1 x double> %99, i32 0
  %116 = extractelement <1 x double> %100, i32 0
  %117 = extractelement <1 x double> %101, i32 0
  %tmp7.i.i = bitcast double %102 to i64
  %tmp6.i.i = bitcast i64 %tmp7.i.i to <2 x i32>
  %tmp2.i.i = shufflevector <2 x i32> %tmp6.i.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %118 = bitcast <16 x i32> %tmp2.i.i to <16 x float>
  %119 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %118, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i.i = bitcast <16 x float> %119 to <16 x i32>
  %conv2.i.i.i.i = bitcast <8 x i64> %tmp5.i.i.i to <16 x i32>
  %120 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i.i, <16 x i32> %conv2.i.i.i.i) nounwind
  %121 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %120, <16 x i32> %conv2.i.i.i.i) nounwind
  %122 = call i32 @llvm.x86.mic.kortestc(i16 %121, i16 %121) nounwind
  %tobool.i.i.i = icmp ne i32 %122, 0
  %cond.i.i.i = zext i1 %tobool.i.i.i to i32
  %tmp7.i49.i = bitcast double %103 to i64
  %tmp6.i50.i = bitcast i64 %tmp7.i49.i to <2 x i32>
  %tmp2.i51.i = shufflevector <2 x i32> %tmp6.i50.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %123 = bitcast <16 x i32> %tmp2.i51.i to <16 x float>
  %124 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %123, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i52.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i53.i = bitcast <16 x float> %124 to <16 x i32>
  %conv2.i.i.i54.i = bitcast <8 x i64> %tmp5.i.i52.i to <16 x i32>
  %125 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i53.i, <16 x i32> %conv2.i.i.i54.i) nounwind
  %126 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %125, <16 x i32> %conv2.i.i.i54.i) nounwind
  %127 = call i32 @llvm.x86.mic.kortestc(i16 %126, i16 %126) nounwind
  %tobool.i.i55.i = icmp ne i32 %127, 0
  %cond.i.i56.i = zext i1 %tobool.i.i55.i to i32
  %tmp7.i57.i = bitcast double %104 to i64
  %tmp6.i58.i = bitcast i64 %tmp7.i57.i to <2 x i32>
  %tmp2.i59.i = shufflevector <2 x i32> %tmp6.i58.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %128 = bitcast <16 x i32> %tmp2.i59.i to <16 x float>
  %129 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %128, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i60.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i61.i = bitcast <16 x float> %129 to <16 x i32>
  %conv2.i.i.i62.i = bitcast <8 x i64> %tmp5.i.i60.i to <16 x i32>
  %130 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i61.i, <16 x i32> %conv2.i.i.i62.i) nounwind
  %131 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %130, <16 x i32> %conv2.i.i.i62.i) nounwind
  %132 = call i32 @llvm.x86.mic.kortestc(i16 %131, i16 %131) nounwind
  %tobool.i.i63.i = icmp ne i32 %132, 0
  %cond.i.i64.i = zext i1 %tobool.i.i63.i to i32
  %tmp7.i65.i = bitcast double %105 to i64
  %tmp6.i66.i = bitcast i64 %tmp7.i65.i to <2 x i32>
  %tmp2.i67.i = shufflevector <2 x i32> %tmp6.i66.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %133 = bitcast <16 x i32> %tmp2.i67.i to <16 x float>
  %134 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %133, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i68.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i69.i = bitcast <16 x float> %134 to <16 x i32>
  %conv2.i.i.i70.i = bitcast <8 x i64> %tmp5.i.i68.i to <16 x i32>
  %135 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i69.i, <16 x i32> %conv2.i.i.i70.i) nounwind
  %136 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %135, <16 x i32> %conv2.i.i.i70.i) nounwind
  %137 = call i32 @llvm.x86.mic.kortestc(i16 %136, i16 %136) nounwind
  %tobool.i.i71.i = icmp ne i32 %137, 0
  %cond.i.i72.i = zext i1 %tobool.i.i71.i to i32
  %tmp7.i73.i = bitcast double %106 to i64
  %tmp6.i74.i = bitcast i64 %tmp7.i73.i to <2 x i32>
  %tmp2.i75.i = shufflevector <2 x i32> %tmp6.i74.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %138 = bitcast <16 x i32> %tmp2.i75.i to <16 x float>
  %139 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %138, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i76.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i77.i = bitcast <16 x float> %139 to <16 x i32>
  %conv2.i.i.i78.i = bitcast <8 x i64> %tmp5.i.i76.i to <16 x i32>
  %140 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i77.i, <16 x i32> %conv2.i.i.i78.i) nounwind
  %141 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %140, <16 x i32> %conv2.i.i.i78.i) nounwind
  %142 = call i32 @llvm.x86.mic.kortestc(i16 %141, i16 %141) nounwind
  %tobool.i.i79.i = icmp ne i32 %142, 0
  %cond.i.i80.i = zext i1 %tobool.i.i79.i to i32
  %tmp7.i81.i = bitcast double %107 to i64
  %tmp6.i82.i = bitcast i64 %tmp7.i81.i to <2 x i32>
  %tmp2.i83.i = shufflevector <2 x i32> %tmp6.i82.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %143 = bitcast <16 x i32> %tmp2.i83.i to <16 x float>
  %144 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %143, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i84.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i85.i = bitcast <16 x float> %144 to <16 x i32>
  %conv2.i.i.i86.i = bitcast <8 x i64> %tmp5.i.i84.i to <16 x i32>
  %145 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i85.i, <16 x i32> %conv2.i.i.i86.i) nounwind
  %146 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %145, <16 x i32> %conv2.i.i.i86.i) nounwind
  %147 = call i32 @llvm.x86.mic.kortestc(i16 %146, i16 %146) nounwind
  %tobool.i.i87.i = icmp ne i32 %147, 0
  %cond.i.i88.i = zext i1 %tobool.i.i87.i to i32
  %tmp7.i89.i = bitcast double %108 to i64
  %tmp6.i90.i = bitcast i64 %tmp7.i89.i to <2 x i32>
  %tmp2.i91.i = shufflevector <2 x i32> %tmp6.i90.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %148 = bitcast <16 x i32> %tmp2.i91.i to <16 x float>
  %149 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %148, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i92.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i93.i = bitcast <16 x float> %149 to <16 x i32>
  %conv2.i.i.i94.i = bitcast <8 x i64> %tmp5.i.i92.i to <16 x i32>
  %150 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i93.i, <16 x i32> %conv2.i.i.i94.i) nounwind
  %151 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %150, <16 x i32> %conv2.i.i.i94.i) nounwind
  %152 = call i32 @llvm.x86.mic.kortestc(i16 %151, i16 %151) nounwind
  %tobool.i.i95.i = icmp ne i32 %152, 0
  %cond.i.i96.i = zext i1 %tobool.i.i95.i to i32
  %tmp7.i97.i = bitcast double %109 to i64
  %tmp6.i98.i = bitcast i64 %tmp7.i97.i to <2 x i32>
  %tmp2.i99.i = shufflevector <2 x i32> %tmp6.i98.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %153 = bitcast <16 x i32> %tmp2.i99.i to <16 x float>
  %154 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %153, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i100.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i101.i = bitcast <16 x float> %154 to <16 x i32>
  %conv2.i.i.i102.i = bitcast <8 x i64> %tmp5.i.i100.i to <16 x i32>
  %155 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i101.i, <16 x i32> %conv2.i.i.i102.i) nounwind
  %156 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %155, <16 x i32> %conv2.i.i.i102.i) nounwind
  %157 = call i32 @llvm.x86.mic.kortestc(i16 %156, i16 %156) nounwind
  %tobool.i.i103.i = icmp ne i32 %157, 0
  %cond.i.i104.i = zext i1 %tobool.i.i103.i to i32
  %tmp7.i105.i = bitcast double %110 to i64
  %tmp6.i106.i = bitcast i64 %tmp7.i105.i to <2 x i32>
  %tmp2.i107.i = shufflevector <2 x i32> %tmp6.i106.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %158 = bitcast <16 x i32> %tmp2.i107.i to <16 x float>
  %159 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %158, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i108.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i109.i = bitcast <16 x float> %159 to <16 x i32>
  %conv2.i.i.i110.i = bitcast <8 x i64> %tmp5.i.i108.i to <16 x i32>
  %160 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i109.i, <16 x i32> %conv2.i.i.i110.i) nounwind
  %161 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %160, <16 x i32> %conv2.i.i.i110.i) nounwind
  %162 = call i32 @llvm.x86.mic.kortestc(i16 %161, i16 %161) nounwind
  %tobool.i.i111.i = icmp ne i32 %162, 0
  %cond.i.i112.i = zext i1 %tobool.i.i111.i to i32
  %tmp7.i113.i = bitcast double %111 to i64
  %tmp6.i114.i = bitcast i64 %tmp7.i113.i to <2 x i32>
  %tmp2.i115.i = shufflevector <2 x i32> %tmp6.i114.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %163 = bitcast <16 x i32> %tmp2.i115.i to <16 x float>
  %164 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %163, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i116.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i117.i = bitcast <16 x float> %164 to <16 x i32>
  %conv2.i.i.i118.i = bitcast <8 x i64> %tmp5.i.i116.i to <16 x i32>
  %165 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i117.i, <16 x i32> %conv2.i.i.i118.i) nounwind
  %166 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %165, <16 x i32> %conv2.i.i.i118.i) nounwind
  %167 = call i32 @llvm.x86.mic.kortestc(i16 %166, i16 %166) nounwind
  %tobool.i.i119.i = icmp ne i32 %167, 0
  %cond.i.i120.i = zext i1 %tobool.i.i119.i to i32
  %tmp7.i121.i = bitcast double %112 to i64
  %tmp6.i122.i = bitcast i64 %tmp7.i121.i to <2 x i32>
  %tmp2.i123.i = shufflevector <2 x i32> %tmp6.i122.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %168 = bitcast <16 x i32> %tmp2.i123.i to <16 x float>
  %169 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %168, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i124.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i125.i = bitcast <16 x float> %169 to <16 x i32>
  %conv2.i.i.i126.i = bitcast <8 x i64> %tmp5.i.i124.i to <16 x i32>
  %170 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i125.i, <16 x i32> %conv2.i.i.i126.i) nounwind
  %171 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %170, <16 x i32> %conv2.i.i.i126.i) nounwind
  %172 = call i32 @llvm.x86.mic.kortestc(i16 %171, i16 %171) nounwind
  %tobool.i.i127.i = icmp ne i32 %172, 0
  %cond.i.i128.i = zext i1 %tobool.i.i127.i to i32
  %tmp7.i129.i = bitcast double %113 to i64
  %tmp6.i130.i = bitcast i64 %tmp7.i129.i to <2 x i32>
  %tmp2.i131.i = shufflevector <2 x i32> %tmp6.i130.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %173 = bitcast <16 x i32> %tmp2.i131.i to <16 x float>
  %174 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %173, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i132.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i133.i = bitcast <16 x float> %174 to <16 x i32>
  %conv2.i.i.i134.i = bitcast <8 x i64> %tmp5.i.i132.i to <16 x i32>
  %175 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i133.i, <16 x i32> %conv2.i.i.i134.i) nounwind
  %176 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %175, <16 x i32> %conv2.i.i.i134.i) nounwind
  %177 = call i32 @llvm.x86.mic.kortestc(i16 %176, i16 %176) nounwind
  %tobool.i.i135.i = icmp ne i32 %177, 0
  %cond.i.i136.i = zext i1 %tobool.i.i135.i to i32
  %tmp7.i137.i = bitcast double %114 to i64
  %tmp6.i138.i = bitcast i64 %tmp7.i137.i to <2 x i32>
  %tmp2.i139.i = shufflevector <2 x i32> %tmp6.i138.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %178 = bitcast <16 x i32> %tmp2.i139.i to <16 x float>
  %179 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %178, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i140.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i141.i = bitcast <16 x float> %179 to <16 x i32>
  %conv2.i.i.i142.i = bitcast <8 x i64> %tmp5.i.i140.i to <16 x i32>
  %180 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i141.i, <16 x i32> %conv2.i.i.i142.i) nounwind
  %181 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %180, <16 x i32> %conv2.i.i.i142.i) nounwind
  %182 = call i32 @llvm.x86.mic.kortestc(i16 %181, i16 %181) nounwind
  %tobool.i.i143.i = icmp ne i32 %182, 0
  %cond.i.i144.i = zext i1 %tobool.i.i143.i to i32
  %tmp7.i145.i = bitcast double %115 to i64
  %tmp6.i146.i = bitcast i64 %tmp7.i145.i to <2 x i32>
  %tmp2.i147.i = shufflevector <2 x i32> %tmp6.i146.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %183 = bitcast <16 x i32> %tmp2.i147.i to <16 x float>
  %184 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %183, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i148.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i149.i = bitcast <16 x float> %184 to <16 x i32>
  %conv2.i.i.i150.i = bitcast <8 x i64> %tmp5.i.i148.i to <16 x i32>
  %185 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i149.i, <16 x i32> %conv2.i.i.i150.i) nounwind
  %186 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %185, <16 x i32> %conv2.i.i.i150.i) nounwind
  %187 = call i32 @llvm.x86.mic.kortestc(i16 %186, i16 %186) nounwind
  %tobool.i.i151.i = icmp ne i32 %187, 0
  %cond.i.i152.i = zext i1 %tobool.i.i151.i to i32
  %tmp7.i153.i = bitcast double %116 to i64
  %tmp6.i154.i = bitcast i64 %tmp7.i153.i to <2 x i32>
  %tmp2.i155.i = shufflevector <2 x i32> %tmp6.i154.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %188 = bitcast <16 x i32> %tmp2.i155.i to <16 x float>
  %189 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %188, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i156.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i157.i = bitcast <16 x float> %189 to <16 x i32>
  %conv2.i.i.i158.i = bitcast <8 x i64> %tmp5.i.i156.i to <16 x i32>
  %190 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i157.i, <16 x i32> %conv2.i.i.i158.i) nounwind
  %191 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %190, <16 x i32> %conv2.i.i.i158.i) nounwind
  %192 = call i32 @llvm.x86.mic.kortestc(i16 %191, i16 %191) nounwind
  %tobool.i.i159.i = icmp ne i32 %192, 0
  %cond.i.i160.i = zext i1 %tobool.i.i159.i to i32
  %tmp7.i161.i = bitcast double %117 to i64
  %tmp6.i162.i = bitcast i64 %tmp7.i161.i to <2 x i32>
  %tmp2.i163.i = shufflevector <2 x i32> %tmp6.i162.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %193 = bitcast <16 x i32> %tmp2.i163.i to <16 x float>
  %194 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %193, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i164.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i165.i = bitcast <16 x float> %194 to <16 x i32>
  %conv2.i.i.i166.i = bitcast <8 x i64> %tmp5.i.i164.i to <16 x i32>
  %195 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i165.i, <16 x i32> %conv2.i.i.i166.i) nounwind
  %196 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %195, <16 x i32> %conv2.i.i.i166.i) nounwind
  %197 = call i32 @llvm.x86.mic.kortestc(i16 %196, i16 %196) nounwind
  %tobool.i.i167.i = icmp ne i32 %197, 0
  %cond.i.i168.i = zext i1 %tobool.i.i167.i to i32
  %temp.vect202.i = insertelement <16 x i32> undef, i32 %cond.i.i.i, i32 0
  %temp.vect203.i = insertelement <16 x i32> %temp.vect202.i, i32 %cond.i.i56.i, i32 1
  %temp.vect204.i = insertelement <16 x i32> %temp.vect203.i, i32 %cond.i.i64.i, i32 2
  %temp.vect205.i = insertelement <16 x i32> %temp.vect204.i, i32 %cond.i.i72.i, i32 3
  %temp.vect206.i = insertelement <16 x i32> %temp.vect205.i, i32 %cond.i.i80.i, i32 4
  %temp.vect207.i = insertelement <16 x i32> %temp.vect206.i, i32 %cond.i.i88.i, i32 5
  %temp.vect208.i = insertelement <16 x i32> %temp.vect207.i, i32 %cond.i.i96.i, i32 6
  %temp.vect209.i = insertelement <16 x i32> %temp.vect208.i, i32 %cond.i.i104.i, i32 7
  %temp.vect210.i = insertelement <16 x i32> %temp.vect209.i, i32 %cond.i.i112.i, i32 8
  %temp.vect211.i = insertelement <16 x i32> %temp.vect210.i, i32 %cond.i.i120.i, i32 9
  %temp.vect212.i = insertelement <16 x i32> %temp.vect211.i, i32 %cond.i.i128.i, i32 10
  %temp.vect213.i = insertelement <16 x i32> %temp.vect212.i, i32 %cond.i.i136.i, i32 11
  %temp.vect214.i = insertelement <16 x i32> %temp.vect213.i, i32 %cond.i.i144.i, i32 12
  %temp.vect215.i = insertelement <16 x i32> %temp.vect214.i, i32 %cond.i.i152.i, i32 13
  %temp.vect216.i = insertelement <16 x i32> %temp.vect215.i, i32 %cond.i.i160.i, i32 14
  %temp.vect217.i = insertelement <16 x i32> %temp.vect216.i, i32 %cond.i.i168.i, i32 15
  %198 = icmp eq <16 x i32> %temp.vect217.i, zeroinitializer
  %extract219.i = extractelement <16 x i1> %198, i32 0
  %extract220.i = extractelement <16 x i1> %198, i32 1
  %extract221.i = extractelement <16 x i1> %198, i32 2
  %extract222.i = extractelement <16 x i1> %198, i32 3
  %extract223.i = extractelement <16 x i1> %198, i32 4
  %extract224.i = extractelement <16 x i1> %198, i32 5
  %extract225.i = extractelement <16 x i1> %198, i32 6
  %extract226.i = extractelement <16 x i1> %198, i32 7
  %extract227.i = extractelement <16 x i1> %198, i32 8
  %extract228.i = extractelement <16 x i1> %198, i32 9
  %extract229.i = extractelement <16 x i1> %198, i32 10
  %extract230.i = extractelement <16 x i1> %198, i32 11
  %extract231.i = extractelement <16 x i1> %198, i32 12
  %extract232.i = extractelement <16 x i1> %198, i32 13
  %extract233.i = extractelement <16 x i1> %198, i32 14
  %extract234.i = extractelement <16 x i1> %198, i32 15
  br i1 %extract219.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %SyncBB.i
  %199 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %200 = bitcast <16 x float> %199 to <16 x i32>
  %tmp24.i169.i = shufflevector <16 x i32> %200, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i170.i = bitcast <2 x i32> %tmp24.i169.i to <1 x double>
  %tmp31.i171.i = extractelement <1 x double> %tmp32.i170.i, i32 0
  %phitmp483.i = bitcast double %tmp31.i171.i to i64
  %phitmp498.i = bitcast i64 %phitmp483.i to <2 x i32>
  %phitmp513.i = bitcast <2 x i32> %phitmp498.i to <1 x double>
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %SyncBB.i
  %phi.i = phi <1 x double> [ undef, %SyncBB.i ], [ %phitmp513.i, %preload.i ]
  br i1 %extract220.i, label %preload391.i, label %postload392.i

preload391.i:                                     ; preds = %postload.i
  %201 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %202 = bitcast <16 x float> %201 to <16 x i32>
  %tmp24.i172.i = shufflevector <16 x i32> %202, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i173.i = bitcast <2 x i32> %tmp24.i172.i to <1 x double>
  %tmp31.i174.i = extractelement <1 x double> %tmp32.i173.i, i32 0
  %phitmp484.i = bitcast double %tmp31.i174.i to i64
  %phitmp499.i = bitcast i64 %phitmp484.i to <2 x i32>
  %phitmp514.i = bitcast <2 x i32> %phitmp499.i to <1 x double>
  br label %postload392.i

postload392.i:                                    ; preds = %preload391.i, %postload.i
  %phi393.i = phi <1 x double> [ undef, %postload.i ], [ %phitmp514.i, %preload391.i ]
  br i1 %extract221.i, label %preload397.i, label %postload398.i

preload397.i:                                     ; preds = %postload392.i
  %203 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %204 = bitcast <16 x float> %203 to <16 x i32>
  %tmp24.i175.i = shufflevector <16 x i32> %204, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i176.i = bitcast <2 x i32> %tmp24.i175.i to <1 x double>
  %tmp31.i177.i = extractelement <1 x double> %tmp32.i176.i, i32 0
  %phitmp485.i = bitcast double %tmp31.i177.i to i64
  %phitmp500.i = bitcast i64 %phitmp485.i to <2 x i32>
  %phitmp515.i = bitcast <2 x i32> %phitmp500.i to <1 x double>
  br label %postload398.i

postload398.i:                                    ; preds = %preload397.i, %postload392.i
  %phi399.i = phi <1 x double> [ undef, %postload392.i ], [ %phitmp515.i, %preload397.i ]
  br i1 %extract222.i, label %preload403.i, label %postload404.i

preload403.i:                                     ; preds = %postload398.i
  %205 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %206 = bitcast <16 x float> %205 to <16 x i32>
  %tmp24.i178.i = shufflevector <16 x i32> %206, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i179.i = bitcast <2 x i32> %tmp24.i178.i to <1 x double>
  %tmp31.i180.i = extractelement <1 x double> %tmp32.i179.i, i32 0
  %phitmp486.i = bitcast double %tmp31.i180.i to i64
  %phitmp501.i = bitcast i64 %phitmp486.i to <2 x i32>
  %phitmp516.i = bitcast <2 x i32> %phitmp501.i to <1 x double>
  br label %postload404.i

postload404.i:                                    ; preds = %preload403.i, %postload398.i
  %phi405.i = phi <1 x double> [ undef, %postload398.i ], [ %phitmp516.i, %preload403.i ]
  br i1 %extract223.i, label %preload409.i, label %postload410.i

preload409.i:                                     ; preds = %postload404.i
  %207 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %208 = bitcast <16 x float> %207 to <16 x i32>
  %tmp24.i181.i = shufflevector <16 x i32> %208, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i182.i = bitcast <2 x i32> %tmp24.i181.i to <1 x double>
  %tmp31.i183.i = extractelement <1 x double> %tmp32.i182.i, i32 0
  %phitmp487.i = bitcast double %tmp31.i183.i to i64
  %phitmp502.i = bitcast i64 %phitmp487.i to <2 x i32>
  %phitmp517.i = bitcast <2 x i32> %phitmp502.i to <1 x double>
  br label %postload410.i

postload410.i:                                    ; preds = %preload409.i, %postload404.i
  %phi411.i = phi <1 x double> [ undef, %postload404.i ], [ %phitmp517.i, %preload409.i ]
  br i1 %extract224.i, label %preload415.i, label %postload416.i

preload415.i:                                     ; preds = %postload410.i
  %209 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %210 = bitcast <16 x float> %209 to <16 x i32>
  %tmp24.i184.i = shufflevector <16 x i32> %210, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i185.i = bitcast <2 x i32> %tmp24.i184.i to <1 x double>
  %tmp31.i186.i = extractelement <1 x double> %tmp32.i185.i, i32 0
  %phitmp488.i = bitcast double %tmp31.i186.i to i64
  %phitmp503.i = bitcast i64 %phitmp488.i to <2 x i32>
  %phitmp518.i = bitcast <2 x i32> %phitmp503.i to <1 x double>
  br label %postload416.i

postload416.i:                                    ; preds = %preload415.i, %postload410.i
  %phi417.i = phi <1 x double> [ undef, %postload410.i ], [ %phitmp518.i, %preload415.i ]
  br i1 %extract225.i, label %preload421.i, label %postload422.i

preload421.i:                                     ; preds = %postload416.i
  %211 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %212 = bitcast <16 x float> %211 to <16 x i32>
  %tmp24.i187.i = shufflevector <16 x i32> %212, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i188.i = bitcast <2 x i32> %tmp24.i187.i to <1 x double>
  %tmp31.i189.i = extractelement <1 x double> %tmp32.i188.i, i32 0
  %phitmp489.i = bitcast double %tmp31.i189.i to i64
  %phitmp504.i = bitcast i64 %phitmp489.i to <2 x i32>
  %phitmp519.i = bitcast <2 x i32> %phitmp504.i to <1 x double>
  br label %postload422.i

postload422.i:                                    ; preds = %preload421.i, %postload416.i
  %phi423.i = phi <1 x double> [ undef, %postload416.i ], [ %phitmp519.i, %preload421.i ]
  br i1 %extract226.i, label %preload427.i, label %postload428.i

preload427.i:                                     ; preds = %postload422.i
  %213 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %214 = bitcast <16 x float> %213 to <16 x i32>
  %tmp24.i190.i = shufflevector <16 x i32> %214, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i191.i = bitcast <2 x i32> %tmp24.i190.i to <1 x double>
  %tmp31.i192.i = extractelement <1 x double> %tmp32.i191.i, i32 0
  %phitmp490.i = bitcast double %tmp31.i192.i to i64
  %phitmp505.i = bitcast i64 %phitmp490.i to <2 x i32>
  %phitmp520.i = bitcast <2 x i32> %phitmp505.i to <1 x double>
  br label %postload428.i

postload428.i:                                    ; preds = %preload427.i, %postload422.i
  %phi429.i = phi <1 x double> [ undef, %postload422.i ], [ %phitmp520.i, %preload427.i ]
  br i1 %extract227.i, label %preload433.i, label %postload434.i

preload433.i:                                     ; preds = %postload428.i
  %215 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %216 = bitcast <16 x float> %215 to <16 x i32>
  %tmp24.i193.i = shufflevector <16 x i32> %216, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i194.i = bitcast <2 x i32> %tmp24.i193.i to <1 x double>
  %tmp31.i195.i = extractelement <1 x double> %tmp32.i194.i, i32 0
  %phitmp491.i = bitcast double %tmp31.i195.i to i64
  %phitmp506.i = bitcast i64 %phitmp491.i to <2 x i32>
  %phitmp521.i = bitcast <2 x i32> %phitmp506.i to <1 x double>
  br label %postload434.i

postload434.i:                                    ; preds = %preload433.i, %postload428.i
  %phi435.i = phi <1 x double> [ undef, %postload428.i ], [ %phitmp521.i, %preload433.i ]
  br i1 %extract228.i, label %preload439.i, label %postload440.i

preload439.i:                                     ; preds = %postload434.i
  %217 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %218 = bitcast <16 x float> %217 to <16 x i32>
  %tmp24.i196.i = shufflevector <16 x i32> %218, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i197.i = bitcast <2 x i32> %tmp24.i196.i to <1 x double>
  %tmp31.i198.i = extractelement <1 x double> %tmp32.i197.i, i32 0
  %phitmp492.i = bitcast double %tmp31.i198.i to i64
  %phitmp507.i = bitcast i64 %phitmp492.i to <2 x i32>
  %phitmp522.i = bitcast <2 x i32> %phitmp507.i to <1 x double>
  br label %postload440.i

postload440.i:                                    ; preds = %preload439.i, %postload434.i
  %phi441.i = phi <1 x double> [ undef, %postload434.i ], [ %phitmp522.i, %preload439.i ]
  br i1 %extract229.i, label %preload445.i, label %postload446.i

preload445.i:                                     ; preds = %postload440.i
  %219 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %220 = bitcast <16 x float> %219 to <16 x i32>
  %tmp24.i199.i = shufflevector <16 x i32> %220, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i200.i = bitcast <2 x i32> %tmp24.i199.i to <1 x double>
  %tmp31.i201.i = extractelement <1 x double> %tmp32.i200.i, i32 0
  %phitmp493.i = bitcast double %tmp31.i201.i to i64
  %phitmp508.i = bitcast i64 %phitmp493.i to <2 x i32>
  %phitmp523.i = bitcast <2 x i32> %phitmp508.i to <1 x double>
  br label %postload446.i

postload446.i:                                    ; preds = %preload445.i, %postload440.i
  %phi447.i = phi <1 x double> [ undef, %postload440.i ], [ %phitmp523.i, %preload445.i ]
  br i1 %extract230.i, label %preload451.i, label %postload452.i

preload451.i:                                     ; preds = %postload446.i
  %221 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %222 = bitcast <16 x float> %221 to <16 x i32>
  %tmp24.i202.i = shufflevector <16 x i32> %222, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i203.i = bitcast <2 x i32> %tmp24.i202.i to <1 x double>
  %tmp31.i204.i = extractelement <1 x double> %tmp32.i203.i, i32 0
  %phitmp494.i = bitcast double %tmp31.i204.i to i64
  %phitmp509.i = bitcast i64 %phitmp494.i to <2 x i32>
  %phitmp524.i = bitcast <2 x i32> %phitmp509.i to <1 x double>
  br label %postload452.i

postload452.i:                                    ; preds = %preload451.i, %postload446.i
  %phi453.i = phi <1 x double> [ undef, %postload446.i ], [ %phitmp524.i, %preload451.i ]
  br i1 %extract231.i, label %preload457.i, label %postload458.i

preload457.i:                                     ; preds = %postload452.i
  %223 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %224 = bitcast <16 x float> %223 to <16 x i32>
  %tmp24.i205.i = shufflevector <16 x i32> %224, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i206.i = bitcast <2 x i32> %tmp24.i205.i to <1 x double>
  %tmp31.i207.i = extractelement <1 x double> %tmp32.i206.i, i32 0
  %phitmp495.i = bitcast double %tmp31.i207.i to i64
  %phitmp510.i = bitcast i64 %phitmp495.i to <2 x i32>
  %phitmp525.i = bitcast <2 x i32> %phitmp510.i to <1 x double>
  br label %postload458.i

postload458.i:                                    ; preds = %preload457.i, %postload452.i
  %phi459.i = phi <1 x double> [ undef, %postload452.i ], [ %phitmp525.i, %preload457.i ]
  br i1 %extract232.i, label %preload463.i, label %postload464.i

preload463.i:                                     ; preds = %postload458.i
  %225 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %226 = bitcast <16 x float> %225 to <16 x i32>
  %tmp24.i208.i = shufflevector <16 x i32> %226, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i209.i = bitcast <2 x i32> %tmp24.i208.i to <1 x double>
  %tmp31.i210.i = extractelement <1 x double> %tmp32.i209.i, i32 0
  %phitmp496.i = bitcast double %tmp31.i210.i to i64
  %phitmp511.i = bitcast i64 %phitmp496.i to <2 x i32>
  %phitmp526.i = bitcast <2 x i32> %phitmp511.i to <1 x double>
  br label %postload464.i

postload464.i:                                    ; preds = %preload463.i, %postload458.i
  %phi465.i = phi <1 x double> [ undef, %postload458.i ], [ %phitmp526.i, %preload463.i ]
  br i1 %extract233.i, label %preload469.i, label %postload470.i

preload469.i:                                     ; preds = %postload464.i
  %227 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %228 = bitcast <16 x float> %227 to <16 x i32>
  %tmp24.i211.i = shufflevector <16 x i32> %228, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i212.i = bitcast <2 x i32> %tmp24.i211.i to <1 x double>
  %tmp31.i213.i = extractelement <1 x double> %tmp32.i212.i, i32 0
  %phitmp497.i = bitcast double %tmp31.i213.i to i64
  %phitmp512.i = bitcast i64 %phitmp497.i to <2 x i32>
  %phitmp527.i = bitcast <2 x i32> %phitmp512.i to <1 x double>
  br label %postload470.i

postload470.i:                                    ; preds = %preload469.i, %postload464.i
  %phi471.i = phi <1 x double> [ undef, %postload464.i ], [ %phitmp527.i, %preload469.i ]
  br i1 %extract234.i, label %preload475.i, label %postload476.i

preload475.i:                                     ; preds = %postload470.i
  %229 = call <16 x float> @llvm.x86.mic.zero.ps() nounwind
  %230 = bitcast <16 x float> %229 to <16 x i32>
  %tmp24.i214.i = shufflevector <16 x i32> %230, <16 x i32> undef, <2 x i32> <i32 0, i32 1>
  %tmp32.i215.i = bitcast <2 x i32> %tmp24.i214.i to <1 x double>
  %tmp31.i216.i = extractelement <1 x double> %tmp32.i215.i, i32 0
  %phitmp.i = bitcast double %tmp31.i216.i to i64
  %phitmp481.i = bitcast i64 %phitmp.i to <2 x i32>
  %phitmp482.i = bitcast <2 x i32> %phitmp481.i to <1 x double>
  br label %postload476.i

postload476.i:                                    ; preds = %preload475.i, %postload470.i
  %phi477.i = phi <1 x double> [ undef, %postload470.i ], [ %phitmp482.i, %preload475.i ]
  %231 = extractelement <1 x double> %phi393.i, i32 0
  %232 = extractelement <1 x double> %phi399.i, i32 0
  %233 = extractelement <1 x double> %phi405.i, i32 0
  %234 = extractelement <1 x double> %phi411.i, i32 0
  %235 = extractelement <1 x double> %phi417.i, i32 0
  %236 = extractelement <1 x double> %phi423.i, i32 0
  %237 = extractelement <1 x double> %phi429.i, i32 0
  %238 = extractelement <1 x double> %phi435.i, i32 0
  %239 = extractelement <1 x double> %phi441.i, i32 0
  %240 = extractelement <1 x double> %phi447.i, i32 0
  %241 = extractelement <1 x double> %phi453.i, i32 0
  %242 = extractelement <1 x double> %phi459.i, i32 0
  %243 = extractelement <1 x double> %phi465.i, i32 0
  %244 = extractelement <1 x double> %phi471.i, i32 0
  %245 = extractelement <1 x double> %phi477.i, i32 0
  br i1 %extract219.i, label %preload388.i, label %postload389.i

preload388.i:                                     ; preds = %postload476.i
  %246 = extractelement <1 x double> %phi.i, i32 0
  %tmp7.i217.i = bitcast double %246 to i64
  %tmp6.i218.i = bitcast i64 %tmp7.i217.i to <2 x i32>
  %tmp2.i219.i = shufflevector <2 x i32> %tmp6.i218.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %247 = bitcast <16 x i32> %tmp2.i219.i to <16 x float>
  %248 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %247, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i220.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i221.i = bitcast <16 x float> %248 to <16 x i32>
  %conv2.i.i.i222.i = bitcast <8 x i64> %tmp5.i.i220.i to <16 x i32>
  %249 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i221.i, <16 x i32> %conv2.i.i.i222.i) nounwind
  %250 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %249, <16 x i32> %conv2.i.i.i222.i) nounwind
  %251 = call i32 @llvm.x86.mic.kortestc(i16 %250, i16 %250) nounwind
  %tobool.i.i223.i = icmp ne i32 %251, 0
  %cond.i.i224.i = zext i1 %tobool.i.i223.i to i32
  br label %postload389.i

postload389.i:                                    ; preds = %preload388.i, %postload476.i
  %phi390.i = phi i32 [ undef, %postload476.i ], [ %cond.i.i224.i, %preload388.i ]
  br i1 %extract220.i, label %preload394.i, label %postload395.i

preload394.i:                                     ; preds = %postload389.i
  %tmp7.i225.i = bitcast double %231 to i64
  %tmp6.i226.i = bitcast i64 %tmp7.i225.i to <2 x i32>
  %tmp2.i227.i = shufflevector <2 x i32> %tmp6.i226.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %252 = bitcast <16 x i32> %tmp2.i227.i to <16 x float>
  %253 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %252, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i228.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i229.i = bitcast <16 x float> %253 to <16 x i32>
  %conv2.i.i.i230.i = bitcast <8 x i64> %tmp5.i.i228.i to <16 x i32>
  %254 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i229.i, <16 x i32> %conv2.i.i.i230.i) nounwind
  %255 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %254, <16 x i32> %conv2.i.i.i230.i) nounwind
  %256 = call i32 @llvm.x86.mic.kortestc(i16 %255, i16 %255) nounwind
  %tobool.i.i231.i = icmp ne i32 %256, 0
  %cond.i.i232.i = zext i1 %tobool.i.i231.i to i32
  br label %postload395.i

postload395.i:                                    ; preds = %preload394.i, %postload389.i
  %phi396.i = phi i32 [ undef, %postload389.i ], [ %cond.i.i232.i, %preload394.i ]
  br i1 %extract221.i, label %preload400.i, label %postload401.i

preload400.i:                                     ; preds = %postload395.i
  %tmp7.i233.i = bitcast double %232 to i64
  %tmp6.i234.i = bitcast i64 %tmp7.i233.i to <2 x i32>
  %tmp2.i235.i = shufflevector <2 x i32> %tmp6.i234.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %257 = bitcast <16 x i32> %tmp2.i235.i to <16 x float>
  %258 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %257, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i236.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i237.i = bitcast <16 x float> %258 to <16 x i32>
  %conv2.i.i.i238.i = bitcast <8 x i64> %tmp5.i.i236.i to <16 x i32>
  %259 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i237.i, <16 x i32> %conv2.i.i.i238.i) nounwind
  %260 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %259, <16 x i32> %conv2.i.i.i238.i) nounwind
  %261 = call i32 @llvm.x86.mic.kortestc(i16 %260, i16 %260) nounwind
  %tobool.i.i239.i = icmp ne i32 %261, 0
  %cond.i.i240.i = zext i1 %tobool.i.i239.i to i32
  br label %postload401.i

postload401.i:                                    ; preds = %preload400.i, %postload395.i
  %phi402.i = phi i32 [ undef, %postload395.i ], [ %cond.i.i240.i, %preload400.i ]
  br i1 %extract222.i, label %preload406.i, label %postload407.i

preload406.i:                                     ; preds = %postload401.i
  %tmp7.i241.i = bitcast double %233 to i64
  %tmp6.i242.i = bitcast i64 %tmp7.i241.i to <2 x i32>
  %tmp2.i243.i = shufflevector <2 x i32> %tmp6.i242.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %262 = bitcast <16 x i32> %tmp2.i243.i to <16 x float>
  %263 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %262, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i244.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i245.i = bitcast <16 x float> %263 to <16 x i32>
  %conv2.i.i.i246.i = bitcast <8 x i64> %tmp5.i.i244.i to <16 x i32>
  %264 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i245.i, <16 x i32> %conv2.i.i.i246.i) nounwind
  %265 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %264, <16 x i32> %conv2.i.i.i246.i) nounwind
  %266 = call i32 @llvm.x86.mic.kortestc(i16 %265, i16 %265) nounwind
  %tobool.i.i247.i = icmp ne i32 %266, 0
  %cond.i.i248.i = zext i1 %tobool.i.i247.i to i32
  br label %postload407.i

postload407.i:                                    ; preds = %preload406.i, %postload401.i
  %phi408.i = phi i32 [ undef, %postload401.i ], [ %cond.i.i248.i, %preload406.i ]
  br i1 %extract223.i, label %preload412.i, label %postload413.i

preload412.i:                                     ; preds = %postload407.i
  %tmp7.i249.i = bitcast double %234 to i64
  %tmp6.i250.i = bitcast i64 %tmp7.i249.i to <2 x i32>
  %tmp2.i251.i = shufflevector <2 x i32> %tmp6.i250.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %267 = bitcast <16 x i32> %tmp2.i251.i to <16 x float>
  %268 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %267, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i252.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i253.i = bitcast <16 x float> %268 to <16 x i32>
  %conv2.i.i.i254.i = bitcast <8 x i64> %tmp5.i.i252.i to <16 x i32>
  %269 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i253.i, <16 x i32> %conv2.i.i.i254.i) nounwind
  %270 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %269, <16 x i32> %conv2.i.i.i254.i) nounwind
  %271 = call i32 @llvm.x86.mic.kortestc(i16 %270, i16 %270) nounwind
  %tobool.i.i255.i = icmp ne i32 %271, 0
  %cond.i.i256.i = zext i1 %tobool.i.i255.i to i32
  br label %postload413.i

postload413.i:                                    ; preds = %preload412.i, %postload407.i
  %phi414.i = phi i32 [ undef, %postload407.i ], [ %cond.i.i256.i, %preload412.i ]
  br i1 %extract224.i, label %preload418.i, label %postload419.i

preload418.i:                                     ; preds = %postload413.i
  %tmp7.i257.i = bitcast double %235 to i64
  %tmp6.i258.i = bitcast i64 %tmp7.i257.i to <2 x i32>
  %tmp2.i259.i = shufflevector <2 x i32> %tmp6.i258.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %272 = bitcast <16 x i32> %tmp2.i259.i to <16 x float>
  %273 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %272, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i260.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i261.i = bitcast <16 x float> %273 to <16 x i32>
  %conv2.i.i.i262.i = bitcast <8 x i64> %tmp5.i.i260.i to <16 x i32>
  %274 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i261.i, <16 x i32> %conv2.i.i.i262.i) nounwind
  %275 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %274, <16 x i32> %conv2.i.i.i262.i) nounwind
  %276 = call i32 @llvm.x86.mic.kortestc(i16 %275, i16 %275) nounwind
  %tobool.i.i263.i = icmp ne i32 %276, 0
  %cond.i.i264.i = zext i1 %tobool.i.i263.i to i32
  br label %postload419.i

postload419.i:                                    ; preds = %preload418.i, %postload413.i
  %phi420.i = phi i32 [ undef, %postload413.i ], [ %cond.i.i264.i, %preload418.i ]
  br i1 %extract225.i, label %preload424.i, label %postload425.i

preload424.i:                                     ; preds = %postload419.i
  %tmp7.i265.i = bitcast double %236 to i64
  %tmp6.i266.i = bitcast i64 %tmp7.i265.i to <2 x i32>
  %tmp2.i267.i = shufflevector <2 x i32> %tmp6.i266.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %277 = bitcast <16 x i32> %tmp2.i267.i to <16 x float>
  %278 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %277, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i268.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i269.i = bitcast <16 x float> %278 to <16 x i32>
  %conv2.i.i.i270.i = bitcast <8 x i64> %tmp5.i.i268.i to <16 x i32>
  %279 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i269.i, <16 x i32> %conv2.i.i.i270.i) nounwind
  %280 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %279, <16 x i32> %conv2.i.i.i270.i) nounwind
  %281 = call i32 @llvm.x86.mic.kortestc(i16 %280, i16 %280) nounwind
  %tobool.i.i271.i = icmp ne i32 %281, 0
  %cond.i.i272.i = zext i1 %tobool.i.i271.i to i32
  br label %postload425.i

postload425.i:                                    ; preds = %preload424.i, %postload419.i
  %phi426.i = phi i32 [ undef, %postload419.i ], [ %cond.i.i272.i, %preload424.i ]
  br i1 %extract226.i, label %preload430.i, label %postload431.i

preload430.i:                                     ; preds = %postload425.i
  %tmp7.i273.i = bitcast double %237 to i64
  %tmp6.i274.i = bitcast i64 %tmp7.i273.i to <2 x i32>
  %tmp2.i275.i = shufflevector <2 x i32> %tmp6.i274.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %282 = bitcast <16 x i32> %tmp2.i275.i to <16 x float>
  %283 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %282, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i276.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i277.i = bitcast <16 x float> %283 to <16 x i32>
  %conv2.i.i.i278.i = bitcast <8 x i64> %tmp5.i.i276.i to <16 x i32>
  %284 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i277.i, <16 x i32> %conv2.i.i.i278.i) nounwind
  %285 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %284, <16 x i32> %conv2.i.i.i278.i) nounwind
  %286 = call i32 @llvm.x86.mic.kortestc(i16 %285, i16 %285) nounwind
  %tobool.i.i279.i = icmp ne i32 %286, 0
  %cond.i.i280.i = zext i1 %tobool.i.i279.i to i32
  br label %postload431.i

postload431.i:                                    ; preds = %preload430.i, %postload425.i
  %phi432.i = phi i32 [ undef, %postload425.i ], [ %cond.i.i280.i, %preload430.i ]
  br i1 %extract227.i, label %preload436.i, label %postload437.i

preload436.i:                                     ; preds = %postload431.i
  %tmp7.i281.i = bitcast double %238 to i64
  %tmp6.i282.i = bitcast i64 %tmp7.i281.i to <2 x i32>
  %tmp2.i283.i = shufflevector <2 x i32> %tmp6.i282.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %287 = bitcast <16 x i32> %tmp2.i283.i to <16 x float>
  %288 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %287, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i284.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i285.i = bitcast <16 x float> %288 to <16 x i32>
  %conv2.i.i.i286.i = bitcast <8 x i64> %tmp5.i.i284.i to <16 x i32>
  %289 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i285.i, <16 x i32> %conv2.i.i.i286.i) nounwind
  %290 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %289, <16 x i32> %conv2.i.i.i286.i) nounwind
  %291 = call i32 @llvm.x86.mic.kortestc(i16 %290, i16 %290) nounwind
  %tobool.i.i287.i = icmp ne i32 %291, 0
  %cond.i.i288.i = zext i1 %tobool.i.i287.i to i32
  br label %postload437.i

postload437.i:                                    ; preds = %preload436.i, %postload431.i
  %phi438.i = phi i32 [ undef, %postload431.i ], [ %cond.i.i288.i, %preload436.i ]
  br i1 %extract228.i, label %preload442.i, label %postload443.i

preload442.i:                                     ; preds = %postload437.i
  %tmp7.i289.i = bitcast double %239 to i64
  %tmp6.i290.i = bitcast i64 %tmp7.i289.i to <2 x i32>
  %tmp2.i291.i = shufflevector <2 x i32> %tmp6.i290.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %292 = bitcast <16 x i32> %tmp2.i291.i to <16 x float>
  %293 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %292, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i292.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i293.i = bitcast <16 x float> %293 to <16 x i32>
  %conv2.i.i.i294.i = bitcast <8 x i64> %tmp5.i.i292.i to <16 x i32>
  %294 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i293.i, <16 x i32> %conv2.i.i.i294.i) nounwind
  %295 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %294, <16 x i32> %conv2.i.i.i294.i) nounwind
  %296 = call i32 @llvm.x86.mic.kortestc(i16 %295, i16 %295) nounwind
  %tobool.i.i295.i = icmp ne i32 %296, 0
  %cond.i.i296.i = zext i1 %tobool.i.i295.i to i32
  br label %postload443.i

postload443.i:                                    ; preds = %preload442.i, %postload437.i
  %phi444.i = phi i32 [ undef, %postload437.i ], [ %cond.i.i296.i, %preload442.i ]
  br i1 %extract229.i, label %preload448.i, label %postload449.i

preload448.i:                                     ; preds = %postload443.i
  %tmp7.i297.i = bitcast double %240 to i64
  %tmp6.i298.i = bitcast i64 %tmp7.i297.i to <2 x i32>
  %tmp2.i299.i = shufflevector <2 x i32> %tmp6.i298.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %297 = bitcast <16 x i32> %tmp2.i299.i to <16 x float>
  %298 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %297, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i300.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i301.i = bitcast <16 x float> %298 to <16 x i32>
  %conv2.i.i.i302.i = bitcast <8 x i64> %tmp5.i.i300.i to <16 x i32>
  %299 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i301.i, <16 x i32> %conv2.i.i.i302.i) nounwind
  %300 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %299, <16 x i32> %conv2.i.i.i302.i) nounwind
  %301 = call i32 @llvm.x86.mic.kortestc(i16 %300, i16 %300) nounwind
  %tobool.i.i303.i = icmp ne i32 %301, 0
  %cond.i.i304.i = zext i1 %tobool.i.i303.i to i32
  br label %postload449.i

postload449.i:                                    ; preds = %preload448.i, %postload443.i
  %phi450.i = phi i32 [ undef, %postload443.i ], [ %cond.i.i304.i, %preload448.i ]
  br i1 %extract230.i, label %preload454.i, label %postload455.i

preload454.i:                                     ; preds = %postload449.i
  %tmp7.i305.i = bitcast double %241 to i64
  %tmp6.i306.i = bitcast i64 %tmp7.i305.i to <2 x i32>
  %tmp2.i307.i = shufflevector <2 x i32> %tmp6.i306.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %302 = bitcast <16 x i32> %tmp2.i307.i to <16 x float>
  %303 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %302, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i308.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i309.i = bitcast <16 x float> %303 to <16 x i32>
  %conv2.i.i.i310.i = bitcast <8 x i64> %tmp5.i.i308.i to <16 x i32>
  %304 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i309.i, <16 x i32> %conv2.i.i.i310.i) nounwind
  %305 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %304, <16 x i32> %conv2.i.i.i310.i) nounwind
  %306 = call i32 @llvm.x86.mic.kortestc(i16 %305, i16 %305) nounwind
  %tobool.i.i311.i = icmp ne i32 %306, 0
  %cond.i.i312.i = zext i1 %tobool.i.i311.i to i32
  br label %postload455.i

postload455.i:                                    ; preds = %preload454.i, %postload449.i
  %phi456.i = phi i32 [ undef, %postload449.i ], [ %cond.i.i312.i, %preload454.i ]
  br i1 %extract231.i, label %preload460.i, label %postload461.i

preload460.i:                                     ; preds = %postload455.i
  %tmp7.i313.i = bitcast double %242 to i64
  %tmp6.i314.i = bitcast i64 %tmp7.i313.i to <2 x i32>
  %tmp2.i315.i = shufflevector <2 x i32> %tmp6.i314.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %307 = bitcast <16 x i32> %tmp2.i315.i to <16 x float>
  %308 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %307, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i316.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i317.i = bitcast <16 x float> %308 to <16 x i32>
  %conv2.i.i.i318.i = bitcast <8 x i64> %tmp5.i.i316.i to <16 x i32>
  %309 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i317.i, <16 x i32> %conv2.i.i.i318.i) nounwind
  %310 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %309, <16 x i32> %conv2.i.i.i318.i) nounwind
  %311 = call i32 @llvm.x86.mic.kortestc(i16 %310, i16 %310) nounwind
  %tobool.i.i319.i = icmp ne i32 %311, 0
  %cond.i.i320.i = zext i1 %tobool.i.i319.i to i32
  br label %postload461.i

postload461.i:                                    ; preds = %preload460.i, %postload455.i
  %phi462.i = phi i32 [ undef, %postload455.i ], [ %cond.i.i320.i, %preload460.i ]
  br i1 %extract232.i, label %preload466.i, label %postload467.i

preload466.i:                                     ; preds = %postload461.i
  %tmp7.i321.i = bitcast double %243 to i64
  %tmp6.i322.i = bitcast i64 %tmp7.i321.i to <2 x i32>
  %tmp2.i323.i = shufflevector <2 x i32> %tmp6.i322.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %312 = bitcast <16 x i32> %tmp2.i323.i to <16 x float>
  %313 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %312, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i324.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i325.i = bitcast <16 x float> %313 to <16 x i32>
  %conv2.i.i.i326.i = bitcast <8 x i64> %tmp5.i.i324.i to <16 x i32>
  %314 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i325.i, <16 x i32> %conv2.i.i.i326.i) nounwind
  %315 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %314, <16 x i32> %conv2.i.i.i326.i) nounwind
  %316 = call i32 @llvm.x86.mic.kortestc(i16 %315, i16 %315) nounwind
  %tobool.i.i327.i = icmp ne i32 %316, 0
  %cond.i.i328.i = zext i1 %tobool.i.i327.i to i32
  br label %postload467.i

postload467.i:                                    ; preds = %preload466.i, %postload461.i
  %phi468.i = phi i32 [ undef, %postload461.i ], [ %cond.i.i328.i, %preload466.i ]
  br i1 %extract233.i, label %preload472.i, label %postload473.i

preload472.i:                                     ; preds = %postload467.i
  %tmp7.i329.i = bitcast double %244 to i64
  %tmp6.i330.i = bitcast i64 %tmp7.i329.i to <2 x i32>
  %tmp2.i331.i = shufflevector <2 x i32> %tmp6.i330.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %317 = bitcast <16 x i32> %tmp2.i331.i to <16 x float>
  %318 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %317, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i332.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i333.i = bitcast <16 x float> %318 to <16 x i32>
  %conv2.i.i.i334.i = bitcast <8 x i64> %tmp5.i.i332.i to <16 x i32>
  %319 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i333.i, <16 x i32> %conv2.i.i.i334.i) nounwind
  %320 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %319, <16 x i32> %conv2.i.i.i334.i) nounwind
  %321 = call i32 @llvm.x86.mic.kortestc(i16 %320, i16 %320) nounwind
  %tobool.i.i335.i = icmp ne i32 %321, 0
  %cond.i.i336.i = zext i1 %tobool.i.i335.i to i32
  br label %postload473.i

postload473.i:                                    ; preds = %preload472.i, %postload467.i
  %phi474.i = phi i32 [ undef, %postload467.i ], [ %cond.i.i336.i, %preload472.i ]
  br i1 %extract234.i, label %preload478.i, label %postload479.i

preload478.i:                                     ; preds = %postload473.i
  %tmp7.i337.i = bitcast double %245 to i64
  %tmp6.i338.i = bitcast i64 %tmp7.i337.i to <2 x i32>
  %tmp2.i339.i = shufflevector <2 x i32> %tmp6.i338.i, <2 x i32> undef, <16 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %322 = bitcast <16 x i32> %tmp2.i339.i to <16 x float>
  %323 = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %322, i16 -4, i8* bitcast ([16 x i32]* @const_vector_zeros to i8*), i32 0, i32 0, i32 0) nounwind
  %tmp5.i.i340.i = load <8 x i64>* bitcast ([16 x i32]* @const_vector_msb to <8 x i64>*), align 64
  %conv.i.i.i341.i = bitcast <16 x float> %323 to <16 x i32>
  %conv2.i.i.i342.i = bitcast <8 x i64> %tmp5.i.i340.i to <16 x i32>
  %324 = call <16 x i32> @llvm.x86.mic.and.pi(<16 x i32> %conv.i.i.i341.i, <16 x i32> %conv2.i.i.i342.i) nounwind
  %325 = call i16 @llvm.x86.mic.cmpeq.pi(<16 x i32> %324, <16 x i32> %conv2.i.i.i342.i) nounwind
  %326 = call i32 @llvm.x86.mic.kortestc(i16 %325, i16 %325) nounwind
  %tobool.i.i343.i = icmp ne i32 %326, 0
  %cond.i.i344.i = zext i1 %tobool.i.i343.i to i32
  br label %postload479.i

postload479.i:                                    ; preds = %preload478.i, %postload473.i
  %phi480.i = phi i32 [ undef, %postload473.i ], [ %cond.i.i344.i, %preload478.i ]
  %temp.vect268.i = insertelement <16 x i32> undef, i32 %phi390.i, i32 0
  %temp.vect269.i = insertelement <16 x i32> %temp.vect268.i, i32 %phi396.i, i32 1
  %temp.vect270.i = insertelement <16 x i32> %temp.vect269.i, i32 %phi402.i, i32 2
  %temp.vect271.i = insertelement <16 x i32> %temp.vect270.i, i32 %phi408.i, i32 3
  %temp.vect272.i = insertelement <16 x i32> %temp.vect271.i, i32 %phi414.i, i32 4
  %temp.vect273.i = insertelement <16 x i32> %temp.vect272.i, i32 %phi420.i, i32 5
  %temp.vect274.i = insertelement <16 x i32> %temp.vect273.i, i32 %phi426.i, i32 6
  %temp.vect275.i = insertelement <16 x i32> %temp.vect274.i, i32 %phi432.i, i32 7
  %temp.vect276.i = insertelement <16 x i32> %temp.vect275.i, i32 %phi438.i, i32 8
  %temp.vect277.i = insertelement <16 x i32> %temp.vect276.i, i32 %phi444.i, i32 9
  %temp.vect278.i = insertelement <16 x i32> %temp.vect277.i, i32 %phi450.i, i32 10
  %temp.vect279.i = insertelement <16 x i32> %temp.vect278.i, i32 %phi456.i, i32 11
  %temp.vect280.i = insertelement <16 x i32> %temp.vect279.i, i32 %phi462.i, i32 12
  %temp.vect281.i = insertelement <16 x i32> %temp.vect280.i, i32 %phi468.i, i32 13
  %temp.vect282.i = insertelement <16 x i32> %temp.vect281.i, i32 %phi474.i, i32 14
  %temp.vect283.i = insertelement <16 x i32> %temp.vect282.i, i32 %phi480.i, i32 15
  %327 = icmp ne <16 x i32> %temp.vect283.i, zeroinitializer
  %phitmp.i20288.i = select <16 x i1> %327, <16 x float> %vector285.i, <16 x float> %vector287.i
  %phitmp.i21293.i = select <16 x i1> %327, <16 x float> %vector290.i, <16 x float> %vector292.i
  %phitmp.i22298.i = select <16 x i1> %327, <16 x float> %vector295.i, <16 x float> %vector297.i
  %phitmp.i23303.i = select <16 x i1> %327, <16 x float> %vector300.i, <16 x float> %vector302.i
  %merge51304.i = select <16 x i1> %198, <16 x float> %phitmp.i23303.i, <16 x float> %vector300.i
  %extract356.i = extractelement <16 x float> %merge51304.i, i32 0
  %extract357.i = extractelement <16 x float> %merge51304.i, i32 1
  %extract358.i = extractelement <16 x float> %merge51304.i, i32 2
  %extract359.i = extractelement <16 x float> %merge51304.i, i32 3
  %extract360.i = extractelement <16 x float> %merge51304.i, i32 4
  %extract361.i = extractelement <16 x float> %merge51304.i, i32 5
  %extract362.i = extractelement <16 x float> %merge51304.i, i32 6
  %extract363.i = extractelement <16 x float> %merge51304.i, i32 7
  %extract364.i = extractelement <16 x float> %merge51304.i, i32 8
  %extract365.i = extractelement <16 x float> %merge51304.i, i32 9
  %extract366.i = extractelement <16 x float> %merge51304.i, i32 10
  %extract367.i = extractelement <16 x float> %merge51304.i, i32 11
  %extract368.i = extractelement <16 x float> %merge51304.i, i32 12
  %extract369.i = extractelement <16 x float> %merge51304.i, i32 13
  %extract370.i = extractelement <16 x float> %merge51304.i, i32 14
  %extract371.i = extractelement <16 x float> %merge51304.i, i32 15
  %merge49305.i = select <16 x i1> %198, <16 x float> %phitmp.i22298.i, <16 x float> %vector295.i
  %extract340.i = extractelement <16 x float> %merge49305.i, i32 0
  %extract341.i = extractelement <16 x float> %merge49305.i, i32 1
  %extract342.i = extractelement <16 x float> %merge49305.i, i32 2
  %extract343.i = extractelement <16 x float> %merge49305.i, i32 3
  %extract344.i = extractelement <16 x float> %merge49305.i, i32 4
  %extract345.i = extractelement <16 x float> %merge49305.i, i32 5
  %extract346.i = extractelement <16 x float> %merge49305.i, i32 6
  %extract347.i = extractelement <16 x float> %merge49305.i, i32 7
  %extract348.i = extractelement <16 x float> %merge49305.i, i32 8
  %extract349.i = extractelement <16 x float> %merge49305.i, i32 9
  %extract350.i = extractelement <16 x float> %merge49305.i, i32 10
  %extract351.i = extractelement <16 x float> %merge49305.i, i32 11
  %extract352.i = extractelement <16 x float> %merge49305.i, i32 12
  %extract353.i = extractelement <16 x float> %merge49305.i, i32 13
  %extract354.i = extractelement <16 x float> %merge49305.i, i32 14
  %extract355.i = extractelement <16 x float> %merge49305.i, i32 15
  %merge47306.i = select <16 x i1> %198, <16 x float> %phitmp.i21293.i, <16 x float> %vector290.i
  %extract324.i = extractelement <16 x float> %merge47306.i, i32 0
  %extract325.i = extractelement <16 x float> %merge47306.i, i32 1
  %extract326.i = extractelement <16 x float> %merge47306.i, i32 2
  %extract327.i = extractelement <16 x float> %merge47306.i, i32 3
  %extract328.i = extractelement <16 x float> %merge47306.i, i32 4
  %extract329.i = extractelement <16 x float> %merge47306.i, i32 5
  %extract330.i = extractelement <16 x float> %merge47306.i, i32 6
  %extract331.i = extractelement <16 x float> %merge47306.i, i32 7
  %extract332.i = extractelement <16 x float> %merge47306.i, i32 8
  %extract333.i = extractelement <16 x float> %merge47306.i, i32 9
  %extract334.i = extractelement <16 x float> %merge47306.i, i32 10
  %extract335.i = extractelement <16 x float> %merge47306.i, i32 11
  %extract336.i = extractelement <16 x float> %merge47306.i, i32 12
  %extract337.i = extractelement <16 x float> %merge47306.i, i32 13
  %extract338.i = extractelement <16 x float> %merge47306.i, i32 14
  %extract339.i = extractelement <16 x float> %merge47306.i, i32 15
  %merge307.i = select <16 x i1> %198, <16 x float> %phitmp.i20288.i, <16 x float> %vector285.i
  %extract308.i = extractelement <16 x float> %merge307.i, i32 0
  %extract309.i = extractelement <16 x float> %merge307.i, i32 1
  %extract310.i = extractelement <16 x float> %merge307.i, i32 2
  %extract311.i = extractelement <16 x float> %merge307.i, i32 3
  %extract312.i = extractelement <16 x float> %merge307.i, i32 4
  %extract313.i = extractelement <16 x float> %merge307.i, i32 5
  %extract314.i = extractelement <16 x float> %merge307.i, i32 6
  %extract315.i = extractelement <16 x float> %merge307.i, i32 7
  %extract316.i = extractelement <16 x float> %merge307.i, i32 8
  %extract317.i = extractelement <16 x float> %merge307.i, i32 9
  %extract318.i = extractelement <16 x float> %merge307.i, i32 10
  %extract319.i = extractelement <16 x float> %merge307.i, i32 11
  %extract320.i = extractelement <16 x float> %merge307.i, i32 12
  %extract321.i = extractelement <16 x float> %merge307.i, i32 13
  %extract322.i = extractelement <16 x float> %merge307.i, i32 14
  %extract323.i = extractelement <16 x float> %merge307.i, i32 15
  %328 = insertelement <4 x float> undef, float %extract308.i, i32 0
  %329 = insertelement <4 x float> undef, float %extract309.i, i32 0
  %330 = insertelement <4 x float> undef, float %extract310.i, i32 0
  %331 = insertelement <4 x float> undef, float %extract311.i, i32 0
  %332 = insertelement <4 x float> undef, float %extract312.i, i32 0
  %333 = insertelement <4 x float> undef, float %extract313.i, i32 0
  %334 = insertelement <4 x float> undef, float %extract314.i, i32 0
  %335 = insertelement <4 x float> undef, float %extract315.i, i32 0
  %336 = insertelement <4 x float> undef, float %extract316.i, i32 0
  %337 = insertelement <4 x float> undef, float %extract317.i, i32 0
  %338 = insertelement <4 x float> undef, float %extract318.i, i32 0
  %339 = insertelement <4 x float> undef, float %extract319.i, i32 0
  %340 = insertelement <4 x float> undef, float %extract320.i, i32 0
  %341 = insertelement <4 x float> undef, float %extract321.i, i32 0
  %342 = insertelement <4 x float> undef, float %extract322.i, i32 0
  %343 = insertelement <4 x float> undef, float %extract323.i, i32 0
  %344 = insertelement <4 x float> %328, float %extract324.i, i32 1
  %345 = insertelement <4 x float> %329, float %extract325.i, i32 1
  %346 = insertelement <4 x float> %330, float %extract326.i, i32 1
  %347 = insertelement <4 x float> %331, float %extract327.i, i32 1
  %348 = insertelement <4 x float> %332, float %extract328.i, i32 1
  %349 = insertelement <4 x float> %333, float %extract329.i, i32 1
  %350 = insertelement <4 x float> %334, float %extract330.i, i32 1
  %351 = insertelement <4 x float> %335, float %extract331.i, i32 1
  %352 = insertelement <4 x float> %336, float %extract332.i, i32 1
  %353 = insertelement <4 x float> %337, float %extract333.i, i32 1
  %354 = insertelement <4 x float> %338, float %extract334.i, i32 1
  %355 = insertelement <4 x float> %339, float %extract335.i, i32 1
  %356 = insertelement <4 x float> %340, float %extract336.i, i32 1
  %357 = insertelement <4 x float> %341, float %extract337.i, i32 1
  %358 = insertelement <4 x float> %342, float %extract338.i, i32 1
  %359 = insertelement <4 x float> %343, float %extract339.i, i32 1
  %360 = insertelement <4 x float> %344, float %extract340.i, i32 2
  %361 = insertelement <4 x float> %345, float %extract341.i, i32 2
  %362 = insertelement <4 x float> %346, float %extract342.i, i32 2
  %363 = insertelement <4 x float> %347, float %extract343.i, i32 2
  %364 = insertelement <4 x float> %348, float %extract344.i, i32 2
  %365 = insertelement <4 x float> %349, float %extract345.i, i32 2
  %366 = insertelement <4 x float> %350, float %extract346.i, i32 2
  %367 = insertelement <4 x float> %351, float %extract347.i, i32 2
  %368 = insertelement <4 x float> %352, float %extract348.i, i32 2
  %369 = insertelement <4 x float> %353, float %extract349.i, i32 2
  %370 = insertelement <4 x float> %354, float %extract350.i, i32 2
  %371 = insertelement <4 x float> %355, float %extract351.i, i32 2
  %372 = insertelement <4 x float> %356, float %extract352.i, i32 2
  %373 = insertelement <4 x float> %357, float %extract353.i, i32 2
  %374 = insertelement <4 x float> %358, float %extract354.i, i32 2
  %375 = insertelement <4 x float> %359, float %extract355.i, i32 2
  %376 = insertelement <4 x float> %360, float %extract356.i, i32 3
  %377 = insertelement <4 x float> %361, float %extract357.i, i32 3
  %378 = insertelement <4 x float> %362, float %extract358.i, i32 3
  %379 = insertelement <4 x float> %363, float %extract359.i, i32 3
  %380 = insertelement <4 x float> %364, float %extract360.i, i32 3
  %381 = insertelement <4 x float> %365, float %extract361.i, i32 3
  %382 = insertelement <4 x float> %366, float %extract362.i, i32 3
  %383 = insertelement <4 x float> %367, float %extract363.i, i32 3
  %384 = insertelement <4 x float> %368, float %extract364.i, i32 3
  %385 = insertelement <4 x float> %369, float %extract365.i, i32 3
  %386 = insertelement <4 x float> %370, float %extract366.i, i32 3
  %387 = insertelement <4 x float> %371, float %extract367.i, i32 3
  %388 = insertelement <4 x float> %372, float %extract368.i, i32 3
  %389 = insertelement <4 x float> %373, float %extract369.i, i32 3
  %390 = insertelement <4 x float> %374, float %extract370.i, i32 3
  %391 = insertelement <4 x float> %375, float %extract371.i, i32 3
  %392 = extractelement <16 x i32> %37, i32 0
  %393 = sext i32 %392 to i64
  %394 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %393
  %395 = extractelement <16 x i32> %37, i32 1
  %396 = sext i32 %395 to i64
  %397 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %396
  %398 = extractelement <16 x i32> %37, i32 2
  %399 = sext i32 %398 to i64
  %400 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %399
  %401 = extractelement <16 x i32> %37, i32 3
  %402 = sext i32 %401 to i64
  %403 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %402
  %404 = extractelement <16 x i32> %37, i32 4
  %405 = sext i32 %404 to i64
  %406 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %405
  %407 = extractelement <16 x i32> %37, i32 5
  %408 = sext i32 %407 to i64
  %409 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %408
  %410 = extractelement <16 x i32> %37, i32 6
  %411 = sext i32 %410 to i64
  %412 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %411
  %413 = extractelement <16 x i32> %37, i32 7
  %414 = sext i32 %413 to i64
  %415 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %414
  %416 = extractelement <16 x i32> %37, i32 8
  %417 = sext i32 %416 to i64
  %418 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %417
  %419 = extractelement <16 x i32> %37, i32 9
  %420 = sext i32 %419 to i64
  %421 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %420
  %422 = extractelement <16 x i32> %37, i32 10
  %423 = sext i32 %422 to i64
  %424 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %423
  %425 = extractelement <16 x i32> %37, i32 11
  %426 = sext i32 %425 to i64
  %427 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %426
  %428 = extractelement <16 x i32> %37, i32 12
  %429 = sext i32 %428 to i64
  %430 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %429
  %431 = extractelement <16 x i32> %37, i32 13
  %432 = sext i32 %431 to i64
  %433 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %432
  %434 = extractelement <16 x i32> %37, i32 14
  %435 = sext i32 %434 to i64
  %436 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %435
  %437 = extractelement <16 x i32> %37, i32 15
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %438
  store <4 x float> %376, <4 x float> addrspace(1)* %394, align 16
  store <4 x float> %377, <4 x float> addrspace(1)* %397, align 16
  store <4 x float> %378, <4 x float> addrspace(1)* %400, align 16
  store <4 x float> %379, <4 x float> addrspace(1)* %403, align 16
  store <4 x float> %380, <4 x float> addrspace(1)* %406, align 16
  store <4 x float> %381, <4 x float> addrspace(1)* %409, align 16
  store <4 x float> %382, <4 x float> addrspace(1)* %412, align 16
  store <4 x float> %383, <4 x float> addrspace(1)* %415, align 16
  store <4 x float> %384, <4 x float> addrspace(1)* %418, align 16
  store <4 x float> %385, <4 x float> addrspace(1)* %421, align 16
  store <4 x float> %386, <4 x float> addrspace(1)* %424, align 16
  store <4 x float> %387, <4 x float> addrspace(1)* %427, align 16
  store <4 x float> %388, <4 x float> addrspace(1)* %430, align 16
  store <4 x float> %389, <4 x float> addrspace(1)* %433, align 16
  store <4 x float> %390, <4 x float> addrspace(1)* %436, align 16
  store <4 x float> %391, <4 x float> addrspace(1)* %439, align 16
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.checkerboard2D_separated_args.exit

thenBB.i:                                         ; preds = %postload479.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.checkerboard2D_separated_args.exit: ; preds = %postload479.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, double, <4 x float>, <4 x float>, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__checkerboard2D_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float2 const, float4 const, float4 const", metadata !"opencl_checkerboard2D_locals_anchor", void (i8*)* @checkerboard2D}
!1 = metadata !{i32 0, i32 0, i32 0}
