; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'tcc.cl'

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@uv_min_val_16 = addrspace(2) global i16 -512, align 2		; <i16 addrspace(2)*> [#uses=15]
@uv_max_val_16 = addrspace(2) global i16 511, align 2		; <i16 addrspace(2)*> [#uses=5]
@hi_key_mask_16 = addrspace(2) global i16 -32, align 2		; <i16 addrspace(2)*> [#uses=2]
@alpha_mask_SF_16 = addrspace(2) global i16 31, align 2		; <i16 addrspace(2)*> [#uses=4]
@rounding_SF_16 = addrspace(2) global i16 16, align 2		; <i16 addrspace(2)*> [#uses=6]
@uv_final_rounding_32 = addrspace(2) global i32 64, align 4		; <i32 addrspace(2)*> [#uses=4]
@uv_min_val_8_s16 = addrspace(2) global <8 x i16> <i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512>, align 16		; <<8 x i16> addrspace(2)*> [#uses=4]
@uv_max_val_8_s16 = addrspace(2) global <8 x i16> <i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511>, align 16		; <<8 x i16> addrspace(2)*> [#uses=2]
@hi_key_mask_8_s16 = addrspace(2) global <8 x i16> <i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32>, align 16		; <<8 x i16> addrspace(2)*> [#uses=1]
@alpha_mask_SF_8_s16 = addrspace(2) global <8 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>, align 16		; <<8 x i16> addrspace(2)*> [#uses=2]
@rounding_SF_8_s16 = addrspace(2) global <8 x i16> <i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16>, align 16		; <<8 x i16> addrspace(2)*> [#uses=3]
@precision_8_s16 = addrspace(2) global <8 x i16> <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>, align 16		; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_8_s16 = addrspace(2) global <8 x i16> zeroinitializer, align 16		; <<8 x i16> addrspace(2)*> [#uses=2]
@u_key_16_mask_8_s16 = addrspace(2) global <8 x i16> <i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528>, align 16		; <<8 x i16> addrspace(2)*> [#uses=1]
@ff_const_8_u16 = addrspace(2) global <8 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 16		; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_8_u16 = addrspace(2) global <8 x i16> zeroinitializer, align 16		; <<8 x i16> addrspace(2)*> [#uses=0]
@eight_const_8_u16 = addrspace(2) global <8 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 16		; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_16_s16 = addrspace(2) global <16 x i16> zeroinitializer, align 32		; <<16 x i16> addrspace(2)*> [#uses=0]
@ff_const_16_u16 = addrspace(2) global <16 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 32		; <<16 x i16> addrspace(2)*> [#uses=1]
@zero_const_16_u16 = addrspace(2) global <16 x i16> zeroinitializer, align 32		; <<16 x i16> addrspace(2)*> [#uses=0]
@eight_const_16_u16 = addrspace(2) global <16 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 32		; <<16 x i16> addrspace(2)*> [#uses=0]
@sat_bit_minus_one_4_s32 = addrspace(2) global <4 x i32> <i32 7, i32 7, i32 7, i32 7>, align 16		; <<4 x i32> addrspace(2)*> [#uses=0]
@uv_final_rounding_4_s32 = addrspace(2) global <4 x i32> <i32 64, i32 64, i32 64, i32 64>, align 16		; <<4 x i32> addrspace(2)*> [#uses=0]
@zero_const_4_u32 = addrspace(2) global <4 x i32> zeroinitializer, align 16		; <<4 x i32> addrspace(2)*> [#uses=0]
@ff00ff_const_4_u32 = addrspace(2) global <4 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 16		; <<4 x i32> addrspace(2)*> [#uses=0]
@sat_bit_minus_one_8_s32 = addrspace(2) global <8 x i32> <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>, align 32		; <<8 x i32> addrspace(2)*> [#uses=0]
@uv_final_rounding_8_s32 = addrspace(2) global <8 x i32> <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>, align 32		; <<8 x i32> addrspace(2)*> [#uses=2]
@zero_const_8_u32 = addrspace(2) global <8 x i32> zeroinitializer, align 32		; <<8 x i32> addrspace(2)*> [#uses=1]
@ff00ff_const_8_u32 = addrspace(2) global <8 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 32		; <<8 x i32> addrspace(2)*> [#uses=0]
@sgv = internal constant [8 x i8] c"1122082\00"		; <[8 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [7 x i8] c"112208\00"		; <[7 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [2 x %0] [%0 { i8* bitcast (void (<8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, i32, i8 addrspace(2)*, i32 addrspace(1)*, ...)* @tcc_vector8_third_optimization to i8*), i8* getelementptr ([8 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (i16 addrspace(1)*, i16 addrspace(1)*, i16 addrspace(1)*, i16 addrspace(1)*, i32, i8 addrspace(2)*, ...)* @tcc_scalar_unroll2 to i8*), i8* getelementptr ([7 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv3 to i8*), i32 0 }], section "llvm.metadata"		; <[2 x %0]*> [#uses=0]

; CHECK: @tcc_vector8_third_optimization

define void @tcc_vector8_third_optimization(<8 x i16> addrspace(1)* %Uin, <8 x i16> addrspace(1)* %Vin, <8 x i16> addrspace(1)* %Uout, <8 x i16> addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry, i32 addrspace(1)* %timeStamps, ...) nounwind {
entry:
	%Uin.addr = alloca <8 x i16> addrspace(1)*		; <<8 x i16> addrspace(1)**> [#uses=2]
	%Vin.addr = alloca <8 x i16> addrspace(1)*		; <<8 x i16> addrspace(1)**> [#uses=2]
	%Uout.addr = alloca <8 x i16> addrspace(1)*		; <<8 x i16> addrspace(1)**> [#uses=2]
	%Vout.addr = alloca <8 x i16> addrspace(1)*		; <<8 x i16> addrspace(1)**> [#uses=2]
	%bufferSize.addr = alloca i32		; <i32*> [#uses=2]
	%SatLUTEntry.addr = alloca i8 addrspace(2)*		; <i8 addrspace(2)**> [#uses=9]
	%timeStamps.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=1]
	%globalId = alloca i32, align 4		; <i32*> [#uses=2]
	%globalSize = alloca i32, align 4		; <i32*> [#uses=2]
	%numOfPixelsToProcess = alloca i32, align 4		; <i32*> [#uses=3]
	%pixel_index = alloca i32, align 4		; <i32*> [#uses=7]
	%iter = alloca i32, align 4		; <i32*> [#uses=4]
	%u16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=11]
	%v16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=11]
	%u_key_16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=15]
	%v_key_16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=2]
	%uv_tmp_16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=5]
	%sat_data_uint8 = alloca <8 x i32>, align 32		; <<8 x i32>*> [#uses=18]
	%sat_data = alloca <16 x i16>, align 32		; <<16 x i16>*> [#uses=5]
	%sat_data_shifted_8 = alloca <16 x i16>, align 32		; <<16 x i16>*> [#uses=2]
	%delta_sat_data = alloca <16 x i16>, align 32		; <<16 x i16>*> [#uses=3]
	%SF0 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=15]
	%SF1 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=17]
	%SF_original = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=6]
	%uvl_short16 = alloca <16 x i16>, align 32		; <<16 x i16>*> [#uses=14]
	%uvl_int8 = alloca <8 x i32>, align 32		; <<8 x i32>*> [#uses=12]
	%uvl_short8 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=4]
	%uv_min_val_8_16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=1]
	%uv_max_val_8_16 = alloca <8 x i16>, align 16		; <<8 x i16>*> [#uses=1]
	store <8 x i16> addrspace(1)* %Uin, <8 x i16> addrspace(1)** %Uin.addr
	store <8 x i16> addrspace(1)* %Vin, <8 x i16> addrspace(1)** %Vin.addr
	store <8 x i16> addrspace(1)* %Uout, <8 x i16> addrspace(1)** %Uout.addr
	store <8 x i16> addrspace(1)* %Vout, <8 x i16> addrspace(1)** %Vout.addr
	store i32 %bufferSize, i32* %bufferSize.addr
	store i8 addrspace(2)* %SatLUTEntry, i8 addrspace(2)** %SatLUTEntry.addr
	store i32 addrspace(1)* %timeStamps, i32 addrspace(1)** %timeStamps.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalId
	%call1 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalSize
	%tmp = load i32* %bufferSize.addr		; <i32> [#uses=1]
	%tmp2 = load i32* %globalSize		; <i32> [#uses=2]
	%cmp = icmp ne i32 %tmp2, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp, i32 %tmp2, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp, %nonzero		; <i32> [#uses=1]
	store i32 %div, i32* %numOfPixelsToProcess
	%tmp4 = load i32* %globalId		; <i32> [#uses=1]
	%tmp5 = load i32* %numOfPixelsToProcess		; <i32> [#uses=1]
	%mul = mul i32 %tmp4, %tmp5		; <i32> [#uses=1]
	%div6 = udiv i32 %mul, 8		; <i32> [#uses=1]
	store i32 %div6, i32* %pixel_index
	store i32 0, i32* %iter
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp8 = load i32* %iter		; <i32> [#uses=1]
	%tmp9 = load i32* %numOfPixelsToProcess		; <i32> [#uses=1]
	%cmp10 = icmp ult i32 %tmp8, %tmp9		; <i1> [#uses=1]
	br i1 %cmp10, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp12 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp13 = load <8 x i16> addrspace(1)** %Uin.addr		; <<8 x i16> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <8 x i16> addrspace(1)* %tmp13, i32 %tmp12		; <<8 x i16> addrspace(1)*> [#uses=1]
	%tmp14 = load <8 x i16> addrspace(1)* %arrayidx		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp14, <8 x i16>* %u16
	%tmp16 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp17 = load <8 x i16> addrspace(1)** %Vin.addr		; <<8 x i16> addrspace(1)*> [#uses=1]
	%arrayidx18 = getelementptr <8 x i16> addrspace(1)* %tmp17, i32 %tmp16		; <<8 x i16> addrspace(1)*> [#uses=1]
	%tmp19 = load <8 x i16> addrspace(1)* %arrayidx18		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp19, <8 x i16>* %v16
	%tmp20 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%tmp21 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16		; <<8 x i16>> [#uses=1]
	%add = add <8 x i16> %tmp20, %tmp21		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add, <8 x i16>* %u16
	%tmp22 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%tmp23 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16		; <<8 x i16>> [#uses=1]
	%add24 = add <8 x i16> %tmp22, %tmp23		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add24, <8 x i16>* %v16
	%tmp26 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%shr = ashr <8 x i16> %tmp26, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %shr, <8 x i16>* %u_key_16
	%tmp28 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%tmp29 = load <8 x i16> addrspace(2)* @hi_key_mask_8_s16		; <<8 x i16>> [#uses=1]
	%and = and <8 x i16> %tmp28, %tmp29		; <<8 x i16>> [#uses=1]
	store <8 x i16> %and, <8 x i16>* %v_key_16
	%tmp30 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%tmp31 = load <8 x i16>* %v_key_16		; <<8 x i16>> [#uses=1]
	%add32 = add <8 x i16> %tmp30, %tmp31		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add32, <8 x i16>* %u_key_16
	%tmp34 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%tmp35 = load <8 x i16> addrspace(2)* @alpha_mask_SF_8_s16		; <<8 x i16>> [#uses=1]
	%and36 = and <8 x i16> %tmp34, %tmp35		; <<8 x i16>> [#uses=1]
	store <8 x i16> %and36, <8 x i16>* %uv_tmp_16
	%tmp38 = load <8 x i32> addrspace(2)* @zero_const_8_u32		; <<8 x i32>> [#uses=1]
	store <8 x i32> %tmp38, <8 x i32>* %sat_data_uint8
	%tmp39 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%tmp40 = load <8 x i16> addrspace(2)* @u_key_16_mask_8_s16		; <<8 x i16>> [#uses=1]
	%add41 = add <8 x i16> %tmp39, %tmp40		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add41, <8 x i16>* %u_key_16
	%tmp42 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%shl = shl <8 x i16> %tmp42, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %shl, <8 x i16>* %u_key_16
	%tmp43 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext = extractelement <8 x i16> %tmp43, i32 0		; <i16> [#uses=1]
	%tmp44 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom = sext i16 %vecext to i32		; <i32> [#uses=1]
	%arrayidx45 = getelementptr i8 addrspace(2)* %tmp44, i32 %idxprom		; <i8 addrspace(2)*> [#uses=1]
	%conv = bitcast i8 addrspace(2)* %arrayidx45 to i32*		; <i32*> [#uses=1]
	%tmp46 = load i32* %conv		; <i32> [#uses=1]
	%tmp47 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins = insertelement <8 x i32> %tmp47, i32 %tmp46, i32 0		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins, <8 x i32>* %sat_data_uint8
	%tmp48 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext49 = extractelement <8 x i16> %tmp48, i32 1		; <i16> [#uses=1]
	%tmp50 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom51 = sext i16 %vecext49 to i32		; <i32> [#uses=1]
	%arrayidx52 = getelementptr i8 addrspace(2)* %tmp50, i32 %idxprom51		; <i8 addrspace(2)*> [#uses=1]
	%conv53 = bitcast i8 addrspace(2)* %arrayidx52 to i32*		; <i32*> [#uses=1]
	%tmp54 = load i32* %conv53		; <i32> [#uses=1]
	%tmp55 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins56 = insertelement <8 x i32> %tmp55, i32 %tmp54, i32 1		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins56, <8 x i32>* %sat_data_uint8
	%tmp57 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext58 = extractelement <8 x i16> %tmp57, i32 2		; <i16> [#uses=1]
	%tmp59 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom60 = sext i16 %vecext58 to i32		; <i32> [#uses=1]
	%arrayidx61 = getelementptr i8 addrspace(2)* %tmp59, i32 %idxprom60		; <i8 addrspace(2)*> [#uses=1]
	%conv62 = bitcast i8 addrspace(2)* %arrayidx61 to i32*		; <i32*> [#uses=1]
	%tmp63 = load i32* %conv62		; <i32> [#uses=1]
	%tmp64 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins65 = insertelement <8 x i32> %tmp64, i32 %tmp63, i32 2		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins65, <8 x i32>* %sat_data_uint8
	%tmp66 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext67 = extractelement <8 x i16> %tmp66, i32 3		; <i16> [#uses=1]
	%tmp68 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom69 = sext i16 %vecext67 to i32		; <i32> [#uses=1]
	%arrayidx70 = getelementptr i8 addrspace(2)* %tmp68, i32 %idxprom69		; <i8 addrspace(2)*> [#uses=1]
	%conv71 = bitcast i8 addrspace(2)* %arrayidx70 to i32*		; <i32*> [#uses=1]
	%tmp72 = load i32* %conv71		; <i32> [#uses=1]
	%tmp73 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins74 = insertelement <8 x i32> %tmp73, i32 %tmp72, i32 3		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins74, <8 x i32>* %sat_data_uint8
	%tmp75 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext76 = extractelement <8 x i16> %tmp75, i32 4		; <i16> [#uses=1]
	%tmp77 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom78 = sext i16 %vecext76 to i32		; <i32> [#uses=1]
	%arrayidx79 = getelementptr i8 addrspace(2)* %tmp77, i32 %idxprom78		; <i8 addrspace(2)*> [#uses=1]
	%conv80 = bitcast i8 addrspace(2)* %arrayidx79 to i32*		; <i32*> [#uses=1]
	%tmp81 = load i32* %conv80		; <i32> [#uses=1]
	%tmp82 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins83 = insertelement <8 x i32> %tmp82, i32 %tmp81, i32 4		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins83, <8 x i32>* %sat_data_uint8
	%tmp84 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext85 = extractelement <8 x i16> %tmp84, i32 5		; <i16> [#uses=1]
	%tmp86 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom87 = sext i16 %vecext85 to i32		; <i32> [#uses=1]
	%arrayidx88 = getelementptr i8 addrspace(2)* %tmp86, i32 %idxprom87		; <i8 addrspace(2)*> [#uses=1]
	%conv89 = bitcast i8 addrspace(2)* %arrayidx88 to i32*		; <i32*> [#uses=1]
	%tmp90 = load i32* %conv89		; <i32> [#uses=1]
	%tmp91 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins92 = insertelement <8 x i32> %tmp91, i32 %tmp90, i32 5		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins92, <8 x i32>* %sat_data_uint8
	%tmp93 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext94 = extractelement <8 x i16> %tmp93, i32 6		; <i16> [#uses=1]
	%tmp95 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom96 = sext i16 %vecext94 to i32		; <i32> [#uses=1]
	%arrayidx97 = getelementptr i8 addrspace(2)* %tmp95, i32 %idxprom96		; <i8 addrspace(2)*> [#uses=1]
	%conv98 = bitcast i8 addrspace(2)* %arrayidx97 to i32*		; <i32*> [#uses=1]
	%tmp99 = load i32* %conv98		; <i32> [#uses=1]
	%tmp100 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins101 = insertelement <8 x i32> %tmp100, i32 %tmp99, i32 6		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins101, <8 x i32>* %sat_data_uint8
	%tmp102 = load <8 x i16>* %u_key_16		; <<8 x i16>> [#uses=1]
	%vecext103 = extractelement <8 x i16> %tmp102, i32 7		; <i16> [#uses=1]
	%tmp104 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom105 = sext i16 %vecext103 to i32		; <i32> [#uses=1]
	%arrayidx106 = getelementptr i8 addrspace(2)* %tmp104, i32 %idxprom105		; <i8 addrspace(2)*> [#uses=1]
	%conv107 = bitcast i8 addrspace(2)* %arrayidx106 to i32*		; <i32*> [#uses=1]
	%tmp108 = load i32* %conv107		; <i32> [#uses=1]
	%tmp109 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%vecins110 = insertelement <8 x i32> %tmp109, i32 %tmp108, i32 7		; <<8 x i32>> [#uses=1]
	store <8 x i32> %vecins110, <8 x i32>* %sat_data_uint8
	%tmp112 = load <8 x i32>* %sat_data_uint8		; <<8 x i32>> [#uses=1]
	%astype = bitcast <8 x i32> %tmp112 to <16 x i16>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %astype, <16 x i16>* %sat_data
	%tmp114 = load <16 x i16>* %sat_data		; <<16 x i16>> [#uses=1]
	%shr115 = lshr <16 x i16> %tmp114, <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %shr115, <16 x i16>* %sat_data_shifted_8
	%tmp116 = load <16 x i16>* %sat_data		; <<16 x i16>> [#uses=1]
	%tmp117 = load <16 x i16> addrspace(2)* @ff_const_16_u16		; <<16 x i16>> [#uses=1]
	%and118 = and <16 x i16> %tmp116, %tmp117		; <<16 x i16>> [#uses=1]
	store <16 x i16> %and118, <16 x i16>* %sat_data
	%tmp120 = load <16 x i16>* %sat_data		; <<16 x i16>> [#uses=1]
	%tmp121 = load <16 x i16>* %sat_data_shifted_8		; <<16 x i16>> [#uses=1]
	%sub = sub <16 x i16> %tmp120, %tmp121		; <<16 x i16>> [#uses=1]
	store <16 x i16> %sub, <16 x i16>* %delta_sat_data
	%tmp123 = load <8 x i16> addrspace(2)* @zero_const_8_s16		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp123, <8 x i16>* %SF0
	%tmp125 = load <8 x i16> addrspace(2)* @zero_const_8_s16		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp125, <8 x i16>* %SF1
	%tmp127 = load <16 x i16>* %delta_sat_data		; <<16 x i16>> [#uses=1]
	%tmp128 = shufflevector <16 x i16> %tmp127, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp128, <8 x i16>* %SF_original
	%tmp129 = load <8 x i16>* %SF_original		; <<8 x i16>> [#uses=1]
	%tmp130 = load <8 x i16>* %uv_tmp_16		; <<8 x i16>> [#uses=1]
	%mul131 = mul <8 x i16> %tmp129, %tmp130		; <<8 x i16>> [#uses=1]
	store <8 x i16> %mul131, <8 x i16>* %SF0
	%tmp132 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%tmp133 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16		; <<8 x i16>> [#uses=1]
	%add134 = add <8 x i16> %tmp132, %tmp133		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add134, <8 x i16>* %SF0
	%tmp135 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%shr136 = ashr <8 x i16> %tmp135, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %shr136, <8 x i16>* %SF0
	%tmp137 = load <8 x i16>* %SF_original		; <<8 x i16>> [#uses=1]
	%tmp138 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%add139 = add <8 x i16> %tmp137, %tmp138		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add139, <8 x i16>* %SF0
	%tmp140 = load <16 x i16>* %delta_sat_data		; <<16 x i16>> [#uses=1]
	%tmp141 = shufflevector <16 x i16> %tmp140, <16 x i16> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp141, <8 x i16>* %SF_original
	%tmp142 = load <8 x i16>* %SF_original		; <<8 x i16>> [#uses=1]
	%tmp143 = load <8 x i16>* %uv_tmp_16		; <<8 x i16>> [#uses=1]
	%mul144 = mul <8 x i16> %tmp142, %tmp143		; <<8 x i16>> [#uses=1]
	store <8 x i16> %mul144, <8 x i16>* %SF1
	%tmp145 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%tmp146 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16		; <<8 x i16>> [#uses=1]
	%add147 = add <8 x i16> %tmp145, %tmp146		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add147, <8 x i16>* %SF1
	%tmp148 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%shr149 = ashr <8 x i16> %tmp148, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %shr149, <8 x i16>* %SF1
	%tmp150 = load <8 x i16>* %SF_original		; <<8 x i16>> [#uses=1]
	%tmp151 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%add152 = add <8 x i16> %tmp150, %tmp151		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add152, <8 x i16>* %SF1
	%tmp153 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%tmp154 = load <8 x i16> addrspace(2)* @alpha_mask_SF_8_s16		; <<8 x i16>> [#uses=1]
	%and155 = and <8 x i16> %tmp153, %tmp154		; <<8 x i16>> [#uses=1]
	store <8 x i16> %and155, <8 x i16>* %uv_tmp_16
	%tmp156 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%tmp157 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%sub158 = sub <8 x i16> %tmp156, %tmp157		; <<8 x i16>> [#uses=1]
	store <8 x i16> %sub158, <8 x i16>* %SF1
	%tmp159 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%tmp160 = load <8 x i16>* %uv_tmp_16		; <<8 x i16>> [#uses=1]
	%mul161 = mul <8 x i16> %tmp159, %tmp160		; <<8 x i16>> [#uses=1]
	store <8 x i16> %mul161, <8 x i16>* %SF1
	%tmp162 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%tmp163 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16		; <<8 x i16>> [#uses=1]
	%add164 = add <8 x i16> %tmp162, %tmp163		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add164, <8 x i16>* %SF1
	%tmp165 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%shr166 = ashr <8 x i16> %tmp165, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %shr166, <8 x i16>* %SF1
	%tmp167 = load <8 x i16>* %SF1		; <<8 x i16>> [#uses=1]
	%tmp168 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%add169 = add <8 x i16> %tmp167, %tmp168		; <<8 x i16>> [#uses=1]
	store <8 x i16> %add169, <8 x i16>* %SF0
	%tmp171 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%tmp172 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%mul173 = mul <8 x i16> %tmp171, %tmp172		; <<8 x i16>> [#uses=1]
	%tmp174 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp175 = shufflevector <8 x i16> %mul173, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>		; <<16 x i16>> [#uses=1]
	%tmp176 = shufflevector <16 x i16> %tmp174, <16 x i16> %tmp175, <16 x i32> <i32 16, i32 1, i32 17, i32 3, i32 18, i32 5, i32 19, i32 7, i32 20, i32 9, i32 21, i32 11, i32 22, i32 13, i32 23, i32 15>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %tmp176, <16 x i16>* %uvl_short16
	%tmp177 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%tmp178 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%call179 = call <8 x i16> @__mul_hi_8i16(<8 x i16> %tmp177, <8 x i16> %tmp178)		; <<8 x i16>> [#uses=1]
	%tmp180 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp181 = shufflevector <8 x i16> %call179, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>		; <<16 x i16>> [#uses=1]
	%tmp182 = shufflevector <16 x i16> %tmp180, <16 x i16> %tmp181, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %tmp182, <16 x i16>* %uvl_short16
	%tmp184 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%astype185 = bitcast <16 x i16> %tmp184 to <8 x i32>		; <<8 x i32>> [#uses=1]
	store <8 x i32> %astype185, <8 x i32>* %uvl_int8
	%tmp186 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%tmp187 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32		; <<8 x i32>> [#uses=1]
	%add188 = add <8 x i32> %tmp186, %tmp187		; <<8 x i32>> [#uses=1]
	store <8 x i32> %add188, <8 x i32>* %uvl_int8
	%tmp189 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%shr190 = ashr <8 x i32> %tmp189, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>		; <<8 x i32>> [#uses=1]
	store <8 x i32> %shr190, <8 x i32>* %uvl_int8
	%tmp191 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%astype192 = bitcast <8 x i32> %tmp191 to <16 x i16>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %astype192, <16 x i16>* %uvl_short16
	%tmp194 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp195 = shufflevector <16 x i16> %tmp194, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp195, <8 x i16>* %uvl_short8
	%tmp197 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%tmp198 = insertelement <8 x i16> undef, i16 %tmp197, i32 0		; <<8 x i16>> [#uses=2]
	%splat = shufflevector <8 x i16> %tmp198, <8 x i16> %tmp198, <8 x i32> zeroinitializer		; <<8 x i16>> [#uses=1]
	store <8 x i16> %splat, <8 x i16>* %uv_min_val_8_16
	%tmp200 = load i16 addrspace(2)* @uv_max_val_16		; <i16> [#uses=1]
	%tmp201 = insertelement <8 x i16> undef, i16 %tmp200, i32 0		; <<8 x i16>> [#uses=2]
	%splat202 = shufflevector <8 x i16> %tmp201, <8 x i16> %tmp201, <8 x i32> zeroinitializer		; <<8 x i16>> [#uses=1]
	store <8 x i16> %splat202, <8 x i16>* %uv_max_val_8_16
	%tmp203 = load <8 x i16>* %uvl_short8		; <<8 x i16>> [#uses=1]
	%tmp204 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16		; <<8 x i16>> [#uses=1]
	%call205 = call <8 x i16> @__max_8i16(<8 x i16> %tmp203, <8 x i16> %tmp204)		; <<8 x i16>> [#uses=1]
	%tmp206 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16		; <<8 x i16>> [#uses=1]
	%call207 = call <8 x i16> @__min_8i16(<8 x i16> %call205, <8 x i16> %tmp206)		; <<8 x i16>> [#uses=1]
	store <8 x i16> %call207, <8 x i16>* %u16
	%tmp208 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	%tmp209 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%tmp210 = insertelement <8 x i16> undef, i16 %tmp209, i32 0		; <<8 x i16>> [#uses=2]
	%splat211 = shufflevector <8 x i16> %tmp210, <8 x i16> %tmp210, <8 x i32> zeroinitializer		; <<8 x i16>> [#uses=1]
	%sub212 = sub <8 x i16> %tmp208, %splat211		; <<8 x i16>> [#uses=1]
	store <8 x i16> %sub212, <8 x i16>* %u16
	%tmp213 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp214 = load <8 x i16> addrspace(1)** %Uout.addr		; <<8 x i16> addrspace(1)*> [#uses=1]
	%arrayidx215 = getelementptr <8 x i16> addrspace(1)* %tmp214, i32 %tmp213		; <<8 x i16> addrspace(1)*> [#uses=1]
	%tmp216 = load <8 x i16>* %u16		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp216, <8 x i16> addrspace(1)* %arrayidx215
	%tmp217 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%tmp218 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%mul219 = mul <8 x i16> %tmp217, %tmp218		; <<8 x i16>> [#uses=1]
	%tmp220 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp221 = shufflevector <8 x i16> %mul219, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>		; <<16 x i16>> [#uses=1]
	%tmp222 = shufflevector <16 x i16> %tmp220, <16 x i16> %tmp221, <16 x i32> <i32 16, i32 1, i32 17, i32 3, i32 18, i32 5, i32 19, i32 7, i32 20, i32 9, i32 21, i32 11, i32 22, i32 13, i32 23, i32 15>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %tmp222, <16 x i16>* %uvl_short16
	%tmp223 = load <8 x i16>* %SF0		; <<8 x i16>> [#uses=1]
	%tmp224 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%call225 = call <8 x i16> @__mul_hi_8i16(<8 x i16> %tmp223, <8 x i16> %tmp224)		; <<8 x i16>> [#uses=1]
	%tmp226 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp227 = shufflevector <8 x i16> %call225, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>		; <<16 x i16>> [#uses=1]
	%tmp228 = shufflevector <16 x i16> %tmp226, <16 x i16> %tmp227, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %tmp228, <16 x i16>* %uvl_short16
	%tmp229 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%astype230 = bitcast <16 x i16> %tmp229 to <8 x i32>		; <<8 x i32>> [#uses=1]
	store <8 x i32> %astype230, <8 x i32>* %uvl_int8
	%tmp231 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%tmp232 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32		; <<8 x i32>> [#uses=1]
	%add233 = add <8 x i32> %tmp231, %tmp232		; <<8 x i32>> [#uses=1]
	store <8 x i32> %add233, <8 x i32>* %uvl_int8
	%tmp234 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%shr235 = ashr <8 x i32> %tmp234, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>		; <<8 x i32>> [#uses=1]
	store <8 x i32> %shr235, <8 x i32>* %uvl_int8
	%tmp236 = load <8 x i32>* %uvl_int8		; <<8 x i32>> [#uses=1]
	%astype237 = bitcast <8 x i32> %tmp236 to <16 x i16>		; <<16 x i16>> [#uses=1]
	store <16 x i16> %astype237, <16 x i16>* %uvl_short16
	%tmp238 = load <16 x i16>* %uvl_short16		; <<16 x i16>> [#uses=1]
	%tmp239 = shufflevector <16 x i16> %tmp238, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14>		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp239, <8 x i16>* %uvl_short8
	%tmp240 = load <8 x i16>* %uvl_short8		; <<8 x i16>> [#uses=1]
	%tmp241 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16		; <<8 x i16>> [#uses=1]
	%call242 = call <8 x i16> @__max_8i16(<8 x i16> %tmp240, <8 x i16> %tmp241)		; <<8 x i16>> [#uses=1]
	%tmp243 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16		; <<8 x i16>> [#uses=1]
	%call244 = call <8 x i16> @__min_8i16(<8 x i16> %call242, <8 x i16> %tmp243)		; <<8 x i16>> [#uses=1]
	store <8 x i16> %call244, <8 x i16>* %v16
	%tmp245 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	%tmp246 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%tmp247 = insertelement <8 x i16> undef, i16 %tmp246, i32 0		; <<8 x i16>> [#uses=2]
	%splat248 = shufflevector <8 x i16> %tmp247, <8 x i16> %tmp247, <8 x i32> zeroinitializer		; <<8 x i16>> [#uses=1]
	%sub249 = sub <8 x i16> %tmp245, %splat248		; <<8 x i16>> [#uses=1]
	store <8 x i16> %sub249, <8 x i16>* %v16
	%tmp250 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp251 = load <8 x i16> addrspace(1)** %Vout.addr		; <<8 x i16> addrspace(1)*> [#uses=1]
	%arrayidx252 = getelementptr <8 x i16> addrspace(1)* %tmp251, i32 %tmp250		; <<8 x i16> addrspace(1)*> [#uses=1]
	%tmp253 = load <8 x i16>* %v16		; <<8 x i16>> [#uses=1]
	store <8 x i16> %tmp253, <8 x i16> addrspace(1)* %arrayidx252
	%tmp254 = load i32* %pixel_index		; <i32> [#uses=1]
	%inc = add i32 %tmp254, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %pixel_index
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp255 = load i32* %iter		; <i32> [#uses=1]
	%add256 = add i32 %tmp255, 8		; <i32> [#uses=1]
	store i32 %add256, i32* %iter
	br label %for.cond

for.end:		; preds = %for.cond
	ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare <8 x i16> @__mul_hi_8i16(<8 x i16>, <8 x i16>)

declare <8 x i16> @__min_8i16(<8 x i16>, <8 x i16>)

declare <8 x i16> @__max_8i16(<8 x i16>, <8 x i16>)

define void @tcc_scalar_unroll2(i16 addrspace(1)* %Uin, i16 addrspace(1)* %Vin, i16 addrspace(1)* %Uout, i16 addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry, ...) nounwind {
entry:
	%Uin.addr = alloca i16 addrspace(1)*		; <i16 addrspace(1)**> [#uses=3]
	%Vin.addr = alloca i16 addrspace(1)*		; <i16 addrspace(1)**> [#uses=3]
	%Uout.addr = alloca i16 addrspace(1)*		; <i16 addrspace(1)**> [#uses=2]
	%Vout.addr = alloca i16 addrspace(1)*		; <i16 addrspace(1)**> [#uses=2]
	%bufferSize.addr = alloca i32		; <i32*> [#uses=2]
	%SatLUTEntry.addr = alloca i8 addrspace(2)*		; <i8 addrspace(2)**> [#uses=9]
	%globalId = alloca i32, align 4		; <i32*> [#uses=2]
	%globalSize = alloca i32, align 4		; <i32*> [#uses=2]
	%numOfPixelsToProcess = alloca i32, align 4		; <i32*> [#uses=3]
	%pixel_index = alloca i32, align 4		; <i32*> [#uses=11]
	%iter = alloca i32, align 4		; <i32*> [#uses=4]
	%uOut = alloca [2 x i32], align 4		; <[2 x i32]*> [#uses=4]
	%vOut = alloca [2 x i32], align 4		; <[2 x i32]*> [#uses=4]
	%UoutPtr = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=2]
	%VoutPtr = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=2]
	%u16 = alloca i16, align 2		; <i16*> [#uses=16]
	%v16 = alloca i16, align 2		; <i16*> [#uses=16]
	%u_key_16 = alloca i16, align 2		; <i16*> [#uses=8]
	%v_key_16 = alloca i16, align 2		; <i16*> [#uses=2]
	%uv_tmp_16 = alloca i16, align 2		; <i16*> [#uses=10]
	%SF0 = alloca i32, align 4		; <i32*> [#uses=29]
	%SF1 = alloca i32, align 4		; <i32*> [#uses=37]
	%index = alloca i16, align 2		; <i16*> [#uses=10]
	%sat_data = alloca <4 x i8>, align 4		; <<4 x i8>*> [#uses=28]
	%uvl = alloca i32, align 4		; <i32*> [#uses=24]
	%v_key_16246 = alloca i16, align 2		; <i16*> [#uses=2]
	store i16 addrspace(1)* %Uin, i16 addrspace(1)** %Uin.addr
	store i16 addrspace(1)* %Vin, i16 addrspace(1)** %Vin.addr
	store i16 addrspace(1)* %Uout, i16 addrspace(1)** %Uout.addr
	store i16 addrspace(1)* %Vout, i16 addrspace(1)** %Vout.addr
	store i32 %bufferSize, i32* %bufferSize.addr
	store i8 addrspace(2)* %SatLUTEntry, i8 addrspace(2)** %SatLUTEntry.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalId
	%call1 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalSize
	%tmp = load i32* %bufferSize.addr		; <i32> [#uses=1]
	%tmp2 = load i32* %globalSize		; <i32> [#uses=2]
	%cmp = icmp ne i32 %tmp2, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp, i32 %tmp2, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp, %nonzero		; <i32> [#uses=1]
	store i32 %div, i32* %numOfPixelsToProcess
	%tmp4 = load i32* %globalId		; <i32> [#uses=1]
	%tmp5 = load i32* %numOfPixelsToProcess		; <i32> [#uses=1]
	%mul = mul i32 %tmp4, %tmp5		; <i32> [#uses=1]
	store i32 %mul, i32* %pixel_index
	store i32 0, i32* %iter
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp7 = load i32* %iter		; <i32> [#uses=1]
	%tmp8 = load i32* %numOfPixelsToProcess		; <i32> [#uses=1]
	%cmp9 = icmp ult i32 %tmp7, %tmp8		; <i1> [#uses=1]
	br i1 %cmp9, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp13 = load i16 addrspace(1)** %Uout.addr		; <i16 addrspace(1)*> [#uses=1]
	%tmp14 = load i32* %pixel_index		; <i32> [#uses=1]
	%add.ptr = getelementptr i16 addrspace(1)* %tmp13, i32 %tmp14		; <i16 addrspace(1)*> [#uses=1]
	%conv = bitcast i16 addrspace(1)* %add.ptr to i32 addrspace(1)*		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %conv, i32 addrspace(1)** %UoutPtr
	%tmp16 = load i16 addrspace(1)** %Vout.addr		; <i16 addrspace(1)*> [#uses=1]
	%tmp17 = load i32* %pixel_index		; <i32> [#uses=1]
	%add.ptr18 = getelementptr i16 addrspace(1)* %tmp16, i32 %tmp17		; <i16 addrspace(1)*> [#uses=1]
	%conv19 = bitcast i16 addrspace(1)* %add.ptr18 to i32 addrspace(1)*		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %conv19, i32 addrspace(1)** %VoutPtr
	%tmp21 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp22 = load i16 addrspace(1)** %Uin.addr		; <i16 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i16 addrspace(1)* %tmp22, i32 %tmp21		; <i16 addrspace(1)*> [#uses=1]
	%tmp23 = load i16 addrspace(1)* %arrayidx		; <i16> [#uses=1]
	%conv24 = zext i16 %tmp23 to i32		; <i32> [#uses=1]
	%conv25 = trunc i32 %conv24 to i16		; <i16> [#uses=1]
	%conv26 = sext i16 %conv25 to i32		; <i32> [#uses=1]
	%tmp27 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv28 = sext i16 %tmp27 to i32		; <i32> [#uses=1]
	%add = add i32 %conv26, %conv28		; <i32> [#uses=1]
	%conv29 = trunc i32 %add to i16		; <i16> [#uses=1]
	store i16 %conv29, i16* %u16
	%tmp31 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp32 = load i16 addrspace(1)** %Vin.addr		; <i16 addrspace(1)*> [#uses=1]
	%arrayidx33 = getelementptr i16 addrspace(1)* %tmp32, i32 %tmp31		; <i16 addrspace(1)*> [#uses=1]
	%tmp34 = load i16 addrspace(1)* %arrayidx33		; <i16> [#uses=1]
	%conv35 = zext i16 %tmp34 to i32		; <i32> [#uses=1]
	%conv36 = trunc i32 %conv35 to i16		; <i16> [#uses=1]
	%conv37 = sext i16 %conv36 to i32		; <i32> [#uses=1]
	%tmp38 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv39 = sext i16 %tmp38 to i32		; <i32> [#uses=1]
	%add40 = add i32 %conv37, %conv39		; <i32> [#uses=1]
	%conv41 = trunc i32 %add40 to i16		; <i16> [#uses=1]
	store i16 %conv41, i16* %v16
	%tmp43 = load i16* %u16		; <i16> [#uses=1]
	%conv44 = sext i16 %tmp43 to i32		; <i32> [#uses=1]
	%shr = ashr i32 %conv44, 5		; <i32> [#uses=1]
	%conv45 = trunc i32 %shr to i16		; <i16> [#uses=1]
	store i16 %conv45, i16* %u_key_16
	%tmp47 = load i16* %v16		; <i16> [#uses=1]
	%conv48 = sext i16 %tmp47 to i32		; <i32> [#uses=1]
	%tmp49 = load i16 addrspace(2)* @hi_key_mask_16		; <i16> [#uses=1]
	%conv50 = sext i16 %tmp49 to i32		; <i32> [#uses=1]
	%and = and i32 %conv48, %conv50		; <i32> [#uses=1]
	%conv51 = trunc i32 %and to i16		; <i16> [#uses=1]
	store i16 %conv51, i16* %v_key_16
	%tmp52 = load i16* %u_key_16		; <i16> [#uses=1]
	%conv53 = sext i16 %tmp52 to i32		; <i32> [#uses=1]
	%tmp54 = load i16* %v_key_16		; <i16> [#uses=1]
	%conv55 = sext i16 %tmp54 to i32		; <i32> [#uses=1]
	%add56 = add i32 %conv53, %conv55		; <i32> [#uses=1]
	%conv57 = trunc i32 %add56 to i16		; <i16> [#uses=1]
	store i16 %conv57, i16* %u_key_16
	%tmp59 = load i16* %v16		; <i16> [#uses=1]
	%conv60 = sext i16 %tmp59 to i32		; <i32> [#uses=1]
	%tmp61 = load i16 addrspace(2)* @alpha_mask_SF_16		; <i16> [#uses=1]
	%conv62 = sext i16 %tmp61 to i32		; <i32> [#uses=1]
	%and63 = and i32 %conv60, %conv62		; <i32> [#uses=1]
	%conv64 = trunc i32 %and63 to i16		; <i16> [#uses=1]
	store i16 %conv64, i16* %uv_tmp_16
	store i32 0, i32* %SF0
	store i32 0, i32* %SF1
	%tmp68 = load i16* %u_key_16		; <i16> [#uses=1]
	%conv69 = sext i16 %tmp68 to i32		; <i32> [#uses=1]
	%add70 = add i32 %conv69, 528		; <i32> [#uses=1]
	%conv71 = trunc i32 %add70 to i16		; <i16> [#uses=1]
	store i16 %conv71, i16* %index
	%tmp73 = load i16* %index		; <i16> [#uses=1]
	%tmp74 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom = sext i16 %tmp73 to i32		; <i32> [#uses=1]
	%arrayidx75 = getelementptr i8 addrspace(2)* %tmp74, i32 %idxprom		; <i8 addrspace(2)*> [#uses=1]
	%tmp76 = load i8 addrspace(2)* %arrayidx75		; <i8> [#uses=1]
	%tmp77 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp78 = insertelement <4 x i8> %tmp77, i8 %tmp76, i32 0		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp78, <4 x i8>* %sat_data
	%tmp79 = load i16* %index		; <i16> [#uses=1]
	%conv80 = sext i16 %tmp79 to i32		; <i32> [#uses=1]
	%add81 = add i32 %conv80, 1		; <i32> [#uses=1]
	%tmp82 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx83 = getelementptr i8 addrspace(2)* %tmp82, i32 %add81		; <i8 addrspace(2)*> [#uses=1]
	%tmp84 = load i8 addrspace(2)* %arrayidx83		; <i8> [#uses=1]
	%tmp85 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp86 = insertelement <4 x i8> %tmp85, i8 %tmp84, i32 1		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp86, <4 x i8>* %sat_data
	%tmp87 = load i16* %index		; <i16> [#uses=1]
	%conv88 = sext i16 %tmp87 to i32		; <i32> [#uses=1]
	%add89 = add i32 %conv88, 2		; <i32> [#uses=1]
	%tmp90 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx91 = getelementptr i8 addrspace(2)* %tmp90, i32 %add89		; <i8 addrspace(2)*> [#uses=1]
	%tmp92 = load i8 addrspace(2)* %arrayidx91		; <i8> [#uses=1]
	%tmp93 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp94 = insertelement <4 x i8> %tmp93, i8 %tmp92, i32 2		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp94, <4 x i8>* %sat_data
	%tmp95 = load i16* %index		; <i16> [#uses=1]
	%conv96 = sext i16 %tmp95 to i32		; <i32> [#uses=1]
	%add97 = add i32 %conv96, 3		; <i32> [#uses=1]
	%tmp98 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx99 = getelementptr i8 addrspace(2)* %tmp98, i32 %add97		; <i8 addrspace(2)*> [#uses=1]
	%tmp100 = load i8 addrspace(2)* %arrayidx99		; <i8> [#uses=1]
	%tmp101 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp102 = insertelement <4 x i8> %tmp101, i8 %tmp100, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp102, <4 x i8>* %sat_data
	%tmp103 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp104 = extractelement <4 x i8> %tmp103, i32 0		; <i8> [#uses=1]
	%conv105 = zext i8 %tmp104 to i32		; <i32> [#uses=1]
	%tmp106 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp107 = extractelement <4 x i8> %tmp106, i32 1		; <i8> [#uses=1]
	%conv108 = zext i8 %tmp107 to i32		; <i32> [#uses=1]
	%sub = sub i32 %conv105, %conv108		; <i32> [#uses=1]
	store i32 %sub, i32* %SF0
	%tmp109 = load i32* %SF0		; <i32> [#uses=1]
	%tmp110 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv111 = sext i16 %tmp110 to i32		; <i32> [#uses=1]
	%mul112 = mul i32 %tmp109, %conv111		; <i32> [#uses=1]
	store i32 %mul112, i32* %SF0
	%tmp113 = load i32* %SF0		; <i32> [#uses=1]
	%tmp114 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv115 = sext i16 %tmp114 to i32		; <i32> [#uses=1]
	%add116 = add i32 %tmp113, %conv115		; <i32> [#uses=1]
	store i32 %add116, i32* %SF0
	%tmp117 = load i32* %SF0		; <i32> [#uses=1]
	%shr118 = ashr i32 %tmp117, 5		; <i32> [#uses=1]
	store i32 %shr118, i32* %SF0
	%tmp119 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp120 = extractelement <4 x i8> %tmp119, i32 0		; <i8> [#uses=1]
	%conv121 = zext i8 %tmp120 to i32		; <i32> [#uses=1]
	%tmp122 = load i32* %SF0		; <i32> [#uses=1]
	%add123 = add i32 %conv121, %tmp122		; <i32> [#uses=1]
	store i32 %add123, i32* %SF0
	%tmp124 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp125 = extractelement <4 x i8> %tmp124, i32 2		; <i8> [#uses=1]
	%conv126 = zext i8 %tmp125 to i32		; <i32> [#uses=1]
	%tmp127 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp128 = extractelement <4 x i8> %tmp127, i32 3		; <i8> [#uses=1]
	%conv129 = zext i8 %tmp128 to i32		; <i32> [#uses=1]
	%sub130 = sub i32 %conv126, %conv129		; <i32> [#uses=1]
	store i32 %sub130, i32* %SF1
	%tmp131 = load i32* %SF1		; <i32> [#uses=1]
	%tmp132 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv133 = sext i16 %tmp132 to i32		; <i32> [#uses=1]
	%mul134 = mul i32 %tmp131, %conv133		; <i32> [#uses=1]
	store i32 %mul134, i32* %SF1
	%tmp135 = load i32* %SF1		; <i32> [#uses=1]
	%tmp136 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv137 = sext i16 %tmp136 to i32		; <i32> [#uses=1]
	%add138 = add i32 %tmp135, %conv137		; <i32> [#uses=1]
	store i32 %add138, i32* %SF1
	%tmp139 = load i32* %SF1		; <i32> [#uses=1]
	%shr140 = ashr i32 %tmp139, 5		; <i32> [#uses=1]
	store i32 %shr140, i32* %SF1
	%tmp141 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp142 = extractelement <4 x i8> %tmp141, i32 2		; <i8> [#uses=1]
	%conv143 = zext i8 %tmp142 to i32		; <i32> [#uses=1]
	%tmp144 = load i32* %SF1		; <i32> [#uses=1]
	%add145 = add i32 %conv143, %tmp144		; <i32> [#uses=1]
	store i32 %add145, i32* %SF1
	%tmp146 = load i16* %u16		; <i16> [#uses=1]
	%conv147 = sext i16 %tmp146 to i32		; <i32> [#uses=1]
	%tmp148 = load i16 addrspace(2)* @alpha_mask_SF_16		; <i16> [#uses=1]
	%conv149 = sext i16 %tmp148 to i32		; <i32> [#uses=1]
	%and150 = and i32 %conv147, %conv149		; <i32> [#uses=1]
	%conv151 = trunc i32 %and150 to i16		; <i16> [#uses=1]
	store i16 %conv151, i16* %uv_tmp_16
	%tmp152 = load i32* %SF1		; <i32> [#uses=1]
	%tmp153 = load i32* %SF0		; <i32> [#uses=1]
	%sub154 = sub i32 %tmp152, %tmp153		; <i32> [#uses=1]
	store i32 %sub154, i32* %SF1
	%tmp155 = load i32* %SF1		; <i32> [#uses=1]
	%tmp156 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv157 = sext i16 %tmp156 to i32		; <i32> [#uses=1]
	%mul158 = mul i32 %tmp155, %conv157		; <i32> [#uses=1]
	store i32 %mul158, i32* %SF1
	%tmp159 = load i32* %SF1		; <i32> [#uses=1]
	%tmp160 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv161 = sext i16 %tmp160 to i32		; <i32> [#uses=1]
	%add162 = add i32 %tmp159, %conv161		; <i32> [#uses=1]
	store i32 %add162, i32* %SF1
	%tmp163 = load i32* %SF1		; <i32> [#uses=1]
	%shr164 = ashr i32 %tmp163, 5		; <i32> [#uses=1]
	store i32 %shr164, i32* %SF1
	%tmp165 = load i32* %SF1		; <i32> [#uses=1]
	%tmp166 = load i32* %SF0		; <i32> [#uses=1]
	%add167 = add i32 %tmp165, %tmp166		; <i32> [#uses=1]
	store i32 %add167, i32* %SF0
	%tmp169 = load i32* %SF0		; <i32> [#uses=1]
	%tmp170 = load i16* %u16		; <i16> [#uses=1]
	%conv171 = sext i16 %tmp170 to i32		; <i32> [#uses=1]
	%mul172 = mul i32 %tmp169, %conv171		; <i32> [#uses=1]
	store i32 %mul172, i32* %uvl
	%tmp173 = load i32* %uvl		; <i32> [#uses=1]
	%tmp174 = load i32 addrspace(2)* @uv_final_rounding_32		; <i32> [#uses=1]
	%add175 = add i32 %tmp173, %tmp174		; <i32> [#uses=1]
	store i32 %add175, i32* %uvl
	%tmp176 = load i32* %uvl		; <i32> [#uses=1]
	%shr177 = ashr i32 %tmp176, 7		; <i32> [#uses=1]
	store i32 %shr177, i32* %uvl
	%tmp178 = load i32* %uvl		; <i32> [#uses=1]
	%conv179 = trunc i32 %tmp178 to i16		; <i16> [#uses=1]
	%tmp180 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%call181 = call signext i16 @__max_1i16(i16 signext %conv179, i16 signext %tmp180)		; <i16> [#uses=1]
	%tmp182 = load i16 addrspace(2)* @uv_max_val_16		; <i16> [#uses=1]
	%call183 = call signext i16 @__min_1i16(i16 signext %call181, i16 signext %tmp182)		; <i16> [#uses=1]
	store i16 %call183, i16* %u16
	%tmp184 = load i16* %u16		; <i16> [#uses=1]
	%conv185 = sext i16 %tmp184 to i32		; <i32> [#uses=1]
	%tmp186 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv187 = sext i16 %tmp186 to i32		; <i32> [#uses=1]
	%sub188 = sub i32 %conv185, %conv187		; <i32> [#uses=1]
	%conv189 = trunc i32 %sub188 to i16		; <i16> [#uses=1]
	store i16 %conv189, i16* %u16
	%tmp190 = load i32* %SF0		; <i32> [#uses=1]
	%tmp191 = load i16* %v16		; <i16> [#uses=1]
	%conv192 = sext i16 %tmp191 to i32		; <i32> [#uses=1]
	%mul193 = mul i32 %tmp190, %conv192		; <i32> [#uses=1]
	store i32 %mul193, i32* %uvl
	%tmp194 = load i32* %uvl		; <i32> [#uses=1]
	%tmp195 = load i32 addrspace(2)* @uv_final_rounding_32		; <i32> [#uses=1]
	%add196 = add i32 %tmp194, %tmp195		; <i32> [#uses=1]
	store i32 %add196, i32* %uvl
	%tmp197 = load i32* %uvl		; <i32> [#uses=1]
	%shr198 = ashr i32 %tmp197, 7		; <i32> [#uses=1]
	store i32 %shr198, i32* %uvl
	%tmp199 = load i32* %uvl		; <i32> [#uses=1]
	%conv200 = trunc i32 %tmp199 to i16		; <i16> [#uses=1]
	%tmp201 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%call202 = call signext i16 @__max_1i16(i16 signext %conv200, i16 signext %tmp201)		; <i16> [#uses=1]
	%tmp203 = load i16 addrspace(2)* @uv_max_val_16		; <i16> [#uses=1]
	%call204 = call signext i16 @__min_1i16(i16 signext %call202, i16 signext %tmp203)		; <i16> [#uses=1]
	store i16 %call204, i16* %v16
	%tmp205 = load i16* %v16		; <i16> [#uses=1]
	%conv206 = sext i16 %tmp205 to i32		; <i32> [#uses=1]
	%tmp207 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv208 = sext i16 %tmp207 to i32		; <i32> [#uses=1]
	%sub209 = sub i32 %conv206, %conv208		; <i32> [#uses=1]
	%conv210 = trunc i32 %sub209 to i16		; <i16> [#uses=1]
	store i16 %conv210, i16* %v16
	%arraydecay = getelementptr [2 x i32]* %uOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx211 = getelementptr i32* %arraydecay, i32 0		; <i32*> [#uses=1]
	%tmp212 = load i16* %u16		; <i16> [#uses=1]
	%conv213 = sext i16 %tmp212 to i32		; <i32> [#uses=1]
	store i32 %conv213, i32* %arrayidx211
	%arraydecay214 = getelementptr [2 x i32]* %vOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx215 = getelementptr i32* %arraydecay214, i32 0		; <i32*> [#uses=1]
	%tmp216 = load i16* %v16		; <i16> [#uses=1]
	%conv217 = sext i16 %tmp216 to i32		; <i32> [#uses=1]
	store i32 %conv217, i32* %arrayidx215
	%tmp218 = load i32* %pixel_index		; <i32> [#uses=1]
	%inc = add i32 %tmp218, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %pixel_index
	%tmp219 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp220 = load i16 addrspace(1)** %Uin.addr		; <i16 addrspace(1)*> [#uses=1]
	%arrayidx221 = getelementptr i16 addrspace(1)* %tmp220, i32 %tmp219		; <i16 addrspace(1)*> [#uses=1]
	%tmp222 = load i16 addrspace(1)* %arrayidx221		; <i16> [#uses=1]
	%conv223 = zext i16 %tmp222 to i32		; <i32> [#uses=1]
	%conv224 = trunc i32 %conv223 to i16		; <i16> [#uses=1]
	%conv225 = sext i16 %conv224 to i32		; <i32> [#uses=1]
	%tmp226 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv227 = sext i16 %tmp226 to i32		; <i32> [#uses=1]
	%add228 = add i32 %conv225, %conv227		; <i32> [#uses=1]
	%conv229 = trunc i32 %add228 to i16		; <i16> [#uses=1]
	store i16 %conv229, i16* %u16
	%tmp230 = load i32* %pixel_index		; <i32> [#uses=1]
	%tmp231 = load i16 addrspace(1)** %Vin.addr		; <i16 addrspace(1)*> [#uses=1]
	%arrayidx232 = getelementptr i16 addrspace(1)* %tmp231, i32 %tmp230		; <i16 addrspace(1)*> [#uses=1]
	%tmp233 = load i16 addrspace(1)* %arrayidx232		; <i16> [#uses=1]
	%conv234 = zext i16 %tmp233 to i32		; <i32> [#uses=1]
	%conv235 = trunc i32 %conv234 to i16		; <i16> [#uses=1]
	%conv236 = sext i16 %conv235 to i32		; <i32> [#uses=1]
	%tmp237 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv238 = sext i16 %tmp237 to i32		; <i32> [#uses=1]
	%add239 = add i32 %conv236, %conv238		; <i32> [#uses=1]
	%conv240 = trunc i32 %add239 to i16		; <i16> [#uses=1]
	store i16 %conv240, i16* %v16
	%tmp241 = load i16* %u16		; <i16> [#uses=1]
	%conv242 = sext i16 %tmp241 to i32		; <i32> [#uses=1]
	%shr243 = ashr i32 %conv242, 5		; <i32> [#uses=1]
	%conv244 = trunc i32 %shr243 to i16		; <i16> [#uses=1]
	store i16 %conv244, i16* %u_key_16
	%tmp247 = load i16* %v16		; <i16> [#uses=1]
	%conv248 = sext i16 %tmp247 to i32		; <i32> [#uses=1]
	%tmp249 = load i16 addrspace(2)* @hi_key_mask_16		; <i16> [#uses=1]
	%conv250 = sext i16 %tmp249 to i32		; <i32> [#uses=1]
	%and251 = and i32 %conv248, %conv250		; <i32> [#uses=1]
	%conv252 = trunc i32 %and251 to i16		; <i16> [#uses=1]
	store i16 %conv252, i16* %v_key_16246
	%tmp253 = load i16* %u_key_16		; <i16> [#uses=1]
	%conv254 = sext i16 %tmp253 to i32		; <i32> [#uses=1]
	%tmp255 = load i16* %v_key_16246		; <i16> [#uses=1]
	%conv256 = sext i16 %tmp255 to i32		; <i32> [#uses=1]
	%add257 = add i32 %conv254, %conv256		; <i32> [#uses=1]
	%conv258 = trunc i32 %add257 to i16		; <i16> [#uses=1]
	store i16 %conv258, i16* %u_key_16
	%tmp259 = load i16* %v16		; <i16> [#uses=1]
	%conv260 = sext i16 %tmp259 to i32		; <i32> [#uses=1]
	%tmp261 = load i16 addrspace(2)* @alpha_mask_SF_16		; <i16> [#uses=1]
	%conv262 = sext i16 %tmp261 to i32		; <i32> [#uses=1]
	%and263 = and i32 %conv260, %conv262		; <i32> [#uses=1]
	%conv264 = trunc i32 %and263 to i16		; <i16> [#uses=1]
	store i16 %conv264, i16* %uv_tmp_16
	%tmp265 = load i16* %u_key_16		; <i16> [#uses=1]
	%conv266 = sext i16 %tmp265 to i32		; <i32> [#uses=1]
	%add267 = add i32 %conv266, 528		; <i32> [#uses=1]
	%conv268 = trunc i32 %add267 to i16		; <i16> [#uses=1]
	store i16 %conv268, i16* %index
	%tmp269 = load i16* %index		; <i16> [#uses=1]
	%tmp270 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%idxprom271 = sext i16 %tmp269 to i32		; <i32> [#uses=1]
	%arrayidx272 = getelementptr i8 addrspace(2)* %tmp270, i32 %idxprom271		; <i8 addrspace(2)*> [#uses=1]
	%tmp273 = load i8 addrspace(2)* %arrayidx272		; <i8> [#uses=1]
	%tmp274 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp275 = insertelement <4 x i8> %tmp274, i8 %tmp273, i32 0		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp275, <4 x i8>* %sat_data
	%tmp276 = load i16* %index		; <i16> [#uses=1]
	%conv277 = sext i16 %tmp276 to i32		; <i32> [#uses=1]
	%add278 = add i32 %conv277, 1		; <i32> [#uses=1]
	%tmp279 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx280 = getelementptr i8 addrspace(2)* %tmp279, i32 %add278		; <i8 addrspace(2)*> [#uses=1]
	%tmp281 = load i8 addrspace(2)* %arrayidx280		; <i8> [#uses=1]
	%tmp282 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp283 = insertelement <4 x i8> %tmp282, i8 %tmp281, i32 1		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp283, <4 x i8>* %sat_data
	%tmp284 = load i16* %index		; <i16> [#uses=1]
	%conv285 = sext i16 %tmp284 to i32		; <i32> [#uses=1]
	%add286 = add i32 %conv285, 2		; <i32> [#uses=1]
	%tmp287 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx288 = getelementptr i8 addrspace(2)* %tmp287, i32 %add286		; <i8 addrspace(2)*> [#uses=1]
	%tmp289 = load i8 addrspace(2)* %arrayidx288		; <i8> [#uses=1]
	%tmp290 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp291 = insertelement <4 x i8> %tmp290, i8 %tmp289, i32 2		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp291, <4 x i8>* %sat_data
	%tmp292 = load i16* %index		; <i16> [#uses=1]
	%conv293 = sext i16 %tmp292 to i32		; <i32> [#uses=1]
	%add294 = add i32 %conv293, 3		; <i32> [#uses=1]
	%tmp295 = load i8 addrspace(2)** %SatLUTEntry.addr		; <i8 addrspace(2)*> [#uses=1]
	%arrayidx296 = getelementptr i8 addrspace(2)* %tmp295, i32 %add294		; <i8 addrspace(2)*> [#uses=1]
	%tmp297 = load i8 addrspace(2)* %arrayidx296		; <i8> [#uses=1]
	%tmp298 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp299 = insertelement <4 x i8> %tmp298, i8 %tmp297, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp299, <4 x i8>* %sat_data
	%tmp300 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp301 = extractelement <4 x i8> %tmp300, i32 0		; <i8> [#uses=1]
	%conv302 = zext i8 %tmp301 to i32		; <i32> [#uses=1]
	%tmp303 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp304 = extractelement <4 x i8> %tmp303, i32 1		; <i8> [#uses=1]
	%conv305 = zext i8 %tmp304 to i32		; <i32> [#uses=1]
	%sub306 = sub i32 %conv302, %conv305		; <i32> [#uses=1]
	store i32 %sub306, i32* %SF0
	%tmp307 = load i32* %SF0		; <i32> [#uses=1]
	%tmp308 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv309 = sext i16 %tmp308 to i32		; <i32> [#uses=1]
	%mul310 = mul i32 %tmp307, %conv309		; <i32> [#uses=1]
	store i32 %mul310, i32* %SF0
	%tmp311 = load i32* %SF0		; <i32> [#uses=1]
	%tmp312 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv313 = sext i16 %tmp312 to i32		; <i32> [#uses=1]
	%add314 = add i32 %tmp311, %conv313		; <i32> [#uses=1]
	store i32 %add314, i32* %SF0
	%tmp315 = load i32* %SF0		; <i32> [#uses=1]
	%shr316 = ashr i32 %tmp315, 5		; <i32> [#uses=1]
	store i32 %shr316, i32* %SF0
	%tmp317 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp318 = extractelement <4 x i8> %tmp317, i32 0		; <i8> [#uses=1]
	%conv319 = zext i8 %tmp318 to i32		; <i32> [#uses=1]
	%tmp320 = load i32* %SF0		; <i32> [#uses=1]
	%add321 = add i32 %conv319, %tmp320		; <i32> [#uses=1]
	store i32 %add321, i32* %SF0
	%tmp322 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp323 = extractelement <4 x i8> %tmp322, i32 2		; <i8> [#uses=1]
	%conv324 = zext i8 %tmp323 to i32		; <i32> [#uses=1]
	%tmp325 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp326 = extractelement <4 x i8> %tmp325, i32 3		; <i8> [#uses=1]
	%conv327 = zext i8 %tmp326 to i32		; <i32> [#uses=1]
	%sub328 = sub i32 %conv324, %conv327		; <i32> [#uses=1]
	store i32 %sub328, i32* %SF1
	%tmp329 = load i32* %SF1		; <i32> [#uses=1]
	%tmp330 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv331 = sext i16 %tmp330 to i32		; <i32> [#uses=1]
	%mul332 = mul i32 %tmp329, %conv331		; <i32> [#uses=1]
	store i32 %mul332, i32* %SF1
	%tmp333 = load i32* %SF1		; <i32> [#uses=1]
	%tmp334 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv335 = sext i16 %tmp334 to i32		; <i32> [#uses=1]
	%add336 = add i32 %tmp333, %conv335		; <i32> [#uses=1]
	store i32 %add336, i32* %SF1
	%tmp337 = load i32* %SF1		; <i32> [#uses=1]
	%shr338 = ashr i32 %tmp337, 5		; <i32> [#uses=1]
	store i32 %shr338, i32* %SF1
	%tmp339 = load <4 x i8>* %sat_data		; <<4 x i8>> [#uses=1]
	%tmp340 = extractelement <4 x i8> %tmp339, i32 2		; <i8> [#uses=1]
	%conv341 = zext i8 %tmp340 to i32		; <i32> [#uses=1]
	%tmp342 = load i32* %SF1		; <i32> [#uses=1]
	%add343 = add i32 %conv341, %tmp342		; <i32> [#uses=1]
	store i32 %add343, i32* %SF1
	%tmp344 = load i16* %u16		; <i16> [#uses=1]
	%conv345 = sext i16 %tmp344 to i32		; <i32> [#uses=1]
	%tmp346 = load i16 addrspace(2)* @alpha_mask_SF_16		; <i16> [#uses=1]
	%conv347 = sext i16 %tmp346 to i32		; <i32> [#uses=1]
	%and348 = and i32 %conv345, %conv347		; <i32> [#uses=1]
	%conv349 = trunc i32 %and348 to i16		; <i16> [#uses=1]
	store i16 %conv349, i16* %uv_tmp_16
	%tmp350 = load i32* %SF1		; <i32> [#uses=1]
	%tmp351 = load i32* %SF0		; <i32> [#uses=1]
	%sub352 = sub i32 %tmp350, %tmp351		; <i32> [#uses=1]
	store i32 %sub352, i32* %SF1
	%tmp353 = load i32* %SF1		; <i32> [#uses=1]
	%tmp354 = load i16* %uv_tmp_16		; <i16> [#uses=1]
	%conv355 = sext i16 %tmp354 to i32		; <i32> [#uses=1]
	%mul356 = mul i32 %tmp353, %conv355		; <i32> [#uses=1]
	store i32 %mul356, i32* %SF1
	%tmp357 = load i32* %SF1		; <i32> [#uses=1]
	%tmp358 = load i16 addrspace(2)* @rounding_SF_16		; <i16> [#uses=1]
	%conv359 = sext i16 %tmp358 to i32		; <i32> [#uses=1]
	%add360 = add i32 %tmp357, %conv359		; <i32> [#uses=1]
	store i32 %add360, i32* %SF1
	%tmp361 = load i32* %SF1		; <i32> [#uses=1]
	%shr362 = ashr i32 %tmp361, 5		; <i32> [#uses=1]
	store i32 %shr362, i32* %SF1
	%tmp363 = load i32* %SF1		; <i32> [#uses=1]
	%tmp364 = load i32* %SF0		; <i32> [#uses=1]
	%add365 = add i32 %tmp363, %tmp364		; <i32> [#uses=1]
	store i32 %add365, i32* %SF0
	%tmp366 = load i32* %SF0		; <i32> [#uses=1]
	%tmp367 = load i16* %u16		; <i16> [#uses=1]
	%conv368 = sext i16 %tmp367 to i32		; <i32> [#uses=1]
	%mul369 = mul i32 %tmp366, %conv368		; <i32> [#uses=1]
	store i32 %mul369, i32* %uvl
	%tmp370 = load i32* %uvl		; <i32> [#uses=1]
	%tmp371 = load i32 addrspace(2)* @uv_final_rounding_32		; <i32> [#uses=1]
	%add372 = add i32 %tmp370, %tmp371		; <i32> [#uses=1]
	store i32 %add372, i32* %uvl
	%tmp373 = load i32* %uvl		; <i32> [#uses=1]
	%shr374 = ashr i32 %tmp373, 7		; <i32> [#uses=1]
	store i32 %shr374, i32* %uvl
	%tmp375 = load i32* %uvl		; <i32> [#uses=1]
	%conv376 = trunc i32 %tmp375 to i16		; <i16> [#uses=1]
	%tmp377 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%call378 = call signext i16 @__max_1i16(i16 signext %conv376, i16 signext %tmp377)		; <i16> [#uses=1]
	%tmp379 = load i16 addrspace(2)* @uv_max_val_16		; <i16> [#uses=1]
	%call380 = call signext i16 @__min_1i16(i16 signext %call378, i16 signext %tmp379)		; <i16> [#uses=1]
	store i16 %call380, i16* %u16
	%tmp381 = load i16* %u16		; <i16> [#uses=1]
	%conv382 = sext i16 %tmp381 to i32		; <i32> [#uses=1]
	%tmp383 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv384 = sext i16 %tmp383 to i32		; <i32> [#uses=1]
	%sub385 = sub i32 %conv382, %conv384		; <i32> [#uses=1]
	%conv386 = trunc i32 %sub385 to i16		; <i16> [#uses=1]
	store i16 %conv386, i16* %u16
	%tmp387 = load i32* %SF0		; <i32> [#uses=1]
	%tmp388 = load i16* %v16		; <i16> [#uses=1]
	%conv389 = sext i16 %tmp388 to i32		; <i32> [#uses=1]
	%mul390 = mul i32 %tmp387, %conv389		; <i32> [#uses=1]
	store i32 %mul390, i32* %uvl
	%tmp391 = load i32* %uvl		; <i32> [#uses=1]
	%tmp392 = load i32 addrspace(2)* @uv_final_rounding_32		; <i32> [#uses=1]
	%add393 = add i32 %tmp391, %tmp392		; <i32> [#uses=1]
	store i32 %add393, i32* %uvl
	%tmp394 = load i32* %uvl		; <i32> [#uses=1]
	%shr395 = ashr i32 %tmp394, 7		; <i32> [#uses=1]
	store i32 %shr395, i32* %uvl
	%tmp396 = load i32* %uvl		; <i32> [#uses=1]
	%conv397 = trunc i32 %tmp396 to i16		; <i16> [#uses=1]
	%tmp398 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%call399 = call signext i16 @__max_1i16(i16 signext %conv397, i16 signext %tmp398)		; <i16> [#uses=1]
	%tmp400 = load i16 addrspace(2)* @uv_max_val_16		; <i16> [#uses=1]
	%call401 = call signext i16 @__min_1i16(i16 signext %call399, i16 signext %tmp400)		; <i16> [#uses=1]
	store i16 %call401, i16* %v16
	%tmp402 = load i16* %v16		; <i16> [#uses=1]
	%conv403 = sext i16 %tmp402 to i32		; <i32> [#uses=1]
	%tmp404 = load i16 addrspace(2)* @uv_min_val_16		; <i16> [#uses=1]
	%conv405 = sext i16 %tmp404 to i32		; <i32> [#uses=1]
	%sub406 = sub i32 %conv403, %conv405		; <i32> [#uses=1]
	%conv407 = trunc i32 %sub406 to i16		; <i16> [#uses=1]
	store i16 %conv407, i16* %v16
	%arraydecay408 = getelementptr [2 x i32]* %uOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx409 = getelementptr i32* %arraydecay408, i32 1		; <i32*> [#uses=1]
	%tmp410 = load i16* %u16		; <i16> [#uses=1]
	%conv411 = sext i16 %tmp410 to i32		; <i32> [#uses=1]
	store i32 %conv411, i32* %arrayidx409
	%arraydecay412 = getelementptr [2 x i32]* %vOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx413 = getelementptr i32* %arraydecay412, i32 1		; <i32*> [#uses=1]
	%tmp414 = load i16* %v16		; <i16> [#uses=1]
	%conv415 = sext i16 %tmp414 to i32		; <i32> [#uses=1]
	store i32 %conv415, i32* %arrayidx413
	%tmp416 = load i32 addrspace(1)** %UoutPtr		; <i32 addrspace(1)*> [#uses=1]
	%arraydecay417 = getelementptr [2 x i32]* %uOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx418 = getelementptr i32* %arraydecay417, i32 0		; <i32*> [#uses=1]
	%tmp419 = load i32* %arrayidx418		; <i32> [#uses=1]
	%and420 = and i32 %tmp419, 65535		; <i32> [#uses=1]
	%shl = shl i32 %and420, 16		; <i32> [#uses=1]
	%arraydecay421 = getelementptr [2 x i32]* %uOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx422 = getelementptr i32* %arraydecay421, i32 1		; <i32*> [#uses=1]
	%tmp423 = load i32* %arrayidx422		; <i32> [#uses=1]
	%and424 = and i32 %tmp423, 65535		; <i32> [#uses=1]
	%or = or i32 %shl, %and424		; <i32> [#uses=1]
	store i32 %or, i32 addrspace(1)* %tmp416
	%tmp425 = load i32 addrspace(1)** %VoutPtr		; <i32 addrspace(1)*> [#uses=1]
	%arraydecay426 = getelementptr [2 x i32]* %vOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx427 = getelementptr i32* %arraydecay426, i32 0		; <i32*> [#uses=1]
	%tmp428 = load i32* %arrayidx427		; <i32> [#uses=1]
	%and429 = and i32 %tmp428, 65535		; <i32> [#uses=1]
	%shl430 = shl i32 %and429, 16		; <i32> [#uses=1]
	%arraydecay431 = getelementptr [2 x i32]* %vOut, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx432 = getelementptr i32* %arraydecay431, i32 1		; <i32*> [#uses=1]
	%tmp433 = load i32* %arrayidx432		; <i32> [#uses=1]
	%and434 = and i32 %tmp433, 65535		; <i32> [#uses=1]
	%or435 = or i32 %shl430, %and434		; <i32> [#uses=1]
	store i32 %or435, i32 addrspace(1)* %tmp425
	%tmp436 = load i32* %pixel_index		; <i32> [#uses=1]
	%inc437 = add i32 %tmp436, 1		; <i32> [#uses=1]
	store i32 %inc437, i32* %pixel_index
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp438 = load i32* %iter		; <i32> [#uses=1]
	%inc439 = add i32 %tmp438, 1		; <i32> [#uses=1]
	store i32 %inc439, i32* %iter
	br label %for.cond

for.end:		; preds = %for.cond
	ret void
}

declare signext i16 @__min_1i16(i16 signext, i16 signext)

declare signext i16 @__max_1i16(i16 signext, i16 signext)
