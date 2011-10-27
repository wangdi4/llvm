; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\tcc.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@uv_min_val_16 = addrspace(2) global i16 -512, align 2 ; <i16 addrspace(2)*> [#uses=15]
@uv_max_val_16 = addrspace(2) global i16 511, align 2 ; <i16 addrspace(2)*> [#uses=5]
@hi_key_mask_16 = addrspace(2) global i16 -32, align 2 ; <i16 addrspace(2)*> [#uses=2]
@alpha_mask_SF_16 = addrspace(2) global i16 31, align 2 ; <i16 addrspace(2)*> [#uses=4]
@rounding_SF_16 = addrspace(2) global i16 16, align 2 ; <i16 addrspace(2)*> [#uses=6]
@uv_final_rounding_32 = addrspace(2) global i32 64, align 4 ; <i32 addrspace(2)*> [#uses=4]
@uv_min_val_8_s16 = addrspace(2) global <8 x i16> <i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512, i16 -512>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=4]
@uv_max_val_8_s16 = addrspace(2) global <8 x i16> <i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511, i16 511>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=2]
@hi_key_mask_8_s16 = addrspace(2) global <8 x i16> <i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32, i16 -32>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=1]
@alpha_mask_SF_8_s16 = addrspace(2) global <8 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=2]
@rounding_SF_8_s16 = addrspace(2) global <8 x i16> <i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=3]
@precision_8_s16 = addrspace(2) global <8 x i16> <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_8_s16 = addrspace(2) global <8 x i16> zeroinitializer, align 16 ; <<8 x i16> addrspace(2)*> [#uses=2]
@u_key_16_mask_8_s16 = addrspace(2) global <8 x i16> <i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528, i16 528>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=1]
@ff_const_8_u16 = addrspace(2) global <8 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_8_u16 = addrspace(2) global <8 x i16> zeroinitializer, align 16 ; <<8 x i16> addrspace(2)*> [#uses=0]
@eight_const_8_u16 = addrspace(2) global <8 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 16 ; <<8 x i16> addrspace(2)*> [#uses=0]
@zero_const_16_s16 = addrspace(2) global <16 x i16> zeroinitializer, align 32 ; <<16 x i16> addrspace(2)*> [#uses=0]
@ff_const_16_u16 = addrspace(2) global <16 x i16> <i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255, i16 255>, align 32 ; <<16 x i16> addrspace(2)*> [#uses=1]
@zero_const_16_u16 = addrspace(2) global <16 x i16> zeroinitializer, align 32 ; <<16 x i16> addrspace(2)*> [#uses=0]
@eight_const_16_u16 = addrspace(2) global <16 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, align 32 ; <<16 x i16> addrspace(2)*> [#uses=0]
@sat_bit_minus_one_4_s32 = addrspace(2) global <4 x i32> <i32 7, i32 7, i32 7, i32 7>, align 16 ; <<4 x i32> addrspace(2)*> [#uses=0]
@uv_final_rounding_4_s32 = addrspace(2) global <4 x i32> <i32 64, i32 64, i32 64, i32 64>, align 16 ; <<4 x i32> addrspace(2)*> [#uses=0]
@zero_const_4_u32 = addrspace(2) global <4 x i32> zeroinitializer, align 16 ; <<4 x i32> addrspace(2)*> [#uses=0]
@ff00ff_const_4_u32 = addrspace(2) global <4 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 16 ; <<4 x i32> addrspace(2)*> [#uses=0]
@sat_bit_minus_one_8_s32 = addrspace(2) global <8 x i32> <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>, align 32 ; <<8 x i32> addrspace(2)*> [#uses=0]
@uv_final_rounding_8_s32 = addrspace(2) global <8 x i32> <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>, align 32 ; <<8 x i32> addrspace(2)*> [#uses=2]
@zero_const_8_u32 = addrspace(2) global <8 x i32> zeroinitializer, align 32 ; <<8 x i32> addrspace(2)*> [#uses=1]
@ff00ff_const_8_u32 = addrspace(2) global <8 x i32> <i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935, i32 16711935>, align 32 ; <<8 x i32> addrspace(2)*> [#uses=0]
@opencl_tcc_vector8_third_optimization_vec_type_hint = internal global <4 x i32> zeroinitializer ; <<4 x i32>*> [#uses=1]
@opencl_tcc_vector8_third_optimization_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_tcc_vector8_third_optimization_parameters = appending global [329 x i8] c"short8 const __attribute__((address_space(1))) *, short8 const __attribute__((address_space(1))) *, ushort8 __attribute__((address_space(1))) *, ushort8 __attribute__((address_space(1))) *, size_t const __attribute__((address_space(1))), uchar const __attribute__((address_space(2))) *, ulong __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[329 x i8]*> [#uses=1]
@opencl_tcc_scalar_unroll2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_tcc_scalar_unroll2_parameters = appending global [250 x i8] c"ushort const __attribute__((address_space(1))) *, ushort const __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *, unsigned int, uchar const __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[250 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, <8 x i16> addrspace(1)*, i32, i8 addrspace(2)*, i64 addrspace(1)*)* @tcc_vector8_third_optimization to i8*), i8* bitcast (<4 x i32>* @opencl_tcc_vector8_third_optimization_vec_type_hint to i8*), [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_tcc_vector8_third_optimization_locals to i8*), i8* getelementptr inbounds ([329 x i8]* @opencl_tcc_vector8_third_optimization_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i16 addrspace(1)*, i16 addrspace(1)*, i16 addrspace(1)*, i16 addrspace(1)*, i32, i8 addrspace(2)*)* @tcc_scalar_unroll2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_tcc_scalar_unroll2_locals to i8*), i8* getelementptr inbounds ([250 x i8]* @opencl_tcc_scalar_unroll2_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @tcc_vector8_third_optimization(<8 x i16> addrspace(1)* %Uin, <8 x i16> addrspace(1)* %Vin, <8 x i16> addrspace(1)* %Uout, <8 x i16> addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry, i64 addrspace(1)* %timeStamps) nounwind {
entry:
  %Uin.addr = alloca <8 x i16> addrspace(1)*, align 4 ; <<8 x i16> addrspace(1)**> [#uses=2]
  %Vin.addr = alloca <8 x i16> addrspace(1)*, align 4 ; <<8 x i16> addrspace(1)**> [#uses=2]
  %Uout.addr = alloca <8 x i16> addrspace(1)*, align 4 ; <<8 x i16> addrspace(1)**> [#uses=2]
  %Vout.addr = alloca <8 x i16> addrspace(1)*, align 4 ; <<8 x i16> addrspace(1)**> [#uses=2]
  %bufferSize.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %SatLUTEntry.addr = alloca i8 addrspace(2)*, align 4 ; <i8 addrspace(2)**> [#uses=9]
  %timeStamps.addr = alloca i64 addrspace(1)*, align 4 ; <i64 addrspace(1)**> [#uses=1]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=2]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=2]
  %numOfPixelsToProcess = alloca i32, align 4     ; <i32*> [#uses=3]
  %pixel_index = alloca i32, align 4              ; <i32*> [#uses=7]
  %iter = alloca i32, align 4                     ; <i32*> [#uses=4]
  %u16 = alloca <8 x i16>, align 16               ; <<8 x i16>*> [#uses=11]
  %v16 = alloca <8 x i16>, align 16               ; <<8 x i16>*> [#uses=11]
  %u_key_16 = alloca <8 x i16>, align 16          ; <<8 x i16>*> [#uses=15]
  %v_key_16 = alloca <8 x i16>, align 16          ; <<8 x i16>*> [#uses=2]
  %uv_tmp_16 = alloca <8 x i16>, align 16         ; <<8 x i16>*> [#uses=5]
  %sat_data_uint8 = alloca <8 x i32>, align 32    ; <<8 x i32>*> [#uses=18]
  %sat_data = alloca <16 x i16>, align 32         ; <<16 x i16>*> [#uses=5]
  %sat_data_shifted_8 = alloca <16 x i16>, align 32 ; <<16 x i16>*> [#uses=2]
  %.compoundliteral = alloca <16 x i16>, align 32 ; <<16 x i16>*> [#uses=2]
  %delta_sat_data = alloca <16 x i16>, align 32   ; <<16 x i16>*> [#uses=3]
  %SF0 = alloca <8 x i16>, align 16               ; <<8 x i16>*> [#uses=15]
  %SF1 = alloca <8 x i16>, align 16               ; <<8 x i16>*> [#uses=17]
  %SF_original = alloca <8 x i16>, align 16       ; <<8 x i16>*> [#uses=6]
  %uvl_short16 = alloca <16 x i16>, align 32      ; <<16 x i16>*> [#uses=14]
  %uvl_int8 = alloca <8 x i32>, align 32          ; <<8 x i32>*> [#uses=12]
  %uvl_short8 = alloca <8 x i16>, align 16        ; <<8 x i16>*> [#uses=4]
  %uv_min_val_8_16 = alloca <8 x i16>, align 16   ; <<8 x i16>*> [#uses=1]
  %uv_max_val_8_16 = alloca <8 x i16>, align 16   ; <<8 x i16>*> [#uses=1]
  store <8 x i16> addrspace(1)* %Uin, <8 x i16> addrspace(1)** %Uin.addr
  store <8 x i16> addrspace(1)* %Vin, <8 x i16> addrspace(1)** %Vin.addr
  store <8 x i16> addrspace(1)* %Uout, <8 x i16> addrspace(1)** %Uout.addr
  store <8 x i16> addrspace(1)* %Vout, <8 x i16> addrspace(1)** %Vout.addr
  store i32 %bufferSize, i32* %bufferSize.addr
  store i8 addrspace(2)* %SatLUTEntry, i8 addrspace(2)** %SatLUTEntry.addr
  store i64 addrspace(1)* %timeStamps, i64 addrspace(1)** %timeStamps.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalId
  %call1 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call1, i32* %globalSize
  %tmp = load i32* %bufferSize.addr               ; <i32> [#uses=1]
  %tmp2 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %numOfPixelsToProcess
  %tmp4 = load i32* %globalId                     ; <i32> [#uses=1]
  %tmp5 = load i32* %numOfPixelsToProcess         ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  %div6 = udiv i32 %mul, 8                        ; <i32> [#uses=1]
  store i32 %div6, i32* %pixel_index
  store i32 0, i32* %iter
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp8 = load i32* %iter                         ; <i32> [#uses=1]
  %tmp9 = load i32* %numOfPixelsToProcess         ; <i32> [#uses=1]
  %cmp10 = icmp ult i32 %tmp8, %tmp9              ; <i1> [#uses=1]
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp12 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %tmp13 = load <8 x i16> addrspace(1)** %Uin.addr ; <<8 x i16> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x i16> addrspace(1)* %tmp13, i32 %tmp12 ; <<8 x i16> addrspace(1)*> [#uses=1]
  %tmp14 = load <8 x i16> addrspace(1)* %arrayidx ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp14, <8 x i16>* %u16
  %tmp16 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %tmp17 = load <8 x i16> addrspace(1)** %Vin.addr ; <<8 x i16> addrspace(1)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds <8 x i16> addrspace(1)* %tmp17, i32 %tmp16 ; <<8 x i16> addrspace(1)*> [#uses=1]
  %tmp19 = load <8 x i16> addrspace(1)* %arrayidx18 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp19, <8 x i16>* %v16
  %tmp20 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16 ; <<8 x i16>> [#uses=1]
  %tmp21 = load <8 x i16>* %u16                   ; <<8 x i16>> [#uses=1]
  %add = add nsw <8 x i16> %tmp21, %tmp20         ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add, <8 x i16>* %u16
  %tmp22 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16 ; <<8 x i16>> [#uses=1]
  %tmp23 = load <8 x i16>* %v16                   ; <<8 x i16>> [#uses=1]
  %add24 = add nsw <8 x i16> %tmp23, %tmp22       ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add24, <8 x i16>* %v16
  %tmp26 = load <8 x i16>* %u16                   ; <<8 x i16>> [#uses=1]
  %shr = ashr <8 x i16> %tmp26, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %shr, <8 x i16>* %u_key_16
  %tmp28 = load <8 x i16>* %v16                   ; <<8 x i16>> [#uses=1]
  %tmp29 = load <8 x i16> addrspace(2)* @hi_key_mask_8_s16 ; <<8 x i16>> [#uses=1]
  %and = and <8 x i16> %tmp28, %tmp29             ; <<8 x i16>> [#uses=1]
  store <8 x i16> %and, <8 x i16>* %v_key_16
  %tmp30 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %tmp31 = load <8 x i16>* %v_key_16              ; <<8 x i16>> [#uses=1]
  %add32 = add nsw <8 x i16> %tmp30, %tmp31       ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add32, <8 x i16>* %u_key_16
  %tmp34 = load <8 x i16>* %v16                   ; <<8 x i16>> [#uses=1]
  %tmp35 = load <8 x i16> addrspace(2)* @alpha_mask_SF_8_s16 ; <<8 x i16>> [#uses=1]
  %and36 = and <8 x i16> %tmp34, %tmp35           ; <<8 x i16>> [#uses=1]
  store <8 x i16> %and36, <8 x i16>* %uv_tmp_16
  %tmp38 = load <8 x i32> addrspace(2)* @zero_const_8_u32 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %tmp38, <8 x i32>* %sat_data_uint8
  %tmp39 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %tmp40 = load <8 x i16> addrspace(2)* @u_key_16_mask_8_s16 ; <<8 x i16>> [#uses=1]
  %add41 = add nsw <8 x i16> %tmp39, %tmp40       ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add41, <8 x i16>* %u_key_16
  %tmp42 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %shl = shl <8 x i16> %tmp42, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %shl, <8 x i16>* %u_key_16
  %tmp43 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext = extractelement <8 x i16> %tmp43, i32 0 ; <i16> [#uses=1]
  %tmp44 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom = sext i16 %vecext to i32              ; <i32> [#uses=1]
  %arrayidx45 = getelementptr inbounds i8 addrspace(2)* %tmp44, i32 %idxprom ; <i8 addrspace(2)*> [#uses=1]
  %0 = bitcast i8 addrspace(2)* %arrayidx45 to i32* ; <i32*> [#uses=1]
  %tmp46 = load i32* %0                           ; <i32> [#uses=1]
  %tmp47 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins = insertelement <8 x i32> %tmp47, i32 %tmp46, i32 0 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins, <8 x i32>* %sat_data_uint8
  %tmp48 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext49 = extractelement <8 x i16> %tmp48, i32 1 ; <i16> [#uses=1]
  %tmp50 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom51 = sext i16 %vecext49 to i32          ; <i32> [#uses=1]
  %arrayidx52 = getelementptr inbounds i8 addrspace(2)* %tmp50, i32 %idxprom51 ; <i8 addrspace(2)*> [#uses=1]
  %1 = bitcast i8 addrspace(2)* %arrayidx52 to i32* ; <i32*> [#uses=1]
  %tmp53 = load i32* %1                           ; <i32> [#uses=1]
  %tmp54 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins55 = insertelement <8 x i32> %tmp54, i32 %tmp53, i32 1 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins55, <8 x i32>* %sat_data_uint8
  %tmp56 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext57 = extractelement <8 x i16> %tmp56, i32 2 ; <i16> [#uses=1]
  %tmp58 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom59 = sext i16 %vecext57 to i32          ; <i32> [#uses=1]
  %arrayidx60 = getelementptr inbounds i8 addrspace(2)* %tmp58, i32 %idxprom59 ; <i8 addrspace(2)*> [#uses=1]
  %2 = bitcast i8 addrspace(2)* %arrayidx60 to i32* ; <i32*> [#uses=1]
  %tmp61 = load i32* %2                           ; <i32> [#uses=1]
  %tmp62 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins63 = insertelement <8 x i32> %tmp62, i32 %tmp61, i32 2 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins63, <8 x i32>* %sat_data_uint8
  %tmp64 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext65 = extractelement <8 x i16> %tmp64, i32 3 ; <i16> [#uses=1]
  %tmp66 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom67 = sext i16 %vecext65 to i32          ; <i32> [#uses=1]
  %arrayidx68 = getelementptr inbounds i8 addrspace(2)* %tmp66, i32 %idxprom67 ; <i8 addrspace(2)*> [#uses=1]
  %3 = bitcast i8 addrspace(2)* %arrayidx68 to i32* ; <i32*> [#uses=1]
  %tmp69 = load i32* %3                           ; <i32> [#uses=1]
  %tmp70 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins71 = insertelement <8 x i32> %tmp70, i32 %tmp69, i32 3 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins71, <8 x i32>* %sat_data_uint8
  %tmp72 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext73 = extractelement <8 x i16> %tmp72, i32 4 ; <i16> [#uses=1]
  %tmp74 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom75 = sext i16 %vecext73 to i32          ; <i32> [#uses=1]
  %arrayidx76 = getelementptr inbounds i8 addrspace(2)* %tmp74, i32 %idxprom75 ; <i8 addrspace(2)*> [#uses=1]
  %4 = bitcast i8 addrspace(2)* %arrayidx76 to i32* ; <i32*> [#uses=1]
  %tmp77 = load i32* %4                           ; <i32> [#uses=1]
  %tmp78 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins79 = insertelement <8 x i32> %tmp78, i32 %tmp77, i32 4 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins79, <8 x i32>* %sat_data_uint8
  %tmp80 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext81 = extractelement <8 x i16> %tmp80, i32 5 ; <i16> [#uses=1]
  %tmp82 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom83 = sext i16 %vecext81 to i32          ; <i32> [#uses=1]
  %arrayidx84 = getelementptr inbounds i8 addrspace(2)* %tmp82, i32 %idxprom83 ; <i8 addrspace(2)*> [#uses=1]
  %5 = bitcast i8 addrspace(2)* %arrayidx84 to i32* ; <i32*> [#uses=1]
  %tmp85 = load i32* %5                           ; <i32> [#uses=1]
  %tmp86 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins87 = insertelement <8 x i32> %tmp86, i32 %tmp85, i32 5 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins87, <8 x i32>* %sat_data_uint8
  %tmp88 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext89 = extractelement <8 x i16> %tmp88, i32 6 ; <i16> [#uses=1]
  %tmp90 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom91 = sext i16 %vecext89 to i32          ; <i32> [#uses=1]
  %arrayidx92 = getelementptr inbounds i8 addrspace(2)* %tmp90, i32 %idxprom91 ; <i8 addrspace(2)*> [#uses=1]
  %6 = bitcast i8 addrspace(2)* %arrayidx92 to i32* ; <i32*> [#uses=1]
  %tmp93 = load i32* %6                           ; <i32> [#uses=1]
  %tmp94 = load <8 x i32>* %sat_data_uint8        ; <<8 x i32>> [#uses=1]
  %vecins95 = insertelement <8 x i32> %tmp94, i32 %tmp93, i32 6 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins95, <8 x i32>* %sat_data_uint8
  %tmp96 = load <8 x i16>* %u_key_16              ; <<8 x i16>> [#uses=1]
  %vecext97 = extractelement <8 x i16> %tmp96, i32 7 ; <i16> [#uses=1]
  %tmp98 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %idxprom99 = sext i16 %vecext97 to i32          ; <i32> [#uses=1]
  %arrayidx100 = getelementptr inbounds i8 addrspace(2)* %tmp98, i32 %idxprom99 ; <i8 addrspace(2)*> [#uses=1]
  %7 = bitcast i8 addrspace(2)* %arrayidx100 to i32* ; <i32*> [#uses=1]
  %tmp101 = load i32* %7                          ; <i32> [#uses=1]
  %tmp102 = load <8 x i32>* %sat_data_uint8       ; <<8 x i32>> [#uses=1]
  %vecins103 = insertelement <8 x i32> %tmp102, i32 %tmp101, i32 7 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecins103, <8 x i32>* %sat_data_uint8
  %tmp105 = load <8 x i32>* %sat_data_uint8       ; <<8 x i32>> [#uses=1]
  %as_typen = bitcast <8 x i32> %tmp105 to <16 x i16> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %as_typen, <16 x i16>* %sat_data
  %tmp107 = load <16 x i16>* %sat_data            ; <<16 x i16>> [#uses=1]
  store <16 x i16> <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>, <16 x i16>* %.compoundliteral
  %tmp108 = load <16 x i16>* %.compoundliteral    ; <<16 x i16>> [#uses=1]
  %shr109 = lshr <16 x i16> %tmp107, %tmp108      ; <<16 x i16>> [#uses=1]
  store <16 x i16> %shr109, <16 x i16>* %sat_data_shifted_8
  %tmp110 = load <16 x i16>* %sat_data            ; <<16 x i16>> [#uses=1]
  %tmp111 = load <16 x i16> addrspace(2)* @ff_const_16_u16 ; <<16 x i16>> [#uses=1]
  %and112 = and <16 x i16> %tmp110, %tmp111       ; <<16 x i16>> [#uses=1]
  store <16 x i16> %and112, <16 x i16>* %sat_data
  %tmp114 = load <16 x i16>* %sat_data            ; <<16 x i16>> [#uses=1]
  %tmp115 = load <16 x i16>* %sat_data_shifted_8  ; <<16 x i16>> [#uses=1]
  %sub = sub <16 x i16> %tmp114, %tmp115          ; <<16 x i16>> [#uses=1]
  store <16 x i16> %sub, <16 x i16>* %delta_sat_data
  %tmp117 = load <8 x i16> addrspace(2)* @zero_const_8_s16 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp117, <8 x i16>* %SF0
  %tmp119 = load <8 x i16> addrspace(2)* @zero_const_8_s16 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp119, <8 x i16>* %SF1
  %tmp121 = load <16 x i16>* %delta_sat_data      ; <<16 x i16>> [#uses=1]
  %tmp122 = shufflevector <16 x i16> %tmp121, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp122, <8 x i16>* %SF_original
  %tmp123 = load <8 x i16>* %SF_original          ; <<8 x i16>> [#uses=1]
  %tmp124 = load <8 x i16>* %uv_tmp_16            ; <<8 x i16>> [#uses=1]
  %mul125 = mul <8 x i16> %tmp123, %tmp124        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %mul125, <8 x i16>* %SF0
  %tmp126 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %tmp127 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16 ; <<8 x i16>> [#uses=1]
  %add128 = add nsw <8 x i16> %tmp126, %tmp127    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add128, <8 x i16>* %SF0
  %tmp129 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %shr130 = ashr <8 x i16> %tmp129, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %shr130, <8 x i16>* %SF0
  %tmp131 = load <8 x i16>* %SF_original          ; <<8 x i16>> [#uses=1]
  %tmp132 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %add133 = add nsw <8 x i16> %tmp131, %tmp132    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add133, <8 x i16>* %SF0
  %tmp134 = load <16 x i16>* %delta_sat_data      ; <<16 x i16>> [#uses=1]
  %tmp135 = shufflevector <16 x i16> %tmp134, <16 x i16> undef, <8 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp135, <8 x i16>* %SF_original
  %tmp136 = load <8 x i16>* %SF_original          ; <<8 x i16>> [#uses=1]
  %tmp137 = load <8 x i16>* %uv_tmp_16            ; <<8 x i16>> [#uses=1]
  %mul138 = mul <8 x i16> %tmp136, %tmp137        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %mul138, <8 x i16>* %SF1
  %tmp139 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %tmp140 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16 ; <<8 x i16>> [#uses=1]
  %add141 = add nsw <8 x i16> %tmp139, %tmp140    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add141, <8 x i16>* %SF1
  %tmp142 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %shr143 = ashr <8 x i16> %tmp142, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %shr143, <8 x i16>* %SF1
  %tmp144 = load <8 x i16>* %SF_original          ; <<8 x i16>> [#uses=1]
  %tmp145 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %add146 = add nsw <8 x i16> %tmp144, %tmp145    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add146, <8 x i16>* %SF1
  %tmp147 = load <8 x i16>* %u16                  ; <<8 x i16>> [#uses=1]
  %tmp148 = load <8 x i16> addrspace(2)* @alpha_mask_SF_8_s16 ; <<8 x i16>> [#uses=1]
  %and149 = and <8 x i16> %tmp147, %tmp148        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %and149, <8 x i16>* %uv_tmp_16
  %tmp150 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %tmp151 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %sub152 = sub <8 x i16> %tmp150, %tmp151        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %sub152, <8 x i16>* %SF1
  %tmp153 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %tmp154 = load <8 x i16>* %uv_tmp_16            ; <<8 x i16>> [#uses=1]
  %mul155 = mul <8 x i16> %tmp153, %tmp154        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %mul155, <8 x i16>* %SF1
  %tmp156 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %tmp157 = load <8 x i16> addrspace(2)* @rounding_SF_8_s16 ; <<8 x i16>> [#uses=1]
  %add158 = add nsw <8 x i16> %tmp156, %tmp157    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add158, <8 x i16>* %SF1
  %tmp159 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %shr160 = ashr <8 x i16> %tmp159, <i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %shr160, <8 x i16>* %SF1
  %tmp161 = load <8 x i16>* %SF1                  ; <<8 x i16>> [#uses=1]
  %tmp162 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %add163 = add nsw <8 x i16> %tmp161, %tmp162    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add163, <8 x i16>* %SF0
  %tmp165 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %tmp166 = load <8 x i16>* %u16                  ; <<8 x i16>> [#uses=1]
  %mul167 = mul <8 x i16> %tmp165, %tmp166        ; <<8 x i16>> [#uses=1]
  %tmp168 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp169 = shufflevector <8 x i16> %mul167, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<16 x i16>> [#uses=1]
  %tmp170 = shufflevector <16 x i16> %tmp168, <16 x i16> %tmp169, <16 x i32> <i32 16, i32 1, i32 17, i32 3, i32 18, i32 5, i32 19, i32 7, i32 20, i32 9, i32 21, i32 11, i32 22, i32 13, i32 23, i32 15> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %tmp170, <16 x i16>* %uvl_short16
  %tmp171 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %tmp172 = load <8 x i16>* %u16                  ; <<8 x i16>> [#uses=1]
  %call173 = call <8 x i16> @_Z6mul_hiDv8_sS_(<8 x i16> %tmp171, <8 x i16> %tmp172) ; <<8 x i16>> [#uses=1]
  %tmp174 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp175 = shufflevector <8 x i16> %call173, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<16 x i16>> [#uses=1]
  %tmp176 = shufflevector <16 x i16> %tmp174, <16 x i16> %tmp175, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %tmp176, <16 x i16>* %uvl_short16
  %tmp178 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %as_typen179 = bitcast <16 x i16> %tmp178 to <8 x i32> ; <<8 x i32>> [#uses=1]
  store <8 x i32> %as_typen179, <8 x i32>* %uvl_int8
  %tmp180 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %tmp181 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32 ; <<8 x i32>> [#uses=1]
  %add182 = add nsw <8 x i32> %tmp180, %tmp181    ; <<8 x i32>> [#uses=1]
  store <8 x i32> %add182, <8 x i32>* %uvl_int8
  %tmp183 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %shr184 = ashr <8 x i32> %tmp183, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7> ; <<8 x i32>> [#uses=1]
  store <8 x i32> %shr184, <8 x i32>* %uvl_int8
  %tmp185 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %as_typen186 = bitcast <8 x i32> %tmp185 to <16 x i16> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %as_typen186, <16 x i16>* %uvl_short16
  %tmp188 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp189 = shufflevector <16 x i16> %tmp188, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp189, <8 x i16>* %uvl_short8
  %tmp191 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %tmp192 = insertelement <8 x i16> undef, i16 %tmp191, i32 0 ; <<8 x i16>> [#uses=2]
  %splat = shufflevector <8 x i16> %tmp192, <8 x i16> %tmp192, <8 x i32> zeroinitializer ; <<8 x i16>> [#uses=1]
  store <8 x i16> %splat, <8 x i16>* %uv_min_val_8_16
  %tmp194 = load i16 addrspace(2)* @uv_max_val_16 ; <i16> [#uses=1]
  %tmp195 = insertelement <8 x i16> undef, i16 %tmp194, i32 0 ; <<8 x i16>> [#uses=2]
  %splat196 = shufflevector <8 x i16> %tmp195, <8 x i16> %tmp195, <8 x i32> zeroinitializer ; <<8 x i16>> [#uses=1]
  store <8 x i16> %splat196, <8 x i16>* %uv_max_val_8_16
  %tmp197 = load <8 x i16>* %uvl_short8           ; <<8 x i16>> [#uses=1]
  %tmp198 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16 ; <<8 x i16>> [#uses=1]
  %call199 = call <8 x i16> @_Z3maxDv8_sS_(<8 x i16> %tmp197, <8 x i16> %tmp198) ; <<8 x i16>> [#uses=1]
  %tmp200 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16 ; <<8 x i16>> [#uses=1]
  %call201 = call <8 x i16> @_Z3minDv8_sS_(<8 x i16> %call199, <8 x i16> %tmp200) ; <<8 x i16>> [#uses=1]
  store <8 x i16> %call201, <8 x i16>* %u16
  %tmp202 = load <8 x i16>* %u16                  ; <<8 x i16>> [#uses=1]
  %tmp203 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %tmp204 = insertelement <8 x i16> undef, i16 %tmp203, i32 0 ; <<8 x i16>> [#uses=2]
  %splat205 = shufflevector <8 x i16> %tmp204, <8 x i16> %tmp204, <8 x i32> zeroinitializer ; <<8 x i16>> [#uses=1]
  %sub206 = sub <8 x i16> %tmp202, %splat205      ; <<8 x i16>> [#uses=1]
  store <8 x i16> %sub206, <8 x i16>* %u16
  %tmp207 = load <8 x i16>* %u16                  ; <<8 x i16>> [#uses=1]
  %tmp208 = load i32* %pixel_index                ; <i32> [#uses=1]
  %tmp209 = load <8 x i16> addrspace(1)** %Uout.addr ; <<8 x i16> addrspace(1)*> [#uses=1]
  %arrayidx210 = getelementptr inbounds <8 x i16> addrspace(1)* %tmp209, i32 %tmp208 ; <<8 x i16> addrspace(1)*> [#uses=1]
  store <8 x i16> %tmp207, <8 x i16> addrspace(1)* %arrayidx210
  %tmp211 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %tmp212 = load <8 x i16>* %v16                  ; <<8 x i16>> [#uses=1]
  %mul213 = mul <8 x i16> %tmp211, %tmp212        ; <<8 x i16>> [#uses=1]
  %tmp214 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp215 = shufflevector <8 x i16> %mul213, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<16 x i16>> [#uses=1]
  %tmp216 = shufflevector <16 x i16> %tmp214, <16 x i16> %tmp215, <16 x i32> <i32 16, i32 1, i32 17, i32 3, i32 18, i32 5, i32 19, i32 7, i32 20, i32 9, i32 21, i32 11, i32 22, i32 13, i32 23, i32 15> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %tmp216, <16 x i16>* %uvl_short16
  %tmp217 = load <8 x i16>* %SF0                  ; <<8 x i16>> [#uses=1]
  %tmp218 = load <8 x i16>* %v16                  ; <<8 x i16>> [#uses=1]
  %call219 = call <8 x i16> @_Z6mul_hiDv8_sS_(<8 x i16> %tmp217, <8 x i16> %tmp218) ; <<8 x i16>> [#uses=1]
  %tmp220 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp221 = shufflevector <8 x i16> %call219, <8 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<16 x i16>> [#uses=1]
  %tmp222 = shufflevector <16 x i16> %tmp220, <16 x i16> %tmp221, <16 x i32> <i32 0, i32 16, i32 2, i32 17, i32 4, i32 18, i32 6, i32 19, i32 8, i32 20, i32 10, i32 21, i32 12, i32 22, i32 14, i32 23> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %tmp222, <16 x i16>* %uvl_short16
  %tmp223 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %as_typen224 = bitcast <16 x i16> %tmp223 to <8 x i32> ; <<8 x i32>> [#uses=1]
  store <8 x i32> %as_typen224, <8 x i32>* %uvl_int8
  %tmp225 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %tmp226 = load <8 x i32> addrspace(2)* @uv_final_rounding_8_s32 ; <<8 x i32>> [#uses=1]
  %add227 = add nsw <8 x i32> %tmp225, %tmp226    ; <<8 x i32>> [#uses=1]
  store <8 x i32> %add227, <8 x i32>* %uvl_int8
  %tmp228 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %shr229 = ashr <8 x i32> %tmp228, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7> ; <<8 x i32>> [#uses=1]
  store <8 x i32> %shr229, <8 x i32>* %uvl_int8
  %tmp230 = load <8 x i32>* %uvl_int8             ; <<8 x i32>> [#uses=1]
  %as_typen231 = bitcast <8 x i32> %tmp230 to <16 x i16> ; <<16 x i16>> [#uses=1]
  store <16 x i16> %as_typen231, <16 x i16>* %uvl_short16
  %tmp232 = load <16 x i16>* %uvl_short16         ; <<16 x i16>> [#uses=1]
  %tmp233 = shufflevector <16 x i16> %tmp232, <16 x i16> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14> ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp233, <8 x i16>* %uvl_short8
  %tmp234 = load <8 x i16>* %uvl_short8           ; <<8 x i16>> [#uses=1]
  %tmp235 = load <8 x i16> addrspace(2)* @uv_min_val_8_s16 ; <<8 x i16>> [#uses=1]
  %call236 = call <8 x i16> @_Z3maxDv8_sS_(<8 x i16> %tmp234, <8 x i16> %tmp235) ; <<8 x i16>> [#uses=1]
  %tmp237 = load <8 x i16> addrspace(2)* @uv_max_val_8_s16 ; <<8 x i16>> [#uses=1]
  %call238 = call <8 x i16> @_Z3minDv8_sS_(<8 x i16> %call236, <8 x i16> %tmp237) ; <<8 x i16>> [#uses=1]
  store <8 x i16> %call238, <8 x i16>* %v16
  %tmp239 = load <8 x i16>* %v16                  ; <<8 x i16>> [#uses=1]
  %tmp240 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %tmp241 = insertelement <8 x i16> undef, i16 %tmp240, i32 0 ; <<8 x i16>> [#uses=2]
  %splat242 = shufflevector <8 x i16> %tmp241, <8 x i16> %tmp241, <8 x i32> zeroinitializer ; <<8 x i16>> [#uses=1]
  %sub243 = sub <8 x i16> %tmp239, %splat242      ; <<8 x i16>> [#uses=1]
  store <8 x i16> %sub243, <8 x i16>* %v16
  %tmp244 = load <8 x i16>* %v16                  ; <<8 x i16>> [#uses=1]
  %tmp245 = load i32* %pixel_index                ; <i32> [#uses=1]
  %tmp246 = load <8 x i16> addrspace(1)** %Vout.addr ; <<8 x i16> addrspace(1)*> [#uses=1]
  %arrayidx247 = getelementptr inbounds <8 x i16> addrspace(1)* %tmp246, i32 %tmp245 ; <<8 x i16> addrspace(1)*> [#uses=1]
  store <8 x i16> %tmp244, <8 x i16> addrspace(1)* %arrayidx247
  %tmp248 = load i32* %pixel_index                ; <i32> [#uses=1]
  %inc = add i32 %tmp248, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %pixel_index
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp249 = load i32* %iter                       ; <i32> [#uses=1]
  %add250 = add nsw i32 %tmp249, 8                ; <i32> [#uses=1]
  store i32 %add250, i32* %iter
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare <8 x i16> @_Z6mul_hiDv8_sS_(<8 x i16>, <8 x i16>)

declare <8 x i16> @_Z3minDv8_sS_(<8 x i16>, <8 x i16>)

declare <8 x i16> @_Z3maxDv8_sS_(<8 x i16>, <8 x i16>)

; CHECK: ret
define void @tcc_scalar_unroll2(i16 addrspace(1)* %Uin, i16 addrspace(1)* %Vin, i16 addrspace(1)* %Uout, i16 addrspace(1)* %Vout, i32 %bufferSize, i8 addrspace(2)* %SatLUTEntry) nounwind {
entry:
  %Uin.addr = alloca i16 addrspace(1)*, align 4   ; <i16 addrspace(1)**> [#uses=3]
  %Vin.addr = alloca i16 addrspace(1)*, align 4   ; <i16 addrspace(1)**> [#uses=3]
  %Uout.addr = alloca i16 addrspace(1)*, align 4  ; <i16 addrspace(1)**> [#uses=2]
  %Vout.addr = alloca i16 addrspace(1)*, align 4  ; <i16 addrspace(1)**> [#uses=2]
  %bufferSize.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %SatLUTEntry.addr = alloca i8 addrspace(2)*, align 4 ; <i8 addrspace(2)**> [#uses=5]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=2]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=2]
  %numOfPixelsToProcess = alloca i32, align 4     ; <i32*> [#uses=3]
  %pixel_index = alloca i32, align 4              ; <i32*> [#uses=11]
  %iter = alloca i32, align 4                     ; <i32*> [#uses=4]
  %uOut = alloca [2 x i32], align 4               ; <[2 x i32]*> [#uses=4]
  %vOut = alloca [2 x i32], align 4               ; <[2 x i32]*> [#uses=4]
  %UoutPtr = alloca i32 addrspace(1)*, align 4    ; <i32 addrspace(1)**> [#uses=2]
  %VoutPtr = alloca i32 addrspace(1)*, align 4    ; <i32 addrspace(1)**> [#uses=2]
  %u16 = alloca i16, align 2                      ; <i16*> [#uses=16]
  %v16 = alloca i16, align 2                      ; <i16*> [#uses=16]
  %u_key_16 = alloca i16, align 2                 ; <i16*> [#uses=8]
  %v_key_16 = alloca i16, align 2                 ; <i16*> [#uses=2]
  %uv_tmp_16 = alloca i16, align 2                ; <i16*> [#uses=10]
  %SF0 = alloca i32, align 4                      ; <i32*> [#uses=29]
  %SF1 = alloca i32, align 4                      ; <i32*> [#uses=37]
  %index = alloca i16, align 2                    ; <i16*> [#uses=6]
  %sat_data = alloca <4 x i8>, align 4            ; <<4 x i8>*> [#uses=20]
  %uvl = alloca i32, align 4                      ; <i32*> [#uses=24]
  %v_key_16221 = alloca i16, align 2              ; <i16*> [#uses=2]
  store i16 addrspace(1)* %Uin, i16 addrspace(1)** %Uin.addr
  store i16 addrspace(1)* %Vin, i16 addrspace(1)** %Vin.addr
  store i16 addrspace(1)* %Uout, i16 addrspace(1)** %Uout.addr
  store i16 addrspace(1)* %Vout, i16 addrspace(1)** %Vout.addr
  store i32 %bufferSize, i32* %bufferSize.addr
  store i8 addrspace(2)* %SatLUTEntry, i8 addrspace(2)** %SatLUTEntry.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalId
  %call1 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call1, i32* %globalSize
  %tmp = load i32* %bufferSize.addr               ; <i32> [#uses=1]
  %tmp2 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %numOfPixelsToProcess
  %tmp4 = load i32* %globalId                     ; <i32> [#uses=1]
  %tmp5 = load i32* %numOfPixelsToProcess         ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  store i32 %mul, i32* %pixel_index
  store i32 0, i32* %iter
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp7 = load i32* %iter                         ; <i32> [#uses=1]
  %tmp8 = load i32* %numOfPixelsToProcess         ; <i32> [#uses=1]
  %cmp9 = icmp ult i32 %tmp7, %tmp8               ; <i1> [#uses=1]
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp13 = load i16 addrspace(1)** %Uout.addr     ; <i16 addrspace(1)*> [#uses=1]
  %tmp14 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i16 addrspace(1)* %tmp13, i32 %tmp14 ; <i16 addrspace(1)*> [#uses=1]
  %0 = bitcast i16 addrspace(1)* %add.ptr to i32 addrspace(1)* ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %0, i32 addrspace(1)** %UoutPtr
  %tmp16 = load i16 addrspace(1)** %Vout.addr     ; <i16 addrspace(1)*> [#uses=1]
  %tmp17 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %add.ptr18 = getelementptr inbounds i16 addrspace(1)* %tmp16, i32 %tmp17 ; <i16 addrspace(1)*> [#uses=1]
  %1 = bitcast i16 addrspace(1)* %add.ptr18 to i32 addrspace(1)* ; <i32 addrspace(1)*> [#uses=1]
  store i32 addrspace(1)* %1, i32 addrspace(1)** %VoutPtr
  %tmp20 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %tmp21 = load i16 addrspace(1)** %Uin.addr      ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp21, i32 %tmp20 ; <i16 addrspace(1)*> [#uses=1]
  %tmp22 = load i16 addrspace(1)* %arrayidx       ; <i16> [#uses=1]
  %conv = sext i16 %tmp22 to i32                  ; <i32> [#uses=1]
  %tmp23 = load i16 addrspace(2)* @uv_min_val_16  ; <i16> [#uses=1]
  %conv24 = sext i16 %tmp23 to i32                ; <i32> [#uses=1]
  %add = add nsw i32 %conv, %conv24               ; <i32> [#uses=1]
  %conv25 = trunc i32 %add to i16                 ; <i16> [#uses=1]
  store i16 %conv25, i16* %u16
  %tmp27 = load i32* %pixel_index                 ; <i32> [#uses=1]
  %tmp28 = load i16 addrspace(1)** %Vin.addr      ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds i16 addrspace(1)* %tmp28, i32 %tmp27 ; <i16 addrspace(1)*> [#uses=1]
  %tmp30 = load i16 addrspace(1)* %arrayidx29     ; <i16> [#uses=1]
  %conv31 = sext i16 %tmp30 to i32                ; <i32> [#uses=1]
  %tmp32 = load i16 addrspace(2)* @uv_min_val_16  ; <i16> [#uses=1]
  %conv33 = sext i16 %tmp32 to i32                ; <i32> [#uses=1]
  %add34 = add nsw i32 %conv31, %conv33           ; <i32> [#uses=1]
  %conv35 = trunc i32 %add34 to i16               ; <i16> [#uses=1]
  store i16 %conv35, i16* %v16
  %tmp37 = load i16* %u16                         ; <i16> [#uses=1]
  %conv38 = sext i16 %tmp37 to i32                ; <i32> [#uses=1]
  %shr = ashr i32 %conv38, 5                      ; <i32> [#uses=1]
  %conv39 = trunc i32 %shr to i16                 ; <i16> [#uses=1]
  store i16 %conv39, i16* %u_key_16
  %tmp41 = load i16* %v16                         ; <i16> [#uses=1]
  %conv42 = sext i16 %tmp41 to i32                ; <i32> [#uses=1]
  %tmp43 = load i16 addrspace(2)* @hi_key_mask_16 ; <i16> [#uses=1]
  %conv44 = sext i16 %tmp43 to i32                ; <i32> [#uses=1]
  %and = and i32 %conv42, %conv44                 ; <i32> [#uses=1]
  %conv45 = trunc i32 %and to i16                 ; <i16> [#uses=1]
  store i16 %conv45, i16* %v_key_16
  %tmp46 = load i16* %u_key_16                    ; <i16> [#uses=1]
  %conv47 = sext i16 %tmp46 to i32                ; <i32> [#uses=1]
  %tmp48 = load i16* %v_key_16                    ; <i16> [#uses=1]
  %conv49 = sext i16 %tmp48 to i32                ; <i32> [#uses=1]
  %add50 = add nsw i32 %conv47, %conv49           ; <i32> [#uses=1]
  %conv51 = trunc i32 %add50 to i16               ; <i16> [#uses=1]
  store i16 %conv51, i16* %u_key_16
  %tmp53 = load i16* %v16                         ; <i16> [#uses=1]
  %conv54 = sext i16 %tmp53 to i32                ; <i32> [#uses=1]
  %tmp55 = load i16 addrspace(2)* @alpha_mask_SF_16 ; <i16> [#uses=1]
  %conv56 = sext i16 %tmp55 to i32                ; <i32> [#uses=1]
  %and57 = and i32 %conv54, %conv56               ; <i32> [#uses=1]
  %conv58 = trunc i32 %and57 to i16               ; <i16> [#uses=1]
  store i16 %conv58, i16* %uv_tmp_16
  store i32 0, i32* %SF0
  store i32 0, i32* %SF1
  %tmp62 = load i16* %u_key_16                    ; <i16> [#uses=1]
  %conv63 = sext i16 %tmp62 to i32                ; <i32> [#uses=1]
  %add64 = add nsw i32 %conv63, 528               ; <i32> [#uses=1]
  %conv65 = trunc i32 %add64 to i16               ; <i16> [#uses=1]
  store i16 %conv65, i16* %index
  %tmp67 = load i16* %index                       ; <i16> [#uses=1]
  %conv68 = sext i16 %tmp67 to i32                ; <i32> [#uses=1]
  %tmp69 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %call70 = call <2 x i8> @_Z6vload2jPKo2h(i32 %conv68, i8 addrspace(2)* %tmp69) ; <<2 x i8>> [#uses=1]
  %tmp71 = load <4 x i8>* %sat_data               ; <<4 x i8>> [#uses=1]
  %tmp72 = shufflevector <2 x i8> %call70, <2 x i8> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x i8>> [#uses=1]
  %tmp73 = shufflevector <4 x i8> %tmp71, <4 x i8> %tmp72, <4 x i32> <i32 4, i32 5, i32 2, i32 3> ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp73, <4 x i8>* %sat_data
  %tmp74 = load i16* %index                       ; <i16> [#uses=1]
  %conv75 = sext i16 %tmp74 to i32                ; <i32> [#uses=1]
  %add76 = add nsw i32 %conv75, 1                 ; <i32> [#uses=1]
  %tmp77 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %call78 = call <2 x i8> @_Z6vload2jPKo2h(i32 %add76, i8 addrspace(2)* %tmp77) ; <<2 x i8>> [#uses=1]
  %tmp79 = load <4 x i8>* %sat_data               ; <<4 x i8>> [#uses=1]
  %tmp80 = shufflevector <2 x i8> %call78, <2 x i8> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x i8>> [#uses=1]
  %tmp81 = shufflevector <4 x i8> %tmp79, <4 x i8> %tmp80, <4 x i32> <i32 0, i32 1, i32 4, i32 5> ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp81, <4 x i8>* %sat_data
  %tmp82 = load <4 x i8>* %sat_data               ; <<4 x i8>> [#uses=1]
  %tmp83 = extractelement <4 x i8> %tmp82, i32 0  ; <i8> [#uses=1]
  %conv84 = zext i8 %tmp83 to i32                 ; <i32> [#uses=1]
  %tmp85 = load <4 x i8>* %sat_data               ; <<4 x i8>> [#uses=1]
  %tmp86 = extractelement <4 x i8> %tmp85, i32 1  ; <i8> [#uses=1]
  %conv87 = zext i8 %tmp86 to i32                 ; <i32> [#uses=1]
  %sub = sub i32 %conv84, %conv87                 ; <i32> [#uses=1]
  store i32 %sub, i32* %SF0
  %tmp88 = load i32* %SF0                         ; <i32> [#uses=1]
  %tmp89 = load i16* %uv_tmp_16                   ; <i16> [#uses=1]
  %conv90 = sext i16 %tmp89 to i32                ; <i32> [#uses=1]
  %mul91 = mul i32 %tmp88, %conv90                ; <i32> [#uses=1]
  store i32 %mul91, i32* %SF0
  %tmp92 = load i32* %SF0                         ; <i32> [#uses=1]
  %tmp93 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv94 = sext i16 %tmp93 to i32                ; <i32> [#uses=1]
  %add95 = add nsw i32 %tmp92, %conv94            ; <i32> [#uses=1]
  store i32 %add95, i32* %SF0
  %tmp96 = load i32* %SF0                         ; <i32> [#uses=1]
  %shr97 = ashr i32 %tmp96, 5                     ; <i32> [#uses=1]
  store i32 %shr97, i32* %SF0
  %tmp98 = load <4 x i8>* %sat_data               ; <<4 x i8>> [#uses=1]
  %tmp99 = extractelement <4 x i8> %tmp98, i32 0  ; <i8> [#uses=1]
  %conv100 = zext i8 %tmp99 to i32                ; <i32> [#uses=1]
  %tmp101 = load i32* %SF0                        ; <i32> [#uses=1]
  %add102 = add nsw i32 %conv100, %tmp101         ; <i32> [#uses=1]
  store i32 %add102, i32* %SF0
  %tmp103 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp104 = extractelement <4 x i8> %tmp103, i32 2 ; <i8> [#uses=1]
  %conv105 = zext i8 %tmp104 to i32               ; <i32> [#uses=1]
  %tmp106 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp107 = extractelement <4 x i8> %tmp106, i32 3 ; <i8> [#uses=1]
  %conv108 = zext i8 %tmp107 to i32               ; <i32> [#uses=1]
  %sub109 = sub i32 %conv105, %conv108            ; <i32> [#uses=1]
  store i32 %sub109, i32* %SF1
  %tmp110 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp111 = load i16* %uv_tmp_16                  ; <i16> [#uses=1]
  %conv112 = sext i16 %tmp111 to i32              ; <i32> [#uses=1]
  %mul113 = mul i32 %tmp110, %conv112             ; <i32> [#uses=1]
  store i32 %mul113, i32* %SF1
  %tmp114 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp115 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv116 = sext i16 %tmp115 to i32              ; <i32> [#uses=1]
  %add117 = add nsw i32 %tmp114, %conv116         ; <i32> [#uses=1]
  store i32 %add117, i32* %SF1
  %tmp118 = load i32* %SF1                        ; <i32> [#uses=1]
  %shr119 = ashr i32 %tmp118, 5                   ; <i32> [#uses=1]
  store i32 %shr119, i32* %SF1
  %tmp120 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp121 = extractelement <4 x i8> %tmp120, i32 2 ; <i8> [#uses=1]
  %conv122 = zext i8 %tmp121 to i32               ; <i32> [#uses=1]
  %tmp123 = load i32* %SF1                        ; <i32> [#uses=1]
  %add124 = add nsw i32 %conv122, %tmp123         ; <i32> [#uses=1]
  store i32 %add124, i32* %SF1
  %tmp125 = load i16* %u16                        ; <i16> [#uses=1]
  %conv126 = sext i16 %tmp125 to i32              ; <i32> [#uses=1]
  %tmp127 = load i16 addrspace(2)* @alpha_mask_SF_16 ; <i16> [#uses=1]
  %conv128 = sext i16 %tmp127 to i32              ; <i32> [#uses=1]
  %and129 = and i32 %conv126, %conv128            ; <i32> [#uses=1]
  %conv130 = trunc i32 %and129 to i16             ; <i16> [#uses=1]
  store i16 %conv130, i16* %uv_tmp_16
  %tmp131 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp132 = load i32* %SF0                        ; <i32> [#uses=1]
  %sub133 = sub i32 %tmp131, %tmp132              ; <i32> [#uses=1]
  store i32 %sub133, i32* %SF1
  %tmp134 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp135 = load i16* %uv_tmp_16                  ; <i16> [#uses=1]
  %conv136 = sext i16 %tmp135 to i32              ; <i32> [#uses=1]
  %mul137 = mul i32 %tmp134, %conv136             ; <i32> [#uses=1]
  store i32 %mul137, i32* %SF1
  %tmp138 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp139 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv140 = sext i16 %tmp139 to i32              ; <i32> [#uses=1]
  %add141 = add nsw i32 %tmp138, %conv140         ; <i32> [#uses=1]
  store i32 %add141, i32* %SF1
  %tmp142 = load i32* %SF1                        ; <i32> [#uses=1]
  %shr143 = ashr i32 %tmp142, 5                   ; <i32> [#uses=1]
  store i32 %shr143, i32* %SF1
  %tmp144 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp145 = load i32* %SF0                        ; <i32> [#uses=1]
  %add146 = add nsw i32 %tmp144, %tmp145          ; <i32> [#uses=1]
  store i32 %add146, i32* %SF0
  %tmp148 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp149 = load i16* %u16                        ; <i16> [#uses=1]
  %conv150 = sext i16 %tmp149 to i32              ; <i32> [#uses=1]
  %mul151 = mul i32 %tmp148, %conv150             ; <i32> [#uses=1]
  store i32 %mul151, i32* %uvl
  %tmp152 = load i32* %uvl                        ; <i32> [#uses=1]
  %tmp153 = load i32 addrspace(2)* @uv_final_rounding_32 ; <i32> [#uses=1]
  %add154 = add nsw i32 %tmp152, %tmp153          ; <i32> [#uses=1]
  store i32 %add154, i32* %uvl
  %tmp155 = load i32* %uvl                        ; <i32> [#uses=1]
  %shr156 = ashr i32 %tmp155, 7                   ; <i32> [#uses=1]
  store i32 %shr156, i32* %uvl
  %tmp157 = load i32* %uvl                        ; <i32> [#uses=1]
  %conv158 = trunc i32 %tmp157 to i16             ; <i16> [#uses=1]
  %tmp159 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %call160 = call signext i16 @_Z3maxss(i16 signext %conv158, i16 signext %tmp159) ; <i16> [#uses=1]
  %tmp161 = load i16 addrspace(2)* @uv_max_val_16 ; <i16> [#uses=1]
  %call162 = call signext i16 @_Z3minss(i16 signext %call160, i16 signext %tmp161) ; <i16> [#uses=1]
  store i16 %call162, i16* %u16
  %tmp163 = load i16* %u16                        ; <i16> [#uses=1]
  %conv164 = sext i16 %tmp163 to i32              ; <i32> [#uses=1]
  %tmp165 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv166 = sext i16 %tmp165 to i32              ; <i32> [#uses=1]
  %sub167 = sub i32 %conv164, %conv166            ; <i32> [#uses=1]
  %conv168 = trunc i32 %sub167 to i16             ; <i16> [#uses=1]
  store i16 %conv168, i16* %u16
  %tmp169 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp170 = load i16* %v16                        ; <i16> [#uses=1]
  %conv171 = sext i16 %tmp170 to i32              ; <i32> [#uses=1]
  %mul172 = mul i32 %tmp169, %conv171             ; <i32> [#uses=1]
  store i32 %mul172, i32* %uvl
  %tmp173 = load i32* %uvl                        ; <i32> [#uses=1]
  %tmp174 = load i32 addrspace(2)* @uv_final_rounding_32 ; <i32> [#uses=1]
  %add175 = add nsw i32 %tmp173, %tmp174          ; <i32> [#uses=1]
  store i32 %add175, i32* %uvl
  %tmp176 = load i32* %uvl                        ; <i32> [#uses=1]
  %shr177 = ashr i32 %tmp176, 7                   ; <i32> [#uses=1]
  store i32 %shr177, i32* %uvl
  %tmp178 = load i32* %uvl                        ; <i32> [#uses=1]
  %conv179 = trunc i32 %tmp178 to i16             ; <i16> [#uses=1]
  %tmp180 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %call181 = call signext i16 @_Z3maxss(i16 signext %conv179, i16 signext %tmp180) ; <i16> [#uses=1]
  %tmp182 = load i16 addrspace(2)* @uv_max_val_16 ; <i16> [#uses=1]
  %call183 = call signext i16 @_Z3minss(i16 signext %call181, i16 signext %tmp182) ; <i16> [#uses=1]
  store i16 %call183, i16* %v16
  %tmp184 = load i16* %v16                        ; <i16> [#uses=1]
  %conv185 = sext i16 %tmp184 to i32              ; <i32> [#uses=1]
  %tmp186 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv187 = sext i16 %tmp186 to i32              ; <i32> [#uses=1]
  %sub188 = sub i32 %conv185, %conv187            ; <i32> [#uses=1]
  %conv189 = trunc i32 %sub188 to i16             ; <i16> [#uses=1]
  store i16 %conv189, i16* %v16
  %tmp190 = load i16* %u16                        ; <i16> [#uses=1]
  %conv191 = sext i16 %tmp190 to i32              ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [2 x i32]* %uOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx192 = getelementptr inbounds i32* %arraydecay, i32 0 ; <i32*> [#uses=1]
  store i32 %conv191, i32* %arrayidx192
  %tmp193 = load i16* %v16                        ; <i16> [#uses=1]
  %conv194 = sext i16 %tmp193 to i32              ; <i32> [#uses=1]
  %arraydecay195 = getelementptr inbounds [2 x i32]* %vOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx196 = getelementptr inbounds i32* %arraydecay195, i32 0 ; <i32*> [#uses=1]
  store i32 %conv194, i32* %arrayidx196
  %tmp197 = load i32* %pixel_index                ; <i32> [#uses=1]
  %inc = add i32 %tmp197, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %pixel_index
  %tmp198 = load i32* %pixel_index                ; <i32> [#uses=1]
  %tmp199 = load i16 addrspace(1)** %Uin.addr     ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx200 = getelementptr inbounds i16 addrspace(1)* %tmp199, i32 %tmp198 ; <i16 addrspace(1)*> [#uses=1]
  %tmp201 = load i16 addrspace(1)* %arrayidx200   ; <i16> [#uses=1]
  %conv202 = sext i16 %tmp201 to i32              ; <i32> [#uses=1]
  %tmp203 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv204 = sext i16 %tmp203 to i32              ; <i32> [#uses=1]
  %add205 = add nsw i32 %conv202, %conv204        ; <i32> [#uses=1]
  %conv206 = trunc i32 %add205 to i16             ; <i16> [#uses=1]
  store i16 %conv206, i16* %u16
  %tmp207 = load i32* %pixel_index                ; <i32> [#uses=1]
  %tmp208 = load i16 addrspace(1)** %Vin.addr     ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx209 = getelementptr inbounds i16 addrspace(1)* %tmp208, i32 %tmp207 ; <i16 addrspace(1)*> [#uses=1]
  %tmp210 = load i16 addrspace(1)* %arrayidx209   ; <i16> [#uses=1]
  %conv211 = sext i16 %tmp210 to i32              ; <i32> [#uses=1]
  %tmp212 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv213 = sext i16 %tmp212 to i32              ; <i32> [#uses=1]
  %add214 = add nsw i32 %conv211, %conv213        ; <i32> [#uses=1]
  %conv215 = trunc i32 %add214 to i16             ; <i16> [#uses=1]
  store i16 %conv215, i16* %v16
  %tmp216 = load i16* %u16                        ; <i16> [#uses=1]
  %conv217 = sext i16 %tmp216 to i32              ; <i32> [#uses=1]
  %shr218 = ashr i32 %conv217, 5                  ; <i32> [#uses=1]
  %conv219 = trunc i32 %shr218 to i16             ; <i16> [#uses=1]
  store i16 %conv219, i16* %u_key_16
  %tmp222 = load i16* %v16                        ; <i16> [#uses=1]
  %conv223 = sext i16 %tmp222 to i32              ; <i32> [#uses=1]
  %tmp224 = load i16 addrspace(2)* @hi_key_mask_16 ; <i16> [#uses=1]
  %conv225 = sext i16 %tmp224 to i32              ; <i32> [#uses=1]
  %and226 = and i32 %conv223, %conv225            ; <i32> [#uses=1]
  %conv227 = trunc i32 %and226 to i16             ; <i16> [#uses=1]
  store i16 %conv227, i16* %v_key_16221
  %tmp228 = load i16* %u_key_16                   ; <i16> [#uses=1]
  %conv229 = sext i16 %tmp228 to i32              ; <i32> [#uses=1]
  %tmp230 = load i16* %v_key_16221                ; <i16> [#uses=1]
  %conv231 = sext i16 %tmp230 to i32              ; <i32> [#uses=1]
  %add232 = add nsw i32 %conv229, %conv231        ; <i32> [#uses=1]
  %conv233 = trunc i32 %add232 to i16             ; <i16> [#uses=1]
  store i16 %conv233, i16* %u_key_16
  %tmp234 = load i16* %v16                        ; <i16> [#uses=1]
  %conv235 = sext i16 %tmp234 to i32              ; <i32> [#uses=1]
  %tmp236 = load i16 addrspace(2)* @alpha_mask_SF_16 ; <i16> [#uses=1]
  %conv237 = sext i16 %tmp236 to i32              ; <i32> [#uses=1]
  %and238 = and i32 %conv235, %conv237            ; <i32> [#uses=1]
  %conv239 = trunc i32 %and238 to i16             ; <i16> [#uses=1]
  store i16 %conv239, i16* %uv_tmp_16
  %tmp240 = load i16* %u_key_16                   ; <i16> [#uses=1]
  %conv241 = sext i16 %tmp240 to i32              ; <i32> [#uses=1]
  %add242 = add nsw i32 %conv241, 528             ; <i32> [#uses=1]
  %conv243 = trunc i32 %add242 to i16             ; <i16> [#uses=1]
  store i16 %conv243, i16* %index
  %tmp244 = load i16* %index                      ; <i16> [#uses=1]
  %conv245 = sext i16 %tmp244 to i32              ; <i32> [#uses=1]
  %tmp246 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %call247 = call <2 x i8> @_Z6vload2jPKo2h(i32 %conv245, i8 addrspace(2)* %tmp246) ; <<2 x i8>> [#uses=1]
  %tmp248 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp249 = shufflevector <2 x i8> %call247, <2 x i8> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x i8>> [#uses=1]
  %tmp250 = shufflevector <4 x i8> %tmp248, <4 x i8> %tmp249, <4 x i32> <i32 4, i32 5, i32 2, i32 3> ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp250, <4 x i8>* %sat_data
  %tmp251 = load i16* %index                      ; <i16> [#uses=1]
  %conv252 = sext i16 %tmp251 to i32              ; <i32> [#uses=1]
  %add253 = add nsw i32 %conv252, 1               ; <i32> [#uses=1]
  %tmp254 = load i8 addrspace(2)** %SatLUTEntry.addr ; <i8 addrspace(2)*> [#uses=1]
  %call255 = call <2 x i8> @_Z6vload2jPKo2h(i32 %add253, i8 addrspace(2)* %tmp254) ; <<2 x i8>> [#uses=1]
  %tmp256 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp257 = shufflevector <2 x i8> %call255, <2 x i8> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x i8>> [#uses=1]
  %tmp258 = shufflevector <4 x i8> %tmp256, <4 x i8> %tmp257, <4 x i32> <i32 0, i32 1, i32 4, i32 5> ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp258, <4 x i8>* %sat_data
  %tmp259 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp260 = extractelement <4 x i8> %tmp259, i32 0 ; <i8> [#uses=1]
  %conv261 = zext i8 %tmp260 to i32               ; <i32> [#uses=1]
  %tmp262 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp263 = extractelement <4 x i8> %tmp262, i32 1 ; <i8> [#uses=1]
  %conv264 = zext i8 %tmp263 to i32               ; <i32> [#uses=1]
  %sub265 = sub i32 %conv261, %conv264            ; <i32> [#uses=1]
  store i32 %sub265, i32* %SF0
  %tmp266 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp267 = load i16* %uv_tmp_16                  ; <i16> [#uses=1]
  %conv268 = sext i16 %tmp267 to i32              ; <i32> [#uses=1]
  %mul269 = mul i32 %tmp266, %conv268             ; <i32> [#uses=1]
  store i32 %mul269, i32* %SF0
  %tmp270 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp271 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv272 = sext i16 %tmp271 to i32              ; <i32> [#uses=1]
  %add273 = add nsw i32 %tmp270, %conv272         ; <i32> [#uses=1]
  store i32 %add273, i32* %SF0
  %tmp274 = load i32* %SF0                        ; <i32> [#uses=1]
  %shr275 = ashr i32 %tmp274, 5                   ; <i32> [#uses=1]
  store i32 %shr275, i32* %SF0
  %tmp276 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp277 = extractelement <4 x i8> %tmp276, i32 0 ; <i8> [#uses=1]
  %conv278 = zext i8 %tmp277 to i32               ; <i32> [#uses=1]
  %tmp279 = load i32* %SF0                        ; <i32> [#uses=1]
  %add280 = add nsw i32 %conv278, %tmp279         ; <i32> [#uses=1]
  store i32 %add280, i32* %SF0
  %tmp281 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp282 = extractelement <4 x i8> %tmp281, i32 2 ; <i8> [#uses=1]
  %conv283 = zext i8 %tmp282 to i32               ; <i32> [#uses=1]
  %tmp284 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp285 = extractelement <4 x i8> %tmp284, i32 3 ; <i8> [#uses=1]
  %conv286 = zext i8 %tmp285 to i32               ; <i32> [#uses=1]
  %sub287 = sub i32 %conv283, %conv286            ; <i32> [#uses=1]
  store i32 %sub287, i32* %SF1
  %tmp288 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp289 = load i16* %uv_tmp_16                  ; <i16> [#uses=1]
  %conv290 = sext i16 %tmp289 to i32              ; <i32> [#uses=1]
  %mul291 = mul i32 %tmp288, %conv290             ; <i32> [#uses=1]
  store i32 %mul291, i32* %SF1
  %tmp292 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp293 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv294 = sext i16 %tmp293 to i32              ; <i32> [#uses=1]
  %add295 = add nsw i32 %tmp292, %conv294         ; <i32> [#uses=1]
  store i32 %add295, i32* %SF1
  %tmp296 = load i32* %SF1                        ; <i32> [#uses=1]
  %shr297 = ashr i32 %tmp296, 5                   ; <i32> [#uses=1]
  store i32 %shr297, i32* %SF1
  %tmp298 = load <4 x i8>* %sat_data              ; <<4 x i8>> [#uses=1]
  %tmp299 = extractelement <4 x i8> %tmp298, i32 2 ; <i8> [#uses=1]
  %conv300 = zext i8 %tmp299 to i32               ; <i32> [#uses=1]
  %tmp301 = load i32* %SF1                        ; <i32> [#uses=1]
  %add302 = add nsw i32 %conv300, %tmp301         ; <i32> [#uses=1]
  store i32 %add302, i32* %SF1
  %tmp303 = load i16* %u16                        ; <i16> [#uses=1]
  %conv304 = sext i16 %tmp303 to i32              ; <i32> [#uses=1]
  %tmp305 = load i16 addrspace(2)* @alpha_mask_SF_16 ; <i16> [#uses=1]
  %conv306 = sext i16 %tmp305 to i32              ; <i32> [#uses=1]
  %and307 = and i32 %conv304, %conv306            ; <i32> [#uses=1]
  %conv308 = trunc i32 %and307 to i16             ; <i16> [#uses=1]
  store i16 %conv308, i16* %uv_tmp_16
  %tmp309 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp310 = load i32* %SF0                        ; <i32> [#uses=1]
  %sub311 = sub i32 %tmp309, %tmp310              ; <i32> [#uses=1]
  store i32 %sub311, i32* %SF1
  %tmp312 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp313 = load i16* %uv_tmp_16                  ; <i16> [#uses=1]
  %conv314 = sext i16 %tmp313 to i32              ; <i32> [#uses=1]
  %mul315 = mul i32 %tmp312, %conv314             ; <i32> [#uses=1]
  store i32 %mul315, i32* %SF1
  %tmp316 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp317 = load i16 addrspace(2)* @rounding_SF_16 ; <i16> [#uses=1]
  %conv318 = sext i16 %tmp317 to i32              ; <i32> [#uses=1]
  %add319 = add nsw i32 %tmp316, %conv318         ; <i32> [#uses=1]
  store i32 %add319, i32* %SF1
  %tmp320 = load i32* %SF1                        ; <i32> [#uses=1]
  %shr321 = ashr i32 %tmp320, 5                   ; <i32> [#uses=1]
  store i32 %shr321, i32* %SF1
  %tmp322 = load i32* %SF1                        ; <i32> [#uses=1]
  %tmp323 = load i32* %SF0                        ; <i32> [#uses=1]
  %add324 = add nsw i32 %tmp322, %tmp323          ; <i32> [#uses=1]
  store i32 %add324, i32* %SF0
  %tmp325 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp326 = load i16* %u16                        ; <i16> [#uses=1]
  %conv327 = sext i16 %tmp326 to i32              ; <i32> [#uses=1]
  %mul328 = mul i32 %tmp325, %conv327             ; <i32> [#uses=1]
  store i32 %mul328, i32* %uvl
  %tmp329 = load i32* %uvl                        ; <i32> [#uses=1]
  %tmp330 = load i32 addrspace(2)* @uv_final_rounding_32 ; <i32> [#uses=1]
  %add331 = add nsw i32 %tmp329, %tmp330          ; <i32> [#uses=1]
  store i32 %add331, i32* %uvl
  %tmp332 = load i32* %uvl                        ; <i32> [#uses=1]
  %shr333 = ashr i32 %tmp332, 7                   ; <i32> [#uses=1]
  store i32 %shr333, i32* %uvl
  %tmp334 = load i32* %uvl                        ; <i32> [#uses=1]
  %conv335 = trunc i32 %tmp334 to i16             ; <i16> [#uses=1]
  %tmp336 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %call337 = call signext i16 @_Z3maxss(i16 signext %conv335, i16 signext %tmp336) ; <i16> [#uses=1]
  %tmp338 = load i16 addrspace(2)* @uv_max_val_16 ; <i16> [#uses=1]
  %call339 = call signext i16 @_Z3minss(i16 signext %call337, i16 signext %tmp338) ; <i16> [#uses=1]
  store i16 %call339, i16* %u16
  %tmp340 = load i16* %u16                        ; <i16> [#uses=1]
  %conv341 = sext i16 %tmp340 to i32              ; <i32> [#uses=1]
  %tmp342 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv343 = sext i16 %tmp342 to i32              ; <i32> [#uses=1]
  %sub344 = sub i32 %conv341, %conv343            ; <i32> [#uses=1]
  %conv345 = trunc i32 %sub344 to i16             ; <i16> [#uses=1]
  store i16 %conv345, i16* %u16
  %tmp346 = load i32* %SF0                        ; <i32> [#uses=1]
  %tmp347 = load i16* %v16                        ; <i16> [#uses=1]
  %conv348 = sext i16 %tmp347 to i32              ; <i32> [#uses=1]
  %mul349 = mul i32 %tmp346, %conv348             ; <i32> [#uses=1]
  store i32 %mul349, i32* %uvl
  %tmp350 = load i32* %uvl                        ; <i32> [#uses=1]
  %tmp351 = load i32 addrspace(2)* @uv_final_rounding_32 ; <i32> [#uses=1]
  %add352 = add nsw i32 %tmp350, %tmp351          ; <i32> [#uses=1]
  store i32 %add352, i32* %uvl
  %tmp353 = load i32* %uvl                        ; <i32> [#uses=1]
  %shr354 = ashr i32 %tmp353, 7                   ; <i32> [#uses=1]
  store i32 %shr354, i32* %uvl
  %tmp355 = load i32* %uvl                        ; <i32> [#uses=1]
  %conv356 = trunc i32 %tmp355 to i16             ; <i16> [#uses=1]
  %tmp357 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %call358 = call signext i16 @_Z3maxss(i16 signext %conv356, i16 signext %tmp357) ; <i16> [#uses=1]
  %tmp359 = load i16 addrspace(2)* @uv_max_val_16 ; <i16> [#uses=1]
  %call360 = call signext i16 @_Z3minss(i16 signext %call358, i16 signext %tmp359) ; <i16> [#uses=1]
  store i16 %call360, i16* %v16
  %tmp361 = load i16* %v16                        ; <i16> [#uses=1]
  %conv362 = sext i16 %tmp361 to i32              ; <i32> [#uses=1]
  %tmp363 = load i16 addrspace(2)* @uv_min_val_16 ; <i16> [#uses=1]
  %conv364 = sext i16 %tmp363 to i32              ; <i32> [#uses=1]
  %sub365 = sub i32 %conv362, %conv364            ; <i32> [#uses=1]
  %conv366 = trunc i32 %sub365 to i16             ; <i16> [#uses=1]
  store i16 %conv366, i16* %v16
  %tmp367 = load i16* %u16                        ; <i16> [#uses=1]
  %conv368 = sext i16 %tmp367 to i32              ; <i32> [#uses=1]
  %arraydecay369 = getelementptr inbounds [2 x i32]* %uOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx370 = getelementptr inbounds i32* %arraydecay369, i32 1 ; <i32*> [#uses=1]
  store i32 %conv368, i32* %arrayidx370
  %tmp371 = load i16* %v16                        ; <i16> [#uses=1]
  %conv372 = sext i16 %tmp371 to i32              ; <i32> [#uses=1]
  %arraydecay373 = getelementptr inbounds [2 x i32]* %vOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx374 = getelementptr inbounds i32* %arraydecay373, i32 1 ; <i32*> [#uses=1]
  store i32 %conv372, i32* %arrayidx374
  %arraydecay375 = getelementptr inbounds [2 x i32]* %uOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx376 = getelementptr inbounds i32* %arraydecay375, i32 0 ; <i32*> [#uses=1]
  %tmp377 = load i32* %arrayidx376                ; <i32> [#uses=1]
  %and378 = and i32 %tmp377, 65535                ; <i32> [#uses=1]
  %shl = shl i32 %and378, 16                      ; <i32> [#uses=1]
  %arraydecay379 = getelementptr inbounds [2 x i32]* %uOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx380 = getelementptr inbounds i32* %arraydecay379, i32 1 ; <i32*> [#uses=1]
  %tmp381 = load i32* %arrayidx380                ; <i32> [#uses=1]
  %and382 = and i32 %tmp381, 65535                ; <i32> [#uses=1]
  %or = or i32 %shl, %and382                      ; <i32> [#uses=1]
  %tmp383 = load i32 addrspace(1)** %UoutPtr      ; <i32 addrspace(1)*> [#uses=1]
  store i32 %or, i32 addrspace(1)* %tmp383
  %arraydecay384 = getelementptr inbounds [2 x i32]* %vOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx385 = getelementptr inbounds i32* %arraydecay384, i32 0 ; <i32*> [#uses=1]
  %tmp386 = load i32* %arrayidx385                ; <i32> [#uses=1]
  %and387 = and i32 %tmp386, 65535                ; <i32> [#uses=1]
  %shl388 = shl i32 %and387, 16                   ; <i32> [#uses=1]
  %arraydecay389 = getelementptr inbounds [2 x i32]* %vOut, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx390 = getelementptr inbounds i32* %arraydecay389, i32 1 ; <i32*> [#uses=1]
  %tmp391 = load i32* %arrayidx390                ; <i32> [#uses=1]
  %and392 = and i32 %tmp391, 65535                ; <i32> [#uses=1]
  %or393 = or i32 %shl388, %and392                ; <i32> [#uses=1]
  %tmp394 = load i32 addrspace(1)** %VoutPtr      ; <i32 addrspace(1)*> [#uses=1]
  store i32 %or393, i32 addrspace(1)* %tmp394
  %tmp395 = load i32* %pixel_index                ; <i32> [#uses=1]
  %inc396 = add i32 %tmp395, 1                    ; <i32> [#uses=1]
  store i32 %inc396, i32* %pixel_index
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp397 = load i32* %iter                       ; <i32> [#uses=1]
  %add398 = add nsw i32 %tmp397, 2                ; <i32> [#uses=1]
  store i32 %add398, i32* %iter
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare <2 x i8> @_Z6vload2jPKo2h(i32, i8 addrspace(2)*)

declare signext i16 @_Z3minss(i16 signext, i16 signext)

declare signext i16 @_Z3maxss(i16 signext, i16 signext)
