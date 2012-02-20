; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__simpleConvolution_original(float addrspace(1)* nocapture, i32 addrspace(1)* nocapture, float addrspace(1)* nocapture, double, double) nounwind

declare i64 @get_global_id(i32)

declare void @____Vectorized_.simpleConvolution_original(float addrspace(1)* nocapture, i32 addrspace(1)* nocapture, float addrspace(1)* nocapture, double, double) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare i32 @masked_load_align4_0(i1, i32 addrspace(1)*)

declare float @masked_load_align4_1(i1, float addrspace(1)*)

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__simpleConvolution_separated_args(float addrspace(1)* nocapture %output, i32 addrspace(1)* nocapture %input, float addrspace(1)* nocapture %mask, double %inputDimensions.coerce, double %maskDimensions.coerce, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %tmp6 = bitcast double %inputDimensions.coerce to i64
  %tmp5 = bitcast i64 %tmp6 to <2 x i32>
  %tmp3 = bitcast double %maskDimensions.coerce to i64
  %tmp2 = bitcast i64 %tmp3 to <2 x i32>
  %0 = extractelement <2 x i32> %tmp5, i32 0
  %1 = extractelement <2 x i32> %tmp5, i32 1
  %2 = icmp eq i32 %0, 0
  %3 = select i1 %2, i32 1, i32 %0
  %4 = extractelement <2 x i32> %tmp2, i32 0
  %5 = extractelement <2 x i32> %tmp2, i32 1
  %6 = add i32 %4, -1
  %7 = lshr i32 %6, 1
  %8 = add i32 %5, -1
  %9 = lshr i32 %8, 1
  %10 = add i32 %0, -1
  %11 = add i32 %1, -1
  %tmp32 = icmp ugt i32 %0, 1
  %umax33 = select i1 %tmp32, i32 %0, i32 1
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %12 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %15 = load i64* %14, align 8
  %16 = add i64 %13, %15
  %17 = trunc i64 %16 to i32
  %18 = urem i32 %17, %3
  %19 = udiv i32 %17, %3
  %20 = sub i32 %18, %7
  %21 = icmp ult i32 %18, %7
  %. = select i1 %21, i32 0, i32 %20
  %22 = add i32 %18, %7
  %23 = icmp ult i32 %22, %0
  %24 = select i1 %23, i32 %22, i32 %10
  %25 = add i32 %19, %9
  %26 = icmp ult i32 %25, %1
  %27 = select i1 %26, i32 %25, i32 %11
  %28 = icmp ugt i32 %., %24
  br i1 %28, label %._crit_edge13, label %bb.nph12

bb.nph12:                                         ; preds = %SyncBB
  %29 = icmp ult i32 %19, %9
  %30 = sub i32 %19, %9
  %.1 = select i1 %29, i32 0, i32 %30
  %31 = icmp ugt i32 %.1, %27
  br i1 %31, label %bb.nph12.split.us, label %bb.nph12.bb.nph12.split_crit_edge

bb.nph12.bb.nph12.split_crit_edge:                ; preds = %bb.nph12
  %tmp34 = udiv i32 %17, %umax33
  %tmp35 = icmp ugt i32 %9, %tmp34
  %umax36 = select i1 %tmp35, i32 %9, i32 %tmp34
  %tmp50 = add i32 %umax36, 1
  %tmp51 = sub i32 %tmp50, %9
  %tmp56 = icmp ugt i32 %18, %7
  %umax57 = select i1 %tmp56, i32 %18, i32 %7
  %tmp66 = sub i32 %umax36, %9
  %tmp67 = mul i32 %0, %tmp66
  %tmp68 = add i32 %umax57, %tmp67
  %tmp69 = sub i32 %tmp68, %7
  %tmp73 = sub i32 %umax36, %tmp34
  %tmp74 = mul i32 %4, %tmp73
  %tmp75 = add i32 %umax57, %tmp74
  %tmp76 = sub i32 %tmp75, %18
  %tmp79 = add i32 %umax57, 1
  %tmp80 = sub i32 %tmp79, %7
  br label %bb.nph

bb.nph12.split.us:                                ; preds = %bb.nph12
  %tmp16 = icmp ugt i32 %18, %7
  %umax = select i1 %tmp16, i32 %18, i32 %7
  %tmp17 = add i32 %umax, 1
  %tmp18 = sub i32 %tmp17, %7
  br label %32

; <label>:32                                      ; preds = %32, %bb.nph12.split.us
  %indvar = phi i32 [ %indvar.next, %32 ], [ 0, %bb.nph12.split.us ]
  %tmp19 = add i32 %tmp18, %indvar
  %33 = icmp ugt i32 %tmp19, %24
  %indvar.next = add i32 %indvar, 1
  br i1 %33, label %._crit_edge13, label %32

bb.nph:                                           ; preds = %._crit_edge, %bb.nph12.bb.nph12.split_crit_edge
  %indvar23 = phi i32 [ 0, %bb.nph12.bb.nph12.split_crit_edge ], [ %indvar.next24, %._crit_edge ]
  %sumFX.110 = phi float [ 0.000000e+00, %bb.nph12.bb.nph12.split_crit_edge ], [ %43, %._crit_edge ]
  %tmp70 = add i32 %tmp69, %indvar23
  %tmp77 = add i32 %tmp76, %indvar23
  %tmp81 = add i32 %tmp80, %indvar23
  br label %34

; <label>:34                                      ; preds = %34, %bb.nph
  %indvar20 = phi i32 [ 0, %bb.nph ], [ %indvar.next21, %34 ]
  %sumFX.08 = phi float [ %sumFX.110, %bb.nph ], [ %43, %34 ]
  %tmp53 = mul i32 %0, %indvar20
  %tmp49 = add i32 %tmp70, %tmp53
  %tmp72 = mul i32 %4, %indvar20
  %tmp42 = add i32 %tmp77, %tmp72
  %35 = zext i32 %tmp49 to i64
  %36 = getelementptr inbounds i32 addrspace(1)* %input, i64 %35
  %37 = load i32 addrspace(1)* %36, align 4
  %38 = uitofp i32 %37 to float
  %39 = zext i32 %tmp42 to i64
  %40 = getelementptr inbounds float addrspace(1)* %mask, i64 %39
  %41 = load float addrspace(1)* %40, align 4
  %42 = fmul float %38, %41
  %43 = fadd float %sumFX.08, %42
  %tmp52 = add i32 %tmp51, %indvar20
  %44 = icmp ugt i32 %tmp52, %27
  %indvar.next21 = add i32 %indvar20, 1
  br i1 %44, label %._crit_edge, label %34

._crit_edge:                                      ; preds = %34
  %45 = icmp ugt i32 %tmp81, %24
  %indvar.next24 = add i32 %indvar23, 1
  br i1 %45, label %._crit_edge13, label %bb.nph

._crit_edge13:                                    ; preds = %._crit_edge, %32, %SyncBB
  %sumFX.1.lcssa = phi float [ 0.000000e+00, %SyncBB ], [ %43, %._crit_edge ], [ 0.000000e+00, %32 ]
  %46 = fadd float %sumFX.1.lcssa, 5.000000e-01
  %47 = and i64 %16, 4294967295
  %48 = getelementptr inbounds float addrspace(1)* %output, i64 %47
  store float %46, float addrspace(1)* %48, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB83

thenBB:                                           ; preds = %._crit_edge13
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB83:                                         ; preds = %._crit_edge13
  ret void
}

define void @____Vectorized_.simpleConvolution_separated_args(float addrspace(1)* nocapture %output, i32 addrspace(1)* nocapture %input, float addrspace(1)* nocapture %mask, double %inputDimensions.coerce, double %maskDimensions.coerce, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
header:
  %tmp6 = bitcast double %inputDimensions.coerce to i64
  %tmp5 = bitcast i64 %tmp6 to <2 x i32>
  %scalar = extractelement <2 x i32> %tmp5, i32 0
  %temp79 = insertelement <16 x i32> undef, i32 %scalar, i32 0
  %vector80 = shufflevector <16 x i32> %temp79, <16 x i32> undef, <16 x i32> zeroinitializer
  %scalar1 = extractelement <2 x i32> %tmp5, i32 1
  %temp85 = insertelement <16 x i32> undef, i32 %scalar1, i32 0
  %vector86 = shufflevector <16 x i32> %temp85, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp3 = bitcast double %maskDimensions.coerce to i64
  %tmp2 = bitcast i64 %tmp3 to <2 x i32>
  %scalar2 = extractelement <2 x i32> %tmp2, i32 0
  %scalar3 = extractelement <2 x i32> %tmp2, i32 1
  %0 = icmp eq i32 %scalar, 0
  %1 = select i1 %0, i32 1, i32 %scalar
  %temp = insertelement <16 x i32> undef, i32 %1, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %2 = add i32 %scalar2, -1
  %3 = lshr i32 %2, 1
  %temp76 = insertelement <16 x i32> undef, i32 %3, i32 0
  %vector77 = shufflevector <16 x i32> %temp76, <16 x i32> undef, <16 x i32> zeroinitializer
  %4 = add i32 %scalar3, -1
  %5 = lshr i32 %4, 1
  %temp83 = insertelement <16 x i32> undef, i32 %5, i32 0
  %vector84 = shufflevector <16 x i32> %temp83, <16 x i32> undef, <16 x i32> zeroinitializer
  %6 = add i32 %scalar, -1
  %temp81 = insertelement <16 x i32> undef, i32 %6, i32 0
  %vector82 = shufflevector <16 x i32> %temp81, <16 x i32> undef, <16 x i32> zeroinitializer
  %7 = add i32 %scalar1, -1
  %temp87 = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector88 = shufflevector <16 x i32> %temp87, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp108 = insertelement <16 x i32> undef, i32 %scalar2, i32 0
  %vector109 = shufflevector <16 x i32> %temp108, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp32 = icmp ugt i32 %scalar, 1
  %umax33 = select i1 %tmp32, i32 %scalar, i32 1
  %temp96 = insertelement <16 x i32> undef, i32 %umax33, i32 0
  %vector97 = shufflevector <16 x i32> %temp96, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB408

SyncBB408:                                        ; preds = %thenBB, %header
  %CurrWI..0 = phi i64 [ 0, %header ], [ %"CurrWI++", %thenBB ]
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %broadcast1 = insertelement <16 x i64> undef, i64 %12, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %13 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %14 = trunc <16 x i64> %13 to <16 x i32>
  %15 = urem <16 x i32> %14, %vector
  %16 = udiv <16 x i32> %14, %vector
  %17 = sub <16 x i32> %15, %vector77
  %18 = icmp ult <16 x i32> %15, %vector77
  %.78 = select <16 x i1> %18, <16 x i32> zeroinitializer, <16 x i32> %17
  %19 = add <16 x i32> %15, %vector77
  %20 = icmp ult <16 x i32> %19, %vector80
  %21 = select <16 x i1> %20, <16 x i32> %19, <16 x i32> %vector82
  %22 = add <16 x i32> %16, %vector84
  %23 = icmp ult <16 x i32> %22, %vector86
  %24 = select <16 x i1> %23, <16 x i32> %22, <16 x i32> %vector88
  %25 = icmp ugt <16 x i32> %.78, %21
  %Mneg89 = xor <16 x i1> %25, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %ipred.i = bitcast <16 x i1> %Mneg89 to i16
  %val.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  br i1 %res.i, label %bb.nph12, label %._crit_edge13

bb.nph12:                                         ; preds = %SyncBB408
  %26 = icmp ult <16 x i32> %16, %vector84
  %27 = sub <16 x i32> %16, %vector84
  %.192 = select <16 x i1> %26, <16 x i32> zeroinitializer, <16 x i32> %27
  %28 = icmp ugt <16 x i32> %.192, %24
  %Mneg1193 = xor <16 x i1> %28, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94 = and <16 x i1> %Mneg1193, %Mneg89
  %bb.nph12_to_bb.nph12.split.us95 = and <16 x i1> %28, %Mneg89
  %tmp3498 = udiv <16 x i32> %14, %vector97
  %tmp35 = icmp ugt <16 x i32> %vector84, %tmp3498
  %umax3699 = select <16 x i1> %tmp35, <16 x i32> %vector84, <16 x i32> %tmp3498
  %tmp50100 = add <16 x i32> %umax3699, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp51101 = sub <16 x i32> %tmp50100, %vector84
  %tmp56 = icmp ugt <16 x i32> %15, %vector77
  %umax57102 = select <16 x i1> %tmp56, <16 x i32> %15, <16 x i32> %vector77
  %tmp66103 = sub <16 x i32> %umax3699, %vector84
  %tmp67104 = mul <16 x i32> %vector80, %tmp66103
  %tmp68105 = add <16 x i32> %umax57102, %tmp67104
  %tmp69106 = sub <16 x i32> %tmp68105, %vector77
  %tmp73107 = sub <16 x i32> %umax3699, %tmp3498
  %tmp74110 = mul <16 x i32> %vector109, %tmp73107
  %tmp75111 = add <16 x i32> %umax57102, %tmp74110
  %tmp76112 = sub <16 x i32> %tmp75111, %15
  %tmp79113 = add <16 x i32> %umax57102, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp80114 = sub <16 x i32> %tmp79113, %vector77
  %ipred.i1 = bitcast <16 x i1> %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94 to i16
  %val.i2 = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1, i16 %ipred.i1) nounwind
  %tmp.i3 = and i32 %val.i2, 1
  %res.i4 = icmp eq i32 %tmp.i3, 0
  br i1 %res.i4, label %bb.nph.preheader, label %bb.nph12.split.us

