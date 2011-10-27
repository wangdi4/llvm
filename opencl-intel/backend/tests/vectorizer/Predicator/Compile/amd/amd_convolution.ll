; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_convolution.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [6 x i8] c"22200\00"		; <[6 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (float addrspace(1)*, i32 addrspace(1)*, float addrspace(1)*, <2 x i32>, <2 x i32>, ...)* @simpleConvolution to i8*), i8* getelementptr ([6 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: simpleConvolution

define void @simpleConvolution(float addrspace(1)* %output, i32 addrspace(1)* %input, float addrspace(1)* %mask, <2 x i32> %inputDimensions, <2 x i32> %maskDimensions, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%input.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%mask.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%inputDimensions.addr = alloca <2 x i32>		; <<2 x i32>*> [#uses=3]
	%maskDimensions.addr = alloca <2 x i32>		; <<2 x i32>*> [#uses=3]
	%tid = alloca i32, align 4		; <i32*> [#uses=4]
	%width = alloca i32, align 4		; <i32*> [#uses=6]
	%height = alloca i32, align 4		; <i32*> [#uses=3]
	%x = alloca i32, align 4		; <i32*> [#uses=6]
	%y = alloca i32, align 4		; <i32*> [#uses=6]
	%maskWidth = alloca i32, align 4		; <i32*> [#uses=3]
	%maskHeight = alloca i32, align 4		; <i32*> [#uses=2]
	%vstep = alloca i32, align 4		; <i32*> [#uses=6]
	%hstep = alloca i32, align 4		; <i32*> [#uses=6]
	%left = alloca i32, align 4		; <i32*> [#uses=2]
	%right = alloca i32, align 4		; <i32*> [#uses=2]
	%top = alloca i32, align 4		; <i32*> [#uses=2]
	%bottom = alloca i32, align 4		; <i32*> [#uses=2]
	%sumFX = alloca float, align 4		; <float*> [#uses=6]
	%i = alloca i32, align 4		; <i32*> [#uses=6]
	%j = alloca i32, align 4		; <i32*> [#uses=6]
	%maskIndex = alloca i32, align 4		; <i32*> [#uses=2]
	%index = alloca i32, align 4		; <i32*> [#uses=2]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr
	store float addrspace(1)* %mask, float addrspace(1)** %mask.addr
	store <2 x i32> %inputDimensions, <2 x i32>* %inputDimensions.addr
	store <2 x i32> %maskDimensions, <2 x i32>* %maskDimensions.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %tid
	%tmp = load <2 x i32>* %inputDimensions.addr		; <<2 x i32>> [#uses=1]
	%tmp1 = extractelement <2 x i32> %tmp, i32 0		; <i32> [#uses=1]
	store i32 %tmp1, i32* %width
	%tmp3 = load <2 x i32>* %inputDimensions.addr		; <<2 x i32>> [#uses=1]
	%tmp4 = extractelement <2 x i32> %tmp3, i32 1		; <i32> [#uses=1]
	store i32 %tmp4, i32* %height
	%tmp6 = load i32* %tid		; <i32> [#uses=1]
	%tmp7 = load i32* %width		; <i32> [#uses=2]
	%cmp = icmp ne i32 %tmp7, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp, i32 %tmp7, i32 1		; <i32> [#uses=1]
	%rem = urem i32 %tmp6, %nonzero		; <i32> [#uses=1]
	store i32 %rem, i32* %x
	%tmp9 = load i32* %tid		; <i32> [#uses=1]
	%tmp10 = load i32* %width		; <i32> [#uses=2]
	%cmp11 = icmp ne i32 %tmp10, 0		; <i1> [#uses=1]
	%nonzero12 = select i1 %cmp11, i32 %tmp10, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp9, %nonzero12		; <i32> [#uses=1]
	store i32 %div, i32* %y
	%tmp14 = load <2 x i32>* %maskDimensions.addr		; <<2 x i32>> [#uses=1]
	%tmp15 = extractelement <2 x i32> %tmp14, i32 0		; <i32> [#uses=1]
	store i32 %tmp15, i32* %maskWidth
	%tmp17 = load <2 x i32>* %maskDimensions.addr		; <<2 x i32>> [#uses=1]
	%tmp18 = extractelement <2 x i32> %tmp17, i32 1		; <i32> [#uses=1]
	store i32 %tmp18, i32* %maskHeight
	%tmp20 = load i32* %maskWidth		; <i32> [#uses=1]
	%sub = sub i32 %tmp20, 1		; <i32> [#uses=1]
	%div21 = udiv i32 %sub, 2		; <i32> [#uses=1]
	store i32 %div21, i32* %vstep
	%tmp23 = load i32* %maskHeight		; <i32> [#uses=1]
	%sub24 = sub i32 %tmp23, 1		; <i32> [#uses=1]
	%div25 = udiv i32 %sub24, 2		; <i32> [#uses=1]
	store i32 %div25, i32* %hstep
	%tmp27 = load i32* %x		; <i32> [#uses=1]
	%tmp28 = load i32* %vstep		; <i32> [#uses=1]
	%cmp29 = icmp ult i32 %tmp27, %tmp28		; <i1> [#uses=1]
	br i1 %cmp29, label %cond.true, label %cond.false

cond.true:		; preds = %entry
	br label %cond.end

cond.false:		; preds = %entry
	%tmp30 = load i32* %x		; <i32> [#uses=1]
	%tmp31 = load i32* %vstep		; <i32> [#uses=1]
	%sub32 = sub i32 %tmp30, %tmp31		; <i32> [#uses=1]
	br label %cond.end

cond.end:		; preds = %cond.false, %cond.true
	%cond = phi i32 [ 0, %cond.true ], [ %sub32, %cond.false ]		; <i32> [#uses=1]
	store i32 %cond, i32* %left
	%tmp34 = load i32* %x		; <i32> [#uses=1]
	%tmp35 = load i32* %vstep		; <i32> [#uses=1]
	%add = add i32 %tmp34, %tmp35		; <i32> [#uses=1]
	%tmp36 = load i32* %width		; <i32> [#uses=1]
	%cmp37 = icmp uge i32 %add, %tmp36		; <i1> [#uses=1]
	br i1 %cmp37, label %cond.true38, label %cond.false41

cond.true38:		; preds = %cond.end
	%tmp39 = load i32* %width		; <i32> [#uses=1]
	%sub40 = sub i32 %tmp39, 1		; <i32> [#uses=1]
	br label %cond.end45

cond.false41:		; preds = %cond.end
	%tmp42 = load i32* %x		; <i32> [#uses=1]
	%tmp43 = load i32* %vstep		; <i32> [#uses=1]
	%add44 = add i32 %tmp42, %tmp43		; <i32> [#uses=1]
	br label %cond.end45

cond.end45:		; preds = %cond.false41, %cond.true38
	%cond46 = phi i32 [ %sub40, %cond.true38 ], [ %add44, %cond.false41 ]		; <i32> [#uses=1]
	store i32 %cond46, i32* %right
	%tmp48 = load i32* %y		; <i32> [#uses=1]
	%tmp49 = load i32* %hstep		; <i32> [#uses=1]
	%cmp50 = icmp ult i32 %tmp48, %tmp49		; <i1> [#uses=1]
	br i1 %cmp50, label %cond.true51, label %cond.false52

cond.true51:		; preds = %cond.end45
	br label %cond.end56

cond.false52:		; preds = %cond.end45
	%tmp53 = load i32* %y		; <i32> [#uses=1]
	%tmp54 = load i32* %hstep		; <i32> [#uses=1]
	%sub55 = sub i32 %tmp53, %tmp54		; <i32> [#uses=1]
	br label %cond.end56

cond.end56:		; preds = %cond.false52, %cond.true51
	%cond57 = phi i32 [ 0, %cond.true51 ], [ %sub55, %cond.false52 ]		; <i32> [#uses=1]
	store i32 %cond57, i32* %top
	%tmp59 = load i32* %y		; <i32> [#uses=1]
	%tmp60 = load i32* %hstep		; <i32> [#uses=1]
	%add61 = add i32 %tmp59, %tmp60		; <i32> [#uses=1]
	%tmp62 = load i32* %height		; <i32> [#uses=1]
	%cmp63 = icmp uge i32 %add61, %tmp62		; <i1> [#uses=1]
	br i1 %cmp63, label %cond.true64, label %cond.false67

cond.true64:		; preds = %cond.end56
	%tmp65 = load i32* %height		; <i32> [#uses=1]
	%sub66 = sub i32 %tmp65, 1		; <i32> [#uses=1]
	br label %cond.end71

cond.false67:		; preds = %cond.end56
	%tmp68 = load i32* %y		; <i32> [#uses=1]
	%tmp69 = load i32* %hstep		; <i32> [#uses=1]
	%add70 = add i32 %tmp68, %tmp69		; <i32> [#uses=1]
	br label %cond.end71

cond.end71:		; preds = %cond.false67, %cond.true64
	%cond72 = phi i32 [ %sub66, %cond.true64 ], [ %add70, %cond.false67 ]		; <i32> [#uses=1]
	store i32 %cond72, i32* %bottom
	store float 0.000000e+000, float* %sumFX
	%tmp75 = load i32* %left		; <i32> [#uses=1]
	store i32 %tmp75, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc116, %cond.end71
	%tmp76 = load i32* %i		; <i32> [#uses=1]
	%tmp77 = load i32* %right		; <i32> [#uses=1]
	%cmp78 = icmp ule i32 %tmp76, %tmp77		; <i1> [#uses=1]
	br i1 %cmp78, label %for.body, label %for.end119

for.body:		; preds = %for.cond
	%tmp80 = load i32* %top		; <i32> [#uses=1]
	store i32 %tmp80, i32* %j
	br label %for.cond81

for.cond81:		; preds = %for.inc, %for.body
	%tmp82 = load i32* %j		; <i32> [#uses=1]
	%tmp83 = load i32* %bottom		; <i32> [#uses=1]
	%cmp84 = icmp ule i32 %tmp82, %tmp83		; <i1> [#uses=1]
	br i1 %cmp84, label %for.body85, label %for.end

for.body85:		; preds = %for.cond81
	%tmp87 = load i32* %j		; <i32> [#uses=1]
	%tmp88 = load i32* %y		; <i32> [#uses=1]
	%tmp89 = load i32* %hstep		; <i32> [#uses=1]
	%sub90 = sub i32 %tmp88, %tmp89		; <i32> [#uses=1]
	%sub91 = sub i32 %tmp87, %sub90		; <i32> [#uses=1]
	%tmp92 = load i32* %maskWidth		; <i32> [#uses=1]
	%mul = mul i32 %sub91, %tmp92		; <i32> [#uses=1]
	%tmp93 = load i32* %i		; <i32> [#uses=1]
	%tmp94 = load i32* %x		; <i32> [#uses=1]
	%tmp95 = load i32* %vstep		; <i32> [#uses=1]
	%sub96 = sub i32 %tmp94, %tmp95		; <i32> [#uses=1]
	%sub97 = sub i32 %tmp93, %sub96		; <i32> [#uses=1]
	%add98 = add i32 %mul, %sub97		; <i32> [#uses=1]
	store i32 %add98, i32* %maskIndex
	%tmp100 = load i32* %j		; <i32> [#uses=1]
	%tmp101 = load i32* %width		; <i32> [#uses=1]
	%mul102 = mul i32 %tmp100, %tmp101		; <i32> [#uses=1]
	%tmp103 = load i32* %i		; <i32> [#uses=1]
	%add104 = add i32 %mul102, %tmp103		; <i32> [#uses=1]
	store i32 %add104, i32* %index
	%tmp105 = load float* %sumFX		; <float> [#uses=1]
	%tmp106 = load i32* %index		; <i32> [#uses=1]
	%tmp107 = load i32 addrspace(1)** %input.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i32 addrspace(1)* %tmp107, i32 %tmp106		; <i32 addrspace(1)*> [#uses=1]
	%tmp108 = load i32 addrspace(1)* %arrayidx		; <i32> [#uses=1]
	%conv = uitofp i32 %tmp108 to float		; <float> [#uses=1]
	%tmp109 = load i32* %maskIndex		; <i32> [#uses=1]
	%tmp110 = load float addrspace(1)** %mask.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx111 = getelementptr float addrspace(1)* %tmp110, i32 %tmp109		; <float addrspace(1)*> [#uses=1]
	%tmp112 = load float addrspace(1)* %arrayidx111		; <float> [#uses=1]
	%mul113 = fmul float %conv, %tmp112		; <float> [#uses=1]
	%add114 = fadd float %tmp105, %mul113		; <float> [#uses=1]
	store float %add114, float* %sumFX
	br label %for.inc

for.inc:		; preds = %for.body85
	%tmp115 = load i32* %j		; <i32> [#uses=1]
	%inc = add i32 %tmp115, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %j
	br label %for.cond81

for.end:		; preds = %for.cond81
	br label %for.inc116

for.inc116:		; preds = %for.end
	%tmp117 = load i32* %i		; <i32> [#uses=1]
	%inc118 = add i32 %tmp117, 1		; <i32> [#uses=1]
	store i32 %inc118, i32* %i
	br label %for.cond

for.end119:		; preds = %for.cond
	%tmp120 = load float* %sumFX		; <float> [#uses=1]
	%add121 = fadd float %tmp120, 5.000000e-001		; <float> [#uses=1]
	store float %add121, float* %sumFX
	%tmp122 = load i32* %tid		; <i32> [#uses=1]
	%tmp123 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx124 = getelementptr float addrspace(1)* %tmp123, i32 %tmp122		; <float addrspace(1)*> [#uses=1]
	%tmp125 = load float* %sumFX		; <float> [#uses=1]
	store float %tmp125, float addrspace(1)* %arrayidx124
	ret void
}

declare i32 @get_global_id(i32)
