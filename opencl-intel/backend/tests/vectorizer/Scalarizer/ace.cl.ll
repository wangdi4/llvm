; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\ace.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type { [8 x i16], [8 x i16], [8 x i16], i32 }

@g_pui16HistBrightX = constant [8 x i16] [i16 0, i16 321, i16 582, i16 770, i16 1023, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16HistBrightY = constant [8 x i16] [i16 200, i16 140, i16 480, i16 784, i16 940, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16HistDarkX = constant [8 x i16] [i16 0, i16 481, i16 762, i16 818, i16 1023, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16HistDarkY = constant [8 x i16] [i16 200, i16 100, i16 360, i16 740, i16 940, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16BrCorrBrightX = constant [8 x i16] [i16 200, i16 232, i16 628, i16 940, i16 0, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16BrCorrBrightY = constant [8 x i16] [i16 200, i16 352, i16 748, i16 940, i16 0, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16BrCorrDarkX = constant [8 x i16] [i16 200, i16 352, i16 748, i16 940, i16 0, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@g_pui16BrCorrDarkY = constant [8 x i16] [i16 200, i16 232, i16 628, i16 940, i16 0, i16 0, i16 0, i16 0], align 2 ; <[8 x i16]*> [#uses=1]
@zeroVec = global <8 x i16> zeroinitializer, align 16 ; <<8 x i16>*> [#uses=0]
@signChangerVec = global <8 x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, align 16 ; <<8 x i16>*> [#uses=0]
@maxYVec = global <8 x i16> <i16 940, i16 940, i16 940, i16 940, i16 940, i16 940, i16 940, i16 940>, align 16 ; <<8 x i16>*> [#uses=1]
@minYVec = global <8 x i16> <i16 200, i16 200, i16 200, i16 200, i16 200, i16 200, i16 200, i16 200>, align 16 ; <<8 x i16>*> [#uses=1]
@generateOutputHistogram.rangeMask = internal constant [8 x i16] [i16 0, i16 -1, i16 -1, i16 -1, i16 -1, i16 0, i16 0, i16 0] ; <[8 x i16]*> [#uses=1]
@opencl_calculateHistogram_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_calculateHistogram_parameters = appending global [123 x i8] c"uint, uint, uint, uint, uint, uint, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[123 x i8]*> [#uses=1]
@opencl_gatherHistograms_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_gatherHistograms_parameters = appending global [129 x i8] c"uint, uint, uint, uint, uint, uint, uint, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[129 x i8]*> [#uses=1]
@opencl_gatherHistograms_vector_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_gatherHistograms_vector_parameters = appending global [129 x i8] c"uint, uint, uint, uint, uint, uint, uint, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[129 x i8]*> [#uses=1]
@opencl_generateOutputHistogram_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_generateOutputHistogram_parameters = appending global [231 x i8] c"uint, uint, ushort __attribute__((address_space(1))) *, AceLut __attribute__((address_space(1))) *, AceLut __attribute__((address_space(1))) *, AceLut __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[231 x i8]*> [#uses=1]
@opencl_applyLut_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_applyLut_parameters = appending global [143 x i8] c"uint, uint, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *, ushort __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[143 x i8]*> [#uses=1]
@opencl_metadata = appending global [5 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32, i32, i32, i32, i32, i32, i16 addrspace(1)*, i16 addrspace(1)*)* @calculateHistogram to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_calculateHistogram_locals to i8*), i8* getelementptr inbounds ([123 x i8]* @opencl_calculateHistogram_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, i32, i32, i32, i32, i32, i32, i16 addrspace(1)*, i16 addrspace(1)*)* @gatherHistograms to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_gatherHistograms_locals to i8*), i8* getelementptr inbounds ([129 x i8]* @opencl_gatherHistograms_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, i32, i32, i32, i32, i32, i32, i16 addrspace(1)*, i16 addrspace(1)*)* @gatherHistograms_vector to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_gatherHistograms_vector_locals to i8*), i8* getelementptr inbounds ([129 x i8]* @opencl_gatherHistograms_vector_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, i32, i16 addrspace(1)*, %struct.anon addrspace(1)*, %struct.anon addrspace(1)*, %struct.anon addrspace(1)*, i16 addrspace(1)*)* @generateOutputHistogram to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_generateOutputHistogram_locals to i8*), i8* getelementptr inbounds ([231 x i8]* @opencl_generateOutputHistogram_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, i32, i16 addrspace(1)*, i16 addrspace(1)*, i16 addrspace(1)*)* @applyLut to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_applyLut_locals to i8*), i8* getelementptr inbounds ([143 x i8]* @opencl_applyLut_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[5 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @kernelMemset(i8 addrspace(1)* %dst, i32 %size, i8 zeroext %val) nounwind {
entry:
  %dst.addr = alloca i8 addrspace(1)*, align 4    ; <i8 addrspace(1)**> [#uses=2]
  %size.addr = alloca i32, align 4                ; <i32*> [#uses=2]
  %val.addr = alloca i8, align 1                  ; <i8*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %ptr = alloca i8 addrspace(1)*, align 4         ; <i8 addrspace(1)**> [#uses=2]
  store i8 addrspace(1)* %dst, i8 addrspace(1)** %dst.addr
  store i32 %size, i32* %size.addr
  store i8 %val, i8* %val.addr
  %tmp = load i8 addrspace(1)** %dst.addr         ; <i8 addrspace(1)*> [#uses=1]
  store i8 addrspace(1)* %tmp, i8 addrspace(1)** %ptr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp1 = load i32* %i                            ; <i32> [#uses=1]
  %tmp2 = load i32* %size.addr                    ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp1, %tmp2                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp3 = load i8* %val.addr                      ; <i8> [#uses=1]
  %tmp4 = load i32* %i                            ; <i32> [#uses=1]
  %tmp5 = load i8 addrspace(1)** %ptr             ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %tmp5, i32 %tmp4 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %tmp3, i8 addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %inc = add i32 %tmp6, 1                         ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define void @kernelMemcpy(i8 addrspace(1)* %dst, i8 addrspace(1)* %src, i32 %size) nounwind {
entry:
  %dst.addr = alloca i8 addrspace(1)*, align 4    ; <i8 addrspace(1)**> [#uses=2]
  %src.addr = alloca i8 addrspace(1)*, align 4    ; <i8 addrspace(1)**> [#uses=2]
  %size.addr = alloca i32, align 4                ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %dstPtr = alloca i8 addrspace(1)*, align 4      ; <i8 addrspace(1)**> [#uses=2]
  %srcPtr = alloca i8 addrspace(1)*, align 4      ; <i8 addrspace(1)**> [#uses=2]
  store i8 addrspace(1)* %dst, i8 addrspace(1)** %dst.addr
  store i8 addrspace(1)* %src, i8 addrspace(1)** %src.addr
  store i32 %size, i32* %size.addr
  %tmp = load i8 addrspace(1)** %dst.addr         ; <i8 addrspace(1)*> [#uses=1]
  store i8 addrspace(1)* %tmp, i8 addrspace(1)** %dstPtr
  %tmp2 = load i8 addrspace(1)** %src.addr        ; <i8 addrspace(1)*> [#uses=1]
  store i8 addrspace(1)* %tmp2, i8 addrspace(1)** %srcPtr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %tmp4 = load i32* %size.addr                    ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp3, %tmp4                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  %tmp6 = load i8 addrspace(1)** %srcPtr          ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %tmp6, i32 %tmp5 ; <i8 addrspace(1)*> [#uses=1]
  %tmp7 = load i8 addrspace(1)* %arrayidx         ; <i8> [#uses=1]
  %tmp8 = load i32* %i                            ; <i32> [#uses=1]
  %tmp9 = load i8 addrspace(1)** %dstPtr          ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx10 = getelementptr inbounds i8 addrspace(1)* %tmp9, i32 %tmp8 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %tmp7, i8 addrspace(1)* %arrayidx10
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp11 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add i32 %tmp11, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %val1, i16 zeroext %val2, i16 zeroext %ui16Alpha) nounwind {
entry:
  %retval = alloca i16, align 2                   ; <i16*> [#uses=3]
  %val1.addr = alloca i16, align 2                ; <i16*> [#uses=3]
  %val2.addr = alloca i16, align 2                ; <i16*> [#uses=3]
  %ui16Alpha.addr = alloca i16, align 2           ; <i16*> [#uses=3]
  %ui16MaxAlphaVal = alloca i16, align 2          ; <i16*> [#uses=2]
  %diff = alloca i16, align 2                     ; <i16*> [#uses=2]
  %shifted = alloca i16, align 2                  ; <i16*> [#uses=2]
  %mul = alloca i32, align 4                      ; <i32*> [#uses=2]
  %rounded = alloca i32, align 4                  ; <i32*> [#uses=2]
  store i16 %val1, i16* %val1.addr
  store i16 %val2, i16* %val2.addr
  store i16 %ui16Alpha, i16* %ui16Alpha.addr
  store i16 255, i16* %ui16MaxAlphaVal
  %tmp = load i16* %ui16Alpha.addr                ; <i16> [#uses=1]
  %conv = zext i16 %tmp to i32                    ; <i32> [#uses=1]
  %tmp1 = load i16* %ui16MaxAlphaVal              ; <i16> [#uses=1]
  %conv2 = zext i16 %tmp1 to i32                  ; <i32> [#uses=1]
  %cmp = icmp sge i32 %conv, %conv2               ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp4 = load i16* %val1.addr                    ; <i16> [#uses=1]
  store i16 %tmp4, i16* %retval
  br label %return

if.end:                                           ; preds = %entry
  %tmp5 = load i16* %val1.addr                    ; <i16> [#uses=1]
  %conv6 = zext i16 %tmp5 to i32                  ; <i32> [#uses=1]
  %tmp7 = load i16* %val2.addr                    ; <i16> [#uses=1]
  %conv8 = zext i16 %tmp7 to i32                  ; <i32> [#uses=1]
  %sub = sub i32 %conv6, %conv8                   ; <i32> [#uses=1]
  %conv9 = trunc i32 %sub to i16                  ; <i16> [#uses=1]
  store i16 %conv9, i16* %diff
  %tmp10 = load i16* %diff                        ; <i16> [#uses=1]
  %conv11 = sext i16 %tmp10 to i32                ; <i32> [#uses=1]
  %tmp12 = load i16* %ui16Alpha.addr              ; <i16> [#uses=1]
  %conv13 = zext i16 %tmp12 to i32                ; <i32> [#uses=1]
  %mul14 = mul i32 %conv11, %conv13               ; <i32> [#uses=1]
  store i32 %mul14, i32* %mul
  %tmp15 = load i32* %mul                         ; <i32> [#uses=1]
  %add = add nsw i32 %tmp15, 128                  ; <i32> [#uses=1]
  store i32 %add, i32* %rounded
  %tmp16 = load i32* %rounded                     ; <i32> [#uses=1]
  %shr = ashr i32 %tmp16, 8                       ; <i32> [#uses=1]
  %conv17 = trunc i32 %shr to i16                 ; <i16> [#uses=1]
  store i16 %conv17, i16* %shifted
  %tmp18 = load i16* %shifted                     ; <i16> [#uses=1]
  %conv19 = sext i16 %tmp18 to i32                ; <i32> [#uses=1]
  %tmp20 = load i16* %val2.addr                   ; <i16> [#uses=1]
  %conv21 = zext i16 %tmp20 to i32                ; <i32> [#uses=1]
  %add22 = add nsw i32 %conv19, %conv21           ; <i32> [#uses=1]
  %conv23 = trunc i32 %add22 to i16               ; <i16> [#uses=1]
  store i16 %conv23, i16* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %0 = load i16* %retval                          ; <i16> [#uses=1]
  ret i16 %0
}

define <8 x i16> @AlphaBlendRound_Alpha8Bit_vector(<8 x i16> %val1, <8 x i16> %val2, i16 zeroext %ui16Alpha) nounwind {
entry:
  %retval = alloca <8 x i16>, align 16            ; <<8 x i16>*> [#uses=3]
  %val1.addr = alloca <8 x i16>, align 16         ; <<8 x i16>*> [#uses=3]
  %val2.addr = alloca <8 x i16>, align 16         ; <<8 x i16>*> [#uses=3]
  %ui16Alpha.addr = alloca i16, align 2           ; <i16*> [#uses=3]
  %ui16MaxAlphaVal = alloca i16, align 2          ; <i16*> [#uses=2]
  %diff = alloca <8 x i16>, align 16              ; <<8 x i16>*> [#uses=9]
  %shifted = alloca <8 x i16>, align 16           ; <<8 x i16>*> [#uses=9]
  %mul = alloca <8 x i32>, align 32               ; <<8 x i32>*> [#uses=2]
  %rounded = alloca <8 x i32>, align 32           ; <<8 x i32>*> [#uses=11]
  %.compoundliteral = alloca <8 x i32>, align 32  ; <<8 x i32>*> [#uses=2]
  %.compoundliteral44 = alloca <8 x i32>, align 32 ; <<8 x i32>*> [#uses=2]
  %.compoundliteral47 = alloca <8 x i16>, align 16 ; <<8 x i16>*> [#uses=2]
  %.compoundliteral81 = alloca <8 x i16>, align 16 ; <<8 x i16>*> [#uses=2]
  store <8 x i16> %val1, <8 x i16>* %val1.addr
  store <8 x i16> %val2, <8 x i16>* %val2.addr
  store i16 %ui16Alpha, i16* %ui16Alpha.addr
  store i16 255, i16* %ui16MaxAlphaVal
  %tmp = load i16* %ui16Alpha.addr                ; <i16> [#uses=1]
  %conv = zext i16 %tmp to i32                    ; <i32> [#uses=1]
  %tmp1 = load i16* %ui16MaxAlphaVal              ; <i16> [#uses=1]
  %conv2 = zext i16 %tmp1 to i32                  ; <i32> [#uses=1]
  %cmp = icmp sge i32 %conv, %conv2               ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp4 = load <8 x i16>* %val1.addr              ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp4, <8 x i16>* %retval
  br label %return

if.end:                                           ; preds = %entry
  %tmp5 = load <8 x i16>* %val1.addr              ; <<8 x i16>> [#uses=1]
  %tmp6 = load <8 x i16>* %val2.addr              ; <<8 x i16>> [#uses=1]
  %sub = sub <8 x i16> %tmp5, %tmp6               ; <<8 x i16>> [#uses=1]
  store <8 x i16> %sub, <8 x i16>* %diff
  %tmp7 = load <8 x i16>* %diff                   ; <<8 x i16>> [#uses=1]
  %tmp8 = extractelement <8 x i16> %tmp7, i32 0   ; <i16> [#uses=1]
  %conv9 = zext i16 %tmp8 to i32                  ; <i32> [#uses=1]
  %vecinit = insertelement <8 x i32> undef, i32 %conv9, i32 0 ; <<8 x i32>> [#uses=1]
  %tmp10 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp11 = extractelement <8 x i16> %tmp10, i32 1 ; <i16> [#uses=1]
  %conv12 = zext i16 %tmp11 to i32                ; <i32> [#uses=1]
  %vecinit13 = insertelement <8 x i32> %vecinit, i32 %conv12, i32 1 ; <<8 x i32>> [#uses=1]
  %tmp14 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp15 = extractelement <8 x i16> %tmp14, i32 2 ; <i16> [#uses=1]
  %conv16 = zext i16 %tmp15 to i32                ; <i32> [#uses=1]
  %vecinit17 = insertelement <8 x i32> %vecinit13, i32 %conv16, i32 2 ; <<8 x i32>> [#uses=1]
  %tmp18 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp19 = extractelement <8 x i16> %tmp18, i32 3 ; <i16> [#uses=1]
  %conv20 = zext i16 %tmp19 to i32                ; <i32> [#uses=1]
  %vecinit21 = insertelement <8 x i32> %vecinit17, i32 %conv20, i32 3 ; <<8 x i32>> [#uses=1]
  %tmp22 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp23 = extractelement <8 x i16> %tmp22, i32 4 ; <i16> [#uses=1]
  %conv24 = zext i16 %tmp23 to i32                ; <i32> [#uses=1]
  %vecinit25 = insertelement <8 x i32> %vecinit21, i32 %conv24, i32 4 ; <<8 x i32>> [#uses=1]
  %tmp26 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp27 = extractelement <8 x i16> %tmp26, i32 5 ; <i16> [#uses=1]
  %conv28 = zext i16 %tmp27 to i32                ; <i32> [#uses=1]
  %vecinit29 = insertelement <8 x i32> %vecinit25, i32 %conv28, i32 5 ; <<8 x i32>> [#uses=1]
  %tmp30 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp31 = extractelement <8 x i16> %tmp30, i32 6 ; <i16> [#uses=1]
  %conv32 = zext i16 %tmp31 to i32                ; <i32> [#uses=1]
  %vecinit33 = insertelement <8 x i32> %vecinit29, i32 %conv32, i32 6 ; <<8 x i32>> [#uses=1]
  %tmp34 = load <8 x i16>* %diff                  ; <<8 x i16>> [#uses=1]
  %tmp35 = extractelement <8 x i16> %tmp34, i32 7 ; <i16> [#uses=1]
  %conv36 = zext i16 %tmp35 to i32                ; <i32> [#uses=1]
  %vecinit37 = insertelement <8 x i32> %vecinit33, i32 %conv36, i32 7 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %vecinit37, <8 x i32>* %.compoundliteral
  %tmp38 = load <8 x i32>* %.compoundliteral      ; <<8 x i32>> [#uses=1]
  %tmp39 = load i16* %ui16Alpha.addr              ; <i16> [#uses=1]
  %conv40 = zext i16 %tmp39 to i32                ; <i32> [#uses=1]
  %tmp41 = insertelement <8 x i32> undef, i32 %conv40, i32 0 ; <<8 x i32>> [#uses=2]
  %splat = shufflevector <8 x i32> %tmp41, <8 x i32> %tmp41, <8 x i32> zeroinitializer ; <<8 x i32>> [#uses=1]
  %mul42 = mul <8 x i32> %tmp38, %splat           ; <<8 x i32>> [#uses=1]
  store <8 x i32> %mul42, <8 x i32>* %mul
  %tmp43 = load <8 x i32>* %mul                   ; <<8 x i32>> [#uses=1]
  store <8 x i32> <i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128>, <8 x i32>* %.compoundliteral44
  %tmp45 = load <8 x i32>* %.compoundliteral44    ; <<8 x i32>> [#uses=1]
  %add = add nsw <8 x i32> %tmp43, %tmp45         ; <<8 x i32>> [#uses=1]
  store <8 x i32> %add, <8 x i32>* %rounded
  %tmp46 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %shr = ashr <8 x i32> %tmp46, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8> ; <<8 x i32>> [#uses=1]
  store <8 x i32> %shr, <8 x i32>* %rounded
  %tmp48 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp49 = extractelement <8 x i32> %tmp48, i32 0 ; <i32> [#uses=1]
  %conv50 = trunc i32 %tmp49 to i16               ; <i16> [#uses=1]
  %vecinit51 = insertelement <8 x i16> undef, i16 %conv50, i32 0 ; <<8 x i16>> [#uses=1]
  %tmp52 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp53 = extractelement <8 x i32> %tmp52, i32 1 ; <i32> [#uses=1]
  %conv54 = trunc i32 %tmp53 to i16               ; <i16> [#uses=1]
  %vecinit55 = insertelement <8 x i16> %vecinit51, i16 %conv54, i32 1 ; <<8 x i16>> [#uses=1]
  %tmp56 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp57 = extractelement <8 x i32> %tmp56, i32 2 ; <i32> [#uses=1]
  %conv58 = trunc i32 %tmp57 to i16               ; <i16> [#uses=1]
  %vecinit59 = insertelement <8 x i16> %vecinit55, i16 %conv58, i32 2 ; <<8 x i16>> [#uses=1]
  %tmp60 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp61 = extractelement <8 x i32> %tmp60, i32 3 ; <i32> [#uses=1]
  %conv62 = trunc i32 %tmp61 to i16               ; <i16> [#uses=1]
  %vecinit63 = insertelement <8 x i16> %vecinit59, i16 %conv62, i32 3 ; <<8 x i16>> [#uses=1]
  %tmp64 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp65 = extractelement <8 x i32> %tmp64, i32 4 ; <i32> [#uses=1]
  %conv66 = trunc i32 %tmp65 to i16               ; <i16> [#uses=1]
  %vecinit67 = insertelement <8 x i16> %vecinit63, i16 %conv66, i32 4 ; <<8 x i16>> [#uses=1]
  %tmp68 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp69 = extractelement <8 x i32> %tmp68, i32 5 ; <i32> [#uses=1]
  %conv70 = trunc i32 %tmp69 to i16               ; <i16> [#uses=1]
  %vecinit71 = insertelement <8 x i16> %vecinit67, i16 %conv70, i32 5 ; <<8 x i16>> [#uses=1]
  %tmp72 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp73 = extractelement <8 x i32> %tmp72, i32 6 ; <i32> [#uses=1]
  %conv74 = trunc i32 %tmp73 to i16               ; <i16> [#uses=1]
  %vecinit75 = insertelement <8 x i16> %vecinit71, i16 %conv74, i32 6 ; <<8 x i16>> [#uses=1]
  %tmp76 = load <8 x i32>* %rounded               ; <<8 x i32>> [#uses=1]
  %tmp77 = extractelement <8 x i32> %tmp76, i32 7 ; <i32> [#uses=1]
  %conv78 = trunc i32 %tmp77 to i16               ; <i16> [#uses=1]
  %vecinit79 = insertelement <8 x i16> %vecinit75, i16 %conv78, i32 7 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %vecinit79, <8 x i16>* %.compoundliteral47
  %tmp80 = load <8 x i16>* %.compoundliteral47    ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp80, <8 x i16>* %shifted
  %tmp82 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=3]
  %tmp83 = extractelement <8 x i16> %tmp82, i32 0 ; <i16> [#uses=0]
  %0 = shufflevector <8 x i16> %tmp82, <8 x i16> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<8 x i16>> [#uses=0]
  %tmp84 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=2]
  %tmp85 = extractelement <8 x i16> %tmp84, i32 1 ; <i16> [#uses=0]
  %1 = shufflevector <8 x i16> %tmp82, <8 x i16> %tmp84, <8 x i32> <i32 0, i32 9, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ; <<8 x i16>> [#uses=1]
  %tmp86 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=1]
  %tmp87 = extractelement <8 x i16> %tmp86, i32 2 ; <i16> [#uses=1]
  %vecinit88 = insertelement <8 x i16> %1, i16 %tmp87, i32 2 ; <<8 x i16>> [#uses=1]
  %tmp89 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=1]
  %tmp90 = extractelement <8 x i16> %tmp89, i32 3 ; <i16> [#uses=1]
  %vecinit91 = insertelement <8 x i16> %vecinit88, i16 %tmp90, i32 3 ; <<8 x i16>> [#uses=1]
  %tmp92 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=1]
  %tmp93 = extractelement <8 x i16> %tmp92, i32 4 ; <i16> [#uses=1]
  %vecinit94 = insertelement <8 x i16> %vecinit91, i16 %tmp93, i32 4 ; <<8 x i16>> [#uses=1]
  %tmp95 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=1]
  %tmp96 = extractelement <8 x i16> %tmp95, i32 5 ; <i16> [#uses=1]
  %vecinit97 = insertelement <8 x i16> %vecinit94, i16 %tmp96, i32 5 ; <<8 x i16>> [#uses=1]
  %tmp98 = load <8 x i16>* %shifted               ; <<8 x i16>> [#uses=1]
  %tmp99 = extractelement <8 x i16> %tmp98, i32 6 ; <i16> [#uses=1]
  %vecinit100 = insertelement <8 x i16> %vecinit97, i16 %tmp99, i32 6 ; <<8 x i16>> [#uses=1]
  %tmp101 = load <8 x i16>* %shifted              ; <<8 x i16>> [#uses=1]
  %tmp102 = extractelement <8 x i16> %tmp101, i32 7 ; <i16> [#uses=1]
  %vecinit103 = insertelement <8 x i16> %vecinit100, i16 %tmp102, i32 7 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %vecinit103, <8 x i16>* %.compoundliteral81
  %tmp104 = load <8 x i16>* %.compoundliteral81   ; <<8 x i16>> [#uses=1]
  %tmp105 = load <8 x i16>* %val2.addr            ; <<8 x i16>> [#uses=1]
  %add106 = add <8 x i16> %tmp104, %tmp105        ; <<8 x i16>> [#uses=1]
  store <8 x i16> %add106, <8 x i16>* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %2 = load <8 x i16>* %retval                    ; <<8 x i16>> [#uses=1]
  ret <8 x i16> %2
}

define void @FindPoint(i16* %pui16SrcHistX, i16* %pui16SrcHistY, i16* %pui16DstHistPnt, i16* %pui16DstDestPnt, i16 addrspace(1)* %pHist, i32 %histNumPixels) nounwind {
entry:
  %pui16SrcHistX.addr = alloca i16*, align 4      ; <i16**> [#uses=3]
  %pui16SrcHistY.addr = alloca i16*, align 4      ; <i16**> [#uses=2]
  %pui16DstHistPnt.addr = alloca i16*, align 4    ; <i16**> [#uses=8]
  %pui16DstDestPnt.addr = alloca i16*, align 4    ; <i16**> [#uses=2]
  %pHist.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=6]
  %histNumPixels.addr = alloca i32, align 4       ; <i32*> [#uses=2]
  %SizeCounter = alloca i32, align 4              ; <i32*> [#uses=5]
  %Index = alloca i32, align 4                    ; <i32*> [#uses=15]
  %PrevSizeCounter = alloca i32, align 4          ; <i32*> [#uses=3]
  %BinPart = alloca i32, align 4                  ; <i32*> [#uses=7]
  %i = alloca i32, align 4                        ; <i32*> [#uses=28]
  %CurRange = alloca i32, align 4                 ; <i32*> [#uses=3]
  %val = alloca i16, align 2                      ; <i16*> [#uses=8]
  store i16* %pui16SrcHistX, i16** %pui16SrcHistX.addr
  store i16* %pui16SrcHistY, i16** %pui16SrcHistY.addr
  store i16* %pui16DstHistPnt, i16** %pui16DstHistPnt.addr
  store i16* %pui16DstDestPnt, i16** %pui16DstDestPnt.addr
  store i16 addrspace(1)* %pHist, i16 addrspace(1)** %pHist.addr
  store i32 %histNumPixels, i32* %histNumPixels.addr
  store i32 0, i32* %SizeCounter
  store i32 0, i32* %Index
  store i32 0, i32* %PrevSizeCounter
  store i32 0, i32* %BinPart
  store i32 0, i32* %i
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 8                     ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp1 = load i32* %i                            ; <i32> [#uses=1]
  %tmp2 = load i16** %pui16SrcHistX.addr          ; <i16*> [#uses=1]
  %arrayidx = getelementptr inbounds i16* %tmp2, i32 %tmp1 ; <i16*> [#uses=1]
  %tmp3 = load i16* %arrayidx                     ; <i16> [#uses=1]
  %tmp4 = load i32* %i                            ; <i32> [#uses=1]
  %tmp5 = load i16** %pui16DstHistPnt.addr        ; <i16*> [#uses=1]
  %arrayidx6 = getelementptr inbounds i16* %tmp5, i32 %tmp4 ; <i16*> [#uses=1]
  store i16 %tmp3, i16* %arrayidx6
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp7 = load i32* %i                            ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp7, 1                     ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 1, i32* %i
  br label %for.cond8

for.cond8:                                        ; preds = %for.inc113, %for.end
  %tmp9 = load i32* %i                            ; <i32> [#uses=1]
  %cmp10 = icmp slt i32 %tmp9, 4                  ; <i1> [#uses=1]
  br i1 %cmp10, label %for.body11, label %for.end116

for.body11:                                       ; preds = %for.cond8
  %tmp13 = load i32* %histNumPixels.addr          ; <i32> [#uses=1]
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load i16** %pui16SrcHistX.addr         ; <i16*> [#uses=1]
  %arrayidx16 = getelementptr inbounds i16* %tmp15, i32 %tmp14 ; <i16*> [#uses=1]
  %tmp17 = load i16* %arrayidx16                  ; <i16> [#uses=1]
  %conv = zext i16 %tmp17 to i32                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp13, %conv                    ; <i32> [#uses=1]
  %add = add i32 %mul, 512                        ; <i32> [#uses=1]
  %shr = lshr i32 %add, 10                        ; <i32> [#uses=1]
  store i32 %shr, i32* %CurRange
  br label %while.cond

while.cond:                                       ; preds = %while.body, %for.body11
  %tmp18 = load i32* %Index                       ; <i32> [#uses=1]
  %tmp19 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds i16 addrspace(1)* %tmp19, i32 %tmp18 ; <i16 addrspace(1)*> [#uses=1]
  %tmp21 = load i16 addrspace(1)* %arrayidx20     ; <i16> [#uses=1]
  %conv22 = zext i16 %tmp21 to i32                ; <i32> [#uses=1]
  %cmp23 = icmp eq i32 %conv22, 0                 ; <i1> [#uses=1]
  br i1 %cmp23, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %tmp25 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp26 = icmp slt i32 %tmp25, 128               ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %0 = phi i1 [ false, %while.cond ], [ %cmp26, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %tmp28 = load i32* %Index                       ; <i32> [#uses=1]
  %inc29 = add nsw i32 %tmp28, 1                  ; <i32> [#uses=1]
  store i32 %inc29, i32* %Index
  br label %while.cond

while.end:                                        ; preds = %land.end
  br label %while.cond30

while.cond30:                                     ; preds = %while.body40, %while.end
  %tmp31 = load i32* %SizeCounter                 ; <i32> [#uses=1]
  %tmp32 = load i32* %CurRange                    ; <i32> [#uses=1]
  %cmp33 = icmp slt i32 %tmp31, %tmp32            ; <i1> [#uses=1]
  br i1 %cmp33, label %land.rhs35, label %land.end39

land.rhs35:                                       ; preds = %while.cond30
  %tmp36 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp37 = icmp slt i32 %tmp36, 128               ; <i1> [#uses=1]
  br label %land.end39

land.end39:                                       ; preds = %land.rhs35, %while.cond30
  %1 = phi i1 [ false, %while.cond30 ], [ %cmp37, %land.rhs35 ] ; <i1> [#uses=1]
  br i1 %1, label %while.body40, label %while.end49

while.body40:                                     ; preds = %land.end39
  %tmp41 = load i32* %Index                       ; <i32> [#uses=2]
  %inc42 = add nsw i32 %tmp41, 1                  ; <i32> [#uses=1]
  store i32 %inc42, i32* %Index
  %tmp43 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds i16 addrspace(1)* %tmp43, i32 %tmp41 ; <i16 addrspace(1)*> [#uses=1]
  %tmp45 = load i16 addrspace(1)* %arrayidx44     ; <i16> [#uses=1]
  %conv46 = zext i16 %tmp45 to i32                ; <i32> [#uses=1]
  %tmp47 = load i32* %SizeCounter                 ; <i32> [#uses=1]
  %add48 = add nsw i32 %tmp47, %conv46            ; <i32> [#uses=1]
  store i32 %add48, i32* %SizeCounter
  br label %while.cond30

while.end49:                                      ; preds = %land.end39
  %tmp50 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp51 = icmp slt i32 %tmp50, 127               ; <i1> [#uses=1]
  %tmp53 = load i32* %Index                       ; <i32> [#uses=1]
  %cond = select i1 %cmp51, i32 %tmp53, i32 127   ; <i32> [#uses=1]
  store i32 %cond, i32* %Index
  %tmp54 = load i32* %SizeCounter                 ; <i32> [#uses=1]
  %tmp55 = load i32* %Index                       ; <i32> [#uses=1]
  %sub = sub i32 %tmp55, 1                        ; <i32> [#uses=1]
  %tmp56 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx57 = getelementptr inbounds i16 addrspace(1)* %tmp56, i32 %sub ; <i16 addrspace(1)*> [#uses=1]
  %tmp58 = load i16 addrspace(1)* %arrayidx57     ; <i16> [#uses=1]
  %conv59 = zext i16 %tmp58 to i32                ; <i32> [#uses=1]
  %sub60 = sub i32 %tmp54, %conv59                ; <i32> [#uses=1]
  store i32 %sub60, i32* %PrevSizeCounter
  %tmp61 = load i32* %Index                       ; <i32> [#uses=1]
  %sub62 = sub i32 %tmp61, 1                      ; <i32> [#uses=1]
  %tmp63 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx64 = getelementptr inbounds i16 addrspace(1)* %tmp63, i32 %sub62 ; <i16 addrspace(1)*> [#uses=1]
  %tmp65 = load i16 addrspace(1)* %arrayidx64     ; <i16> [#uses=1]
  %conv66 = zext i16 %tmp65 to i32                ; <i32> [#uses=1]
  %cmp67 = icmp eq i32 %conv66, 0                 ; <i1> [#uses=1]
  br i1 %cmp67, label %if.then, label %if.else

if.then:                                          ; preds = %while.end49
  store i32 3, i32* %BinPart
  br label %if.end

if.else:                                          ; preds = %while.end49
  %tmp69 = load i32* %CurRange                    ; <i32> [#uses=1]
  %tmp70 = load i32* %PrevSizeCounter             ; <i32> [#uses=1]
  %sub71 = sub i32 %tmp69, %tmp70                 ; <i32> [#uses=1]
  %mul72 = mul i32 %sub71, 8                      ; <i32> [#uses=1]
  %tmp73 = load i32* %Index                       ; <i32> [#uses=1]
  %sub74 = sub i32 %tmp73, 1                      ; <i32> [#uses=1]
  %tmp75 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx76 = getelementptr inbounds i16 addrspace(1)* %tmp75, i32 %sub74 ; <i16 addrspace(1)*> [#uses=1]
  %tmp77 = load i16 addrspace(1)* %arrayidx76     ; <i16> [#uses=1]
  %conv78 = zext i16 %tmp77 to i32                ; <i32> [#uses=2]
  %cmp79 = icmp eq i32 0, %conv78                 ; <i1> [#uses=1]
  %sel = select i1 %cmp79, i32 1, i32 %conv78     ; <i32> [#uses=1]
  %div = sdiv i32 %mul72, %sel                    ; <i32> [#uses=1]
  store i32 %div, i32* %BinPart
  %tmp80 = load i32* %BinPart                     ; <i32> [#uses=1]
  %cmp81 = icmp sgt i32 0, %tmp80                 ; <i1> [#uses=1]
  %tmp83 = load i32* %BinPart                     ; <i32> [#uses=1]
  %cond84 = select i1 %cmp81, i32 0, i32 %tmp83   ; <i32> [#uses=1]
  store i32 %cond84, i32* %BinPart
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp85 = load i32* %Index                       ; <i32> [#uses=1]
  %sub86 = sub i32 %tmp85, 1                      ; <i32> [#uses=1]
  %shl = shl i32 %sub86, 3                        ; <i32> [#uses=1]
  %tmp87 = load i32* %BinPart                     ; <i32> [#uses=1]
  %add88 = add nsw i32 %shl, %tmp87               ; <i32> [#uses=1]
  %conv89 = trunc i32 %add88 to i16               ; <i16> [#uses=1]
  %tmp90 = load i32* %i                           ; <i32> [#uses=1]
  %tmp91 = load i16** %pui16DstHistPnt.addr       ; <i16*> [#uses=1]
  %arrayidx92 = getelementptr inbounds i16* %tmp91, i32 %tmp90 ; <i16*> [#uses=1]
  store i16 %conv89, i16* %arrayidx92
  %tmp93 = load i32* %i                           ; <i32> [#uses=1]
  %tmp94 = load i16** %pui16DstHistPnt.addr       ; <i16*> [#uses=1]
  %arrayidx95 = getelementptr inbounds i16* %tmp94, i32 %tmp93 ; <i16*> [#uses=1]
  %tmp96 = load i16* %arrayidx95                  ; <i16> [#uses=1]
  %conv97 = zext i16 %tmp96 to i32                ; <i32> [#uses=1]
  %tmp98 = load i32* %i                           ; <i32> [#uses=1]
  %sub99 = sub i32 %tmp98, 1                      ; <i32> [#uses=1]
  %tmp100 = load i16** %pui16DstHistPnt.addr      ; <i16*> [#uses=1]
  %arrayidx101 = getelementptr inbounds i16* %tmp100, i32 %sub99 ; <i16*> [#uses=1]
  %tmp102 = load i16* %arrayidx101                ; <i16> [#uses=1]
  %conv103 = zext i16 %tmp102 to i32              ; <i32> [#uses=1]
  %cmp104 = icmp eq i32 %conv97, %conv103         ; <i1> [#uses=1]
  br i1 %cmp104, label %if.then106, label %if.end112

if.then106:                                       ; preds = %if.end
  %tmp107 = load i32* %i                          ; <i32> [#uses=1]
  %tmp108 = load i16** %pui16DstHistPnt.addr      ; <i16*> [#uses=1]
  %arrayidx109 = getelementptr inbounds i16* %tmp108, i32 %tmp107 ; <i16*> [#uses=2]
  %tmp110 = load i16* %arrayidx109                ; <i16> [#uses=1]
  %inc111 = add i16 %tmp110, 1                    ; <i16> [#uses=1]
  store i16 %inc111, i16* %arrayidx109
  br label %if.end112

if.end112:                                        ; preds = %if.then106, %if.end
  br label %for.inc113

for.inc113:                                       ; preds = %if.end112
  %tmp114 = load i32* %i                          ; <i32> [#uses=1]
  %inc115 = add nsw i32 %tmp114, 1                ; <i32> [#uses=1]
  store i32 %inc115, i32* %i
  br label %for.cond8

for.end116:                                       ; preds = %for.cond8
  store i32 0, i32* %i
  br label %for.cond117

for.cond117:                                      ; preds = %for.inc150, %for.end116
  %tmp118 = load i32* %i                          ; <i32> [#uses=1]
  %cmp119 = icmp slt i32 %tmp118, 8               ; <i1> [#uses=1]
  br i1 %cmp119, label %for.body121, label %for.end153

for.body121:                                      ; preds = %for.cond117
  %tmp123 = load i32* %i                          ; <i32> [#uses=1]
  %tmp124 = load i16** %pui16DstHistPnt.addr      ; <i16*> [#uses=1]
  %arrayidx125 = getelementptr inbounds i16* %tmp124, i32 %tmp123 ; <i16*> [#uses=1]
  %tmp126 = load i16* %arrayidx125                ; <i16> [#uses=1]
  store i16 %tmp126, i16* %val
  %tmp127 = load i16* %val                        ; <i16> [#uses=1]
  %conv128 = zext i16 %tmp127 to i32              ; <i32> [#uses=1]
  %cmp129 = icmp sgt i32 %conv128, 200            ; <i1> [#uses=1]
  br i1 %cmp129, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body121
  %tmp131 = load i16* %val                        ; <i16> [#uses=1]
  %conv132 = zext i16 %tmp131 to i32              ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %for.body121
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond133 = phi i32 [ %conv132, %cond.true ], [ 200, %cond.false ] ; <i32> [#uses=1]
  %conv134 = trunc i32 %cond133 to i16            ; <i16> [#uses=1]
  store i16 %conv134, i16* %val
  %tmp135 = load i16* %val                        ; <i16> [#uses=1]
  %conv136 = zext i16 %tmp135 to i32              ; <i32> [#uses=1]
  %cmp137 = icmp slt i32 %conv136, 940            ; <i1> [#uses=1]
  br i1 %cmp137, label %cond.true139, label %cond.false142

cond.true139:                                     ; preds = %cond.end
  %tmp140 = load i16* %val                        ; <i16> [#uses=1]
  %conv141 = zext i16 %tmp140 to i32              ; <i32> [#uses=1]
  br label %cond.end143

cond.false142:                                    ; preds = %cond.end
  br label %cond.end143

cond.end143:                                      ; preds = %cond.false142, %cond.true139
  %cond144 = phi i32 [ %conv141, %cond.true139 ], [ 940, %cond.false142 ] ; <i32> [#uses=1]
  %conv145 = trunc i32 %cond144 to i16            ; <i16> [#uses=1]
  store i16 %conv145, i16* %val
  %tmp146 = load i16* %val                        ; <i16> [#uses=1]
  %tmp147 = load i32* %i                          ; <i32> [#uses=1]
  %tmp148 = load i16** %pui16DstHistPnt.addr      ; <i16*> [#uses=1]
  %arrayidx149 = getelementptr inbounds i16* %tmp148, i32 %tmp147 ; <i16*> [#uses=1]
  store i16 %tmp146, i16* %arrayidx149
  br label %for.inc150

for.inc150:                                       ; preds = %cond.end143
  %tmp151 = load i32* %i                          ; <i32> [#uses=1]
  %inc152 = add nsw i32 %tmp151, 1                ; <i32> [#uses=1]
  store i32 %inc152, i32* %i
  br label %for.cond117

for.end153:                                       ; preds = %for.cond117
  store i32 0, i32* %i
  br label %for.cond154

for.cond154:                                      ; preds = %for.inc166, %for.end153
  %tmp155 = load i32* %i                          ; <i32> [#uses=1]
  %cmp156 = icmp slt i32 %tmp155, 8               ; <i1> [#uses=1]
  br i1 %cmp156, label %for.body158, label %for.end169

for.body158:                                      ; preds = %for.cond154
  %tmp159 = load i32* %i                          ; <i32> [#uses=1]
  %tmp160 = load i16** %pui16SrcHistY.addr        ; <i16*> [#uses=1]
  %arrayidx161 = getelementptr inbounds i16* %tmp160, i32 %tmp159 ; <i16*> [#uses=1]
  %tmp162 = load i16* %arrayidx161                ; <i16> [#uses=1]
  %tmp163 = load i32* %i                          ; <i32> [#uses=1]
  %tmp164 = load i16** %pui16DstDestPnt.addr      ; <i16*> [#uses=1]
  %arrayidx165 = getelementptr inbounds i16* %tmp164, i32 %tmp163 ; <i16*> [#uses=1]
  store i16 %tmp162, i16* %arrayidx165
  br label %for.inc166

for.inc166:                                       ; preds = %for.body158
  %tmp167 = load i32* %i                          ; <i32> [#uses=1]
  %inc168 = add nsw i32 %tmp167, 1                ; <i32> [#uses=1]
  store i32 %inc168, i32* %i
  br label %for.cond154

for.end169:                                       ; preds = %for.cond154
  ret void
}

define void @FindPoint_vector(i16 addrspace(1)* %pui16SrcHistX, i16 addrspace(1)* %pui16SrcHistY, <8 x i16>* %pui16DstHistPnt, <8 x i16>* %pui16DstDestPnt, i16 addrspace(1)* %pHist, i32 %histNumPixels) nounwind {
entry:
  %pui16SrcHistX.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=10]
  %pui16SrcHistY.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=1]
  %pui16DstHistPnt.addr = alloca <8 x i16>*, align 4 ; <<8 x i16>**> [#uses=6]
  %pui16DstDestPnt.addr = alloca <8 x i16>*, align 4 ; <<8 x i16>**> [#uses=1]
  %pHist.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=6]
  %histNumPixels.addr = alloca i32, align 4       ; <i32*> [#uses=2]
  %SizeCounter = alloca i32, align 4              ; <i32*> [#uses=5]
  %Index = alloca i32, align 4                    ; <i32*> [#uses=15]
  %PrevSizeCounter = alloca i32, align 4          ; <i32*> [#uses=3]
  %BinPart = alloca i32, align 4                  ; <i32*> [#uses=7]
  %i = alloca i32, align 4                        ; <i32*> [#uses=10]
  %pui16DstHistPntStorage = alloca [8 x i16], align 2 ; <[8 x i16]*> [#uses=6]
  %histVec = alloca <8 x i16>, align 16           ; <<8 x i16>*> [#uses=2]
  %.compoundliteral = alloca <8 x i16>, align 16  ; <<8 x i16>*> [#uses=2]
  %CurRange = alloca i32, align 4                 ; <i32*> [#uses=3]
  store i16 addrspace(1)* %pui16SrcHistX, i16 addrspace(1)** %pui16SrcHistX.addr
  store i16 addrspace(1)* %pui16SrcHistY, i16 addrspace(1)** %pui16SrcHistY.addr
  store <8 x i16>* %pui16DstHistPnt, <8 x i16>** %pui16DstHistPnt.addr
  store <8 x i16>* %pui16DstDestPnt, <8 x i16>** %pui16DstDestPnt.addr
  store i16 addrspace(1)* %pHist, i16 addrspace(1)** %pHist.addr
  store i32 %histNumPixels, i32* %histNumPixels.addr
  store i32 0, i32* %SizeCounter
  store i32 0, i32* %Index
  store i32 0, i32* %PrevSizeCounter
  store i32 0, i32* %BinPart
  store i32 0, i32* %i
  %tmp = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %tmp1 = load i16 addrspace(1)* %arrayidx        ; <i16> [#uses=1]
  %conv = zext i16 %tmp1 to i32                   ; <i32> [#uses=1]
  %add = add nsw i32 %conv, 0                     ; <i32> [#uses=1]
  %conv2 = trunc i32 %add to i16                  ; <i16> [#uses=1]
  %vecinit = insertelement <8 x i16> undef, i16 %conv2, i32 0 ; <<8 x i16>> [#uses=1]
  %tmp3 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx4 = getelementptr inbounds i16 addrspace(1)* %tmp3, i32 1 ; <i16 addrspace(1)*> [#uses=1]
  %tmp5 = load i16 addrspace(1)* %arrayidx4       ; <i16> [#uses=1]
  %conv6 = zext i16 %tmp5 to i32                  ; <i32> [#uses=1]
  %add7 = add nsw i32 %conv6, 0                   ; <i32> [#uses=1]
  %conv8 = trunc i32 %add7 to i16                 ; <i16> [#uses=1]
  %vecinit9 = insertelement <8 x i16> %vecinit, i16 %conv8, i32 1 ; <<8 x i16>> [#uses=1]
  %tmp10 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx11 = getelementptr inbounds i16 addrspace(1)* %tmp10, i32 2 ; <i16 addrspace(1)*> [#uses=1]
  %tmp12 = load i16 addrspace(1)* %arrayidx11     ; <i16> [#uses=1]
  %conv13 = zext i16 %tmp12 to i32                ; <i32> [#uses=1]
  %add14 = add nsw i32 %conv13, 0                 ; <i32> [#uses=1]
  %conv15 = trunc i32 %add14 to i16               ; <i16> [#uses=1]
  %vecinit16 = insertelement <8 x i16> %vecinit9, i16 %conv15, i32 2 ; <<8 x i16>> [#uses=1]
  %tmp17 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds i16 addrspace(1)* %tmp17, i32 3 ; <i16 addrspace(1)*> [#uses=1]
  %tmp19 = load i16 addrspace(1)* %arrayidx18     ; <i16> [#uses=1]
  %conv20 = zext i16 %tmp19 to i32                ; <i32> [#uses=1]
  %add21 = add nsw i32 %conv20, 0                 ; <i32> [#uses=1]
  %conv22 = trunc i32 %add21 to i16               ; <i16> [#uses=1]
  %vecinit23 = insertelement <8 x i16> %vecinit16, i16 %conv22, i32 3 ; <<8 x i16>> [#uses=1]
  %tmp24 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i16 addrspace(1)* %tmp24, i32 4 ; <i16 addrspace(1)*> [#uses=1]
  %tmp26 = load i16 addrspace(1)* %arrayidx25     ; <i16> [#uses=1]
  %conv27 = zext i16 %tmp26 to i32                ; <i32> [#uses=1]
  %add28 = add nsw i32 %conv27, 0                 ; <i32> [#uses=1]
  %conv29 = trunc i32 %add28 to i16               ; <i16> [#uses=1]
  %vecinit30 = insertelement <8 x i16> %vecinit23, i16 %conv29, i32 4 ; <<8 x i16>> [#uses=1]
  %tmp31 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx32 = getelementptr inbounds i16 addrspace(1)* %tmp31, i32 5 ; <i16 addrspace(1)*> [#uses=1]
  %tmp33 = load i16 addrspace(1)* %arrayidx32     ; <i16> [#uses=1]
  %conv34 = zext i16 %tmp33 to i32                ; <i32> [#uses=1]
  %add35 = add nsw i32 %conv34, 0                 ; <i32> [#uses=1]
  %conv36 = trunc i32 %add35 to i16               ; <i16> [#uses=1]
  %vecinit37 = insertelement <8 x i16> %vecinit30, i16 %conv36, i32 5 ; <<8 x i16>> [#uses=1]
  %tmp38 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds i16 addrspace(1)* %tmp38, i32 6 ; <i16 addrspace(1)*> [#uses=1]
  %tmp40 = load i16 addrspace(1)* %arrayidx39     ; <i16> [#uses=1]
  %conv41 = zext i16 %tmp40 to i32                ; <i32> [#uses=1]
  %add42 = add nsw i32 %conv41, 0                 ; <i32> [#uses=1]
  %conv43 = trunc i32 %add42 to i16               ; <i16> [#uses=1]
  %vecinit44 = insertelement <8 x i16> %vecinit37, i16 %conv43, i32 6 ; <<8 x i16>> [#uses=1]
  %tmp45 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds i16 addrspace(1)* %tmp45, i32 7 ; <i16 addrspace(1)*> [#uses=1]
  %tmp47 = load i16 addrspace(1)* %arrayidx46     ; <i16> [#uses=1]
  %conv48 = zext i16 %tmp47 to i32                ; <i32> [#uses=1]
  %add49 = add nsw i32 %conv48, 0                 ; <i32> [#uses=1]
  %conv50 = trunc i32 %add49 to i16               ; <i16> [#uses=1]
  %vecinit51 = insertelement <8 x i16> %vecinit44, i16 %conv50, i32 7 ; <<8 x i16>> [#uses=1]
  store <8 x i16> %vecinit51, <8 x i16>* %.compoundliteral
  %tmp52 = load <8 x i16>* %.compoundliteral      ; <<8 x i16>> [#uses=1]
  store <8 x i16> %tmp52, <8 x i16>* %histVec
  %tmp53 = load <8 x i16>* %histVec               ; <<8 x i16>> [#uses=1]
  %tmp54 = load <8 x i16>** %pui16DstHistPnt.addr ; <<8 x i16>*> [#uses=1]
  store <8 x i16> %tmp53, <8 x i16>* %tmp54
  %tmp55 = load <8 x i16>** %pui16DstHistPnt.addr ; <<8 x i16>*> [#uses=1]
  %tmp56 = load <8 x i16>* %tmp55                 ; <<8 x i16>> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  call void @_Z7vstore8Dv8_tjPt(<8 x i16> %tmp56, i32 0, i16* %arraydecay)
  store i32 1, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp57 = load i32* %i                           ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp57, 4                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp60 = load i32* %histNumPixels.addr          ; <i32> [#uses=1]
  %tmp61 = load i32* %i                           ; <i32> [#uses=1]
  %tmp62 = load i16 addrspace(1)** %pui16SrcHistX.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx63 = getelementptr inbounds i16 addrspace(1)* %tmp62, i32 %tmp61 ; <i16 addrspace(1)*> [#uses=1]
  %tmp64 = load i16 addrspace(1)* %arrayidx63     ; <i16> [#uses=1]
  %conv65 = zext i16 %tmp64 to i32                ; <i32> [#uses=1]
  %mul = mul i32 %tmp60, %conv65                  ; <i32> [#uses=1]
  %add66 = add i32 %mul, 512                      ; <i32> [#uses=1]
  %shr = lshr i32 %add66, 10                      ; <i32> [#uses=1]
  store i32 %shr, i32* %CurRange
  br label %while.cond

while.cond:                                       ; preds = %while.body, %for.body
  %tmp67 = load i32* %Index                       ; <i32> [#uses=1]
  %tmp68 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx69 = getelementptr inbounds i16 addrspace(1)* %tmp68, i32 %tmp67 ; <i16 addrspace(1)*> [#uses=1]
  %tmp70 = load i16 addrspace(1)* %arrayidx69     ; <i16> [#uses=1]
  %conv71 = zext i16 %tmp70 to i32                ; <i32> [#uses=1]
  %cmp72 = icmp eq i32 %conv71, 0                 ; <i1> [#uses=1]
  br i1 %cmp72, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %tmp74 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp75 = icmp slt i32 %tmp74, 128               ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %0 = phi i1 [ false, %while.cond ], [ %cmp75, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %tmp77 = load i32* %Index                       ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp77, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %Index
  br label %while.cond

while.end:                                        ; preds = %land.end
  br label %while.cond78

while.cond78:                                     ; preds = %while.body88, %while.end
  %tmp79 = load i32* %SizeCounter                 ; <i32> [#uses=1]
  %tmp80 = load i32* %CurRange                    ; <i32> [#uses=1]
  %cmp81 = icmp slt i32 %tmp79, %tmp80            ; <i1> [#uses=1]
  br i1 %cmp81, label %land.rhs83, label %land.end87

land.rhs83:                                       ; preds = %while.cond78
  %tmp84 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp85 = icmp slt i32 %tmp84, 128               ; <i1> [#uses=1]
  br label %land.end87

land.end87:                                       ; preds = %land.rhs83, %while.cond78
  %1 = phi i1 [ false, %while.cond78 ], [ %cmp85, %land.rhs83 ] ; <i1> [#uses=1]
  br i1 %1, label %while.body88, label %while.end97

while.body88:                                     ; preds = %land.end87
  %tmp89 = load i32* %Index                       ; <i32> [#uses=2]
  %inc90 = add nsw i32 %tmp89, 1                  ; <i32> [#uses=1]
  store i32 %inc90, i32* %Index
  %tmp91 = load i16 addrspace(1)** %pHist.addr    ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx92 = getelementptr inbounds i16 addrspace(1)* %tmp91, i32 %tmp89 ; <i16 addrspace(1)*> [#uses=1]
  %tmp93 = load i16 addrspace(1)* %arrayidx92     ; <i16> [#uses=1]
  %conv94 = zext i16 %tmp93 to i32                ; <i32> [#uses=1]
  %tmp95 = load i32* %SizeCounter                 ; <i32> [#uses=1]
  %add96 = add nsw i32 %tmp95, %conv94            ; <i32> [#uses=1]
  store i32 %add96, i32* %SizeCounter
  br label %while.cond78

while.end97:                                      ; preds = %land.end87
  %tmp98 = load i32* %Index                       ; <i32> [#uses=1]
  %cmp99 = icmp slt i32 %tmp98, 127               ; <i1> [#uses=1]
  %tmp101 = load i32* %Index                      ; <i32> [#uses=1]
  %cond = select i1 %cmp99, i32 %tmp101, i32 127  ; <i32> [#uses=1]
  store i32 %cond, i32* %Index
  %tmp102 = load i32* %SizeCounter                ; <i32> [#uses=1]
  %tmp103 = load i32* %Index                      ; <i32> [#uses=1]
  %sub = sub i32 %tmp103, 1                       ; <i32> [#uses=1]
  %tmp104 = load i16 addrspace(1)** %pHist.addr   ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx105 = getelementptr inbounds i16 addrspace(1)* %tmp104, i32 %sub ; <i16 addrspace(1)*> [#uses=1]
  %tmp106 = load i16 addrspace(1)* %arrayidx105   ; <i16> [#uses=1]
  %conv107 = zext i16 %tmp106 to i32              ; <i32> [#uses=1]
  %sub108 = sub i32 %tmp102, %conv107             ; <i32> [#uses=1]
  store i32 %sub108, i32* %PrevSizeCounter
  %tmp109 = load i32* %Index                      ; <i32> [#uses=1]
  %sub110 = sub i32 %tmp109, 1                    ; <i32> [#uses=1]
  %tmp111 = load i16 addrspace(1)** %pHist.addr   ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx112 = getelementptr inbounds i16 addrspace(1)* %tmp111, i32 %sub110 ; <i16 addrspace(1)*> [#uses=1]
  %tmp113 = load i16 addrspace(1)* %arrayidx112   ; <i16> [#uses=1]
  %conv114 = zext i16 %tmp113 to i32              ; <i32> [#uses=1]
  %cmp115 = icmp eq i32 %conv114, 0               ; <i1> [#uses=1]
  br i1 %cmp115, label %if.then, label %if.else

if.then:                                          ; preds = %while.end97
  store i32 3, i32* %BinPart
  br label %if.end

if.else:                                          ; preds = %while.end97
  %tmp117 = load i32* %CurRange                   ; <i32> [#uses=1]
  %tmp118 = load i32* %PrevSizeCounter            ; <i32> [#uses=1]
  %sub119 = sub i32 %tmp117, %tmp118              ; <i32> [#uses=1]
  %mul120 = mul i32 %sub119, 8                    ; <i32> [#uses=1]
  %tmp121 = load i32* %Index                      ; <i32> [#uses=1]
  %sub122 = sub i32 %tmp121, 1                    ; <i32> [#uses=1]
  %tmp123 = load i16 addrspace(1)** %pHist.addr   ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx124 = getelementptr inbounds i16 addrspace(1)* %tmp123, i32 %sub122 ; <i16 addrspace(1)*> [#uses=1]
  %tmp125 = load i16 addrspace(1)* %arrayidx124   ; <i16> [#uses=1]
  %conv126 = zext i16 %tmp125 to i32              ; <i32> [#uses=2]
  %cmp127 = icmp eq i32 0, %conv126               ; <i1> [#uses=1]
  %sel = select i1 %cmp127, i32 1, i32 %conv126   ; <i32> [#uses=1]
  %div = sdiv i32 %mul120, %sel                   ; <i32> [#uses=1]
  store i32 %div, i32* %BinPart
  %tmp128 = load i32* %BinPart                    ; <i32> [#uses=1]
  %cmp129 = icmp sgt i32 0, %tmp128               ; <i1> [#uses=1]
  %tmp131 = load i32* %BinPart                    ; <i32> [#uses=1]
  %cond132 = select i1 %cmp129, i32 0, i32 %tmp131 ; <i32> [#uses=1]
  store i32 %cond132, i32* %BinPart
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp133 = load i32* %Index                      ; <i32> [#uses=1]
  %sub134 = sub i32 %tmp133, 1                    ; <i32> [#uses=1]
  %shl = shl i32 %sub134, 3                       ; <i32> [#uses=1]
  %tmp135 = load i32* %BinPart                    ; <i32> [#uses=1]
  %add136 = add nsw i32 %shl, %tmp135             ; <i32> [#uses=1]
  %conv137 = trunc i32 %add136 to i16             ; <i16> [#uses=1]
  %tmp138 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay139 = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx140 = getelementptr inbounds i16* %arraydecay139, i32 %tmp138 ; <i16*> [#uses=1]
  store i16 %conv137, i16* %arrayidx140
  %tmp141 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay142 = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx143 = getelementptr inbounds i16* %arraydecay142, i32 %tmp141 ; <i16*> [#uses=1]
  %tmp144 = load i16* %arrayidx143                ; <i16> [#uses=1]
  %conv145 = zext i16 %tmp144 to i32              ; <i32> [#uses=1]
  %tmp146 = load i32* %i                          ; <i32> [#uses=1]
  %sub147 = sub i32 %tmp146, 1                    ; <i32> [#uses=1]
  %arraydecay148 = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx149 = getelementptr inbounds i16* %arraydecay148, i32 %sub147 ; <i16*> [#uses=1]
  %tmp150 = load i16* %arrayidx149                ; <i16> [#uses=1]
  %conv151 = zext i16 %tmp150 to i32              ; <i32> [#uses=1]
  %cmp152 = icmp eq i32 %conv145, %conv151        ; <i1> [#uses=1]
  br i1 %cmp152, label %if.then154, label %if.end160

if.then154:                                       ; preds = %if.end
  %tmp155 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay156 = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx157 = getelementptr inbounds i16* %arraydecay156, i32 %tmp155 ; <i16*> [#uses=2]
  %tmp158 = load i16* %arrayidx157                ; <i16> [#uses=1]
  %inc159 = add i16 %tmp158, 1                    ; <i16> [#uses=1]
  store i16 %inc159, i16* %arrayidx157
  br label %if.end160

if.end160:                                        ; preds = %if.then154, %if.end
  br label %for.inc

for.inc:                                          ; preds = %if.end160
  %tmp161 = load i32* %i                          ; <i32> [#uses=1]
  %inc162 = add nsw i32 %tmp161, 1                ; <i32> [#uses=1]
  store i32 %inc162, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arraydecay163 = getelementptr inbounds [8 x i16]* %pui16DstHistPntStorage, i32 0, i32 0 ; <i16*> [#uses=1]
  %call = call <8 x i16> @_Z6vload8jPKt(i32 0, i16* %arraydecay163) ; <<8 x i16>> [#uses=1]
  %tmp164 = load <8 x i16>** %pui16DstHistPnt.addr ; <<8 x i16>*> [#uses=1]
  store <8 x i16> %call, <8 x i16>* %tmp164
  %tmp165 = load <8 x i16>** %pui16DstHistPnt.addr ; <<8 x i16>*> [#uses=1]
  %tmp166 = load <8 x i16>* %tmp165               ; <<8 x i16>> [#uses=1]
  %tmp167 = load <8 x i16>* @minYVec              ; <<8 x i16>> [#uses=1]
  %call168 = call <8 x i16> @_Z3maxDv8_tS_(<8 x i16> %tmp166, <8 x i16> %tmp167) ; <<8 x i16>> [#uses=1]
  %tmp169 = load <8 x i16>* @maxYVec              ; <<8 x i16>> [#uses=1]
  %call170 = call <8 x i16> @_Z3minDv8_tS_(<8 x i16> %call168, <8 x i16> %tmp169) ; <<8 x i16>> [#uses=1]
  %tmp171 = load <8 x i16>** %pui16DstHistPnt.addr ; <<8 x i16>*> [#uses=1]
  store <8 x i16> %call170, <8 x i16>* %tmp171
  ret void
}

declare void @_Z7vstore8Dv8_tjPt(<8 x i16>, i32, i16*)

declare <8 x i16> @_Z6vload8jPKt(i32, i16*)

declare <8 x i16> @_Z3minDv8_tS_(<8 x i16>, <8 x i16>)

declare <8 x i16> @_Z3maxDv8_tS_(<8 x i16>, <8 x i16>)

define void @BuildBrightnessLUT(%struct.anon* %brightnessLut) nounwind {
entry:
  %brightnessLut.addr = alloca %struct.anon*, align 4 ; <%struct.anon**> [#uses=10]
  %i = alloca i32, align 4                        ; <i32*> [#uses=21]
  %slp = alloca i32, align 4                      ; <i32*> [#uses=4]
  store %struct.anon* %brightnessLut, %struct.anon** %brightnessLut.addr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 8                     ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp1 = load i32* %i                            ; <i32> [#uses=1]
  %arrayidx = getelementptr inbounds i16* getelementptr inbounds ([8 x i16]* @g_pui16BrCorrBrightX, i32 0, i32 0), i32 %tmp1 ; <i16*> [#uses=1]
  %tmp2 = load i16* %arrayidx                     ; <i16> [#uses=1]
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %arrayidx4 = getelementptr inbounds i16* getelementptr inbounds ([8 x i16]* @g_pui16BrCorrDarkX, i32 0, i32 0), i32 %tmp3 ; <i16*> [#uses=1]
  %tmp5 = load i16* %arrayidx4                    ; <i16> [#uses=1]
  %call = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp2, i16 zeroext %tmp5, i16 zeroext 127) ; <i16> [#uses=1]
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %tmp7 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp8 = getelementptr inbounds %struct.anon* %tmp7, i32 0, i32 0 ; <[8 x i16]*> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x i16]* %tmp8, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx9 = getelementptr inbounds i16* %arraydecay, i32 %tmp6 ; <i16*> [#uses=1]
  store i16 %call, i16* %arrayidx9
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx11 = getelementptr inbounds i16* getelementptr inbounds ([8 x i16]* @g_pui16BrCorrBrightY, i32 0, i32 0), i32 %tmp10 ; <i16*> [#uses=1]
  %tmp12 = load i16* %arrayidx11                  ; <i16> [#uses=1]
  %tmp13 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx14 = getelementptr inbounds i16* getelementptr inbounds ([8 x i16]* @g_pui16BrCorrDarkY, i32 0, i32 0), i32 %tmp13 ; <i16*> [#uses=1]
  %tmp15 = load i16* %arrayidx14                  ; <i16> [#uses=1]
  %call16 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp12, i16 zeroext %tmp15, i16 zeroext 127) ; <i16> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %tmp18 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon* %tmp18, i32 0, i32 1 ; <[8 x i16]*> [#uses=1]
  %arraydecay20 = getelementptr inbounds [8 x i16]* %tmp19, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx21 = getelementptr inbounds i16* %arraydecay20, i32 %tmp17 ; <i16*> [#uses=1]
  store i16 %call16, i16* %arrayidx21
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp22, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc90, %for.end
  %tmp24 = load i32* %i                           ; <i32> [#uses=1]
  %cmp25 = icmp slt i32 %tmp24, 3                 ; <i1> [#uses=1]
  br i1 %cmp25, label %for.body26, label %for.end93

for.body26:                                       ; preds = %for.cond23
  store i32 0, i32* %slp
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %add = add nsw i32 %tmp28, 1                    ; <i32> [#uses=1]
  %tmp29 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp30 = getelementptr inbounds %struct.anon* %tmp29, i32 0, i32 0 ; <[8 x i16]*> [#uses=1]
  %arraydecay31 = getelementptr inbounds [8 x i16]* %tmp30, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx32 = getelementptr inbounds i16* %arraydecay31, i32 %add ; <i16*> [#uses=1]
  %tmp33 = load i16* %arrayidx32                  ; <i16> [#uses=1]
  %conv = zext i16 %tmp33 to i32                  ; <i32> [#uses=1]
  %tmp34 = load i32* %i                           ; <i32> [#uses=1]
  %tmp35 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp36 = getelementptr inbounds %struct.anon* %tmp35, i32 0, i32 0 ; <[8 x i16]*> [#uses=1]
  %arraydecay37 = getelementptr inbounds [8 x i16]* %tmp36, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx38 = getelementptr inbounds i16* %arraydecay37, i32 %tmp34 ; <i16*> [#uses=1]
  %tmp39 = load i16* %arrayidx38                  ; <i16> [#uses=1]
  %conv40 = zext i16 %tmp39 to i32                ; <i32> [#uses=1]
  %sub = sub i32 %conv, %conv40                   ; <i32> [#uses=1]
  %cmp41 = icmp sgt i32 %sub, 0                   ; <i1> [#uses=1]
  br i1 %cmp41, label %if.then, label %if.end

if.then:                                          ; preds = %for.body26
  %tmp43 = load i32* %i                           ; <i32> [#uses=1]
  %add44 = add nsw i32 %tmp43, 1                  ; <i32> [#uses=1]
  %tmp45 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp46 = getelementptr inbounds %struct.anon* %tmp45, i32 0, i32 1 ; <[8 x i16]*> [#uses=1]
  %arraydecay47 = getelementptr inbounds [8 x i16]* %tmp46, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx48 = getelementptr inbounds i16* %arraydecay47, i32 %add44 ; <i16*> [#uses=1]
  %tmp49 = load i16* %arrayidx48                  ; <i16> [#uses=1]
  %conv50 = zext i16 %tmp49 to i32                ; <i32> [#uses=1]
  %tmp51 = load i32* %i                           ; <i32> [#uses=1]
  %tmp52 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp53 = getelementptr inbounds %struct.anon* %tmp52, i32 0, i32 1 ; <[8 x i16]*> [#uses=1]
  %arraydecay54 = getelementptr inbounds [8 x i16]* %tmp53, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx55 = getelementptr inbounds i16* %arraydecay54, i32 %tmp51 ; <i16*> [#uses=1]
  %tmp56 = load i16* %arrayidx55                  ; <i16> [#uses=1]
  %conv57 = zext i16 %tmp56 to i32                ; <i32> [#uses=1]
  %sub58 = sub i32 %conv50, %conv57               ; <i32> [#uses=1]
  %conv59 = sext i32 %sub58 to i64                ; <i64> [#uses=1]
  %shl = shl i64 %conv59, 16                      ; <i64> [#uses=1]
  %tmp60 = load i32* %i                           ; <i32> [#uses=1]
  %add61 = add nsw i32 %tmp60, 1                  ; <i32> [#uses=1]
  %tmp62 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp63 = getelementptr inbounds %struct.anon* %tmp62, i32 0, i32 0 ; <[8 x i16]*> [#uses=1]
  %arraydecay64 = getelementptr inbounds [8 x i16]* %tmp63, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx65 = getelementptr inbounds i16* %arraydecay64, i32 %add61 ; <i16*> [#uses=1]
  %tmp66 = load i16* %arrayidx65                  ; <i16> [#uses=1]
  %conv67 = zext i16 %tmp66 to i32                ; <i32> [#uses=1]
  %tmp68 = load i32* %i                           ; <i32> [#uses=1]
  %tmp69 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp70 = getelementptr inbounds %struct.anon* %tmp69, i32 0, i32 0 ; <[8 x i16]*> [#uses=1]
  %arraydecay71 = getelementptr inbounds [8 x i16]* %tmp70, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx72 = getelementptr inbounds i16* %arraydecay71, i32 %tmp68 ; <i16*> [#uses=1]
  %tmp73 = load i16* %arrayidx72                  ; <i16> [#uses=1]
  %conv74 = zext i16 %tmp73 to i32                ; <i32> [#uses=1]
  %sub75 = sub i32 %conv67, %conv74               ; <i32> [#uses=1]
  %conv76 = sext i32 %sub75 to i64                ; <i64> [#uses=2]
  %cmp77 = icmp eq i64 0, %conv76                 ; <i1> [#uses=1]
  %sel = select i1 %cmp77, i64 1, i64 %conv76     ; <i64> [#uses=1]
  %div = sdiv i64 %shl, %sel                      ; <i64> [#uses=1]
  %add78 = add nsw i64 %div, 128                  ; <i64> [#uses=1]
  %shr = ashr i64 %add78, 8                       ; <i64> [#uses=1]
  %conv79 = trunc i64 %shr to i32                 ; <i32> [#uses=1]
  store i32 %conv79, i32* %slp
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body26
  %tmp80 = load i32* %slp                         ; <i32> [#uses=1]
  %cmp81 = icmp slt i32 %tmp80, 2048              ; <i1> [#uses=1]
  br i1 %cmp81, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.end
  %tmp83 = load i32* %slp                         ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %if.end
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %tmp83, %cond.true ], [ 2048, %cond.false ] ; <i32> [#uses=1]
  %conv84 = trunc i32 %cond to i16                ; <i16> [#uses=1]
  %tmp85 = load i32* %i                           ; <i32> [#uses=1]
  %tmp86 = load %struct.anon** %brightnessLut.addr ; <%struct.anon*> [#uses=1]
  %tmp87 = getelementptr inbounds %struct.anon* %tmp86, i32 0, i32 2 ; <[8 x i16]*> [#uses=1]
  %arraydecay88 = getelementptr inbounds [8 x i16]* %tmp87, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx89 = getelementptr inbounds i16* %arraydecay88, i32 %tmp85 ; <i16*> [#uses=1]
  store i16 %conv84, i16* %arrayidx89
  br label %for.inc90

for.inc90:                                        ; preds = %cond.end
  %tmp91 = load i32* %i                           ; <i32> [#uses=1]
  %inc92 = add nsw i32 %tmp91, 1                  ; <i32> [#uses=1]
  store i32 %inc92, i32* %i
  br label %for.cond23

for.end93:                                        ; preds = %for.cond23
  ret void
}

define void @calculateHistogram(i32 %imageWidth, i32 %imageHeight, i32 %roiStartY, i32 %roiEndY, i32 %roiStartX, i32 %roiEndX, i16 addrspace(1)* %imageLumaChannel, i16 addrspace(1)* %outputHistogram) nounwind {
entry:
  %imageWidth.addr = alloca i32, align 4          ; <i32*> [#uses=4]
  %imageHeight.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %roiStartY.addr = alloca i32, align 4           ; <i32*> [#uses=5]
  %roiEndY.addr = alloca i32, align 4             ; <i32*> [#uses=4]
  %roiStartX.addr = alloca i32, align 4           ; <i32*> [#uses=3]
  %roiEndX.addr = alloca i32, align 4             ; <i32*> [#uses=4]
  %imageLumaChannel.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %outputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %pix_phase = alloca i32, align 4                ; <i32*> [#uses=7]
  %ui32StartX = alloca i32, align 4               ; <i32*> [#uses=5]
  %ui32StartY = alloca i32, align 4               ; <i32*> [#uses=11]
  %ui32EndX = alloca i32, align 4                 ; <i32*> [#uses=8]
  %ui32EndY = alloca i32, align 4                 ; <i32*> [#uses=8]
  %pLine = alloca i16 addrspace(1)*, align 4      ; <i16 addrspace(1)**> [#uses=4]
  %pTempHist = alloca i16 addrspace(1)*, align 4  ; <i16 addrspace(1)**> [#uses=3]
  %x = alloca i32, align 4                        ; <i32*> [#uses=5]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=3]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=2]
  %numOfLinesToProcess = alloca i32, align 4      ; <i32*> [#uses=3]
  %yStart = alloca i32, align 4                   ; <i32*> [#uses=3]
  %yEnd = alloca i32, align 4                     ; <i32*> [#uses=2]
  %val = alloca i32, align 4                      ; <i32*> [#uses=6]
  store i32 %imageWidth, i32* %imageWidth.addr
  store i32 %imageHeight, i32* %imageHeight.addr
  store i32 %roiStartY, i32* %roiStartY.addr
  store i32 %roiEndY, i32* %roiEndY.addr
  store i32 %roiStartX, i32* %roiStartX.addr
  store i32 %roiEndX, i32* %roiEndX.addr
  store i16 addrspace(1)* %imageLumaChannel, i16 addrspace(1)** %imageLumaChannel.addr
  store i16 addrspace(1)* %outputHistogram, i16 addrspace(1)** %outputHistogram.addr
  store i32 0, i32* %pix_phase
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalId
  %call1 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call1, i32* %globalSize
  %tmp = load i32* %imageHeight.addr              ; <i32> [#uses=1]
  %tmp2 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %numOfLinesToProcess
  %tmp4 = load i32* %globalId                     ; <i32> [#uses=1]
  %tmp5 = load i32* %numOfLinesToProcess          ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  store i32 %mul, i32* %yStart
  %tmp7 = load i32* %yStart                       ; <i32> [#uses=1]
  %tmp8 = load i32* %numOfLinesToProcess          ; <i32> [#uses=1]
  %add = add i32 %tmp7, %tmp8                     ; <i32> [#uses=1]
  store i32 %add, i32* %yEnd
  %tmp9 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %tmp10 = load i32* %globalId                    ; <i32> [#uses=1]
  %mul11 = mul i32 128, %tmp10                    ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds i16 addrspace(1)* %tmp9, i32 %mul11 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr, i16 addrspace(1)** %pTempHist
  store i32 0, i32* %ui32StartX
  %tmp12 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  store i32 %tmp12, i32* %ui32EndX
  %tmp13 = load i32* %yStart                      ; <i32> [#uses=1]
  store i32 %tmp13, i32* %ui32StartY
  %tmp14 = load i32* %yEnd                        ; <i32> [#uses=1]
  store i32 %tmp14, i32* %ui32EndY
  %tmp15 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %cmp16 = icmp ne i32 0, %tmp15                  ; <i1> [#uses=1]
  br i1 %cmp16, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp17 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %tmp18 = load i32* %roiStartY.addr              ; <i32> [#uses=1]
  %sub = sub i32 %tmp17, %tmp18                   ; <i32> [#uses=1]
  %rem = urem i32 %sub, 4                         ; <i32> [#uses=1]
  store i32 %rem, i32* %pix_phase
  %tmp19 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %tmp20 = load i32* %roiStartY.addr              ; <i32> [#uses=1]
  %sub21 = sub i32 %tmp19, %tmp20                 ; <i32> [#uses=1]
  %rem22 = urem i32 %sub21, 1                     ; <i32> [#uses=1]
  %tmp23 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %add24 = add i32 %tmp23, %rem22                 ; <i32> [#uses=1]
  store i32 %add24, i32* %ui32StartY
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp25 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %tmp26 = load i32* %roiStartY.addr              ; <i32> [#uses=1]
  %cmp27 = icmp ugt i32 %tmp25, %tmp26            ; <i1> [#uses=1]
  %tmp28 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %tmp29 = load i32* %roiStartY.addr              ; <i32> [#uses=1]
  %cond = select i1 %cmp27, i32 %tmp28, i32 %tmp29 ; <i32> [#uses=1]
  store i32 %cond, i32* %ui32StartY
  %tmp30 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %tmp31 = load i32* %roiEndY.addr                ; <i32> [#uses=1]
  %cmp32 = icmp ult i32 %tmp30, %tmp31            ; <i1> [#uses=1]
  %tmp33 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %tmp34 = load i32* %roiEndY.addr                ; <i32> [#uses=1]
  %cond35 = select i1 %cmp32, i32 %tmp33, i32 %tmp34 ; <i32> [#uses=1]
  store i32 %cond35, i32* %ui32EndY
  %tmp36 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %tmp37 = load i32* %roiEndY.addr                ; <i32> [#uses=1]
  %cmp38 = icmp ult i32 %tmp36, %tmp37            ; <i1> [#uses=1]
  br i1 %cmp38, label %if.then39, label %if.end41

if.then39:                                        ; preds = %if.end
  %tmp40 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %dec = add i32 %tmp40, -1                       ; <i32> [#uses=1]
  store i32 %dec, i32* %ui32EndY
  br label %if.end41

if.end41:                                         ; preds = %if.then39, %if.end
  %tmp42 = load i32* %ui32StartX                  ; <i32> [#uses=1]
  %tmp43 = load i32* %roiStartX.addr              ; <i32> [#uses=1]
  %cmp44 = icmp ugt i32 %tmp42, %tmp43            ; <i1> [#uses=1]
  %tmp45 = load i32* %ui32StartX                  ; <i32> [#uses=1]
  %tmp46 = load i32* %roiStartX.addr              ; <i32> [#uses=1]
  %cond47 = select i1 %cmp44, i32 %tmp45, i32 %tmp46 ; <i32> [#uses=1]
  store i32 %cond47, i32* %ui32StartX
  %tmp48 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %tmp49 = load i32* %roiEndX.addr                ; <i32> [#uses=1]
  %cmp50 = icmp ult i32 %tmp48, %tmp49            ; <i1> [#uses=1]
  %tmp51 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %tmp52 = load i32* %roiEndX.addr                ; <i32> [#uses=1]
  %cond53 = select i1 %cmp50, i32 %tmp51, i32 %tmp52 ; <i32> [#uses=1]
  store i32 %cond53, i32* %ui32EndX
  %tmp54 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %tmp55 = load i32* %roiEndX.addr                ; <i32> [#uses=1]
  %cmp56 = icmp ult i32 %tmp54, %tmp55            ; <i1> [#uses=1]
  br i1 %cmp56, label %if.then57, label %if.end60

if.then57:                                        ; preds = %if.end41
  %tmp58 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %dec59 = add i32 %tmp58, -1                     ; <i32> [#uses=1]
  store i32 %dec59, i32* %ui32EndX
  br label %if.end60

if.end60:                                         ; preds = %if.then57, %if.end41
  %tmp61 = load i16 addrspace(1)** %imageLumaChannel.addr ; <i16 addrspace(1)*> [#uses=1]
  %tmp62 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %tmp63 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  %mul64 = mul i32 %tmp62, %tmp63                 ; <i32> [#uses=1]
  %add.ptr65 = getelementptr inbounds i16 addrspace(1)* %tmp61, i32 %mul64 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr65, i16 addrspace(1)** %pLine
  %tmp66 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  store i32 %tmp66, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc111, %if.end60
  %tmp67 = load i32* %y                           ; <i32> [#uses=1]
  %tmp68 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %cmp69 = icmp ule i32 %tmp67, %tmp68            ; <i1> [#uses=1]
  br i1 %cmp69, label %for.body, label %for.end114

for.body:                                         ; preds = %for.cond
  %tmp70 = load i32* %ui32StartX                  ; <i32> [#uses=1]
  %tmp71 = load i32* %pix_phase                   ; <i32> [#uses=1]
  %add72 = add i32 %tmp70, %tmp71                 ; <i32> [#uses=1]
  store i32 %add72, i32* %x
  br label %for.cond73

for.cond73:                                       ; preds = %for.inc, %for.body
  %tmp74 = load i32* %x                           ; <i32> [#uses=1]
  %tmp75 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %cmp76 = icmp ule i32 %tmp74, %tmp75            ; <i1> [#uses=1]
  br i1 %cmp76, label %for.body77, label %for.end

for.body77:                                       ; preds = %for.cond73
  %tmp79 = load i32* %x                           ; <i32> [#uses=1]
  %tmp80 = load i16 addrspace(1)** %pLine         ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp80, i32 %tmp79 ; <i16 addrspace(1)*> [#uses=1]
  %tmp81 = load i16 addrspace(1)* %arrayidx       ; <i16> [#uses=1]
  %conv = zext i16 %tmp81 to i32                  ; <i32> [#uses=1]
  %add82 = add nsw i32 %conv, 4                   ; <i32> [#uses=1]
  %shr = ashr i32 %add82, 3                       ; <i32> [#uses=1]
  store i32 %shr, i32* %val
  %tmp83 = load i32* %val                         ; <i32> [#uses=1]
  %cmp84 = icmp slt i32 %tmp83, 127               ; <i1> [#uses=1]
  br i1 %cmp84, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body77
  %tmp86 = load i32* %val                         ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %for.body77
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond87 = phi i32 [ %tmp86, %cond.true ], [ 127, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond87, i32* %val
  %tmp88 = load i32* %val                         ; <i32> [#uses=1]
  %tmp89 = load i16 addrspace(1)** %pTempHist     ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx90 = getelementptr inbounds i16 addrspace(1)* %tmp89, i32 %tmp88 ; <i16 addrspace(1)*> [#uses=1]
  %tmp91 = load i16 addrspace(1)* %arrayidx90     ; <i16> [#uses=1]
  %conv92 = zext i16 %tmp91 to i32                ; <i32> [#uses=1]
  %cmp93 = icmp slt i32 %conv92, 7000             ; <i1> [#uses=1]
  br i1 %cmp93, label %if.then95, label %if.end100

if.then95:                                        ; preds = %cond.end
  %tmp96 = load i32* %val                         ; <i32> [#uses=1]
  %tmp97 = load i16 addrspace(1)** %pTempHist     ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx98 = getelementptr inbounds i16 addrspace(1)* %tmp97, i32 %tmp96 ; <i16 addrspace(1)*> [#uses=2]
  %tmp99 = load i16 addrspace(1)* %arrayidx98     ; <i16> [#uses=1]
  %inc = add i16 %tmp99, 1                        ; <i16> [#uses=1]
  store i16 %inc, i16 addrspace(1)* %arrayidx98
  br label %if.end100

if.end100:                                        ; preds = %if.then95, %cond.end
  br label %for.inc

for.inc:                                          ; preds = %if.end100
  %tmp101 = load i32* %x                          ; <i32> [#uses=1]
  %add102 = add i32 %tmp101, 4                    ; <i32> [#uses=1]
  store i32 %add102, i32* %x
  br label %for.cond73

for.end:                                          ; preds = %for.cond73
  %tmp103 = load i32* %pix_phase                  ; <i32> [#uses=1]
  %inc104 = add nsw i32 %tmp103, 1                ; <i32> [#uses=1]
  store i32 %inc104, i32* %pix_phase
  %tmp105 = load i32* %pix_phase                  ; <i32> [#uses=1]
  %rem106 = srem i32 %tmp105, 4                   ; <i32> [#uses=1]
  store i32 %rem106, i32* %pix_phase
  %tmp107 = load i32* %imageWidth.addr            ; <i32> [#uses=1]
  %mul108 = mul i32 %tmp107, 1                    ; <i32> [#uses=1]
  %tmp109 = load i16 addrspace(1)** %pLine        ; <i16 addrspace(1)*> [#uses=1]
  %add.ptr110 = getelementptr inbounds i16 addrspace(1)* %tmp109, i32 %mul108 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr110, i16 addrspace(1)** %pLine
  br label %for.inc111

for.inc111:                                       ; preds = %for.end
  %tmp112 = load i32* %y                          ; <i32> [#uses=1]
  %add113 = add i32 %tmp112, 1                    ; <i32> [#uses=1]
  store i32 %add113, i32* %y
  br label %for.cond

for.end114:                                       ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

define void @gatherHistograms(i32 %imageWidth, i32 %imageHeight, i32 %roiStartY, i32 %roiEndY, i32 %roiStartX, i32 %roiEndX, i32 %numberOfThreads, i16 addrspace(1)* %inputHistograms, i16 addrspace(1)* %outputHistogram) nounwind {
entry:
  %imageWidth.addr = alloca i32, align 4          ; <i32*> [#uses=1]
  %imageHeight.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %roiStartY.addr = alloca i32, align 4           ; <i32*> [#uses=1]
  %roiEndY.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %roiStartX.addr = alloca i32, align 4           ; <i32*> [#uses=1]
  %roiEndX.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %numberOfThreads.addr = alloca i32, align 4     ; <i32*> [#uses=2]
  %inputHistograms.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=3]
  %outputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=12]
  %i = alloca i32, align 4                        ; <i32*> [#uses=21]
  %j = alloca i32, align 4                        ; <i32*> [#uses=5]
  %histVal = alloca i16, align 2                  ; <i16*> [#uses=7]
  %iMax = alloca i32, align 4                     ; <i32*> [#uses=2]
  %ui32Diff = alloca i32, align 4                 ; <i32*> [#uses=2]
  store i32 %imageWidth, i32* %imageWidth.addr
  store i32 %imageHeight, i32* %imageHeight.addr
  store i32 %roiStartY, i32* %roiStartY.addr
  store i32 %roiEndY, i32* %roiEndY.addr
  store i32 %roiStartX, i32* %roiStartX.addr
  store i32 %roiEndX, i32* %roiEndX.addr
  store i32 %numberOfThreads, i32* %numberOfThreads.addr
  store i16 addrspace(1)* %inputHistograms, i16 addrspace(1)** %inputHistograms.addr
  store i16 addrspace(1)* %outputHistogram, i16 addrspace(1)** %outputHistogram.addr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc31, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 128                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end34

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32* %i                            ; <i32> [#uses=1]
  %tmp3 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp3, i32 %tmp2 ; <i16 addrspace(1)*> [#uses=1]
  %tmp4 = load i16 addrspace(1)* %arrayidx        ; <i16> [#uses=1]
  store i16 %tmp4, i16* %histVal
  store i32 1, i32* %j
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc, %for.body
  %tmp6 = load i32* %j                            ; <i32> [#uses=1]
  %tmp7 = load i32* %numberOfThreads.addr         ; <i32> [#uses=1]
  %cmp8 = icmp ult i32 %tmp6, %tmp7               ; <i1> [#uses=1]
  br i1 %cmp8, label %for.body9, label %for.end

for.body9:                                        ; preds = %for.cond5
  %tmp10 = load i32* %j                           ; <i32> [#uses=1]
  %mul = mul i32 128, %tmp10                      ; <i32> [#uses=1]
  %tmp11 = load i32* %i                           ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp11                     ; <i32> [#uses=1]
  %tmp12 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx13 = getelementptr inbounds i16 addrspace(1)* %tmp12, i32 %add ; <i16 addrspace(1)*> [#uses=1]
  %tmp14 = load i16 addrspace(1)* %arrayidx13     ; <i16> [#uses=1]
  %conv = zext i16 %tmp14 to i32                  ; <i32> [#uses=1]
  %tmp15 = load i16* %histVal                     ; <i16> [#uses=1]
  %conv16 = zext i16 %tmp15 to i32                ; <i32> [#uses=1]
  %add17 = add nsw i32 %conv16, %conv             ; <i32> [#uses=1]
  %conv18 = trunc i32 %add17 to i16               ; <i16> [#uses=1]
  store i16 %conv18, i16* %histVal
  %tmp19 = load i16* %histVal                     ; <i16> [#uses=1]
  %conv20 = zext i16 %tmp19 to i32                ; <i32> [#uses=1]
  %cmp21 = icmp slt i32 %conv20, 7000             ; <i1> [#uses=1]
  br i1 %cmp21, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body9
  %tmp23 = load i16* %histVal                     ; <i16> [#uses=1]
  %conv24 = zext i16 %tmp23 to i32                ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %for.body9
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv24, %cond.true ], [ 7000, %cond.false ] ; <i32> [#uses=1]
  %conv25 = trunc i32 %cond to i16                ; <i16> [#uses=1]
  store i16 %conv25, i16* %histVal
  br label %for.inc

for.inc:                                          ; preds = %cond.end
  %tmp26 = load i32* %j                           ; <i32> [#uses=1]
  %inc = add i32 %tmp26, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond5

for.end:                                          ; preds = %for.cond5
  %tmp27 = load i16* %histVal                     ; <i16> [#uses=1]
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %tmp29 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds i16 addrspace(1)* %tmp29, i32 %tmp28 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp27, i16 addrspace(1)* %arrayidx30
  br label %for.inc31

for.inc31:                                        ; preds = %for.end
  %tmp32 = load i32* %i                           ; <i32> [#uses=1]
  %inc33 = add nsw i32 %tmp32, 1                  ; <i32> [#uses=1]
  store i32 %inc33, i32* %i
  br label %for.cond

for.end34:                                        ; preds = %for.cond
  store i32 8, i32* %i
  br label %for.cond35

for.cond35:                                       ; preds = %for.inc115, %for.end34
  %tmp36 = load i32* %i                           ; <i32> [#uses=1]
  %cmp37 = icmp sle i32 %tmp36, 8                 ; <i1> [#uses=1]
  br i1 %cmp37, label %for.body39, label %for.end118

for.body39:                                       ; preds = %for.cond35
  %tmp41 = load i32* %i                           ; <i32> [#uses=1]
  %add42 = add nsw i32 %tmp41, 1                  ; <i32> [#uses=1]
  %tmp43 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds i16 addrspace(1)* %tmp43, i32 %add42 ; <i16 addrspace(1)*> [#uses=1]
  %tmp45 = load i16 addrspace(1)* %arrayidx44     ; <i16> [#uses=1]
  %conv46 = zext i16 %tmp45 to i32                ; <i32> [#uses=1]
  %tmp47 = load i32* %i                           ; <i32> [#uses=1]
  %sub = sub i32 %tmp47, 1                        ; <i32> [#uses=1]
  %tmp48 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx49 = getelementptr inbounds i16 addrspace(1)* %tmp48, i32 %sub ; <i16 addrspace(1)*> [#uses=1]
  %tmp50 = load i16 addrspace(1)* %arrayidx49     ; <i16> [#uses=1]
  %conv51 = zext i16 %tmp50 to i32                ; <i32> [#uses=1]
  %cmp52 = icmp sgt i32 %conv46, %conv51          ; <i1> [#uses=1]
  br i1 %cmp52, label %cond.true54, label %cond.false61

cond.true54:                                      ; preds = %for.body39
  %tmp55 = load i32* %i                           ; <i32> [#uses=1]
  %add56 = add nsw i32 %tmp55, 1                  ; <i32> [#uses=1]
  %tmp57 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx58 = getelementptr inbounds i16 addrspace(1)* %tmp57, i32 %add56 ; <i16 addrspace(1)*> [#uses=1]
  %tmp59 = load i16 addrspace(1)* %arrayidx58     ; <i16> [#uses=1]
  %conv60 = zext i16 %tmp59 to i32                ; <i32> [#uses=1]
  br label %cond.end68

cond.false61:                                     ; preds = %for.body39
  %tmp62 = load i32* %i                           ; <i32> [#uses=1]
  %sub63 = sub i32 %tmp62, 1                      ; <i32> [#uses=1]
  %tmp64 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds i16 addrspace(1)* %tmp64, i32 %sub63 ; <i16 addrspace(1)*> [#uses=1]
  %tmp66 = load i16 addrspace(1)* %arrayidx65     ; <i16> [#uses=1]
  %conv67 = zext i16 %tmp66 to i32                ; <i32> [#uses=1]
  br label %cond.end68

cond.end68:                                       ; preds = %cond.false61, %cond.true54
  %cond69 = phi i32 [ %conv60, %cond.true54 ], [ %conv67, %cond.false61 ] ; <i32> [#uses=1]
  store i32 %cond69, i32* %iMax
  %tmp70 = load i32* %i                           ; <i32> [#uses=1]
  %tmp71 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx72 = getelementptr inbounds i16 addrspace(1)* %tmp71, i32 %tmp70 ; <i16 addrspace(1)*> [#uses=1]
  %tmp73 = load i16 addrspace(1)* %arrayidx72     ; <i16> [#uses=1]
  %conv74 = zext i16 %tmp73 to i32                ; <i32> [#uses=1]
  %cmp75 = icmp sgt i32 %conv74, 1000             ; <i1> [#uses=1]
  br i1 %cmp75, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %cond.end68
  %tmp77 = load i32* %i                           ; <i32> [#uses=1]
  %tmp78 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx79 = getelementptr inbounds i16 addrspace(1)* %tmp78, i32 %tmp77 ; <i16 addrspace(1)*> [#uses=1]
  %tmp80 = load i16 addrspace(1)* %arrayidx79     ; <i16> [#uses=1]
  %conv81 = zext i16 %tmp80 to i32                ; <i32> [#uses=1]
  %tmp82 = load i32* %iMax                        ; <i32> [#uses=1]
  %sub83 = sub i32 %conv81, %tmp82                ; <i32> [#uses=1]
  %cmp84 = icmp sgt i32 %sub83, 1000              ; <i1> [#uses=1]
  br i1 %cmp84, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %tmp87 = load i32* %i                           ; <i32> [#uses=1]
  %tmp88 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx89 = getelementptr inbounds i16 addrspace(1)* %tmp88, i32 %tmp87 ; <i16 addrspace(1)*> [#uses=1]
  %tmp90 = load i16 addrspace(1)* %arrayidx89     ; <i16> [#uses=1]
  %conv91 = zext i16 %tmp90 to i32                ; <i32> [#uses=1]
  %tmp92 = load i32* %i                           ; <i32> [#uses=1]
  %sub93 = sub i32 %tmp92, 1                      ; <i32> [#uses=1]
  %tmp94 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds i16 addrspace(1)* %tmp94, i32 %sub93 ; <i16 addrspace(1)*> [#uses=1]
  %tmp96 = load i16 addrspace(1)* %arrayidx95     ; <i16> [#uses=1]
  %conv97 = zext i16 %tmp96 to i32                ; <i32> [#uses=1]
  %tmp98 = load i32* %i                           ; <i32> [#uses=1]
  %add99 = add nsw i32 %tmp98, 1                  ; <i32> [#uses=1]
  %tmp100 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx101 = getelementptr inbounds i16 addrspace(1)* %tmp100, i32 %add99 ; <i16 addrspace(1)*> [#uses=1]
  %tmp102 = load i16 addrspace(1)* %arrayidx101   ; <i16> [#uses=1]
  %conv103 = zext i16 %tmp102 to i32              ; <i32> [#uses=1]
  %add104 = add nsw i32 %conv97, %conv103         ; <i32> [#uses=1]
  %add105 = add nsw i32 %add104, 1                ; <i32> [#uses=1]
  %div = sdiv i32 %add105, 2                      ; <i32> [#uses=1]
  %sub106 = sub i32 %conv91, %div                 ; <i32> [#uses=1]
  store i32 %sub106, i32* %ui32Diff
  %tmp107 = load i32* %ui32Diff                   ; <i32> [#uses=1]
  %tmp108 = load i32* %i                          ; <i32> [#uses=1]
  %tmp109 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx110 = getelementptr inbounds i16 addrspace(1)* %tmp109, i32 %tmp108 ; <i16 addrspace(1)*> [#uses=2]
  %tmp111 = load i16 addrspace(1)* %arrayidx110   ; <i16> [#uses=1]
  %conv112 = zext i16 %tmp111 to i32              ; <i32> [#uses=1]
  %sub113 = sub i32 %conv112, %tmp107             ; <i32> [#uses=1]
  %conv114 = trunc i32 %sub113 to i16             ; <i16> [#uses=1]
  store i16 %conv114, i16 addrspace(1)* %arrayidx110
  br label %for.end118

if.end:                                           ; preds = %land.lhs.true, %cond.end68
  br label %for.inc115

for.inc115:                                       ; preds = %if.end
  %tmp116 = load i32* %i                          ; <i32> [#uses=1]
  %inc117 = add nsw i32 %tmp116, 1                ; <i32> [#uses=1]
  store i32 %inc117, i32* %i
  br label %for.cond35

for.end118:                                       ; preds = %if.then, %for.cond35
  ret void
}

define void @gatherHistograms_vector(i32 %imageWidth, i32 %imageHeight, i32 %roiStartY, i32 %roiEndY, i32 %roiStartX, i32 %roiEndX, i32 %numberOfThreads, i16 addrspace(1)* %inputHistograms, i16 addrspace(1)* %outputHistogram) nounwind {
entry:
  %imageWidth.addr = alloca i32, align 4          ; <i32*> [#uses=1]
  %imageHeight.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %roiStartY.addr = alloca i32, align 4           ; <i32*> [#uses=1]
  %roiEndY.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %roiStartX.addr = alloca i32, align 4           ; <i32*> [#uses=1]
  %roiEndX.addr = alloca i32, align 4             ; <i32*> [#uses=1]
  %numberOfThreads.addr = alloca i32, align 4     ; <i32*> [#uses=2]
  %inputHistograms.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=33]
  %outputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=27]
  %i = alloca i32, align 4                        ; <i32*> [#uses=66]
  %j = alloca i32, align 4                        ; <i32*> [#uses=20]
  %maxBinVector = alloca <16 x i16>, align 32     ; <<16 x i16>*> [#uses=17]
  %histVals = alloca <16 x i16>, align 32         ; <<16 x i16>*> [#uses=19]
  %.compoundliteral = alloca <16 x i16>, align 32 ; <<16 x i16>*> [#uses=2]
  %tmp361 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp375 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp389 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp403 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp417 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp431 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp445 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp459 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp473 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp487 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp501 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp515 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp529 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp543 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp557 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %tmp571 = alloca <16 x i16>, align 32           ; <<16 x i16>*> [#uses=2]
  %iMax = alloca i32, align 4                     ; <i32*> [#uses=2]
  %ui32Diff = alloca i32, align 4                 ; <i32*> [#uses=2]
  store i32 %imageWidth, i32* %imageWidth.addr
  store i32 %imageHeight, i32* %imageHeight.addr
  store i32 %roiStartY, i32* %roiStartY.addr
  store i32 %roiEndY, i32* %roiEndY.addr
  store i32 %roiStartX, i32* %roiStartX.addr
  store i32 %roiEndX, i32* %roiEndX.addr
  store i32 %numberOfThreads, i32* %numberOfThreads.addr
  store i16 addrspace(1)* %inputHistograms, i16 addrspace(1)** %inputHistograms.addr
  store i16 addrspace(1)* %outputHistogram, i16 addrspace(1)** %outputHistogram.addr
  store <16 x i16> <i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000, i16 7000>, <16 x i16>* %maxBinVector
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc582, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp, 8                     ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end585

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32* %i                            ; <i32> [#uses=1]
  %mul = mul i32 8, %tmp2                         ; <i32> [#uses=1]
  %add = add i32 %mul, 0                          ; <i32> [#uses=1]
  %tmp3 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp3, i32 %add ; <i16 addrspace(1)*> [#uses=1]
  %tmp4 = load i16 addrspace(1)* %arrayidx        ; <i16> [#uses=1]
  %conv = zext i16 %tmp4 to i32                   ; <i32> [#uses=1]
  %add5 = add nsw i32 %conv, 0                    ; <i32> [#uses=1]
  %conv6 = trunc i32 %add5 to i16                 ; <i16> [#uses=1]
  %vecinit = insertelement <16 x i16> undef, i16 %conv6, i32 0 ; <<16 x i16>> [#uses=1]
  %tmp7 = load i32* %i                            ; <i32> [#uses=1]
  %mul8 = mul i32 8, %tmp7                        ; <i32> [#uses=1]
  %add9 = add i32 %mul8, 1                        ; <i32> [#uses=1]
  %tmp10 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx11 = getelementptr inbounds i16 addrspace(1)* %tmp10, i32 %add9 ; <i16 addrspace(1)*> [#uses=1]
  %tmp12 = load i16 addrspace(1)* %arrayidx11     ; <i16> [#uses=1]
  %conv13 = zext i16 %tmp12 to i32                ; <i32> [#uses=1]
  %add14 = add nsw i32 %conv13, 0                 ; <i32> [#uses=1]
  %conv15 = trunc i32 %add14 to i16               ; <i16> [#uses=1]
  %vecinit16 = insertelement <16 x i16> %vecinit, i16 %conv15, i32 1 ; <<16 x i16>> [#uses=1]
  %tmp17 = load i32* %i                           ; <i32> [#uses=1]
  %mul18 = mul i32 8, %tmp17                      ; <i32> [#uses=1]
  %add19 = add i32 %mul18, 2                      ; <i32> [#uses=1]
  %tmp20 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx21 = getelementptr inbounds i16 addrspace(1)* %tmp20, i32 %add19 ; <i16 addrspace(1)*> [#uses=1]
  %tmp22 = load i16 addrspace(1)* %arrayidx21     ; <i16> [#uses=1]
  %conv23 = zext i16 %tmp22 to i32                ; <i32> [#uses=1]
  %add24 = add nsw i32 %conv23, 0                 ; <i32> [#uses=1]
  %conv25 = trunc i32 %add24 to i16               ; <i16> [#uses=1]
  %vecinit26 = insertelement <16 x i16> %vecinit16, i16 %conv25, i32 2 ; <<16 x i16>> [#uses=1]
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %mul28 = mul i32 8, %tmp27                      ; <i32> [#uses=1]
  %add29 = add i32 %mul28, 3                      ; <i32> [#uses=1]
  %tmp30 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx31 = getelementptr inbounds i16 addrspace(1)* %tmp30, i32 %add29 ; <i16 addrspace(1)*> [#uses=1]
  %tmp32 = load i16 addrspace(1)* %arrayidx31     ; <i16> [#uses=1]
  %conv33 = zext i16 %tmp32 to i32                ; <i32> [#uses=1]
  %add34 = add nsw i32 %conv33, 0                 ; <i32> [#uses=1]
  %conv35 = trunc i32 %add34 to i16               ; <i16> [#uses=1]
  %vecinit36 = insertelement <16 x i16> %vecinit26, i16 %conv35, i32 3 ; <<16 x i16>> [#uses=1]
  %tmp37 = load i32* %i                           ; <i32> [#uses=1]
  %mul38 = mul i32 8, %tmp37                      ; <i32> [#uses=1]
  %add39 = add i32 %mul38, 4                      ; <i32> [#uses=1]
  %tmp40 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds i16 addrspace(1)* %tmp40, i32 %add39 ; <i16 addrspace(1)*> [#uses=1]
  %tmp42 = load i16 addrspace(1)* %arrayidx41     ; <i16> [#uses=1]
  %conv43 = zext i16 %tmp42 to i32                ; <i32> [#uses=1]
  %add44 = add nsw i32 %conv43, 0                 ; <i32> [#uses=1]
  %conv45 = trunc i32 %add44 to i16               ; <i16> [#uses=1]
  %vecinit46 = insertelement <16 x i16> %vecinit36, i16 %conv45, i32 4 ; <<16 x i16>> [#uses=1]
  %tmp47 = load i32* %i                           ; <i32> [#uses=1]
  %mul48 = mul i32 8, %tmp47                      ; <i32> [#uses=1]
  %add49 = add i32 %mul48, 5                      ; <i32> [#uses=1]
  %tmp50 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds i16 addrspace(1)* %tmp50, i32 %add49 ; <i16 addrspace(1)*> [#uses=1]
  %tmp52 = load i16 addrspace(1)* %arrayidx51     ; <i16> [#uses=1]
  %conv53 = zext i16 %tmp52 to i32                ; <i32> [#uses=1]
  %add54 = add nsw i32 %conv53, 0                 ; <i32> [#uses=1]
  %conv55 = trunc i32 %add54 to i16               ; <i16> [#uses=1]
  %vecinit56 = insertelement <16 x i16> %vecinit46, i16 %conv55, i32 5 ; <<16 x i16>> [#uses=1]
  %tmp57 = load i32* %i                           ; <i32> [#uses=1]
  %mul58 = mul i32 8, %tmp57                      ; <i32> [#uses=1]
  %add59 = add i32 %mul58, 6                      ; <i32> [#uses=1]
  %tmp60 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx61 = getelementptr inbounds i16 addrspace(1)* %tmp60, i32 %add59 ; <i16 addrspace(1)*> [#uses=1]
  %tmp62 = load i16 addrspace(1)* %arrayidx61     ; <i16> [#uses=1]
  %conv63 = zext i16 %tmp62 to i32                ; <i32> [#uses=1]
  %add64 = add nsw i32 %conv63, 0                 ; <i32> [#uses=1]
  %conv65 = trunc i32 %add64 to i16               ; <i16> [#uses=1]
  %vecinit66 = insertelement <16 x i16> %vecinit56, i16 %conv65, i32 6 ; <<16 x i16>> [#uses=1]
  %tmp67 = load i32* %i                           ; <i32> [#uses=1]
  %mul68 = mul i32 8, %tmp67                      ; <i32> [#uses=1]
  %add69 = add i32 %mul68, 7                      ; <i32> [#uses=1]
  %tmp70 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx71 = getelementptr inbounds i16 addrspace(1)* %tmp70, i32 %add69 ; <i16 addrspace(1)*> [#uses=1]
  %tmp72 = load i16 addrspace(1)* %arrayidx71     ; <i16> [#uses=1]
  %conv73 = zext i16 %tmp72 to i32                ; <i32> [#uses=1]
  %add74 = add nsw i32 %conv73, 0                 ; <i32> [#uses=1]
  %conv75 = trunc i32 %add74 to i16               ; <i16> [#uses=1]
  %vecinit76 = insertelement <16 x i16> %vecinit66, i16 %conv75, i32 7 ; <<16 x i16>> [#uses=1]
  %tmp77 = load i32* %i                           ; <i32> [#uses=1]
  %mul78 = mul i32 8, %tmp77                      ; <i32> [#uses=1]
  %add79 = add i32 %mul78, 8                      ; <i32> [#uses=1]
  %tmp80 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx81 = getelementptr inbounds i16 addrspace(1)* %tmp80, i32 %add79 ; <i16 addrspace(1)*> [#uses=1]
  %tmp82 = load i16 addrspace(1)* %arrayidx81     ; <i16> [#uses=1]
  %conv83 = zext i16 %tmp82 to i32                ; <i32> [#uses=1]
  %add84 = add nsw i32 %conv83, 0                 ; <i32> [#uses=1]
  %conv85 = trunc i32 %add84 to i16               ; <i16> [#uses=1]
  %vecinit86 = insertelement <16 x i16> %vecinit76, i16 %conv85, i32 8 ; <<16 x i16>> [#uses=1]
  %tmp87 = load i32* %i                           ; <i32> [#uses=1]
  %mul88 = mul i32 8, %tmp87                      ; <i32> [#uses=1]
  %add89 = add i32 %mul88, 9                      ; <i32> [#uses=1]
  %tmp90 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds i16 addrspace(1)* %tmp90, i32 %add89 ; <i16 addrspace(1)*> [#uses=1]
  %tmp92 = load i16 addrspace(1)* %arrayidx91     ; <i16> [#uses=1]
  %conv93 = zext i16 %tmp92 to i32                ; <i32> [#uses=1]
  %add94 = add nsw i32 %conv93, 0                 ; <i32> [#uses=1]
  %conv95 = trunc i32 %add94 to i16               ; <i16> [#uses=1]
  %vecinit96 = insertelement <16 x i16> %vecinit86, i16 %conv95, i32 9 ; <<16 x i16>> [#uses=1]
  %tmp97 = load i32* %i                           ; <i32> [#uses=1]
  %mul98 = mul i32 8, %tmp97                      ; <i32> [#uses=1]
  %add99 = add i32 %mul98, 10                     ; <i32> [#uses=1]
  %tmp100 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx101 = getelementptr inbounds i16 addrspace(1)* %tmp100, i32 %add99 ; <i16 addrspace(1)*> [#uses=1]
  %tmp102 = load i16 addrspace(1)* %arrayidx101   ; <i16> [#uses=1]
  %conv103 = zext i16 %tmp102 to i32              ; <i32> [#uses=1]
  %add104 = add nsw i32 %conv103, 0               ; <i32> [#uses=1]
  %conv105 = trunc i32 %add104 to i16             ; <i16> [#uses=1]
  %vecinit106 = insertelement <16 x i16> %vecinit96, i16 %conv105, i32 10 ; <<16 x i16>> [#uses=1]
  %tmp107 = load i32* %i                          ; <i32> [#uses=1]
  %mul108 = mul i32 8, %tmp107                    ; <i32> [#uses=1]
  %add109 = add i32 %mul108, 11                   ; <i32> [#uses=1]
  %tmp110 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx111 = getelementptr inbounds i16 addrspace(1)* %tmp110, i32 %add109 ; <i16 addrspace(1)*> [#uses=1]
  %tmp112 = load i16 addrspace(1)* %arrayidx111   ; <i16> [#uses=1]
  %conv113 = zext i16 %tmp112 to i32              ; <i32> [#uses=1]
  %add114 = add nsw i32 %conv113, 0               ; <i32> [#uses=1]
  %conv115 = trunc i32 %add114 to i16             ; <i16> [#uses=1]
  %vecinit116 = insertelement <16 x i16> %vecinit106, i16 %conv115, i32 11 ; <<16 x i16>> [#uses=1]
  %tmp117 = load i32* %i                          ; <i32> [#uses=1]
  %mul118 = mul i32 8, %tmp117                    ; <i32> [#uses=1]
  %add119 = add i32 %mul118, 12                   ; <i32> [#uses=1]
  %tmp120 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds i16 addrspace(1)* %tmp120, i32 %add119 ; <i16 addrspace(1)*> [#uses=1]
  %tmp122 = load i16 addrspace(1)* %arrayidx121   ; <i16> [#uses=1]
  %conv123 = zext i16 %tmp122 to i32              ; <i32> [#uses=1]
  %add124 = add nsw i32 %conv123, 0               ; <i32> [#uses=1]
  %conv125 = trunc i32 %add124 to i16             ; <i16> [#uses=1]
  %vecinit126 = insertelement <16 x i16> %vecinit116, i16 %conv125, i32 12 ; <<16 x i16>> [#uses=1]
  %tmp127 = load i32* %i                          ; <i32> [#uses=1]
  %mul128 = mul i32 8, %tmp127                    ; <i32> [#uses=1]
  %add129 = add i32 %mul128, 13                   ; <i32> [#uses=1]
  %tmp130 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds i16 addrspace(1)* %tmp130, i32 %add129 ; <i16 addrspace(1)*> [#uses=1]
  %tmp132 = load i16 addrspace(1)* %arrayidx131   ; <i16> [#uses=1]
  %conv133 = zext i16 %tmp132 to i32              ; <i32> [#uses=1]
  %add134 = add nsw i32 %conv133, 0               ; <i32> [#uses=1]
  %conv135 = trunc i32 %add134 to i16             ; <i16> [#uses=1]
  %vecinit136 = insertelement <16 x i16> %vecinit126, i16 %conv135, i32 13 ; <<16 x i16>> [#uses=1]
  %tmp137 = load i32* %i                          ; <i32> [#uses=1]
  %mul138 = mul i32 8, %tmp137                    ; <i32> [#uses=1]
  %add139 = add i32 %mul138, 14                   ; <i32> [#uses=1]
  %tmp140 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx141 = getelementptr inbounds i16 addrspace(1)* %tmp140, i32 %add139 ; <i16 addrspace(1)*> [#uses=1]
  %tmp142 = load i16 addrspace(1)* %arrayidx141   ; <i16> [#uses=1]
  %conv143 = zext i16 %tmp142 to i32              ; <i32> [#uses=1]
  %add144 = add nsw i32 %conv143, 0               ; <i32> [#uses=1]
  %conv145 = trunc i32 %add144 to i16             ; <i16> [#uses=1]
  %vecinit146 = insertelement <16 x i16> %vecinit136, i16 %conv145, i32 14 ; <<16 x i16>> [#uses=1]
  %tmp147 = load i32* %i                          ; <i32> [#uses=1]
  %mul148 = mul i32 8, %tmp147                    ; <i32> [#uses=1]
  %add149 = add i32 %mul148, 15                   ; <i32> [#uses=1]
  %tmp150 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx151 = getelementptr inbounds i16 addrspace(1)* %tmp150, i32 %add149 ; <i16 addrspace(1)*> [#uses=1]
  %tmp152 = load i16 addrspace(1)* %arrayidx151   ; <i16> [#uses=1]
  %conv153 = zext i16 %tmp152 to i32              ; <i32> [#uses=1]
  %add154 = add nsw i32 %conv153, 0               ; <i32> [#uses=1]
  %conv155 = trunc i32 %add154 to i16             ; <i16> [#uses=1]
  %vecinit156 = insertelement <16 x i16> %vecinit146, i16 %conv155, i32 15 ; <<16 x i16>> [#uses=1]
  store <16 x i16> %vecinit156, <16 x i16>* %histVals
  store i32 1, i32* %j
  br label %for.cond157

for.cond157:                                      ; preds = %for.inc, %for.body
  %tmp158 = load i32* %j                          ; <i32> [#uses=1]
  %tmp159 = load i32* %numberOfThreads.addr       ; <i32> [#uses=1]
  %cmp160 = icmp ult i32 %tmp158, %tmp159         ; <i1> [#uses=1]
  br i1 %cmp160, label %for.body162, label %for.end

for.body162:                                      ; preds = %for.cond157
  %tmp163 = load i32* %i                          ; <i32> [#uses=1]
  %mul164 = mul i32 8, %tmp163                    ; <i32> [#uses=1]
  %add165 = add i32 %mul164, 0                    ; <i32> [#uses=1]
  %tmp166 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx167 = getelementptr inbounds i16 addrspace(1)* %tmp166, i32 %add165 ; <i16 addrspace(1)*> [#uses=1]
  %tmp168 = load i16 addrspace(1)* %arrayidx167   ; <i16> [#uses=1]
  %conv169 = zext i16 %tmp168 to i32              ; <i32> [#uses=1]
  %tmp170 = load i32* %j                          ; <i32> [#uses=1]
  %mul171 = mul i32 128, %tmp170                  ; <i32> [#uses=1]
  %add172 = add i32 %conv169, %mul171             ; <i32> [#uses=1]
  %conv173 = trunc i32 %add172 to i16             ; <i16> [#uses=1]
  %vecinit174 = insertelement <16 x i16> undef, i16 %conv173, i32 0 ; <<16 x i16>> [#uses=1]
  %tmp175 = load i32* %i                          ; <i32> [#uses=1]
  %mul176 = mul i32 8, %tmp175                    ; <i32> [#uses=1]
  %add177 = add i32 %mul176, 1                    ; <i32> [#uses=1]
  %tmp178 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx179 = getelementptr inbounds i16 addrspace(1)* %tmp178, i32 %add177 ; <i16 addrspace(1)*> [#uses=1]
  %tmp180 = load i16 addrspace(1)* %arrayidx179   ; <i16> [#uses=1]
  %conv181 = zext i16 %tmp180 to i32              ; <i32> [#uses=1]
  %tmp182 = load i32* %j                          ; <i32> [#uses=1]
  %mul183 = mul i32 128, %tmp182                  ; <i32> [#uses=1]
  %add184 = add i32 %conv181, %mul183             ; <i32> [#uses=1]
  %conv185 = trunc i32 %add184 to i16             ; <i16> [#uses=1]
  %vecinit186 = insertelement <16 x i16> %vecinit174, i16 %conv185, i32 1 ; <<16 x i16>> [#uses=1]
  %tmp187 = load i32* %i                          ; <i32> [#uses=1]
  %mul188 = mul i32 8, %tmp187                    ; <i32> [#uses=1]
  %add189 = add i32 %mul188, 2                    ; <i32> [#uses=1]
  %tmp190 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx191 = getelementptr inbounds i16 addrspace(1)* %tmp190, i32 %add189 ; <i16 addrspace(1)*> [#uses=1]
  %tmp192 = load i16 addrspace(1)* %arrayidx191   ; <i16> [#uses=1]
  %conv193 = zext i16 %tmp192 to i32              ; <i32> [#uses=1]
  %tmp194 = load i32* %j                          ; <i32> [#uses=1]
  %mul195 = mul i32 128, %tmp194                  ; <i32> [#uses=1]
  %add196 = add i32 %conv193, %mul195             ; <i32> [#uses=1]
  %conv197 = trunc i32 %add196 to i16             ; <i16> [#uses=1]
  %vecinit198 = insertelement <16 x i16> %vecinit186, i16 %conv197, i32 2 ; <<16 x i16>> [#uses=1]
  %tmp199 = load i32* %i                          ; <i32> [#uses=1]
  %mul200 = mul i32 8, %tmp199                    ; <i32> [#uses=1]
  %add201 = add i32 %mul200, 3                    ; <i32> [#uses=1]
  %tmp202 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx203 = getelementptr inbounds i16 addrspace(1)* %tmp202, i32 %add201 ; <i16 addrspace(1)*> [#uses=1]
  %tmp204 = load i16 addrspace(1)* %arrayidx203   ; <i16> [#uses=1]
  %conv205 = zext i16 %tmp204 to i32              ; <i32> [#uses=1]
  %tmp206 = load i32* %j                          ; <i32> [#uses=1]
  %mul207 = mul i32 128, %tmp206                  ; <i32> [#uses=1]
  %add208 = add i32 %conv205, %mul207             ; <i32> [#uses=1]
  %conv209 = trunc i32 %add208 to i16             ; <i16> [#uses=1]
  %vecinit210 = insertelement <16 x i16> %vecinit198, i16 %conv209, i32 3 ; <<16 x i16>> [#uses=1]
  %tmp211 = load i32* %i                          ; <i32> [#uses=1]
  %mul212 = mul i32 8, %tmp211                    ; <i32> [#uses=1]
  %add213 = add i32 %mul212, 4                    ; <i32> [#uses=1]
  %tmp214 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx215 = getelementptr inbounds i16 addrspace(1)* %tmp214, i32 %add213 ; <i16 addrspace(1)*> [#uses=1]
  %tmp216 = load i16 addrspace(1)* %arrayidx215   ; <i16> [#uses=1]
  %conv217 = zext i16 %tmp216 to i32              ; <i32> [#uses=1]
  %tmp218 = load i32* %j                          ; <i32> [#uses=1]
  %mul219 = mul i32 128, %tmp218                  ; <i32> [#uses=1]
  %add220 = add i32 %conv217, %mul219             ; <i32> [#uses=1]
  %conv221 = trunc i32 %add220 to i16             ; <i16> [#uses=1]
  %vecinit222 = insertelement <16 x i16> %vecinit210, i16 %conv221, i32 4 ; <<16 x i16>> [#uses=1]
  %tmp223 = load i32* %i                          ; <i32> [#uses=1]
  %mul224 = mul i32 8, %tmp223                    ; <i32> [#uses=1]
  %add225 = add i32 %mul224, 5                    ; <i32> [#uses=1]
  %tmp226 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx227 = getelementptr inbounds i16 addrspace(1)* %tmp226, i32 %add225 ; <i16 addrspace(1)*> [#uses=1]
  %tmp228 = load i16 addrspace(1)* %arrayidx227   ; <i16> [#uses=1]
  %conv229 = zext i16 %tmp228 to i32              ; <i32> [#uses=1]
  %tmp230 = load i32* %j                          ; <i32> [#uses=1]
  %mul231 = mul i32 128, %tmp230                  ; <i32> [#uses=1]
  %add232 = add i32 %conv229, %mul231             ; <i32> [#uses=1]
  %conv233 = trunc i32 %add232 to i16             ; <i16> [#uses=1]
  %vecinit234 = insertelement <16 x i16> %vecinit222, i16 %conv233, i32 5 ; <<16 x i16>> [#uses=1]
  %tmp235 = load i32* %i                          ; <i32> [#uses=1]
  %mul236 = mul i32 8, %tmp235                    ; <i32> [#uses=1]
  %add237 = add i32 %mul236, 6                    ; <i32> [#uses=1]
  %tmp238 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx239 = getelementptr inbounds i16 addrspace(1)* %tmp238, i32 %add237 ; <i16 addrspace(1)*> [#uses=1]
  %tmp240 = load i16 addrspace(1)* %arrayidx239   ; <i16> [#uses=1]
  %conv241 = zext i16 %tmp240 to i32              ; <i32> [#uses=1]
  %tmp242 = load i32* %j                          ; <i32> [#uses=1]
  %mul243 = mul i32 128, %tmp242                  ; <i32> [#uses=1]
  %add244 = add i32 %conv241, %mul243             ; <i32> [#uses=1]
  %conv245 = trunc i32 %add244 to i16             ; <i16> [#uses=1]
  %vecinit246 = insertelement <16 x i16> %vecinit234, i16 %conv245, i32 6 ; <<16 x i16>> [#uses=1]
  %tmp247 = load i32* %i                          ; <i32> [#uses=1]
  %mul248 = mul i32 8, %tmp247                    ; <i32> [#uses=1]
  %add249 = add i32 %mul248, 7                    ; <i32> [#uses=1]
  %tmp250 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx251 = getelementptr inbounds i16 addrspace(1)* %tmp250, i32 %add249 ; <i16 addrspace(1)*> [#uses=1]
  %tmp252 = load i16 addrspace(1)* %arrayidx251   ; <i16> [#uses=1]
  %conv253 = zext i16 %tmp252 to i32              ; <i32> [#uses=1]
  %tmp254 = load i32* %j                          ; <i32> [#uses=1]
  %mul255 = mul i32 128, %tmp254                  ; <i32> [#uses=1]
  %add256 = add i32 %conv253, %mul255             ; <i32> [#uses=1]
  %conv257 = trunc i32 %add256 to i16             ; <i16> [#uses=1]
  %vecinit258 = insertelement <16 x i16> %vecinit246, i16 %conv257, i32 7 ; <<16 x i16>> [#uses=1]
  %tmp259 = load i32* %i                          ; <i32> [#uses=1]
  %mul260 = mul i32 8, %tmp259                    ; <i32> [#uses=1]
  %add261 = add i32 %mul260, 8                    ; <i32> [#uses=1]
  %tmp262 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx263 = getelementptr inbounds i16 addrspace(1)* %tmp262, i32 %add261 ; <i16 addrspace(1)*> [#uses=1]
  %tmp264 = load i16 addrspace(1)* %arrayidx263   ; <i16> [#uses=1]
  %conv265 = zext i16 %tmp264 to i32              ; <i32> [#uses=1]
  %tmp266 = load i32* %j                          ; <i32> [#uses=1]
  %mul267 = mul i32 128, %tmp266                  ; <i32> [#uses=1]
  %add268 = add i32 %conv265, %mul267             ; <i32> [#uses=1]
  %conv269 = trunc i32 %add268 to i16             ; <i16> [#uses=1]
  %vecinit270 = insertelement <16 x i16> %vecinit258, i16 %conv269, i32 8 ; <<16 x i16>> [#uses=1]
  %tmp271 = load i32* %i                          ; <i32> [#uses=1]
  %mul272 = mul i32 8, %tmp271                    ; <i32> [#uses=1]
  %add273 = add i32 %mul272, 9                    ; <i32> [#uses=1]
  %tmp274 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx275 = getelementptr inbounds i16 addrspace(1)* %tmp274, i32 %add273 ; <i16 addrspace(1)*> [#uses=1]
  %tmp276 = load i16 addrspace(1)* %arrayidx275   ; <i16> [#uses=1]
  %conv277 = zext i16 %tmp276 to i32              ; <i32> [#uses=1]
  %tmp278 = load i32* %j                          ; <i32> [#uses=1]
  %mul279 = mul i32 128, %tmp278                  ; <i32> [#uses=1]
  %add280 = add i32 %conv277, %mul279             ; <i32> [#uses=1]
  %conv281 = trunc i32 %add280 to i16             ; <i16> [#uses=1]
  %vecinit282 = insertelement <16 x i16> %vecinit270, i16 %conv281, i32 9 ; <<16 x i16>> [#uses=1]
  %tmp283 = load i32* %i                          ; <i32> [#uses=1]
  %mul284 = mul i32 8, %tmp283                    ; <i32> [#uses=1]
  %add285 = add i32 %mul284, 10                   ; <i32> [#uses=1]
  %tmp286 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx287 = getelementptr inbounds i16 addrspace(1)* %tmp286, i32 %add285 ; <i16 addrspace(1)*> [#uses=1]
  %tmp288 = load i16 addrspace(1)* %arrayidx287   ; <i16> [#uses=1]
  %conv289 = zext i16 %tmp288 to i32              ; <i32> [#uses=1]
  %tmp290 = load i32* %j                          ; <i32> [#uses=1]
  %mul291 = mul i32 128, %tmp290                  ; <i32> [#uses=1]
  %add292 = add i32 %conv289, %mul291             ; <i32> [#uses=1]
  %conv293 = trunc i32 %add292 to i16             ; <i16> [#uses=1]
  %vecinit294 = insertelement <16 x i16> %vecinit282, i16 %conv293, i32 10 ; <<16 x i16>> [#uses=1]
  %tmp295 = load i32* %i                          ; <i32> [#uses=1]
  %mul296 = mul i32 8, %tmp295                    ; <i32> [#uses=1]
  %add297 = add i32 %mul296, 11                   ; <i32> [#uses=1]
  %tmp298 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx299 = getelementptr inbounds i16 addrspace(1)* %tmp298, i32 %add297 ; <i16 addrspace(1)*> [#uses=1]
  %tmp300 = load i16 addrspace(1)* %arrayidx299   ; <i16> [#uses=1]
  %conv301 = zext i16 %tmp300 to i32              ; <i32> [#uses=1]
  %tmp302 = load i32* %j                          ; <i32> [#uses=1]
  %mul303 = mul i32 128, %tmp302                  ; <i32> [#uses=1]
  %add304 = add i32 %conv301, %mul303             ; <i32> [#uses=1]
  %conv305 = trunc i32 %add304 to i16             ; <i16> [#uses=1]
  %vecinit306 = insertelement <16 x i16> %vecinit294, i16 %conv305, i32 11 ; <<16 x i16>> [#uses=1]
  %tmp307 = load i32* %i                          ; <i32> [#uses=1]
  %mul308 = mul i32 8, %tmp307                    ; <i32> [#uses=1]
  %add309 = add i32 %mul308, 12                   ; <i32> [#uses=1]
  %tmp310 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx311 = getelementptr inbounds i16 addrspace(1)* %tmp310, i32 %add309 ; <i16 addrspace(1)*> [#uses=1]
  %tmp312 = load i16 addrspace(1)* %arrayidx311   ; <i16> [#uses=1]
  %conv313 = zext i16 %tmp312 to i32              ; <i32> [#uses=1]
  %tmp314 = load i32* %j                          ; <i32> [#uses=1]
  %mul315 = mul i32 128, %tmp314                  ; <i32> [#uses=1]
  %add316 = add i32 %conv313, %mul315             ; <i32> [#uses=1]
  %conv317 = trunc i32 %add316 to i16             ; <i16> [#uses=1]
  %vecinit318 = insertelement <16 x i16> %vecinit306, i16 %conv317, i32 12 ; <<16 x i16>> [#uses=1]
  %tmp319 = load i32* %i                          ; <i32> [#uses=1]
  %mul320 = mul i32 8, %tmp319                    ; <i32> [#uses=1]
  %add321 = add i32 %mul320, 13                   ; <i32> [#uses=1]
  %tmp322 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx323 = getelementptr inbounds i16 addrspace(1)* %tmp322, i32 %add321 ; <i16 addrspace(1)*> [#uses=1]
  %tmp324 = load i16 addrspace(1)* %arrayidx323   ; <i16> [#uses=1]
  %conv325 = zext i16 %tmp324 to i32              ; <i32> [#uses=1]
  %tmp326 = load i32* %j                          ; <i32> [#uses=1]
  %mul327 = mul i32 128, %tmp326                  ; <i32> [#uses=1]
  %add328 = add i32 %conv325, %mul327             ; <i32> [#uses=1]
  %conv329 = trunc i32 %add328 to i16             ; <i16> [#uses=1]
  %vecinit330 = insertelement <16 x i16> %vecinit318, i16 %conv329, i32 13 ; <<16 x i16>> [#uses=1]
  %tmp331 = load i32* %i                          ; <i32> [#uses=1]
  %mul332 = mul i32 8, %tmp331                    ; <i32> [#uses=1]
  %add333 = add i32 %mul332, 14                   ; <i32> [#uses=1]
  %tmp334 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx335 = getelementptr inbounds i16 addrspace(1)* %tmp334, i32 %add333 ; <i16 addrspace(1)*> [#uses=1]
  %tmp336 = load i16 addrspace(1)* %arrayidx335   ; <i16> [#uses=1]
  %conv337 = zext i16 %tmp336 to i32              ; <i32> [#uses=1]
  %tmp338 = load i32* %j                          ; <i32> [#uses=1]
  %mul339 = mul i32 128, %tmp338                  ; <i32> [#uses=1]
  %add340 = add i32 %conv337, %mul339             ; <i32> [#uses=1]
  %conv341 = trunc i32 %add340 to i16             ; <i16> [#uses=1]
  %vecinit342 = insertelement <16 x i16> %vecinit330, i16 %conv341, i32 14 ; <<16 x i16>> [#uses=1]
  %tmp343 = load i32* %i                          ; <i32> [#uses=1]
  %mul344 = mul i32 8, %tmp343                    ; <i32> [#uses=1]
  %add345 = add i32 %mul344, 15                   ; <i32> [#uses=1]
  %tmp346 = load i16 addrspace(1)** %inputHistograms.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx347 = getelementptr inbounds i16 addrspace(1)* %tmp346, i32 %add345 ; <i16 addrspace(1)*> [#uses=1]
  %tmp348 = load i16 addrspace(1)* %arrayidx347   ; <i16> [#uses=1]
  %conv349 = zext i16 %tmp348 to i32              ; <i32> [#uses=1]
  %tmp350 = load i32* %j                          ; <i32> [#uses=1]
  %mul351 = mul i32 128, %tmp350                  ; <i32> [#uses=1]
  %add352 = add i32 %conv349, %mul351             ; <i32> [#uses=1]
  %conv353 = trunc i32 %add352 to i16             ; <i16> [#uses=1]
  %vecinit354 = insertelement <16 x i16> %vecinit342, i16 %conv353, i32 15 ; <<16 x i16>> [#uses=1]
  store <16 x i16> %vecinit354, <16 x i16>* %.compoundliteral
  %tmp355 = load <16 x i16>* %.compoundliteral    ; <<16 x i16>> [#uses=1]
  %tmp356 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %add357 = add <16 x i16> %tmp356, %tmp355       ; <<16 x i16>> [#uses=1]
  store <16 x i16> %add357, <16 x i16>* %histVals
  br label %for.inc

for.inc:                                          ; preds = %for.body162
  %tmp358 = load i32* %j                          ; <i32> [#uses=1]
  %inc = add i32 %tmp358, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond157

for.end:                                          ; preds = %for.cond157
  %tmp359 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp360 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp359, <16 x i16> %tmp360) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call, <16 x i16>* %tmp361
  %tmp362 = load <16 x i16>* %tmp361              ; <<16 x i16>> [#uses=1]
  %tmp363 = extractelement <16 x i16> %tmp362, i32 0 ; <i16> [#uses=1]
  %conv364 = zext i16 %tmp363 to i32              ; <i32> [#uses=1]
  %add365 = add nsw i32 %conv364, 0               ; <i32> [#uses=1]
  %conv366 = trunc i32 %add365 to i16             ; <i16> [#uses=1]
  %tmp367 = load i32* %i                          ; <i32> [#uses=1]
  %mul368 = mul i32 8, %tmp367                    ; <i32> [#uses=1]
  %add369 = add i32 %mul368, 0                    ; <i32> [#uses=1]
  %tmp370 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx371 = getelementptr inbounds i16 addrspace(1)* %tmp370, i32 %add369 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv366, i16 addrspace(1)* %arrayidx371
  %tmp372 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp373 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call374 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp372, <16 x i16> %tmp373) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call374, <16 x i16>* %tmp375
  %tmp376 = load <16 x i16>* %tmp375              ; <<16 x i16>> [#uses=1]
  %tmp377 = extractelement <16 x i16> %tmp376, i32 1 ; <i16> [#uses=1]
  %conv378 = zext i16 %tmp377 to i32              ; <i32> [#uses=1]
  %add379 = add nsw i32 %conv378, 0               ; <i32> [#uses=1]
  %conv380 = trunc i32 %add379 to i16             ; <i16> [#uses=1]
  %tmp381 = load i32* %i                          ; <i32> [#uses=1]
  %mul382 = mul i32 8, %tmp381                    ; <i32> [#uses=1]
  %add383 = add i32 %mul382, 1                    ; <i32> [#uses=1]
  %tmp384 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx385 = getelementptr inbounds i16 addrspace(1)* %tmp384, i32 %add383 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv380, i16 addrspace(1)* %arrayidx385
  %tmp386 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp387 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call388 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp386, <16 x i16> %tmp387) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call388, <16 x i16>* %tmp389
  %tmp390 = load <16 x i16>* %tmp389              ; <<16 x i16>> [#uses=1]
  %tmp391 = extractelement <16 x i16> %tmp390, i32 2 ; <i16> [#uses=1]
  %conv392 = zext i16 %tmp391 to i32              ; <i32> [#uses=1]
  %add393 = add nsw i32 %conv392, 0               ; <i32> [#uses=1]
  %conv394 = trunc i32 %add393 to i16             ; <i16> [#uses=1]
  %tmp395 = load i32* %i                          ; <i32> [#uses=1]
  %mul396 = mul i32 8, %tmp395                    ; <i32> [#uses=1]
  %add397 = add i32 %mul396, 2                    ; <i32> [#uses=1]
  %tmp398 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx399 = getelementptr inbounds i16 addrspace(1)* %tmp398, i32 %add397 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv394, i16 addrspace(1)* %arrayidx399
  %tmp400 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp401 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call402 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp400, <16 x i16> %tmp401) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call402, <16 x i16>* %tmp403
  %tmp404 = load <16 x i16>* %tmp403              ; <<16 x i16>> [#uses=1]
  %tmp405 = extractelement <16 x i16> %tmp404, i32 3 ; <i16> [#uses=1]
  %conv406 = zext i16 %tmp405 to i32              ; <i32> [#uses=1]
  %add407 = add nsw i32 %conv406, 0               ; <i32> [#uses=1]
  %conv408 = trunc i32 %add407 to i16             ; <i16> [#uses=1]
  %tmp409 = load i32* %i                          ; <i32> [#uses=1]
  %mul410 = mul i32 8, %tmp409                    ; <i32> [#uses=1]
  %add411 = add i32 %mul410, 3                    ; <i32> [#uses=1]
  %tmp412 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx413 = getelementptr inbounds i16 addrspace(1)* %tmp412, i32 %add411 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv408, i16 addrspace(1)* %arrayidx413
  %tmp414 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp415 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call416 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp414, <16 x i16> %tmp415) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call416, <16 x i16>* %tmp417
  %tmp418 = load <16 x i16>* %tmp417              ; <<16 x i16>> [#uses=1]
  %tmp419 = extractelement <16 x i16> %tmp418, i32 4 ; <i16> [#uses=1]
  %conv420 = zext i16 %tmp419 to i32              ; <i32> [#uses=1]
  %add421 = add nsw i32 %conv420, 0               ; <i32> [#uses=1]
  %conv422 = trunc i32 %add421 to i16             ; <i16> [#uses=1]
  %tmp423 = load i32* %i                          ; <i32> [#uses=1]
  %mul424 = mul i32 8, %tmp423                    ; <i32> [#uses=1]
  %add425 = add i32 %mul424, 4                    ; <i32> [#uses=1]
  %tmp426 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx427 = getelementptr inbounds i16 addrspace(1)* %tmp426, i32 %add425 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv422, i16 addrspace(1)* %arrayidx427
  %tmp428 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp429 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call430 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp428, <16 x i16> %tmp429) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call430, <16 x i16>* %tmp431
  %tmp432 = load <16 x i16>* %tmp431              ; <<16 x i16>> [#uses=1]
  %tmp433 = extractelement <16 x i16> %tmp432, i32 5 ; <i16> [#uses=1]
  %conv434 = zext i16 %tmp433 to i32              ; <i32> [#uses=1]
  %add435 = add nsw i32 %conv434, 0               ; <i32> [#uses=1]
  %conv436 = trunc i32 %add435 to i16             ; <i16> [#uses=1]
  %tmp437 = load i32* %i                          ; <i32> [#uses=1]
  %mul438 = mul i32 8, %tmp437                    ; <i32> [#uses=1]
  %add439 = add i32 %mul438, 5                    ; <i32> [#uses=1]
  %tmp440 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx441 = getelementptr inbounds i16 addrspace(1)* %tmp440, i32 %add439 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv436, i16 addrspace(1)* %arrayidx441
  %tmp442 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp443 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call444 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp442, <16 x i16> %tmp443) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call444, <16 x i16>* %tmp445
  %tmp446 = load <16 x i16>* %tmp445              ; <<16 x i16>> [#uses=1]
  %tmp447 = extractelement <16 x i16> %tmp446, i32 6 ; <i16> [#uses=1]
  %conv448 = zext i16 %tmp447 to i32              ; <i32> [#uses=1]
  %add449 = add nsw i32 %conv448, 0               ; <i32> [#uses=1]
  %conv450 = trunc i32 %add449 to i16             ; <i16> [#uses=1]
  %tmp451 = load i32* %i                          ; <i32> [#uses=1]
  %mul452 = mul i32 8, %tmp451                    ; <i32> [#uses=1]
  %add453 = add i32 %mul452, 6                    ; <i32> [#uses=1]
  %tmp454 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx455 = getelementptr inbounds i16 addrspace(1)* %tmp454, i32 %add453 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv450, i16 addrspace(1)* %arrayidx455
  %tmp456 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp457 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call458 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp456, <16 x i16> %tmp457) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call458, <16 x i16>* %tmp459
  %tmp460 = load <16 x i16>* %tmp459              ; <<16 x i16>> [#uses=1]
  %tmp461 = extractelement <16 x i16> %tmp460, i32 7 ; <i16> [#uses=1]
  %conv462 = zext i16 %tmp461 to i32              ; <i32> [#uses=1]
  %add463 = add nsw i32 %conv462, 0               ; <i32> [#uses=1]
  %conv464 = trunc i32 %add463 to i16             ; <i16> [#uses=1]
  %tmp465 = load i32* %i                          ; <i32> [#uses=1]
  %mul466 = mul i32 8, %tmp465                    ; <i32> [#uses=1]
  %add467 = add i32 %mul466, 7                    ; <i32> [#uses=1]
  %tmp468 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx469 = getelementptr inbounds i16 addrspace(1)* %tmp468, i32 %add467 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv464, i16 addrspace(1)* %arrayidx469
  %tmp470 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp471 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call472 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp470, <16 x i16> %tmp471) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call472, <16 x i16>* %tmp473
  %tmp474 = load <16 x i16>* %tmp473              ; <<16 x i16>> [#uses=1]
  %tmp475 = extractelement <16 x i16> %tmp474, i32 8 ; <i16> [#uses=1]
  %conv476 = zext i16 %tmp475 to i32              ; <i32> [#uses=1]
  %add477 = add nsw i32 %conv476, 0               ; <i32> [#uses=1]
  %conv478 = trunc i32 %add477 to i16             ; <i16> [#uses=1]
  %tmp479 = load i32* %i                          ; <i32> [#uses=1]
  %mul480 = mul i32 8, %tmp479                    ; <i32> [#uses=1]
  %add481 = add i32 %mul480, 8                    ; <i32> [#uses=1]
  %tmp482 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx483 = getelementptr inbounds i16 addrspace(1)* %tmp482, i32 %add481 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv478, i16 addrspace(1)* %arrayidx483
  %tmp484 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp485 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call486 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp484, <16 x i16> %tmp485) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call486, <16 x i16>* %tmp487
  %tmp488 = load <16 x i16>* %tmp487              ; <<16 x i16>> [#uses=1]
  %tmp489 = extractelement <16 x i16> %tmp488, i32 9 ; <i16> [#uses=1]
  %conv490 = zext i16 %tmp489 to i32              ; <i32> [#uses=1]
  %add491 = add nsw i32 %conv490, 0               ; <i32> [#uses=1]
  %conv492 = trunc i32 %add491 to i16             ; <i16> [#uses=1]
  %tmp493 = load i32* %i                          ; <i32> [#uses=1]
  %mul494 = mul i32 8, %tmp493                    ; <i32> [#uses=1]
  %add495 = add i32 %mul494, 9                    ; <i32> [#uses=1]
  %tmp496 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx497 = getelementptr inbounds i16 addrspace(1)* %tmp496, i32 %add495 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv492, i16 addrspace(1)* %arrayidx497
  %tmp498 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp499 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call500 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp498, <16 x i16> %tmp499) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call500, <16 x i16>* %tmp501
  %tmp502 = load <16 x i16>* %tmp501              ; <<16 x i16>> [#uses=1]
  %tmp503 = extractelement <16 x i16> %tmp502, i32 10 ; <i16> [#uses=1]
  %conv504 = zext i16 %tmp503 to i32              ; <i32> [#uses=1]
  %add505 = add nsw i32 %conv504, 0               ; <i32> [#uses=1]
  %conv506 = trunc i32 %add505 to i16             ; <i16> [#uses=1]
  %tmp507 = load i32* %i                          ; <i32> [#uses=1]
  %mul508 = mul i32 8, %tmp507                    ; <i32> [#uses=1]
  %add509 = add i32 %mul508, 10                   ; <i32> [#uses=1]
  %tmp510 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx511 = getelementptr inbounds i16 addrspace(1)* %tmp510, i32 %add509 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv506, i16 addrspace(1)* %arrayidx511
  %tmp512 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp513 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call514 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp512, <16 x i16> %tmp513) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call514, <16 x i16>* %tmp515
  %tmp516 = load <16 x i16>* %tmp515              ; <<16 x i16>> [#uses=1]
  %tmp517 = extractelement <16 x i16> %tmp516, i32 11 ; <i16> [#uses=1]
  %conv518 = zext i16 %tmp517 to i32              ; <i32> [#uses=1]
  %add519 = add nsw i32 %conv518, 0               ; <i32> [#uses=1]
  %conv520 = trunc i32 %add519 to i16             ; <i16> [#uses=1]
  %tmp521 = load i32* %i                          ; <i32> [#uses=1]
  %mul522 = mul i32 8, %tmp521                    ; <i32> [#uses=1]
  %add523 = add i32 %mul522, 11                   ; <i32> [#uses=1]
  %tmp524 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx525 = getelementptr inbounds i16 addrspace(1)* %tmp524, i32 %add523 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv520, i16 addrspace(1)* %arrayidx525
  %tmp526 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp527 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call528 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp526, <16 x i16> %tmp527) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call528, <16 x i16>* %tmp529
  %tmp530 = load <16 x i16>* %tmp529              ; <<16 x i16>> [#uses=1]
  %tmp531 = extractelement <16 x i16> %tmp530, i32 12 ; <i16> [#uses=1]
  %conv532 = zext i16 %tmp531 to i32              ; <i32> [#uses=1]
  %add533 = add nsw i32 %conv532, 0               ; <i32> [#uses=1]
  %conv534 = trunc i32 %add533 to i16             ; <i16> [#uses=1]
  %tmp535 = load i32* %i                          ; <i32> [#uses=1]
  %mul536 = mul i32 8, %tmp535                    ; <i32> [#uses=1]
  %add537 = add i32 %mul536, 12                   ; <i32> [#uses=1]
  %tmp538 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx539 = getelementptr inbounds i16 addrspace(1)* %tmp538, i32 %add537 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv534, i16 addrspace(1)* %arrayidx539
  %tmp540 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp541 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call542 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp540, <16 x i16> %tmp541) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call542, <16 x i16>* %tmp543
  %tmp544 = load <16 x i16>* %tmp543              ; <<16 x i16>> [#uses=1]
  %tmp545 = extractelement <16 x i16> %tmp544, i32 13 ; <i16> [#uses=1]
  %conv546 = zext i16 %tmp545 to i32              ; <i32> [#uses=1]
  %add547 = add nsw i32 %conv546, 0               ; <i32> [#uses=1]
  %conv548 = trunc i32 %add547 to i16             ; <i16> [#uses=1]
  %tmp549 = load i32* %i                          ; <i32> [#uses=1]
  %mul550 = mul i32 8, %tmp549                    ; <i32> [#uses=1]
  %add551 = add i32 %mul550, 13                   ; <i32> [#uses=1]
  %tmp552 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx553 = getelementptr inbounds i16 addrspace(1)* %tmp552, i32 %add551 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv548, i16 addrspace(1)* %arrayidx553
  %tmp554 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp555 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call556 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp554, <16 x i16> %tmp555) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call556, <16 x i16>* %tmp557
  %tmp558 = load <16 x i16>* %tmp557              ; <<16 x i16>> [#uses=1]
  %tmp559 = extractelement <16 x i16> %tmp558, i32 14 ; <i16> [#uses=1]
  %conv560 = zext i16 %tmp559 to i32              ; <i32> [#uses=1]
  %add561 = add nsw i32 %conv560, 0               ; <i32> [#uses=1]
  %conv562 = trunc i32 %add561 to i16             ; <i16> [#uses=1]
  %tmp563 = load i32* %i                          ; <i32> [#uses=1]
  %mul564 = mul i32 8, %tmp563                    ; <i32> [#uses=1]
  %add565 = add i32 %mul564, 14                   ; <i32> [#uses=1]
  %tmp566 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx567 = getelementptr inbounds i16 addrspace(1)* %tmp566, i32 %add565 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv562, i16 addrspace(1)* %arrayidx567
  %tmp568 = load <16 x i16>* %histVals            ; <<16 x i16>> [#uses=1]
  %tmp569 = load <16 x i16>* %maxBinVector        ; <<16 x i16>> [#uses=1]
  %call570 = call <16 x i16> @_Z3minDv16_tS_(<16 x i16> %tmp568, <16 x i16> %tmp569) ; <<16 x i16>> [#uses=1]
  store <16 x i16> %call570, <16 x i16>* %tmp571
  %tmp572 = load <16 x i16>* %tmp571              ; <<16 x i16>> [#uses=1]
  %tmp573 = extractelement <16 x i16> %tmp572, i32 15 ; <i16> [#uses=1]
  %conv574 = zext i16 %tmp573 to i32              ; <i32> [#uses=1]
  %add575 = add nsw i32 %conv574, 0               ; <i32> [#uses=1]
  %conv576 = trunc i32 %add575 to i16             ; <i16> [#uses=1]
  %tmp577 = load i32* %i                          ; <i32> [#uses=1]
  %mul578 = mul i32 8, %tmp577                    ; <i32> [#uses=1]
  %add579 = add i32 %mul578, 15                   ; <i32> [#uses=1]
  %tmp580 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx581 = getelementptr inbounds i16 addrspace(1)* %tmp580, i32 %add579 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv576, i16 addrspace(1)* %arrayidx581
  br label %for.inc582

for.inc582:                                       ; preds = %for.end
  %tmp583 = load i32* %i                          ; <i32> [#uses=1]
  %inc584 = add i32 %tmp583, 1                    ; <i32> [#uses=1]
  store i32 %inc584, i32* %i
  br label %for.cond

for.end585:                                       ; preds = %for.cond
  store i32 8, i32* %i
  br label %for.cond586

for.cond586:                                      ; preds = %for.inc662, %for.end585
  %tmp587 = load i32* %i                          ; <i32> [#uses=1]
  %cmp588 = icmp ule i32 %tmp587, 8               ; <i1> [#uses=1]
  br i1 %cmp588, label %for.body590, label %for.end665

for.body590:                                      ; preds = %for.cond586
  %tmp592 = load i32* %i                          ; <i32> [#uses=1]
  %add593 = add i32 %tmp592, 1                    ; <i32> [#uses=1]
  %tmp594 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx595 = getelementptr inbounds i16 addrspace(1)* %tmp594, i32 %add593 ; <i16 addrspace(1)*> [#uses=1]
  %tmp596 = load i16 addrspace(1)* %arrayidx595   ; <i16> [#uses=1]
  %conv597 = zext i16 %tmp596 to i32              ; <i32> [#uses=1]
  %tmp598 = load i32* %i                          ; <i32> [#uses=1]
  %sub = sub i32 %tmp598, 1                       ; <i32> [#uses=1]
  %tmp599 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx600 = getelementptr inbounds i16 addrspace(1)* %tmp599, i32 %sub ; <i16 addrspace(1)*> [#uses=1]
  %tmp601 = load i16 addrspace(1)* %arrayidx600   ; <i16> [#uses=1]
  %conv602 = zext i16 %tmp601 to i32              ; <i32> [#uses=1]
  %cmp603 = icmp sgt i32 %conv597, %conv602       ; <i1> [#uses=1]
  br i1 %cmp603, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.body590
  %tmp605 = load i32* %i                          ; <i32> [#uses=1]
  %add606 = add i32 %tmp605, 1                    ; <i32> [#uses=1]
  %tmp607 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx608 = getelementptr inbounds i16 addrspace(1)* %tmp607, i32 %add606 ; <i16 addrspace(1)*> [#uses=1]
  %tmp609 = load i16 addrspace(1)* %arrayidx608   ; <i16> [#uses=1]
  %conv610 = zext i16 %tmp609 to i32              ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %for.body590
  %tmp611 = load i32* %i                          ; <i32> [#uses=1]
  %sub612 = sub i32 %tmp611, 1                    ; <i32> [#uses=1]
  %tmp613 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx614 = getelementptr inbounds i16 addrspace(1)* %tmp613, i32 %sub612 ; <i16 addrspace(1)*> [#uses=1]
  %tmp615 = load i16 addrspace(1)* %arrayidx614   ; <i16> [#uses=1]
  %conv616 = zext i16 %tmp615 to i32              ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv610, %cond.true ], [ %conv616, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond, i32* %iMax
  %tmp617 = load i32* %i                          ; <i32> [#uses=1]
  %tmp618 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx619 = getelementptr inbounds i16 addrspace(1)* %tmp618, i32 %tmp617 ; <i16 addrspace(1)*> [#uses=1]
  %tmp620 = load i16 addrspace(1)* %arrayidx619   ; <i16> [#uses=1]
  %conv621 = zext i16 %tmp620 to i32              ; <i32> [#uses=1]
  %cmp622 = icmp sgt i32 %conv621, 1000           ; <i1> [#uses=1]
  br i1 %cmp622, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %cond.end
  %tmp624 = load i32* %i                          ; <i32> [#uses=1]
  %tmp625 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx626 = getelementptr inbounds i16 addrspace(1)* %tmp625, i32 %tmp624 ; <i16 addrspace(1)*> [#uses=1]
  %tmp627 = load i16 addrspace(1)* %arrayidx626   ; <i16> [#uses=1]
  %conv628 = zext i16 %tmp627 to i32              ; <i32> [#uses=1]
  %tmp629 = load i32* %iMax                       ; <i32> [#uses=1]
  %sub630 = sub i32 %conv628, %tmp629             ; <i32> [#uses=1]
  %cmp631 = icmp sgt i32 %sub630, 1000            ; <i1> [#uses=1]
  br i1 %cmp631, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %tmp634 = load i32* %i                          ; <i32> [#uses=1]
  %tmp635 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx636 = getelementptr inbounds i16 addrspace(1)* %tmp635, i32 %tmp634 ; <i16 addrspace(1)*> [#uses=1]
  %tmp637 = load i16 addrspace(1)* %arrayidx636   ; <i16> [#uses=1]
  %conv638 = zext i16 %tmp637 to i32              ; <i32> [#uses=1]
  %tmp639 = load i32* %i                          ; <i32> [#uses=1]
  %sub640 = sub i32 %tmp639, 1                    ; <i32> [#uses=1]
  %tmp641 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx642 = getelementptr inbounds i16 addrspace(1)* %tmp641, i32 %sub640 ; <i16 addrspace(1)*> [#uses=1]
  %tmp643 = load i16 addrspace(1)* %arrayidx642   ; <i16> [#uses=1]
  %conv644 = zext i16 %tmp643 to i32              ; <i32> [#uses=1]
  %tmp645 = load i32* %i                          ; <i32> [#uses=1]
  %add646 = add i32 %tmp645, 1                    ; <i32> [#uses=1]
  %tmp647 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx648 = getelementptr inbounds i16 addrspace(1)* %tmp647, i32 %add646 ; <i16 addrspace(1)*> [#uses=1]
  %tmp649 = load i16 addrspace(1)* %arrayidx648   ; <i16> [#uses=1]
  %conv650 = zext i16 %tmp649 to i32              ; <i32> [#uses=1]
  %add651 = add nsw i32 %conv644, %conv650        ; <i32> [#uses=1]
  %add652 = add nsw i32 %add651, 1                ; <i32> [#uses=1]
  %div = sdiv i32 %add652, 2                      ; <i32> [#uses=1]
  %sub653 = sub i32 %conv638, %div                ; <i32> [#uses=1]
  store i32 %sub653, i32* %ui32Diff
  %tmp654 = load i32* %ui32Diff                   ; <i32> [#uses=1]
  %tmp655 = load i32* %i                          ; <i32> [#uses=1]
  %tmp656 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx657 = getelementptr inbounds i16 addrspace(1)* %tmp656, i32 %tmp655 ; <i16 addrspace(1)*> [#uses=2]
  %tmp658 = load i16 addrspace(1)* %arrayidx657   ; <i16> [#uses=1]
  %conv659 = zext i16 %tmp658 to i32              ; <i32> [#uses=1]
  %sub660 = sub i32 %conv659, %tmp654             ; <i32> [#uses=1]
  %conv661 = trunc i32 %sub660 to i16             ; <i16> [#uses=1]
  store i16 %conv661, i16 addrspace(1)* %arrayidx657
  br label %for.end665

if.end:                                           ; preds = %land.lhs.true, %cond.end
  br label %for.inc662

for.inc662:                                       ; preds = %if.end
  %tmp663 = load i32* %i                          ; <i32> [#uses=1]
  %inc664 = add i32 %tmp663, 1                    ; <i32> [#uses=1]
  store i32 %inc664, i32* %i
  br label %for.cond586

for.end665:                                       ; preds = %if.then, %for.cond586
  ret void
}

declare <16 x i16> @_Z3minDv16_tS_(<16 x i16>, <16 x i16>)

define i32 @AnalyzeHistogram(i16 addrspace(1)* %pHistogram, i16* %brightImageFactor, i32* %histNumPixels, i32* %histAverage) nounwind {
entry:
  %retval = alloca i32, align 4                   ; <i32*> [#uses=3]
  %pHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %brightImageFactor.addr = alloca i16*, align 4  ; <i16**> [#uses=2]
  %histNumPixels.addr = alloca i32*, align 4      ; <i32**> [#uses=2]
  %histAverage.addr = alloca i32*, align 4        ; <i32**> [#uses=4]
  %area = alloca i32, align 4                     ; <i32*> [#uses=5]
  %moment = alloca float, align 4                 ; <float*> [#uses=4]
  %L = alloca float, align 4                      ; <float*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %val = alloca i32, align 4                      ; <i32*> [#uses=4]
  %darknNondark = alloca i16, align 2             ; <i16*> [#uses=7]
  store i16 addrspace(1)* %pHistogram, i16 addrspace(1)** %pHistogram.addr
  store i16* %brightImageFactor, i16** %brightImageFactor.addr
  store i32* %histNumPixels, i32** %histNumPixels.addr
  store i32* %histAverage, i32** %histAverage.addr
  store i32 0, i32* %area
  store float 0.000000e+000, float* %moment
  store float 5.000000e-001, float* %L
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 128                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32* %i                            ; <i32> [#uses=1]
  %tmp3 = load i16 addrspace(1)** %pHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp3, i32 %tmp2 ; <i16 addrspace(1)*> [#uses=1]
  %tmp4 = load i16 addrspace(1)* %arrayidx        ; <i16> [#uses=1]
  %conv = zext i16 %tmp4 to i32                   ; <i32> [#uses=1]
  store i32 %conv, i32* %val
  %tmp5 = load i32* %val                          ; <i32> [#uses=1]
  %tmp6 = load i32* %area                         ; <i32> [#uses=1]
  %add = add nsw i32 %tmp6, %tmp5                 ; <i32> [#uses=1]
  store i32 %add, i32* %area
  %tmp7 = load float* %L                          ; <float> [#uses=1]
  %tmp8 = load i32* %val                          ; <i32> [#uses=1]
  %conv9 = sitofp i32 %tmp8 to float              ; <float> [#uses=1]
  %mul = fmul float %tmp7, %conv9                 ; <float> [#uses=1]
  %tmp10 = load float* %moment                    ; <float> [#uses=1]
  %add11 = fadd float %tmp10, %mul                ; <float> [#uses=1]
  store float %add11, float* %moment
  %tmp12 = load i32* %val                         ; <i32> [#uses=1]
  %tmp13 = load i32** %histNumPixels.addr         ; <i32*> [#uses=2]
  %tmp14 = load i32* %tmp13                       ; <i32> [#uses=1]
  %add15 = add i32 %tmp14, %tmp12                 ; <i32> [#uses=1]
  store i32 %add15, i32* %tmp13
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp16 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp16, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  %tmp17 = load float* %L                         ; <float> [#uses=1]
  %conv18 = fpext float %tmp17 to double          ; <double> [#uses=1]
  %add19 = fadd double %conv18, 2.000000e+000     ; <double> [#uses=1]
  %conv20 = fptrunc double %add19 to float        ; <float> [#uses=1]
  store float %conv20, float* %L
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp21 = load i32* %area                        ; <i32> [#uses=1]
  %cmp22 = icmp eq i32 %tmp21, 0                  ; <i1> [#uses=1]
  br i1 %cmp22, label %if.then, label %if.end

if.then:                                          ; preds = %for.end
  store i32 0, i32* %retval
  br label %return

if.end:                                           ; preds = %for.end
  store i16 0, i16* %darknNondark
  %tmp25 = load float* %moment                    ; <float> [#uses=1]
  %tmp26 = load i32* %area                        ; <i32> [#uses=1]
  %conv27 = sitofp i32 %tmp26 to float            ; <float> [#uses=3]
  %cmp28 = fcmp oeq float 0.000000e+000, %conv27  ; <i1> [#uses=1]
  %sel = select i1 %cmp28, float 1.000000e+000, float %conv27 ; <float> [#uses=0]
  %div = fdiv float %tmp25, %conv27               ; <float> [#uses=1]
  %conv29 = fpext float %div to double            ; <double> [#uses=1]
  %add30 = fadd double %conv29, 5.000000e-001     ; <double> [#uses=1]
  %conv31 = fptosi double %add30 to i32           ; <i32> [#uses=1]
  %tmp32 = load i32** %histAverage.addr           ; <i32*> [#uses=1]
  store i32 %conv31, i32* %tmp32
  %tmp33 = load i32** %histAverage.addr           ; <i32*> [#uses=1]
  %tmp34 = load i32* %tmp33                       ; <i32> [#uses=1]
  %cmp35 = icmp slt i32 %tmp34, 30                ; <i1> [#uses=1]
  br i1 %cmp35, label %if.then37, label %if.else

if.then37:                                        ; preds = %if.end
  store i16 0, i16* %darknNondark
  br label %if.end49

if.else:                                          ; preds = %if.end
  %tmp38 = load i32** %histAverage.addr           ; <i32*> [#uses=1]
  %tmp39 = load i32* %tmp38                       ; <i32> [#uses=1]
  %sub = sub i32 %tmp39, 30                       ; <i32> [#uses=1]
  %mul40 = mul i32 %sub, 255                      ; <i32> [#uses=1]
  %shr = ashr i32 %mul40, 6                       ; <i32> [#uses=1]
  %conv41 = trunc i32 %shr to i16                 ; <i16> [#uses=1]
  store i16 %conv41, i16* %darknNondark
  %tmp42 = load i16* %darknNondark                ; <i16> [#uses=1]
  %conv43 = zext i16 %tmp42 to i32                ; <i32> [#uses=1]
  %cmp44 = icmp slt i32 %conv43, 255              ; <i1> [#uses=1]
  br i1 %cmp44, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.else
  %tmp46 = load i16* %darknNondark                ; <i16> [#uses=1]
  %conv47 = zext i16 %tmp46 to i32                ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %if.else
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv47, %cond.true ], [ 255, %cond.false ] ; <i32> [#uses=1]
  %conv48 = trunc i32 %cond to i16                ; <i16> [#uses=1]
  store i16 %conv48, i16* %darknNondark
  br label %if.end49

if.end49:                                         ; preds = %cond.end, %if.then37
  %tmp50 = load i16* %darknNondark                ; <i16> [#uses=1]
  %tmp51 = load i16** %brightImageFactor.addr     ; <i16*> [#uses=1]
  store i16 %tmp50, i16* %tmp51
  store i32 1, i32* %retval
  br label %return

return:                                           ; preds = %if.end49, %if.then
  %0 = load i32* %retval                          ; <i32> [#uses=1]
  ret i32 %0
}

define void @CorrectSlopes(i16 zeroext %brightImageFactor, i16* %histFinalPoint, i16* %destFinalPoint, i16* %rangeMask) nounwind {
entry:
  %brightImageFactor.addr = alloca i16, align 2   ; <i16*> [#uses=3]
  %histFinalPoint.addr = alloca i16*, align 4     ; <i16**> [#uses=7]
  %destFinalPoint.addr = alloca i16*, align 4     ; <i16**> [#uses=4]
  %rangeMask.addr = alloca i16*, align 4          ; <i16**> [#uses=6]
  %negSlope = alloca i32, align 4                 ; <i32*> [#uses=6]
  %posSlope = alloca i32, align 4                 ; <i32*> [#uses=6]
  %i = alloca i32, align 4                        ; <i32*> [#uses=43]
  %change_factor = alloca i16, align 2            ; <i16*> [#uses=2]
  %roundValue = alloca i16, align 2               ; <i16*> [#uses=5]
  %dx = alloca [8 x i16], align 2                 ; <[8 x i16]*> [#uses=9]
  %dy = alloca [8 x i16], align 2                 ; <[8 x i16]*> [#uses=18]
  %sign = alloca [8 x i16], align 2               ; <[8 x i16]*> [#uses=8]
  %maxSlope = alloca i16, align 2                 ; <i16*> [#uses=3]
  %dyMul = alloca i32, align 4                    ; <i32*> [#uses=8]
  %dyMul237 = alloca i32, align 4                 ; <i32*> [#uses=4]
  store i16 %brightImageFactor, i16* %brightImageFactor.addr
  store i16* %histFinalPoint, i16** %histFinalPoint.addr
  store i16* %destFinalPoint, i16** %destFinalPoint.addr
  store i16* %rangeMask, i16** %rangeMask.addr
  store i32 0, i32* %negSlope
  store i32 0, i32* %posSlope
  store i16 128, i16* %roundValue
  %tmp = load i16* %brightImageFactor.addr        ; <i16> [#uses=1]
  %conv = zext i16 %tmp to i32                    ; <i32> [#uses=1]
  %add = add nsw i32 %conv, 1                     ; <i32> [#uses=1]
  %cmp = icmp sge i32 %add, 256                   ; <i1> [#uses=1]
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp2 = load i16* %brightImageFactor.addr       ; <i16> [#uses=1]
  %conv3 = zext i16 %tmp2 to i32                  ; <i32> [#uses=1]
  %mul = mul i32 %conv3, 30                       ; <i32> [#uses=1]
  %add4 = add nsw i32 %mul, 128                   ; <i32> [#uses=1]
  %shr = ashr i32 %add4, 8                        ; <i32> [#uses=1]
  %add5 = add nsw i32 130, %shr                   ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ 160, %cond.true ], [ %add5, %cond.false ] ; <i32> [#uses=1]
  %conv6 = trunc i32 %cond to i16                 ; <i16> [#uses=1]
  store i16 %conv6, i16* %maxSlope
  store i32 1, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %cond.end
  %tmp7 = load i32* %i                            ; <i32> [#uses=1]
  %cmp8 = icmp slt i32 %tmp7, 8                   ; <i1> [#uses=1]
  br i1 %cmp8, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %tmp11 = load i16** %histFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx = getelementptr inbounds i16* %tmp11, i32 %tmp10 ; <i16*> [#uses=1]
  %tmp12 = load i16* %arrayidx                    ; <i16> [#uses=1]
  %conv13 = zext i16 %tmp12 to i32                ; <i32> [#uses=1]
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %sub = sub i32 %tmp14, 1                        ; <i32> [#uses=1]
  %tmp15 = load i16** %histFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx16 = getelementptr inbounds i16* %tmp15, i32 %sub ; <i16*> [#uses=1]
  %tmp17 = load i16* %arrayidx16                  ; <i16> [#uses=1]
  %conv18 = zext i16 %tmp17 to i32                ; <i32> [#uses=1]
  %sub19 = sub i32 %conv13, %conv18               ; <i32> [#uses=1]
  %conv20 = trunc i32 %sub19 to i16               ; <i16> [#uses=1]
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx22 = getelementptr inbounds i16* %arraydecay, i32 %tmp21 ; <i16*> [#uses=1]
  store i16 %conv20, i16* %arrayidx22
  %tmp23 = load i32* %i                           ; <i32> [#uses=1]
  %tmp24 = load i16** %rangeMask.addr             ; <i16*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i16* %tmp24, i32 %tmp23 ; <i16*> [#uses=1]
  %tmp26 = load i16* %arrayidx25                  ; <i16> [#uses=1]
  %conv27 = zext i16 %tmp26 to i32                ; <i32> [#uses=1]
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay29 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx30 = getelementptr inbounds i16* %arraydecay29, i32 %tmp28 ; <i16*> [#uses=2]
  %tmp31 = load i16* %arrayidx30                  ; <i16> [#uses=1]
  %conv32 = sext i16 %tmp31 to i32                ; <i32> [#uses=1]
  %and = and i32 %conv32, %conv27                 ; <i32> [#uses=1]
  %conv33 = trunc i32 %and to i16                 ; <i16> [#uses=1]
  store i16 %conv33, i16* %arrayidx30
  %tmp34 = load i32* %i                           ; <i32> [#uses=1]
  %tmp35 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i16* %tmp35, i32 %tmp34 ; <i16*> [#uses=1]
  %tmp37 = load i16* %arrayidx36                  ; <i16> [#uses=1]
  %conv38 = zext i16 %tmp37 to i32                ; <i32> [#uses=1]
  %tmp39 = load i32* %i                           ; <i32> [#uses=1]
  %sub40 = sub i32 %tmp39, 1                      ; <i32> [#uses=1]
  %tmp41 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx42 = getelementptr inbounds i16* %tmp41, i32 %sub40 ; <i16*> [#uses=1]
  %tmp43 = load i16* %arrayidx42                  ; <i16> [#uses=1]
  %conv44 = zext i16 %tmp43 to i32                ; <i32> [#uses=1]
  %sub45 = sub i32 %conv38, %conv44               ; <i32> [#uses=1]
  %conv46 = trunc i32 %sub45 to i16               ; <i16> [#uses=1]
  %tmp47 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay48 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx49 = getelementptr inbounds i16* %arraydecay48, i32 %tmp47 ; <i16*> [#uses=1]
  store i16 %conv46, i16* %arrayidx49
  %tmp50 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay51 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx52 = getelementptr inbounds i16* %arraydecay51, i32 %tmp50 ; <i16*> [#uses=1]
  %tmp53 = load i16* %arrayidx52                  ; <i16> [#uses=1]
  %conv54 = sext i16 %tmp53 to i32                ; <i32> [#uses=1]
  %tmp55 = load i16* %maxSlope                    ; <i16> [#uses=1]
  %conv56 = sext i16 %tmp55 to i32                ; <i32> [#uses=1]
  %mul57 = mul i32 %conv54, %conv56               ; <i32> [#uses=1]
  store i32 %mul57, i32* %dyMul
  %tmp58 = load i16* %roundValue                  ; <i16> [#uses=1]
  %conv59 = zext i16 %tmp58 to i32                ; <i32> [#uses=1]
  %tmp60 = load i32* %dyMul                       ; <i32> [#uses=1]
  %add61 = add nsw i32 %tmp60, %conv59            ; <i32> [#uses=1]
  store i32 %add61, i32* %dyMul
  %tmp62 = load i32* %dyMul                       ; <i32> [#uses=1]
  %shr63 = ashr i32 %tmp62, 8                     ; <i32> [#uses=1]
  %conv64 = trunc i32 %shr63 to i16               ; <i16> [#uses=1]
  %tmp65 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay66 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx67 = getelementptr inbounds i16* %arraydecay66, i32 %tmp65 ; <i16*> [#uses=1]
  store i16 %conv64, i16* %arrayidx67
  %tmp68 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay69 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx70 = getelementptr inbounds i16* %arraydecay69, i32 %tmp68 ; <i16*> [#uses=1]
  %tmp71 = load i16* %arrayidx70                  ; <i16> [#uses=1]
  %conv72 = sext i16 %tmp71 to i32                ; <i32> [#uses=1]
  %tmp73 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay74 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx75 = getelementptr inbounds i16* %arraydecay74, i32 %tmp73 ; <i16*> [#uses=2]
  %tmp76 = load i16* %arrayidx75                  ; <i16> [#uses=1]
  %conv77 = sext i16 %tmp76 to i32                ; <i32> [#uses=1]
  %sub78 = sub i32 %conv77, %conv72               ; <i32> [#uses=1]
  %conv79 = trunc i32 %sub78 to i16               ; <i16> [#uses=1]
  store i16 %conv79, i16* %arrayidx75
  %tmp80 = load i32* %i                           ; <i32> [#uses=1]
  %tmp81 = load i16** %rangeMask.addr             ; <i16*> [#uses=1]
  %arrayidx82 = getelementptr inbounds i16* %tmp81, i32 %tmp80 ; <i16*> [#uses=1]
  %tmp83 = load i16* %arrayidx82                  ; <i16> [#uses=1]
  %conv84 = zext i16 %tmp83 to i32                ; <i32> [#uses=1]
  %tmp85 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay86 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx87 = getelementptr inbounds i16* %arraydecay86, i32 %tmp85 ; <i16*> [#uses=2]
  %tmp88 = load i16* %arrayidx87                  ; <i16> [#uses=1]
  %conv89 = sext i16 %tmp88 to i32                ; <i32> [#uses=1]
  %and90 = and i32 %conv89, %conv84               ; <i32> [#uses=1]
  %conv91 = trunc i32 %and90 to i16               ; <i16> [#uses=1]
  store i16 %conv91, i16* %arrayidx87
  %tmp92 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay93 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx94 = getelementptr inbounds i16* %arraydecay93, i32 %tmp92 ; <i16*> [#uses=1]
  %tmp95 = load i16* %arrayidx94                  ; <i16> [#uses=1]
  %conv96 = sext i16 %tmp95 to i32                ; <i32> [#uses=1]
  %and97 = and i32 %conv96, 32768                 ; <i32> [#uses=1]
  %shr98 = ashr i32 %and97, 15                    ; <i32> [#uses=1]
  %sub99 = sub i32 0, %shr98                      ; <i32> [#uses=1]
  %conv100 = trunc i32 %sub99 to i16              ; <i16> [#uses=1]
  %tmp101 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay102 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx103 = getelementptr inbounds i16* %arraydecay102, i32 %tmp101 ; <i16*> [#uses=1]
  store i16 %conv100, i16* %arrayidx103
  %tmp104 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay105 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx106 = getelementptr inbounds i16* %arraydecay105, i32 %tmp104 ; <i16*> [#uses=1]
  %tmp107 = load i16* %arrayidx106                ; <i16> [#uses=1]
  %conv108 = sext i16 %tmp107 to i32              ; <i32> [#uses=1]
  %tmp109 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay110 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx111 = getelementptr inbounds i16* %arraydecay110, i32 %tmp109 ; <i16*> [#uses=1]
  %tmp112 = load i16* %arrayidx111                ; <i16> [#uses=1]
  %conv113 = sext i16 %tmp112 to i32              ; <i32> [#uses=1]
  %mul114 = mul i32 %conv108, %conv113            ; <i32> [#uses=1]
  %tmp115 = load i32* %negSlope                   ; <i32> [#uses=1]
  %add116 = add i32 %tmp115, %mul114              ; <i32> [#uses=1]
  store i32 %add116, i32* %negSlope
  %tmp117 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay118 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx119 = getelementptr inbounds i16* %arraydecay118, i32 %tmp117 ; <i16*> [#uses=1]
  %tmp120 = load i16* %arrayidx119                ; <i16> [#uses=1]
  %conv121 = sext i16 %tmp120 to i32              ; <i32> [#uses=1]
  %add122 = add nsw i32 %conv121, 1               ; <i32> [#uses=1]
  %tmp123 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay124 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx125 = getelementptr inbounds i16* %arraydecay124, i32 %tmp123 ; <i16*> [#uses=1]
  %tmp126 = load i16* %arrayidx125                ; <i16> [#uses=1]
  %conv127 = sext i16 %tmp126 to i32              ; <i32> [#uses=1]
  %mul128 = mul i32 %add122, %conv127             ; <i32> [#uses=1]
  %tmp129 = load i32* %posSlope                   ; <i32> [#uses=1]
  %add130 = add i32 %tmp129, %mul128              ; <i32> [#uses=1]
  store i32 %add130, i32* %posSlope
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp131 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp131, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp132 = load i16** %histFinalPoint.addr       ; <i16*> [#uses=1]
  %arrayidx133 = getelementptr inbounds i16* %tmp132, i32 0 ; <i16*> [#uses=1]
  %tmp134 = load i16* %arrayidx133                ; <i16> [#uses=1]
  %conv135 = zext i16 %tmp134 to i32              ; <i32> [#uses=1]
  %tmp136 = load i16** %rangeMask.addr            ; <i16*> [#uses=1]
  %arrayidx137 = getelementptr inbounds i16* %tmp136, i32 0 ; <i16*> [#uses=1]
  %tmp138 = load i16* %arrayidx137                ; <i16> [#uses=1]
  %conv139 = zext i16 %tmp138 to i32              ; <i32> [#uses=1]
  %and140 = and i32 %conv135, %conv139            ; <i32> [#uses=1]
  %conv141 = trunc i32 %and140 to i16             ; <i16> [#uses=1]
  %arraydecay142 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx143 = getelementptr inbounds i16* %arraydecay142, i32 0 ; <i16*> [#uses=1]
  store i16 %conv141, i16* %arrayidx143
  %tmp144 = load i16** %destFinalPoint.addr       ; <i16*> [#uses=1]
  %arrayidx145 = getelementptr inbounds i16* %tmp144, i32 0 ; <i16*> [#uses=1]
  %tmp146 = load i16* %arrayidx145                ; <i16> [#uses=1]
  %conv147 = zext i16 %tmp146 to i32              ; <i32> [#uses=1]
  %tmp148 = load i16** %rangeMask.addr            ; <i16*> [#uses=1]
  %arrayidx149 = getelementptr inbounds i16* %tmp148, i32 0 ; <i16*> [#uses=1]
  %tmp150 = load i16* %arrayidx149                ; <i16> [#uses=1]
  %conv151 = zext i16 %tmp150 to i32              ; <i32> [#uses=1]
  %and152 = and i32 %conv147, %conv151            ; <i32> [#uses=1]
  %conv153 = trunc i32 %and152 to i16             ; <i16> [#uses=1]
  %arraydecay154 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx155 = getelementptr inbounds i16* %arraydecay154, i32 0 ; <i16*> [#uses=1]
  store i16 %conv153, i16* %arrayidx155
  %arraydecay156 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx157 = getelementptr inbounds i16* %arraydecay156, i32 0 ; <i16*> [#uses=1]
  %tmp158 = load i16* %arrayidx157                ; <i16> [#uses=1]
  %conv159 = sext i16 %tmp158 to i32              ; <i32> [#uses=1]
  %tmp160 = load i16* %maxSlope                   ; <i16> [#uses=1]
  %conv161 = sext i16 %tmp160 to i32              ; <i32> [#uses=1]
  %mul162 = mul i32 %conv159, %conv161            ; <i32> [#uses=1]
  store i32 %mul162, i32* %dyMul
  %tmp163 = load i16* %roundValue                 ; <i16> [#uses=1]
  %conv164 = zext i16 %tmp163 to i32              ; <i32> [#uses=1]
  %tmp165 = load i32* %dyMul                      ; <i32> [#uses=1]
  %add166 = add nsw i32 %tmp165, %conv164         ; <i32> [#uses=1]
  store i32 %add166, i32* %dyMul
  %tmp167 = load i32* %dyMul                      ; <i32> [#uses=1]
  %shr168 = ashr i32 %tmp167, 8                   ; <i32> [#uses=1]
  %conv169 = trunc i32 %shr168 to i16             ; <i16> [#uses=1]
  %arraydecay170 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx171 = getelementptr inbounds i16* %arraydecay170, i32 0 ; <i16*> [#uses=1]
  store i16 %conv169, i16* %arrayidx171
  %arraydecay172 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx173 = getelementptr inbounds i16* %arraydecay172, i32 0 ; <i16*> [#uses=1]
  %tmp174 = load i16* %arrayidx173                ; <i16> [#uses=1]
  %conv175 = sext i16 %tmp174 to i32              ; <i32> [#uses=1]
  %arraydecay176 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx177 = getelementptr inbounds i16* %arraydecay176, i32 0 ; <i16*> [#uses=2]
  %tmp178 = load i16* %arrayidx177                ; <i16> [#uses=1]
  %conv179 = sext i16 %tmp178 to i32              ; <i32> [#uses=1]
  %sub180 = sub i32 %conv179, %conv175            ; <i32> [#uses=1]
  %conv181 = trunc i32 %sub180 to i16             ; <i16> [#uses=1]
  store i16 %conv181, i16* %arrayidx177
  %tmp182 = load i16** %rangeMask.addr            ; <i16*> [#uses=1]
  %arrayidx183 = getelementptr inbounds i16* %tmp182, i32 0 ; <i16*> [#uses=1]
  %tmp184 = load i16* %arrayidx183                ; <i16> [#uses=1]
  %conv185 = zext i16 %tmp184 to i32              ; <i32> [#uses=1]
  %arraydecay186 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx187 = getelementptr inbounds i16* %arraydecay186, i32 0 ; <i16*> [#uses=2]
  %tmp188 = load i16* %arrayidx187                ; <i16> [#uses=1]
  %conv189 = sext i16 %tmp188 to i32              ; <i32> [#uses=1]
  %and190 = and i32 %conv189, %conv185            ; <i32> [#uses=1]
  %conv191 = trunc i32 %and190 to i16             ; <i16> [#uses=1]
  store i16 %conv191, i16* %arrayidx187
  %arraydecay192 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx193 = getelementptr inbounds i16* %arraydecay192, i32 0 ; <i16*> [#uses=1]
  %tmp194 = load i16* %arrayidx193                ; <i16> [#uses=1]
  %conv195 = sext i16 %tmp194 to i32              ; <i32> [#uses=1]
  %shr196 = ashr i32 %conv195, 15                 ; <i32> [#uses=1]
  %neg = sub i32 0, %shr196                       ; <i32> [#uses=1]
  %conv197 = trunc i32 %neg to i16                ; <i16> [#uses=1]
  %arraydecay198 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx199 = getelementptr inbounds i16* %arraydecay198, i32 0 ; <i16*> [#uses=1]
  store i16 %conv197, i16* %arrayidx199
  %arraydecay200 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx201 = getelementptr inbounds i16* %arraydecay200, i32 0 ; <i16*> [#uses=1]
  %tmp202 = load i16* %arrayidx201                ; <i16> [#uses=1]
  %conv203 = sext i16 %tmp202 to i32              ; <i32> [#uses=1]
  %arraydecay204 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx205 = getelementptr inbounds i16* %arraydecay204, i32 0 ; <i16*> [#uses=1]
  %tmp206 = load i16* %arrayidx205                ; <i16> [#uses=1]
  %conv207 = sext i16 %tmp206 to i32              ; <i32> [#uses=1]
  %mul208 = mul i32 %conv203, %conv207            ; <i32> [#uses=1]
  %tmp209 = load i32* %negSlope                   ; <i32> [#uses=1]
  %add210 = add i32 %tmp209, %mul208              ; <i32> [#uses=1]
  store i32 %add210, i32* %negSlope
  %arraydecay211 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx212 = getelementptr inbounds i16* %arraydecay211, i32 0 ; <i16*> [#uses=1]
  %tmp213 = load i16* %arrayidx212                ; <i16> [#uses=1]
  %conv214 = sext i16 %tmp213 to i32              ; <i32> [#uses=1]
  %add215 = add nsw i32 %conv214, 1               ; <i32> [#uses=1]
  %arraydecay216 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx217 = getelementptr inbounds i16* %arraydecay216, i32 0 ; <i16*> [#uses=1]
  %tmp218 = load i16* %arrayidx217                ; <i16> [#uses=1]
  %conv219 = sext i16 %tmp218 to i32              ; <i32> [#uses=1]
  %mul220 = mul i32 %add215, %conv219             ; <i32> [#uses=1]
  %tmp221 = load i32* %posSlope                   ; <i32> [#uses=1]
  %add222 = add i32 %tmp221, %mul220              ; <i32> [#uses=1]
  store i32 %add222, i32* %posSlope
  store i16 512, i16* %roundValue
  %tmp223 = load i32* %posSlope                   ; <i32> [#uses=1]
  %conv224 = zext i32 %tmp223 to i64              ; <i64> [#uses=1]
  %shl = shl i64 %conv224, 20                     ; <i64> [#uses=1]
  %tmp225 = load i32* %negSlope                   ; <i32> [#uses=1]
  %conv226 = zext i32 %tmp225 to i64              ; <i64> [#uses=2]
  %cmp227 = icmp eq i64 0, %conv226               ; <i1> [#uses=1]
  %sel = select i1 %cmp227, i64 1, i64 %conv226   ; <i64> [#uses=1]
  %div = sdiv i64 %shl, %sel                      ; <i64> [#uses=1]
  %add228 = add nsw i64 %div, 512                 ; <i64> [#uses=1]
  %shr229 = ashr i64 %add228, 10                  ; <i64> [#uses=1]
  %conv230 = trunc i64 %shr229 to i16             ; <i16> [#uses=1]
  store i16 %conv230, i16* %change_factor
  store i32 0, i32* %i
  br label %for.cond231

for.cond231:                                      ; preds = %for.inc284, %for.end
  %tmp232 = load i32* %i                          ; <i32> [#uses=1]
  %cmp233 = icmp slt i32 %tmp232, 8               ; <i1> [#uses=1]
  br i1 %cmp233, label %for.body235, label %for.end287

for.body235:                                      ; preds = %for.cond231
  %tmp238 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay239 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx240 = getelementptr inbounds i16* %arraydecay239, i32 %tmp238 ; <i16*> [#uses=1]
  %tmp241 = load i16* %arrayidx240                ; <i16> [#uses=1]
  %conv242 = sext i16 %tmp241 to i32              ; <i32> [#uses=1]
  %tmp243 = load i16* %change_factor              ; <i16> [#uses=1]
  %conv244 = zext i16 %tmp243 to i32              ; <i32> [#uses=1]
  %mul245 = mul i32 %conv242, %conv244            ; <i32> [#uses=1]
  store i32 %mul245, i32* %dyMul237
  %tmp246 = load i16* %roundValue                 ; <i16> [#uses=1]
  %conv247 = zext i16 %tmp246 to i32              ; <i32> [#uses=1]
  %tmp248 = load i32* %dyMul237                   ; <i32> [#uses=1]
  %add249 = add nsw i32 %tmp248, %conv247         ; <i32> [#uses=1]
  store i32 %add249, i32* %dyMul237
  %tmp250 = load i32* %dyMul237                   ; <i32> [#uses=1]
  %shr251 = ashr i32 %tmp250, 10                  ; <i32> [#uses=1]
  %tmp252 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay253 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx254 = getelementptr inbounds i16* %arraydecay253, i32 %tmp252 ; <i16*> [#uses=1]
  %tmp255 = load i16* %arrayidx254                ; <i16> [#uses=1]
  %conv256 = sext i16 %tmp255 to i32              ; <i32> [#uses=1]
  %and257 = and i32 %shr251, %conv256             ; <i32> [#uses=1]
  %tmp258 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay259 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx260 = getelementptr inbounds i16* %arraydecay259, i32 %tmp258 ; <i16*> [#uses=2]
  %tmp261 = load i16* %arrayidx260                ; <i16> [#uses=1]
  %conv262 = sext i16 %tmp261 to i32              ; <i32> [#uses=1]
  %add263 = add nsw i32 %conv262, %and257         ; <i32> [#uses=1]
  %conv264 = trunc i32 %add263 to i16             ; <i16> [#uses=1]
  store i16 %conv264, i16* %arrayidx260
  %tmp265 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay266 = getelementptr inbounds [8 x i16]* %dy, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx267 = getelementptr inbounds i16* %arraydecay266, i32 %tmp265 ; <i16*> [#uses=1]
  %tmp268 = load i16* %arrayidx267                ; <i16> [#uses=1]
  %conv269 = sext i16 %tmp268 to i32              ; <i32> [#uses=1]
  %tmp270 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay271 = getelementptr inbounds [8 x i16]* %sign, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx272 = getelementptr inbounds i16* %arraydecay271, i32 %tmp270 ; <i16*> [#uses=1]
  %tmp273 = load i16* %arrayidx272                ; <i16> [#uses=1]
  %conv274 = sext i16 %tmp273 to i32              ; <i32> [#uses=1]
  %neg275 = xor i32 %conv274, -1                  ; <i32> [#uses=1]
  %and276 = and i32 %conv269, %neg275             ; <i32> [#uses=1]
  %tmp277 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay278 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx279 = getelementptr inbounds i16* %arraydecay278, i32 %tmp277 ; <i16*> [#uses=2]
  %tmp280 = load i16* %arrayidx279                ; <i16> [#uses=1]
  %conv281 = sext i16 %tmp280 to i32              ; <i32> [#uses=1]
  %add282 = add nsw i32 %conv281, %and276         ; <i32> [#uses=1]
  %conv283 = trunc i32 %add282 to i16             ; <i16> [#uses=1]
  store i16 %conv283, i16* %arrayidx279
  br label %for.inc284

for.inc284:                                       ; preds = %for.body235
  %tmp285 = load i32* %i                          ; <i32> [#uses=1]
  %inc286 = add nsw i32 %tmp285, 1                ; <i32> [#uses=1]
  store i32 %inc286, i32* %i
  br label %for.cond231

for.end287:                                       ; preds = %for.cond231
  store i32 1, i32* %i
  br label %for.cond288

for.cond288:                                      ; preds = %for.inc327, %for.end287
  %tmp289 = load i32* %i                          ; <i32> [#uses=1]
  %cmp290 = icmp slt i32 %tmp289, 4               ; <i1> [#uses=1]
  br i1 %cmp290, label %for.body292, label %for.end330

for.body292:                                      ; preds = %for.cond288
  %tmp293 = load i32* %i                          ; <i32> [#uses=1]
  %sub294 = sub i32 %tmp293, 1                    ; <i32> [#uses=1]
  %tmp295 = load i16** %histFinalPoint.addr       ; <i16*> [#uses=1]
  %arrayidx296 = getelementptr inbounds i16* %tmp295, i32 %sub294 ; <i16*> [#uses=1]
  %tmp297 = load i16* %arrayidx296                ; <i16> [#uses=1]
  %conv298 = zext i16 %tmp297 to i32              ; <i32> [#uses=1]
  %tmp299 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay300 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx301 = getelementptr inbounds i16* %arraydecay300, i32 %tmp299 ; <i16*> [#uses=1]
  %tmp302 = load i16* %arrayidx301                ; <i16> [#uses=1]
  %conv303 = sext i16 %tmp302 to i32              ; <i32> [#uses=1]
  %add304 = add nsw i32 %conv298, %conv303        ; <i32> [#uses=1]
  %cmp305 = icmp slt i32 %add304, 940             ; <i1> [#uses=1]
  br i1 %cmp305, label %cond.true307, label %cond.false320

cond.true307:                                     ; preds = %for.body292
  %tmp308 = load i32* %i                          ; <i32> [#uses=1]
  %sub309 = sub i32 %tmp308, 1                    ; <i32> [#uses=1]
  %tmp310 = load i16** %histFinalPoint.addr       ; <i16*> [#uses=1]
  %arrayidx311 = getelementptr inbounds i16* %tmp310, i32 %sub309 ; <i16*> [#uses=1]
  %tmp312 = load i16* %arrayidx311                ; <i16> [#uses=1]
  %conv313 = zext i16 %tmp312 to i32              ; <i32> [#uses=1]
  %tmp314 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay315 = getelementptr inbounds [8 x i16]* %dx, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx316 = getelementptr inbounds i16* %arraydecay315, i32 %tmp314 ; <i16*> [#uses=1]
  %tmp317 = load i16* %arrayidx316                ; <i16> [#uses=1]
  %conv318 = sext i16 %tmp317 to i32              ; <i32> [#uses=1]
  %add319 = add nsw i32 %conv313, %conv318        ; <i32> [#uses=1]
  br label %cond.end321

cond.false320:                                    ; preds = %for.body292
  br label %cond.end321

cond.end321:                                      ; preds = %cond.false320, %cond.true307
  %cond322 = phi i32 [ %add319, %cond.true307 ], [ 940, %cond.false320 ] ; <i32> [#uses=1]
  %conv323 = trunc i32 %cond322 to i16            ; <i16> [#uses=1]
  %tmp324 = load i32* %i                          ; <i32> [#uses=1]
  %tmp325 = load i16** %histFinalPoint.addr       ; <i16*> [#uses=1]
  %arrayidx326 = getelementptr inbounds i16* %tmp325, i32 %tmp324 ; <i16*> [#uses=1]
  store i16 %conv323, i16* %arrayidx326
  br label %for.inc327

for.inc327:                                       ; preds = %cond.end321
  %tmp328 = load i32* %i                          ; <i32> [#uses=1]
  %inc329 = add nsw i32 %tmp328, 1                ; <i32> [#uses=1]
  store i32 %inc329, i32* %i
  br label %for.cond288

for.end330:                                       ; preds = %for.cond288
  ret void
}

define i32 @CreateLinCurve(i16 zeroext %brightImageFactor, i16* %histFinalPoint, i16* %destFinalPoint, %struct.anon addrspace(1)* %prevLut, %struct.anon addrspace(1)* %currentLut) nounwind {
entry:
  %retval = alloca i32, align 4                   ; <i32*> [#uses=4]
  %brightImageFactor.addr = alloca i16, align 2   ; <i16*> [#uses=3]
  %histFinalPoint.addr = alloca i16*, align 4     ; <i16**> [#uses=4]
  %destFinalPoint.addr = alloca i16*, align 4     ; <i16**> [#uses=5]
  %prevLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=8]
  %currentLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=12]
  %ui8Alpha = alloca i8, align 1                  ; <i8*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=32]
  store i16 %brightImageFactor, i16* %brightImageFactor.addr
  store i16* %histFinalPoint, i16** %histFinalPoint.addr
  store i16* %destFinalPoint, i16** %destFinalPoint.addr
  store %struct.anon addrspace(1)* %prevLut, %struct.anon addrspace(1)** %prevLut.addr
  store %struct.anon addrspace(1)* %currentLut, %struct.anon addrspace(1)** %currentLut.addr
  %tmp = load i16* %brightImageFactor.addr        ; <i16> [#uses=1]
  %conv = zext i16 %tmp to i32                    ; <i32> [#uses=1]
  %add = add nsw i32 %conv, 1                     ; <i32> [#uses=1]
  %cmp = icmp sge i32 %add, 256                   ; <i1> [#uses=1]
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp2 = load i16* %brightImageFactor.addr       ; <i16> [#uses=1]
  %conv3 = zext i16 %tmp2 to i32                  ; <i32> [#uses=1]
  %mul = mul i32 %conv3, -60                      ; <i32> [#uses=1]
  %add4 = add nsw i32 %mul, 128                   ; <i32> [#uses=1]
  %shr = ashr i32 %add4, 8                        ; <i32> [#uses=1]
  %add5 = add nsw i32 140, %shr                   ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ 80, %cond.true ], [ %add5, %cond.false ] ; <i32> [#uses=1]
  %conv6 = trunc i32 %cond to i8                  ; <i8> [#uses=1]
  store i8 %conv6, i8* %ui8Alpha
  %tmp7 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp8 = getelementptr inbounds %struct.anon addrspace(1)* %tmp7, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp9 = load i32 addrspace(1)* %tmp8            ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp9, 0                  ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else102

if.then:                                          ; preds = %cond.end
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %cmp11 = icmp slt i32 %tmp10, 8                 ; <i1> [#uses=1]
  br i1 %cmp11, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp13 = load i32* %i                           ; <i32> [#uses=1]
  %tmp14 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx = getelementptr inbounds i16* %tmp14, i32 %tmp13 ; <i16*> [#uses=1]
  %tmp15 = load i16* %arrayidx                    ; <i16> [#uses=1]
  %tmp16 = load i32* %i                           ; <i32> [#uses=1]
  %tmp17 = load i16** %histFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx18 = getelementptr inbounds i16* %tmp17, i32 %tmp16 ; <i16*> [#uses=1]
  %tmp19 = load i16* %arrayidx18                  ; <i16> [#uses=1]
  %tmp20 = load i8* %ui8Alpha                     ; <i8> [#uses=1]
  %conv21 = zext i8 %tmp20 to i16                 ; <i16> [#uses=1]
  %call = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp15, i16 zeroext %tmp19, i16 zeroext %conv21) ; <i16> [#uses=1]
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %tmp23 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx24 = getelementptr inbounds i16* %tmp23, i32 %tmp22 ; <i16*> [#uses=1]
  store i16 %call, i16* %arrayidx24
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp25 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp25, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp26 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp27 = getelementptr inbounds %struct.anon addrspace(1)* %tmp26, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp28 = load i32 addrspace(1)* %tmp27          ; <i32> [#uses=1]
  %tobool29 = icmp ne i32 %tmp28, 0               ; <i1> [#uses=1]
  br i1 %tobool29, label %if.then30, label %if.else

if.then30:                                        ; preds = %for.end
  store i32 0, i32* %i
  br label %for.cond31

for.cond31:                                       ; preds = %for.inc67, %if.then30
  %tmp32 = load i32* %i                           ; <i32> [#uses=1]
  %cmp33 = icmp slt i32 %tmp32, 8                 ; <i1> [#uses=1]
  br i1 %cmp33, label %for.body35, label %for.end70

for.body35:                                       ; preds = %for.cond31
  %tmp36 = load i32* %i                           ; <i32> [#uses=1]
  %tmp37 = load i16** %histFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx38 = getelementptr inbounds i16* %tmp37, i32 %tmp36 ; <i16*> [#uses=1]
  %tmp39 = load i16* %arrayidx38                  ; <i16> [#uses=1]
  %tmp40 = load i32* %i                           ; <i32> [#uses=1]
  %tmp41 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp42 = getelementptr inbounds %struct.anon addrspace(1)* %tmp41, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x i16] addrspace(1)* %tmp42, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx43 = getelementptr inbounds i16 addrspace(1)* %arraydecay, i32 %tmp40 ; <i16 addrspace(1)*> [#uses=1]
  %tmp44 = load i16 addrspace(1)* %arrayidx43     ; <i16> [#uses=1]
  %call45 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp39, i16 zeroext %tmp44, i16 zeroext 64) ; <i16> [#uses=1]
  %tmp46 = load i32* %i                           ; <i32> [#uses=1]
  %tmp47 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp48 = getelementptr inbounds %struct.anon addrspace(1)* %tmp47, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay49 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp48, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx50 = getelementptr inbounds i16 addrspace(1)* %arraydecay49, i32 %tmp46 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %call45, i16 addrspace(1)* %arrayidx50
  %tmp51 = load i32* %i                           ; <i32> [#uses=1]
  %tmp52 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx53 = getelementptr inbounds i16* %tmp52, i32 %tmp51 ; <i16*> [#uses=1]
  %tmp54 = load i16* %arrayidx53                  ; <i16> [#uses=1]
  %tmp55 = load i32* %i                           ; <i32> [#uses=1]
  %tmp56 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp57 = getelementptr inbounds %struct.anon addrspace(1)* %tmp56, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay58 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp57, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx59 = getelementptr inbounds i16 addrspace(1)* %arraydecay58, i32 %tmp55 ; <i16 addrspace(1)*> [#uses=1]
  %tmp60 = load i16 addrspace(1)* %arrayidx59     ; <i16> [#uses=1]
  %call61 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp54, i16 zeroext %tmp60, i16 zeroext 64) ; <i16> [#uses=1]
  %tmp62 = load i32* %i                           ; <i32> [#uses=1]
  %tmp63 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp64 = getelementptr inbounds %struct.anon addrspace(1)* %tmp63, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay65 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp64, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds i16 addrspace(1)* %arraydecay65, i32 %tmp62 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %call61, i16 addrspace(1)* %arrayidx66
  br label %for.inc67

for.inc67:                                        ; preds = %for.body35
  %tmp68 = load i32* %i                           ; <i32> [#uses=1]
  %inc69 = add nsw i32 %tmp68, 1                  ; <i32> [#uses=1]
  store i32 %inc69, i32* %i
  br label %for.cond31

for.end70:                                        ; preds = %for.cond31
  br label %if.end

if.else:                                          ; preds = %for.end
  store i32 0, i32* %i
  br label %for.cond71

for.cond71:                                       ; preds = %for.inc94, %if.else
  %tmp72 = load i32* %i                           ; <i32> [#uses=1]
  %cmp73 = icmp slt i32 %tmp72, 8                 ; <i1> [#uses=1]
  br i1 %cmp73, label %for.body75, label %for.end97

for.body75:                                       ; preds = %for.cond71
  %tmp76 = load i32* %i                           ; <i32> [#uses=1]
  %tmp77 = load i16** %histFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx78 = getelementptr inbounds i16* %tmp77, i32 %tmp76 ; <i16*> [#uses=1]
  %tmp79 = load i16* %arrayidx78                  ; <i16> [#uses=1]
  %tmp80 = load i32* %i                           ; <i32> [#uses=1]
  %tmp81 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp82 = getelementptr inbounds %struct.anon addrspace(1)* %tmp81, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay83 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp82, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds i16 addrspace(1)* %arraydecay83, i32 %tmp80 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp79, i16 addrspace(1)* %arrayidx84
  %tmp85 = load i32* %i                           ; <i32> [#uses=1]
  %tmp86 = load i16** %destFinalPoint.addr        ; <i16*> [#uses=1]
  %arrayidx87 = getelementptr inbounds i16* %tmp86, i32 %tmp85 ; <i16*> [#uses=1]
  %tmp88 = load i16* %arrayidx87                  ; <i16> [#uses=1]
  %tmp89 = load i32* %i                           ; <i32> [#uses=1]
  %tmp90 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp91 = getelementptr inbounds %struct.anon addrspace(1)* %tmp90, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay92 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp91, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx93 = getelementptr inbounds i16 addrspace(1)* %arraydecay92, i32 %tmp89 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp88, i16 addrspace(1)* %arrayidx93
  br label %for.inc94

for.inc94:                                        ; preds = %for.body75
  %tmp95 = load i32* %i                           ; <i32> [#uses=1]
  %inc96 = add nsw i32 %tmp95, 1                  ; <i32> [#uses=1]
  store i32 %inc96, i32* %i
  br label %for.cond71

for.end97:                                        ; preds = %for.cond71
  br label %if.end

if.end:                                           ; preds = %for.end97, %for.end70
  %tmp98 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv99 = bitcast %struct.anon addrspace(1)* %tmp98 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  %tmp100 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv101 = bitcast %struct.anon addrspace(1)* %tmp100 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  call void @kernelMemcpy(i8 addrspace(1)* %conv99, i8 addrspace(1)* %conv101, i32 52)
  store i32 1, i32* %retval
  br label %return

if.else102:                                       ; preds = %cond.end
  %tmp103 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp104 = getelementptr inbounds %struct.anon addrspace(1)* %tmp103, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  %tmp105 = load i32 addrspace(1)* %tmp104        ; <i32> [#uses=1]
  %cmp106 = icmp ne i32 %tmp105, 0                ; <i1> [#uses=1]
  br i1 %cmp106, label %if.then108, label %if.end146

if.then108:                                       ; preds = %if.else102
  %tmp109 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv110 = bitcast %struct.anon addrspace(1)* %tmp109 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  %tmp111 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv112 = bitcast %struct.anon addrspace(1)* %tmp111 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  call void @kernelMemcpy(i8 addrspace(1)* %conv110, i8 addrspace(1)* %conv112, i32 52)
  store i32 0, i32* %i
  br label %for.cond113

for.cond113:                                      ; preds = %for.inc138, %if.then108
  %tmp114 = load i32* %i                          ; <i32> [#uses=1]
  %cmp115 = icmp slt i32 %tmp114, 8               ; <i1> [#uses=1]
  br i1 %cmp115, label %for.body117, label %for.end141

for.body117:                                      ; preds = %for.cond113
  %tmp118 = load i32* %i                          ; <i32> [#uses=1]
  %tmp119 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp120 = getelementptr inbounds %struct.anon addrspace(1)* %tmp119, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay121 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp120, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx122 = getelementptr inbounds i16 addrspace(1)* %arraydecay121, i32 %tmp118 ; <i16 addrspace(1)*> [#uses=1]
  %tmp123 = load i16 addrspace(1)* %arrayidx122   ; <i16> [#uses=1]
  %tmp124 = load i32* %i                          ; <i32> [#uses=1]
  %tmp125 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp126 = getelementptr inbounds %struct.anon addrspace(1)* %tmp125, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay127 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp126, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx128 = getelementptr inbounds i16 addrspace(1)* %arraydecay127, i32 %tmp124 ; <i16 addrspace(1)*> [#uses=1]
  %tmp129 = load i16 addrspace(1)* %arrayidx128   ; <i16> [#uses=1]
  %tmp130 = load i8* %ui8Alpha                    ; <i8> [#uses=1]
  %conv131 = zext i8 %tmp130 to i16               ; <i16> [#uses=1]
  %call132 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp123, i16 zeroext %tmp129, i16 zeroext %conv131) ; <i16> [#uses=1]
  %tmp133 = load i32* %i                          ; <i32> [#uses=1]
  %tmp134 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp135 = getelementptr inbounds %struct.anon addrspace(1)* %tmp134, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay136 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp135, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx137 = getelementptr inbounds i16 addrspace(1)* %arraydecay136, i32 %tmp133 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %call132, i16 addrspace(1)* %arrayidx137
  br label %for.inc138

for.inc138:                                       ; preds = %for.body117
  %tmp139 = load i32* %i                          ; <i32> [#uses=1]
  %inc140 = add nsw i32 %tmp139, 1                ; <i32> [#uses=1]
  store i32 %inc140, i32* %i
  br label %for.cond113

for.end141:                                       ; preds = %for.cond113
  %tmp142 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv143 = bitcast %struct.anon addrspace(1)* %tmp142 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  %tmp144 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv145 = bitcast %struct.anon addrspace(1)* %tmp144 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  call void @kernelMemcpy(i8 addrspace(1)* %conv143, i8 addrspace(1)* %conv145, i32 52)
  store i32 1, i32* %retval
  br label %return

if.end146:                                        ; preds = %if.else102
  br label %if.end147

if.end147:                                        ; preds = %if.end146
  store i32 0, i32* %retval
  br label %return

return:                                           ; preds = %if.end147, %for.end141, %if.end
  %0 = load i32* %retval                          ; <i32> [#uses=1]
  ret i32 %0
}

define void @CalcOutLut(%struct.anon addrspace(1)* %currentLut, i16 addrspace(1)* %outputHistogram) nounwind {
entry:
  %currentLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=6]
  %outputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=9]
  %j = alloca i32, align 4                        ; <i32*> [#uses=6]
  %MinJ = alloca i16, align 2                     ; <i16*> [#uses=4]
  %MaxJ = alloca i16, align 2                     ; <i16*> [#uses=3]
  %DX = alloca i16, align 2                       ; <i16*> [#uses=3]
  %DY = alloca i16, align 2                       ; <i16*> [#uses=3]
  %slp = alloca i16, align 2                      ; <i16*> [#uses=6]
  %dx = alloca i16, align 2                       ; <i16*> [#uses=2]
  %mul = alloca i32, align 4                      ; <i32*> [#uses=2]
  %round = alloca i32, align 4                    ; <i32*> [#uses=2]
  %roundShifted = alloca i16, align 2             ; <i16*> [#uses=2]
  %val = alloca i16, align 2                      ; <i16*> [#uses=5]
  store %struct.anon addrspace(1)* %currentLut, %struct.anon addrspace(1)** %currentLut.addr
  store i16 addrspace(1)* %outputHistogram, i16 addrspace(1)** %outputHistogram.addr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc119, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, 4                     ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end122

for.body:                                         ; preds = %for.cond
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %tmp4 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp5 = getelementptr inbounds %struct.anon addrspace(1)* %tmp4, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x i16] addrspace(1)* %tmp5, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %arraydecay, i32 %tmp3 ; <i16 addrspace(1)*> [#uses=1]
  %tmp6 = load i16 addrspace(1)* %arrayidx        ; <i16> [#uses=1]
  store i16 %tmp6, i16* %MinJ
  %tmp8 = load i32* %i                            ; <i32> [#uses=1]
  %add = add nsw i32 %tmp8, 1                     ; <i32> [#uses=1]
  %tmp9 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp10 = getelementptr inbounds %struct.anon addrspace(1)* %tmp9, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay11 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp10, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds i16 addrspace(1)* %arraydecay11, i32 %add ; <i16 addrspace(1)*> [#uses=1]
  %tmp13 = load i16 addrspace(1)* %arrayidx12     ; <i16> [#uses=1]
  store i16 %tmp13, i16* %MaxJ
  %tmp15 = load i16* %MaxJ                        ; <i16> [#uses=1]
  %conv = zext i16 %tmp15 to i32                  ; <i32> [#uses=1]
  %tmp16 = load i16* %MinJ                        ; <i16> [#uses=1]
  %conv17 = zext i16 %tmp16 to i32                ; <i32> [#uses=1]
  %sub = sub i32 %conv, %conv17                   ; <i32> [#uses=1]
  %conv18 = trunc i32 %sub to i16                 ; <i16> [#uses=1]
  store i16 %conv18, i16* %DX
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp20, 1                  ; <i32> [#uses=1]
  %tmp22 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp23 = getelementptr inbounds %struct.anon addrspace(1)* %tmp22, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay24 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp23, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i16 addrspace(1)* %arraydecay24, i32 %add21 ; <i16 addrspace(1)*> [#uses=1]
  %tmp26 = load i16 addrspace(1)* %arrayidx25     ; <i16> [#uses=1]
  %conv27 = zext i16 %tmp26 to i32                ; <i32> [#uses=1]
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %tmp29 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp30 = getelementptr inbounds %struct.anon addrspace(1)* %tmp29, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay31 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp30, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx32 = getelementptr inbounds i16 addrspace(1)* %arraydecay31, i32 %tmp28 ; <i16 addrspace(1)*> [#uses=1]
  %tmp33 = load i16 addrspace(1)* %arrayidx32     ; <i16> [#uses=1]
  %conv34 = zext i16 %tmp33 to i32                ; <i32> [#uses=1]
  %sub35 = sub i32 %conv27, %conv34               ; <i32> [#uses=1]
  %conv36 = trunc i32 %sub35 to i16               ; <i16> [#uses=1]
  store i16 %conv36, i16* %DY
  %tmp38 = load i16* %DX                          ; <i16> [#uses=1]
  %conv39 = zext i16 %tmp38 to i32                ; <i32> [#uses=1]
  %cmp40 = icmp ne i32 %conv39, 0                 ; <i1> [#uses=1]
  br i1 %cmp40, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %tmp42 = load i16* %DY                          ; <i16> [#uses=1]
  %conv43 = zext i16 %tmp42 to i64                ; <i64> [#uses=1]
  %shl = shl i64 %conv43, 16                      ; <i64> [#uses=1]
  %tmp44 = load i16* %DX                          ; <i16> [#uses=1]
  %conv45 = zext i16 %tmp44 to i32                ; <i32> [#uses=1]
  %conv46 = sext i32 %conv45 to i64               ; <i64> [#uses=2]
  %cmp47 = icmp eq i64 0, %conv46                 ; <i1> [#uses=1]
  %sel = select i1 %cmp47, i64 1, i64 %conv46     ; <i64> [#uses=1]
  %div = sdiv i64 %shl, %sel                      ; <i64> [#uses=1]
  %add48 = add nsw i64 %div, 128                  ; <i64> [#uses=1]
  %shr = ashr i64 %add48, 8                       ; <i64> [#uses=1]
  %conv49 = trunc i64 %shr to i16                 ; <i16> [#uses=1]
  store i16 %conv49, i16* %slp
  %tmp50 = load i16* %slp                         ; <i16> [#uses=1]
  %conv51 = zext i16 %tmp50 to i32                ; <i32> [#uses=1]
  %cmp52 = icmp slt i32 %conv51, 2048             ; <i1> [#uses=1]
  br i1 %cmp52, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.then
  %tmp54 = load i16* %slp                         ; <i16> [#uses=1]
  %conv55 = zext i16 %tmp54 to i32                ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %if.then
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv55, %cond.true ], [ 2048, %cond.false ] ; <i32> [#uses=1]
  %conv56 = trunc i32 %cond to i16                ; <i16> [#uses=1]
  store i16 %conv56, i16* %slp
  br label %if.end

if.else:                                          ; preds = %for.body
  %tmp57 = load i16* %DY                          ; <i16> [#uses=1]
  %conv58 = zext i16 %tmp57 to i32                ; <i32> [#uses=1]
  %cmp59 = icmp eq i32 %conv58, 0                 ; <i1> [#uses=1]
  %cond61 = select i1 %cmp59, i32 0, i32 2048     ; <i32> [#uses=1]
  %conv62 = trunc i32 %cond61 to i16              ; <i16> [#uses=1]
  store i16 %conv62, i16* %slp
  br label %if.end

if.end:                                           ; preds = %if.else, %cond.end
  %tmp63 = load i16* %MinJ                        ; <i16> [#uses=1]
  %conv64 = zext i16 %tmp63 to i32                ; <i32> [#uses=1]
  store i32 %conv64, i32* %j
  br label %for.cond65

for.cond65:                                       ; preds = %for.inc, %if.end
  %tmp66 = load i32* %j                           ; <i32> [#uses=1]
  %tmp67 = load i16* %MaxJ                        ; <i16> [#uses=1]
  %conv68 = zext i16 %tmp67 to i32                ; <i32> [#uses=1]
  %cmp69 = icmp slt i32 %tmp66, %conv68           ; <i1> [#uses=1]
  br i1 %cmp69, label %for.body71, label %for.end

for.body71:                                       ; preds = %for.cond65
  %tmp73 = load i32* %j                           ; <i32> [#uses=1]
  %tmp74 = load i16* %MinJ                        ; <i16> [#uses=1]
  %conv75 = zext i16 %tmp74 to i32                ; <i32> [#uses=1]
  %sub76 = sub i32 %tmp73, %conv75                ; <i32> [#uses=1]
  %conv77 = trunc i32 %sub76 to i16               ; <i16> [#uses=1]
  store i16 %conv77, i16* %dx
  %tmp79 = load i16* %dx                          ; <i16> [#uses=1]
  %conv80 = zext i16 %tmp79 to i32                ; <i32> [#uses=1]
  %tmp81 = load i16* %slp                         ; <i16> [#uses=1]
  %conv82 = zext i16 %tmp81 to i32                ; <i32> [#uses=1]
  %mul83 = mul i32 %conv80, %conv82               ; <i32> [#uses=1]
  store i32 %mul83, i32* %mul
  %tmp85 = load i32* %mul                         ; <i32> [#uses=1]
  %add86 = add i32 %tmp85, 128                    ; <i32> [#uses=1]
  store i32 %add86, i32* %round
  %tmp88 = load i32* %round                       ; <i32> [#uses=1]
  %shr89 = lshr i32 %tmp88, 8                     ; <i32> [#uses=1]
  %conv90 = trunc i32 %shr89 to i16               ; <i16> [#uses=1]
  store i16 %conv90, i16* %roundShifted
  %tmp92 = load i16* %roundShifted                ; <i16> [#uses=1]
  %conv93 = zext i16 %tmp92 to i32                ; <i32> [#uses=1]
  %tmp94 = load i32* %i                           ; <i32> [#uses=1]
  %tmp95 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp96 = getelementptr inbounds %struct.anon addrspace(1)* %tmp95, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay97 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp96, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx98 = getelementptr inbounds i16 addrspace(1)* %arraydecay97, i32 %tmp94 ; <i16 addrspace(1)*> [#uses=1]
  %tmp99 = load i16 addrspace(1)* %arrayidx98     ; <i16> [#uses=1]
  %conv100 = zext i16 %tmp99 to i32               ; <i32> [#uses=1]
  %add101 = add nsw i32 %conv93, %conv100         ; <i32> [#uses=1]
  %conv102 = trunc i32 %add101 to i16             ; <i16> [#uses=1]
  store i16 %conv102, i16* %val
  %tmp103 = load i16* %val                        ; <i16> [#uses=1]
  %conv104 = zext i16 %tmp103 to i32              ; <i32> [#uses=1]
  %cmp105 = icmp slt i32 %conv104, 940            ; <i1> [#uses=1]
  br i1 %cmp105, label %cond.true107, label %cond.false110

cond.true107:                                     ; preds = %for.body71
  %tmp108 = load i16* %val                        ; <i16> [#uses=1]
  %conv109 = zext i16 %tmp108 to i32              ; <i32> [#uses=1]
  br label %cond.end111

cond.false110:                                    ; preds = %for.body71
  br label %cond.end111

cond.end111:                                      ; preds = %cond.false110, %cond.true107
  %cond112 = phi i32 [ %conv109, %cond.true107 ], [ 940, %cond.false110 ] ; <i32> [#uses=1]
  %conv113 = trunc i32 %cond112 to i16            ; <i16> [#uses=1]
  store i16 %conv113, i16* %val
  %tmp114 = load i16* %val                        ; <i16> [#uses=1]
  %tmp115 = load i32* %j                          ; <i32> [#uses=1]
  %tmp116 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx117 = getelementptr inbounds i16 addrspace(1)* %tmp116, i32 %tmp115 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp114, i16 addrspace(1)* %arrayidx117
  br label %for.inc

for.inc:                                          ; preds = %cond.end111
  %tmp118 = load i32* %j                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp118, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond65

for.end:                                          ; preds = %for.cond65
  br label %for.inc119

for.inc119:                                       ; preds = %for.end
  %tmp120 = load i32* %i                          ; <i32> [#uses=1]
  %inc121 = add nsw i32 %tmp120, 1                ; <i32> [#uses=1]
  store i32 %inc121, i32* %i
  br label %for.cond

for.end122:                                       ; preds = %for.cond
  ret void
}

define void @generateOutputHistogram(i32 %imageWidth, i32 %imageHeight, i16 addrspace(1)* %inputHistogram, %struct.anon addrspace(1)* %prevLut, %struct.anon addrspace(1)* %currentLut, %struct.anon addrspace(1)* %brightnessLut, i16 addrspace(1)* %outputHistogram) nounwind {
entry:
  %imageWidth.addr = alloca i32, align 4          ; <i32*> [#uses=1]
  %imageHeight.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %inputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=3]
  %prevLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=3]
  %currentLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=8]
  %brightnessLut.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=5]
  %outputHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=4]
  %histDarkPoint = alloca [8 x i16], align 2      ; <[8 x i16]*> [#uses=2]
  %destDarkPoint = alloca [8 x i16], align 2      ; <[8 x i16]*> [#uses=2]
  %histBrightPoint = alloca [8 x i16], align 2    ; <[8 x i16]*> [#uses=2]
  %destBrightPoint = alloca [8 x i16], align 2    ; <[8 x i16]*> [#uses=2]
  %histFinalPoint = alloca [8 x i16], align 2     ; <[8 x i16]*> [#uses=3]
  %destFinalPoint = alloca [8 x i16], align 2     ; <[8 x i16]*> [#uses=3]
  %histNumPixels = alloca i32, align 4            ; <i32*> [#uses=4]
  %pHistogram = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=17]
  %histIsValid = alloca i32, align 4              ; <i32*> [#uses=4]
  %histAverage = alloca i32, align 4              ; <i32*> [#uses=1]
  %brightImageFactor = alloca i16, align 2        ; <i16*> [#uses=5]
  %ID1 = alloca i16, align 2                      ; <i16*> [#uses=2]
  %ID2 = alloca i16, align 2                      ; <i16*> [#uses=2]
  %iBrLUTinx = alloca i32, align 4                ; <i32*> [#uses=8]
  store i32 %imageWidth, i32* %imageWidth.addr
  store i32 %imageHeight, i32* %imageHeight.addr
  store i16 addrspace(1)* %inputHistogram, i16 addrspace(1)** %inputHistogram.addr
  store %struct.anon addrspace(1)* %prevLut, %struct.anon addrspace(1)** %prevLut.addr
  store %struct.anon addrspace(1)* %currentLut, %struct.anon addrspace(1)** %currentLut.addr
  store %struct.anon addrspace(1)* %brightnessLut, %struct.anon addrspace(1)** %brightnessLut.addr
  store i16 addrspace(1)* %outputHistogram, i16 addrspace(1)** %outputHistogram.addr
  store i32 0, i32* %histNumPixels
  %tmp = load i16 addrspace(1)** %inputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %tmp, i16 addrspace(1)** %pHistogram
  store i32 1, i32* %histIsValid
  %tmp7 = load i16 addrspace(1)** %inputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %call = call i32 @AnalyzeHistogram(i16 addrspace(1)* %tmp7, i16* %brightImageFactor, i32* %histNumPixels, i32* %histAverage) ; <i32> [#uses=1]
  store i32 %call, i32* %histIsValid
  %tmp8 = load i32* %histIsValid                  ; <i32> [#uses=1]
  %tmp9 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp10 = getelementptr inbounds %struct.anon addrspace(1)* %tmp9, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp8, i32 addrspace(1)* %tmp10
  %tmp11 = load i32* %histIsValid                 ; <i32> [#uses=1]
  %cmp = icmp ne i32 %tmp11, 0                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call12 = call i32 @get_global_id(i32 0)        ; <i32> [#uses=1]
  %conv = trunc i32 %call12 to i16                ; <i16> [#uses=1]
  store i16 %conv, i16* %ID1
  %arraydecay = getelementptr inbounds [8 x i16]* %histDarkPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arraydecay13 = getelementptr inbounds [8 x i16]* %destDarkPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %tmp14 = load i16 addrspace(1)** %pHistogram    ; <i16 addrspace(1)*> [#uses=1]
  %tmp15 = load i32* %histNumPixels               ; <i32> [#uses=1]
  call void @FindPoint(i16* getelementptr inbounds ([8 x i16]* @g_pui16HistDarkX, i32 0, i32 0), i16* getelementptr inbounds ([8 x i16]* @g_pui16HistDarkY, i32 0, i32 0), i16* %arraydecay, i16* %arraydecay13, i16 addrspace(1)* %tmp14, i32 %tmp15)
  %call16 = call i32 @get_global_id(i32 1)        ; <i32> [#uses=1]
  %conv17 = trunc i32 %call16 to i16              ; <i16> [#uses=1]
  store i16 %conv17, i16* %ID2
  %arraydecay18 = getelementptr inbounds [8 x i16]* %histBrightPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arraydecay19 = getelementptr inbounds [8 x i16]* %destBrightPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %tmp20 = load i16 addrspace(1)** %pHistogram    ; <i16 addrspace(1)*> [#uses=1]
  %tmp21 = load i32* %histNumPixels               ; <i32> [#uses=1]
  call void @FindPoint(i16* getelementptr inbounds ([8 x i16]* @g_pui16HistBrightX, i32 0, i32 0), i16* getelementptr inbounds ([8 x i16]* @g_pui16HistBrightY, i32 0, i32 0), i16* %arraydecay18, i16* %arraydecay19, i16 addrspace(1)* %tmp20, i32 %tmp21)
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %tmp22 = load i32* %i                           ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp22, 8                 ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp25 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay26 = getelementptr inbounds [8 x i16]* %destBrightPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx = getelementptr inbounds i16* %arraydecay26, i32 %tmp25 ; <i16*> [#uses=1]
  %tmp27 = load i16* %arrayidx                    ; <i16> [#uses=1]
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay29 = getelementptr inbounds [8 x i16]* %destDarkPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx30 = getelementptr inbounds i16* %arraydecay29, i32 %tmp28 ; <i16*> [#uses=1]
  %tmp31 = load i16* %arrayidx30                  ; <i16> [#uses=1]
  %tmp32 = load i16* %brightImageFactor           ; <i16> [#uses=1]
  %call33 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp27, i16 zeroext %tmp31, i16 zeroext %tmp32) ; <i16> [#uses=1]
  %tmp34 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay35 = getelementptr inbounds [8 x i16]* %destFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i16* %arraydecay35, i32 %tmp34 ; <i16*> [#uses=1]
  store i16 %call33, i16* %arrayidx36
  %tmp37 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay38 = getelementptr inbounds [8 x i16]* %histBrightPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx39 = getelementptr inbounds i16* %arraydecay38, i32 %tmp37 ; <i16*> [#uses=1]
  %tmp40 = load i16* %arrayidx39                  ; <i16> [#uses=1]
  %tmp41 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay42 = getelementptr inbounds [8 x i16]* %histDarkPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx43 = getelementptr inbounds i16* %arraydecay42, i32 %tmp41 ; <i16*> [#uses=1]
  %tmp44 = load i16* %arrayidx43                  ; <i16> [#uses=1]
  %tmp45 = load i16* %brightImageFactor           ; <i16> [#uses=1]
  %call46 = call zeroext i16 @AlphaBlendRound_Alpha8Bit(i16 zeroext %tmp40, i16 zeroext %tmp44, i16 zeroext %tmp45) ; <i16> [#uses=1]
  %tmp47 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay48 = getelementptr inbounds [8 x i16]* %histFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arrayidx49 = getelementptr inbounds i16* %arraydecay48, i32 %tmp47 ; <i16*> [#uses=1]
  store i16 %call46, i16* %arrayidx49
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp50 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp50, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp51 = load i16* %brightImageFactor           ; <i16> [#uses=1]
  %arraydecay52 = getelementptr inbounds [8 x i16]* %histFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arraydecay53 = getelementptr inbounds [8 x i16]* %destFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  call void @CorrectSlopes(i16 zeroext %tmp51, i16* %arraydecay52, i16* %arraydecay53, i16* getelementptr inbounds ([8 x i16]* @generateOutputHistogram.rangeMask, i32 0, i32 0))
  br label %if.end

if.end:                                           ; preds = %for.end, %entry
  %tmp54 = load i16* %brightImageFactor           ; <i16> [#uses=1]
  %arraydecay55 = getelementptr inbounds [8 x i16]* %histFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %arraydecay56 = getelementptr inbounds [8 x i16]* %destFinalPoint, i32 0, i32 0 ; <i16*> [#uses=1]
  %tmp57 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp58 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %call59 = call i32 @CreateLinCurve(i16 zeroext %tmp54, i16* %arraydecay55, i16* %arraydecay56, %struct.anon addrspace(1)* %tmp57, %struct.anon addrspace(1)* %tmp58) ; <i32> [#uses=1]
  %tobool = icmp ne i32 %call59, 0                ; <i1> [#uses=1]
  br i1 %tobool, label %if.then60, label %if.else

if.then60:                                        ; preds = %if.end
  store i32 1, i32* %iBrLUTinx
  store i32 1, i32* %i
  br label %for.cond62

for.cond62:                                       ; preds = %for.inc126, %if.then60
  %tmp63 = load i32* %i                           ; <i32> [#uses=1]
  %cmp64 = icmp slt i32 %tmp63, 4                 ; <i1> [#uses=1]
  br i1 %cmp64, label %for.body66, label %for.end129

for.body66:                                       ; preds = %for.cond62
  br label %while.cond

while.cond:                                       ; preds = %while.body, %for.body66
  %tmp67 = load i32* %i                           ; <i32> [#uses=1]
  %tmp68 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp69 = getelementptr inbounds %struct.anon addrspace(1)* %tmp68, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay70 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp69, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx71 = getelementptr inbounds i16 addrspace(1)* %arraydecay70, i32 %tmp67 ; <i16 addrspace(1)*> [#uses=1]
  %tmp72 = load i16 addrspace(1)* %arrayidx71     ; <i16> [#uses=1]
  %conv73 = zext i16 %tmp72 to i32                ; <i32> [#uses=1]
  %tmp74 = load i32* %iBrLUTinx                   ; <i32> [#uses=1]
  %tmp75 = load %struct.anon addrspace(1)** %brightnessLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp76 = getelementptr inbounds %struct.anon addrspace(1)* %tmp75, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay77 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp76, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds i16 addrspace(1)* %arraydecay77, i32 %tmp74 ; <i16 addrspace(1)*> [#uses=1]
  %tmp79 = load i16 addrspace(1)* %arrayidx78     ; <i16> [#uses=1]
  %conv80 = zext i16 %tmp79 to i32                ; <i32> [#uses=1]
  %cmp81 = icmp sgt i32 %conv73, %conv80          ; <i1> [#uses=1]
  br i1 %cmp81, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %tmp83 = load i32* %iBrLUTinx                   ; <i32> [#uses=1]
  %cmp84 = icmp slt i32 %tmp83, 4                 ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %0 = phi i1 [ false, %while.cond ], [ %cmp84, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %tmp86 = load i32* %iBrLUTinx                   ; <i32> [#uses=1]
  %inc87 = add nsw i32 %tmp86, 1                  ; <i32> [#uses=1]
  store i32 %inc87, i32* %iBrLUTinx
  br label %while.cond

while.end:                                        ; preds = %land.end
  %tmp88 = load i32* %i                           ; <i32> [#uses=1]
  %tmp89 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp90 = getelementptr inbounds %struct.anon addrspace(1)* %tmp89, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay91 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp90, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx92 = getelementptr inbounds i16 addrspace(1)* %arraydecay91, i32 %tmp88 ; <i16 addrspace(1)*> [#uses=1]
  %tmp93 = load i16 addrspace(1)* %arrayidx92     ; <i16> [#uses=1]
  %conv94 = zext i16 %tmp93 to i32                ; <i32> [#uses=1]
  %tmp95 = load i32* %iBrLUTinx                   ; <i32> [#uses=1]
  %sub = sub i32 %tmp95, 1                        ; <i32> [#uses=1]
  %tmp96 = load %struct.anon addrspace(1)** %brightnessLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp97 = getelementptr inbounds %struct.anon addrspace(1)* %tmp96, i32 0, i32 0 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay98 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp97, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds i16 addrspace(1)* %arraydecay98, i32 %sub ; <i16 addrspace(1)*> [#uses=1]
  %tmp100 = load i16 addrspace(1)* %arrayidx99    ; <i16> [#uses=1]
  %conv101 = zext i16 %tmp100 to i32              ; <i32> [#uses=1]
  %sub102 = sub i32 %conv94, %conv101             ; <i32> [#uses=1]
  %tmp103 = load i32* %iBrLUTinx                  ; <i32> [#uses=1]
  %sub104 = sub i32 %tmp103, 1                    ; <i32> [#uses=1]
  %tmp105 = load %struct.anon addrspace(1)** %brightnessLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp106 = getelementptr inbounds %struct.anon addrspace(1)* %tmp105, i32 0, i32 2 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay107 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp106, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds i16 addrspace(1)* %arraydecay107, i32 %sub104 ; <i16 addrspace(1)*> [#uses=1]
  %tmp109 = load i16 addrspace(1)* %arrayidx108   ; <i16> [#uses=1]
  %conv110 = zext i16 %tmp109 to i32              ; <i32> [#uses=1]
  %mul = mul i32 %sub102, %conv110                ; <i32> [#uses=1]
  %add = add nsw i32 %mul, 128                    ; <i32> [#uses=1]
  %shr = ashr i32 %add, 8                         ; <i32> [#uses=1]
  %tmp111 = load i32* %iBrLUTinx                  ; <i32> [#uses=1]
  %sub112 = sub i32 %tmp111, 1                    ; <i32> [#uses=1]
  %tmp113 = load %struct.anon addrspace(1)** %brightnessLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp114 = getelementptr inbounds %struct.anon addrspace(1)* %tmp113, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay115 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp114, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx116 = getelementptr inbounds i16 addrspace(1)* %arraydecay115, i32 %sub112 ; <i16 addrspace(1)*> [#uses=1]
  %tmp117 = load i16 addrspace(1)* %arrayidx116   ; <i16> [#uses=1]
  %conv118 = zext i16 %tmp117 to i32              ; <i32> [#uses=1]
  %add119 = add nsw i32 %shr, %conv118            ; <i32> [#uses=1]
  %conv120 = trunc i32 %add119 to i16             ; <i16> [#uses=1]
  %tmp121 = load i32* %i                          ; <i32> [#uses=1]
  %tmp122 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp123 = getelementptr inbounds %struct.anon addrspace(1)* %tmp122, i32 0, i32 1 ; <[8 x i16] addrspace(1)*> [#uses=1]
  %arraydecay124 = getelementptr inbounds [8 x i16] addrspace(1)* %tmp123, i32 0, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx125 = getelementptr inbounds i16 addrspace(1)* %arraydecay124, i32 %tmp121 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %conv120, i16 addrspace(1)* %arrayidx125
  br label %for.inc126

for.inc126:                                       ; preds = %while.end
  %tmp127 = load i32* %i                          ; <i32> [#uses=1]
  %inc128 = add nsw i32 %tmp127, 1                ; <i32> [#uses=1]
  store i32 %inc128, i32* %i
  br label %for.cond62

for.end129:                                       ; preds = %for.cond62
  br label %if.end134

if.else:                                          ; preds = %if.end
  %tmp130 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv131 = bitcast %struct.anon addrspace(1)* %tmp130 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  %tmp132 = load %struct.anon addrspace(1)** %prevLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %conv133 = bitcast %struct.anon addrspace(1)* %tmp132 to i8 addrspace(1)* ; <i8 addrspace(1)*> [#uses=1]
  call void @kernelMemcpy(i8 addrspace(1)* %conv131, i8 addrspace(1)* %conv133, i32 52)
  br label %if.end134

if.end134:                                        ; preds = %if.else, %for.end129
  %tmp135 = load %struct.anon addrspace(1)** %currentLut.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp136 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  call void @CalcOutLut(%struct.anon addrspace(1)* %tmp135, i16 addrspace(1)* %tmp136)
  %tmp137 = load i16* %ID1                        ; <i16> [#uses=1]
  %tmp138 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx139 = getelementptr inbounds i16 addrspace(1)* %tmp138, i32 0 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp137, i16 addrspace(1)* %arrayidx139
  %tmp140 = load i16* %ID2                        ; <i16> [#uses=1]
  %tmp141 = load i16 addrspace(1)** %outputHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx142 = getelementptr inbounds i16 addrspace(1)* %tmp141, i32 1 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp140, i16 addrspace(1)* %arrayidx142
  ret void
}

define void @applyLut(i32 %imageWidth, i32 %imageHeight, i16 addrspace(1)* %imageLumaChannel, i16 addrspace(1)* %targetHistogram, i16 addrspace(1)* %outputLumaChannel) nounwind {
entry:
  %imageWidth.addr = alloca i32, align 4          ; <i32*> [#uses=6]
  %imageHeight.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %imageLumaChannel.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %targetHistogram.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %outputLumaChannel.addr = alloca i16 addrspace(1)*, align 4 ; <i16 addrspace(1)**> [#uses=2]
  %pInPtr = alloca i16 addrspace(1)*, align 4     ; <i16 addrspace(1)**> [#uses=6]
  %pOutPtr = alloca i16 addrspace(1)*, align 4    ; <i16 addrspace(1)**> [#uses=6]
  %ui32StartX = alloca i32, align 4               ; <i32*> [#uses=2]
  %ui32StartY = alloca i32, align 4               ; <i32*> [#uses=4]
  %ui32EndX = alloca i32, align 4                 ; <i32*> [#uses=2]
  %ui32EndY = alloca i32, align 4                 ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  %j = alloca i32, align 4                        ; <i32*> [#uses=6]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=2]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=2]
  %numOfLinesToProcess = alloca i32, align 4      ; <i32*> [#uses=3]
  %yStart = alloca i32, align 4                   ; <i32*> [#uses=3]
  %yEnd = alloca i32, align 4                     ; <i32*> [#uses=2]
  store i32 %imageWidth, i32* %imageWidth.addr
  store i32 %imageHeight, i32* %imageHeight.addr
  store i16 addrspace(1)* %imageLumaChannel, i16 addrspace(1)** %imageLumaChannel.addr
  store i16 addrspace(1)* %targetHistogram, i16 addrspace(1)** %targetHistogram.addr
  store i16 addrspace(1)* %outputLumaChannel, i16 addrspace(1)** %outputLumaChannel.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalId
  %call1 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call1, i32* %globalSize
  %tmp = load i32* %imageHeight.addr              ; <i32> [#uses=1]
  %tmp2 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %numOfLinesToProcess
  %tmp4 = load i32* %globalId                     ; <i32> [#uses=1]
  %tmp5 = load i32* %numOfLinesToProcess          ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  store i32 %mul, i32* %yStart
  %tmp7 = load i32* %yStart                       ; <i32> [#uses=1]
  %tmp8 = load i32* %numOfLinesToProcess          ; <i32> [#uses=1]
  %add = add i32 %tmp7, %tmp8                     ; <i32> [#uses=1]
  store i32 %add, i32* %yEnd
  %tmp9 = load i16 addrspace(1)** %imageLumaChannel.addr ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %tmp9, i16 addrspace(1)** %pInPtr
  %tmp10 = load i16 addrspace(1)** %outputLumaChannel.addr ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %tmp10, i16 addrspace(1)** %pOutPtr
  store i32 0, i32* %ui32StartX
  %tmp11 = load i32* %yStart                      ; <i32> [#uses=1]
  store i32 %tmp11, i32* %ui32StartY
  %tmp12 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  store i32 %tmp12, i32* %ui32EndX
  %tmp13 = load i32* %yEnd                        ; <i32> [#uses=1]
  store i32 %tmp13, i32* %ui32EndY
  %tmp14 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  %tmp15 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp14, %tmp15                 ; <i32> [#uses=1]
  %tmp17 = load i16 addrspace(1)** %pInPtr        ; <i16 addrspace(1)*> [#uses=1]
  %add.ptr = getelementptr inbounds i16 addrspace(1)* %tmp17, i32 %mul16 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr, i16 addrspace(1)** %pInPtr
  %tmp18 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  %tmp19 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  %mul20 = mul i32 %tmp18, %tmp19                 ; <i32> [#uses=1]
  %tmp21 = load i16 addrspace(1)** %pOutPtr       ; <i16 addrspace(1)*> [#uses=1]
  %add.ptr22 = getelementptr inbounds i16 addrspace(1)* %tmp21, i32 %mul20 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr22, i16 addrspace(1)** %pOutPtr
  %tmp23 = load i32* %ui32StartY                  ; <i32> [#uses=1]
  store i32 %tmp23, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc49, %entry
  %tmp24 = load i32* %i                           ; <i32> [#uses=1]
  %tmp25 = load i32* %ui32EndY                    ; <i32> [#uses=1]
  %cmp26 = icmp ult i32 %tmp24, %tmp25            ; <i1> [#uses=1]
  br i1 %cmp26, label %for.body, label %for.end52

for.body:                                         ; preds = %for.cond
  %tmp27 = load i32* %ui32StartX                  ; <i32> [#uses=1]
  store i32 %tmp27, i32* %j
  br label %for.cond28

for.cond28:                                       ; preds = %for.inc, %for.body
  %tmp29 = load i32* %j                           ; <i32> [#uses=1]
  %tmp30 = load i32* %ui32EndX                    ; <i32> [#uses=1]
  %cmp31 = icmp ult i32 %tmp29, %tmp30            ; <i1> [#uses=1]
  br i1 %cmp31, label %for.body32, label %for.end

for.body32:                                       ; preds = %for.cond28
  %tmp33 = load i32* %j                           ; <i32> [#uses=1]
  %tmp34 = load i16 addrspace(1)** %pInPtr        ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i16 addrspace(1)* %tmp34, i32 %tmp33 ; <i16 addrspace(1)*> [#uses=1]
  %tmp35 = load i16 addrspace(1)* %arrayidx       ; <i16> [#uses=1]
  %tmp36 = load i16 addrspace(1)** %targetHistogram.addr ; <i16 addrspace(1)*> [#uses=1]
  %idxprom = zext i16 %tmp35 to i32               ; <i32> [#uses=1]
  %arrayidx37 = getelementptr inbounds i16 addrspace(1)* %tmp36, i32 %idxprom ; <i16 addrspace(1)*> [#uses=1]
  %tmp38 = load i16 addrspace(1)* %arrayidx37     ; <i16> [#uses=1]
  %tmp39 = load i32* %j                           ; <i32> [#uses=1]
  %tmp40 = load i16 addrspace(1)** %pOutPtr       ; <i16 addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds i16 addrspace(1)* %tmp40, i32 %tmp39 ; <i16 addrspace(1)*> [#uses=1]
  store i16 %tmp38, i16 addrspace(1)* %arrayidx41
  br label %for.inc

for.inc:                                          ; preds = %for.body32
  %tmp42 = load i32* %j                           ; <i32> [#uses=1]
  %inc = add i32 %tmp42, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond28

for.end:                                          ; preds = %for.cond28
  %tmp43 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  %tmp44 = load i16 addrspace(1)** %pInPtr        ; <i16 addrspace(1)*> [#uses=1]
  %add.ptr45 = getelementptr inbounds i16 addrspace(1)* %tmp44, i32 %tmp43 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr45, i16 addrspace(1)** %pInPtr
  %tmp46 = load i32* %imageWidth.addr             ; <i32> [#uses=1]
  %tmp47 = load i16 addrspace(1)** %pOutPtr       ; <i16 addrspace(1)*> [#uses=1]
  %add.ptr48 = getelementptr inbounds i16 addrspace(1)* %tmp47, i32 %tmp46 ; <i16 addrspace(1)*> [#uses=1]
  store i16 addrspace(1)* %add.ptr48, i16 addrspace(1)** %pOutPtr
  br label %for.inc49

for.inc49:                                        ; preds = %for.end
  %tmp50 = load i32* %i                           ; <i32> [#uses=1]
  %inc51 = add i32 %tmp50, 1                      ; <i32> [#uses=1]
  store i32 %inc51, i32* %i
  br label %for.cond

for.end52:                                        ; preds = %for.cond
  ret void
}