bb.nph.preheader:                                 ; preds = %bb.nph12
  %negIncomingLoopMask44115 = or <16 x i1> %28, %25
  br label %bb.nph

bb.nph12.split.us:                                ; preds = %bb.nph12, %phi-split-bb.loopexit
  %vectorPHI = phi <16 x float> [ %phitmp407, %phi-split-bb.loopexit ], [ zeroinitializer, %bb.nph12 ]
  %tmp16 = icmp ugt <16 x i32> %15, %vector77
  %umax117 = select <16 x i1> %tmp16, <16 x i32> %15, <16 x i32> %vector77
  %tmp17118 = add <16 x i32> %umax117, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp18119 = sub <16 x i32> %tmp17118, %vector77
  %ipred.i5 = bitcast <16 x i1> %bb.nph12_to_bb.nph12.split.us95 to i16
  %val.i6 = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i5, i16 %ipred.i5) nounwind
  %tmp.i7 = and i32 %val.i6, 1
  %res.i8 = icmp eq i32 %tmp.i7, 0
  br i1 %res.i8, label %.preheader, label %._crit_edge13

.preheader:                                       ; preds = %bb.nph12.split.us
  %.not = xor <16 x i1> %28, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %negIncomingLoopMask120 = or <16 x i1> %25, %.not
  br label %29

; <label>:29                                      ; preds = %29, %.preheader
  %vectorPHI121 = phi <16 x i1> [ %loop_mask12130, %29 ], [ %negIncomingLoopMask120, %.preheader ]
  %vectorPHI123 = phi <16 x i1> [ %local_edge132, %29 ], [ %bb.nph12_to_bb.nph12.split.us95, %.preheader ]
  %indvar = phi i32 [ %indvar.next, %29 ], [ 0, %.preheader ]
  %temp124 = insertelement <16 x i32> undef, i32 %indvar, i32 0
  %vector125 = shufflevector <16 x i32> %temp124, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp19126 = add <16 x i32> %tmp18119, %vector125
  %30 = icmp ugt <16 x i32> %tmp19126, %21
  %indvar.next = add i32 %indvar, 1
  %notCond127 = xor <16 x i1> %30, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr128 = and <16 x i1> %vectorPHI123, %30
  %loop_mask12130 = or <16 x i1> %vectorPHI121, %who_left_tr128
  %curr_loop_mask131 = or <16 x i1> %loop_mask12130, %who_left_tr128
  %ipred.i9 = bitcast <16 x i1> %curr_loop_mask131 to i16
  %val.i10 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9, i16 %ipred.i9) nounwind
  %tmp.i11 = and i32 %val.i10, 1
  %res.i12 = icmp eq i32 %tmp.i11, 0
  %local_edge132 = and <16 x i1> %vectorPHI123, %notCond127
  br i1 %res.i12, label %29, label %._crit_edge13

bb.nph:                                           ; preds = %._crit_edge, %bb.nph.preheader
  %vectorPHI133 = phi <16 x i1> [ %ever_left_loop30245, %._crit_edge ], [ zeroinitializer, %bb.nph.preheader ]
  %vectorPHI134 = phi <16 x i1> [ %loop_mask32246, %._crit_edge ], [ %negIncomingLoopMask44115, %bb.nph.preheader ]
  %vectorPHI135 = phi <16 x float> [ %out_sel233, %._crit_edge ], [ undef, %bb.nph.preheader ]
  %vectorPHI136 = phi <16 x i1> [ %local_edge37248, %._crit_edge ], [ %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94, %bb.nph.preheader ]
  %indvar23 = phi i32 [ %indvar.next24, %._crit_edge ], [ 0, %bb.nph.preheader ]
  %vectorPHI137 = phi <16 x float> [ %out_sel233, %._crit_edge ], [ zeroinitializer, %bb.nph.preheader ]
  %temp138 = insertelement <16 x i32> undef, i32 %indvar23, i32 0
  %vector139 = shufflevector <16 x i32> %temp138, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp70140 = add <16 x i32> %tmp69106, %vector139
  %tmp77141 = add <16 x i32> %tmp76112, %vector139
  %tmp81142 = add <16 x i32> %tmp80114, %vector139
  %negIncomingLoopMask49143 = xor <16 x i1> %vectorPHI136, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %31

; <label>:31                                      ; preds = %postload390, %bb.nph
  %vectorPHI144 = phi <16 x i1> [ %negIncomingLoopMask49143, %bb.nph ], [ %loop_mask19240, %postload390 ]
  %vectorPHI145 = phi <16 x i1> [ zeroinitializer, %bb.nph ], [ %ever_left_loop17239, %postload390 ]
  %vectorPHI146 = phi <16 x float> [ %vectorPHI135, %bb.nph ], [ %out_sel233, %postload390 ]
  %vectorPHI147 = phi <16 x i1> [ %vectorPHI136, %bb.nph ], [ %local_edge25242, %postload390 ]
  %indvar20 = phi i32 [ 0, %bb.nph ], [ %indvar.next21, %postload390 ]
  %vectorPHI148 = phi <16 x float> [ %vectorPHI137, %bb.nph ], [ %129, %postload390 ]
  %temp234 = insertelement <16 x i32> undef, i32 %indvar20, i32 0
  %vector235 = shufflevector <16 x i32> %temp234, <16 x i32> undef, <16 x i32> zeroinitializer
  %extract170 = extractelement <16 x i1> %vectorPHI147, i32 0
  %extract171 = extractelement <16 x i1> %vectorPHI147, i32 1
  %extract172 = extractelement <16 x i1> %vectorPHI147, i32 2
  %extract173 = extractelement <16 x i1> %vectorPHI147, i32 3
  %extract174 = extractelement <16 x i1> %vectorPHI147, i32 4
  %extract175 = extractelement <16 x i1> %vectorPHI147, i32 5
  %extract176 = extractelement <16 x i1> %vectorPHI147, i32 6
  %extract177 = extractelement <16 x i1> %vectorPHI147, i32 7
  %extract178 = extractelement <16 x i1> %vectorPHI147, i32 8
  %extract179 = extractelement <16 x i1> %vectorPHI147, i32 9
  %extract180 = extractelement <16 x i1> %vectorPHI147, i32 10
  %extract181 = extractelement <16 x i1> %vectorPHI147, i32 11
  %extract182 = extractelement <16 x i1> %vectorPHI147, i32 12
  %extract183 = extractelement <16 x i1> %vectorPHI147, i32 13
  %extract184 = extractelement <16 x i1> %vectorPHI147, i32 14
  %extract185 = extractelement <16 x i1> %vectorPHI147, i32 15
  %tmp53 = mul i32 %scalar, %indvar20
  %temp149 = insertelement <16 x i32> undef, i32 %tmp53, i32 0
  %vector150 = shufflevector <16 x i32> %temp149, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp49151 = add <16 x i32> %tmp70140, %vector150
  %tmp72 = mul i32 %scalar2, %indvar20
  %temp152 = insertelement <16 x i32> undef, i32 %tmp72, i32 0
  %vector153 = shufflevector <16 x i32> %temp152, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp42154 = add <16 x i32> %tmp77141, %vector153
  %32 = extractelement <16 x i32> %tmp49151, i32 1
  %33 = zext i32 %32 to i64
  %34 = getelementptr inbounds i32 addrspace(1)* %input, i64 %33
  %35 = extractelement <16 x i32> %tmp49151, i32 2
  %36 = zext i32 %35 to i64
  %37 = getelementptr inbounds i32 addrspace(1)* %input, i64 %36
  %38 = extractelement <16 x i32> %tmp49151, i32 3
  %39 = zext i32 %38 to i64
  %40 = getelementptr inbounds i32 addrspace(1)* %input, i64 %39
  %41 = extractelement <16 x i32> %tmp49151, i32 4
  %42 = zext i32 %41 to i64
  %43 = getelementptr inbounds i32 addrspace(1)* %input, i64 %42
  %44 = extractelement <16 x i32> %tmp49151, i32 5
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds i32 addrspace(1)* %input, i64 %45
  %47 = extractelement <16 x i32> %tmp49151, i32 6
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds i32 addrspace(1)* %input, i64 %48
  %50 = extractelement <16 x i32> %tmp49151, i32 7
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds i32 addrspace(1)* %input, i64 %51
  %53 = extractelement <16 x i32> %tmp49151, i32 8
  %54 = zext i32 %53 to i64
  %55 = getelementptr inbounds i32 addrspace(1)* %input, i64 %54
  %56 = extractelement <16 x i32> %tmp49151, i32 9
  %57 = zext i32 %56 to i64
  %58 = getelementptr inbounds i32 addrspace(1)* %input, i64 %57
  %59 = extractelement <16 x i32> %tmp49151, i32 10
  %60 = zext i32 %59 to i64
  %61 = getelementptr inbounds i32 addrspace(1)* %input, i64 %60
  %62 = extractelement <16 x i32> %tmp49151, i32 11
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds i32 addrspace(1)* %input, i64 %63
  %65 = extractelement <16 x i32> %tmp49151, i32 12
  %66 = zext i32 %65 to i64
  %67 = getelementptr inbounds i32 addrspace(1)* %input, i64 %66
  %68 = extractelement <16 x i32> %tmp49151, i32 13
  %69 = zext i32 %68 to i64
  %70 = getelementptr inbounds i32 addrspace(1)* %input, i64 %69
  %71 = extractelement <16 x i32> %tmp49151, i32 14
  %72 = zext i32 %71 to i64
  %73 = getelementptr inbounds i32 addrspace(1)* %input, i64 %72
  %74 = extractelement <16 x i32> %tmp49151, i32 15
  %75 = zext i32 %74 to i64
  %76 = getelementptr inbounds i32 addrspace(1)* %input, i64 %75
  br i1 %extract170, label %preload, label %postload

