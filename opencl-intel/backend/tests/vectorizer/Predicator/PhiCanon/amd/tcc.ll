; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'tcc.cl'

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

; CHECK: @tcc_vector8_third_optimization
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

; CHECK: @tcc_scalar_unroll2
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

@uv_min_val_16 = addrspace(2) global i16 -512, align 2
@uv_max_val_16 = addrspace(2) global i16 511, align 2
@hi_key_mask_16 = addrspace(2) global i16 -32, align 2
@alpha_mask_SF_16 = addrspace(2) global i16 31, align 2
@rounding_SF_16 = addrspace(2) global i16 16, align 2
@uv_final_rounding_32 = addrspace(2) global i32 64, align 4
@uv_min_val_8_s16 = addrspace(2) global <8 x i16> <i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512>, align 16
@uv_max_val_8_s16 = addrspace(2) global <8 x i16> <i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511>, align 16
@hi_key_mask_8_s16 = addrspace(2) global <8 x i16> <i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32>, align 16
@alpha_mask_SF_8_s16 = addrspace(2) global <8 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>, align 16
@rounding_SF_8_s16 = addrspace(2) global <8 x i16> <i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16>, align 16
@precision_8_s16 = addrspace(2) global <8 x i16> <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>, align 16
@zero_const_8_s16 = addrspace(2) global <8 x i16> zeroinitializer, align 16
@u_key_16_mask_8_s16 = addrspace(2) global <8 x i16> <i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528>, align 16
@ff_const_8_u16 = addrspace(2) global <8 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 16
@zero_const_8_u16 = addrspace(2) global <8 x i16> zeroinitializer, align 16
@eight_const_8_u16 = addrspace(2) global <8 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 16
@zero_const_16_s16 = addrspace(2) global <16 x i16> zeroinitializer, align 32
@ff_const_16_u16 = addrspace(2) global <16 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 32
@zero_const_16_u16 = addrspace(2) global <16 x i16> zeroinitializer, align 32
@eight_const_16_u16 = addrspace(2) global <16 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 32
@sat_bit_minus_one_4_s32 = addrspace(2) global <4 x i32> <i32 7, i32 7, i32 7, i32 7>, align 16
@uv_final_rounding_4_s32 = addrspace(2) global <4 x i32> <i32 64, i32 64, i32 64, i32 64>, align 16
@zero_const_4_u32 = addrspace(2) global <4 x i32> zeroinitializer, align 16
@ff00ff_const_4_u32 = addrspace(2) global <4 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 16
@sat_bit_minus_one_8_s32 = addrspace(2) global <8 x i32> <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>, align 32
@uv_final_rounding_8_s32 = addrspace(2) global <8 x i32> <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>, align 32
@zero_const_8_u32 = addrspace(2) global <8 x i32> zeroinitializer, align 32
@ff00ff_const_8_u32 = addrspace(2) global <8 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 32

