; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIMonteCarlo.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [7 x i8] c"200222\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (<4 x float> addrspace(1)*, i32, i32, <4 x i32> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, ...)* @calPriceVega to i8*), i8* getelementptr ([7 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @lshift128

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

define void @generateRand(<4 x i32> %seed, <4 x float>* %gaussianRand1, <4 x float>* %gaussianRand2, <4 x i32>* %nextRand) nounwind {
entry:
	%seed.addr = alloca <4 x i32>		; <<4 x i32>*> [#uses=2]
	%gaussianRand1.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%gaussianRand2.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%nextRand.addr = alloca <4 x i32>*		; <<4 x i32>**> [#uses=2]
	%mulFactor = alloca i32, align 4		; <i32*> [#uses=2]
	%temp = alloca [8 x <4 x i32>], align 16		; <[8 x <4 x i32>]*> [#uses=16]
	%state1 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%state2 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%state3 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%state4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=7]
	%state5 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=4]
	%stateMask = alloca i32, align 4		; <i32*> [#uses=2]
	%thirty = alloca i32, align 4		; <i32*> [#uses=2]
	%mask4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%thirty4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=5]
	%one4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%two4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%three4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%four4 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	%r1 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%r2 = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=12]
	%a = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=10]
	%b = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=9]
	%e = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%f = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=6]
	%thirteen = alloca i32, align 4		; <i32*> [#uses=5]
	%fifteen = alloca i32, align 4		; <i32*> [#uses=5]
	%shift = alloca i32, align 4		; <i32*> [#uses=3]
	%mask11 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask12 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask13 = alloca i32, align 4		; <i32*> [#uses=2]
	%mask14 = alloca i32, align 4		; <i32*> [#uses=2]
	%one = alloca float, align 4		; <float*> [#uses=3]
	%intMax = alloca float, align 4		; <float*> [#uses=3]
	%PI = alloca float, align 4		; <float*> [#uses=2]
	%two = alloca float, align 4		; <float*> [#uses=3]
	%r = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%phi = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%temp1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%temp2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%i = alloca i32, align 4		; <i32*> [#uses=10]
	%tmpValue1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%tmpValue2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	store <4 x i32> %seed, <4 x i32>* %seed.addr
	store <4 x float>* %gaussianRand1, <4 x float>** %gaussianRand1.addr
	store <4 x float>* %gaussianRand2, <4 x float>** %gaussianRand2.addr
	store <4 x i32>* %nextRand, <4 x i32>** %nextRand.addr
	store i32 4, i32* %mulFactor
	%tmp = load <4 x i32>* %seed.addr		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp, <4 x i32>* %state1
	store <4 x i32> zeroinitializer, <4 x i32>* %state2
	store <4 x i32> zeroinitializer, <4 x i32>* %state3
	store <4 x i32> zeroinitializer, <4 x i32>* %state4
	store <4 x i32> zeroinitializer, <4 x i32>* %state5
	store i32 1812433253, i32* %stateMask
	store i32 30, i32* %thirty
	%tmp8 = load i32* %stateMask		; <i32> [#uses=1]
	%tmp9 = insertelement <4 x i32> undef, i32 %tmp8, i32 0		; <<4 x i32>> [#uses=2]
	%splat = shufflevector <4 x i32> %tmp9, <4 x i32> %tmp9, <4 x i32> zeroinitializer		; <<4 x i32>> [#uses=1]
	store <4 x i32> %splat, <4 x i32>* %mask4
	%tmp11 = load i32* %thirty		; <i32> [#uses=1]
	%tmp12 = insertelement <4 x i32> undef, i32 %tmp11, i32 0		; <<4 x i32>> [#uses=2]
	%splat13 = shufflevector <4 x i32> %tmp12, <4 x i32> %tmp12, <4 x i32> zeroinitializer		; <<4 x i32>> [#uses=1]
	store <4 x i32> %splat13, <4 x i32>* %thirty4
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
	store float 1.000000e+000, float* %one
	store float 0x41F0000000000000, float* %intMax
	store float 0x400921FB60000000, float* %PI
	store float 2.000000e+000, float* %two
	%tmp39 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp40 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	%tmp41 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	%tmp42 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and = and <4 x i32> %tmp42, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr = lshr <4 x i32> %tmp41, %and		; <<4 x i32>> [#uses=1]
	%xor = xor <4 x i32> %tmp40, %shr		; <<4 x i32>> [#uses=1]
	%mul = mul <4 x i32> %tmp39, %xor		; <<4 x i32>> [#uses=1]
	%tmp43 = load <4 x i32>* %one4		; <<4 x i32>> [#uses=1]
	%add = add <4 x i32> %mul, %tmp43		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add, <4 x i32>* %state2
	%tmp44 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp45 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	%tmp46 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	%tmp47 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and48 = and <4 x i32> %tmp47, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr49 = lshr <4 x i32> %tmp46, %and48		; <<4 x i32>> [#uses=1]
	%xor50 = xor <4 x i32> %tmp45, %shr49		; <<4 x i32>> [#uses=1]
	%mul51 = mul <4 x i32> %tmp44, %xor50		; <<4 x i32>> [#uses=1]
	%tmp52 = load <4 x i32>* %two4		; <<4 x i32>> [#uses=1]
	%add53 = add <4 x i32> %mul51, %tmp52		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add53, <4 x i32>* %state3
	%tmp54 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp55 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	%tmp56 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	%tmp57 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and58 = and <4 x i32> %tmp57, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr59 = lshr <4 x i32> %tmp56, %and58		; <<4 x i32>> [#uses=1]
	%xor60 = xor <4 x i32> %tmp55, %shr59		; <<4 x i32>> [#uses=1]
	%mul61 = mul <4 x i32> %tmp54, %xor60		; <<4 x i32>> [#uses=1]
	%tmp62 = load <4 x i32>* %three4		; <<4 x i32>> [#uses=1]
	%add63 = add <4 x i32> %mul61, %tmp62		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add63, <4 x i32>* %state4
	%tmp64 = load <4 x i32>* %mask4		; <<4 x i32>> [#uses=1]
	%tmp65 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	%tmp66 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	%tmp67 = load <4 x i32>* %thirty4		; <<4 x i32>> [#uses=1]
	%and68 = and <4 x i32> %tmp67, <i32 31, i32 31, i32 31, i32 31>		; <<4 x i32>> [#uses=1]
	%shr69 = lshr <4 x i32> %tmp66, %and68		; <<4 x i32>> [#uses=1]
	%xor70 = xor <4 x i32> %tmp65, %shr69		; <<4 x i32>> [#uses=1]
	%mul71 = mul <4 x i32> %tmp64, %xor70		; <<4 x i32>> [#uses=1]
	%tmp72 = load <4 x i32>* %four4		; <<4 x i32>> [#uses=1]
	%add73 = add <4 x i32> %mul71, %tmp72		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add73, <4 x i32>* %state5
	store i32 0, i32* %i
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp75 = load i32* %i		; <i32> [#uses=1]
	%tmp76 = load i32* %mulFactor		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp75, %tmp76		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp77 = load i32* %i		; <i32> [#uses=1]
	switch i32 %tmp77, label %sw.default [
		i32 0, label %sw.bb
		i32 1, label %sw.bb82
		i32 2, label %sw.bb87
		i32 3, label %sw.bb94
	]

sw.bb:		; preds = %for.body
	%tmp78 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp78, <4 x i32>* %r1
	%tmp79 = load <4 x i32>* %state5		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp79, <4 x i32>* %r2
	%tmp80 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp80, <4 x i32>* %a
	%tmp81 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp81, <4 x i32>* %b
	br label %sw.epilog

sw.bb82:		; preds = %for.body
	%tmp83 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp83, <4 x i32>* %r1
	%arraydecay = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx = getelementptr <4 x i32>* %arraydecay, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp84 = load <4 x i32>* %arrayidx		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp84, <4 x i32>* %r2
	%tmp85 = load <4 x i32>* %state2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp85, <4 x i32>* %a
	%tmp86 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp86, <4 x i32>* %b
	br label %sw.epilog

sw.bb87:		; preds = %for.body
	%tmp88 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp88, <4 x i32>* %r1
	%arraydecay89 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx90 = getelementptr <4 x i32>* %arraydecay89, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp91 = load <4 x i32>* %arrayidx90		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp91, <4 x i32>* %r2
	%tmp92 = load <4 x i32>* %state3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp92, <4 x i32>* %a
	%tmp93 = load <4 x i32>* %state5		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp93, <4 x i32>* %b
	br label %sw.epilog

sw.bb94:		; preds = %for.body
	%tmp95 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp95, <4 x i32>* %r1
	%arraydecay96 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx97 = getelementptr <4 x i32>* %arraydecay96, i32 2		; <<4 x i32>*> [#uses=1]
	%tmp98 = load <4 x i32>* %arrayidx97		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp98, <4 x i32>* %r2
	%tmp99 = load <4 x i32>* %state4		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp99, <4 x i32>* %a
	%tmp100 = load <4 x i32>* %state1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp100, <4 x i32>* %b
	br label %sw.epilog

sw.default:		; preds = %for.body
	br label %sw.epilog

sw.epilog:		; preds = %sw.default, %sw.bb94, %sw.bb87, %sw.bb82, %sw.bb
	%tmp101 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp102 = load i32* %shift		; <i32> [#uses=1]
	call void @lshift128(<4 x i32> %tmp101, i32 %tmp102, <4 x i32>* %e)
	%tmp103 = load <4 x i32>* %r1		; <<4 x i32>> [#uses=1]
	%tmp104 = load i32* %shift		; <i32> [#uses=1]
	call void @rshift128(<4 x i32> %tmp103, i32 %tmp104, <4 x i32>* %f)
	%tmp105 = load i32* %i		; <i32> [#uses=1]
	%arraydecay106 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx107 = getelementptr <4 x i32>* %arraydecay106, i32 %tmp105		; <<4 x i32>*> [#uses=2]
	%tmp108 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp109 = extractelement <4 x i32> %tmp108, i32 0		; <i32> [#uses=1]
	%tmp110 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp111 = extractelement <4 x i32> %tmp110, i32 0		; <i32> [#uses=1]
	%xor112 = xor i32 %tmp109, %tmp111		; <i32> [#uses=1]
	%tmp113 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp114 = extractelement <4 x i32> %tmp113, i32 0		; <i32> [#uses=1]
	%tmp115 = load i32* %thirteen		; <i32> [#uses=1]
	%and116 = and i32 %tmp115, 31		; <i32> [#uses=1]
	%shr117 = lshr i32 %tmp114, %and116		; <i32> [#uses=1]
	%tmp118 = load i32* %mask11		; <i32> [#uses=1]
	%and119 = and i32 %shr117, %tmp118		; <i32> [#uses=1]
	%xor120 = xor i32 %xor112, %and119		; <i32> [#uses=1]
	%tmp121 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp122 = extractelement <4 x i32> %tmp121, i32 0		; <i32> [#uses=1]
	%xor123 = xor i32 %xor120, %tmp122		; <i32> [#uses=1]
	%tmp124 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp125 = extractelement <4 x i32> %tmp124, i32 0		; <i32> [#uses=1]
	%tmp126 = load i32* %fifteen		; <i32> [#uses=1]
	%and127 = and i32 %tmp126, 31		; <i32> [#uses=1]
	%shl = shl i32 %tmp125, %and127		; <i32> [#uses=1]
	%xor128 = xor i32 %xor123, %shl		; <i32> [#uses=1]
	%tmp129 = load <4 x i32>* %arrayidx107		; <<4 x i32>> [#uses=1]
	%tmp130 = insertelement <4 x i32> %tmp129, i32 %xor128, i32 0		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp130, <4 x i32>* %arrayidx107
	%tmp131 = load i32* %i		; <i32> [#uses=1]
	%arraydecay132 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx133 = getelementptr <4 x i32>* %arraydecay132, i32 %tmp131		; <<4 x i32>*> [#uses=2]
	%tmp134 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp135 = extractelement <4 x i32> %tmp134, i32 1		; <i32> [#uses=1]
	%tmp136 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp137 = extractelement <4 x i32> %tmp136, i32 1		; <i32> [#uses=1]
	%xor138 = xor i32 %tmp135, %tmp137		; <i32> [#uses=1]
	%tmp139 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp140 = extractelement <4 x i32> %tmp139, i32 1		; <i32> [#uses=1]
	%tmp141 = load i32* %thirteen		; <i32> [#uses=1]
	%and142 = and i32 %tmp141, 31		; <i32> [#uses=1]
	%shr143 = lshr i32 %tmp140, %and142		; <i32> [#uses=1]
	%tmp144 = load i32* %mask12		; <i32> [#uses=1]
	%and145 = and i32 %shr143, %tmp144		; <i32> [#uses=1]
	%xor146 = xor i32 %xor138, %and145		; <i32> [#uses=1]
	%tmp147 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp148 = extractelement <4 x i32> %tmp147, i32 1		; <i32> [#uses=1]
	%xor149 = xor i32 %xor146, %tmp148		; <i32> [#uses=1]
	%tmp150 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp151 = extractelement <4 x i32> %tmp150, i32 1		; <i32> [#uses=1]
	%tmp152 = load i32* %fifteen		; <i32> [#uses=1]
	%and153 = and i32 %tmp152, 31		; <i32> [#uses=1]
	%shl154 = shl i32 %tmp151, %and153		; <i32> [#uses=1]
	%xor155 = xor i32 %xor149, %shl154		; <i32> [#uses=1]
	%tmp156 = load <4 x i32>* %arrayidx133		; <<4 x i32>> [#uses=1]
	%tmp157 = insertelement <4 x i32> %tmp156, i32 %xor155, i32 1		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp157, <4 x i32>* %arrayidx133
	%tmp158 = load i32* %i		; <i32> [#uses=1]
	%arraydecay159 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx160 = getelementptr <4 x i32>* %arraydecay159, i32 %tmp158		; <<4 x i32>*> [#uses=2]
	%tmp161 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp162 = extractelement <4 x i32> %tmp161, i32 2		; <i32> [#uses=1]
	%tmp163 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp164 = extractelement <4 x i32> %tmp163, i32 2		; <i32> [#uses=1]
	%xor165 = xor i32 %tmp162, %tmp164		; <i32> [#uses=1]
	%tmp166 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp167 = extractelement <4 x i32> %tmp166, i32 2		; <i32> [#uses=1]
	%tmp168 = load i32* %thirteen		; <i32> [#uses=1]
	%and169 = and i32 %tmp168, 31		; <i32> [#uses=1]
	%shr170 = lshr i32 %tmp167, %and169		; <i32> [#uses=1]
	%tmp171 = load i32* %mask13		; <i32> [#uses=1]
	%and172 = and i32 %shr170, %tmp171		; <i32> [#uses=1]
	%xor173 = xor i32 %xor165, %and172		; <i32> [#uses=1]
	%tmp174 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp175 = extractelement <4 x i32> %tmp174, i32 2		; <i32> [#uses=1]
	%xor176 = xor i32 %xor173, %tmp175		; <i32> [#uses=1]
	%tmp177 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp178 = extractelement <4 x i32> %tmp177, i32 2		; <i32> [#uses=1]
	%tmp179 = load i32* %fifteen		; <i32> [#uses=1]
	%and180 = and i32 %tmp179, 31		; <i32> [#uses=1]
	%shl181 = shl i32 %tmp178, %and180		; <i32> [#uses=1]
	%xor182 = xor i32 %xor176, %shl181		; <i32> [#uses=1]
	%tmp183 = load <4 x i32>* %arrayidx160		; <<4 x i32>> [#uses=1]
	%tmp184 = insertelement <4 x i32> %tmp183, i32 %xor182, i32 2		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp184, <4 x i32>* %arrayidx160
	%tmp185 = load i32* %i		; <i32> [#uses=1]
	%arraydecay186 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx187 = getelementptr <4 x i32>* %arraydecay186, i32 %tmp185		; <<4 x i32>*> [#uses=2]
	%tmp188 = load <4 x i32>* %a		; <<4 x i32>> [#uses=1]
	%tmp189 = extractelement <4 x i32> %tmp188, i32 3		; <i32> [#uses=1]
	%tmp190 = load <4 x i32>* %e		; <<4 x i32>> [#uses=1]
	%tmp191 = extractelement <4 x i32> %tmp190, i32 3		; <i32> [#uses=1]
	%xor192 = xor i32 %tmp189, %tmp191		; <i32> [#uses=1]
	%tmp193 = load <4 x i32>* %b		; <<4 x i32>> [#uses=1]
	%tmp194 = extractelement <4 x i32> %tmp193, i32 3		; <i32> [#uses=1]
	%tmp195 = load i32* %thirteen		; <i32> [#uses=1]
	%and196 = and i32 %tmp195, 31		; <i32> [#uses=1]
	%shr197 = lshr i32 %tmp194, %and196		; <i32> [#uses=1]
	%tmp198 = load i32* %mask14		; <i32> [#uses=1]
	%and199 = and i32 %shr197, %tmp198		; <i32> [#uses=1]
	%xor200 = xor i32 %xor192, %and199		; <i32> [#uses=1]
	%tmp201 = load <4 x i32>* %f		; <<4 x i32>> [#uses=1]
	%tmp202 = extractelement <4 x i32> %tmp201, i32 3		; <i32> [#uses=1]
	%xor203 = xor i32 %xor200, %tmp202		; <i32> [#uses=1]
	%tmp204 = load <4 x i32>* %r2		; <<4 x i32>> [#uses=1]
	%tmp205 = extractelement <4 x i32> %tmp204, i32 3		; <i32> [#uses=1]
	%tmp206 = load i32* %fifteen		; <i32> [#uses=1]
	%and207 = and i32 %tmp206, 31		; <i32> [#uses=1]
	%shl208 = shl i32 %tmp205, %and207		; <i32> [#uses=1]
	%xor209 = xor i32 %xor203, %shl208		; <i32> [#uses=1]
	%tmp210 = load <4 x i32>* %arrayidx187		; <<4 x i32>> [#uses=1]
	%tmp211 = insertelement <4 x i32> %tmp210, i32 %xor209, i32 3		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp211, <4 x i32>* %arrayidx187
	br label %for.inc

for.inc:		; preds = %sw.epilog
	%tmp212 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp212, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%arraydecay214 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx215 = getelementptr <4 x i32>* %arraydecay214, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp216 = load <4 x i32>* %arrayidx215		; <<4 x i32>> [#uses=1]
	%tmp217 = extractelement <4 x i32> %tmp216, i32 0		; <i32> [#uses=1]
	%conv = uitofp i32 %tmp217 to float		; <float> [#uses=1]
	%0 = insertelement <4 x float> undef, float %conv, i32 0		; <<4 x float>> [#uses=1]
	%arraydecay218 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx219 = getelementptr <4 x i32>* %arraydecay218, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp220 = load <4 x i32>* %arrayidx219		; <<4 x i32>> [#uses=1]
	%tmp221 = extractelement <4 x i32> %tmp220, i32 1		; <i32> [#uses=1]
	%conv222 = uitofp i32 %tmp221 to float		; <float> [#uses=1]
	%1 = insertelement <4 x float> %0, float %conv222, i32 1		; <<4 x float>> [#uses=1]
	%arraydecay223 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx224 = getelementptr <4 x i32>* %arraydecay223, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp225 = load <4 x i32>* %arrayidx224		; <<4 x i32>> [#uses=1]
	%tmp226 = extractelement <4 x i32> %tmp225, i32 2		; <i32> [#uses=1]
	%conv227 = uitofp i32 %tmp226 to float		; <float> [#uses=1]
	%2 = insertelement <4 x float> %1, float %conv227, i32 2		; <<4 x float>> [#uses=1]
	%arraydecay228 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx229 = getelementptr <4 x i32>* %arraydecay228, i32 0		; <<4 x i32>*> [#uses=1]
	%tmp230 = load <4 x i32>* %arrayidx229		; <<4 x i32>> [#uses=1]
	%tmp231 = extractelement <4 x i32> %tmp230, i32 3		; <i32> [#uses=1]
	%conv232 = uitofp i32 %tmp231 to float		; <float> [#uses=1]
	%3 = insertelement <4 x float> %2, float %conv232, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %3, <4 x float>* %tmpValue1
	%tmp233 = load <4 x float>* %tmpValue1		; <<4 x float>> [#uses=1]
	%tmp234 = load float* %one		; <float> [#uses=1]
	%tmp235 = insertelement <4 x float> undef, float %tmp234, i32 0		; <<4 x float>> [#uses=2]
	%splat236 = shufflevector <4 x float> %tmp235, <4 x float> %tmp235, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%mul237 = fmul <4 x float> %tmp233, %splat236		; <<4 x float>> [#uses=1]
	%tmp238 = load float* %intMax		; <float> [#uses=1]
	%tmp239 = insertelement <4 x float> undef, float %tmp238, i32 0		; <<4 x float>> [#uses=2]
	%splat240 = shufflevector <4 x float> %tmp239, <4 x float> %tmp239, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div = fdiv <4 x float> %mul237, %splat240		; <<4 x float>> [#uses=1]
	store <4 x float> %div, <4 x float>* %temp1
	%arraydecay242 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx243 = getelementptr <4 x i32>* %arraydecay242, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp244 = load <4 x i32>* %arrayidx243		; <<4 x i32>> [#uses=1]
	%tmp245 = extractelement <4 x i32> %tmp244, i32 0		; <i32> [#uses=1]
	%conv246 = uitofp i32 %tmp245 to float		; <float> [#uses=1]
	%4 = insertelement <4 x float> undef, float %conv246, i32 0		; <<4 x float>> [#uses=1]
	%arraydecay247 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx248 = getelementptr <4 x i32>* %arraydecay247, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp249 = load <4 x i32>* %arrayidx248		; <<4 x i32>> [#uses=1]
	%tmp250 = extractelement <4 x i32> %tmp249, i32 1		; <i32> [#uses=1]
	%conv251 = uitofp i32 %tmp250 to float		; <float> [#uses=1]
	%5 = insertelement <4 x float> %4, float %conv251, i32 1		; <<4 x float>> [#uses=1]
	%arraydecay252 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx253 = getelementptr <4 x i32>* %arraydecay252, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp254 = load <4 x i32>* %arrayidx253		; <<4 x i32>> [#uses=1]
	%tmp255 = extractelement <4 x i32> %tmp254, i32 2		; <i32> [#uses=1]
	%conv256 = uitofp i32 %tmp255 to float		; <float> [#uses=1]
	%6 = insertelement <4 x float> %5, float %conv256, i32 2		; <<4 x float>> [#uses=1]
	%arraydecay257 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx258 = getelementptr <4 x i32>* %arraydecay257, i32 1		; <<4 x i32>*> [#uses=1]
	%tmp259 = load <4 x i32>* %arrayidx258		; <<4 x i32>> [#uses=1]
	%tmp260 = extractelement <4 x i32> %tmp259, i32 3		; <i32> [#uses=1]
	%conv261 = uitofp i32 %tmp260 to float		; <float> [#uses=1]
	%7 = insertelement <4 x float> %6, float %conv261, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %7, <4 x float>* %tmpValue2
	%tmp262 = load <4 x float>* %tmpValue2		; <<4 x float>> [#uses=1]
	%tmp263 = load float* %one		; <float> [#uses=1]
	%tmp264 = insertelement <4 x float> undef, float %tmp263, i32 0		; <<4 x float>> [#uses=2]
	%splat265 = shufflevector <4 x float> %tmp264, <4 x float> %tmp264, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%mul266 = fmul <4 x float> %tmp262, %splat265		; <<4 x float>> [#uses=1]
	%tmp267 = load float* %intMax		; <float> [#uses=1]
	%tmp268 = insertelement <4 x float> undef, float %tmp267, i32 0		; <<4 x float>> [#uses=2]
	%splat269 = shufflevector <4 x float> %tmp268, <4 x float> %tmp268, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div270 = fdiv <4 x float> %mul266, %splat269		; <<4 x float>> [#uses=1]
	store <4 x float> %div270, <4 x float>* %temp2
	%tmp271 = load float* %two		; <float> [#uses=1]
	%neg = fsub float -0.000000e+000, %tmp271		; <float> [#uses=1]
	%tmp272 = insertelement <4 x float> undef, float %neg, i32 0		; <<4 x float>> [#uses=2]
	%splat273 = shufflevector <4 x float> %tmp272, <4 x float> %tmp272, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp274 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%call = call <4 x float> @__logf4(<4 x float> %tmp274)		; <<4 x float>> [#uses=1]
	%mul275 = fmul <4 x float> %splat273, %call		; <<4 x float>> [#uses=1]
	%call276 = call <4 x float> @__sqrtf4(<4 x float> %mul275)		; <<4 x float>> [#uses=1]
	store <4 x float> %call276, <4 x float>* %r
	%tmp277 = load float* %two		; <float> [#uses=1]
	%tmp278 = load float* %PI		; <float> [#uses=1]
	%mul279 = fmul float %tmp277, %tmp278		; <float> [#uses=1]
	%tmp280 = insertelement <4 x float> undef, float %mul279, i32 0		; <<4 x float>> [#uses=2]
	%splat281 = shufflevector <4 x float> %tmp280, <4 x float> %tmp280, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp282 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%mul283 = fmul <4 x float> %splat281, %tmp282		; <<4 x float>> [#uses=1]
	store <4 x float> %mul283, <4 x float>* %phi
	%tmp284 = load <4 x float>** %gaussianRand1.addr		; <<4 x float>*> [#uses=1]
	%tmp285 = load <4 x float>* %r		; <<4 x float>> [#uses=1]
	%tmp286 = load <4 x float>* %phi		; <<4 x float>> [#uses=1]
	%call287 = call <4 x float> @__cosf4(<4 x float> %tmp286)		; <<4 x float>> [#uses=1]
	%mul288 = fmul <4 x float> %tmp285, %call287		; <<4 x float>> [#uses=1]
	store <4 x float> %mul288, <4 x float>* %tmp284
	%tmp289 = load <4 x float>** %gaussianRand2.addr		; <<4 x float>*> [#uses=1]
	%tmp290 = load <4 x float>* %r		; <<4 x float>> [#uses=1]
	%tmp291 = load <4 x float>* %phi		; <<4 x float>> [#uses=1]
	%call292 = call <4 x float> @__sinf4(<4 x float> %tmp291)		; <<4 x float>> [#uses=1]
	%mul293 = fmul <4 x float> %tmp290, %call292		; <<4 x float>> [#uses=1]
	store <4 x float> %mul293, <4 x float>* %tmp289
	%tmp294 = load <4 x i32>** %nextRand.addr		; <<4 x i32>*> [#uses=1]
	%arraydecay295 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0		; <<4 x i32>*> [#uses=1]
	%arrayidx296 = getelementptr <4 x i32>* %arraydecay295, i32 2		; <<4 x i32>*> [#uses=1]
	%tmp297 = load <4 x i32>* %arrayidx296		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp297, <4 x i32>* %tmp294
	ret void
}

declare <4 x float> @__sqrtf4(<4 x float>)

declare <4 x float> @__logf4(<4 x float>)

declare <4 x float> @__cosf4(<4 x float>)

declare <4 x float> @__sinf4(<4 x float>)

define void @calOutputs(<4 x float> %strikePrice, <4 x float> %meanDeriv1, <4 x float> %meanDeriv2, <4 x float> %meanPrice1, <4 x float> %meanPrice2, <4 x float>* %pathDeriv1, <4 x float>* %pathDeriv2, <4 x float>* %priceVec1, <4 x float>* %priceVec2) nounwind {
entry:
	%strikePrice.addr = alloca <4 x float>		; <<4 x float>*> [#uses=3]
	%meanDeriv1.addr = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%meanDeriv2.addr = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%meanPrice1.addr = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%meanPrice2.addr = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%pathDeriv1.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%pathDeriv2.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%priceVec1.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%priceVec2.addr = alloca <4 x float>*		; <<4 x float>**> [#uses=2]
	%temp1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=10]
	%temp2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=10]
	%temp3 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=10]
	%temp4 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=10]
	%tempDiff1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=9]
	%tempDiff2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=9]
	store <4 x float> %strikePrice, <4 x float>* %strikePrice.addr
	store <4 x float> %meanDeriv1, <4 x float>* %meanDeriv1.addr
	store <4 x float> %meanDeriv2, <4 x float>* %meanDeriv2.addr
	store <4 x float> %meanPrice1, <4 x float>* %meanPrice1.addr
	store <4 x float> %meanPrice2, <4 x float>* %meanPrice2.addr
	store <4 x float>* %pathDeriv1, <4 x float>** %pathDeriv1.addr
	store <4 x float>* %pathDeriv2, <4 x float>** %pathDeriv2.addr
	store <4 x float>* %priceVec1, <4 x float>** %priceVec1.addr
	store <4 x float>* %priceVec2, <4 x float>** %priceVec2.addr
	store <4 x float> zeroinitializer, <4 x float>* %temp1
	store <4 x float> zeroinitializer, <4 x float>* %temp2
	store <4 x float> zeroinitializer, <4 x float>* %temp3
	store <4 x float> zeroinitializer, <4 x float>* %temp4
	%tmp = load <4 x float>* %meanPrice1.addr		; <<4 x float>> [#uses=1]
	%tmp1 = load <4 x float>* %strikePrice.addr		; <<4 x float>> [#uses=1]
	%sub = fsub <4 x float> %tmp, %tmp1		; <<4 x float>> [#uses=1]
	store <4 x float> %sub, <4 x float>* %tempDiff1
	%tmp3 = load <4 x float>* %meanPrice2.addr		; <<4 x float>> [#uses=1]
	%tmp4 = load <4 x float>* %strikePrice.addr		; <<4 x float>> [#uses=1]
	%sub5 = fsub <4 x float> %tmp3, %tmp4		; <<4 x float>> [#uses=1]
	store <4 x float> %sub5, <4 x float>* %tempDiff2
	%tmp6 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp7 = extractelement <4 x float> %tmp6, i32 0		; <float> [#uses=1]
	%cmp = fcmp ogt float %tmp7, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp, label %if.then, label %if.end

if.then:		; preds = %entry
	%tmp8 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%tmp9 = insertelement <4 x float> %tmp8, float 1.000000e+000, i32 0		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp9, <4 x float>* %temp1
	%tmp10 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp11 = extractelement <4 x float> %tmp10, i32 0		; <float> [#uses=1]
	%tmp12 = load <4 x float>* %temp3		; <<4 x float>> [#uses=1]
	%tmp13 = insertelement <4 x float> %tmp12, float %tmp11, i32 0		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp13, <4 x float>* %temp3
	br label %if.end

if.end:		; preds = %if.then, %entry
	%tmp14 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp15 = extractelement <4 x float> %tmp14, i32 1		; <float> [#uses=1]
	%cmp16 = fcmp ogt float %tmp15, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp16, label %if.then17, label %if.end24

if.then17:		; preds = %if.end
	%tmp18 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%tmp19 = insertelement <4 x float> %tmp18, float 1.000000e+000, i32 1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp19, <4 x float>* %temp1
	%tmp20 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp21 = extractelement <4 x float> %tmp20, i32 1		; <float> [#uses=1]
	%tmp22 = load <4 x float>* %temp3		; <<4 x float>> [#uses=1]
	%tmp23 = insertelement <4 x float> %tmp22, float %tmp21, i32 1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp23, <4 x float>* %temp3
	br label %if.end24

if.end24:		; preds = %if.then17, %if.end
	%tmp25 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp26 = extractelement <4 x float> %tmp25, i32 2		; <float> [#uses=1]
	%cmp27 = fcmp ogt float %tmp26, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp27, label %if.then28, label %if.end35

if.then28:		; preds = %if.end24
	%tmp29 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%tmp30 = insertelement <4 x float> %tmp29, float 1.000000e+000, i32 2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp30, <4 x float>* %temp1
	%tmp31 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp32 = extractelement <4 x float> %tmp31, i32 2		; <float> [#uses=1]
	%tmp33 = load <4 x float>* %temp3		; <<4 x float>> [#uses=1]
	%tmp34 = insertelement <4 x float> %tmp33, float %tmp32, i32 2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp34, <4 x float>* %temp3
	br label %if.end35

if.end35:		; preds = %if.then28, %if.end24
	%tmp36 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp37 = extractelement <4 x float> %tmp36, i32 3		; <float> [#uses=1]
	%cmp38 = fcmp ogt float %tmp37, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp38, label %if.then39, label %if.end46

if.then39:		; preds = %if.end35
	%tmp40 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%tmp41 = insertelement <4 x float> %tmp40, float 1.000000e+000, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp41, <4 x float>* %temp1
	%tmp42 = load <4 x float>* %tempDiff1		; <<4 x float>> [#uses=1]
	%tmp43 = extractelement <4 x float> %tmp42, i32 3		; <float> [#uses=1]
	%tmp44 = load <4 x float>* %temp3		; <<4 x float>> [#uses=1]
	%tmp45 = insertelement <4 x float> %tmp44, float %tmp43, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp45, <4 x float>* %temp3
	br label %if.end46

if.end46:		; preds = %if.then39, %if.end35
	%tmp47 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp48 = extractelement <4 x float> %tmp47, i32 0		; <float> [#uses=1]
	%cmp49 = fcmp ogt float %tmp48, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp49, label %if.then50, label %if.end57

if.then50:		; preds = %if.end46
	%tmp51 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%tmp52 = insertelement <4 x float> %tmp51, float 1.000000e+000, i32 0		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp52, <4 x float>* %temp2
	%tmp53 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp54 = extractelement <4 x float> %tmp53, i32 0		; <float> [#uses=1]
	%tmp55 = load <4 x float>* %temp4		; <<4 x float>> [#uses=1]
	%tmp56 = insertelement <4 x float> %tmp55, float %tmp54, i32 0		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp56, <4 x float>* %temp4
	br label %if.end57

if.end57:		; preds = %if.then50, %if.end46
	%tmp58 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp59 = extractelement <4 x float> %tmp58, i32 1		; <float> [#uses=1]
	%cmp60 = fcmp ogt float %tmp59, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp60, label %if.then61, label %if.end68

if.then61:		; preds = %if.end57
	%tmp62 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%tmp63 = insertelement <4 x float> %tmp62, float 1.000000e+000, i32 1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp63, <4 x float>* %temp2
	%tmp64 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp65 = extractelement <4 x float> %tmp64, i32 1		; <float> [#uses=1]
	%tmp66 = load <4 x float>* %temp4		; <<4 x float>> [#uses=1]
	%tmp67 = insertelement <4 x float> %tmp66, float %tmp65, i32 1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp67, <4 x float>* %temp4
	br label %if.end68

if.end68:		; preds = %if.then61, %if.end57
	%tmp69 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp70 = extractelement <4 x float> %tmp69, i32 2		; <float> [#uses=1]
	%cmp71 = fcmp ogt float %tmp70, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp71, label %if.then72, label %if.end79

if.then72:		; preds = %if.end68
	%tmp73 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%tmp74 = insertelement <4 x float> %tmp73, float 1.000000e+000, i32 2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp74, <4 x float>* %temp2
	%tmp75 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp76 = extractelement <4 x float> %tmp75, i32 2		; <float> [#uses=1]
	%tmp77 = load <4 x float>* %temp4		; <<4 x float>> [#uses=1]
	%tmp78 = insertelement <4 x float> %tmp77, float %tmp76, i32 2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp78, <4 x float>* %temp4
	br label %if.end79

if.end79:		; preds = %if.then72, %if.end68
	%tmp80 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp81 = extractelement <4 x float> %tmp80, i32 3		; <float> [#uses=1]
	%cmp82 = fcmp ogt float %tmp81, 0.000000e+000		; <i1> [#uses=1]
	br i1 %cmp82, label %if.then83, label %if.end90

if.then83:		; preds = %if.end79
	%tmp84 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%tmp85 = insertelement <4 x float> %tmp84, float 1.000000e+000, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp85, <4 x float>* %temp2
	%tmp86 = load <4 x float>* %tempDiff2		; <<4 x float>> [#uses=1]
	%tmp87 = extractelement <4 x float> %tmp86, i32 3		; <float> [#uses=1]
	%tmp88 = load <4 x float>* %temp4		; <<4 x float>> [#uses=1]
	%tmp89 = insertelement <4 x float> %tmp88, float %tmp87, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp89, <4 x float>* %temp4
	br label %if.end90

if.end90:		; preds = %if.then83, %if.end79
	%tmp91 = load <4 x float>** %pathDeriv1.addr		; <<4 x float>*> [#uses=1]
	%tmp92 = load <4 x float>* %meanDeriv1.addr		; <<4 x float>> [#uses=1]
	%tmp93 = load <4 x float>* %temp1		; <<4 x float>> [#uses=1]
	%mul = fmul <4 x float> %tmp92, %tmp93		; <<4 x float>> [#uses=1]
	store <4 x float> %mul, <4 x float>* %tmp91
	%tmp94 = load <4 x float>** %pathDeriv2.addr		; <<4 x float>*> [#uses=1]
	%tmp95 = load <4 x float>* %meanDeriv2.addr		; <<4 x float>> [#uses=1]
	%tmp96 = load <4 x float>* %temp2		; <<4 x float>> [#uses=1]
	%mul97 = fmul <4 x float> %tmp95, %tmp96		; <<4 x float>> [#uses=1]
	store <4 x float> %mul97, <4 x float>* %tmp94
	%tmp98 = load <4 x float>** %priceVec1.addr		; <<4 x float>*> [#uses=1]
	%tmp99 = load <4 x float>* %temp3		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp99, <4 x float>* %tmp98
	%tmp100 = load <4 x float>** %priceVec2.addr		; <<4 x float>*> [#uses=1]
	%tmp101 = load <4 x float>* %temp4		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp101, <4 x float>* %tmp100
	ret void
}

define void @calPriceVega(<4 x float> addrspace(1)* %attrib, i32 %noOfSum, i32 %width, <4 x i32> addrspace(1)* %randArray, <4 x float> addrspace(1)* %priceSamples, <4 x float> addrspace(1)* %pathDeriv, ...) nounwind {
entry:
	%attrib.addr = alloca <4 x float> addrspace(1)*		; <<4 x float> addrspace(1)**> [#uses=8]
	%noOfSum.addr = alloca i32		; <i32*> [#uses=6]
	%width.addr = alloca i32		; <i32*> [#uses=6]
	%randArray.addr = alloca <4 x i32> addrspace(1)*		; <<4 x i32> addrspace(1)**> [#uses=2]
	%priceSamples.addr = alloca <4 x float> addrspace(1)*		; <<4 x float> addrspace(1)**> [#uses=3]
	%pathDeriv.addr = alloca <4 x float> addrspace(1)*		; <<4 x float> addrspace(1)**> [#uses=3]
	%strikePrice = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%c1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%c2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%c3 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%initPrice = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=7]
	%sigma = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%timeStep = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%xPos = alloca i32, align 4		; <i32*> [#uses=6]
	%yPos = alloca i32, align 4		; <i32*> [#uses=6]
	%temp = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=12]
	%price1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%price2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%pathDeriv1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%pathDeriv2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%trajPrice1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=6]
	%trajPrice2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=6]
	%sumPrice1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%sumPrice2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%meanPrice1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%meanPrice2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%sumDeriv1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%sumDeriv2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%meanDeriv1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%meanDeriv2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%finalRandf1 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%finalRandf2 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%nextRand = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=5]
	%tempRand = alloca <4 x i32>, align 16		; <<4 x i32>*> [#uses=2]
	store <4 x float> addrspace(1)* %attrib, <4 x float> addrspace(1)** %attrib.addr
	store i32 %noOfSum, i32* %noOfSum.addr
	store i32 %width, i32* %width.addr
	store <4 x i32> addrspace(1)* %randArray, <4 x i32> addrspace(1)** %randArray.addr
	store <4 x float> addrspace(1)* %priceSamples, <4 x float> addrspace(1)** %priceSamples.addr
	store <4 x float> addrspace(1)* %pathDeriv, <4 x float> addrspace(1)** %pathDeriv.addr
	%tmp = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <4 x float> addrspace(1)* %tmp, i32 0		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp1 = load <4 x float> addrspace(1)* %arrayidx		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp1, <4 x float>* %strikePrice
	%tmp3 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx4 = getelementptr <4 x float> addrspace(1)* %tmp3, i32 1		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp5 = load <4 x float> addrspace(1)* %arrayidx4		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp5, <4 x float>* %c1
	%tmp7 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx8 = getelementptr <4 x float> addrspace(1)* %tmp7, i32 2		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp9 = load <4 x float> addrspace(1)* %arrayidx8		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp9, <4 x float>* %c2
	%tmp11 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx12 = getelementptr <4 x float> addrspace(1)* %tmp11, i32 3		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp13 = load <4 x float> addrspace(1)* %arrayidx12		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp13, <4 x float>* %c3
	%tmp15 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx16 = getelementptr <4 x float> addrspace(1)* %tmp15, i32 4		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp17 = load <4 x float> addrspace(1)* %arrayidx16		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp17, <4 x float>* %initPrice
	%tmp19 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx20 = getelementptr <4 x float> addrspace(1)* %tmp19, i32 5		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp21 = load <4 x float> addrspace(1)* %arrayidx20		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp21, <4 x float>* %sigma
	%tmp23 = load <4 x float> addrspace(1)** %attrib.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx24 = getelementptr <4 x float> addrspace(1)* %tmp23, i32 6		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp25 = load <4 x float> addrspace(1)* %arrayidx24		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp25, <4 x float>* %timeStep
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %xPos
	%call28 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call28, i32* %yPos
	store <4 x float> zeroinitializer, <4 x float>* %temp
	store <4 x float> zeroinitializer, <4 x float>* %price1
	store <4 x float> zeroinitializer, <4 x float>* %price2
	store <4 x float> zeroinitializer, <4 x float>* %pathDeriv1
	store <4 x float> zeroinitializer, <4 x float>* %pathDeriv2
	%tmp35 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp35, <4 x float>* %trajPrice1
	%tmp37 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp37, <4 x float>* %trajPrice2
	%tmp39 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp39, <4 x float>* %sumPrice1
	%tmp41 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp41, <4 x float>* %sumPrice2
	%tmp43 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp43, <4 x float>* %meanPrice1
	%tmp45 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp45, <4 x float>* %meanPrice2
	%tmp47 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp47, <4 x float>* %sumDeriv1
	%tmp49 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp49, <4 x float>* %sumDeriv2
	%tmp51 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp51, <4 x float>* %meanDeriv1
	%tmp53 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp53, <4 x float>* %meanDeriv2
	%tmp55 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp55, <4 x float>* %finalRandf1
	%tmp57 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp57, <4 x float>* %finalRandf2
	%tmp59 = load i32* %yPos		; <i32> [#uses=1]
	%tmp60 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp59, %tmp60		; <i32> [#uses=1]
	%tmp61 = load i32* %xPos		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp61		; <i32> [#uses=1]
	%tmp62 = load <4 x i32> addrspace(1)** %randArray.addr		; <<4 x i32> addrspace(1)*> [#uses=1]
	%arrayidx63 = getelementptr <4 x i32> addrspace(1)* %tmp62, i32 %add		; <<4 x i32> addrspace(1)*> [#uses=1]
	%tmp64 = load <4 x i32> addrspace(1)* %arrayidx63		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp64, <4 x i32>* %nextRand
	store i32 1, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp66 = load i32* %i		; <i32> [#uses=1]
	%tmp67 = load i32* %noOfSum.addr		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp66, %tmp67		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp69 = load <4 x i32>* %nextRand		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp69, <4 x i32>* %tempRand
	%tmp70 = load <4 x i32>* %tempRand		; <<4 x i32>> [#uses=1]
	call void @generateRand(<4 x i32> %tmp70, <4 x float>* %finalRandf1, <4 x float>* %finalRandf2, <4 x i32>* %nextRand)
	%tmp71 = load <4 x float>* %trajPrice1		; <<4 x float>> [#uses=1]
	%tmp72 = load <4 x float>* %c1		; <<4 x float>> [#uses=1]
	%tmp73 = load <4 x float>* %c2		; <<4 x float>> [#uses=1]
	%tmp74 = load <4 x float>* %finalRandf1		; <<4 x float>> [#uses=1]
	%mul75 = fmul <4 x float> %tmp73, %tmp74		; <<4 x float>> [#uses=1]
	%add76 = fadd <4 x float> %tmp72, %mul75		; <<4 x float>> [#uses=1]
	%call77 = call <4 x float> @__expf4(<4 x float> %add76)		; <<4 x float>> [#uses=1]
	%mul78 = fmul <4 x float> %tmp71, %call77		; <<4 x float>> [#uses=1]
	store <4 x float> %mul78, <4 x float>* %trajPrice1
	%tmp79 = load <4 x float>* %trajPrice2		; <<4 x float>> [#uses=1]
	%tmp80 = load <4 x float>* %c1		; <<4 x float>> [#uses=1]
	%tmp81 = load <4 x float>* %c2		; <<4 x float>> [#uses=1]
	%tmp82 = load <4 x float>* %finalRandf2		; <<4 x float>> [#uses=1]
	%mul83 = fmul <4 x float> %tmp81, %tmp82		; <<4 x float>> [#uses=1]
	%add84 = fadd <4 x float> %tmp80, %mul83		; <<4 x float>> [#uses=1]
	%call85 = call <4 x float> @__expf4(<4 x float> %add84)		; <<4 x float>> [#uses=1]
	%mul86 = fmul <4 x float> %tmp79, %call85		; <<4 x float>> [#uses=1]
	store <4 x float> %mul86, <4 x float>* %trajPrice2
	%tmp87 = load <4 x float>* %sumPrice1		; <<4 x float>> [#uses=1]
	%tmp88 = load <4 x float>* %trajPrice1		; <<4 x float>> [#uses=1]
	%add89 = fadd <4 x float> %tmp87, %tmp88		; <<4 x float>> [#uses=1]
	store <4 x float> %add89, <4 x float>* %sumPrice1
	%tmp90 = load <4 x float>* %sumPrice2		; <<4 x float>> [#uses=1]
	%tmp91 = load <4 x float>* %trajPrice2		; <<4 x float>> [#uses=1]
	%add92 = fadd <4 x float> %tmp90, %tmp91		; <<4 x float>> [#uses=1]
	store <4 x float> %add92, <4 x float>* %sumPrice2
	%tmp93 = load <4 x float>* %c3		; <<4 x float>> [#uses=1]
	%tmp94 = load <4 x float>* %timeStep		; <<4 x float>> [#uses=1]
	%mul95 = fmul <4 x float> %tmp93, %tmp94		; <<4 x float>> [#uses=1]
	%tmp96 = load i32* %i		; <i32> [#uses=1]
	%conv = sitofp i32 %tmp96 to float		; <float> [#uses=1]
	%tmp97 = insertelement <4 x float> undef, float %conv, i32 0		; <<4 x float>> [#uses=2]
	%splat = shufflevector <4 x float> %tmp97, <4 x float> %tmp97, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%mul98 = fmul <4 x float> %mul95, %splat		; <<4 x float>> [#uses=1]
	store <4 x float> %mul98, <4 x float>* %temp
	%tmp99 = load <4 x float>* %sumDeriv1		; <<4 x float>> [#uses=1]
	%tmp100 = load <4 x float>* %trajPrice1		; <<4 x float>> [#uses=1]
	%tmp101 = load <4 x float>* %trajPrice1		; <<4 x float>> [#uses=1]
	%tmp102 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	%div = fdiv <4 x float> %tmp101, %tmp102		; <<4 x float>> [#uses=1]
	%call103 = call <4 x float> @__logf4(<4 x float> %div)		; <<4 x float>> [#uses=1]
	%tmp104 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%sub = fsub <4 x float> %call103, %tmp104		; <<4 x float>> [#uses=1]
	%tmp105 = load <4 x float>* %sigma		; <<4 x float>> [#uses=1]
	%div106 = fdiv <4 x float> %sub, %tmp105		; <<4 x float>> [#uses=1]
	%mul107 = fmul <4 x float> %tmp100, %div106		; <<4 x float>> [#uses=1]
	%add108 = fadd <4 x float> %tmp99, %mul107		; <<4 x float>> [#uses=1]
	store <4 x float> %add108, <4 x float>* %sumDeriv1
	%tmp109 = load <4 x float>* %sumDeriv2		; <<4 x float>> [#uses=1]
	%tmp110 = load <4 x float>* %trajPrice2		; <<4 x float>> [#uses=1]
	%tmp111 = load <4 x float>* %trajPrice2		; <<4 x float>> [#uses=1]
	%tmp112 = load <4 x float>* %initPrice		; <<4 x float>> [#uses=1]
	%div113 = fdiv <4 x float> %tmp111, %tmp112		; <<4 x float>> [#uses=1]
	%call114 = call <4 x float> @__logf4(<4 x float> %div113)		; <<4 x float>> [#uses=1]
	%tmp115 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%sub116 = fsub <4 x float> %call114, %tmp115		; <<4 x float>> [#uses=1]
	%tmp117 = load <4 x float>* %sigma		; <<4 x float>> [#uses=1]
	%div118 = fdiv <4 x float> %sub116, %tmp117		; <<4 x float>> [#uses=1]
	%mul119 = fmul <4 x float> %tmp110, %div118		; <<4 x float>> [#uses=1]
	%add120 = fadd <4 x float> %tmp109, %mul119		; <<4 x float>> [#uses=1]
	store <4 x float> %add120, <4 x float>* %sumDeriv2
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp121 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp121, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp122 = load <4 x float>* %sumPrice1		; <<4 x float>> [#uses=1]
	%tmp123 = load i32* %noOfSum.addr		; <i32> [#uses=1]
	%conv124 = sitofp i32 %tmp123 to float		; <float> [#uses=1]
	%tmp125 = insertelement <4 x float> undef, float %conv124, i32 0		; <<4 x float>> [#uses=2]
	%splat126 = shufflevector <4 x float> %tmp125, <4 x float> %tmp125, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div127 = fdiv <4 x float> %tmp122, %splat126		; <<4 x float>> [#uses=1]
	store <4 x float> %div127, <4 x float>* %meanPrice1
	%tmp128 = load <4 x float>* %sumPrice2		; <<4 x float>> [#uses=1]
	%tmp129 = load i32* %noOfSum.addr		; <i32> [#uses=1]
	%conv130 = sitofp i32 %tmp129 to float		; <float> [#uses=1]
	%tmp131 = insertelement <4 x float> undef, float %conv130, i32 0		; <<4 x float>> [#uses=2]
	%splat132 = shufflevector <4 x float> %tmp131, <4 x float> %tmp131, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div133 = fdiv <4 x float> %tmp128, %splat132		; <<4 x float>> [#uses=1]
	store <4 x float> %div133, <4 x float>* %meanPrice2
	%tmp134 = load <4 x float>* %sumDeriv1		; <<4 x float>> [#uses=1]
	%tmp135 = load i32* %noOfSum.addr		; <i32> [#uses=1]
	%conv136 = sitofp i32 %tmp135 to float		; <float> [#uses=1]
	%tmp137 = insertelement <4 x float> undef, float %conv136, i32 0		; <<4 x float>> [#uses=2]
	%splat138 = shufflevector <4 x float> %tmp137, <4 x float> %tmp137, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div139 = fdiv <4 x float> %tmp134, %splat138		; <<4 x float>> [#uses=1]
	store <4 x float> %div139, <4 x float>* %meanDeriv1
	%tmp140 = load <4 x float>* %sumDeriv2		; <<4 x float>> [#uses=1]
	%tmp141 = load i32* %noOfSum.addr		; <i32> [#uses=1]
	%conv142 = sitofp i32 %tmp141 to float		; <float> [#uses=1]
	%tmp143 = insertelement <4 x float> undef, float %conv142, i32 0		; <<4 x float>> [#uses=2]
	%splat144 = shufflevector <4 x float> %tmp143, <4 x float> %tmp143, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%div145 = fdiv <4 x float> %tmp140, %splat144		; <<4 x float>> [#uses=1]
	store <4 x float> %div145, <4 x float>* %meanDeriv2
	%tmp146 = load <4 x float>* %strikePrice		; <<4 x float>> [#uses=1]
	%tmp147 = load <4 x float>* %meanDeriv1		; <<4 x float>> [#uses=1]
	%tmp148 = load <4 x float>* %meanDeriv2		; <<4 x float>> [#uses=1]
	%tmp149 = load <4 x float>* %meanPrice1		; <<4 x float>> [#uses=1]
	%tmp150 = load <4 x float>* %meanPrice2		; <<4 x float>> [#uses=1]
	call void @calOutputs(<4 x float> %tmp146, <4 x float> %tmp147, <4 x float> %tmp148, <4 x float> %tmp149, <4 x float> %tmp150, <4 x float>* %pathDeriv1, <4 x float>* %pathDeriv2, <4 x float>* %price1, <4 x float>* %price2)
	%tmp151 = load i32* %yPos		; <i32> [#uses=1]
	%tmp152 = load i32* %width.addr		; <i32> [#uses=1]
	%mul153 = mul i32 %tmp151, %tmp152		; <i32> [#uses=1]
	%tmp154 = load i32* %xPos		; <i32> [#uses=1]
	%add155 = add i32 %mul153, %tmp154		; <i32> [#uses=1]
	%mul156 = mul i32 %add155, 2		; <i32> [#uses=1]
	%tmp157 = load <4 x float> addrspace(1)** %priceSamples.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx158 = getelementptr <4 x float> addrspace(1)* %tmp157, i32 %mul156		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp159 = load <4 x float>* %price1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp159, <4 x float> addrspace(1)* %arrayidx158
	%tmp160 = load i32* %yPos		; <i32> [#uses=1]
	%tmp161 = load i32* %width.addr		; <i32> [#uses=1]
	%mul162 = mul i32 %tmp160, %tmp161		; <i32> [#uses=1]
	%tmp163 = load i32* %xPos		; <i32> [#uses=1]
	%add164 = add i32 %mul162, %tmp163		; <i32> [#uses=1]
	%mul165 = mul i32 %add164, 2		; <i32> [#uses=1]
	%add166 = add i32 %mul165, 1		; <i32> [#uses=1]
	%tmp167 = load <4 x float> addrspace(1)** %priceSamples.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx168 = getelementptr <4 x float> addrspace(1)* %tmp167, i32 %add166		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp169 = load <4 x float>* %price2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp169, <4 x float> addrspace(1)* %arrayidx168
	%tmp170 = load i32* %yPos		; <i32> [#uses=1]
	%tmp171 = load i32* %width.addr		; <i32> [#uses=1]
	%mul172 = mul i32 %tmp170, %tmp171		; <i32> [#uses=1]
	%tmp173 = load i32* %xPos		; <i32> [#uses=1]
	%add174 = add i32 %mul172, %tmp173		; <i32> [#uses=1]
	%mul175 = mul i32 %add174, 2		; <i32> [#uses=1]
	%tmp176 = load <4 x float> addrspace(1)** %pathDeriv.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx177 = getelementptr <4 x float> addrspace(1)* %tmp176, i32 %mul175		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp178 = load <4 x float>* %pathDeriv1		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp178, <4 x float> addrspace(1)* %arrayidx177
	%tmp179 = load i32* %yPos		; <i32> [#uses=1]
	%tmp180 = load i32* %width.addr		; <i32> [#uses=1]
	%mul181 = mul i32 %tmp179, %tmp180		; <i32> [#uses=1]
	%tmp182 = load i32* %xPos		; <i32> [#uses=1]
	%add183 = add i32 %mul181, %tmp182		; <i32> [#uses=1]
	%mul184 = mul i32 %add183, 2		; <i32> [#uses=1]
	%add185 = add i32 %mul184, 1		; <i32> [#uses=1]
	%tmp186 = load <4 x float> addrspace(1)** %pathDeriv.addr		; <<4 x float> addrspace(1)*> [#uses=1]
	%arrayidx187 = getelementptr <4 x float> addrspace(1)* %tmp186, i32 %add185		; <<4 x float> addrspace(1)*> [#uses=1]
	%tmp188 = load <4 x float>* %pathDeriv2		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp188, <4 x float> addrspace(1)* %arrayidx187
	ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @__expf4(<4 x float>)