preload:                                          ; preds = %31
  %77 = extractelement <16 x i32> %tmp49151, i32 0
  %78 = zext i32 %77 to i64
  %79 = getelementptr inbounds i32 addrspace(1)* %input, i64 %78
  %masked_load = load i32 addrspace(1)* %79, align 4
  %phitmp = uitofp i32 %masked_load to float
  br label %postload

postload:                                         ; preds = %preload, %31
  %phi = phi float [ 0.000000e+00, %31 ], [ %phitmp, %preload ]
  br i1 %extract171, label %preload302, label %postload303

preload302:                                       ; preds = %postload
  %masked_load268 = load i32 addrspace(1)* %34, align 4
  %phitmp392 = uitofp i32 %masked_load268 to float
  br label %postload303

postload303:                                      ; preds = %preload302, %postload
  %phi304 = phi float [ 0.000000e+00, %postload ], [ %phitmp392, %preload302 ]
  br i1 %extract172, label %preload308, label %postload309

preload308:                                       ; preds = %postload303
  %masked_load269 = load i32 addrspace(1)* %37, align 4
  %phitmp393 = uitofp i32 %masked_load269 to float
  br label %postload309

postload309:                                      ; preds = %preload308, %postload303
  %phi310 = phi float [ 0.000000e+00, %postload303 ], [ %phitmp393, %preload308 ]
  br i1 %extract173, label %preload314, label %postload315

preload314:                                       ; preds = %postload309
  %masked_load270 = load i32 addrspace(1)* %40, align 4
  %phitmp394 = uitofp i32 %masked_load270 to float
  br label %postload315

postload315:                                      ; preds = %preload314, %postload309
  %phi316 = phi float [ 0.000000e+00, %postload309 ], [ %phitmp394, %preload314 ]
  br i1 %extract174, label %preload320, label %postload321

preload320:                                       ; preds = %postload315
  %masked_load271 = load i32 addrspace(1)* %43, align 4
  %phitmp395 = uitofp i32 %masked_load271 to float
  br label %postload321

postload321:                                      ; preds = %preload320, %postload315
  %phi322 = phi float [ 0.000000e+00, %postload315 ], [ %phitmp395, %preload320 ]
  br i1 %extract175, label %preload326, label %postload327

preload326:                                       ; preds = %postload321
  %masked_load272 = load i32 addrspace(1)* %46, align 4
  %phitmp396 = uitofp i32 %masked_load272 to float
  br label %postload327

postload327:                                      ; preds = %preload326, %postload321
  %phi328 = phi float [ 0.000000e+00, %postload321 ], [ %phitmp396, %preload326 ]
  br i1 %extract176, label %preload332, label %postload333

preload332:                                       ; preds = %postload327
  %masked_load273 = load i32 addrspace(1)* %49, align 4
  %phitmp397 = uitofp i32 %masked_load273 to float
  br label %postload333

postload333:                                      ; preds = %preload332, %postload327
  %phi334 = phi float [ 0.000000e+00, %postload327 ], [ %phitmp397, %preload332 ]
  br i1 %extract177, label %preload338, label %postload339

preload338:                                       ; preds = %postload333
  %masked_load274 = load i32 addrspace(1)* %52, align 4
  %phitmp398 = uitofp i32 %masked_load274 to float
  br label %postload339

postload339:                                      ; preds = %preload338, %postload333
  %phi340 = phi float [ 0.000000e+00, %postload333 ], [ %phitmp398, %preload338 ]
  br i1 %extract178, label %preload344, label %postload345

preload344:                                       ; preds = %postload339
  %masked_load275 = load i32 addrspace(1)* %55, align 4
  %phitmp399 = uitofp i32 %masked_load275 to float
  br label %postload345

postload345:                                      ; preds = %preload344, %postload339
  %phi346 = phi float [ 0.000000e+00, %postload339 ], [ %phitmp399, %preload344 ]
  br i1 %extract179, label %preload350, label %postload351

preload350:                                       ; preds = %postload345
  %masked_load276 = load i32 addrspace(1)* %58, align 4
  %phitmp400 = uitofp i32 %masked_load276 to float
  br label %postload351

postload351:                                      ; preds = %preload350, %postload345
  %phi352 = phi float [ 0.000000e+00, %postload345 ], [ %phitmp400, %preload350 ]
  br i1 %extract180, label %preload356, label %postload357

preload356:                                       ; preds = %postload351
  %masked_load277 = load i32 addrspace(1)* %61, align 4
  %phitmp401 = uitofp i32 %masked_load277 to float
  br label %postload357

postload357:                                      ; preds = %preload356, %postload351
  %phi358 = phi float [ 0.000000e+00, %postload351 ], [ %phitmp401, %preload356 ]
  br i1 %extract181, label %preload362, label %postload363

preload362:                                       ; preds = %postload357
  %masked_load278 = load i32 addrspace(1)* %64, align 4
  %phitmp402 = uitofp i32 %masked_load278 to float
  br label %postload363

postload363:                                      ; preds = %preload362, %postload357
  %phi364 = phi float [ 0.000000e+00, %postload357 ], [ %phitmp402, %preload362 ]
  br i1 %extract182, label %preload368, label %postload369

preload368:                                       ; preds = %postload363
  %masked_load279 = load i32 addrspace(1)* %67, align 4
  %phitmp403 = uitofp i32 %masked_load279 to float
  br label %postload369

postload369:                                      ; preds = %preload368, %postload363
  %phi370 = phi float [ 0.000000e+00, %postload363 ], [ %phitmp403, %preload368 ]
  br i1 %extract183, label %preload374, label %postload375

preload374:                                       ; preds = %postload369
  %masked_load280 = load i32 addrspace(1)* %70, align 4
  %phitmp404 = uitofp i32 %masked_load280 to float
  br label %postload375

postload375:                                      ; preds = %preload374, %postload369
  %phi376 = phi float [ 0.000000e+00, %postload369 ], [ %phitmp404, %preload374 ]
  br i1 %extract184, label %preload380, label %postload381

preload380:                                       ; preds = %postload375
  %masked_load281 = load i32 addrspace(1)* %73, align 4
  %phitmp405 = uitofp i32 %masked_load281 to float
  br label %postload381

postload381:                                      ; preds = %preload380, %postload375
  %phi382 = phi float [ 0.000000e+00, %postload375 ], [ %phitmp405, %preload380 ]
  br i1 %extract185, label %preload386, label %postload387

preload386:                                       ; preds = %postload381
  %masked_load282 = load i32 addrspace(1)* %76, align 4
  %phitmp406 = uitofp i32 %masked_load282 to float
  br label %postload387

postload387:                                      ; preds = %preload386, %postload381
  %phi388 = phi float [ 0.000000e+00, %postload381 ], [ %phitmp406, %preload386 ]
  %temp.vect = insertelement <16 x float> undef, float %phi, i32 0
  %temp.vect202 = insertelement <16 x float> %temp.vect, float %phi304, i32 1
  %temp.vect203 = insertelement <16 x float> %temp.vect202, float %phi310, i32 2
  %temp.vect204 = insertelement <16 x float> %temp.vect203, float %phi316, i32 3
  %temp.vect205 = insertelement <16 x float> %temp.vect204, float %phi322, i32 4
  %temp.vect206 = insertelement <16 x float> %temp.vect205, float %phi328, i32 5
  %temp.vect207 = insertelement <16 x float> %temp.vect206, float %phi334, i32 6
  %temp.vect208 = insertelement <16 x float> %temp.vect207, float %phi340, i32 7
  %temp.vect209 = insertelement <16 x float> %temp.vect208, float %phi346, i32 8
  %temp.vect210 = insertelement <16 x float> %temp.vect209, float %phi352, i32 9
  %temp.vect211 = insertelement <16 x float> %temp.vect210, float %phi358, i32 10
  %temp.vect212 = insertelement <16 x float> %temp.vect211, float %phi364, i32 11
  %temp.vect213 = insertelement <16 x float> %temp.vect212, float %phi370, i32 12
  %temp.vect214 = insertelement <16 x float> %temp.vect213, float %phi376, i32 13
  %temp.vect215 = insertelement <16 x float> %temp.vect214, float %phi382, i32 14
  %temp.vect216 = insertelement <16 x float> %temp.vect215, float %phi388, i32 15
  %80 = extractelement <16 x i32> %tmp42154, i32 1
  %81 = zext i32 %80 to i64
  %82 = getelementptr inbounds float addrspace(1)* %mask, i64 %81
  %83 = extractelement <16 x i32> %tmp42154, i32 2
  %84 = zext i32 %83 to i64
  %85 = getelementptr inbounds float addrspace(1)* %mask, i64 %84
  %86 = extractelement <16 x i32> %tmp42154, i32 3
  %87 = zext i32 %86 to i64
  %88 = getelementptr inbounds float addrspace(1)* %mask, i64 %87
  %89 = extractelement <16 x i32> %tmp42154, i32 4
  %90 = zext i32 %89 to i64
  %91 = getelementptr inbounds float addrspace(1)* %mask, i64 %90
  %92 = extractelement <16 x i32> %tmp42154, i32 5
  %93 = zext i32 %92 to i64
  %94 = getelementptr inbounds float addrspace(1)* %mask, i64 %93
  %95 = extractelement <16 x i32> %tmp42154, i32 6
  %96 = zext i32 %95 to i64
  %97 = getelementptr inbounds float addrspace(1)* %mask, i64 %96
  %98 = extractelement <16 x i32> %tmp42154, i32 7
  %99 = zext i32 %98 to i64
  %100 = getelementptr inbounds float addrspace(1)* %mask, i64 %99
  %101 = extractelement <16 x i32> %tmp42154, i32 8
  %102 = zext i32 %101 to i64
  %103 = getelementptr inbounds float addrspace(1)* %mask, i64 %102
  %104 = extractelement <16 x i32> %tmp42154, i32 9
  %105 = zext i32 %104 to i64
  %106 = getelementptr inbounds float addrspace(1)* %mask, i64 %105
  %107 = extractelement <16 x i32> %tmp42154, i32 10
  %108 = zext i32 %107 to i64
  %109 = getelementptr inbounds float addrspace(1)* %mask, i64 %108
  %110 = extractelement <16 x i32> %tmp42154, i32 11
  %111 = zext i32 %110 to i64
  %112 = getelementptr inbounds float addrspace(1)* %mask, i64 %111
  %113 = extractelement <16 x i32> %tmp42154, i32 12
  %114 = zext i32 %113 to i64
  %115 = getelementptr inbounds float addrspace(1)* %mask, i64 %114
  %116 = extractelement <16 x i32> %tmp42154, i32 13
  %117 = zext i32 %116 to i64
  %118 = getelementptr inbounds float addrspace(1)* %mask, i64 %117
  %119 = extractelement <16 x i32> %tmp42154, i32 14
  %120 = zext i32 %119 to i64
  %121 = getelementptr inbounds float addrspace(1)* %mask, i64 %120
  %122 = extractelement <16 x i32> %tmp42154, i32 15
  %123 = zext i32 %122 to i64
  %124 = getelementptr inbounds float addrspace(1)* %mask, i64 %123
  br i1 %extract170, label %preload299, label %postload300

