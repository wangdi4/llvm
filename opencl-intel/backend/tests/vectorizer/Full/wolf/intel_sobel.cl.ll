; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\intel_sobel.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_Sobel_Kernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_Sobel_Kernel_parameters = appending global [173 x i8] c"float __attribute__((address_space(1))) *, int, int, int, int, int, float __attribute__((address_space(1))) *, int, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[173 x i8]*> [#uses=1]
@opencl_Sobel_Kernel_Vec_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_Sobel_Kernel_Vec_parameters = appending global [173 x i8] c"float __attribute__((address_space(1))) *, int, int, int, int, int, float __attribute__((address_space(1))) *, int, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[173 x i8]*> [#uses=1]
@opencl_Sobel_Kernel_AutoVec_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_Sobel_Kernel_AutoVec_parameters = appending global [173 x i8] c"float __attribute__((address_space(1))) *, int, int, int, int, int, float __attribute__((address_space(1))) *, int, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[173 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32, i32, i32, i32, i32, float addrspace(1)*, i32, float addrspace(1)*, i32, i32, i32)* @Sobel_Kernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_Sobel_Kernel_locals to i8*), i8* getelementptr inbounds ([173 x i8]* @opencl_Sobel_Kernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32, i32, i32, i32, i32, float addrspace(1)*, i32, float addrspace(1)*, i32, i32, i32)* @Sobel_Kernel_Vec to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_Sobel_Kernel_Vec_locals to i8*), i8* getelementptr inbounds ([173 x i8]* @opencl_Sobel_Kernel_Vec_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32, i32, i32, i32, i32, float addrspace(1)*, i32, float addrspace(1)*, i32, i32, i32)* @Sobel_Kernel_AutoVec to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_Sobel_Kernel_AutoVec_locals to i8*), i8* getelementptr inbounds ([173 x i8]* @opencl_Sobel_Kernel_AutoVec_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @Sobel_Kernel(float addrspace(1)* %Image, i32 %Image_PitchInFloat, i32 %Image_W, i32 %Image_H, i32 %Image_BW, i32 %Image_BH, float addrspace(1)* %OutX, i32 %OutX_PitchInFloat, float addrspace(1)* %OutY, i32 %OutY_PitchInFloat, i32 %ROIWidth, i32 %ROIHeight) nounwind {
entry:
  %Image.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %Image_PitchInFloat.addr = alloca i32, align 4  ; <i32*> [#uses=5]
  %Image_W.addr = alloca i32, align 4             ; <i32*> [#uses=2]
  %Image_H.addr = alloca i32, align 4             ; <i32*> [#uses=2]
  %Image_BW.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %Image_BH.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %OutX.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutX_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=3]
  %OutY.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutY_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=3]
  %ROIWidth.addr = alloca i32, align 4            ; <i32*> [#uses=4]
  %ROIHeight.addr = alloca i32, align 4           ; <i32*> [#uses=4]
  %x = alloca i32, align 4                        ; <i32*> [#uses=14]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %yi = alloca i32, align 4                       ; <i32*> [#uses=4]
  %xi = alloca i32, align 4                       ; <i32*> [#uses=4]
  %W = alloca i32, align 4                        ; <i32*> [#uses=3]
  %H = alloca i32, align 4                        ; <i32*> [#uses=3]
  %y0 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %x0 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %pSrc0 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=8]
  %pDstX = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=4]
  %pDstY = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=4]
  %pSrc1 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=3]
  %pSrc2 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=4]
  %v00 = alloca float, align 4                    ; <float*> [#uses=3]
  %v01 = alloca float, align 4                    ; <float*> [#uses=2]
  %v02 = alloca float, align 4                    ; <float*> [#uses=3]
  %v10 = alloca float, align 4                    ; <float*> [#uses=2]
  %v12 = alloca float, align 4                    ; <float*> [#uses=2]
  %v20 = alloca float, align 4                    ; <float*> [#uses=3]
  %v21 = alloca float, align 4                    ; <float*> [#uses=2]
  %v22 = alloca float, align 4                    ; <float*> [#uses=3]
  store float addrspace(1)* %Image, float addrspace(1)** %Image.addr
  store i32 %Image_PitchInFloat, i32* %Image_PitchInFloat.addr
  store i32 %Image_W, i32* %Image_W.addr
  store i32 %Image_H, i32* %Image_H.addr
  store i32 %Image_BW, i32* %Image_BW.addr
  store i32 %Image_BH, i32* %Image_BH.addr
  store float addrspace(1)* %OutX, float addrspace(1)** %OutX.addr
  store i32 %OutX_PitchInFloat, i32* %OutX_PitchInFloat.addr
  store float addrspace(1)* %OutY, float addrspace(1)** %OutY.addr
  store i32 %OutY_PitchInFloat, i32* %OutY_PitchInFloat.addr
  store i32 %ROIWidth, i32* %ROIWidth.addr
  store i32 %ROIHeight, i32* %ROIHeight.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %yi
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %xi
  %tmp = load i32* %ROIWidth.addr                 ; <i32> [#uses=1]
  store i32 %tmp, i32* %W
  %tmp3 = load i32* %ROIHeight.addr               ; <i32> [#uses=1]
  store i32 %tmp3, i32* %H
  %tmp5 = load i32* %yi                           ; <i32> [#uses=1]
  %tmp6 = load i32* %ROIHeight.addr               ; <i32> [#uses=1]
  %mul = mul i32 %tmp5, %tmp6                     ; <i32> [#uses=1]
  store i32 %mul, i32* %y0
  %tmp8 = load i32* %xi                           ; <i32> [#uses=1]
  %tmp9 = load i32* %ROIWidth.addr                ; <i32> [#uses=1]
  %mul10 = mul i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  store i32 %mul10, i32* %x0
  %tmp12 = load float addrspace(1)** %Image.addr  ; <float addrspace(1)*> [#uses=1]
  %tmp13 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp14 = load i32* %Image_BH.addr               ; <i32> [#uses=1]
  %sub = sub i32 %tmp14, 1                        ; <i32> [#uses=1]
  %tmp15 = load i32* %y0                          ; <i32> [#uses=1]
  %add = add nsw i32 %sub, %tmp15                 ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp13, %add                   ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(1)* %tmp12, i32 %mul16 ; <float addrspace(1)*> [#uses=1]
  %tmp17 = load i32* %Image_BW.addr               ; <i32> [#uses=1]
  %add.ptr18 = getelementptr inbounds float addrspace(1)* %add.ptr, i32 %tmp17 ; <float addrspace(1)*> [#uses=1]
  %sub.ptr = getelementptr inbounds float addrspace(1)* %add.ptr18, i32 -1 ; <float addrspace(1)*> [#uses=1]
  %tmp19 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr20 = getelementptr inbounds float addrspace(1)* %sub.ptr, i32 %tmp19 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr20, float addrspace(1)** %pSrc0
  %tmp22 = load float addrspace(1)** %OutX.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp23 = load i32* %y0                          ; <i32> [#uses=1]
  %tmp24 = load i32* %OutX_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul25 = mul i32 %tmp23, %tmp24                 ; <i32> [#uses=1]
  %add.ptr26 = getelementptr inbounds float addrspace(1)* %tmp22, i32 %mul25 ; <float addrspace(1)*> [#uses=1]
  %tmp27 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr28 = getelementptr inbounds float addrspace(1)* %add.ptr26, i32 %tmp27 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr28, float addrspace(1)** %pDstX
  %tmp30 = load float addrspace(1)** %OutY.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp31 = load i32* %y0                          ; <i32> [#uses=1]
  %tmp32 = load i32* %OutY_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul33 = mul i32 %tmp31, %tmp32                 ; <i32> [#uses=1]
  %add.ptr34 = getelementptr inbounds float addrspace(1)* %tmp30, i32 %mul33 ; <float addrspace(1)*> [#uses=1]
  %tmp35 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr36 = getelementptr inbounds float addrspace(1)* %add.ptr34, i32 %tmp35 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr36, float addrspace(1)** %pDstY
  %tmp37 = load i32* %yi                          ; <i32> [#uses=1]
  %add38 = add nsw i32 %tmp37, 1                  ; <i32> [#uses=1]
  %call39 = call i32 @get_global_size(i32 0)      ; <i32> [#uses=1]
  %cmp = icmp eq i32 %add38, %call39              ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp40 = load i32* %Image_H.addr                ; <i32> [#uses=1]
  %tmp41 = load i32* %yi                          ; <i32> [#uses=1]
  %tmp42 = load i32* %ROIHeight.addr              ; <i32> [#uses=1]
  %mul43 = mul i32 %tmp41, %tmp42                 ; <i32> [#uses=1]
  %sub44 = sub i32 %tmp40, %mul43                 ; <i32> [#uses=1]
  store i32 %sub44, i32* %H
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp45 = load i32* %xi                          ; <i32> [#uses=1]
  %add46 = add nsw i32 %tmp45, 1                  ; <i32> [#uses=1]
  %call47 = call i32 @get_global_size(i32 1)      ; <i32> [#uses=1]
  %cmp48 = icmp eq i32 %add46, %call47            ; <i1> [#uses=1]
  br i1 %cmp48, label %if.then49, label %if.end55

if.then49:                                        ; preds = %if.end
  %tmp50 = load i32* %Image_W.addr                ; <i32> [#uses=1]
  %tmp51 = load i32* %xi                          ; <i32> [#uses=1]
  %tmp52 = load i32* %ROIWidth.addr               ; <i32> [#uses=1]
  %mul53 = mul i32 %tmp51, %tmp52                 ; <i32> [#uses=1]
  %sub54 = sub i32 %tmp50, %mul53                 ; <i32> [#uses=1]
  store i32 %sub54, i32* %W
  br label %if.end55

if.end55:                                         ; preds = %if.then49, %if.end
  store i32 0, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc160, %if.end55
  %tmp56 = load i32* %y                           ; <i32> [#uses=1]
  %tmp57 = load i32* %H                           ; <i32> [#uses=1]
  %cmp58 = icmp slt i32 %tmp56, %tmp57            ; <i1> [#uses=1]
  br i1 %cmp58, label %for.body, label %for.end163

for.body:                                         ; preds = %for.cond
  %tmp60 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp61 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %add.ptr62 = getelementptr inbounds float addrspace(1)* %tmp60, i32 %tmp61 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr62, float addrspace(1)** %pSrc1
  %tmp64 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp65 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %mul66 = mul i32 2, %tmp65                      ; <i32> [#uses=1]
  %add.ptr67 = getelementptr inbounds float addrspace(1)* %tmp64, i32 %mul66 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr67, float addrspace(1)** %pSrc2
  store i32 0, i32* %x
  br label %for.cond68

for.cond68:                                       ; preds = %for.inc, %for.body
  %tmp69 = load i32* %x                           ; <i32> [#uses=1]
  %tmp70 = load i32* %W                           ; <i32> [#uses=1]
  %cmp71 = icmp slt i32 %tmp69, %tmp70            ; <i1> [#uses=1]
  br i1 %cmp71, label %for.body72, label %for.end

for.body72:                                       ; preds = %for.cond68
  %tmp74 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp75 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr76 = getelementptr inbounds float addrspace(1)* %tmp74, i32 %tmp75 ; <float addrspace(1)*> [#uses=1]
  %add.ptr77 = getelementptr inbounds float addrspace(1)* %add.ptr76, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp78 = load float addrspace(1)* %add.ptr77    ; <float> [#uses=1]
  store float %tmp78, float* %v00
  %tmp80 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp81 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr82 = getelementptr inbounds float addrspace(1)* %tmp80, i32 %tmp81 ; <float addrspace(1)*> [#uses=1]
  %add.ptr83 = getelementptr inbounds float addrspace(1)* %add.ptr82, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp84 = load float addrspace(1)* %add.ptr83    ; <float> [#uses=1]
  store float %tmp84, float* %v01
  %tmp86 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp87 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr88 = getelementptr inbounds float addrspace(1)* %tmp86, i32 %tmp87 ; <float addrspace(1)*> [#uses=1]
  %add.ptr89 = getelementptr inbounds float addrspace(1)* %add.ptr88, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp90 = load float addrspace(1)* %add.ptr89    ; <float> [#uses=1]
  store float %tmp90, float* %v02
  %tmp92 = load float addrspace(1)** %pSrc1       ; <float addrspace(1)*> [#uses=1]
  %tmp93 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr94 = getelementptr inbounds float addrspace(1)* %tmp92, i32 %tmp93 ; <float addrspace(1)*> [#uses=1]
  %add.ptr95 = getelementptr inbounds float addrspace(1)* %add.ptr94, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp96 = load float addrspace(1)* %add.ptr95    ; <float> [#uses=1]
  store float %tmp96, float* %v10
  %tmp98 = load float addrspace(1)** %pSrc1       ; <float addrspace(1)*> [#uses=1]
  %tmp99 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr100 = getelementptr inbounds float addrspace(1)* %tmp98, i32 %tmp99 ; <float addrspace(1)*> [#uses=1]
  %add.ptr101 = getelementptr inbounds float addrspace(1)* %add.ptr100, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp102 = load float addrspace(1)* %add.ptr101  ; <float> [#uses=1]
  store float %tmp102, float* %v12
  %tmp104 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp105 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr106 = getelementptr inbounds float addrspace(1)* %tmp104, i32 %tmp105 ; <float addrspace(1)*> [#uses=1]
  %add.ptr107 = getelementptr inbounds float addrspace(1)* %add.ptr106, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp108 = load float addrspace(1)* %add.ptr107  ; <float> [#uses=1]
  store float %tmp108, float* %v20
  %tmp110 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp111 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr112 = getelementptr inbounds float addrspace(1)* %tmp110, i32 %tmp111 ; <float addrspace(1)*> [#uses=1]
  %add.ptr113 = getelementptr inbounds float addrspace(1)* %add.ptr112, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp114 = load float addrspace(1)* %add.ptr113  ; <float> [#uses=1]
  store float %tmp114, float* %v21
  %tmp116 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp117 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr118 = getelementptr inbounds float addrspace(1)* %tmp116, i32 %tmp117 ; <float addrspace(1)*> [#uses=1]
  %add.ptr119 = getelementptr inbounds float addrspace(1)* %add.ptr118, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp120 = load float addrspace(1)* %add.ptr119  ; <float> [#uses=1]
  store float %tmp120, float* %v22
  %tmp121 = load float* %v02                      ; <float> [#uses=1]
  %tmp122 = load float* %v00                      ; <float> [#uses=1]
  %sub123 = fsub float %tmp121, %tmp122           ; <float> [#uses=1]
  %tmp124 = load float* %v12                      ; <float> [#uses=1]
  %tmp125 = load float* %v10                      ; <float> [#uses=1]
  %sub126 = fsub float %tmp124, %tmp125           ; <float> [#uses=1]
  %mul127 = fmul float 2.000000e+000, %sub126     ; <float> [#uses=1]
  %add128 = fadd float %sub123, %mul127           ; <float> [#uses=1]
  %tmp129 = load float* %v22                      ; <float> [#uses=1]
  %tmp130 = load float* %v20                      ; <float> [#uses=1]
  %sub131 = fsub float %tmp129, %tmp130           ; <float> [#uses=1]
  %add132 = fadd float %add128, %sub131           ; <float> [#uses=1]
  %tmp133 = load i32* %x                          ; <i32> [#uses=1]
  %tmp134 = load float addrspace(1)** %pDstX      ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp134, i32 %tmp133 ; <float addrspace(1)*> [#uses=1]
  store float %add132, float addrspace(1)* %arrayidx
  %tmp135 = load float* %v20                      ; <float> [#uses=1]
  %tmp136 = load float* %v00                      ; <float> [#uses=1]
  %sub137 = fsub float %tmp135, %tmp136           ; <float> [#uses=1]
  %tmp138 = load float* %v21                      ; <float> [#uses=1]
  %tmp139 = load float* %v01                      ; <float> [#uses=1]
  %sub140 = fsub float %tmp138, %tmp139           ; <float> [#uses=1]
  %mul141 = fmul float 2.000000e+000, %sub140     ; <float> [#uses=1]
  %add142 = fadd float %sub137, %mul141           ; <float> [#uses=1]
  %tmp143 = load float* %v22                      ; <float> [#uses=1]
  %tmp144 = load float* %v02                      ; <float> [#uses=1]
  %sub145 = fsub float %tmp143, %tmp144           ; <float> [#uses=1]
  %add146 = fadd float %add142, %sub145           ; <float> [#uses=1]
  %tmp147 = load i32* %x                          ; <i32> [#uses=1]
  %tmp148 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %arrayidx149 = getelementptr inbounds float addrspace(1)* %tmp148, i32 %tmp147 ; <float addrspace(1)*> [#uses=1]
  store float %add146, float addrspace(1)* %arrayidx149
  br label %for.inc

for.inc:                                          ; preds = %for.body72
  %tmp150 = load i32* %x                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp150, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %x
  br label %for.cond68

for.end:                                          ; preds = %for.cond68
  %tmp151 = load i32* %Image_PitchInFloat.addr    ; <i32> [#uses=1]
  %tmp152 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %add.ptr153 = getelementptr inbounds float addrspace(1)* %tmp152, i32 %tmp151 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr153, float addrspace(1)** %pSrc0
  %tmp154 = load i32* %OutX_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp155 = load float addrspace(1)** %pDstX      ; <float addrspace(1)*> [#uses=1]
  %add.ptr156 = getelementptr inbounds float addrspace(1)* %tmp155, i32 %tmp154 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr156, float addrspace(1)** %pDstX
  %tmp157 = load i32* %OutY_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp158 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %add.ptr159 = getelementptr inbounds float addrspace(1)* %tmp158, i32 %tmp157 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr159, float addrspace(1)** %pDstY
  br label %for.inc160

for.inc160:                                       ; preds = %for.end
  %tmp161 = load i32* %y                          ; <i32> [#uses=1]
  %inc162 = add nsw i32 %tmp161, 1                ; <i32> [#uses=1]
  store i32 %inc162, i32* %y
  br label %for.cond

for.end163:                                       ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

; CHECK: ret
define void @Sobel_Kernel_Vec(float addrspace(1)* %Image, i32 %Image_PitchInFloat, i32 %Image_W, i32 %Image_H, i32 %Image_BW, i32 %Image_BH, float addrspace(1)* %OutX, i32 %OutX_PitchInFloat, float addrspace(1)* %OutY, i32 %OutY_PitchInFloat, i32 %ROIWidth, i32 %ROIHeight) nounwind {
entry:
  %Image.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %Image_PitchInFloat.addr = alloca i32, align 4  ; <i32*> [#uses=5]
  %Image_W.addr = alloca i32, align 4             ; <i32*> [#uses=2]
  %Image_H.addr = alloca i32, align 4             ; <i32*> [#uses=2]
  %Image_BW.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %Image_BH.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %OutX.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutX_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=3]
  %OutY.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutY_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=3]
  %ROIWidth.addr = alloca i32, align 4            ; <i32*> [#uses=4]
  %ROIHeight.addr = alloca i32, align 4           ; <i32*> [#uses=4]
  %two = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %x = alloca i32, align 4                        ; <i32*> [#uses=30]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %yi = alloca i32, align 4                       ; <i32*> [#uses=4]
  %xi = alloca i32, align 4                       ; <i32*> [#uses=4]
  %W = alloca i32, align 4                        ; <i32*> [#uses=4]
  %H = alloca i32, align 4                        ; <i32*> [#uses=3]
  %y0 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %x0 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %pSrc0 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=13]
  %pDstX = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=5]
  %pDstY = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=5]
  %pSrc1 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=5]
  %pSrc2 = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=9]
  %v00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %v01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %v02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %v10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %v12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %v20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %v21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %v22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  store float addrspace(1)* %Image, float addrspace(1)** %Image.addr
  store i32 %Image_PitchInFloat, i32* %Image_PitchInFloat.addr
  store i32 %Image_W, i32* %Image_W.addr
  store i32 %Image_H, i32* %Image_H.addr
  store i32 %Image_BW, i32* %Image_BW.addr
  store i32 %Image_BH, i32* %Image_BH.addr
  store float addrspace(1)* %OutX, float addrspace(1)** %OutX.addr
  store i32 %OutX_PitchInFloat, i32* %OutX_PitchInFloat.addr
  store float addrspace(1)* %OutY, float addrspace(1)** %OutY.addr
  store i32 %OutY_PitchInFloat, i32* %OutY_PitchInFloat.addr
  store i32 %ROIWidth, i32* %ROIWidth.addr
  store i32 %ROIHeight, i32* %ROIHeight.addr
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %two
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %yi
  %call5 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call5, i32* %xi
  %tmp7 = load i32* %ROIWidth.addr                ; <i32> [#uses=1]
  store i32 %tmp7, i32* %W
  %tmp9 = load i32* %ROIHeight.addr               ; <i32> [#uses=1]
  store i32 %tmp9, i32* %H
  %tmp11 = load i32* %yi                          ; <i32> [#uses=1]
  %tmp12 = load i32* %ROIHeight.addr              ; <i32> [#uses=1]
  %mul = mul i32 %tmp11, %tmp12                   ; <i32> [#uses=1]
  store i32 %mul, i32* %y0
  %tmp14 = load i32* %xi                          ; <i32> [#uses=1]
  %tmp15 = load i32* %ROIWidth.addr               ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp14, %tmp15                 ; <i32> [#uses=1]
  store i32 %mul16, i32* %x0
  %tmp18 = load float addrspace(1)** %Image.addr  ; <float addrspace(1)*> [#uses=1]
  %tmp19 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp20 = load i32* %Image_BH.addr               ; <i32> [#uses=1]
  %sub = sub i32 %tmp20, 1                        ; <i32> [#uses=1]
  %tmp21 = load i32* %y0                          ; <i32> [#uses=1]
  %add = add nsw i32 %sub, %tmp21                 ; <i32> [#uses=1]
  %mul22 = mul i32 %tmp19, %add                   ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(1)* %tmp18, i32 %mul22 ; <float addrspace(1)*> [#uses=1]
  %tmp23 = load i32* %Image_BW.addr               ; <i32> [#uses=1]
  %add.ptr24 = getelementptr inbounds float addrspace(1)* %add.ptr, i32 %tmp23 ; <float addrspace(1)*> [#uses=1]
  %sub.ptr = getelementptr inbounds float addrspace(1)* %add.ptr24, i32 -1 ; <float addrspace(1)*> [#uses=1]
  %tmp25 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr26 = getelementptr inbounds float addrspace(1)* %sub.ptr, i32 %tmp25 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr26, float addrspace(1)** %pSrc0
  %tmp28 = load float addrspace(1)** %OutX.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp29 = load i32* %y0                          ; <i32> [#uses=1]
  %tmp30 = load i32* %OutX_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul31 = mul i32 %tmp29, %tmp30                 ; <i32> [#uses=1]
  %add.ptr32 = getelementptr inbounds float addrspace(1)* %tmp28, i32 %mul31 ; <float addrspace(1)*> [#uses=1]
  %tmp33 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr34 = getelementptr inbounds float addrspace(1)* %add.ptr32, i32 %tmp33 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr34, float addrspace(1)** %pDstX
  %tmp36 = load float addrspace(1)** %OutY.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp37 = load i32* %y0                          ; <i32> [#uses=1]
  %tmp38 = load i32* %OutY_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul39 = mul i32 %tmp37, %tmp38                 ; <i32> [#uses=1]
  %add.ptr40 = getelementptr inbounds float addrspace(1)* %tmp36, i32 %mul39 ; <float addrspace(1)*> [#uses=1]
  %tmp41 = load i32* %x0                          ; <i32> [#uses=1]
  %add.ptr42 = getelementptr inbounds float addrspace(1)* %add.ptr40, i32 %tmp41 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr42, float addrspace(1)** %pDstY
  %tmp43 = load i32* %yi                          ; <i32> [#uses=1]
  %add44 = add nsw i32 %tmp43, 1                  ; <i32> [#uses=1]
  %call45 = call i32 @get_global_size(i32 0)      ; <i32> [#uses=1]
  %cmp = icmp eq i32 %add44, %call45              ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp46 = load i32* %Image_H.addr                ; <i32> [#uses=1]
  %tmp47 = load i32* %yi                          ; <i32> [#uses=1]
  %tmp48 = load i32* %ROIHeight.addr              ; <i32> [#uses=1]
  %mul49 = mul i32 %tmp47, %tmp48                 ; <i32> [#uses=1]
  %sub50 = sub i32 %tmp46, %mul49                 ; <i32> [#uses=1]
  store i32 %sub50, i32* %H
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp51 = load i32* %xi                          ; <i32> [#uses=1]
  %add52 = add nsw i32 %tmp51, 1                  ; <i32> [#uses=1]
  %call53 = call i32 @get_global_size(i32 1)      ; <i32> [#uses=1]
  %cmp54 = icmp eq i32 %add52, %call53            ; <i1> [#uses=1]
  br i1 %cmp54, label %if.then55, label %if.end61

if.then55:                                        ; preds = %if.end
  %tmp56 = load i32* %Image_W.addr                ; <i32> [#uses=1]
  %tmp57 = load i32* %xi                          ; <i32> [#uses=1]
  %tmp58 = load i32* %ROIWidth.addr               ; <i32> [#uses=1]
  %mul59 = mul i32 %tmp57, %tmp58                 ; <i32> [#uses=1]
  %sub60 = sub i32 %tmp56, %mul59                 ; <i32> [#uses=1]
  store i32 %sub60, i32* %W
  br label %if.end61

if.end61:                                         ; preds = %if.then55, %if.end
  store i32 0, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc257, %if.end61
  %tmp62 = load i32* %y                           ; <i32> [#uses=1]
  %tmp63 = load i32* %H                           ; <i32> [#uses=1]
  %cmp64 = icmp slt i32 %tmp62, %tmp63            ; <i1> [#uses=1]
  br i1 %cmp64, label %for.body, label %for.end259

for.body:                                         ; preds = %for.cond
  %tmp66 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp67 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %add.ptr68 = getelementptr inbounds float addrspace(1)* %tmp66, i32 %tmp67 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr68, float addrspace(1)** %pSrc1
  %tmp70 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp71 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %mul72 = mul i32 2, %tmp71                      ; <i32> [#uses=1]
  %add.ptr73 = getelementptr inbounds float addrspace(1)* %tmp70, i32 %mul72 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr73, float addrspace(1)** %pSrc2
  store i32 0, i32* %x
  br label %for.cond74

for.cond74:                                       ; preds = %for.inc, %for.body
  %tmp75 = load i32* %x                           ; <i32> [#uses=1]
  %tmp76 = load i32* %W                           ; <i32> [#uses=1]
  %sub77 = sub i32 %tmp76, 3                      ; <i32> [#uses=1]
  %cmp78 = icmp slt i32 %tmp75, %sub77            ; <i1> [#uses=1]
  br i1 %cmp78, label %for.body79, label %for.end

for.body79:                                       ; preds = %for.cond74
  %tmp81 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp82 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr83 = getelementptr inbounds float addrspace(1)* %tmp81, i32 %tmp82 ; <float addrspace(1)*> [#uses=1]
  %add.ptr84 = getelementptr inbounds float addrspace(1)* %add.ptr83, i32 0 ; <float addrspace(1)*> [#uses=1]
  %call85 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr84) ; <<4 x float>> [#uses=1]
  store <4 x float> %call85, <4 x float>* %v00
  %tmp87 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp88 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr89 = getelementptr inbounds float addrspace(1)* %tmp87, i32 %tmp88 ; <float addrspace(1)*> [#uses=1]
  %add.ptr90 = getelementptr inbounds float addrspace(1)* %add.ptr89, i32 1 ; <float addrspace(1)*> [#uses=1]
  %call91 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr90) ; <<4 x float>> [#uses=1]
  store <4 x float> %call91, <4 x float>* %v01
  %tmp93 = load float addrspace(1)** %pSrc0       ; <float addrspace(1)*> [#uses=1]
  %tmp94 = load i32* %x                           ; <i32> [#uses=1]
  %add.ptr95 = getelementptr inbounds float addrspace(1)* %tmp93, i32 %tmp94 ; <float addrspace(1)*> [#uses=1]
  %add.ptr96 = getelementptr inbounds float addrspace(1)* %add.ptr95, i32 2 ; <float addrspace(1)*> [#uses=1]
  %call97 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr96) ; <<4 x float>> [#uses=1]
  store <4 x float> %call97, <4 x float>* %v02
  %tmp99 = load float addrspace(1)** %pSrc1       ; <float addrspace(1)*> [#uses=1]
  %tmp100 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr101 = getelementptr inbounds float addrspace(1)* %tmp99, i32 %tmp100 ; <float addrspace(1)*> [#uses=1]
  %add.ptr102 = getelementptr inbounds float addrspace(1)* %add.ptr101, i32 0 ; <float addrspace(1)*> [#uses=1]
  %call103 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr102) ; <<4 x float>> [#uses=1]
  store <4 x float> %call103, <4 x float>* %v10
  %tmp105 = load float addrspace(1)** %pSrc1      ; <float addrspace(1)*> [#uses=1]
  %tmp106 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr107 = getelementptr inbounds float addrspace(1)* %tmp105, i32 %tmp106 ; <float addrspace(1)*> [#uses=1]
  %add.ptr108 = getelementptr inbounds float addrspace(1)* %add.ptr107, i32 2 ; <float addrspace(1)*> [#uses=1]
  %call109 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr108) ; <<4 x float>> [#uses=1]
  store <4 x float> %call109, <4 x float>* %v12
  %tmp111 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp112 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr113 = getelementptr inbounds float addrspace(1)* %tmp111, i32 %tmp112 ; <float addrspace(1)*> [#uses=1]
  %add.ptr114 = getelementptr inbounds float addrspace(1)* %add.ptr113, i32 0 ; <float addrspace(1)*> [#uses=1]
  %call115 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr114) ; <<4 x float>> [#uses=1]
  store <4 x float> %call115, <4 x float>* %v20
  %tmp117 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp118 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr119 = getelementptr inbounds float addrspace(1)* %tmp117, i32 %tmp118 ; <float addrspace(1)*> [#uses=1]
  %add.ptr120 = getelementptr inbounds float addrspace(1)* %add.ptr119, i32 1 ; <float addrspace(1)*> [#uses=1]
  %call121 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr120) ; <<4 x float>> [#uses=1]
  store <4 x float> %call121, <4 x float>* %v21
  %tmp123 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp124 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr125 = getelementptr inbounds float addrspace(1)* %tmp123, i32 %tmp124 ; <float addrspace(1)*> [#uses=1]
  %add.ptr126 = getelementptr inbounds float addrspace(1)* %add.ptr125, i32 2 ; <float addrspace(1)*> [#uses=1]
  %call127 = call <4 x float> @_Z6vload4jPKo1f(i32 0, float addrspace(1)* %add.ptr126) ; <<4 x float>> [#uses=1]
  store <4 x float> %call127, <4 x float>* %v22
  %tmp128 = load <4 x float>* %v02                ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %v00                ; <<4 x float>> [#uses=1]
  %sub130 = fsub <4 x float> %tmp128, %tmp129     ; <<4 x float>> [#uses=1]
  %tmp131 = load <4 x float>* %two                ; <<4 x float>> [#uses=1]
  %tmp132 = load <4 x float>* %v12                ; <<4 x float>> [#uses=1]
  %tmp133 = load <4 x float>* %v10                ; <<4 x float>> [#uses=1]
  %sub134 = fsub <4 x float> %tmp132, %tmp133     ; <<4 x float>> [#uses=1]
  %mul135 = fmul <4 x float> %tmp131, %sub134     ; <<4 x float>> [#uses=1]
  %add136 = fadd <4 x float> %sub130, %mul135     ; <<4 x float>> [#uses=1]
  %tmp137 = load <4 x float>* %v22                ; <<4 x float>> [#uses=1]
  %tmp138 = load <4 x float>* %v20                ; <<4 x float>> [#uses=1]
  %sub139 = fsub <4 x float> %tmp137, %tmp138     ; <<4 x float>> [#uses=1]
  %add140 = fadd <4 x float> %add136, %sub139     ; <<4 x float>> [#uses=1]
  %tmp141 = load float addrspace(1)** %pDstX      ; <float addrspace(1)*> [#uses=1]
  %tmp142 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr143 = getelementptr inbounds float addrspace(1)* %tmp141, i32 %tmp142 ; <float addrspace(1)*> [#uses=1]
  %0 = bitcast float addrspace(1)* %add.ptr143 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %add140, <4 x float> addrspace(1)* %0
  %tmp144 = load <4 x float>* %v20                ; <<4 x float>> [#uses=1]
  %tmp145 = load <4 x float>* %v00                ; <<4 x float>> [#uses=1]
  %sub146 = fsub <4 x float> %tmp144, %tmp145     ; <<4 x float>> [#uses=1]
  %tmp147 = load <4 x float>* %two                ; <<4 x float>> [#uses=1]
  %tmp148 = load <4 x float>* %v21                ; <<4 x float>> [#uses=1]
  %tmp149 = load <4 x float>* %v01                ; <<4 x float>> [#uses=1]
  %sub150 = fsub <4 x float> %tmp148, %tmp149     ; <<4 x float>> [#uses=1]
  %mul151 = fmul <4 x float> %tmp147, %sub150     ; <<4 x float>> [#uses=1]
  %add152 = fadd <4 x float> %sub146, %mul151     ; <<4 x float>> [#uses=1]
  %tmp153 = load <4 x float>* %v22                ; <<4 x float>> [#uses=1]
  %tmp154 = load <4 x float>* %v02                ; <<4 x float>> [#uses=1]
  %sub155 = fsub <4 x float> %tmp153, %tmp154     ; <<4 x float>> [#uses=1]
  %add156 = fadd <4 x float> %add152, %sub155     ; <<4 x float>> [#uses=1]
  %tmp157 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %tmp158 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr159 = getelementptr inbounds float addrspace(1)* %tmp157, i32 %tmp158 ; <float addrspace(1)*> [#uses=1]
  %1 = bitcast float addrspace(1)* %add.ptr159 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %add156, <4 x float> addrspace(1)* %1
  br label %for.inc

for.inc:                                          ; preds = %for.body79
  %tmp160 = load i32* %x                          ; <i32> [#uses=1]
  %add161 = add nsw i32 %tmp160, 4                ; <i32> [#uses=1]
  store i32 %add161, i32* %x
  br label %for.cond74

for.end:                                          ; preds = %for.cond74
  br label %for.cond162

for.cond162:                                      ; preds = %for.inc245, %for.end
  %tmp163 = load i32* %x                          ; <i32> [#uses=1]
  %tmp164 = load i32* %W                          ; <i32> [#uses=1]
  %cmp165 = icmp slt i32 %tmp163, %tmp164         ; <i1> [#uses=1]
  br i1 %cmp165, label %for.body166, label %for.end247

for.body166:                                      ; preds = %for.cond162
  %tmp167 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %tmp168 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr169 = getelementptr inbounds float addrspace(1)* %tmp167, i32 %tmp168 ; <float addrspace(1)*> [#uses=1]
  %add.ptr170 = getelementptr inbounds float addrspace(1)* %add.ptr169, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp171 = load float addrspace(1)* %add.ptr170  ; <float> [#uses=1]
  %tmp172 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %tmp173 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr174 = getelementptr inbounds float addrspace(1)* %tmp172, i32 %tmp173 ; <float addrspace(1)*> [#uses=1]
  %add.ptr175 = getelementptr inbounds float addrspace(1)* %add.ptr174, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp176 = load float addrspace(1)* %add.ptr175  ; <float> [#uses=1]
  %sub177 = fsub float %tmp171, %tmp176           ; <float> [#uses=1]
  %tmp178 = load float addrspace(1)** %pSrc1      ; <float addrspace(1)*> [#uses=1]
  %tmp179 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr180 = getelementptr inbounds float addrspace(1)* %tmp178, i32 %tmp179 ; <float addrspace(1)*> [#uses=1]
  %add.ptr181 = getelementptr inbounds float addrspace(1)* %add.ptr180, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp182 = load float addrspace(1)* %add.ptr181  ; <float> [#uses=1]
  %tmp183 = load float addrspace(1)** %pSrc1      ; <float addrspace(1)*> [#uses=1]
  %tmp184 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr185 = getelementptr inbounds float addrspace(1)* %tmp183, i32 %tmp184 ; <float addrspace(1)*> [#uses=1]
  %add.ptr186 = getelementptr inbounds float addrspace(1)* %add.ptr185, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp187 = load float addrspace(1)* %add.ptr186  ; <float> [#uses=1]
  %sub188 = fsub float %tmp182, %tmp187           ; <float> [#uses=1]
  %mul189 = fmul float 2.000000e+000, %sub188     ; <float> [#uses=1]
  %add190 = fadd float %sub177, %mul189           ; <float> [#uses=1]
  %tmp191 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp192 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr193 = getelementptr inbounds float addrspace(1)* %tmp191, i32 %tmp192 ; <float addrspace(1)*> [#uses=1]
  %add.ptr194 = getelementptr inbounds float addrspace(1)* %add.ptr193, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp195 = load float addrspace(1)* %add.ptr194  ; <float> [#uses=1]
  %tmp196 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp197 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr198 = getelementptr inbounds float addrspace(1)* %tmp196, i32 %tmp197 ; <float addrspace(1)*> [#uses=1]
  %add.ptr199 = getelementptr inbounds float addrspace(1)* %add.ptr198, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp200 = load float addrspace(1)* %add.ptr199  ; <float> [#uses=1]
  %sub201 = fsub float %tmp195, %tmp200           ; <float> [#uses=1]
  %add202 = fadd float %add190, %sub201           ; <float> [#uses=1]
  %tmp203 = load float addrspace(1)** %pDstX      ; <float addrspace(1)*> [#uses=1]
  %tmp204 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr205 = getelementptr inbounds float addrspace(1)* %tmp203, i32 %tmp204 ; <float addrspace(1)*> [#uses=1]
  store float %add202, float addrspace(1)* %add.ptr205
  %tmp206 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp207 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr208 = getelementptr inbounds float addrspace(1)* %tmp206, i32 %tmp207 ; <float addrspace(1)*> [#uses=1]
  %add.ptr209 = getelementptr inbounds float addrspace(1)* %add.ptr208, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp210 = load float addrspace(1)* %add.ptr209  ; <float> [#uses=1]
  %tmp211 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %tmp212 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr213 = getelementptr inbounds float addrspace(1)* %tmp211, i32 %tmp212 ; <float addrspace(1)*> [#uses=1]
  %add.ptr214 = getelementptr inbounds float addrspace(1)* %add.ptr213, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp215 = load float addrspace(1)* %add.ptr214  ; <float> [#uses=1]
  %sub216 = fsub float %tmp210, %tmp215           ; <float> [#uses=1]
  %tmp217 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp218 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr219 = getelementptr inbounds float addrspace(1)* %tmp217, i32 %tmp218 ; <float addrspace(1)*> [#uses=1]
  %add.ptr220 = getelementptr inbounds float addrspace(1)* %add.ptr219, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp221 = load float addrspace(1)* %add.ptr220  ; <float> [#uses=1]
  %tmp222 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %tmp223 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr224 = getelementptr inbounds float addrspace(1)* %tmp222, i32 %tmp223 ; <float addrspace(1)*> [#uses=1]
  %add.ptr225 = getelementptr inbounds float addrspace(1)* %add.ptr224, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp226 = load float addrspace(1)* %add.ptr225  ; <float> [#uses=1]
  %sub227 = fsub float %tmp221, %tmp226           ; <float> [#uses=1]
  %mul228 = fmul float 2.000000e+000, %sub227     ; <float> [#uses=1]
  %add229 = fadd float %sub216, %mul228           ; <float> [#uses=1]
  %tmp230 = load float addrspace(1)** %pSrc2      ; <float addrspace(1)*> [#uses=1]
  %tmp231 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr232 = getelementptr inbounds float addrspace(1)* %tmp230, i32 %tmp231 ; <float addrspace(1)*> [#uses=1]
  %add.ptr233 = getelementptr inbounds float addrspace(1)* %add.ptr232, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp234 = load float addrspace(1)* %add.ptr233  ; <float> [#uses=1]
  %tmp235 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %tmp236 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr237 = getelementptr inbounds float addrspace(1)* %tmp235, i32 %tmp236 ; <float addrspace(1)*> [#uses=1]
  %add.ptr238 = getelementptr inbounds float addrspace(1)* %add.ptr237, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp239 = load float addrspace(1)* %add.ptr238  ; <float> [#uses=1]
  %sub240 = fsub float %tmp234, %tmp239           ; <float> [#uses=1]
  %add241 = fadd float %add229, %sub240           ; <float> [#uses=1]
  %tmp242 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %tmp243 = load i32* %x                          ; <i32> [#uses=1]
  %add.ptr244 = getelementptr inbounds float addrspace(1)* %tmp242, i32 %tmp243 ; <float addrspace(1)*> [#uses=1]
  store float %add241, float addrspace(1)* %add.ptr244
  br label %for.inc245

for.inc245:                                       ; preds = %for.body166
  %tmp246 = load i32* %x                          ; <i32> [#uses=0]
  br label %for.cond162

for.end247:                                       ; preds = %for.cond162
  %tmp248 = load i32* %Image_PitchInFloat.addr    ; <i32> [#uses=1]
  %tmp249 = load float addrspace(1)** %pSrc0      ; <float addrspace(1)*> [#uses=1]
  %add.ptr250 = getelementptr inbounds float addrspace(1)* %tmp249, i32 %tmp248 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr250, float addrspace(1)** %pSrc0
  %tmp251 = load i32* %OutX_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp252 = load float addrspace(1)** %pDstX      ; <float addrspace(1)*> [#uses=1]
  %add.ptr253 = getelementptr inbounds float addrspace(1)* %tmp252, i32 %tmp251 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr253, float addrspace(1)** %pDstX
  %tmp254 = load i32* %OutY_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp255 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %add.ptr256 = getelementptr inbounds float addrspace(1)* %tmp255, i32 %tmp254 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr256, float addrspace(1)** %pDstY
  br label %for.inc257

for.inc257:                                       ; preds = %for.end247
  %tmp258 = load i32* %y                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp258, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %y
  br label %for.cond

for.end259:                                       ; preds = %for.cond
  ret void
}

declare <4 x float> @_Z6vload4jPKo1f(i32, float addrspace(1)*)

; CHECK: ret
define void @Sobel_Kernel_AutoVec(float addrspace(1)* %Image, i32 %Image_PitchInFloat, i32 %Image_W, i32 %Image_H, i32 %Image_BW, i32 %Image_BH, float addrspace(1)* %OutX, i32 %OutX_PitchInFloat, float addrspace(1)* %OutY, i32 %OutY_PitchInFloat, i32 %ROIWidth, i32 %ROIHeight) nounwind {
entry:
  %Image.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %Image_PitchInFloat.addr = alloca i32, align 4  ; <i32*> [#uses=7]
  %Image_W.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %Image_H.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %Image_BW.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %Image_BH.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %OutX.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutX_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=2]
  %OutY.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %OutY_PitchInFloat.addr = alloca i32, align 4   ; <i32*> [#uses=2]
  %ROIWidth.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %ROIHeight.addr = alloca i32, align 4           ; <i32*> [#uses=1]
  %x = alloca i32, align 4                        ; <i32*> [#uses=0]
  %y = alloca i32, align 4                        ; <i32*> [#uses=0]
  %x0 = alloca i32, align 4                       ; <i32*> [#uses=11]
  %y0 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %pSrc = alloca float addrspace(1)*, align 4     ; <float addrspace(1)**> [#uses=9]
  %pDstX = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=2]
  %pDstY = alloca float addrspace(1)*, align 4    ; <float addrspace(1)**> [#uses=2]
  %v00 = alloca float, align 4                    ; <float*> [#uses=3]
  %v01 = alloca float, align 4                    ; <float*> [#uses=2]
  %v02 = alloca float, align 4                    ; <float*> [#uses=3]
  %v10 = alloca float, align 4                    ; <float*> [#uses=2]
  %v12 = alloca float, align 4                    ; <float*> [#uses=2]
  %v20 = alloca float, align 4                    ; <float*> [#uses=3]
  %v21 = alloca float, align 4                    ; <float*> [#uses=2]
  %v22 = alloca float, align 4                    ; <float*> [#uses=3]
  store float addrspace(1)* %Image, float addrspace(1)** %Image.addr
  store i32 %Image_PitchInFloat, i32* %Image_PitchInFloat.addr
  store i32 %Image_W, i32* %Image_W.addr
  store i32 %Image_H, i32* %Image_H.addr
  store i32 %Image_BW, i32* %Image_BW.addr
  store i32 %Image_BH, i32* %Image_BH.addr
  store float addrspace(1)* %OutX, float addrspace(1)** %OutX.addr
  store i32 %OutX_PitchInFloat, i32* %OutX_PitchInFloat.addr
  store float addrspace(1)* %OutY, float addrspace(1)** %OutY.addr
  store i32 %OutY_PitchInFloat, i32* %OutY_PitchInFloat.addr
  store i32 %ROIWidth, i32* %ROIWidth.addr
  store i32 %ROIHeight, i32* %ROIHeight.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %x0
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %y0
  %tmp = load float addrspace(1)** %Image.addr    ; <float addrspace(1)*> [#uses=1]
  %tmp2 = load i32* %Image_PitchInFloat.addr      ; <i32> [#uses=1]
  %tmp3 = load i32* %Image_BH.addr                ; <i32> [#uses=1]
  %sub = sub i32 %tmp3, 1                         ; <i32> [#uses=1]
  %tmp4 = load i32* %y0                           ; <i32> [#uses=1]
  %add = add nsw i32 %sub, %tmp4                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp2, %add                      ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(1)* %tmp, i32 %mul ; <float addrspace(1)*> [#uses=1]
  %tmp5 = load i32* %Image_BW.addr                ; <i32> [#uses=1]
  %add.ptr6 = getelementptr inbounds float addrspace(1)* %add.ptr, i32 %tmp5 ; <float addrspace(1)*> [#uses=1]
  %sub.ptr = getelementptr inbounds float addrspace(1)* %add.ptr6, i32 -1 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %sub.ptr, float addrspace(1)** %pSrc
  %tmp8 = load float addrspace(1)** %OutX.addr    ; <float addrspace(1)*> [#uses=1]
  %tmp9 = load i32* %y0                           ; <i32> [#uses=1]
  %tmp10 = load i32* %OutX_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul11 = mul i32 %tmp9, %tmp10                  ; <i32> [#uses=1]
  %add.ptr12 = getelementptr inbounds float addrspace(1)* %tmp8, i32 %mul11 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr12, float addrspace(1)** %pDstX
  %tmp14 = load float addrspace(1)** %OutY.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load i32* %y0                          ; <i32> [#uses=1]
  %tmp16 = load i32* %OutY_PitchInFloat.addr      ; <i32> [#uses=1]
  %mul17 = mul i32 %tmp15, %tmp16                 ; <i32> [#uses=1]
  %add.ptr18 = getelementptr inbounds float addrspace(1)* %tmp14, i32 %mul17 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr18, float addrspace(1)** %pDstY
  %tmp20 = load i32* %x0                          ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp20, 0                  ; <i32> [#uses=1]
  %tmp22 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp22, i32 %add21 ; <float addrspace(1)*> [#uses=1]
  %tmp23 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  store float %tmp23, float* %v00
  %tmp25 = load i32* %x0                          ; <i32> [#uses=1]
  %add26 = add nsw i32 %tmp25, 1                  ; <i32> [#uses=1]
  %tmp27 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx28 = getelementptr inbounds float addrspace(1)* %tmp27, i32 %add26 ; <float addrspace(1)*> [#uses=1]
  %tmp29 = load float addrspace(1)* %arrayidx28   ; <float> [#uses=1]
  store float %tmp29, float* %v01
  %tmp31 = load i32* %x0                          ; <i32> [#uses=1]
  %add32 = add nsw i32 %tmp31, 2                  ; <i32> [#uses=1]
  %tmp33 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 %add32 ; <float addrspace(1)*> [#uses=1]
  %tmp35 = load float addrspace(1)* %arrayidx34   ; <float> [#uses=1]
  store float %tmp35, float* %v02
  %tmp37 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp38 = load i32* %x0                          ; <i32> [#uses=1]
  %add39 = add nsw i32 %tmp37, %tmp38             ; <i32> [#uses=1]
  %add40 = add nsw i32 %add39, 0                  ; <i32> [#uses=1]
  %tmp41 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx42 = getelementptr inbounds float addrspace(1)* %tmp41, i32 %add40 ; <float addrspace(1)*> [#uses=1]
  %tmp43 = load float addrspace(1)* %arrayidx42   ; <float> [#uses=1]
  store float %tmp43, float* %v10
  %tmp45 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %tmp46 = load i32* %x0                          ; <i32> [#uses=1]
  %add47 = add nsw i32 %tmp45, %tmp46             ; <i32> [#uses=1]
  %add48 = add nsw i32 %add47, 2                  ; <i32> [#uses=1]
  %tmp49 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx50 = getelementptr inbounds float addrspace(1)* %tmp49, i32 %add48 ; <float addrspace(1)*> [#uses=1]
  %tmp51 = load float addrspace(1)* %arrayidx50   ; <float> [#uses=1]
  store float %tmp51, float* %v12
  %tmp53 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %mul54 = mul i32 2, %tmp53                      ; <i32> [#uses=1]
  %tmp55 = load i32* %x0                          ; <i32> [#uses=1]
  %add56 = add nsw i32 %mul54, %tmp55             ; <i32> [#uses=1]
  %add57 = add nsw i32 %add56, 0                  ; <i32> [#uses=1]
  %tmp58 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx59 = getelementptr inbounds float addrspace(1)* %tmp58, i32 %add57 ; <float addrspace(1)*> [#uses=1]
  %tmp60 = load float addrspace(1)* %arrayidx59   ; <float> [#uses=1]
  store float %tmp60, float* %v20
  %tmp62 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %mul63 = mul i32 2, %tmp62                      ; <i32> [#uses=1]
  %tmp64 = load i32* %x0                          ; <i32> [#uses=1]
  %add65 = add nsw i32 %mul63, %tmp64             ; <i32> [#uses=1]
  %add66 = add nsw i32 %add65, 1                  ; <i32> [#uses=1]
  %tmp67 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx68 = getelementptr inbounds float addrspace(1)* %tmp67, i32 %add66 ; <float addrspace(1)*> [#uses=1]
  %tmp69 = load float addrspace(1)* %arrayidx68   ; <float> [#uses=1]
  store float %tmp69, float* %v21
  %tmp71 = load i32* %Image_PitchInFloat.addr     ; <i32> [#uses=1]
  %mul72 = mul i32 2, %tmp71                      ; <i32> [#uses=1]
  %tmp73 = load i32* %x0                          ; <i32> [#uses=1]
  %add74 = add nsw i32 %mul72, %tmp73             ; <i32> [#uses=1]
  %add75 = add nsw i32 %add74, 2                  ; <i32> [#uses=1]
  %tmp76 = load float addrspace(1)** %pSrc        ; <float addrspace(1)*> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(1)* %tmp76, i32 %add75 ; <float addrspace(1)*> [#uses=1]
  %tmp78 = load float addrspace(1)* %arrayidx77   ; <float> [#uses=1]
  store float %tmp78, float* %v22
  %tmp79 = load float* %v02                       ; <float> [#uses=1]
  %tmp80 = load float* %v00                       ; <float> [#uses=1]
  %sub81 = fsub float %tmp79, %tmp80              ; <float> [#uses=1]
  %tmp82 = load float* %v12                       ; <float> [#uses=1]
  %tmp83 = load float* %v10                       ; <float> [#uses=1]
  %sub84 = fsub float %tmp82, %tmp83              ; <float> [#uses=1]
  %mul85 = fmul float 2.000000e+000, %sub84       ; <float> [#uses=1]
  %add86 = fadd float %sub81, %mul85              ; <float> [#uses=1]
  %tmp87 = load float* %v22                       ; <float> [#uses=1]
  %tmp88 = load float* %v20                       ; <float> [#uses=1]
  %sub89 = fsub float %tmp87, %tmp88              ; <float> [#uses=1]
  %add90 = fadd float %add86, %sub89              ; <float> [#uses=1]
  %tmp91 = load i32* %x0                          ; <i32> [#uses=1]
  %tmp92 = load float addrspace(1)** %pDstX       ; <float addrspace(1)*> [#uses=1]
  %arrayidx93 = getelementptr inbounds float addrspace(1)* %tmp92, i32 %tmp91 ; <float addrspace(1)*> [#uses=1]
  store float %add90, float addrspace(1)* %arrayidx93
  %tmp94 = load float* %v20                       ; <float> [#uses=1]
  %tmp95 = load float* %v00                       ; <float> [#uses=1]
  %sub96 = fsub float %tmp94, %tmp95              ; <float> [#uses=1]
  %tmp97 = load float* %v21                       ; <float> [#uses=1]
  %tmp98 = load float* %v01                       ; <float> [#uses=1]
  %sub99 = fsub float %tmp97, %tmp98              ; <float> [#uses=1]
  %mul100 = fmul float 2.000000e+000, %sub99      ; <float> [#uses=1]
  %add101 = fadd float %sub96, %mul100            ; <float> [#uses=1]
  %tmp102 = load float* %v22                      ; <float> [#uses=1]
  %tmp103 = load float* %v02                      ; <float> [#uses=1]
  %sub104 = fsub float %tmp102, %tmp103           ; <float> [#uses=1]
  %add105 = fadd float %add101, %sub104           ; <float> [#uses=1]
  %tmp106 = load i32* %x0                         ; <i32> [#uses=1]
  %tmp107 = load float addrspace(1)** %pDstY      ; <float addrspace(1)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds float addrspace(1)* %tmp107, i32 %tmp106 ; <float addrspace(1)*> [#uses=1]
  store float %add105, float addrspace(1)* %arrayidx108
  ret void
}