define void @tcc_vector8_third_optimization(<8 x i16> addrspace(1)* %Uin, <8 x i16> addrspace(1)* %Vin, <8 x i16> addrspace(1)* %Uout, <8 x i16> addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry, i32 addrspace(1)* nocapture %timeStamps, ...) nounwind {
entry:
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind
  %call1 = call i32 @get_global_size(i32 0) nounwind
  %cmp = icmp ne i32 %call1, 0
  %nonzero = select i1 %cmp, i32 %call1, i32 1
  %div = udiv i32 %bufferSize, %nonzero
  %cmp103 = icmp eq i32 %div, 0
  br i1 %cmp103, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul = mul i32 %div, %call
  %div6 = lshr i32 %mul, 3
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %storemerge5 = phi i32 [ 0, %for.body.lr.ph ], [ %add256, %for.body ]
  %inc14 = phi i32 [ %div6, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr <8 x i16> addrspace(1)* %Uin, i32 %inc14
  %tmp14 = load <8 x i16> addrspace(1)* %arrayidx, align 16
  %arrayidx18 = getelementptr <8 x i16> addrspace(1)* %Vin, i32 %inc14
  %tmp19 = load <8 x i16> addrspace(1)* %arrayidx18, align 16
  %tmp21 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16, align 16
  %add = add <8 x i16> %tmp14, %tmp21
  %add24 = add <8 x i16> %tmp19, %tmp21
  %shr = ashr <8 x i16> %add, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>
  %tmp29 = load <8 x i16> addrspace(2)* @hi_key_mask_8_s16, align 16
  %and = and <8 x i16> %add24, %tmp29
  %add32 = add <8 x i16> %shr, %and
  %tmp35 = load <8 x i16> addrspace(2)* @alpha_mask_SF_8_s16, align 16
  %and36 = and <8 x i16> %add24, %tmp35
  %tmp40 = load <8 x i16> addrspace(2)* @u_key_16_mask_8_s16, align 16
  %add41 = add <8 x i16> %add32, %tmp40
  %shl = shl <8 x i16> %add41, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %vecext = extractelement <8 x i16> %shl, i32 0
  %idxprom = sext i16 %vecext to i32
  %arrayidx45 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom
  %conv = addrspacecast i8 addrspace(2)* %arrayidx45 to i32*
  %tmp46 = load i32* %conv, align 4
  %vecins = insertelement <8 x i32> undef, i32 %tmp46, i32 0
  %vecext49 = extractelement <8 x i16> %shl, i32 1
  %idxprom51 = sext i16 %vecext49 to i32
  %arrayidx52 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom51
  %conv53 = addrspacecast i8 addrspace(2)* %arrayidx52 to i32*
  %tmp54 = load i32* %conv53, align 4
  %vecins56 = insertelement <8 x i32> %vecins, i32 %tmp54, i32 1
  %vecext58 = extractelement <8 x i16> %shl, i32 2
  %idxprom60 = sext i16 %vecext58 to i32
  %arrayidx61 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom60
  %conv62 = addrspacecast i8 addrspace(2)* %arrayidx61 to i32*
  %tmp63 = load i32* %conv62, align 4
  %vecins65 = insertelement <8 x i32> %vecins56, i32 %tmp63, i32 2
  %vecext67 = extractelement <8 x i16> %shl, i32 3
  %idxprom69 = sext i16 %vecext67 to i32
  %arrayidx70 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom69
  %conv71 = addrspacecast i8 addrspace(2)* %arrayidx70 to i32*
  %tmp72 = load i32* %conv71, align 4
  %vecins74 = insertelement <8 x i32> %vecins65, i32 %tmp72, i32 3
  %vecext76 = extractelement <8 x i16> %shl, i32 4
  %idxprom78 = sext i16 %vecext76 to i32
  %arrayidx79 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom78
  %conv80 = addrspacecast i8 addrspace(2)* %arrayidx79 to i32*
  %tmp81 = load i32* %conv80, align 4
  %vecins83 = insertelement <8 x i32> %vecins74, i32 %tmp81, i32 4
  %vecext85 = extractelement <8 x i16> %shl, i32 5
  %idxprom87 = sext i16 %vecext85 to i32
  %arrayidx88 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom87
  %conv89 = addrspacecast i8 addrspace(2)* %arrayidx88 to i32*
  %tmp90 = load i32* %conv89, align 4
  %vecins92 = insertelement <8 x i32> %vecins83, i32 %tmp90, i32 5
  %vecext94 = extractelement <8 x i16> %shl, i32 6
  %idxprom96 = sext i16 %vecext94 to i32
  %arrayidx97 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom96
  %conv98 = addrspacecast i8 addrspace(2)* %arrayidx97 to i32*
  %tmp99 = load i32* %conv98, align 4
  %vecins101 = insertelement <8 x i32> %vecins92, i32 %tmp99, i32 6
  %vecext103 = extractelement <8 x i16> %shl, i32 7
  %idxprom105 = sext i16 %vecext103 to i32
  %arrayidx106 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom105
  %conv107 = addrspacecast i8 addrspace(2)* %arrayidx106 to i32*
  %tmp108 = load i32* %conv107, align 4
  %vecins110 = insertelement <8 x i32> %vecins101, i32 %tmp108, i32 7
  %astype = bitcast <8 x i32> %vecins110 to <16 x i16>
  %shr115 = lshr <16 x i16> %astype, <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>
  %tmp117 = load <16 x i16> addrspace(2)* @ff_const_16_u16, align 32
  %and118 = and <16 x i16> %astype, %tmp117
  %sub = sub <16 x i16> %and118, %shr115
  %tmp128 = shufflevector <16 x i16> %sub, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
  %mul131 = mul <8 x i16> %tmp128, %and36
  %tmp133 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16, align 16
  %add134 = add <8 x i16> %mul131, %tmp133
  %shr136 = ashr <8 x i16> %add134, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>
  %add139 = add <8 x i16> %tmp128, %shr136
  %tmp141 = shufflevector <16 x i16> %sub, <16 x i16> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>
  %mul144 = mul <8 x i16> %tmp141, %and36
  %add147 = add <8 x i16> %mul144, %tmp133
  %shr149 = ashr <8 x i16> %add147, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>
  %add152 = add <8 x i16> %tmp141, %shr149
  %and155 = and <8 x i16> %add, %tmp35
  %sub158 = sub <8 x i16> %add152, %add139
  %mul161 = mul <8 x i16> %sub158, %and155
  %add164 = add <8 x i16> %mul161, %tmp133
  %shr166 = ashr <8 x i16> %add164, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>
  %add169 = add <8 x i16> %shr166, %add139
  %mul173 = mul <8 x i16> %add169, %add
  %tmp176 = shufflevector <8 x i16> %mul173, <8 x i16> undef, <16 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef>
  %call179 = call <8 x i16> @__mul_hi_8i16(<8 x i16> %add169, <8 x i16> %add) nounwind
  %tmp181 = shufflevector <8 x i16> %call179, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp182 = shufflevector <16 x i16> %tmp176, <16 x i16> %tmp181, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23>
  %astype185 = bitcast <16 x i16> %tmp182 to <8 x i32>
  %tmp187 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32, align 32
  %add188 = add <8 x i32> %astype185, %tmp187
  %shr190 = ashr <8 x i32> %add188, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %astype192 = bitcast <8 x i32> %shr190 to <16 x i16>
  %tmp195 = shufflevector <16 x i16> %astype192, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
  %tmp204 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16, align 16
  %call205 = call <8 x i16> @__max_8i16(<8 x i16> %tmp195, <8 x i16> %tmp204) nounwind
  %tmp206 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16, align 16
  %call207 = call <8 x i16> @__min_8i16(<8 x i16> %call205, <8 x i16> %tmp206) nounwind
  %tmp209 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %tmp210 = insertelement <8 x i16> undef, i16 %tmp209, i32 0
  %splat211 = shufflevector <8 x i16> %tmp210, <8 x i16> undef, <8 x i32> zeroinitializer
  %sub212 = sub <8 x i16> %call207, %splat211
  %arrayidx215 = getelementptr <8 x i16> addrspace(1)* %Uout, i32 %inc14
  store <8 x i16> %sub212, <8 x i16> addrspace(1)* %arrayidx215, align 16
  %mul219 = mul <8 x i16> %add169, %add24
  %tmp222 = shufflevector <8 x i16> %mul219, <8 x i16> undef, <16 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef>
  %call225 = call <8 x i16> @__mul_hi_8i16(<8 x i16> %add169, <8 x i16> %add24) nounwind
  %tmp227 = shufflevector <8 x i16> %call225, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp228 = shufflevector <16 x i16> %tmp222, <16 x i16> %tmp227, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23>
  %astype230 = bitcast <16 x i16> %tmp228 to <8 x i32>
  %tmp232 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32, align 32
  %add233 = add <8 x i32> %astype230, %tmp232
  %shr235 = ashr <8 x i32> %add233, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %astype237 = bitcast <8 x i32> %shr235 to <16 x i16>
  %tmp239 = shufflevector <16 x i16> %astype237, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>
  %tmp241 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16, align 16
  %call242 = call <8 x i16> @__max_8i16(<8 x i16> %tmp239, <8 x i16> %tmp241) nounwind
  %tmp243 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16, align 16
  %call244 = call <8 x i16> @__min_8i16(<8 x i16> %call242, <8 x i16> %tmp243) nounwind
  %tmp246 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %tmp247 = insertelement <8 x i16> undef, i16 %tmp246, i32 0
  %splat248 = shufflevector <8 x i16> %tmp247, <8 x i16> undef, <8 x i32> zeroinitializer
  %sub249 = sub <8 x i16> %call244, %splat248
  %arrayidx252 = getelementptr <8 x i16> addrspace(1)* %Vout, i32 %inc14
  store <8 x i16> %sub249, <8 x i16> addrspace(1)* %arrayidx252, align 16
  %inc = add i32 %inc14, 1
  %add256 = add i32 %storemerge5, 8
  %cmp10 = icmp ult i32 %add256, %div
  br i1 %cmp10, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare i32 @_Z13get_global_idj(i32)

declare i32 @get_global_size(i32)

declare <8 x i16> @__mul_hi_8i16(<8 x i16>, <8 x i16>)

declare <8 x i16> @__min_8i16(<8 x i16>, <8 x i16>)

declare <8 x i16> @__max_8i16(<8 x i16>, <8 x i16>)

define void @tcc_scalar_unroll2(i16 addrspace(1)* %Uin, i16 addrspace(1)* %Vin, i16 addrspace(1)* %Uout, i16 addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry, ...) nounwind {
entry:
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind
  %call1 = call i32 @get_global_size(i32 0) nounwind
  %cmp = icmp ne i32 %call1, 0
  %nonzero = select i1 %cmp, i32 %call1, i32 1
  %div = udiv i32 %bufferSize, %nonzero
  %cmp941 = icmp eq i32 %div, 0
  br i1 %cmp941, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul = mul i32 %div, %call
  %tmp27.pre = load i16 addrspace(2)* @uv_min_val_16, align 2
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %tmp27 = phi i16 [ %tmp27.pre, %for.body.lr.ph ], [ %tmp404, %for.body ]
  %storemerge43 = phi i32 [ 0, %for.body.lr.ph ], [ %inc439, %for.body ]
  %inc4373142 = phi i32 [ %mul, %for.body.lr.ph ], [ %inc437, %for.body ]
  %add.ptr = getelementptr i16 addrspace(1)* %Uout, i32 %inc4373142
  %conv = bitcast i16 addrspace(1)* %add.ptr to i32 addrspace(1)*
  %add.ptr18 = getelementptr i16 addrspace(1)* %Vout, i32 %inc4373142
  %conv19 = bitcast i16 addrspace(1)* %add.ptr18 to i32 addrspace(1)*
  %arrayidx = getelementptr i16 addrspace(1)* %Uin, i32 %inc4373142
  %tmp23 = load i16 addrspace(1)* %arrayidx, align 2
  %add = add i16 %tmp27, %tmp23
  %arrayidx33 = getelementptr i16 addrspace(1)* %Vin, i32 %inc4373142
  %tmp34 = load i16 addrspace(1)* %arrayidx33, align 2
  %add40 = add i16 %tmp34, %tmp27
  %conv44 = sext i16 %add to i32
  %shr5 = lshr i32 %conv44, 5
  %conv45 = trunc i32 %shr5 to i16
  %tmp49 = load i16 addrspace(2)* @hi_key_mask_16, align 2
  %and6 = and i16 %add40, %tmp49
  %tmp61 = load i16 addrspace(2)* @alpha_mask_SF_16, align 2
  %and639 = and i16 %tmp61, %add40
  %add56 = add i16 %and6, 528
  %add70 = add i16 %add56, %conv45
  %idxprom = sext i16 %add70 to i32
  %arrayidx75 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom
  %tmp76 = load i8 addrspace(2)* %arrayidx75, align 1
  %add81 = add i32 %idxprom, 1
  %arrayidx83 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add81
  %tmp84 = load i8 addrspace(2)* %arrayidx83, align 1
  %add89 = add i32 %idxprom, 2
  %arrayidx91 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add89
  %tmp92 = load i8 addrspace(2)* %arrayidx91, align 1
  %add97 = add i32 %idxprom, 3
  %arrayidx99 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add97
  %tmp100 = load i8 addrspace(2)* %arrayidx99, align 1
  %conv105 = zext i8 %tmp76 to i32
  %conv108 = zext i8 %tmp84 to i32
  %sub = sub i32 %conv105, %conv108
  %conv111 = sext i16 %and639 to i32
  %mul112 = mul i32 %sub, %conv111
  %tmp114 = load i16 addrspace(2)* @rounding_SF_16, align 2
  %conv115 = sext i16 %tmp114 to i32
  %add116 = add i32 %conv115, %mul112
  %shr118 = ashr i32 %add116, 5
  %add123 = add i32 %shr118, %conv105
  %conv126 = zext i8 %tmp92 to i32
  %conv129 = zext i8 %tmp100 to i32
  %sub130 = sub i32 %conv126, %conv129
  %mul134 = mul i32 %sub130, %conv111
  %add138 = add i32 %mul134, %conv115
  %shr140 = ashr i32 %add138, 5
  %and15011 = and i16 %tmp61, %add
  %add145 = sub i32 %conv126, %add123
  %sub154 = add i32 %add145, %shr140
  %conv157 = sext i16 %and15011 to i32
  %mul158 = mul i32 %sub154, %conv157
  %add162 = add i32 %mul158, %conv115
  %shr164 = ashr i32 %add162, 5
  %add167 = add i32 %shr164, %add123
  %mul172 = mul i32 %add167, %conv44
  %tmp174 = load i32 addrspace(2)* @uv_final_rounding_32, align 4
  %add175 = add i32 %mul172, %tmp174
  %shr17733 = lshr i32 %add175, 7
  %conv179 = trunc i32 %shr17733 to i16
  %call181 = call signext i16 @__max_1i16(i16 signext %conv179, i16 signext %tmp27) nounwind
  %tmp182 = load i16 addrspace(2)* @uv_max_val_16, align 2
  %call183 = call signext i16 @__min_1i16(i16 signext %call181, i16 signext %tmp182) nounwind
  %tmp186 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %sub188 = sub i16 %call183, %tmp186
  %conv192 = sext i16 %add40 to i32
  %mul193 = mul i32 %add167, %conv192
  %tmp195 = load i32 addrspace(2)* @uv_final_rounding_32, align 4
  %add196 = add i32 %mul193, %tmp195
  %shr19834 = lshr i32 %add196, 7
  %conv200 = trunc i32 %shr19834 to i16
  %call202 = call signext i16 @__max_1i16(i16 signext %conv200, i16 signext %tmp186) nounwind
  %tmp203 = load i16 addrspace(2)* @uv_max_val_16, align 2
  %call204 = call signext i16 @__min_1i16(i16 signext %call202, i16 signext %tmp203) nounwind
  %tmp207 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %sub209 = sub i16 %call204, %tmp207
  %conv21337 = zext i16 %sub188 to i32
  %conv21739 = zext i16 %sub209 to i32
  %inc = add i32 %inc4373142, 1
  %arrayidx221 = getelementptr i16 addrspace(1)* %Uin, i32 %inc
  %tmp222 = load i16 addrspace(1)* %arrayidx221, align 2
  %add228 = add i16 %tmp222, %tmp207
  %arrayidx232 = getelementptr i16 addrspace(1)* %Vin, i32 %inc
  %tmp233 = load i16 addrspace(1)* %arrayidx232, align 2
  %add239 = add i16 %tmp233, %tmp207
  %conv242 = sext i16 %add228 to i32
  %shr24320 = lshr i32 %conv242, 5
  %conv244 = trunc i32 %shr24320 to i16
  %tmp249 = load i16 addrspace(2)* @hi_key_mask_16, align 2
  %and25121 = and i16 %add239, %tmp249
  %tmp261 = load i16 addrspace(2)* @alpha_mask_SF_16, align 2
  %and26324 = and i16 %tmp261, %add239
  %add257 = add i16 %and25121, 528
  %add267 = add i16 %add257, %conv244
  %idxprom271 = sext i16 %add267 to i32
  %arrayidx272 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %idxprom271
  %tmp273 = load i8 addrspace(2)* %arrayidx272, align 1
  %add278 = add i32 %idxprom271, 1
  %arrayidx280 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add278
  %tmp281 = load i8 addrspace(2)* %arrayidx280, align 1
  %add286 = add i32 %idxprom271, 2
  %arrayidx288 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add286
  %tmp289 = load i8 addrspace(2)* %arrayidx288, align 1
  %add294 = add i32 %idxprom271, 3
  %arrayidx296 = getelementptr i8 addrspace(2)* %SatLUTEntry, i32 %add294
  %tmp297 = load i8 addrspace(2)* %arrayidx296, align 1
  %conv302 = zext i8 %tmp273 to i32
  %conv305 = zext i8 %tmp281 to i32
  %sub306 = sub i32 %conv302, %conv305
  %conv309 = sext i16 %and26324 to i32
  %mul310 = mul i32 %sub306, %conv309
  %tmp312 = load i16 addrspace(2)* @rounding_SF_16, align 2
  %conv313 = sext i16 %tmp312 to i32
  %add314 = add i32 %conv313, %mul310
  %shr316 = ashr i32 %add314, 5
  %add321 = add i32 %shr316, %conv302
  %conv324 = zext i8 %tmp289 to i32
  %conv327 = zext i8 %tmp297 to i32
  %sub328 = sub i32 %conv324, %conv327
  %mul332 = mul i32 %sub328, %conv309
  %add336 = add i32 %mul332, %conv313
  %shr338 = ashr i32 %add336, 5
  %and34826 = and i16 %tmp261, %add228
  %add343 = sub i32 %conv324, %add321
  %sub352 = add i32 %add343, %shr338
  %conv355 = sext i16 %and34826 to i32
  %mul356 = mul i32 %sub352, %conv355
  %add360 = add i32 %mul356, %conv313
  %shr362 = ashr i32 %add360, 5
  %add365 = add i32 %shr362, %add321
  %mul369 = mul i32 %add365, %conv242
  %tmp371 = load i32 addrspace(2)* @uv_final_rounding_32, align 4
  %add372 = add i32 %mul369, %tmp371
  %shr37435 = lshr i32 %add372, 7
  %conv376 = trunc i32 %shr37435 to i16
  %call378 = call signext i16 @__max_1i16(i16 signext %conv376, i16 signext %tmp207) nounwind
  %tmp379 = load i16 addrspace(2)* @uv_max_val_16, align 2
  %call380 = call signext i16 @__min_1i16(i16 signext %call378, i16 signext %tmp379) nounwind
  %tmp383 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %sub385 = sub i16 %call380, %tmp383
  %conv389 = sext i16 %add239 to i32
  %mul390 = mul i32 %add365, %conv389
  %tmp392 = load i32 addrspace(2)* @uv_final_rounding_32, align 4
  %add393 = add i32 %mul390, %tmp392
  %shr39536 = lshr i32 %add393, 7
  %conv397 = trunc i32 %shr39536 to i16
  %call399 = call signext i16 @__max_1i16(i16 signext %conv397, i16 signext %tmp383) nounwind
  %tmp400 = load i16 addrspace(2)* @uv_max_val_16, align 2
  %call401 = call signext i16 @__min_1i16(i16 signext %call399, i16 signext %tmp400) nounwind
  %tmp404 = load i16 addrspace(2)* @uv_min_val_16, align 2
  %sub406 = sub i16 %call401, %tmp404
  %conv41138 = zext i16 %sub385 to i32
  %conv41540 = zext i16 %sub406 to i32
  %shl = shl nuw i32 %conv21337, 16
  %or = or i32 %conv41138, %shl
  store i32 %or, i32 addrspace(1)* %conv, align 4
  %shl430 = shl nuw i32 %conv21739, 16
  %or435 = or i32 %conv41540, %shl430
  store i32 %or435, i32 addrspace(1)* %conv19, align 4
  %inc437 = add i32 %inc4373142, 2
  %inc439 = add i32 %storemerge43, 1
  %cmp9 = icmp ult i32 %inc439, %div
  br i1 %cmp9, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare signext i16 @__min_1i16(i16 signext, i16 signext)

declare signext i16 @__max_1i16(i16 signext, i16 signext)
