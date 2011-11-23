; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlMersenneTwister.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"1002\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (<4 x i32> addrspace(1)*, i32, i32, <4 x float> addrspace(1)*, ...)* @gaussianRand to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @lshift128
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %LeafBlock55, %LeafBlock57
; CHECK: phi-split-bb65:                                   ; preds = %LeafBlock51, %phi-split-bb
; CHECK: phi-split-bb66:                                   ; preds = %LeafBlock49, %phi-split-bb65
; CHECK: phi-split-bb67:                                   ; preds = %LeafBlock43, %phi-split-bb66
; CHECK: phi-split-bb68:                                   ; preds = %LeafBlock41, %phi-split-bb67
; CHECK: phi-split-bb69:                                   ; preds = %LeafBlock39, %phi-split-bb68
; CHECK: phi-split-bb70:                                   ; preds = %sw.bb128, %sw.bb139
; CHECK: phi-split-bb74:                                   ; preds = %sw.bb110, %sw.bb117
; CHECK: phi-split-bb79:                                   ; preds = %phi-split-bb70, %phi-split-bb74
; CHECK: phi-split-bb84:                                   ; preds = %sw.bb96, %sw.bb103
; CHECK: phi-split-bb89:                                   ; preds = %sw.bb, %sw.bb90
; CHECK: phi-split-bb94:                                   ; preds = %phi-split-bb84, %phi-split-bb89
; CHECK: phi-split-bb99:                                   ; preds = %phi-split-bb79, %phi-split-bb94
; CHECK: phi-split-bb104:                                  ; preds = %for.end, %for.end328.loopexit
; CHECK: ret