preload299:                                       ; preds = %postload387
  %125 = extractelement <16 x i32> %tmp42154, i32 0
  %126 = zext i32 %125 to i64
  %127 = getelementptr inbounds float addrspace(1)* %mask, i64 %126
  %masked_load283 = load float addrspace(1)* %127, align 4
  br label %postload300

postload300:                                      ; preds = %preload299, %postload387
  %phi301 = phi float [ undef, %postload387 ], [ %masked_load283, %preload299 ]
  br i1 %extract171, label %preload305, label %postload306

preload305:                                       ; preds = %postload300
  %masked_load284 = load float addrspace(1)* %82, align 4
  br label %postload306

postload306:                                      ; preds = %preload305, %postload300
  %phi307 = phi float [ undef, %postload300 ], [ %masked_load284, %preload305 ]
  br i1 %extract172, label %preload311, label %postload312

preload311:                                       ; preds = %postload306
  %masked_load285 = load float addrspace(1)* %85, align 4
  br label %postload312

postload312:                                      ; preds = %preload311, %postload306
  %phi313 = phi float [ undef, %postload306 ], [ %masked_load285, %preload311 ]
  br i1 %extract173, label %preload317, label %postload318

preload317:                                       ; preds = %postload312
  %masked_load286 = load float addrspace(1)* %88, align 4
  br label %postload318

postload318:                                      ; preds = %preload317, %postload312
  %phi319 = phi float [ undef, %postload312 ], [ %masked_load286, %preload317 ]
  br i1 %extract174, label %preload323, label %postload324

preload323:                                       ; preds = %postload318
  %masked_load287 = load float addrspace(1)* %91, align 4
  br label %postload324

postload324:                                      ; preds = %preload323, %postload318
  %phi325 = phi float [ undef, %postload318 ], [ %masked_load287, %preload323 ]
  br i1 %extract175, label %preload329, label %postload330

preload329:                                       ; preds = %postload324
  %masked_load288 = load float addrspace(1)* %94, align 4
  br label %postload330

postload330:                                      ; preds = %preload329, %postload324
  %phi331 = phi float [ undef, %postload324 ], [ %masked_load288, %preload329 ]
  br i1 %extract176, label %preload335, label %postload336

preload335:                                       ; preds = %postload330
  %masked_load289 = load float addrspace(1)* %97, align 4
  br label %postload336

postload336:                                      ; preds = %preload335, %postload330
  %phi337 = phi float [ undef, %postload330 ], [ %masked_load289, %preload335 ]
  br i1 %extract177, label %preload341, label %postload342

preload341:                                       ; preds = %postload336
  %masked_load290 = load float addrspace(1)* %100, align 4
  br label %postload342

postload342:                                      ; preds = %preload341, %postload336
  %phi343 = phi float [ undef, %postload336 ], [ %masked_load290, %preload341 ]
  br i1 %extract178, label %preload347, label %postload348

preload347:                                       ; preds = %postload342
  %masked_load291 = load float addrspace(1)* %103, align 4
  br label %postload348

postload348:                                      ; preds = %preload347, %postload342
  %phi349 = phi float [ undef, %postload342 ], [ %masked_load291, %preload347 ]
  br i1 %extract179, label %preload353, label %postload354

preload353:                                       ; preds = %postload348
  %masked_load292 = load float addrspace(1)* %106, align 4
  br label %postload354

postload354:                                      ; preds = %preload353, %postload348
  %phi355 = phi float [ undef, %postload348 ], [ %masked_load292, %preload353 ]
  br i1 %extract180, label %preload359, label %postload360

preload359:                                       ; preds = %postload354
  %masked_load293 = load float addrspace(1)* %109, align 4
  br label %postload360

postload360:                                      ; preds = %preload359, %postload354
  %phi361 = phi float [ undef, %postload354 ], [ %masked_load293, %preload359 ]
  br i1 %extract181, label %preload365, label %postload366

preload365:                                       ; preds = %postload360
  %masked_load294 = load float addrspace(1)* %112, align 4
  br label %postload366

postload366:                                      ; preds = %preload365, %postload360
  %phi367 = phi float [ undef, %postload360 ], [ %masked_load294, %preload365 ]
  br i1 %extract182, label %preload371, label %postload372

preload371:                                       ; preds = %postload366
  %masked_load295 = load float addrspace(1)* %115, align 4
  br label %postload372

postload372:                                      ; preds = %preload371, %postload366
  %phi373 = phi float [ undef, %postload366 ], [ %masked_load295, %preload371 ]
  br i1 %extract183, label %preload377, label %postload378

preload377:                                       ; preds = %postload372
  %masked_load296 = load float addrspace(1)* %118, align 4
  br label %postload378

postload378:                                      ; preds = %preload377, %postload372
  %phi379 = phi float [ undef, %postload372 ], [ %masked_load296, %preload377 ]
  br i1 %extract184, label %preload383, label %postload384

preload383:                                       ; preds = %postload378
  %masked_load297 = load float addrspace(1)* %121, align 4
  br label %postload384

postload384:                                      ; preds = %preload383, %postload378
  %phi385 = phi float [ undef, %postload378 ], [ %masked_load297, %preload383 ]
  br i1 %extract185, label %preload389, label %postload390

preload389:                                       ; preds = %postload384
  %masked_load298 = load float addrspace(1)* %124, align 4
  br label %postload390

postload390:                                      ; preds = %preload389, %postload384
  %phi391 = phi float [ undef, %postload384 ], [ %masked_load298, %preload389 ]
  %temp.vect217 = insertelement <16 x float> undef, float %phi301, i32 0
  %temp.vect218 = insertelement <16 x float> %temp.vect217, float %phi307, i32 1
  %temp.vect219 = insertelement <16 x float> %temp.vect218, float %phi313, i32 2
  %temp.vect220 = insertelement <16 x float> %temp.vect219, float %phi319, i32 3
  %temp.vect221 = insertelement <16 x float> %temp.vect220, float %phi325, i32 4
  %temp.vect222 = insertelement <16 x float> %temp.vect221, float %phi331, i32 5
  %temp.vect223 = insertelement <16 x float> %temp.vect222, float %phi337, i32 6
  %temp.vect224 = insertelement <16 x float> %temp.vect223, float %phi343, i32 7
  %temp.vect225 = insertelement <16 x float> %temp.vect224, float %phi349, i32 8
  %temp.vect226 = insertelement <16 x float> %temp.vect225, float %phi355, i32 9
  %temp.vect227 = insertelement <16 x float> %temp.vect226, float %phi361, i32 10
  %temp.vect228 = insertelement <16 x float> %temp.vect227, float %phi367, i32 11
  %temp.vect229 = insertelement <16 x float> %temp.vect228, float %phi373, i32 12
  %temp.vect230 = insertelement <16 x float> %temp.vect229, float %phi379, i32 13
  %temp.vect231 = insertelement <16 x float> %temp.vect230, float %phi385, i32 14
  %temp.vect232 = insertelement <16 x float> %temp.vect231, float %phi391, i32 15
  %128 = fmul <16 x float> %temp.vect216, %temp.vect232
  %129 = fadd <16 x float> %vectorPHI148, %128
  %out_sel233 = select <16 x i1> %vectorPHI147, <16 x float> %129, <16 x float> %vectorPHI146
  %tmp52236 = add <16 x i32> %tmp51101, %vector235
  %130 = icmp ugt <16 x i32> %tmp52236, %24
  %indvar.next21 = add i32 %indvar20, 1
  %notCond15237 = xor <16 x i1> %130, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr16238 = and <16 x i1> %vectorPHI147, %130
  %ever_left_loop17239 = or <16 x i1> %vectorPHI145, %who_left_tr16238
  %loop_mask19240 = or <16 x i1> %vectorPHI144, %who_left_tr16238
  %curr_loop_mask21241 = or <16 x i1> %loop_mask19240, %who_left_tr16238
  %ipred.i13 = bitcast <16 x i1> %curr_loop_mask21241 to i16
  %val.i14 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i13, i16 %ipred.i13) nounwind
  %tmp.i15 = and i32 %val.i14, 1
  %res.i16 = icmp eq i32 %tmp.i15, 0
  %local_edge25242 = and <16 x i1> %vectorPHI147, %notCond15237
  br i1 %res.i16, label %31, label %._crit_edge

._crit_edge:                                      ; preds = %postload390
  %131 = icmp ugt <16 x i32> %tmp81142, %21
  %indvar.next24 = add i32 %indvar23, 1
  %notCond28243 = xor <16 x i1> %131, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr29244 = and <16 x i1> %ever_left_loop17239, %131
  %ever_left_loop30245 = or <16 x i1> %vectorPHI133, %who_left_tr29244
  %loop_mask32246 = or <16 x i1> %vectorPHI134, %who_left_tr29244
  %curr_loop_mask34247 = or <16 x i1> %loop_mask32246, %who_left_tr29244
  %ipred.i17 = bitcast <16 x i1> %curr_loop_mask34247 to i16
  %val.i18 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i17, i16 %ipred.i17) nounwind
  %tmp.i19 = and i32 %val.i18, 1
  %res.i20 = icmp eq i32 %tmp.i19, 0
  %local_edge37248 = and <16 x i1> %ever_left_loop17239, %notCond28243
  br i1 %res.i20, label %bb.nph, label %phi-split-bb.loopexit

phi-split-bb.loopexit:                            ; preds = %._crit_edge
  %phitmp407 = select <16 x i1> %ever_left_loop30245, <16 x float> %out_sel233, <16 x float> zeroinitializer
  br label %bb.nph12.split.us

._crit_edge13:                                    ; preds = %bb.nph12.split.us, %29, %SyncBB408
  %vectorPHI250 = phi <16 x float> [ undef, %SyncBB408 ], [ %vectorPHI, %29 ], [ %vectorPHI, %bb.nph12.split.us ]
  %merge55251 = select <16 x i1> %25, <16 x float> zeroinitializer, <16 x float> %vectorPHI250
  %132 = fadd <16 x float> %merge55251, <float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01>
  %extract252.lhs = extractelement <16 x i64> %13, i32 0
  %extract252 = and i64 %extract252.lhs, 4294967295
  %133 = getelementptr inbounds float addrspace(1)* %output, i64 %extract252
  %ptrTypeCast = bitcast float addrspace(1)* %133 to <16 x float> addrspace(1)*
  store <16 x float> %132, <16 x float> addrspace(1)* %ptrTypeCast, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge13
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB408

