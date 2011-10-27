; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\intel_median.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_ckMedian1_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ckMedian1_parameters = appending global [103 x i8] c"uchar4 __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int\00", section "llvm.metadata" ; <[103 x i8]*> [#uses=1]
@opencl_ckMedianScalar_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ckMedianScalar_parameters = appending global [106 x i8] c"uchar __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, int const, int const\00", section "llvm.metadata" ; <[106 x i8]*> [#uses=1]
@opencl_ckMedianBitonic_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ckMedianBitonic_parameters = appending global [105 x i8] c"uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, int const, int const\00", section "llvm.metadata" ; <[105 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @ckMedian1 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ckMedian1_locals to i8*), i8* getelementptr inbounds ([103 x i8]* @opencl_ckMedian1_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @ckMedianScalar to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ckMedianScalar_locals to i8*), i8* getelementptr inbounds ([106 x i8]* @opencl_ckMedianScalar_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @ckMedianBitonic to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ckMedianBitonic_locals to i8*), i8* getelementptr inbounds ([105 x i8]* @opencl_ckMedianBitonic_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @ckMedian1(<4 x i8> addrspace(1)* %uc4Source, i32 addrspace(1)* %uiDest, i32 %uiImageWidth, i32 %uiDevImageHeight) nounwind {
entry:
  %uc4Source.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=4]
  %uiDest.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=4]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=1]
  %y = alloca i32, align 4                        ; <i32*> [#uses=3]
  %x = alloca i32, align 4                        ; <i32*> [#uses=6]
  %ucRGBA = alloca <4 x i8>, align 4              ; <<4 x i8>*> [#uses=12]
  %uiZero = alloca i32, align 4                   ; <i32*> [#uses=1]
  %fMedianEstimate = alloca <4 x i32>, align 16   ; <<4 x i32>*> [#uses=10]
  %fMinBound = alloca <4 x i32>, align 16         ; <<4 x i32>*> [#uses=4]
  %fMaxBound = alloca <4 x i32>, align 16         ; <<4 x i32>*> [#uses=4]
  %zerro = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=2]
  %four = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %mask = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=5]
  %pixels = alloca [9 x <4 x i32>], align 16      ; <[9 x <4 x i32>]*> [#uses=12]
  %pixel_count = alloca i32, align 4              ; <i32*> [#uses=26]
  %iRow = alloca i32, align 4                     ; <i32*> [#uses=5]
  %iLocalOffset = alloca i32, align 4             ; <i32*> [#uses=4]
  %iSearch = alloca i32, align 4                  ; <i32*> [#uses=4]
  %uiHighCount = alloca <4 x i32>, align 16       ; <<4 x i32>*> [#uses=10]
  %iRow121 = alloca i32, align 4                  ; <i32*> [#uses=4]
  %uiPackedPix = alloca i32, align 4              ; <i32*> [#uses=6]
  store <4 x i8> addrspace(1)* %uc4Source, <4 x i8> addrspace(1)** %uc4Source.addr
  store i32 addrspace(1)* %uiDest, i32 addrspace(1)** %uiDest.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %y
  store i32 0, i32* %x
  br label %for.cond

for.cond:                                         ; preds = %for.inc214, %entry
  %tmp = load i32* %x                             ; <i32> [#uses=1]
  %tmp1 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end217

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %uiZero
  store <4 x i32> <i32 128, i32 128, i32 128, i32 128>, <4 x i32>* %fMedianEstimate
  store <4 x i32> zeroinitializer, <4 x i32>* %fMinBound
  store <4 x i32> <i32 255, i32 255, i32 255, i32 255>, <4 x i32>* %fMaxBound
  store <4 x i32> zeroinitializer, <4 x i32>* %zerro
  store <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32>* %four
  store i32 0, i32* %pixel_count
  store i32 -1, i32* %iRow
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc, %for.body
  %tmp14 = load i32* %iRow                        ; <i32> [#uses=1]
  %cmp15 = icmp sle i32 %tmp14, 1                 ; <i1> [#uses=1]
  br i1 %cmp15, label %for.body16, label %for.end

for.body16:                                       ; preds = %for.cond13
  %tmp18 = load i32* %y                           ; <i32> [#uses=1]
  %tmp19 = load i32* %iRow                        ; <i32> [#uses=1]
  %add = add nsw i32 %tmp18, %tmp19               ; <i32> [#uses=1]
  %add20 = add nsw i32 %add, 2                    ; <i32> [#uses=1]
  %tmp21 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %mul = mul i32 %add20, %tmp21                   ; <i32> [#uses=1]
  %tmp22 = load i32* %x                           ; <i32> [#uses=1]
  %add23 = add nsw i32 %mul, %tmp22               ; <i32> [#uses=1]
  store i32 %add23, i32* %iLocalOffset
  %tmp24 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %sub = sub i32 %tmp24, 1                        ; <i32> [#uses=1]
  %tmp25 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %tmp25, i32 %sub ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp26 = load <4 x i8> addrspace(1)* %arrayidx  ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp26, <4 x i8>* %ucRGBA
  %tmp27 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp28 = extractelement <4 x i8> %tmp27, i32 0  ; <i8> [#uses=1]
  %conv = zext i8 %tmp28 to i32                   ; <i32> [#uses=1]
  %tmp29 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx30 = getelementptr inbounds <4 x i32>* %arraydecay, i32 %tmp29 ; <<4 x i32>*> [#uses=2]
  %tmp31 = load <4 x i32>* %arrayidx30            ; <<4 x i32>> [#uses=1]
  %tmp32 = insertelement <4 x i32> %tmp31, i32 %conv, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp32, <4 x i32>* %arrayidx30
  %tmp33 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp34 = extractelement <4 x i8> %tmp33, i32 1  ; <i8> [#uses=1]
  %conv35 = zext i8 %tmp34 to i32                 ; <i32> [#uses=1]
  %tmp36 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay37 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx38 = getelementptr inbounds <4 x i32>* %arraydecay37, i32 %tmp36 ; <<4 x i32>*> [#uses=2]
  %tmp39 = load <4 x i32>* %arrayidx38            ; <<4 x i32>> [#uses=1]
  %tmp40 = insertelement <4 x i32> %tmp39, i32 %conv35, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp40, <4 x i32>* %arrayidx38
  %tmp41 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp42 = extractelement <4 x i8> %tmp41, i32 2  ; <i8> [#uses=1]
  %conv43 = zext i8 %tmp42 to i32                 ; <i32> [#uses=1]
  %tmp44 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay45 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx46 = getelementptr inbounds <4 x i32>* %arraydecay45, i32 %tmp44 ; <<4 x i32>*> [#uses=2]
  %tmp47 = load <4 x i32>* %arrayidx46            ; <<4 x i32>> [#uses=1]
  %tmp48 = insertelement <4 x i32> %tmp47, i32 %conv43, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp48, <4 x i32>* %arrayidx46
  %tmp49 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp49, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %pixel_count
  %tmp50 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %tmp51 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx52 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp51, i32 %tmp50 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp53 = load <4 x i8> addrspace(1)* %arrayidx52 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp53, <4 x i8>* %ucRGBA
  %tmp54 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp55 = extractelement <4 x i8> %tmp54, i32 0  ; <i8> [#uses=1]
  %conv56 = zext i8 %tmp55 to i32                 ; <i32> [#uses=1]
  %tmp57 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay58 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx59 = getelementptr inbounds <4 x i32>* %arraydecay58, i32 %tmp57 ; <<4 x i32>*> [#uses=2]
  %tmp60 = load <4 x i32>* %arrayidx59            ; <<4 x i32>> [#uses=1]
  %tmp61 = insertelement <4 x i32> %tmp60, i32 %conv56, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp61, <4 x i32>* %arrayidx59
  %tmp62 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp63 = extractelement <4 x i8> %tmp62, i32 1  ; <i8> [#uses=1]
  %conv64 = zext i8 %tmp63 to i32                 ; <i32> [#uses=1]
  %tmp65 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay66 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx67 = getelementptr inbounds <4 x i32>* %arraydecay66, i32 %tmp65 ; <<4 x i32>*> [#uses=2]
  %tmp68 = load <4 x i32>* %arrayidx67            ; <<4 x i32>> [#uses=1]
  %tmp69 = insertelement <4 x i32> %tmp68, i32 %conv64, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp69, <4 x i32>* %arrayidx67
  %tmp70 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp71 = extractelement <4 x i8> %tmp70, i32 2  ; <i8> [#uses=1]
  %conv72 = zext i8 %tmp71 to i32                 ; <i32> [#uses=1]
  %tmp73 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay74 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx75 = getelementptr inbounds <4 x i32>* %arraydecay74, i32 %tmp73 ; <<4 x i32>*> [#uses=2]
  %tmp76 = load <4 x i32>* %arrayidx75            ; <<4 x i32>> [#uses=1]
  %tmp77 = insertelement <4 x i32> %tmp76, i32 %conv72, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp77, <4 x i32>* %arrayidx75
  %tmp78 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc79 = add nsw i32 %tmp78, 1                  ; <i32> [#uses=1]
  store i32 %inc79, i32* %pixel_count
  %tmp80 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %add81 = add nsw i32 %tmp80, 1                  ; <i32> [#uses=1]
  %tmp82 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp82, i32 %add81 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp84 = load <4 x i8> addrspace(1)* %arrayidx83 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %tmp84, <4 x i8>* %ucRGBA
  %tmp85 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp86 = extractelement <4 x i8> %tmp85, i32 0  ; <i8> [#uses=1]
  %conv87 = zext i8 %tmp86 to i32                 ; <i32> [#uses=1]
  %tmp88 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay89 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx90 = getelementptr inbounds <4 x i32>* %arraydecay89, i32 %tmp88 ; <<4 x i32>*> [#uses=2]
  %tmp91 = load <4 x i32>* %arrayidx90            ; <<4 x i32>> [#uses=1]
  %tmp92 = insertelement <4 x i32> %tmp91, i32 %conv87, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp92, <4 x i32>* %arrayidx90
  %tmp93 = load <4 x i8>* %ucRGBA                 ; <<4 x i8>> [#uses=1]
  %tmp94 = extractelement <4 x i8> %tmp93, i32 1  ; <i8> [#uses=1]
  %conv95 = zext i8 %tmp94 to i32                 ; <i32> [#uses=1]
  %tmp96 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay97 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx98 = getelementptr inbounds <4 x i32>* %arraydecay97, i32 %tmp96 ; <<4 x i32>*> [#uses=2]
  %tmp99 = load <4 x i32>* %arrayidx98            ; <<4 x i32>> [#uses=1]
  %tmp100 = insertelement <4 x i32> %tmp99, i32 %conv95, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp100, <4 x i32>* %arrayidx98
  %tmp101 = load <4 x i8>* %ucRGBA                ; <<4 x i8>> [#uses=1]
  %tmp102 = extractelement <4 x i8> %tmp101, i32 2 ; <i8> [#uses=1]
  %conv103 = zext i8 %tmp102 to i32               ; <i32> [#uses=1]
  %tmp104 = load i32* %pixel_count                ; <i32> [#uses=1]
  %arraydecay105 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx106 = getelementptr inbounds <4 x i32>* %arraydecay105, i32 %tmp104 ; <<4 x i32>*> [#uses=2]
  %tmp107 = load <4 x i32>* %arrayidx106          ; <<4 x i32>> [#uses=1]
  %tmp108 = insertelement <4 x i32> %tmp107, i32 %conv103, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp108, <4 x i32>* %arrayidx106
  %tmp109 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc110 = add nsw i32 %tmp109, 1                ; <i32> [#uses=1]
  store i32 %inc110, i32* %pixel_count
  br label %for.inc

for.inc:                                          ; preds = %for.body16
  %tmp111 = load i32* %iRow                       ; <i32> [#uses=1]
  %inc112 = add nsw i32 %tmp111, 1                ; <i32> [#uses=1]
  store i32 %inc112, i32* %iRow
  br label %for.cond13

for.end:                                          ; preds = %for.cond13
  store i32 0, i32* %iSearch
  br label %for.cond114

for.cond114:                                      ; preds = %for.inc186, %for.end
  %tmp115 = load i32* %iSearch                    ; <i32> [#uses=1]
  %cmp116 = icmp slt i32 %tmp115, 8               ; <i1> [#uses=1]
  br i1 %cmp116, label %for.body118, label %for.end189

for.body118:                                      ; preds = %for.cond114
  store <4 x i32> zeroinitializer, <4 x i32>* %uiHighCount
  store i32 0, i32* %pixel_count
  store i32 -1, i32* %iRow121
  br label %for.cond122

for.cond122:                                      ; preds = %for.inc159, %for.body118
  %tmp123 = load i32* %iRow121                    ; <i32> [#uses=1]
  %cmp124 = icmp sle i32 %tmp123, 1               ; <i1> [#uses=1]
  br i1 %cmp124, label %for.body126, label %for.end162

for.body126:                                      ; preds = %for.cond122
  %tmp127 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp128 = load i32* %pixel_count                ; <i32> [#uses=1]
  %arraydecay129 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx130 = getelementptr inbounds <4 x i32>* %arraydecay129, i32 %tmp128 ; <<4 x i32>*> [#uses=1]
  %tmp131 = load <4 x i32>* %arrayidx130          ; <<4 x i32>> [#uses=1]
  %cmp132 = icmp slt <4 x i32> %tmp127, %tmp131   ; <<4 x i1>> [#uses=1]
  %sext = sext <4 x i1> %cmp132 to <4 x i32>      ; <<4 x i32>> [#uses=1]
  %tmp133 = load <4 x i32>* %uiHighCount          ; <<4 x i32>> [#uses=1]
  %add134 = add nsw <4 x i32> %tmp133, %sext      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add134, <4 x i32>* %uiHighCount
  %tmp135 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc136 = add nsw i32 %tmp135, 1                ; <i32> [#uses=1]
  store i32 %inc136, i32* %pixel_count
  %tmp137 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp138 = load i32* %pixel_count                ; <i32> [#uses=1]
  %arraydecay139 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx140 = getelementptr inbounds <4 x i32>* %arraydecay139, i32 %tmp138 ; <<4 x i32>*> [#uses=1]
  %tmp141 = load <4 x i32>* %arrayidx140          ; <<4 x i32>> [#uses=1]
  %cmp142 = icmp slt <4 x i32> %tmp137, %tmp141   ; <<4 x i1>> [#uses=1]
  %sext143 = sext <4 x i1> %cmp142 to <4 x i32>   ; <<4 x i32>> [#uses=1]
  %tmp144 = load <4 x i32>* %uiHighCount          ; <<4 x i32>> [#uses=1]
  %add145 = add nsw <4 x i32> %tmp144, %sext143   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add145, <4 x i32>* %uiHighCount
  %tmp146 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc147 = add nsw i32 %tmp146, 1                ; <i32> [#uses=1]
  store i32 %inc147, i32* %pixel_count
  %tmp148 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp149 = load i32* %pixel_count                ; <i32> [#uses=1]
  %arraydecay150 = getelementptr inbounds [9 x <4 x i32>]* %pixels, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx151 = getelementptr inbounds <4 x i32>* %arraydecay150, i32 %tmp149 ; <<4 x i32>*> [#uses=1]
  %tmp152 = load <4 x i32>* %arrayidx151          ; <<4 x i32>> [#uses=1]
  %cmp153 = icmp slt <4 x i32> %tmp148, %tmp152   ; <<4 x i1>> [#uses=1]
  %sext154 = sext <4 x i1> %cmp153 to <4 x i32>   ; <<4 x i32>> [#uses=1]
  %tmp155 = load <4 x i32>* %uiHighCount          ; <<4 x i32>> [#uses=1]
  %add156 = add nsw <4 x i32> %tmp155, %sext154   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add156, <4 x i32>* %uiHighCount
  %tmp157 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc158 = add nsw i32 %tmp157, 1                ; <i32> [#uses=1]
  store i32 %inc158, i32* %pixel_count
  br label %for.inc159

for.inc159:                                       ; preds = %for.body126
  %tmp160 = load i32* %iRow121                    ; <i32> [#uses=1]
  %inc161 = add nsw i32 %tmp160, 1                ; <i32> [#uses=1]
  store i32 %inc161, i32* %iRow121
  br label %for.cond122

for.end162:                                       ; preds = %for.cond122
  %tmp163 = load <4 x i32>* %zerro                ; <<4 x i32>> [#uses=1]
  %tmp164 = load <4 x i32>* %uiHighCount          ; <<4 x i32>> [#uses=1]
  %sub165 = sub <4 x i32> %tmp163, %tmp164        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub165, <4 x i32>* %uiHighCount
  %tmp166 = load <4 x i32>* %uiHighCount          ; <<4 x i32>> [#uses=1]
  %tmp167 = load <4 x i32>* %four                 ; <<4 x i32>> [#uses=1]
  %cmp168 = icmp sgt <4 x i32> %tmp166, %tmp167   ; <<4 x i1>> [#uses=1]
  %sext169 = sext <4 x i1> %cmp168 to <4 x i32>   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sext169, <4 x i32>* %mask
  %tmp170 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp171 = load <4 x i32>* %mask                 ; <<4 x i32>> [#uses=1]
  %and = and <4 x i32> %tmp170, %tmp171           ; <<4 x i32>> [#uses=1]
  %tmp172 = load <4 x i32>* %fMinBound            ; <<4 x i32>> [#uses=1]
  %tmp173 = load <4 x i32>* %mask                 ; <<4 x i32>> [#uses=1]
  %neg = xor <4 x i32> %tmp173, <i32 -1, i32 -1, i32 -1, i32 -1> ; <<4 x i32>> [#uses=1]
  %and174 = and <4 x i32> %tmp172, %neg           ; <<4 x i32>> [#uses=1]
  %or = or <4 x i32> %and, %and174                ; <<4 x i32>> [#uses=1]
  store <4 x i32> %or, <4 x i32>* %fMinBound
  %tmp175 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp176 = load <4 x i32>* %mask                 ; <<4 x i32>> [#uses=1]
  %neg177 = xor <4 x i32> %tmp176, <i32 -1, i32 -1, i32 -1, i32 -1> ; <<4 x i32>> [#uses=1]
  %and178 = and <4 x i32> %tmp175, %neg177        ; <<4 x i32>> [#uses=1]
  %tmp179 = load <4 x i32>* %fMaxBound            ; <<4 x i32>> [#uses=1]
  %tmp180 = load <4 x i32>* %mask                 ; <<4 x i32>> [#uses=1]
  %and181 = and <4 x i32> %tmp179, %tmp180        ; <<4 x i32>> [#uses=1]
  %or182 = or <4 x i32> %and178, %and181          ; <<4 x i32>> [#uses=1]
  store <4 x i32> %or182, <4 x i32>* %fMaxBound
  %tmp183 = load <4 x i32>* %fMaxBound            ; <<4 x i32>> [#uses=1]
  %tmp184 = load <4 x i32>* %fMinBound            ; <<4 x i32>> [#uses=1]
  %add185 = add nsw <4 x i32> %tmp183, %tmp184    ; <<4 x i32>> [#uses=1]
  %shr = ashr <4 x i32> %add185, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %shr, <4 x i32>* %fMedianEstimate
  br label %for.inc186

for.inc186:                                       ; preds = %for.end162
  %tmp187 = load i32* %iSearch                    ; <i32> [#uses=1]
  %inc188 = add nsw i32 %tmp187, 1                ; <i32> [#uses=1]
  store i32 %inc188, i32* %iSearch
  br label %for.cond114

for.end189:                                       ; preds = %for.cond114
  %tmp191 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp192 = extractelement <4 x i32> %tmp191, i32 0 ; <i32> [#uses=1]
  %and193 = and i32 255, %tmp192                  ; <i32> [#uses=1]
  store i32 %and193, i32* %uiPackedPix
  %tmp194 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp195 = extractelement <4 x i32> %tmp194, i32 1 ; <i32> [#uses=1]
  %shl = shl i32 %tmp195, 8                       ; <i32> [#uses=1]
  %and196 = and i32 65280, %shl                   ; <i32> [#uses=1]
  %tmp197 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or198 = or i32 %tmp197, %and196                ; <i32> [#uses=1]
  store i32 %or198, i32* %uiPackedPix
  %tmp199 = load <4 x i32>* %fMedianEstimate      ; <<4 x i32>> [#uses=1]
  %tmp200 = extractelement <4 x i32> %tmp199, i32 2 ; <i32> [#uses=1]
  %shl201 = shl i32 %tmp200, 16                   ; <i32> [#uses=1]
  %and202 = and i32 16711680, %shl201             ; <i32> [#uses=1]
  %tmp203 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or204 = or i32 %tmp203, %and202                ; <i32> [#uses=1]
  store i32 %or204, i32* %uiPackedPix
  %tmp205 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %tmp206 = load i32* %y                          ; <i32> [#uses=1]
  %add207 = add nsw i32 %tmp206, 2                ; <i32> [#uses=1]
  %tmp208 = load i32* %uiImageWidth.addr          ; <i32> [#uses=1]
  %mul209 = mul i32 %add207, %tmp208              ; <i32> [#uses=1]
  %tmp210 = load i32* %x                          ; <i32> [#uses=1]
  %add211 = add nsw i32 %mul209, %tmp210          ; <i32> [#uses=1]
  %tmp212 = load i32 addrspace(1)** %uiDest.addr  ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx213 = getelementptr inbounds i32 addrspace(1)* %tmp212, i32 %add211 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp205, i32 addrspace(1)* %arrayidx213
  br label %for.inc214

for.inc214:                                       ; preds = %for.end189
  %tmp215 = load i32* %x                          ; <i32> [#uses=1]
  %inc216 = add nsw i32 %tmp215, 1                ; <i32> [#uses=1]
  store i32 %inc216, i32* %x
  br label %for.cond

for.end217:                                       ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @ckMedianScalar(i8 addrspace(1)* %uc4Source, i32 addrspace(1)* %uiDest, i32 %uiImageWidth, i32 %uiDevImageHeight) nounwind {
entry:
  %uc4Source.addr = alloca i8 addrspace(1)*, align 4 ; <i8 addrspace(1)**> [#uses=4]
  %uiDest.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=4]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=1]
  %y = alloca i32, align 4                        ; <i32*> [#uses=3]
  %x = alloca i32, align 4                        ; <i32*> [#uses=6]
  %ucRGBA = alloca i8*, align 4                   ; <i8**> [#uses=0]
  %uiZero = alloca i32, align 4                   ; <i32*> [#uses=1]
  %zerro = alloca i32, align 4                    ; <i32*> [#uses=1]
  %four = alloca i32, align 4                     ; <i32*> [#uses=3]
  %result = alloca [4 x i32], align 4             ; <[4 x i32]*> [#uses=4]
  %ch = alloca i32, align 4                       ; <i32*> [#uses=8]
  %iMedianEstimate = alloca i32, align 4          ; <i32*> [#uses=8]
  %iMinBound = alloca i32, align 4                ; <i32*> [#uses=4]
  %iMaxBound = alloca i32, align 4                ; <i32*> [#uses=4]
  %mask = alloca i32, align 4                     ; <i32*> [#uses=0]
  %pixels = alloca [9 x i32], align 4             ; <[9 x i32]*> [#uses=6]
  %pixel_count = alloca i32, align 4              ; <i32*> [#uses=20]
  %iRow = alloca i32, align 4                     ; <i32*> [#uses=5]
  %iLocalOffset = alloca i32, align 4             ; <i32*> [#uses=4]
  %iSearch = alloca i32, align 4                  ; <i32*> [#uses=4]
  %uiHighCount = alloca i32, align 4              ; <i32*> [#uses=9]
  %iRow76 = alloca i32, align 4                   ; <i32*> [#uses=4]
  %uiPackedPix = alloca i32, align 4              ; <i32*> [#uses=6]
  store i8 addrspace(1)* %uc4Source, i8 addrspace(1)** %uc4Source.addr
  store i32 addrspace(1)* %uiDest, i32 addrspace(1)** %uiDest.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %y
  store i32 0, i32* %x
  br label %for.cond

for.cond:                                         ; preds = %for.inc172, %entry
  %tmp = load i32* %x                             ; <i32> [#uses=1]
  %tmp1 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end175

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %uiZero
  store i32 0, i32* %zerro
  store i32 4, i32* %four
  store i32 0, i32* %ch
  br label %for.cond8

for.cond8:                                        ; preds = %for.inc143, %for.body
  %tmp9 = load i32* %ch                           ; <i32> [#uses=1]
  %cmp10 = icmp slt i32 %tmp9, 3                  ; <i1> [#uses=1]
  br i1 %cmp10, label %for.body11, label %for.end146

for.body11:                                       ; preds = %for.cond8
  store i32 128, i32* %iMedianEstimate
  store i32 0, i32* %iMinBound
  store i32 255, i32* %iMaxBound
  store i32 0, i32* %pixel_count
  store i32 -1, i32* %iRow
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc, %for.body11
  %tmp20 = load i32* %iRow                        ; <i32> [#uses=1]
  %cmp21 = icmp sle i32 %tmp20, 1                 ; <i1> [#uses=1]
  br i1 %cmp21, label %for.body22, label %for.end

for.body22:                                       ; preds = %for.cond19
  %tmp24 = load i32* %y                           ; <i32> [#uses=1]
  %tmp25 = load i32* %iRow                        ; <i32> [#uses=1]
  %add = add nsw i32 %tmp24, %tmp25               ; <i32> [#uses=1]
  %add26 = add nsw i32 %add, 2                    ; <i32> [#uses=1]
  %tmp27 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %mul = mul i32 %add26, %tmp27                   ; <i32> [#uses=1]
  %tmp28 = load i32* %x                           ; <i32> [#uses=1]
  %add29 = add nsw i32 %mul, %tmp28               ; <i32> [#uses=1]
  store i32 %add29, i32* %iLocalOffset
  %tmp30 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %sub = sub i32 %tmp30, 1                        ; <i32> [#uses=1]
  %mul31 = mul i32 %sub, 4                        ; <i32> [#uses=1]
  %tmp32 = load i32* %ch                          ; <i32> [#uses=1]
  %add33 = add nsw i32 %mul31, %tmp32             ; <i32> [#uses=1]
  %tmp34 = load i8 addrspace(1)** %uc4Source.addr ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %tmp34, i32 %add33 ; <i8 addrspace(1)*> [#uses=1]
  %tmp35 = load i8 addrspace(1)* %arrayidx        ; <i8> [#uses=1]
  %conv = zext i8 %tmp35 to i32                   ; <i32> [#uses=1]
  %tmp36 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx37 = getelementptr inbounds i32* %arraydecay, i32 %tmp36 ; <i32*> [#uses=1]
  store i32 %conv, i32* %arrayidx37
  %tmp38 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp38, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %pixel_count
  %tmp39 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %mul40 = mul i32 %tmp39, 4                      ; <i32> [#uses=1]
  %tmp41 = load i32* %ch                          ; <i32> [#uses=1]
  %add42 = add nsw i32 %mul40, %tmp41             ; <i32> [#uses=1]
  %tmp43 = load i8 addrspace(1)** %uc4Source.addr ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds i8 addrspace(1)* %tmp43, i32 %add42 ; <i8 addrspace(1)*> [#uses=1]
  %tmp45 = load i8 addrspace(1)* %arrayidx44      ; <i8> [#uses=1]
  %conv46 = zext i8 %tmp45 to i32                 ; <i32> [#uses=1]
  %tmp47 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay48 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx49 = getelementptr inbounds i32* %arraydecay48, i32 %tmp47 ; <i32*> [#uses=1]
  store i32 %conv46, i32* %arrayidx49
  %tmp50 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc51 = add nsw i32 %tmp50, 1                  ; <i32> [#uses=1]
  store i32 %inc51, i32* %pixel_count
  %tmp52 = load i32* %iLocalOffset                ; <i32> [#uses=1]
  %add53 = add nsw i32 %tmp52, 1                  ; <i32> [#uses=1]
  %mul54 = mul i32 %add53, 4                      ; <i32> [#uses=1]
  %tmp55 = load i32* %ch                          ; <i32> [#uses=1]
  %add56 = add nsw i32 %mul54, %tmp55             ; <i32> [#uses=1]
  %tmp57 = load i8 addrspace(1)** %uc4Source.addr ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx58 = getelementptr inbounds i8 addrspace(1)* %tmp57, i32 %add56 ; <i8 addrspace(1)*> [#uses=1]
  %tmp59 = load i8 addrspace(1)* %arrayidx58      ; <i8> [#uses=1]
  %conv60 = zext i8 %tmp59 to i32                 ; <i32> [#uses=1]
  %tmp61 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay62 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx63 = getelementptr inbounds i32* %arraydecay62, i32 %tmp61 ; <i32*> [#uses=1]
  store i32 %conv60, i32* %arrayidx63
  %tmp64 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc65 = add nsw i32 %tmp64, 1                  ; <i32> [#uses=1]
  store i32 %inc65, i32* %pixel_count
  br label %for.inc

for.inc:                                          ; preds = %for.body22
  %tmp66 = load i32* %iRow                        ; <i32> [#uses=1]
  %inc67 = add nsw i32 %tmp66, 1                  ; <i32> [#uses=1]
  store i32 %inc67, i32* %iRow
  br label %for.cond19

for.end:                                          ; preds = %for.cond19
  store i32 0, i32* %iSearch
  br label %for.cond69

for.cond69:                                       ; preds = %for.inc135, %for.end
  %tmp70 = load i32* %iSearch                     ; <i32> [#uses=1]
  %cmp71 = icmp slt i32 %tmp70, 8                 ; <i1> [#uses=1]
  br i1 %cmp71, label %for.body73, label %for.end138

for.body73:                                       ; preds = %for.cond69
  store i32 0, i32* %uiHighCount
  store i32 0, i32* %pixel_count
  store i32 -1, i32* %iRow76
  br label %for.cond77

for.cond77:                                       ; preds = %for.inc115, %for.body73
  %tmp78 = load i32* %iRow76                      ; <i32> [#uses=1]
  %cmp79 = icmp sle i32 %tmp78, 1                 ; <i1> [#uses=1]
  br i1 %cmp79, label %for.body81, label %for.end118

for.body81:                                       ; preds = %for.cond77
  %tmp82 = load i32* %iMedianEstimate             ; <i32> [#uses=1]
  %tmp83 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay84 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx85 = getelementptr inbounds i32* %arraydecay84, i32 %tmp83 ; <i32*> [#uses=1]
  %tmp86 = load i32* %arrayidx85                  ; <i32> [#uses=1]
  %cmp87 = icmp slt i32 %tmp82, %tmp86            ; <i1> [#uses=1]
  %conv88 = zext i1 %cmp87 to i32                 ; <i32> [#uses=1]
  %tmp89 = load i32* %uiHighCount                 ; <i32> [#uses=1]
  %add90 = add nsw i32 %tmp89, %conv88            ; <i32> [#uses=1]
  store i32 %add90, i32* %uiHighCount
  %tmp91 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %inc92 = add nsw i32 %tmp91, 1                  ; <i32> [#uses=1]
  store i32 %inc92, i32* %pixel_count
  %tmp93 = load i32* %iMedianEstimate             ; <i32> [#uses=1]
  %tmp94 = load i32* %pixel_count                 ; <i32> [#uses=1]
  %arraydecay95 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx96 = getelementptr inbounds i32* %arraydecay95, i32 %tmp94 ; <i32*> [#uses=1]
  %tmp97 = load i32* %arrayidx96                  ; <i32> [#uses=1]
  %cmp98 = icmp slt i32 %tmp93, %tmp97            ; <i1> [#uses=1]
  %conv99 = zext i1 %cmp98 to i32                 ; <i32> [#uses=1]
  %tmp100 = load i32* %uiHighCount                ; <i32> [#uses=1]
  %add101 = add nsw i32 %tmp100, %conv99          ; <i32> [#uses=1]
  store i32 %add101, i32* %uiHighCount
  %tmp102 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc103 = add nsw i32 %tmp102, 1                ; <i32> [#uses=1]
  store i32 %inc103, i32* %pixel_count
  %tmp104 = load i32* %iMedianEstimate            ; <i32> [#uses=1]
  %tmp105 = load i32* %pixel_count                ; <i32> [#uses=1]
  %arraydecay106 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx107 = getelementptr inbounds i32* %arraydecay106, i32 %tmp105 ; <i32*> [#uses=1]
  %tmp108 = load i32* %arrayidx107                ; <i32> [#uses=1]
  %cmp109 = icmp slt i32 %tmp104, %tmp108         ; <i1> [#uses=1]
  %conv110 = zext i1 %cmp109 to i32               ; <i32> [#uses=1]
  %tmp111 = load i32* %uiHighCount                ; <i32> [#uses=1]
  %add112 = add nsw i32 %tmp111, %conv110         ; <i32> [#uses=1]
  store i32 %add112, i32* %uiHighCount
  %tmp113 = load i32* %pixel_count                ; <i32> [#uses=1]
  %inc114 = add nsw i32 %tmp113, 1                ; <i32> [#uses=1]
  store i32 %inc114, i32* %pixel_count
  br label %for.inc115

for.inc115:                                       ; preds = %for.body81
  %tmp116 = load i32* %iRow76                     ; <i32> [#uses=1]
  %inc117 = add nsw i32 %tmp116, 1                ; <i32> [#uses=1]
  store i32 %inc117, i32* %iRow76
  br label %for.cond77

for.end118:                                       ; preds = %for.cond77
  %tmp119 = load i32* %uiHighCount                ; <i32> [#uses=1]
  %tmp120 = load i32* %four                       ; <i32> [#uses=1]
  %cmp121 = icmp sgt i32 %tmp119, %tmp120         ; <i1> [#uses=1]
  %tmp123 = load i32* %iMedianEstimate            ; <i32> [#uses=1]
  %tmp124 = load i32* %iMinBound                  ; <i32> [#uses=1]
  %cond = select i1 %cmp121, i32 %tmp123, i32 %tmp124 ; <i32> [#uses=1]
  store i32 %cond, i32* %iMinBound
  %tmp125 = load i32* %uiHighCount                ; <i32> [#uses=1]
  %tmp126 = load i32* %four                       ; <i32> [#uses=1]
  %cmp127 = icmp sle i32 %tmp125, %tmp126         ; <i1> [#uses=1]
  %tmp129 = load i32* %iMedianEstimate            ; <i32> [#uses=1]
  %tmp130 = load i32* %iMaxBound                  ; <i32> [#uses=1]
  %cond131 = select i1 %cmp127, i32 %tmp129, i32 %tmp130 ; <i32> [#uses=1]
  store i32 %cond131, i32* %iMaxBound
  %tmp132 = load i32* %iMaxBound                  ; <i32> [#uses=1]
  %tmp133 = load i32* %iMinBound                  ; <i32> [#uses=1]
  %add134 = add nsw i32 %tmp132, %tmp133          ; <i32> [#uses=1]
  %shr = ashr i32 %add134, 1                      ; <i32> [#uses=1]
  store i32 %shr, i32* %iMedianEstimate
  br label %for.inc135

for.inc135:                                       ; preds = %for.end118
  %tmp136 = load i32* %iSearch                    ; <i32> [#uses=1]
  %inc137 = add nsw i32 %tmp136, 1                ; <i32> [#uses=1]
  store i32 %inc137, i32* %iSearch
  br label %for.cond69

for.end138:                                       ; preds = %for.cond69
  %tmp139 = load i32* %iMedianEstimate            ; <i32> [#uses=1]
  %tmp140 = load i32* %ch                         ; <i32> [#uses=1]
  %arraydecay141 = getelementptr inbounds [4 x i32]* %result, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx142 = getelementptr inbounds i32* %arraydecay141, i32 %tmp140 ; <i32*> [#uses=1]
  store i32 %tmp139, i32* %arrayidx142
  br label %for.inc143

for.inc143:                                       ; preds = %for.end138
  %tmp144 = load i32* %ch                         ; <i32> [#uses=1]
  %inc145 = add nsw i32 %tmp144, 1                ; <i32> [#uses=1]
  store i32 %inc145, i32* %ch
  br label %for.cond8

for.end146:                                       ; preds = %for.cond8
  %arraydecay148 = getelementptr inbounds [4 x i32]* %result, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx149 = getelementptr inbounds i32* %arraydecay148, i32 0 ; <i32*> [#uses=1]
  %tmp150 = load i32* %arrayidx149                ; <i32> [#uses=1]
  %and = and i32 255, %tmp150                     ; <i32> [#uses=1]
  store i32 %and, i32* %uiPackedPix
  %arraydecay151 = getelementptr inbounds [4 x i32]* %result, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx152 = getelementptr inbounds i32* %arraydecay151, i32 1 ; <i32*> [#uses=1]
  %tmp153 = load i32* %arrayidx152                ; <i32> [#uses=1]
  %shl = shl i32 %tmp153, 8                       ; <i32> [#uses=1]
  %and154 = and i32 65280, %shl                   ; <i32> [#uses=1]
  %tmp155 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or = or i32 %tmp155, %and154                   ; <i32> [#uses=1]
  store i32 %or, i32* %uiPackedPix
  %arraydecay156 = getelementptr inbounds [4 x i32]* %result, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx157 = getelementptr inbounds i32* %arraydecay156, i32 2 ; <i32*> [#uses=1]
  %tmp158 = load i32* %arrayidx157                ; <i32> [#uses=1]
  %shl159 = shl i32 %tmp158, 16                   ; <i32> [#uses=1]
  %and160 = and i32 16711680, %shl159             ; <i32> [#uses=1]
  %tmp161 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or162 = or i32 %tmp161, %and160                ; <i32> [#uses=1]
  store i32 %or162, i32* %uiPackedPix
  %tmp163 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %tmp164 = load i32* %y                          ; <i32> [#uses=1]
  %add165 = add nsw i32 %tmp164, 2                ; <i32> [#uses=1]
  %tmp166 = load i32* %uiImageWidth.addr          ; <i32> [#uses=1]
  %mul167 = mul i32 %add165, %tmp166              ; <i32> [#uses=1]
  %tmp168 = load i32* %x                          ; <i32> [#uses=1]
  %add169 = add nsw i32 %mul167, %tmp168          ; <i32> [#uses=1]
  %tmp170 = load i32 addrspace(1)** %uiDest.addr  ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx171 = getelementptr inbounds i32 addrspace(1)* %tmp170, i32 %add169 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp163, i32 addrspace(1)* %arrayidx171
  br label %for.inc172

for.inc172:                                       ; preds = %for.end146
  %tmp173 = load i32* %x                          ; <i32> [#uses=1]
  %inc174 = add nsw i32 %tmp173, 1                ; <i32> [#uses=1]
  store i32 %inc174, i32* %x
  br label %for.cond

for.end175:                                       ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @ckMedianBitonic(i32 addrspace(1)* %uc4Source, i32 addrspace(1)* %uc4Dest, i32 %uiImageWidth, i32 %uiDevImageHeight) nounwind {
entry:
  %uc4Source.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=10]
  %uc4Dest.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=6]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=2]
  %last_row_offset = alloca i32, align 4          ; <i32*> [#uses=1]
  %y = alloca i32, align 4                        ; <i32*> [#uses=2]
  %of = alloca i32, align 4                       ; <i32*> [#uses=7]
  %op = alloca i32, align 4                       ; <i32*> [#uses=4]
  %on = alloca i32, align 4                       ; <i32*> [#uses=4]
  %x = alloca i32, align 4                        ; <i32*> [#uses=14]
  %ucRGBA = alloca [9 x i32], align 4             ; <[9 x i32]*> [#uses=10]
  %res = alloca i32, align 4                      ; <i32*> [#uses=4]
  %pixels = alloca [9 x i32], align 4             ; <[9 x i32]*> [#uses=117]
  %mask = alloca i32, align 4                     ; <i32*> [#uses=4]
  %c = alloca i32, align 4                        ; <i32*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %fmin = alloca i32, align 4                     ; <i32*> [#uses=32]
  %fmax = alloca i32, align 4                     ; <i32*> [#uses=32]
  store i32 addrspace(1)* %uc4Source, i32 addrspace(1)** %uc4Source.addr
  store i32 addrspace(1)* %uc4Dest, i32 addrspace(1)** %uc4Dest.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  %tmp = load i32* %uiImageWidth.addr             ; <i32> [#uses=1]
  %tmp1 = load i32* %uiDevImageHeight.addr        ; <i32> [#uses=1]
  %sub = sub i32 %tmp1, 1                         ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %sub                       ; <i32> [#uses=1]
  store i32 %mul, i32* %last_row_offset
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %y
  %tmp4 = load i32* %y                            ; <i32> [#uses=1]
  %add = add nsw i32 %tmp4, 2                     ; <i32> [#uses=1]
  %tmp5 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %mul6 = mul i32 %add, %tmp5                     ; <i32> [#uses=1]
  store i32 %mul6, i32* %of
  %tmp8 = load i32* %of                           ; <i32> [#uses=1]
  %tmp9 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %sub10 = sub i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  store i32 %sub10, i32* %op
  %tmp12 = load i32* %of                          ; <i32> [#uses=1]
  %tmp13 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %add14 = add nsw i32 %tmp12, %tmp13             ; <i32> [#uses=1]
  store i32 %add14, i32* %on
  store i32 0, i32* %x
  br label %for.cond

for.cond:                                         ; preds = %for.inc512, %entry
  %tmp16 = load i32* %x                           ; <i32> [#uses=1]
  %tmp17 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp16, %tmp17              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end515

for.body:                                         ; preds = %for.cond
  %tmp19 = load i32* %op                          ; <i32> [#uses=1]
  %tmp20 = load i32* %x                           ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp19, %tmp20             ; <i32> [#uses=1]
  %sub22 = sub i32 %add21, 1                      ; <i32> [#uses=1]
  %tmp23 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp23, i32 %sub22 ; <i32 addrspace(1)*> [#uses=1]
  %tmp24 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i32* %arraydecay, i32 0 ; <i32*> [#uses=1]
  store i32 %tmp24, i32* %arrayidx25
  %tmp26 = load i32* %op                          ; <i32> [#uses=1]
  %tmp27 = load i32* %x                           ; <i32> [#uses=1]
  %add28 = add nsw i32 %tmp26, %tmp27             ; <i32> [#uses=1]
  %tmp29 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds i32 addrspace(1)* %tmp29, i32 %add28 ; <i32 addrspace(1)*> [#uses=1]
  %tmp31 = load i32 addrspace(1)* %arrayidx30     ; <i32> [#uses=1]
  %arraydecay32 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx33 = getelementptr inbounds i32* %arraydecay32, i32 1 ; <i32*> [#uses=1]
  store i32 %tmp31, i32* %arrayidx33
  %tmp34 = load i32* %op                          ; <i32> [#uses=1]
  %tmp35 = load i32* %x                           ; <i32> [#uses=1]
  %add36 = add nsw i32 %tmp34, %tmp35             ; <i32> [#uses=1]
  %add37 = add nsw i32 %add36, 1                  ; <i32> [#uses=1]
  %tmp38 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds i32 addrspace(1)* %tmp38, i32 %add37 ; <i32 addrspace(1)*> [#uses=1]
  %tmp40 = load i32 addrspace(1)* %arrayidx39     ; <i32> [#uses=1]
  %arraydecay41 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx42 = getelementptr inbounds i32* %arraydecay41, i32 2 ; <i32*> [#uses=1]
  store i32 %tmp40, i32* %arrayidx42
  %tmp43 = load i32* %of                          ; <i32> [#uses=1]
  %tmp44 = load i32* %x                           ; <i32> [#uses=1]
  %add45 = add nsw i32 %tmp43, %tmp44             ; <i32> [#uses=1]
  %sub46 = sub i32 %add45, 1                      ; <i32> [#uses=1]
  %tmp47 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx48 = getelementptr inbounds i32 addrspace(1)* %tmp47, i32 %sub46 ; <i32 addrspace(1)*> [#uses=1]
  %tmp49 = load i32 addrspace(1)* %arrayidx48     ; <i32> [#uses=1]
  %arraydecay50 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx51 = getelementptr inbounds i32* %arraydecay50, i32 3 ; <i32*> [#uses=1]
  store i32 %tmp49, i32* %arrayidx51
  %tmp52 = load i32* %of                          ; <i32> [#uses=1]
  %tmp53 = load i32* %x                           ; <i32> [#uses=1]
  %add54 = add nsw i32 %tmp52, %tmp53             ; <i32> [#uses=1]
  %tmp55 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds i32 addrspace(1)* %tmp55, i32 %add54 ; <i32 addrspace(1)*> [#uses=1]
  %tmp57 = load i32 addrspace(1)* %arrayidx56     ; <i32> [#uses=1]
  %arraydecay58 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx59 = getelementptr inbounds i32* %arraydecay58, i32 4 ; <i32*> [#uses=1]
  store i32 %tmp57, i32* %arrayidx59
  %tmp60 = load i32* %of                          ; <i32> [#uses=1]
  %tmp61 = load i32* %x                           ; <i32> [#uses=1]
  %add62 = add nsw i32 %tmp60, %tmp61             ; <i32> [#uses=1]
  %add63 = add nsw i32 %add62, 1                  ; <i32> [#uses=1]
  %tmp64 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds i32 addrspace(1)* %tmp64, i32 %add63 ; <i32 addrspace(1)*> [#uses=1]
  %tmp66 = load i32 addrspace(1)* %arrayidx65     ; <i32> [#uses=1]
  %arraydecay67 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx68 = getelementptr inbounds i32* %arraydecay67, i32 5 ; <i32*> [#uses=1]
  store i32 %tmp66, i32* %arrayidx68
  %tmp69 = load i32* %on                          ; <i32> [#uses=1]
  %tmp70 = load i32* %x                           ; <i32> [#uses=1]
  %add71 = add nsw i32 %tmp69, %tmp70             ; <i32> [#uses=1]
  %sub72 = sub i32 %add71, 1                      ; <i32> [#uses=1]
  %tmp73 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx74 = getelementptr inbounds i32 addrspace(1)* %tmp73, i32 %sub72 ; <i32 addrspace(1)*> [#uses=1]
  %tmp75 = load i32 addrspace(1)* %arrayidx74     ; <i32> [#uses=1]
  %arraydecay76 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx77 = getelementptr inbounds i32* %arraydecay76, i32 6 ; <i32*> [#uses=1]
  store i32 %tmp75, i32* %arrayidx77
  %tmp78 = load i32* %on                          ; <i32> [#uses=1]
  %tmp79 = load i32* %x                           ; <i32> [#uses=1]
  %add80 = add nsw i32 %tmp78, %tmp79             ; <i32> [#uses=1]
  %tmp81 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx82 = getelementptr inbounds i32 addrspace(1)* %tmp81, i32 %add80 ; <i32 addrspace(1)*> [#uses=1]
  %tmp83 = load i32 addrspace(1)* %arrayidx82     ; <i32> [#uses=1]
  %arraydecay84 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx85 = getelementptr inbounds i32* %arraydecay84, i32 7 ; <i32*> [#uses=1]
  store i32 %tmp83, i32* %arrayidx85
  %tmp86 = load i32* %on                          ; <i32> [#uses=1]
  %tmp87 = load i32* %x                           ; <i32> [#uses=1]
  %add88 = add nsw i32 %tmp86, %tmp87             ; <i32> [#uses=1]
  %add89 = add nsw i32 %add88, 1                  ; <i32> [#uses=1]
  %tmp90 = load i32 addrspace(1)** %uc4Source.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds i32 addrspace(1)* %tmp90, i32 %add89 ; <i32 addrspace(1)*> [#uses=1]
  %tmp92 = load i32 addrspace(1)* %arrayidx91     ; <i32> [#uses=1]
  %arraydecay93 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx94 = getelementptr inbounds i32* %arraydecay93, i32 8 ; <i32*> [#uses=1]
  store i32 %tmp92, i32* %arrayidx94
  store i32 0, i32* %res
  store i32 255, i32* %mask
  store i32 0, i32* %c
  br label %for.cond99

for.cond99:                                       ; preds = %for.inc502, %for.body
  %tmp100 = load i32* %c                          ; <i32> [#uses=1]
  %cmp101 = icmp slt i32 %tmp100, 3               ; <i1> [#uses=1]
  br i1 %cmp101, label %for.body102, label %for.end505

for.body102:                                      ; preds = %for.cond99
  store i32 0, i32* %i
  br label %for.cond104

for.cond104:                                      ; preds = %for.inc, %for.body102
  %tmp105 = load i32* %i                          ; <i32> [#uses=1]
  %cmp106 = icmp slt i32 %tmp105, 9               ; <i1> [#uses=1]
  br i1 %cmp106, label %for.body107, label %for.end

for.body107:                                      ; preds = %for.cond104
  %tmp108 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay109 = getelementptr inbounds [9 x i32]* %ucRGBA, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx110 = getelementptr inbounds i32* %arraydecay109, i32 %tmp108 ; <i32*> [#uses=1]
  %tmp111 = load i32* %arrayidx110                ; <i32> [#uses=1]
  %tmp112 = load i32* %mask                       ; <i32> [#uses=1]
  %and = and i32 %tmp111, %tmp112                 ; <i32> [#uses=1]
  %tmp113 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay114 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx115 = getelementptr inbounds i32* %arraydecay114, i32 %tmp113 ; <i32*> [#uses=1]
  store i32 %and, i32* %arrayidx115
  br label %for.inc

for.inc:                                          ; preds = %for.body107
  %tmp116 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp116, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond104

for.end:                                          ; preds = %for.cond104
  %arraydecay118 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx119 = getelementptr inbounds i32* %arraydecay118, i32 0 ; <i32*> [#uses=1]
  %tmp120 = load i32* %arrayidx119                ; <i32> [#uses=1]
  %arraydecay121 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx122 = getelementptr inbounds i32* %arraydecay121, i32 1 ; <i32*> [#uses=1]
  %tmp123 = load i32* %arrayidx122                ; <i32> [#uses=1]
  %call124 = call i32 @_Z3minjj(i32 %tmp120, i32 %tmp123) ; <i32> [#uses=1]
  store i32 %call124, i32* %fmin
  %arraydecay126 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx127 = getelementptr inbounds i32* %arraydecay126, i32 0 ; <i32*> [#uses=1]
  %tmp128 = load i32* %arrayidx127                ; <i32> [#uses=1]
  %arraydecay129 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx130 = getelementptr inbounds i32* %arraydecay129, i32 1 ; <i32*> [#uses=1]
  %tmp131 = load i32* %arrayidx130                ; <i32> [#uses=1]
  %call132 = call i32 @_Z3maxjj(i32 %tmp128, i32 %tmp131) ; <i32> [#uses=1]
  store i32 %call132, i32* %fmax
  %tmp133 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay134 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx135 = getelementptr inbounds i32* %arraydecay134, i32 0 ; <i32*> [#uses=1]
  store i32 %tmp133, i32* %arrayidx135
  %tmp136 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay137 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx138 = getelementptr inbounds i32* %arraydecay137, i32 1 ; <i32*> [#uses=1]
  store i32 %tmp136, i32* %arrayidx138
  %arraydecay139 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx140 = getelementptr inbounds i32* %arraydecay139, i32 3 ; <i32*> [#uses=1]
  %tmp141 = load i32* %arrayidx140                ; <i32> [#uses=1]
  %arraydecay142 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx143 = getelementptr inbounds i32* %arraydecay142, i32 2 ; <i32*> [#uses=1]
  %tmp144 = load i32* %arrayidx143                ; <i32> [#uses=1]
  %call145 = call i32 @_Z3minjj(i32 %tmp141, i32 %tmp144) ; <i32> [#uses=1]
  store i32 %call145, i32* %fmin
  %arraydecay146 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx147 = getelementptr inbounds i32* %arraydecay146, i32 3 ; <i32*> [#uses=1]
  %tmp148 = load i32* %arrayidx147                ; <i32> [#uses=1]
  %arraydecay149 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx150 = getelementptr inbounds i32* %arraydecay149, i32 2 ; <i32*> [#uses=1]
  %tmp151 = load i32* %arrayidx150                ; <i32> [#uses=1]
  %call152 = call i32 @_Z3maxjj(i32 %tmp148, i32 %tmp151) ; <i32> [#uses=1]
  store i32 %call152, i32* %fmax
  %tmp153 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay154 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx155 = getelementptr inbounds i32* %arraydecay154, i32 3 ; <i32*> [#uses=1]
  store i32 %tmp153, i32* %arrayidx155
  %tmp156 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay157 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx158 = getelementptr inbounds i32* %arraydecay157, i32 2 ; <i32*> [#uses=1]
  store i32 %tmp156, i32* %arrayidx158
  %arraydecay159 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx160 = getelementptr inbounds i32* %arraydecay159, i32 2 ; <i32*> [#uses=1]
  %tmp161 = load i32* %arrayidx160                ; <i32> [#uses=1]
  %arraydecay162 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx163 = getelementptr inbounds i32* %arraydecay162, i32 0 ; <i32*> [#uses=1]
  %tmp164 = load i32* %arrayidx163                ; <i32> [#uses=1]
  %call165 = call i32 @_Z3minjj(i32 %tmp161, i32 %tmp164) ; <i32> [#uses=1]
  store i32 %call165, i32* %fmin
  %arraydecay166 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx167 = getelementptr inbounds i32* %arraydecay166, i32 2 ; <i32*> [#uses=1]
  %tmp168 = load i32* %arrayidx167                ; <i32> [#uses=1]
  %arraydecay169 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx170 = getelementptr inbounds i32* %arraydecay169, i32 0 ; <i32*> [#uses=1]
  %tmp171 = load i32* %arrayidx170                ; <i32> [#uses=1]
  %call172 = call i32 @_Z3maxjj(i32 %tmp168, i32 %tmp171) ; <i32> [#uses=1]
  store i32 %call172, i32* %fmax
  %tmp173 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay174 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx175 = getelementptr inbounds i32* %arraydecay174, i32 2 ; <i32*> [#uses=1]
  store i32 %tmp173, i32* %arrayidx175
  %tmp176 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay177 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx178 = getelementptr inbounds i32* %arraydecay177, i32 0 ; <i32*> [#uses=1]
  store i32 %tmp176, i32* %arrayidx178
  %arraydecay179 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx180 = getelementptr inbounds i32* %arraydecay179, i32 3 ; <i32*> [#uses=1]
  %tmp181 = load i32* %arrayidx180                ; <i32> [#uses=1]
  %arraydecay182 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx183 = getelementptr inbounds i32* %arraydecay182, i32 1 ; <i32*> [#uses=1]
  %tmp184 = load i32* %arrayidx183                ; <i32> [#uses=1]
  %call185 = call i32 @_Z3minjj(i32 %tmp181, i32 %tmp184) ; <i32> [#uses=1]
  store i32 %call185, i32* %fmin
  %arraydecay186 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx187 = getelementptr inbounds i32* %arraydecay186, i32 3 ; <i32*> [#uses=1]
  %tmp188 = load i32* %arrayidx187                ; <i32> [#uses=1]
  %arraydecay189 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx190 = getelementptr inbounds i32* %arraydecay189, i32 1 ; <i32*> [#uses=1]
  %tmp191 = load i32* %arrayidx190                ; <i32> [#uses=1]
  %call192 = call i32 @_Z3maxjj(i32 %tmp188, i32 %tmp191) ; <i32> [#uses=1]
  store i32 %call192, i32* %fmax
  %tmp193 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay194 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx195 = getelementptr inbounds i32* %arraydecay194, i32 3 ; <i32*> [#uses=1]
  store i32 %tmp193, i32* %arrayidx195
  %tmp196 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay197 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx198 = getelementptr inbounds i32* %arraydecay197, i32 1 ; <i32*> [#uses=1]
  store i32 %tmp196, i32* %arrayidx198
  %arraydecay199 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx200 = getelementptr inbounds i32* %arraydecay199, i32 1 ; <i32*> [#uses=1]
  %tmp201 = load i32* %arrayidx200                ; <i32> [#uses=1]
  %arraydecay202 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx203 = getelementptr inbounds i32* %arraydecay202, i32 0 ; <i32*> [#uses=1]
  %tmp204 = load i32* %arrayidx203                ; <i32> [#uses=1]
  %call205 = call i32 @_Z3minjj(i32 %tmp201, i32 %tmp204) ; <i32> [#uses=1]
  store i32 %call205, i32* %fmin
  %arraydecay206 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx207 = getelementptr inbounds i32* %arraydecay206, i32 1 ; <i32*> [#uses=1]
  %tmp208 = load i32* %arrayidx207                ; <i32> [#uses=1]
  %arraydecay209 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx210 = getelementptr inbounds i32* %arraydecay209, i32 0 ; <i32*> [#uses=1]
  %tmp211 = load i32* %arrayidx210                ; <i32> [#uses=1]
  %call212 = call i32 @_Z3maxjj(i32 %tmp208, i32 %tmp211) ; <i32> [#uses=1]
  store i32 %call212, i32* %fmax
  %tmp213 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay214 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx215 = getelementptr inbounds i32* %arraydecay214, i32 1 ; <i32*> [#uses=1]
  store i32 %tmp213, i32* %arrayidx215
  %tmp216 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay217 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx218 = getelementptr inbounds i32* %arraydecay217, i32 0 ; <i32*> [#uses=1]
  store i32 %tmp216, i32* %arrayidx218
  %arraydecay219 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx220 = getelementptr inbounds i32* %arraydecay219, i32 3 ; <i32*> [#uses=1]
  %tmp221 = load i32* %arrayidx220                ; <i32> [#uses=1]
  %arraydecay222 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx223 = getelementptr inbounds i32* %arraydecay222, i32 2 ; <i32*> [#uses=1]
  %tmp224 = load i32* %arrayidx223                ; <i32> [#uses=1]
  %call225 = call i32 @_Z3minjj(i32 %tmp221, i32 %tmp224) ; <i32> [#uses=1]
  store i32 %call225, i32* %fmin
  %arraydecay226 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx227 = getelementptr inbounds i32* %arraydecay226, i32 3 ; <i32*> [#uses=1]
  %tmp228 = load i32* %arrayidx227                ; <i32> [#uses=1]
  %arraydecay229 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx230 = getelementptr inbounds i32* %arraydecay229, i32 2 ; <i32*> [#uses=1]
  %tmp231 = load i32* %arrayidx230                ; <i32> [#uses=1]
  %call232 = call i32 @_Z3maxjj(i32 %tmp228, i32 %tmp231) ; <i32> [#uses=1]
  store i32 %call232, i32* %fmax
  %tmp233 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay234 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx235 = getelementptr inbounds i32* %arraydecay234, i32 3 ; <i32*> [#uses=1]
  store i32 %tmp233, i32* %arrayidx235
  %tmp236 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay237 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx238 = getelementptr inbounds i32* %arraydecay237, i32 2 ; <i32*> [#uses=1]
  store i32 %tmp236, i32* %arrayidx238
  %arraydecay239 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx240 = getelementptr inbounds i32* %arraydecay239, i32 5 ; <i32*> [#uses=1]
  %tmp241 = load i32* %arrayidx240                ; <i32> [#uses=1]
  %arraydecay242 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx243 = getelementptr inbounds i32* %arraydecay242, i32 4 ; <i32*> [#uses=1]
  %tmp244 = load i32* %arrayidx243                ; <i32> [#uses=1]
  %call245 = call i32 @_Z3minjj(i32 %tmp241, i32 %tmp244) ; <i32> [#uses=1]
  store i32 %call245, i32* %fmin
  %arraydecay246 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx247 = getelementptr inbounds i32* %arraydecay246, i32 5 ; <i32*> [#uses=1]
  %tmp248 = load i32* %arrayidx247                ; <i32> [#uses=1]
  %arraydecay249 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx250 = getelementptr inbounds i32* %arraydecay249, i32 4 ; <i32*> [#uses=1]
  %tmp251 = load i32* %arrayidx250                ; <i32> [#uses=1]
  %call252 = call i32 @_Z3maxjj(i32 %tmp248, i32 %tmp251) ; <i32> [#uses=1]
  store i32 %call252, i32* %fmax
  %tmp253 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay254 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx255 = getelementptr inbounds i32* %arraydecay254, i32 5 ; <i32*> [#uses=1]
  store i32 %tmp253, i32* %arrayidx255
  %tmp256 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay257 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx258 = getelementptr inbounds i32* %arraydecay257, i32 4 ; <i32*> [#uses=1]
  store i32 %tmp256, i32* %arrayidx258
  %arraydecay259 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx260 = getelementptr inbounds i32* %arraydecay259, i32 7 ; <i32*> [#uses=1]
  %tmp261 = load i32* %arrayidx260                ; <i32> [#uses=1]
  %arraydecay262 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx263 = getelementptr inbounds i32* %arraydecay262, i32 8 ; <i32*> [#uses=1]
  %tmp264 = load i32* %arrayidx263                ; <i32> [#uses=1]
  %call265 = call i32 @_Z3minjj(i32 %tmp261, i32 %tmp264) ; <i32> [#uses=1]
  store i32 %call265, i32* %fmin
  %arraydecay266 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx267 = getelementptr inbounds i32* %arraydecay266, i32 7 ; <i32*> [#uses=1]
  %tmp268 = load i32* %arrayidx267                ; <i32> [#uses=1]
  %arraydecay269 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx270 = getelementptr inbounds i32* %arraydecay269, i32 8 ; <i32*> [#uses=1]
  %tmp271 = load i32* %arrayidx270                ; <i32> [#uses=1]
  %call272 = call i32 @_Z3maxjj(i32 %tmp268, i32 %tmp271) ; <i32> [#uses=1]
  store i32 %call272, i32* %fmax
  %tmp273 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay274 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx275 = getelementptr inbounds i32* %arraydecay274, i32 7 ; <i32*> [#uses=1]
  store i32 %tmp273, i32* %arrayidx275
  %tmp276 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay277 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx278 = getelementptr inbounds i32* %arraydecay277, i32 8 ; <i32*> [#uses=1]
  store i32 %tmp276, i32* %arrayidx278
  %arraydecay279 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx280 = getelementptr inbounds i32* %arraydecay279, i32 6 ; <i32*> [#uses=1]
  %tmp281 = load i32* %arrayidx280                ; <i32> [#uses=1]
  %arraydecay282 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx283 = getelementptr inbounds i32* %arraydecay282, i32 8 ; <i32*> [#uses=1]
  %tmp284 = load i32* %arrayidx283                ; <i32> [#uses=1]
  %call285 = call i32 @_Z3minjj(i32 %tmp281, i32 %tmp284) ; <i32> [#uses=1]
  store i32 %call285, i32* %fmin
  %arraydecay286 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx287 = getelementptr inbounds i32* %arraydecay286, i32 6 ; <i32*> [#uses=1]
  %tmp288 = load i32* %arrayidx287                ; <i32> [#uses=1]
  %arraydecay289 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx290 = getelementptr inbounds i32* %arraydecay289, i32 8 ; <i32*> [#uses=1]
  %tmp291 = load i32* %arrayidx290                ; <i32> [#uses=1]
  %call292 = call i32 @_Z3maxjj(i32 %tmp288, i32 %tmp291) ; <i32> [#uses=1]
  store i32 %call292, i32* %fmax
  %tmp293 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay294 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx295 = getelementptr inbounds i32* %arraydecay294, i32 6 ; <i32*> [#uses=1]
  store i32 %tmp293, i32* %arrayidx295
  %tmp296 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay297 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx298 = getelementptr inbounds i32* %arraydecay297, i32 8 ; <i32*> [#uses=1]
  store i32 %tmp296, i32* %arrayidx298
  %arraydecay299 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx300 = getelementptr inbounds i32* %arraydecay299, i32 6 ; <i32*> [#uses=1]
  %tmp301 = load i32* %arrayidx300                ; <i32> [#uses=1]
  %arraydecay302 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx303 = getelementptr inbounds i32* %arraydecay302, i32 7 ; <i32*> [#uses=1]
  %tmp304 = load i32* %arrayidx303                ; <i32> [#uses=1]
  %call305 = call i32 @_Z3minjj(i32 %tmp301, i32 %tmp304) ; <i32> [#uses=1]
  store i32 %call305, i32* %fmin
  %arraydecay306 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx307 = getelementptr inbounds i32* %arraydecay306, i32 6 ; <i32*> [#uses=1]
  %tmp308 = load i32* %arrayidx307                ; <i32> [#uses=1]
  %arraydecay309 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx310 = getelementptr inbounds i32* %arraydecay309, i32 7 ; <i32*> [#uses=1]
  %tmp311 = load i32* %arrayidx310                ; <i32> [#uses=1]
  %call312 = call i32 @_Z3maxjj(i32 %tmp308, i32 %tmp311) ; <i32> [#uses=1]
  store i32 %call312, i32* %fmax
  %tmp313 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay314 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx315 = getelementptr inbounds i32* %arraydecay314, i32 6 ; <i32*> [#uses=1]
  store i32 %tmp313, i32* %arrayidx315
  %tmp316 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay317 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx318 = getelementptr inbounds i32* %arraydecay317, i32 7 ; <i32*> [#uses=1]
  store i32 %tmp316, i32* %arrayidx318
  %arraydecay319 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx320 = getelementptr inbounds i32* %arraydecay319, i32 4 ; <i32*> [#uses=1]
  %tmp321 = load i32* %arrayidx320                ; <i32> [#uses=1]
  %arraydecay322 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx323 = getelementptr inbounds i32* %arraydecay322, i32 8 ; <i32*> [#uses=1]
  %tmp324 = load i32* %arrayidx323                ; <i32> [#uses=1]
  %call325 = call i32 @_Z3minjj(i32 %tmp321, i32 %tmp324) ; <i32> [#uses=1]
  store i32 %call325, i32* %fmin
  %arraydecay326 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx327 = getelementptr inbounds i32* %arraydecay326, i32 4 ; <i32*> [#uses=1]
  %tmp328 = load i32* %arrayidx327                ; <i32> [#uses=1]
  %arraydecay329 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx330 = getelementptr inbounds i32* %arraydecay329, i32 8 ; <i32*> [#uses=1]
  %tmp331 = load i32* %arrayidx330                ; <i32> [#uses=1]
  %call332 = call i32 @_Z3maxjj(i32 %tmp328, i32 %tmp331) ; <i32> [#uses=1]
  store i32 %call332, i32* %fmax
  %tmp333 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay334 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx335 = getelementptr inbounds i32* %arraydecay334, i32 4 ; <i32*> [#uses=1]
  store i32 %tmp333, i32* %arrayidx335
  %tmp336 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay337 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx338 = getelementptr inbounds i32* %arraydecay337, i32 8 ; <i32*> [#uses=1]
  store i32 %tmp336, i32* %arrayidx338
  %arraydecay339 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx340 = getelementptr inbounds i32* %arraydecay339, i32 4 ; <i32*> [#uses=1]
  %tmp341 = load i32* %arrayidx340                ; <i32> [#uses=1]
  %arraydecay342 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx343 = getelementptr inbounds i32* %arraydecay342, i32 6 ; <i32*> [#uses=1]
  %tmp344 = load i32* %arrayidx343                ; <i32> [#uses=1]
  %call345 = call i32 @_Z3minjj(i32 %tmp341, i32 %tmp344) ; <i32> [#uses=1]
  store i32 %call345, i32* %fmin
  %arraydecay346 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx347 = getelementptr inbounds i32* %arraydecay346, i32 4 ; <i32*> [#uses=1]
  %tmp348 = load i32* %arrayidx347                ; <i32> [#uses=1]
  %arraydecay349 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx350 = getelementptr inbounds i32* %arraydecay349, i32 6 ; <i32*> [#uses=1]
  %tmp351 = load i32* %arrayidx350                ; <i32> [#uses=1]
  %call352 = call i32 @_Z3maxjj(i32 %tmp348, i32 %tmp351) ; <i32> [#uses=1]
  store i32 %call352, i32* %fmax
  %tmp353 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay354 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx355 = getelementptr inbounds i32* %arraydecay354, i32 4 ; <i32*> [#uses=1]
  store i32 %tmp353, i32* %arrayidx355
  %tmp356 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay357 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx358 = getelementptr inbounds i32* %arraydecay357, i32 6 ; <i32*> [#uses=1]
  store i32 %tmp356, i32* %arrayidx358
  %arraydecay359 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx360 = getelementptr inbounds i32* %arraydecay359, i32 5 ; <i32*> [#uses=1]
  %tmp361 = load i32* %arrayidx360                ; <i32> [#uses=1]
  %arraydecay362 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx363 = getelementptr inbounds i32* %arraydecay362, i32 7 ; <i32*> [#uses=1]
  %tmp364 = load i32* %arrayidx363                ; <i32> [#uses=1]
  %call365 = call i32 @_Z3minjj(i32 %tmp361, i32 %tmp364) ; <i32> [#uses=1]
  store i32 %call365, i32* %fmin
  %arraydecay366 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx367 = getelementptr inbounds i32* %arraydecay366, i32 5 ; <i32*> [#uses=1]
  %tmp368 = load i32* %arrayidx367                ; <i32> [#uses=1]
  %arraydecay369 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx370 = getelementptr inbounds i32* %arraydecay369, i32 7 ; <i32*> [#uses=1]
  %tmp371 = load i32* %arrayidx370                ; <i32> [#uses=1]
  %call372 = call i32 @_Z3maxjj(i32 %tmp368, i32 %tmp371) ; <i32> [#uses=1]
  store i32 %call372, i32* %fmax
  %tmp373 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay374 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx375 = getelementptr inbounds i32* %arraydecay374, i32 5 ; <i32*> [#uses=1]
  store i32 %tmp373, i32* %arrayidx375
  %tmp376 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay377 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx378 = getelementptr inbounds i32* %arraydecay377, i32 7 ; <i32*> [#uses=1]
  store i32 %tmp376, i32* %arrayidx378
  %arraydecay379 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx380 = getelementptr inbounds i32* %arraydecay379, i32 4 ; <i32*> [#uses=1]
  %tmp381 = load i32* %arrayidx380                ; <i32> [#uses=1]
  %arraydecay382 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx383 = getelementptr inbounds i32* %arraydecay382, i32 5 ; <i32*> [#uses=1]
  %tmp384 = load i32* %arrayidx383                ; <i32> [#uses=1]
  %call385 = call i32 @_Z3minjj(i32 %tmp381, i32 %tmp384) ; <i32> [#uses=1]
  store i32 %call385, i32* %fmin
  %arraydecay386 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx387 = getelementptr inbounds i32* %arraydecay386, i32 4 ; <i32*> [#uses=1]
  %tmp388 = load i32* %arrayidx387                ; <i32> [#uses=1]
  %arraydecay389 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx390 = getelementptr inbounds i32* %arraydecay389, i32 5 ; <i32*> [#uses=1]
  %tmp391 = load i32* %arrayidx390                ; <i32> [#uses=1]
  %call392 = call i32 @_Z3maxjj(i32 %tmp388, i32 %tmp391) ; <i32> [#uses=1]
  store i32 %call392, i32* %fmax
  %tmp393 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay394 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx395 = getelementptr inbounds i32* %arraydecay394, i32 4 ; <i32*> [#uses=1]
  store i32 %tmp393, i32* %arrayidx395
  %tmp396 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay397 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx398 = getelementptr inbounds i32* %arraydecay397, i32 5 ; <i32*> [#uses=1]
  store i32 %tmp396, i32* %arrayidx398
  %arraydecay399 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx400 = getelementptr inbounds i32* %arraydecay399, i32 6 ; <i32*> [#uses=1]
  %tmp401 = load i32* %arrayidx400                ; <i32> [#uses=1]
  %arraydecay402 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx403 = getelementptr inbounds i32* %arraydecay402, i32 7 ; <i32*> [#uses=1]
  %tmp404 = load i32* %arrayidx403                ; <i32> [#uses=1]
  %call405 = call i32 @_Z3minjj(i32 %tmp401, i32 %tmp404) ; <i32> [#uses=1]
  store i32 %call405, i32* %fmin
  %arraydecay406 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx407 = getelementptr inbounds i32* %arraydecay406, i32 6 ; <i32*> [#uses=1]
  %tmp408 = load i32* %arrayidx407                ; <i32> [#uses=1]
  %arraydecay409 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx410 = getelementptr inbounds i32* %arraydecay409, i32 7 ; <i32*> [#uses=1]
  %tmp411 = load i32* %arrayidx410                ; <i32> [#uses=1]
  %call412 = call i32 @_Z3maxjj(i32 %tmp408, i32 %tmp411) ; <i32> [#uses=1]
  store i32 %call412, i32* %fmax
  %tmp413 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay414 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx415 = getelementptr inbounds i32* %arraydecay414, i32 6 ; <i32*> [#uses=1]
  store i32 %tmp413, i32* %arrayidx415
  %tmp416 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay417 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx418 = getelementptr inbounds i32* %arraydecay417, i32 7 ; <i32*> [#uses=1]
  store i32 %tmp416, i32* %arrayidx418
  %arraydecay419 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx420 = getelementptr inbounds i32* %arraydecay419, i32 0 ; <i32*> [#uses=1]
  %tmp421 = load i32* %arrayidx420                ; <i32> [#uses=1]
  %arraydecay422 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx423 = getelementptr inbounds i32* %arraydecay422, i32 8 ; <i32*> [#uses=1]
  %tmp424 = load i32* %arrayidx423                ; <i32> [#uses=1]
  %call425 = call i32 @_Z3minjj(i32 %tmp421, i32 %tmp424) ; <i32> [#uses=1]
  store i32 %call425, i32* %fmin
  %arraydecay426 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx427 = getelementptr inbounds i32* %arraydecay426, i32 0 ; <i32*> [#uses=1]
  %tmp428 = load i32* %arrayidx427                ; <i32> [#uses=1]
  %arraydecay429 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx430 = getelementptr inbounds i32* %arraydecay429, i32 8 ; <i32*> [#uses=1]
  %tmp431 = load i32* %arrayidx430                ; <i32> [#uses=1]
  %call432 = call i32 @_Z3maxjj(i32 %tmp428, i32 %tmp431) ; <i32> [#uses=1]
  store i32 %call432, i32* %fmax
  %tmp433 = load i32* %fmin                       ; <i32> [#uses=1]
  %arraydecay434 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx435 = getelementptr inbounds i32* %arraydecay434, i32 0 ; <i32*> [#uses=1]
  store i32 %tmp433, i32* %arrayidx435
  %tmp436 = load i32* %fmax                       ; <i32> [#uses=1]
  %arraydecay437 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx438 = getelementptr inbounds i32* %arraydecay437, i32 8 ; <i32*> [#uses=1]
  store i32 %tmp436, i32* %arrayidx438
  %arraydecay439 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx440 = getelementptr inbounds i32* %arraydecay439, i32 0 ; <i32*> [#uses=1]
  %tmp441 = load i32* %arrayidx440                ; <i32> [#uses=1]
  %arraydecay442 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx443 = getelementptr inbounds i32* %arraydecay442, i32 4 ; <i32*> [#uses=1]
  %tmp444 = load i32* %arrayidx443                ; <i32> [#uses=1]
  %call445 = call i32 @_Z3maxjj(i32 %tmp441, i32 %tmp444) ; <i32> [#uses=1]
  %arraydecay446 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx447 = getelementptr inbounds i32* %arraydecay446, i32 4 ; <i32*> [#uses=1]
  store i32 %call445, i32* %arrayidx447
  %arraydecay448 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx449 = getelementptr inbounds i32* %arraydecay448, i32 1 ; <i32*> [#uses=1]
  %tmp450 = load i32* %arrayidx449                ; <i32> [#uses=1]
  %arraydecay451 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx452 = getelementptr inbounds i32* %arraydecay451, i32 5 ; <i32*> [#uses=1]
  %tmp453 = load i32* %arrayidx452                ; <i32> [#uses=1]
  %call454 = call i32 @_Z3maxjj(i32 %tmp450, i32 %tmp453) ; <i32> [#uses=1]
  %arraydecay455 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx456 = getelementptr inbounds i32* %arraydecay455, i32 5 ; <i32*> [#uses=1]
  store i32 %call454, i32* %arrayidx456
  %arraydecay457 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx458 = getelementptr inbounds i32* %arraydecay457, i32 2 ; <i32*> [#uses=1]
  %tmp459 = load i32* %arrayidx458                ; <i32> [#uses=1]
  %arraydecay460 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx461 = getelementptr inbounds i32* %arraydecay460, i32 6 ; <i32*> [#uses=1]
  %tmp462 = load i32* %arrayidx461                ; <i32> [#uses=1]
  %call463 = call i32 @_Z3maxjj(i32 %tmp459, i32 %tmp462) ; <i32> [#uses=1]
  %arraydecay464 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx465 = getelementptr inbounds i32* %arraydecay464, i32 6 ; <i32*> [#uses=1]
  store i32 %call463, i32* %arrayidx465
  %arraydecay466 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx467 = getelementptr inbounds i32* %arraydecay466, i32 3 ; <i32*> [#uses=1]
  %tmp468 = load i32* %arrayidx467                ; <i32> [#uses=1]
  %arraydecay469 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx470 = getelementptr inbounds i32* %arraydecay469, i32 7 ; <i32*> [#uses=1]
  %tmp471 = load i32* %arrayidx470                ; <i32> [#uses=1]
  %call472 = call i32 @_Z3maxjj(i32 %tmp468, i32 %tmp471) ; <i32> [#uses=1]
  %arraydecay473 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx474 = getelementptr inbounds i32* %arraydecay473, i32 7 ; <i32*> [#uses=1]
  store i32 %call472, i32* %arrayidx474
  %arraydecay475 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx476 = getelementptr inbounds i32* %arraydecay475, i32 4 ; <i32*> [#uses=1]
  %tmp477 = load i32* %arrayidx476                ; <i32> [#uses=1]
  %arraydecay478 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx479 = getelementptr inbounds i32* %arraydecay478, i32 6 ; <i32*> [#uses=1]
  %tmp480 = load i32* %arrayidx479                ; <i32> [#uses=1]
  %call481 = call i32 @_Z3minjj(i32 %tmp477, i32 %tmp480) ; <i32> [#uses=1]
  %arraydecay482 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx483 = getelementptr inbounds i32* %arraydecay482, i32 4 ; <i32*> [#uses=1]
  store i32 %call481, i32* %arrayidx483
  %arraydecay484 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx485 = getelementptr inbounds i32* %arraydecay484, i32 5 ; <i32*> [#uses=1]
  %tmp486 = load i32* %arrayidx485                ; <i32> [#uses=1]
  %arraydecay487 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx488 = getelementptr inbounds i32* %arraydecay487, i32 7 ; <i32*> [#uses=1]
  %tmp489 = load i32* %arrayidx488                ; <i32> [#uses=1]
  %call490 = call i32 @_Z3minjj(i32 %tmp486, i32 %tmp489) ; <i32> [#uses=1]
  %arraydecay491 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx492 = getelementptr inbounds i32* %arraydecay491, i32 5 ; <i32*> [#uses=1]
  store i32 %call490, i32* %arrayidx492
  %arraydecay493 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx494 = getelementptr inbounds i32* %arraydecay493, i32 4 ; <i32*> [#uses=1]
  %tmp495 = load i32* %arrayidx494                ; <i32> [#uses=1]
  %arraydecay496 = getelementptr inbounds [9 x i32]* %pixels, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx497 = getelementptr inbounds i32* %arraydecay496, i32 5 ; <i32*> [#uses=1]
  %tmp498 = load i32* %arrayidx497                ; <i32> [#uses=1]
  %call499 = call i32 @_Z3minjj(i32 %tmp495, i32 %tmp498) ; <i32> [#uses=1]
  %tmp500 = load i32* %res                        ; <i32> [#uses=1]
  %or = or i32 %tmp500, %call499                  ; <i32> [#uses=1]
  store i32 %or, i32* %res
  %tmp501 = load i32* %mask                       ; <i32> [#uses=1]
  %shl = shl i32 %tmp501, 8                       ; <i32> [#uses=1]
  store i32 %shl, i32* %mask
  br label %for.inc502

for.inc502:                                       ; preds = %for.end
  %tmp503 = load i32* %c                          ; <i32> [#uses=1]
  %inc504 = add nsw i32 %tmp503, 1                ; <i32> [#uses=1]
  store i32 %inc504, i32* %c
  br label %for.cond99

for.end505:                                       ; preds = %for.cond99
  %tmp506 = load i32* %res                        ; <i32> [#uses=1]
  %tmp507 = load i32* %of                         ; <i32> [#uses=1]
  %tmp508 = load i32* %x                          ; <i32> [#uses=1]
  %add509 = add nsw i32 %tmp507, %tmp508          ; <i32> [#uses=1]
  %tmp510 = load i32 addrspace(1)** %uc4Dest.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx511 = getelementptr inbounds i32 addrspace(1)* %tmp510, i32 %add509 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp506, i32 addrspace(1)* %arrayidx511
  br label %for.inc512

for.inc512:                                       ; preds = %for.end505
  %tmp513 = load i32* %x                          ; <i32> [#uses=1]
  %inc514 = add nsw i32 %tmp513, 1                ; <i32> [#uses=1]
  store i32 %inc514, i32* %x
  br label %for.cond

for.end515:                                       ; preds = %for.cond
  ret void
}

declare i32 @_Z3minjj(i32, i32)

declare i32 @_Z3maxjj(i32, i32)
