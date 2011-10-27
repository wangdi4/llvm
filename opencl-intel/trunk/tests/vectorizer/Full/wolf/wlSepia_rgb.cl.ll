; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlSepia_rgb.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@sepia_clamp.transMat = internal constant [4 x [4 x float]] [[4 x float] [float 0x3FD926E980000000, float 0x3FE89BA5E0000000, float 0x3FC83126E0000000, float 0.000000e+000], [4 x float] [float 0x3FD6560420000000, float 0x3FE5F3B640000000, float 0x3FC5810620000000, float 0.000000e+000], [4 x float] [float 0x3FD16872C0000000, float 0x3FE1168720000000, float 0x3FC0C49BA0000000, float 0.000000e+000], [4 x float] [float 0.000000e+000, float 0.000000e+000, float 0.000000e+000, float 1.000000e+000]], align 4 ; <[4 x [4 x float]]*> [#uses=1]
@opencl_sepia_clamp_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_clamp_parameters = appending global [91 x i8] c"float __attribute__((address_space(2))) *, float __attribute__((address_space(1))) *, uint\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(2)*, float addrspace(1)*, i32)* @sepia_clamp to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_clamp_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_sepia_clamp_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @sepia_clamp(float addrspace(2)* %inputImage, float addrspace(1)* %outputImage, i32 %wgRowSize) nounwind {
entry:
  %inputImage.addr = alloca float addrspace(2)*, align 4 ; <float addrspace(2)**> [#uses=2]
  %outputImage.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %wgRowSize.addr = alloca i32, align 4           ; <i32*> [#uses=3]
  %wgNum = alloca i32, align 4                    ; <i32*> [#uses=2]
  %wgRowOffset = alloca i32, align 4              ; <i32*> [#uses=3]
  %wgNumOf = alloca i32, align 4                  ; <i32*> [#uses=1]
  %pixelComponentNum = alloca i32, align 4        ; <i32*> [#uses=5]
  %transMat = alloca [4 x [4 x float]], align 4   ; <[4 x [4 x float]]*> [#uses=2]
  %dotTmp = alloca float, align 4                 ; <float*> [#uses=5]
  %imagePixelWidth = alloca i32, align 4          ; <i32*> [#uses=4]
  %irow = alloca i32, align 4                     ; <i32*> [#uses=6]
  %icol = alloca i32, align 4                     ; <i32*> [#uses=6]
  %iOutpixcomp = alloca i32, align 4              ; <i32*> [#uses=6]
  %iInpixcomp = alloca i32, align 4               ; <i32*> [#uses=6]
  store float addrspace(2)* %inputImage, float addrspace(2)** %inputImage.addr
  store float addrspace(1)* %outputImage, float addrspace(1)** %outputImage.addr
  store i32 %wgRowSize, i32* %wgRowSize.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %wgNum
  %tmp = load i32* %wgRowSize.addr                ; <i32> [#uses=1]
  %tmp1 = load i32* %wgNum                        ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %wgRowOffset
  %call3 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call3, i32* %wgNumOf
  store i32 4, i32* %pixelComponentNum
  %tmp6 = bitcast [4 x [4 x float]]* %transMat to i8* ; <i8*> [#uses=1]
  call void @llvm.memcpy.i32(i8* %tmp6, i8* bitcast ([4 x [4 x float]]* @sepia_clamp.transMat to i8*), i32 64, i32 4)
  store i32 1024, i32* %imagePixelWidth
  store i32 0, i32* %irow
  br label %for.cond

for.cond:                                         ; preds = %for.inc76, %entry
  %tmp10 = load i32* %irow                        ; <i32> [#uses=1]
  %tmp11 = load i32* %wgRowSize.addr              ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp10, %tmp11              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end79

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %icol
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc72, %for.body
  %tmp14 = load i32* %icol                        ; <i32> [#uses=1]
  %tmp15 = load i32* %imagePixelWidth             ; <i32> [#uses=1]
  %cmp16 = icmp slt i32 %tmp14, %tmp15            ; <i1> [#uses=1]
  br i1 %cmp16, label %for.body17, label %for.end75

for.body17:                                       ; preds = %for.cond13
  store i32 0, i32* %iOutpixcomp
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc68, %for.body17
  %tmp20 = load i32* %iOutpixcomp                 ; <i32> [#uses=1]
  %tmp21 = load i32* %pixelComponentNum           ; <i32> [#uses=1]
  %cmp22 = icmp ult i32 %tmp20, %tmp21            ; <i1> [#uses=1]
  br i1 %cmp22, label %for.body23, label %for.end71

for.body23:                                       ; preds = %for.cond19
  store float 0.000000e+000, float* %dotTmp
  store i32 0, i32* %iInpixcomp
  br label %for.cond25

for.cond25:                                       ; preds = %for.inc, %for.body23
  %tmp26 = load i32* %iInpixcomp                  ; <i32> [#uses=1]
  %tmp27 = load i32* %pixelComponentNum           ; <i32> [#uses=1]
  %cmp28 = icmp ult i32 %tmp26, %tmp27            ; <i1> [#uses=1]
  br i1 %cmp28, label %for.body29, label %for.end

for.body29:                                       ; preds = %for.cond25
  %tmp30 = load i32* %iInpixcomp                  ; <i32> [#uses=1]
  %tmp31 = load i32* %iOutpixcomp                 ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [4 x [4 x float]]* %transMat, i32 0, i32 0 ; <[4 x float]*> [#uses=1]
  %arrayidx = getelementptr inbounds [4 x float]* %arraydecay, i32 %tmp31 ; <[4 x float]*> [#uses=1]
  %arraydecay32 = getelementptr inbounds [4 x float]* %arrayidx, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx33 = getelementptr inbounds float* %arraydecay32, i32 %tmp30 ; <float*> [#uses=1]
  %tmp34 = load float* %arrayidx33                ; <float> [#uses=1]
  %tmp35 = load i32* %wgRowOffset                 ; <i32> [#uses=1]
  %tmp36 = load i32* %irow                        ; <i32> [#uses=1]
  %add = add i32 %tmp35, %tmp36                   ; <i32> [#uses=1]
  %tmp37 = load i32* %imagePixelWidth             ; <i32> [#uses=1]
  %mul38 = mul i32 %add, %tmp37                   ; <i32> [#uses=1]
  %tmp39 = load i32* %icol                        ; <i32> [#uses=1]
  %add40 = add i32 %mul38, %tmp39                 ; <i32> [#uses=1]
  %tmp41 = load i32* %pixelComponentNum           ; <i32> [#uses=1]
  %mul42 = mul i32 %add40, %tmp41                 ; <i32> [#uses=1]
  %tmp43 = load i32* %iInpixcomp                  ; <i32> [#uses=1]
  %add44 = add i32 %mul42, %tmp43                 ; <i32> [#uses=1]
  %tmp45 = load float addrspace(2)** %inputImage.addr ; <float addrspace(2)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float addrspace(2)* %tmp45, i32 %add44 ; <float addrspace(2)*> [#uses=1]
  %tmp47 = load float addrspace(2)* %arrayidx46   ; <float> [#uses=1]
  %mul48 = fmul float %tmp34, %tmp47              ; <float> [#uses=1]
  %tmp49 = load float* %dotTmp                    ; <float> [#uses=1]
  %add50 = fadd float %tmp49, %mul48              ; <float> [#uses=1]
  store float %add50, float* %dotTmp
  br label %for.inc

for.inc:                                          ; preds = %for.body29
  %tmp51 = load i32* %iInpixcomp                  ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp51, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %iInpixcomp
  br label %for.cond25

for.end:                                          ; preds = %for.cond25
  %tmp52 = load float* %dotTmp                    ; <float> [#uses=1]
  %cmp53 = fcmp olt float %tmp52, 1.000000e+000   ; <i1> [#uses=1]
  %tmp54 = load float* %dotTmp                    ; <float> [#uses=1]
  %cond = select i1 %cmp53, float %tmp54, float 1.000000e+000 ; <float> [#uses=1]
  %tmp55 = load i32* %wgRowOffset                 ; <i32> [#uses=1]
  %tmp56 = load i32* %irow                        ; <i32> [#uses=1]
  %add57 = add i32 %tmp55, %tmp56                 ; <i32> [#uses=1]
  %tmp58 = load i32* %imagePixelWidth             ; <i32> [#uses=1]
  %mul59 = mul i32 %add57, %tmp58                 ; <i32> [#uses=1]
  %tmp60 = load i32* %icol                        ; <i32> [#uses=1]
  %add61 = add i32 %mul59, %tmp60                 ; <i32> [#uses=1]
  %tmp62 = load i32* %pixelComponentNum           ; <i32> [#uses=1]
  %mul63 = mul i32 %add61, %tmp62                 ; <i32> [#uses=1]
  %tmp64 = load i32* %iOutpixcomp                 ; <i32> [#uses=1]
  %add65 = add i32 %mul63, %tmp64                 ; <i32> [#uses=1]
  %tmp66 = load float addrspace(1)** %outputImage.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx67 = getelementptr inbounds float addrspace(1)* %tmp66, i32 %add65 ; <float addrspace(1)*> [#uses=1]
  store float %cond, float addrspace(1)* %arrayidx67
  br label %for.inc68

for.inc68:                                        ; preds = %for.end
  %tmp69 = load i32* %iOutpixcomp                 ; <i32> [#uses=1]
  %inc70 = add nsw i32 %tmp69, 1                  ; <i32> [#uses=1]
  store i32 %inc70, i32* %iOutpixcomp
  br label %for.cond19

for.end71:                                        ; preds = %for.cond19
  br label %for.inc72

for.inc72:                                        ; preds = %for.end71
  %tmp73 = load i32* %icol                        ; <i32> [#uses=1]
  %inc74 = add nsw i32 %tmp73, 1                  ; <i32> [#uses=1]
  store i32 %inc74, i32* %icol
  br label %for.cond13

for.end75:                                        ; preds = %for.cond13
  br label %for.inc76

for.inc76:                                        ; preds = %for.end75
  %tmp77 = load i32* %irow                        ; <i32> [#uses=1]
  %inc78 = add nsw i32 %tmp77, 1                  ; <i32> [#uses=1]
  store i32 %inc78, i32* %irow
  br label %for.cond

for.end79:                                        ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare void @llvm.memcpy.i32(i8* nocapture, i8* nocapture, i32, i32) nounwind