SyncBB:                                           ; preds = %._crit_edge13
  ret void
}

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @simpleConvolution(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %tmp6.i = bitcast double %10 to i64
  %tmp5.i = bitcast i64 %tmp6.i to <2 x i32>
  %tmp3.i = bitcast double %13 to i64
  %tmp2.i = bitcast i64 %tmp3.i to <2 x i32>
  %23 = extractelement <2 x i32> %tmp5.i, i32 0
  %24 = extractelement <2 x i32> %tmp5.i, i32 1
  %25 = icmp eq i32 %23, 0
  %26 = select i1 %25, i32 1, i32 %23
  %27 = extractelement <2 x i32> %tmp2.i, i32 0
  %28 = extractelement <2 x i32> %tmp2.i, i32 1
  %29 = add i32 %27, -1
  %30 = lshr i32 %29, 1
  %31 = add i32 %28, -1
  %32 = lshr i32 %31, 1
  %33 = add i32 %23, -1
  %34 = add i32 %24, -1
  %tmp32.i = icmp ugt i32 %23, 1
  %umax33.i = select i1 %tmp32.i, i32 %23, i32 1
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %35 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %38 = load i64* %37, align 8
  %39 = add i64 %36, %38
  %40 = trunc i64 %39 to i32
  %41 = urem i32 %40, %26
  %42 = udiv i32 %40, %26
  %43 = sub i32 %41, %30
  %44 = icmp ult i32 %41, %30
  %..i = select i1 %44, i32 0, i32 %43
  %45 = add i32 %41, %30
  %46 = icmp ult i32 %45, %23
  %47 = select i1 %46, i32 %45, i32 %33
  %48 = add i32 %42, %32
  %49 = icmp ult i32 %48, %24
  %50 = select i1 %49, i32 %48, i32 %34
  %51 = icmp ugt i32 %..i, %47
  br i1 %51, label %._crit_edge13.i, label %bb.nph12.i

bb.nph12.i:                                       ; preds = %SyncBB.i
  %52 = icmp ult i32 %42, %32
  %53 = sub i32 %42, %32
  %.1.i = select i1 %52, i32 0, i32 %53
  %54 = icmp ugt i32 %.1.i, %50
  br i1 %54, label %bb.nph12.split.us.i, label %bb.nph12.bb.nph12.split_crit_edge.i

bb.nph12.bb.nph12.split_crit_edge.i:              ; preds = %bb.nph12.i
  %tmp34.i = udiv i32 %40, %umax33.i
  %tmp35.i = icmp ugt i32 %32, %tmp34.i
  %umax36.i = select i1 %tmp35.i, i32 %32, i32 %tmp34.i
  %tmp50.i = add i32 %umax36.i, 1
  %tmp51.i = sub i32 %tmp50.i, %32
  %tmp56.i = icmp ugt i32 %41, %30
  %umax57.i = select i1 %tmp56.i, i32 %41, i32 %30
  %tmp66.i = sub i32 %umax36.i, %32
  %tmp67.i = mul i32 %23, %tmp66.i
  %tmp68.i = add i32 %umax57.i, %tmp67.i
  %tmp69.i = sub i32 %tmp68.i, %30
  %tmp73.i = sub i32 %umax36.i, %tmp34.i
  %tmp74.i = mul i32 %27, %tmp73.i
  %tmp75.i = add i32 %umax57.i, %tmp74.i
  %tmp76.i = sub i32 %tmp75.i, %41
  %tmp79.i = add i32 %umax57.i, 1
  %tmp80.i = sub i32 %tmp79.i, %30
  br label %bb.nph.i

bb.nph12.split.us.i:                              ; preds = %bb.nph12.i
  %tmp16.i = icmp ugt i32 %41, %30
  %umax.i = select i1 %tmp16.i, i32 %41, i32 %30
  %tmp17.i = add i32 %umax.i, 1
  %tmp18.i = sub i32 %tmp17.i, %30
  br label %55

; <label>:55                                      ; preds = %55, %bb.nph12.split.us.i
  %indvar.i = phi i32 [ %indvar.next.i, %55 ], [ 0, %bb.nph12.split.us.i ]
  %tmp19.i = add i32 %tmp18.i, %indvar.i
  %56 = icmp ugt i32 %tmp19.i, %47
  %indvar.next.i = add i32 %indvar.i, 1
  br i1 %56, label %._crit_edge13.i, label %55

bb.nph.i:                                         ; preds = %._crit_edge.i, %bb.nph12.bb.nph12.split_crit_edge.i
  %indvar23.i = phi i32 [ 0, %bb.nph12.bb.nph12.split_crit_edge.i ], [ %indvar.next24.i, %._crit_edge.i ]
  %sumFX.110.i = phi float [ 0.000000e+00, %bb.nph12.bb.nph12.split_crit_edge.i ], [ %66, %._crit_edge.i ]
  %tmp70.i = add i32 %tmp69.i, %indvar23.i
  %tmp77.i = add i32 %tmp76.i, %indvar23.i
  %tmp81.i = add i32 %tmp80.i, %indvar23.i
  br label %57

; <label>:57                                      ; preds = %57, %bb.nph.i
  %indvar20.i = phi i32 [ 0, %bb.nph.i ], [ %indvar.next21.i, %57 ]
  %sumFX.08.i = phi float [ %sumFX.110.i, %bb.nph.i ], [ %66, %57 ]
  %tmp53.i = mul i32 %23, %indvar20.i
  %tmp49.i = add i32 %tmp70.i, %tmp53.i
  %tmp72.i = mul i32 %27, %indvar20.i
  %tmp42.i = add i32 %tmp77.i, %tmp72.i
  %58 = zext i32 %tmp49.i to i64
  %59 = getelementptr inbounds i32 addrspace(1)* %4, i64 %58
  %60 = load i32 addrspace(1)* %59, align 4
  %61 = uitofp i32 %60 to float
  %62 = zext i32 %tmp42.i to i64
  %63 = getelementptr inbounds float addrspace(1)* %7, i64 %62
  %64 = load float addrspace(1)* %63, align 4
  %65 = fmul float %61, %64
  %66 = fadd float %sumFX.08.i, %65
  %tmp52.i = add i32 %tmp51.i, %indvar20.i
  %67 = icmp ugt i32 %tmp52.i, %50
  %indvar.next21.i = add i32 %indvar20.i, 1
  br i1 %67, label %._crit_edge.i, label %57

._crit_edge.i:                                    ; preds = %57
  %68 = icmp ugt i32 %tmp81.i, %47
  %indvar.next24.i = add i32 %indvar23.i, 1
  br i1 %68, label %._crit_edge13.i, label %bb.nph.i

._crit_edge13.i:                                  ; preds = %._crit_edge.i, %55, %SyncBB.i
  %sumFX.1.lcssa.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %66, %._crit_edge.i ], [ 0.000000e+00, %55 ]
  %69 = fadd float %sumFX.1.lcssa.i, 5.000000e-01
  %70 = and i64 %39, 4294967295
  %71 = getelementptr inbounds float addrspace(1)* %1, i64 %70
  store float %69, float addrspace(1)* %71, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %__simpleConvolution_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge13.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__simpleConvolution_separated_args.exit:          ; preds = %._crit_edge13.i
  ret void
}

define void @__Vectorized_.simpleConvolution(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %tmp6.i = bitcast double %10 to i64
  %tmp5.i = bitcast i64 %tmp6.i to <2 x i32>
  %scalar.i = extractelement <2 x i32> %tmp5.i, i32 0
  %temp79.i = insertelement <16 x i32> undef, i32 %scalar.i, i32 0
  %vector80.i = shufflevector <16 x i32> %temp79.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %scalar1.i = extractelement <2 x i32> %tmp5.i, i32 1
  %temp85.i = insertelement <16 x i32> undef, i32 %scalar1.i, i32 0
  %vector86.i = shufflevector <16 x i32> %temp85.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp3.i = bitcast double %13 to i64
  %tmp2.i = bitcast i64 %tmp3.i to <2 x i32>
  %scalar2.i = extractelement <2 x i32> %tmp2.i, i32 0
  %scalar3.i = extractelement <2 x i32> %tmp2.i, i32 1
  %23 = icmp eq i32 %scalar.i, 0
  %24 = select i1 %23, i32 1, i32 %scalar.i
  %temp.i = insertelement <16 x i32> undef, i32 %24, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %25 = add i32 %scalar2.i, -1
  %26 = lshr i32 %25, 1
  %temp76.i = insertelement <16 x i32> undef, i32 %26, i32 0
  %vector77.i = shufflevector <16 x i32> %temp76.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %27 = add i32 %scalar3.i, -1
  %28 = lshr i32 %27, 1
  %temp83.i = insertelement <16 x i32> undef, i32 %28, i32 0
  %vector84.i = shufflevector <16 x i32> %temp83.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %29 = add i32 %scalar.i, -1
  %temp81.i = insertelement <16 x i32> undef, i32 %29, i32 0
  %vector82.i = shufflevector <16 x i32> %temp81.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %30 = add i32 %scalar1.i, -1
  %temp87.i = insertelement <16 x i32> undef, i32 %30, i32 0
  %vector88.i = shufflevector <16 x i32> %temp87.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp108.i = insertelement <16 x i32> undef, i32 %scalar2.i, i32 0
  %vector109.i = shufflevector <16 x i32> %temp108.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp32.i = icmp ugt i32 %scalar.i, 1
  %umax33.i = select i1 %tmp32.i, i32 %scalar.i, i32 1
  %temp96.i = insertelement <16 x i32> undef, i32 %umax33.i, i32 0
  %vector97.i = shufflevector <16 x i32> %temp96.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB408.i

SyncBB408.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %31 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = add i64 %32, %34
  %broadcast1.i = insertelement <16 x i64> undef, i64 %35, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %36 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %37 = trunc <16 x i64> %36 to <16 x i32>
  %38 = urem <16 x i32> %37, %vector.i
  %39 = udiv <16 x i32> %37, %vector.i
  %40 = sub <16 x i32> %38, %vector77.i
  %41 = icmp ult <16 x i32> %38, %vector77.i
  %.78.i = select <16 x i1> %41, <16 x i32> zeroinitializer, <16 x i32> %40
  %42 = add <16 x i32> %38, %vector77.i
  %43 = icmp ult <16 x i32> %42, %vector80.i
  %44 = select <16 x i1> %43, <16 x i32> %42, <16 x i32> %vector82.i
  %45 = add <16 x i32> %39, %vector84.i
  %46 = icmp ult <16 x i32> %45, %vector86.i
  %47 = select <16 x i1> %46, <16 x i32> %45, <16 x i32> %vector88.i
  %48 = icmp ugt <16 x i32> %.78.i, %44
  %Mneg89.i = xor <16 x i1> %48, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %ipred.i.i = bitcast <16 x i1> %Mneg89.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  br i1 %res.i.i, label %bb.nph12.i, label %._crit_edge13.i

bb.nph12.i:                                       ; preds = %SyncBB408.i
  %49 = icmp ult <16 x i32> %39, %vector84.i
  %50 = sub <16 x i32> %39, %vector84.i
  %.192.i = select <16 x i1> %49, <16 x i32> zeroinitializer, <16 x i32> %50
  %51 = icmp ugt <16 x i32> %.192.i, %47
  %Mneg1193.i = xor <16 x i1> %51, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94.i = and <16 x i1> %Mneg1193.i, %Mneg89.i
  %bb.nph12_to_bb.nph12.split.us95.i = and <16 x i1> %51, %Mneg89.i
  %tmp3498.i = udiv <16 x i32> %37, %vector97.i
  %tmp35.i = icmp ugt <16 x i32> %vector84.i, %tmp3498.i
  %umax3699.i = select <16 x i1> %tmp35.i, <16 x i32> %vector84.i, <16 x i32> %tmp3498.i
  %tmp50100.i = add <16 x i32> %umax3699.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp51101.i = sub <16 x i32> %tmp50100.i, %vector84.i
  %tmp56.i = icmp ugt <16 x i32> %38, %vector77.i
  %umax57102.i = select <16 x i1> %tmp56.i, <16 x i32> %38, <16 x i32> %vector77.i
  %tmp66103.i = sub <16 x i32> %umax3699.i, %vector84.i
  %tmp67104.i = mul <16 x i32> %vector80.i, %tmp66103.i
  %tmp68105.i = add <16 x i32> %umax57102.i, %tmp67104.i
  %tmp69106.i = sub <16 x i32> %tmp68105.i, %vector77.i
  %tmp73107.i = sub <16 x i32> %umax3699.i, %tmp3498.i
  %tmp74110.i = mul <16 x i32> %vector109.i, %tmp73107.i
  %tmp75111.i = add <16 x i32> %umax57102.i, %tmp74110.i
  %tmp76112.i = sub <16 x i32> %tmp75111.i, %38
  %tmp79113.i = add <16 x i32> %umax57102.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp80114.i = sub <16 x i32> %tmp79113.i, %vector77.i
  %ipred.i1.i = bitcast <16 x i1> %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %tmp.i3.i = and i32 %val.i2.i, 1
  %res.i4.i = icmp eq i32 %tmp.i3.i, 0
  br i1 %res.i4.i, label %bb.nph.preheader.i, label %bb.nph12.split.us.i

bb.nph.preheader.i:                               ; preds = %bb.nph12.i
  %negIncomingLoopMask44115.i = or <16 x i1> %51, %48
  br label %bb.nph.i

bb.nph12.split.us.i:                              ; preds = %phi-split-bb.loopexit.i, %bb.nph12.i
  %vectorPHI.i = phi <16 x float> [ %phitmp407.i, %phi-split-bb.loopexit.i ], [ zeroinitializer, %bb.nph12.i ]
  %tmp16.i = icmp ugt <16 x i32> %38, %vector77.i
  %umax117.i = select <16 x i1> %tmp16.i, <16 x i32> %38, <16 x i32> %vector77.i
  %tmp17118.i = add <16 x i32> %umax117.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp18119.i = sub <16 x i32> %tmp17118.i, %vector77.i
  %ipred.i5.i = bitcast <16 x i1> %bb.nph12_to_bb.nph12.split.us95.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %tmp.i7.i = and i32 %val.i6.i, 1
  %res.i8.i = icmp eq i32 %tmp.i7.i, 0
  br i1 %res.i8.i, label %.preheader.i, label %._crit_edge13.i

.preheader.i:                                     ; preds = %bb.nph12.split.us.i
  %.not.i = xor <16 x i1> %51, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %negIncomingLoopMask120.i = or <16 x i1> %48, %.not.i
  br label %52

; <label>:52                                      ; preds = %52, %.preheader.i
  %vectorPHI121.i = phi <16 x i1> [ %loop_mask12130.i, %52 ], [ %negIncomingLoopMask120.i, %.preheader.i ]
  %vectorPHI123.i = phi <16 x i1> [ %local_edge132.i, %52 ], [ %bb.nph12_to_bb.nph12.split.us95.i, %.preheader.i ]
  %indvar.i = phi i32 [ %indvar.next.i, %52 ], [ 0, %.preheader.i ]
  %temp124.i = insertelement <16 x i32> undef, i32 %indvar.i, i32 0
  %vector125.i = shufflevector <16 x i32> %temp124.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp19126.i = add <16 x i32> %tmp18119.i, %vector125.i
  %53 = icmp ugt <16 x i32> %tmp19126.i, %44
  %indvar.next.i = add i32 %indvar.i, 1
  %notCond127.i = xor <16 x i1> %53, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr128.i = and <16 x i1> %vectorPHI123.i, %53
  %loop_mask12130.i = or <16 x i1> %vectorPHI121.i, %who_left_tr128.i
  %curr_loop_mask131.i = or <16 x i1> %loop_mask12130.i, %who_left_tr128.i
  %ipred.i9.i = bitcast <16 x i1> %curr_loop_mask131.i to i16
  %val.i10.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9.i, i16 %ipred.i9.i) nounwind
  %tmp.i11.i = and i32 %val.i10.i, 1
  %res.i12.i = icmp eq i32 %tmp.i11.i, 0
  %local_edge132.i = and <16 x i1> %vectorPHI123.i, %notCond127.i
  br i1 %res.i12.i, label %52, label %._crit_edge13.i

