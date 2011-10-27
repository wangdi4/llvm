; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\AIF_RadialBlur.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque
%struct.anon = type <{ <4 x float>, [2 x <2 x float>], [2 x <2 x float>], <2 x float>, float, float, float, i32 }>

@MAX_RADIAL_ITERATIONS = addrspace(2) global i32 127, align 4 ; <i32 addrspace(2)*> [#uses=5]
@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [75 x i8] c"float, float2, int, float4, kernelArgs __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[75 x i8]*> [#uses=1]
@opencl_AIF_RadialBlur2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_AIF_RadialBlur2D_parameters = appending global [107 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[107 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float, <2 x float>, i32, <4 x float>, %struct.anon addrspace(1)*)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([75 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, %struct.anon addrspace(2)*)* @AIF_RadialBlur2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_AIF_RadialBlur2D_locals to i8*), i8* getelementptr inbounds ([107 x i8]* @opencl_AIF_RadialBlur2D_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @evaluateDependents(float %amount, <2 x float> %center, i32 %blur_type, <4 x float> %boundary, %struct.anon addrspace(1)* %pArgs) nounwind {
entry:
  %amount.addr = alloca float, align 4            ; <float*> [#uses=3]
  %center.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %blur_type.addr = alloca i32, align 4           ; <i32*> [#uses=2]
  %boundary.addr = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=4]
  %pArgs.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=43]
  %halfWidth = alloca float, align 4              ; <float*> [#uses=4]
  %cosAngle = alloca float, align 4               ; <float*> [#uses=9]
  %sinAngle = alloca float, align 4               ; <float*> [#uses=6]
  %radiansAngle = alloca float, align 4           ; <float*> [#uses=3]
  %negN = alloca i32, align 4                     ; <i32*> [#uses=3]
  %zoom = alloca float, align 4                   ; <float*> [#uses=3]
  %bnd_adjust = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  store float %amount, float* %amount.addr
  store <2 x float> %center, <2 x float>* %center.addr
  store i32 %blur_type, i32* %blur_type.addr
  store <4 x float> %boundary, <4 x float>* %boundary.addr
  store %struct.anon addrspace(1)* %pArgs, %struct.anon addrspace(1)** %pArgs.addr
  %tmp = load <2 x float>* %center.addr           ; <<2 x float>> [#uses=1]
  %tmp1 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp2 = getelementptr inbounds %struct.anon addrspace(1)* %tmp1, i32 0, i32 3 ; <<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %tmp, <2 x float> addrspace(1)* %tmp2
  %tmp4 = load <4 x float>* %boundary.addr        ; <<4 x float>> [#uses=1]
  %tmp5 = extractelement <4 x float> %tmp4, i32 2 ; <float> [#uses=1]
  %tmp6 = load <4 x float>* %boundary.addr        ; <<4 x float>> [#uses=1]
  %tmp7 = extractelement <4 x float> %tmp6, i32 0 ; <float> [#uses=1]
  %sub = fsub float %tmp5, %tmp7                  ; <float> [#uses=1]
  %mul = fmul float %sub, 5.000000e-001           ; <float> [#uses=1]
  store float %mul, float* %halfWidth
  %tmp10 = load i32* %blur_type.addr              ; <i32> [#uses=1]
  %cmp = icmp eq i32 %tmp10, 0                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %tmp12 = load float* %amount.addr               ; <float> [#uses=1]
  %call = call float @_Z7radiansf(float %tmp12)   ; <float> [#uses=1]
  store float %call, float* %radiansAngle
  %tmp13 = load float* %radiansAngle              ; <float> [#uses=1]
  %call14 = call float @_Z4fabsf(float %tmp13)    ; <float> [#uses=1]
  %tmp15 = load float* %halfWidth                 ; <float> [#uses=1]
  %mul16 = fmul float %call14, %tmp15             ; <float> [#uses=1]
  %call17 = call float @_Z4ceilf(float %mul16)    ; <float> [#uses=1]
  %call18 = call i32 @_Z11convert_intf(float %call17) ; <i32> [#uses=1]
  %tmp19 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp20 = getelementptr inbounds %struct.anon addrspace(1)* %tmp19, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %call18, i32 addrspace(1)* %tmp20
  %tmp21 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp22 = getelementptr inbounds %struct.anon addrspace(1)* %tmp21, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp23 = load i32 addrspace(1)* %tmp22          ; <i32> [#uses=1]
  %tmp24 = load i32 addrspace(2)* @MAX_RADIAL_ITERATIONS ; <i32> [#uses=1]
  %cmp25 = icmp sgt i32 %tmp23, %tmp24            ; <i1> [#uses=1]
  br i1 %cmp25, label %if.then26, label %if.end

if.then26:                                        ; preds = %if.then
  %tmp27 = load i32 addrspace(2)* @MAX_RADIAL_ITERATIONS ; <i32> [#uses=1]
  %tmp28 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp29 = getelementptr inbounds %struct.anon addrspace(1)* %tmp28, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp27, i32 addrspace(1)* %tmp29
  br label %if.end

if.end:                                           ; preds = %if.then26, %if.then
  %tmp30 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp31 = getelementptr inbounds %struct.anon addrspace(1)* %tmp30, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp32 = load i32 addrspace(1)* %tmp31          ; <i32> [#uses=1]
  %mul33 = mul i32 2, %tmp32                      ; <i32> [#uses=1]
  %add = add nsw i32 %mul33, 1                    ; <i32> [#uses=1]
  %call34 = call float @_Z13convert_floati(i32 %add) ; <float> [#uses=3]
  %cmp35 = fcmp oeq float 0.000000e+000, %call34  ; <i1> [#uses=1]
  %sel = select i1 %cmp35, float 1.000000e+000, float %call34 ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %call34        ; <float> [#uses=1]
  %tmp36 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp37 = getelementptr inbounds %struct.anon addrspace(1)* %tmp36, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  store float %div, float addrspace(1)* %tmp37
  %tmp38 = load float* %radiansAngle              ; <float> [#uses=1]
  %tmp39 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp40 = getelementptr inbounds %struct.anon addrspace(1)* %tmp39, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  %tmp41 = load float addrspace(1)* %tmp40        ; <float> [#uses=1]
  %mul42 = fmul float %tmp38, %tmp41              ; <float> [#uses=1]
  %tmp43 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp44 = getelementptr inbounds %struct.anon addrspace(1)* %tmp43, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  store float %mul42, float addrspace(1)* %tmp44
  %tmp46 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp47 = getelementptr inbounds %struct.anon addrspace(1)* %tmp46, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp48 = load i32 addrspace(1)* %tmp47          ; <i32> [#uses=1]
  %mul49 = mul i32 -1, %tmp48                     ; <i32> [#uses=1]
  store i32 %mul49, i32* %negN
  %tmp50 = load i32* %negN                        ; <i32> [#uses=1]
  %call51 = call float @_Z13convert_floati(i32 %tmp50) ; <float> [#uses=1]
  %tmp52 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp53 = getelementptr inbounds %struct.anon addrspace(1)* %tmp52, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  %tmp54 = load float addrspace(1)* %tmp53        ; <float> [#uses=1]
  %mul55 = fmul float %call51, %tmp54             ; <float> [#uses=1]
  %call56 = call float @_Z3cosf(float %mul55)     ; <float> [#uses=1]
  store float %call56, float* %cosAngle
  %tmp57 = load i32* %negN                        ; <i32> [#uses=1]
  %call58 = call float @_Z13convert_floati(i32 %tmp57) ; <float> [#uses=1]
  %tmp59 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp60 = getelementptr inbounds %struct.anon addrspace(1)* %tmp59, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  %tmp61 = load float addrspace(1)* %tmp60        ; <float> [#uses=1]
  %mul62 = fmul float %call58, %tmp61             ; <float> [#uses=1]
  %call63 = call float @_Z3sinf(float %mul62)     ; <float> [#uses=1]
  store float %call63, float* %sinAngle
  %tmp64 = load float* %cosAngle                  ; <float> [#uses=1]
  %tmp65 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp66 = getelementptr inbounds %struct.anon addrspace(1)* %tmp65, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp66, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp67 = load <2 x float> addrspace(1)* %arrayidx ; <<2 x float>> [#uses=1]
  %tmp68 = insertelement <2 x float> %tmp67, float %tmp64, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp68, <2 x float> addrspace(1)* %arrayidx
  %tmp69 = load float* %sinAngle                  ; <float> [#uses=1]
  %tmp70 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp71 = getelementptr inbounds %struct.anon addrspace(1)* %tmp70, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay72 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp71, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay72, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp74 = load <2 x float> addrspace(1)* %arrayidx73 ; <<2 x float>> [#uses=1]
  %tmp75 = insertelement <2 x float> %tmp74, float %tmp69, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp75, <2 x float> addrspace(1)* %arrayidx73
  %tmp76 = load float* %sinAngle                  ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp76        ; <float> [#uses=1]
  %tmp77 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp78 = getelementptr inbounds %struct.anon addrspace(1)* %tmp77, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay79 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp78, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx80 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay79, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp81 = load <2 x float> addrspace(1)* %arrayidx80 ; <<2 x float>> [#uses=1]
  %tmp82 = insertelement <2 x float> %tmp81, float %neg, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp82, <2 x float> addrspace(1)* %arrayidx80
  %tmp83 = load float* %cosAngle                  ; <float> [#uses=1]
  %tmp84 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp85 = getelementptr inbounds %struct.anon addrspace(1)* %tmp84, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay86 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp85, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx87 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay86, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp88 = load <2 x float> addrspace(1)* %arrayidx87 ; <<2 x float>> [#uses=1]
  %tmp89 = insertelement <2 x float> %tmp88, float %tmp83, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp89, <2 x float> addrspace(1)* %arrayidx87
  %tmp90 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp91 = getelementptr inbounds %struct.anon addrspace(1)* %tmp90, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  %tmp92 = load float addrspace(1)* %tmp91        ; <float> [#uses=1]
  %call93 = call float @_Z3cosf(float %tmp92)     ; <float> [#uses=1]
  store float %call93, float* %cosAngle
  %tmp94 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp95 = getelementptr inbounds %struct.anon addrspace(1)* %tmp94, i32 0, i32 5 ; <float addrspace(1)*> [#uses=1]
  %tmp96 = load float addrspace(1)* %tmp95        ; <float> [#uses=1]
  %call97 = call float @_Z3sinf(float %tmp96)     ; <float> [#uses=1]
  store float %call97, float* %sinAngle
  %tmp98 = load float* %cosAngle                  ; <float> [#uses=1]
  %tmp99 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp100 = getelementptr inbounds %struct.anon addrspace(1)* %tmp99, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay101 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp100, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx102 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay101, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp103 = load <2 x float> addrspace(1)* %arrayidx102 ; <<2 x float>> [#uses=1]
  %tmp104 = insertelement <2 x float> %tmp103, float %tmp98, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp104, <2 x float> addrspace(1)* %arrayidx102
  %tmp105 = load float* %sinAngle                 ; <float> [#uses=1]
  %tmp106 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp107 = getelementptr inbounds %struct.anon addrspace(1)* %tmp106, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay108 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp107, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay108, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp110 = load <2 x float> addrspace(1)* %arrayidx109 ; <<2 x float>> [#uses=1]
  %tmp111 = insertelement <2 x float> %tmp110, float %tmp105, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp111, <2 x float> addrspace(1)* %arrayidx109
  %tmp112 = load float* %sinAngle                 ; <float> [#uses=1]
  %neg113 = fsub float -0.000000e+000, %tmp112    ; <float> [#uses=1]
  %tmp114 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp115 = getelementptr inbounds %struct.anon addrspace(1)* %tmp114, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay116 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp115, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx117 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay116, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp118 = load <2 x float> addrspace(1)* %arrayidx117 ; <<2 x float>> [#uses=1]
  %tmp119 = insertelement <2 x float> %tmp118, float %neg113, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp119, <2 x float> addrspace(1)* %arrayidx117
  %tmp120 = load float* %cosAngle                 ; <float> [#uses=1]
  %tmp121 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp122 = getelementptr inbounds %struct.anon addrspace(1)* %tmp121, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay123 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp122, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx124 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay123, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp125 = load <2 x float> addrspace(1)* %arrayidx124 ; <<2 x float>> [#uses=1]
  %tmp126 = insertelement <2 x float> %tmp125, float %tmp120, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp126, <2 x float> addrspace(1)* %arrayidx124
  br label %if.end246

if.else:                                          ; preds = %entry
  %tmp128 = load float* %amount.addr              ; <float> [#uses=1]
  %mul129 = fmul float %tmp128, 0x3F747AE140000000 ; <float> [#uses=1]
  %add130 = fadd float %mul129, 1.000000e+000     ; <float> [#uses=1]
  store float %add130, float* %zoom
  %tmp131 = load float* %halfWidth                ; <float> [#uses=1]
  %cmp132 = fcmp ogt float %tmp131, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp132, label %if.then133, label %if.else159

if.then133:                                       ; preds = %if.else
  %tmp134 = load float* %zoom                     ; <float> [#uses=1]
  %call135 = call float @_Z3logf(float %tmp134)   ; <float> [#uses=1]
  %tmp136 = load float* %halfWidth                ; <float> [#uses=3]
  %cmp137 = fcmp oeq float 0.000000e+000, %tmp136 ; <i1> [#uses=1]
  %sel138 = select i1 %cmp137, float 1.000000e+000, float %tmp136 ; <float> [#uses=0]
  %div139 = fdiv float 1.000000e+000, %tmp136     ; <float> [#uses=1]
  %sub140 = fsub float 1.000000e+000, %div139     ; <float> [#uses=1]
  %call141 = call float @_Z3logf(float %sub140)   ; <float> [#uses=3]
  %cmp142 = fcmp oeq float 0.000000e+000, %call141 ; <i1> [#uses=1]
  %sel143 = select i1 %cmp142, float 1.000000e+000, float %call141 ; <float> [#uses=0]
  %div144 = fdiv float %call135, %call141         ; <float> [#uses=1]
  %call145 = call float @_Z4fabsf(float %div144)  ; <float> [#uses=1]
  %call146 = call i32 @_Z11convert_intf(float %call145) ; <i32> [#uses=1]
  %tmp147 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp148 = getelementptr inbounds %struct.anon addrspace(1)* %tmp147, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %call146, i32 addrspace(1)* %tmp148
  %tmp149 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp150 = getelementptr inbounds %struct.anon addrspace(1)* %tmp149, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp151 = load i32 addrspace(1)* %tmp150        ; <i32> [#uses=1]
  %tmp152 = load i32 addrspace(2)* @MAX_RADIAL_ITERATIONS ; <i32> [#uses=1]
  %cmp153 = icmp sgt i32 %tmp151, %tmp152         ; <i1> [#uses=1]
  br i1 %cmp153, label %if.then154, label %if.end158

if.then154:                                       ; preds = %if.then133
  %tmp155 = load i32 addrspace(2)* @MAX_RADIAL_ITERATIONS ; <i32> [#uses=1]
  %tmp156 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp157 = getelementptr inbounds %struct.anon addrspace(1)* %tmp156, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp155, i32 addrspace(1)* %tmp157
  br label %if.end158

if.end158:                                        ; preds = %if.then154, %if.then133
  br label %if.end162

if.else159:                                       ; preds = %if.else
  %tmp160 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp161 = getelementptr inbounds %struct.anon addrspace(1)* %tmp160, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  store i32 0, i32 addrspace(1)* %tmp161
  br label %if.end162

if.end162:                                        ; preds = %if.else159, %if.end158
  %tmp163 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp164 = getelementptr inbounds %struct.anon addrspace(1)* %tmp163, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp165 = load i32 addrspace(1)* %tmp164        ; <i32> [#uses=1]
  %mul166 = mul i32 2, %tmp165                    ; <i32> [#uses=1]
  %add167 = add nsw i32 %mul166, 1                ; <i32> [#uses=1]
  %call168 = call float @_Z13convert_floati(i32 %add167) ; <float> [#uses=3]
  %cmp169 = fcmp oeq float 0.000000e+000, %call168 ; <i1> [#uses=1]
  %sel170 = select i1 %cmp169, float 1.000000e+000, float %call168 ; <float> [#uses=0]
  %div171 = fdiv float 1.000000e+000, %call168    ; <float> [#uses=1]
  %tmp172 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp173 = getelementptr inbounds %struct.anon addrspace(1)* %tmp172, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  store float %div171, float addrspace(1)* %tmp173
  %tmp174 = load float* %zoom                     ; <float> [#uses=1]
  %tmp175 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp176 = getelementptr inbounds %struct.anon addrspace(1)* %tmp175, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  %tmp177 = load float addrspace(1)* %tmp176      ; <float> [#uses=1]
  %call178 = call float @_Z3powff(float %tmp174, float %tmp177) ; <float> [#uses=1]
  %tmp179 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp180 = getelementptr inbounds %struct.anon addrspace(1)* %tmp179, i32 0, i32 6 ; <float addrspace(1)*> [#uses=1]
  store float %call178, float addrspace(1)* %tmp180
  %tmp181 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp182 = getelementptr inbounds %struct.anon addrspace(1)* %tmp181, i32 0, i32 6 ; <float addrspace(1)*> [#uses=1]
  %tmp183 = load float addrspace(1)* %tmp182      ; <float> [#uses=1]
  %tmp184 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp185 = getelementptr inbounds %struct.anon addrspace(1)* %tmp184, i32 0, i32 7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp186 = load i32 addrspace(1)* %tmp185        ; <i32> [#uses=1]
  %call187 = call float @_Z13convert_floati(i32 %tmp186) ; <float> [#uses=1]
  %neg188 = fsub float -0.000000e+000, %call187   ; <float> [#uses=1]
  %call189 = call float @_Z3powff(float %tmp183, float %neg188) ; <float> [#uses=1]
  store float %call189, float* %cosAngle
  %tmp190 = load float* %cosAngle                 ; <float> [#uses=1]
  %tmp191 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp192 = getelementptr inbounds %struct.anon addrspace(1)* %tmp191, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay193 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp192, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx194 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay193, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp195 = load <2 x float> addrspace(1)* %arrayidx194 ; <<2 x float>> [#uses=1]
  %tmp196 = insertelement <2 x float> %tmp195, float %tmp190, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp196, <2 x float> addrspace(1)* %arrayidx194
  %tmp197 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp198 = getelementptr inbounds %struct.anon addrspace(1)* %tmp197, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay199 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp198, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx200 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay199, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp201 = load <2 x float> addrspace(1)* %arrayidx200 ; <<2 x float>> [#uses=1]
  %tmp202 = insertelement <2 x float> %tmp201, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp202, <2 x float> addrspace(1)* %arrayidx200
  %tmp203 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp204 = getelementptr inbounds %struct.anon addrspace(1)* %tmp203, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay205 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp204, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx206 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay205, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp207 = load <2 x float> addrspace(1)* %arrayidx206 ; <<2 x float>> [#uses=1]
  %tmp208 = insertelement <2 x float> %tmp207, float 0.000000e+000, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp208, <2 x float> addrspace(1)* %arrayidx206
  %tmp209 = load float* %cosAngle                 ; <float> [#uses=1]
  %tmp210 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp211 = getelementptr inbounds %struct.anon addrspace(1)* %tmp210, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay212 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp211, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx213 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay212, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp214 = load <2 x float> addrspace(1)* %arrayidx213 ; <<2 x float>> [#uses=1]
  %tmp215 = insertelement <2 x float> %tmp214, float %tmp209, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp215, <2 x float> addrspace(1)* %arrayidx213
  %tmp216 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp217 = getelementptr inbounds %struct.anon addrspace(1)* %tmp216, i32 0, i32 6 ; <float addrspace(1)*> [#uses=1]
  %tmp218 = load float addrspace(1)* %tmp217      ; <float> [#uses=1]
  %tmp219 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp220 = getelementptr inbounds %struct.anon addrspace(1)* %tmp219, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay221 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp220, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx222 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay221, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp223 = load <2 x float> addrspace(1)* %arrayidx222 ; <<2 x float>> [#uses=1]
  %tmp224 = insertelement <2 x float> %tmp223, float %tmp218, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp224, <2 x float> addrspace(1)* %arrayidx222
  %tmp225 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp226 = getelementptr inbounds %struct.anon addrspace(1)* %tmp225, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay227 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp226, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx228 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay227, i32 0 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp229 = load <2 x float> addrspace(1)* %arrayidx228 ; <<2 x float>> [#uses=1]
  %tmp230 = insertelement <2 x float> %tmp229, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp230, <2 x float> addrspace(1)* %arrayidx228
  %tmp231 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp232 = getelementptr inbounds %struct.anon addrspace(1)* %tmp231, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay233 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp232, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx234 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay233, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp235 = load <2 x float> addrspace(1)* %arrayidx234 ; <<2 x float>> [#uses=1]
  %tmp236 = insertelement <2 x float> %tmp235, float 0.000000e+000, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp236, <2 x float> addrspace(1)* %arrayidx234
  %tmp237 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp238 = getelementptr inbounds %struct.anon addrspace(1)* %tmp237, i32 0, i32 6 ; <float addrspace(1)*> [#uses=1]
  %tmp239 = load float addrspace(1)* %tmp238      ; <float> [#uses=1]
  %tmp240 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp241 = getelementptr inbounds %struct.anon addrspace(1)* %tmp240, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(1)*> [#uses=1]
  %arraydecay242 = getelementptr inbounds [2 x <2 x float>] addrspace(1)* %tmp241, i32 0, i32 0 ; <<2 x float> addrspace(1)*> [#uses=1]
  %arrayidx243 = getelementptr inbounds <2 x float> addrspace(1)* %arraydecay242, i32 1 ; <<2 x float> addrspace(1)*> [#uses=2]
  %tmp244 = load <2 x float> addrspace(1)* %arrayidx243 ; <<2 x float>> [#uses=1]
  %tmp245 = insertelement <2 x float> %tmp244, float %tmp239, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp245, <2 x float> addrspace(1)* %arrayidx243
  br label %if.end246

if.end246:                                        ; preds = %if.end162, %if.end
  store <4 x float> <float 5.000000e-001, float 5.000000e-001, float -5.000000e-001, float -5.000000e-001>, <4 x float>* %bnd_adjust
  %tmp248 = load <4 x float>* %boundary.addr      ; <<4 x float>> [#uses=1]
  %tmp249 = load <4 x float>* %bnd_adjust         ; <<4 x float>> [#uses=1]
  %add250 = fadd <4 x float> %tmp248, %tmp249     ; <<4 x float>> [#uses=1]
  %tmp251 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp252 = getelementptr inbounds %struct.anon addrspace(1)* %tmp251, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %add250, <4 x float> addrspace(1)* %tmp252
  ret void
}

declare float @_Z7radiansf(float)

declare i32 @_Z11convert_intf(float)

declare float @_Z4ceilf(float)

declare float @_Z4fabsf(float)

declare float @_Z13convert_floati(i32)

declare float @_Z3cosf(float)

declare float @_Z3sinf(float)

declare float @_Z3logf(float)

declare float @_Z3powff(float, float)

; CHECK: ret
define <2 x float> @mat2x2_mul_float2(<2 x float> addrspace(2)* %mat, <2 x float> %v) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %mat.addr = alloca <2 x float> addrspace(2)*, align 4 ; <<2 x float> addrspace(2)**> [#uses=5]
  %v.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=5]
  store <2 x float> addrspace(2)* %mat, <2 x float> addrspace(2)** %mat.addr
  store <2 x float> %v, <2 x float>* %v.addr
  %tmp = load <2 x float> addrspace(2)** %mat.addr ; <<2 x float> addrspace(2)*> [#uses=1]
  %arrayidx = getelementptr inbounds <2 x float> addrspace(2)* %tmp, i32 0 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp1 = load <2 x float> addrspace(2)* %arrayidx ; <<2 x float>> [#uses=1]
  %tmp2 = extractelement <2 x float> %tmp1, i32 0 ; <float> [#uses=1]
  %tmp3 = load <2 x float>* %v.addr               ; <<2 x float>> [#uses=1]
  %tmp4 = extractelement <2 x float> %tmp3, i32 0 ; <float> [#uses=1]
  %mul = fmul float %tmp2, %tmp4                  ; <float> [#uses=1]
  %tmp5 = load <2 x float> addrspace(2)** %mat.addr ; <<2 x float> addrspace(2)*> [#uses=1]
  %arrayidx6 = getelementptr inbounds <2 x float> addrspace(2)* %tmp5, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp7 = load <2 x float> addrspace(2)* %arrayidx6 ; <<2 x float>> [#uses=1]
  %tmp8 = extractelement <2 x float> %tmp7, i32 0 ; <float> [#uses=1]
  %tmp9 = load <2 x float>* %v.addr               ; <<2 x float>> [#uses=1]
  %tmp10 = extractelement <2 x float> %tmp9, i32 1 ; <float> [#uses=1]
  %mul11 = fmul float %tmp8, %tmp10               ; <float> [#uses=1]
  %add = fadd float %mul, %mul11                  ; <float> [#uses=1]
  %tmp12 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  %tmp13 = insertelement <2 x float> %tmp12, float %add, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp13, <2 x float>* %rval
  %tmp14 = load <2 x float> addrspace(2)** %mat.addr ; <<2 x float> addrspace(2)*> [#uses=1]
  %arrayidx15 = getelementptr inbounds <2 x float> addrspace(2)* %tmp14, i32 0 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp16 = load <2 x float> addrspace(2)* %arrayidx15 ; <<2 x float>> [#uses=1]
  %tmp17 = extractelement <2 x float> %tmp16, i32 1 ; <float> [#uses=1]
  %tmp18 = load <2 x float>* %v.addr              ; <<2 x float>> [#uses=1]
  %tmp19 = extractelement <2 x float> %tmp18, i32 0 ; <float> [#uses=1]
  %mul20 = fmul float %tmp17, %tmp19              ; <float> [#uses=1]
  %tmp21 = load <2 x float> addrspace(2)** %mat.addr ; <<2 x float> addrspace(2)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds <2 x float> addrspace(2)* %tmp21, i32 1 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp23 = load <2 x float> addrspace(2)* %arrayidx22 ; <<2 x float>> [#uses=1]
  %tmp24 = extractelement <2 x float> %tmp23, i32 1 ; <float> [#uses=1]
  %tmp25 = load <2 x float>* %v.addr              ; <<2 x float>> [#uses=1]
  %tmp26 = extractelement <2 x float> %tmp25, i32 1 ; <float> [#uses=1]
  %mul27 = fmul float %tmp24, %tmp26              ; <float> [#uses=1]
  %add28 = fadd float %mul20, %mul27              ; <float> [#uses=1]
  %tmp29 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  %tmp30 = insertelement <2 x float> %tmp29, float %add28, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp30, <2 x float>* %rval
  %tmp31 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp31, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=9]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %acc = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %n = alloca i32, align 4                        ; <i32*> [#uses=2]
  %rs = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=11]
  %uv = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=3]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store i32 17, i32* %samplerLinear
  %tmp = load <2 x float>* %outCrd.addr           ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp, <2 x float>* %rs
  %tmp1 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp2 = getelementptr inbounds %struct.anon addrspace(2)* %tmp1, i32 0, i32 3 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp3 = load <2 x float> addrspace(2)* %tmp2    ; <<2 x float>> [#uses=1]
  %tmp4 = load <2 x float>* %rs                   ; <<2 x float>> [#uses=1]
  %sub = fsub <2 x float> %tmp4, %tmp3            ; <<2 x float>> [#uses=1]
  store <2 x float> %sub, <2 x float>* %rs
  %tmp6 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp7 = getelementptr inbounds %struct.anon addrspace(2)* %tmp6, i32 0, i32 1 ; <[2 x <2 x float>] addrspace(2)*> [#uses=1]
  %arraydecay = getelementptr inbounds [2 x <2 x float>] addrspace(2)* %tmp7, i32 0, i32 0 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp8 = load <2 x float>* %rs                   ; <<2 x float>> [#uses=1]
  %call = call <2 x float> @mat2x2_mul_float2(<2 x float> addrspace(2)* %arraydecay, <2 x float> %tmp8) ; <<2 x float>> [#uses=1]
  store <2 x float> %call, <2 x float>* %uv
  store <4 x float> zeroinitializer, <4 x float>* %acc
  %tmp9 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp10 = getelementptr inbounds %struct.anon addrspace(2)* %tmp9, i32 0, i32 7 ; <i32 addrspace(2)*> [#uses=1]
  %tmp11 = load i32 addrspace(2)* %tmp10          ; <i32> [#uses=1]
  %mul = mul i32 2, %tmp11                        ; <i32> [#uses=1]
  store i32 %mul, i32* %n
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %i                           ; <i32> [#uses=1]
  %tmp13 = load i32 addrspace(2)* @MAX_RADIAL_ITERATIONS ; <i32> [#uses=1]
  %mul14 = mul i32 %tmp13, 2                      ; <i32> [#uses=1]
  %cmp = icmp sle i32 %tmp12, %mul14              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %n                           ; <i32> [#uses=1]
  %cmp17 = icmp sgt i32 %tmp15, %tmp16            ; <i1> [#uses=1]
  br i1 %cmp17, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  br label %for.end

if.end:                                           ; preds = %for.body
  %tmp18 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon addrspace(2)* %tmp18, i32 0, i32 2 ; <[2 x <2 x float>] addrspace(2)*> [#uses=1]
  %arraydecay20 = getelementptr inbounds [2 x <2 x float>] addrspace(2)* %tmp19, i32 0, i32 0 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp21 = load <2 x float>* %uv                  ; <<2 x float>> [#uses=1]
  %call22 = call <2 x float> @mat2x2_mul_float2(<2 x float> addrspace(2)* %arraydecay20, <2 x float> %tmp21) ; <<2 x float>> [#uses=1]
  store <2 x float> %call22, <2 x float>* %rs
  %tmp23 = load <2 x float>* %rs                  ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp23, <2 x float>* %uv
  %tmp24 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp25 = getelementptr inbounds %struct.anon addrspace(2)* %tmp24, i32 0, i32 3 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp26 = load <2 x float> addrspace(2)* %tmp25  ; <<2 x float>> [#uses=1]
  %tmp27 = load <2 x float>* %rs                  ; <<2 x float>> [#uses=1]
  %add = fadd <2 x float> %tmp27, %tmp26          ; <<2 x float>> [#uses=1]
  store <2 x float> %add, <2 x float>* %rs
  %tmp28 = load <2 x float>* %rs                  ; <<2 x float>> [#uses=1]
  %tmp29 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp30 = getelementptr inbounds %struct.anon addrspace(2)* %tmp29, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp31 = load <4 x float> addrspace(2)* %tmp30  ; <<4 x float>> [#uses=1]
  %tmp32 = shufflevector <4 x float> %tmp31, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %tmp33 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp34 = getelementptr inbounds %struct.anon addrspace(2)* %tmp33, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp35 = load <4 x float> addrspace(2)* %tmp34  ; <<4 x float>> [#uses=1]
  %tmp36 = shufflevector <4 x float> %tmp35, <4 x float> undef, <2 x i32> <i32 2, i32 3> ; <<2 x float>> [#uses=1]
  %call37 = call <2 x float> @_Z5clampDv2_fS_S_(<2 x float> %tmp28, <2 x float> %tmp32, <2 x float> %tmp36) ; <<2 x float>> [#uses=1]
  store <2 x float> %call37, <2 x float>* %rs
  %tmp38 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp39 = load <2 x float>* %rs                  ; <<2 x float>> [#uses=1]
  %call40 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %tmp38, i32 17, <2 x float> %tmp39) ; <<4 x float>> [#uses=1]
  %tmp41 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %add42 = fadd <4 x float> %tmp41, %call40       ; <<4 x float>> [#uses=1]
  store <4 x float> %add42, <4 x float>* %acc
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp43 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp43, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %if.then, %for.cond
  %tmp44 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp45 = getelementptr inbounds %struct.anon addrspace(2)* %tmp44, i32 0, i32 7 ; <i32 addrspace(2)*> [#uses=1]
  %tmp46 = load i32 addrspace(2)* %tmp45          ; <i32> [#uses=1]
  %mul47 = mul i32 2, %tmp46                      ; <i32> [#uses=1]
  %add48 = add nsw i32 %mul47, 1                  ; <i32> [#uses=1]
  %call49 = call float @_Z13convert_floati(i32 %add48) ; <float> [#uses=1]
  %tmp50 = insertelement <4 x float> undef, float %call49, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp50, <4 x float> %tmp50, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %tmp51 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %cmp52 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp52, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp51, %splat          ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %acc
  %tmp53 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %tmp54 = extractelement <4 x float> %tmp53, i32 3 ; <float> [#uses=1]
  %cmp55 = fcmp une float %tmp54, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp55, label %if.then56, label %if.end69

if.then56:                                        ; preds = %for.end
  %tmp57 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %tmp58 = extractelement <4 x float> %tmp57, i32 3 ; <float> [#uses=1]
  %tmp59 = insertelement <3 x float> undef, float %tmp58, i32 0 ; <<3 x float>> [#uses=2]
  %splat60 = shufflevector <3 x float> %tmp59, <3 x float> %tmp59, <3 x i32> zeroinitializer ; <<3 x float>> [#uses=3]
  %tmp61 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %tmp62 = shufflevector <4 x float> %tmp61, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %cmp63 = fcmp oeq <3 x float> zeroinitializer, %splat60 ; <<3 x i1>> [#uses=1]
  %sel64 = select <3 x i1> %cmp63, <3 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <3 x float> %splat60 ; <<3 x float>> [#uses=0]
  %div65 = fdiv <3 x float> %tmp62, %splat60      ; <<3 x float>> [#uses=1]
  %tmp66 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %tmp67 = shufflevector <3 x float> %div65, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp68 = shufflevector <4 x float> %tmp66, <4 x float> %tmp67, <4 x i32> <i32 4, i32 5, i32 6, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68, <4 x float>* %acc
  br label %if.end69

if.end69:                                         ; preds = %if.then56, %for.end
  %tmp70 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp70, <4 x float>* %dst
  %tmp71 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp71, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <2 x float> @_Z5clampDv2_fS_S_(<2 x float>, <2 x float>, <2 x float>)

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t*, i32, <2 x float>)

; CHECK: ret
define void @AIF_RadialBlur2D(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %gid_pos = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=6]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %call, i32 0 ; <<2 x i32>> [#uses=1]
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %vecinit2 = insertelement <2 x i32> %vecinit, i32 %call1, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit2, <2 x i32>* %gid_pos
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call3 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call3, <2 x i32>* %imgSize
  %tmp4 = load <2 x i32>* %gid_pos                ; <<2 x i32>> [#uses=1]
  %tmp5 = extractelement <2 x i32> %tmp4, i32 0   ; <i32> [#uses=1]
  %tmp6 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp7 = extractelement <2 x i32> %tmp6, i32 0   ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp5, %tmp7                ; <i1> [#uses=1]
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %tmp8 = load <2 x i32>* %gid_pos                ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %tmp10 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 1 ; <i32> [#uses=1]
  %cmp12 = icmp slt i32 %tmp9, %tmp11             ; <i1> [#uses=1]
  br i1 %cmp12, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %tmp14 = load <2 x i32>* %gid_pos               ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 1 ; <i32> [#uses=1]
  %tmp16 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp17 = extractelement <2 x i32> %tmp16, i32 0 ; <i32> [#uses=1]
  %mul = mul i32 %tmp15, %tmp17                   ; <i32> [#uses=1]
  %tmp18 = load <2 x i32>* %gid_pos               ; <<2 x i32>> [#uses=1]
  %tmp19 = extractelement <2 x i32> %tmp18, i32 0 ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp19                 ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp21 = load <2 x i32>* %gid_pos               ; <<2 x i32>> [#uses=1]
  %call22 = call <2 x float> @_Z14convert_float2Dv2_i(<2 x i32> %tmp21) ; <<2 x float>> [#uses=1]
  store <2 x float> %call22, <2 x float>* %curCrd
  %tmp23 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp24 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp25 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call26 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp23, <2 x float> %tmp24, %struct.anon addrspace(2)* %tmp25) ; <<4 x float>> [#uses=1]
  %tmp27 = load i32* %index                       ; <i32> [#uses=1]
  %tmp28 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp28, i32 %tmp27 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call26, <4 x float> addrspace(1)* %arrayidx
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  ret void
}

declare i32 @get_global_id(i32)

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare <2 x float> @_Z14convert_float2Dv2_i(<2 x i32>)
