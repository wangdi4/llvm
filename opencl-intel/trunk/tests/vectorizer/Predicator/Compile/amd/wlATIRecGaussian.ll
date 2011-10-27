; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIRecGaussian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [7 x i8] c"229000\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [13 x i8] c"120000000000\00"		; <[13 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [2 x %0] [%0 { i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(3)*, i32, i32, i32, ...)* @transpose_kernel to i8*), i8* getelementptr ([7 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i32, i32, float, float, float, float, float, float, float, float, ...)* @RecursiveGaussian_kernel to i8*), i8* getelementptr ([13 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv3 to i8*), i32 0 }], section "llvm.metadata"		; <[2 x %0]*> [#uses=0]

; CHECK: @transpose_kernel
define void @transpose_kernel(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(3)* %block, i32 %width, i32 %height, i32 %blockSize, ...) nounwind {
entry:
	%output.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%input.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%block.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=2]
	%height.addr = alloca i32		; <i32*> [#uses=2]
	%blockSize.addr = alloca i32		; <i32*> [#uses=3]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%localIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%localIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%sourceIndex = alloca i32, align 4		; <i32*> [#uses=2]
	%targetIndex = alloca i32, align 4		; <i32*> [#uses=2]
	store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
	store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
	store <4 x i8> addrspace(3)* %block, <4 x i8> addrspace(3)** %block.addr
	store i32 %width, i32* %width.addr
	store i32 %height, i32* %height.addr
	store i32 %blockSize, i32* %blockSize.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalIdx
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalIdy
	%call2 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %localIdx
	%call3 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %localIdy
	%tmp = load i32* %localIdy		; <i32> [#uses=1]
	%tmp4 = load i32* %blockSize.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp4		; <i32> [#uses=1]
	%tmp5 = load i32* %localIdx		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp5		; <i32> [#uses=1]
	%tmp6 = load <4 x i8> addrspace(3)** %block.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr <4 x i8> addrspace(3)* %tmp6, i32 %add		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp7 = load i32* %globalIdy		; <i32> [#uses=1]
	%tmp8 = load i32* %width.addr		; <i32> [#uses=1]
	%mul9 = mul i32 %tmp7, %tmp8		; <i32> [#uses=1]
	%tmp10 = load i32* %globalIdx		; <i32> [#uses=1]
	%add11 = add i32 %mul9, %tmp10		; <i32> [#uses=1]
	%tmp12 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx13 = getelementptr <4 x i8> addrspace(1)* %tmp12, i32 %add11		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp14 = load <4 x i8> addrspace(1)* %arrayidx13		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp14, <4 x i8> addrspace(3)* %arrayidx
	call void @barrier(i32 1)
	%tmp16 = load i32* %localIdy		; <i32> [#uses=1]
	%tmp17 = load i32* %blockSize.addr		; <i32> [#uses=1]
	%mul18 = mul i32 %tmp16, %tmp17		; <i32> [#uses=1]
	%tmp19 = load i32* %localIdx		; <i32> [#uses=1]
	%add20 = add i32 %mul18, %tmp19		; <i32> [#uses=1]
	store i32 %add20, i32* %sourceIndex
	%tmp22 = load i32* %globalIdy		; <i32> [#uses=1]
	%tmp23 = load i32* %globalIdx		; <i32> [#uses=1]
	%tmp24 = load i32* %height.addr		; <i32> [#uses=1]
	%mul25 = mul i32 %tmp23, %tmp24		; <i32> [#uses=1]
	%add26 = add i32 %tmp22, %mul25		; <i32> [#uses=1]
	store i32 %add26, i32* %targetIndex
	%tmp27 = load i32* %targetIndex		; <i32> [#uses=1]
	%tmp28 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx29 = getelementptr <4 x i8> addrspace(1)* %tmp28, i32 %tmp27		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp30 = load i32* %sourceIndex		; <i32> [#uses=1]
	%tmp31 = load <4 x i8> addrspace(3)** %block.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx32 = getelementptr <4 x i8> addrspace(3)* %tmp31, i32 %tmp30		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp33 = load <4 x i8> addrspace(3)* %arrayidx32		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp33, <4 x i8> addrspace(1)* %arrayidx29
	ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

define void @RecursiveGaussian_kernel(<4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %output, i32 %width, i32 %height, float %a0, float %a1, float %a2, float %a3, float %b1, float %b2, float %coefp, float %coefn, ...) nounwind {
entry:
	%input.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=9]
	%output.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=7]
	%width.addr = alloca i32		; <i32*> [#uses=4]
	%height.addr = alloca i32		; <i32*> [#uses=3]
	%a0.addr = alloca float		; <float*> [#uses=2]
	%a1.addr = alloca float		; <float*> [#uses=2]
	%a2.addr = alloca float		; <float*> [#uses=2]
	%a3.addr = alloca float		; <float*> [#uses=2]
	%b1.addr = alloca float		; <float*> [#uses=3]
	%b2.addr = alloca float		; <float*> [#uses=3]
	%coefp.addr = alloca float		; <float*> [#uses=1]
	%coefn.addr = alloca float		; <float*> [#uses=1]
	%x = alloca i32, align 4		; <i32*> [#uses=4]
	%xp = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%yp = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%yb = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%y = alloca i32, align 4		; <i32*> [#uses=5]
	%pos = alloca i32, align 4		; <i32*> [#uses=6]
	%xc = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%.compoundliteral = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%yc = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=6]
	%.compoundliteral62 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%xn = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%xa = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%yn = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=4]
	%ya = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%y85 = alloca i32, align 4		; <i32*> [#uses=5]
	%pos94 = alloca i32, align 4		; <i32*> [#uses=10]
	%xc101 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%.compoundliteral102 = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%yc129 = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%temp = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=5]
	%.compoundliteral158 = alloca <4 x float>		; <<4 x float>*> [#uses=2]
	%.compoundliteral189 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
	store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
	store i32 %width, i32* %width.addr
	store i32 %height, i32* %height.addr
	store float %a0, float* %a0.addr
	store float %a1, float* %a1.addr
	store float %a2, float* %a2.addr
	store float %a3, float* %a3.addr
	store float %b1, float* %b1.addr
	store float %b2, float* %b2.addr
	store float %coefp, float* %coefp.addr
	store float %coefn, float* %coefn.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %x
	%tmp = load i32* %x		; <i32> [#uses=1]
	%tmp1 = load i32* %width.addr		; <i32> [#uses=1]
	%cmp = icmp uge i32 %tmp, %tmp1		; <i1> [#uses=1]
	br i1 %cmp, label %if.then, label %if.end

if.then:		; preds = %entry
	br label %return

if.end:		; preds = %entry
	store <4 x float> zeroinitializer, <4 x float>* %xp
	store <4 x float> zeroinitializer, <4 x float>* %yp
	store <4 x float> zeroinitializer, <4 x float>* %yb
	store i32 0, i32* %y
	br label %for.cond

for.cond:		; preds = %for.inc, %if.end
	%tmp6 = load i32* %y		; <i32> [#uses=1]
	%tmp7 = load i32* %height.addr		; <i32> [#uses=1]
	%cmp8 = icmp slt i32 %tmp6, %tmp7		; <i1> [#uses=1]
	br i1 %cmp8, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp10 = load i32* %x		; <i32> [#uses=1]
	%tmp11 = load i32* %y		; <i32> [#uses=1]
	%tmp12 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp11, %tmp12		; <i32> [#uses=1]
	%add = add i32 %tmp10, %mul		; <i32> [#uses=1]
	store i32 %add, i32* %pos
	%tmp14 = load i32* %pos		; <i32> [#uses=1]
	%tmp15 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <4 x i8> addrspace(1)* %tmp15, i32 %tmp14		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp16 = load <4 x i8> addrspace(1)* %arrayidx		; <<4 x i8>> [#uses=1]
	%tmp17 = extractelement <4 x i8> %tmp16, i32 0		; <i8> [#uses=1]
	%conv = uitofp i8 %tmp17 to float		; <float> [#uses=1]
	%0 = insertelement <4 x float> undef, float %conv, i32 0		; <<4 x float>> [#uses=1]
	%tmp18 = load i32* %pos		; <i32> [#uses=1]
	%tmp19 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx20 = getelementptr <4 x i8> addrspace(1)* %tmp19, i32 %tmp18		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp21 = load <4 x i8> addrspace(1)* %arrayidx20		; <<4 x i8>> [#uses=1]
	%tmp22 = extractelement <4 x i8> %tmp21, i32 1		; <i8> [#uses=1]
	%conv23 = uitofp i8 %tmp22 to float		; <float> [#uses=1]
	%1 = insertelement <4 x float> %0, float %conv23, i32 1		; <<4 x float>> [#uses=1]
	%tmp24 = load i32* %pos		; <i32> [#uses=1]
	%tmp25 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx26 = getelementptr <4 x i8> addrspace(1)* %tmp25, i32 %tmp24		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp27 = load <4 x i8> addrspace(1)* %arrayidx26		; <<4 x i8>> [#uses=1]
	%tmp28 = extractelement <4 x i8> %tmp27, i32 2		; <i8> [#uses=1]
	%conv29 = uitofp i8 %tmp28 to float		; <float> [#uses=1]
	%2 = insertelement <4 x float> %1, float %conv29, i32 2		; <<4 x float>> [#uses=1]
	%tmp30 = load i32* %pos		; <i32> [#uses=1]
	%tmp31 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx32 = getelementptr <4 x i8> addrspace(1)* %tmp31, i32 %tmp30		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp33 = load <4 x i8> addrspace(1)* %arrayidx32		; <<4 x i8>> [#uses=1]
	%tmp34 = extractelement <4 x i8> %tmp33, i32 3		; <i8> [#uses=1]
	%conv35 = uitofp i8 %tmp34 to float		; <float> [#uses=1]
	%3 = insertelement <4 x float> %2, float %conv35, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %3, <4 x float>* %.compoundliteral
	%tmp36 = load <4 x float>* %.compoundliteral		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp36, <4 x float>* %xc
	%tmp38 = load float* %a0.addr		; <float> [#uses=1]
	%tmp39 = insertelement <4 x float> undef, float %tmp38, i32 0		; <<4 x float>> [#uses=2]
	%splat = shufflevector <4 x float> %tmp39, <4 x float> %tmp39, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp40 = load <4 x float>* %xc		; <<4 x float>> [#uses=1]
	%mul41 = fmul <4 x float> %splat, %tmp40		; <<4 x float>> [#uses=1]
	%tmp42 = load float* %a1.addr		; <float> [#uses=1]
	%tmp43 = insertelement <4 x float> undef, float %tmp42, i32 0		; <<4 x float>> [#uses=2]
	%splat44 = shufflevector <4 x float> %tmp43, <4 x float> %tmp43, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp45 = load <4 x float>* %xp		; <<4 x float>> [#uses=1]
	%mul46 = fmul <4 x float> %splat44, %tmp45		; <<4 x float>> [#uses=1]
	%add47 = fadd <4 x float> %mul41, %mul46		; <<4 x float>> [#uses=1]
	%tmp48 = load float* %b1.addr		; <float> [#uses=1]
	%tmp49 = insertelement <4 x float> undef, float %tmp48, i32 0		; <<4 x float>> [#uses=2]
	%splat50 = shufflevector <4 x float> %tmp49, <4 x float> %tmp49, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp51 = load <4 x float>* %yp		; <<4 x float>> [#uses=1]
	%mul52 = fmul <4 x float> %splat50, %tmp51		; <<4 x float>> [#uses=1]
	%sub = fsub <4 x float> %add47, %mul52		; <<4 x float>> [#uses=1]
	%tmp53 = load float* %b2.addr		; <float> [#uses=1]
	%tmp54 = insertelement <4 x float> undef, float %tmp53, i32 0		; <<4 x float>> [#uses=2]
	%splat55 = shufflevector <4 x float> %tmp54, <4 x float> %tmp54, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp56 = load <4 x float>* %yb		; <<4 x float>> [#uses=1]
	%mul57 = fmul <4 x float> %splat55, %tmp56		; <<4 x float>> [#uses=1]
	%sub58 = fsub <4 x float> %sub, %mul57		; <<4 x float>> [#uses=1]
	store <4 x float> %sub58, <4 x float>* %yc
	%tmp59 = load i32* %pos		; <i32> [#uses=1]
	%tmp60 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx61 = getelementptr <4 x i8> addrspace(1)* %tmp60, i32 %tmp59		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp63 = load <4 x float>* %yc		; <<4 x float>> [#uses=1]
	%tmp64 = extractelement <4 x float> %tmp63, i32 0		; <float> [#uses=1]
	%conv65 = fptoui float %tmp64 to i8		; <i8> [#uses=1]
	%4 = insertelement <4 x i8> undef, i8 %conv65, i32 0		; <<4 x i8>> [#uses=1]
	%tmp66 = load <4 x float>* %yc		; <<4 x float>> [#uses=1]
	%tmp67 = extractelement <4 x float> %tmp66, i32 1		; <float> [#uses=1]
	%conv68 = fptoui float %tmp67 to i8		; <i8> [#uses=1]
	%5 = insertelement <4 x i8> %4, i8 %conv68, i32 1		; <<4 x i8>> [#uses=1]
	%tmp69 = load <4 x float>* %yc		; <<4 x float>> [#uses=1]
	%tmp70 = extractelement <4 x float> %tmp69, i32 2		; <float> [#uses=1]
	%conv71 = fptoui float %tmp70 to i8		; <i8> [#uses=1]
	%6 = insertelement <4 x i8> %5, i8 %conv71, i32 2		; <<4 x i8>> [#uses=1]
	%tmp72 = load <4 x float>* %yc		; <<4 x float>> [#uses=1]
	%tmp73 = extractelement <4 x float> %tmp72, i32 3		; <float> [#uses=1]
	%conv74 = fptoui float %tmp73 to i8		; <i8> [#uses=1]
	%7 = insertelement <4 x i8> %6, i8 %conv74, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %7, <4 x i8>* %.compoundliteral62
	%tmp75 = load <4 x i8>* %.compoundliteral62		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp75, <4 x i8> addrspace(1)* %arrayidx61
	%tmp76 = load <4 x float>* %xc		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp76, <4 x float>* %xp
	%tmp77 = load <4 x float>* %yp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp77, <4 x float>* %yb
	%tmp78 = load <4 x float>* %yc		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp78, <4 x float>* %yp
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp79 = load i32* %y		; <i32> [#uses=1]
	%inc = add i32 %tmp79, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %y
	br label %for.cond

for.end:		; preds = %for.cond
	call void @barrier(i32 2)
	store <4 x float> zeroinitializer, <4 x float>* %xn
	store <4 x float> zeroinitializer, <4 x float>* %xa
	store <4 x float> zeroinitializer, <4 x float>* %yn
	store <4 x float> zeroinitializer, <4 x float>* %ya
	%tmp86 = load i32* %height.addr		; <i32> [#uses=1]
	%sub87 = sub i32 %tmp86, 1		; <i32> [#uses=1]
	store i32 %sub87, i32* %y85
	br label %for.cond88

for.cond88:		; preds = %for.inc203, %for.end
	%tmp89 = load i32* %y85		; <i32> [#uses=1]
	%cmp90 = icmp sgt i32 %tmp89, -1		; <i1> [#uses=1]
	br i1 %cmp90, label %for.body92, label %for.end205

for.body92:		; preds = %for.cond88
	%tmp95 = load i32* %x		; <i32> [#uses=1]
	%tmp96 = load i32* %y85		; <i32> [#uses=1]
	%tmp97 = load i32* %width.addr		; <i32> [#uses=1]
	%mul98 = mul i32 %tmp96, %tmp97		; <i32> [#uses=1]
	%add99 = add i32 %tmp95, %mul98		; <i32> [#uses=1]
	store i32 %add99, i32* %pos94
	%tmp103 = load i32* %pos94		; <i32> [#uses=1]
	%tmp104 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx105 = getelementptr <4 x i8> addrspace(1)* %tmp104, i32 %tmp103		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp106 = load <4 x i8> addrspace(1)* %arrayidx105		; <<4 x i8>> [#uses=1]
	%tmp107 = extractelement <4 x i8> %tmp106, i32 0		; <i8> [#uses=1]
	%conv108 = uitofp i8 %tmp107 to float		; <float> [#uses=1]
	%8 = insertelement <4 x float> undef, float %conv108, i32 0		; <<4 x float>> [#uses=1]
	%tmp109 = load i32* %pos94		; <i32> [#uses=1]
	%tmp110 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx111 = getelementptr <4 x i8> addrspace(1)* %tmp110, i32 %tmp109		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp112 = load <4 x i8> addrspace(1)* %arrayidx111		; <<4 x i8>> [#uses=1]
	%tmp113 = extractelement <4 x i8> %tmp112, i32 1		; <i8> [#uses=1]
	%conv114 = uitofp i8 %tmp113 to float		; <float> [#uses=1]
	%9 = insertelement <4 x float> %8, float %conv114, i32 1		; <<4 x float>> [#uses=1]
	%tmp115 = load i32* %pos94		; <i32> [#uses=1]
	%tmp116 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx117 = getelementptr <4 x i8> addrspace(1)* %tmp116, i32 %tmp115		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp118 = load <4 x i8> addrspace(1)* %arrayidx117		; <<4 x i8>> [#uses=1]
	%tmp119 = extractelement <4 x i8> %tmp118, i32 2		; <i8> [#uses=1]
	%conv120 = uitofp i8 %tmp119 to float		; <float> [#uses=1]
	%10 = insertelement <4 x float> %9, float %conv120, i32 2		; <<4 x float>> [#uses=1]
	%tmp121 = load i32* %pos94		; <i32> [#uses=1]
	%tmp122 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx123 = getelementptr <4 x i8> addrspace(1)* %tmp122, i32 %tmp121		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp124 = load <4 x i8> addrspace(1)* %arrayidx123		; <<4 x i8>> [#uses=1]
	%tmp125 = extractelement <4 x i8> %tmp124, i32 3		; <i8> [#uses=1]
	%conv126 = uitofp i8 %tmp125 to float		; <float> [#uses=1]
	%11 = insertelement <4 x float> %10, float %conv126, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %11, <4 x float>* %.compoundliteral102
	%tmp127 = load <4 x float>* %.compoundliteral102		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp127, <4 x float>* %xc101
	%tmp130 = load float* %a2.addr		; <float> [#uses=1]
	%tmp131 = insertelement <4 x float> undef, float %tmp130, i32 0		; <<4 x float>> [#uses=2]
	%splat132 = shufflevector <4 x float> %tmp131, <4 x float> %tmp131, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp133 = load <4 x float>* %xn		; <<4 x float>> [#uses=1]
	%mul134 = fmul <4 x float> %splat132, %tmp133		; <<4 x float>> [#uses=1]
	%tmp135 = load float* %a3.addr		; <float> [#uses=1]
	%tmp136 = insertelement <4 x float> undef, float %tmp135, i32 0		; <<4 x float>> [#uses=2]
	%splat137 = shufflevector <4 x float> %tmp136, <4 x float> %tmp136, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp138 = load <4 x float>* %xa		; <<4 x float>> [#uses=1]
	%mul139 = fmul <4 x float> %splat137, %tmp138		; <<4 x float>> [#uses=1]
	%add140 = fadd <4 x float> %mul134, %mul139		; <<4 x float>> [#uses=1]
	%tmp141 = load float* %b1.addr		; <float> [#uses=1]
	%tmp142 = insertelement <4 x float> undef, float %tmp141, i32 0		; <<4 x float>> [#uses=2]
	%splat143 = shufflevector <4 x float> %tmp142, <4 x float> %tmp142, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp144 = load <4 x float>* %yn		; <<4 x float>> [#uses=1]
	%mul145 = fmul <4 x float> %splat143, %tmp144		; <<4 x float>> [#uses=1]
	%sub146 = fsub <4 x float> %add140, %mul145		; <<4 x float>> [#uses=1]
	%tmp147 = load float* %b2.addr		; <float> [#uses=1]
	%tmp148 = insertelement <4 x float> undef, float %tmp147, i32 0		; <<4 x float>> [#uses=2]
	%splat149 = shufflevector <4 x float> %tmp148, <4 x float> %tmp148, <4 x i32> zeroinitializer		; <<4 x float>> [#uses=1]
	%tmp150 = load <4 x float>* %ya		; <<4 x float>> [#uses=1]
	%mul151 = fmul <4 x float> %splat149, %tmp150		; <<4 x float>> [#uses=1]
	%sub152 = fsub <4 x float> %sub146, %mul151		; <<4 x float>> [#uses=1]
	store <4 x float> %sub152, <4 x float>* %yc129
	%tmp153 = load <4 x float>* %xn		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp153, <4 x float>* %xa
	%tmp154 = load <4 x float>* %xc101		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp154, <4 x float>* %xn
	%tmp155 = load <4 x float>* %yn		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp155, <4 x float>* %ya
	%tmp156 = load <4 x float>* %yc129		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp156, <4 x float>* %yn
	%tmp159 = load i32* %pos94		; <i32> [#uses=1]
	%tmp160 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx161 = getelementptr <4 x i8> addrspace(1)* %tmp160, i32 %tmp159		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp162 = load <4 x i8> addrspace(1)* %arrayidx161		; <<4 x i8>> [#uses=1]
	%tmp163 = extractelement <4 x i8> %tmp162, i32 0		; <i8> [#uses=1]
	%conv164 = uitofp i8 %tmp163 to float		; <float> [#uses=1]
	%12 = insertelement <4 x float> undef, float %conv164, i32 0		; <<4 x float>> [#uses=1]
	%tmp165 = load i32* %pos94		; <i32> [#uses=1]
	%tmp166 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx167 = getelementptr <4 x i8> addrspace(1)* %tmp166, i32 %tmp165		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp168 = load <4 x i8> addrspace(1)* %arrayidx167		; <<4 x i8>> [#uses=1]
	%tmp169 = extractelement <4 x i8> %tmp168, i32 1		; <i8> [#uses=1]
	%conv170 = uitofp i8 %tmp169 to float		; <float> [#uses=1]
	%13 = insertelement <4 x float> %12, float %conv170, i32 1		; <<4 x float>> [#uses=1]
	%tmp171 = load i32* %pos94		; <i32> [#uses=1]
	%tmp172 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx173 = getelementptr <4 x i8> addrspace(1)* %tmp172, i32 %tmp171		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp174 = load <4 x i8> addrspace(1)* %arrayidx173		; <<4 x i8>> [#uses=1]
	%tmp175 = extractelement <4 x i8> %tmp174, i32 2		; <i8> [#uses=1]
	%conv176 = uitofp i8 %tmp175 to float		; <float> [#uses=1]
	%14 = insertelement <4 x float> %13, float %conv176, i32 2		; <<4 x float>> [#uses=1]
	%tmp177 = load i32* %pos94		; <i32> [#uses=1]
	%tmp178 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx179 = getelementptr <4 x i8> addrspace(1)* %tmp178, i32 %tmp177		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp180 = load <4 x i8> addrspace(1)* %arrayidx179		; <<4 x i8>> [#uses=1]
	%tmp181 = extractelement <4 x i8> %tmp180, i32 3		; <i8> [#uses=1]
	%conv182 = uitofp i8 %tmp181 to float		; <float> [#uses=1]
	%15 = insertelement <4 x float> %14, float %conv182, i32 3		; <<4 x float>> [#uses=1]
	store <4 x float> %15, <4 x float>* %.compoundliteral158
	%tmp183 = load <4 x float>* %.compoundliteral158		; <<4 x float>> [#uses=1]
	%tmp184 = load <4 x float>* %yc129		; <<4 x float>> [#uses=1]
	%add185 = fadd <4 x float> %tmp183, %tmp184		; <<4 x float>> [#uses=1]
	store <4 x float> %add185, <4 x float>* %temp
	%tmp186 = load i32* %pos94		; <i32> [#uses=1]
	%tmp187 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx188 = getelementptr <4 x i8> addrspace(1)* %tmp187, i32 %tmp186		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp190 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%tmp191 = extractelement <4 x float> %tmp190, i32 0		; <float> [#uses=1]
	%conv192 = fptoui float %tmp191 to i8		; <i8> [#uses=1]
	%16 = insertelement <4 x i8> undef, i8 %conv192, i32 0		; <<4 x i8>> [#uses=1]
	%tmp193 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%tmp194 = extractelement <4 x float> %tmp193, i32 1		; <float> [#uses=1]
	%conv195 = fptoui float %tmp194 to i8		; <i8> [#uses=1]
	%17 = insertelement <4 x i8> %16, i8 %conv195, i32 1		; <<4 x i8>> [#uses=1]
	%tmp196 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%tmp197 = extractelement <4 x float> %tmp196, i32 2		; <float> [#uses=1]
	%conv198 = fptoui float %tmp197 to i8		; <i8> [#uses=1]
	%18 = insertelement <4 x i8> %17, i8 %conv198, i32 2		; <<4 x i8>> [#uses=1]
	%tmp199 = load <4 x float>* %temp		; <<4 x float>> [#uses=1]
	%tmp200 = extractelement <4 x float> %tmp199, i32 3		; <float> [#uses=1]
	%conv201 = fptoui float %tmp200 to i8		; <i8> [#uses=1]
	%19 = insertelement <4 x i8> %18, i8 %conv201, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %19, <4 x i8>* %.compoundliteral189
	%tmp202 = load <4 x i8>* %.compoundliteral189		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp202, <4 x i8> addrspace(1)* %arrayidx188
	br label %for.inc203

for.inc203:		; preds = %for.body92
	%tmp204 = load i32* %y85		; <i32> [#uses=1]
	%dec = add i32 %tmp204, -1		; <i32> [#uses=1]
	store i32 %dec, i32* %y85
	br label %for.cond88

for.end205:		; preds = %for.cond88
	br label %return

return:		; preds = %for.end205, %if.then
	ret void
}