bb.nph.i:                                         ; preds = %._crit_edge.i, %bb.nph.preheader.i
  %vectorPHI133.i = phi <16 x i1> [ %ever_left_loop30245.i, %._crit_edge.i ], [ zeroinitializer, %bb.nph.preheader.i ]
  %vectorPHI134.i = phi <16 x i1> [ %loop_mask32246.i, %._crit_edge.i ], [ %negIncomingLoopMask44115.i, %bb.nph.preheader.i ]
  %vectorPHI135.i = phi <16 x float> [ %out_sel233.i, %._crit_edge.i ], [ undef, %bb.nph.preheader.i ]
  %vectorPHI136.i = phi <16 x i1> [ %local_edge37248.i, %._crit_edge.i ], [ %bb.nph12_to_bb.nph12.bb.nph12.split_crit_edge94.i, %bb.nph.preheader.i ]
  %indvar23.i = phi i32 [ %indvar.next24.i, %._crit_edge.i ], [ 0, %bb.nph.preheader.i ]
  %vectorPHI137.i = phi <16 x float> [ %out_sel233.i, %._crit_edge.i ], [ zeroinitializer, %bb.nph.preheader.i ]
  %temp138.i = insertelement <16 x i32> undef, i32 %indvar23.i, i32 0
  %vector139.i = shufflevector <16 x i32> %temp138.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp70140.i = add <16 x i32> %tmp69106.i, %vector139.i
  %tmp77141.i = add <16 x i32> %tmp76112.i, %vector139.i
  %tmp81142.i = add <16 x i32> %tmp80114.i, %vector139.i
  %negIncomingLoopMask49143.i = xor <16 x i1> %vectorPHI136.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %54

; <label>:54                                      ; preds = %postload390.i, %bb.nph.i
  %vectorPHI144.i = phi <16 x i1> [ %negIncomingLoopMask49143.i, %bb.nph.i ], [ %loop_mask19240.i, %postload390.i ]
  %vectorPHI145.i = phi <16 x i1> [ zeroinitializer, %bb.nph.i ], [ %ever_left_loop17239.i, %postload390.i ]
  %vectorPHI146.i = phi <16 x float> [ %vectorPHI135.i, %bb.nph.i ], [ %out_sel233.i, %postload390.i ]
  %vectorPHI147.i = phi <16 x i1> [ %vectorPHI136.i, %bb.nph.i ], [ %local_edge25242.i, %postload390.i ]
  %indvar20.i = phi i32 [ 0, %bb.nph.i ], [ %indvar.next21.i, %postload390.i ]
  %vectorPHI148.i = phi <16 x float> [ %vectorPHI137.i, %bb.nph.i ], [ %152, %postload390.i ]
  %temp234.i = insertelement <16 x i32> undef, i32 %indvar20.i, i32 0
  %vector235.i = shufflevector <16 x i32> %temp234.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %extract170.i = extractelement <16 x i1> %vectorPHI147.i, i32 0
  %extract171.i = extractelement <16 x i1> %vectorPHI147.i, i32 1
  %extract172.i = extractelement <16 x i1> %vectorPHI147.i, i32 2
  %extract173.i = extractelement <16 x i1> %vectorPHI147.i, i32 3
  %extract174.i = extractelement <16 x i1> %vectorPHI147.i, i32 4
  %extract175.i = extractelement <16 x i1> %vectorPHI147.i, i32 5
  %extract176.i = extractelement <16 x i1> %vectorPHI147.i, i32 6
  %extract177.i = extractelement <16 x i1> %vectorPHI147.i, i32 7
  %extract178.i = extractelement <16 x i1> %vectorPHI147.i, i32 8
  %extract179.i = extractelement <16 x i1> %vectorPHI147.i, i32 9
  %extract180.i = extractelement <16 x i1> %vectorPHI147.i, i32 10
  %extract181.i = extractelement <16 x i1> %vectorPHI147.i, i32 11
  %extract182.i = extractelement <16 x i1> %vectorPHI147.i, i32 12
  %extract183.i = extractelement <16 x i1> %vectorPHI147.i, i32 13
  %extract184.i = extractelement <16 x i1> %vectorPHI147.i, i32 14
  %extract185.i = extractelement <16 x i1> %vectorPHI147.i, i32 15
  %tmp53.i = mul i32 %scalar.i, %indvar20.i
  %temp149.i = insertelement <16 x i32> undef, i32 %tmp53.i, i32 0
  %vector150.i = shufflevector <16 x i32> %temp149.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp49151.i = add <16 x i32> %tmp70140.i, %vector150.i
  %tmp72.i = mul i32 %scalar2.i, %indvar20.i
  %temp152.i = insertelement <16 x i32> undef, i32 %tmp72.i, i32 0
  %vector153.i = shufflevector <16 x i32> %temp152.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp42154.i = add <16 x i32> %tmp77141.i, %vector153.i
  %55 = extractelement <16 x i32> %tmp49151.i, i32 1
  %56 = zext i32 %55 to i64
  %57 = getelementptr inbounds i32 addrspace(1)* %4, i64 %56
  %58 = extractelement <16 x i32> %tmp49151.i, i32 2
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds i32 addrspace(1)* %4, i64 %59
  %61 = extractelement <16 x i32> %tmp49151.i, i32 3
  %62 = zext i32 %61 to i64
  %63 = getelementptr inbounds i32 addrspace(1)* %4, i64 %62
  %64 = extractelement <16 x i32> %tmp49151.i, i32 4
  %65 = zext i32 %64 to i64
  %66 = getelementptr inbounds i32 addrspace(1)* %4, i64 %65
  %67 = extractelement <16 x i32> %tmp49151.i, i32 5
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds i32 addrspace(1)* %4, i64 %68
  %70 = extractelement <16 x i32> %tmp49151.i, i32 6
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds i32 addrspace(1)* %4, i64 %71
  %73 = extractelement <16 x i32> %tmp49151.i, i32 7
  %74 = zext i32 %73 to i64
  %75 = getelementptr inbounds i32 addrspace(1)* %4, i64 %74
  %76 = extractelement <16 x i32> %tmp49151.i, i32 8
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds i32 addrspace(1)* %4, i64 %77
  %79 = extractelement <16 x i32> %tmp49151.i, i32 9
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds i32 addrspace(1)* %4, i64 %80
  %82 = extractelement <16 x i32> %tmp49151.i, i32 10
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds i32 addrspace(1)* %4, i64 %83
  %85 = extractelement <16 x i32> %tmp49151.i, i32 11
  %86 = zext i32 %85 to i64
  %87 = getelementptr inbounds i32 addrspace(1)* %4, i64 %86
  %88 = extractelement <16 x i32> %tmp49151.i, i32 12
  %89 = zext i32 %88 to i64
  %90 = getelementptr inbounds i32 addrspace(1)* %4, i64 %89
  %91 = extractelement <16 x i32> %tmp49151.i, i32 13
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds i32 addrspace(1)* %4, i64 %92
  %94 = extractelement <16 x i32> %tmp49151.i, i32 14
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds i32 addrspace(1)* %4, i64 %95
  %97 = extractelement <16 x i32> %tmp49151.i, i32 15
  %98 = zext i32 %97 to i64
  %99 = getelementptr inbounds i32 addrspace(1)* %4, i64 %98
  br i1 %extract170.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %54
  %100 = extractelement <16 x i32> %tmp49151.i, i32 0
  %101 = zext i32 %100 to i64
  %102 = getelementptr inbounds i32 addrspace(1)* %4, i64 %101
  %masked_load.i = load i32 addrspace(1)* %102, align 4
  %phitmp.i = uitofp i32 %masked_load.i to float
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %54
  %phi.i = phi float [ 0.000000e+00, %54 ], [ %phitmp.i, %preload.i ]
  br i1 %extract171.i, label %preload302.i, label %postload303.i

preload302.i:                                     ; preds = %postload.i
  %masked_load268.i = load i32 addrspace(1)* %57, align 4
  %phitmp392.i = uitofp i32 %masked_load268.i to float
  br label %postload303.i

postload303.i:                                    ; preds = %preload302.i, %postload.i
  %phi304.i = phi float [ 0.000000e+00, %postload.i ], [ %phitmp392.i, %preload302.i ]
  br i1 %extract172.i, label %preload308.i, label %postload309.i

