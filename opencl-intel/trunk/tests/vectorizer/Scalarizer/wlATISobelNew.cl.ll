; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATISobelNew.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_sobel_filter_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sobel_filter_parameters = appending global [93 x i8] c"uchar4 const __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*)* @sobel_filter to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sobel_filter_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_sobel_filter_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @sobel_filter(<4 x i8> addrspace(1)* %inputImage, <4 x i8> addrspace(1)* %outputImage) nounwind {
entry:
  %inputImage.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=10]
  %outputImage.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=5]
  %x = alloca i32, align 4                        ; <i32*> [#uses=4]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %width = alloca i32, align 4                    ; <i32*> [#uses=9]
  %height = alloca i32, align 4                   ; <i32*> [#uses=2]
  %Gx = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %Gy = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %c = alloca i32, align 4                        ; <i32*> [#uses=14]
  %i00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %i10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %i20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %i01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %i11 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=1]
  %i21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %i02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %i12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %i22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %.compoundliteral94 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral103 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral113 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral118 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tmp128 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=5]
  %.compoundliteral132 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x i8> addrspace(1)* %inputImage, <4 x i8> addrspace(1)** %inputImage.addr
  store <4 x i8> addrspace(1)* %outputImage, <4 x i8> addrspace(1)** %outputImage.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %x
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %y
  %call2 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call2, i32* %width
  %call3 = call i32 @get_global_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call3, i32* %height
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %Gx
  %tmp5 = load <4 x float>* %Gx                   ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp5, <4 x float>* %Gy
  %tmp7 = load i32* %x                            ; <i32> [#uses=1]
  %tmp8 = load i32* %y                            ; <i32> [#uses=1]
  %tmp9 = load i32* %width                        ; <i32> [#uses=1]
  %mul = mul i32 %tmp8, %tmp9                     ; <i32> [#uses=1]
  %add = add i32 %tmp7, %mul                      ; <i32> [#uses=1]
  store i32 %add, i32* %c
  %tmp10 = load i32* %x                           ; <i32> [#uses=1]
  %cmp = icmp uge i32 %tmp10, 1                   ; <i1> [#uses=1]
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %tmp11 = load i32* %x                           ; <i32> [#uses=1]
  %tmp12 = load i32* %width                       ; <i32> [#uses=1]
  %sub = sub i32 %tmp12, 1                        ; <i32> [#uses=1]
  %cmp13 = icmp ult i32 %tmp11, %sub              ; <i1> [#uses=1]
  br i1 %cmp13, label %land.lhs.true14, label %if.end

land.lhs.true14:                                  ; preds = %land.lhs.true
  %tmp15 = load i32* %y                           ; <i32> [#uses=1]
  %cmp16 = icmp uge i32 %tmp15, 1                 ; <i1> [#uses=1]
  br i1 %cmp16, label %land.lhs.true17, label %if.end

land.lhs.true17:                                  ; preds = %land.lhs.true14
  %tmp18 = load i32* %y                           ; <i32> [#uses=1]
  %tmp19 = load i32* %height                      ; <i32> [#uses=1]
  %sub20 = sub i32 %tmp19, 1                      ; <i32> [#uses=1]
  %cmp21 = icmp ult i32 %tmp18, %sub20            ; <i1> [#uses=1]
  br i1 %cmp21, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true17
  %tmp23 = load i32* %c                           ; <i32> [#uses=1]
  %sub24 = sub i32 %tmp23, 1                      ; <i32> [#uses=1]
  %tmp25 = load i32* %width                       ; <i32> [#uses=1]
  %sub26 = sub i32 %sub24, %tmp25                 ; <i32> [#uses=1]
  %tmp27 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %tmp27, i32 %sub26 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp28 = load <4 x i8> addrspace(1)* %arrayidx  ; <<4 x i8>> [#uses=1]
  %call29 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp28) ; <<4 x float>> [#uses=1]
  store <4 x float> %call29, <4 x float>* %i00
  %tmp31 = load i32* %c                           ; <i32> [#uses=1]
  %tmp32 = load i32* %width                       ; <i32> [#uses=1]
  %sub33 = sub i32 %tmp31, %tmp32                 ; <i32> [#uses=1]
  %tmp34 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx35 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp34, i32 %sub33 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp36 = load <4 x i8> addrspace(1)* %arrayidx35 ; <<4 x i8>> [#uses=1]
  %call37 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp36) ; <<4 x float>> [#uses=1]
  store <4 x float> %call37, <4 x float>* %i10
  %tmp39 = load i32* %c                           ; <i32> [#uses=1]
  %add40 = add nsw i32 %tmp39, 1                  ; <i32> [#uses=1]
  %tmp41 = load i32* %width                       ; <i32> [#uses=1]
  %sub42 = sub i32 %add40, %tmp41                 ; <i32> [#uses=1]
  %tmp43 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp43, i32 %sub42 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp45 = load <4 x i8> addrspace(1)* %arrayidx44 ; <<4 x i8>> [#uses=1]
  %call46 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp45) ; <<4 x float>> [#uses=1]
  store <4 x float> %call46, <4 x float>* %i20
  %tmp48 = load i32* %c                           ; <i32> [#uses=1]
  %sub49 = sub i32 %tmp48, 1                      ; <i32> [#uses=1]
  %tmp50 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp50, i32 %sub49 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp52 = load <4 x i8> addrspace(1)* %arrayidx51 ; <<4 x i8>> [#uses=1]
  %call53 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp52) ; <<4 x float>> [#uses=1]
  store <4 x float> %call53, <4 x float>* %i01
  %tmp55 = load i32* %c                           ; <i32> [#uses=1]
  %tmp56 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx57 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp56, i32 %tmp55 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp58 = load <4 x i8> addrspace(1)* %arrayidx57 ; <<4 x i8>> [#uses=1]
  %call59 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp58) ; <<4 x float>> [#uses=1]
  store <4 x float> %call59, <4 x float>* %i11
  %tmp61 = load i32* %c                           ; <i32> [#uses=1]
  %add62 = add nsw i32 %tmp61, 1                  ; <i32> [#uses=1]
  %tmp63 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx64 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp63, i32 %add62 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp65 = load <4 x i8> addrspace(1)* %arrayidx64 ; <<4 x i8>> [#uses=1]
  %call66 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp65) ; <<4 x float>> [#uses=1]
  store <4 x float> %call66, <4 x float>* %i21
  %tmp68 = load i32* %c                           ; <i32> [#uses=1]
  %sub69 = sub i32 %tmp68, 1                      ; <i32> [#uses=1]
  %tmp70 = load i32* %width                       ; <i32> [#uses=1]
  %add71 = add i32 %sub69, %tmp70                 ; <i32> [#uses=1]
  %tmp72 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp72, i32 %add71 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp74 = load <4 x i8> addrspace(1)* %arrayidx73 ; <<4 x i8>> [#uses=1]
  %call75 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp74) ; <<4 x float>> [#uses=1]
  store <4 x float> %call75, <4 x float>* %i02
  %tmp77 = load i32* %c                           ; <i32> [#uses=1]
  %tmp78 = load i32* %width                       ; <i32> [#uses=1]
  %add79 = add i32 %tmp77, %tmp78                 ; <i32> [#uses=1]
  %tmp80 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx81 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp80, i32 %add79 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp82 = load <4 x i8> addrspace(1)* %arrayidx81 ; <<4 x i8>> [#uses=1]
  %call83 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp82) ; <<4 x float>> [#uses=1]
  store <4 x float> %call83, <4 x float>* %i12
  %tmp85 = load i32* %c                           ; <i32> [#uses=1]
  %add86 = add nsw i32 %tmp85, 1                  ; <i32> [#uses=1]
  %tmp87 = load i32* %width                       ; <i32> [#uses=1]
  %add88 = add i32 %add86, %tmp87                 ; <i32> [#uses=1]
  %tmp89 = load <4 x i8> addrspace(1)** %inputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx90 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp89, i32 %add88 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp91 = load <4 x i8> addrspace(1)* %arrayidx90 ; <<4 x i8>> [#uses=1]
  %call92 = call <4 x float> @_Z14convert_float4Dv4_h(<4 x i8> %tmp91) ; <<4 x float>> [#uses=1]
  store <4 x float> %call92, <4 x float>* %i22
  %tmp93 = load <4 x float>* %i00                 ; <<4 x float>> [#uses=1]
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral94
  %tmp95 = load <4 x float>* %.compoundliteral94  ; <<4 x float>> [#uses=1]
  %tmp96 = load <4 x float>* %i10                 ; <<4 x float>> [#uses=1]
  %mul97 = fmul <4 x float> %tmp95, %tmp96        ; <<4 x float>> [#uses=1]
  %add98 = fadd <4 x float> %tmp93, %mul97        ; <<4 x float>> [#uses=1]
  %tmp99 = load <4 x float>* %i20                 ; <<4 x float>> [#uses=1]
  %add100 = fadd <4 x float> %add98, %tmp99       ; <<4 x float>> [#uses=1]
  %tmp101 = load <4 x float>* %i02                ; <<4 x float>> [#uses=1]
  %sub102 = fsub <4 x float> %add100, %tmp101     ; <<4 x float>> [#uses=1]
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral103
  %tmp104 = load <4 x float>* %.compoundliteral103 ; <<4 x float>> [#uses=1]
  %tmp105 = load <4 x float>* %i12                ; <<4 x float>> [#uses=1]
  %mul106 = fmul <4 x float> %tmp104, %tmp105     ; <<4 x float>> [#uses=1]
  %sub107 = fsub <4 x float> %sub102, %mul106     ; <<4 x float>> [#uses=1]
  %tmp108 = load <4 x float>* %i22                ; <<4 x float>> [#uses=1]
  %sub109 = fsub <4 x float> %sub107, %tmp108     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub109, <4 x float>* %Gx
  %tmp110 = load <4 x float>* %i00                ; <<4 x float>> [#uses=1]
  %tmp111 = load <4 x float>* %i20                ; <<4 x float>> [#uses=1]
  %sub112 = fsub <4 x float> %tmp110, %tmp111     ; <<4 x float>> [#uses=1]
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral113
  %tmp114 = load <4 x float>* %.compoundliteral113 ; <<4 x float>> [#uses=1]
  %tmp115 = load <4 x float>* %i01                ; <<4 x float>> [#uses=1]
  %mul116 = fmul <4 x float> %tmp114, %tmp115     ; <<4 x float>> [#uses=1]
  %add117 = fadd <4 x float> %sub112, %mul116     ; <<4 x float>> [#uses=1]
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral118
  %tmp119 = load <4 x float>* %.compoundliteral118 ; <<4 x float>> [#uses=1]
  %tmp120 = load <4 x float>* %i21                ; <<4 x float>> [#uses=1]
  %mul121 = fmul <4 x float> %tmp119, %tmp120     ; <<4 x float>> [#uses=1]
  %sub122 = fsub <4 x float> %add117, %mul121     ; <<4 x float>> [#uses=1]
  %tmp123 = load <4 x float>* %i02                ; <<4 x float>> [#uses=1]
  %add124 = fadd <4 x float> %sub122, %tmp123     ; <<4 x float>> [#uses=1]
  %tmp125 = load <4 x float>* %i22                ; <<4 x float>> [#uses=1]
  %sub126 = fsub <4 x float> %add124, %tmp125     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub126, <4 x float>* %Gy
  %tmp129 = load <4 x float>* %Gx                 ; <<4 x float>> [#uses=1]
  %tmp130 = load <4 x float>* %Gy                 ; <<4 x float>> [#uses=1]
  %call131 = call <4 x float> @_Z5hypotDv4_fS_(<4 x float> %tmp129, <4 x float> %tmp130) ; <<4 x float>> [#uses=1]
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral132
  %tmp133 = load <4 x float>* %.compoundliteral132 ; <<4 x float>> [#uses=3]
  %cmp134 = fcmp oeq <4 x float> zeroinitializer, %tmp133 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp134, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp133 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %call131, %tmp133       ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %tmp128
  %tmp135 = load <4 x float>* %tmp128             ; <<4 x float>> [#uses=1]
  %tmp136 = extractelement <4 x float> %tmp135, i32 0 ; <float> [#uses=1]
  %conv = fptoui float %tmp136 to i8              ; <i8> [#uses=1]
  %tmp137 = load i32* %c                          ; <i32> [#uses=1]
  %tmp138 = load <4 x i8> addrspace(1)** %outputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx139 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp138, i32 %tmp137 ; <<4 x i8> addrspace(1)*> [#uses=2]
  %tmp140 = load <4 x i8> addrspace(1)* %arrayidx139 ; <<4 x i8>> [#uses=1]
  %tmp141 = insertelement <4 x i8> %tmp140, i8 %conv, i32 0 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp141, <4 x i8> addrspace(1)* %arrayidx139
  %tmp142 = load <4 x float>* %tmp128             ; <<4 x float>> [#uses=1]
  %tmp143 = extractelement <4 x float> %tmp142, i32 1 ; <float> [#uses=1]
  %conv144 = fptoui float %tmp143 to i8           ; <i8> [#uses=1]
  %tmp145 = load i32* %c                          ; <i32> [#uses=1]
  %tmp146 = load <4 x i8> addrspace(1)** %outputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx147 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp146, i32 %tmp145 ; <<4 x i8> addrspace(1)*> [#uses=2]
  %tmp148 = load <4 x i8> addrspace(1)* %arrayidx147 ; <<4 x i8>> [#uses=1]
  %tmp149 = insertelement <4 x i8> %tmp148, i8 %conv144, i32 1 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp149, <4 x i8> addrspace(1)* %arrayidx147
  %tmp150 = load <4 x float>* %tmp128             ; <<4 x float>> [#uses=1]
  %tmp151 = extractelement <4 x float> %tmp150, i32 2 ; <float> [#uses=1]
  %conv152 = fptoui float %tmp151 to i8           ; <i8> [#uses=1]
  %tmp153 = load i32* %c                          ; <i32> [#uses=1]
  %tmp154 = load <4 x i8> addrspace(1)** %outputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx155 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp154, i32 %tmp153 ; <<4 x i8> addrspace(1)*> [#uses=2]
  %tmp156 = load <4 x i8> addrspace(1)* %arrayidx155 ; <<4 x i8>> [#uses=1]
  %tmp157 = insertelement <4 x i8> %tmp156, i8 %conv152, i32 2 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp157, <4 x i8> addrspace(1)* %arrayidx155
  %tmp158 = load <4 x float>* %tmp128             ; <<4 x float>> [#uses=1]
  %tmp159 = extractelement <4 x float> %tmp158, i32 3 ; <float> [#uses=1]
  %conv160 = fptoui float %tmp159 to i8           ; <i8> [#uses=1]
  %tmp161 = load i32* %c                          ; <i32> [#uses=1]
  %tmp162 = load <4 x i8> addrspace(1)** %outputImage.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx163 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp162, i32 %tmp161 ; <<4 x i8> addrspace(1)*> [#uses=2]
  %tmp164 = load <4 x i8> addrspace(1)* %arrayidx163 ; <<4 x i8>> [#uses=1]
  %tmp165 = insertelement <4 x i8> %tmp164, i8 %conv160, i32 3 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp165, <4 x i8> addrspace(1)* %arrayidx163
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true17, %land.lhs.true14, %land.lhs.true, %entry
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare <4 x float> @_Z14convert_float4Dv4_h(<4 x i8>)

declare <4 x float> @_Z5hypotDv4_fS_(<4 x float>, <4 x float>)
