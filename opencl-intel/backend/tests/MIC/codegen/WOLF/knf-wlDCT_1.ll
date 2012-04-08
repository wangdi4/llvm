; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__DCT_CPU_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind

declare void @__DCT_CPU_VECTOR_original(float addrspace(1)* nocapture, <8 x float> addrspace(1)* nocapture, <8 x float> addrspace(1)* nocapture, i32) nounwind

define <8 x float> @_Z6vload8mPKf(i64 %offset, float* %p) nounwind readonly {
entry:
  %0 = bitcast float* %p to i32*
  %1 = tail call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i = shl i64 %offset, 3
  %add.ptr26.i.i = getelementptr inbounds i32* %0, i64 %mul25.i.i
  %conv27.i.i = bitcast i32* %add.ptr26.i.i to i8*
  %2 = tail call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %1, i16 255, i8* %conv27.i.i, i32 0, i32 0) nounwind
  %3 = tail call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %2, i16 255, i8* %conv27.i.i, i32 0, i32 0) nounwind
  %tmp3 = shufflevector <16 x float> %3, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x float> %tmp3
}

declare void @____Vectorized_.DCT_CPU_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.DCT_CPU_VECTOR_original(float addrspace(1)* nocapture, <8 x float> addrspace(1)* nocapture, <8 x float> addrspace(1)* nocapture, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__DCT_CPU_separated_args(float addrspace(1)* nocapture %output, float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %dct, i32 %width, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph26:
  %tmp134 = zext i32 %width to i64
  %tmp65 = zext i32 %width to i64
  br label %SyncBB229

SyncBB229:                                        ; preds = %thenBB, %bb.nph26
  %CurrWI..0 = phi i64 [ 0, %bb.nph26 ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph26 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %5 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %6 = load i64* %5, align 8
  %7 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %8 = load i64* %7, align 8
  %9 = add i64 %6, %8
  %"&pSB[currWI].offset227" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset227", i8 0, i64 256, i32 16, i1 false)
  %tmp136 = trunc i64 %9 to i32
  %tmp137 = mul i32 %tmp136, %width
  %tmp138 = trunc i64 %4 to i32
  %tmp139 = add i32 %tmp137, %tmp138
  %tmp140 = shl i32 %tmp139, 3
  %tmp141 = zext i32 %tmp140 to i64
  %tmp142165 = or i64 %tmp141, 7
  %tmp145166 = or i64 %tmp141, 6
  %tmp148167 = or i64 %tmp141, 5
  %tmp151168 = or i64 %tmp141, 4
  %tmp154169 = or i64 %tmp141, 3
  %tmp157170 = or i64 %tmp141, 2
  %tmp160171 = or i64 %tmp141, 1
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to [64 x float]*
  br label %bb.nph22

bb.nph22:                                         ; preds = %._crit_edge23, %SyncBB229
  %indvar98 = phi i64 [ 0, %SyncBB229 ], [ %indvar.next99, %._crit_edge23 ]
  %tmp135 = mul i64 %tmp134, %indvar98
  %tmp143 = add i64 %tmp142165, %tmp135
  %tmp146 = add i64 %tmp145166, %tmp135
  %tmp149 = add i64 %tmp148167, %tmp135
  %tmp152 = add i64 %tmp151168, %tmp135
  %tmp155 = add i64 %tmp154169, %tmp135
  %tmp158 = add i64 %tmp157170, %tmp135
  %tmp161 = add i64 %tmp160171, %tmp135
  %tmp163 = add i64 %tmp141, %tmp135
  %10 = and i64 %tmp163, 4294967295
  %11 = and i64 %tmp161, 4294967295
  %12 = getelementptr inbounds float addrspace(1)* %input, i64 %10
  %13 = getelementptr inbounds float addrspace(1)* %input, i64 %11
  %14 = and i64 %tmp158, 4294967295
  %15 = and i64 %tmp155, 4294967295
  %16 = getelementptr inbounds float addrspace(1)* %input, i64 %14
  %17 = getelementptr inbounds float addrspace(1)* %input, i64 %15
  %18 = and i64 %tmp152, 4294967295
  %19 = and i64 %tmp149, 4294967295
  %20 = getelementptr inbounds float addrspace(1)* %input, i64 %18
  %21 = getelementptr inbounds float addrspace(1)* %input, i64 %19
  %22 = and i64 %tmp146, 4294967295
  %23 = and i64 %tmp143, 4294967295
  %24 = getelementptr inbounds float addrspace(1)* %input, i64 %22
  %25 = getelementptr inbounds float addrspace(1)* %input, i64 %23
  br label %26

; <label>:26                                      ; preds = %26, %bb.nph22
  %indvar93 = phi i64 [ 0, %bb.nph22 ], [ %indvar.next94, %26 ]
  %tmp131 = shl i64 %indvar93, 3
  %tmp132 = add i64 %indvar98, %tmp131
  %scevgep129 = getelementptr [64 x float]* %CastToValueType, i64 0, i64 %tmp132
  %tmp125178 = or i64 %tmp131, 1
  %tmp123177 = or i64 %tmp131, 2
  %tmp121176 = or i64 %tmp131, 3
  %tmp119175 = or i64 %tmp131, 4
  %tmp117174 = or i64 %tmp131, 5
  %tmp115173 = or i64 %tmp131, 6
  %tmp113172 = or i64 %tmp131, 7
  %scevgep97 = getelementptr float addrspace(1)* %dct, i64 %tmp131
  %scevgep97.1 = getelementptr float addrspace(1)* %dct, i64 %tmp125178
  %scevgep97.2 = getelementptr float addrspace(1)* %dct, i64 %tmp123177
  %scevgep97.3 = getelementptr float addrspace(1)* %dct, i64 %tmp121176
  %scevgep97.4 = getelementptr float addrspace(1)* %dct, i64 %tmp119175
  %scevgep97.5 = getelementptr float addrspace(1)* %dct, i64 %tmp117174
  %scevgep97.6 = getelementptr float addrspace(1)* %dct, i64 %tmp115173
  %scevgep97.7 = getelementptr float addrspace(1)* %dct, i64 %tmp113172
  %27 = load float addrspace(1)* %scevgep97, align 4
  %28 = load float addrspace(1)* %12, align 4
  %29 = load float addrspace(1)* %13, align 4
  %30 = load float addrspace(1)* %scevgep97.1, align 4
  %.promoted = load float* %scevgep129, align 4
  %31 = fmul float %27, %28
  %32 = fmul float %30, %29
  %33 = fadd float %.promoted, %31
  %34 = load float addrspace(1)* %scevgep97.2, align 4
  %35 = load float addrspace(1)* %16, align 4
  %36 = load float addrspace(1)* %17, align 4
  %37 = load float addrspace(1)* %scevgep97.3, align 4
  %38 = fadd float %33, %32
  %39 = fmul float %34, %35
  %40 = fmul float %37, %36
  %41 = fadd float %38, %39
  %42 = load float addrspace(1)* %scevgep97.4, align 4
  %43 = load float addrspace(1)* %20, align 4
  %44 = load float addrspace(1)* %21, align 4
  %45 = load float addrspace(1)* %scevgep97.5, align 4
  %46 = fadd float %41, %40
  %47 = fmul float %42, %43
  %48 = fmul float %45, %44
  %49 = fadd float %46, %47
  %50 = load float addrspace(1)* %scevgep97.6, align 4
  %51 = load float addrspace(1)* %24, align 4
  %52 = load float addrspace(1)* %25, align 4
  %53 = load float addrspace(1)* %scevgep97.7, align 4
  %54 = fadd float %49, %48
  %55 = fmul float %50, %51
  %56 = fmul float %53, %52
  %57 = fadd float %54, %55
  %58 = fadd float %57, %56
  store float %58, float* %scevgep129, align 4
  %indvar.next94 = add i64 %indvar93, 1
  %exitcond111 = icmp eq i64 %indvar.next94, 8
  br i1 %exitcond111, label %._crit_edge23, label %26

._crit_edge23:                                    ; preds = %26
  %indvar.next99 = add i64 %indvar98, 1
  %exitcond130 = icmp eq i64 %indvar.next99, 8
  br i1 %exitcond130, label %bb.nph12, label %bb.nph22

bb.nph12:                                         ; preds = %._crit_edge23
  %tmp67 = trunc i64 %9 to i32
  %tmp68 = mul i32 %tmp67, %width
  %tmp69 = trunc i64 %4 to i32
  %tmp70 = add i32 %tmp68, %tmp69
  %tmp71 = shl i32 %tmp70, 3
  %tmp72 = zext i32 %tmp71 to i64
  %"&pSB[currWI].offset195" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType196 = bitcast i8* %"&pSB[currWI].offset195" to [64 x float]*
  %"&pSB[currWI].offset199" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType200 = bitcast i8* %"&pSB[currWI].offset199" to [64 x float]*
  %"&pSB[currWI].offset203" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType204 = bitcast i8* %"&pSB[currWI].offset203" to [64 x float]*
  %"&pSB[currWI].offset207" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType208 = bitcast i8* %"&pSB[currWI].offset207" to [64 x float]*
  %"&pSB[currWI].offset211" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType212 = bitcast i8* %"&pSB[currWI].offset211" to [64 x float]*
  %"&pSB[currWI].offset215" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType216 = bitcast i8* %"&pSB[currWI].offset215" to [64 x float]*
  %"&pSB[currWI].offset219" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType220 = bitcast i8* %"&pSB[currWI].offset219" to [64 x float]*
  %"&pSB[currWI].offset223" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType224 = bitcast i8* %"&pSB[currWI].offset223" to [64 x float]*
  br label %bb.nph7

bb.nph7:                                          ; preds = %._crit_edge8, %bb.nph12
  %indvar28 = phi i64 [ 0, %bb.nph12 ], [ %indvar.next29, %._crit_edge8 ]
  %tmp66 = mul i64 %tmp65, %indvar28
  %tmp73 = add i64 %tmp72, %tmp66
  %tmp76 = shl i64 %indvar28, 3
  %tmp77179 = or i64 %tmp76, 7
  %scevgep.7 = getelementptr float addrspace(1)* %dct, i64 %tmp77179
  %tmp79180 = or i64 %tmp76, 6
  %scevgep.6 = getelementptr float addrspace(1)* %dct, i64 %tmp79180
  %tmp81181 = or i64 %tmp76, 5
  %scevgep.5 = getelementptr float addrspace(1)* %dct, i64 %tmp81181
  %tmp83182 = or i64 %tmp76, 4
  %scevgep.4 = getelementptr float addrspace(1)* %dct, i64 %tmp83182
  %tmp85183 = or i64 %tmp76, 3
  %scevgep.3 = getelementptr float addrspace(1)* %dct, i64 %tmp85183
  %tmp87184 = or i64 %tmp76, 2
  %scevgep.2 = getelementptr float addrspace(1)* %dct, i64 %tmp87184
  %tmp89185 = or i64 %tmp76, 1
  %scevgep.1 = getelementptr float addrspace(1)* %dct, i64 %tmp89185
  %scevgep = getelementptr float addrspace(1)* %dct, i64 %tmp76
  br label %59

; <label>:59                                      ; preds = %59, %bb.nph7
  %indvar32 = phi i64 [ 0, %bb.nph7 ], [ %indvar.next33, %59 ]
  %tmp74 = add i64 %tmp73, %indvar32
  %tmp37 = shl i64 %indvar32, 3
  %60 = and i64 %tmp74, 4294967295
  %61 = getelementptr inbounds float addrspace(1)* %output, i64 %60
  store float 0.000000e+00, float addrspace(1)* %61, align 4
  %tmp50192 = or i64 %tmp37, 1
  %tmp48191 = or i64 %tmp37, 2
  %tmp46190 = or i64 %tmp37, 3
  %tmp44189 = or i64 %tmp37, 4
  %tmp42188 = or i64 %tmp37, 5
  %tmp40187 = or i64 %tmp37, 6
  %tmp38186 = or i64 %tmp37, 7
  %scevgep36 = getelementptr [64 x float]* %CastToValueType196, i64 0, i64 %tmp37
  %scevgep36.1 = getelementptr [64 x float]* %CastToValueType200, i64 0, i64 %tmp50192
  %scevgep36.2 = getelementptr [64 x float]* %CastToValueType204, i64 0, i64 %tmp48191
  %scevgep36.3 = getelementptr [64 x float]* %CastToValueType208, i64 0, i64 %tmp46190
  %scevgep36.4 = getelementptr [64 x float]* %CastToValueType212, i64 0, i64 %tmp44189
  %scevgep36.5 = getelementptr [64 x float]* %CastToValueType216, i64 0, i64 %tmp42188
  %scevgep36.6 = getelementptr [64 x float]* %CastToValueType220, i64 0, i64 %tmp40187
  %scevgep36.7 = getelementptr [64 x float]* %CastToValueType224, i64 0, i64 %tmp38186
  %62 = load float addrspace(1)* %scevgep, align 4
  %63 = load float* %scevgep36, align 16
  %64 = fmul float %62, %63
  %65 = fadd float %64, 0.000000e+00
  store float %65, float addrspace(1)* %61, align 4
  %66 = load float addrspace(1)* %scevgep.1, align 4
  %67 = load float* %scevgep36.1, align 4
  %68 = fmul float %66, %67
  %69 = fadd float %65, %68
  store float %69, float addrspace(1)* %61, align 4
  %70 = load float addrspace(1)* %scevgep.2, align 4
  %71 = load float* %scevgep36.2, align 8
  %72 = fmul float %70, %71
  %73 = fadd float %69, %72
  store float %73, float addrspace(1)* %61, align 4
  %74 = load float addrspace(1)* %scevgep.3, align 4
  %75 = load float* %scevgep36.3, align 4
  %76 = fmul float %74, %75
  %77 = fadd float %73, %76
  store float %77, float addrspace(1)* %61, align 4
  %78 = load float addrspace(1)* %scevgep.4, align 4
  %79 = load float* %scevgep36.4, align 16
  %80 = fmul float %78, %79
  %81 = fadd float %77, %80
  store float %81, float addrspace(1)* %61, align 4
  %82 = load float addrspace(1)* %scevgep.5, align 4
  %83 = load float* %scevgep36.5, align 4
  %84 = fmul float %82, %83
  %85 = fadd float %81, %84
  store float %85, float addrspace(1)* %61, align 4
  %86 = load float addrspace(1)* %scevgep.6, align 4
  %87 = load float* %scevgep36.6, align 8
  %88 = fmul float %86, %87
  %89 = fadd float %85, %88
  store float %89, float addrspace(1)* %61, align 4
  %90 = load float addrspace(1)* %scevgep.7, align 4
  %91 = load float* %scevgep36.7, align 4
  %92 = fmul float %90, %91
  %93 = fadd float %89, %92
  store float %93, float addrspace(1)* %61, align 4
  %indvar.next33 = add i64 %indvar32, 1
  %exitcond = icmp eq i64 %indvar.next33, 8
  br i1 %exitcond, label %._crit_edge8, label %59

._crit_edge8:                                     ; preds = %59
  %indvar.next29 = add i64 %indvar28, 1
  %exitcond64 = icmp eq i64 %indvar.next29, 8
  br i1 %exitcond64, label %._crit_edge13, label %bb.nph7

._crit_edge13:                                    ; preds = %._crit_edge8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge13
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 13056
  br label %SyncBB229

SyncBB:                                           ; preds = %._crit_edge13
  ret void
}

define void @__DCT_CPU_VECTOR_separated_args(float addrspace(1)* nocapture %output, <8 x float> addrspace(1)* nocapture %input, <8 x float> addrspace(1)* nocapture %dct, i32 %width, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph18:
  %tmp61 = lshr i32 %width, 3
  %tmp62 = zext i32 %tmp61 to i64
  %tmp33 = zext i32 %width to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph18
  %CurrWI..0 = phi i64 [ 0, %bb.nph18 ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph18 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %5 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %6 = load i64* %5, align 8
  %7 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %8 = load i64* %7, align 8
  %9 = add i64 %6, %8
  %tmp64 = trunc i64 %9 to i32
  %tmp65 = mul i32 %tmp64, %width
  %tmp66 = and i32 %tmp65, 536870911
  %tmp68 = trunc i64 %4 to i32
  %tmp69 = add i32 %tmp66, %tmp68
  %tmp70 = zext i32 %tmp69 to i64
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to [64 x float]*
  br label %bb.nph14

bb.nph14:                                         ; preds = %._crit_edge15, %SyncBB
  %indvar52 = phi i64 [ 0, %SyncBB ], [ %indvar.next53, %._crit_edge15 ]
  %tmp63 = mul i64 %tmp62, %indvar52
  %tmp71 = add i64 %tmp70, %tmp63
  %10 = and i64 %tmp71, 4294967295
  %11 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %10
  %12 = load <8 x float> addrspace(1)* %11, align 32
  br label %13

; <label>:13                                      ; preds = %13, %bb.nph14
  %indvar49 = phi i64 [ 0, %bb.nph14 ], [ %indvar.next50, %13 ]
  %tmp58 = shl i64 %indvar49, 3
  %tmp59 = add i64 %indvar52, %tmp58
  %scevgep55 = getelementptr [64 x float]* %CastToValueType, i64 0, i64 %tmp59
  %scevgep56 = getelementptr <8 x float> addrspace(1)* %dct, i64 %indvar49
  %14 = load <8 x float> addrspace(1)* %scevgep56, align 32
  %15 = fmul <8 x float> %14, %12
  %16 = extractelement <8 x float> %15, i32 0
  %17 = extractelement <8 x float> %15, i32 1
  %18 = fadd float %16, %17
  %19 = extractelement <8 x float> %15, i32 2
  %20 = fadd float %18, %19
  %21 = extractelement <8 x float> %15, i32 3
  %22 = fadd float %20, %21
  %23 = extractelement <8 x float> %15, i32 4
  %24 = fadd float %22, %23
  %25 = extractelement <8 x float> %15, i32 5
  %26 = fadd float %24, %25
  %27 = extractelement <8 x float> %15, i32 6
  %28 = fadd float %26, %27
  %29 = extractelement <8 x float> %15, i32 7
  %30 = fadd float %28, %29
  store float %30, float* %scevgep55, align 4
  %indvar.next50 = add i64 %indvar49, 1
  %exitcond51 = icmp eq i64 %indvar.next50, 8
  br i1 %exitcond51, label %._crit_edge15, label %13

._crit_edge15:                                    ; preds = %13
  %indvar.next53 = add i64 %indvar52, 1
  %exitcond57 = icmp eq i64 %indvar.next53, 8
  br i1 %exitcond57, label %bb.nph9, label %bb.nph14

bb.nph9:                                          ; preds = %._crit_edge15
  %"&(pSB[currWI].offset)74" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset75" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)74"
  %"&(pSB[currWI].offset)78" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset79" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)78"
  %CastToValueType80 = bitcast i8* %"&pSB[currWI].offset79" to [8 x <8 x float>]*
  br label %31

; <label>:31                                      ; preds = %31, %bb.nph9
  %indvar45 = phi i64 [ 0, %bb.nph9 ], [ %indvar.next46, %31 ]
  %scevgep48 = getelementptr [8 x <8 x float>]* %CastToValueType80, i64 0, i64 %indvar45
  %32 = bitcast i8* %"&pSB[currWI].offset75" to i32*
  %33 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i = shl i64 %indvar45, 3
  %add.ptr26.i.i.i = getelementptr inbounds i32* %32, i64 %mul25.i.i.i
  %conv27.i.i.i = bitcast i32* %add.ptr26.i.i.i to i8*
  %34 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %33, i16 255, i8* %conv27.i.i.i, i32 0, i32 0) nounwind
  %35 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %34, i16 255, i8* %conv27.i.i.i, i32 0, i32 0) nounwind
  %tmp3.i = shufflevector <16 x float> %35, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x float> %tmp3.i, <8 x float>* %scevgep48, align 32
  %indvar.next46 = add i64 %indvar45, 1
  %exitcond47 = icmp eq i64 %indvar.next46, 8
  br i1 %exitcond47, label %bb.nph5, label %31

bb.nph5:                                          ; preds = %31
  %tmp35 = trunc i64 %9 to i32
  %tmp36 = mul i32 %tmp35, %width
  %tmp37 = trunc i64 %4 to i32
  %tmp38 = add i32 %tmp36, %tmp37
  %tmp39 = shl i32 %tmp38, 3
  %tmp40 = zext i32 %tmp39 to i64
  %"&(pSB[currWI].offset)82" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset83" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)82"
  %CastToValueType84 = bitcast i8* %"&pSB[currWI].offset83" to [8 x <8 x float>]*
  br label %bb.nph

bb.nph:                                           ; preds = %._crit_edge, %bb.nph5
  %indvar19 = phi i64 [ 0, %bb.nph5 ], [ %indvar.next20, %._crit_edge ]
  %tmp34 = mul i64 %tmp33, %indvar19
  %tmp41 = add i64 %tmp40, %tmp34
  %scevgep44 = getelementptr <8 x float> addrspace(1)* %dct, i64 %indvar19
  br label %36

; <label>:36                                      ; preds = %36, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %36 ]
  %tmp42 = add i64 %tmp41, %indvar
  %scevgep = getelementptr [8 x <8 x float>]* %CastToValueType84, i64 0, i64 %indvar
  %37 = load <8 x float> addrspace(1)* %scevgep44, align 32
  %38 = load <8 x float>* %scevgep, align 32
  %39 = fmul <8 x float> %37, %38
  %40 = extractelement <8 x float> %39, i32 0
  %41 = extractelement <8 x float> %39, i32 1
  %42 = fadd float %40, %41
  %43 = extractelement <8 x float> %39, i32 2
  %44 = fadd float %42, %43
  %45 = extractelement <8 x float> %39, i32 3
  %46 = fadd float %44, %45
  %47 = extractelement <8 x float> %39, i32 4
  %48 = fadd float %46, %47
  %49 = extractelement <8 x float> %39, i32 5
  %50 = fadd float %48, %49
  %51 = extractelement <8 x float> %39, i32 6
  %52 = fadd float %50, %51
  %53 = extractelement <8 x float> %39, i32 7
  %54 = fadd float %52, %53
  %55 = and i64 %tmp42, 4294967295
  %56 = getelementptr inbounds float addrspace(1)* %output, i64 %55
  store float %54, float addrspace(1)* %56, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 8
  br i1 %exitcond, label %._crit_edge, label %36

._crit_edge:                                      ; preds = %36
  %indvar.next20 = add i64 %indvar19, 1
  %exitcond32 = icmp eq i64 %indvar.next20, 8
  br i1 %exitcond32, label %._crit_edge6, label %bb.nph

._crit_edge6:                                     ; preds = %._crit_edge
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB85

thenBB:                                           ; preds = %._crit_edge6
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 13056
  br label %SyncBB

SyncBB85:                                         ; preds = %._crit_edge6
  ret void
}

define void @____Vectorized_.DCT_CPU_separated_args(float addrspace(1)* nocapture %output, float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %dct, i32 %width, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph26:
  %tmp134 = zext i32 %width to i64
  %tmp65 = zext i32 %width to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph26
  %CurrWI..0 = phi i64 [ 0, %bb.nph26 ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph26 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %broadcast1 = insertelement <16 x i64> undef, i64 %4, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %5 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %6 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %7 = load i64* %6, align 8
  %8 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = add i64 %7, %9
  %"&(pSB[currWI].offset)657" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset658" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)657"
  %"&(pSB[currWI].offset)697" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset698" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)697"
  %"&(pSB[currWI].offset)737" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset738" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)737"
  %"&(pSB[currWI].offset)777" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset778" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)777"
  %"&(pSB[currWI].offset)817" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset818" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)817"
  %"&(pSB[currWI].offset)857" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset858" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)857"
  %"&(pSB[currWI].offset)897" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset898" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)897"
  %"&(pSB[currWI].offset)937" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset938" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)937"
  %"&(pSB[currWI].offset)977" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset978" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)977"
  %"&(pSB[currWI].offset)1017" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset1018" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1017"
  %"&(pSB[currWI].offset)1057" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1058" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1057"
  %"&(pSB[currWI].offset)1097" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1098" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1097"
  %"&(pSB[currWI].offset)1137" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1138" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1137"
  %"&(pSB[currWI].offset)1177" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1178" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1177"
  %"&(pSB[currWI].offset)1217" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1218" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1217"
  %"&(pSB[currWI].offset)1257" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1258" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1257"
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset658", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset698", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset738", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset778", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset818", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset858", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset898", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset938", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset978", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1018", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1058", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1098", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1138", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1178", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1218", i8 0, i64 256, i32 16, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1258", i8 0, i64 256, i32 16, i1 false)
  %tmp136 = trunc i64 %10 to i32
  %tmp137 = mul i32 %tmp136, %width
  %temp = insertelement <16 x i32> undef, i32 %tmp137, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp1381 = trunc <16 x i64> %5 to <16 x i32>
  %tmp1392 = add <16 x i32> %vector, %tmp1381
  %tmp1403 = shl <16 x i32> %tmp1392, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp1414 = zext <16 x i32> %tmp1403 to <16 x i64>
  %tmp1421655 = or <16 x i64> %tmp1414, <i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7>
  %tmp1451666 = or <16 x i64> %tmp1414, <i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6>
  %tmp1481677 = or <16 x i64> %tmp1414, <i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5>
  %tmp1511688 = or <16 x i64> %tmp1414, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  %tmp1541699 = or <16 x i64> %tmp1414, <i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3>
  %tmp15717010 = or <16 x i64> %tmp1414, <i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2>
  %tmp16017111 = or <16 x i64> %tmp1414, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %"&(pSB[currWI].offset)653" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset654" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)653"
  %CastToValueType655 = bitcast i8* %"&pSB[currWI].offset654" to [64 x float]*
  %"&(pSB[currWI].offset)693" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset694" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)693"
  %CastToValueType695 = bitcast i8* %"&pSB[currWI].offset694" to [64 x float]*
  %"&(pSB[currWI].offset)733" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset734" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)733"
  %CastToValueType735 = bitcast i8* %"&pSB[currWI].offset734" to [64 x float]*
  %"&(pSB[currWI].offset)773" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset774" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)773"
  %CastToValueType775 = bitcast i8* %"&pSB[currWI].offset774" to [64 x float]*
  %"&(pSB[currWI].offset)813" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset814" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)813"
  %CastToValueType815 = bitcast i8* %"&pSB[currWI].offset814" to [64 x float]*
  %"&(pSB[currWI].offset)853" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset854" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)853"
  %CastToValueType855 = bitcast i8* %"&pSB[currWI].offset854" to [64 x float]*
  %"&(pSB[currWI].offset)893" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset894" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)893"
  %CastToValueType895 = bitcast i8* %"&pSB[currWI].offset894" to [64 x float]*
  %"&(pSB[currWI].offset)933" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset934" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)933"
  %CastToValueType935 = bitcast i8* %"&pSB[currWI].offset934" to [64 x float]*
  %"&(pSB[currWI].offset)973" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset974" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)973"
  %CastToValueType975 = bitcast i8* %"&pSB[currWI].offset974" to [64 x float]*
  %"&(pSB[currWI].offset)1013" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset1014" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1013"
  %CastToValueType1015 = bitcast i8* %"&pSB[currWI].offset1014" to [64 x float]*
  %"&(pSB[currWI].offset)1053" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1054" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1053"
  %CastToValueType1055 = bitcast i8* %"&pSB[currWI].offset1054" to [64 x float]*
  %"&(pSB[currWI].offset)1093" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1094" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1093"
  %CastToValueType1095 = bitcast i8* %"&pSB[currWI].offset1094" to [64 x float]*
  %"&(pSB[currWI].offset)1133" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1134" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1133"
  %CastToValueType1135 = bitcast i8* %"&pSB[currWI].offset1134" to [64 x float]*
  %"&(pSB[currWI].offset)1173" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1174" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1173"
  %CastToValueType1175 = bitcast i8* %"&pSB[currWI].offset1174" to [64 x float]*
  %"&(pSB[currWI].offset)1213" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1214" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1213"
  %CastToValueType1215 = bitcast i8* %"&pSB[currWI].offset1214" to [64 x float]*
  %"&(pSB[currWI].offset)1253" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1254" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1253"
  %CastToValueType1255 = bitcast i8* %"&pSB[currWI].offset1254" to [64 x float]*
  br label %bb.nph22

bb.nph22:                                         ; preds = %._crit_edge23, %SyncBB
  %indvar98 = phi i64 [ 0, %SyncBB ], [ %indvar.next99, %._crit_edge23 ]
  %tmp135 = mul i64 %tmp134, %indvar98
  %temp12 = insertelement <16 x i64> undef, i64 %tmp135, i32 0
  %vector13 = shufflevector <16 x i64> %temp12, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp14314 = add <16 x i64> %tmp1421655, %vector13
  %tmp14615 = add <16 x i64> %tmp1451666, %vector13
  %tmp14916 = add <16 x i64> %tmp1481677, %vector13
  %tmp15217 = add <16 x i64> %tmp1511688, %vector13
  %tmp15518 = add <16 x i64> %tmp1541699, %vector13
  %tmp15819 = add <16 x i64> %tmp15717010, %vector13
  %tmp16120 = add <16 x i64> %tmp16017111, %vector13
  %tmp16321 = add <16 x i64> %tmp1414, %vector13
  %11 = and <16 x i64> %tmp16321, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract = extractelement <16 x i64> %11, i32 0
  %extract22 = extractelement <16 x i64> %11, i32 1
  %extract23 = extractelement <16 x i64> %11, i32 2
  %extract24 = extractelement <16 x i64> %11, i32 3
  %extract25 = extractelement <16 x i64> %11, i32 4
  %extract26 = extractelement <16 x i64> %11, i32 5
  %extract27 = extractelement <16 x i64> %11, i32 6
  %extract28 = extractelement <16 x i64> %11, i32 7
  %extract29 = extractelement <16 x i64> %11, i32 8
  %extract30 = extractelement <16 x i64> %11, i32 9
  %extract31 = extractelement <16 x i64> %11, i32 10
  %extract32 = extractelement <16 x i64> %11, i32 11
  %extract33 = extractelement <16 x i64> %11, i32 12
  %extract34 = extractelement <16 x i64> %11, i32 13
  %extract35 = extractelement <16 x i64> %11, i32 14
  %extract36 = extractelement <16 x i64> %11, i32 15
  %12 = and <16 x i64> %tmp16120, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract37 = extractelement <16 x i64> %12, i32 0
  %extract38 = extractelement <16 x i64> %12, i32 1
  %extract39 = extractelement <16 x i64> %12, i32 2
  %extract40 = extractelement <16 x i64> %12, i32 3
  %extract41 = extractelement <16 x i64> %12, i32 4
  %extract42 = extractelement <16 x i64> %12, i32 5
  %extract43 = extractelement <16 x i64> %12, i32 6
  %extract44 = extractelement <16 x i64> %12, i32 7
  %extract45 = extractelement <16 x i64> %12, i32 8
  %extract46 = extractelement <16 x i64> %12, i32 9
  %extract47 = extractelement <16 x i64> %12, i32 10
  %extract48 = extractelement <16 x i64> %12, i32 11
  %extract49 = extractelement <16 x i64> %12, i32 12
  %extract50 = extractelement <16 x i64> %12, i32 13
  %extract51 = extractelement <16 x i64> %12, i32 14
  %extract52 = extractelement <16 x i64> %12, i32 15
  %13 = getelementptr inbounds float addrspace(1)* %input, i64 %extract
  %14 = getelementptr inbounds float addrspace(1)* %input, i64 %extract22
  %15 = getelementptr inbounds float addrspace(1)* %input, i64 %extract23
  %16 = getelementptr inbounds float addrspace(1)* %input, i64 %extract24
  %17 = getelementptr inbounds float addrspace(1)* %input, i64 %extract25
  %18 = getelementptr inbounds float addrspace(1)* %input, i64 %extract26
  %19 = getelementptr inbounds float addrspace(1)* %input, i64 %extract27
  %20 = getelementptr inbounds float addrspace(1)* %input, i64 %extract28
  %21 = getelementptr inbounds float addrspace(1)* %input, i64 %extract29
  %22 = getelementptr inbounds float addrspace(1)* %input, i64 %extract30
  %23 = getelementptr inbounds float addrspace(1)* %input, i64 %extract31
  %24 = getelementptr inbounds float addrspace(1)* %input, i64 %extract32
  %25 = getelementptr inbounds float addrspace(1)* %input, i64 %extract33
  %26 = getelementptr inbounds float addrspace(1)* %input, i64 %extract34
  %27 = getelementptr inbounds float addrspace(1)* %input, i64 %extract35
  %28 = getelementptr inbounds float addrspace(1)* %input, i64 %extract36
  %29 = getelementptr inbounds float addrspace(1)* %input, i64 %extract37
  %30 = getelementptr inbounds float addrspace(1)* %input, i64 %extract38
  %31 = getelementptr inbounds float addrspace(1)* %input, i64 %extract39
  %32 = getelementptr inbounds float addrspace(1)* %input, i64 %extract40
  %33 = getelementptr inbounds float addrspace(1)* %input, i64 %extract41
  %34 = getelementptr inbounds float addrspace(1)* %input, i64 %extract42
  %35 = getelementptr inbounds float addrspace(1)* %input, i64 %extract43
  %36 = getelementptr inbounds float addrspace(1)* %input, i64 %extract44
  %37 = getelementptr inbounds float addrspace(1)* %input, i64 %extract45
  %38 = getelementptr inbounds float addrspace(1)* %input, i64 %extract46
  %39 = getelementptr inbounds float addrspace(1)* %input, i64 %extract47
  %40 = getelementptr inbounds float addrspace(1)* %input, i64 %extract48
  %41 = getelementptr inbounds float addrspace(1)* %input, i64 %extract49
  %42 = getelementptr inbounds float addrspace(1)* %input, i64 %extract50
  %43 = getelementptr inbounds float addrspace(1)* %input, i64 %extract51
  %44 = getelementptr inbounds float addrspace(1)* %input, i64 %extract52
  %45 = and <16 x i64> %tmp15819, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract70 = extractelement <16 x i64> %45, i32 0
  %extract71 = extractelement <16 x i64> %45, i32 1
  %extract72 = extractelement <16 x i64> %45, i32 2
  %extract73 = extractelement <16 x i64> %45, i32 3
  %extract74 = extractelement <16 x i64> %45, i32 4
  %extract75 = extractelement <16 x i64> %45, i32 5
  %extract76 = extractelement <16 x i64> %45, i32 6
  %extract77 = extractelement <16 x i64> %45, i32 7
  %extract78 = extractelement <16 x i64> %45, i32 8
  %extract79 = extractelement <16 x i64> %45, i32 9
  %extract80 = extractelement <16 x i64> %45, i32 10
  %extract81 = extractelement <16 x i64> %45, i32 11
  %extract82 = extractelement <16 x i64> %45, i32 12
  %extract83 = extractelement <16 x i64> %45, i32 13
  %extract84 = extractelement <16 x i64> %45, i32 14
  %extract85 = extractelement <16 x i64> %45, i32 15
  %46 = and <16 x i64> %tmp15518, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract86 = extractelement <16 x i64> %46, i32 0
  %extract87 = extractelement <16 x i64> %46, i32 1
  %extract88 = extractelement <16 x i64> %46, i32 2
  %extract89 = extractelement <16 x i64> %46, i32 3
  %extract90 = extractelement <16 x i64> %46, i32 4
  %extract91 = extractelement <16 x i64> %46, i32 5
  %extract92 = extractelement <16 x i64> %46, i32 6
  %extract93 = extractelement <16 x i64> %46, i32 7
  %extract94 = extractelement <16 x i64> %46, i32 8
  %extract95 = extractelement <16 x i64> %46, i32 9
  %extract96 = extractelement <16 x i64> %46, i32 10
  %extract97 = extractelement <16 x i64> %46, i32 11
  %extract98 = extractelement <16 x i64> %46, i32 12
  %extract99 = extractelement <16 x i64> %46, i32 13
  %extract100 = extractelement <16 x i64> %46, i32 14
  %extract101 = extractelement <16 x i64> %46, i32 15
  %47 = getelementptr inbounds float addrspace(1)* %input, i64 %extract70
  %48 = getelementptr inbounds float addrspace(1)* %input, i64 %extract71
  %49 = getelementptr inbounds float addrspace(1)* %input, i64 %extract72
  %50 = getelementptr inbounds float addrspace(1)* %input, i64 %extract73
  %51 = getelementptr inbounds float addrspace(1)* %input, i64 %extract74
  %52 = getelementptr inbounds float addrspace(1)* %input, i64 %extract75
  %53 = getelementptr inbounds float addrspace(1)* %input, i64 %extract76
  %54 = getelementptr inbounds float addrspace(1)* %input, i64 %extract77
  %55 = getelementptr inbounds float addrspace(1)* %input, i64 %extract78
  %56 = getelementptr inbounds float addrspace(1)* %input, i64 %extract79
  %57 = getelementptr inbounds float addrspace(1)* %input, i64 %extract80
  %58 = getelementptr inbounds float addrspace(1)* %input, i64 %extract81
  %59 = getelementptr inbounds float addrspace(1)* %input, i64 %extract82
  %60 = getelementptr inbounds float addrspace(1)* %input, i64 %extract83
  %61 = getelementptr inbounds float addrspace(1)* %input, i64 %extract84
  %62 = getelementptr inbounds float addrspace(1)* %input, i64 %extract85
  %63 = getelementptr inbounds float addrspace(1)* %input, i64 %extract86
  %64 = getelementptr inbounds float addrspace(1)* %input, i64 %extract87
  %65 = getelementptr inbounds float addrspace(1)* %input, i64 %extract88
  %66 = getelementptr inbounds float addrspace(1)* %input, i64 %extract89
  %67 = getelementptr inbounds float addrspace(1)* %input, i64 %extract90
  %68 = getelementptr inbounds float addrspace(1)* %input, i64 %extract91
  %69 = getelementptr inbounds float addrspace(1)* %input, i64 %extract92
  %70 = getelementptr inbounds float addrspace(1)* %input, i64 %extract93
  %71 = getelementptr inbounds float addrspace(1)* %input, i64 %extract94
  %72 = getelementptr inbounds float addrspace(1)* %input, i64 %extract95
  %73 = getelementptr inbounds float addrspace(1)* %input, i64 %extract96
  %74 = getelementptr inbounds float addrspace(1)* %input, i64 %extract97
  %75 = getelementptr inbounds float addrspace(1)* %input, i64 %extract98
  %76 = getelementptr inbounds float addrspace(1)* %input, i64 %extract99
  %77 = getelementptr inbounds float addrspace(1)* %input, i64 %extract100
  %78 = getelementptr inbounds float addrspace(1)* %input, i64 %extract101
  %79 = and <16 x i64> %tmp15217, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract154 = extractelement <16 x i64> %79, i32 0
  %extract155 = extractelement <16 x i64> %79, i32 1
  %extract156 = extractelement <16 x i64> %79, i32 2
  %extract157 = extractelement <16 x i64> %79, i32 3
  %extract158 = extractelement <16 x i64> %79, i32 4
  %extract159 = extractelement <16 x i64> %79, i32 5
  %extract160 = extractelement <16 x i64> %79, i32 6
  %extract161 = extractelement <16 x i64> %79, i32 7
  %extract162 = extractelement <16 x i64> %79, i32 8
  %extract163 = extractelement <16 x i64> %79, i32 9
  %extract164 = extractelement <16 x i64> %79, i32 10
  %extract165 = extractelement <16 x i64> %79, i32 11
  %extract166 = extractelement <16 x i64> %79, i32 12
  %extract167 = extractelement <16 x i64> %79, i32 13
  %extract168 = extractelement <16 x i64> %79, i32 14
  %extract169 = extractelement <16 x i64> %79, i32 15
  %80 = and <16 x i64> %tmp14916, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract170 = extractelement <16 x i64> %80, i32 0
  %extract171 = extractelement <16 x i64> %80, i32 1
  %extract172 = extractelement <16 x i64> %80, i32 2
  %extract173 = extractelement <16 x i64> %80, i32 3
  %extract174 = extractelement <16 x i64> %80, i32 4
  %extract175 = extractelement <16 x i64> %80, i32 5
  %extract176 = extractelement <16 x i64> %80, i32 6
  %extract177 = extractelement <16 x i64> %80, i32 7
  %extract178 = extractelement <16 x i64> %80, i32 8
  %extract179 = extractelement <16 x i64> %80, i32 9
  %extract180 = extractelement <16 x i64> %80, i32 10
  %extract181 = extractelement <16 x i64> %80, i32 11
  %extract182 = extractelement <16 x i64> %80, i32 12
  %extract183 = extractelement <16 x i64> %80, i32 13
  %extract184 = extractelement <16 x i64> %80, i32 14
  %extract185 = extractelement <16 x i64> %80, i32 15
  %81 = getelementptr inbounds float addrspace(1)* %input, i64 %extract154
  %82 = getelementptr inbounds float addrspace(1)* %input, i64 %extract155
  %83 = getelementptr inbounds float addrspace(1)* %input, i64 %extract156
  %84 = getelementptr inbounds float addrspace(1)* %input, i64 %extract157
  %85 = getelementptr inbounds float addrspace(1)* %input, i64 %extract158
  %86 = getelementptr inbounds float addrspace(1)* %input, i64 %extract159
  %87 = getelementptr inbounds float addrspace(1)* %input, i64 %extract160
  %88 = getelementptr inbounds float addrspace(1)* %input, i64 %extract161
  %89 = getelementptr inbounds float addrspace(1)* %input, i64 %extract162
  %90 = getelementptr inbounds float addrspace(1)* %input, i64 %extract163
  %91 = getelementptr inbounds float addrspace(1)* %input, i64 %extract164
  %92 = getelementptr inbounds float addrspace(1)* %input, i64 %extract165
  %93 = getelementptr inbounds float addrspace(1)* %input, i64 %extract166
  %94 = getelementptr inbounds float addrspace(1)* %input, i64 %extract167
  %95 = getelementptr inbounds float addrspace(1)* %input, i64 %extract168
  %96 = getelementptr inbounds float addrspace(1)* %input, i64 %extract169
  %97 = getelementptr inbounds float addrspace(1)* %input, i64 %extract170
  %98 = getelementptr inbounds float addrspace(1)* %input, i64 %extract171
  %99 = getelementptr inbounds float addrspace(1)* %input, i64 %extract172
  %100 = getelementptr inbounds float addrspace(1)* %input, i64 %extract173
  %101 = getelementptr inbounds float addrspace(1)* %input, i64 %extract174
  %102 = getelementptr inbounds float addrspace(1)* %input, i64 %extract175
  %103 = getelementptr inbounds float addrspace(1)* %input, i64 %extract176
  %104 = getelementptr inbounds float addrspace(1)* %input, i64 %extract177
  %105 = getelementptr inbounds float addrspace(1)* %input, i64 %extract178
  %106 = getelementptr inbounds float addrspace(1)* %input, i64 %extract179
  %107 = getelementptr inbounds float addrspace(1)* %input, i64 %extract180
  %108 = getelementptr inbounds float addrspace(1)* %input, i64 %extract181
  %109 = getelementptr inbounds float addrspace(1)* %input, i64 %extract182
  %110 = getelementptr inbounds float addrspace(1)* %input, i64 %extract183
  %111 = getelementptr inbounds float addrspace(1)* %input, i64 %extract184
  %112 = getelementptr inbounds float addrspace(1)* %input, i64 %extract185
  %113 = and <16 x i64> %tmp14615, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract222 = extractelement <16 x i64> %113, i32 0
  %extract223 = extractelement <16 x i64> %113, i32 1
  %extract224 = extractelement <16 x i64> %113, i32 2
  %extract225 = extractelement <16 x i64> %113, i32 3
  %extract226 = extractelement <16 x i64> %113, i32 4
  %extract227 = extractelement <16 x i64> %113, i32 5
  %extract228 = extractelement <16 x i64> %113, i32 6
  %extract229 = extractelement <16 x i64> %113, i32 7
  %extract230 = extractelement <16 x i64> %113, i32 8
  %extract231 = extractelement <16 x i64> %113, i32 9
  %extract232 = extractelement <16 x i64> %113, i32 10
  %extract233 = extractelement <16 x i64> %113, i32 11
  %extract234 = extractelement <16 x i64> %113, i32 12
  %extract235 = extractelement <16 x i64> %113, i32 13
  %extract236 = extractelement <16 x i64> %113, i32 14
  %extract237 = extractelement <16 x i64> %113, i32 15
  %114 = and <16 x i64> %tmp14314, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract238 = extractelement <16 x i64> %114, i32 0
  %extract239 = extractelement <16 x i64> %114, i32 1
  %extract240 = extractelement <16 x i64> %114, i32 2
  %extract241 = extractelement <16 x i64> %114, i32 3
  %extract242 = extractelement <16 x i64> %114, i32 4
  %extract243 = extractelement <16 x i64> %114, i32 5
  %extract244 = extractelement <16 x i64> %114, i32 6
  %extract245 = extractelement <16 x i64> %114, i32 7
  %extract246 = extractelement <16 x i64> %114, i32 8
  %extract247 = extractelement <16 x i64> %114, i32 9
  %extract248 = extractelement <16 x i64> %114, i32 10
  %extract249 = extractelement <16 x i64> %114, i32 11
  %extract250 = extractelement <16 x i64> %114, i32 12
  %extract251 = extractelement <16 x i64> %114, i32 13
  %extract252 = extractelement <16 x i64> %114, i32 14
  %extract253 = extractelement <16 x i64> %114, i32 15
  %115 = getelementptr inbounds float addrspace(1)* %input, i64 %extract222
  %116 = getelementptr inbounds float addrspace(1)* %input, i64 %extract223
  %117 = getelementptr inbounds float addrspace(1)* %input, i64 %extract224
  %118 = getelementptr inbounds float addrspace(1)* %input, i64 %extract225
  %119 = getelementptr inbounds float addrspace(1)* %input, i64 %extract226
  %120 = getelementptr inbounds float addrspace(1)* %input, i64 %extract227
  %121 = getelementptr inbounds float addrspace(1)* %input, i64 %extract228
  %122 = getelementptr inbounds float addrspace(1)* %input, i64 %extract229
  %123 = getelementptr inbounds float addrspace(1)* %input, i64 %extract230
  %124 = getelementptr inbounds float addrspace(1)* %input, i64 %extract231
  %125 = getelementptr inbounds float addrspace(1)* %input, i64 %extract232
  %126 = getelementptr inbounds float addrspace(1)* %input, i64 %extract233
  %127 = getelementptr inbounds float addrspace(1)* %input, i64 %extract234
  %128 = getelementptr inbounds float addrspace(1)* %input, i64 %extract235
  %129 = getelementptr inbounds float addrspace(1)* %input, i64 %extract236
  %130 = getelementptr inbounds float addrspace(1)* %input, i64 %extract237
  %131 = getelementptr inbounds float addrspace(1)* %input, i64 %extract238
  %132 = getelementptr inbounds float addrspace(1)* %input, i64 %extract239
  %133 = getelementptr inbounds float addrspace(1)* %input, i64 %extract240
  %134 = getelementptr inbounds float addrspace(1)* %input, i64 %extract241
  %135 = getelementptr inbounds float addrspace(1)* %input, i64 %extract242
  %136 = getelementptr inbounds float addrspace(1)* %input, i64 %extract243
  %137 = getelementptr inbounds float addrspace(1)* %input, i64 %extract244
  %138 = getelementptr inbounds float addrspace(1)* %input, i64 %extract245
  %139 = getelementptr inbounds float addrspace(1)* %input, i64 %extract246
  %140 = getelementptr inbounds float addrspace(1)* %input, i64 %extract247
  %141 = getelementptr inbounds float addrspace(1)* %input, i64 %extract248
  %142 = getelementptr inbounds float addrspace(1)* %input, i64 %extract249
  %143 = getelementptr inbounds float addrspace(1)* %input, i64 %extract250
  %144 = getelementptr inbounds float addrspace(1)* %input, i64 %extract251
  %145 = getelementptr inbounds float addrspace(1)* %input, i64 %extract252
  %146 = getelementptr inbounds float addrspace(1)* %input, i64 %extract253
  br label %147

; <label>:147                                     ; preds = %147, %bb.nph22
  %indvar93 = phi i64 [ 0, %bb.nph22 ], [ %indvar.next94, %147 ]
  %tmp131 = shl i64 %indvar93, 3
  %tmp132 = add i64 %indvar98, %tmp131
  %148 = getelementptr [64 x float]* %CastToValueType655, i64 0, i64 %tmp132
  %149 = getelementptr [64 x float]* %CastToValueType695, i64 0, i64 %tmp132
  %150 = getelementptr [64 x float]* %CastToValueType735, i64 0, i64 %tmp132
  %151 = getelementptr [64 x float]* %CastToValueType775, i64 0, i64 %tmp132
  %152 = getelementptr [64 x float]* %CastToValueType815, i64 0, i64 %tmp132
  %153 = getelementptr [64 x float]* %CastToValueType855, i64 0, i64 %tmp132
  %154 = getelementptr [64 x float]* %CastToValueType895, i64 0, i64 %tmp132
  %155 = getelementptr [64 x float]* %CastToValueType935, i64 0, i64 %tmp132
  %156 = getelementptr [64 x float]* %CastToValueType975, i64 0, i64 %tmp132
  %157 = getelementptr [64 x float]* %CastToValueType1015, i64 0, i64 %tmp132
  %158 = getelementptr [64 x float]* %CastToValueType1055, i64 0, i64 %tmp132
  %159 = getelementptr [64 x float]* %CastToValueType1095, i64 0, i64 %tmp132
  %160 = getelementptr [64 x float]* %CastToValueType1135, i64 0, i64 %tmp132
  %161 = getelementptr [64 x float]* %CastToValueType1175, i64 0, i64 %tmp132
  %162 = getelementptr [64 x float]* %CastToValueType1215, i64 0, i64 %tmp132
  %163 = getelementptr [64 x float]* %CastToValueType1255, i64 0, i64 %tmp132
  %tmp125178 = or i64 %tmp131, 1
  %tmp123177 = or i64 %tmp131, 2
  %tmp121176 = or i64 %tmp131, 3
  %tmp119175 = or i64 %tmp131, 4
  %tmp117174 = or i64 %tmp131, 5
  %tmp115173 = or i64 %tmp131, 6
  %tmp113172 = or i64 %tmp131, 7
  %scevgep97 = getelementptr float addrspace(1)* %dct, i64 %tmp131
  %scevgep97.1 = getelementptr float addrspace(1)* %dct, i64 %tmp125178
  %scevgep97.2 = getelementptr float addrspace(1)* %dct, i64 %tmp123177
  %scevgep97.3 = getelementptr float addrspace(1)* %dct, i64 %tmp121176
  %scevgep97.4 = getelementptr float addrspace(1)* %dct, i64 %tmp119175
  %scevgep97.5 = getelementptr float addrspace(1)* %dct, i64 %tmp117174
  %scevgep97.6 = getelementptr float addrspace(1)* %dct, i64 %tmp115173
  %scevgep97.7 = getelementptr float addrspace(1)* %dct, i64 %tmp113172
  %164 = load float addrspace(1)* %scevgep97, align 4
  %temp53 = insertelement <16 x float> undef, float %164, i32 0
  %vector54 = shufflevector <16 x float> %temp53, <16 x float> undef, <16 x i32> zeroinitializer
  %165 = load float addrspace(1)* %13, align 4
  %166 = load float addrspace(1)* %14, align 4
  %167 = load float addrspace(1)* %15, align 4
  %168 = load float addrspace(1)* %16, align 4
  %169 = load float addrspace(1)* %17, align 4
  %170 = load float addrspace(1)* %18, align 4
  %171 = load float addrspace(1)* %19, align 4
  %172 = load float addrspace(1)* %20, align 4
  %173 = load float addrspace(1)* %21, align 4
  %174 = load float addrspace(1)* %22, align 4
  %175 = load float addrspace(1)* %23, align 4
  %176 = load float addrspace(1)* %24, align 4
  %177 = load float addrspace(1)* %25, align 4
  %178 = load float addrspace(1)* %26, align 4
  %179 = load float addrspace(1)* %27, align 4
  %180 = load float addrspace(1)* %28, align 4
  %temp.vect = insertelement <16 x float> undef, float %165, i32 0
  %temp.vect55 = insertelement <16 x float> %temp.vect, float %166, i32 1
  %temp.vect56 = insertelement <16 x float> %temp.vect55, float %167, i32 2
  %temp.vect57 = insertelement <16 x float> %temp.vect56, float %168, i32 3
  %temp.vect58 = insertelement <16 x float> %temp.vect57, float %169, i32 4
  %temp.vect59 = insertelement <16 x float> %temp.vect58, float %170, i32 5
  %temp.vect60 = insertelement <16 x float> %temp.vect59, float %171, i32 6
  %temp.vect61 = insertelement <16 x float> %temp.vect60, float %172, i32 7
  %temp.vect62 = insertelement <16 x float> %temp.vect61, float %173, i32 8
  %temp.vect63 = insertelement <16 x float> %temp.vect62, float %174, i32 9
  %temp.vect64 = insertelement <16 x float> %temp.vect63, float %175, i32 10
  %temp.vect65 = insertelement <16 x float> %temp.vect64, float %176, i32 11
  %temp.vect66 = insertelement <16 x float> %temp.vect65, float %177, i32 12
  %temp.vect67 = insertelement <16 x float> %temp.vect66, float %178, i32 13
  %temp.vect68 = insertelement <16 x float> %temp.vect67, float %179, i32 14
  %temp.vect69 = insertelement <16 x float> %temp.vect68, float %180, i32 15
  %181 = load float addrspace(1)* %29, align 4
  %182 = load float addrspace(1)* %30, align 4
  %183 = load float addrspace(1)* %31, align 4
  %184 = load float addrspace(1)* %32, align 4
  %185 = load float addrspace(1)* %33, align 4
  %186 = load float addrspace(1)* %34, align 4
  %187 = load float addrspace(1)* %35, align 4
  %188 = load float addrspace(1)* %36, align 4
  %189 = load float addrspace(1)* %37, align 4
  %190 = load float addrspace(1)* %38, align 4
  %191 = load float addrspace(1)* %39, align 4
  %192 = load float addrspace(1)* %40, align 4
  %193 = load float addrspace(1)* %41, align 4
  %194 = load float addrspace(1)* %42, align 4
  %195 = load float addrspace(1)* %43, align 4
  %196 = load float addrspace(1)* %44, align 4
  %temp.vect104 = insertelement <16 x float> undef, float %181, i32 0
  %temp.vect105 = insertelement <16 x float> %temp.vect104, float %182, i32 1
  %temp.vect106 = insertelement <16 x float> %temp.vect105, float %183, i32 2
  %temp.vect107 = insertelement <16 x float> %temp.vect106, float %184, i32 3
  %temp.vect108 = insertelement <16 x float> %temp.vect107, float %185, i32 4
  %temp.vect109 = insertelement <16 x float> %temp.vect108, float %186, i32 5
  %temp.vect110 = insertelement <16 x float> %temp.vect109, float %187, i32 6
  %temp.vect111 = insertelement <16 x float> %temp.vect110, float %188, i32 7
  %temp.vect112 = insertelement <16 x float> %temp.vect111, float %189, i32 8
  %temp.vect113 = insertelement <16 x float> %temp.vect112, float %190, i32 9
  %temp.vect114 = insertelement <16 x float> %temp.vect113, float %191, i32 10
  %temp.vect115 = insertelement <16 x float> %temp.vect114, float %192, i32 11
  %temp.vect116 = insertelement <16 x float> %temp.vect115, float %193, i32 12
  %temp.vect117 = insertelement <16 x float> %temp.vect116, float %194, i32 13
  %temp.vect118 = insertelement <16 x float> %temp.vect117, float %195, i32 14
  %temp.vect119 = insertelement <16 x float> %temp.vect118, float %196, i32 15
  %197 = load float addrspace(1)* %scevgep97.1, align 4
  %temp102 = insertelement <16 x float> undef, float %197, i32 0
  %vector103 = shufflevector <16 x float> %temp102, <16 x float> undef, <16 x i32> zeroinitializer
  %198 = load float* %148, align 4
  %199 = load float* %149, align 4
  %200 = load float* %150, align 4
  %201 = load float* %151, align 4
  %202 = load float* %152, align 4
  %203 = load float* %153, align 4
  %204 = load float* %154, align 4
  %205 = load float* %155, align 4
  %206 = load float* %156, align 4
  %207 = load float* %157, align 4
  %208 = load float* %158, align 4
  %209 = load float* %159, align 4
  %210 = load float* %160, align 4
  %211 = load float* %161, align 4
  %212 = load float* %162, align 4
  %213 = load float* %163, align 4
  %temp.vect120 = insertelement <16 x float> undef, float %198, i32 0
  %temp.vect121 = insertelement <16 x float> %temp.vect120, float %199, i32 1
  %temp.vect122 = insertelement <16 x float> %temp.vect121, float %200, i32 2
  %temp.vect123 = insertelement <16 x float> %temp.vect122, float %201, i32 3
  %temp.vect124 = insertelement <16 x float> %temp.vect123, float %202, i32 4
  %temp.vect125 = insertelement <16 x float> %temp.vect124, float %203, i32 5
  %temp.vect126 = insertelement <16 x float> %temp.vect125, float %204, i32 6
  %temp.vect127 = insertelement <16 x float> %temp.vect126, float %205, i32 7
  %temp.vect128 = insertelement <16 x float> %temp.vect127, float %206, i32 8
  %temp.vect129 = insertelement <16 x float> %temp.vect128, float %207, i32 9
  %temp.vect130 = insertelement <16 x float> %temp.vect129, float %208, i32 10
  %temp.vect131 = insertelement <16 x float> %temp.vect130, float %209, i32 11
  %temp.vect132 = insertelement <16 x float> %temp.vect131, float %210, i32 12
  %temp.vect133 = insertelement <16 x float> %temp.vect132, float %211, i32 13
  %temp.vect134 = insertelement <16 x float> %temp.vect133, float %212, i32 14
  %temp.vect135 = insertelement <16 x float> %temp.vect134, float %213, i32 15
  %214 = fmul <16 x float> %vector54, %temp.vect69
  %215 = fmul <16 x float> %vector103, %temp.vect119
  %216 = fadd <16 x float> %temp.vect135, %214
  %217 = load float addrspace(1)* %scevgep97.2, align 4
  %temp136 = insertelement <16 x float> undef, float %217, i32 0
  %vector137 = shufflevector <16 x float> %temp136, <16 x float> undef, <16 x i32> zeroinitializer
  %218 = load float addrspace(1)* %47, align 4
  %219 = load float addrspace(1)* %48, align 4
  %220 = load float addrspace(1)* %49, align 4
  %221 = load float addrspace(1)* %50, align 4
  %222 = load float addrspace(1)* %51, align 4
  %223 = load float addrspace(1)* %52, align 4
  %224 = load float addrspace(1)* %53, align 4
  %225 = load float addrspace(1)* %54, align 4
  %226 = load float addrspace(1)* %55, align 4
  %227 = load float addrspace(1)* %56, align 4
  %228 = load float addrspace(1)* %57, align 4
  %229 = load float addrspace(1)* %58, align 4
  %230 = load float addrspace(1)* %59, align 4
  %231 = load float addrspace(1)* %60, align 4
  %232 = load float addrspace(1)* %61, align 4
  %233 = load float addrspace(1)* %62, align 4
  %temp.vect138 = insertelement <16 x float> undef, float %218, i32 0
  %temp.vect139 = insertelement <16 x float> %temp.vect138, float %219, i32 1
  %temp.vect140 = insertelement <16 x float> %temp.vect139, float %220, i32 2
  %temp.vect141 = insertelement <16 x float> %temp.vect140, float %221, i32 3
  %temp.vect142 = insertelement <16 x float> %temp.vect141, float %222, i32 4
  %temp.vect143 = insertelement <16 x float> %temp.vect142, float %223, i32 5
  %temp.vect144 = insertelement <16 x float> %temp.vect143, float %224, i32 6
  %temp.vect145 = insertelement <16 x float> %temp.vect144, float %225, i32 7
  %temp.vect146 = insertelement <16 x float> %temp.vect145, float %226, i32 8
  %temp.vect147 = insertelement <16 x float> %temp.vect146, float %227, i32 9
  %temp.vect148 = insertelement <16 x float> %temp.vect147, float %228, i32 10
  %temp.vect149 = insertelement <16 x float> %temp.vect148, float %229, i32 11
  %temp.vect150 = insertelement <16 x float> %temp.vect149, float %230, i32 12
  %temp.vect151 = insertelement <16 x float> %temp.vect150, float %231, i32 13
  %temp.vect152 = insertelement <16 x float> %temp.vect151, float %232, i32 14
  %temp.vect153 = insertelement <16 x float> %temp.vect152, float %233, i32 15
  %234 = load float addrspace(1)* %63, align 4
  %235 = load float addrspace(1)* %64, align 4
  %236 = load float addrspace(1)* %65, align 4
  %237 = load float addrspace(1)* %66, align 4
  %238 = load float addrspace(1)* %67, align 4
  %239 = load float addrspace(1)* %68, align 4
  %240 = load float addrspace(1)* %69, align 4
  %241 = load float addrspace(1)* %70, align 4
  %242 = load float addrspace(1)* %71, align 4
  %243 = load float addrspace(1)* %72, align 4
  %244 = load float addrspace(1)* %73, align 4
  %245 = load float addrspace(1)* %74, align 4
  %246 = load float addrspace(1)* %75, align 4
  %247 = load float addrspace(1)* %76, align 4
  %248 = load float addrspace(1)* %77, align 4
  %249 = load float addrspace(1)* %78, align 4
  %temp.vect188 = insertelement <16 x float> undef, float %234, i32 0
  %temp.vect189 = insertelement <16 x float> %temp.vect188, float %235, i32 1
  %temp.vect190 = insertelement <16 x float> %temp.vect189, float %236, i32 2
  %temp.vect191 = insertelement <16 x float> %temp.vect190, float %237, i32 3
  %temp.vect192 = insertelement <16 x float> %temp.vect191, float %238, i32 4
  %temp.vect193 = insertelement <16 x float> %temp.vect192, float %239, i32 5
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %240, i32 6
  %temp.vect195 = insertelement <16 x float> %temp.vect194, float %241, i32 7
  %temp.vect196 = insertelement <16 x float> %temp.vect195, float %242, i32 8
  %temp.vect197 = insertelement <16 x float> %temp.vect196, float %243, i32 9
  %temp.vect198 = insertelement <16 x float> %temp.vect197, float %244, i32 10
  %temp.vect199 = insertelement <16 x float> %temp.vect198, float %245, i32 11
  %temp.vect200 = insertelement <16 x float> %temp.vect199, float %246, i32 12
  %temp.vect201 = insertelement <16 x float> %temp.vect200, float %247, i32 13
  %temp.vect202 = insertelement <16 x float> %temp.vect201, float %248, i32 14
  %temp.vect203 = insertelement <16 x float> %temp.vect202, float %249, i32 15
  %250 = load float addrspace(1)* %scevgep97.3, align 4
  %temp186 = insertelement <16 x float> undef, float %250, i32 0
  %vector187 = shufflevector <16 x float> %temp186, <16 x float> undef, <16 x i32> zeroinitializer
  %251 = fadd <16 x float> %216, %215
  %252 = fmul <16 x float> %vector137, %temp.vect153
  %253 = fmul <16 x float> %vector187, %temp.vect203
  %254 = fadd <16 x float> %251, %252
  %255 = load float addrspace(1)* %scevgep97.4, align 4
  %temp204 = insertelement <16 x float> undef, float %255, i32 0
  %vector205 = shufflevector <16 x float> %temp204, <16 x float> undef, <16 x i32> zeroinitializer
  %256 = load float addrspace(1)* %81, align 4
  %257 = load float addrspace(1)* %82, align 4
  %258 = load float addrspace(1)* %83, align 4
  %259 = load float addrspace(1)* %84, align 4
  %260 = load float addrspace(1)* %85, align 4
  %261 = load float addrspace(1)* %86, align 4
  %262 = load float addrspace(1)* %87, align 4
  %263 = load float addrspace(1)* %88, align 4
  %264 = load float addrspace(1)* %89, align 4
  %265 = load float addrspace(1)* %90, align 4
  %266 = load float addrspace(1)* %91, align 4
  %267 = load float addrspace(1)* %92, align 4
  %268 = load float addrspace(1)* %93, align 4
  %269 = load float addrspace(1)* %94, align 4
  %270 = load float addrspace(1)* %95, align 4
  %271 = load float addrspace(1)* %96, align 4
  %temp.vect206 = insertelement <16 x float> undef, float %256, i32 0
  %temp.vect207 = insertelement <16 x float> %temp.vect206, float %257, i32 1
  %temp.vect208 = insertelement <16 x float> %temp.vect207, float %258, i32 2
  %temp.vect209 = insertelement <16 x float> %temp.vect208, float %259, i32 3
  %temp.vect210 = insertelement <16 x float> %temp.vect209, float %260, i32 4
  %temp.vect211 = insertelement <16 x float> %temp.vect210, float %261, i32 5
  %temp.vect212 = insertelement <16 x float> %temp.vect211, float %262, i32 6
  %temp.vect213 = insertelement <16 x float> %temp.vect212, float %263, i32 7
  %temp.vect214 = insertelement <16 x float> %temp.vect213, float %264, i32 8
  %temp.vect215 = insertelement <16 x float> %temp.vect214, float %265, i32 9
  %temp.vect216 = insertelement <16 x float> %temp.vect215, float %266, i32 10
  %temp.vect217 = insertelement <16 x float> %temp.vect216, float %267, i32 11
  %temp.vect218 = insertelement <16 x float> %temp.vect217, float %268, i32 12
  %temp.vect219 = insertelement <16 x float> %temp.vect218, float %269, i32 13
  %temp.vect220 = insertelement <16 x float> %temp.vect219, float %270, i32 14
  %temp.vect221 = insertelement <16 x float> %temp.vect220, float %271, i32 15
  %272 = load float addrspace(1)* %97, align 4
  %273 = load float addrspace(1)* %98, align 4
  %274 = load float addrspace(1)* %99, align 4
  %275 = load float addrspace(1)* %100, align 4
  %276 = load float addrspace(1)* %101, align 4
  %277 = load float addrspace(1)* %102, align 4
  %278 = load float addrspace(1)* %103, align 4
  %279 = load float addrspace(1)* %104, align 4
  %280 = load float addrspace(1)* %105, align 4
  %281 = load float addrspace(1)* %106, align 4
  %282 = load float addrspace(1)* %107, align 4
  %283 = load float addrspace(1)* %108, align 4
  %284 = load float addrspace(1)* %109, align 4
  %285 = load float addrspace(1)* %110, align 4
  %286 = load float addrspace(1)* %111, align 4
  %287 = load float addrspace(1)* %112, align 4
  %temp.vect256 = insertelement <16 x float> undef, float %272, i32 0
  %temp.vect257 = insertelement <16 x float> %temp.vect256, float %273, i32 1
  %temp.vect258 = insertelement <16 x float> %temp.vect257, float %274, i32 2
  %temp.vect259 = insertelement <16 x float> %temp.vect258, float %275, i32 3
  %temp.vect260 = insertelement <16 x float> %temp.vect259, float %276, i32 4
  %temp.vect261 = insertelement <16 x float> %temp.vect260, float %277, i32 5
  %temp.vect262 = insertelement <16 x float> %temp.vect261, float %278, i32 6
  %temp.vect263 = insertelement <16 x float> %temp.vect262, float %279, i32 7
  %temp.vect264 = insertelement <16 x float> %temp.vect263, float %280, i32 8
  %temp.vect265 = insertelement <16 x float> %temp.vect264, float %281, i32 9
  %temp.vect266 = insertelement <16 x float> %temp.vect265, float %282, i32 10
  %temp.vect267 = insertelement <16 x float> %temp.vect266, float %283, i32 11
  %temp.vect268 = insertelement <16 x float> %temp.vect267, float %284, i32 12
  %temp.vect269 = insertelement <16 x float> %temp.vect268, float %285, i32 13
  %temp.vect270 = insertelement <16 x float> %temp.vect269, float %286, i32 14
  %temp.vect271 = insertelement <16 x float> %temp.vect270, float %287, i32 15
  %288 = load float addrspace(1)* %scevgep97.5, align 4
  %temp254 = insertelement <16 x float> undef, float %288, i32 0
  %vector255 = shufflevector <16 x float> %temp254, <16 x float> undef, <16 x i32> zeroinitializer
  %289 = fadd <16 x float> %254, %253
  %290 = fmul <16 x float> %vector205, %temp.vect221
  %291 = fmul <16 x float> %vector255, %temp.vect271
  %292 = fadd <16 x float> %289, %290
  %293 = load float addrspace(1)* %scevgep97.6, align 4
  %temp272 = insertelement <16 x float> undef, float %293, i32 0
  %vector273 = shufflevector <16 x float> %temp272, <16 x float> undef, <16 x i32> zeroinitializer
  %294 = load float addrspace(1)* %115, align 4
  %295 = load float addrspace(1)* %116, align 4
  %296 = load float addrspace(1)* %117, align 4
  %297 = load float addrspace(1)* %118, align 4
  %298 = load float addrspace(1)* %119, align 4
  %299 = load float addrspace(1)* %120, align 4
  %300 = load float addrspace(1)* %121, align 4
  %301 = load float addrspace(1)* %122, align 4
  %302 = load float addrspace(1)* %123, align 4
  %303 = load float addrspace(1)* %124, align 4
  %304 = load float addrspace(1)* %125, align 4
  %305 = load float addrspace(1)* %126, align 4
  %306 = load float addrspace(1)* %127, align 4
  %307 = load float addrspace(1)* %128, align 4
  %308 = load float addrspace(1)* %129, align 4
  %309 = load float addrspace(1)* %130, align 4
  %temp.vect274 = insertelement <16 x float> undef, float %294, i32 0
  %temp.vect275 = insertelement <16 x float> %temp.vect274, float %295, i32 1
  %temp.vect276 = insertelement <16 x float> %temp.vect275, float %296, i32 2
  %temp.vect277 = insertelement <16 x float> %temp.vect276, float %297, i32 3
  %temp.vect278 = insertelement <16 x float> %temp.vect277, float %298, i32 4
  %temp.vect279 = insertelement <16 x float> %temp.vect278, float %299, i32 5
  %temp.vect280 = insertelement <16 x float> %temp.vect279, float %300, i32 6
  %temp.vect281 = insertelement <16 x float> %temp.vect280, float %301, i32 7
  %temp.vect282 = insertelement <16 x float> %temp.vect281, float %302, i32 8
  %temp.vect283 = insertelement <16 x float> %temp.vect282, float %303, i32 9
  %temp.vect284 = insertelement <16 x float> %temp.vect283, float %304, i32 10
  %temp.vect285 = insertelement <16 x float> %temp.vect284, float %305, i32 11
  %temp.vect286 = insertelement <16 x float> %temp.vect285, float %306, i32 12
  %temp.vect287 = insertelement <16 x float> %temp.vect286, float %307, i32 13
  %temp.vect288 = insertelement <16 x float> %temp.vect287, float %308, i32 14
  %temp.vect289 = insertelement <16 x float> %temp.vect288, float %309, i32 15
  %310 = load float addrspace(1)* %131, align 4
  %311 = load float addrspace(1)* %132, align 4
  %312 = load float addrspace(1)* %133, align 4
  %313 = load float addrspace(1)* %134, align 4
  %314 = load float addrspace(1)* %135, align 4
  %315 = load float addrspace(1)* %136, align 4
  %316 = load float addrspace(1)* %137, align 4
  %317 = load float addrspace(1)* %138, align 4
  %318 = load float addrspace(1)* %139, align 4
  %319 = load float addrspace(1)* %140, align 4
  %320 = load float addrspace(1)* %141, align 4
  %321 = load float addrspace(1)* %142, align 4
  %322 = load float addrspace(1)* %143, align 4
  %323 = load float addrspace(1)* %144, align 4
  %324 = load float addrspace(1)* %145, align 4
  %325 = load float addrspace(1)* %146, align 4
  %temp.vect292 = insertelement <16 x float> undef, float %310, i32 0
  %temp.vect293 = insertelement <16 x float> %temp.vect292, float %311, i32 1
  %temp.vect294 = insertelement <16 x float> %temp.vect293, float %312, i32 2
  %temp.vect295 = insertelement <16 x float> %temp.vect294, float %313, i32 3
  %temp.vect296 = insertelement <16 x float> %temp.vect295, float %314, i32 4
  %temp.vect297 = insertelement <16 x float> %temp.vect296, float %315, i32 5
  %temp.vect298 = insertelement <16 x float> %temp.vect297, float %316, i32 6
  %temp.vect299 = insertelement <16 x float> %temp.vect298, float %317, i32 7
  %temp.vect300 = insertelement <16 x float> %temp.vect299, float %318, i32 8
  %temp.vect301 = insertelement <16 x float> %temp.vect300, float %319, i32 9
  %temp.vect302 = insertelement <16 x float> %temp.vect301, float %320, i32 10
  %temp.vect303 = insertelement <16 x float> %temp.vect302, float %321, i32 11
  %temp.vect304 = insertelement <16 x float> %temp.vect303, float %322, i32 12
  %temp.vect305 = insertelement <16 x float> %temp.vect304, float %323, i32 13
  %temp.vect306 = insertelement <16 x float> %temp.vect305, float %324, i32 14
  %temp.vect307 = insertelement <16 x float> %temp.vect306, float %325, i32 15
  %326 = load float addrspace(1)* %scevgep97.7, align 4
  %temp290 = insertelement <16 x float> undef, float %326, i32 0
  %vector291 = shufflevector <16 x float> %temp290, <16 x float> undef, <16 x i32> zeroinitializer
  %327 = fadd <16 x float> %292, %291
  %328 = fmul <16 x float> %vector273, %temp.vect289
  %329 = fmul <16 x float> %vector291, %temp.vect307
  %330 = fadd <16 x float> %327, %328
  %331 = fadd <16 x float> %330, %329
  %extract308 = extractelement <16 x float> %331, i32 0
  %extract309 = extractelement <16 x float> %331, i32 1
  %extract310 = extractelement <16 x float> %331, i32 2
  %extract311 = extractelement <16 x float> %331, i32 3
  %extract312 = extractelement <16 x float> %331, i32 4
  %extract313 = extractelement <16 x float> %331, i32 5
  %extract314 = extractelement <16 x float> %331, i32 6
  %extract315 = extractelement <16 x float> %331, i32 7
  %extract316 = extractelement <16 x float> %331, i32 8
  %extract317 = extractelement <16 x float> %331, i32 9
  %extract318 = extractelement <16 x float> %331, i32 10
  %extract319 = extractelement <16 x float> %331, i32 11
  %extract320 = extractelement <16 x float> %331, i32 12
  %extract321 = extractelement <16 x float> %331, i32 13
  %extract322 = extractelement <16 x float> %331, i32 14
  %extract323 = extractelement <16 x float> %331, i32 15
  store float %extract308, float* %148, align 4
  store float %extract309, float* %149, align 4
  store float %extract310, float* %150, align 4
  store float %extract311, float* %151, align 4
  store float %extract312, float* %152, align 4
  store float %extract313, float* %153, align 4
  store float %extract314, float* %154, align 4
  store float %extract315, float* %155, align 4
  store float %extract316, float* %156, align 4
  store float %extract317, float* %157, align 4
  store float %extract318, float* %158, align 4
  store float %extract319, float* %159, align 4
  store float %extract320, float* %160, align 4
  store float %extract321, float* %161, align 4
  store float %extract322, float* %162, align 4
  store float %extract323, float* %163, align 4
  %indvar.next94 = add i64 %indvar93, 1
  %exitcond111 = icmp eq i64 %indvar.next94, 8
  br i1 %exitcond111, label %._crit_edge23, label %147

._crit_edge23:                                    ; preds = %147
  %indvar.next99 = add i64 %indvar98, 1
  %exitcond130 = icmp eq i64 %indvar.next99, 8
  br i1 %exitcond130, label %bb.nph12, label %bb.nph22

bb.nph12:                                         ; preds = %._crit_edge23
  %tmp67 = trunc i64 %10 to i32
  %tmp68 = mul i32 %tmp67, %width
  %temp325 = insertelement <16 x i32> undef, i32 %tmp68, i32 0
  %vector326 = shufflevector <16 x i32> %temp325, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp69324 = trunc <16 x i64> %5 to <16 x i32>
  %tmp70327 = add <16 x i32> %vector326, %tmp69324
  %tmp71328 = shl <16 x i32> %tmp70327, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp72329 = zext <16 x i32> %tmp71328 to <16 x i64>
  %"&(pSB[currWI].offset)649" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset650" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)649"
  %CastToValueType651 = bitcast i8* %"&pSB[currWI].offset650" to [64 x float]*
  %"&(pSB[currWI].offset)689" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset690" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)689"
  %CastToValueType691 = bitcast i8* %"&pSB[currWI].offset690" to [64 x float]*
  %"&(pSB[currWI].offset)729" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset730" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)729"
  %CastToValueType731 = bitcast i8* %"&pSB[currWI].offset730" to [64 x float]*
  %"&(pSB[currWI].offset)769" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset770" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)769"
  %CastToValueType771 = bitcast i8* %"&pSB[currWI].offset770" to [64 x float]*
  %"&(pSB[currWI].offset)809" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset810" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)809"
  %CastToValueType811 = bitcast i8* %"&pSB[currWI].offset810" to [64 x float]*
  %"&(pSB[currWI].offset)849" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset850" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)849"
  %CastToValueType851 = bitcast i8* %"&pSB[currWI].offset850" to [64 x float]*
  %"&(pSB[currWI].offset)889" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset890" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)889"
  %CastToValueType891 = bitcast i8* %"&pSB[currWI].offset890" to [64 x float]*
  %"&(pSB[currWI].offset)929" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset930" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)929"
  %CastToValueType931 = bitcast i8* %"&pSB[currWI].offset930" to [64 x float]*
  %"&(pSB[currWI].offset)969" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset970" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)969"
  %CastToValueType971 = bitcast i8* %"&pSB[currWI].offset970" to [64 x float]*
  %"&(pSB[currWI].offset)1009" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset1010" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1009"
  %CastToValueType1011 = bitcast i8* %"&pSB[currWI].offset1010" to [64 x float]*
  %"&(pSB[currWI].offset)1049" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1050" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1049"
  %CastToValueType1051 = bitcast i8* %"&pSB[currWI].offset1050" to [64 x float]*
  %"&(pSB[currWI].offset)1089" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1090" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1089"
  %CastToValueType1091 = bitcast i8* %"&pSB[currWI].offset1090" to [64 x float]*
  %"&(pSB[currWI].offset)1129" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1130" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1129"
  %CastToValueType1131 = bitcast i8* %"&pSB[currWI].offset1130" to [64 x float]*
  %"&(pSB[currWI].offset)1169" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1170" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1169"
  %CastToValueType1171 = bitcast i8* %"&pSB[currWI].offset1170" to [64 x float]*
  %"&(pSB[currWI].offset)1209" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1210" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1209"
  %CastToValueType1211 = bitcast i8* %"&pSB[currWI].offset1210" to [64 x float]*
  %"&(pSB[currWI].offset)1249" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1250" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1249"
  %CastToValueType1251 = bitcast i8* %"&pSB[currWI].offset1250" to [64 x float]*
  %"&(pSB[currWI].offset)645" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset646" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)645"
  %CastToValueType647 = bitcast i8* %"&pSB[currWI].offset646" to [64 x float]*
  %"&(pSB[currWI].offset)685" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset686" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)685"
  %CastToValueType687 = bitcast i8* %"&pSB[currWI].offset686" to [64 x float]*
  %"&(pSB[currWI].offset)725" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset726" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)725"
  %CastToValueType727 = bitcast i8* %"&pSB[currWI].offset726" to [64 x float]*
  %"&(pSB[currWI].offset)765" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset766" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)765"
  %CastToValueType767 = bitcast i8* %"&pSB[currWI].offset766" to [64 x float]*
  %"&(pSB[currWI].offset)805" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset806" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)805"
  %CastToValueType807 = bitcast i8* %"&pSB[currWI].offset806" to [64 x float]*
  %"&(pSB[currWI].offset)845" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset846" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)845"
  %CastToValueType847 = bitcast i8* %"&pSB[currWI].offset846" to [64 x float]*
  %"&(pSB[currWI].offset)885" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset886" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)885"
  %CastToValueType887 = bitcast i8* %"&pSB[currWI].offset886" to [64 x float]*
  %"&(pSB[currWI].offset)925" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset926" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)925"
  %CastToValueType927 = bitcast i8* %"&pSB[currWI].offset926" to [64 x float]*
  %"&(pSB[currWI].offset)965" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset966" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)965"
  %CastToValueType967 = bitcast i8* %"&pSB[currWI].offset966" to [64 x float]*
  %"&(pSB[currWI].offset)1005" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset1006" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1005"
  %CastToValueType1007 = bitcast i8* %"&pSB[currWI].offset1006" to [64 x float]*
  %"&(pSB[currWI].offset)1045" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1046" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1045"
  %CastToValueType1047 = bitcast i8* %"&pSB[currWI].offset1046" to [64 x float]*
  %"&(pSB[currWI].offset)1085" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1086" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1085"
  %CastToValueType1087 = bitcast i8* %"&pSB[currWI].offset1086" to [64 x float]*
  %"&(pSB[currWI].offset)1125" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1126" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1125"
  %CastToValueType1127 = bitcast i8* %"&pSB[currWI].offset1126" to [64 x float]*
  %"&(pSB[currWI].offset)1165" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1166" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1165"
  %CastToValueType1167 = bitcast i8* %"&pSB[currWI].offset1166" to [64 x float]*
  %"&(pSB[currWI].offset)1205" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1206" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1205"
  %CastToValueType1207 = bitcast i8* %"&pSB[currWI].offset1206" to [64 x float]*
  %"&(pSB[currWI].offset)1245" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1246" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1245"
  %CastToValueType1247 = bitcast i8* %"&pSB[currWI].offset1246" to [64 x float]*
  %"&(pSB[currWI].offset)641" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset642" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)641"
  %CastToValueType643 = bitcast i8* %"&pSB[currWI].offset642" to [64 x float]*
  %"&(pSB[currWI].offset)681" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset682" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)681"
  %CastToValueType683 = bitcast i8* %"&pSB[currWI].offset682" to [64 x float]*
  %"&(pSB[currWI].offset)721" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset722" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)721"
  %CastToValueType723 = bitcast i8* %"&pSB[currWI].offset722" to [64 x float]*
  %"&(pSB[currWI].offset)761" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset762" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)761"
  %CastToValueType763 = bitcast i8* %"&pSB[currWI].offset762" to [64 x float]*
  %"&(pSB[currWI].offset)801" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset802" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)801"
  %CastToValueType803 = bitcast i8* %"&pSB[currWI].offset802" to [64 x float]*
  %"&(pSB[currWI].offset)841" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset842" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)841"
  %CastToValueType843 = bitcast i8* %"&pSB[currWI].offset842" to [64 x float]*
  %"&(pSB[currWI].offset)881" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset882" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)881"
  %CastToValueType883 = bitcast i8* %"&pSB[currWI].offset882" to [64 x float]*
  %"&(pSB[currWI].offset)921" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset922" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)921"
  %CastToValueType923 = bitcast i8* %"&pSB[currWI].offset922" to [64 x float]*
  %"&(pSB[currWI].offset)961" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset962" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)961"
  %CastToValueType963 = bitcast i8* %"&pSB[currWI].offset962" to [64 x float]*
  %"&(pSB[currWI].offset)1001" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset1002" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1001"
  %CastToValueType1003 = bitcast i8* %"&pSB[currWI].offset1002" to [64 x float]*
  %"&(pSB[currWI].offset)1041" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1042" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1041"
  %CastToValueType1043 = bitcast i8* %"&pSB[currWI].offset1042" to [64 x float]*
  %"&(pSB[currWI].offset)1081" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1082" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1081"
  %CastToValueType1083 = bitcast i8* %"&pSB[currWI].offset1082" to [64 x float]*
  %"&(pSB[currWI].offset)1121" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1122" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1121"
  %CastToValueType1123 = bitcast i8* %"&pSB[currWI].offset1122" to [64 x float]*
  %"&(pSB[currWI].offset)1161" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1161"
  %CastToValueType1163 = bitcast i8* %"&pSB[currWI].offset1162" to [64 x float]*
  %"&(pSB[currWI].offset)1201" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1202" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1201"
  %CastToValueType1203 = bitcast i8* %"&pSB[currWI].offset1202" to [64 x float]*
  %"&(pSB[currWI].offset)1241" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1242" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1241"
  %CastToValueType1243 = bitcast i8* %"&pSB[currWI].offset1242" to [64 x float]*
  %"&(pSB[currWI].offset)637" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset638" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)637"
  %CastToValueType639 = bitcast i8* %"&pSB[currWI].offset638" to [64 x float]*
  %"&(pSB[currWI].offset)677" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset678" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)677"
  %CastToValueType679 = bitcast i8* %"&pSB[currWI].offset678" to [64 x float]*
  %"&(pSB[currWI].offset)717" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset718" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)717"
  %CastToValueType719 = bitcast i8* %"&pSB[currWI].offset718" to [64 x float]*
  %"&(pSB[currWI].offset)757" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset758" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)757"
  %CastToValueType759 = bitcast i8* %"&pSB[currWI].offset758" to [64 x float]*
  %"&(pSB[currWI].offset)797" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset798" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)797"
  %CastToValueType799 = bitcast i8* %"&pSB[currWI].offset798" to [64 x float]*
  %"&(pSB[currWI].offset)837" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset838" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)837"
  %CastToValueType839 = bitcast i8* %"&pSB[currWI].offset838" to [64 x float]*
  %"&(pSB[currWI].offset)877" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset878" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)877"
  %CastToValueType879 = bitcast i8* %"&pSB[currWI].offset878" to [64 x float]*
  %"&(pSB[currWI].offset)917" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset918" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)917"
  %CastToValueType919 = bitcast i8* %"&pSB[currWI].offset918" to [64 x float]*
  %"&(pSB[currWI].offset)957" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset958" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)957"
  %CastToValueType959 = bitcast i8* %"&pSB[currWI].offset958" to [64 x float]*
  %"&(pSB[currWI].offset)997" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset998" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)997"
  %CastToValueType999 = bitcast i8* %"&pSB[currWI].offset998" to [64 x float]*
  %"&(pSB[currWI].offset)1037" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1038" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1037"
  %CastToValueType1039 = bitcast i8* %"&pSB[currWI].offset1038" to [64 x float]*
  %"&(pSB[currWI].offset)1077" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1078" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1077"
  %CastToValueType1079 = bitcast i8* %"&pSB[currWI].offset1078" to [64 x float]*
  %"&(pSB[currWI].offset)1117" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1118" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1117"
  %CastToValueType1119 = bitcast i8* %"&pSB[currWI].offset1118" to [64 x float]*
  %"&(pSB[currWI].offset)1157" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1158" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1157"
  %CastToValueType1159 = bitcast i8* %"&pSB[currWI].offset1158" to [64 x float]*
  %"&(pSB[currWI].offset)1197" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1198" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1197"
  %CastToValueType1199 = bitcast i8* %"&pSB[currWI].offset1198" to [64 x float]*
  %"&(pSB[currWI].offset)1237" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1238" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1237"
  %CastToValueType1239 = bitcast i8* %"&pSB[currWI].offset1238" to [64 x float]*
  %"&(pSB[currWI].offset)633" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset634" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)633"
  %CastToValueType635 = bitcast i8* %"&pSB[currWI].offset634" to [64 x float]*
  %"&(pSB[currWI].offset)673" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset674" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)673"
  %CastToValueType675 = bitcast i8* %"&pSB[currWI].offset674" to [64 x float]*
  %"&(pSB[currWI].offset)713" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset714" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)713"
  %CastToValueType715 = bitcast i8* %"&pSB[currWI].offset714" to [64 x float]*
  %"&(pSB[currWI].offset)753" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset754" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)753"
  %CastToValueType755 = bitcast i8* %"&pSB[currWI].offset754" to [64 x float]*
  %"&(pSB[currWI].offset)793" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset794" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)793"
  %CastToValueType795 = bitcast i8* %"&pSB[currWI].offset794" to [64 x float]*
  %"&(pSB[currWI].offset)833" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset834" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)833"
  %CastToValueType835 = bitcast i8* %"&pSB[currWI].offset834" to [64 x float]*
  %"&(pSB[currWI].offset)873" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset874" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)873"
  %CastToValueType875 = bitcast i8* %"&pSB[currWI].offset874" to [64 x float]*
  %"&(pSB[currWI].offset)913" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset914" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)913"
  %CastToValueType915 = bitcast i8* %"&pSB[currWI].offset914" to [64 x float]*
  %"&(pSB[currWI].offset)953" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset954" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)953"
  %CastToValueType955 = bitcast i8* %"&pSB[currWI].offset954" to [64 x float]*
  %"&(pSB[currWI].offset)993" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset994" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)993"
  %CastToValueType995 = bitcast i8* %"&pSB[currWI].offset994" to [64 x float]*
  %"&(pSB[currWI].offset)1033" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1034" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1033"
  %CastToValueType1035 = bitcast i8* %"&pSB[currWI].offset1034" to [64 x float]*
  %"&(pSB[currWI].offset)1073" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1074" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1073"
  %CastToValueType1075 = bitcast i8* %"&pSB[currWI].offset1074" to [64 x float]*
  %"&(pSB[currWI].offset)1113" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1114" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1113"
  %CastToValueType1115 = bitcast i8* %"&pSB[currWI].offset1114" to [64 x float]*
  %"&(pSB[currWI].offset)1153" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1154" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1153"
  %CastToValueType1155 = bitcast i8* %"&pSB[currWI].offset1154" to [64 x float]*
  %"&(pSB[currWI].offset)1193" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1194" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1193"
  %CastToValueType1195 = bitcast i8* %"&pSB[currWI].offset1194" to [64 x float]*
  %"&(pSB[currWI].offset)1233" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1234" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1233"
  %CastToValueType1235 = bitcast i8* %"&pSB[currWI].offset1234" to [64 x float]*
  %"&(pSB[currWI].offset)629" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset630" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)629"
  %CastToValueType631 = bitcast i8* %"&pSB[currWI].offset630" to [64 x float]*
  %"&(pSB[currWI].offset)669" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset670" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)669"
  %CastToValueType671 = bitcast i8* %"&pSB[currWI].offset670" to [64 x float]*
  %"&(pSB[currWI].offset)709" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset710" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)709"
  %CastToValueType711 = bitcast i8* %"&pSB[currWI].offset710" to [64 x float]*
  %"&(pSB[currWI].offset)749" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset750" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)749"
  %CastToValueType751 = bitcast i8* %"&pSB[currWI].offset750" to [64 x float]*
  %"&(pSB[currWI].offset)789" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset790" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)789"
  %CastToValueType791 = bitcast i8* %"&pSB[currWI].offset790" to [64 x float]*
  %"&(pSB[currWI].offset)829" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset830" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)829"
  %CastToValueType831 = bitcast i8* %"&pSB[currWI].offset830" to [64 x float]*
  %"&(pSB[currWI].offset)869" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset870" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)869"
  %CastToValueType871 = bitcast i8* %"&pSB[currWI].offset870" to [64 x float]*
  %"&(pSB[currWI].offset)909" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset910" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)909"
  %CastToValueType911 = bitcast i8* %"&pSB[currWI].offset910" to [64 x float]*
  %"&(pSB[currWI].offset)949" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset950" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)949"
  %CastToValueType951 = bitcast i8* %"&pSB[currWI].offset950" to [64 x float]*
  %"&(pSB[currWI].offset)989" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset990" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)989"
  %CastToValueType991 = bitcast i8* %"&pSB[currWI].offset990" to [64 x float]*
  %"&(pSB[currWI].offset)1029" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1030" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1029"
  %CastToValueType1031 = bitcast i8* %"&pSB[currWI].offset1030" to [64 x float]*
  %"&(pSB[currWI].offset)1069" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1070" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1069"
  %CastToValueType1071 = bitcast i8* %"&pSB[currWI].offset1070" to [64 x float]*
  %"&(pSB[currWI].offset)1109" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1110" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1109"
  %CastToValueType1111 = bitcast i8* %"&pSB[currWI].offset1110" to [64 x float]*
  %"&(pSB[currWI].offset)1149" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1150" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1149"
  %CastToValueType1151 = bitcast i8* %"&pSB[currWI].offset1150" to [64 x float]*
  %"&(pSB[currWI].offset)1189" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1190" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1189"
  %CastToValueType1191 = bitcast i8* %"&pSB[currWI].offset1190" to [64 x float]*
  %"&(pSB[currWI].offset)1229" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1230" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1229"
  %CastToValueType1231 = bitcast i8* %"&pSB[currWI].offset1230" to [64 x float]*
  %"&(pSB[currWI].offset)625" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset626" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)625"
  %CastToValueType627 = bitcast i8* %"&pSB[currWI].offset626" to [64 x float]*
  %"&(pSB[currWI].offset)665" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset666" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)665"
  %CastToValueType667 = bitcast i8* %"&pSB[currWI].offset666" to [64 x float]*
  %"&(pSB[currWI].offset)705" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset706" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)705"
  %CastToValueType707 = bitcast i8* %"&pSB[currWI].offset706" to [64 x float]*
  %"&(pSB[currWI].offset)745" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset746" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)745"
  %CastToValueType747 = bitcast i8* %"&pSB[currWI].offset746" to [64 x float]*
  %"&(pSB[currWI].offset)785" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset786" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)785"
  %CastToValueType787 = bitcast i8* %"&pSB[currWI].offset786" to [64 x float]*
  %"&(pSB[currWI].offset)825" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset826" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)825"
  %CastToValueType827 = bitcast i8* %"&pSB[currWI].offset826" to [64 x float]*
  %"&(pSB[currWI].offset)865" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset866" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)865"
  %CastToValueType867 = bitcast i8* %"&pSB[currWI].offset866" to [64 x float]*
  %"&(pSB[currWI].offset)905" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset906" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)905"
  %CastToValueType907 = bitcast i8* %"&pSB[currWI].offset906" to [64 x float]*
  %"&(pSB[currWI].offset)945" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset946" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)945"
  %CastToValueType947 = bitcast i8* %"&pSB[currWI].offset946" to [64 x float]*
  %"&(pSB[currWI].offset)985" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset986" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)985"
  %CastToValueType987 = bitcast i8* %"&pSB[currWI].offset986" to [64 x float]*
  %"&(pSB[currWI].offset)1025" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1026" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1025"
  %CastToValueType1027 = bitcast i8* %"&pSB[currWI].offset1026" to [64 x float]*
  %"&(pSB[currWI].offset)1065" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1066" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1065"
  %CastToValueType1067 = bitcast i8* %"&pSB[currWI].offset1066" to [64 x float]*
  %"&(pSB[currWI].offset)1105" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1106" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1105"
  %CastToValueType1107 = bitcast i8* %"&pSB[currWI].offset1106" to [64 x float]*
  %"&(pSB[currWI].offset)1145" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1146" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1145"
  %CastToValueType1147 = bitcast i8* %"&pSB[currWI].offset1146" to [64 x float]*
  %"&(pSB[currWI].offset)1185" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1186" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1185"
  %CastToValueType1187 = bitcast i8* %"&pSB[currWI].offset1186" to [64 x float]*
  %"&(pSB[currWI].offset)1225" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1226" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1225"
  %CastToValueType1227 = bitcast i8* %"&pSB[currWI].offset1226" to [64 x float]*
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to [64 x float]*
  %"&(pSB[currWI].offset)661" = add nuw i64 %CurrSBIndex..0, 1024
  %"&pSB[currWI].offset662" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)661"
  %CastToValueType663 = bitcast i8* %"&pSB[currWI].offset662" to [64 x float]*
  %"&(pSB[currWI].offset)701" = add nuw i64 %CurrSBIndex..0, 1280
  %"&pSB[currWI].offset702" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)701"
  %CastToValueType703 = bitcast i8* %"&pSB[currWI].offset702" to [64 x float]*
  %"&(pSB[currWI].offset)741" = add nuw i64 %CurrSBIndex..0, 1536
  %"&pSB[currWI].offset742" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)741"
  %CastToValueType743 = bitcast i8* %"&pSB[currWI].offset742" to [64 x float]*
  %"&(pSB[currWI].offset)781" = add nuw i64 %CurrSBIndex..0, 1792
  %"&pSB[currWI].offset782" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)781"
  %CastToValueType783 = bitcast i8* %"&pSB[currWI].offset782" to [64 x float]*
  %"&(pSB[currWI].offset)821" = add nuw i64 %CurrSBIndex..0, 2048
  %"&pSB[currWI].offset822" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)821"
  %CastToValueType823 = bitcast i8* %"&pSB[currWI].offset822" to [64 x float]*
  %"&(pSB[currWI].offset)861" = add nuw i64 %CurrSBIndex..0, 2304
  %"&pSB[currWI].offset862" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)861"
  %CastToValueType863 = bitcast i8* %"&pSB[currWI].offset862" to [64 x float]*
  %"&(pSB[currWI].offset)901" = add nuw i64 %CurrSBIndex..0, 2560
  %"&pSB[currWI].offset902" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)901"
  %CastToValueType903 = bitcast i8* %"&pSB[currWI].offset902" to [64 x float]*
  %"&(pSB[currWI].offset)941" = add nuw i64 %CurrSBIndex..0, 2816
  %"&pSB[currWI].offset942" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)941"
  %CastToValueType943 = bitcast i8* %"&pSB[currWI].offset942" to [64 x float]*
  %"&(pSB[currWI].offset)981" = add nuw i64 %CurrSBIndex..0, 3072
  %"&pSB[currWI].offset982" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)981"
  %CastToValueType983 = bitcast i8* %"&pSB[currWI].offset982" to [64 x float]*
  %"&(pSB[currWI].offset)1021" = add nuw i64 %CurrSBIndex..0, 3328
  %"&pSB[currWI].offset1022" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1021"
  %CastToValueType1023 = bitcast i8* %"&pSB[currWI].offset1022" to [64 x float]*
  %"&(pSB[currWI].offset)1061" = add nuw i64 %CurrSBIndex..0, 3584
  %"&pSB[currWI].offset1062" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1061"
  %CastToValueType1063 = bitcast i8* %"&pSB[currWI].offset1062" to [64 x float]*
  %"&(pSB[currWI].offset)1101" = add nuw i64 %CurrSBIndex..0, 3840
  %"&pSB[currWI].offset1102" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1101"
  %CastToValueType1103 = bitcast i8* %"&pSB[currWI].offset1102" to [64 x float]*
  %"&(pSB[currWI].offset)1141" = add nuw i64 %CurrSBIndex..0, 4096
  %"&pSB[currWI].offset1142" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1141"
  %CastToValueType1143 = bitcast i8* %"&pSB[currWI].offset1142" to [64 x float]*
  %"&(pSB[currWI].offset)1181" = add nuw i64 %CurrSBIndex..0, 4352
  %"&pSB[currWI].offset1182" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1181"
  %CastToValueType1183 = bitcast i8* %"&pSB[currWI].offset1182" to [64 x float]*
  %"&(pSB[currWI].offset)1221" = add nuw i64 %CurrSBIndex..0, 4608
  %"&pSB[currWI].offset1222" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1221"
  %CastToValueType1223 = bitcast i8* %"&pSB[currWI].offset1222" to [64 x float]*
  br label %bb.nph7

bb.nph7:                                          ; preds = %._crit_edge8, %bb.nph12
  %indvar28 = phi i64 [ 0, %bb.nph12 ], [ %indvar.next29, %._crit_edge8 ]
  %tmp66 = mul i64 %tmp65, %indvar28
  %temp330 = insertelement <16 x i64> undef, i64 %tmp66, i32 0
  %vector331 = shufflevector <16 x i64> %temp330, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp73332 = add <16 x i64> %tmp72329, %vector331
  %tmp76 = shl i64 %indvar28, 3
  %tmp77179 = or i64 %tmp76, 7
  %scevgep.7 = getelementptr float addrspace(1)* %dct, i64 %tmp77179
  %tmp79180 = or i64 %tmp76, 6
  %scevgep.6 = getelementptr float addrspace(1)* %dct, i64 %tmp79180
  %tmp81181 = or i64 %tmp76, 5
  %scevgep.5 = getelementptr float addrspace(1)* %dct, i64 %tmp81181
  %tmp83182 = or i64 %tmp76, 4
  %scevgep.4 = getelementptr float addrspace(1)* %dct, i64 %tmp83182
  %tmp85183 = or i64 %tmp76, 3
  %scevgep.3 = getelementptr float addrspace(1)* %dct, i64 %tmp85183
  %tmp87184 = or i64 %tmp76, 2
  %scevgep.2 = getelementptr float addrspace(1)* %dct, i64 %tmp87184
  %tmp89185 = or i64 %tmp76, 1
  %scevgep.1 = getelementptr float addrspace(1)* %dct, i64 %tmp89185
  %scevgep = getelementptr float addrspace(1)* %dct, i64 %tmp76
  br label %332

; <label>:332                                     ; preds = %332, %bb.nph7
  %indvar32 = phi i64 [ 0, %bb.nph7 ], [ %indvar.next33, %332 ]
  %temp333 = insertelement <16 x i64> undef, i64 %indvar32, i32 0
  %vector334 = shufflevector <16 x i64> %temp333, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp74335 = add <16 x i64> %tmp73332, %vector334
  %tmp37 = shl i64 %indvar32, 3
  %333 = and <16 x i64> %tmp74335, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract336 = extractelement <16 x i64> %333, i32 0
  %extract337 = extractelement <16 x i64> %333, i32 1
  %extract338 = extractelement <16 x i64> %333, i32 2
  %extract339 = extractelement <16 x i64> %333, i32 3
  %extract340 = extractelement <16 x i64> %333, i32 4
  %extract341 = extractelement <16 x i64> %333, i32 5
  %extract342 = extractelement <16 x i64> %333, i32 6
  %extract343 = extractelement <16 x i64> %333, i32 7
  %extract344 = extractelement <16 x i64> %333, i32 8
  %extract345 = extractelement <16 x i64> %333, i32 9
  %extract346 = extractelement <16 x i64> %333, i32 10
  %extract347 = extractelement <16 x i64> %333, i32 11
  %extract348 = extractelement <16 x i64> %333, i32 12
  %extract349 = extractelement <16 x i64> %333, i32 13
  %extract350 = extractelement <16 x i64> %333, i32 14
  %extract351 = extractelement <16 x i64> %333, i32 15
  %334 = getelementptr inbounds float addrspace(1)* %output, i64 %extract336
  %335 = getelementptr inbounds float addrspace(1)* %output, i64 %extract337
  %336 = getelementptr inbounds float addrspace(1)* %output, i64 %extract338
  %337 = getelementptr inbounds float addrspace(1)* %output, i64 %extract339
  %338 = getelementptr inbounds float addrspace(1)* %output, i64 %extract340
  %339 = getelementptr inbounds float addrspace(1)* %output, i64 %extract341
  %340 = getelementptr inbounds float addrspace(1)* %output, i64 %extract342
  %341 = getelementptr inbounds float addrspace(1)* %output, i64 %extract343
  %342 = getelementptr inbounds float addrspace(1)* %output, i64 %extract344
  %343 = getelementptr inbounds float addrspace(1)* %output, i64 %extract345
  %344 = getelementptr inbounds float addrspace(1)* %output, i64 %extract346
  %345 = getelementptr inbounds float addrspace(1)* %output, i64 %extract347
  %346 = getelementptr inbounds float addrspace(1)* %output, i64 %extract348
  %347 = getelementptr inbounds float addrspace(1)* %output, i64 %extract349
  %348 = getelementptr inbounds float addrspace(1)* %output, i64 %extract350
  %349 = getelementptr inbounds float addrspace(1)* %output, i64 %extract351
  store float 0.000000e+00, float addrspace(1)* %334, align 4
  store float 0.000000e+00, float addrspace(1)* %335, align 4
  store float 0.000000e+00, float addrspace(1)* %336, align 4
  store float 0.000000e+00, float addrspace(1)* %337, align 4
  store float 0.000000e+00, float addrspace(1)* %338, align 4
  store float 0.000000e+00, float addrspace(1)* %339, align 4
  store float 0.000000e+00, float addrspace(1)* %340, align 4
  store float 0.000000e+00, float addrspace(1)* %341, align 4
  store float 0.000000e+00, float addrspace(1)* %342, align 4
  store float 0.000000e+00, float addrspace(1)* %343, align 4
  store float 0.000000e+00, float addrspace(1)* %344, align 4
  store float 0.000000e+00, float addrspace(1)* %345, align 4
  store float 0.000000e+00, float addrspace(1)* %346, align 4
  store float 0.000000e+00, float addrspace(1)* %347, align 4
  store float 0.000000e+00, float addrspace(1)* %348, align 4
  store float 0.000000e+00, float addrspace(1)* %349, align 4
  %tmp50192 = or i64 %tmp37, 1
  %tmp48191 = or i64 %tmp37, 2
  %tmp46190 = or i64 %tmp37, 3
  %tmp44189 = or i64 %tmp37, 4
  %tmp42188 = or i64 %tmp37, 5
  %tmp40187 = or i64 %tmp37, 6
  %tmp38186 = or i64 %tmp37, 7
  %350 = getelementptr [64 x float]* %CastToValueType651, i64 0, i64 %tmp37
  %351 = getelementptr [64 x float]* %CastToValueType691, i64 0, i64 %tmp37
  %352 = getelementptr [64 x float]* %CastToValueType731, i64 0, i64 %tmp37
  %353 = getelementptr [64 x float]* %CastToValueType771, i64 0, i64 %tmp37
  %354 = getelementptr [64 x float]* %CastToValueType811, i64 0, i64 %tmp37
  %355 = getelementptr [64 x float]* %CastToValueType851, i64 0, i64 %tmp37
  %356 = getelementptr [64 x float]* %CastToValueType891, i64 0, i64 %tmp37
  %357 = getelementptr [64 x float]* %CastToValueType931, i64 0, i64 %tmp37
  %358 = getelementptr [64 x float]* %CastToValueType971, i64 0, i64 %tmp37
  %359 = getelementptr [64 x float]* %CastToValueType1011, i64 0, i64 %tmp37
  %360 = getelementptr [64 x float]* %CastToValueType1051, i64 0, i64 %tmp37
  %361 = getelementptr [64 x float]* %CastToValueType1091, i64 0, i64 %tmp37
  %362 = getelementptr [64 x float]* %CastToValueType1131, i64 0, i64 %tmp37
  %363 = getelementptr [64 x float]* %CastToValueType1171, i64 0, i64 %tmp37
  %364 = getelementptr [64 x float]* %CastToValueType1211, i64 0, i64 %tmp37
  %365 = getelementptr [64 x float]* %CastToValueType1251, i64 0, i64 %tmp37
  %366 = getelementptr [64 x float]* %CastToValueType647, i64 0, i64 %tmp50192
  %367 = getelementptr [64 x float]* %CastToValueType687, i64 0, i64 %tmp50192
  %368 = getelementptr [64 x float]* %CastToValueType727, i64 0, i64 %tmp50192
  %369 = getelementptr [64 x float]* %CastToValueType767, i64 0, i64 %tmp50192
  %370 = getelementptr [64 x float]* %CastToValueType807, i64 0, i64 %tmp50192
  %371 = getelementptr [64 x float]* %CastToValueType847, i64 0, i64 %tmp50192
  %372 = getelementptr [64 x float]* %CastToValueType887, i64 0, i64 %tmp50192
  %373 = getelementptr [64 x float]* %CastToValueType927, i64 0, i64 %tmp50192
  %374 = getelementptr [64 x float]* %CastToValueType967, i64 0, i64 %tmp50192
  %375 = getelementptr [64 x float]* %CastToValueType1007, i64 0, i64 %tmp50192
  %376 = getelementptr [64 x float]* %CastToValueType1047, i64 0, i64 %tmp50192
  %377 = getelementptr [64 x float]* %CastToValueType1087, i64 0, i64 %tmp50192
  %378 = getelementptr [64 x float]* %CastToValueType1127, i64 0, i64 %tmp50192
  %379 = getelementptr [64 x float]* %CastToValueType1167, i64 0, i64 %tmp50192
  %380 = getelementptr [64 x float]* %CastToValueType1207, i64 0, i64 %tmp50192
  %381 = getelementptr [64 x float]* %CastToValueType1247, i64 0, i64 %tmp50192
  %382 = getelementptr [64 x float]* %CastToValueType643, i64 0, i64 %tmp48191
  %383 = getelementptr [64 x float]* %CastToValueType683, i64 0, i64 %tmp48191
  %384 = getelementptr [64 x float]* %CastToValueType723, i64 0, i64 %tmp48191
  %385 = getelementptr [64 x float]* %CastToValueType763, i64 0, i64 %tmp48191
  %386 = getelementptr [64 x float]* %CastToValueType803, i64 0, i64 %tmp48191
  %387 = getelementptr [64 x float]* %CastToValueType843, i64 0, i64 %tmp48191
  %388 = getelementptr [64 x float]* %CastToValueType883, i64 0, i64 %tmp48191
  %389 = getelementptr [64 x float]* %CastToValueType923, i64 0, i64 %tmp48191
  %390 = getelementptr [64 x float]* %CastToValueType963, i64 0, i64 %tmp48191
  %391 = getelementptr [64 x float]* %CastToValueType1003, i64 0, i64 %tmp48191
  %392 = getelementptr [64 x float]* %CastToValueType1043, i64 0, i64 %tmp48191
  %393 = getelementptr [64 x float]* %CastToValueType1083, i64 0, i64 %tmp48191
  %394 = getelementptr [64 x float]* %CastToValueType1123, i64 0, i64 %tmp48191
  %395 = getelementptr [64 x float]* %CastToValueType1163, i64 0, i64 %tmp48191
  %396 = getelementptr [64 x float]* %CastToValueType1203, i64 0, i64 %tmp48191
  %397 = getelementptr [64 x float]* %CastToValueType1243, i64 0, i64 %tmp48191
  %398 = getelementptr [64 x float]* %CastToValueType639, i64 0, i64 %tmp46190
  %399 = getelementptr [64 x float]* %CastToValueType679, i64 0, i64 %tmp46190
  %400 = getelementptr [64 x float]* %CastToValueType719, i64 0, i64 %tmp46190
  %401 = getelementptr [64 x float]* %CastToValueType759, i64 0, i64 %tmp46190
  %402 = getelementptr [64 x float]* %CastToValueType799, i64 0, i64 %tmp46190
  %403 = getelementptr [64 x float]* %CastToValueType839, i64 0, i64 %tmp46190
  %404 = getelementptr [64 x float]* %CastToValueType879, i64 0, i64 %tmp46190
  %405 = getelementptr [64 x float]* %CastToValueType919, i64 0, i64 %tmp46190
  %406 = getelementptr [64 x float]* %CastToValueType959, i64 0, i64 %tmp46190
  %407 = getelementptr [64 x float]* %CastToValueType999, i64 0, i64 %tmp46190
  %408 = getelementptr [64 x float]* %CastToValueType1039, i64 0, i64 %tmp46190
  %409 = getelementptr [64 x float]* %CastToValueType1079, i64 0, i64 %tmp46190
  %410 = getelementptr [64 x float]* %CastToValueType1119, i64 0, i64 %tmp46190
  %411 = getelementptr [64 x float]* %CastToValueType1159, i64 0, i64 %tmp46190
  %412 = getelementptr [64 x float]* %CastToValueType1199, i64 0, i64 %tmp46190
  %413 = getelementptr [64 x float]* %CastToValueType1239, i64 0, i64 %tmp46190
  %414 = getelementptr [64 x float]* %CastToValueType635, i64 0, i64 %tmp44189
  %415 = getelementptr [64 x float]* %CastToValueType675, i64 0, i64 %tmp44189
  %416 = getelementptr [64 x float]* %CastToValueType715, i64 0, i64 %tmp44189
  %417 = getelementptr [64 x float]* %CastToValueType755, i64 0, i64 %tmp44189
  %418 = getelementptr [64 x float]* %CastToValueType795, i64 0, i64 %tmp44189
  %419 = getelementptr [64 x float]* %CastToValueType835, i64 0, i64 %tmp44189
  %420 = getelementptr [64 x float]* %CastToValueType875, i64 0, i64 %tmp44189
  %421 = getelementptr [64 x float]* %CastToValueType915, i64 0, i64 %tmp44189
  %422 = getelementptr [64 x float]* %CastToValueType955, i64 0, i64 %tmp44189
  %423 = getelementptr [64 x float]* %CastToValueType995, i64 0, i64 %tmp44189
  %424 = getelementptr [64 x float]* %CastToValueType1035, i64 0, i64 %tmp44189
  %425 = getelementptr [64 x float]* %CastToValueType1075, i64 0, i64 %tmp44189
  %426 = getelementptr [64 x float]* %CastToValueType1115, i64 0, i64 %tmp44189
  %427 = getelementptr [64 x float]* %CastToValueType1155, i64 0, i64 %tmp44189
  %428 = getelementptr [64 x float]* %CastToValueType1195, i64 0, i64 %tmp44189
  %429 = getelementptr [64 x float]* %CastToValueType1235, i64 0, i64 %tmp44189
  %430 = getelementptr [64 x float]* %CastToValueType631, i64 0, i64 %tmp42188
  %431 = getelementptr [64 x float]* %CastToValueType671, i64 0, i64 %tmp42188
  %432 = getelementptr [64 x float]* %CastToValueType711, i64 0, i64 %tmp42188
  %433 = getelementptr [64 x float]* %CastToValueType751, i64 0, i64 %tmp42188
  %434 = getelementptr [64 x float]* %CastToValueType791, i64 0, i64 %tmp42188
  %435 = getelementptr [64 x float]* %CastToValueType831, i64 0, i64 %tmp42188
  %436 = getelementptr [64 x float]* %CastToValueType871, i64 0, i64 %tmp42188
  %437 = getelementptr [64 x float]* %CastToValueType911, i64 0, i64 %tmp42188
  %438 = getelementptr [64 x float]* %CastToValueType951, i64 0, i64 %tmp42188
  %439 = getelementptr [64 x float]* %CastToValueType991, i64 0, i64 %tmp42188
  %440 = getelementptr [64 x float]* %CastToValueType1031, i64 0, i64 %tmp42188
  %441 = getelementptr [64 x float]* %CastToValueType1071, i64 0, i64 %tmp42188
  %442 = getelementptr [64 x float]* %CastToValueType1111, i64 0, i64 %tmp42188
  %443 = getelementptr [64 x float]* %CastToValueType1151, i64 0, i64 %tmp42188
  %444 = getelementptr [64 x float]* %CastToValueType1191, i64 0, i64 %tmp42188
  %445 = getelementptr [64 x float]* %CastToValueType1231, i64 0, i64 %tmp42188
  %446 = getelementptr [64 x float]* %CastToValueType627, i64 0, i64 %tmp40187
  %447 = getelementptr [64 x float]* %CastToValueType667, i64 0, i64 %tmp40187
  %448 = getelementptr [64 x float]* %CastToValueType707, i64 0, i64 %tmp40187
  %449 = getelementptr [64 x float]* %CastToValueType747, i64 0, i64 %tmp40187
  %450 = getelementptr [64 x float]* %CastToValueType787, i64 0, i64 %tmp40187
  %451 = getelementptr [64 x float]* %CastToValueType827, i64 0, i64 %tmp40187
  %452 = getelementptr [64 x float]* %CastToValueType867, i64 0, i64 %tmp40187
  %453 = getelementptr [64 x float]* %CastToValueType907, i64 0, i64 %tmp40187
  %454 = getelementptr [64 x float]* %CastToValueType947, i64 0, i64 %tmp40187
  %455 = getelementptr [64 x float]* %CastToValueType987, i64 0, i64 %tmp40187
  %456 = getelementptr [64 x float]* %CastToValueType1027, i64 0, i64 %tmp40187
  %457 = getelementptr [64 x float]* %CastToValueType1067, i64 0, i64 %tmp40187
  %458 = getelementptr [64 x float]* %CastToValueType1107, i64 0, i64 %tmp40187
  %459 = getelementptr [64 x float]* %CastToValueType1147, i64 0, i64 %tmp40187
  %460 = getelementptr [64 x float]* %CastToValueType1187, i64 0, i64 %tmp40187
  %461 = getelementptr [64 x float]* %CastToValueType1227, i64 0, i64 %tmp40187
  %462 = getelementptr [64 x float]* %CastToValueType, i64 0, i64 %tmp38186
  %463 = getelementptr [64 x float]* %CastToValueType663, i64 0, i64 %tmp38186
  %464 = getelementptr [64 x float]* %CastToValueType703, i64 0, i64 %tmp38186
  %465 = getelementptr [64 x float]* %CastToValueType743, i64 0, i64 %tmp38186
  %466 = getelementptr [64 x float]* %CastToValueType783, i64 0, i64 %tmp38186
  %467 = getelementptr [64 x float]* %CastToValueType823, i64 0, i64 %tmp38186
  %468 = getelementptr [64 x float]* %CastToValueType863, i64 0, i64 %tmp38186
  %469 = getelementptr [64 x float]* %CastToValueType903, i64 0, i64 %tmp38186
  %470 = getelementptr [64 x float]* %CastToValueType943, i64 0, i64 %tmp38186
  %471 = getelementptr [64 x float]* %CastToValueType983, i64 0, i64 %tmp38186
  %472 = getelementptr [64 x float]* %CastToValueType1023, i64 0, i64 %tmp38186
  %473 = getelementptr [64 x float]* %CastToValueType1063, i64 0, i64 %tmp38186
  %474 = getelementptr [64 x float]* %CastToValueType1103, i64 0, i64 %tmp38186
  %475 = getelementptr [64 x float]* %CastToValueType1143, i64 0, i64 %tmp38186
  %476 = getelementptr [64 x float]* %CastToValueType1183, i64 0, i64 %tmp38186
  %477 = getelementptr [64 x float]* %CastToValueType1223, i64 0, i64 %tmp38186
  %478 = load float addrspace(1)* %scevgep, align 4
  %temp352 = insertelement <16 x float> undef, float %478, i32 0
  %vector353 = shufflevector <16 x float> %temp352, <16 x float> undef, <16 x i32> zeroinitializer
  %479 = load float* %350, align 16
  %480 = load float* %351, align 16
  %481 = load float* %352, align 16
  %482 = load float* %353, align 16
  %483 = load float* %354, align 16
  %484 = load float* %355, align 16
  %485 = load float* %356, align 16
  %486 = load float* %357, align 16
  %487 = load float* %358, align 16
  %488 = load float* %359, align 16
  %489 = load float* %360, align 16
  %490 = load float* %361, align 16
  %491 = load float* %362, align 16
  %492 = load float* %363, align 16
  %493 = load float* %364, align 16
  %494 = load float* %365, align 16
  %temp.vect354 = insertelement <16 x float> undef, float %479, i32 0
  %temp.vect355 = insertelement <16 x float> %temp.vect354, float %480, i32 1
  %temp.vect356 = insertelement <16 x float> %temp.vect355, float %481, i32 2
  %temp.vect357 = insertelement <16 x float> %temp.vect356, float %482, i32 3
  %temp.vect358 = insertelement <16 x float> %temp.vect357, float %483, i32 4
  %temp.vect359 = insertelement <16 x float> %temp.vect358, float %484, i32 5
  %temp.vect360 = insertelement <16 x float> %temp.vect359, float %485, i32 6
  %temp.vect361 = insertelement <16 x float> %temp.vect360, float %486, i32 7
  %temp.vect362 = insertelement <16 x float> %temp.vect361, float %487, i32 8
  %temp.vect363 = insertelement <16 x float> %temp.vect362, float %488, i32 9
  %temp.vect364 = insertelement <16 x float> %temp.vect363, float %489, i32 10
  %temp.vect365 = insertelement <16 x float> %temp.vect364, float %490, i32 11
  %temp.vect366 = insertelement <16 x float> %temp.vect365, float %491, i32 12
  %temp.vect367 = insertelement <16 x float> %temp.vect366, float %492, i32 13
  %temp.vect368 = insertelement <16 x float> %temp.vect367, float %493, i32 14
  %temp.vect369 = insertelement <16 x float> %temp.vect368, float %494, i32 15
  %495 = fmul <16 x float> %vector353, %temp.vect369
  %496 = fadd <16 x float> %495, zeroinitializer
  %extract370 = extractelement <16 x float> %496, i32 0
  %extract371 = extractelement <16 x float> %496, i32 1
  %extract372 = extractelement <16 x float> %496, i32 2
  %extract373 = extractelement <16 x float> %496, i32 3
  %extract374 = extractelement <16 x float> %496, i32 4
  %extract375 = extractelement <16 x float> %496, i32 5
  %extract376 = extractelement <16 x float> %496, i32 6
  %extract377 = extractelement <16 x float> %496, i32 7
  %extract378 = extractelement <16 x float> %496, i32 8
  %extract379 = extractelement <16 x float> %496, i32 9
  %extract380 = extractelement <16 x float> %496, i32 10
  %extract381 = extractelement <16 x float> %496, i32 11
  %extract382 = extractelement <16 x float> %496, i32 12
  %extract383 = extractelement <16 x float> %496, i32 13
  %extract384 = extractelement <16 x float> %496, i32 14
  %extract385 = extractelement <16 x float> %496, i32 15
  store float %extract370, float addrspace(1)* %334, align 4
  store float %extract371, float addrspace(1)* %335, align 4
  store float %extract372, float addrspace(1)* %336, align 4
  store float %extract373, float addrspace(1)* %337, align 4
  store float %extract374, float addrspace(1)* %338, align 4
  store float %extract375, float addrspace(1)* %339, align 4
  store float %extract376, float addrspace(1)* %340, align 4
  store float %extract377, float addrspace(1)* %341, align 4
  store float %extract378, float addrspace(1)* %342, align 4
  store float %extract379, float addrspace(1)* %343, align 4
  store float %extract380, float addrspace(1)* %344, align 4
  store float %extract381, float addrspace(1)* %345, align 4
  store float %extract382, float addrspace(1)* %346, align 4
  store float %extract383, float addrspace(1)* %347, align 4
  store float %extract384, float addrspace(1)* %348, align 4
  store float %extract385, float addrspace(1)* %349, align 4
  %497 = load float addrspace(1)* %scevgep.1, align 4
  %temp386 = insertelement <16 x float> undef, float %497, i32 0
  %vector387 = shufflevector <16 x float> %temp386, <16 x float> undef, <16 x i32> zeroinitializer
  %498 = load float* %366, align 4
  %499 = load float* %367, align 4
  %500 = load float* %368, align 4
  %501 = load float* %369, align 4
  %502 = load float* %370, align 4
  %503 = load float* %371, align 4
  %504 = load float* %372, align 4
  %505 = load float* %373, align 4
  %506 = load float* %374, align 4
  %507 = load float* %375, align 4
  %508 = load float* %376, align 4
  %509 = load float* %377, align 4
  %510 = load float* %378, align 4
  %511 = load float* %379, align 4
  %512 = load float* %380, align 4
  %513 = load float* %381, align 4
  %temp.vect388 = insertelement <16 x float> undef, float %498, i32 0
  %temp.vect389 = insertelement <16 x float> %temp.vect388, float %499, i32 1
  %temp.vect390 = insertelement <16 x float> %temp.vect389, float %500, i32 2
  %temp.vect391 = insertelement <16 x float> %temp.vect390, float %501, i32 3
  %temp.vect392 = insertelement <16 x float> %temp.vect391, float %502, i32 4
  %temp.vect393 = insertelement <16 x float> %temp.vect392, float %503, i32 5
  %temp.vect394 = insertelement <16 x float> %temp.vect393, float %504, i32 6
  %temp.vect395 = insertelement <16 x float> %temp.vect394, float %505, i32 7
  %temp.vect396 = insertelement <16 x float> %temp.vect395, float %506, i32 8
  %temp.vect397 = insertelement <16 x float> %temp.vect396, float %507, i32 9
  %temp.vect398 = insertelement <16 x float> %temp.vect397, float %508, i32 10
  %temp.vect399 = insertelement <16 x float> %temp.vect398, float %509, i32 11
  %temp.vect400 = insertelement <16 x float> %temp.vect399, float %510, i32 12
  %temp.vect401 = insertelement <16 x float> %temp.vect400, float %511, i32 13
  %temp.vect402 = insertelement <16 x float> %temp.vect401, float %512, i32 14
  %temp.vect403 = insertelement <16 x float> %temp.vect402, float %513, i32 15
  %514 = fmul <16 x float> %vector387, %temp.vect403
  %515 = fadd <16 x float> %496, %514
  %extract404 = extractelement <16 x float> %515, i32 0
  %extract405 = extractelement <16 x float> %515, i32 1
  %extract406 = extractelement <16 x float> %515, i32 2
  %extract407 = extractelement <16 x float> %515, i32 3
  %extract408 = extractelement <16 x float> %515, i32 4
  %extract409 = extractelement <16 x float> %515, i32 5
  %extract410 = extractelement <16 x float> %515, i32 6
  %extract411 = extractelement <16 x float> %515, i32 7
  %extract412 = extractelement <16 x float> %515, i32 8
  %extract413 = extractelement <16 x float> %515, i32 9
  %extract414 = extractelement <16 x float> %515, i32 10
  %extract415 = extractelement <16 x float> %515, i32 11
  %extract416 = extractelement <16 x float> %515, i32 12
  %extract417 = extractelement <16 x float> %515, i32 13
  %extract418 = extractelement <16 x float> %515, i32 14
  %extract419 = extractelement <16 x float> %515, i32 15
  store float %extract404, float addrspace(1)* %334, align 4
  store float %extract405, float addrspace(1)* %335, align 4
  store float %extract406, float addrspace(1)* %336, align 4
  store float %extract407, float addrspace(1)* %337, align 4
  store float %extract408, float addrspace(1)* %338, align 4
  store float %extract409, float addrspace(1)* %339, align 4
  store float %extract410, float addrspace(1)* %340, align 4
  store float %extract411, float addrspace(1)* %341, align 4
  store float %extract412, float addrspace(1)* %342, align 4
  store float %extract413, float addrspace(1)* %343, align 4
  store float %extract414, float addrspace(1)* %344, align 4
  store float %extract415, float addrspace(1)* %345, align 4
  store float %extract416, float addrspace(1)* %346, align 4
  store float %extract417, float addrspace(1)* %347, align 4
  store float %extract418, float addrspace(1)* %348, align 4
  store float %extract419, float addrspace(1)* %349, align 4
  %516 = load float addrspace(1)* %scevgep.2, align 4
  %temp420 = insertelement <16 x float> undef, float %516, i32 0
  %vector421 = shufflevector <16 x float> %temp420, <16 x float> undef, <16 x i32> zeroinitializer
  %517 = load float* %382, align 8
  %518 = load float* %383, align 8
  %519 = load float* %384, align 8
  %520 = load float* %385, align 8
  %521 = load float* %386, align 8
  %522 = load float* %387, align 8
  %523 = load float* %388, align 8
  %524 = load float* %389, align 8
  %525 = load float* %390, align 8
  %526 = load float* %391, align 8
  %527 = load float* %392, align 8
  %528 = load float* %393, align 8
  %529 = load float* %394, align 8
  %530 = load float* %395, align 8
  %531 = load float* %396, align 8
  %532 = load float* %397, align 8
  %temp.vect422 = insertelement <16 x float> undef, float %517, i32 0
  %temp.vect423 = insertelement <16 x float> %temp.vect422, float %518, i32 1
  %temp.vect424 = insertelement <16 x float> %temp.vect423, float %519, i32 2
  %temp.vect425 = insertelement <16 x float> %temp.vect424, float %520, i32 3
  %temp.vect426 = insertelement <16 x float> %temp.vect425, float %521, i32 4
  %temp.vect427 = insertelement <16 x float> %temp.vect426, float %522, i32 5
  %temp.vect428 = insertelement <16 x float> %temp.vect427, float %523, i32 6
  %temp.vect429 = insertelement <16 x float> %temp.vect428, float %524, i32 7
  %temp.vect430 = insertelement <16 x float> %temp.vect429, float %525, i32 8
  %temp.vect431 = insertelement <16 x float> %temp.vect430, float %526, i32 9
  %temp.vect432 = insertelement <16 x float> %temp.vect431, float %527, i32 10
  %temp.vect433 = insertelement <16 x float> %temp.vect432, float %528, i32 11
  %temp.vect434 = insertelement <16 x float> %temp.vect433, float %529, i32 12
  %temp.vect435 = insertelement <16 x float> %temp.vect434, float %530, i32 13
  %temp.vect436 = insertelement <16 x float> %temp.vect435, float %531, i32 14
  %temp.vect437 = insertelement <16 x float> %temp.vect436, float %532, i32 15
  %533 = fmul <16 x float> %vector421, %temp.vect437
  %534 = fadd <16 x float> %515, %533
  %extract438 = extractelement <16 x float> %534, i32 0
  %extract439 = extractelement <16 x float> %534, i32 1
  %extract440 = extractelement <16 x float> %534, i32 2
  %extract441 = extractelement <16 x float> %534, i32 3
  %extract442 = extractelement <16 x float> %534, i32 4
  %extract443 = extractelement <16 x float> %534, i32 5
  %extract444 = extractelement <16 x float> %534, i32 6
  %extract445 = extractelement <16 x float> %534, i32 7
  %extract446 = extractelement <16 x float> %534, i32 8
  %extract447 = extractelement <16 x float> %534, i32 9
  %extract448 = extractelement <16 x float> %534, i32 10
  %extract449 = extractelement <16 x float> %534, i32 11
  %extract450 = extractelement <16 x float> %534, i32 12
  %extract451 = extractelement <16 x float> %534, i32 13
  %extract452 = extractelement <16 x float> %534, i32 14
  %extract453 = extractelement <16 x float> %534, i32 15
  store float %extract438, float addrspace(1)* %334, align 4
  store float %extract439, float addrspace(1)* %335, align 4
  store float %extract440, float addrspace(1)* %336, align 4
  store float %extract441, float addrspace(1)* %337, align 4
  store float %extract442, float addrspace(1)* %338, align 4
  store float %extract443, float addrspace(1)* %339, align 4
  store float %extract444, float addrspace(1)* %340, align 4
  store float %extract445, float addrspace(1)* %341, align 4
  store float %extract446, float addrspace(1)* %342, align 4
  store float %extract447, float addrspace(1)* %343, align 4
  store float %extract448, float addrspace(1)* %344, align 4
  store float %extract449, float addrspace(1)* %345, align 4
  store float %extract450, float addrspace(1)* %346, align 4
  store float %extract451, float addrspace(1)* %347, align 4
  store float %extract452, float addrspace(1)* %348, align 4
  store float %extract453, float addrspace(1)* %349, align 4
  %535 = load float addrspace(1)* %scevgep.3, align 4
  %temp454 = insertelement <16 x float> undef, float %535, i32 0
  %vector455 = shufflevector <16 x float> %temp454, <16 x float> undef, <16 x i32> zeroinitializer
  %536 = load float* %398, align 4
  %537 = load float* %399, align 4
  %538 = load float* %400, align 4
  %539 = load float* %401, align 4
  %540 = load float* %402, align 4
  %541 = load float* %403, align 4
  %542 = load float* %404, align 4
  %543 = load float* %405, align 4
  %544 = load float* %406, align 4
  %545 = load float* %407, align 4
  %546 = load float* %408, align 4
  %547 = load float* %409, align 4
  %548 = load float* %410, align 4
  %549 = load float* %411, align 4
  %550 = load float* %412, align 4
  %551 = load float* %413, align 4
  %temp.vect456 = insertelement <16 x float> undef, float %536, i32 0
  %temp.vect457 = insertelement <16 x float> %temp.vect456, float %537, i32 1
  %temp.vect458 = insertelement <16 x float> %temp.vect457, float %538, i32 2
  %temp.vect459 = insertelement <16 x float> %temp.vect458, float %539, i32 3
  %temp.vect460 = insertelement <16 x float> %temp.vect459, float %540, i32 4
  %temp.vect461 = insertelement <16 x float> %temp.vect460, float %541, i32 5
  %temp.vect462 = insertelement <16 x float> %temp.vect461, float %542, i32 6
  %temp.vect463 = insertelement <16 x float> %temp.vect462, float %543, i32 7
  %temp.vect464 = insertelement <16 x float> %temp.vect463, float %544, i32 8
  %temp.vect465 = insertelement <16 x float> %temp.vect464, float %545, i32 9
  %temp.vect466 = insertelement <16 x float> %temp.vect465, float %546, i32 10
  %temp.vect467 = insertelement <16 x float> %temp.vect466, float %547, i32 11
  %temp.vect468 = insertelement <16 x float> %temp.vect467, float %548, i32 12
  %temp.vect469 = insertelement <16 x float> %temp.vect468, float %549, i32 13
  %temp.vect470 = insertelement <16 x float> %temp.vect469, float %550, i32 14
  %temp.vect471 = insertelement <16 x float> %temp.vect470, float %551, i32 15
  %552 = fmul <16 x float> %vector455, %temp.vect471
  %553 = fadd <16 x float> %534, %552
  %extract472 = extractelement <16 x float> %553, i32 0
  %extract473 = extractelement <16 x float> %553, i32 1
  %extract474 = extractelement <16 x float> %553, i32 2
  %extract475 = extractelement <16 x float> %553, i32 3
  %extract476 = extractelement <16 x float> %553, i32 4
  %extract477 = extractelement <16 x float> %553, i32 5
  %extract478 = extractelement <16 x float> %553, i32 6
  %extract479 = extractelement <16 x float> %553, i32 7
  %extract480 = extractelement <16 x float> %553, i32 8
  %extract481 = extractelement <16 x float> %553, i32 9
  %extract482 = extractelement <16 x float> %553, i32 10
  %extract483 = extractelement <16 x float> %553, i32 11
  %extract484 = extractelement <16 x float> %553, i32 12
  %extract485 = extractelement <16 x float> %553, i32 13
  %extract486 = extractelement <16 x float> %553, i32 14
  %extract487 = extractelement <16 x float> %553, i32 15
  store float %extract472, float addrspace(1)* %334, align 4
  store float %extract473, float addrspace(1)* %335, align 4
  store float %extract474, float addrspace(1)* %336, align 4
  store float %extract475, float addrspace(1)* %337, align 4
  store float %extract476, float addrspace(1)* %338, align 4
  store float %extract477, float addrspace(1)* %339, align 4
  store float %extract478, float addrspace(1)* %340, align 4
  store float %extract479, float addrspace(1)* %341, align 4
  store float %extract480, float addrspace(1)* %342, align 4
  store float %extract481, float addrspace(1)* %343, align 4
  store float %extract482, float addrspace(1)* %344, align 4
  store float %extract483, float addrspace(1)* %345, align 4
  store float %extract484, float addrspace(1)* %346, align 4
  store float %extract485, float addrspace(1)* %347, align 4
  store float %extract486, float addrspace(1)* %348, align 4
  store float %extract487, float addrspace(1)* %349, align 4
  %554 = load float addrspace(1)* %scevgep.4, align 4
  %temp488 = insertelement <16 x float> undef, float %554, i32 0
  %vector489 = shufflevector <16 x float> %temp488, <16 x float> undef, <16 x i32> zeroinitializer
  %555 = load float* %414, align 16
  %556 = load float* %415, align 16
  %557 = load float* %416, align 16
  %558 = load float* %417, align 16
  %559 = load float* %418, align 16
  %560 = load float* %419, align 16
  %561 = load float* %420, align 16
  %562 = load float* %421, align 16
  %563 = load float* %422, align 16
  %564 = load float* %423, align 16
  %565 = load float* %424, align 16
  %566 = load float* %425, align 16
  %567 = load float* %426, align 16
  %568 = load float* %427, align 16
  %569 = load float* %428, align 16
  %570 = load float* %429, align 16
  %temp.vect490 = insertelement <16 x float> undef, float %555, i32 0
  %temp.vect491 = insertelement <16 x float> %temp.vect490, float %556, i32 1
  %temp.vect492 = insertelement <16 x float> %temp.vect491, float %557, i32 2
  %temp.vect493 = insertelement <16 x float> %temp.vect492, float %558, i32 3
  %temp.vect494 = insertelement <16 x float> %temp.vect493, float %559, i32 4
  %temp.vect495 = insertelement <16 x float> %temp.vect494, float %560, i32 5
  %temp.vect496 = insertelement <16 x float> %temp.vect495, float %561, i32 6
  %temp.vect497 = insertelement <16 x float> %temp.vect496, float %562, i32 7
  %temp.vect498 = insertelement <16 x float> %temp.vect497, float %563, i32 8
  %temp.vect499 = insertelement <16 x float> %temp.vect498, float %564, i32 9
  %temp.vect500 = insertelement <16 x float> %temp.vect499, float %565, i32 10
  %temp.vect501 = insertelement <16 x float> %temp.vect500, float %566, i32 11
  %temp.vect502 = insertelement <16 x float> %temp.vect501, float %567, i32 12
  %temp.vect503 = insertelement <16 x float> %temp.vect502, float %568, i32 13
  %temp.vect504 = insertelement <16 x float> %temp.vect503, float %569, i32 14
  %temp.vect505 = insertelement <16 x float> %temp.vect504, float %570, i32 15
  %571 = fmul <16 x float> %vector489, %temp.vect505
  %572 = fadd <16 x float> %553, %571
  %extract506 = extractelement <16 x float> %572, i32 0
  %extract507 = extractelement <16 x float> %572, i32 1
  %extract508 = extractelement <16 x float> %572, i32 2
  %extract509 = extractelement <16 x float> %572, i32 3
  %extract510 = extractelement <16 x float> %572, i32 4
  %extract511 = extractelement <16 x float> %572, i32 5
  %extract512 = extractelement <16 x float> %572, i32 6
  %extract513 = extractelement <16 x float> %572, i32 7
  %extract514 = extractelement <16 x float> %572, i32 8
  %extract515 = extractelement <16 x float> %572, i32 9
  %extract516 = extractelement <16 x float> %572, i32 10
  %extract517 = extractelement <16 x float> %572, i32 11
  %extract518 = extractelement <16 x float> %572, i32 12
  %extract519 = extractelement <16 x float> %572, i32 13
  %extract520 = extractelement <16 x float> %572, i32 14
  %extract521 = extractelement <16 x float> %572, i32 15
  store float %extract506, float addrspace(1)* %334, align 4
  store float %extract507, float addrspace(1)* %335, align 4
  store float %extract508, float addrspace(1)* %336, align 4
  store float %extract509, float addrspace(1)* %337, align 4
  store float %extract510, float addrspace(1)* %338, align 4
  store float %extract511, float addrspace(1)* %339, align 4
  store float %extract512, float addrspace(1)* %340, align 4
  store float %extract513, float addrspace(1)* %341, align 4
  store float %extract514, float addrspace(1)* %342, align 4
  store float %extract515, float addrspace(1)* %343, align 4
  store float %extract516, float addrspace(1)* %344, align 4
  store float %extract517, float addrspace(1)* %345, align 4
  store float %extract518, float addrspace(1)* %346, align 4
  store float %extract519, float addrspace(1)* %347, align 4
  store float %extract520, float addrspace(1)* %348, align 4
  store float %extract521, float addrspace(1)* %349, align 4
  %573 = load float addrspace(1)* %scevgep.5, align 4
  %temp522 = insertelement <16 x float> undef, float %573, i32 0
  %vector523 = shufflevector <16 x float> %temp522, <16 x float> undef, <16 x i32> zeroinitializer
  %574 = load float* %430, align 4
  %575 = load float* %431, align 4
  %576 = load float* %432, align 4
  %577 = load float* %433, align 4
  %578 = load float* %434, align 4
  %579 = load float* %435, align 4
  %580 = load float* %436, align 4
  %581 = load float* %437, align 4
  %582 = load float* %438, align 4
  %583 = load float* %439, align 4
  %584 = load float* %440, align 4
  %585 = load float* %441, align 4
  %586 = load float* %442, align 4
  %587 = load float* %443, align 4
  %588 = load float* %444, align 4
  %589 = load float* %445, align 4
  %temp.vect524 = insertelement <16 x float> undef, float %574, i32 0
  %temp.vect525 = insertelement <16 x float> %temp.vect524, float %575, i32 1
  %temp.vect526 = insertelement <16 x float> %temp.vect525, float %576, i32 2
  %temp.vect527 = insertelement <16 x float> %temp.vect526, float %577, i32 3
  %temp.vect528 = insertelement <16 x float> %temp.vect527, float %578, i32 4
  %temp.vect529 = insertelement <16 x float> %temp.vect528, float %579, i32 5
  %temp.vect530 = insertelement <16 x float> %temp.vect529, float %580, i32 6
  %temp.vect531 = insertelement <16 x float> %temp.vect530, float %581, i32 7
  %temp.vect532 = insertelement <16 x float> %temp.vect531, float %582, i32 8
  %temp.vect533 = insertelement <16 x float> %temp.vect532, float %583, i32 9
  %temp.vect534 = insertelement <16 x float> %temp.vect533, float %584, i32 10
  %temp.vect535 = insertelement <16 x float> %temp.vect534, float %585, i32 11
  %temp.vect536 = insertelement <16 x float> %temp.vect535, float %586, i32 12
  %temp.vect537 = insertelement <16 x float> %temp.vect536, float %587, i32 13
  %temp.vect538 = insertelement <16 x float> %temp.vect537, float %588, i32 14
  %temp.vect539 = insertelement <16 x float> %temp.vect538, float %589, i32 15
  %590 = fmul <16 x float> %vector523, %temp.vect539
  %591 = fadd <16 x float> %572, %590
  %extract540 = extractelement <16 x float> %591, i32 0
  %extract541 = extractelement <16 x float> %591, i32 1
  %extract542 = extractelement <16 x float> %591, i32 2
  %extract543 = extractelement <16 x float> %591, i32 3
  %extract544 = extractelement <16 x float> %591, i32 4
  %extract545 = extractelement <16 x float> %591, i32 5
  %extract546 = extractelement <16 x float> %591, i32 6
  %extract547 = extractelement <16 x float> %591, i32 7
  %extract548 = extractelement <16 x float> %591, i32 8
  %extract549 = extractelement <16 x float> %591, i32 9
  %extract550 = extractelement <16 x float> %591, i32 10
  %extract551 = extractelement <16 x float> %591, i32 11
  %extract552 = extractelement <16 x float> %591, i32 12
  %extract553 = extractelement <16 x float> %591, i32 13
  %extract554 = extractelement <16 x float> %591, i32 14
  %extract555 = extractelement <16 x float> %591, i32 15
  store float %extract540, float addrspace(1)* %334, align 4
  store float %extract541, float addrspace(1)* %335, align 4
  store float %extract542, float addrspace(1)* %336, align 4
  store float %extract543, float addrspace(1)* %337, align 4
  store float %extract544, float addrspace(1)* %338, align 4
  store float %extract545, float addrspace(1)* %339, align 4
  store float %extract546, float addrspace(1)* %340, align 4
  store float %extract547, float addrspace(1)* %341, align 4
  store float %extract548, float addrspace(1)* %342, align 4
  store float %extract549, float addrspace(1)* %343, align 4
  store float %extract550, float addrspace(1)* %344, align 4
  store float %extract551, float addrspace(1)* %345, align 4
  store float %extract552, float addrspace(1)* %346, align 4
  store float %extract553, float addrspace(1)* %347, align 4
  store float %extract554, float addrspace(1)* %348, align 4
  store float %extract555, float addrspace(1)* %349, align 4
  %592 = load float addrspace(1)* %scevgep.6, align 4
  %temp556 = insertelement <16 x float> undef, float %592, i32 0
  %vector557 = shufflevector <16 x float> %temp556, <16 x float> undef, <16 x i32> zeroinitializer
  %593 = load float* %446, align 8
  %594 = load float* %447, align 8
  %595 = load float* %448, align 8
  %596 = load float* %449, align 8
  %597 = load float* %450, align 8
  %598 = load float* %451, align 8
  %599 = load float* %452, align 8
  %600 = load float* %453, align 8
  %601 = load float* %454, align 8
  %602 = load float* %455, align 8
  %603 = load float* %456, align 8
  %604 = load float* %457, align 8
  %605 = load float* %458, align 8
  %606 = load float* %459, align 8
  %607 = load float* %460, align 8
  %608 = load float* %461, align 8
  %temp.vect558 = insertelement <16 x float> undef, float %593, i32 0
  %temp.vect559 = insertelement <16 x float> %temp.vect558, float %594, i32 1
  %temp.vect560 = insertelement <16 x float> %temp.vect559, float %595, i32 2
  %temp.vect561 = insertelement <16 x float> %temp.vect560, float %596, i32 3
  %temp.vect562 = insertelement <16 x float> %temp.vect561, float %597, i32 4
  %temp.vect563 = insertelement <16 x float> %temp.vect562, float %598, i32 5
  %temp.vect564 = insertelement <16 x float> %temp.vect563, float %599, i32 6
  %temp.vect565 = insertelement <16 x float> %temp.vect564, float %600, i32 7
  %temp.vect566 = insertelement <16 x float> %temp.vect565, float %601, i32 8
  %temp.vect567 = insertelement <16 x float> %temp.vect566, float %602, i32 9
  %temp.vect568 = insertelement <16 x float> %temp.vect567, float %603, i32 10
  %temp.vect569 = insertelement <16 x float> %temp.vect568, float %604, i32 11
  %temp.vect570 = insertelement <16 x float> %temp.vect569, float %605, i32 12
  %temp.vect571 = insertelement <16 x float> %temp.vect570, float %606, i32 13
  %temp.vect572 = insertelement <16 x float> %temp.vect571, float %607, i32 14
  %temp.vect573 = insertelement <16 x float> %temp.vect572, float %608, i32 15
  %609 = fmul <16 x float> %vector557, %temp.vect573
  %610 = fadd <16 x float> %591, %609
  %extract574 = extractelement <16 x float> %610, i32 0
  %extract575 = extractelement <16 x float> %610, i32 1
  %extract576 = extractelement <16 x float> %610, i32 2
  %extract577 = extractelement <16 x float> %610, i32 3
  %extract578 = extractelement <16 x float> %610, i32 4
  %extract579 = extractelement <16 x float> %610, i32 5
  %extract580 = extractelement <16 x float> %610, i32 6
  %extract581 = extractelement <16 x float> %610, i32 7
  %extract582 = extractelement <16 x float> %610, i32 8
  %extract583 = extractelement <16 x float> %610, i32 9
  %extract584 = extractelement <16 x float> %610, i32 10
  %extract585 = extractelement <16 x float> %610, i32 11
  %extract586 = extractelement <16 x float> %610, i32 12
  %extract587 = extractelement <16 x float> %610, i32 13
  %extract588 = extractelement <16 x float> %610, i32 14
  %extract589 = extractelement <16 x float> %610, i32 15
  store float %extract574, float addrspace(1)* %334, align 4
  store float %extract575, float addrspace(1)* %335, align 4
  store float %extract576, float addrspace(1)* %336, align 4
  store float %extract577, float addrspace(1)* %337, align 4
  store float %extract578, float addrspace(1)* %338, align 4
  store float %extract579, float addrspace(1)* %339, align 4
  store float %extract580, float addrspace(1)* %340, align 4
  store float %extract581, float addrspace(1)* %341, align 4
  store float %extract582, float addrspace(1)* %342, align 4
  store float %extract583, float addrspace(1)* %343, align 4
  store float %extract584, float addrspace(1)* %344, align 4
  store float %extract585, float addrspace(1)* %345, align 4
  store float %extract586, float addrspace(1)* %346, align 4
  store float %extract587, float addrspace(1)* %347, align 4
  store float %extract588, float addrspace(1)* %348, align 4
  store float %extract589, float addrspace(1)* %349, align 4
  %611 = load float addrspace(1)* %scevgep.7, align 4
  %temp590 = insertelement <16 x float> undef, float %611, i32 0
  %vector591 = shufflevector <16 x float> %temp590, <16 x float> undef, <16 x i32> zeroinitializer
  %612 = load float* %462, align 4
  %613 = load float* %463, align 4
  %614 = load float* %464, align 4
  %615 = load float* %465, align 4
  %616 = load float* %466, align 4
  %617 = load float* %467, align 4
  %618 = load float* %468, align 4
  %619 = load float* %469, align 4
  %620 = load float* %470, align 4
  %621 = load float* %471, align 4
  %622 = load float* %472, align 4
  %623 = load float* %473, align 4
  %624 = load float* %474, align 4
  %625 = load float* %475, align 4
  %626 = load float* %476, align 4
  %627 = load float* %477, align 4
  %temp.vect592 = insertelement <16 x float> undef, float %612, i32 0
  %temp.vect593 = insertelement <16 x float> %temp.vect592, float %613, i32 1
  %temp.vect594 = insertelement <16 x float> %temp.vect593, float %614, i32 2
  %temp.vect595 = insertelement <16 x float> %temp.vect594, float %615, i32 3
  %temp.vect596 = insertelement <16 x float> %temp.vect595, float %616, i32 4
  %temp.vect597 = insertelement <16 x float> %temp.vect596, float %617, i32 5
  %temp.vect598 = insertelement <16 x float> %temp.vect597, float %618, i32 6
  %temp.vect599 = insertelement <16 x float> %temp.vect598, float %619, i32 7
  %temp.vect600 = insertelement <16 x float> %temp.vect599, float %620, i32 8
  %temp.vect601 = insertelement <16 x float> %temp.vect600, float %621, i32 9
  %temp.vect602 = insertelement <16 x float> %temp.vect601, float %622, i32 10
  %temp.vect603 = insertelement <16 x float> %temp.vect602, float %623, i32 11
  %temp.vect604 = insertelement <16 x float> %temp.vect603, float %624, i32 12
  %temp.vect605 = insertelement <16 x float> %temp.vect604, float %625, i32 13
  %temp.vect606 = insertelement <16 x float> %temp.vect605, float %626, i32 14
  %temp.vect607 = insertelement <16 x float> %temp.vect606, float %627, i32 15
  %628 = fmul <16 x float> %vector591, %temp.vect607
  %629 = fadd <16 x float> %610, %628
  %extract608 = extractelement <16 x float> %629, i32 0
  %extract609 = extractelement <16 x float> %629, i32 1
  %extract610 = extractelement <16 x float> %629, i32 2
  %extract611 = extractelement <16 x float> %629, i32 3
  %extract612 = extractelement <16 x float> %629, i32 4
  %extract613 = extractelement <16 x float> %629, i32 5
  %extract614 = extractelement <16 x float> %629, i32 6
  %extract615 = extractelement <16 x float> %629, i32 7
  %extract616 = extractelement <16 x float> %629, i32 8
  %extract617 = extractelement <16 x float> %629, i32 9
  %extract618 = extractelement <16 x float> %629, i32 10
  %extract619 = extractelement <16 x float> %629, i32 11
  %extract620 = extractelement <16 x float> %629, i32 12
  %extract621 = extractelement <16 x float> %629, i32 13
  %extract622 = extractelement <16 x float> %629, i32 14
  %extract623 = extractelement <16 x float> %629, i32 15
  store float %extract608, float addrspace(1)* %334, align 4
  store float %extract609, float addrspace(1)* %335, align 4
  store float %extract610, float addrspace(1)* %336, align 4
  store float %extract611, float addrspace(1)* %337, align 4
  store float %extract612, float addrspace(1)* %338, align 4
  store float %extract613, float addrspace(1)* %339, align 4
  store float %extract614, float addrspace(1)* %340, align 4
  store float %extract615, float addrspace(1)* %341, align 4
  store float %extract616, float addrspace(1)* %342, align 4
  store float %extract617, float addrspace(1)* %343, align 4
  store float %extract618, float addrspace(1)* %344, align 4
  store float %extract619, float addrspace(1)* %345, align 4
  store float %extract620, float addrspace(1)* %346, align 4
  store float %extract621, float addrspace(1)* %347, align 4
  store float %extract622, float addrspace(1)* %348, align 4
  store float %extract623, float addrspace(1)* %349, align 4
  %indvar.next33 = add i64 %indvar32, 1
  %exitcond = icmp eq i64 %indvar.next33, 8
  br i1 %exitcond, label %._crit_edge8, label %332

._crit_edge8:                                     ; preds = %332
  %indvar.next29 = add i64 %indvar28, 1
  %exitcond64 = icmp eq i64 %indvar.next29, 8
  br i1 %exitcond64, label %._crit_edge13, label %bb.nph7

._crit_edge13:                                    ; preds = %._crit_edge8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1260

thenBB:                                           ; preds = %._crit_edge13
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 13056
  br label %SyncBB

SyncBB1260:                                       ; preds = %._crit_edge13
  ret void
}

define void @____Vectorized_.DCT_CPU_VECTOR_separated_args(float addrspace(1)* nocapture %output, <8 x float> addrspace(1)* nocapture %input, <8 x float> addrspace(1)* nocapture %dct, i32 %width, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph18:
  %tmp61 = lshr i32 %width, 3
  %tmp62 = zext i32 %tmp61 to i64
  %tmp33 = zext i32 %width to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph18
  %CurrWI..0 = phi i64 [ 0, %bb.nph18 ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph18 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %broadcast1 = insertelement <16 x i64> undef, i64 %4, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %5 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %6 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %7 = load i64* %6, align 8
  %8 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = add i64 %7, %9
  %tmp64 = trunc i64 %10 to i32
  %tmp65 = mul i32 %tmp64, %width
  %tmp66 = and i32 %tmp65, 536870911
  %temp = insertelement <16 x i32> undef, i32 %tmp66, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp6832 = trunc <16 x i64> %5 to <16 x i32>
  %tmp6933 = add <16 x i32> %vector, %tmp6832
  %tmp7034 = zext <16 x i32> %tmp6933 to <16 x i64>
  %"&(pSB[currWI].offset)401" = add nuw i64 %CurrSBIndex..0, 4864
  %"&pSB[currWI].offset402" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)401"
  %CastToValueType403 = bitcast i8* %"&pSB[currWI].offset402" to [64 x float]*
  %"&(pSB[currWI].offset)409" = add nuw i64 %CurrSBIndex..0, 5120
  %"&pSB[currWI].offset410" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)409"
  %CastToValueType411 = bitcast i8* %"&pSB[currWI].offset410" to [64 x float]*
  %"&(pSB[currWI].offset)417" = add nuw i64 %CurrSBIndex..0, 5376
  %"&pSB[currWI].offset418" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)417"
  %CastToValueType419 = bitcast i8* %"&pSB[currWI].offset418" to [64 x float]*
  %"&(pSB[currWI].offset)425" = add nuw i64 %CurrSBIndex..0, 5632
  %"&pSB[currWI].offset426" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)425"
  %CastToValueType427 = bitcast i8* %"&pSB[currWI].offset426" to [64 x float]*
  %"&(pSB[currWI].offset)433" = add nuw i64 %CurrSBIndex..0, 5888
  %"&pSB[currWI].offset434" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)433"
  %CastToValueType435 = bitcast i8* %"&pSB[currWI].offset434" to [64 x float]*
  %"&(pSB[currWI].offset)441" = add nuw i64 %CurrSBIndex..0, 6144
  %"&pSB[currWI].offset442" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)441"
  %CastToValueType443 = bitcast i8* %"&pSB[currWI].offset442" to [64 x float]*
  %"&(pSB[currWI].offset)449" = add nuw i64 %CurrSBIndex..0, 6400
  %"&pSB[currWI].offset450" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)449"
  %CastToValueType451 = bitcast i8* %"&pSB[currWI].offset450" to [64 x float]*
  %"&(pSB[currWI].offset)457" = add nuw i64 %CurrSBIndex..0, 6656
  %"&pSB[currWI].offset458" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)457"
  %CastToValueType459 = bitcast i8* %"&pSB[currWI].offset458" to [64 x float]*
  %"&(pSB[currWI].offset)465" = add nuw i64 %CurrSBIndex..0, 6912
  %"&pSB[currWI].offset466" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)465"
  %CastToValueType467 = bitcast i8* %"&pSB[currWI].offset466" to [64 x float]*
  %"&(pSB[currWI].offset)473" = add nuw i64 %CurrSBIndex..0, 7168
  %"&pSB[currWI].offset474" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)473"
  %CastToValueType475 = bitcast i8* %"&pSB[currWI].offset474" to [64 x float]*
  %"&(pSB[currWI].offset)481" = add nuw i64 %CurrSBIndex..0, 7424
  %"&pSB[currWI].offset482" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)481"
  %CastToValueType483 = bitcast i8* %"&pSB[currWI].offset482" to [64 x float]*
  %"&(pSB[currWI].offset)489" = add nuw i64 %CurrSBIndex..0, 7680
  %"&pSB[currWI].offset490" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)489"
  %CastToValueType491 = bitcast i8* %"&pSB[currWI].offset490" to [64 x float]*
  %"&(pSB[currWI].offset)497" = add nuw i64 %CurrSBIndex..0, 7936
  %"&pSB[currWI].offset498" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)497"
  %CastToValueType499 = bitcast i8* %"&pSB[currWI].offset498" to [64 x float]*
  %"&(pSB[currWI].offset)505" = add nuw i64 %CurrSBIndex..0, 8192
  %"&pSB[currWI].offset506" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)505"
  %CastToValueType507 = bitcast i8* %"&pSB[currWI].offset506" to [64 x float]*
  %"&(pSB[currWI].offset)513" = add nuw i64 %CurrSBIndex..0, 8448
  %"&pSB[currWI].offset514" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)513"
  %CastToValueType515 = bitcast i8* %"&pSB[currWI].offset514" to [64 x float]*
  %"&(pSB[currWI].offset)521" = add nuw i64 %CurrSBIndex..0, 8704
  %"&pSB[currWI].offset522" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)521"
  %CastToValueType523 = bitcast i8* %"&pSB[currWI].offset522" to [64 x float]*
  br label %bb.nph14

bb.nph14:                                         ; preds = %._crit_edge15, %SyncBB
  %indvar52 = phi i64 [ 0, %SyncBB ], [ %indvar.next53, %._crit_edge15 ]
  %tmp63 = mul i64 %tmp62, %indvar52
  %temp35 = insertelement <16 x i64> undef, i64 %tmp63, i32 0
  %vector36 = shufflevector <16 x i64> %temp35, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp7137 = add <16 x i64> %tmp7034, %vector36
  %11 = and <16 x i64> %tmp7137, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract = extractelement <16 x i64> %11, i32 0
  %extract38 = extractelement <16 x i64> %11, i32 1
  %extract39 = extractelement <16 x i64> %11, i32 2
  %extract40 = extractelement <16 x i64> %11, i32 3
  %extract41 = extractelement <16 x i64> %11, i32 4
  %extract42 = extractelement <16 x i64> %11, i32 5
  %extract43 = extractelement <16 x i64> %11, i32 6
  %extract44 = extractelement <16 x i64> %11, i32 7
  %extract45 = extractelement <16 x i64> %11, i32 8
  %extract46 = extractelement <16 x i64> %11, i32 9
  %extract47 = extractelement <16 x i64> %11, i32 10
  %extract48 = extractelement <16 x i64> %11, i32 11
  %extract49 = extractelement <16 x i64> %11, i32 12
  %extract50 = extractelement <16 x i64> %11, i32 13
  %extract51 = extractelement <16 x i64> %11, i32 14
  %extract52 = extractelement <16 x i64> %11, i32 15
  %12 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract
  %13 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract38
  %14 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract39
  %15 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract40
  %16 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract41
  %17 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract42
  %18 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract43
  %19 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract44
  %20 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract45
  %21 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract46
  %22 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract47
  %23 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract48
  %24 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract49
  %25 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract50
  %26 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract51
  %27 = getelementptr inbounds <8 x float> addrspace(1)* %input, i64 %extract52
  %28 = load <8 x float> addrspace(1)* %12, align 32
  %29 = load <8 x float> addrspace(1)* %13, align 32
  %30 = load <8 x float> addrspace(1)* %14, align 32
  %31 = load <8 x float> addrspace(1)* %15, align 32
  %32 = load <8 x float> addrspace(1)* %16, align 32
  %33 = load <8 x float> addrspace(1)* %17, align 32
  %34 = load <8 x float> addrspace(1)* %18, align 32
  %35 = load <8 x float> addrspace(1)* %19, align 32
  %36 = load <8 x float> addrspace(1)* %20, align 32
  %37 = load <8 x float> addrspace(1)* %21, align 32
  %38 = load <8 x float> addrspace(1)* %22, align 32
  %39 = load <8 x float> addrspace(1)* %23, align 32
  %40 = load <8 x float> addrspace(1)* %24, align 32
  %41 = load <8 x float> addrspace(1)* %25, align 32
  %42 = load <8 x float> addrspace(1)* %26, align 32
  %43 = load <8 x float> addrspace(1)* %27, align 32
  %44 = extractelement <8 x float> %28, i32 0
  %45 = extractelement <8 x float> %29, i32 0
  %46 = extractelement <8 x float> %30, i32 0
  %47 = extractelement <8 x float> %31, i32 0
  %48 = extractelement <8 x float> %32, i32 0
  %49 = extractelement <8 x float> %33, i32 0
  %50 = extractelement <8 x float> %34, i32 0
  %51 = extractelement <8 x float> %35, i32 0
  %52 = extractelement <8 x float> %36, i32 0
  %53 = extractelement <8 x float> %37, i32 0
  %54 = extractelement <8 x float> %38, i32 0
  %55 = extractelement <8 x float> %39, i32 0
  %56 = extractelement <8 x float> %40, i32 0
  %57 = extractelement <8 x float> %41, i32 0
  %58 = extractelement <8 x float> %42, i32 0
  %59 = extractelement <8 x float> %43, i32 0
  %temp.vect = insertelement <16 x float> undef, float %44, i32 0
  %temp.vect55 = insertelement <16 x float> %temp.vect, float %45, i32 1
  %temp.vect56 = insertelement <16 x float> %temp.vect55, float %46, i32 2
  %temp.vect57 = insertelement <16 x float> %temp.vect56, float %47, i32 3
  %temp.vect58 = insertelement <16 x float> %temp.vect57, float %48, i32 4
  %temp.vect59 = insertelement <16 x float> %temp.vect58, float %49, i32 5
  %temp.vect60 = insertelement <16 x float> %temp.vect59, float %50, i32 6
  %temp.vect61 = insertelement <16 x float> %temp.vect60, float %51, i32 7
  %temp.vect62 = insertelement <16 x float> %temp.vect61, float %52, i32 8
  %temp.vect63 = insertelement <16 x float> %temp.vect62, float %53, i32 9
  %temp.vect64 = insertelement <16 x float> %temp.vect63, float %54, i32 10
  %temp.vect65 = insertelement <16 x float> %temp.vect64, float %55, i32 11
  %temp.vect66 = insertelement <16 x float> %temp.vect65, float %56, i32 12
  %temp.vect67 = insertelement <16 x float> %temp.vect66, float %57, i32 13
  %temp.vect68 = insertelement <16 x float> %temp.vect67, float %58, i32 14
  %temp.vect69 = insertelement <16 x float> %temp.vect68, float %59, i32 15
  %60 = extractelement <8 x float> %28, i32 1
  %61 = extractelement <8 x float> %29, i32 1
  %62 = extractelement <8 x float> %30, i32 1
  %63 = extractelement <8 x float> %31, i32 1
  %64 = extractelement <8 x float> %32, i32 1
  %65 = extractelement <8 x float> %33, i32 1
  %66 = extractelement <8 x float> %34, i32 1
  %67 = extractelement <8 x float> %35, i32 1
  %68 = extractelement <8 x float> %36, i32 1
  %69 = extractelement <8 x float> %37, i32 1
  %70 = extractelement <8 x float> %38, i32 1
  %71 = extractelement <8 x float> %39, i32 1
  %72 = extractelement <8 x float> %40, i32 1
  %73 = extractelement <8 x float> %41, i32 1
  %74 = extractelement <8 x float> %42, i32 1
  %75 = extractelement <8 x float> %43, i32 1
  %temp.vect72 = insertelement <16 x float> undef, float %60, i32 0
  %temp.vect73 = insertelement <16 x float> %temp.vect72, float %61, i32 1
  %temp.vect74 = insertelement <16 x float> %temp.vect73, float %62, i32 2
  %temp.vect75 = insertelement <16 x float> %temp.vect74, float %63, i32 3
  %temp.vect76 = insertelement <16 x float> %temp.vect75, float %64, i32 4
  %temp.vect77 = insertelement <16 x float> %temp.vect76, float %65, i32 5
  %temp.vect78 = insertelement <16 x float> %temp.vect77, float %66, i32 6
  %temp.vect79 = insertelement <16 x float> %temp.vect78, float %67, i32 7
  %temp.vect80 = insertelement <16 x float> %temp.vect79, float %68, i32 8
  %temp.vect81 = insertelement <16 x float> %temp.vect80, float %69, i32 9
  %temp.vect82 = insertelement <16 x float> %temp.vect81, float %70, i32 10
  %temp.vect83 = insertelement <16 x float> %temp.vect82, float %71, i32 11
  %temp.vect84 = insertelement <16 x float> %temp.vect83, float %72, i32 12
  %temp.vect85 = insertelement <16 x float> %temp.vect84, float %73, i32 13
  %temp.vect86 = insertelement <16 x float> %temp.vect85, float %74, i32 14
  %temp.vect87 = insertelement <16 x float> %temp.vect86, float %75, i32 15
  %76 = extractelement <8 x float> %28, i32 2
  %77 = extractelement <8 x float> %29, i32 2
  %78 = extractelement <8 x float> %30, i32 2
  %79 = extractelement <8 x float> %31, i32 2
  %80 = extractelement <8 x float> %32, i32 2
  %81 = extractelement <8 x float> %33, i32 2
  %82 = extractelement <8 x float> %34, i32 2
  %83 = extractelement <8 x float> %35, i32 2
  %84 = extractelement <8 x float> %36, i32 2
  %85 = extractelement <8 x float> %37, i32 2
  %86 = extractelement <8 x float> %38, i32 2
  %87 = extractelement <8 x float> %39, i32 2
  %88 = extractelement <8 x float> %40, i32 2
  %89 = extractelement <8 x float> %41, i32 2
  %90 = extractelement <8 x float> %42, i32 2
  %91 = extractelement <8 x float> %43, i32 2
  %temp.vect90 = insertelement <16 x float> undef, float %76, i32 0
  %temp.vect91 = insertelement <16 x float> %temp.vect90, float %77, i32 1
  %temp.vect92 = insertelement <16 x float> %temp.vect91, float %78, i32 2
  %temp.vect93 = insertelement <16 x float> %temp.vect92, float %79, i32 3
  %temp.vect94 = insertelement <16 x float> %temp.vect93, float %80, i32 4
  %temp.vect95 = insertelement <16 x float> %temp.vect94, float %81, i32 5
  %temp.vect96 = insertelement <16 x float> %temp.vect95, float %82, i32 6
  %temp.vect97 = insertelement <16 x float> %temp.vect96, float %83, i32 7
  %temp.vect98 = insertelement <16 x float> %temp.vect97, float %84, i32 8
  %temp.vect99 = insertelement <16 x float> %temp.vect98, float %85, i32 9
  %temp.vect100 = insertelement <16 x float> %temp.vect99, float %86, i32 10
  %temp.vect101 = insertelement <16 x float> %temp.vect100, float %87, i32 11
  %temp.vect102 = insertelement <16 x float> %temp.vect101, float %88, i32 12
  %temp.vect103 = insertelement <16 x float> %temp.vect102, float %89, i32 13
  %temp.vect104 = insertelement <16 x float> %temp.vect103, float %90, i32 14
  %temp.vect105 = insertelement <16 x float> %temp.vect104, float %91, i32 15
  %92 = extractelement <8 x float> %28, i32 3
  %93 = extractelement <8 x float> %29, i32 3
  %94 = extractelement <8 x float> %30, i32 3
  %95 = extractelement <8 x float> %31, i32 3
  %96 = extractelement <8 x float> %32, i32 3
  %97 = extractelement <8 x float> %33, i32 3
  %98 = extractelement <8 x float> %34, i32 3
  %99 = extractelement <8 x float> %35, i32 3
  %100 = extractelement <8 x float> %36, i32 3
  %101 = extractelement <8 x float> %37, i32 3
  %102 = extractelement <8 x float> %38, i32 3
  %103 = extractelement <8 x float> %39, i32 3
  %104 = extractelement <8 x float> %40, i32 3
  %105 = extractelement <8 x float> %41, i32 3
  %106 = extractelement <8 x float> %42, i32 3
  %107 = extractelement <8 x float> %43, i32 3
  %temp.vect108 = insertelement <16 x float> undef, float %92, i32 0
  %temp.vect109 = insertelement <16 x float> %temp.vect108, float %93, i32 1
  %temp.vect110 = insertelement <16 x float> %temp.vect109, float %94, i32 2
  %temp.vect111 = insertelement <16 x float> %temp.vect110, float %95, i32 3
  %temp.vect112 = insertelement <16 x float> %temp.vect111, float %96, i32 4
  %temp.vect113 = insertelement <16 x float> %temp.vect112, float %97, i32 5
  %temp.vect114 = insertelement <16 x float> %temp.vect113, float %98, i32 6
  %temp.vect115 = insertelement <16 x float> %temp.vect114, float %99, i32 7
  %temp.vect116 = insertelement <16 x float> %temp.vect115, float %100, i32 8
  %temp.vect117 = insertelement <16 x float> %temp.vect116, float %101, i32 9
  %temp.vect118 = insertelement <16 x float> %temp.vect117, float %102, i32 10
  %temp.vect119 = insertelement <16 x float> %temp.vect118, float %103, i32 11
  %temp.vect120 = insertelement <16 x float> %temp.vect119, float %104, i32 12
  %temp.vect121 = insertelement <16 x float> %temp.vect120, float %105, i32 13
  %temp.vect122 = insertelement <16 x float> %temp.vect121, float %106, i32 14
  %temp.vect123 = insertelement <16 x float> %temp.vect122, float %107, i32 15
  %108 = extractelement <8 x float> %28, i32 4
  %109 = extractelement <8 x float> %29, i32 4
  %110 = extractelement <8 x float> %30, i32 4
  %111 = extractelement <8 x float> %31, i32 4
  %112 = extractelement <8 x float> %32, i32 4
  %113 = extractelement <8 x float> %33, i32 4
  %114 = extractelement <8 x float> %34, i32 4
  %115 = extractelement <8 x float> %35, i32 4
  %116 = extractelement <8 x float> %36, i32 4
  %117 = extractelement <8 x float> %37, i32 4
  %118 = extractelement <8 x float> %38, i32 4
  %119 = extractelement <8 x float> %39, i32 4
  %120 = extractelement <8 x float> %40, i32 4
  %121 = extractelement <8 x float> %41, i32 4
  %122 = extractelement <8 x float> %42, i32 4
  %123 = extractelement <8 x float> %43, i32 4
  %temp.vect126 = insertelement <16 x float> undef, float %108, i32 0
  %temp.vect127 = insertelement <16 x float> %temp.vect126, float %109, i32 1
  %temp.vect128 = insertelement <16 x float> %temp.vect127, float %110, i32 2
  %temp.vect129 = insertelement <16 x float> %temp.vect128, float %111, i32 3
  %temp.vect130 = insertelement <16 x float> %temp.vect129, float %112, i32 4
  %temp.vect131 = insertelement <16 x float> %temp.vect130, float %113, i32 5
  %temp.vect132 = insertelement <16 x float> %temp.vect131, float %114, i32 6
  %temp.vect133 = insertelement <16 x float> %temp.vect132, float %115, i32 7
  %temp.vect134 = insertelement <16 x float> %temp.vect133, float %116, i32 8
  %temp.vect135 = insertelement <16 x float> %temp.vect134, float %117, i32 9
  %temp.vect136 = insertelement <16 x float> %temp.vect135, float %118, i32 10
  %temp.vect137 = insertelement <16 x float> %temp.vect136, float %119, i32 11
  %temp.vect138 = insertelement <16 x float> %temp.vect137, float %120, i32 12
  %temp.vect139 = insertelement <16 x float> %temp.vect138, float %121, i32 13
  %temp.vect140 = insertelement <16 x float> %temp.vect139, float %122, i32 14
  %temp.vect141 = insertelement <16 x float> %temp.vect140, float %123, i32 15
  %124 = extractelement <8 x float> %28, i32 5
  %125 = extractelement <8 x float> %29, i32 5
  %126 = extractelement <8 x float> %30, i32 5
  %127 = extractelement <8 x float> %31, i32 5
  %128 = extractelement <8 x float> %32, i32 5
  %129 = extractelement <8 x float> %33, i32 5
  %130 = extractelement <8 x float> %34, i32 5
  %131 = extractelement <8 x float> %35, i32 5
  %132 = extractelement <8 x float> %36, i32 5
  %133 = extractelement <8 x float> %37, i32 5
  %134 = extractelement <8 x float> %38, i32 5
  %135 = extractelement <8 x float> %39, i32 5
  %136 = extractelement <8 x float> %40, i32 5
  %137 = extractelement <8 x float> %41, i32 5
  %138 = extractelement <8 x float> %42, i32 5
  %139 = extractelement <8 x float> %43, i32 5
  %temp.vect144 = insertelement <16 x float> undef, float %124, i32 0
  %temp.vect145 = insertelement <16 x float> %temp.vect144, float %125, i32 1
  %temp.vect146 = insertelement <16 x float> %temp.vect145, float %126, i32 2
  %temp.vect147 = insertelement <16 x float> %temp.vect146, float %127, i32 3
  %temp.vect148 = insertelement <16 x float> %temp.vect147, float %128, i32 4
  %temp.vect149 = insertelement <16 x float> %temp.vect148, float %129, i32 5
  %temp.vect150 = insertelement <16 x float> %temp.vect149, float %130, i32 6
  %temp.vect151 = insertelement <16 x float> %temp.vect150, float %131, i32 7
  %temp.vect152 = insertelement <16 x float> %temp.vect151, float %132, i32 8
  %temp.vect153 = insertelement <16 x float> %temp.vect152, float %133, i32 9
  %temp.vect154 = insertelement <16 x float> %temp.vect153, float %134, i32 10
  %temp.vect155 = insertelement <16 x float> %temp.vect154, float %135, i32 11
  %temp.vect156 = insertelement <16 x float> %temp.vect155, float %136, i32 12
  %temp.vect157 = insertelement <16 x float> %temp.vect156, float %137, i32 13
  %temp.vect158 = insertelement <16 x float> %temp.vect157, float %138, i32 14
  %temp.vect159 = insertelement <16 x float> %temp.vect158, float %139, i32 15
  %140 = extractelement <8 x float> %28, i32 6
  %141 = extractelement <8 x float> %29, i32 6
  %142 = extractelement <8 x float> %30, i32 6
  %143 = extractelement <8 x float> %31, i32 6
  %144 = extractelement <8 x float> %32, i32 6
  %145 = extractelement <8 x float> %33, i32 6
  %146 = extractelement <8 x float> %34, i32 6
  %147 = extractelement <8 x float> %35, i32 6
  %148 = extractelement <8 x float> %36, i32 6
  %149 = extractelement <8 x float> %37, i32 6
  %150 = extractelement <8 x float> %38, i32 6
  %151 = extractelement <8 x float> %39, i32 6
  %152 = extractelement <8 x float> %40, i32 6
  %153 = extractelement <8 x float> %41, i32 6
  %154 = extractelement <8 x float> %42, i32 6
  %155 = extractelement <8 x float> %43, i32 6
  %temp.vect162 = insertelement <16 x float> undef, float %140, i32 0
  %temp.vect163 = insertelement <16 x float> %temp.vect162, float %141, i32 1
  %temp.vect164 = insertelement <16 x float> %temp.vect163, float %142, i32 2
  %temp.vect165 = insertelement <16 x float> %temp.vect164, float %143, i32 3
  %temp.vect166 = insertelement <16 x float> %temp.vect165, float %144, i32 4
  %temp.vect167 = insertelement <16 x float> %temp.vect166, float %145, i32 5
  %temp.vect168 = insertelement <16 x float> %temp.vect167, float %146, i32 6
  %temp.vect169 = insertelement <16 x float> %temp.vect168, float %147, i32 7
  %temp.vect170 = insertelement <16 x float> %temp.vect169, float %148, i32 8
  %temp.vect171 = insertelement <16 x float> %temp.vect170, float %149, i32 9
  %temp.vect172 = insertelement <16 x float> %temp.vect171, float %150, i32 10
  %temp.vect173 = insertelement <16 x float> %temp.vect172, float %151, i32 11
  %temp.vect174 = insertelement <16 x float> %temp.vect173, float %152, i32 12
  %temp.vect175 = insertelement <16 x float> %temp.vect174, float %153, i32 13
  %temp.vect176 = insertelement <16 x float> %temp.vect175, float %154, i32 14
  %temp.vect177 = insertelement <16 x float> %temp.vect176, float %155, i32 15
  %156 = extractelement <8 x float> %28, i32 7
  %157 = extractelement <8 x float> %29, i32 7
  %158 = extractelement <8 x float> %30, i32 7
  %159 = extractelement <8 x float> %31, i32 7
  %160 = extractelement <8 x float> %32, i32 7
  %161 = extractelement <8 x float> %33, i32 7
  %162 = extractelement <8 x float> %34, i32 7
  %163 = extractelement <8 x float> %35, i32 7
  %164 = extractelement <8 x float> %36, i32 7
  %165 = extractelement <8 x float> %37, i32 7
  %166 = extractelement <8 x float> %38, i32 7
  %167 = extractelement <8 x float> %39, i32 7
  %168 = extractelement <8 x float> %40, i32 7
  %169 = extractelement <8 x float> %41, i32 7
  %170 = extractelement <8 x float> %42, i32 7
  %171 = extractelement <8 x float> %43, i32 7
  %temp.vect180 = insertelement <16 x float> undef, float %156, i32 0
  %temp.vect181 = insertelement <16 x float> %temp.vect180, float %157, i32 1
  %temp.vect182 = insertelement <16 x float> %temp.vect181, float %158, i32 2
  %temp.vect183 = insertelement <16 x float> %temp.vect182, float %159, i32 3
  %temp.vect184 = insertelement <16 x float> %temp.vect183, float %160, i32 4
  %temp.vect185 = insertelement <16 x float> %temp.vect184, float %161, i32 5
  %temp.vect186 = insertelement <16 x float> %temp.vect185, float %162, i32 6
  %temp.vect187 = insertelement <16 x float> %temp.vect186, float %163, i32 7
  %temp.vect188 = insertelement <16 x float> %temp.vect187, float %164, i32 8
  %temp.vect189 = insertelement <16 x float> %temp.vect188, float %165, i32 9
  %temp.vect190 = insertelement <16 x float> %temp.vect189, float %166, i32 10
  %temp.vect191 = insertelement <16 x float> %temp.vect190, float %167, i32 11
  %temp.vect192 = insertelement <16 x float> %temp.vect191, float %168, i32 12
  %temp.vect193 = insertelement <16 x float> %temp.vect192, float %169, i32 13
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %170, i32 14
  %temp.vect195 = insertelement <16 x float> %temp.vect194, float %171, i32 15
  br label %172

; <label>:172                                     ; preds = %172, %bb.nph14
  %indvar49 = phi i64 [ 0, %bb.nph14 ], [ %indvar.next50, %172 ]
  %tmp58 = shl i64 %indvar49, 3
  %tmp59 = add i64 %indvar52, %tmp58
  %173 = getelementptr [64 x float]* %CastToValueType403, i64 0, i64 %tmp59
  %174 = getelementptr [64 x float]* %CastToValueType411, i64 0, i64 %tmp59
  %175 = getelementptr [64 x float]* %CastToValueType419, i64 0, i64 %tmp59
  %176 = getelementptr [64 x float]* %CastToValueType427, i64 0, i64 %tmp59
  %177 = getelementptr [64 x float]* %CastToValueType435, i64 0, i64 %tmp59
  %178 = getelementptr [64 x float]* %CastToValueType443, i64 0, i64 %tmp59
  %179 = getelementptr [64 x float]* %CastToValueType451, i64 0, i64 %tmp59
  %180 = getelementptr [64 x float]* %CastToValueType459, i64 0, i64 %tmp59
  %181 = getelementptr [64 x float]* %CastToValueType467, i64 0, i64 %tmp59
  %182 = getelementptr [64 x float]* %CastToValueType475, i64 0, i64 %tmp59
  %183 = getelementptr [64 x float]* %CastToValueType483, i64 0, i64 %tmp59
  %184 = getelementptr [64 x float]* %CastToValueType491, i64 0, i64 %tmp59
  %185 = getelementptr [64 x float]* %CastToValueType499, i64 0, i64 %tmp59
  %186 = getelementptr [64 x float]* %CastToValueType507, i64 0, i64 %tmp59
  %187 = getelementptr [64 x float]* %CastToValueType515, i64 0, i64 %tmp59
  %188 = getelementptr [64 x float]* %CastToValueType523, i64 0, i64 %tmp59
  %scevgep56 = getelementptr <8 x float> addrspace(1)* %dct, i64 %indvar49
  %189 = load <8 x float> addrspace(1)* %scevgep56, align 32
  %scalar = extractelement <8 x float> %189, i32 0
  %temp53 = insertelement <16 x float> undef, float %scalar, i32 0
  %vector54 = shufflevector <16 x float> %temp53, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar1 = extractelement <8 x float> %189, i32 1
  %temp70 = insertelement <16 x float> undef, float %scalar1, i32 0
  %vector71 = shufflevector <16 x float> %temp70, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar2 = extractelement <8 x float> %189, i32 2
  %temp88 = insertelement <16 x float> undef, float %scalar2, i32 0
  %vector89 = shufflevector <16 x float> %temp88, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar3 = extractelement <8 x float> %189, i32 3
  %temp106 = insertelement <16 x float> undef, float %scalar3, i32 0
  %vector107 = shufflevector <16 x float> %temp106, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar4 = extractelement <8 x float> %189, i32 4
  %temp124 = insertelement <16 x float> undef, float %scalar4, i32 0
  %vector125 = shufflevector <16 x float> %temp124, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar5 = extractelement <8 x float> %189, i32 5
  %temp142 = insertelement <16 x float> undef, float %scalar5, i32 0
  %vector143 = shufflevector <16 x float> %temp142, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar6 = extractelement <8 x float> %189, i32 6
  %temp160 = insertelement <16 x float> undef, float %scalar6, i32 0
  %vector161 = shufflevector <16 x float> %temp160, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar7 = extractelement <8 x float> %189, i32 7
  %temp178 = insertelement <16 x float> undef, float %scalar7, i32 0
  %vector179 = shufflevector <16 x float> %temp178, <16 x float> undef, <16 x i32> zeroinitializer
  %190 = fmul <16 x float> %vector54, %temp.vect69
  %191 = fmul <16 x float> %vector71, %temp.vect87
  %192 = fmul <16 x float> %vector89, %temp.vect105
  %193 = fmul <16 x float> %vector107, %temp.vect123
  %194 = fmul <16 x float> %vector125, %temp.vect141
  %195 = fmul <16 x float> %vector143, %temp.vect159
  %196 = fmul <16 x float> %vector161, %temp.vect177
  %197 = fmul <16 x float> %vector179, %temp.vect195
  %198 = fadd <16 x float> %190, %191
  %199 = fadd <16 x float> %198, %192
  %200 = fadd <16 x float> %199, %193
  %201 = fadd <16 x float> %200, %194
  %202 = fadd <16 x float> %201, %195
  %203 = fadd <16 x float> %202, %196
  %204 = fadd <16 x float> %203, %197
  %extract196 = extractelement <16 x float> %204, i32 0
  %extract197 = extractelement <16 x float> %204, i32 1
  %extract198 = extractelement <16 x float> %204, i32 2
  %extract199 = extractelement <16 x float> %204, i32 3
  %extract200 = extractelement <16 x float> %204, i32 4
  %extract201 = extractelement <16 x float> %204, i32 5
  %extract202 = extractelement <16 x float> %204, i32 6
  %extract203 = extractelement <16 x float> %204, i32 7
  %extract204 = extractelement <16 x float> %204, i32 8
  %extract205 = extractelement <16 x float> %204, i32 9
  %extract206 = extractelement <16 x float> %204, i32 10
  %extract207 = extractelement <16 x float> %204, i32 11
  %extract208 = extractelement <16 x float> %204, i32 12
  %extract209 = extractelement <16 x float> %204, i32 13
  %extract210 = extractelement <16 x float> %204, i32 14
  %extract211 = extractelement <16 x float> %204, i32 15
  store float %extract196, float* %173, align 4
  store float %extract197, float* %174, align 4
  store float %extract198, float* %175, align 4
  store float %extract199, float* %176, align 4
  store float %extract200, float* %177, align 4
  store float %extract201, float* %178, align 4
  store float %extract202, float* %179, align 4
  store float %extract203, float* %180, align 4
  store float %extract204, float* %181, align 4
  store float %extract205, float* %182, align 4
  store float %extract206, float* %183, align 4
  store float %extract207, float* %184, align 4
  store float %extract208, float* %185, align 4
  store float %extract209, float* %186, align 4
  store float %extract210, float* %187, align 4
  store float %extract211, float* %188, align 4
  %indvar.next50 = add i64 %indvar49, 1
  %exitcond51 = icmp eq i64 %indvar.next50, 8
  br i1 %exitcond51, label %._crit_edge15, label %172

._crit_edge15:                                    ; preds = %172
  %indvar.next53 = add i64 %indvar52, 1
  %exitcond57 = icmp eq i64 %indvar.next53, 8
  br i1 %exitcond57, label %bb.nph9, label %bb.nph14

bb.nph9:                                          ; preds = %._crit_edge15
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 4864
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %"&(pSB[currWI].offset)405" = add nuw i64 %CurrSBIndex..0, 5120
  %"&pSB[currWI].offset406" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)405"
  %"&(pSB[currWI].offset)413" = add nuw i64 %CurrSBIndex..0, 5376
  %"&pSB[currWI].offset414" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)413"
  %"&(pSB[currWI].offset)421" = add nuw i64 %CurrSBIndex..0, 5632
  %"&pSB[currWI].offset422" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)421"
  %"&(pSB[currWI].offset)429" = add nuw i64 %CurrSBIndex..0, 5888
  %"&pSB[currWI].offset430" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)429"
  %"&(pSB[currWI].offset)437" = add nuw i64 %CurrSBIndex..0, 6144
  %"&pSB[currWI].offset438" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)437"
  %"&(pSB[currWI].offset)445" = add nuw i64 %CurrSBIndex..0, 6400
  %"&pSB[currWI].offset446" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)445"
  %"&(pSB[currWI].offset)453" = add nuw i64 %CurrSBIndex..0, 6656
  %"&pSB[currWI].offset454" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)453"
  %"&(pSB[currWI].offset)461" = add nuw i64 %CurrSBIndex..0, 6912
  %"&pSB[currWI].offset462" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)461"
  %"&(pSB[currWI].offset)469" = add nuw i64 %CurrSBIndex..0, 7168
  %"&pSB[currWI].offset470" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)469"
  %"&(pSB[currWI].offset)477" = add nuw i64 %CurrSBIndex..0, 7424
  %"&pSB[currWI].offset478" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)477"
  %"&(pSB[currWI].offset)485" = add nuw i64 %CurrSBIndex..0, 7680
  %"&pSB[currWI].offset486" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)485"
  %"&(pSB[currWI].offset)493" = add nuw i64 %CurrSBIndex..0, 7936
  %"&pSB[currWI].offset494" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)493"
  %"&(pSB[currWI].offset)501" = add nuw i64 %CurrSBIndex..0, 8192
  %"&pSB[currWI].offset502" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)501"
  %"&(pSB[currWI].offset)509" = add nuw i64 %CurrSBIndex..0, 8448
  %"&pSB[currWI].offset510" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)509"
  %"&(pSB[currWI].offset)517" = add nuw i64 %CurrSBIndex..0, 8704
  %"&pSB[currWI].offset518" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)517"
  %"&(pSB[currWI].offset)529" = add nuw i64 %CurrSBIndex..0, 8960
  %"&pSB[currWI].offset530" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)529"
  %CastToValueType531 = bitcast i8* %"&pSB[currWI].offset530" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)537" = add nuw i64 %CurrSBIndex..0, 9216
  %"&pSB[currWI].offset538" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)537"
  %CastToValueType539 = bitcast i8* %"&pSB[currWI].offset538" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)545" = add nuw i64 %CurrSBIndex..0, 9472
  %"&pSB[currWI].offset546" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)545"
  %CastToValueType547 = bitcast i8* %"&pSB[currWI].offset546" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)553" = add nuw i64 %CurrSBIndex..0, 9728
  %"&pSB[currWI].offset554" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)553"
  %CastToValueType555 = bitcast i8* %"&pSB[currWI].offset554" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)561" = add nuw i64 %CurrSBIndex..0, 9984
  %"&pSB[currWI].offset562" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)561"
  %CastToValueType563 = bitcast i8* %"&pSB[currWI].offset562" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)569" = add nuw i64 %CurrSBIndex..0, 10240
  %"&pSB[currWI].offset570" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)569"
  %CastToValueType571 = bitcast i8* %"&pSB[currWI].offset570" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)577" = add nuw i64 %CurrSBIndex..0, 10496
  %"&pSB[currWI].offset578" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)577"
  %CastToValueType579 = bitcast i8* %"&pSB[currWI].offset578" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)585" = add nuw i64 %CurrSBIndex..0, 10752
  %"&pSB[currWI].offset586" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)585"
  %CastToValueType587 = bitcast i8* %"&pSB[currWI].offset586" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)593" = add nuw i64 %CurrSBIndex..0, 11008
  %"&pSB[currWI].offset594" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)593"
  %CastToValueType595 = bitcast i8* %"&pSB[currWI].offset594" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)601" = add nuw i64 %CurrSBIndex..0, 11264
  %"&pSB[currWI].offset602" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)601"
  %CastToValueType603 = bitcast i8* %"&pSB[currWI].offset602" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)609" = add nuw i64 %CurrSBIndex..0, 11520
  %"&pSB[currWI].offset610" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)609"
  %CastToValueType611 = bitcast i8* %"&pSB[currWI].offset610" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)617" = add nuw i64 %CurrSBIndex..0, 11776
  %"&pSB[currWI].offset618" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)617"
  %CastToValueType619 = bitcast i8* %"&pSB[currWI].offset618" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)625" = add nuw i64 %CurrSBIndex..0, 12032
  %"&pSB[currWI].offset626" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)625"
  %CastToValueType627 = bitcast i8* %"&pSB[currWI].offset626" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)633" = add nuw i64 %CurrSBIndex..0, 12288
  %"&pSB[currWI].offset634" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)633"
  %CastToValueType635 = bitcast i8* %"&pSB[currWI].offset634" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)641" = add nuw i64 %CurrSBIndex..0, 12544
  %"&pSB[currWI].offset642" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)641"
  %CastToValueType643 = bitcast i8* %"&pSB[currWI].offset642" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)649" = add nuw i64 %CurrSBIndex..0, 12800
  %"&pSB[currWI].offset650" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)649"
  %CastToValueType651 = bitcast i8* %"&pSB[currWI].offset650" to [8 x <8 x float>]*
  br label %205

; <label>:205                                     ; preds = %205, %bb.nph9
  %indvar45 = phi i64 [ 0, %bb.nph9 ], [ %indvar.next46, %205 ]
  %206 = getelementptr [8 x <8 x float>]* %CastToValueType531, i64 0, i64 %indvar45
  %207 = getelementptr [8 x <8 x float>]* %CastToValueType539, i64 0, i64 %indvar45
  %208 = getelementptr [8 x <8 x float>]* %CastToValueType547, i64 0, i64 %indvar45
  %209 = getelementptr [8 x <8 x float>]* %CastToValueType555, i64 0, i64 %indvar45
  %210 = getelementptr [8 x <8 x float>]* %CastToValueType563, i64 0, i64 %indvar45
  %211 = getelementptr [8 x <8 x float>]* %CastToValueType571, i64 0, i64 %indvar45
  %212 = getelementptr [8 x <8 x float>]* %CastToValueType579, i64 0, i64 %indvar45
  %213 = getelementptr [8 x <8 x float>]* %CastToValueType587, i64 0, i64 %indvar45
  %214 = getelementptr [8 x <8 x float>]* %CastToValueType595, i64 0, i64 %indvar45
  %215 = getelementptr [8 x <8 x float>]* %CastToValueType603, i64 0, i64 %indvar45
  %216 = getelementptr [8 x <8 x float>]* %CastToValueType611, i64 0, i64 %indvar45
  %217 = getelementptr [8 x <8 x float>]* %CastToValueType619, i64 0, i64 %indvar45
  %218 = getelementptr [8 x <8 x float>]* %CastToValueType627, i64 0, i64 %indvar45
  %219 = getelementptr [8 x <8 x float>]* %CastToValueType635, i64 0, i64 %indvar45
  %220 = getelementptr [8 x <8 x float>]* %CastToValueType643, i64 0, i64 %indvar45
  %221 = getelementptr [8 x <8 x float>]* %CastToValueType651, i64 0, i64 %indvar45
  %222 = bitcast i8* %"&pSB[currWI].offset" to i32*
  %223 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i = shl i64 %indvar45, 3
  %add.ptr26.i.i.i = getelementptr inbounds i32* %222, i64 %mul25.i.i.i
  %conv27.i.i.i = bitcast i32* %add.ptr26.i.i.i to i8*
  %224 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %223, i16 255, i8* %conv27.i.i.i, i32 0, i32 0) nounwind
  %225 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %224, i16 255, i8* %conv27.i.i.i, i32 0, i32 0) nounwind
  %tmp3.i = shufflevector <16 x float> %225, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %226 = bitcast i8* %"&pSB[currWI].offset406" to i32*
  %227 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i1 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i2 = getelementptr inbounds i32* %226, i64 %mul25.i.i.i1
  %conv27.i.i.i3 = bitcast i32* %add.ptr26.i.i.i2 to i8*
  %228 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %227, i16 255, i8* %conv27.i.i.i3, i32 0, i32 0) nounwind
  %229 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %228, i16 255, i8* %conv27.i.i.i3, i32 0, i32 0) nounwind
  %tmp3.i4 = shufflevector <16 x float> %229, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %230 = bitcast i8* %"&pSB[currWI].offset414" to i32*
  %231 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i5 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i6 = getelementptr inbounds i32* %230, i64 %mul25.i.i.i5
  %conv27.i.i.i7 = bitcast i32* %add.ptr26.i.i.i6 to i8*
  %232 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %231, i16 255, i8* %conv27.i.i.i7, i32 0, i32 0) nounwind
  %233 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %232, i16 255, i8* %conv27.i.i.i7, i32 0, i32 0) nounwind
  %tmp3.i8 = shufflevector <16 x float> %233, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %234 = bitcast i8* %"&pSB[currWI].offset422" to i32*
  %235 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i9 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i10 = getelementptr inbounds i32* %234, i64 %mul25.i.i.i9
  %conv27.i.i.i11 = bitcast i32* %add.ptr26.i.i.i10 to i8*
  %236 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %235, i16 255, i8* %conv27.i.i.i11, i32 0, i32 0) nounwind
  %237 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %236, i16 255, i8* %conv27.i.i.i11, i32 0, i32 0) nounwind
  %tmp3.i12 = shufflevector <16 x float> %237, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %238 = bitcast i8* %"&pSB[currWI].offset430" to i32*
  %239 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i13 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i14 = getelementptr inbounds i32* %238, i64 %mul25.i.i.i13
  %conv27.i.i.i15 = bitcast i32* %add.ptr26.i.i.i14 to i8*
  %240 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %239, i16 255, i8* %conv27.i.i.i15, i32 0, i32 0) nounwind
  %241 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %240, i16 255, i8* %conv27.i.i.i15, i32 0, i32 0) nounwind
  %tmp3.i16 = shufflevector <16 x float> %241, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %242 = bitcast i8* %"&pSB[currWI].offset438" to i32*
  %243 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i17 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i18 = getelementptr inbounds i32* %242, i64 %mul25.i.i.i17
  %conv27.i.i.i19 = bitcast i32* %add.ptr26.i.i.i18 to i8*
  %244 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %243, i16 255, i8* %conv27.i.i.i19, i32 0, i32 0) nounwind
  %245 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %244, i16 255, i8* %conv27.i.i.i19, i32 0, i32 0) nounwind
  %tmp3.i20 = shufflevector <16 x float> %245, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %246 = bitcast i8* %"&pSB[currWI].offset446" to i32*
  %247 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i21 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i22 = getelementptr inbounds i32* %246, i64 %mul25.i.i.i21
  %conv27.i.i.i23 = bitcast i32* %add.ptr26.i.i.i22 to i8*
  %248 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %247, i16 255, i8* %conv27.i.i.i23, i32 0, i32 0) nounwind
  %249 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %248, i16 255, i8* %conv27.i.i.i23, i32 0, i32 0) nounwind
  %tmp3.i24 = shufflevector <16 x float> %249, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %250 = bitcast i8* %"&pSB[currWI].offset454" to i32*
  %251 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i25 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i26 = getelementptr inbounds i32* %250, i64 %mul25.i.i.i25
  %conv27.i.i.i27 = bitcast i32* %add.ptr26.i.i.i26 to i8*
  %252 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %251, i16 255, i8* %conv27.i.i.i27, i32 0, i32 0) nounwind
  %253 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %252, i16 255, i8* %conv27.i.i.i27, i32 0, i32 0) nounwind
  %tmp3.i28 = shufflevector <16 x float> %253, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %254 = bitcast i8* %"&pSB[currWI].offset462" to i32*
  %255 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i29 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i30 = getelementptr inbounds i32* %254, i64 %mul25.i.i.i29
  %conv27.i.i.i31 = bitcast i32* %add.ptr26.i.i.i30 to i8*
  %256 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %255, i16 255, i8* %conv27.i.i.i31, i32 0, i32 0) nounwind
  %257 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %256, i16 255, i8* %conv27.i.i.i31, i32 0, i32 0) nounwind
  %tmp3.i32 = shufflevector <16 x float> %257, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %258 = bitcast i8* %"&pSB[currWI].offset470" to i32*
  %259 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i33 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i34 = getelementptr inbounds i32* %258, i64 %mul25.i.i.i33
  %conv27.i.i.i35 = bitcast i32* %add.ptr26.i.i.i34 to i8*
  %260 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %259, i16 255, i8* %conv27.i.i.i35, i32 0, i32 0) nounwind
  %261 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %260, i16 255, i8* %conv27.i.i.i35, i32 0, i32 0) nounwind
  %tmp3.i36 = shufflevector <16 x float> %261, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %262 = bitcast i8* %"&pSB[currWI].offset478" to i32*
  %263 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i37 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i38 = getelementptr inbounds i32* %262, i64 %mul25.i.i.i37
  %conv27.i.i.i39 = bitcast i32* %add.ptr26.i.i.i38 to i8*
  %264 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %263, i16 255, i8* %conv27.i.i.i39, i32 0, i32 0) nounwind
  %265 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %264, i16 255, i8* %conv27.i.i.i39, i32 0, i32 0) nounwind
  %tmp3.i40 = shufflevector <16 x float> %265, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %266 = bitcast i8* %"&pSB[currWI].offset486" to i32*
  %267 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i41 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i42 = getelementptr inbounds i32* %266, i64 %mul25.i.i.i41
  %conv27.i.i.i43 = bitcast i32* %add.ptr26.i.i.i42 to i8*
  %268 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %267, i16 255, i8* %conv27.i.i.i43, i32 0, i32 0) nounwind
  %269 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %268, i16 255, i8* %conv27.i.i.i43, i32 0, i32 0) nounwind
  %tmp3.i44 = shufflevector <16 x float> %269, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %270 = bitcast i8* %"&pSB[currWI].offset494" to i32*
  %271 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i45 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i46 = getelementptr inbounds i32* %270, i64 %mul25.i.i.i45
  %conv27.i.i.i47 = bitcast i32* %add.ptr26.i.i.i46 to i8*
  %272 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %271, i16 255, i8* %conv27.i.i.i47, i32 0, i32 0) nounwind
  %273 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %272, i16 255, i8* %conv27.i.i.i47, i32 0, i32 0) nounwind
  %tmp3.i48 = shufflevector <16 x float> %273, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %274 = bitcast i8* %"&pSB[currWI].offset502" to i32*
  %275 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i49 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i50 = getelementptr inbounds i32* %274, i64 %mul25.i.i.i49
  %conv27.i.i.i51 = bitcast i32* %add.ptr26.i.i.i50 to i8*
  %276 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %275, i16 255, i8* %conv27.i.i.i51, i32 0, i32 0) nounwind
  %277 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %276, i16 255, i8* %conv27.i.i.i51, i32 0, i32 0) nounwind
  %tmp3.i52 = shufflevector <16 x float> %277, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %278 = bitcast i8* %"&pSB[currWI].offset510" to i32*
  %279 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i53 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i54 = getelementptr inbounds i32* %278, i64 %mul25.i.i.i53
  %conv27.i.i.i55 = bitcast i32* %add.ptr26.i.i.i54 to i8*
  %280 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %279, i16 255, i8* %conv27.i.i.i55, i32 0, i32 0) nounwind
  %281 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %280, i16 255, i8* %conv27.i.i.i55, i32 0, i32 0) nounwind
  %tmp3.i56 = shufflevector <16 x float> %281, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %282 = bitcast i8* %"&pSB[currWI].offset518" to i32*
  %283 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i57 = shl i64 %indvar45, 3
  %add.ptr26.i.i.i58 = getelementptr inbounds i32* %282, i64 %mul25.i.i.i57
  %conv27.i.i.i59 = bitcast i32* %add.ptr26.i.i.i58 to i8*
  %284 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %283, i16 255, i8* %conv27.i.i.i59, i32 0, i32 0) nounwind
  %285 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %284, i16 255, i8* %conv27.i.i.i59, i32 0, i32 0) nounwind
  %tmp3.i60 = shufflevector <16 x float> %285, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x float> %tmp3.i, <8 x float>* %206, align 32
  store <8 x float> %tmp3.i4, <8 x float>* %207, align 32
  store <8 x float> %tmp3.i8, <8 x float>* %208, align 32
  store <8 x float> %tmp3.i12, <8 x float>* %209, align 32
  store <8 x float> %tmp3.i16, <8 x float>* %210, align 32
  store <8 x float> %tmp3.i20, <8 x float>* %211, align 32
  store <8 x float> %tmp3.i24, <8 x float>* %212, align 32
  store <8 x float> %tmp3.i28, <8 x float>* %213, align 32
  store <8 x float> %tmp3.i32, <8 x float>* %214, align 32
  store <8 x float> %tmp3.i36, <8 x float>* %215, align 32
  store <8 x float> %tmp3.i40, <8 x float>* %216, align 32
  store <8 x float> %tmp3.i44, <8 x float>* %217, align 32
  store <8 x float> %tmp3.i48, <8 x float>* %218, align 32
  store <8 x float> %tmp3.i52, <8 x float>* %219, align 32
  store <8 x float> %tmp3.i56, <8 x float>* %220, align 32
  store <8 x float> %tmp3.i60, <8 x float>* %221, align 32
  %indvar.next46 = add i64 %indvar45, 1
  %exitcond47 = icmp eq i64 %indvar.next46, 8
  br i1 %exitcond47, label %bb.nph5, label %205

bb.nph5:                                          ; preds = %205
  %tmp35 = trunc i64 %10 to i32
  %tmp36 = mul i32 %tmp35, %width
  %temp213 = insertelement <16 x i32> undef, i32 %tmp36, i32 0
  %vector214 = shufflevector <16 x i32> %temp213, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp37212 = trunc <16 x i64> %5 to <16 x i32>
  %tmp38215 = add <16 x i32> %vector214, %tmp37212
  %tmp39216 = shl <16 x i32> %tmp38215, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp40217 = zext <16 x i32> %tmp39216 to <16 x i64>
  %"&(pSB[currWI].offset)525" = add nuw i64 %CurrSBIndex..0, 8960
  %"&pSB[currWI].offset526" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)525"
  %CastToValueType527 = bitcast i8* %"&pSB[currWI].offset526" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)533" = add nuw i64 %CurrSBIndex..0, 9216
  %"&pSB[currWI].offset534" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)533"
  %CastToValueType535 = bitcast i8* %"&pSB[currWI].offset534" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)541" = add nuw i64 %CurrSBIndex..0, 9472
  %"&pSB[currWI].offset542" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)541"
  %CastToValueType543 = bitcast i8* %"&pSB[currWI].offset542" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)549" = add nuw i64 %CurrSBIndex..0, 9728
  %"&pSB[currWI].offset550" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)549"
  %CastToValueType551 = bitcast i8* %"&pSB[currWI].offset550" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)557" = add nuw i64 %CurrSBIndex..0, 9984
  %"&pSB[currWI].offset558" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)557"
  %CastToValueType559 = bitcast i8* %"&pSB[currWI].offset558" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)565" = add nuw i64 %CurrSBIndex..0, 10240
  %"&pSB[currWI].offset566" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)565"
  %CastToValueType567 = bitcast i8* %"&pSB[currWI].offset566" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)573" = add nuw i64 %CurrSBIndex..0, 10496
  %"&pSB[currWI].offset574" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)573"
  %CastToValueType575 = bitcast i8* %"&pSB[currWI].offset574" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)581" = add nuw i64 %CurrSBIndex..0, 10752
  %"&pSB[currWI].offset582" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)581"
  %CastToValueType583 = bitcast i8* %"&pSB[currWI].offset582" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)589" = add nuw i64 %CurrSBIndex..0, 11008
  %"&pSB[currWI].offset590" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)589"
  %CastToValueType591 = bitcast i8* %"&pSB[currWI].offset590" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)597" = add nuw i64 %CurrSBIndex..0, 11264
  %"&pSB[currWI].offset598" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)597"
  %CastToValueType599 = bitcast i8* %"&pSB[currWI].offset598" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)605" = add nuw i64 %CurrSBIndex..0, 11520
  %"&pSB[currWI].offset606" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)605"
  %CastToValueType607 = bitcast i8* %"&pSB[currWI].offset606" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)613" = add nuw i64 %CurrSBIndex..0, 11776
  %"&pSB[currWI].offset614" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)613"
  %CastToValueType615 = bitcast i8* %"&pSB[currWI].offset614" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)621" = add nuw i64 %CurrSBIndex..0, 12032
  %"&pSB[currWI].offset622" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)621"
  %CastToValueType623 = bitcast i8* %"&pSB[currWI].offset622" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)629" = add nuw i64 %CurrSBIndex..0, 12288
  %"&pSB[currWI].offset630" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)629"
  %CastToValueType631 = bitcast i8* %"&pSB[currWI].offset630" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)637" = add nuw i64 %CurrSBIndex..0, 12544
  %"&pSB[currWI].offset638" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)637"
  %CastToValueType639 = bitcast i8* %"&pSB[currWI].offset638" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)645" = add nuw i64 %CurrSBIndex..0, 12800
  %"&pSB[currWI].offset646" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)645"
  %CastToValueType647 = bitcast i8* %"&pSB[currWI].offset646" to [8 x <8 x float>]*
  br label %bb.nph

bb.nph:                                           ; preds = %._crit_edge, %bb.nph5
  %indvar19 = phi i64 [ 0, %bb.nph5 ], [ %indvar.next20, %._crit_edge ]
  %tmp34 = mul i64 %tmp33, %indvar19
  %temp218 = insertelement <16 x i64> undef, i64 %tmp34, i32 0
  %vector219 = shufflevector <16 x i64> %temp218, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp41220 = add <16 x i64> %tmp40217, %vector219
  %scevgep44 = getelementptr <8 x float> addrspace(1)* %dct, i64 %indvar19
  br label %286

; <label>:286                                     ; preds = %286, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %286 ]
  %temp221 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector222 = shufflevector <16 x i64> %temp221, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp42223 = add <16 x i64> %tmp41220, %vector222
  %287 = getelementptr [8 x <8 x float>]* %CastToValueType527, i64 0, i64 %indvar
  %288 = getelementptr [8 x <8 x float>]* %CastToValueType535, i64 0, i64 %indvar
  %289 = getelementptr [8 x <8 x float>]* %CastToValueType543, i64 0, i64 %indvar
  %290 = getelementptr [8 x <8 x float>]* %CastToValueType551, i64 0, i64 %indvar
  %291 = getelementptr [8 x <8 x float>]* %CastToValueType559, i64 0, i64 %indvar
  %292 = getelementptr [8 x <8 x float>]* %CastToValueType567, i64 0, i64 %indvar
  %293 = getelementptr [8 x <8 x float>]* %CastToValueType575, i64 0, i64 %indvar
  %294 = getelementptr [8 x <8 x float>]* %CastToValueType583, i64 0, i64 %indvar
  %295 = getelementptr [8 x <8 x float>]* %CastToValueType591, i64 0, i64 %indvar
  %296 = getelementptr [8 x <8 x float>]* %CastToValueType599, i64 0, i64 %indvar
  %297 = getelementptr [8 x <8 x float>]* %CastToValueType607, i64 0, i64 %indvar
  %298 = getelementptr [8 x <8 x float>]* %CastToValueType615, i64 0, i64 %indvar
  %299 = getelementptr [8 x <8 x float>]* %CastToValueType623, i64 0, i64 %indvar
  %300 = getelementptr [8 x <8 x float>]* %CastToValueType631, i64 0, i64 %indvar
  %301 = getelementptr [8 x <8 x float>]* %CastToValueType639, i64 0, i64 %indvar
  %302 = getelementptr [8 x <8 x float>]* %CastToValueType647, i64 0, i64 %indvar
  %303 = load <8 x float> addrspace(1)* %scevgep44, align 32
  %scalar16 = extractelement <8 x float> %303, i32 0
  %temp224 = insertelement <16 x float> undef, float %scalar16, i32 0
  %vector225 = shufflevector <16 x float> %temp224, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar17 = extractelement <8 x float> %303, i32 1
  %temp242 = insertelement <16 x float> undef, float %scalar17, i32 0
  %vector243 = shufflevector <16 x float> %temp242, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar18 = extractelement <8 x float> %303, i32 2
  %temp260 = insertelement <16 x float> undef, float %scalar18, i32 0
  %vector261 = shufflevector <16 x float> %temp260, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar19 = extractelement <8 x float> %303, i32 3
  %temp278 = insertelement <16 x float> undef, float %scalar19, i32 0
  %vector279 = shufflevector <16 x float> %temp278, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar20 = extractelement <8 x float> %303, i32 4
  %temp296 = insertelement <16 x float> undef, float %scalar20, i32 0
  %vector297 = shufflevector <16 x float> %temp296, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar21 = extractelement <8 x float> %303, i32 5
  %temp314 = insertelement <16 x float> undef, float %scalar21, i32 0
  %vector315 = shufflevector <16 x float> %temp314, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar22 = extractelement <8 x float> %303, i32 6
  %temp332 = insertelement <16 x float> undef, float %scalar22, i32 0
  %vector333 = shufflevector <16 x float> %temp332, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar23 = extractelement <8 x float> %303, i32 7
  %temp350 = insertelement <16 x float> undef, float %scalar23, i32 0
  %vector351 = shufflevector <16 x float> %temp350, <16 x float> undef, <16 x i32> zeroinitializer
  %304 = load <8 x float>* %287, align 32
  %305 = load <8 x float>* %288, align 32
  %306 = load <8 x float>* %289, align 32
  %307 = load <8 x float>* %290, align 32
  %308 = load <8 x float>* %291, align 32
  %309 = load <8 x float>* %292, align 32
  %310 = load <8 x float>* %293, align 32
  %311 = load <8 x float>* %294, align 32
  %312 = load <8 x float>* %295, align 32
  %313 = load <8 x float>* %296, align 32
  %314 = load <8 x float>* %297, align 32
  %315 = load <8 x float>* %298, align 32
  %316 = load <8 x float>* %299, align 32
  %317 = load <8 x float>* %300, align 32
  %318 = load <8 x float>* %301, align 32
  %319 = load <8 x float>* %302, align 32
  %320 = extractelement <8 x float> %304, i32 0
  %321 = extractelement <8 x float> %305, i32 0
  %322 = extractelement <8 x float> %306, i32 0
  %323 = extractelement <8 x float> %307, i32 0
  %324 = extractelement <8 x float> %308, i32 0
  %325 = extractelement <8 x float> %309, i32 0
  %326 = extractelement <8 x float> %310, i32 0
  %327 = extractelement <8 x float> %311, i32 0
  %328 = extractelement <8 x float> %312, i32 0
  %329 = extractelement <8 x float> %313, i32 0
  %330 = extractelement <8 x float> %314, i32 0
  %331 = extractelement <8 x float> %315, i32 0
  %332 = extractelement <8 x float> %316, i32 0
  %333 = extractelement <8 x float> %317, i32 0
  %334 = extractelement <8 x float> %318, i32 0
  %335 = extractelement <8 x float> %319, i32 0
  %temp.vect226 = insertelement <16 x float> undef, float %320, i32 0
  %temp.vect227 = insertelement <16 x float> %temp.vect226, float %321, i32 1
  %temp.vect228 = insertelement <16 x float> %temp.vect227, float %322, i32 2
  %temp.vect229 = insertelement <16 x float> %temp.vect228, float %323, i32 3
  %temp.vect230 = insertelement <16 x float> %temp.vect229, float %324, i32 4
  %temp.vect231 = insertelement <16 x float> %temp.vect230, float %325, i32 5
  %temp.vect232 = insertelement <16 x float> %temp.vect231, float %326, i32 6
  %temp.vect233 = insertelement <16 x float> %temp.vect232, float %327, i32 7
  %temp.vect234 = insertelement <16 x float> %temp.vect233, float %328, i32 8
  %temp.vect235 = insertelement <16 x float> %temp.vect234, float %329, i32 9
  %temp.vect236 = insertelement <16 x float> %temp.vect235, float %330, i32 10
  %temp.vect237 = insertelement <16 x float> %temp.vect236, float %331, i32 11
  %temp.vect238 = insertelement <16 x float> %temp.vect237, float %332, i32 12
  %temp.vect239 = insertelement <16 x float> %temp.vect238, float %333, i32 13
  %temp.vect240 = insertelement <16 x float> %temp.vect239, float %334, i32 14
  %temp.vect241 = insertelement <16 x float> %temp.vect240, float %335, i32 15
  %336 = extractelement <8 x float> %304, i32 1
  %337 = extractelement <8 x float> %305, i32 1
  %338 = extractelement <8 x float> %306, i32 1
  %339 = extractelement <8 x float> %307, i32 1
  %340 = extractelement <8 x float> %308, i32 1
  %341 = extractelement <8 x float> %309, i32 1
  %342 = extractelement <8 x float> %310, i32 1
  %343 = extractelement <8 x float> %311, i32 1
  %344 = extractelement <8 x float> %312, i32 1
  %345 = extractelement <8 x float> %313, i32 1
  %346 = extractelement <8 x float> %314, i32 1
  %347 = extractelement <8 x float> %315, i32 1
  %348 = extractelement <8 x float> %316, i32 1
  %349 = extractelement <8 x float> %317, i32 1
  %350 = extractelement <8 x float> %318, i32 1
  %351 = extractelement <8 x float> %319, i32 1
  %temp.vect244 = insertelement <16 x float> undef, float %336, i32 0
  %temp.vect245 = insertelement <16 x float> %temp.vect244, float %337, i32 1
  %temp.vect246 = insertelement <16 x float> %temp.vect245, float %338, i32 2
  %temp.vect247 = insertelement <16 x float> %temp.vect246, float %339, i32 3
  %temp.vect248 = insertelement <16 x float> %temp.vect247, float %340, i32 4
  %temp.vect249 = insertelement <16 x float> %temp.vect248, float %341, i32 5
  %temp.vect250 = insertelement <16 x float> %temp.vect249, float %342, i32 6
  %temp.vect251 = insertelement <16 x float> %temp.vect250, float %343, i32 7
  %temp.vect252 = insertelement <16 x float> %temp.vect251, float %344, i32 8
  %temp.vect253 = insertelement <16 x float> %temp.vect252, float %345, i32 9
  %temp.vect254 = insertelement <16 x float> %temp.vect253, float %346, i32 10
  %temp.vect255 = insertelement <16 x float> %temp.vect254, float %347, i32 11
  %temp.vect256 = insertelement <16 x float> %temp.vect255, float %348, i32 12
  %temp.vect257 = insertelement <16 x float> %temp.vect256, float %349, i32 13
  %temp.vect258 = insertelement <16 x float> %temp.vect257, float %350, i32 14
  %temp.vect259 = insertelement <16 x float> %temp.vect258, float %351, i32 15
  %352 = extractelement <8 x float> %304, i32 2
  %353 = extractelement <8 x float> %305, i32 2
  %354 = extractelement <8 x float> %306, i32 2
  %355 = extractelement <8 x float> %307, i32 2
  %356 = extractelement <8 x float> %308, i32 2
  %357 = extractelement <8 x float> %309, i32 2
  %358 = extractelement <8 x float> %310, i32 2
  %359 = extractelement <8 x float> %311, i32 2
  %360 = extractelement <8 x float> %312, i32 2
  %361 = extractelement <8 x float> %313, i32 2
  %362 = extractelement <8 x float> %314, i32 2
  %363 = extractelement <8 x float> %315, i32 2
  %364 = extractelement <8 x float> %316, i32 2
  %365 = extractelement <8 x float> %317, i32 2
  %366 = extractelement <8 x float> %318, i32 2
  %367 = extractelement <8 x float> %319, i32 2
  %temp.vect262 = insertelement <16 x float> undef, float %352, i32 0
  %temp.vect263 = insertelement <16 x float> %temp.vect262, float %353, i32 1
  %temp.vect264 = insertelement <16 x float> %temp.vect263, float %354, i32 2
  %temp.vect265 = insertelement <16 x float> %temp.vect264, float %355, i32 3
  %temp.vect266 = insertelement <16 x float> %temp.vect265, float %356, i32 4
  %temp.vect267 = insertelement <16 x float> %temp.vect266, float %357, i32 5
  %temp.vect268 = insertelement <16 x float> %temp.vect267, float %358, i32 6
  %temp.vect269 = insertelement <16 x float> %temp.vect268, float %359, i32 7
  %temp.vect270 = insertelement <16 x float> %temp.vect269, float %360, i32 8
  %temp.vect271 = insertelement <16 x float> %temp.vect270, float %361, i32 9
  %temp.vect272 = insertelement <16 x float> %temp.vect271, float %362, i32 10
  %temp.vect273 = insertelement <16 x float> %temp.vect272, float %363, i32 11
  %temp.vect274 = insertelement <16 x float> %temp.vect273, float %364, i32 12
  %temp.vect275 = insertelement <16 x float> %temp.vect274, float %365, i32 13
  %temp.vect276 = insertelement <16 x float> %temp.vect275, float %366, i32 14
  %temp.vect277 = insertelement <16 x float> %temp.vect276, float %367, i32 15
  %368 = extractelement <8 x float> %304, i32 3
  %369 = extractelement <8 x float> %305, i32 3
  %370 = extractelement <8 x float> %306, i32 3
  %371 = extractelement <8 x float> %307, i32 3
  %372 = extractelement <8 x float> %308, i32 3
  %373 = extractelement <8 x float> %309, i32 3
  %374 = extractelement <8 x float> %310, i32 3
  %375 = extractelement <8 x float> %311, i32 3
  %376 = extractelement <8 x float> %312, i32 3
  %377 = extractelement <8 x float> %313, i32 3
  %378 = extractelement <8 x float> %314, i32 3
  %379 = extractelement <8 x float> %315, i32 3
  %380 = extractelement <8 x float> %316, i32 3
  %381 = extractelement <8 x float> %317, i32 3
  %382 = extractelement <8 x float> %318, i32 3
  %383 = extractelement <8 x float> %319, i32 3
  %temp.vect280 = insertelement <16 x float> undef, float %368, i32 0
  %temp.vect281 = insertelement <16 x float> %temp.vect280, float %369, i32 1
  %temp.vect282 = insertelement <16 x float> %temp.vect281, float %370, i32 2
  %temp.vect283 = insertelement <16 x float> %temp.vect282, float %371, i32 3
  %temp.vect284 = insertelement <16 x float> %temp.vect283, float %372, i32 4
  %temp.vect285 = insertelement <16 x float> %temp.vect284, float %373, i32 5
  %temp.vect286 = insertelement <16 x float> %temp.vect285, float %374, i32 6
  %temp.vect287 = insertelement <16 x float> %temp.vect286, float %375, i32 7
  %temp.vect288 = insertelement <16 x float> %temp.vect287, float %376, i32 8
  %temp.vect289 = insertelement <16 x float> %temp.vect288, float %377, i32 9
  %temp.vect290 = insertelement <16 x float> %temp.vect289, float %378, i32 10
  %temp.vect291 = insertelement <16 x float> %temp.vect290, float %379, i32 11
  %temp.vect292 = insertelement <16 x float> %temp.vect291, float %380, i32 12
  %temp.vect293 = insertelement <16 x float> %temp.vect292, float %381, i32 13
  %temp.vect294 = insertelement <16 x float> %temp.vect293, float %382, i32 14
  %temp.vect295 = insertelement <16 x float> %temp.vect294, float %383, i32 15
  %384 = extractelement <8 x float> %304, i32 4
  %385 = extractelement <8 x float> %305, i32 4
  %386 = extractelement <8 x float> %306, i32 4
  %387 = extractelement <8 x float> %307, i32 4
  %388 = extractelement <8 x float> %308, i32 4
  %389 = extractelement <8 x float> %309, i32 4
  %390 = extractelement <8 x float> %310, i32 4
  %391 = extractelement <8 x float> %311, i32 4
  %392 = extractelement <8 x float> %312, i32 4
  %393 = extractelement <8 x float> %313, i32 4
  %394 = extractelement <8 x float> %314, i32 4
  %395 = extractelement <8 x float> %315, i32 4
  %396 = extractelement <8 x float> %316, i32 4
  %397 = extractelement <8 x float> %317, i32 4
  %398 = extractelement <8 x float> %318, i32 4
  %399 = extractelement <8 x float> %319, i32 4
  %temp.vect298 = insertelement <16 x float> undef, float %384, i32 0
  %temp.vect299 = insertelement <16 x float> %temp.vect298, float %385, i32 1
  %temp.vect300 = insertelement <16 x float> %temp.vect299, float %386, i32 2
  %temp.vect301 = insertelement <16 x float> %temp.vect300, float %387, i32 3
  %temp.vect302 = insertelement <16 x float> %temp.vect301, float %388, i32 4
  %temp.vect303 = insertelement <16 x float> %temp.vect302, float %389, i32 5
  %temp.vect304 = insertelement <16 x float> %temp.vect303, float %390, i32 6
  %temp.vect305 = insertelement <16 x float> %temp.vect304, float %391, i32 7
  %temp.vect306 = insertelement <16 x float> %temp.vect305, float %392, i32 8
  %temp.vect307 = insertelement <16 x float> %temp.vect306, float %393, i32 9
  %temp.vect308 = insertelement <16 x float> %temp.vect307, float %394, i32 10
  %temp.vect309 = insertelement <16 x float> %temp.vect308, float %395, i32 11
  %temp.vect310 = insertelement <16 x float> %temp.vect309, float %396, i32 12
  %temp.vect311 = insertelement <16 x float> %temp.vect310, float %397, i32 13
  %temp.vect312 = insertelement <16 x float> %temp.vect311, float %398, i32 14
  %temp.vect313 = insertelement <16 x float> %temp.vect312, float %399, i32 15
  %400 = extractelement <8 x float> %304, i32 5
  %401 = extractelement <8 x float> %305, i32 5
  %402 = extractelement <8 x float> %306, i32 5
  %403 = extractelement <8 x float> %307, i32 5
  %404 = extractelement <8 x float> %308, i32 5
  %405 = extractelement <8 x float> %309, i32 5
  %406 = extractelement <8 x float> %310, i32 5
  %407 = extractelement <8 x float> %311, i32 5
  %408 = extractelement <8 x float> %312, i32 5
  %409 = extractelement <8 x float> %313, i32 5
  %410 = extractelement <8 x float> %314, i32 5
  %411 = extractelement <8 x float> %315, i32 5
  %412 = extractelement <8 x float> %316, i32 5
  %413 = extractelement <8 x float> %317, i32 5
  %414 = extractelement <8 x float> %318, i32 5
  %415 = extractelement <8 x float> %319, i32 5
  %temp.vect316 = insertelement <16 x float> undef, float %400, i32 0
  %temp.vect317 = insertelement <16 x float> %temp.vect316, float %401, i32 1
  %temp.vect318 = insertelement <16 x float> %temp.vect317, float %402, i32 2
  %temp.vect319 = insertelement <16 x float> %temp.vect318, float %403, i32 3
  %temp.vect320 = insertelement <16 x float> %temp.vect319, float %404, i32 4
  %temp.vect321 = insertelement <16 x float> %temp.vect320, float %405, i32 5
  %temp.vect322 = insertelement <16 x float> %temp.vect321, float %406, i32 6
  %temp.vect323 = insertelement <16 x float> %temp.vect322, float %407, i32 7
  %temp.vect324 = insertelement <16 x float> %temp.vect323, float %408, i32 8
  %temp.vect325 = insertelement <16 x float> %temp.vect324, float %409, i32 9
  %temp.vect326 = insertelement <16 x float> %temp.vect325, float %410, i32 10
  %temp.vect327 = insertelement <16 x float> %temp.vect326, float %411, i32 11
  %temp.vect328 = insertelement <16 x float> %temp.vect327, float %412, i32 12
  %temp.vect329 = insertelement <16 x float> %temp.vect328, float %413, i32 13
  %temp.vect330 = insertelement <16 x float> %temp.vect329, float %414, i32 14
  %temp.vect331 = insertelement <16 x float> %temp.vect330, float %415, i32 15
  %416 = extractelement <8 x float> %304, i32 6
  %417 = extractelement <8 x float> %305, i32 6
  %418 = extractelement <8 x float> %306, i32 6
  %419 = extractelement <8 x float> %307, i32 6
  %420 = extractelement <8 x float> %308, i32 6
  %421 = extractelement <8 x float> %309, i32 6
  %422 = extractelement <8 x float> %310, i32 6
  %423 = extractelement <8 x float> %311, i32 6
  %424 = extractelement <8 x float> %312, i32 6
  %425 = extractelement <8 x float> %313, i32 6
  %426 = extractelement <8 x float> %314, i32 6
  %427 = extractelement <8 x float> %315, i32 6
  %428 = extractelement <8 x float> %316, i32 6
  %429 = extractelement <8 x float> %317, i32 6
  %430 = extractelement <8 x float> %318, i32 6
  %431 = extractelement <8 x float> %319, i32 6
  %temp.vect334 = insertelement <16 x float> undef, float %416, i32 0
  %temp.vect335 = insertelement <16 x float> %temp.vect334, float %417, i32 1
  %temp.vect336 = insertelement <16 x float> %temp.vect335, float %418, i32 2
  %temp.vect337 = insertelement <16 x float> %temp.vect336, float %419, i32 3
  %temp.vect338 = insertelement <16 x float> %temp.vect337, float %420, i32 4
  %temp.vect339 = insertelement <16 x float> %temp.vect338, float %421, i32 5
  %temp.vect340 = insertelement <16 x float> %temp.vect339, float %422, i32 6
  %temp.vect341 = insertelement <16 x float> %temp.vect340, float %423, i32 7
  %temp.vect342 = insertelement <16 x float> %temp.vect341, float %424, i32 8
  %temp.vect343 = insertelement <16 x float> %temp.vect342, float %425, i32 9
  %temp.vect344 = insertelement <16 x float> %temp.vect343, float %426, i32 10
  %temp.vect345 = insertelement <16 x float> %temp.vect344, float %427, i32 11
  %temp.vect346 = insertelement <16 x float> %temp.vect345, float %428, i32 12
  %temp.vect347 = insertelement <16 x float> %temp.vect346, float %429, i32 13
  %temp.vect348 = insertelement <16 x float> %temp.vect347, float %430, i32 14
  %temp.vect349 = insertelement <16 x float> %temp.vect348, float %431, i32 15
  %432 = extractelement <8 x float> %304, i32 7
  %433 = extractelement <8 x float> %305, i32 7
  %434 = extractelement <8 x float> %306, i32 7
  %435 = extractelement <8 x float> %307, i32 7
  %436 = extractelement <8 x float> %308, i32 7
  %437 = extractelement <8 x float> %309, i32 7
  %438 = extractelement <8 x float> %310, i32 7
  %439 = extractelement <8 x float> %311, i32 7
  %440 = extractelement <8 x float> %312, i32 7
  %441 = extractelement <8 x float> %313, i32 7
  %442 = extractelement <8 x float> %314, i32 7
  %443 = extractelement <8 x float> %315, i32 7
  %444 = extractelement <8 x float> %316, i32 7
  %445 = extractelement <8 x float> %317, i32 7
  %446 = extractelement <8 x float> %318, i32 7
  %447 = extractelement <8 x float> %319, i32 7
  %temp.vect352 = insertelement <16 x float> undef, float %432, i32 0
  %temp.vect353 = insertelement <16 x float> %temp.vect352, float %433, i32 1
  %temp.vect354 = insertelement <16 x float> %temp.vect353, float %434, i32 2
  %temp.vect355 = insertelement <16 x float> %temp.vect354, float %435, i32 3
  %temp.vect356 = insertelement <16 x float> %temp.vect355, float %436, i32 4
  %temp.vect357 = insertelement <16 x float> %temp.vect356, float %437, i32 5
  %temp.vect358 = insertelement <16 x float> %temp.vect357, float %438, i32 6
  %temp.vect359 = insertelement <16 x float> %temp.vect358, float %439, i32 7
  %temp.vect360 = insertelement <16 x float> %temp.vect359, float %440, i32 8
  %temp.vect361 = insertelement <16 x float> %temp.vect360, float %441, i32 9
  %temp.vect362 = insertelement <16 x float> %temp.vect361, float %442, i32 10
  %temp.vect363 = insertelement <16 x float> %temp.vect362, float %443, i32 11
  %temp.vect364 = insertelement <16 x float> %temp.vect363, float %444, i32 12
  %temp.vect365 = insertelement <16 x float> %temp.vect364, float %445, i32 13
  %temp.vect366 = insertelement <16 x float> %temp.vect365, float %446, i32 14
  %temp.vect367 = insertelement <16 x float> %temp.vect366, float %447, i32 15
  %448 = fmul <16 x float> %vector225, %temp.vect241
  %449 = fmul <16 x float> %vector243, %temp.vect259
  %450 = fmul <16 x float> %vector261, %temp.vect277
  %451 = fmul <16 x float> %vector279, %temp.vect295
  %452 = fmul <16 x float> %vector297, %temp.vect313
  %453 = fmul <16 x float> %vector315, %temp.vect331
  %454 = fmul <16 x float> %vector333, %temp.vect349
  %455 = fmul <16 x float> %vector351, %temp.vect367
  %456 = fadd <16 x float> %448, %449
  %457 = fadd <16 x float> %456, %450
  %458 = fadd <16 x float> %457, %451
  %459 = fadd <16 x float> %458, %452
  %460 = fadd <16 x float> %459, %453
  %461 = fadd <16 x float> %460, %454
  %462 = fadd <16 x float> %461, %455
  %extract384 = extractelement <16 x float> %462, i32 0
  %extract385 = extractelement <16 x float> %462, i32 1
  %extract386 = extractelement <16 x float> %462, i32 2
  %extract387 = extractelement <16 x float> %462, i32 3
  %extract388 = extractelement <16 x float> %462, i32 4
  %extract389 = extractelement <16 x float> %462, i32 5
  %extract390 = extractelement <16 x float> %462, i32 6
  %extract391 = extractelement <16 x float> %462, i32 7
  %extract392 = extractelement <16 x float> %462, i32 8
  %extract393 = extractelement <16 x float> %462, i32 9
  %extract394 = extractelement <16 x float> %462, i32 10
  %extract395 = extractelement <16 x float> %462, i32 11
  %extract396 = extractelement <16 x float> %462, i32 12
  %extract397 = extractelement <16 x float> %462, i32 13
  %extract398 = extractelement <16 x float> %462, i32 14
  %extract399 = extractelement <16 x float> %462, i32 15
  %463 = and <16 x i64> %tmp42223, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract368 = extractelement <16 x i64> %463, i32 0
  %extract369 = extractelement <16 x i64> %463, i32 1
  %extract370 = extractelement <16 x i64> %463, i32 2
  %extract371 = extractelement <16 x i64> %463, i32 3
  %extract372 = extractelement <16 x i64> %463, i32 4
  %extract373 = extractelement <16 x i64> %463, i32 5
  %extract374 = extractelement <16 x i64> %463, i32 6
  %extract375 = extractelement <16 x i64> %463, i32 7
  %extract376 = extractelement <16 x i64> %463, i32 8
  %extract377 = extractelement <16 x i64> %463, i32 9
  %extract378 = extractelement <16 x i64> %463, i32 10
  %extract379 = extractelement <16 x i64> %463, i32 11
  %extract380 = extractelement <16 x i64> %463, i32 12
  %extract381 = extractelement <16 x i64> %463, i32 13
  %extract382 = extractelement <16 x i64> %463, i32 14
  %extract383 = extractelement <16 x i64> %463, i32 15
  %464 = getelementptr inbounds float addrspace(1)* %output, i64 %extract368
  %465 = getelementptr inbounds float addrspace(1)* %output, i64 %extract369
  %466 = getelementptr inbounds float addrspace(1)* %output, i64 %extract370
  %467 = getelementptr inbounds float addrspace(1)* %output, i64 %extract371
  %468 = getelementptr inbounds float addrspace(1)* %output, i64 %extract372
  %469 = getelementptr inbounds float addrspace(1)* %output, i64 %extract373
  %470 = getelementptr inbounds float addrspace(1)* %output, i64 %extract374
  %471 = getelementptr inbounds float addrspace(1)* %output, i64 %extract375
  %472 = getelementptr inbounds float addrspace(1)* %output, i64 %extract376
  %473 = getelementptr inbounds float addrspace(1)* %output, i64 %extract377
  %474 = getelementptr inbounds float addrspace(1)* %output, i64 %extract378
  %475 = getelementptr inbounds float addrspace(1)* %output, i64 %extract379
  %476 = getelementptr inbounds float addrspace(1)* %output, i64 %extract380
  %477 = getelementptr inbounds float addrspace(1)* %output, i64 %extract381
  %478 = getelementptr inbounds float addrspace(1)* %output, i64 %extract382
  %479 = getelementptr inbounds float addrspace(1)* %output, i64 %extract383
  store float %extract384, float addrspace(1)* %464, align 4
  store float %extract385, float addrspace(1)* %465, align 4
  store float %extract386, float addrspace(1)* %466, align 4
  store float %extract387, float addrspace(1)* %467, align 4
  store float %extract388, float addrspace(1)* %468, align 4
  store float %extract389, float addrspace(1)* %469, align 4
  store float %extract390, float addrspace(1)* %470, align 4
  store float %extract391, float addrspace(1)* %471, align 4
  store float %extract392, float addrspace(1)* %472, align 4
  store float %extract393, float addrspace(1)* %473, align 4
  store float %extract394, float addrspace(1)* %474, align 4
  store float %extract395, float addrspace(1)* %475, align 4
  store float %extract396, float addrspace(1)* %476, align 4
  store float %extract397, float addrspace(1)* %477, align 4
  store float %extract398, float addrspace(1)* %478, align 4
  store float %extract399, float addrspace(1)* %479, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 8
  br i1 %exitcond, label %._crit_edge, label %286

._crit_edge:                                      ; preds = %286
  %indvar.next20 = add i64 %indvar19, 1
  %exitcond32 = icmp eq i64 %indvar.next20, 8
  br i1 %exitcond32, label %._crit_edge6, label %bb.nph

._crit_edge6:                                     ; preds = %._crit_edge
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB652

thenBB:                                           ; preds = %._crit_edge6
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 13056
  br label %SyncBB

SyncBB652:                                        ; preds = %._crit_edge6
  ret void
}

declare <16 x float> @llvm.x86.mic.undef.ps() nounwind readnone

declare <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float>, i16, i8*, i32, i32) nounwind readonly

declare <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float>, i16, i8*, i32, i32) nounwind readonly

define void @DCT_CPU_VECTOR(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <8 x float> addrspace(1)**
  %4 = load <8 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to <8 x float> addrspace(1)**
  %7 = load <8 x float> addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i8**
  %22 = load i8** %21, align 8
  %tmp61.i = lshr i32 %10, 3
  %tmp62.i = zext i32 %tmp61.i to i64
  %tmp33.i = zext i32 %10 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %23 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = add i64 %24, %26
  %28 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %29 = load i64* %28, align 8
  %30 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %31 = load i64* %30, align 8
  %32 = add i64 %29, %31
  %tmp64.i = trunc i64 %32 to i32
  %tmp65.i = mul i32 %tmp64.i, %10
  %tmp66.i = and i32 %tmp65.i, 536870911
  %tmp68.i = trunc i64 %27 to i32
  %tmp69.i = add i32 %tmp66.i, %tmp68.i
  %tmp70.i = zext i32 %tmp69.i to i64
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to [64 x float]*
  br label %bb.nph14.i

bb.nph14.i:                                       ; preds = %._crit_edge15.i, %SyncBB.i
  %indvar52.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next53.i, %._crit_edge15.i ]
  %tmp63.i = mul i64 %tmp62.i, %indvar52.i
  %tmp71.i = add i64 %tmp70.i, %tmp63.i
  %33 = and i64 %tmp71.i, 4294967295
  %34 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %33
  %35 = load <8 x float> addrspace(1)* %34, align 32
  br label %36

; <label>:36                                      ; preds = %36, %bb.nph14.i
  %indvar49.i = phi i64 [ 0, %bb.nph14.i ], [ %indvar.next50.i, %36 ]
  %tmp58.i = shl i64 %indvar49.i, 3
  %tmp59.i = add i64 %indvar52.i, %tmp58.i
  %scevgep55.i = getelementptr [64 x float]* %CastToValueType.i, i64 0, i64 %tmp59.i
  %scevgep56.i = getelementptr <8 x float> addrspace(1)* %7, i64 %indvar49.i
  %37 = load <8 x float> addrspace(1)* %scevgep56.i, align 32
  %38 = fmul <8 x float> %37, %35
  %39 = extractelement <8 x float> %38, i32 0
  %40 = extractelement <8 x float> %38, i32 1
  %41 = fadd float %39, %40
  %42 = extractelement <8 x float> %38, i32 2
  %43 = fadd float %41, %42
  %44 = extractelement <8 x float> %38, i32 3
  %45 = fadd float %43, %44
  %46 = extractelement <8 x float> %38, i32 4
  %47 = fadd float %45, %46
  %48 = extractelement <8 x float> %38, i32 5
  %49 = fadd float %47, %48
  %50 = extractelement <8 x float> %38, i32 6
  %51 = fadd float %49, %50
  %52 = extractelement <8 x float> %38, i32 7
  %53 = fadd float %51, %52
  store float %53, float* %scevgep55.i, align 4
  %indvar.next50.i = add i64 %indvar49.i, 1
  %exitcond51.i = icmp eq i64 %indvar.next50.i, 8
  br i1 %exitcond51.i, label %._crit_edge15.i, label %36

._crit_edge15.i:                                  ; preds = %36
  %indvar.next53.i = add i64 %indvar52.i, 1
  %exitcond57.i = icmp eq i64 %indvar.next53.i, 8
  br i1 %exitcond57.i, label %bb.nph9.i, label %bb.nph14.i

bb.nph9.i:                                        ; preds = %._crit_edge15.i
  %"&(pSB[currWI].offset)74.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset75.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)74.i"
  %"&(pSB[currWI].offset)78.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset79.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)78.i"
  %CastToValueType80.i = bitcast i8* %"&pSB[currWI].offset79.i" to [8 x <8 x float>]*
  br label %54

; <label>:54                                      ; preds = %54, %bb.nph9.i
  %indvar45.i = phi i64 [ 0, %bb.nph9.i ], [ %indvar.next46.i, %54 ]
  %scevgep48.i = getelementptr [8 x <8 x float>]* %CastToValueType80.i, i64 0, i64 %indvar45.i
  %55 = bitcast i8* %"&pSB[currWI].offset75.i" to i32*
  %56 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i.i = getelementptr inbounds i32* %55, i64 %mul25.i.i.i.i
  %conv27.i.i.i.i = bitcast i32* %add.ptr26.i.i.i.i to i8*
  %57 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %56, i16 255, i8* %conv27.i.i.i.i, i32 0, i32 0) nounwind
  %58 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %57, i16 255, i8* %conv27.i.i.i.i, i32 0, i32 0) nounwind
  %tmp3.i.i = shufflevector <16 x float> %58, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x float> %tmp3.i.i, <8 x float>* %scevgep48.i, align 32
  %indvar.next46.i = add i64 %indvar45.i, 1
  %exitcond47.i = icmp eq i64 %indvar.next46.i, 8
  br i1 %exitcond47.i, label %bb.nph5.i, label %54

bb.nph5.i:                                        ; preds = %54
  %tmp35.i = trunc i64 %32 to i32
  %tmp36.i = mul i32 %tmp35.i, %10
  %tmp37.i = trunc i64 %27 to i32
  %tmp38.i = add i32 %tmp36.i, %tmp37.i
  %tmp39.i = shl i32 %tmp38.i, 3
  %tmp40.i = zext i32 %tmp39.i to i64
  %"&(pSB[currWI].offset)82.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset83.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)82.i"
  %CastToValueType84.i = bitcast i8* %"&pSB[currWI].offset83.i" to [8 x <8 x float>]*
  br label %bb.nph.i

bb.nph.i:                                         ; preds = %._crit_edge.i, %bb.nph5.i
  %indvar19.i = phi i64 [ 0, %bb.nph5.i ], [ %indvar.next20.i, %._crit_edge.i ]
  %tmp34.i = mul i64 %tmp33.i, %indvar19.i
  %tmp41.i = add i64 %tmp40.i, %tmp34.i
  %scevgep44.i = getelementptr <8 x float> addrspace(1)* %7, i64 %indvar19.i
  br label %59

; <label>:59                                      ; preds = %59, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %59 ]
  %tmp42.i = add i64 %tmp41.i, %indvar.i
  %scevgep.i = getelementptr [8 x <8 x float>]* %CastToValueType84.i, i64 0, i64 %indvar.i
  %60 = load <8 x float> addrspace(1)* %scevgep44.i, align 32
  %61 = load <8 x float>* %scevgep.i, align 32
  %62 = fmul <8 x float> %60, %61
  %63 = extractelement <8 x float> %62, i32 0
  %64 = extractelement <8 x float> %62, i32 1
  %65 = fadd float %63, %64
  %66 = extractelement <8 x float> %62, i32 2
  %67 = fadd float %65, %66
  %68 = extractelement <8 x float> %62, i32 3
  %69 = fadd float %67, %68
  %70 = extractelement <8 x float> %62, i32 4
  %71 = fadd float %69, %70
  %72 = extractelement <8 x float> %62, i32 5
  %73 = fadd float %71, %72
  %74 = extractelement <8 x float> %62, i32 6
  %75 = fadd float %73, %74
  %76 = extractelement <8 x float> %62, i32 7
  %77 = fadd float %75, %76
  %78 = and i64 %tmp42.i, 4294967295
  %79 = getelementptr inbounds float addrspace(1)* %1, i64 %78
  store float %77, float addrspace(1)* %79, align 4
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 8
  br i1 %exitcond.i, label %._crit_edge.i, label %59

._crit_edge.i:                                    ; preds = %59
  %indvar.next20.i = add i64 %indvar19.i, 1
  %exitcond32.i = icmp eq i64 %indvar.next20.i, 8
  br i1 %exitcond32.i, label %._crit_edge6.i, label %bb.nph.i

._crit_edge6.i:                                   ; preds = %._crit_edge.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__DCT_CPU_VECTOR_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge6.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 13056
  br label %SyncBB.i

__DCT_CPU_VECTOR_separated_args.exit:             ; preds = %._crit_edge6.i
  ret void
}

define void @DCT_CPU(i8* %pBuffer) {
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
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i8**
  %22 = load i8** %21, align 8
  %tmp134.i = zext i32 %10 to i64
  %tmp65.i = zext i32 %10 to i64
  br label %SyncBB229.i

SyncBB229.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %23 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = add i64 %24, %26
  %28 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %29 = load i64* %28, align 8
  %30 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %31 = load i64* %30, align 8
  %32 = add i64 %29, %31
  %"&pSB[currWI].offset227.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset227.i", i8 0, i64 256, i32 16, i1 false) nounwind
  %tmp136.i = trunc i64 %32 to i32
  %tmp137.i = mul i32 %tmp136.i, %10
  %tmp138.i = trunc i64 %27 to i32
  %tmp139.i = add i32 %tmp137.i, %tmp138.i
  %tmp140.i = shl i32 %tmp139.i, 3
  %tmp141.i = zext i32 %tmp140.i to i64
  %tmp142165.i = or i64 %tmp141.i, 7
  %tmp145166.i = or i64 %tmp141.i, 6
  %tmp148167.i = or i64 %tmp141.i, 5
  %tmp151168.i = or i64 %tmp141.i, 4
  %tmp154169.i = or i64 %tmp141.i, 3
  %tmp157170.i = or i64 %tmp141.i, 2
  %tmp160171.i = or i64 %tmp141.i, 1
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to [64 x float]*
  br label %bb.nph22.i

bb.nph22.i:                                       ; preds = %._crit_edge23.i, %SyncBB229.i
  %indvar98.i = phi i64 [ 0, %SyncBB229.i ], [ %indvar.next99.i, %._crit_edge23.i ]
  %tmp135.i = mul i64 %tmp134.i, %indvar98.i
  %tmp143.i = add i64 %tmp142165.i, %tmp135.i
  %tmp146.i = add i64 %tmp145166.i, %tmp135.i
  %tmp149.i = add i64 %tmp148167.i, %tmp135.i
  %tmp152.i = add i64 %tmp151168.i, %tmp135.i
  %tmp155.i = add i64 %tmp154169.i, %tmp135.i
  %tmp158.i = add i64 %tmp157170.i, %tmp135.i
  %tmp161.i = add i64 %tmp160171.i, %tmp135.i
  %tmp163.i = add i64 %tmp141.i, %tmp135.i
  %33 = and i64 %tmp163.i, 4294967295
  %34 = and i64 %tmp161.i, 4294967295
  %35 = getelementptr inbounds float addrspace(1)* %4, i64 %33
  %36 = getelementptr inbounds float addrspace(1)* %4, i64 %34
  %37 = and i64 %tmp158.i, 4294967295
  %38 = and i64 %tmp155.i, 4294967295
  %39 = getelementptr inbounds float addrspace(1)* %4, i64 %37
  %40 = getelementptr inbounds float addrspace(1)* %4, i64 %38
  %41 = and i64 %tmp152.i, 4294967295
  %42 = and i64 %tmp149.i, 4294967295
  %43 = getelementptr inbounds float addrspace(1)* %4, i64 %41
  %44 = getelementptr inbounds float addrspace(1)* %4, i64 %42
  %45 = and i64 %tmp146.i, 4294967295
  %46 = and i64 %tmp143.i, 4294967295
  %47 = getelementptr inbounds float addrspace(1)* %4, i64 %45
  %48 = getelementptr inbounds float addrspace(1)* %4, i64 %46
  br label %49

; <label>:49                                      ; preds = %49, %bb.nph22.i
  %indvar93.i = phi i64 [ 0, %bb.nph22.i ], [ %indvar.next94.i, %49 ]
  %tmp131.i = shl i64 %indvar93.i, 3
  %tmp132.i = add i64 %indvar98.i, %tmp131.i
  %scevgep129.i = getelementptr [64 x float]* %CastToValueType.i, i64 0, i64 %tmp132.i
  %tmp125178.i = or i64 %tmp131.i, 1
  %tmp123177.i = or i64 %tmp131.i, 2
  %tmp121176.i = or i64 %tmp131.i, 3
  %tmp119175.i = or i64 %tmp131.i, 4
  %tmp117174.i = or i64 %tmp131.i, 5
  %tmp115173.i = or i64 %tmp131.i, 6
  %tmp113172.i = or i64 %tmp131.i, 7
  %scevgep97.i = getelementptr float addrspace(1)* %7, i64 %tmp131.i
  %scevgep97.1.i = getelementptr float addrspace(1)* %7, i64 %tmp125178.i
  %scevgep97.2.i = getelementptr float addrspace(1)* %7, i64 %tmp123177.i
  %scevgep97.3.i = getelementptr float addrspace(1)* %7, i64 %tmp121176.i
  %scevgep97.4.i = getelementptr float addrspace(1)* %7, i64 %tmp119175.i
  %scevgep97.5.i = getelementptr float addrspace(1)* %7, i64 %tmp117174.i
  %scevgep97.6.i = getelementptr float addrspace(1)* %7, i64 %tmp115173.i
  %scevgep97.7.i = getelementptr float addrspace(1)* %7, i64 %tmp113172.i
  %50 = load float addrspace(1)* %scevgep97.i, align 4
  %51 = load float addrspace(1)* %35, align 4
  %52 = load float addrspace(1)* %36, align 4
  %53 = load float addrspace(1)* %scevgep97.1.i, align 4
  %.promoted.i = load float* %scevgep129.i, align 4
  %54 = fmul float %50, %51
  %55 = fmul float %53, %52
  %56 = fadd float %.promoted.i, %54
  %57 = load float addrspace(1)* %scevgep97.2.i, align 4
  %58 = load float addrspace(1)* %39, align 4
  %59 = load float addrspace(1)* %40, align 4
  %60 = load float addrspace(1)* %scevgep97.3.i, align 4
  %61 = fadd float %56, %55
  %62 = fmul float %57, %58
  %63 = fmul float %60, %59
  %64 = fadd float %61, %62
  %65 = load float addrspace(1)* %scevgep97.4.i, align 4
  %66 = load float addrspace(1)* %43, align 4
  %67 = load float addrspace(1)* %44, align 4
  %68 = load float addrspace(1)* %scevgep97.5.i, align 4
  %69 = fadd float %64, %63
  %70 = fmul float %65, %66
  %71 = fmul float %68, %67
  %72 = fadd float %69, %70
  %73 = load float addrspace(1)* %scevgep97.6.i, align 4
  %74 = load float addrspace(1)* %47, align 4
  %75 = load float addrspace(1)* %48, align 4
  %76 = load float addrspace(1)* %scevgep97.7.i, align 4
  %77 = fadd float %72, %71
  %78 = fmul float %73, %74
  %79 = fmul float %76, %75
  %80 = fadd float %77, %78
  %81 = fadd float %80, %79
  store float %81, float* %scevgep129.i, align 4
  %indvar.next94.i = add i64 %indvar93.i, 1
  %exitcond111.i = icmp eq i64 %indvar.next94.i, 8
  br i1 %exitcond111.i, label %._crit_edge23.i, label %49

._crit_edge23.i:                                  ; preds = %49
  %indvar.next99.i = add i64 %indvar98.i, 1
  %exitcond130.i = icmp eq i64 %indvar.next99.i, 8
  br i1 %exitcond130.i, label %bb.nph12.i, label %bb.nph22.i

bb.nph12.i:                                       ; preds = %._crit_edge23.i
  %tmp67.i = trunc i64 %32 to i32
  %tmp68.i = mul i32 %tmp67.i, %10
  %tmp69.i = trunc i64 %27 to i32
  %tmp70.i = add i32 %tmp68.i, %tmp69.i
  %tmp71.i = shl i32 %tmp70.i, 3
  %tmp72.i = zext i32 %tmp71.i to i64
  %"&pSB[currWI].offset195.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType196.i = bitcast i8* %"&pSB[currWI].offset195.i" to [64 x float]*
  %"&pSB[currWI].offset199.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType200.i = bitcast i8* %"&pSB[currWI].offset199.i" to [64 x float]*
  %"&pSB[currWI].offset203.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType204.i = bitcast i8* %"&pSB[currWI].offset203.i" to [64 x float]*
  %"&pSB[currWI].offset207.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType208.i = bitcast i8* %"&pSB[currWI].offset207.i" to [64 x float]*
  %"&pSB[currWI].offset211.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType212.i = bitcast i8* %"&pSB[currWI].offset211.i" to [64 x float]*
  %"&pSB[currWI].offset215.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType216.i = bitcast i8* %"&pSB[currWI].offset215.i" to [64 x float]*
  %"&pSB[currWI].offset219.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType220.i = bitcast i8* %"&pSB[currWI].offset219.i" to [64 x float]*
  %"&pSB[currWI].offset223.i" = getelementptr inbounds i8* %22, i64 %CurrSBIndex..0.i
  %CastToValueType224.i = bitcast i8* %"&pSB[currWI].offset223.i" to [64 x float]*
  br label %bb.nph7.i

bb.nph7.i:                                        ; preds = %._crit_edge8.i, %bb.nph12.i
  %indvar28.i = phi i64 [ 0, %bb.nph12.i ], [ %indvar.next29.i, %._crit_edge8.i ]
  %tmp66.i = mul i64 %tmp65.i, %indvar28.i
  %tmp73.i = add i64 %tmp72.i, %tmp66.i
  %tmp76.i = shl i64 %indvar28.i, 3
  %tmp77179.i = or i64 %tmp76.i, 7
  %scevgep.7.i = getelementptr float addrspace(1)* %7, i64 %tmp77179.i
  %tmp79180.i = or i64 %tmp76.i, 6
  %scevgep.6.i = getelementptr float addrspace(1)* %7, i64 %tmp79180.i
  %tmp81181.i = or i64 %tmp76.i, 5
  %scevgep.5.i = getelementptr float addrspace(1)* %7, i64 %tmp81181.i
  %tmp83182.i = or i64 %tmp76.i, 4
  %scevgep.4.i = getelementptr float addrspace(1)* %7, i64 %tmp83182.i
  %tmp85183.i = or i64 %tmp76.i, 3
  %scevgep.3.i = getelementptr float addrspace(1)* %7, i64 %tmp85183.i
  %tmp87184.i = or i64 %tmp76.i, 2
  %scevgep.2.i = getelementptr float addrspace(1)* %7, i64 %tmp87184.i
  %tmp89185.i = or i64 %tmp76.i, 1
  %scevgep.1.i = getelementptr float addrspace(1)* %7, i64 %tmp89185.i
  %scevgep.i = getelementptr float addrspace(1)* %7, i64 %tmp76.i
  br label %82

; <label>:82                                      ; preds = %82, %bb.nph7.i
  %indvar32.i = phi i64 [ 0, %bb.nph7.i ], [ %indvar.next33.i, %82 ]
  %tmp74.i = add i64 %tmp73.i, %indvar32.i
  %tmp37.i = shl i64 %indvar32.i, 3
  %83 = and i64 %tmp74.i, 4294967295
  %84 = getelementptr inbounds float addrspace(1)* %1, i64 %83
  store float 0.000000e+00, float addrspace(1)* %84, align 4
  %tmp50192.i = or i64 %tmp37.i, 1
  %tmp48191.i = or i64 %tmp37.i, 2
  %tmp46190.i = or i64 %tmp37.i, 3
  %tmp44189.i = or i64 %tmp37.i, 4
  %tmp42188.i = or i64 %tmp37.i, 5
  %tmp40187.i = or i64 %tmp37.i, 6
  %tmp38186.i = or i64 %tmp37.i, 7
  %scevgep36.i = getelementptr [64 x float]* %CastToValueType196.i, i64 0, i64 %tmp37.i
  %scevgep36.1.i = getelementptr [64 x float]* %CastToValueType200.i, i64 0, i64 %tmp50192.i
  %scevgep36.2.i = getelementptr [64 x float]* %CastToValueType204.i, i64 0, i64 %tmp48191.i
  %scevgep36.3.i = getelementptr [64 x float]* %CastToValueType208.i, i64 0, i64 %tmp46190.i
  %scevgep36.4.i = getelementptr [64 x float]* %CastToValueType212.i, i64 0, i64 %tmp44189.i
  %scevgep36.5.i = getelementptr [64 x float]* %CastToValueType216.i, i64 0, i64 %tmp42188.i
  %scevgep36.6.i = getelementptr [64 x float]* %CastToValueType220.i, i64 0, i64 %tmp40187.i
  %scevgep36.7.i = getelementptr [64 x float]* %CastToValueType224.i, i64 0, i64 %tmp38186.i
  %85 = load float addrspace(1)* %scevgep.i, align 4
  %86 = load float* %scevgep36.i, align 16
  %87 = fmul float %85, %86
  %88 = fadd float %87, 0.000000e+00
  store float %88, float addrspace(1)* %84, align 4
  %89 = load float addrspace(1)* %scevgep.1.i, align 4
  %90 = load float* %scevgep36.1.i, align 4
  %91 = fmul float %89, %90
  %92 = fadd float %88, %91
  store float %92, float addrspace(1)* %84, align 4
  %93 = load float addrspace(1)* %scevgep.2.i, align 4
  %94 = load float* %scevgep36.2.i, align 8
  %95 = fmul float %93, %94
  %96 = fadd float %92, %95
  store float %96, float addrspace(1)* %84, align 4
  %97 = load float addrspace(1)* %scevgep.3.i, align 4
  %98 = load float* %scevgep36.3.i, align 4
  %99 = fmul float %97, %98
  %100 = fadd float %96, %99
  store float %100, float addrspace(1)* %84, align 4
  %101 = load float addrspace(1)* %scevgep.4.i, align 4
  %102 = load float* %scevgep36.4.i, align 16
  %103 = fmul float %101, %102
  %104 = fadd float %100, %103
  store float %104, float addrspace(1)* %84, align 4
  %105 = load float addrspace(1)* %scevgep.5.i, align 4
  %106 = load float* %scevgep36.5.i, align 4
  %107 = fmul float %105, %106
  %108 = fadd float %104, %107
  store float %108, float addrspace(1)* %84, align 4
  %109 = load float addrspace(1)* %scevgep.6.i, align 4
  %110 = load float* %scevgep36.6.i, align 8
  %111 = fmul float %109, %110
  %112 = fadd float %108, %111
  store float %112, float addrspace(1)* %84, align 4
  %113 = load float addrspace(1)* %scevgep.7.i, align 4
  %114 = load float* %scevgep36.7.i, align 4
  %115 = fmul float %113, %114
  %116 = fadd float %112, %115
  store float %116, float addrspace(1)* %84, align 4
  %indvar.next33.i = add i64 %indvar32.i, 1
  %exitcond.i = icmp eq i64 %indvar.next33.i, 8
  br i1 %exitcond.i, label %._crit_edge8.i, label %82

._crit_edge8.i:                                   ; preds = %82
  %indvar.next29.i = add i64 %indvar28.i, 1
  %exitcond64.i = icmp eq i64 %indvar.next29.i, 8
  br i1 %exitcond64.i, label %._crit_edge13.i, label %bb.nph7.i

._crit_edge13.i:                                  ; preds = %._crit_edge8.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__DCT_CPU_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge13.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 13056
  br label %SyncBB229.i

__DCT_CPU_separated_args.exit:                    ; preds = %._crit_edge13.i
  ret void
}

define void @__Vectorized_.DCT_CPU(i8* %pBuffer) {
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
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i8**
  %22 = load i8** %21, align 8
  %tmp134.i = zext i32 %10 to i64
  %tmp65.i = zext i32 %10 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %23 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = add i64 %24, %26
  %broadcast1.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %29 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %"&(pSB[currWI].offset)657.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset658.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)657.i"
  %"&(pSB[currWI].offset)697.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset698.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)697.i"
  %"&(pSB[currWI].offset)737.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset738.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)737.i"
  %"&(pSB[currWI].offset)777.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset778.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)777.i"
  %"&(pSB[currWI].offset)817.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset818.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)817.i"
  %"&(pSB[currWI].offset)857.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset858.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)857.i"
  %"&(pSB[currWI].offset)897.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset898.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)897.i"
  %"&(pSB[currWI].offset)937.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset938.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)937.i"
  %"&(pSB[currWI].offset)977.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset978.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)977.i"
  %"&(pSB[currWI].offset)1017.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset1018.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1017.i"
  %"&(pSB[currWI].offset)1057.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1058.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1057.i"
  %"&(pSB[currWI].offset)1097.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1098.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1097.i"
  %"&(pSB[currWI].offset)1137.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1138.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1137.i"
  %"&(pSB[currWI].offset)1177.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1178.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1177.i"
  %"&(pSB[currWI].offset)1217.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1218.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1217.i"
  %"&(pSB[currWI].offset)1257.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1258.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1257.i"
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset658.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset698.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset738.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset778.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset818.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset858.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset898.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset938.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset978.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1018.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1058.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1098.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1138.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1178.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1218.i", i8 0, i64 256, i32 16, i1 false) nounwind
  call void @llvm.memset.p0i8.i64(i8* %"&pSB[currWI].offset1258.i", i8 0, i64 256, i32 16, i1 false) nounwind
  %tmp136.i = trunc i64 %33 to i32
  %tmp137.i = mul i32 %tmp136.i, %10
  %temp.i = insertelement <16 x i32> undef, i32 %tmp137.i, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp1381.i = trunc <16 x i64> %28 to <16 x i32>
  %tmp1392.i = add <16 x i32> %vector.i, %tmp1381.i
  %tmp1403.i = shl <16 x i32> %tmp1392.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp1414.i = zext <16 x i32> %tmp1403.i to <16 x i64>
  %tmp1421655.i = or <16 x i64> %tmp1414.i, <i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7>
  %tmp1451666.i = or <16 x i64> %tmp1414.i, <i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6, i64 6>
  %tmp1481677.i = or <16 x i64> %tmp1414.i, <i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5, i64 5>
  %tmp1511688.i = or <16 x i64> %tmp1414.i, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  %tmp1541699.i = or <16 x i64> %tmp1414.i, <i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3>
  %tmp15717010.i = or <16 x i64> %tmp1414.i, <i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2, i64 2>
  %tmp16017111.i = or <16 x i64> %tmp1414.i, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %"&(pSB[currWI].offset)653.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset654.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)653.i"
  %CastToValueType655.i = bitcast i8* %"&pSB[currWI].offset654.i" to [64 x float]*
  %"&(pSB[currWI].offset)693.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset694.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)693.i"
  %CastToValueType695.i = bitcast i8* %"&pSB[currWI].offset694.i" to [64 x float]*
  %"&(pSB[currWI].offset)733.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset734.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)733.i"
  %CastToValueType735.i = bitcast i8* %"&pSB[currWI].offset734.i" to [64 x float]*
  %"&(pSB[currWI].offset)773.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset774.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)773.i"
  %CastToValueType775.i = bitcast i8* %"&pSB[currWI].offset774.i" to [64 x float]*
  %"&(pSB[currWI].offset)813.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset814.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)813.i"
  %CastToValueType815.i = bitcast i8* %"&pSB[currWI].offset814.i" to [64 x float]*
  %"&(pSB[currWI].offset)853.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset854.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)853.i"
  %CastToValueType855.i = bitcast i8* %"&pSB[currWI].offset854.i" to [64 x float]*
  %"&(pSB[currWI].offset)893.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset894.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)893.i"
  %CastToValueType895.i = bitcast i8* %"&pSB[currWI].offset894.i" to [64 x float]*
  %"&(pSB[currWI].offset)933.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset934.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)933.i"
  %CastToValueType935.i = bitcast i8* %"&pSB[currWI].offset934.i" to [64 x float]*
  %"&(pSB[currWI].offset)973.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset974.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)973.i"
  %CastToValueType975.i = bitcast i8* %"&pSB[currWI].offset974.i" to [64 x float]*
  %"&(pSB[currWI].offset)1013.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset1014.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1013.i"
  %CastToValueType1015.i = bitcast i8* %"&pSB[currWI].offset1014.i" to [64 x float]*
  %"&(pSB[currWI].offset)1053.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1054.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1053.i"
  %CastToValueType1055.i = bitcast i8* %"&pSB[currWI].offset1054.i" to [64 x float]*
  %"&(pSB[currWI].offset)1093.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1094.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1093.i"
  %CastToValueType1095.i = bitcast i8* %"&pSB[currWI].offset1094.i" to [64 x float]*
  %"&(pSB[currWI].offset)1133.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1134.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1133.i"
  %CastToValueType1135.i = bitcast i8* %"&pSB[currWI].offset1134.i" to [64 x float]*
  %"&(pSB[currWI].offset)1173.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1174.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1173.i"
  %CastToValueType1175.i = bitcast i8* %"&pSB[currWI].offset1174.i" to [64 x float]*
  %"&(pSB[currWI].offset)1213.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1214.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1213.i"
  %CastToValueType1215.i = bitcast i8* %"&pSB[currWI].offset1214.i" to [64 x float]*
  %"&(pSB[currWI].offset)1253.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1254.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1253.i"
  %CastToValueType1255.i = bitcast i8* %"&pSB[currWI].offset1254.i" to [64 x float]*
  br label %bb.nph22.i

bb.nph22.i:                                       ; preds = %._crit_edge23.i, %SyncBB.i
  %indvar98.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next99.i, %._crit_edge23.i ]
  %tmp135.i = mul i64 %tmp134.i, %indvar98.i
  %temp12.i = insertelement <16 x i64> undef, i64 %tmp135.i, i32 0
  %vector13.i = shufflevector <16 x i64> %temp12.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp14314.i = add <16 x i64> %tmp1421655.i, %vector13.i
  %tmp14615.i = add <16 x i64> %tmp1451666.i, %vector13.i
  %tmp14916.i = add <16 x i64> %tmp1481677.i, %vector13.i
  %tmp15217.i = add <16 x i64> %tmp1511688.i, %vector13.i
  %tmp15518.i = add <16 x i64> %tmp1541699.i, %vector13.i
  %tmp15819.i = add <16 x i64> %tmp15717010.i, %vector13.i
  %tmp16120.i = add <16 x i64> %tmp16017111.i, %vector13.i
  %tmp16321.i = add <16 x i64> %tmp1414.i, %vector13.i
  %34 = and <16 x i64> %tmp16321.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract.i = extractelement <16 x i64> %34, i32 0
  %extract22.i = extractelement <16 x i64> %34, i32 1
  %extract23.i = extractelement <16 x i64> %34, i32 2
  %extract24.i = extractelement <16 x i64> %34, i32 3
  %extract25.i = extractelement <16 x i64> %34, i32 4
  %extract26.i = extractelement <16 x i64> %34, i32 5
  %extract27.i = extractelement <16 x i64> %34, i32 6
  %extract28.i = extractelement <16 x i64> %34, i32 7
  %extract29.i = extractelement <16 x i64> %34, i32 8
  %extract30.i = extractelement <16 x i64> %34, i32 9
  %extract31.i = extractelement <16 x i64> %34, i32 10
  %extract32.i = extractelement <16 x i64> %34, i32 11
  %extract33.i = extractelement <16 x i64> %34, i32 12
  %extract34.i = extractelement <16 x i64> %34, i32 13
  %extract35.i = extractelement <16 x i64> %34, i32 14
  %extract36.i = extractelement <16 x i64> %34, i32 15
  %35 = and <16 x i64> %tmp16120.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract37.i = extractelement <16 x i64> %35, i32 0
  %extract38.i = extractelement <16 x i64> %35, i32 1
  %extract39.i = extractelement <16 x i64> %35, i32 2
  %extract40.i = extractelement <16 x i64> %35, i32 3
  %extract41.i = extractelement <16 x i64> %35, i32 4
  %extract42.i = extractelement <16 x i64> %35, i32 5
  %extract43.i = extractelement <16 x i64> %35, i32 6
  %extract44.i = extractelement <16 x i64> %35, i32 7
  %extract45.i = extractelement <16 x i64> %35, i32 8
  %extract46.i = extractelement <16 x i64> %35, i32 9
  %extract47.i = extractelement <16 x i64> %35, i32 10
  %extract48.i = extractelement <16 x i64> %35, i32 11
  %extract49.i = extractelement <16 x i64> %35, i32 12
  %extract50.i = extractelement <16 x i64> %35, i32 13
  %extract51.i = extractelement <16 x i64> %35, i32 14
  %extract52.i = extractelement <16 x i64> %35, i32 15
  %36 = getelementptr inbounds float addrspace(1)* %4, i64 %extract.i
  %37 = getelementptr inbounds float addrspace(1)* %4, i64 %extract22.i
  %38 = getelementptr inbounds float addrspace(1)* %4, i64 %extract23.i
  %39 = getelementptr inbounds float addrspace(1)* %4, i64 %extract24.i
  %40 = getelementptr inbounds float addrspace(1)* %4, i64 %extract25.i
  %41 = getelementptr inbounds float addrspace(1)* %4, i64 %extract26.i
  %42 = getelementptr inbounds float addrspace(1)* %4, i64 %extract27.i
  %43 = getelementptr inbounds float addrspace(1)* %4, i64 %extract28.i
  %44 = getelementptr inbounds float addrspace(1)* %4, i64 %extract29.i
  %45 = getelementptr inbounds float addrspace(1)* %4, i64 %extract30.i
  %46 = getelementptr inbounds float addrspace(1)* %4, i64 %extract31.i
  %47 = getelementptr inbounds float addrspace(1)* %4, i64 %extract32.i
  %48 = getelementptr inbounds float addrspace(1)* %4, i64 %extract33.i
  %49 = getelementptr inbounds float addrspace(1)* %4, i64 %extract34.i
  %50 = getelementptr inbounds float addrspace(1)* %4, i64 %extract35.i
  %51 = getelementptr inbounds float addrspace(1)* %4, i64 %extract36.i
  %52 = getelementptr inbounds float addrspace(1)* %4, i64 %extract37.i
  %53 = getelementptr inbounds float addrspace(1)* %4, i64 %extract38.i
  %54 = getelementptr inbounds float addrspace(1)* %4, i64 %extract39.i
  %55 = getelementptr inbounds float addrspace(1)* %4, i64 %extract40.i
  %56 = getelementptr inbounds float addrspace(1)* %4, i64 %extract41.i
  %57 = getelementptr inbounds float addrspace(1)* %4, i64 %extract42.i
  %58 = getelementptr inbounds float addrspace(1)* %4, i64 %extract43.i
  %59 = getelementptr inbounds float addrspace(1)* %4, i64 %extract44.i
  %60 = getelementptr inbounds float addrspace(1)* %4, i64 %extract45.i
  %61 = getelementptr inbounds float addrspace(1)* %4, i64 %extract46.i
  %62 = getelementptr inbounds float addrspace(1)* %4, i64 %extract47.i
  %63 = getelementptr inbounds float addrspace(1)* %4, i64 %extract48.i
  %64 = getelementptr inbounds float addrspace(1)* %4, i64 %extract49.i
  %65 = getelementptr inbounds float addrspace(1)* %4, i64 %extract50.i
  %66 = getelementptr inbounds float addrspace(1)* %4, i64 %extract51.i
  %67 = getelementptr inbounds float addrspace(1)* %4, i64 %extract52.i
  %68 = and <16 x i64> %tmp15819.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract70.i = extractelement <16 x i64> %68, i32 0
  %extract71.i = extractelement <16 x i64> %68, i32 1
  %extract72.i = extractelement <16 x i64> %68, i32 2
  %extract73.i = extractelement <16 x i64> %68, i32 3
  %extract74.i = extractelement <16 x i64> %68, i32 4
  %extract75.i = extractelement <16 x i64> %68, i32 5
  %extract76.i = extractelement <16 x i64> %68, i32 6
  %extract77.i = extractelement <16 x i64> %68, i32 7
  %extract78.i = extractelement <16 x i64> %68, i32 8
  %extract79.i = extractelement <16 x i64> %68, i32 9
  %extract80.i = extractelement <16 x i64> %68, i32 10
  %extract81.i = extractelement <16 x i64> %68, i32 11
  %extract82.i = extractelement <16 x i64> %68, i32 12
  %extract83.i = extractelement <16 x i64> %68, i32 13
  %extract84.i = extractelement <16 x i64> %68, i32 14
  %extract85.i = extractelement <16 x i64> %68, i32 15
  %69 = and <16 x i64> %tmp15518.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract86.i = extractelement <16 x i64> %69, i32 0
  %extract87.i = extractelement <16 x i64> %69, i32 1
  %extract88.i = extractelement <16 x i64> %69, i32 2
  %extract89.i = extractelement <16 x i64> %69, i32 3
  %extract90.i = extractelement <16 x i64> %69, i32 4
  %extract91.i = extractelement <16 x i64> %69, i32 5
  %extract92.i = extractelement <16 x i64> %69, i32 6
  %extract93.i = extractelement <16 x i64> %69, i32 7
  %extract94.i = extractelement <16 x i64> %69, i32 8
  %extract95.i = extractelement <16 x i64> %69, i32 9
  %extract96.i = extractelement <16 x i64> %69, i32 10
  %extract97.i = extractelement <16 x i64> %69, i32 11
  %extract98.i = extractelement <16 x i64> %69, i32 12
  %extract99.i = extractelement <16 x i64> %69, i32 13
  %extract100.i = extractelement <16 x i64> %69, i32 14
  %extract101.i = extractelement <16 x i64> %69, i32 15
  %70 = getelementptr inbounds float addrspace(1)* %4, i64 %extract70.i
  %71 = getelementptr inbounds float addrspace(1)* %4, i64 %extract71.i
  %72 = getelementptr inbounds float addrspace(1)* %4, i64 %extract72.i
  %73 = getelementptr inbounds float addrspace(1)* %4, i64 %extract73.i
  %74 = getelementptr inbounds float addrspace(1)* %4, i64 %extract74.i
  %75 = getelementptr inbounds float addrspace(1)* %4, i64 %extract75.i
  %76 = getelementptr inbounds float addrspace(1)* %4, i64 %extract76.i
  %77 = getelementptr inbounds float addrspace(1)* %4, i64 %extract77.i
  %78 = getelementptr inbounds float addrspace(1)* %4, i64 %extract78.i
  %79 = getelementptr inbounds float addrspace(1)* %4, i64 %extract79.i
  %80 = getelementptr inbounds float addrspace(1)* %4, i64 %extract80.i
  %81 = getelementptr inbounds float addrspace(1)* %4, i64 %extract81.i
  %82 = getelementptr inbounds float addrspace(1)* %4, i64 %extract82.i
  %83 = getelementptr inbounds float addrspace(1)* %4, i64 %extract83.i
  %84 = getelementptr inbounds float addrspace(1)* %4, i64 %extract84.i
  %85 = getelementptr inbounds float addrspace(1)* %4, i64 %extract85.i
  %86 = getelementptr inbounds float addrspace(1)* %4, i64 %extract86.i
  %87 = getelementptr inbounds float addrspace(1)* %4, i64 %extract87.i
  %88 = getelementptr inbounds float addrspace(1)* %4, i64 %extract88.i
  %89 = getelementptr inbounds float addrspace(1)* %4, i64 %extract89.i
  %90 = getelementptr inbounds float addrspace(1)* %4, i64 %extract90.i
  %91 = getelementptr inbounds float addrspace(1)* %4, i64 %extract91.i
  %92 = getelementptr inbounds float addrspace(1)* %4, i64 %extract92.i
  %93 = getelementptr inbounds float addrspace(1)* %4, i64 %extract93.i
  %94 = getelementptr inbounds float addrspace(1)* %4, i64 %extract94.i
  %95 = getelementptr inbounds float addrspace(1)* %4, i64 %extract95.i
  %96 = getelementptr inbounds float addrspace(1)* %4, i64 %extract96.i
  %97 = getelementptr inbounds float addrspace(1)* %4, i64 %extract97.i
  %98 = getelementptr inbounds float addrspace(1)* %4, i64 %extract98.i
  %99 = getelementptr inbounds float addrspace(1)* %4, i64 %extract99.i
  %100 = getelementptr inbounds float addrspace(1)* %4, i64 %extract100.i
  %101 = getelementptr inbounds float addrspace(1)* %4, i64 %extract101.i
  %102 = and <16 x i64> %tmp15217.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract154.i = extractelement <16 x i64> %102, i32 0
  %extract155.i = extractelement <16 x i64> %102, i32 1
  %extract156.i = extractelement <16 x i64> %102, i32 2
  %extract157.i = extractelement <16 x i64> %102, i32 3
  %extract158.i = extractelement <16 x i64> %102, i32 4
  %extract159.i = extractelement <16 x i64> %102, i32 5
  %extract160.i = extractelement <16 x i64> %102, i32 6
  %extract161.i = extractelement <16 x i64> %102, i32 7
  %extract162.i = extractelement <16 x i64> %102, i32 8
  %extract163.i = extractelement <16 x i64> %102, i32 9
  %extract164.i = extractelement <16 x i64> %102, i32 10
  %extract165.i = extractelement <16 x i64> %102, i32 11
  %extract166.i = extractelement <16 x i64> %102, i32 12
  %extract167.i = extractelement <16 x i64> %102, i32 13
  %extract168.i = extractelement <16 x i64> %102, i32 14
  %extract169.i = extractelement <16 x i64> %102, i32 15
  %103 = and <16 x i64> %tmp14916.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract170.i = extractelement <16 x i64> %103, i32 0
  %extract171.i = extractelement <16 x i64> %103, i32 1
  %extract172.i = extractelement <16 x i64> %103, i32 2
  %extract173.i = extractelement <16 x i64> %103, i32 3
  %extract174.i = extractelement <16 x i64> %103, i32 4
  %extract175.i = extractelement <16 x i64> %103, i32 5
  %extract176.i = extractelement <16 x i64> %103, i32 6
  %extract177.i = extractelement <16 x i64> %103, i32 7
  %extract178.i = extractelement <16 x i64> %103, i32 8
  %extract179.i = extractelement <16 x i64> %103, i32 9
  %extract180.i = extractelement <16 x i64> %103, i32 10
  %extract181.i = extractelement <16 x i64> %103, i32 11
  %extract182.i = extractelement <16 x i64> %103, i32 12
  %extract183.i = extractelement <16 x i64> %103, i32 13
  %extract184.i = extractelement <16 x i64> %103, i32 14
  %extract185.i = extractelement <16 x i64> %103, i32 15
  %104 = getelementptr inbounds float addrspace(1)* %4, i64 %extract154.i
  %105 = getelementptr inbounds float addrspace(1)* %4, i64 %extract155.i
  %106 = getelementptr inbounds float addrspace(1)* %4, i64 %extract156.i
  %107 = getelementptr inbounds float addrspace(1)* %4, i64 %extract157.i
  %108 = getelementptr inbounds float addrspace(1)* %4, i64 %extract158.i
  %109 = getelementptr inbounds float addrspace(1)* %4, i64 %extract159.i
  %110 = getelementptr inbounds float addrspace(1)* %4, i64 %extract160.i
  %111 = getelementptr inbounds float addrspace(1)* %4, i64 %extract161.i
  %112 = getelementptr inbounds float addrspace(1)* %4, i64 %extract162.i
  %113 = getelementptr inbounds float addrspace(1)* %4, i64 %extract163.i
  %114 = getelementptr inbounds float addrspace(1)* %4, i64 %extract164.i
  %115 = getelementptr inbounds float addrspace(1)* %4, i64 %extract165.i
  %116 = getelementptr inbounds float addrspace(1)* %4, i64 %extract166.i
  %117 = getelementptr inbounds float addrspace(1)* %4, i64 %extract167.i
  %118 = getelementptr inbounds float addrspace(1)* %4, i64 %extract168.i
  %119 = getelementptr inbounds float addrspace(1)* %4, i64 %extract169.i
  %120 = getelementptr inbounds float addrspace(1)* %4, i64 %extract170.i
  %121 = getelementptr inbounds float addrspace(1)* %4, i64 %extract171.i
  %122 = getelementptr inbounds float addrspace(1)* %4, i64 %extract172.i
  %123 = getelementptr inbounds float addrspace(1)* %4, i64 %extract173.i
  %124 = getelementptr inbounds float addrspace(1)* %4, i64 %extract174.i
  %125 = getelementptr inbounds float addrspace(1)* %4, i64 %extract175.i
  %126 = getelementptr inbounds float addrspace(1)* %4, i64 %extract176.i
  %127 = getelementptr inbounds float addrspace(1)* %4, i64 %extract177.i
  %128 = getelementptr inbounds float addrspace(1)* %4, i64 %extract178.i
  %129 = getelementptr inbounds float addrspace(1)* %4, i64 %extract179.i
  %130 = getelementptr inbounds float addrspace(1)* %4, i64 %extract180.i
  %131 = getelementptr inbounds float addrspace(1)* %4, i64 %extract181.i
  %132 = getelementptr inbounds float addrspace(1)* %4, i64 %extract182.i
  %133 = getelementptr inbounds float addrspace(1)* %4, i64 %extract183.i
  %134 = getelementptr inbounds float addrspace(1)* %4, i64 %extract184.i
  %135 = getelementptr inbounds float addrspace(1)* %4, i64 %extract185.i
  %136 = and <16 x i64> %tmp14615.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract222.i = extractelement <16 x i64> %136, i32 0
  %extract223.i = extractelement <16 x i64> %136, i32 1
  %extract224.i = extractelement <16 x i64> %136, i32 2
  %extract225.i = extractelement <16 x i64> %136, i32 3
  %extract226.i = extractelement <16 x i64> %136, i32 4
  %extract227.i = extractelement <16 x i64> %136, i32 5
  %extract228.i = extractelement <16 x i64> %136, i32 6
  %extract229.i = extractelement <16 x i64> %136, i32 7
  %extract230.i = extractelement <16 x i64> %136, i32 8
  %extract231.i = extractelement <16 x i64> %136, i32 9
  %extract232.i = extractelement <16 x i64> %136, i32 10
  %extract233.i = extractelement <16 x i64> %136, i32 11
  %extract234.i = extractelement <16 x i64> %136, i32 12
  %extract235.i = extractelement <16 x i64> %136, i32 13
  %extract236.i = extractelement <16 x i64> %136, i32 14
  %extract237.i = extractelement <16 x i64> %136, i32 15
  %137 = and <16 x i64> %tmp14314.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract238.i = extractelement <16 x i64> %137, i32 0
  %extract239.i = extractelement <16 x i64> %137, i32 1
  %extract240.i = extractelement <16 x i64> %137, i32 2
  %extract241.i = extractelement <16 x i64> %137, i32 3
  %extract242.i = extractelement <16 x i64> %137, i32 4
  %extract243.i = extractelement <16 x i64> %137, i32 5
  %extract244.i = extractelement <16 x i64> %137, i32 6
  %extract245.i = extractelement <16 x i64> %137, i32 7
  %extract246.i = extractelement <16 x i64> %137, i32 8
  %extract247.i = extractelement <16 x i64> %137, i32 9
  %extract248.i = extractelement <16 x i64> %137, i32 10
  %extract249.i = extractelement <16 x i64> %137, i32 11
  %extract250.i = extractelement <16 x i64> %137, i32 12
  %extract251.i = extractelement <16 x i64> %137, i32 13
  %extract252.i = extractelement <16 x i64> %137, i32 14
  %extract253.i = extractelement <16 x i64> %137, i32 15
  %138 = getelementptr inbounds float addrspace(1)* %4, i64 %extract222.i
  %139 = getelementptr inbounds float addrspace(1)* %4, i64 %extract223.i
  %140 = getelementptr inbounds float addrspace(1)* %4, i64 %extract224.i
  %141 = getelementptr inbounds float addrspace(1)* %4, i64 %extract225.i
  %142 = getelementptr inbounds float addrspace(1)* %4, i64 %extract226.i
  %143 = getelementptr inbounds float addrspace(1)* %4, i64 %extract227.i
  %144 = getelementptr inbounds float addrspace(1)* %4, i64 %extract228.i
  %145 = getelementptr inbounds float addrspace(1)* %4, i64 %extract229.i
  %146 = getelementptr inbounds float addrspace(1)* %4, i64 %extract230.i
  %147 = getelementptr inbounds float addrspace(1)* %4, i64 %extract231.i
  %148 = getelementptr inbounds float addrspace(1)* %4, i64 %extract232.i
  %149 = getelementptr inbounds float addrspace(1)* %4, i64 %extract233.i
  %150 = getelementptr inbounds float addrspace(1)* %4, i64 %extract234.i
  %151 = getelementptr inbounds float addrspace(1)* %4, i64 %extract235.i
  %152 = getelementptr inbounds float addrspace(1)* %4, i64 %extract236.i
  %153 = getelementptr inbounds float addrspace(1)* %4, i64 %extract237.i
  %154 = getelementptr inbounds float addrspace(1)* %4, i64 %extract238.i
  %155 = getelementptr inbounds float addrspace(1)* %4, i64 %extract239.i
  %156 = getelementptr inbounds float addrspace(1)* %4, i64 %extract240.i
  %157 = getelementptr inbounds float addrspace(1)* %4, i64 %extract241.i
  %158 = getelementptr inbounds float addrspace(1)* %4, i64 %extract242.i
  %159 = getelementptr inbounds float addrspace(1)* %4, i64 %extract243.i
  %160 = getelementptr inbounds float addrspace(1)* %4, i64 %extract244.i
  %161 = getelementptr inbounds float addrspace(1)* %4, i64 %extract245.i
  %162 = getelementptr inbounds float addrspace(1)* %4, i64 %extract246.i
  %163 = getelementptr inbounds float addrspace(1)* %4, i64 %extract247.i
  %164 = getelementptr inbounds float addrspace(1)* %4, i64 %extract248.i
  %165 = getelementptr inbounds float addrspace(1)* %4, i64 %extract249.i
  %166 = getelementptr inbounds float addrspace(1)* %4, i64 %extract250.i
  %167 = getelementptr inbounds float addrspace(1)* %4, i64 %extract251.i
  %168 = getelementptr inbounds float addrspace(1)* %4, i64 %extract252.i
  %169 = getelementptr inbounds float addrspace(1)* %4, i64 %extract253.i
  br label %170

; <label>:170                                     ; preds = %170, %bb.nph22.i
  %indvar93.i = phi i64 [ 0, %bb.nph22.i ], [ %indvar.next94.i, %170 ]
  %tmp131.i = shl i64 %indvar93.i, 3
  %tmp132.i = add i64 %indvar98.i, %tmp131.i
  %171 = getelementptr [64 x float]* %CastToValueType655.i, i64 0, i64 %tmp132.i
  %172 = getelementptr [64 x float]* %CastToValueType695.i, i64 0, i64 %tmp132.i
  %173 = getelementptr [64 x float]* %CastToValueType735.i, i64 0, i64 %tmp132.i
  %174 = getelementptr [64 x float]* %CastToValueType775.i, i64 0, i64 %tmp132.i
  %175 = getelementptr [64 x float]* %CastToValueType815.i, i64 0, i64 %tmp132.i
  %176 = getelementptr [64 x float]* %CastToValueType855.i, i64 0, i64 %tmp132.i
  %177 = getelementptr [64 x float]* %CastToValueType895.i, i64 0, i64 %tmp132.i
  %178 = getelementptr [64 x float]* %CastToValueType935.i, i64 0, i64 %tmp132.i
  %179 = getelementptr [64 x float]* %CastToValueType975.i, i64 0, i64 %tmp132.i
  %180 = getelementptr [64 x float]* %CastToValueType1015.i, i64 0, i64 %tmp132.i
  %181 = getelementptr [64 x float]* %CastToValueType1055.i, i64 0, i64 %tmp132.i
  %182 = getelementptr [64 x float]* %CastToValueType1095.i, i64 0, i64 %tmp132.i
  %183 = getelementptr [64 x float]* %CastToValueType1135.i, i64 0, i64 %tmp132.i
  %184 = getelementptr [64 x float]* %CastToValueType1175.i, i64 0, i64 %tmp132.i
  %185 = getelementptr [64 x float]* %CastToValueType1215.i, i64 0, i64 %tmp132.i
  %186 = getelementptr [64 x float]* %CastToValueType1255.i, i64 0, i64 %tmp132.i
  %tmp125178.i = or i64 %tmp131.i, 1
  %tmp123177.i = or i64 %tmp131.i, 2
  %tmp121176.i = or i64 %tmp131.i, 3
  %tmp119175.i = or i64 %tmp131.i, 4
  %tmp117174.i = or i64 %tmp131.i, 5
  %tmp115173.i = or i64 %tmp131.i, 6
  %tmp113172.i = or i64 %tmp131.i, 7
  %scevgep97.i = getelementptr float addrspace(1)* %7, i64 %tmp131.i
  %scevgep97.1.i = getelementptr float addrspace(1)* %7, i64 %tmp125178.i
  %scevgep97.2.i = getelementptr float addrspace(1)* %7, i64 %tmp123177.i
  %scevgep97.3.i = getelementptr float addrspace(1)* %7, i64 %tmp121176.i
  %scevgep97.4.i = getelementptr float addrspace(1)* %7, i64 %tmp119175.i
  %scevgep97.5.i = getelementptr float addrspace(1)* %7, i64 %tmp117174.i
  %scevgep97.6.i = getelementptr float addrspace(1)* %7, i64 %tmp115173.i
  %scevgep97.7.i = getelementptr float addrspace(1)* %7, i64 %tmp113172.i
  %187 = load float addrspace(1)* %scevgep97.i, align 4
  %temp53.i = insertelement <16 x float> undef, float %187, i32 0
  %vector54.i = shufflevector <16 x float> %temp53.i, <16 x float> undef, <16 x i32> zeroinitializer
  %188 = load float addrspace(1)* %36, align 4
  %189 = load float addrspace(1)* %37, align 4
  %190 = load float addrspace(1)* %38, align 4
  %191 = load float addrspace(1)* %39, align 4
  %192 = load float addrspace(1)* %40, align 4
  %193 = load float addrspace(1)* %41, align 4
  %194 = load float addrspace(1)* %42, align 4
  %195 = load float addrspace(1)* %43, align 4
  %196 = load float addrspace(1)* %44, align 4
  %197 = load float addrspace(1)* %45, align 4
  %198 = load float addrspace(1)* %46, align 4
  %199 = load float addrspace(1)* %47, align 4
  %200 = load float addrspace(1)* %48, align 4
  %201 = load float addrspace(1)* %49, align 4
  %202 = load float addrspace(1)* %50, align 4
  %203 = load float addrspace(1)* %51, align 4
  %temp.vect.i = insertelement <16 x float> undef, float %188, i32 0
  %temp.vect55.i = insertelement <16 x float> %temp.vect.i, float %189, i32 1
  %temp.vect56.i = insertelement <16 x float> %temp.vect55.i, float %190, i32 2
  %temp.vect57.i = insertelement <16 x float> %temp.vect56.i, float %191, i32 3
  %temp.vect58.i = insertelement <16 x float> %temp.vect57.i, float %192, i32 4
  %temp.vect59.i = insertelement <16 x float> %temp.vect58.i, float %193, i32 5
  %temp.vect60.i = insertelement <16 x float> %temp.vect59.i, float %194, i32 6
  %temp.vect61.i = insertelement <16 x float> %temp.vect60.i, float %195, i32 7
  %temp.vect62.i = insertelement <16 x float> %temp.vect61.i, float %196, i32 8
  %temp.vect63.i = insertelement <16 x float> %temp.vect62.i, float %197, i32 9
  %temp.vect64.i = insertelement <16 x float> %temp.vect63.i, float %198, i32 10
  %temp.vect65.i = insertelement <16 x float> %temp.vect64.i, float %199, i32 11
  %temp.vect66.i = insertelement <16 x float> %temp.vect65.i, float %200, i32 12
  %temp.vect67.i = insertelement <16 x float> %temp.vect66.i, float %201, i32 13
  %temp.vect68.i = insertelement <16 x float> %temp.vect67.i, float %202, i32 14
  %temp.vect69.i = insertelement <16 x float> %temp.vect68.i, float %203, i32 15
  %204 = load float addrspace(1)* %52, align 4
  %205 = load float addrspace(1)* %53, align 4
  %206 = load float addrspace(1)* %54, align 4
  %207 = load float addrspace(1)* %55, align 4
  %208 = load float addrspace(1)* %56, align 4
  %209 = load float addrspace(1)* %57, align 4
  %210 = load float addrspace(1)* %58, align 4
  %211 = load float addrspace(1)* %59, align 4
  %212 = load float addrspace(1)* %60, align 4
  %213 = load float addrspace(1)* %61, align 4
  %214 = load float addrspace(1)* %62, align 4
  %215 = load float addrspace(1)* %63, align 4
  %216 = load float addrspace(1)* %64, align 4
  %217 = load float addrspace(1)* %65, align 4
  %218 = load float addrspace(1)* %66, align 4
  %219 = load float addrspace(1)* %67, align 4
  %temp.vect104.i = insertelement <16 x float> undef, float %204, i32 0
  %temp.vect105.i = insertelement <16 x float> %temp.vect104.i, float %205, i32 1
  %temp.vect106.i = insertelement <16 x float> %temp.vect105.i, float %206, i32 2
  %temp.vect107.i = insertelement <16 x float> %temp.vect106.i, float %207, i32 3
  %temp.vect108.i = insertelement <16 x float> %temp.vect107.i, float %208, i32 4
  %temp.vect109.i = insertelement <16 x float> %temp.vect108.i, float %209, i32 5
  %temp.vect110.i = insertelement <16 x float> %temp.vect109.i, float %210, i32 6
  %temp.vect111.i = insertelement <16 x float> %temp.vect110.i, float %211, i32 7
  %temp.vect112.i = insertelement <16 x float> %temp.vect111.i, float %212, i32 8
  %temp.vect113.i = insertelement <16 x float> %temp.vect112.i, float %213, i32 9
  %temp.vect114.i = insertelement <16 x float> %temp.vect113.i, float %214, i32 10
  %temp.vect115.i = insertelement <16 x float> %temp.vect114.i, float %215, i32 11
  %temp.vect116.i = insertelement <16 x float> %temp.vect115.i, float %216, i32 12
  %temp.vect117.i = insertelement <16 x float> %temp.vect116.i, float %217, i32 13
  %temp.vect118.i = insertelement <16 x float> %temp.vect117.i, float %218, i32 14
  %temp.vect119.i = insertelement <16 x float> %temp.vect118.i, float %219, i32 15
  %220 = load float addrspace(1)* %scevgep97.1.i, align 4
  %temp102.i = insertelement <16 x float> undef, float %220, i32 0
  %vector103.i = shufflevector <16 x float> %temp102.i, <16 x float> undef, <16 x i32> zeroinitializer
  %221 = load float* %171, align 4
  %222 = load float* %172, align 4
  %223 = load float* %173, align 4
  %224 = load float* %174, align 4
  %225 = load float* %175, align 4
  %226 = load float* %176, align 4
  %227 = load float* %177, align 4
  %228 = load float* %178, align 4
  %229 = load float* %179, align 4
  %230 = load float* %180, align 4
  %231 = load float* %181, align 4
  %232 = load float* %182, align 4
  %233 = load float* %183, align 4
  %234 = load float* %184, align 4
  %235 = load float* %185, align 4
  %236 = load float* %186, align 4
  %temp.vect120.i = insertelement <16 x float> undef, float %221, i32 0
  %temp.vect121.i = insertelement <16 x float> %temp.vect120.i, float %222, i32 1
  %temp.vect122.i = insertelement <16 x float> %temp.vect121.i, float %223, i32 2
  %temp.vect123.i = insertelement <16 x float> %temp.vect122.i, float %224, i32 3
  %temp.vect124.i = insertelement <16 x float> %temp.vect123.i, float %225, i32 4
  %temp.vect125.i = insertelement <16 x float> %temp.vect124.i, float %226, i32 5
  %temp.vect126.i = insertelement <16 x float> %temp.vect125.i, float %227, i32 6
  %temp.vect127.i = insertelement <16 x float> %temp.vect126.i, float %228, i32 7
  %temp.vect128.i = insertelement <16 x float> %temp.vect127.i, float %229, i32 8
  %temp.vect129.i = insertelement <16 x float> %temp.vect128.i, float %230, i32 9
  %temp.vect130.i = insertelement <16 x float> %temp.vect129.i, float %231, i32 10
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %232, i32 11
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %233, i32 12
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %234, i32 13
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %235, i32 14
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %236, i32 15
  %237 = fmul <16 x float> %vector54.i, %temp.vect69.i
  %238 = fmul <16 x float> %vector103.i, %temp.vect119.i
  %239 = fadd <16 x float> %temp.vect135.i, %237
  %240 = load float addrspace(1)* %scevgep97.2.i, align 4
  %temp136.i = insertelement <16 x float> undef, float %240, i32 0
  %vector137.i = shufflevector <16 x float> %temp136.i, <16 x float> undef, <16 x i32> zeroinitializer
  %241 = load float addrspace(1)* %70, align 4
  %242 = load float addrspace(1)* %71, align 4
  %243 = load float addrspace(1)* %72, align 4
  %244 = load float addrspace(1)* %73, align 4
  %245 = load float addrspace(1)* %74, align 4
  %246 = load float addrspace(1)* %75, align 4
  %247 = load float addrspace(1)* %76, align 4
  %248 = load float addrspace(1)* %77, align 4
  %249 = load float addrspace(1)* %78, align 4
  %250 = load float addrspace(1)* %79, align 4
  %251 = load float addrspace(1)* %80, align 4
  %252 = load float addrspace(1)* %81, align 4
  %253 = load float addrspace(1)* %82, align 4
  %254 = load float addrspace(1)* %83, align 4
  %255 = load float addrspace(1)* %84, align 4
  %256 = load float addrspace(1)* %85, align 4
  %temp.vect138.i = insertelement <16 x float> undef, float %241, i32 0
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %242, i32 1
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %243, i32 2
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %244, i32 3
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %245, i32 4
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %246, i32 5
  %temp.vect144.i = insertelement <16 x float> %temp.vect143.i, float %247, i32 6
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %248, i32 7
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %249, i32 8
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %250, i32 9
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %251, i32 10
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %252, i32 11
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %253, i32 12
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %254, i32 13
  %temp.vect152.i = insertelement <16 x float> %temp.vect151.i, float %255, i32 14
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %256, i32 15
  %257 = load float addrspace(1)* %86, align 4
  %258 = load float addrspace(1)* %87, align 4
  %259 = load float addrspace(1)* %88, align 4
  %260 = load float addrspace(1)* %89, align 4
  %261 = load float addrspace(1)* %90, align 4
  %262 = load float addrspace(1)* %91, align 4
  %263 = load float addrspace(1)* %92, align 4
  %264 = load float addrspace(1)* %93, align 4
  %265 = load float addrspace(1)* %94, align 4
  %266 = load float addrspace(1)* %95, align 4
  %267 = load float addrspace(1)* %96, align 4
  %268 = load float addrspace(1)* %97, align 4
  %269 = load float addrspace(1)* %98, align 4
  %270 = load float addrspace(1)* %99, align 4
  %271 = load float addrspace(1)* %100, align 4
  %272 = load float addrspace(1)* %101, align 4
  %temp.vect188.i = insertelement <16 x float> undef, float %257, i32 0
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %258, i32 1
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %259, i32 2
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %260, i32 3
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %261, i32 4
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %262, i32 5
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %263, i32 6
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %264, i32 7
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %265, i32 8
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %266, i32 9
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %267, i32 10
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %268, i32 11
  %temp.vect200.i = insertelement <16 x float> %temp.vect199.i, float %269, i32 12
  %temp.vect201.i = insertelement <16 x float> %temp.vect200.i, float %270, i32 13
  %temp.vect202.i = insertelement <16 x float> %temp.vect201.i, float %271, i32 14
  %temp.vect203.i = insertelement <16 x float> %temp.vect202.i, float %272, i32 15
  %273 = load float addrspace(1)* %scevgep97.3.i, align 4
  %temp186.i = insertelement <16 x float> undef, float %273, i32 0
  %vector187.i = shufflevector <16 x float> %temp186.i, <16 x float> undef, <16 x i32> zeroinitializer
  %274 = fadd <16 x float> %239, %238
  %275 = fmul <16 x float> %vector137.i, %temp.vect153.i
  %276 = fmul <16 x float> %vector187.i, %temp.vect203.i
  %277 = fadd <16 x float> %274, %275
  %278 = load float addrspace(1)* %scevgep97.4.i, align 4
  %temp204.i = insertelement <16 x float> undef, float %278, i32 0
  %vector205.i = shufflevector <16 x float> %temp204.i, <16 x float> undef, <16 x i32> zeroinitializer
  %279 = load float addrspace(1)* %104, align 4
  %280 = load float addrspace(1)* %105, align 4
  %281 = load float addrspace(1)* %106, align 4
  %282 = load float addrspace(1)* %107, align 4
  %283 = load float addrspace(1)* %108, align 4
  %284 = load float addrspace(1)* %109, align 4
  %285 = load float addrspace(1)* %110, align 4
  %286 = load float addrspace(1)* %111, align 4
  %287 = load float addrspace(1)* %112, align 4
  %288 = load float addrspace(1)* %113, align 4
  %289 = load float addrspace(1)* %114, align 4
  %290 = load float addrspace(1)* %115, align 4
  %291 = load float addrspace(1)* %116, align 4
  %292 = load float addrspace(1)* %117, align 4
  %293 = load float addrspace(1)* %118, align 4
  %294 = load float addrspace(1)* %119, align 4
  %temp.vect206.i = insertelement <16 x float> undef, float %279, i32 0
  %temp.vect207.i = insertelement <16 x float> %temp.vect206.i, float %280, i32 1
  %temp.vect208.i = insertelement <16 x float> %temp.vect207.i, float %281, i32 2
  %temp.vect209.i = insertelement <16 x float> %temp.vect208.i, float %282, i32 3
  %temp.vect210.i = insertelement <16 x float> %temp.vect209.i, float %283, i32 4
  %temp.vect211.i = insertelement <16 x float> %temp.vect210.i, float %284, i32 5
  %temp.vect212.i = insertelement <16 x float> %temp.vect211.i, float %285, i32 6
  %temp.vect213.i = insertelement <16 x float> %temp.vect212.i, float %286, i32 7
  %temp.vect214.i = insertelement <16 x float> %temp.vect213.i, float %287, i32 8
  %temp.vect215.i = insertelement <16 x float> %temp.vect214.i, float %288, i32 9
  %temp.vect216.i = insertelement <16 x float> %temp.vect215.i, float %289, i32 10
  %temp.vect217.i = insertelement <16 x float> %temp.vect216.i, float %290, i32 11
  %temp.vect218.i = insertelement <16 x float> %temp.vect217.i, float %291, i32 12
  %temp.vect219.i = insertelement <16 x float> %temp.vect218.i, float %292, i32 13
  %temp.vect220.i = insertelement <16 x float> %temp.vect219.i, float %293, i32 14
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %294, i32 15
  %295 = load float addrspace(1)* %120, align 4
  %296 = load float addrspace(1)* %121, align 4
  %297 = load float addrspace(1)* %122, align 4
  %298 = load float addrspace(1)* %123, align 4
  %299 = load float addrspace(1)* %124, align 4
  %300 = load float addrspace(1)* %125, align 4
  %301 = load float addrspace(1)* %126, align 4
  %302 = load float addrspace(1)* %127, align 4
  %303 = load float addrspace(1)* %128, align 4
  %304 = load float addrspace(1)* %129, align 4
  %305 = load float addrspace(1)* %130, align 4
  %306 = load float addrspace(1)* %131, align 4
  %307 = load float addrspace(1)* %132, align 4
  %308 = load float addrspace(1)* %133, align 4
  %309 = load float addrspace(1)* %134, align 4
  %310 = load float addrspace(1)* %135, align 4
  %temp.vect256.i = insertelement <16 x float> undef, float %295, i32 0
  %temp.vect257.i = insertelement <16 x float> %temp.vect256.i, float %296, i32 1
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %297, i32 2
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %298, i32 3
  %temp.vect260.i = insertelement <16 x float> %temp.vect259.i, float %299, i32 4
  %temp.vect261.i = insertelement <16 x float> %temp.vect260.i, float %300, i32 5
  %temp.vect262.i = insertelement <16 x float> %temp.vect261.i, float %301, i32 6
  %temp.vect263.i = insertelement <16 x float> %temp.vect262.i, float %302, i32 7
  %temp.vect264.i = insertelement <16 x float> %temp.vect263.i, float %303, i32 8
  %temp.vect265.i = insertelement <16 x float> %temp.vect264.i, float %304, i32 9
  %temp.vect266.i = insertelement <16 x float> %temp.vect265.i, float %305, i32 10
  %temp.vect267.i = insertelement <16 x float> %temp.vect266.i, float %306, i32 11
  %temp.vect268.i = insertelement <16 x float> %temp.vect267.i, float %307, i32 12
  %temp.vect269.i = insertelement <16 x float> %temp.vect268.i, float %308, i32 13
  %temp.vect270.i = insertelement <16 x float> %temp.vect269.i, float %309, i32 14
  %temp.vect271.i = insertelement <16 x float> %temp.vect270.i, float %310, i32 15
  %311 = load float addrspace(1)* %scevgep97.5.i, align 4
  %temp254.i = insertelement <16 x float> undef, float %311, i32 0
  %vector255.i = shufflevector <16 x float> %temp254.i, <16 x float> undef, <16 x i32> zeroinitializer
  %312 = fadd <16 x float> %277, %276
  %313 = fmul <16 x float> %vector205.i, %temp.vect221.i
  %314 = fmul <16 x float> %vector255.i, %temp.vect271.i
  %315 = fadd <16 x float> %312, %313
  %316 = load float addrspace(1)* %scevgep97.6.i, align 4
  %temp272.i = insertelement <16 x float> undef, float %316, i32 0
  %vector273.i = shufflevector <16 x float> %temp272.i, <16 x float> undef, <16 x i32> zeroinitializer
  %317 = load float addrspace(1)* %138, align 4
  %318 = load float addrspace(1)* %139, align 4
  %319 = load float addrspace(1)* %140, align 4
  %320 = load float addrspace(1)* %141, align 4
  %321 = load float addrspace(1)* %142, align 4
  %322 = load float addrspace(1)* %143, align 4
  %323 = load float addrspace(1)* %144, align 4
  %324 = load float addrspace(1)* %145, align 4
  %325 = load float addrspace(1)* %146, align 4
  %326 = load float addrspace(1)* %147, align 4
  %327 = load float addrspace(1)* %148, align 4
  %328 = load float addrspace(1)* %149, align 4
  %329 = load float addrspace(1)* %150, align 4
  %330 = load float addrspace(1)* %151, align 4
  %331 = load float addrspace(1)* %152, align 4
  %332 = load float addrspace(1)* %153, align 4
  %temp.vect274.i = insertelement <16 x float> undef, float %317, i32 0
  %temp.vect275.i = insertelement <16 x float> %temp.vect274.i, float %318, i32 1
  %temp.vect276.i = insertelement <16 x float> %temp.vect275.i, float %319, i32 2
  %temp.vect277.i = insertelement <16 x float> %temp.vect276.i, float %320, i32 3
  %temp.vect278.i = insertelement <16 x float> %temp.vect277.i, float %321, i32 4
  %temp.vect279.i = insertelement <16 x float> %temp.vect278.i, float %322, i32 5
  %temp.vect280.i = insertelement <16 x float> %temp.vect279.i, float %323, i32 6
  %temp.vect281.i = insertelement <16 x float> %temp.vect280.i, float %324, i32 7
  %temp.vect282.i = insertelement <16 x float> %temp.vect281.i, float %325, i32 8
  %temp.vect283.i = insertelement <16 x float> %temp.vect282.i, float %326, i32 9
  %temp.vect284.i = insertelement <16 x float> %temp.vect283.i, float %327, i32 10
  %temp.vect285.i = insertelement <16 x float> %temp.vect284.i, float %328, i32 11
  %temp.vect286.i = insertelement <16 x float> %temp.vect285.i, float %329, i32 12
  %temp.vect287.i = insertelement <16 x float> %temp.vect286.i, float %330, i32 13
  %temp.vect288.i = insertelement <16 x float> %temp.vect287.i, float %331, i32 14
  %temp.vect289.i = insertelement <16 x float> %temp.vect288.i, float %332, i32 15
  %333 = load float addrspace(1)* %154, align 4
  %334 = load float addrspace(1)* %155, align 4
  %335 = load float addrspace(1)* %156, align 4
  %336 = load float addrspace(1)* %157, align 4
  %337 = load float addrspace(1)* %158, align 4
  %338 = load float addrspace(1)* %159, align 4
  %339 = load float addrspace(1)* %160, align 4
  %340 = load float addrspace(1)* %161, align 4
  %341 = load float addrspace(1)* %162, align 4
  %342 = load float addrspace(1)* %163, align 4
  %343 = load float addrspace(1)* %164, align 4
  %344 = load float addrspace(1)* %165, align 4
  %345 = load float addrspace(1)* %166, align 4
  %346 = load float addrspace(1)* %167, align 4
  %347 = load float addrspace(1)* %168, align 4
  %348 = load float addrspace(1)* %169, align 4
  %temp.vect292.i = insertelement <16 x float> undef, float %333, i32 0
  %temp.vect293.i = insertelement <16 x float> %temp.vect292.i, float %334, i32 1
  %temp.vect294.i = insertelement <16 x float> %temp.vect293.i, float %335, i32 2
  %temp.vect295.i = insertelement <16 x float> %temp.vect294.i, float %336, i32 3
  %temp.vect296.i = insertelement <16 x float> %temp.vect295.i, float %337, i32 4
  %temp.vect297.i = insertelement <16 x float> %temp.vect296.i, float %338, i32 5
  %temp.vect298.i = insertelement <16 x float> %temp.vect297.i, float %339, i32 6
  %temp.vect299.i = insertelement <16 x float> %temp.vect298.i, float %340, i32 7
  %temp.vect300.i = insertelement <16 x float> %temp.vect299.i, float %341, i32 8
  %temp.vect301.i = insertelement <16 x float> %temp.vect300.i, float %342, i32 9
  %temp.vect302.i = insertelement <16 x float> %temp.vect301.i, float %343, i32 10
  %temp.vect303.i = insertelement <16 x float> %temp.vect302.i, float %344, i32 11
  %temp.vect304.i = insertelement <16 x float> %temp.vect303.i, float %345, i32 12
  %temp.vect305.i = insertelement <16 x float> %temp.vect304.i, float %346, i32 13
  %temp.vect306.i = insertelement <16 x float> %temp.vect305.i, float %347, i32 14
  %temp.vect307.i = insertelement <16 x float> %temp.vect306.i, float %348, i32 15
  %349 = load float addrspace(1)* %scevgep97.7.i, align 4
  %temp290.i = insertelement <16 x float> undef, float %349, i32 0
  %vector291.i = shufflevector <16 x float> %temp290.i, <16 x float> undef, <16 x i32> zeroinitializer
  %350 = fadd <16 x float> %315, %314
  %351 = fmul <16 x float> %vector273.i, %temp.vect289.i
  %352 = fmul <16 x float> %vector291.i, %temp.vect307.i
  %353 = fadd <16 x float> %350, %351
  %354 = fadd <16 x float> %353, %352
  %extract308.i = extractelement <16 x float> %354, i32 0
  %extract309.i = extractelement <16 x float> %354, i32 1
  %extract310.i = extractelement <16 x float> %354, i32 2
  %extract311.i = extractelement <16 x float> %354, i32 3
  %extract312.i = extractelement <16 x float> %354, i32 4
  %extract313.i = extractelement <16 x float> %354, i32 5
  %extract314.i = extractelement <16 x float> %354, i32 6
  %extract315.i = extractelement <16 x float> %354, i32 7
  %extract316.i = extractelement <16 x float> %354, i32 8
  %extract317.i = extractelement <16 x float> %354, i32 9
  %extract318.i = extractelement <16 x float> %354, i32 10
  %extract319.i = extractelement <16 x float> %354, i32 11
  %extract320.i = extractelement <16 x float> %354, i32 12
  %extract321.i = extractelement <16 x float> %354, i32 13
  %extract322.i = extractelement <16 x float> %354, i32 14
  %extract323.i = extractelement <16 x float> %354, i32 15
  store float %extract308.i, float* %171, align 4
  store float %extract309.i, float* %172, align 4
  store float %extract310.i, float* %173, align 4
  store float %extract311.i, float* %174, align 4
  store float %extract312.i, float* %175, align 4
  store float %extract313.i, float* %176, align 4
  store float %extract314.i, float* %177, align 4
  store float %extract315.i, float* %178, align 4
  store float %extract316.i, float* %179, align 4
  store float %extract317.i, float* %180, align 4
  store float %extract318.i, float* %181, align 4
  store float %extract319.i, float* %182, align 4
  store float %extract320.i, float* %183, align 4
  store float %extract321.i, float* %184, align 4
  store float %extract322.i, float* %185, align 4
  store float %extract323.i, float* %186, align 4
  %indvar.next94.i = add i64 %indvar93.i, 1
  %exitcond111.i = icmp eq i64 %indvar.next94.i, 8
  br i1 %exitcond111.i, label %._crit_edge23.i, label %170

._crit_edge23.i:                                  ; preds = %170
  %indvar.next99.i = add i64 %indvar98.i, 1
  %exitcond130.i = icmp eq i64 %indvar.next99.i, 8
  br i1 %exitcond130.i, label %bb.nph12.i, label %bb.nph22.i

bb.nph12.i:                                       ; preds = %._crit_edge23.i
  %tmp67.i = trunc i64 %33 to i32
  %tmp68.i = mul i32 %tmp67.i, %10
  %temp325.i = insertelement <16 x i32> undef, i32 %tmp68.i, i32 0
  %vector326.i = shufflevector <16 x i32> %temp325.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp69324.i = trunc <16 x i64> %28 to <16 x i32>
  %tmp70327.i = add <16 x i32> %vector326.i, %tmp69324.i
  %tmp71328.i = shl <16 x i32> %tmp70327.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp72329.i = zext <16 x i32> %tmp71328.i to <16 x i64>
  %"&(pSB[currWI].offset)649.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset650.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)649.i"
  %CastToValueType651.i = bitcast i8* %"&pSB[currWI].offset650.i" to [64 x float]*
  %"&(pSB[currWI].offset)689.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset690.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)689.i"
  %CastToValueType691.i = bitcast i8* %"&pSB[currWI].offset690.i" to [64 x float]*
  %"&(pSB[currWI].offset)729.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset730.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)729.i"
  %CastToValueType731.i = bitcast i8* %"&pSB[currWI].offset730.i" to [64 x float]*
  %"&(pSB[currWI].offset)769.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset770.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)769.i"
  %CastToValueType771.i = bitcast i8* %"&pSB[currWI].offset770.i" to [64 x float]*
  %"&(pSB[currWI].offset)809.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset810.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)809.i"
  %CastToValueType811.i = bitcast i8* %"&pSB[currWI].offset810.i" to [64 x float]*
  %"&(pSB[currWI].offset)849.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset850.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)849.i"
  %CastToValueType851.i = bitcast i8* %"&pSB[currWI].offset850.i" to [64 x float]*
  %"&(pSB[currWI].offset)889.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset890.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)889.i"
  %CastToValueType891.i = bitcast i8* %"&pSB[currWI].offset890.i" to [64 x float]*
  %"&(pSB[currWI].offset)929.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset930.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)929.i"
  %CastToValueType931.i = bitcast i8* %"&pSB[currWI].offset930.i" to [64 x float]*
  %"&(pSB[currWI].offset)969.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset970.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)969.i"
  %CastToValueType971.i = bitcast i8* %"&pSB[currWI].offset970.i" to [64 x float]*
  %"&(pSB[currWI].offset)1009.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset1010.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1009.i"
  %CastToValueType1011.i = bitcast i8* %"&pSB[currWI].offset1010.i" to [64 x float]*
  %"&(pSB[currWI].offset)1049.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1050.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1049.i"
  %CastToValueType1051.i = bitcast i8* %"&pSB[currWI].offset1050.i" to [64 x float]*
  %"&(pSB[currWI].offset)1089.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1090.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1089.i"
  %CastToValueType1091.i = bitcast i8* %"&pSB[currWI].offset1090.i" to [64 x float]*
  %"&(pSB[currWI].offset)1129.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1130.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1129.i"
  %CastToValueType1131.i = bitcast i8* %"&pSB[currWI].offset1130.i" to [64 x float]*
  %"&(pSB[currWI].offset)1169.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1170.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1169.i"
  %CastToValueType1171.i = bitcast i8* %"&pSB[currWI].offset1170.i" to [64 x float]*
  %"&(pSB[currWI].offset)1209.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1210.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1209.i"
  %CastToValueType1211.i = bitcast i8* %"&pSB[currWI].offset1210.i" to [64 x float]*
  %"&(pSB[currWI].offset)1249.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1250.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1249.i"
  %CastToValueType1251.i = bitcast i8* %"&pSB[currWI].offset1250.i" to [64 x float]*
  %"&(pSB[currWI].offset)645.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset646.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)645.i"
  %CastToValueType647.i = bitcast i8* %"&pSB[currWI].offset646.i" to [64 x float]*
  %"&(pSB[currWI].offset)685.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset686.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)685.i"
  %CastToValueType687.i = bitcast i8* %"&pSB[currWI].offset686.i" to [64 x float]*
  %"&(pSB[currWI].offset)725.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset726.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)725.i"
  %CastToValueType727.i = bitcast i8* %"&pSB[currWI].offset726.i" to [64 x float]*
  %"&(pSB[currWI].offset)765.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset766.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)765.i"
  %CastToValueType767.i = bitcast i8* %"&pSB[currWI].offset766.i" to [64 x float]*
  %"&(pSB[currWI].offset)805.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset806.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)805.i"
  %CastToValueType807.i = bitcast i8* %"&pSB[currWI].offset806.i" to [64 x float]*
  %"&(pSB[currWI].offset)845.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset846.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)845.i"
  %CastToValueType847.i = bitcast i8* %"&pSB[currWI].offset846.i" to [64 x float]*
  %"&(pSB[currWI].offset)885.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset886.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)885.i"
  %CastToValueType887.i = bitcast i8* %"&pSB[currWI].offset886.i" to [64 x float]*
  %"&(pSB[currWI].offset)925.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset926.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)925.i"
  %CastToValueType927.i = bitcast i8* %"&pSB[currWI].offset926.i" to [64 x float]*
  %"&(pSB[currWI].offset)965.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset966.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)965.i"
  %CastToValueType967.i = bitcast i8* %"&pSB[currWI].offset966.i" to [64 x float]*
  %"&(pSB[currWI].offset)1005.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset1006.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1005.i"
  %CastToValueType1007.i = bitcast i8* %"&pSB[currWI].offset1006.i" to [64 x float]*
  %"&(pSB[currWI].offset)1045.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1046.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1045.i"
  %CastToValueType1047.i = bitcast i8* %"&pSB[currWI].offset1046.i" to [64 x float]*
  %"&(pSB[currWI].offset)1085.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1086.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1085.i"
  %CastToValueType1087.i = bitcast i8* %"&pSB[currWI].offset1086.i" to [64 x float]*
  %"&(pSB[currWI].offset)1125.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1126.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1125.i"
  %CastToValueType1127.i = bitcast i8* %"&pSB[currWI].offset1126.i" to [64 x float]*
  %"&(pSB[currWI].offset)1165.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1166.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1165.i"
  %CastToValueType1167.i = bitcast i8* %"&pSB[currWI].offset1166.i" to [64 x float]*
  %"&(pSB[currWI].offset)1205.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1206.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1205.i"
  %CastToValueType1207.i = bitcast i8* %"&pSB[currWI].offset1206.i" to [64 x float]*
  %"&(pSB[currWI].offset)1245.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1246.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1245.i"
  %CastToValueType1247.i = bitcast i8* %"&pSB[currWI].offset1246.i" to [64 x float]*
  %"&(pSB[currWI].offset)641.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset642.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)641.i"
  %CastToValueType643.i = bitcast i8* %"&pSB[currWI].offset642.i" to [64 x float]*
  %"&(pSB[currWI].offset)681.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset682.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)681.i"
  %CastToValueType683.i = bitcast i8* %"&pSB[currWI].offset682.i" to [64 x float]*
  %"&(pSB[currWI].offset)721.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset722.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)721.i"
  %CastToValueType723.i = bitcast i8* %"&pSB[currWI].offset722.i" to [64 x float]*
  %"&(pSB[currWI].offset)761.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset762.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)761.i"
  %CastToValueType763.i = bitcast i8* %"&pSB[currWI].offset762.i" to [64 x float]*
  %"&(pSB[currWI].offset)801.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset802.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)801.i"
  %CastToValueType803.i = bitcast i8* %"&pSB[currWI].offset802.i" to [64 x float]*
  %"&(pSB[currWI].offset)841.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset842.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)841.i"
  %CastToValueType843.i = bitcast i8* %"&pSB[currWI].offset842.i" to [64 x float]*
  %"&(pSB[currWI].offset)881.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset882.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)881.i"
  %CastToValueType883.i = bitcast i8* %"&pSB[currWI].offset882.i" to [64 x float]*
  %"&(pSB[currWI].offset)921.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset922.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)921.i"
  %CastToValueType923.i = bitcast i8* %"&pSB[currWI].offset922.i" to [64 x float]*
  %"&(pSB[currWI].offset)961.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset962.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)961.i"
  %CastToValueType963.i = bitcast i8* %"&pSB[currWI].offset962.i" to [64 x float]*
  %"&(pSB[currWI].offset)1001.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset1002.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1001.i"
  %CastToValueType1003.i = bitcast i8* %"&pSB[currWI].offset1002.i" to [64 x float]*
  %"&(pSB[currWI].offset)1041.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1042.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1041.i"
  %CastToValueType1043.i = bitcast i8* %"&pSB[currWI].offset1042.i" to [64 x float]*
  %"&(pSB[currWI].offset)1081.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1082.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1081.i"
  %CastToValueType1083.i = bitcast i8* %"&pSB[currWI].offset1082.i" to [64 x float]*
  %"&(pSB[currWI].offset)1121.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1122.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1121.i"
  %CastToValueType1123.i = bitcast i8* %"&pSB[currWI].offset1122.i" to [64 x float]*
  %"&(pSB[currWI].offset)1161.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1162.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1161.i"
  %CastToValueType1163.i = bitcast i8* %"&pSB[currWI].offset1162.i" to [64 x float]*
  %"&(pSB[currWI].offset)1201.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1202.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1201.i"
  %CastToValueType1203.i = bitcast i8* %"&pSB[currWI].offset1202.i" to [64 x float]*
  %"&(pSB[currWI].offset)1241.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1242.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1241.i"
  %CastToValueType1243.i = bitcast i8* %"&pSB[currWI].offset1242.i" to [64 x float]*
  %"&(pSB[currWI].offset)637.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset638.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)637.i"
  %CastToValueType639.i = bitcast i8* %"&pSB[currWI].offset638.i" to [64 x float]*
  %"&(pSB[currWI].offset)677.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset678.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)677.i"
  %CastToValueType679.i = bitcast i8* %"&pSB[currWI].offset678.i" to [64 x float]*
  %"&(pSB[currWI].offset)717.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset718.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)717.i"
  %CastToValueType719.i = bitcast i8* %"&pSB[currWI].offset718.i" to [64 x float]*
  %"&(pSB[currWI].offset)757.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset758.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)757.i"
  %CastToValueType759.i = bitcast i8* %"&pSB[currWI].offset758.i" to [64 x float]*
  %"&(pSB[currWI].offset)797.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset798.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)797.i"
  %CastToValueType799.i = bitcast i8* %"&pSB[currWI].offset798.i" to [64 x float]*
  %"&(pSB[currWI].offset)837.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset838.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)837.i"
  %CastToValueType839.i = bitcast i8* %"&pSB[currWI].offset838.i" to [64 x float]*
  %"&(pSB[currWI].offset)877.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset878.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)877.i"
  %CastToValueType879.i = bitcast i8* %"&pSB[currWI].offset878.i" to [64 x float]*
  %"&(pSB[currWI].offset)917.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset918.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)917.i"
  %CastToValueType919.i = bitcast i8* %"&pSB[currWI].offset918.i" to [64 x float]*
  %"&(pSB[currWI].offset)957.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset958.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)957.i"
  %CastToValueType959.i = bitcast i8* %"&pSB[currWI].offset958.i" to [64 x float]*
  %"&(pSB[currWI].offset)997.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset998.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)997.i"
  %CastToValueType999.i = bitcast i8* %"&pSB[currWI].offset998.i" to [64 x float]*
  %"&(pSB[currWI].offset)1037.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1038.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1037.i"
  %CastToValueType1039.i = bitcast i8* %"&pSB[currWI].offset1038.i" to [64 x float]*
  %"&(pSB[currWI].offset)1077.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1078.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1077.i"
  %CastToValueType1079.i = bitcast i8* %"&pSB[currWI].offset1078.i" to [64 x float]*
  %"&(pSB[currWI].offset)1117.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1118.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1117.i"
  %CastToValueType1119.i = bitcast i8* %"&pSB[currWI].offset1118.i" to [64 x float]*
  %"&(pSB[currWI].offset)1157.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1158.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1157.i"
  %CastToValueType1159.i = bitcast i8* %"&pSB[currWI].offset1158.i" to [64 x float]*
  %"&(pSB[currWI].offset)1197.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1198.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1197.i"
  %CastToValueType1199.i = bitcast i8* %"&pSB[currWI].offset1198.i" to [64 x float]*
  %"&(pSB[currWI].offset)1237.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1238.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1237.i"
  %CastToValueType1239.i = bitcast i8* %"&pSB[currWI].offset1238.i" to [64 x float]*
  %"&(pSB[currWI].offset)633.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset634.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)633.i"
  %CastToValueType635.i = bitcast i8* %"&pSB[currWI].offset634.i" to [64 x float]*
  %"&(pSB[currWI].offset)673.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset674.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)673.i"
  %CastToValueType675.i = bitcast i8* %"&pSB[currWI].offset674.i" to [64 x float]*
  %"&(pSB[currWI].offset)713.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset714.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)713.i"
  %CastToValueType715.i = bitcast i8* %"&pSB[currWI].offset714.i" to [64 x float]*
  %"&(pSB[currWI].offset)753.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset754.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)753.i"
  %CastToValueType755.i = bitcast i8* %"&pSB[currWI].offset754.i" to [64 x float]*
  %"&(pSB[currWI].offset)793.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset794.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)793.i"
  %CastToValueType795.i = bitcast i8* %"&pSB[currWI].offset794.i" to [64 x float]*
  %"&(pSB[currWI].offset)833.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset834.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)833.i"
  %CastToValueType835.i = bitcast i8* %"&pSB[currWI].offset834.i" to [64 x float]*
  %"&(pSB[currWI].offset)873.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset874.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)873.i"
  %CastToValueType875.i = bitcast i8* %"&pSB[currWI].offset874.i" to [64 x float]*
  %"&(pSB[currWI].offset)913.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset914.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)913.i"
  %CastToValueType915.i = bitcast i8* %"&pSB[currWI].offset914.i" to [64 x float]*
  %"&(pSB[currWI].offset)953.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset954.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)953.i"
  %CastToValueType955.i = bitcast i8* %"&pSB[currWI].offset954.i" to [64 x float]*
  %"&(pSB[currWI].offset)993.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset994.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)993.i"
  %CastToValueType995.i = bitcast i8* %"&pSB[currWI].offset994.i" to [64 x float]*
  %"&(pSB[currWI].offset)1033.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1034.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1033.i"
  %CastToValueType1035.i = bitcast i8* %"&pSB[currWI].offset1034.i" to [64 x float]*
  %"&(pSB[currWI].offset)1073.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1074.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1073.i"
  %CastToValueType1075.i = bitcast i8* %"&pSB[currWI].offset1074.i" to [64 x float]*
  %"&(pSB[currWI].offset)1113.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1114.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1113.i"
  %CastToValueType1115.i = bitcast i8* %"&pSB[currWI].offset1114.i" to [64 x float]*
  %"&(pSB[currWI].offset)1153.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1154.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1153.i"
  %CastToValueType1155.i = bitcast i8* %"&pSB[currWI].offset1154.i" to [64 x float]*
  %"&(pSB[currWI].offset)1193.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1194.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1193.i"
  %CastToValueType1195.i = bitcast i8* %"&pSB[currWI].offset1194.i" to [64 x float]*
  %"&(pSB[currWI].offset)1233.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1234.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1233.i"
  %CastToValueType1235.i = bitcast i8* %"&pSB[currWI].offset1234.i" to [64 x float]*
  %"&(pSB[currWI].offset)629.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset630.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)629.i"
  %CastToValueType631.i = bitcast i8* %"&pSB[currWI].offset630.i" to [64 x float]*
  %"&(pSB[currWI].offset)669.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset670.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)669.i"
  %CastToValueType671.i = bitcast i8* %"&pSB[currWI].offset670.i" to [64 x float]*
  %"&(pSB[currWI].offset)709.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset710.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)709.i"
  %CastToValueType711.i = bitcast i8* %"&pSB[currWI].offset710.i" to [64 x float]*
  %"&(pSB[currWI].offset)749.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset750.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)749.i"
  %CastToValueType751.i = bitcast i8* %"&pSB[currWI].offset750.i" to [64 x float]*
  %"&(pSB[currWI].offset)789.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset790.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)789.i"
  %CastToValueType791.i = bitcast i8* %"&pSB[currWI].offset790.i" to [64 x float]*
  %"&(pSB[currWI].offset)829.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset830.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)829.i"
  %CastToValueType831.i = bitcast i8* %"&pSB[currWI].offset830.i" to [64 x float]*
  %"&(pSB[currWI].offset)869.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset870.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)869.i"
  %CastToValueType871.i = bitcast i8* %"&pSB[currWI].offset870.i" to [64 x float]*
  %"&(pSB[currWI].offset)909.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset910.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)909.i"
  %CastToValueType911.i = bitcast i8* %"&pSB[currWI].offset910.i" to [64 x float]*
  %"&(pSB[currWI].offset)949.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset950.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)949.i"
  %CastToValueType951.i = bitcast i8* %"&pSB[currWI].offset950.i" to [64 x float]*
  %"&(pSB[currWI].offset)989.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset990.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)989.i"
  %CastToValueType991.i = bitcast i8* %"&pSB[currWI].offset990.i" to [64 x float]*
  %"&(pSB[currWI].offset)1029.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1030.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1029.i"
  %CastToValueType1031.i = bitcast i8* %"&pSB[currWI].offset1030.i" to [64 x float]*
  %"&(pSB[currWI].offset)1069.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1070.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1069.i"
  %CastToValueType1071.i = bitcast i8* %"&pSB[currWI].offset1070.i" to [64 x float]*
  %"&(pSB[currWI].offset)1109.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1110.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1109.i"
  %CastToValueType1111.i = bitcast i8* %"&pSB[currWI].offset1110.i" to [64 x float]*
  %"&(pSB[currWI].offset)1149.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1150.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1149.i"
  %CastToValueType1151.i = bitcast i8* %"&pSB[currWI].offset1150.i" to [64 x float]*
  %"&(pSB[currWI].offset)1189.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1190.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1189.i"
  %CastToValueType1191.i = bitcast i8* %"&pSB[currWI].offset1190.i" to [64 x float]*
  %"&(pSB[currWI].offset)1229.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1230.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1229.i"
  %CastToValueType1231.i = bitcast i8* %"&pSB[currWI].offset1230.i" to [64 x float]*
  %"&(pSB[currWI].offset)625.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset626.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)625.i"
  %CastToValueType627.i = bitcast i8* %"&pSB[currWI].offset626.i" to [64 x float]*
  %"&(pSB[currWI].offset)665.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset666.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)665.i"
  %CastToValueType667.i = bitcast i8* %"&pSB[currWI].offset666.i" to [64 x float]*
  %"&(pSB[currWI].offset)705.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset706.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)705.i"
  %CastToValueType707.i = bitcast i8* %"&pSB[currWI].offset706.i" to [64 x float]*
  %"&(pSB[currWI].offset)745.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset746.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)745.i"
  %CastToValueType747.i = bitcast i8* %"&pSB[currWI].offset746.i" to [64 x float]*
  %"&(pSB[currWI].offset)785.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset786.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)785.i"
  %CastToValueType787.i = bitcast i8* %"&pSB[currWI].offset786.i" to [64 x float]*
  %"&(pSB[currWI].offset)825.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset826.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)825.i"
  %CastToValueType827.i = bitcast i8* %"&pSB[currWI].offset826.i" to [64 x float]*
  %"&(pSB[currWI].offset)865.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset866.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)865.i"
  %CastToValueType867.i = bitcast i8* %"&pSB[currWI].offset866.i" to [64 x float]*
  %"&(pSB[currWI].offset)905.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset906.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)905.i"
  %CastToValueType907.i = bitcast i8* %"&pSB[currWI].offset906.i" to [64 x float]*
  %"&(pSB[currWI].offset)945.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset946.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)945.i"
  %CastToValueType947.i = bitcast i8* %"&pSB[currWI].offset946.i" to [64 x float]*
  %"&(pSB[currWI].offset)985.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset986.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)985.i"
  %CastToValueType987.i = bitcast i8* %"&pSB[currWI].offset986.i" to [64 x float]*
  %"&(pSB[currWI].offset)1025.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1026.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1025.i"
  %CastToValueType1027.i = bitcast i8* %"&pSB[currWI].offset1026.i" to [64 x float]*
  %"&(pSB[currWI].offset)1065.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1066.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1065.i"
  %CastToValueType1067.i = bitcast i8* %"&pSB[currWI].offset1066.i" to [64 x float]*
  %"&(pSB[currWI].offset)1105.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1106.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1105.i"
  %CastToValueType1107.i = bitcast i8* %"&pSB[currWI].offset1106.i" to [64 x float]*
  %"&(pSB[currWI].offset)1145.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1146.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1145.i"
  %CastToValueType1147.i = bitcast i8* %"&pSB[currWI].offset1146.i" to [64 x float]*
  %"&(pSB[currWI].offset)1185.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1186.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1185.i"
  %CastToValueType1187.i = bitcast i8* %"&pSB[currWI].offset1186.i" to [64 x float]*
  %"&(pSB[currWI].offset)1225.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1226.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1225.i"
  %CastToValueType1227.i = bitcast i8* %"&pSB[currWI].offset1226.i" to [64 x float]*
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to [64 x float]*
  %"&(pSB[currWI].offset)661.i" = add nuw i64 %CurrSBIndex..0.i, 1024
  %"&pSB[currWI].offset662.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)661.i"
  %CastToValueType663.i = bitcast i8* %"&pSB[currWI].offset662.i" to [64 x float]*
  %"&(pSB[currWI].offset)701.i" = add nuw i64 %CurrSBIndex..0.i, 1280
  %"&pSB[currWI].offset702.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)701.i"
  %CastToValueType703.i = bitcast i8* %"&pSB[currWI].offset702.i" to [64 x float]*
  %"&(pSB[currWI].offset)741.i" = add nuw i64 %CurrSBIndex..0.i, 1536
  %"&pSB[currWI].offset742.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)741.i"
  %CastToValueType743.i = bitcast i8* %"&pSB[currWI].offset742.i" to [64 x float]*
  %"&(pSB[currWI].offset)781.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  %"&pSB[currWI].offset782.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)781.i"
  %CastToValueType783.i = bitcast i8* %"&pSB[currWI].offset782.i" to [64 x float]*
  %"&(pSB[currWI].offset)821.i" = add nuw i64 %CurrSBIndex..0.i, 2048
  %"&pSB[currWI].offset822.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)821.i"
  %CastToValueType823.i = bitcast i8* %"&pSB[currWI].offset822.i" to [64 x float]*
  %"&(pSB[currWI].offset)861.i" = add nuw i64 %CurrSBIndex..0.i, 2304
  %"&pSB[currWI].offset862.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)861.i"
  %CastToValueType863.i = bitcast i8* %"&pSB[currWI].offset862.i" to [64 x float]*
  %"&(pSB[currWI].offset)901.i" = add nuw i64 %CurrSBIndex..0.i, 2560
  %"&pSB[currWI].offset902.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)901.i"
  %CastToValueType903.i = bitcast i8* %"&pSB[currWI].offset902.i" to [64 x float]*
  %"&(pSB[currWI].offset)941.i" = add nuw i64 %CurrSBIndex..0.i, 2816
  %"&pSB[currWI].offset942.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)941.i"
  %CastToValueType943.i = bitcast i8* %"&pSB[currWI].offset942.i" to [64 x float]*
  %"&(pSB[currWI].offset)981.i" = add nuw i64 %CurrSBIndex..0.i, 3072
  %"&pSB[currWI].offset982.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)981.i"
  %CastToValueType983.i = bitcast i8* %"&pSB[currWI].offset982.i" to [64 x float]*
  %"&(pSB[currWI].offset)1021.i" = add nuw i64 %CurrSBIndex..0.i, 3328
  %"&pSB[currWI].offset1022.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1021.i"
  %CastToValueType1023.i = bitcast i8* %"&pSB[currWI].offset1022.i" to [64 x float]*
  %"&(pSB[currWI].offset)1061.i" = add nuw i64 %CurrSBIndex..0.i, 3584
  %"&pSB[currWI].offset1062.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1061.i"
  %CastToValueType1063.i = bitcast i8* %"&pSB[currWI].offset1062.i" to [64 x float]*
  %"&(pSB[currWI].offset)1101.i" = add nuw i64 %CurrSBIndex..0.i, 3840
  %"&pSB[currWI].offset1102.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1101.i"
  %CastToValueType1103.i = bitcast i8* %"&pSB[currWI].offset1102.i" to [64 x float]*
  %"&(pSB[currWI].offset)1141.i" = add nuw i64 %CurrSBIndex..0.i, 4096
  %"&pSB[currWI].offset1142.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1141.i"
  %CastToValueType1143.i = bitcast i8* %"&pSB[currWI].offset1142.i" to [64 x float]*
  %"&(pSB[currWI].offset)1181.i" = add nuw i64 %CurrSBIndex..0.i, 4352
  %"&pSB[currWI].offset1182.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1181.i"
  %CastToValueType1183.i = bitcast i8* %"&pSB[currWI].offset1182.i" to [64 x float]*
  %"&(pSB[currWI].offset)1221.i" = add nuw i64 %CurrSBIndex..0.i, 4608
  %"&pSB[currWI].offset1222.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)1221.i"
  %CastToValueType1223.i = bitcast i8* %"&pSB[currWI].offset1222.i" to [64 x float]*
  br label %bb.nph7.i

bb.nph7.i:                                        ; preds = %._crit_edge8.i, %bb.nph12.i
  %indvar28.i = phi i64 [ 0, %bb.nph12.i ], [ %indvar.next29.i, %._crit_edge8.i ]
  %tmp66.i = mul i64 %tmp65.i, %indvar28.i
  %temp330.i = insertelement <16 x i64> undef, i64 %tmp66.i, i32 0
  %vector331.i = shufflevector <16 x i64> %temp330.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp73332.i = add <16 x i64> %tmp72329.i, %vector331.i
  %tmp76.i = shl i64 %indvar28.i, 3
  %tmp77179.i = or i64 %tmp76.i, 7
  %scevgep.7.i = getelementptr float addrspace(1)* %7, i64 %tmp77179.i
  %tmp79180.i = or i64 %tmp76.i, 6
  %scevgep.6.i = getelementptr float addrspace(1)* %7, i64 %tmp79180.i
  %tmp81181.i = or i64 %tmp76.i, 5
  %scevgep.5.i = getelementptr float addrspace(1)* %7, i64 %tmp81181.i
  %tmp83182.i = or i64 %tmp76.i, 4
  %scevgep.4.i = getelementptr float addrspace(1)* %7, i64 %tmp83182.i
  %tmp85183.i = or i64 %tmp76.i, 3
  %scevgep.3.i = getelementptr float addrspace(1)* %7, i64 %tmp85183.i
  %tmp87184.i = or i64 %tmp76.i, 2
  %scevgep.2.i = getelementptr float addrspace(1)* %7, i64 %tmp87184.i
  %tmp89185.i = or i64 %tmp76.i, 1
  %scevgep.1.i = getelementptr float addrspace(1)* %7, i64 %tmp89185.i
  %scevgep.i = getelementptr float addrspace(1)* %7, i64 %tmp76.i
  br label %355

; <label>:355                                     ; preds = %355, %bb.nph7.i
  %indvar32.i = phi i64 [ 0, %bb.nph7.i ], [ %indvar.next33.i, %355 ]
  %temp333.i = insertelement <16 x i64> undef, i64 %indvar32.i, i32 0
  %vector334.i = shufflevector <16 x i64> %temp333.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp74335.i = add <16 x i64> %tmp73332.i, %vector334.i
  %tmp37.i = shl i64 %indvar32.i, 3
  %356 = and <16 x i64> %tmp74335.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract336.i = extractelement <16 x i64> %356, i32 0
  %extract337.i = extractelement <16 x i64> %356, i32 1
  %extract338.i = extractelement <16 x i64> %356, i32 2
  %extract339.i = extractelement <16 x i64> %356, i32 3
  %extract340.i = extractelement <16 x i64> %356, i32 4
  %extract341.i = extractelement <16 x i64> %356, i32 5
  %extract342.i = extractelement <16 x i64> %356, i32 6
  %extract343.i = extractelement <16 x i64> %356, i32 7
  %extract344.i = extractelement <16 x i64> %356, i32 8
  %extract345.i = extractelement <16 x i64> %356, i32 9
  %extract346.i = extractelement <16 x i64> %356, i32 10
  %extract347.i = extractelement <16 x i64> %356, i32 11
  %extract348.i = extractelement <16 x i64> %356, i32 12
  %extract349.i = extractelement <16 x i64> %356, i32 13
  %extract350.i = extractelement <16 x i64> %356, i32 14
  %extract351.i = extractelement <16 x i64> %356, i32 15
  %357 = getelementptr inbounds float addrspace(1)* %1, i64 %extract336.i
  %358 = getelementptr inbounds float addrspace(1)* %1, i64 %extract337.i
  %359 = getelementptr inbounds float addrspace(1)* %1, i64 %extract338.i
  %360 = getelementptr inbounds float addrspace(1)* %1, i64 %extract339.i
  %361 = getelementptr inbounds float addrspace(1)* %1, i64 %extract340.i
  %362 = getelementptr inbounds float addrspace(1)* %1, i64 %extract341.i
  %363 = getelementptr inbounds float addrspace(1)* %1, i64 %extract342.i
  %364 = getelementptr inbounds float addrspace(1)* %1, i64 %extract343.i
  %365 = getelementptr inbounds float addrspace(1)* %1, i64 %extract344.i
  %366 = getelementptr inbounds float addrspace(1)* %1, i64 %extract345.i
  %367 = getelementptr inbounds float addrspace(1)* %1, i64 %extract346.i
  %368 = getelementptr inbounds float addrspace(1)* %1, i64 %extract347.i
  %369 = getelementptr inbounds float addrspace(1)* %1, i64 %extract348.i
  %370 = getelementptr inbounds float addrspace(1)* %1, i64 %extract349.i
  %371 = getelementptr inbounds float addrspace(1)* %1, i64 %extract350.i
  %372 = getelementptr inbounds float addrspace(1)* %1, i64 %extract351.i
  store float 0.000000e+00, float addrspace(1)* %357, align 4
  store float 0.000000e+00, float addrspace(1)* %358, align 4
  store float 0.000000e+00, float addrspace(1)* %359, align 4
  store float 0.000000e+00, float addrspace(1)* %360, align 4
  store float 0.000000e+00, float addrspace(1)* %361, align 4
  store float 0.000000e+00, float addrspace(1)* %362, align 4
  store float 0.000000e+00, float addrspace(1)* %363, align 4
  store float 0.000000e+00, float addrspace(1)* %364, align 4
  store float 0.000000e+00, float addrspace(1)* %365, align 4
  store float 0.000000e+00, float addrspace(1)* %366, align 4
  store float 0.000000e+00, float addrspace(1)* %367, align 4
  store float 0.000000e+00, float addrspace(1)* %368, align 4
  store float 0.000000e+00, float addrspace(1)* %369, align 4
  store float 0.000000e+00, float addrspace(1)* %370, align 4
  store float 0.000000e+00, float addrspace(1)* %371, align 4
  store float 0.000000e+00, float addrspace(1)* %372, align 4
  %tmp50192.i = or i64 %tmp37.i, 1
  %tmp48191.i = or i64 %tmp37.i, 2
  %tmp46190.i = or i64 %tmp37.i, 3
  %tmp44189.i = or i64 %tmp37.i, 4
  %tmp42188.i = or i64 %tmp37.i, 5
  %tmp40187.i = or i64 %tmp37.i, 6
  %tmp38186.i = or i64 %tmp37.i, 7
  %373 = getelementptr [64 x float]* %CastToValueType651.i, i64 0, i64 %tmp37.i
  %374 = getelementptr [64 x float]* %CastToValueType691.i, i64 0, i64 %tmp37.i
  %375 = getelementptr [64 x float]* %CastToValueType731.i, i64 0, i64 %tmp37.i
  %376 = getelementptr [64 x float]* %CastToValueType771.i, i64 0, i64 %tmp37.i
  %377 = getelementptr [64 x float]* %CastToValueType811.i, i64 0, i64 %tmp37.i
  %378 = getelementptr [64 x float]* %CastToValueType851.i, i64 0, i64 %tmp37.i
  %379 = getelementptr [64 x float]* %CastToValueType891.i, i64 0, i64 %tmp37.i
  %380 = getelementptr [64 x float]* %CastToValueType931.i, i64 0, i64 %tmp37.i
  %381 = getelementptr [64 x float]* %CastToValueType971.i, i64 0, i64 %tmp37.i
  %382 = getelementptr [64 x float]* %CastToValueType1011.i, i64 0, i64 %tmp37.i
  %383 = getelementptr [64 x float]* %CastToValueType1051.i, i64 0, i64 %tmp37.i
  %384 = getelementptr [64 x float]* %CastToValueType1091.i, i64 0, i64 %tmp37.i
  %385 = getelementptr [64 x float]* %CastToValueType1131.i, i64 0, i64 %tmp37.i
  %386 = getelementptr [64 x float]* %CastToValueType1171.i, i64 0, i64 %tmp37.i
  %387 = getelementptr [64 x float]* %CastToValueType1211.i, i64 0, i64 %tmp37.i
  %388 = getelementptr [64 x float]* %CastToValueType1251.i, i64 0, i64 %tmp37.i
  %389 = getelementptr [64 x float]* %CastToValueType647.i, i64 0, i64 %tmp50192.i
  %390 = getelementptr [64 x float]* %CastToValueType687.i, i64 0, i64 %tmp50192.i
  %391 = getelementptr [64 x float]* %CastToValueType727.i, i64 0, i64 %tmp50192.i
  %392 = getelementptr [64 x float]* %CastToValueType767.i, i64 0, i64 %tmp50192.i
  %393 = getelementptr [64 x float]* %CastToValueType807.i, i64 0, i64 %tmp50192.i
  %394 = getelementptr [64 x float]* %CastToValueType847.i, i64 0, i64 %tmp50192.i
  %395 = getelementptr [64 x float]* %CastToValueType887.i, i64 0, i64 %tmp50192.i
  %396 = getelementptr [64 x float]* %CastToValueType927.i, i64 0, i64 %tmp50192.i
  %397 = getelementptr [64 x float]* %CastToValueType967.i, i64 0, i64 %tmp50192.i
  %398 = getelementptr [64 x float]* %CastToValueType1007.i, i64 0, i64 %tmp50192.i
  %399 = getelementptr [64 x float]* %CastToValueType1047.i, i64 0, i64 %tmp50192.i
  %400 = getelementptr [64 x float]* %CastToValueType1087.i, i64 0, i64 %tmp50192.i
  %401 = getelementptr [64 x float]* %CastToValueType1127.i, i64 0, i64 %tmp50192.i
  %402 = getelementptr [64 x float]* %CastToValueType1167.i, i64 0, i64 %tmp50192.i
  %403 = getelementptr [64 x float]* %CastToValueType1207.i, i64 0, i64 %tmp50192.i
  %404 = getelementptr [64 x float]* %CastToValueType1247.i, i64 0, i64 %tmp50192.i
  %405 = getelementptr [64 x float]* %CastToValueType643.i, i64 0, i64 %tmp48191.i
  %406 = getelementptr [64 x float]* %CastToValueType683.i, i64 0, i64 %tmp48191.i
  %407 = getelementptr [64 x float]* %CastToValueType723.i, i64 0, i64 %tmp48191.i
  %408 = getelementptr [64 x float]* %CastToValueType763.i, i64 0, i64 %tmp48191.i
  %409 = getelementptr [64 x float]* %CastToValueType803.i, i64 0, i64 %tmp48191.i
  %410 = getelementptr [64 x float]* %CastToValueType843.i, i64 0, i64 %tmp48191.i
  %411 = getelementptr [64 x float]* %CastToValueType883.i, i64 0, i64 %tmp48191.i
  %412 = getelementptr [64 x float]* %CastToValueType923.i, i64 0, i64 %tmp48191.i
  %413 = getelementptr [64 x float]* %CastToValueType963.i, i64 0, i64 %tmp48191.i
  %414 = getelementptr [64 x float]* %CastToValueType1003.i, i64 0, i64 %tmp48191.i
  %415 = getelementptr [64 x float]* %CastToValueType1043.i, i64 0, i64 %tmp48191.i
  %416 = getelementptr [64 x float]* %CastToValueType1083.i, i64 0, i64 %tmp48191.i
  %417 = getelementptr [64 x float]* %CastToValueType1123.i, i64 0, i64 %tmp48191.i
  %418 = getelementptr [64 x float]* %CastToValueType1163.i, i64 0, i64 %tmp48191.i
  %419 = getelementptr [64 x float]* %CastToValueType1203.i, i64 0, i64 %tmp48191.i
  %420 = getelementptr [64 x float]* %CastToValueType1243.i, i64 0, i64 %tmp48191.i
  %421 = getelementptr [64 x float]* %CastToValueType639.i, i64 0, i64 %tmp46190.i
  %422 = getelementptr [64 x float]* %CastToValueType679.i, i64 0, i64 %tmp46190.i
  %423 = getelementptr [64 x float]* %CastToValueType719.i, i64 0, i64 %tmp46190.i
  %424 = getelementptr [64 x float]* %CastToValueType759.i, i64 0, i64 %tmp46190.i
  %425 = getelementptr [64 x float]* %CastToValueType799.i, i64 0, i64 %tmp46190.i
  %426 = getelementptr [64 x float]* %CastToValueType839.i, i64 0, i64 %tmp46190.i
  %427 = getelementptr [64 x float]* %CastToValueType879.i, i64 0, i64 %tmp46190.i
  %428 = getelementptr [64 x float]* %CastToValueType919.i, i64 0, i64 %tmp46190.i
  %429 = getelementptr [64 x float]* %CastToValueType959.i, i64 0, i64 %tmp46190.i
  %430 = getelementptr [64 x float]* %CastToValueType999.i, i64 0, i64 %tmp46190.i
  %431 = getelementptr [64 x float]* %CastToValueType1039.i, i64 0, i64 %tmp46190.i
  %432 = getelementptr [64 x float]* %CastToValueType1079.i, i64 0, i64 %tmp46190.i
  %433 = getelementptr [64 x float]* %CastToValueType1119.i, i64 0, i64 %tmp46190.i
  %434 = getelementptr [64 x float]* %CastToValueType1159.i, i64 0, i64 %tmp46190.i
  %435 = getelementptr [64 x float]* %CastToValueType1199.i, i64 0, i64 %tmp46190.i
  %436 = getelementptr [64 x float]* %CastToValueType1239.i, i64 0, i64 %tmp46190.i
  %437 = getelementptr [64 x float]* %CastToValueType635.i, i64 0, i64 %tmp44189.i
  %438 = getelementptr [64 x float]* %CastToValueType675.i, i64 0, i64 %tmp44189.i
  %439 = getelementptr [64 x float]* %CastToValueType715.i, i64 0, i64 %tmp44189.i
  %440 = getelementptr [64 x float]* %CastToValueType755.i, i64 0, i64 %tmp44189.i
  %441 = getelementptr [64 x float]* %CastToValueType795.i, i64 0, i64 %tmp44189.i
  %442 = getelementptr [64 x float]* %CastToValueType835.i, i64 0, i64 %tmp44189.i
  %443 = getelementptr [64 x float]* %CastToValueType875.i, i64 0, i64 %tmp44189.i
  %444 = getelementptr [64 x float]* %CastToValueType915.i, i64 0, i64 %tmp44189.i
  %445 = getelementptr [64 x float]* %CastToValueType955.i, i64 0, i64 %tmp44189.i
  %446 = getelementptr [64 x float]* %CastToValueType995.i, i64 0, i64 %tmp44189.i
  %447 = getelementptr [64 x float]* %CastToValueType1035.i, i64 0, i64 %tmp44189.i
  %448 = getelementptr [64 x float]* %CastToValueType1075.i, i64 0, i64 %tmp44189.i
  %449 = getelementptr [64 x float]* %CastToValueType1115.i, i64 0, i64 %tmp44189.i
  %450 = getelementptr [64 x float]* %CastToValueType1155.i, i64 0, i64 %tmp44189.i
  %451 = getelementptr [64 x float]* %CastToValueType1195.i, i64 0, i64 %tmp44189.i
  %452 = getelementptr [64 x float]* %CastToValueType1235.i, i64 0, i64 %tmp44189.i
  %453 = getelementptr [64 x float]* %CastToValueType631.i, i64 0, i64 %tmp42188.i
  %454 = getelementptr [64 x float]* %CastToValueType671.i, i64 0, i64 %tmp42188.i
  %455 = getelementptr [64 x float]* %CastToValueType711.i, i64 0, i64 %tmp42188.i
  %456 = getelementptr [64 x float]* %CastToValueType751.i, i64 0, i64 %tmp42188.i
  %457 = getelementptr [64 x float]* %CastToValueType791.i, i64 0, i64 %tmp42188.i
  %458 = getelementptr [64 x float]* %CastToValueType831.i, i64 0, i64 %tmp42188.i
  %459 = getelementptr [64 x float]* %CastToValueType871.i, i64 0, i64 %tmp42188.i
  %460 = getelementptr [64 x float]* %CastToValueType911.i, i64 0, i64 %tmp42188.i
  %461 = getelementptr [64 x float]* %CastToValueType951.i, i64 0, i64 %tmp42188.i
  %462 = getelementptr [64 x float]* %CastToValueType991.i, i64 0, i64 %tmp42188.i
  %463 = getelementptr [64 x float]* %CastToValueType1031.i, i64 0, i64 %tmp42188.i
  %464 = getelementptr [64 x float]* %CastToValueType1071.i, i64 0, i64 %tmp42188.i
  %465 = getelementptr [64 x float]* %CastToValueType1111.i, i64 0, i64 %tmp42188.i
  %466 = getelementptr [64 x float]* %CastToValueType1151.i, i64 0, i64 %tmp42188.i
  %467 = getelementptr [64 x float]* %CastToValueType1191.i, i64 0, i64 %tmp42188.i
  %468 = getelementptr [64 x float]* %CastToValueType1231.i, i64 0, i64 %tmp42188.i
  %469 = getelementptr [64 x float]* %CastToValueType627.i, i64 0, i64 %tmp40187.i
  %470 = getelementptr [64 x float]* %CastToValueType667.i, i64 0, i64 %tmp40187.i
  %471 = getelementptr [64 x float]* %CastToValueType707.i, i64 0, i64 %tmp40187.i
  %472 = getelementptr [64 x float]* %CastToValueType747.i, i64 0, i64 %tmp40187.i
  %473 = getelementptr [64 x float]* %CastToValueType787.i, i64 0, i64 %tmp40187.i
  %474 = getelementptr [64 x float]* %CastToValueType827.i, i64 0, i64 %tmp40187.i
  %475 = getelementptr [64 x float]* %CastToValueType867.i, i64 0, i64 %tmp40187.i
  %476 = getelementptr [64 x float]* %CastToValueType907.i, i64 0, i64 %tmp40187.i
  %477 = getelementptr [64 x float]* %CastToValueType947.i, i64 0, i64 %tmp40187.i
  %478 = getelementptr [64 x float]* %CastToValueType987.i, i64 0, i64 %tmp40187.i
  %479 = getelementptr [64 x float]* %CastToValueType1027.i, i64 0, i64 %tmp40187.i
  %480 = getelementptr [64 x float]* %CastToValueType1067.i, i64 0, i64 %tmp40187.i
  %481 = getelementptr [64 x float]* %CastToValueType1107.i, i64 0, i64 %tmp40187.i
  %482 = getelementptr [64 x float]* %CastToValueType1147.i, i64 0, i64 %tmp40187.i
  %483 = getelementptr [64 x float]* %CastToValueType1187.i, i64 0, i64 %tmp40187.i
  %484 = getelementptr [64 x float]* %CastToValueType1227.i, i64 0, i64 %tmp40187.i
  %485 = getelementptr [64 x float]* %CastToValueType.i, i64 0, i64 %tmp38186.i
  %486 = getelementptr [64 x float]* %CastToValueType663.i, i64 0, i64 %tmp38186.i
  %487 = getelementptr [64 x float]* %CastToValueType703.i, i64 0, i64 %tmp38186.i
  %488 = getelementptr [64 x float]* %CastToValueType743.i, i64 0, i64 %tmp38186.i
  %489 = getelementptr [64 x float]* %CastToValueType783.i, i64 0, i64 %tmp38186.i
  %490 = getelementptr [64 x float]* %CastToValueType823.i, i64 0, i64 %tmp38186.i
  %491 = getelementptr [64 x float]* %CastToValueType863.i, i64 0, i64 %tmp38186.i
  %492 = getelementptr [64 x float]* %CastToValueType903.i, i64 0, i64 %tmp38186.i
  %493 = getelementptr [64 x float]* %CastToValueType943.i, i64 0, i64 %tmp38186.i
  %494 = getelementptr [64 x float]* %CastToValueType983.i, i64 0, i64 %tmp38186.i
  %495 = getelementptr [64 x float]* %CastToValueType1023.i, i64 0, i64 %tmp38186.i
  %496 = getelementptr [64 x float]* %CastToValueType1063.i, i64 0, i64 %tmp38186.i
  %497 = getelementptr [64 x float]* %CastToValueType1103.i, i64 0, i64 %tmp38186.i
  %498 = getelementptr [64 x float]* %CastToValueType1143.i, i64 0, i64 %tmp38186.i
  %499 = getelementptr [64 x float]* %CastToValueType1183.i, i64 0, i64 %tmp38186.i
  %500 = getelementptr [64 x float]* %CastToValueType1223.i, i64 0, i64 %tmp38186.i
  %501 = load float addrspace(1)* %scevgep.i, align 4
  %temp352.i = insertelement <16 x float> undef, float %501, i32 0
  %vector353.i = shufflevector <16 x float> %temp352.i, <16 x float> undef, <16 x i32> zeroinitializer
  %502 = load float* %373, align 16
  %503 = load float* %374, align 16
  %504 = load float* %375, align 16
  %505 = load float* %376, align 16
  %506 = load float* %377, align 16
  %507 = load float* %378, align 16
  %508 = load float* %379, align 16
  %509 = load float* %380, align 16
  %510 = load float* %381, align 16
  %511 = load float* %382, align 16
  %512 = load float* %383, align 16
  %513 = load float* %384, align 16
  %514 = load float* %385, align 16
  %515 = load float* %386, align 16
  %516 = load float* %387, align 16
  %517 = load float* %388, align 16
  %temp.vect354.i = insertelement <16 x float> undef, float %502, i32 0
  %temp.vect355.i = insertelement <16 x float> %temp.vect354.i, float %503, i32 1
  %temp.vect356.i = insertelement <16 x float> %temp.vect355.i, float %504, i32 2
  %temp.vect357.i = insertelement <16 x float> %temp.vect356.i, float %505, i32 3
  %temp.vect358.i = insertelement <16 x float> %temp.vect357.i, float %506, i32 4
  %temp.vect359.i = insertelement <16 x float> %temp.vect358.i, float %507, i32 5
  %temp.vect360.i = insertelement <16 x float> %temp.vect359.i, float %508, i32 6
  %temp.vect361.i = insertelement <16 x float> %temp.vect360.i, float %509, i32 7
  %temp.vect362.i = insertelement <16 x float> %temp.vect361.i, float %510, i32 8
  %temp.vect363.i = insertelement <16 x float> %temp.vect362.i, float %511, i32 9
  %temp.vect364.i = insertelement <16 x float> %temp.vect363.i, float %512, i32 10
  %temp.vect365.i = insertelement <16 x float> %temp.vect364.i, float %513, i32 11
  %temp.vect366.i = insertelement <16 x float> %temp.vect365.i, float %514, i32 12
  %temp.vect367.i = insertelement <16 x float> %temp.vect366.i, float %515, i32 13
  %temp.vect368.i = insertelement <16 x float> %temp.vect367.i, float %516, i32 14
  %temp.vect369.i = insertelement <16 x float> %temp.vect368.i, float %517, i32 15
  %518 = fmul <16 x float> %vector353.i, %temp.vect369.i
  %519 = fadd <16 x float> %518, zeroinitializer
  %extract370.i = extractelement <16 x float> %519, i32 0
  %extract371.i = extractelement <16 x float> %519, i32 1
  %extract372.i = extractelement <16 x float> %519, i32 2
  %extract373.i = extractelement <16 x float> %519, i32 3
  %extract374.i = extractelement <16 x float> %519, i32 4
  %extract375.i = extractelement <16 x float> %519, i32 5
  %extract376.i = extractelement <16 x float> %519, i32 6
  %extract377.i = extractelement <16 x float> %519, i32 7
  %extract378.i = extractelement <16 x float> %519, i32 8
  %extract379.i = extractelement <16 x float> %519, i32 9
  %extract380.i = extractelement <16 x float> %519, i32 10
  %extract381.i = extractelement <16 x float> %519, i32 11
  %extract382.i = extractelement <16 x float> %519, i32 12
  %extract383.i = extractelement <16 x float> %519, i32 13
  %extract384.i = extractelement <16 x float> %519, i32 14
  %extract385.i = extractelement <16 x float> %519, i32 15
  store float %extract370.i, float addrspace(1)* %357, align 4
  store float %extract371.i, float addrspace(1)* %358, align 4
  store float %extract372.i, float addrspace(1)* %359, align 4
  store float %extract373.i, float addrspace(1)* %360, align 4
  store float %extract374.i, float addrspace(1)* %361, align 4
  store float %extract375.i, float addrspace(1)* %362, align 4
  store float %extract376.i, float addrspace(1)* %363, align 4
  store float %extract377.i, float addrspace(1)* %364, align 4
  store float %extract378.i, float addrspace(1)* %365, align 4
  store float %extract379.i, float addrspace(1)* %366, align 4
  store float %extract380.i, float addrspace(1)* %367, align 4
  store float %extract381.i, float addrspace(1)* %368, align 4
  store float %extract382.i, float addrspace(1)* %369, align 4
  store float %extract383.i, float addrspace(1)* %370, align 4
  store float %extract384.i, float addrspace(1)* %371, align 4
  store float %extract385.i, float addrspace(1)* %372, align 4
  %520 = load float addrspace(1)* %scevgep.1.i, align 4
  %temp386.i = insertelement <16 x float> undef, float %520, i32 0
  %vector387.i = shufflevector <16 x float> %temp386.i, <16 x float> undef, <16 x i32> zeroinitializer
  %521 = load float* %389, align 4
  %522 = load float* %390, align 4
  %523 = load float* %391, align 4
  %524 = load float* %392, align 4
  %525 = load float* %393, align 4
  %526 = load float* %394, align 4
  %527 = load float* %395, align 4
  %528 = load float* %396, align 4
  %529 = load float* %397, align 4
  %530 = load float* %398, align 4
  %531 = load float* %399, align 4
  %532 = load float* %400, align 4
  %533 = load float* %401, align 4
  %534 = load float* %402, align 4
  %535 = load float* %403, align 4
  %536 = load float* %404, align 4
  %temp.vect388.i = insertelement <16 x float> undef, float %521, i32 0
  %temp.vect389.i = insertelement <16 x float> %temp.vect388.i, float %522, i32 1
  %temp.vect390.i = insertelement <16 x float> %temp.vect389.i, float %523, i32 2
  %temp.vect391.i = insertelement <16 x float> %temp.vect390.i, float %524, i32 3
  %temp.vect392.i = insertelement <16 x float> %temp.vect391.i, float %525, i32 4
  %temp.vect393.i = insertelement <16 x float> %temp.vect392.i, float %526, i32 5
  %temp.vect394.i = insertelement <16 x float> %temp.vect393.i, float %527, i32 6
  %temp.vect395.i = insertelement <16 x float> %temp.vect394.i, float %528, i32 7
  %temp.vect396.i = insertelement <16 x float> %temp.vect395.i, float %529, i32 8
  %temp.vect397.i = insertelement <16 x float> %temp.vect396.i, float %530, i32 9
  %temp.vect398.i = insertelement <16 x float> %temp.vect397.i, float %531, i32 10
  %temp.vect399.i = insertelement <16 x float> %temp.vect398.i, float %532, i32 11
  %temp.vect400.i = insertelement <16 x float> %temp.vect399.i, float %533, i32 12
  %temp.vect401.i = insertelement <16 x float> %temp.vect400.i, float %534, i32 13
  %temp.vect402.i = insertelement <16 x float> %temp.vect401.i, float %535, i32 14
  %temp.vect403.i = insertelement <16 x float> %temp.vect402.i, float %536, i32 15
  %537 = fmul <16 x float> %vector387.i, %temp.vect403.i
  %538 = fadd <16 x float> %519, %537
  %extract404.i = extractelement <16 x float> %538, i32 0
  %extract405.i = extractelement <16 x float> %538, i32 1
  %extract406.i = extractelement <16 x float> %538, i32 2
  %extract407.i = extractelement <16 x float> %538, i32 3
  %extract408.i = extractelement <16 x float> %538, i32 4
  %extract409.i = extractelement <16 x float> %538, i32 5
  %extract410.i = extractelement <16 x float> %538, i32 6
  %extract411.i = extractelement <16 x float> %538, i32 7
  %extract412.i = extractelement <16 x float> %538, i32 8
  %extract413.i = extractelement <16 x float> %538, i32 9
  %extract414.i = extractelement <16 x float> %538, i32 10
  %extract415.i = extractelement <16 x float> %538, i32 11
  %extract416.i = extractelement <16 x float> %538, i32 12
  %extract417.i = extractelement <16 x float> %538, i32 13
  %extract418.i = extractelement <16 x float> %538, i32 14
  %extract419.i = extractelement <16 x float> %538, i32 15
  store float %extract404.i, float addrspace(1)* %357, align 4
  store float %extract405.i, float addrspace(1)* %358, align 4
  store float %extract406.i, float addrspace(1)* %359, align 4
  store float %extract407.i, float addrspace(1)* %360, align 4
  store float %extract408.i, float addrspace(1)* %361, align 4
  store float %extract409.i, float addrspace(1)* %362, align 4
  store float %extract410.i, float addrspace(1)* %363, align 4
  store float %extract411.i, float addrspace(1)* %364, align 4
  store float %extract412.i, float addrspace(1)* %365, align 4
  store float %extract413.i, float addrspace(1)* %366, align 4
  store float %extract414.i, float addrspace(1)* %367, align 4
  store float %extract415.i, float addrspace(1)* %368, align 4
  store float %extract416.i, float addrspace(1)* %369, align 4
  store float %extract417.i, float addrspace(1)* %370, align 4
  store float %extract418.i, float addrspace(1)* %371, align 4
  store float %extract419.i, float addrspace(1)* %372, align 4
  %539 = load float addrspace(1)* %scevgep.2.i, align 4
  %temp420.i = insertelement <16 x float> undef, float %539, i32 0
  %vector421.i = shufflevector <16 x float> %temp420.i, <16 x float> undef, <16 x i32> zeroinitializer
  %540 = load float* %405, align 8
  %541 = load float* %406, align 8
  %542 = load float* %407, align 8
  %543 = load float* %408, align 8
  %544 = load float* %409, align 8
  %545 = load float* %410, align 8
  %546 = load float* %411, align 8
  %547 = load float* %412, align 8
  %548 = load float* %413, align 8
  %549 = load float* %414, align 8
  %550 = load float* %415, align 8
  %551 = load float* %416, align 8
  %552 = load float* %417, align 8
  %553 = load float* %418, align 8
  %554 = load float* %419, align 8
  %555 = load float* %420, align 8
  %temp.vect422.i = insertelement <16 x float> undef, float %540, i32 0
  %temp.vect423.i = insertelement <16 x float> %temp.vect422.i, float %541, i32 1
  %temp.vect424.i = insertelement <16 x float> %temp.vect423.i, float %542, i32 2
  %temp.vect425.i = insertelement <16 x float> %temp.vect424.i, float %543, i32 3
  %temp.vect426.i = insertelement <16 x float> %temp.vect425.i, float %544, i32 4
  %temp.vect427.i = insertelement <16 x float> %temp.vect426.i, float %545, i32 5
  %temp.vect428.i = insertelement <16 x float> %temp.vect427.i, float %546, i32 6
  %temp.vect429.i = insertelement <16 x float> %temp.vect428.i, float %547, i32 7
  %temp.vect430.i = insertelement <16 x float> %temp.vect429.i, float %548, i32 8
  %temp.vect431.i = insertelement <16 x float> %temp.vect430.i, float %549, i32 9
  %temp.vect432.i = insertelement <16 x float> %temp.vect431.i, float %550, i32 10
  %temp.vect433.i = insertelement <16 x float> %temp.vect432.i, float %551, i32 11
  %temp.vect434.i = insertelement <16 x float> %temp.vect433.i, float %552, i32 12
  %temp.vect435.i = insertelement <16 x float> %temp.vect434.i, float %553, i32 13
  %temp.vect436.i = insertelement <16 x float> %temp.vect435.i, float %554, i32 14
  %temp.vect437.i = insertelement <16 x float> %temp.vect436.i, float %555, i32 15
  %556 = fmul <16 x float> %vector421.i, %temp.vect437.i
  %557 = fadd <16 x float> %538, %556
  %extract438.i = extractelement <16 x float> %557, i32 0
  %extract439.i = extractelement <16 x float> %557, i32 1
  %extract440.i = extractelement <16 x float> %557, i32 2
  %extract441.i = extractelement <16 x float> %557, i32 3
  %extract442.i = extractelement <16 x float> %557, i32 4
  %extract443.i = extractelement <16 x float> %557, i32 5
  %extract444.i = extractelement <16 x float> %557, i32 6
  %extract445.i = extractelement <16 x float> %557, i32 7
  %extract446.i = extractelement <16 x float> %557, i32 8
  %extract447.i = extractelement <16 x float> %557, i32 9
  %extract448.i = extractelement <16 x float> %557, i32 10
  %extract449.i = extractelement <16 x float> %557, i32 11
  %extract450.i = extractelement <16 x float> %557, i32 12
  %extract451.i = extractelement <16 x float> %557, i32 13
  %extract452.i = extractelement <16 x float> %557, i32 14
  %extract453.i = extractelement <16 x float> %557, i32 15
  store float %extract438.i, float addrspace(1)* %357, align 4
  store float %extract439.i, float addrspace(1)* %358, align 4
  store float %extract440.i, float addrspace(1)* %359, align 4
  store float %extract441.i, float addrspace(1)* %360, align 4
  store float %extract442.i, float addrspace(1)* %361, align 4
  store float %extract443.i, float addrspace(1)* %362, align 4
  store float %extract444.i, float addrspace(1)* %363, align 4
  store float %extract445.i, float addrspace(1)* %364, align 4
  store float %extract446.i, float addrspace(1)* %365, align 4
  store float %extract447.i, float addrspace(1)* %366, align 4
  store float %extract448.i, float addrspace(1)* %367, align 4
  store float %extract449.i, float addrspace(1)* %368, align 4
  store float %extract450.i, float addrspace(1)* %369, align 4
  store float %extract451.i, float addrspace(1)* %370, align 4
  store float %extract452.i, float addrspace(1)* %371, align 4
  store float %extract453.i, float addrspace(1)* %372, align 4
  %558 = load float addrspace(1)* %scevgep.3.i, align 4
  %temp454.i = insertelement <16 x float> undef, float %558, i32 0
  %vector455.i = shufflevector <16 x float> %temp454.i, <16 x float> undef, <16 x i32> zeroinitializer
  %559 = load float* %421, align 4
  %560 = load float* %422, align 4
  %561 = load float* %423, align 4
  %562 = load float* %424, align 4
  %563 = load float* %425, align 4
  %564 = load float* %426, align 4
  %565 = load float* %427, align 4
  %566 = load float* %428, align 4
  %567 = load float* %429, align 4
  %568 = load float* %430, align 4
  %569 = load float* %431, align 4
  %570 = load float* %432, align 4
  %571 = load float* %433, align 4
  %572 = load float* %434, align 4
  %573 = load float* %435, align 4
  %574 = load float* %436, align 4
  %temp.vect456.i = insertelement <16 x float> undef, float %559, i32 0
  %temp.vect457.i = insertelement <16 x float> %temp.vect456.i, float %560, i32 1
  %temp.vect458.i = insertelement <16 x float> %temp.vect457.i, float %561, i32 2
  %temp.vect459.i = insertelement <16 x float> %temp.vect458.i, float %562, i32 3
  %temp.vect460.i = insertelement <16 x float> %temp.vect459.i, float %563, i32 4
  %temp.vect461.i = insertelement <16 x float> %temp.vect460.i, float %564, i32 5
  %temp.vect462.i = insertelement <16 x float> %temp.vect461.i, float %565, i32 6
  %temp.vect463.i = insertelement <16 x float> %temp.vect462.i, float %566, i32 7
  %temp.vect464.i = insertelement <16 x float> %temp.vect463.i, float %567, i32 8
  %temp.vect465.i = insertelement <16 x float> %temp.vect464.i, float %568, i32 9
  %temp.vect466.i = insertelement <16 x float> %temp.vect465.i, float %569, i32 10
  %temp.vect467.i = insertelement <16 x float> %temp.vect466.i, float %570, i32 11
  %temp.vect468.i = insertelement <16 x float> %temp.vect467.i, float %571, i32 12
  %temp.vect469.i = insertelement <16 x float> %temp.vect468.i, float %572, i32 13
  %temp.vect470.i = insertelement <16 x float> %temp.vect469.i, float %573, i32 14
  %temp.vect471.i = insertelement <16 x float> %temp.vect470.i, float %574, i32 15
  %575 = fmul <16 x float> %vector455.i, %temp.vect471.i
  %576 = fadd <16 x float> %557, %575
  %extract472.i = extractelement <16 x float> %576, i32 0
  %extract473.i = extractelement <16 x float> %576, i32 1
  %extract474.i = extractelement <16 x float> %576, i32 2
  %extract475.i = extractelement <16 x float> %576, i32 3
  %extract476.i = extractelement <16 x float> %576, i32 4
  %extract477.i = extractelement <16 x float> %576, i32 5
  %extract478.i = extractelement <16 x float> %576, i32 6
  %extract479.i = extractelement <16 x float> %576, i32 7
  %extract480.i = extractelement <16 x float> %576, i32 8
  %extract481.i = extractelement <16 x float> %576, i32 9
  %extract482.i = extractelement <16 x float> %576, i32 10
  %extract483.i = extractelement <16 x float> %576, i32 11
  %extract484.i = extractelement <16 x float> %576, i32 12
  %extract485.i = extractelement <16 x float> %576, i32 13
  %extract486.i = extractelement <16 x float> %576, i32 14
  %extract487.i = extractelement <16 x float> %576, i32 15
  store float %extract472.i, float addrspace(1)* %357, align 4
  store float %extract473.i, float addrspace(1)* %358, align 4
  store float %extract474.i, float addrspace(1)* %359, align 4
  store float %extract475.i, float addrspace(1)* %360, align 4
  store float %extract476.i, float addrspace(1)* %361, align 4
  store float %extract477.i, float addrspace(1)* %362, align 4
  store float %extract478.i, float addrspace(1)* %363, align 4
  store float %extract479.i, float addrspace(1)* %364, align 4
  store float %extract480.i, float addrspace(1)* %365, align 4
  store float %extract481.i, float addrspace(1)* %366, align 4
  store float %extract482.i, float addrspace(1)* %367, align 4
  store float %extract483.i, float addrspace(1)* %368, align 4
  store float %extract484.i, float addrspace(1)* %369, align 4
  store float %extract485.i, float addrspace(1)* %370, align 4
  store float %extract486.i, float addrspace(1)* %371, align 4
  store float %extract487.i, float addrspace(1)* %372, align 4
  %577 = load float addrspace(1)* %scevgep.4.i, align 4
  %temp488.i = insertelement <16 x float> undef, float %577, i32 0
  %vector489.i = shufflevector <16 x float> %temp488.i, <16 x float> undef, <16 x i32> zeroinitializer
  %578 = load float* %437, align 16
  %579 = load float* %438, align 16
  %580 = load float* %439, align 16
  %581 = load float* %440, align 16
  %582 = load float* %441, align 16
  %583 = load float* %442, align 16
  %584 = load float* %443, align 16
  %585 = load float* %444, align 16
  %586 = load float* %445, align 16
  %587 = load float* %446, align 16
  %588 = load float* %447, align 16
  %589 = load float* %448, align 16
  %590 = load float* %449, align 16
  %591 = load float* %450, align 16
  %592 = load float* %451, align 16
  %593 = load float* %452, align 16
  %temp.vect490.i = insertelement <16 x float> undef, float %578, i32 0
  %temp.vect491.i = insertelement <16 x float> %temp.vect490.i, float %579, i32 1
  %temp.vect492.i = insertelement <16 x float> %temp.vect491.i, float %580, i32 2
  %temp.vect493.i = insertelement <16 x float> %temp.vect492.i, float %581, i32 3
  %temp.vect494.i = insertelement <16 x float> %temp.vect493.i, float %582, i32 4
  %temp.vect495.i = insertelement <16 x float> %temp.vect494.i, float %583, i32 5
  %temp.vect496.i = insertelement <16 x float> %temp.vect495.i, float %584, i32 6
  %temp.vect497.i = insertelement <16 x float> %temp.vect496.i, float %585, i32 7
  %temp.vect498.i = insertelement <16 x float> %temp.vect497.i, float %586, i32 8
  %temp.vect499.i = insertelement <16 x float> %temp.vect498.i, float %587, i32 9
  %temp.vect500.i = insertelement <16 x float> %temp.vect499.i, float %588, i32 10
  %temp.vect501.i = insertelement <16 x float> %temp.vect500.i, float %589, i32 11
  %temp.vect502.i = insertelement <16 x float> %temp.vect501.i, float %590, i32 12
  %temp.vect503.i = insertelement <16 x float> %temp.vect502.i, float %591, i32 13
  %temp.vect504.i = insertelement <16 x float> %temp.vect503.i, float %592, i32 14
  %temp.vect505.i = insertelement <16 x float> %temp.vect504.i, float %593, i32 15
  %594 = fmul <16 x float> %vector489.i, %temp.vect505.i
  %595 = fadd <16 x float> %576, %594
  %extract506.i = extractelement <16 x float> %595, i32 0
  %extract507.i = extractelement <16 x float> %595, i32 1
  %extract508.i = extractelement <16 x float> %595, i32 2
  %extract509.i = extractelement <16 x float> %595, i32 3
  %extract510.i = extractelement <16 x float> %595, i32 4
  %extract511.i = extractelement <16 x float> %595, i32 5
  %extract512.i = extractelement <16 x float> %595, i32 6
  %extract513.i = extractelement <16 x float> %595, i32 7
  %extract514.i = extractelement <16 x float> %595, i32 8
  %extract515.i = extractelement <16 x float> %595, i32 9
  %extract516.i = extractelement <16 x float> %595, i32 10
  %extract517.i = extractelement <16 x float> %595, i32 11
  %extract518.i = extractelement <16 x float> %595, i32 12
  %extract519.i = extractelement <16 x float> %595, i32 13
  %extract520.i = extractelement <16 x float> %595, i32 14
  %extract521.i = extractelement <16 x float> %595, i32 15
  store float %extract506.i, float addrspace(1)* %357, align 4
  store float %extract507.i, float addrspace(1)* %358, align 4
  store float %extract508.i, float addrspace(1)* %359, align 4
  store float %extract509.i, float addrspace(1)* %360, align 4
  store float %extract510.i, float addrspace(1)* %361, align 4
  store float %extract511.i, float addrspace(1)* %362, align 4
  store float %extract512.i, float addrspace(1)* %363, align 4
  store float %extract513.i, float addrspace(1)* %364, align 4
  store float %extract514.i, float addrspace(1)* %365, align 4
  store float %extract515.i, float addrspace(1)* %366, align 4
  store float %extract516.i, float addrspace(1)* %367, align 4
  store float %extract517.i, float addrspace(1)* %368, align 4
  store float %extract518.i, float addrspace(1)* %369, align 4
  store float %extract519.i, float addrspace(1)* %370, align 4
  store float %extract520.i, float addrspace(1)* %371, align 4
  store float %extract521.i, float addrspace(1)* %372, align 4
  %596 = load float addrspace(1)* %scevgep.5.i, align 4
  %temp522.i = insertelement <16 x float> undef, float %596, i32 0
  %vector523.i = shufflevector <16 x float> %temp522.i, <16 x float> undef, <16 x i32> zeroinitializer
  %597 = load float* %453, align 4
  %598 = load float* %454, align 4
  %599 = load float* %455, align 4
  %600 = load float* %456, align 4
  %601 = load float* %457, align 4
  %602 = load float* %458, align 4
  %603 = load float* %459, align 4
  %604 = load float* %460, align 4
  %605 = load float* %461, align 4
  %606 = load float* %462, align 4
  %607 = load float* %463, align 4
  %608 = load float* %464, align 4
  %609 = load float* %465, align 4
  %610 = load float* %466, align 4
  %611 = load float* %467, align 4
  %612 = load float* %468, align 4
  %temp.vect524.i = insertelement <16 x float> undef, float %597, i32 0
  %temp.vect525.i = insertelement <16 x float> %temp.vect524.i, float %598, i32 1
  %temp.vect526.i = insertelement <16 x float> %temp.vect525.i, float %599, i32 2
  %temp.vect527.i = insertelement <16 x float> %temp.vect526.i, float %600, i32 3
  %temp.vect528.i = insertelement <16 x float> %temp.vect527.i, float %601, i32 4
  %temp.vect529.i = insertelement <16 x float> %temp.vect528.i, float %602, i32 5
  %temp.vect530.i = insertelement <16 x float> %temp.vect529.i, float %603, i32 6
  %temp.vect531.i = insertelement <16 x float> %temp.vect530.i, float %604, i32 7
  %temp.vect532.i = insertelement <16 x float> %temp.vect531.i, float %605, i32 8
  %temp.vect533.i = insertelement <16 x float> %temp.vect532.i, float %606, i32 9
  %temp.vect534.i = insertelement <16 x float> %temp.vect533.i, float %607, i32 10
  %temp.vect535.i = insertelement <16 x float> %temp.vect534.i, float %608, i32 11
  %temp.vect536.i = insertelement <16 x float> %temp.vect535.i, float %609, i32 12
  %temp.vect537.i = insertelement <16 x float> %temp.vect536.i, float %610, i32 13
  %temp.vect538.i = insertelement <16 x float> %temp.vect537.i, float %611, i32 14
  %temp.vect539.i = insertelement <16 x float> %temp.vect538.i, float %612, i32 15
  %613 = fmul <16 x float> %vector523.i, %temp.vect539.i
  %614 = fadd <16 x float> %595, %613
  %extract540.i = extractelement <16 x float> %614, i32 0
  %extract541.i = extractelement <16 x float> %614, i32 1
  %extract542.i = extractelement <16 x float> %614, i32 2
  %extract543.i = extractelement <16 x float> %614, i32 3
  %extract544.i = extractelement <16 x float> %614, i32 4
  %extract545.i = extractelement <16 x float> %614, i32 5
  %extract546.i = extractelement <16 x float> %614, i32 6
  %extract547.i = extractelement <16 x float> %614, i32 7
  %extract548.i = extractelement <16 x float> %614, i32 8
  %extract549.i = extractelement <16 x float> %614, i32 9
  %extract550.i = extractelement <16 x float> %614, i32 10
  %extract551.i = extractelement <16 x float> %614, i32 11
  %extract552.i = extractelement <16 x float> %614, i32 12
  %extract553.i = extractelement <16 x float> %614, i32 13
  %extract554.i = extractelement <16 x float> %614, i32 14
  %extract555.i = extractelement <16 x float> %614, i32 15
  store float %extract540.i, float addrspace(1)* %357, align 4
  store float %extract541.i, float addrspace(1)* %358, align 4
  store float %extract542.i, float addrspace(1)* %359, align 4
  store float %extract543.i, float addrspace(1)* %360, align 4
  store float %extract544.i, float addrspace(1)* %361, align 4
  store float %extract545.i, float addrspace(1)* %362, align 4
  store float %extract546.i, float addrspace(1)* %363, align 4
  store float %extract547.i, float addrspace(1)* %364, align 4
  store float %extract548.i, float addrspace(1)* %365, align 4
  store float %extract549.i, float addrspace(1)* %366, align 4
  store float %extract550.i, float addrspace(1)* %367, align 4
  store float %extract551.i, float addrspace(1)* %368, align 4
  store float %extract552.i, float addrspace(1)* %369, align 4
  store float %extract553.i, float addrspace(1)* %370, align 4
  store float %extract554.i, float addrspace(1)* %371, align 4
  store float %extract555.i, float addrspace(1)* %372, align 4
  %615 = load float addrspace(1)* %scevgep.6.i, align 4
  %temp556.i = insertelement <16 x float> undef, float %615, i32 0
  %vector557.i = shufflevector <16 x float> %temp556.i, <16 x float> undef, <16 x i32> zeroinitializer
  %616 = load float* %469, align 8
  %617 = load float* %470, align 8
  %618 = load float* %471, align 8
  %619 = load float* %472, align 8
  %620 = load float* %473, align 8
  %621 = load float* %474, align 8
  %622 = load float* %475, align 8
  %623 = load float* %476, align 8
  %624 = load float* %477, align 8
  %625 = load float* %478, align 8
  %626 = load float* %479, align 8
  %627 = load float* %480, align 8
  %628 = load float* %481, align 8
  %629 = load float* %482, align 8
  %630 = load float* %483, align 8
  %631 = load float* %484, align 8
  %temp.vect558.i = insertelement <16 x float> undef, float %616, i32 0
  %temp.vect559.i = insertelement <16 x float> %temp.vect558.i, float %617, i32 1
  %temp.vect560.i = insertelement <16 x float> %temp.vect559.i, float %618, i32 2
  %temp.vect561.i = insertelement <16 x float> %temp.vect560.i, float %619, i32 3
  %temp.vect562.i = insertelement <16 x float> %temp.vect561.i, float %620, i32 4
  %temp.vect563.i = insertelement <16 x float> %temp.vect562.i, float %621, i32 5
  %temp.vect564.i = insertelement <16 x float> %temp.vect563.i, float %622, i32 6
  %temp.vect565.i = insertelement <16 x float> %temp.vect564.i, float %623, i32 7
  %temp.vect566.i = insertelement <16 x float> %temp.vect565.i, float %624, i32 8
  %temp.vect567.i = insertelement <16 x float> %temp.vect566.i, float %625, i32 9
  %temp.vect568.i = insertelement <16 x float> %temp.vect567.i, float %626, i32 10
  %temp.vect569.i = insertelement <16 x float> %temp.vect568.i, float %627, i32 11
  %temp.vect570.i = insertelement <16 x float> %temp.vect569.i, float %628, i32 12
  %temp.vect571.i = insertelement <16 x float> %temp.vect570.i, float %629, i32 13
  %temp.vect572.i = insertelement <16 x float> %temp.vect571.i, float %630, i32 14
  %temp.vect573.i = insertelement <16 x float> %temp.vect572.i, float %631, i32 15
  %632 = fmul <16 x float> %vector557.i, %temp.vect573.i
  %633 = fadd <16 x float> %614, %632
  %extract574.i = extractelement <16 x float> %633, i32 0
  %extract575.i = extractelement <16 x float> %633, i32 1
  %extract576.i = extractelement <16 x float> %633, i32 2
  %extract577.i = extractelement <16 x float> %633, i32 3
  %extract578.i = extractelement <16 x float> %633, i32 4
  %extract579.i = extractelement <16 x float> %633, i32 5
  %extract580.i = extractelement <16 x float> %633, i32 6
  %extract581.i = extractelement <16 x float> %633, i32 7
  %extract582.i = extractelement <16 x float> %633, i32 8
  %extract583.i = extractelement <16 x float> %633, i32 9
  %extract584.i = extractelement <16 x float> %633, i32 10
  %extract585.i = extractelement <16 x float> %633, i32 11
  %extract586.i = extractelement <16 x float> %633, i32 12
  %extract587.i = extractelement <16 x float> %633, i32 13
  %extract588.i = extractelement <16 x float> %633, i32 14
  %extract589.i = extractelement <16 x float> %633, i32 15
  store float %extract574.i, float addrspace(1)* %357, align 4
  store float %extract575.i, float addrspace(1)* %358, align 4
  store float %extract576.i, float addrspace(1)* %359, align 4
  store float %extract577.i, float addrspace(1)* %360, align 4
  store float %extract578.i, float addrspace(1)* %361, align 4
  store float %extract579.i, float addrspace(1)* %362, align 4
  store float %extract580.i, float addrspace(1)* %363, align 4
  store float %extract581.i, float addrspace(1)* %364, align 4
  store float %extract582.i, float addrspace(1)* %365, align 4
  store float %extract583.i, float addrspace(1)* %366, align 4
  store float %extract584.i, float addrspace(1)* %367, align 4
  store float %extract585.i, float addrspace(1)* %368, align 4
  store float %extract586.i, float addrspace(1)* %369, align 4
  store float %extract587.i, float addrspace(1)* %370, align 4
  store float %extract588.i, float addrspace(1)* %371, align 4
  store float %extract589.i, float addrspace(1)* %372, align 4
  %634 = load float addrspace(1)* %scevgep.7.i, align 4
  %temp590.i = insertelement <16 x float> undef, float %634, i32 0
  %vector591.i = shufflevector <16 x float> %temp590.i, <16 x float> undef, <16 x i32> zeroinitializer
  %635 = load float* %485, align 4
  %636 = load float* %486, align 4
  %637 = load float* %487, align 4
  %638 = load float* %488, align 4
  %639 = load float* %489, align 4
  %640 = load float* %490, align 4
  %641 = load float* %491, align 4
  %642 = load float* %492, align 4
  %643 = load float* %493, align 4
  %644 = load float* %494, align 4
  %645 = load float* %495, align 4
  %646 = load float* %496, align 4
  %647 = load float* %497, align 4
  %648 = load float* %498, align 4
  %649 = load float* %499, align 4
  %650 = load float* %500, align 4
  %temp.vect592.i = insertelement <16 x float> undef, float %635, i32 0
  %temp.vect593.i = insertelement <16 x float> %temp.vect592.i, float %636, i32 1
  %temp.vect594.i = insertelement <16 x float> %temp.vect593.i, float %637, i32 2
  %temp.vect595.i = insertelement <16 x float> %temp.vect594.i, float %638, i32 3
  %temp.vect596.i = insertelement <16 x float> %temp.vect595.i, float %639, i32 4
  %temp.vect597.i = insertelement <16 x float> %temp.vect596.i, float %640, i32 5
  %temp.vect598.i = insertelement <16 x float> %temp.vect597.i, float %641, i32 6
  %temp.vect599.i = insertelement <16 x float> %temp.vect598.i, float %642, i32 7
  %temp.vect600.i = insertelement <16 x float> %temp.vect599.i, float %643, i32 8
  %temp.vect601.i = insertelement <16 x float> %temp.vect600.i, float %644, i32 9
  %temp.vect602.i = insertelement <16 x float> %temp.vect601.i, float %645, i32 10
  %temp.vect603.i = insertelement <16 x float> %temp.vect602.i, float %646, i32 11
  %temp.vect604.i = insertelement <16 x float> %temp.vect603.i, float %647, i32 12
  %temp.vect605.i = insertelement <16 x float> %temp.vect604.i, float %648, i32 13
  %temp.vect606.i = insertelement <16 x float> %temp.vect605.i, float %649, i32 14
  %temp.vect607.i = insertelement <16 x float> %temp.vect606.i, float %650, i32 15
  %651 = fmul <16 x float> %vector591.i, %temp.vect607.i
  %652 = fadd <16 x float> %633, %651
  %extract608.i = extractelement <16 x float> %652, i32 0
  %extract609.i = extractelement <16 x float> %652, i32 1
  %extract610.i = extractelement <16 x float> %652, i32 2
  %extract611.i = extractelement <16 x float> %652, i32 3
  %extract612.i = extractelement <16 x float> %652, i32 4
  %extract613.i = extractelement <16 x float> %652, i32 5
  %extract614.i = extractelement <16 x float> %652, i32 6
  %extract615.i = extractelement <16 x float> %652, i32 7
  %extract616.i = extractelement <16 x float> %652, i32 8
  %extract617.i = extractelement <16 x float> %652, i32 9
  %extract618.i = extractelement <16 x float> %652, i32 10
  %extract619.i = extractelement <16 x float> %652, i32 11
  %extract620.i = extractelement <16 x float> %652, i32 12
  %extract621.i = extractelement <16 x float> %652, i32 13
  %extract622.i = extractelement <16 x float> %652, i32 14
  %extract623.i = extractelement <16 x float> %652, i32 15
  store float %extract608.i, float addrspace(1)* %357, align 4
  store float %extract609.i, float addrspace(1)* %358, align 4
  store float %extract610.i, float addrspace(1)* %359, align 4
  store float %extract611.i, float addrspace(1)* %360, align 4
  store float %extract612.i, float addrspace(1)* %361, align 4
  store float %extract613.i, float addrspace(1)* %362, align 4
  store float %extract614.i, float addrspace(1)* %363, align 4
  store float %extract615.i, float addrspace(1)* %364, align 4
  store float %extract616.i, float addrspace(1)* %365, align 4
  store float %extract617.i, float addrspace(1)* %366, align 4
  store float %extract618.i, float addrspace(1)* %367, align 4
  store float %extract619.i, float addrspace(1)* %368, align 4
  store float %extract620.i, float addrspace(1)* %369, align 4
  store float %extract621.i, float addrspace(1)* %370, align 4
  store float %extract622.i, float addrspace(1)* %371, align 4
  store float %extract623.i, float addrspace(1)* %372, align 4
  %indvar.next33.i = add i64 %indvar32.i, 1
  %exitcond.i = icmp eq i64 %indvar.next33.i, 8
  br i1 %exitcond.i, label %._crit_edge8.i, label %355

._crit_edge8.i:                                   ; preds = %355
  %indvar.next29.i = add i64 %indvar28.i, 1
  %exitcond64.i = icmp eq i64 %indvar.next29.i, 8
  br i1 %exitcond64.i, label %._crit_edge13.i, label %bb.nph7.i

._crit_edge13.i:                                  ; preds = %._crit_edge8.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.DCT_CPU_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge13.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 13056
  br label %SyncBB.i

____Vectorized_.DCT_CPU_separated_args.exit:      ; preds = %._crit_edge13.i
  ret void
}

define void @__Vectorized_.DCT_CPU_VECTOR(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <8 x float> addrspace(1)**
  %4 = load <8 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to <8 x float> addrspace(1)**
  %7 = load <8 x float> addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 88
  %21 = bitcast i8* %20 to i8**
  %22 = load i8** %21, align 8
  %tmp61.i = lshr i32 %10, 3
  %tmp62.i = zext i32 %tmp61.i to i64
  %tmp33.i = zext i32 %10 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %23 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = add i64 %24, %26
  %broadcast1.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %29 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 1
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 1
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %tmp64.i = trunc i64 %33 to i32
  %tmp65.i = mul i32 %tmp64.i, %10
  %tmp66.i = and i32 %tmp65.i, 536870911
  %temp.i = insertelement <16 x i32> undef, i32 %tmp66.i, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp6832.i = trunc <16 x i64> %28 to <16 x i32>
  %tmp6933.i = add <16 x i32> %vector.i, %tmp6832.i
  %tmp7034.i = zext <16 x i32> %tmp6933.i to <16 x i64>
  %"&(pSB[currWI].offset)401.i" = add nuw i64 %CurrSBIndex..0.i, 4864
  %"&pSB[currWI].offset402.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)401.i"
  %CastToValueType403.i = bitcast i8* %"&pSB[currWI].offset402.i" to [64 x float]*
  %"&(pSB[currWI].offset)409.i" = add nuw i64 %CurrSBIndex..0.i, 5120
  %"&pSB[currWI].offset410.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)409.i"
  %CastToValueType411.i = bitcast i8* %"&pSB[currWI].offset410.i" to [64 x float]*
  %"&(pSB[currWI].offset)417.i" = add nuw i64 %CurrSBIndex..0.i, 5376
  %"&pSB[currWI].offset418.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)417.i"
  %CastToValueType419.i = bitcast i8* %"&pSB[currWI].offset418.i" to [64 x float]*
  %"&(pSB[currWI].offset)425.i" = add nuw i64 %CurrSBIndex..0.i, 5632
  %"&pSB[currWI].offset426.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)425.i"
  %CastToValueType427.i = bitcast i8* %"&pSB[currWI].offset426.i" to [64 x float]*
  %"&(pSB[currWI].offset)433.i" = add nuw i64 %CurrSBIndex..0.i, 5888
  %"&pSB[currWI].offset434.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)433.i"
  %CastToValueType435.i = bitcast i8* %"&pSB[currWI].offset434.i" to [64 x float]*
  %"&(pSB[currWI].offset)441.i" = add nuw i64 %CurrSBIndex..0.i, 6144
  %"&pSB[currWI].offset442.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)441.i"
  %CastToValueType443.i = bitcast i8* %"&pSB[currWI].offset442.i" to [64 x float]*
  %"&(pSB[currWI].offset)449.i" = add nuw i64 %CurrSBIndex..0.i, 6400
  %"&pSB[currWI].offset450.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)449.i"
  %CastToValueType451.i = bitcast i8* %"&pSB[currWI].offset450.i" to [64 x float]*
  %"&(pSB[currWI].offset)457.i" = add nuw i64 %CurrSBIndex..0.i, 6656
  %"&pSB[currWI].offset458.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)457.i"
  %CastToValueType459.i = bitcast i8* %"&pSB[currWI].offset458.i" to [64 x float]*
  %"&(pSB[currWI].offset)465.i" = add nuw i64 %CurrSBIndex..0.i, 6912
  %"&pSB[currWI].offset466.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)465.i"
  %CastToValueType467.i = bitcast i8* %"&pSB[currWI].offset466.i" to [64 x float]*
  %"&(pSB[currWI].offset)473.i" = add nuw i64 %CurrSBIndex..0.i, 7168
  %"&pSB[currWI].offset474.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)473.i"
  %CastToValueType475.i = bitcast i8* %"&pSB[currWI].offset474.i" to [64 x float]*
  %"&(pSB[currWI].offset)481.i" = add nuw i64 %CurrSBIndex..0.i, 7424
  %"&pSB[currWI].offset482.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)481.i"
  %CastToValueType483.i = bitcast i8* %"&pSB[currWI].offset482.i" to [64 x float]*
  %"&(pSB[currWI].offset)489.i" = add nuw i64 %CurrSBIndex..0.i, 7680
  %"&pSB[currWI].offset490.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)489.i"
  %CastToValueType491.i = bitcast i8* %"&pSB[currWI].offset490.i" to [64 x float]*
  %"&(pSB[currWI].offset)497.i" = add nuw i64 %CurrSBIndex..0.i, 7936
  %"&pSB[currWI].offset498.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)497.i"
  %CastToValueType499.i = bitcast i8* %"&pSB[currWI].offset498.i" to [64 x float]*
  %"&(pSB[currWI].offset)505.i" = add nuw i64 %CurrSBIndex..0.i, 8192
  %"&pSB[currWI].offset506.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)505.i"
  %CastToValueType507.i = bitcast i8* %"&pSB[currWI].offset506.i" to [64 x float]*
  %"&(pSB[currWI].offset)513.i" = add nuw i64 %CurrSBIndex..0.i, 8448
  %"&pSB[currWI].offset514.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)513.i"
  %CastToValueType515.i = bitcast i8* %"&pSB[currWI].offset514.i" to [64 x float]*
  %"&(pSB[currWI].offset)521.i" = add nuw i64 %CurrSBIndex..0.i, 8704
  %"&pSB[currWI].offset522.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)521.i"
  %CastToValueType523.i = bitcast i8* %"&pSB[currWI].offset522.i" to [64 x float]*
  br label %bb.nph14.i

bb.nph14.i:                                       ; preds = %._crit_edge15.i, %SyncBB.i
  %indvar52.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next53.i, %._crit_edge15.i ]
  %tmp63.i = mul i64 %tmp62.i, %indvar52.i
  %temp35.i = insertelement <16 x i64> undef, i64 %tmp63.i, i32 0
  %vector36.i = shufflevector <16 x i64> %temp35.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp7137.i = add <16 x i64> %tmp7034.i, %vector36.i
  %34 = and <16 x i64> %tmp7137.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract.i = extractelement <16 x i64> %34, i32 0
  %extract38.i = extractelement <16 x i64> %34, i32 1
  %extract39.i = extractelement <16 x i64> %34, i32 2
  %extract40.i = extractelement <16 x i64> %34, i32 3
  %extract41.i = extractelement <16 x i64> %34, i32 4
  %extract42.i = extractelement <16 x i64> %34, i32 5
  %extract43.i = extractelement <16 x i64> %34, i32 6
  %extract44.i = extractelement <16 x i64> %34, i32 7
  %extract45.i = extractelement <16 x i64> %34, i32 8
  %extract46.i = extractelement <16 x i64> %34, i32 9
  %extract47.i = extractelement <16 x i64> %34, i32 10
  %extract48.i = extractelement <16 x i64> %34, i32 11
  %extract49.i = extractelement <16 x i64> %34, i32 12
  %extract50.i = extractelement <16 x i64> %34, i32 13
  %extract51.i = extractelement <16 x i64> %34, i32 14
  %extract52.i = extractelement <16 x i64> %34, i32 15
  %35 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract.i
  %36 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract38.i
  %37 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract39.i
  %38 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract40.i
  %39 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract41.i
  %40 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract42.i
  %41 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract43.i
  %42 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract44.i
  %43 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract45.i
  %44 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract46.i
  %45 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract47.i
  %46 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract48.i
  %47 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract49.i
  %48 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract50.i
  %49 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract51.i
  %50 = getelementptr inbounds <8 x float> addrspace(1)* %4, i64 %extract52.i
  %51 = load <8 x float> addrspace(1)* %35, align 32
  %52 = load <8 x float> addrspace(1)* %36, align 32
  %53 = load <8 x float> addrspace(1)* %37, align 32
  %54 = load <8 x float> addrspace(1)* %38, align 32
  %55 = load <8 x float> addrspace(1)* %39, align 32
  %56 = load <8 x float> addrspace(1)* %40, align 32
  %57 = load <8 x float> addrspace(1)* %41, align 32
  %58 = load <8 x float> addrspace(1)* %42, align 32
  %59 = load <8 x float> addrspace(1)* %43, align 32
  %60 = load <8 x float> addrspace(1)* %44, align 32
  %61 = load <8 x float> addrspace(1)* %45, align 32
  %62 = load <8 x float> addrspace(1)* %46, align 32
  %63 = load <8 x float> addrspace(1)* %47, align 32
  %64 = load <8 x float> addrspace(1)* %48, align 32
  %65 = load <8 x float> addrspace(1)* %49, align 32
  %66 = load <8 x float> addrspace(1)* %50, align 32
  %67 = extractelement <8 x float> %51, i32 0
  %68 = extractelement <8 x float> %52, i32 0
  %69 = extractelement <8 x float> %53, i32 0
  %70 = extractelement <8 x float> %54, i32 0
  %71 = extractelement <8 x float> %55, i32 0
  %72 = extractelement <8 x float> %56, i32 0
  %73 = extractelement <8 x float> %57, i32 0
  %74 = extractelement <8 x float> %58, i32 0
  %75 = extractelement <8 x float> %59, i32 0
  %76 = extractelement <8 x float> %60, i32 0
  %77 = extractelement <8 x float> %61, i32 0
  %78 = extractelement <8 x float> %62, i32 0
  %79 = extractelement <8 x float> %63, i32 0
  %80 = extractelement <8 x float> %64, i32 0
  %81 = extractelement <8 x float> %65, i32 0
  %82 = extractelement <8 x float> %66, i32 0
  %temp.vect.i = insertelement <16 x float> undef, float %67, i32 0
  %temp.vect55.i = insertelement <16 x float> %temp.vect.i, float %68, i32 1
  %temp.vect56.i = insertelement <16 x float> %temp.vect55.i, float %69, i32 2
  %temp.vect57.i = insertelement <16 x float> %temp.vect56.i, float %70, i32 3
  %temp.vect58.i = insertelement <16 x float> %temp.vect57.i, float %71, i32 4
  %temp.vect59.i = insertelement <16 x float> %temp.vect58.i, float %72, i32 5
  %temp.vect60.i = insertelement <16 x float> %temp.vect59.i, float %73, i32 6
  %temp.vect61.i = insertelement <16 x float> %temp.vect60.i, float %74, i32 7
  %temp.vect62.i = insertelement <16 x float> %temp.vect61.i, float %75, i32 8
  %temp.vect63.i = insertelement <16 x float> %temp.vect62.i, float %76, i32 9
  %temp.vect64.i = insertelement <16 x float> %temp.vect63.i, float %77, i32 10
  %temp.vect65.i = insertelement <16 x float> %temp.vect64.i, float %78, i32 11
  %temp.vect66.i = insertelement <16 x float> %temp.vect65.i, float %79, i32 12
  %temp.vect67.i = insertelement <16 x float> %temp.vect66.i, float %80, i32 13
  %temp.vect68.i = insertelement <16 x float> %temp.vect67.i, float %81, i32 14
  %temp.vect69.i = insertelement <16 x float> %temp.vect68.i, float %82, i32 15
  %83 = extractelement <8 x float> %51, i32 1
  %84 = extractelement <8 x float> %52, i32 1
  %85 = extractelement <8 x float> %53, i32 1
  %86 = extractelement <8 x float> %54, i32 1
  %87 = extractelement <8 x float> %55, i32 1
  %88 = extractelement <8 x float> %56, i32 1
  %89 = extractelement <8 x float> %57, i32 1
  %90 = extractelement <8 x float> %58, i32 1
  %91 = extractelement <8 x float> %59, i32 1
  %92 = extractelement <8 x float> %60, i32 1
  %93 = extractelement <8 x float> %61, i32 1
  %94 = extractelement <8 x float> %62, i32 1
  %95 = extractelement <8 x float> %63, i32 1
  %96 = extractelement <8 x float> %64, i32 1
  %97 = extractelement <8 x float> %65, i32 1
  %98 = extractelement <8 x float> %66, i32 1
  %temp.vect72.i = insertelement <16 x float> undef, float %83, i32 0
  %temp.vect73.i = insertelement <16 x float> %temp.vect72.i, float %84, i32 1
  %temp.vect74.i = insertelement <16 x float> %temp.vect73.i, float %85, i32 2
  %temp.vect75.i = insertelement <16 x float> %temp.vect74.i, float %86, i32 3
  %temp.vect76.i = insertelement <16 x float> %temp.vect75.i, float %87, i32 4
  %temp.vect77.i = insertelement <16 x float> %temp.vect76.i, float %88, i32 5
  %temp.vect78.i = insertelement <16 x float> %temp.vect77.i, float %89, i32 6
  %temp.vect79.i = insertelement <16 x float> %temp.vect78.i, float %90, i32 7
  %temp.vect80.i = insertelement <16 x float> %temp.vect79.i, float %91, i32 8
  %temp.vect81.i = insertelement <16 x float> %temp.vect80.i, float %92, i32 9
  %temp.vect82.i = insertelement <16 x float> %temp.vect81.i, float %93, i32 10
  %temp.vect83.i = insertelement <16 x float> %temp.vect82.i, float %94, i32 11
  %temp.vect84.i = insertelement <16 x float> %temp.vect83.i, float %95, i32 12
  %temp.vect85.i = insertelement <16 x float> %temp.vect84.i, float %96, i32 13
  %temp.vect86.i = insertelement <16 x float> %temp.vect85.i, float %97, i32 14
  %temp.vect87.i = insertelement <16 x float> %temp.vect86.i, float %98, i32 15
  %99 = extractelement <8 x float> %51, i32 2
  %100 = extractelement <8 x float> %52, i32 2
  %101 = extractelement <8 x float> %53, i32 2
  %102 = extractelement <8 x float> %54, i32 2
  %103 = extractelement <8 x float> %55, i32 2
  %104 = extractelement <8 x float> %56, i32 2
  %105 = extractelement <8 x float> %57, i32 2
  %106 = extractelement <8 x float> %58, i32 2
  %107 = extractelement <8 x float> %59, i32 2
  %108 = extractelement <8 x float> %60, i32 2
  %109 = extractelement <8 x float> %61, i32 2
  %110 = extractelement <8 x float> %62, i32 2
  %111 = extractelement <8 x float> %63, i32 2
  %112 = extractelement <8 x float> %64, i32 2
  %113 = extractelement <8 x float> %65, i32 2
  %114 = extractelement <8 x float> %66, i32 2
  %temp.vect90.i = insertelement <16 x float> undef, float %99, i32 0
  %temp.vect91.i = insertelement <16 x float> %temp.vect90.i, float %100, i32 1
  %temp.vect92.i = insertelement <16 x float> %temp.vect91.i, float %101, i32 2
  %temp.vect93.i = insertelement <16 x float> %temp.vect92.i, float %102, i32 3
  %temp.vect94.i = insertelement <16 x float> %temp.vect93.i, float %103, i32 4
  %temp.vect95.i = insertelement <16 x float> %temp.vect94.i, float %104, i32 5
  %temp.vect96.i = insertelement <16 x float> %temp.vect95.i, float %105, i32 6
  %temp.vect97.i = insertelement <16 x float> %temp.vect96.i, float %106, i32 7
  %temp.vect98.i = insertelement <16 x float> %temp.vect97.i, float %107, i32 8
  %temp.vect99.i = insertelement <16 x float> %temp.vect98.i, float %108, i32 9
  %temp.vect100.i = insertelement <16 x float> %temp.vect99.i, float %109, i32 10
  %temp.vect101.i = insertelement <16 x float> %temp.vect100.i, float %110, i32 11
  %temp.vect102.i = insertelement <16 x float> %temp.vect101.i, float %111, i32 12
  %temp.vect103.i = insertelement <16 x float> %temp.vect102.i, float %112, i32 13
  %temp.vect104.i = insertelement <16 x float> %temp.vect103.i, float %113, i32 14
  %temp.vect105.i = insertelement <16 x float> %temp.vect104.i, float %114, i32 15
  %115 = extractelement <8 x float> %51, i32 3
  %116 = extractelement <8 x float> %52, i32 3
  %117 = extractelement <8 x float> %53, i32 3
  %118 = extractelement <8 x float> %54, i32 3
  %119 = extractelement <8 x float> %55, i32 3
  %120 = extractelement <8 x float> %56, i32 3
  %121 = extractelement <8 x float> %57, i32 3
  %122 = extractelement <8 x float> %58, i32 3
  %123 = extractelement <8 x float> %59, i32 3
  %124 = extractelement <8 x float> %60, i32 3
  %125 = extractelement <8 x float> %61, i32 3
  %126 = extractelement <8 x float> %62, i32 3
  %127 = extractelement <8 x float> %63, i32 3
  %128 = extractelement <8 x float> %64, i32 3
  %129 = extractelement <8 x float> %65, i32 3
  %130 = extractelement <8 x float> %66, i32 3
  %temp.vect108.i = insertelement <16 x float> undef, float %115, i32 0
  %temp.vect109.i = insertelement <16 x float> %temp.vect108.i, float %116, i32 1
  %temp.vect110.i = insertelement <16 x float> %temp.vect109.i, float %117, i32 2
  %temp.vect111.i = insertelement <16 x float> %temp.vect110.i, float %118, i32 3
  %temp.vect112.i = insertelement <16 x float> %temp.vect111.i, float %119, i32 4
  %temp.vect113.i = insertelement <16 x float> %temp.vect112.i, float %120, i32 5
  %temp.vect114.i = insertelement <16 x float> %temp.vect113.i, float %121, i32 6
  %temp.vect115.i = insertelement <16 x float> %temp.vect114.i, float %122, i32 7
  %temp.vect116.i = insertelement <16 x float> %temp.vect115.i, float %123, i32 8
  %temp.vect117.i = insertelement <16 x float> %temp.vect116.i, float %124, i32 9
  %temp.vect118.i = insertelement <16 x float> %temp.vect117.i, float %125, i32 10
  %temp.vect119.i = insertelement <16 x float> %temp.vect118.i, float %126, i32 11
  %temp.vect120.i = insertelement <16 x float> %temp.vect119.i, float %127, i32 12
  %temp.vect121.i = insertelement <16 x float> %temp.vect120.i, float %128, i32 13
  %temp.vect122.i = insertelement <16 x float> %temp.vect121.i, float %129, i32 14
  %temp.vect123.i = insertelement <16 x float> %temp.vect122.i, float %130, i32 15
  %131 = extractelement <8 x float> %51, i32 4
  %132 = extractelement <8 x float> %52, i32 4
  %133 = extractelement <8 x float> %53, i32 4
  %134 = extractelement <8 x float> %54, i32 4
  %135 = extractelement <8 x float> %55, i32 4
  %136 = extractelement <8 x float> %56, i32 4
  %137 = extractelement <8 x float> %57, i32 4
  %138 = extractelement <8 x float> %58, i32 4
  %139 = extractelement <8 x float> %59, i32 4
  %140 = extractelement <8 x float> %60, i32 4
  %141 = extractelement <8 x float> %61, i32 4
  %142 = extractelement <8 x float> %62, i32 4
  %143 = extractelement <8 x float> %63, i32 4
  %144 = extractelement <8 x float> %64, i32 4
  %145 = extractelement <8 x float> %65, i32 4
  %146 = extractelement <8 x float> %66, i32 4
  %temp.vect126.i = insertelement <16 x float> undef, float %131, i32 0
  %temp.vect127.i = insertelement <16 x float> %temp.vect126.i, float %132, i32 1
  %temp.vect128.i = insertelement <16 x float> %temp.vect127.i, float %133, i32 2
  %temp.vect129.i = insertelement <16 x float> %temp.vect128.i, float %134, i32 3
  %temp.vect130.i = insertelement <16 x float> %temp.vect129.i, float %135, i32 4
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %136, i32 5
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %137, i32 6
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %138, i32 7
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %139, i32 8
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %140, i32 9
  %temp.vect136.i = insertelement <16 x float> %temp.vect135.i, float %141, i32 10
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %142, i32 11
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %143, i32 12
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %144, i32 13
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %145, i32 14
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %146, i32 15
  %147 = extractelement <8 x float> %51, i32 5
  %148 = extractelement <8 x float> %52, i32 5
  %149 = extractelement <8 x float> %53, i32 5
  %150 = extractelement <8 x float> %54, i32 5
  %151 = extractelement <8 x float> %55, i32 5
  %152 = extractelement <8 x float> %56, i32 5
  %153 = extractelement <8 x float> %57, i32 5
  %154 = extractelement <8 x float> %58, i32 5
  %155 = extractelement <8 x float> %59, i32 5
  %156 = extractelement <8 x float> %60, i32 5
  %157 = extractelement <8 x float> %61, i32 5
  %158 = extractelement <8 x float> %62, i32 5
  %159 = extractelement <8 x float> %63, i32 5
  %160 = extractelement <8 x float> %64, i32 5
  %161 = extractelement <8 x float> %65, i32 5
  %162 = extractelement <8 x float> %66, i32 5
  %temp.vect144.i = insertelement <16 x float> undef, float %147, i32 0
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %148, i32 1
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %149, i32 2
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %150, i32 3
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %151, i32 4
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %152, i32 5
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %153, i32 6
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %154, i32 7
  %temp.vect152.i = insertelement <16 x float> %temp.vect151.i, float %155, i32 8
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %156, i32 9
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %157, i32 10
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %158, i32 11
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %159, i32 12
  %temp.vect157.i = insertelement <16 x float> %temp.vect156.i, float %160, i32 13
  %temp.vect158.i = insertelement <16 x float> %temp.vect157.i, float %161, i32 14
  %temp.vect159.i = insertelement <16 x float> %temp.vect158.i, float %162, i32 15
  %163 = extractelement <8 x float> %51, i32 6
  %164 = extractelement <8 x float> %52, i32 6
  %165 = extractelement <8 x float> %53, i32 6
  %166 = extractelement <8 x float> %54, i32 6
  %167 = extractelement <8 x float> %55, i32 6
  %168 = extractelement <8 x float> %56, i32 6
  %169 = extractelement <8 x float> %57, i32 6
  %170 = extractelement <8 x float> %58, i32 6
  %171 = extractelement <8 x float> %59, i32 6
  %172 = extractelement <8 x float> %60, i32 6
  %173 = extractelement <8 x float> %61, i32 6
  %174 = extractelement <8 x float> %62, i32 6
  %175 = extractelement <8 x float> %63, i32 6
  %176 = extractelement <8 x float> %64, i32 6
  %177 = extractelement <8 x float> %65, i32 6
  %178 = extractelement <8 x float> %66, i32 6
  %temp.vect162.i = insertelement <16 x float> undef, float %163, i32 0
  %temp.vect163.i = insertelement <16 x float> %temp.vect162.i, float %164, i32 1
  %temp.vect164.i = insertelement <16 x float> %temp.vect163.i, float %165, i32 2
  %temp.vect165.i = insertelement <16 x float> %temp.vect164.i, float %166, i32 3
  %temp.vect166.i = insertelement <16 x float> %temp.vect165.i, float %167, i32 4
  %temp.vect167.i = insertelement <16 x float> %temp.vect166.i, float %168, i32 5
  %temp.vect168.i = insertelement <16 x float> %temp.vect167.i, float %169, i32 6
  %temp.vect169.i = insertelement <16 x float> %temp.vect168.i, float %170, i32 7
  %temp.vect170.i = insertelement <16 x float> %temp.vect169.i, float %171, i32 8
  %temp.vect171.i = insertelement <16 x float> %temp.vect170.i, float %172, i32 9
  %temp.vect172.i = insertelement <16 x float> %temp.vect171.i, float %173, i32 10
  %temp.vect173.i = insertelement <16 x float> %temp.vect172.i, float %174, i32 11
  %temp.vect174.i = insertelement <16 x float> %temp.vect173.i, float %175, i32 12
  %temp.vect175.i = insertelement <16 x float> %temp.vect174.i, float %176, i32 13
  %temp.vect176.i = insertelement <16 x float> %temp.vect175.i, float %177, i32 14
  %temp.vect177.i = insertelement <16 x float> %temp.vect176.i, float %178, i32 15
  %179 = extractelement <8 x float> %51, i32 7
  %180 = extractelement <8 x float> %52, i32 7
  %181 = extractelement <8 x float> %53, i32 7
  %182 = extractelement <8 x float> %54, i32 7
  %183 = extractelement <8 x float> %55, i32 7
  %184 = extractelement <8 x float> %56, i32 7
  %185 = extractelement <8 x float> %57, i32 7
  %186 = extractelement <8 x float> %58, i32 7
  %187 = extractelement <8 x float> %59, i32 7
  %188 = extractelement <8 x float> %60, i32 7
  %189 = extractelement <8 x float> %61, i32 7
  %190 = extractelement <8 x float> %62, i32 7
  %191 = extractelement <8 x float> %63, i32 7
  %192 = extractelement <8 x float> %64, i32 7
  %193 = extractelement <8 x float> %65, i32 7
  %194 = extractelement <8 x float> %66, i32 7
  %temp.vect180.i = insertelement <16 x float> undef, float %179, i32 0
  %temp.vect181.i = insertelement <16 x float> %temp.vect180.i, float %180, i32 1
  %temp.vect182.i = insertelement <16 x float> %temp.vect181.i, float %181, i32 2
  %temp.vect183.i = insertelement <16 x float> %temp.vect182.i, float %182, i32 3
  %temp.vect184.i = insertelement <16 x float> %temp.vect183.i, float %183, i32 4
  %temp.vect185.i = insertelement <16 x float> %temp.vect184.i, float %184, i32 5
  %temp.vect186.i = insertelement <16 x float> %temp.vect185.i, float %185, i32 6
  %temp.vect187.i = insertelement <16 x float> %temp.vect186.i, float %186, i32 7
  %temp.vect188.i = insertelement <16 x float> %temp.vect187.i, float %187, i32 8
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %188, i32 9
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %189, i32 10
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %190, i32 11
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %191, i32 12
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %192, i32 13
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %193, i32 14
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %194, i32 15
  br label %195

; <label>:195                                     ; preds = %195, %bb.nph14.i
  %indvar49.i = phi i64 [ 0, %bb.nph14.i ], [ %indvar.next50.i, %195 ]
  %tmp58.i = shl i64 %indvar49.i, 3
  %tmp59.i = add i64 %indvar52.i, %tmp58.i
  %196 = getelementptr [64 x float]* %CastToValueType403.i, i64 0, i64 %tmp59.i
  %197 = getelementptr [64 x float]* %CastToValueType411.i, i64 0, i64 %tmp59.i
  %198 = getelementptr [64 x float]* %CastToValueType419.i, i64 0, i64 %tmp59.i
  %199 = getelementptr [64 x float]* %CastToValueType427.i, i64 0, i64 %tmp59.i
  %200 = getelementptr [64 x float]* %CastToValueType435.i, i64 0, i64 %tmp59.i
  %201 = getelementptr [64 x float]* %CastToValueType443.i, i64 0, i64 %tmp59.i
  %202 = getelementptr [64 x float]* %CastToValueType451.i, i64 0, i64 %tmp59.i
  %203 = getelementptr [64 x float]* %CastToValueType459.i, i64 0, i64 %tmp59.i
  %204 = getelementptr [64 x float]* %CastToValueType467.i, i64 0, i64 %tmp59.i
  %205 = getelementptr [64 x float]* %CastToValueType475.i, i64 0, i64 %tmp59.i
  %206 = getelementptr [64 x float]* %CastToValueType483.i, i64 0, i64 %tmp59.i
  %207 = getelementptr [64 x float]* %CastToValueType491.i, i64 0, i64 %tmp59.i
  %208 = getelementptr [64 x float]* %CastToValueType499.i, i64 0, i64 %tmp59.i
  %209 = getelementptr [64 x float]* %CastToValueType507.i, i64 0, i64 %tmp59.i
  %210 = getelementptr [64 x float]* %CastToValueType515.i, i64 0, i64 %tmp59.i
  %211 = getelementptr [64 x float]* %CastToValueType523.i, i64 0, i64 %tmp59.i
  %scevgep56.i = getelementptr <8 x float> addrspace(1)* %7, i64 %indvar49.i
  %212 = load <8 x float> addrspace(1)* %scevgep56.i, align 32
  %scalar.i = extractelement <8 x float> %212, i32 0
  %temp53.i = insertelement <16 x float> undef, float %scalar.i, i32 0
  %vector54.i = shufflevector <16 x float> %temp53.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar1.i = extractelement <8 x float> %212, i32 1
  %temp70.i = insertelement <16 x float> undef, float %scalar1.i, i32 0
  %vector71.i = shufflevector <16 x float> %temp70.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar2.i = extractelement <8 x float> %212, i32 2
  %temp88.i = insertelement <16 x float> undef, float %scalar2.i, i32 0
  %vector89.i = shufflevector <16 x float> %temp88.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar3.i = extractelement <8 x float> %212, i32 3
  %temp106.i = insertelement <16 x float> undef, float %scalar3.i, i32 0
  %vector107.i = shufflevector <16 x float> %temp106.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar4.i = extractelement <8 x float> %212, i32 4
  %temp124.i = insertelement <16 x float> undef, float %scalar4.i, i32 0
  %vector125.i = shufflevector <16 x float> %temp124.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar5.i = extractelement <8 x float> %212, i32 5
  %temp142.i = insertelement <16 x float> undef, float %scalar5.i, i32 0
  %vector143.i = shufflevector <16 x float> %temp142.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar6.i = extractelement <8 x float> %212, i32 6
  %temp160.i = insertelement <16 x float> undef, float %scalar6.i, i32 0
  %vector161.i = shufflevector <16 x float> %temp160.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar7.i = extractelement <8 x float> %212, i32 7
  %temp178.i = insertelement <16 x float> undef, float %scalar7.i, i32 0
  %vector179.i = shufflevector <16 x float> %temp178.i, <16 x float> undef, <16 x i32> zeroinitializer
  %213 = fmul <16 x float> %vector54.i, %temp.vect69.i
  %214 = fmul <16 x float> %vector71.i, %temp.vect87.i
  %215 = fmul <16 x float> %vector89.i, %temp.vect105.i
  %216 = fmul <16 x float> %vector107.i, %temp.vect123.i
  %217 = fmul <16 x float> %vector125.i, %temp.vect141.i
  %218 = fmul <16 x float> %vector143.i, %temp.vect159.i
  %219 = fmul <16 x float> %vector161.i, %temp.vect177.i
  %220 = fmul <16 x float> %vector179.i, %temp.vect195.i
  %221 = fadd <16 x float> %213, %214
  %222 = fadd <16 x float> %221, %215
  %223 = fadd <16 x float> %222, %216
  %224 = fadd <16 x float> %223, %217
  %225 = fadd <16 x float> %224, %218
  %226 = fadd <16 x float> %225, %219
  %227 = fadd <16 x float> %226, %220
  %extract196.i = extractelement <16 x float> %227, i32 0
  %extract197.i = extractelement <16 x float> %227, i32 1
  %extract198.i = extractelement <16 x float> %227, i32 2
  %extract199.i = extractelement <16 x float> %227, i32 3
  %extract200.i = extractelement <16 x float> %227, i32 4
  %extract201.i = extractelement <16 x float> %227, i32 5
  %extract202.i = extractelement <16 x float> %227, i32 6
  %extract203.i = extractelement <16 x float> %227, i32 7
  %extract204.i = extractelement <16 x float> %227, i32 8
  %extract205.i = extractelement <16 x float> %227, i32 9
  %extract206.i = extractelement <16 x float> %227, i32 10
  %extract207.i = extractelement <16 x float> %227, i32 11
  %extract208.i = extractelement <16 x float> %227, i32 12
  %extract209.i = extractelement <16 x float> %227, i32 13
  %extract210.i = extractelement <16 x float> %227, i32 14
  %extract211.i = extractelement <16 x float> %227, i32 15
  store float %extract196.i, float* %196, align 4
  store float %extract197.i, float* %197, align 4
  store float %extract198.i, float* %198, align 4
  store float %extract199.i, float* %199, align 4
  store float %extract200.i, float* %200, align 4
  store float %extract201.i, float* %201, align 4
  store float %extract202.i, float* %202, align 4
  store float %extract203.i, float* %203, align 4
  store float %extract204.i, float* %204, align 4
  store float %extract205.i, float* %205, align 4
  store float %extract206.i, float* %206, align 4
  store float %extract207.i, float* %207, align 4
  store float %extract208.i, float* %208, align 4
  store float %extract209.i, float* %209, align 4
  store float %extract210.i, float* %210, align 4
  store float %extract211.i, float* %211, align 4
  %indvar.next50.i = add i64 %indvar49.i, 1
  %exitcond51.i = icmp eq i64 %indvar.next50.i, 8
  br i1 %exitcond51.i, label %._crit_edge15.i, label %195

._crit_edge15.i:                                  ; preds = %195
  %indvar.next53.i = add i64 %indvar52.i, 1
  %exitcond57.i = icmp eq i64 %indvar.next53.i, 8
  br i1 %exitcond57.i, label %bb.nph9.i, label %bb.nph14.i

bb.nph9.i:                                        ; preds = %._crit_edge15.i
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 4864
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset).i"
  %"&(pSB[currWI].offset)405.i" = add nuw i64 %CurrSBIndex..0.i, 5120
  %"&pSB[currWI].offset406.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)405.i"
  %"&(pSB[currWI].offset)413.i" = add nuw i64 %CurrSBIndex..0.i, 5376
  %"&pSB[currWI].offset414.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)413.i"
  %"&(pSB[currWI].offset)421.i" = add nuw i64 %CurrSBIndex..0.i, 5632
  %"&pSB[currWI].offset422.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)421.i"
  %"&(pSB[currWI].offset)429.i" = add nuw i64 %CurrSBIndex..0.i, 5888
  %"&pSB[currWI].offset430.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)429.i"
  %"&(pSB[currWI].offset)437.i" = add nuw i64 %CurrSBIndex..0.i, 6144
  %"&pSB[currWI].offset438.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)437.i"
  %"&(pSB[currWI].offset)445.i" = add nuw i64 %CurrSBIndex..0.i, 6400
  %"&pSB[currWI].offset446.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)445.i"
  %"&(pSB[currWI].offset)453.i" = add nuw i64 %CurrSBIndex..0.i, 6656
  %"&pSB[currWI].offset454.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)453.i"
  %"&(pSB[currWI].offset)461.i" = add nuw i64 %CurrSBIndex..0.i, 6912
  %"&pSB[currWI].offset462.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)461.i"
  %"&(pSB[currWI].offset)469.i" = add nuw i64 %CurrSBIndex..0.i, 7168
  %"&pSB[currWI].offset470.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)469.i"
  %"&(pSB[currWI].offset)477.i" = add nuw i64 %CurrSBIndex..0.i, 7424
  %"&pSB[currWI].offset478.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)477.i"
  %"&(pSB[currWI].offset)485.i" = add nuw i64 %CurrSBIndex..0.i, 7680
  %"&pSB[currWI].offset486.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)485.i"
  %"&(pSB[currWI].offset)493.i" = add nuw i64 %CurrSBIndex..0.i, 7936
  %"&pSB[currWI].offset494.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)493.i"
  %"&(pSB[currWI].offset)501.i" = add nuw i64 %CurrSBIndex..0.i, 8192
  %"&pSB[currWI].offset502.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)501.i"
  %"&(pSB[currWI].offset)509.i" = add nuw i64 %CurrSBIndex..0.i, 8448
  %"&pSB[currWI].offset510.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)509.i"
  %"&(pSB[currWI].offset)517.i" = add nuw i64 %CurrSBIndex..0.i, 8704
  %"&pSB[currWI].offset518.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)517.i"
  %"&(pSB[currWI].offset)529.i" = add nuw i64 %CurrSBIndex..0.i, 8960
  %"&pSB[currWI].offset530.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)529.i"
  %CastToValueType531.i = bitcast i8* %"&pSB[currWI].offset530.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)537.i" = add nuw i64 %CurrSBIndex..0.i, 9216
  %"&pSB[currWI].offset538.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)537.i"
  %CastToValueType539.i = bitcast i8* %"&pSB[currWI].offset538.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)545.i" = add nuw i64 %CurrSBIndex..0.i, 9472
  %"&pSB[currWI].offset546.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)545.i"
  %CastToValueType547.i = bitcast i8* %"&pSB[currWI].offset546.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)553.i" = add nuw i64 %CurrSBIndex..0.i, 9728
  %"&pSB[currWI].offset554.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)553.i"
  %CastToValueType555.i = bitcast i8* %"&pSB[currWI].offset554.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)561.i" = add nuw i64 %CurrSBIndex..0.i, 9984
  %"&pSB[currWI].offset562.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)561.i"
  %CastToValueType563.i = bitcast i8* %"&pSB[currWI].offset562.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)569.i" = add nuw i64 %CurrSBIndex..0.i, 10240
  %"&pSB[currWI].offset570.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)569.i"
  %CastToValueType571.i = bitcast i8* %"&pSB[currWI].offset570.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)577.i" = add nuw i64 %CurrSBIndex..0.i, 10496
  %"&pSB[currWI].offset578.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)577.i"
  %CastToValueType579.i = bitcast i8* %"&pSB[currWI].offset578.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)585.i" = add nuw i64 %CurrSBIndex..0.i, 10752
  %"&pSB[currWI].offset586.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)585.i"
  %CastToValueType587.i = bitcast i8* %"&pSB[currWI].offset586.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)593.i" = add nuw i64 %CurrSBIndex..0.i, 11008
  %"&pSB[currWI].offset594.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)593.i"
  %CastToValueType595.i = bitcast i8* %"&pSB[currWI].offset594.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)601.i" = add nuw i64 %CurrSBIndex..0.i, 11264
  %"&pSB[currWI].offset602.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)601.i"
  %CastToValueType603.i = bitcast i8* %"&pSB[currWI].offset602.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)609.i" = add nuw i64 %CurrSBIndex..0.i, 11520
  %"&pSB[currWI].offset610.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)609.i"
  %CastToValueType611.i = bitcast i8* %"&pSB[currWI].offset610.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)617.i" = add nuw i64 %CurrSBIndex..0.i, 11776
  %"&pSB[currWI].offset618.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)617.i"
  %CastToValueType619.i = bitcast i8* %"&pSB[currWI].offset618.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)625.i" = add nuw i64 %CurrSBIndex..0.i, 12032
  %"&pSB[currWI].offset626.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)625.i"
  %CastToValueType627.i = bitcast i8* %"&pSB[currWI].offset626.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)633.i" = add nuw i64 %CurrSBIndex..0.i, 12288
  %"&pSB[currWI].offset634.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)633.i"
  %CastToValueType635.i = bitcast i8* %"&pSB[currWI].offset634.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)641.i" = add nuw i64 %CurrSBIndex..0.i, 12544
  %"&pSB[currWI].offset642.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)641.i"
  %CastToValueType643.i = bitcast i8* %"&pSB[currWI].offset642.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)649.i" = add nuw i64 %CurrSBIndex..0.i, 12800
  %"&pSB[currWI].offset650.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)649.i"
  %CastToValueType651.i = bitcast i8* %"&pSB[currWI].offset650.i" to [8 x <8 x float>]*
  br label %228

; <label>:228                                     ; preds = %228, %bb.nph9.i
  %indvar45.i = phi i64 [ 0, %bb.nph9.i ], [ %indvar.next46.i, %228 ]
  %229 = getelementptr [8 x <8 x float>]* %CastToValueType531.i, i64 0, i64 %indvar45.i
  %230 = getelementptr [8 x <8 x float>]* %CastToValueType539.i, i64 0, i64 %indvar45.i
  %231 = getelementptr [8 x <8 x float>]* %CastToValueType547.i, i64 0, i64 %indvar45.i
  %232 = getelementptr [8 x <8 x float>]* %CastToValueType555.i, i64 0, i64 %indvar45.i
  %233 = getelementptr [8 x <8 x float>]* %CastToValueType563.i, i64 0, i64 %indvar45.i
  %234 = getelementptr [8 x <8 x float>]* %CastToValueType571.i, i64 0, i64 %indvar45.i
  %235 = getelementptr [8 x <8 x float>]* %CastToValueType579.i, i64 0, i64 %indvar45.i
  %236 = getelementptr [8 x <8 x float>]* %CastToValueType587.i, i64 0, i64 %indvar45.i
  %237 = getelementptr [8 x <8 x float>]* %CastToValueType595.i, i64 0, i64 %indvar45.i
  %238 = getelementptr [8 x <8 x float>]* %CastToValueType603.i, i64 0, i64 %indvar45.i
  %239 = getelementptr [8 x <8 x float>]* %CastToValueType611.i, i64 0, i64 %indvar45.i
  %240 = getelementptr [8 x <8 x float>]* %CastToValueType619.i, i64 0, i64 %indvar45.i
  %241 = getelementptr [8 x <8 x float>]* %CastToValueType627.i, i64 0, i64 %indvar45.i
  %242 = getelementptr [8 x <8 x float>]* %CastToValueType635.i, i64 0, i64 %indvar45.i
  %243 = getelementptr [8 x <8 x float>]* %CastToValueType643.i, i64 0, i64 %indvar45.i
  %244 = getelementptr [8 x <8 x float>]* %CastToValueType651.i, i64 0, i64 %indvar45.i
  %245 = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  %246 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i.i = getelementptr inbounds i32* %245, i64 %mul25.i.i.i.i
  %conv27.i.i.i.i = bitcast i32* %add.ptr26.i.i.i.i to i8*
  %247 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %246, i16 255, i8* %conv27.i.i.i.i, i32 0, i32 0) nounwind
  %248 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %247, i16 255, i8* %conv27.i.i.i.i, i32 0, i32 0) nounwind
  %tmp3.i.i = shufflevector <16 x float> %248, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %249 = bitcast i8* %"&pSB[currWI].offset406.i" to i32*
  %250 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i1.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i2.i = getelementptr inbounds i32* %249, i64 %mul25.i.i.i1.i
  %conv27.i.i.i3.i = bitcast i32* %add.ptr26.i.i.i2.i to i8*
  %251 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %250, i16 255, i8* %conv27.i.i.i3.i, i32 0, i32 0) nounwind
  %252 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %251, i16 255, i8* %conv27.i.i.i3.i, i32 0, i32 0) nounwind
  %tmp3.i4.i = shufflevector <16 x float> %252, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %253 = bitcast i8* %"&pSB[currWI].offset414.i" to i32*
  %254 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i5.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i6.i = getelementptr inbounds i32* %253, i64 %mul25.i.i.i5.i
  %conv27.i.i.i7.i = bitcast i32* %add.ptr26.i.i.i6.i to i8*
  %255 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %254, i16 255, i8* %conv27.i.i.i7.i, i32 0, i32 0) nounwind
  %256 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %255, i16 255, i8* %conv27.i.i.i7.i, i32 0, i32 0) nounwind
  %tmp3.i8.i = shufflevector <16 x float> %256, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %257 = bitcast i8* %"&pSB[currWI].offset422.i" to i32*
  %258 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i9.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i10.i = getelementptr inbounds i32* %257, i64 %mul25.i.i.i9.i
  %conv27.i.i.i11.i = bitcast i32* %add.ptr26.i.i.i10.i to i8*
  %259 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %258, i16 255, i8* %conv27.i.i.i11.i, i32 0, i32 0) nounwind
  %260 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %259, i16 255, i8* %conv27.i.i.i11.i, i32 0, i32 0) nounwind
  %tmp3.i12.i = shufflevector <16 x float> %260, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %261 = bitcast i8* %"&pSB[currWI].offset430.i" to i32*
  %262 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i13.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i14.i = getelementptr inbounds i32* %261, i64 %mul25.i.i.i13.i
  %conv27.i.i.i15.i = bitcast i32* %add.ptr26.i.i.i14.i to i8*
  %263 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %262, i16 255, i8* %conv27.i.i.i15.i, i32 0, i32 0) nounwind
  %264 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %263, i16 255, i8* %conv27.i.i.i15.i, i32 0, i32 0) nounwind
  %tmp3.i16.i = shufflevector <16 x float> %264, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %265 = bitcast i8* %"&pSB[currWI].offset438.i" to i32*
  %266 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i17.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i18.i = getelementptr inbounds i32* %265, i64 %mul25.i.i.i17.i
  %conv27.i.i.i19.i = bitcast i32* %add.ptr26.i.i.i18.i to i8*
  %267 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %266, i16 255, i8* %conv27.i.i.i19.i, i32 0, i32 0) nounwind
  %268 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %267, i16 255, i8* %conv27.i.i.i19.i, i32 0, i32 0) nounwind
  %tmp3.i20.i = shufflevector <16 x float> %268, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %269 = bitcast i8* %"&pSB[currWI].offset446.i" to i32*
  %270 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i21.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i22.i = getelementptr inbounds i32* %269, i64 %mul25.i.i.i21.i
  %conv27.i.i.i23.i = bitcast i32* %add.ptr26.i.i.i22.i to i8*
  %271 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %270, i16 255, i8* %conv27.i.i.i23.i, i32 0, i32 0) nounwind
  %272 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %271, i16 255, i8* %conv27.i.i.i23.i, i32 0, i32 0) nounwind
  %tmp3.i24.i = shufflevector <16 x float> %272, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %273 = bitcast i8* %"&pSB[currWI].offset454.i" to i32*
  %274 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i25.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i26.i = getelementptr inbounds i32* %273, i64 %mul25.i.i.i25.i
  %conv27.i.i.i27.i = bitcast i32* %add.ptr26.i.i.i26.i to i8*
  %275 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %274, i16 255, i8* %conv27.i.i.i27.i, i32 0, i32 0) nounwind
  %276 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %275, i16 255, i8* %conv27.i.i.i27.i, i32 0, i32 0) nounwind
  %tmp3.i28.i = shufflevector <16 x float> %276, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %277 = bitcast i8* %"&pSB[currWI].offset462.i" to i32*
  %278 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i29.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i30.i = getelementptr inbounds i32* %277, i64 %mul25.i.i.i29.i
  %conv27.i.i.i31.i = bitcast i32* %add.ptr26.i.i.i30.i to i8*
  %279 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %278, i16 255, i8* %conv27.i.i.i31.i, i32 0, i32 0) nounwind
  %280 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %279, i16 255, i8* %conv27.i.i.i31.i, i32 0, i32 0) nounwind
  %tmp3.i32.i = shufflevector <16 x float> %280, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %281 = bitcast i8* %"&pSB[currWI].offset470.i" to i32*
  %282 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i33.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i34.i = getelementptr inbounds i32* %281, i64 %mul25.i.i.i33.i
  %conv27.i.i.i35.i = bitcast i32* %add.ptr26.i.i.i34.i to i8*
  %283 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %282, i16 255, i8* %conv27.i.i.i35.i, i32 0, i32 0) nounwind
  %284 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %283, i16 255, i8* %conv27.i.i.i35.i, i32 0, i32 0) nounwind
  %tmp3.i36.i = shufflevector <16 x float> %284, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %285 = bitcast i8* %"&pSB[currWI].offset478.i" to i32*
  %286 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i37.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i38.i = getelementptr inbounds i32* %285, i64 %mul25.i.i.i37.i
  %conv27.i.i.i39.i = bitcast i32* %add.ptr26.i.i.i38.i to i8*
  %287 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %286, i16 255, i8* %conv27.i.i.i39.i, i32 0, i32 0) nounwind
  %288 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %287, i16 255, i8* %conv27.i.i.i39.i, i32 0, i32 0) nounwind
  %tmp3.i40.i = shufflevector <16 x float> %288, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %289 = bitcast i8* %"&pSB[currWI].offset486.i" to i32*
  %290 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i41.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i42.i = getelementptr inbounds i32* %289, i64 %mul25.i.i.i41.i
  %conv27.i.i.i43.i = bitcast i32* %add.ptr26.i.i.i42.i to i8*
  %291 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %290, i16 255, i8* %conv27.i.i.i43.i, i32 0, i32 0) nounwind
  %292 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %291, i16 255, i8* %conv27.i.i.i43.i, i32 0, i32 0) nounwind
  %tmp3.i44.i = shufflevector <16 x float> %292, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %293 = bitcast i8* %"&pSB[currWI].offset494.i" to i32*
  %294 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i45.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i46.i = getelementptr inbounds i32* %293, i64 %mul25.i.i.i45.i
  %conv27.i.i.i47.i = bitcast i32* %add.ptr26.i.i.i46.i to i8*
  %295 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %294, i16 255, i8* %conv27.i.i.i47.i, i32 0, i32 0) nounwind
  %296 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %295, i16 255, i8* %conv27.i.i.i47.i, i32 0, i32 0) nounwind
  %tmp3.i48.i = shufflevector <16 x float> %296, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %297 = bitcast i8* %"&pSB[currWI].offset502.i" to i32*
  %298 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i49.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i50.i = getelementptr inbounds i32* %297, i64 %mul25.i.i.i49.i
  %conv27.i.i.i51.i = bitcast i32* %add.ptr26.i.i.i50.i to i8*
  %299 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %298, i16 255, i8* %conv27.i.i.i51.i, i32 0, i32 0) nounwind
  %300 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %299, i16 255, i8* %conv27.i.i.i51.i, i32 0, i32 0) nounwind
  %tmp3.i52.i = shufflevector <16 x float> %300, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %301 = bitcast i8* %"&pSB[currWI].offset510.i" to i32*
  %302 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i53.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i54.i = getelementptr inbounds i32* %301, i64 %mul25.i.i.i53.i
  %conv27.i.i.i55.i = bitcast i32* %add.ptr26.i.i.i54.i to i8*
  %303 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %302, i16 255, i8* %conv27.i.i.i55.i, i32 0, i32 0) nounwind
  %304 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %303, i16 255, i8* %conv27.i.i.i55.i, i32 0, i32 0) nounwind
  %tmp3.i56.i = shufflevector <16 x float> %304, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %305 = bitcast i8* %"&pSB[currWI].offset518.i" to i32*
  %306 = call <16 x float> @llvm.x86.mic.undef.ps() nounwind
  %mul25.i.i.i57.i = shl i64 %indvar45.i, 3
  %add.ptr26.i.i.i58.i = getelementptr inbounds i32* %305, i64 %mul25.i.i.i57.i
  %conv27.i.i.i59.i = bitcast i32* %add.ptr26.i.i.i58.i to i8*
  %307 = call <16 x float> @llvm.x86.mic.mask.loadunpackl.ps(<16 x float> %306, i16 255, i8* %conv27.i.i.i59.i, i32 0, i32 0) nounwind
  %308 = call <16 x float> @llvm.x86.mic.mask.loadunpackh.ps(<16 x float> %307, i16 255, i8* %conv27.i.i.i59.i, i32 0, i32 0) nounwind
  %tmp3.i60.i = shufflevector <16 x float> %308, <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x float> %tmp3.i.i, <8 x float>* %229, align 32
  store <8 x float> %tmp3.i4.i, <8 x float>* %230, align 32
  store <8 x float> %tmp3.i8.i, <8 x float>* %231, align 32
  store <8 x float> %tmp3.i12.i, <8 x float>* %232, align 32
  store <8 x float> %tmp3.i16.i, <8 x float>* %233, align 32
  store <8 x float> %tmp3.i20.i, <8 x float>* %234, align 32
  store <8 x float> %tmp3.i24.i, <8 x float>* %235, align 32
  store <8 x float> %tmp3.i28.i, <8 x float>* %236, align 32
  store <8 x float> %tmp3.i32.i, <8 x float>* %237, align 32
  store <8 x float> %tmp3.i36.i, <8 x float>* %238, align 32
  store <8 x float> %tmp3.i40.i, <8 x float>* %239, align 32
  store <8 x float> %tmp3.i44.i, <8 x float>* %240, align 32
  store <8 x float> %tmp3.i48.i, <8 x float>* %241, align 32
  store <8 x float> %tmp3.i52.i, <8 x float>* %242, align 32
  store <8 x float> %tmp3.i56.i, <8 x float>* %243, align 32
  store <8 x float> %tmp3.i60.i, <8 x float>* %244, align 32
  %indvar.next46.i = add i64 %indvar45.i, 1
  %exitcond47.i = icmp eq i64 %indvar.next46.i, 8
  br i1 %exitcond47.i, label %bb.nph5.i, label %228

bb.nph5.i:                                        ; preds = %228
  %tmp35.i = trunc i64 %33 to i32
  %tmp36.i = mul i32 %tmp35.i, %10
  %temp213.i = insertelement <16 x i32> undef, i32 %tmp36.i, i32 0
  %vector214.i = shufflevector <16 x i32> %temp213.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp37212.i = trunc <16 x i64> %28 to <16 x i32>
  %tmp38215.i = add <16 x i32> %vector214.i, %tmp37212.i
  %tmp39216.i = shl <16 x i32> %tmp38215.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp40217.i = zext <16 x i32> %tmp39216.i to <16 x i64>
  %"&(pSB[currWI].offset)525.i" = add nuw i64 %CurrSBIndex..0.i, 8960
  %"&pSB[currWI].offset526.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)525.i"
  %CastToValueType527.i = bitcast i8* %"&pSB[currWI].offset526.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)533.i" = add nuw i64 %CurrSBIndex..0.i, 9216
  %"&pSB[currWI].offset534.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)533.i"
  %CastToValueType535.i = bitcast i8* %"&pSB[currWI].offset534.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)541.i" = add nuw i64 %CurrSBIndex..0.i, 9472
  %"&pSB[currWI].offset542.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)541.i"
  %CastToValueType543.i = bitcast i8* %"&pSB[currWI].offset542.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)549.i" = add nuw i64 %CurrSBIndex..0.i, 9728
  %"&pSB[currWI].offset550.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)549.i"
  %CastToValueType551.i = bitcast i8* %"&pSB[currWI].offset550.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)557.i" = add nuw i64 %CurrSBIndex..0.i, 9984
  %"&pSB[currWI].offset558.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)557.i"
  %CastToValueType559.i = bitcast i8* %"&pSB[currWI].offset558.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)565.i" = add nuw i64 %CurrSBIndex..0.i, 10240
  %"&pSB[currWI].offset566.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)565.i"
  %CastToValueType567.i = bitcast i8* %"&pSB[currWI].offset566.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)573.i" = add nuw i64 %CurrSBIndex..0.i, 10496
  %"&pSB[currWI].offset574.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)573.i"
  %CastToValueType575.i = bitcast i8* %"&pSB[currWI].offset574.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)581.i" = add nuw i64 %CurrSBIndex..0.i, 10752
  %"&pSB[currWI].offset582.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)581.i"
  %CastToValueType583.i = bitcast i8* %"&pSB[currWI].offset582.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)589.i" = add nuw i64 %CurrSBIndex..0.i, 11008
  %"&pSB[currWI].offset590.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)589.i"
  %CastToValueType591.i = bitcast i8* %"&pSB[currWI].offset590.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)597.i" = add nuw i64 %CurrSBIndex..0.i, 11264
  %"&pSB[currWI].offset598.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)597.i"
  %CastToValueType599.i = bitcast i8* %"&pSB[currWI].offset598.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)605.i" = add nuw i64 %CurrSBIndex..0.i, 11520
  %"&pSB[currWI].offset606.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)605.i"
  %CastToValueType607.i = bitcast i8* %"&pSB[currWI].offset606.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)613.i" = add nuw i64 %CurrSBIndex..0.i, 11776
  %"&pSB[currWI].offset614.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)613.i"
  %CastToValueType615.i = bitcast i8* %"&pSB[currWI].offset614.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)621.i" = add nuw i64 %CurrSBIndex..0.i, 12032
  %"&pSB[currWI].offset622.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)621.i"
  %CastToValueType623.i = bitcast i8* %"&pSB[currWI].offset622.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)629.i" = add nuw i64 %CurrSBIndex..0.i, 12288
  %"&pSB[currWI].offset630.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)629.i"
  %CastToValueType631.i = bitcast i8* %"&pSB[currWI].offset630.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)637.i" = add nuw i64 %CurrSBIndex..0.i, 12544
  %"&pSB[currWI].offset638.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)637.i"
  %CastToValueType639.i = bitcast i8* %"&pSB[currWI].offset638.i" to [8 x <8 x float>]*
  %"&(pSB[currWI].offset)645.i" = add nuw i64 %CurrSBIndex..0.i, 12800
  %"&pSB[currWI].offset646.i" = getelementptr inbounds i8* %22, i64 %"&(pSB[currWI].offset)645.i"
  %CastToValueType647.i = bitcast i8* %"&pSB[currWI].offset646.i" to [8 x <8 x float>]*
  br label %bb.nph.i

bb.nph.i:                                         ; preds = %._crit_edge.i, %bb.nph5.i
  %indvar19.i = phi i64 [ 0, %bb.nph5.i ], [ %indvar.next20.i, %._crit_edge.i ]
  %tmp34.i = mul i64 %tmp33.i, %indvar19.i
  %temp218.i = insertelement <16 x i64> undef, i64 %tmp34.i, i32 0
  %vector219.i = shufflevector <16 x i64> %temp218.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp41220.i = add <16 x i64> %tmp40217.i, %vector219.i
  %scevgep44.i = getelementptr <8 x float> addrspace(1)* %7, i64 %indvar19.i
  br label %309

; <label>:309                                     ; preds = %309, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %309 ]
  %temp221.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector222.i = shufflevector <16 x i64> %temp221.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp42223.i = add <16 x i64> %tmp41220.i, %vector222.i
  %310 = getelementptr [8 x <8 x float>]* %CastToValueType527.i, i64 0, i64 %indvar.i
  %311 = getelementptr [8 x <8 x float>]* %CastToValueType535.i, i64 0, i64 %indvar.i
  %312 = getelementptr [8 x <8 x float>]* %CastToValueType543.i, i64 0, i64 %indvar.i
  %313 = getelementptr [8 x <8 x float>]* %CastToValueType551.i, i64 0, i64 %indvar.i
  %314 = getelementptr [8 x <8 x float>]* %CastToValueType559.i, i64 0, i64 %indvar.i
  %315 = getelementptr [8 x <8 x float>]* %CastToValueType567.i, i64 0, i64 %indvar.i
  %316 = getelementptr [8 x <8 x float>]* %CastToValueType575.i, i64 0, i64 %indvar.i
  %317 = getelementptr [8 x <8 x float>]* %CastToValueType583.i, i64 0, i64 %indvar.i
  %318 = getelementptr [8 x <8 x float>]* %CastToValueType591.i, i64 0, i64 %indvar.i
  %319 = getelementptr [8 x <8 x float>]* %CastToValueType599.i, i64 0, i64 %indvar.i
  %320 = getelementptr [8 x <8 x float>]* %CastToValueType607.i, i64 0, i64 %indvar.i
  %321 = getelementptr [8 x <8 x float>]* %CastToValueType615.i, i64 0, i64 %indvar.i
  %322 = getelementptr [8 x <8 x float>]* %CastToValueType623.i, i64 0, i64 %indvar.i
  %323 = getelementptr [8 x <8 x float>]* %CastToValueType631.i, i64 0, i64 %indvar.i
  %324 = getelementptr [8 x <8 x float>]* %CastToValueType639.i, i64 0, i64 %indvar.i
  %325 = getelementptr [8 x <8 x float>]* %CastToValueType647.i, i64 0, i64 %indvar.i
  %326 = load <8 x float> addrspace(1)* %scevgep44.i, align 32
  %scalar16.i = extractelement <8 x float> %326, i32 0
  %temp224.i = insertelement <16 x float> undef, float %scalar16.i, i32 0
  %vector225.i = shufflevector <16 x float> %temp224.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar17.i = extractelement <8 x float> %326, i32 1
  %temp242.i = insertelement <16 x float> undef, float %scalar17.i, i32 0
  %vector243.i = shufflevector <16 x float> %temp242.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar18.i = extractelement <8 x float> %326, i32 2
  %temp260.i = insertelement <16 x float> undef, float %scalar18.i, i32 0
  %vector261.i = shufflevector <16 x float> %temp260.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar19.i = extractelement <8 x float> %326, i32 3
  %temp278.i = insertelement <16 x float> undef, float %scalar19.i, i32 0
  %vector279.i = shufflevector <16 x float> %temp278.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar20.i = extractelement <8 x float> %326, i32 4
  %temp296.i = insertelement <16 x float> undef, float %scalar20.i, i32 0
  %vector297.i = shufflevector <16 x float> %temp296.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar21.i = extractelement <8 x float> %326, i32 5
  %temp314.i = insertelement <16 x float> undef, float %scalar21.i, i32 0
  %vector315.i = shufflevector <16 x float> %temp314.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar22.i = extractelement <8 x float> %326, i32 6
  %temp332.i = insertelement <16 x float> undef, float %scalar22.i, i32 0
  %vector333.i = shufflevector <16 x float> %temp332.i, <16 x float> undef, <16 x i32> zeroinitializer
  %scalar23.i = extractelement <8 x float> %326, i32 7
  %temp350.i = insertelement <16 x float> undef, float %scalar23.i, i32 0
  %vector351.i = shufflevector <16 x float> %temp350.i, <16 x float> undef, <16 x i32> zeroinitializer
  %327 = load <8 x float>* %310, align 32
  %328 = load <8 x float>* %311, align 32
  %329 = load <8 x float>* %312, align 32
  %330 = load <8 x float>* %313, align 32
  %331 = load <8 x float>* %314, align 32
  %332 = load <8 x float>* %315, align 32
  %333 = load <8 x float>* %316, align 32
  %334 = load <8 x float>* %317, align 32
  %335 = load <8 x float>* %318, align 32
  %336 = load <8 x float>* %319, align 32
  %337 = load <8 x float>* %320, align 32
  %338 = load <8 x float>* %321, align 32
  %339 = load <8 x float>* %322, align 32
  %340 = load <8 x float>* %323, align 32
  %341 = load <8 x float>* %324, align 32
  %342 = load <8 x float>* %325, align 32
  %343 = extractelement <8 x float> %327, i32 0
  %344 = extractelement <8 x float> %328, i32 0
  %345 = extractelement <8 x float> %329, i32 0
  %346 = extractelement <8 x float> %330, i32 0
  %347 = extractelement <8 x float> %331, i32 0
  %348 = extractelement <8 x float> %332, i32 0
  %349 = extractelement <8 x float> %333, i32 0
  %350 = extractelement <8 x float> %334, i32 0
  %351 = extractelement <8 x float> %335, i32 0
  %352 = extractelement <8 x float> %336, i32 0
  %353 = extractelement <8 x float> %337, i32 0
  %354 = extractelement <8 x float> %338, i32 0
  %355 = extractelement <8 x float> %339, i32 0
  %356 = extractelement <8 x float> %340, i32 0
  %357 = extractelement <8 x float> %341, i32 0
  %358 = extractelement <8 x float> %342, i32 0
  %temp.vect226.i = insertelement <16 x float> undef, float %343, i32 0
  %temp.vect227.i = insertelement <16 x float> %temp.vect226.i, float %344, i32 1
  %temp.vect228.i = insertelement <16 x float> %temp.vect227.i, float %345, i32 2
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %346, i32 3
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %347, i32 4
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %348, i32 5
  %temp.vect232.i = insertelement <16 x float> %temp.vect231.i, float %349, i32 6
  %temp.vect233.i = insertelement <16 x float> %temp.vect232.i, float %350, i32 7
  %temp.vect234.i = insertelement <16 x float> %temp.vect233.i, float %351, i32 8
  %temp.vect235.i = insertelement <16 x float> %temp.vect234.i, float %352, i32 9
  %temp.vect236.i = insertelement <16 x float> %temp.vect235.i, float %353, i32 10
  %temp.vect237.i = insertelement <16 x float> %temp.vect236.i, float %354, i32 11
  %temp.vect238.i = insertelement <16 x float> %temp.vect237.i, float %355, i32 12
  %temp.vect239.i = insertelement <16 x float> %temp.vect238.i, float %356, i32 13
  %temp.vect240.i = insertelement <16 x float> %temp.vect239.i, float %357, i32 14
  %temp.vect241.i = insertelement <16 x float> %temp.vect240.i, float %358, i32 15
  %359 = extractelement <8 x float> %327, i32 1
  %360 = extractelement <8 x float> %328, i32 1
  %361 = extractelement <8 x float> %329, i32 1
  %362 = extractelement <8 x float> %330, i32 1
  %363 = extractelement <8 x float> %331, i32 1
  %364 = extractelement <8 x float> %332, i32 1
  %365 = extractelement <8 x float> %333, i32 1
  %366 = extractelement <8 x float> %334, i32 1
  %367 = extractelement <8 x float> %335, i32 1
  %368 = extractelement <8 x float> %336, i32 1
  %369 = extractelement <8 x float> %337, i32 1
  %370 = extractelement <8 x float> %338, i32 1
  %371 = extractelement <8 x float> %339, i32 1
  %372 = extractelement <8 x float> %340, i32 1
  %373 = extractelement <8 x float> %341, i32 1
  %374 = extractelement <8 x float> %342, i32 1
  %temp.vect244.i = insertelement <16 x float> undef, float %359, i32 0
  %temp.vect245.i = insertelement <16 x float> %temp.vect244.i, float %360, i32 1
  %temp.vect246.i = insertelement <16 x float> %temp.vect245.i, float %361, i32 2
  %temp.vect247.i = insertelement <16 x float> %temp.vect246.i, float %362, i32 3
  %temp.vect248.i = insertelement <16 x float> %temp.vect247.i, float %363, i32 4
  %temp.vect249.i = insertelement <16 x float> %temp.vect248.i, float %364, i32 5
  %temp.vect250.i = insertelement <16 x float> %temp.vect249.i, float %365, i32 6
  %temp.vect251.i = insertelement <16 x float> %temp.vect250.i, float %366, i32 7
  %temp.vect252.i = insertelement <16 x float> %temp.vect251.i, float %367, i32 8
  %temp.vect253.i = insertelement <16 x float> %temp.vect252.i, float %368, i32 9
  %temp.vect254.i = insertelement <16 x float> %temp.vect253.i, float %369, i32 10
  %temp.vect255.i = insertelement <16 x float> %temp.vect254.i, float %370, i32 11
  %temp.vect256.i = insertelement <16 x float> %temp.vect255.i, float %371, i32 12
  %temp.vect257.i = insertelement <16 x float> %temp.vect256.i, float %372, i32 13
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %373, i32 14
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %374, i32 15
  %375 = extractelement <8 x float> %327, i32 2
  %376 = extractelement <8 x float> %328, i32 2
  %377 = extractelement <8 x float> %329, i32 2
  %378 = extractelement <8 x float> %330, i32 2
  %379 = extractelement <8 x float> %331, i32 2
  %380 = extractelement <8 x float> %332, i32 2
  %381 = extractelement <8 x float> %333, i32 2
  %382 = extractelement <8 x float> %334, i32 2
  %383 = extractelement <8 x float> %335, i32 2
  %384 = extractelement <8 x float> %336, i32 2
  %385 = extractelement <8 x float> %337, i32 2
  %386 = extractelement <8 x float> %338, i32 2
  %387 = extractelement <8 x float> %339, i32 2
  %388 = extractelement <8 x float> %340, i32 2
  %389 = extractelement <8 x float> %341, i32 2
  %390 = extractelement <8 x float> %342, i32 2
  %temp.vect262.i = insertelement <16 x float> undef, float %375, i32 0
  %temp.vect263.i = insertelement <16 x float> %temp.vect262.i, float %376, i32 1
  %temp.vect264.i = insertelement <16 x float> %temp.vect263.i, float %377, i32 2
  %temp.vect265.i = insertelement <16 x float> %temp.vect264.i, float %378, i32 3
  %temp.vect266.i = insertelement <16 x float> %temp.vect265.i, float %379, i32 4
  %temp.vect267.i = insertelement <16 x float> %temp.vect266.i, float %380, i32 5
  %temp.vect268.i = insertelement <16 x float> %temp.vect267.i, float %381, i32 6
  %temp.vect269.i = insertelement <16 x float> %temp.vect268.i, float %382, i32 7
  %temp.vect270.i = insertelement <16 x float> %temp.vect269.i, float %383, i32 8
  %temp.vect271.i = insertelement <16 x float> %temp.vect270.i, float %384, i32 9
  %temp.vect272.i = insertelement <16 x float> %temp.vect271.i, float %385, i32 10
  %temp.vect273.i = insertelement <16 x float> %temp.vect272.i, float %386, i32 11
  %temp.vect274.i = insertelement <16 x float> %temp.vect273.i, float %387, i32 12
  %temp.vect275.i = insertelement <16 x float> %temp.vect274.i, float %388, i32 13
  %temp.vect276.i = insertelement <16 x float> %temp.vect275.i, float %389, i32 14
  %temp.vect277.i = insertelement <16 x float> %temp.vect276.i, float %390, i32 15
  %391 = extractelement <8 x float> %327, i32 3
  %392 = extractelement <8 x float> %328, i32 3
  %393 = extractelement <8 x float> %329, i32 3
  %394 = extractelement <8 x float> %330, i32 3
  %395 = extractelement <8 x float> %331, i32 3
  %396 = extractelement <8 x float> %332, i32 3
  %397 = extractelement <8 x float> %333, i32 3
  %398 = extractelement <8 x float> %334, i32 3
  %399 = extractelement <8 x float> %335, i32 3
  %400 = extractelement <8 x float> %336, i32 3
  %401 = extractelement <8 x float> %337, i32 3
  %402 = extractelement <8 x float> %338, i32 3
  %403 = extractelement <8 x float> %339, i32 3
  %404 = extractelement <8 x float> %340, i32 3
  %405 = extractelement <8 x float> %341, i32 3
  %406 = extractelement <8 x float> %342, i32 3
  %temp.vect280.i = insertelement <16 x float> undef, float %391, i32 0
  %temp.vect281.i = insertelement <16 x float> %temp.vect280.i, float %392, i32 1
  %temp.vect282.i = insertelement <16 x float> %temp.vect281.i, float %393, i32 2
  %temp.vect283.i = insertelement <16 x float> %temp.vect282.i, float %394, i32 3
  %temp.vect284.i = insertelement <16 x float> %temp.vect283.i, float %395, i32 4
  %temp.vect285.i = insertelement <16 x float> %temp.vect284.i, float %396, i32 5
  %temp.vect286.i = insertelement <16 x float> %temp.vect285.i, float %397, i32 6
  %temp.vect287.i = insertelement <16 x float> %temp.vect286.i, float %398, i32 7
  %temp.vect288.i = insertelement <16 x float> %temp.vect287.i, float %399, i32 8
  %temp.vect289.i = insertelement <16 x float> %temp.vect288.i, float %400, i32 9
  %temp.vect290.i = insertelement <16 x float> %temp.vect289.i, float %401, i32 10
  %temp.vect291.i = insertelement <16 x float> %temp.vect290.i, float %402, i32 11
  %temp.vect292.i = insertelement <16 x float> %temp.vect291.i, float %403, i32 12
  %temp.vect293.i = insertelement <16 x float> %temp.vect292.i, float %404, i32 13
  %temp.vect294.i = insertelement <16 x float> %temp.vect293.i, float %405, i32 14
  %temp.vect295.i = insertelement <16 x float> %temp.vect294.i, float %406, i32 15
  %407 = extractelement <8 x float> %327, i32 4
  %408 = extractelement <8 x float> %328, i32 4
  %409 = extractelement <8 x float> %329, i32 4
  %410 = extractelement <8 x float> %330, i32 4
  %411 = extractelement <8 x float> %331, i32 4
  %412 = extractelement <8 x float> %332, i32 4
  %413 = extractelement <8 x float> %333, i32 4
  %414 = extractelement <8 x float> %334, i32 4
  %415 = extractelement <8 x float> %335, i32 4
  %416 = extractelement <8 x float> %336, i32 4
  %417 = extractelement <8 x float> %337, i32 4
  %418 = extractelement <8 x float> %338, i32 4
  %419 = extractelement <8 x float> %339, i32 4
  %420 = extractelement <8 x float> %340, i32 4
  %421 = extractelement <8 x float> %341, i32 4
  %422 = extractelement <8 x float> %342, i32 4
  %temp.vect298.i = insertelement <16 x float> undef, float %407, i32 0
  %temp.vect299.i = insertelement <16 x float> %temp.vect298.i, float %408, i32 1
  %temp.vect300.i = insertelement <16 x float> %temp.vect299.i, float %409, i32 2
  %temp.vect301.i = insertelement <16 x float> %temp.vect300.i, float %410, i32 3
  %temp.vect302.i = insertelement <16 x float> %temp.vect301.i, float %411, i32 4
  %temp.vect303.i = insertelement <16 x float> %temp.vect302.i, float %412, i32 5
  %temp.vect304.i = insertelement <16 x float> %temp.vect303.i, float %413, i32 6
  %temp.vect305.i = insertelement <16 x float> %temp.vect304.i, float %414, i32 7
  %temp.vect306.i = insertelement <16 x float> %temp.vect305.i, float %415, i32 8
  %temp.vect307.i = insertelement <16 x float> %temp.vect306.i, float %416, i32 9
  %temp.vect308.i = insertelement <16 x float> %temp.vect307.i, float %417, i32 10
  %temp.vect309.i = insertelement <16 x float> %temp.vect308.i, float %418, i32 11
  %temp.vect310.i = insertelement <16 x float> %temp.vect309.i, float %419, i32 12
  %temp.vect311.i = insertelement <16 x float> %temp.vect310.i, float %420, i32 13
  %temp.vect312.i = insertelement <16 x float> %temp.vect311.i, float %421, i32 14
  %temp.vect313.i = insertelement <16 x float> %temp.vect312.i, float %422, i32 15
  %423 = extractelement <8 x float> %327, i32 5
  %424 = extractelement <8 x float> %328, i32 5
  %425 = extractelement <8 x float> %329, i32 5
  %426 = extractelement <8 x float> %330, i32 5
  %427 = extractelement <8 x float> %331, i32 5
  %428 = extractelement <8 x float> %332, i32 5
  %429 = extractelement <8 x float> %333, i32 5
  %430 = extractelement <8 x float> %334, i32 5
  %431 = extractelement <8 x float> %335, i32 5
  %432 = extractelement <8 x float> %336, i32 5
  %433 = extractelement <8 x float> %337, i32 5
  %434 = extractelement <8 x float> %338, i32 5
  %435 = extractelement <8 x float> %339, i32 5
  %436 = extractelement <8 x float> %340, i32 5
  %437 = extractelement <8 x float> %341, i32 5
  %438 = extractelement <8 x float> %342, i32 5
  %temp.vect316.i = insertelement <16 x float> undef, float %423, i32 0
  %temp.vect317.i = insertelement <16 x float> %temp.vect316.i, float %424, i32 1
  %temp.vect318.i = insertelement <16 x float> %temp.vect317.i, float %425, i32 2
  %temp.vect319.i = insertelement <16 x float> %temp.vect318.i, float %426, i32 3
  %temp.vect320.i = insertelement <16 x float> %temp.vect319.i, float %427, i32 4
  %temp.vect321.i = insertelement <16 x float> %temp.vect320.i, float %428, i32 5
  %temp.vect322.i = insertelement <16 x float> %temp.vect321.i, float %429, i32 6
  %temp.vect323.i = insertelement <16 x float> %temp.vect322.i, float %430, i32 7
  %temp.vect324.i = insertelement <16 x float> %temp.vect323.i, float %431, i32 8
  %temp.vect325.i = insertelement <16 x float> %temp.vect324.i, float %432, i32 9
  %temp.vect326.i = insertelement <16 x float> %temp.vect325.i, float %433, i32 10
  %temp.vect327.i = insertelement <16 x float> %temp.vect326.i, float %434, i32 11
  %temp.vect328.i = insertelement <16 x float> %temp.vect327.i, float %435, i32 12
  %temp.vect329.i = insertelement <16 x float> %temp.vect328.i, float %436, i32 13
  %temp.vect330.i = insertelement <16 x float> %temp.vect329.i, float %437, i32 14
  %temp.vect331.i = insertelement <16 x float> %temp.vect330.i, float %438, i32 15
  %439 = extractelement <8 x float> %327, i32 6
  %440 = extractelement <8 x float> %328, i32 6
  %441 = extractelement <8 x float> %329, i32 6
  %442 = extractelement <8 x float> %330, i32 6
  %443 = extractelement <8 x float> %331, i32 6
  %444 = extractelement <8 x float> %332, i32 6
  %445 = extractelement <8 x float> %333, i32 6
  %446 = extractelement <8 x float> %334, i32 6
  %447 = extractelement <8 x float> %335, i32 6
  %448 = extractelement <8 x float> %336, i32 6
  %449 = extractelement <8 x float> %337, i32 6
  %450 = extractelement <8 x float> %338, i32 6
  %451 = extractelement <8 x float> %339, i32 6
  %452 = extractelement <8 x float> %340, i32 6
  %453 = extractelement <8 x float> %341, i32 6
  %454 = extractelement <8 x float> %342, i32 6
  %temp.vect334.i = insertelement <16 x float> undef, float %439, i32 0
  %temp.vect335.i = insertelement <16 x float> %temp.vect334.i, float %440, i32 1
  %temp.vect336.i = insertelement <16 x float> %temp.vect335.i, float %441, i32 2
  %temp.vect337.i = insertelement <16 x float> %temp.vect336.i, float %442, i32 3
  %temp.vect338.i = insertelement <16 x float> %temp.vect337.i, float %443, i32 4
  %temp.vect339.i = insertelement <16 x float> %temp.vect338.i, float %444, i32 5
  %temp.vect340.i = insertelement <16 x float> %temp.vect339.i, float %445, i32 6
  %temp.vect341.i = insertelement <16 x float> %temp.vect340.i, float %446, i32 7
  %temp.vect342.i = insertelement <16 x float> %temp.vect341.i, float %447, i32 8
  %temp.vect343.i = insertelement <16 x float> %temp.vect342.i, float %448, i32 9
  %temp.vect344.i = insertelement <16 x float> %temp.vect343.i, float %449, i32 10
  %temp.vect345.i = insertelement <16 x float> %temp.vect344.i, float %450, i32 11
  %temp.vect346.i = insertelement <16 x float> %temp.vect345.i, float %451, i32 12
  %temp.vect347.i = insertelement <16 x float> %temp.vect346.i, float %452, i32 13
  %temp.vect348.i = insertelement <16 x float> %temp.vect347.i, float %453, i32 14
  %temp.vect349.i = insertelement <16 x float> %temp.vect348.i, float %454, i32 15
  %455 = extractelement <8 x float> %327, i32 7
  %456 = extractelement <8 x float> %328, i32 7
  %457 = extractelement <8 x float> %329, i32 7
  %458 = extractelement <8 x float> %330, i32 7
  %459 = extractelement <8 x float> %331, i32 7
  %460 = extractelement <8 x float> %332, i32 7
  %461 = extractelement <8 x float> %333, i32 7
  %462 = extractelement <8 x float> %334, i32 7
  %463 = extractelement <8 x float> %335, i32 7
  %464 = extractelement <8 x float> %336, i32 7
  %465 = extractelement <8 x float> %337, i32 7
  %466 = extractelement <8 x float> %338, i32 7
  %467 = extractelement <8 x float> %339, i32 7
  %468 = extractelement <8 x float> %340, i32 7
  %469 = extractelement <8 x float> %341, i32 7
  %470 = extractelement <8 x float> %342, i32 7
  %temp.vect352.i = insertelement <16 x float> undef, float %455, i32 0
  %temp.vect353.i = insertelement <16 x float> %temp.vect352.i, float %456, i32 1
  %temp.vect354.i = insertelement <16 x float> %temp.vect353.i, float %457, i32 2
  %temp.vect355.i = insertelement <16 x float> %temp.vect354.i, float %458, i32 3
  %temp.vect356.i = insertelement <16 x float> %temp.vect355.i, float %459, i32 4
  %temp.vect357.i = insertelement <16 x float> %temp.vect356.i, float %460, i32 5
  %temp.vect358.i = insertelement <16 x float> %temp.vect357.i, float %461, i32 6
  %temp.vect359.i = insertelement <16 x float> %temp.vect358.i, float %462, i32 7
  %temp.vect360.i = insertelement <16 x float> %temp.vect359.i, float %463, i32 8
  %temp.vect361.i = insertelement <16 x float> %temp.vect360.i, float %464, i32 9
  %temp.vect362.i = insertelement <16 x float> %temp.vect361.i, float %465, i32 10
  %temp.vect363.i = insertelement <16 x float> %temp.vect362.i, float %466, i32 11
  %temp.vect364.i = insertelement <16 x float> %temp.vect363.i, float %467, i32 12
  %temp.vect365.i = insertelement <16 x float> %temp.vect364.i, float %468, i32 13
  %temp.vect366.i = insertelement <16 x float> %temp.vect365.i, float %469, i32 14
  %temp.vect367.i = insertelement <16 x float> %temp.vect366.i, float %470, i32 15
  %471 = fmul <16 x float> %vector225.i, %temp.vect241.i
  %472 = fmul <16 x float> %vector243.i, %temp.vect259.i
  %473 = fmul <16 x float> %vector261.i, %temp.vect277.i
  %474 = fmul <16 x float> %vector279.i, %temp.vect295.i
  %475 = fmul <16 x float> %vector297.i, %temp.vect313.i
  %476 = fmul <16 x float> %vector315.i, %temp.vect331.i
  %477 = fmul <16 x float> %vector333.i, %temp.vect349.i
  %478 = fmul <16 x float> %vector351.i, %temp.vect367.i
  %479 = fadd <16 x float> %471, %472
  %480 = fadd <16 x float> %479, %473
  %481 = fadd <16 x float> %480, %474
  %482 = fadd <16 x float> %481, %475
  %483 = fadd <16 x float> %482, %476
  %484 = fadd <16 x float> %483, %477
  %485 = fadd <16 x float> %484, %478
  %extract384.i = extractelement <16 x float> %485, i32 0
  %extract385.i = extractelement <16 x float> %485, i32 1
  %extract386.i = extractelement <16 x float> %485, i32 2
  %extract387.i = extractelement <16 x float> %485, i32 3
  %extract388.i = extractelement <16 x float> %485, i32 4
  %extract389.i = extractelement <16 x float> %485, i32 5
  %extract390.i = extractelement <16 x float> %485, i32 6
  %extract391.i = extractelement <16 x float> %485, i32 7
  %extract392.i = extractelement <16 x float> %485, i32 8
  %extract393.i = extractelement <16 x float> %485, i32 9
  %extract394.i = extractelement <16 x float> %485, i32 10
  %extract395.i = extractelement <16 x float> %485, i32 11
  %extract396.i = extractelement <16 x float> %485, i32 12
  %extract397.i = extractelement <16 x float> %485, i32 13
  %extract398.i = extractelement <16 x float> %485, i32 14
  %extract399.i = extractelement <16 x float> %485, i32 15
  %486 = and <16 x i64> %tmp42223.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract368.i = extractelement <16 x i64> %486, i32 0
  %extract369.i = extractelement <16 x i64> %486, i32 1
  %extract370.i = extractelement <16 x i64> %486, i32 2
  %extract371.i = extractelement <16 x i64> %486, i32 3
  %extract372.i = extractelement <16 x i64> %486, i32 4
  %extract373.i = extractelement <16 x i64> %486, i32 5
  %extract374.i = extractelement <16 x i64> %486, i32 6
  %extract375.i = extractelement <16 x i64> %486, i32 7
  %extract376.i = extractelement <16 x i64> %486, i32 8
  %extract377.i = extractelement <16 x i64> %486, i32 9
  %extract378.i = extractelement <16 x i64> %486, i32 10
  %extract379.i = extractelement <16 x i64> %486, i32 11
  %extract380.i = extractelement <16 x i64> %486, i32 12
  %extract381.i = extractelement <16 x i64> %486, i32 13
  %extract382.i = extractelement <16 x i64> %486, i32 14
  %extract383.i = extractelement <16 x i64> %486, i32 15
  %487 = getelementptr inbounds float addrspace(1)* %1, i64 %extract368.i
  %488 = getelementptr inbounds float addrspace(1)* %1, i64 %extract369.i
  %489 = getelementptr inbounds float addrspace(1)* %1, i64 %extract370.i
  %490 = getelementptr inbounds float addrspace(1)* %1, i64 %extract371.i
  %491 = getelementptr inbounds float addrspace(1)* %1, i64 %extract372.i
  %492 = getelementptr inbounds float addrspace(1)* %1, i64 %extract373.i
  %493 = getelementptr inbounds float addrspace(1)* %1, i64 %extract374.i
  %494 = getelementptr inbounds float addrspace(1)* %1, i64 %extract375.i
  %495 = getelementptr inbounds float addrspace(1)* %1, i64 %extract376.i
  %496 = getelementptr inbounds float addrspace(1)* %1, i64 %extract377.i
  %497 = getelementptr inbounds float addrspace(1)* %1, i64 %extract378.i
  %498 = getelementptr inbounds float addrspace(1)* %1, i64 %extract379.i
  %499 = getelementptr inbounds float addrspace(1)* %1, i64 %extract380.i
  %500 = getelementptr inbounds float addrspace(1)* %1, i64 %extract381.i
  %501 = getelementptr inbounds float addrspace(1)* %1, i64 %extract382.i
  %502 = getelementptr inbounds float addrspace(1)* %1, i64 %extract383.i
  store float %extract384.i, float addrspace(1)* %487, align 4
  store float %extract385.i, float addrspace(1)* %488, align 4
  store float %extract386.i, float addrspace(1)* %489, align 4
  store float %extract387.i, float addrspace(1)* %490, align 4
  store float %extract388.i, float addrspace(1)* %491, align 4
  store float %extract389.i, float addrspace(1)* %492, align 4
  store float %extract390.i, float addrspace(1)* %493, align 4
  store float %extract391.i, float addrspace(1)* %494, align 4
  store float %extract392.i, float addrspace(1)* %495, align 4
  store float %extract393.i, float addrspace(1)* %496, align 4
  store float %extract394.i, float addrspace(1)* %497, align 4
  store float %extract395.i, float addrspace(1)* %498, align 4
  store float %extract396.i, float addrspace(1)* %499, align 4
  store float %extract397.i, float addrspace(1)* %500, align 4
  store float %extract398.i, float addrspace(1)* %501, align 4
  store float %extract399.i, float addrspace(1)* %502, align 4
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 8
  br i1 %exitcond.i, label %._crit_edge.i, label %309

._crit_edge.i:                                    ; preds = %309
  %indvar.next20.i = add i64 %indvar19.i, 1
  %exitcond32.i = icmp eq i64 %indvar.next20.i, 8
  br i1 %exitcond32.i, label %._crit_edge6.i, label %bb.nph.i

._crit_edge6.i:                                   ; preds = %._crit_edge.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.DCT_CPU_VECTOR_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge6.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 13056
  br label %SyncBB.i

____Vectorized_.DCT_CPU_VECTOR_separated_args.exit: ; preds = %._crit_edge6.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__DCT_CPU_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const", metadata !"opencl_DCT_CPU_locals_anchor", void (i8*)* @DCT_CPU}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__DCT_CPU_VECTOR_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, uint const", metadata !"opencl_DCT_CPU_VECTOR_locals_anchor", void (i8*)* @DCT_CPU_VECTOR}