preload308.i:                                     ; preds = %postload303.i
  %masked_load269.i = load i32 addrspace(1)* %60, align 4
  %phitmp393.i = uitofp i32 %masked_load269.i to float
  br label %postload309.i

postload309.i:                                    ; preds = %preload308.i, %postload303.i
  %phi310.i = phi float [ 0.000000e+00, %postload303.i ], [ %phitmp393.i, %preload308.i ]
  br i1 %extract173.i, label %preload314.i, label %postload315.i

preload314.i:                                     ; preds = %postload309.i
  %masked_load270.i = load i32 addrspace(1)* %63, align 4
  %phitmp394.i = uitofp i32 %masked_load270.i to float
  br label %postload315.i

postload315.i:                                    ; preds = %preload314.i, %postload309.i
  %phi316.i = phi float [ 0.000000e+00, %postload309.i ], [ %phitmp394.i, %preload314.i ]
  br i1 %extract174.i, label %preload320.i, label %postload321.i

preload320.i:                                     ; preds = %postload315.i
  %masked_load271.i = load i32 addrspace(1)* %66, align 4
  %phitmp395.i = uitofp i32 %masked_load271.i to float
  br label %postload321.i

postload321.i:                                    ; preds = %preload320.i, %postload315.i
  %phi322.i = phi float [ 0.000000e+00, %postload315.i ], [ %phitmp395.i, %preload320.i ]
  br i1 %extract175.i, label %preload326.i, label %postload327.i

preload326.i:                                     ; preds = %postload321.i
  %masked_load272.i = load i32 addrspace(1)* %69, align 4
  %phitmp396.i = uitofp i32 %masked_load272.i to float
  br label %postload327.i

postload327.i:                                    ; preds = %preload326.i, %postload321.i
  %phi328.i = phi float [ 0.000000e+00, %postload321.i ], [ %phitmp396.i, %preload326.i ]
  br i1 %extract176.i, label %preload332.i, label %postload333.i

preload332.i:                                     ; preds = %postload327.i
  %masked_load273.i = load i32 addrspace(1)* %72, align 4
  %phitmp397.i = uitofp i32 %masked_load273.i to float
  br label %postload333.i

postload333.i:                                    ; preds = %preload332.i, %postload327.i
  %phi334.i = phi float [ 0.000000e+00, %postload327.i ], [ %phitmp397.i, %preload332.i ]
  br i1 %extract177.i, label %preload338.i, label %postload339.i

preload338.i:                                     ; preds = %postload333.i
  %masked_load274.i = load i32 addrspace(1)* %75, align 4
  %phitmp398.i = uitofp i32 %masked_load274.i to float
  br label %postload339.i

postload339.i:                                    ; preds = %preload338.i, %postload333.i
  %phi340.i = phi float [ 0.000000e+00, %postload333.i ], [ %phitmp398.i, %preload338.i ]
  br i1 %extract178.i, label %preload344.i, label %postload345.i

preload344.i:                                     ; preds = %postload339.i
  %masked_load275.i = load i32 addrspace(1)* %78, align 4
  %phitmp399.i = uitofp i32 %masked_load275.i to float
  br label %postload345.i

postload345.i:                                    ; preds = %preload344.i, %postload339.i
  %phi346.i = phi float [ 0.000000e+00, %postload339.i ], [ %phitmp399.i, %preload344.i ]
  br i1 %extract179.i, label %preload350.i, label %postload351.i

preload350.i:                                     ; preds = %postload345.i
  %masked_load276.i = load i32 addrspace(1)* %81, align 4
  %phitmp400.i = uitofp i32 %masked_load276.i to float
  br label %postload351.i

postload351.i:                                    ; preds = %preload350.i, %postload345.i
  %phi352.i = phi float [ 0.000000e+00, %postload345.i ], [ %phitmp400.i, %preload350.i ]
  br i1 %extract180.i, label %preload356.i, label %postload357.i

preload356.i:                                     ; preds = %postload351.i
  %masked_load277.i = load i32 addrspace(1)* %84, align 4
  %phitmp401.i = uitofp i32 %masked_load277.i to float
  br label %postload357.i

postload357.i:                                    ; preds = %preload356.i, %postload351.i
  %phi358.i = phi float [ 0.000000e+00, %postload351.i ], [ %phitmp401.i, %preload356.i ]
  br i1 %extract181.i, label %preload362.i, label %postload363.i

preload362.i:                                     ; preds = %postload357.i
  %masked_load278.i = load i32 addrspace(1)* %87, align 4
  %phitmp402.i = uitofp i32 %masked_load278.i to float
  br label %postload363.i

postload363.i:                                    ; preds = %preload362.i, %postload357.i
  %phi364.i = phi float [ 0.000000e+00, %postload357.i ], [ %phitmp402.i, %preload362.i ]
  br i1 %extract182.i, label %preload368.i, label %postload369.i

preload368.i:                                     ; preds = %postload363.i
  %masked_load279.i = load i32 addrspace(1)* %90, align 4
  %phitmp403.i = uitofp i32 %masked_load279.i to float
  br label %postload369.i

postload369.i:                                    ; preds = %preload368.i, %postload363.i
  %phi370.i = phi float [ 0.000000e+00, %postload363.i ], [ %phitmp403.i, %preload368.i ]
  br i1 %extract183.i, label %preload374.i, label %postload375.i

preload374.i:                                     ; preds = %postload369.i
  %masked_load280.i = load i32 addrspace(1)* %93, align 4
  %phitmp404.i = uitofp i32 %masked_load280.i to float
  br label %postload375.i

postload375.i:                                    ; preds = %preload374.i, %postload369.i
  %phi376.i = phi float [ 0.000000e+00, %postload369.i ], [ %phitmp404.i, %preload374.i ]
  br i1 %extract184.i, label %preload380.i, label %postload381.i

preload380.i:                                     ; preds = %postload375.i
  %masked_load281.i = load i32 addrspace(1)* %96, align 4
  %phitmp405.i = uitofp i32 %masked_load281.i to float
  br label %postload381.i

postload381.i:                                    ; preds = %preload380.i, %postload375.i
  %phi382.i = phi float [ 0.000000e+00, %postload375.i ], [ %phitmp405.i, %preload380.i ]
  br i1 %extract185.i, label %preload386.i, label %postload387.i

preload386.i:                                     ; preds = %postload381.i
  %masked_load282.i = load i32 addrspace(1)* %99, align 4
  %phitmp406.i = uitofp i32 %masked_load282.i to float
  br label %postload387.i

postload387.i:                                    ; preds = %preload386.i, %postload381.i
  %phi388.i = phi float [ 0.000000e+00, %postload381.i ], [ %phitmp406.i, %preload386.i ]
  %temp.vect.i = insertelement <16 x float> undef, float %phi.i, i32 0
  %temp.vect202.i = insertelement <16 x float> %temp.vect.i, float %phi304.i, i32 1
  %temp.vect203.i = insertelement <16 x float> %temp.vect202.i, float %phi310.i, i32 2
  %temp.vect204.i = insertelement <16 x float> %temp.vect203.i, float %phi316.i, i32 3
  %temp.vect205.i = insertelement <16 x float> %temp.vect204.i, float %phi322.i, i32 4
  %temp.vect206.i = insertelement <16 x float> %temp.vect205.i, float %phi328.i, i32 5
  %temp.vect207.i = insertelement <16 x float> %temp.vect206.i, float %phi334.i, i32 6
  %temp.vect208.i = insertelement <16 x float> %temp.vect207.i, float %phi340.i, i32 7
  %temp.vect209.i = insertelement <16 x float> %temp.vect208.i, float %phi346.i, i32 8
  %temp.vect210.i = insertelement <16 x float> %temp.vect209.i, float %phi352.i, i32 9
  %temp.vect211.i = insertelement <16 x float> %temp.vect210.i, float %phi358.i, i32 10
  %temp.vect212.i = insertelement <16 x float> %temp.vect211.i, float %phi364.i, i32 11
  %temp.vect213.i = insertelement <16 x float> %temp.vect212.i, float %phi370.i, i32 12
  %temp.vect214.i = insertelement <16 x float> %temp.vect213.i, float %phi376.i, i32 13
  %temp.vect215.i = insertelement <16 x float> %temp.vect214.i, float %phi382.i, i32 14
  %temp.vect216.i = insertelement <16 x float> %temp.vect215.i, float %phi388.i, i32 15
  %103 = extractelement <16 x i32> %tmp42154.i, i32 1
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds float addrspace(1)* %7, i64 %104
  %106 = extractelement <16 x i32> %tmp42154.i, i32 2
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds float addrspace(1)* %7, i64 %107
  %109 = extractelement <16 x i32> %tmp42154.i, i32 3
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds float addrspace(1)* %7, i64 %110
  %112 = extractelement <16 x i32> %tmp42154.i, i32 4
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds float addrspace(1)* %7, i64 %113
  %115 = extractelement <16 x i32> %tmp42154.i, i32 5
  %116 = zext i32 %115 to i64
  %117 = getelementptr inbounds float addrspace(1)* %7, i64 %116
  %118 = extractelement <16 x i32> %tmp42154.i, i32 6
  %119 = zext i32 %118 to i64
  %120 = getelementptr inbounds float addrspace(1)* %7, i64 %119
  %121 = extractelement <16 x i32> %tmp42154.i, i32 7
  %122 = zext i32 %121 to i64
  %123 = getelementptr inbounds float addrspace(1)* %7, i64 %122
  %124 = extractelement <16 x i32> %tmp42154.i, i32 8
  %125 = zext i32 %124 to i64
  %126 = getelementptr inbounds float addrspace(1)* %7, i64 %125
  %127 = extractelement <16 x i32> %tmp42154.i, i32 9
  %128 = zext i32 %127 to i64
  %129 = getelementptr inbounds float addrspace(1)* %7, i64 %128
  %130 = extractelement <16 x i32> %tmp42154.i, i32 10
  %131 = zext i32 %130 to i64
  %132 = getelementptr inbounds float addrspace(1)* %7, i64 %131
  %133 = extractelement <16 x i32> %tmp42154.i, i32 11
  %134 = zext i32 %133 to i64
  %135 = getelementptr inbounds float addrspace(1)* %7, i64 %134
  %136 = extractelement <16 x i32> %tmp42154.i, i32 12
  %137 = zext i32 %136 to i64
  %138 = getelementptr inbounds float addrspace(1)* %7, i64 %137
  %139 = extractelement <16 x i32> %tmp42154.i, i32 13
  %140 = zext i32 %139 to i64
  %141 = getelementptr inbounds float addrspace(1)* %7, i64 %140
  %142 = extractelement <16 x i32> %tmp42154.i, i32 14
  %143 = zext i32 %142 to i64
  %144 = getelementptr inbounds float addrspace(1)* %7, i64 %143
  %145 = extractelement <16 x i32> %tmp42154.i, i32 15
  %146 = zext i32 %145 to i64
  %147 = getelementptr inbounds float addrspace(1)* %7, i64 %146
  br i1 %extract170.i, label %preload299.i, label %postload300.i

preload299.i:                                     ; preds = %postload387.i
  %148 = extractelement <16 x i32> %tmp42154.i, i32 0
  %149 = zext i32 %148 to i64
  %150 = getelementptr inbounds float addrspace(1)* %7, i64 %149
  %masked_load283.i = load float addrspace(1)* %150, align 4
  br label %postload300.i