define void @lshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
	%input.addr = alloca <4 x i32>		; <<4 x i32>*> [#uses=8]
	%shift.addr = alloca i32		; <i32*> [#uses=6]
	%output.addr = alloca <4 x i32>*		; <<4 x i32>**> [#uses=2]
	%invshift = alloca i32, align 4		; <i32*> [#uses=4]
	%temp = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=9]
	store <4 x i32> %input, <4 x i32>* %input.addr
	store i32 %shift, i32* %shift.addr
	store <4 x i32>* %output, <4 x i32>** %output.addr
	%tmp = load i32* %shift.addr		; <i32> [#uses=1]
	%sub = sub i32 32, %tmp		; <i32> [#uses=1]
	store i32 %sub, i32* %invshift
	%tmp2 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp3 = extractelement <4 x i32> %tmp2, i32 0		; <i32> [#uses=1]
	%tmp4 = load i32* %shift.addr		; <i32> [#uses=1]
	%and = and i32 %tmp4, 31		; <i32> [#uses=1]
	%shl = shl i32 %tmp3, %and		; <i32> [#uses=1]
	%tmp5 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp6 = insertelement <4 x i32> %tmp5, i32 %shl, i32 0		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp6, <4 x i32>* %temp
	%tmp7 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp8 = extractelement <4 x i32> %tmp7, i32 1		; <i32> [#uses=1]
	%tmp9 = load i32* %shift.addr		; <i32> [#uses=1]
	%and10 = and i32 %tmp9, 31		; <i32> [#uses=1]
	%shl11 = shl i32 %tmp8, %and10		; <i32> [#uses=1]
	%tmp12 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp13 = extractelement <4 x i32> %tmp12, i32 0		; <i32> [#uses=1]
	%tmp14 = load i32* %invshift		; <i32> [#uses=1]
	%and15 = and i32 %tmp14, 31		; <i32> [#uses=1]
	%shr = lshr i32 %tmp13, %and15		; <i32> [#uses=1]
	%or = or i32 %shl11, %shr		; <i32> [#uses=1]
	%tmp16 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp17 = insertelement <4 x i32> %tmp16, i32 %or, i32 1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp17, <4 x i32>* %temp
	%tmp18 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp19 = extractelement <4 x i32> %tmp18, i32 2		; <i32> [#uses=1]
	%tmp20 = load i32* %shift.addr		; <i32> [#uses=1]
	%and21 = and i32 %tmp20, 31		; <i32> [#uses=1]
	%shl22 = shl i32 %tmp19, %and21		; <i32> [#uses=1]
	%tmp23 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp24 = extractelement <4 x i32> %tmp23, i32 1		; <i32> [#uses=1]
	%tmp25 = load i32* %invshift		; <i32> [#uses=1]
	%and26 = and i32 %tmp25, 31		; <i32> [#uses=1]
	%shr27 = lshr i32 %tmp24, %and26		; <i32> [#uses=1]
	%or28 = or i32 %shl22, %shr27		; <i32> [#uses=1]
	%tmp29 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp30 = insertelement <4 x i32> %tmp29, i32 %or28, i32 2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp30, <4 x i32>* %temp
	%tmp31 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp32 = extractelement <4 x i32> %tmp31, i32 3		; <i32> [#uses=1]
	%tmp33 = load i32* %shift.addr		; <i32> [#uses=1]
	%and34 = and i32 %tmp33, 31		; <i32> [#uses=1]
	%shl35 = shl i32 %tmp32, %and34		; <i32> [#uses=1]
	%tmp36 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp37 = extractelement <4 x i32> %tmp36, i32 2		; <i32> [#uses=1]
	%tmp38 = load i32* %invshift		; <i32> [#uses=1]
	%and39 = and i32 %tmp38, 31		; <i32> [#uses=1]
	%shr40 = lshr i32 %tmp37, %and39		; <i32> [#uses=1]
	%or41 = or i32 %shl35, %shr40		; <i32> [#uses=1]
	%tmp42 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp43 = insertelement <4 x i32> %tmp42, i32 %or41, i32 3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp43, <4 x i32>* %temp
	%tmp44 = load <4 x i32>** %output.addr		; <<4 x i32>*> [#uses=1]
	%tmp45 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp45, <4 x i32>* %tmp44
	ret void
}

define void @rshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
	%input.addr = alloca <4 x i32>		; <<4 x i32>*> [#uses=8]
	%shift.addr = alloca i32		; <i32*> [#uses=6]
	%output.addr = alloca <4 x i32>*		; <<4 x i32>**> [#uses=2]
	%invshift = alloca i32, align 4		; <i32*> [#uses=4]
	%temp = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=9]
	store <4 x i32> %input, <4 x i32>* %input.addr
	store i32 %shift, i32* %shift.addr
	store <4 x i32>* %output, <4 x i32>** %output.addr
	%tmp = load i32* %shift.addr		; <i32> [#uses=1]
	%sub = sub i32 32, %tmp		; <i32> [#uses=1]
	store i32 %sub, i32* %invshift
	%tmp2 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp3 = extractelement <4 x i32> %tmp2, i32 3		; <i32> [#uses=1]
	%tmp4 = load i32* %shift.addr		; <i32> [#uses=1]
	%and = and i32 %tmp4, 31		; <i32> [#uses=1]
	%shr = lshr i32 %tmp3, %and		; <i32> [#uses=1]
	%tmp5 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp6 = insertelement <4 x i32> %tmp5, i32 %shr, i32 3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp6, <4 x i32>* %temp
	%tmp7 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp8 = extractelement <4 x i32> %tmp7, i32 2		; <i32> [#uses=1]
	%tmp9 = load i32* %shift.addr		; <i32> [#uses=1]
	%and10 = and i32 %tmp9, 31		; <i32> [#uses=1]
	%shr11 = lshr i32 %tmp8, %and10		; <i32> [#uses=1]
	%tmp12 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp13 = extractelement <4 x i32> %tmp12, i32 3		; <i32> [#uses=1]
	%tmp14 = load i32* %invshift		; <i32> [#uses=1]
	%and15 = and i32 %tmp14, 31		; <i32> [#uses=1]
	%shl = shl i32 %tmp13, %and15		; <i32> [#uses=1]
	%or = or i32 %shr11, %shl		; <i32> [#uses=1]
	%tmp16 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp17 = insertelement <4 x i32> %tmp16, i32 %or, i32 2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp17, <4 x i32>* %temp
	%tmp18 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp19 = extractelement <4 x i32> %tmp18, i32 1		; <i32> [#uses=1]
	%tmp20 = load i32* %shift.addr		; <i32> [#uses=1]
	%and21 = and i32 %tmp20, 31		; <i32> [#uses=1]
	%shr22 = lshr i32 %tmp19, %and21		; <i32> [#uses=1]
	%tmp23 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp24 = extractelement <4 x i32> %tmp23, i32 2		; <i32> [#uses=1]
	%tmp25 = load i32* %invshift		; <i32> [#uses=1]
	%and26 = and i32 %tmp25, 31		; <i32> [#uses=1]
	%shl27 = shl i32 %tmp24, %and26		; <i32> [#uses=1]
	%or28 = or i32 %shr22, %shl27		; <i32> [#uses=1]
	%tmp29 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp30 = insertelement <4 x i32> %tmp29, i32 %or28, i32 1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp30, <4 x i32>* %temp
	%tmp31 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp32 = extractelement <4 x i32> %tmp31, i32 0		; <i32> [#uses=1]
	%tmp33 = load i32* %shift.addr		; <i32> [#uses=1]
	%and34 = and i32 %tmp33, 31		; <i32> [#uses=1]
	%shr35 = lshr i32 %tmp32, %and34		; <i32> [#uses=1]
	%tmp36 = load <4 x i32>* %input.addr		; <<4 x i32>> [#uses=1]
	%tmp37 = extractelement <4 x i32> %tmp36, i32 1		; <i32> [#uses=1]
	%tmp38 = load i32* %invshift		; <i32> [#uses=1]
	%and39 = and i32 %tmp38, 31		; <i32> [#uses=1]
	%shl40 = shl i32 %tmp37, %and39		; <i32> [#uses=1]
	%or41 = or i32 %shr35, %shl40		; <i32> [#uses=1]
	%tmp42 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	%tmp43 = insertelement <4 x i32> %tmp42, i32 %or41, i32 0		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp43, <4 x i32>* %temp
	%tmp44 = load <4 x i32>** %output.addr		; <<4 x i32>*> [#uses=1]
	%tmp45 = load <4 x i32>* %temp		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp45, <4 x i32>* %tmp44
	ret void
}

define void @gaussianRand(<4 x i32> addrspace(1)* %seedArray, i32 %width, i32 %mulFactor, <4 x float> addrspace(1)* %gaussianRand, ...) nounwind {
entry:
	%seedArray.addr = alloca <4 x i32> addrspace(1)*		; <<4 x i32> addrspace(1)**> [#uses=2]
	%width.addr = alloca i32		; <i32*> [#uses=3]
	%mulFactor.addr = alloca i32		; <i32*> [#uses=4]
	%gaussianRand.addr = alloca <4 x float> addrspace(1)*		; <<4 x float> addrspace(1)**> [#uses=3]
	%temp = alloca [8 x <4 x i32>], align 16		; <[8 x <4 x i32>]*> [#uses=19]
	%xPid = alloca i32, align 4		; <i32*> [#uses=3]
	%yPid = alloca i32, align 4		; <i32*> [#uses=3]
	%state1 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%state2 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%state3 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%state4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=7]
	%state5 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%stateMask = alloca i32, align 4		; <i32*> [#uses=2]
	%thirty = alloca i32, align 4		; <i32*> [#uses=2]
	%mask4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%thirty4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%one4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%two4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%three4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%four4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%r1 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=10]
	%r2 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=20]
	%a = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=14]
	%b = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=13]
	%e = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%f = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%thirteen = alloca i32, align 4		; <i32*> [#uses=5]
	%fifteen = alloca i32, align 4		; <i32*> [#uses=5]
	%shift = alloca i32, align 4		; <i32*> [#uses=3]
	%mask11 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask12 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask13 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask14 = alloca i32, align 4		; <i32*> [#uses=2]
	%actualPos = alloca i32, align 4		; <i32*> [#uses=4]
	%one = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%intMax = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%PI = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%two = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%r = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%phi = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%temp1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%temp2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%i = alloca i32, align 4		; <i32*> [#uses=18]
	store <4 x i32> addrspace(1)* %seedArray, <4 x i32> addrspace(1)** %seedArray.addr
	store i32 %width, i32* %width.addr
	store i32 %mulFactor, i32* %mulFactor.addr
	store <4 x float> addrspace(1)* %gaussianRand, <4 x float> addrspace(1)** %gaussianRand.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %xPid
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %yPid
	%tmp = load i32* %yPid		; <i32> [#uses=1]
	%tmp2 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp2		; <i32> [#uses=1]
	%tmp3 = load i32* %xPid		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp3		; <i32> [#uses=1]
	%tmp4 = load <4 x i32> addrspace(1)** %seedArray.addr		; <<4 x i32> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <4 x i32> addrspace(1)* %tmp4, i32 %add		; <<4 x i32> addrspace(1)*> [#uses=1]
	%tmp5 = load <4 x i32> addrspace(1)* %arrayidx		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp5, <4 x i32>* %state1
	store <4 x i32> zeroinitializer, <4 x i32>* %state2
	store <4 x i32> zeroinitializer, <4 x i32>* %state3
	store <4 x i32> zeroinitializer, <4 x i32>* %state4
	store <4 x i32> zeroinitializer, <4 x i32>* %state5
	store i32 1812433253, i32* %stateMask
	store i32 30, i32* %thirty
	%tmp13 = load i32* %stateMask		; <i32> [#uses=1]
	%tmp14 = insertelement <4 x i32> undef, i32 %tmp13, i32 0		; <<4 x i32>> [#uses=2]
	%splat = shufflevector <4 x i32> %tmp14, <4 x i32> %tmp14, <4 x i32> zeroinitializer		; <<4 x i32>> [#uses=1]
	store <4 x i32> %splat, <4 x i32>* %mask4
	%tmp16 = load i32* %thirty		; <i32> [#uses=1]
	%tmp17 = insertelement <4 x i32> undef, i32 %tmp16, i32 0		; <<4 x i32>> [#uses=2]
	%splat18 = shufflevector <4 x i32> %tmp17, <4 x i32> %tmp17, <4 x i32> zeroinitializer		; <<4 x i32>> [#uses=1]
	store <4 x i32> %splat18, <4 x i32>* %thirty4
	store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %one4
	store <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32>* %two4
	store <4 x i32> <i32 3, i32 3, i32 3, i32 3>, <4 x i32>* %three4
	store <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32>* %four4
	store <4 x i32> zeroinitializer, <4 x i32>* %r1
	store <4 x i32> zeroinitializer, <4 x i32>* %r2
	store <4 x i32> zeroinitializer, <4 x i32>* %a
	store <4 x i32> zeroinitializer, <4 x i32>* %b
	store <4 x i32> zeroinitializer, <4 x i32>* %e
	store <4 x i32> zeroinitializer, <4 x i32>* %f
	store i32 13, i32* %thirteen
	store i32 15, i32* %fifteen
	store i32 24, i32* %shift
	store i32 -33605633, i32* %mask11
	store i32 -276873347, i32* %mask12
	store i32 -8946819, i32* %mask13
	store i32 2146958127, i32* %mask14
	store i32 0, i32* %actualPos
	store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %one
	store <4 x float> <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>, <4 x float>* %intMax
	store <4 x float> <float 0x400921FB60000000, float 0x400921FB60000000, float 0x400921FB60000000, float 0x400921FB60000000>, <4 x float>* %PI
	store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %two
	%tmp45 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp46 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	%tmp47 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	%tmp48 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and = and <4 x i32> %tmp48, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr = lshr <4 x i32> %tmp47, %and		; <<4 x i32>> [#uses=1]
	%xor = xor <4 x i32> %tmp46, %shr		; <<4 x i32>> [#uses=1]
	%mul49 = mul <4 x i32> %tmp45, %xor		; <<4 x i32>> [#uses=1]
	%tmp50 = load <4 x i32>* %one4		; <<4 x i32>> [#uses=1]
	%add51 = add <4 x i32> %mul49, %tmp50		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add51, <4 x i32>* %state2
	%tmp52 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp53 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	%tmp54 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	%tmp55 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and56 = and <4 x i32> %tmp55, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr57 = lshr <4 x i32> %tmp54, %and56		; <<4 x i32>> [#uses=1]
	%xor58 = xor <4 x i32> %tmp53, %shr57		; <<4 x i32>> [#uses=1]
	%mul59 = mul <4 x i32> %tmp52, %xor58		; <<4 x i32>> [#uses=1]
	%tmp60 = load <4 x i32>* %two4		; <<4 x i32>> [#uses=1]
	%add61 = add <4 x i32> %mul59, %tmp60		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add61, <4 x i32>* %state3
	%tmp62 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp63 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	%tmp64 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	%tmp65 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and66 = and <4 x i32> %tmp65, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr67 = lshr <4 x i32> %tmp64, %and66		; <<4 x i32>> [#uses=1]
	%xor68 = xor <4 x i32> %tmp63, %shr67		; <<4 x i32>> [#uses=1]
	%mul69 = mul <4 x i32> %tmp62, %xor68		; <<4 x i32>> [#uses=1]
	%tmp70 = load <4 x i32>* %three4		; <<4 x i32>> [#uses=1]
	%add71 = add <4 x i32> %mul69, %tmp70		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add71, <4 x i32>* %state4
	%tmp72 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp73 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	%tmp74 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	%tmp75 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and76 = and <4 x i32> %tmp75, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr77 = lshr <4 x i32> %tmp74, %and76		; <<4 x i32>> [#uses=1]
	%xor78 = xor <4 x i32> %tmp73, %shr77		; <<4 x i32>> [#uses=1]
	%mul79 = mul <4 x i32> %tmp72, %xor78		; <<4 x i32>> [#uses=1]
	%tmp80 = load <4 x i32>* %four4		; <<4 x i32>> [#uses=1]
	%add81 = add <4 x i32> %mul79, %tmp80		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add81, <4 x i32>* %state5
	store i32 0, i32* %i
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp83 = load i32* %i		; <i32> [#uses=1]
	%tmp84 = load i32* %mulFactor.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp83, %tmp84		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp85 = load i32* %i		; <i32> [#uses=1]
	switch i32 %tmp85, label %sw.default [
		i32 0, label %sw.bb
		i32 1, label %sw.bb90
		i32 2, label %sw.bb96
		i32 3, label %sw.bb103
		i32 4, label %sw.bb110
		i32 5, label %sw.bb117
		i32 6, label %sw.bb128
		i32 7, label %sw.bb139
	]

sw.bb:		; preds = %for.body
	%tmp86 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp86, <4 x i32>* %r1
	%tmp87 = load <4 x i32>* %state5		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp87, <4 x i32>* %r2
	%tmp88 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp88, <4 x i32>* %a
	%tmp89 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp89, <4 x i32>* %b
	br label %sw.epilog

sw.bb90:		; preds = %for.body
	%tmp91 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp91, <4 x i32>* %r1
	%arraydecay = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx92 = getelementptr <4 x i32>* %arraydecay, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp93 = load <4 x i32>* %arrayidx92		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp93, <4 x i32>* %r2
	%tmp94 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp94, <4 x i32>* %a
	%tmp95 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp95, <4 x i32>* %b
	br label %sw.epilog

sw.bb96:		; preds = %for.body
	%tmp97 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp97, <4 x i32>* %r1
	%arraydecay98 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx99 = getelementptr <4 x i32>* %arraydecay98, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp100 = load <4 x i32>* %arrayidx99		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp100, <4 x i32>* %r2
	%tmp101 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp101, <4 x i32>* %a
	%tmp102 = load <4 x i32>* %state5		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp102, <4 x i32>* %b
	br label %sw.epilog

sw.bb103:		; preds = %for.body
	%tmp104 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp104, <4 x i32>* %r1
	%arraydecay105 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx106 = getelementptr <4 x i32>* %arraydecay105, i32 2		; <<4 x i32>*> [#uses=1]
	%tmp107 = load <4 x i32>* %arrayidx106		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp107, <4 x i32>* %r2
	%tmp108 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp108, <4 x i32>* %a
	%tmp109 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp109, <4 x i32>* %b
	br label %sw.epilog

sw.bb110:		; preds = %for.body
	%tmp111 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp111, <4 x i32>* %r1
	%arraydecay112 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx113 = getelementptr <4 x i32>* %arraydecay112, i32 3		; <<4 x i32>*> [#uses=1]
	%tmp114 = load <4 x i32>* %arrayidx113		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp114, <4 x i32>* %r2
	%tmp115 = load <4 x i32>* %state5		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp115, <4 x i32>* %a
	%tmp116 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp116, <4 x i32>* %b
	br label %sw.epilog

sw.bb117:		; preds = %for.body
	%tmp118 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp118, <4 x i32>* %r1
	%arraydecay119 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx120 = getelementptr <4 x i32>* %arraydecay119, i32 4		; <<4 x i32>*> [#uses=1]
	%tmp121 = load <4 x i32>* %arrayidx120		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp121, <4 x i32>* %r2
	%arraydecay122 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx123 = getelementptr <4 x i32>* %arraydecay122, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp124 = load <4 x i32>* %arrayidx123		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp124, <4 x i32>* %a
	%arraydecay125 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx126 = getelementptr <4 x i32>* %arraydecay125, i32 2		; <<4 x i32>*> [#uses=1]
	%tmp127 = load <4 x i32>* %arrayidx126		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp127, <4 x i32>* %b
	br label %sw.epilog

sw.bb128:		; preds = %for.body
	%tmp129 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp129, <4 x i32>* %r1
	%arraydecay130 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx131 = getelementptr <4 x i32>* %arraydecay130, i32 5		; <<4 x i32>*> [#uses=1]
	%tmp132 = load <4 x i32>* %arrayidx131		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp132, <4 x i32>* %r2
	%arraydecay133 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx134 = getelementptr <4 x i32>* %arraydecay133, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp135 = load <4 x i32>* %arrayidx134		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp135, <4 x i32>* %a
	%arraydecay136 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx137 = getelementptr <4 x i32>* %arraydecay136, i32 3		; <<4 x i32>*> [#uses=1]
	%tmp138 = load <4 x i32>* %arrayidx137		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp138, <4 x i32>* %b
	br label %sw.epilog

sw.bb139:		; preds = %for.body
	%tmp140 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp140, <4 x i32>* %r1
	%arraydecay141 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx142 = getelementptr <4 x i32>* %arraydecay141, i32 6		; <<4 x i32>*> [#uses=1]
	%tmp143 = load <4 x i32>* %arrayidx142		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp143, <4 x i32>* %r2
	%arraydecay144 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx145 = getelementptr <4 x i32>* %arraydecay144, i32 2		; <<4 x i32>*> [#uses=1]
	%tmp146 = load <4 x i32>* %arrayidx145		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp146, <4 x i32>* %a
	%arraydecay147 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx148 = getelementptr <4 x i32>* %arraydecay147, i32 4		; <<4 x i32>*> [#uses=1]
	%tmp149 = load <4 x i32>* %arrayidx148		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp149, <4 x i32>* %b
	br label %sw.epilog

sw.default:		; preds = %for.body
	br label %sw.epilog

sw.epilog:		; preds = %sw.default, %sw.bb139, %sw.bb128, %sw.bb117, %sw.bb110, %sw.bb103, %sw.bb96, %sw.bb90, %sw.bb
	%tmp150 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp151 = load i32* %shift		; <i32> [#uses=1]
	call void @lshift128(<4 x i32> %tmp150, i32 %tmp151, <4 x i32>* %e)
	%tmp152 = load <4 x i32>* %r1		; <<4 x i32>> [#uses=1]
	%tmp153 = load i32* %shift		; <i32> [#uses=1]
	call void @rshift128(<4 x i32> %tmp152, i32 %tmp153, <4 x i32>* %f)
	%tmp154 = load i32* %i		; <i32> [#uses=1]
	%arraydecay155 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx156 = getelementptr <4 x i32>* %arraydecay155, i32 %tmp154		; <<4 x i32>*> [#uses=2]
	%tmp157 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp158 = extractelement <4 x i32> %tmp157, i32 0		; <i32> [#uses=1]
	%tmp159 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp160 = extractelement <4 x i32> %tmp159, i32 0		; <i32> [#uses=1]
	%xor161 = xor i32 %tmp158, %tmp160		; <i32> [#uses=1]
	%tmp162 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp163 = extractelement <4 x i32> %tmp162, i32 0		; <i32> [#uses=1]
	%tmp164 = load i32* %thirteen		; <i32> [#uses=1]
	%and165 = and i32 %tmp164, 31		; <i32> [#uses=1]
	%shr166 = lshr i32 %tmp163, %and165		; <i32> [#uses=1]
	%tmp167 = load i32* %mask11		; <i32> [#uses=1]
	%and168 = and i32 %shr166, %tmp167		; <i32> [#uses=1]
	%xor169 = xor i32 %xor161, %and168		; <i32> [#uses=1]
	%tmp170 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp171 = extractelement <4 x i32> %tmp170, i32 0		; <i32> [#uses=1]
	%xor172 = xor i32 %xor169, %tmp171		; <i32> [#uses=1]
	%tmp173 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp174 = extractelement <4 x i32> %tmp173, i32 0		; <i32> [#uses=1]
	%tmp175 = load i32* %fifteen		; <i32> [#uses=1]
	%and176 = and i32 %tmp175, 31		; <i32> [#uses=1]
	%shl = shl i32 %tmp174, %and176		; <i32> [#uses=1]
	%xor177 = xor i32 %xor172, %shl		; <i32> [#uses=1]
	%tmp178 = load <4 x i32>* %arrayidx156		; <<4 x i32>> [#uses=1]
	%tmp179 = insertelement <4 x i32> %tmp178, i32 %xor177, i32 0		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp179, <4 x i32>* %arrayidx156
	%tmp180 = load i32* %i		; <i32> [#uses=1]
	%arraydecay181 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx182 = getelementptr <4 x i32>* %arraydecay181, i32 %tmp180		; <<4 x i32>*> [#uses=2]
	%tmp183 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp184 = extractelement <4 x i32> %tmp183, i32 1		; <i32> [#uses=1]
	%tmp185 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp186 = extractelement <4 x i32> %tmp185, i32 1		; <i32> [#uses=1]
	%xor187 = xor i32 %tmp184, %tmp186		; <i32> [#uses=1]
	%tmp188 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp189 = extractelement <4 x i32> %tmp188, i32 1		; <i32> [#uses=1]
	%tmp190 = load i32* %thirteen		; <i32> [#uses=1]
	%and191 = and i32 %tmp190, 31		; <i32> [#uses=1]
	%shr192 = lshr i32 %tmp189, %and191		; <i32> [#uses=1]
	%tmp193 = load i32* %mask12		; <i32> [#uses=1]
	%and194 = and i32 %shr192, %tmp193		; <i32> [#uses=1]
	%xor195 = xor i32 %xor187, %and194		; <i32> [#uses=1]
	%tmp196 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp197 = extractelement <4 x i32> %tmp196, i32 1		; <i32> [#uses=1]
	%xor198 = xor i32 %xor195, %tmp197		; <i32> [#uses=1]
	%tmp199 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp200 = extractelement <4 x i32> %tmp199, i32 1		; <i32> [#uses=1]
	%tmp201 = load i32* %fifteen		; <i32> [#uses=1]
	%and202 = and i32 %tmp201, 31		; <i32> [#uses=1]
	%shl203 = shl i32 %tmp200, %and202		; <i32> [#uses=1]
	%xor204 = xor i32 %xor198, %shl203		; <i32> [#uses=1]
	%tmp205 = load <4 x i32>* %arrayidx182		; <<4 x i32>> [#uses=1]
	%tmp206 = insertelement <4 x i32> %tmp205, i32 %xor204, i32 1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp206, <4 x i32>* %arrayidx182
	%tmp207 = load i32* %i		; <i32> [#uses=1]
	%arraydecay208 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx209 = getelementptr <4 x i32>* %arraydecay208, i32 %tmp207		; <<4 x i32>*> [#uses=2]
	%tmp210 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp211 = extractelement <4 x i32> %tmp210, i32 2		; <i32> [#uses=1]
	%tmp212 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp213 = extractelement <4 x i32> %tmp212, i32 2		; <i32> [#uses=1]
	%xor214 = xor i32 %tmp211, %tmp213		; <i32> [#uses=1]
	%tmp215 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp216 = extractelement <4 x i32> %tmp215, i32 2		; <i32> [#uses=1]
	%tmp217 = load i32* %thirteen		; <i32> [#uses=1]
	%and218 = and i32 %tmp217, 31		; <i32> [#uses=1]
	%shr219 = lshr i32 %tmp216, %and218		; <i32> [#uses=1]
	%tmp220 = load i32* %mask13		; <i32> [#uses=1]
	%and221 = and i32 %shr219, %tmp220		; <i32> [#uses=1]
	%xor222 = xor i32 %xor214, %and221		; <i32> [#uses=1]
	%tmp223 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp224 = extractelement <4 x i32> %tmp223, i32 2		; <i32> [#uses=1]
	%xor225 = xor i32 %xor222, %tmp224		; <i32> [#uses=1]
	%tmp226 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp227 = extractelement <4 x i32> %tmp226, i32 2		; <i32> [#uses=1]
	%tmp228 = load i32* %fifteen		; <i32> [#uses=1]
	%and229 = and i32 %tmp228, 31		; <i32> [#uses=1]
	%shl230 = shl i32 %tmp227, %and229		; <i32> [#uses=1]
	%xor231 = xor i32 %xor225, %shl230		; <i32> [#uses=1]
	%tmp232 = load <4 x i32>* %arrayidx209		; <<4 x i32>> [#uses=1]
	%tmp233 = insertelement <4 x i32> %tmp232, i32 %xor231, i32 2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp233, <4 x i32>* %arrayidx209
	%tmp234 = load i32* %i		; <i32> [#uses=1]
	%arraydecay235 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx236 = getelementptr <4 x i32>* %arraydecay235, i32 %tmp234		; <<4 x i32>*> [#uses=2]
	%tmp237 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp238 = extractelement <4 x i32> %tmp237, i32 3		; <i32> [#uses=1]
	%tmp239 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp240 = extractelement <4 x i32> %tmp239, i32 3		; <i32> [#uses=1]
	%xor241 = xor i32 %tmp238, %tmp240		; <i32> [#uses=1]
	%tmp242 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp243 = extractelement <4 x i32> %tmp242, i32 3		; <i32> [#uses=1]
	%tmp244 = load i32* %thirteen		; <i32> [#uses=1]
	%and245 = and i32 %tmp244, 31		; <i32> [#uses=1]
	%shr246 = lshr i32 %tmp243, %and245		; <i32> [#uses=1]
	%tmp247 = load i32* %mask14		; <i32> [#uses=1]
	%and248 = and i32 %shr246, %tmp247		; <i32> [#uses=1]
	%xor249 = xor i32 %xor241, %and248		; <i32> [#uses=1]
	%tmp250 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp251 = extractelement <4 x i32> %tmp250, i32 3		; <i32> [#uses=1]
	%xor252 = xor i32 %xor249, %tmp251		; <i32> [#uses=1]
	%tmp253 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp254 = extractelement <4 x i32> %tmp253, i32 3		; <i32> [#uses=1]
	%tmp255 = load i32* %fifteen		; <i32> [#uses=1]
	%and256 = and i32 %tmp255, 31		; <i32> [#uses=1]
	%shl257 = shl i32 %tmp254, %and256		; <i32> [#uses=1]
	%xor258 = xor i32 %xor252, %shl257		; <i32> [#uses=1]
	%tmp259 = load <4 x i32>* %arrayidx236		; <<4 x i32>> [#uses=1]
	%tmp260 = insertelement <4 x i32> %tmp259, i32 %xor258, i32 3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp260, <4 x i32>* %arrayidx236
	br label %for.inc

for.inc:		; preds = %sw.epilog
	%tmp261 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp261, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp262 = load i32* %yPid		; <i32> [#uses=1]
	%tmp263 = load i32* %width.addr		; <i32> [#uses=1]
	%mul264 = mul i32 %tmp262, %tmp263		; <i32> [#uses=1]
	%tmp265 = load i32* %xPid		; <i32> [#uses=1]
	%add266 = add i32 %mul264, %tmp265		; <i32> [#uses=1]
	%tmp267 = load i32* %mulFactor.addr		; <i32> [#uses=1]
	%mul268 = mul i32 %add266, %tmp267		; <i32> [#uses=1]
	store i32 %mul268, i32* %actualPos
	store i32 0, i32* %i
	br label %for.cond269

for.cond269:		; preds = %for.inc325, %for.end
	%tmp270 = load i32* %i		; <i32> [#uses=1]
	%tmp271 = load i32* %mulFactor.addr		; <i32> [#uses=1]
	%div = udiv i32 %tmp271, 2		; <i32> [#uses=1]
	%cmp272 = icmp ult i32 %tmp270, %div		; <i1> [#uses=1]
	br i1 %cmp272, label %for.body273, label %for.end328

for.body273:		; preds = %for.cond269
	%tmp274 = load i32* %i		; <i32> [#uses=1]
	%arraydecay275 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx276 = getelementptr <4 x i32>* %arraydecay275, i32 %tmp274		; <<4 x i32>*> [#uses=1]
	%tmp277 = load <4 x i32>* %arrayidx276		; <<4 x i32>> [#uses=1]
	%call278 = call <4 x float> @__convert_float4_uint4(<4 x i32> %tmp277)		; <<4 x float>> [#uses=1]
	%tmp279 = load <4 x float>* %one		; <<4 x float>> [#uses=1]
	%mul280 = fmul <4 x float> %call278, %tmp279		; <<4 x float>> [#uses=1]
	%tmp281 = load <4 x float>* %intMax		; <<4 x float>> [#uses=1]
	%div282 = fdiv <4 x float> %mul280, %tmp281		; <<4 x float>> [#uses=1]
	store <4 x float> %div282, <4 x float>* %temp1
	%tmp283 = load i32* %i		; <i32> [#uses=1]
	%add284 = add i32 %tmp283, 1		; <i32> [#uses=1]
	%arraydecay285 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx286 = getelementptr <4 x i32>* %arraydecay285, i32 %add284		; <<4 x i32>*> [#uses=1]
	%tmp287 = load <4 x i32>* %arrayidx286		; <<4 x i32>> [#uses=1]
	%call288 = call <4 x float> @__convert_float4_uint4(<4 x i32> %tmp287)		; <<4 x float>> [#uses=1]
	%tmp289 = load <4 x float>* %one		; <<4 x float>> [#uses=1]
	%mul290 = fmul <4 x float> %call288, %tmp289		; <<4 x float>> [#uses=1]
	%tmp291 = load <4 x float>* %intMax		; <<4 x float>> [#uses=1]
	%div292 = fdiv <4 x float> %mul290, %tmp291		; <<4 x float>> [#uses=1]
	store <4 x float> %div292, <4 x float>* %temp2
	%tmp293 = load <4 x float>* %two		; <<4 x float>> [#uses=1]
	%neg = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp293		; <<4 x float>> [#uses=1]
	%tmp294 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%call295 = call <4 x float> @__logf4(<4 x float> %tmp294)		; <<4 x float>> [#uses=1]
	%mul296 = fmul <4 x float> %neg, %call295		; <<4 x float>> [#uses=1]
	%call297 = call <4 x float> @__sqrtf4(<4 x float> %mul296)		; <<4 x float>> [#uses=1]
	store <4 x float> %call297, <4 x float>* %r
	%tmp298 = load <4 x float>* %two		; <<4 x float>> [#uses=1]
	%tmp299 = load <4 x float>* %PI		; <<4 x float>> [#uses=1]
	%mul300 = fmul <4 x float> %tmp298, %tmp299		; <<4 x float>> [#uses=1]
	%tmp301 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%mul302 = fmul <4 x float> %mul300, %tmp301		; <<4 x float>> [#uses=1]
	store <4 x float> %mul302, <4 x float>* %phi
	%tmp303 = load i32* %actualPos		; <i32> [#uses=1]
	%tmp304 = load i32* %i		; <i32> [#uses=1]
	%mul305 = mul i32 %tmp304, 2		; <i32> [#uses=1]
	%add306 = add i32 %tmp303, %mul305		; <i32> [#uses=1]
	%add307 = add i32 %add306, 0		; <i32> [#uses=1]
	%tmp308 = load <4 x float> addrspace(1)** %gaussianRand.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx309 = getelementptr <4 x float> addrspace(1)* %tmp308, i32 %add307		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp310 = load <4 x float>* %r		; <<4 x float>> [#uses=1]
	%tmp311 = load <4 x float>* %phi		; <<4 x float>> [#uses=1]
	%call312 = call <4 x float> @__cosf4(<4 x float> %tmp311)		; <<4 x float>> [#uses=1]
	%mul313 = fmul <4 x float> %tmp310, %call312		; <<4 x float>> [#uses=1]
	store <4 x float> %mul313, <4 x float> addrspace(1)* %arrayidx309
	%tmp314 = load i32* %actualPos		; <i32> [#uses=1]
	%tmp315 = load i32* %i		; <i32> [#uses=1]
	%mul316 = mul i32 %tmp315, 2		; <i32> [#uses=1]
	%add317 = add i32 %tmp314, %mul316		; <i32> [#uses=1]
	%add318 = add i32 %add317, 1		; <i32> [#uses=1]
	%tmp319 = load <4 x float> addrspace(1)** %gaussianRand.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx320 = getelementptr <4 x float> addrspace(1)* %tmp319, i32 %add318		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp321 = load <4 x float>* %r		; <<4 x float>> [#uses=1]
	%tmp322 = load <4 x float>* %phi		; <<4 x float>> [#uses=1]
	%call323 = call <4 x float> @__sinf4(<4 x float> %tmp322)		; <<4 x float>> [#uses=1]
	%mul324 = fmul <4 x float> %tmp321, %call323		; <<4 x float>> [#uses=1]
	store <4 x float> %mul324, <4 x float> addrspace(1)* %arrayidx320
	br label %for.inc325

for.inc325:		; preds = %for.body273
	%tmp326 = load i32* %i		; <i32> [#uses=1]
	%inc327 = add i32 %tmp326, 1		; <i32> [#uses=1]
	store i32 %inc327, i32* %i
	br label %for.cond269

for.end328:		; preds = %for.cond269
	ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @__convert_float4_uint4(<4 x i32>)

declare <4 x float> @__sqrtf4(<4 x float>)

declare <4 x float> @__logf4(<4 x float>)

declare <4 x float> @__cosf4(<4 x float>)

declare <4 x float> @__sinf4(<4 x float>)