postload300.i:                                    ; preds = %preload299.i, %postload387.i
  %phi301.i = phi float [ undef, %postload387.i ], [ %masked_load283.i, %preload299.i ]
  br i1 %extract171.i, label %preload305.i, label %postload306.i

preload305.i:                                     ; preds = %postload300.i
  %masked_load284.i = load float addrspace(1)* %105, align 4
  br label %postload306.i

postload306.i:                                    ; preds = %preload305.i, %postload300.i
  %phi307.i = phi float [ undef, %postload300.i ], [ %masked_load284.i, %preload305.i ]
  br i1 %extract172.i, label %preload311.i, label %postload312.i

preload311.i:                                     ; preds = %postload306.i
  %masked_load285.i = load float addrspace(1)* %108, align 4
  br label %postload312.i

postload312.i:                                    ; preds = %preload311.i, %postload306.i
  %phi313.i = phi float [ undef, %postload306.i ], [ %masked_load285.i, %preload311.i ]
  br i1 %extract173.i, label %preload317.i, label %postload318.i

preload317.i:                                     ; preds = %postload312.i
  %masked_load286.i = load float addrspace(1)* %111, align 4
  br label %postload318.i

postload318.i:                                    ; preds = %preload317.i, %postload312.i
  %phi319.i = phi float [ undef, %postload312.i ], [ %masked_load286.i, %preload317.i ]
  br i1 %extract174.i, label %preload323.i, label %postload324.i

preload323.i:                                     ; preds = %postload318.i
  %masked_load287.i = load float addrspace(1)* %114, align 4
  br label %postload324.i

postload324.i:                                    ; preds = %preload323.i, %postload318.i
  %phi325.i = phi float [ undef, %postload318.i ], [ %masked_load287.i, %preload323.i ]
  br i1 %extract175.i, label %preload329.i, label %postload330.i

preload329.i:                                     ; preds = %postload324.i
  %masked_load288.i = load float addrspace(1)* %117, align 4
  br label %postload330.i

postload330.i:                                    ; preds = %preload329.i, %postload324.i
  %phi331.i = phi float [ undef, %postload324.i ], [ %masked_load288.i, %preload329.i ]
  br i1 %extract176.i, label %preload335.i, label %postload336.i

preload335.i:                                     ; preds = %postload330.i
  %masked_load289.i = load float addrspace(1)* %120, align 4
  br label %postload336.i

postload336.i:                                    ; preds = %preload335.i, %postload330.i
  %phi337.i = phi float [ undef, %postload330.i ], [ %masked_load289.i, %preload335.i ]
  br i1 %extract177.i, label %preload341.i, label %postload342.i

preload341.i:                                     ; preds = %postload336.i
  %masked_load290.i = load float addrspace(1)* %123, align 4
  br label %postload342.i

postload342.i:                                    ; preds = %preload341.i, %postload336.i
  %phi343.i = phi float [ undef, %postload336.i ], [ %masked_load290.i, %preload341.i ]
  br i1 %extract178.i, label %preload347.i, label %postload348.i

preload347.i:                                     ; preds = %postload342.i
  %masked_load291.i = load float addrspace(1)* %126, align 4
  br label %postload348.i

postload348.i:                                    ; preds = %preload347.i, %postload342.i
  %phi349.i = phi float [ undef, %postload342.i ], [ %masked_load291.i, %preload347.i ]
  br i1 %extract179.i, label %preload353.i, label %postload354.i

preload353.i:                                     ; preds = %postload348.i
  %masked_load292.i = load float addrspace(1)* %129, align 4
  br label %postload354.i

postload354.i:                                    ; preds = %preload353.i, %postload348.i
  %phi355.i = phi float [ undef, %postload348.i ], [ %masked_load292.i, %preload353.i ]
  br i1 %extract180.i, label %preload359.i, label %postload360.i

preload359.i:                                     ; preds = %postload354.i
  %masked_load293.i = load float addrspace(1)* %132, align 4
  br label %postload360.i

postload360.i:                                    ; preds = %preload359.i, %postload354.i
  %phi361.i = phi float [ undef, %postload354.i ], [ %masked_load293.i, %preload359.i ]
  br i1 %extract181.i, label %preload365.i, label %postload366.i

preload365.i:                                     ; preds = %postload360.i
  %masked_load294.i = load float addrspace(1)* %135, align 4
  br label %postload366.i

postload366.i:                                    ; preds = %preload365.i, %postload360.i
  %phi367.i = phi float [ undef, %postload360.i ], [ %masked_load294.i, %preload365.i ]
  br i1 %extract182.i, label %preload371.i, label %postload372.i

preload371.i:                                     ; preds = %postload366.i
  %masked_load295.i = load float addrspace(1)* %138, align 4
  br label %postload372.i

postload372.i:                                    ; preds = %preload371.i, %postload366.i
  %phi373.i = phi float [ undef, %postload366.i ], [ %masked_load295.i, %preload371.i ]
  br i1 %extract183.i, label %preload377.i, label %postload378.i

preload377.i:                                     ; preds = %postload372.i
  %masked_load296.i = load float addrspace(1)* %141, align 4
  br label %postload378.i

postload378.i:                                    ; preds = %preload377.i, %postload372.i
  %phi379.i = phi float [ undef, %postload372.i ], [ %masked_load296.i, %preload377.i ]
  br i1 %extract184.i, label %preload383.i, label %postload384.i

preload383.i:                                     ; preds = %postload378.i
  %masked_load297.i = load float addrspace(1)* %144, align 4
  br label %postload384.i

postload384.i:                                    ; preds = %preload383.i, %postload378.i
  %phi385.i = phi float [ undef, %postload378.i ], [ %masked_load297.i, %preload383.i ]
  br i1 %extract185.i, label %preload389.i, label %postload390.i

preload389.i:                                     ; preds = %postload384.i
  %masked_load298.i = load float addrspace(1)* %147, align 4
  br label %postload390.i

postload390.i:                                    ; preds = %preload389.i, %postload384.i
  %phi391.i = phi float [ undef, %postload384.i ], [ %masked_load298.i, %preload389.i ]
  %temp.vect217.i = insertelement <16 x float> undef, float %phi301.i, i32 0
  %temp.vect218.i = insertelement <16 x float> %temp.vect217.i, float %phi307.i, i32 1
  %temp.vect219.i = insertelement <16 x float> %temp.vect218.i, float %phi313.i, i32 2
  %temp.vect220.i = insertelement <16 x float> %temp.vect219.i, float %phi319.i, i32 3
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %phi325.i, i32 4
  %temp.vect222.i = insertelement <16 x float> %temp.vect221.i, float %phi331.i, i32 5
  %temp.vect223.i = insertelement <16 x float> %temp.vect222.i, float %phi337.i, i32 6
  %temp.vect224.i = insertelement <16 x float> %temp.vect223.i, float %phi343.i, i32 7
  %temp.vect225.i = insertelement <16 x float> %temp.vect224.i, float %phi349.i, i32 8
  %temp.vect226.i = insertelement <16 x float> %temp.vect225.i, float %phi355.i, i32 9
  %temp.vect227.i = insertelement <16 x float> %temp.vect226.i, float %phi361.i, i32 10
  %temp.vect228.i = insertelement <16 x float> %temp.vect227.i, float %phi367.i, i32 11
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %phi373.i, i32 12
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %phi379.i, i32 13
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %phi385.i, i32 14
  %temp.vect232.i = insertelement <16 x float> %temp.vect231.i, float %phi391.i, i32 15
  %151 = fmul <16 x float> %temp.vect216.i, %temp.vect232.i
  %152 = fadd <16 x float> %vectorPHI148.i, %151
  %out_sel233.i = select <16 x i1> %vectorPHI147.i, <16 x float> %152, <16 x float> %vectorPHI146.i
  %tmp52236.i = add <16 x i32> %tmp51101.i, %vector235.i
  %153 = icmp ugt <16 x i32> %tmp52236.i, %47
  %indvar.next21.i = add i32 %indvar20.i, 1
  %notCond15237.i = xor <16 x i1> %153, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr16238.i = and <16 x i1> %vectorPHI147.i, %153
  %ever_left_loop17239.i = or <16 x i1> %vectorPHI145.i, %who_left_tr16238.i
  %loop_mask19240.i = or <16 x i1> %vectorPHI144.i, %who_left_tr16238.i
  %curr_loop_mask21241.i = or <16 x i1> %loop_mask19240.i, %who_left_tr16238.i
  %ipred.i13.i = bitcast <16 x i1> %curr_loop_mask21241.i to i16
  %val.i14.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i13.i, i16 %ipred.i13.i) nounwind
  %tmp.i15.i = and i32 %val.i14.i, 1
  %res.i16.i = icmp eq i32 %tmp.i15.i, 0
  %local_edge25242.i = and <16 x i1> %vectorPHI147.i, %notCond15237.i
  br i1 %res.i16.i, label %54, label %._crit_edge.i

._crit_edge.i:                                    ; preds = %postload390.i
  %154 = icmp ugt <16 x i32> %tmp81142.i, %44
  %indvar.next24.i = add i32 %indvar23.i, 1
  %notCond28243.i = xor <16 x i1> %154, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr29244.i = and <16 x i1> %ever_left_loop17239.i, %154
  %ever_left_loop30245.i = or <16 x i1> %vectorPHI133.i, %who_left_tr29244.i
  %loop_mask32246.i = or <16 x i1> %vectorPHI134.i, %who_left_tr29244.i
  %curr_loop_mask34247.i = or <16 x i1> %loop_mask32246.i, %who_left_tr29244.i
  %ipred.i17.i = bitcast <16 x i1> %curr_loop_mask34247.i to i16
  %val.i18.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i17.i, i16 %ipred.i17.i) nounwind
  %tmp.i19.i = and i32 %val.i18.i, 1
  %res.i20.i = icmp eq i32 %tmp.i19.i, 0
  %local_edge37248.i = and <16 x i1> %ever_left_loop17239.i, %notCond28243.i
  br i1 %res.i20.i, label %bb.nph.i, label %phi-split-bb.loopexit.i

phi-split-bb.loopexit.i:                          ; preds = %._crit_edge.i
  %phitmp407.i = select <16 x i1> %ever_left_loop30245.i, <16 x float> %out_sel233.i, <16 x float> zeroinitializer
  br label %bb.nph12.split.us.i

._crit_edge13.i:                                  ; preds = %52, %bb.nph12.split.us.i, %SyncBB408.i
  %vectorPHI250.i = phi <16 x float> [ undef, %SyncBB408.i ], [ %vectorPHI.i, %52 ], [ %vectorPHI.i, %bb.nph12.split.us.i ]
  %merge55251.i = select <16 x i1> %48, <16 x float> zeroinitializer, <16 x float> %vectorPHI250.i
  %155 = fadd <16 x float> %merge55251.i, <float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01>
  %extract252.lhs.i = extractelement <16 x i64> %36, i32 0
  %extract252.i = and i64 %extract252.lhs.i, 4294967295
  %156 = getelementptr inbounds float addrspace(1)* %1, i64 %extract252.i
  %ptrTypeCast.i = bitcast float addrspace(1)* %156 to <16 x float> addrspace(1)*
  store <16 x float> %155, <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.simpleConvolution_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge13.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB408.i

____Vectorized_.simpleConvolution_separated_args.exit: ; preds = %._crit_edge13.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32 addrspace(1)*, float addrspace(1)*, double, double, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__simpleConvolution_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint2 const, uint2 const", metadata !"opencl_simpleConvolution_locals_anchor", void (i8*)* @simpleConvolution}
!1 = metadata !{i32 0, i32 0, i32 0}
