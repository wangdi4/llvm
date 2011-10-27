; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlDCT.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@DCT_cllocal_inter = internal addrspace(3) global [64 x float] zeroinitializer		; <[64 x float] addrspace(3)*> [#uses=2]
@sgv = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [1 x i8*] [i8* bitcast ([64 x float] addrspace(3)* @DCT_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@DCT_VECTOR_cllocal_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer		; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@sgv1 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [1 x i8*] [i8* bitcast ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@DCT_VECTOR_DOT_cllocal_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer		; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@sgv4 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv5 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv6 = internal constant [1 x i8*] [i8* bitcast ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@sgv7 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv8 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv9 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv10 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv11 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv12 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [5 x %0] [%0 { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, ...)* @DCT to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([1 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32, ...)* @DCT_VECTOR to i8*), i8* getelementptr ([5 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([1 x i8*]* @lvgv3 to i8*), i32 0 }, %0 { i8* bitcast (void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32, ...)* @DCT_VECTOR_DOT to i8*), i8* getelementptr ([5 x i8]* @sgv4, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv5, i32 0, i32 0), i8* bitcast ([1 x i8*]* @lvgv6 to i8*), i32 0 }, %0 { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, ...)* @DCT_CPU to i8*), i8* getelementptr ([5 x i8]* @sgv7, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv8, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv9 to i8*), i32 0 }, %0 { i8* bitcast (void (float addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32, ...)* @DCT_CPU_VECTOR to i8*), i8* getelementptr ([5 x i8]* @sgv10, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv11, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv12 to i8*), i32 0 }], section "llvm.metadata"		; <[5 x %0]*> [#uses=0]

; CHECK: @DCT
define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=9]
	%input.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%dct.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=4]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=1]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=1]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=3]
	%k1 = alloca i32, align 4		; <i32*> [#uses=3]
	%k2 = alloca i32, align 4		; <i32*> [#uses=3]
	%n1 = alloca i32, align 4		; <i32*> [#uses=6]
	%n2 = alloca i32, align 4		; <i32*> [#uses=6]
	%idx = alloca i32, align 4		; <i32*> [#uses=19]
	%index1 = alloca i32, align 4		; <i32*> [#uses=7]
	%index2 = alloca i32, align 4		; <i32*> [#uses=7]
	%acc = alloca [8 x float], align 4		; <[8 x float]*> [#uses=27]
	%ind = alloca i32, align 4		; <i32*> [#uses=5]
	%ind100 = alloca i32, align 4		; <i32*> [#uses=5]
	%ind116 = alloca i32, align 4		; <i32*> [#uses=5]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store float addrspace(1)* %input, float addrspace(1)** %input.addr
	store float addrspace(1)* %dct, float addrspace(1)** %dct.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalIdx
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalIdy
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupIdx
	%call3 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %groupIdy
	%call4 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call4, i32* %i
	store i32 0, i32* %idx
	store i32 0, i32* %index1
	store i32 0, i32* %index2
	%.array = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array
	%.array5 = getelementptr [8 x float]* %acc, i32 0, i32 1		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array5
	%.array6 = getelementptr [8 x float]* %acc, i32 0, i32 2		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array6
	%.array7 = getelementptr [8 x float]* %acc, i32 0, i32 3		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array7
	%.array8 = getelementptr [8 x float]* %acc, i32 0, i32 4		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array8
	%.array9 = getelementptr [8 x float]* %acc, i32 0, i32 5		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array9
	%.array10 = getelementptr [8 x float]* %acc, i32 0, i32 6		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array10
	%.array11 = getelementptr [8 x float]* %acc, i32 0, i32 7		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array11
	%tmp = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp, i32* %k1
	%tmp12 = load i32* %k1		; <i32> [#uses=1]
	%mul = mul i32 %tmp12, 8		; <i32> [#uses=1]
	store i32 %mul, i32* %index1
	%tmp13 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul14 = mul i32 %tmp13, 8		; <i32> [#uses=1]
	%tmp15 = load i32* %width.addr		; <i32> [#uses=1]
	%mul16 = mul i32 %mul14, %tmp15		; <i32> [#uses=1]
	%tmp17 = load i32* %groupIdx		; <i32> [#uses=1]
	%mul18 = mul i32 %tmp17, 8		; <i32> [#uses=1]
	%add = add i32 %mul16, %mul18		; <i32> [#uses=1]
	store i32 %add, i32* %index2
	store i32 0, i32* %ind
	br label %for.cond

for.cond:		; preds = %for.inc45, %entry
	%tmp20 = load i32* %ind		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp20, 8		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end48

for.body:		; preds = %for.cond
	store i32 0, i32* %n1
	br label %for.cond21

for.cond21:		; preds = %for.inc, %for.body
	%tmp22 = load i32* %n1		; <i32> [#uses=1]
	%cmp23 = icmp ult i32 %tmp22, 8		; <i1> [#uses=1]
	br i1 %cmp23, label %for.body24, label %for.end

for.body24:		; preds = %for.cond21
	%tmp25 = load i32* %ind		; <i32> [#uses=1]
	%arraydecay = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx = getelementptr float* %arraydecay, i32 %tmp25		; <float*> [#uses=2]
	%tmp26 = load float* %arrayidx		; <float> [#uses=1]
	%tmp27 = load i32* %index1		; <i32> [#uses=1]
	%tmp28 = load i32* %n1		; <i32> [#uses=1]
	%add29 = add i32 %tmp27, %tmp28		; <i32> [#uses=1]
	%tmp30 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx31 = getelementptr float addrspace(1)* %tmp30, i32 %add29		; <float addrspace(1)*> [#uses=1]
	%tmp32 = load float addrspace(1)* %arrayidx31		; <float> [#uses=1]
	%tmp33 = load i32* %index2		; <i32> [#uses=1]
	%tmp34 = load i32* %n1		; <i32> [#uses=1]
	%add35 = add i32 %tmp33, %tmp34		; <i32> [#uses=1]
	%tmp36 = load float addrspace(1)** %input.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx37 = getelementptr float addrspace(1)* %tmp36, i32 %add35		; <float addrspace(1)*> [#uses=1]
	%tmp38 = load float addrspace(1)* %arrayidx37		; <float> [#uses=1]
	%mul39 = fmul float %tmp32, %tmp38		; <float> [#uses=1]
	%add40 = fadd float %tmp26, %mul39		; <float> [#uses=1]
	store float %add40, float* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body24
	%tmp41 = load i32* %n1		; <i32> [#uses=1]
	%inc = add i32 %tmp41, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %n1
	br label %for.cond21

for.end:		; preds = %for.cond21
	%tmp42 = load i32* %index2		; <i32> [#uses=1]
	%tmp43 = load i32* %width.addr		; <i32> [#uses=1]
	%add44 = add i32 %tmp42, %tmp43		; <i32> [#uses=1]
	store i32 %add44, i32* %index2
	br label %for.inc45

for.inc45:		; preds = %for.end
	%tmp46 = load i32* %ind		; <i32> [#uses=1]
	%inc47 = add i32 %tmp46, 1		; <i32> [#uses=1]
	store i32 %inc47, i32* %ind
	br label %for.cond

for.end48:		; preds = %for.cond
	%tmp49 = load i32* %k1		; <i32> [#uses=1]
	%mul50 = mul i32 %tmp49, 8		; <i32> [#uses=1]
	store i32 %mul50, i32* %idx
	%tmp51 = load i32* %idx		; <i32> [#uses=1]
	%add52 = add i32 %tmp51, 0		; <i32> [#uses=1]
	%arrayidx53 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add52		; <float addrspace(3)*> [#uses=1]
	%arraydecay54 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx55 = getelementptr float* %arraydecay54, i32 0		; <float*> [#uses=1]
	%tmp56 = load float* %arrayidx55		; <float> [#uses=1]
	store float %tmp56, float addrspace(3)* %arrayidx53
	%tmp57 = load i32* %idx		; <i32> [#uses=1]
	%add58 = add i32 %tmp57, 1		; <i32> [#uses=1]
	%arrayidx59 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add58		; <float addrspace(3)*> [#uses=1]
	%arraydecay60 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx61 = getelementptr float* %arraydecay60, i32 1		; <float*> [#uses=1]
	%tmp62 = load float* %arrayidx61		; <float> [#uses=1]
	store float %tmp62, float addrspace(3)* %arrayidx59
	%tmp63 = load i32* %idx		; <i32> [#uses=1]
	%add64 = add i32 %tmp63, 2		; <i32> [#uses=1]
	%arrayidx65 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add64		; <float addrspace(3)*> [#uses=1]
	%arraydecay66 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx67 = getelementptr float* %arraydecay66, i32 2		; <float*> [#uses=1]
	%tmp68 = load float* %arrayidx67		; <float> [#uses=1]
	store float %tmp68, float addrspace(3)* %arrayidx65
	%tmp69 = load i32* %idx		; <i32> [#uses=1]
	%add70 = add i32 %tmp69, 3		; <i32> [#uses=1]
	%arrayidx71 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add70		; <float addrspace(3)*> [#uses=1]
	%arraydecay72 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx73 = getelementptr float* %arraydecay72, i32 3		; <float*> [#uses=1]
	%tmp74 = load float* %arrayidx73		; <float> [#uses=1]
	store float %tmp74, float addrspace(3)* %arrayidx71
	%tmp75 = load i32* %idx		; <i32> [#uses=1]
	%add76 = add i32 %tmp75, 4		; <i32> [#uses=1]
	%arrayidx77 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add76		; <float addrspace(3)*> [#uses=1]
	%arraydecay78 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx79 = getelementptr float* %arraydecay78, i32 4		; <float*> [#uses=1]
	%tmp80 = load float* %arrayidx79		; <float> [#uses=1]
	store float %tmp80, float addrspace(3)* %arrayidx77
	%tmp81 = load i32* %idx		; <i32> [#uses=1]
	%add82 = add i32 %tmp81, 5		; <i32> [#uses=1]
	%arrayidx83 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add82		; <float addrspace(3)*> [#uses=1]
	%arraydecay84 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx85 = getelementptr float* %arraydecay84, i32 5		; <float*> [#uses=1]
	%tmp86 = load float* %arrayidx85		; <float> [#uses=1]
	store float %tmp86, float addrspace(3)* %arrayidx83
	%tmp87 = load i32* %idx		; <i32> [#uses=1]
	%add88 = add i32 %tmp87, 6		; <i32> [#uses=1]
	%arrayidx89 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add88		; <float addrspace(3)*> [#uses=1]
	%arraydecay90 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx91 = getelementptr float* %arraydecay90, i32 6		; <float*> [#uses=1]
	%tmp92 = load float* %arrayidx91		; <float> [#uses=1]
	store float %tmp92, float addrspace(3)* %arrayidx89
	%tmp93 = load i32* %idx		; <i32> [#uses=1]
	%add94 = add i32 %tmp93, 7		; <i32> [#uses=1]
	%arrayidx95 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add94		; <float addrspace(3)*> [#uses=1]
	%arraydecay96 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx97 = getelementptr float* %arraydecay96, i32 7		; <float*> [#uses=1]
	%tmp98 = load float* %arrayidx97		; <float> [#uses=1]
	store float %tmp98, float addrspace(3)* %arrayidx95
	call void @barrier(i32 1)
	store i32 0, i32* %ind100
	br label %for.cond101

for.cond101:		; preds = %for.inc108, %for.end48
	%tmp102 = load i32* %ind100		; <i32> [#uses=1]
	%cmp103 = icmp slt i32 %tmp102, 8		; <i1> [#uses=1]
	br i1 %cmp103, label %for.body104, label %for.end111

for.body104:		; preds = %for.cond101
	%tmp105 = load i32* %ind100		; <i32> [#uses=1]
	%arraydecay106 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx107 = getelementptr float* %arraydecay106, i32 %tmp105		; <float*> [#uses=1]
	store float 0.000000e+000, float* %arrayidx107
	br label %for.inc108

for.inc108:		; preds = %for.body104
	%tmp109 = load i32* %ind100		; <i32> [#uses=1]
	%inc110 = add i32 %tmp109, 1		; <i32> [#uses=1]
	store i32 %inc110, i32* %ind100
	br label %for.cond101

for.end111:		; preds = %for.cond101
	%tmp112 = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp112, i32* %k2
	store i32 0, i32* %index1
	%tmp113 = load i32* %k2		; <i32> [#uses=1]
	%mul114 = mul i32 %tmp113, 8		; <i32> [#uses=1]
	store i32 %mul114, i32* %index2
	store i32 0, i32* %ind116
	br label %for.cond117

for.cond117:		; preds = %for.inc148, %for.end111
	%tmp118 = load i32* %ind116		; <i32> [#uses=1]
	%cmp119 = icmp slt i32 %tmp118, 8		; <i1> [#uses=1]
	br i1 %cmp119, label %for.body120, label %for.end151

for.body120:		; preds = %for.cond117
	store i32 0, i32* %n2
	br label %for.cond121

for.cond121:		; preds = %for.inc142, %for.body120
	%tmp122 = load i32* %n2		; <i32> [#uses=1]
	%cmp123 = icmp ult i32 %tmp122, 8		; <i1> [#uses=1]
	br i1 %cmp123, label %for.body124, label %for.end145

for.body124:		; preds = %for.cond121
	%tmp125 = load i32* %ind116		; <i32> [#uses=1]
	%arraydecay126 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx127 = getelementptr float* %arraydecay126, i32 %tmp125		; <float*> [#uses=2]
	%tmp128 = load float* %arrayidx127		; <float> [#uses=1]
	%tmp129 = load i32* %index1		; <i32> [#uses=1]
	%tmp130 = load i32* %n2		; <i32> [#uses=1]
	%add131 = add i32 %tmp129, %tmp130		; <i32> [#uses=1]
	%arrayidx132 = getelementptr float addrspace(3)* getelementptr ([64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 0), i32 %add131		; <float addrspace(3)*> [#uses=1]
	%tmp133 = load float addrspace(3)* %arrayidx132		; <float> [#uses=1]
	%tmp134 = load i32* %index2		; <i32> [#uses=1]
	%tmp135 = load i32* %n2		; <i32> [#uses=1]
	%add136 = add i32 %tmp134, %tmp135		; <i32> [#uses=1]
	%tmp137 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx138 = getelementptr float addrspace(1)* %tmp137, i32 %add136		; <float addrspace(1)*> [#uses=1]
	%tmp139 = load float addrspace(1)* %arrayidx138		; <float> [#uses=1]
	%mul140 = fmul float %tmp133, %tmp139		; <float> [#uses=1]
	%add141 = fadd float %tmp128, %mul140		; <float> [#uses=1]
	store float %add141, float* %arrayidx127
	br label %for.inc142

for.inc142:		; preds = %for.body124
	%tmp143 = load i32* %n2		; <i32> [#uses=1]
	%inc144 = add i32 %tmp143, 1		; <i32> [#uses=1]
	store i32 %inc144, i32* %n2
	br label %for.cond121

for.end145:		; preds = %for.cond121
	%tmp146 = load i32* %index1		; <i32> [#uses=1]
	%add147 = add i32 %tmp146, 8		; <i32> [#uses=1]
	store i32 %add147, i32* %index1
	br label %for.inc148

for.inc148:		; preds = %for.end145
	%tmp149 = load i32* %ind116		; <i32> [#uses=1]
	%inc150 = add i32 %tmp149, 1		; <i32> [#uses=1]
	store i32 %inc150, i32* %ind116
	br label %for.cond117

for.end151:		; preds = %for.cond117
	%tmp152 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul153 = mul i32 %tmp152, 8		; <i32> [#uses=1]
	%tmp154 = load i32* %k2		; <i32> [#uses=1]
	%add155 = add i32 %mul153, %tmp154		; <i32> [#uses=1]
	%tmp156 = load i32* %width.addr		; <i32> [#uses=1]
	%mul157 = mul i32 %add155, %tmp156		; <i32> [#uses=1]
	%tmp158 = load i32* %groupIdx		; <i32> [#uses=1]
	%mul159 = mul i32 %tmp158, 8		; <i32> [#uses=1]
	%add160 = add i32 %mul157, %mul159		; <i32> [#uses=1]
	store i32 %add160, i32* %idx
	%tmp161 = load i32* %idx		; <i32> [#uses=1]
	%add162 = add i32 %tmp161, 0		; <i32> [#uses=1]
	%tmp163 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx164 = getelementptr float addrspace(1)* %tmp163, i32 %add162		; <float addrspace(1)*> [#uses=1]
	%arraydecay165 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx166 = getelementptr float* %arraydecay165, i32 0		; <float*> [#uses=1]
	%tmp167 = load float* %arrayidx166		; <float> [#uses=1]
	store float %tmp167, float addrspace(1)* %arrayidx164
	%tmp168 = load i32* %idx		; <i32> [#uses=1]
	%add169 = add i32 %tmp168, 1		; <i32> [#uses=1]
	%tmp170 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx171 = getelementptr float addrspace(1)* %tmp170, i32 %add169		; <float addrspace(1)*> [#uses=1]
	%arraydecay172 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx173 = getelementptr float* %arraydecay172, i32 1		; <float*> [#uses=1]
	%tmp174 = load float* %arrayidx173		; <float> [#uses=1]
	store float %tmp174, float addrspace(1)* %arrayidx171
	%tmp175 = load i32* %idx		; <i32> [#uses=1]
	%add176 = add i32 %tmp175, 2		; <i32> [#uses=1]
	%tmp177 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx178 = getelementptr float addrspace(1)* %tmp177, i32 %add176		; <float addrspace(1)*> [#uses=1]
	%arraydecay179 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx180 = getelementptr float* %arraydecay179, i32 2		; <float*> [#uses=1]
	%tmp181 = load float* %arrayidx180		; <float> [#uses=1]
	store float %tmp181, float addrspace(1)* %arrayidx178
	%tmp182 = load i32* %idx		; <i32> [#uses=1]
	%add183 = add i32 %tmp182, 3		; <i32> [#uses=1]
	%tmp184 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx185 = getelementptr float addrspace(1)* %tmp184, i32 %add183		; <float addrspace(1)*> [#uses=1]
	%arraydecay186 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx187 = getelementptr float* %arraydecay186, i32 3		; <float*> [#uses=1]
	%tmp188 = load float* %arrayidx187		; <float> [#uses=1]
	store float %tmp188, float addrspace(1)* %arrayidx185
	%tmp189 = load i32* %idx		; <i32> [#uses=1]
	%add190 = add i32 %tmp189, 4		; <i32> [#uses=1]
	%tmp191 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx192 = getelementptr float addrspace(1)* %tmp191, i32 %add190		; <float addrspace(1)*> [#uses=1]
	%arraydecay193 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx194 = getelementptr float* %arraydecay193, i32 4		; <float*> [#uses=1]
	%tmp195 = load float* %arrayidx194		; <float> [#uses=1]
	store float %tmp195, float addrspace(1)* %arrayidx192
	%tmp196 = load i32* %idx		; <i32> [#uses=1]
	%add197 = add i32 %tmp196, 5		; <i32> [#uses=1]
	%tmp198 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx199 = getelementptr float addrspace(1)* %tmp198, i32 %add197		; <float addrspace(1)*> [#uses=1]
	%arraydecay200 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx201 = getelementptr float* %arraydecay200, i32 5		; <float*> [#uses=1]
	%tmp202 = load float* %arrayidx201		; <float> [#uses=1]
	store float %tmp202, float addrspace(1)* %arrayidx199
	%tmp203 = load i32* %idx		; <i32> [#uses=1]
	%add204 = add i32 %tmp203, 6		; <i32> [#uses=1]
	%tmp205 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx206 = getelementptr float addrspace(1)* %tmp205, i32 %add204		; <float addrspace(1)*> [#uses=1]
	%arraydecay207 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx208 = getelementptr float* %arraydecay207, i32 6		; <float*> [#uses=1]
	%tmp209 = load float* %arrayidx208		; <float> [#uses=1]
	store float %tmp209, float addrspace(1)* %arrayidx206
	%tmp210 = load i32* %idx		; <i32> [#uses=1]
	%add211 = add i32 %tmp210, 7		; <i32> [#uses=1]
	%tmp212 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx213 = getelementptr float addrspace(1)* %tmp212, i32 %add211		; <float addrspace(1)*> [#uses=1]
	%arraydecay214 = getelementptr [8 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx215 = getelementptr float* %arraydecay214, i32 7		; <float*> [#uses=1]
	%tmp216 = load float* %arrayidx215		; <float> [#uses=1]
	store float %tmp216, float addrspace(1)* %arrayidx213
	ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

define void @DCT_VECTOR(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
	%output.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=2]
	%input.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=9]
	%dct.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=17]
	%width.addr = alloca i32		; <i32*> [#uses=2]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=1]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=1]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=3]
	%k1 = alloca i32, align 4		; <i32*> [#uses=3]
	%k2 = alloca i32, align 4		; <i32*> [#uses=3]
	%n1 = alloca i32, align 4		; <i32*> [#uses=0]
	%n2 = alloca i32, align 4		; <i32*> [#uses=0]
	%idx = alloca i32, align 4		; <i32*> [#uses=5]
	%acc = alloca <8 x float>, align 32		; <<8 x float>*> [#uses=35]
	%temp = alloca <8 x float>, align 32		; <<8 x float>*> [#uses=144]
	%step = alloca i32, align 4		; <i32*> [#uses=10]
	%index1 = alloca i32, align 4		; <i32*> [#uses=9]
	%index2 = alloca i32, align 4		; <i32*> [#uses=32]
	store <8 x float> addrspace(1)* %output, <8 x float> addrspace(1)** %output.addr
	store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
	store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalIdx
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalIdy
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupIdx
	%call3 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %groupIdy
	%call4 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call4, i32* %i
	store i32 0, i32* %idx
	store <8 x float> zeroinitializer, <8 x float>* %acc
	%tmp = load i32* %width.addr		; <i32> [#uses=1]
	%div = udiv i32 %tmp, 8		; <i32> [#uses=1]
	store i32 %div, i32* %step
	%tmp5 = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp5, i32* %k1
	%tmp7 = load i32* %k1		; <i32> [#uses=1]
	store i32 %tmp7, i32* %index1
	%tmp9 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul = mul i32 %tmp9, 8		; <i32> [#uses=1]
	%tmp10 = load i32* %step		; <i32> [#uses=1]
	%mul11 = mul i32 %mul, %tmp10		; <i32> [#uses=1]
	%tmp12 = load i32* %groupIdx		; <i32> [#uses=1]
	%add = add i32 %mul11, %tmp12		; <i32> [#uses=1]
	store i32 %add, i32* %index2
	%tmp13 = load i32* %index1		; <i32> [#uses=1]
	%tmp14 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <8 x float> addrspace(1)* %tmp14, i32 %tmp13		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp15 = load <8 x float> addrspace(1)* %arrayidx		; <<8 x float>> [#uses=1]
	%tmp16 = load i32* %index2		; <i32> [#uses=1]
	%tmp17 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx18 = getelementptr <8 x float> addrspace(1)* %tmp17, i32 %tmp16		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp19 = load <8 x float> addrspace(1)* %arrayidx18		; <<8 x float>> [#uses=1]
	%mul20 = fmul <8 x float> %tmp15, %tmp19		; <<8 x float>> [#uses=1]
	store <8 x float> %mul20, <8 x float>* %temp
	%tmp21 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp22 = extractelement <8 x float> %tmp21, i32 0		; <float> [#uses=1]
	%tmp23 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp24 = extractelement <8 x float> %tmp23, i32 1		; <float> [#uses=1]
	%add25 = fadd float %tmp22, %tmp24		; <float> [#uses=1]
	%tmp26 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp27 = extractelement <8 x float> %tmp26, i32 2		; <float> [#uses=1]
	%add28 = fadd float %add25, %tmp27		; <float> [#uses=1]
	%tmp29 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp30 = extractelement <8 x float> %tmp29, i32 3		; <float> [#uses=1]
	%add31 = fadd float %add28, %tmp30		; <float> [#uses=1]
	%tmp32 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp33 = extractelement <8 x float> %tmp32, i32 4		; <float> [#uses=1]
	%add34 = fadd float %add31, %tmp33		; <float> [#uses=1]
	%tmp35 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp36 = extractelement <8 x float> %tmp35, i32 5		; <float> [#uses=1]
	%add37 = fadd float %add34, %tmp36		; <float> [#uses=1]
	%tmp38 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp39 = extractelement <8 x float> %tmp38, i32 6		; <float> [#uses=1]
	%add40 = fadd float %add37, %tmp39		; <float> [#uses=1]
	%tmp41 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp42 = extractelement <8 x float> %tmp41, i32 7		; <float> [#uses=1]
	%add43 = fadd float %add40, %tmp42		; <float> [#uses=1]
	%tmp44 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp45 = insertelement <8 x float> %tmp44, float %add43, i32 0		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp45, <8 x float>* %acc
	%tmp46 = load i32* %index2		; <i32> [#uses=1]
	%tmp47 = load i32* %step		; <i32> [#uses=1]
	%add48 = add i32 %tmp46, %tmp47		; <i32> [#uses=1]
	store i32 %add48, i32* %index2
	%tmp49 = load i32* %index1		; <i32> [#uses=1]
	%tmp50 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx51 = getelementptr <8 x float> addrspace(1)* %tmp50, i32 %tmp49		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp52 = load <8 x float> addrspace(1)* %arrayidx51		; <<8 x float>> [#uses=1]
	%tmp53 = load i32* %index2		; <i32> [#uses=1]
	%tmp54 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx55 = getelementptr <8 x float> addrspace(1)* %tmp54, i32 %tmp53		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp56 = load <8 x float> addrspace(1)* %arrayidx55		; <<8 x float>> [#uses=1]
	%mul57 = fmul <8 x float> %tmp52, %tmp56		; <<8 x float>> [#uses=1]
	store <8 x float> %mul57, <8 x float>* %temp
	%tmp58 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp59 = extractelement <8 x float> %tmp58, i32 0		; <float> [#uses=1]
	%tmp60 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp61 = extractelement <8 x float> %tmp60, i32 1		; <float> [#uses=1]
	%add62 = fadd float %tmp59, %tmp61		; <float> [#uses=1]
	%tmp63 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp64 = extractelement <8 x float> %tmp63, i32 2		; <float> [#uses=1]
	%add65 = fadd float %add62, %tmp64		; <float> [#uses=1]
	%tmp66 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp67 = extractelement <8 x float> %tmp66, i32 3		; <float> [#uses=1]
	%add68 = fadd float %add65, %tmp67		; <float> [#uses=1]
	%tmp69 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp70 = extractelement <8 x float> %tmp69, i32 4		; <float> [#uses=1]
	%add71 = fadd float %add68, %tmp70		; <float> [#uses=1]
	%tmp72 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp73 = extractelement <8 x float> %tmp72, i32 5		; <float> [#uses=1]
	%add74 = fadd float %add71, %tmp73		; <float> [#uses=1]
	%tmp75 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp76 = extractelement <8 x float> %tmp75, i32 6		; <float> [#uses=1]
	%add77 = fadd float %add74, %tmp76		; <float> [#uses=1]
	%tmp78 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp79 = extractelement <8 x float> %tmp78, i32 7		; <float> [#uses=1]
	%add80 = fadd float %add77, %tmp79		; <float> [#uses=1]
	%tmp81 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp82 = insertelement <8 x float> %tmp81, float %add80, i32 1		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp82, <8 x float>* %acc
	%tmp83 = load i32* %index2		; <i32> [#uses=1]
	%tmp84 = load i32* %step		; <i32> [#uses=1]
	%add85 = add i32 %tmp83, %tmp84		; <i32> [#uses=1]
	store i32 %add85, i32* %index2
	%tmp86 = load i32* %index1		; <i32> [#uses=1]
	%tmp87 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx88 = getelementptr <8 x float> addrspace(1)* %tmp87, i32 %tmp86		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp89 = load <8 x float> addrspace(1)* %arrayidx88		; <<8 x float>> [#uses=1]
	%tmp90 = load i32* %index2		; <i32> [#uses=1]
	%tmp91 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx92 = getelementptr <8 x float> addrspace(1)* %tmp91, i32 %tmp90		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp93 = load <8 x float> addrspace(1)* %arrayidx92		; <<8 x float>> [#uses=1]
	%mul94 = fmul <8 x float> %tmp89, %tmp93		; <<8 x float>> [#uses=1]
	store <8 x float> %mul94, <8 x float>* %temp
	%tmp95 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp96 = extractelement <8 x float> %tmp95, i32 0		; <float> [#uses=1]
	%tmp97 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp98 = extractelement <8 x float> %tmp97, i32 1		; <float> [#uses=1]
	%add99 = fadd float %tmp96, %tmp98		; <float> [#uses=1]
	%tmp100 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp101 = extractelement <8 x float> %tmp100, i32 2		; <float> [#uses=1]
	%add102 = fadd float %add99, %tmp101		; <float> [#uses=1]
	%tmp103 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp104 = extractelement <8 x float> %tmp103, i32 3		; <float> [#uses=1]
	%add105 = fadd float %add102, %tmp104		; <float> [#uses=1]
	%tmp106 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp107 = extractelement <8 x float> %tmp106, i32 4		; <float> [#uses=1]
	%add108 = fadd float %add105, %tmp107		; <float> [#uses=1]
	%tmp109 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp110 = extractelement <8 x float> %tmp109, i32 5		; <float> [#uses=1]
	%add111 = fadd float %add108, %tmp110		; <float> [#uses=1]
	%tmp112 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp113 = extractelement <8 x float> %tmp112, i32 6		; <float> [#uses=1]
	%add114 = fadd float %add111, %tmp113		; <float> [#uses=1]
	%tmp115 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp116 = extractelement <8 x float> %tmp115, i32 7		; <float> [#uses=1]
	%add117 = fadd float %add114, %tmp116		; <float> [#uses=1]
	%tmp118 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp119 = insertelement <8 x float> %tmp118, float %add117, i32 2		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp119, <8 x float>* %acc
	%tmp120 = load i32* %index2		; <i32> [#uses=1]
	%tmp121 = load i32* %step		; <i32> [#uses=1]
	%add122 = add i32 %tmp120, %tmp121		; <i32> [#uses=1]
	store i32 %add122, i32* %index2
	%tmp123 = load i32* %index1		; <i32> [#uses=1]
	%tmp124 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx125 = getelementptr <8 x float> addrspace(1)* %tmp124, i32 %tmp123		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp126 = load <8 x float> addrspace(1)* %arrayidx125		; <<8 x float>> [#uses=1]
	%tmp127 = load i32* %index2		; <i32> [#uses=1]
	%tmp128 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx129 = getelementptr <8 x float> addrspace(1)* %tmp128, i32 %tmp127		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp130 = load <8 x float> addrspace(1)* %arrayidx129		; <<8 x float>> [#uses=1]
	%mul131 = fmul <8 x float> %tmp126, %tmp130		; <<8 x float>> [#uses=1]
	store <8 x float> %mul131, <8 x float>* %temp
	%tmp132 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp133 = extractelement <8 x float> %tmp132, i32 0		; <float> [#uses=1]
	%tmp134 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp135 = extractelement <8 x float> %tmp134, i32 1		; <float> [#uses=1]
	%add136 = fadd float %tmp133, %tmp135		; <float> [#uses=1]
	%tmp137 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp138 = extractelement <8 x float> %tmp137, i32 2		; <float> [#uses=1]
	%add139 = fadd float %add136, %tmp138		; <float> [#uses=1]
	%tmp140 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp141 = extractelement <8 x float> %tmp140, i32 3		; <float> [#uses=1]
	%add142 = fadd float %add139, %tmp141		; <float> [#uses=1]
	%tmp143 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp144 = extractelement <8 x float> %tmp143, i32 4		; <float> [#uses=1]
	%add145 = fadd float %add142, %tmp144		; <float> [#uses=1]
	%tmp146 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp147 = extractelement <8 x float> %tmp146, i32 5		; <float> [#uses=1]
	%add148 = fadd float %add145, %tmp147		; <float> [#uses=1]
	%tmp149 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp150 = extractelement <8 x float> %tmp149, i32 6		; <float> [#uses=1]
	%add151 = fadd float %add148, %tmp150		; <float> [#uses=1]
	%tmp152 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp153 = extractelement <8 x float> %tmp152, i32 7		; <float> [#uses=1]
	%add154 = fadd float %add151, %tmp153		; <float> [#uses=1]
	%tmp155 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp156 = insertelement <8 x float> %tmp155, float %add154, i32 3		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp156, <8 x float>* %acc
	%tmp157 = load i32* %index2		; <i32> [#uses=1]
	%tmp158 = load i32* %step		; <i32> [#uses=1]
	%add159 = add i32 %tmp157, %tmp158		; <i32> [#uses=1]
	store i32 %add159, i32* %index2
	%tmp160 = load i32* %index1		; <i32> [#uses=1]
	%tmp161 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx162 = getelementptr <8 x float> addrspace(1)* %tmp161, i32 %tmp160		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp163 = load <8 x float> addrspace(1)* %arrayidx162		; <<8 x float>> [#uses=1]
	%tmp164 = load i32* %index2		; <i32> [#uses=1]
	%tmp165 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx166 = getelementptr <8 x float> addrspace(1)* %tmp165, i32 %tmp164		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp167 = load <8 x float> addrspace(1)* %arrayidx166		; <<8 x float>> [#uses=1]
	%mul168 = fmul <8 x float> %tmp163, %tmp167		; <<8 x float>> [#uses=1]
	store <8 x float> %mul168, <8 x float>* %temp
	%tmp169 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp170 = extractelement <8 x float> %tmp169, i32 0		; <float> [#uses=1]
	%tmp171 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp172 = extractelement <8 x float> %tmp171, i32 1		; <float> [#uses=1]
	%add173 = fadd float %tmp170, %tmp172		; <float> [#uses=1]
	%tmp174 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp175 = extractelement <8 x float> %tmp174, i32 2		; <float> [#uses=1]
	%add176 = fadd float %add173, %tmp175		; <float> [#uses=1]
	%tmp177 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp178 = extractelement <8 x float> %tmp177, i32 3		; <float> [#uses=1]
	%add179 = fadd float %add176, %tmp178		; <float> [#uses=1]
	%tmp180 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp181 = extractelement <8 x float> %tmp180, i32 4		; <float> [#uses=1]
	%add182 = fadd float %add179, %tmp181		; <float> [#uses=1]
	%tmp183 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp184 = extractelement <8 x float> %tmp183, i32 5		; <float> [#uses=1]
	%add185 = fadd float %add182, %tmp184		; <float> [#uses=1]
	%tmp186 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp187 = extractelement <8 x float> %tmp186, i32 6		; <float> [#uses=1]
	%add188 = fadd float %add185, %tmp187		; <float> [#uses=1]
	%tmp189 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp190 = extractelement <8 x float> %tmp189, i32 7		; <float> [#uses=1]
	%add191 = fadd float %add188, %tmp190		; <float> [#uses=1]
	%tmp192 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp193 = insertelement <8 x float> %tmp192, float %add191, i32 4		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp193, <8 x float>* %acc
	%tmp194 = load i32* %index2		; <i32> [#uses=1]
	%tmp195 = load i32* %step		; <i32> [#uses=1]
	%add196 = add i32 %tmp194, %tmp195		; <i32> [#uses=1]
	store i32 %add196, i32* %index2
	%tmp197 = load i32* %index1		; <i32> [#uses=1]
	%tmp198 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx199 = getelementptr <8 x float> addrspace(1)* %tmp198, i32 %tmp197		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp200 = load <8 x float> addrspace(1)* %arrayidx199		; <<8 x float>> [#uses=1]
	%tmp201 = load i32* %index2		; <i32> [#uses=1]
	%tmp202 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx203 = getelementptr <8 x float> addrspace(1)* %tmp202, i32 %tmp201		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp204 = load <8 x float> addrspace(1)* %arrayidx203		; <<8 x float>> [#uses=1]
	%mul205 = fmul <8 x float> %tmp200, %tmp204		; <<8 x float>> [#uses=1]
	store <8 x float> %mul205, <8 x float>* %temp
	%tmp206 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp207 = extractelement <8 x float> %tmp206, i32 0		; <float> [#uses=1]
	%tmp208 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp209 = extractelement <8 x float> %tmp208, i32 1		; <float> [#uses=1]
	%add210 = fadd float %tmp207, %tmp209		; <float> [#uses=1]
	%tmp211 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp212 = extractelement <8 x float> %tmp211, i32 2		; <float> [#uses=1]
	%add213 = fadd float %add210, %tmp212		; <float> [#uses=1]
	%tmp214 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp215 = extractelement <8 x float> %tmp214, i32 3		; <float> [#uses=1]
	%add216 = fadd float %add213, %tmp215		; <float> [#uses=1]
	%tmp217 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp218 = extractelement <8 x float> %tmp217, i32 4		; <float> [#uses=1]
	%add219 = fadd float %add216, %tmp218		; <float> [#uses=1]
	%tmp220 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp221 = extractelement <8 x float> %tmp220, i32 5		; <float> [#uses=1]
	%add222 = fadd float %add219, %tmp221		; <float> [#uses=1]
	%tmp223 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp224 = extractelement <8 x float> %tmp223, i32 6		; <float> [#uses=1]
	%add225 = fadd float %add222, %tmp224		; <float> [#uses=1]
	%tmp226 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp227 = extractelement <8 x float> %tmp226, i32 7		; <float> [#uses=1]
	%add228 = fadd float %add225, %tmp227		; <float> [#uses=1]
	%tmp229 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp230 = insertelement <8 x float> %tmp229, float %add228, i32 5		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp230, <8 x float>* %acc
	%tmp231 = load i32* %index2		; <i32> [#uses=1]
	%tmp232 = load i32* %step		; <i32> [#uses=1]
	%add233 = add i32 %tmp231, %tmp232		; <i32> [#uses=1]
	store i32 %add233, i32* %index2
	%tmp234 = load i32* %index1		; <i32> [#uses=1]
	%tmp235 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx236 = getelementptr <8 x float> addrspace(1)* %tmp235, i32 %tmp234		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp237 = load <8 x float> addrspace(1)* %arrayidx236		; <<8 x float>> [#uses=1]
	%tmp238 = load i32* %index2		; <i32> [#uses=1]
	%tmp239 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx240 = getelementptr <8 x float> addrspace(1)* %tmp239, i32 %tmp238		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp241 = load <8 x float> addrspace(1)* %arrayidx240		; <<8 x float>> [#uses=1]
	%mul242 = fmul <8 x float> %tmp237, %tmp241		; <<8 x float>> [#uses=1]
	store <8 x float> %mul242, <8 x float>* %temp
	%tmp243 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp244 = extractelement <8 x float> %tmp243, i32 0		; <float> [#uses=1]
	%tmp245 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp246 = extractelement <8 x float> %tmp245, i32 1		; <float> [#uses=1]
	%add247 = fadd float %tmp244, %tmp246		; <float> [#uses=1]
	%tmp248 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp249 = extractelement <8 x float> %tmp248, i32 2		; <float> [#uses=1]
	%add250 = fadd float %add247, %tmp249		; <float> [#uses=1]
	%tmp251 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp252 = extractelement <8 x float> %tmp251, i32 3		; <float> [#uses=1]
	%add253 = fadd float %add250, %tmp252		; <float> [#uses=1]
	%tmp254 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp255 = extractelement <8 x float> %tmp254, i32 4		; <float> [#uses=1]
	%add256 = fadd float %add253, %tmp255		; <float> [#uses=1]
	%tmp257 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp258 = extractelement <8 x float> %tmp257, i32 5		; <float> [#uses=1]
	%add259 = fadd float %add256, %tmp258		; <float> [#uses=1]
	%tmp260 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp261 = extractelement <8 x float> %tmp260, i32 6		; <float> [#uses=1]
	%add262 = fadd float %add259, %tmp261		; <float> [#uses=1]
	%tmp263 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp264 = extractelement <8 x float> %tmp263, i32 7		; <float> [#uses=1]
	%add265 = fadd float %add262, %tmp264		; <float> [#uses=1]
	%tmp266 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp267 = insertelement <8 x float> %tmp266, float %add265, i32 6		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp267, <8 x float>* %acc
	%tmp268 = load i32* %index2		; <i32> [#uses=1]
	%tmp269 = load i32* %step		; <i32> [#uses=1]
	%add270 = add i32 %tmp268, %tmp269		; <i32> [#uses=1]
	store i32 %add270, i32* %index2
	%tmp271 = load i32* %index1		; <i32> [#uses=1]
	%tmp272 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx273 = getelementptr <8 x float> addrspace(1)* %tmp272, i32 %tmp271		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp274 = load <8 x float> addrspace(1)* %arrayidx273		; <<8 x float>> [#uses=1]
	%tmp275 = load i32* %index2		; <i32> [#uses=1]
	%tmp276 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx277 = getelementptr <8 x float> addrspace(1)* %tmp276, i32 %tmp275		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp278 = load <8 x float> addrspace(1)* %arrayidx277		; <<8 x float>> [#uses=1]
	%mul279 = fmul <8 x float> %tmp274, %tmp278		; <<8 x float>> [#uses=1]
	store <8 x float> %mul279, <8 x float>* %temp
	%tmp280 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp281 = extractelement <8 x float> %tmp280, i32 0		; <float> [#uses=1]
	%tmp282 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp283 = extractelement <8 x float> %tmp282, i32 1		; <float> [#uses=1]
	%add284 = fadd float %tmp281, %tmp283		; <float> [#uses=1]
	%tmp285 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp286 = extractelement <8 x float> %tmp285, i32 2		; <float> [#uses=1]
	%add287 = fadd float %add284, %tmp286		; <float> [#uses=1]
	%tmp288 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp289 = extractelement <8 x float> %tmp288, i32 3		; <float> [#uses=1]
	%add290 = fadd float %add287, %tmp289		; <float> [#uses=1]
	%tmp291 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp292 = extractelement <8 x float> %tmp291, i32 4		; <float> [#uses=1]
	%add293 = fadd float %add290, %tmp292		; <float> [#uses=1]
	%tmp294 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp295 = extractelement <8 x float> %tmp294, i32 5		; <float> [#uses=1]
	%add296 = fadd float %add293, %tmp295		; <float> [#uses=1]
	%tmp297 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp298 = extractelement <8 x float> %tmp297, i32 6		; <float> [#uses=1]
	%add299 = fadd float %add296, %tmp298		; <float> [#uses=1]
	%tmp300 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp301 = extractelement <8 x float> %tmp300, i32 7		; <float> [#uses=1]
	%add302 = fadd float %add299, %tmp301		; <float> [#uses=1]
	%tmp303 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp304 = insertelement <8 x float> %tmp303, float %add302, i32 7		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp304, <8 x float>* %acc
	%tmp305 = load i32* %k1		; <i32> [#uses=1]
	store i32 %tmp305, i32* %idx
	%tmp306 = load i32* %idx		; <i32> [#uses=1]
	%arrayidx307 = getelementptr <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 0), i32 %tmp306		; <<8 x float> addrspace(3)*> [#uses=1]
	%tmp308 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp308, <8 x float> addrspace(3)* %arrayidx307
	call void @barrier(i32 1)
	%tmp309 = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp309, i32* %k2
	%tmp310 = load i32* %k2		; <i32> [#uses=1]
	store i32 %tmp310, i32* %index2
	%tmp311 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 0)		; <<8 x float>> [#uses=1]
	%tmp312 = load i32* %index2		; <i32> [#uses=1]
	%tmp313 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx314 = getelementptr <8 x float> addrspace(1)* %tmp313, i32 %tmp312		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp315 = load <8 x float> addrspace(1)* %arrayidx314		; <<8 x float>> [#uses=1]
	%mul316 = fmul <8 x float> %tmp311, %tmp315		; <<8 x float>> [#uses=1]
	store <8 x float> %mul316, <8 x float>* %temp
	%tmp317 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp318 = extractelement <8 x float> %tmp317, i32 0		; <float> [#uses=1]
	%tmp319 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp320 = extractelement <8 x float> %tmp319, i32 1		; <float> [#uses=1]
	%add321 = fadd float %tmp318, %tmp320		; <float> [#uses=1]
	%tmp322 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp323 = extractelement <8 x float> %tmp322, i32 2		; <float> [#uses=1]
	%add324 = fadd float %add321, %tmp323		; <float> [#uses=1]
	%tmp325 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp326 = extractelement <8 x float> %tmp325, i32 3		; <float> [#uses=1]
	%add327 = fadd float %add324, %tmp326		; <float> [#uses=1]
	%tmp328 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp329 = extractelement <8 x float> %tmp328, i32 4		; <float> [#uses=1]
	%add330 = fadd float %add327, %tmp329		; <float> [#uses=1]
	%tmp331 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp332 = extractelement <8 x float> %tmp331, i32 5		; <float> [#uses=1]
	%add333 = fadd float %add330, %tmp332		; <float> [#uses=1]
	%tmp334 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp335 = extractelement <8 x float> %tmp334, i32 6		; <float> [#uses=1]
	%add336 = fadd float %add333, %tmp335		; <float> [#uses=1]
	%tmp337 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp338 = extractelement <8 x float> %tmp337, i32 7		; <float> [#uses=1]
	%add339 = fadd float %add336, %tmp338		; <float> [#uses=1]
	%tmp340 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp341 = insertelement <8 x float> %tmp340, float %add339, i32 0		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp341, <8 x float>* %acc
	%tmp342 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 1)		; <<8 x float>> [#uses=1]
	%tmp343 = load i32* %index2		; <i32> [#uses=1]
	%tmp344 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx345 = getelementptr <8 x float> addrspace(1)* %tmp344, i32 %tmp343		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp346 = load <8 x float> addrspace(1)* %arrayidx345		; <<8 x float>> [#uses=1]
	%mul347 = fmul <8 x float> %tmp342, %tmp346		; <<8 x float>> [#uses=1]
	store <8 x float> %mul347, <8 x float>* %temp
	%tmp348 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp349 = extractelement <8 x float> %tmp348, i32 0		; <float> [#uses=1]
	%tmp350 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp351 = extractelement <8 x float> %tmp350, i32 1		; <float> [#uses=1]
	%add352 = fadd float %tmp349, %tmp351		; <float> [#uses=1]
	%tmp353 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp354 = extractelement <8 x float> %tmp353, i32 2		; <float> [#uses=1]
	%add355 = fadd float %add352, %tmp354		; <float> [#uses=1]
	%tmp356 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp357 = extractelement <8 x float> %tmp356, i32 3		; <float> [#uses=1]
	%add358 = fadd float %add355, %tmp357		; <float> [#uses=1]
	%tmp359 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp360 = extractelement <8 x float> %tmp359, i32 4		; <float> [#uses=1]
	%add361 = fadd float %add358, %tmp360		; <float> [#uses=1]
	%tmp362 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp363 = extractelement <8 x float> %tmp362, i32 5		; <float> [#uses=1]
	%add364 = fadd float %add361, %tmp363		; <float> [#uses=1]
	%tmp365 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp366 = extractelement <8 x float> %tmp365, i32 6		; <float> [#uses=1]
	%add367 = fadd float %add364, %tmp366		; <float> [#uses=1]
	%tmp368 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp369 = extractelement <8 x float> %tmp368, i32 7		; <float> [#uses=1]
	%add370 = fadd float %add367, %tmp369		; <float> [#uses=1]
	%tmp371 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp372 = insertelement <8 x float> %tmp371, float %add370, i32 1		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp372, <8 x float>* %acc
	%tmp373 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 2)		; <<8 x float>> [#uses=1]
	%tmp374 = load i32* %index2		; <i32> [#uses=1]
	%tmp375 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx376 = getelementptr <8 x float> addrspace(1)* %tmp375, i32 %tmp374		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp377 = load <8 x float> addrspace(1)* %arrayidx376		; <<8 x float>> [#uses=1]
	%mul378 = fmul <8 x float> %tmp373, %tmp377		; <<8 x float>> [#uses=1]
	store <8 x float> %mul378, <8 x float>* %temp
	%tmp379 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp380 = extractelement <8 x float> %tmp379, i32 0		; <float> [#uses=1]
	%tmp381 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp382 = extractelement <8 x float> %tmp381, i32 1		; <float> [#uses=1]
	%add383 = fadd float %tmp380, %tmp382		; <float> [#uses=1]
	%tmp384 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp385 = extractelement <8 x float> %tmp384, i32 2		; <float> [#uses=1]
	%add386 = fadd float %add383, %tmp385		; <float> [#uses=1]
	%tmp387 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp388 = extractelement <8 x float> %tmp387, i32 3		; <float> [#uses=1]
	%add389 = fadd float %add386, %tmp388		; <float> [#uses=1]
	%tmp390 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp391 = extractelement <8 x float> %tmp390, i32 4		; <float> [#uses=1]
	%add392 = fadd float %add389, %tmp391		; <float> [#uses=1]
	%tmp393 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp394 = extractelement <8 x float> %tmp393, i32 5		; <float> [#uses=1]
	%add395 = fadd float %add392, %tmp394		; <float> [#uses=1]
	%tmp396 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp397 = extractelement <8 x float> %tmp396, i32 6		; <float> [#uses=1]
	%add398 = fadd float %add395, %tmp397		; <float> [#uses=1]
	%tmp399 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp400 = extractelement <8 x float> %tmp399, i32 7		; <float> [#uses=1]
	%add401 = fadd float %add398, %tmp400		; <float> [#uses=1]
	%tmp402 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp403 = insertelement <8 x float> %tmp402, float %add401, i32 2		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp403, <8 x float>* %acc
	%tmp404 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 3)		; <<8 x float>> [#uses=1]
	%tmp405 = load i32* %index2		; <i32> [#uses=1]
	%tmp406 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx407 = getelementptr <8 x float> addrspace(1)* %tmp406, i32 %tmp405		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp408 = load <8 x float> addrspace(1)* %arrayidx407		; <<8 x float>> [#uses=1]
	%mul409 = fmul <8 x float> %tmp404, %tmp408		; <<8 x float>> [#uses=1]
	store <8 x float> %mul409, <8 x float>* %temp
	%tmp410 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp411 = extractelement <8 x float> %tmp410, i32 0		; <float> [#uses=1]
	%tmp412 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp413 = extractelement <8 x float> %tmp412, i32 1		; <float> [#uses=1]
	%add414 = fadd float %tmp411, %tmp413		; <float> [#uses=1]
	%tmp415 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp416 = extractelement <8 x float> %tmp415, i32 2		; <float> [#uses=1]
	%add417 = fadd float %add414, %tmp416		; <float> [#uses=1]
	%tmp418 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp419 = extractelement <8 x float> %tmp418, i32 3		; <float> [#uses=1]
	%add420 = fadd float %add417, %tmp419		; <float> [#uses=1]
	%tmp421 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp422 = extractelement <8 x float> %tmp421, i32 4		; <float> [#uses=1]
	%add423 = fadd float %add420, %tmp422		; <float> [#uses=1]
	%tmp424 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp425 = extractelement <8 x float> %tmp424, i32 5		; <float> [#uses=1]
	%add426 = fadd float %add423, %tmp425		; <float> [#uses=1]
	%tmp427 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp428 = extractelement <8 x float> %tmp427, i32 6		; <float> [#uses=1]
	%add429 = fadd float %add426, %tmp428		; <float> [#uses=1]
	%tmp430 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp431 = extractelement <8 x float> %tmp430, i32 7		; <float> [#uses=1]
	%add432 = fadd float %add429, %tmp431		; <float> [#uses=1]
	%tmp433 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp434 = insertelement <8 x float> %tmp433, float %add432, i32 3		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp434, <8 x float>* %acc
	%tmp435 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 4)		; <<8 x float>> [#uses=1]
	%tmp436 = load i32* %index2		; <i32> [#uses=1]
	%tmp437 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx438 = getelementptr <8 x float> addrspace(1)* %tmp437, i32 %tmp436		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp439 = load <8 x float> addrspace(1)* %arrayidx438		; <<8 x float>> [#uses=1]
	%mul440 = fmul <8 x float> %tmp435, %tmp439		; <<8 x float>> [#uses=1]
	store <8 x float> %mul440, <8 x float>* %temp
	%tmp441 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp442 = extractelement <8 x float> %tmp441, i32 0		; <float> [#uses=1]
	%tmp443 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp444 = extractelement <8 x float> %tmp443, i32 1		; <float> [#uses=1]
	%add445 = fadd float %tmp442, %tmp444		; <float> [#uses=1]
	%tmp446 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp447 = extractelement <8 x float> %tmp446, i32 2		; <float> [#uses=1]
	%add448 = fadd float %add445, %tmp447		; <float> [#uses=1]
	%tmp449 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp450 = extractelement <8 x float> %tmp449, i32 3		; <float> [#uses=1]
	%add451 = fadd float %add448, %tmp450		; <float> [#uses=1]
	%tmp452 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp453 = extractelement <8 x float> %tmp452, i32 4		; <float> [#uses=1]
	%add454 = fadd float %add451, %tmp453		; <float> [#uses=1]
	%tmp455 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp456 = extractelement <8 x float> %tmp455, i32 5		; <float> [#uses=1]
	%add457 = fadd float %add454, %tmp456		; <float> [#uses=1]
	%tmp458 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp459 = extractelement <8 x float> %tmp458, i32 6		; <float> [#uses=1]
	%add460 = fadd float %add457, %tmp459		; <float> [#uses=1]
	%tmp461 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp462 = extractelement <8 x float> %tmp461, i32 7		; <float> [#uses=1]
	%add463 = fadd float %add460, %tmp462		; <float> [#uses=1]
	%tmp464 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp465 = insertelement <8 x float> %tmp464, float %add463, i32 4		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp465, <8 x float>* %acc
	%tmp466 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 5)		; <<8 x float>> [#uses=1]
	%tmp467 = load i32* %index2		; <i32> [#uses=1]
	%tmp468 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx469 = getelementptr <8 x float> addrspace(1)* %tmp468, i32 %tmp467		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp470 = load <8 x float> addrspace(1)* %arrayidx469		; <<8 x float>> [#uses=1]
	%mul471 = fmul <8 x float> %tmp466, %tmp470		; <<8 x float>> [#uses=1]
	store <8 x float> %mul471, <8 x float>* %temp
	%tmp472 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp473 = extractelement <8 x float> %tmp472, i32 0		; <float> [#uses=1]
	%tmp474 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp475 = extractelement <8 x float> %tmp474, i32 1		; <float> [#uses=1]
	%add476 = fadd float %tmp473, %tmp475		; <float> [#uses=1]
	%tmp477 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp478 = extractelement <8 x float> %tmp477, i32 2		; <float> [#uses=1]
	%add479 = fadd float %add476, %tmp478		; <float> [#uses=1]
	%tmp480 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp481 = extractelement <8 x float> %tmp480, i32 3		; <float> [#uses=1]
	%add482 = fadd float %add479, %tmp481		; <float> [#uses=1]
	%tmp483 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp484 = extractelement <8 x float> %tmp483, i32 4		; <float> [#uses=1]
	%add485 = fadd float %add482, %tmp484		; <float> [#uses=1]
	%tmp486 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp487 = extractelement <8 x float> %tmp486, i32 5		; <float> [#uses=1]
	%add488 = fadd float %add485, %tmp487		; <float> [#uses=1]
	%tmp489 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp490 = extractelement <8 x float> %tmp489, i32 6		; <float> [#uses=1]
	%add491 = fadd float %add488, %tmp490		; <float> [#uses=1]
	%tmp492 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp493 = extractelement <8 x float> %tmp492, i32 7		; <float> [#uses=1]
	%add494 = fadd float %add491, %tmp493		; <float> [#uses=1]
	%tmp495 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp496 = insertelement <8 x float> %tmp495, float %add494, i32 5		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp496, <8 x float>* %acc
	%tmp497 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 6)		; <<8 x float>> [#uses=1]
	%tmp498 = load i32* %index2		; <i32> [#uses=1]
	%tmp499 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx500 = getelementptr <8 x float> addrspace(1)* %tmp499, i32 %tmp498		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp501 = load <8 x float> addrspace(1)* %arrayidx500		; <<8 x float>> [#uses=1]
	%mul502 = fmul <8 x float> %tmp497, %tmp501		; <<8 x float>> [#uses=1]
	store <8 x float> %mul502, <8 x float>* %temp
	%tmp503 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp504 = extractelement <8 x float> %tmp503, i32 0		; <float> [#uses=1]
	%tmp505 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp506 = extractelement <8 x float> %tmp505, i32 1		; <float> [#uses=1]
	%add507 = fadd float %tmp504, %tmp506		; <float> [#uses=1]
	%tmp508 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp509 = extractelement <8 x float> %tmp508, i32 2		; <float> [#uses=1]
	%add510 = fadd float %add507, %tmp509		; <float> [#uses=1]
	%tmp511 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp512 = extractelement <8 x float> %tmp511, i32 3		; <float> [#uses=1]
	%add513 = fadd float %add510, %tmp512		; <float> [#uses=1]
	%tmp514 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp515 = extractelement <8 x float> %tmp514, i32 4		; <float> [#uses=1]
	%add516 = fadd float %add513, %tmp515		; <float> [#uses=1]
	%tmp517 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp518 = extractelement <8 x float> %tmp517, i32 5		; <float> [#uses=1]
	%add519 = fadd float %add516, %tmp518		; <float> [#uses=1]
	%tmp520 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp521 = extractelement <8 x float> %tmp520, i32 6		; <float> [#uses=1]
	%add522 = fadd float %add519, %tmp521		; <float> [#uses=1]
	%tmp523 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp524 = extractelement <8 x float> %tmp523, i32 7		; <float> [#uses=1]
	%add525 = fadd float %add522, %tmp524		; <float> [#uses=1]
	%tmp526 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp527 = insertelement <8 x float> %tmp526, float %add525, i32 6		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp527, <8 x float>* %acc
	%tmp528 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 7)		; <<8 x float>> [#uses=1]
	%tmp529 = load i32* %index2		; <i32> [#uses=1]
	%tmp530 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx531 = getelementptr <8 x float> addrspace(1)* %tmp530, i32 %tmp529		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp532 = load <8 x float> addrspace(1)* %arrayidx531		; <<8 x float>> [#uses=1]
	%mul533 = fmul <8 x float> %tmp528, %tmp532		; <<8 x float>> [#uses=1]
	store <8 x float> %mul533, <8 x float>* %temp
	%tmp534 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp535 = extractelement <8 x float> %tmp534, i32 0		; <float> [#uses=1]
	%tmp536 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp537 = extractelement <8 x float> %tmp536, i32 1		; <float> [#uses=1]
	%add538 = fadd float %tmp535, %tmp537		; <float> [#uses=1]
	%tmp539 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp540 = extractelement <8 x float> %tmp539, i32 2		; <float> [#uses=1]
	%add541 = fadd float %add538, %tmp540		; <float> [#uses=1]
	%tmp542 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp543 = extractelement <8 x float> %tmp542, i32 3		; <float> [#uses=1]
	%add544 = fadd float %add541, %tmp543		; <float> [#uses=1]
	%tmp545 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp546 = extractelement <8 x float> %tmp545, i32 4		; <float> [#uses=1]
	%add547 = fadd float %add544, %tmp546		; <float> [#uses=1]
	%tmp548 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp549 = extractelement <8 x float> %tmp548, i32 5		; <float> [#uses=1]
	%add550 = fadd float %add547, %tmp549		; <float> [#uses=1]
	%tmp551 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp552 = extractelement <8 x float> %tmp551, i32 6		; <float> [#uses=1]
	%add553 = fadd float %add550, %tmp552		; <float> [#uses=1]
	%tmp554 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp555 = extractelement <8 x float> %tmp554, i32 7		; <float> [#uses=1]
	%add556 = fadd float %add553, %tmp555		; <float> [#uses=1]
	%tmp557 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp558 = insertelement <8 x float> %tmp557, float %add556, i32 7		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp558, <8 x float>* %acc
	%tmp559 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul560 = mul i32 %tmp559, 8		; <i32> [#uses=1]
	%tmp561 = load i32* %k2		; <i32> [#uses=1]
	%add562 = add i32 %mul560, %tmp561		; <i32> [#uses=1]
	%tmp563 = load i32* %step		; <i32> [#uses=1]
	%mul564 = mul i32 %add562, %tmp563		; <i32> [#uses=1]
	%tmp565 = load i32* %groupIdx		; <i32> [#uses=1]
	%add566 = add i32 %mul564, %tmp565		; <i32> [#uses=1]
	store i32 %add566, i32* %idx
	%tmp567 = load i32* %idx		; <i32> [#uses=1]
	%tmp568 = load <8 x float> addrspace(1)** %output.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx569 = getelementptr <8 x float> addrspace(1)* %tmp568, i32 %tmp567		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp570 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp570, <8 x float> addrspace(1)* %arrayidx569
	ret void
}

define void @DCT_VECTOR_DOT(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
	%output.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=2]
	%input.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=17]
	%dct.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=33]
	%width.addr = alloca i32		; <i32*> [#uses=2]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=1]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=1]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=3]
	%k1 = alloca i32, align 4		; <i32*> [#uses=3]
	%k2 = alloca i32, align 4		; <i32*> [#uses=3]
	%n1 = alloca i32, align 4		; <i32*> [#uses=0]
	%n2 = alloca i32, align 4		; <i32*> [#uses=0]
	%idx = alloca i32, align 4		; <i32*> [#uses=5]
	%acc = alloca <8 x float>, align 32		; <<8 x float>*> [#uses=35]
	%step = alloca i32, align 4		; <i32*> [#uses=10]
	%index1 = alloca i32, align 4		; <i32*> [#uses=17]
	%index2 = alloca i32, align 4		; <i32*> [#uses=48]
	store <8 x float> addrspace(1)* %output, <8 x float> addrspace(1)** %output.addr
	store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
	store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalIdx
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalIdy
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupIdx
	%call3 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %groupIdy
	%call4 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call4, i32* %i
	store i32 0, i32* %idx
	store <8 x float> zeroinitializer, <8 x float>* %acc
	%tmp = load i32* %width.addr		; <i32> [#uses=1]
	%div = udiv i32 %tmp, 8		; <i32> [#uses=1]
	store i32 %div, i32* %step
	%tmp5 = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp5, i32* %k1
	%tmp7 = load i32* %k1		; <i32> [#uses=1]
	store i32 %tmp7, i32* %index1
	%tmp9 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul = mul i32 %tmp9, 8		; <i32> [#uses=1]
	%tmp10 = load i32* %step		; <i32> [#uses=1]
	%mul11 = mul i32 %mul, %tmp10		; <i32> [#uses=1]
	%tmp12 = load i32* %groupIdx		; <i32> [#uses=1]
	%add = add i32 %mul11, %tmp12		; <i32> [#uses=1]
	store i32 %add, i32* %index2
	%tmp13 = load i32* %index1		; <i32> [#uses=1]
	%tmp14 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <8 x float> addrspace(1)* %tmp14, i32 %tmp13		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp15 = load <8 x float> addrspace(1)* %arrayidx		; <<8 x float>> [#uses=1]
	%tmp16 = shufflevector <8 x float> %tmp15, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp17 = load i32* %index2		; <i32> [#uses=1]
	%tmp18 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx19 = getelementptr <8 x float> addrspace(1)* %tmp18, i32 %tmp17		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp20 = load <8 x float> addrspace(1)* %arrayidx19		; <<8 x float>> [#uses=1]
	%tmp21 = shufflevector <8 x float> %tmp20, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call22 = call float @__dotf4(<4 x float> %tmp16, <4 x float> %tmp21)		; <float> [#uses=1]
	%tmp23 = load i32* %index1		; <i32> [#uses=1]
	%tmp24 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx25 = getelementptr <8 x float> addrspace(1)* %tmp24, i32 %tmp23		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp26 = load <8 x float> addrspace(1)* %arrayidx25		; <<8 x float>> [#uses=1]
	%tmp27 = shufflevector <8 x float> %tmp26, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp28 = load i32* %index2		; <i32> [#uses=1]
	%tmp29 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx30 = getelementptr <8 x float> addrspace(1)* %tmp29, i32 %tmp28		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp31 = load <8 x float> addrspace(1)* %arrayidx30		; <<8 x float>> [#uses=1]
	%tmp32 = shufflevector <8 x float> %tmp31, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call33 = call float @__dotf4(<4 x float> %tmp27, <4 x float> %tmp32)		; <float> [#uses=1]
	%add34 = fadd float %call22, %call33		; <float> [#uses=1]
	%tmp35 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp36 = insertelement <8 x float> %tmp35, float %add34, i32 0		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp36, <8 x float>* %acc
	%tmp37 = load i32* %index2		; <i32> [#uses=1]
	%tmp38 = load i32* %step		; <i32> [#uses=1]
	%add39 = add i32 %tmp37, %tmp38		; <i32> [#uses=1]
	store i32 %add39, i32* %index2
	%tmp40 = load i32* %index1		; <i32> [#uses=1]
	%tmp41 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx42 = getelementptr <8 x float> addrspace(1)* %tmp41, i32 %tmp40		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp43 = load <8 x float> addrspace(1)* %arrayidx42		; <<8 x float>> [#uses=1]
	%tmp44 = shufflevector <8 x float> %tmp43, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp45 = load i32* %index2		; <i32> [#uses=1]
	%tmp46 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx47 = getelementptr <8 x float> addrspace(1)* %tmp46, i32 %tmp45		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp48 = load <8 x float> addrspace(1)* %arrayidx47		; <<8 x float>> [#uses=1]
	%tmp49 = shufflevector <8 x float> %tmp48, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call50 = call float @__dotf4(<4 x float> %tmp44, <4 x float> %tmp49)		; <float> [#uses=1]
	%tmp51 = load i32* %index1		; <i32> [#uses=1]
	%tmp52 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx53 = getelementptr <8 x float> addrspace(1)* %tmp52, i32 %tmp51		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp54 = load <8 x float> addrspace(1)* %arrayidx53		; <<8 x float>> [#uses=1]
	%tmp55 = shufflevector <8 x float> %tmp54, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp56 = load i32* %index2		; <i32> [#uses=1]
	%tmp57 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx58 = getelementptr <8 x float> addrspace(1)* %tmp57, i32 %tmp56		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp59 = load <8 x float> addrspace(1)* %arrayidx58		; <<8 x float>> [#uses=1]
	%tmp60 = shufflevector <8 x float> %tmp59, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call61 = call float @__dotf4(<4 x float> %tmp55, <4 x float> %tmp60)		; <float> [#uses=1]
	%add62 = fadd float %call50, %call61		; <float> [#uses=1]
	%tmp63 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp64 = insertelement <8 x float> %tmp63, float %add62, i32 1		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp64, <8 x float>* %acc
	%tmp65 = load i32* %index2		; <i32> [#uses=1]
	%tmp66 = load i32* %step		; <i32> [#uses=1]
	%add67 = add i32 %tmp65, %tmp66		; <i32> [#uses=1]
	store i32 %add67, i32* %index2
	%tmp68 = load i32* %index1		; <i32> [#uses=1]
	%tmp69 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx70 = getelementptr <8 x float> addrspace(1)* %tmp69, i32 %tmp68		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp71 = load <8 x float> addrspace(1)* %arrayidx70		; <<8 x float>> [#uses=1]
	%tmp72 = shufflevector <8 x float> %tmp71, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp73 = load i32* %index2		; <i32> [#uses=1]
	%tmp74 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx75 = getelementptr <8 x float> addrspace(1)* %tmp74, i32 %tmp73		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp76 = load <8 x float> addrspace(1)* %arrayidx75		; <<8 x float>> [#uses=1]
	%tmp77 = shufflevector <8 x float> %tmp76, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call78 = call float @__dotf4(<4 x float> %tmp72, <4 x float> %tmp77)		; <float> [#uses=1]
	%tmp79 = load i32* %index1		; <i32> [#uses=1]
	%tmp80 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx81 = getelementptr <8 x float> addrspace(1)* %tmp80, i32 %tmp79		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp82 = load <8 x float> addrspace(1)* %arrayidx81		; <<8 x float>> [#uses=1]
	%tmp83 = shufflevector <8 x float> %tmp82, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp84 = load i32* %index2		; <i32> [#uses=1]
	%tmp85 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx86 = getelementptr <8 x float> addrspace(1)* %tmp85, i32 %tmp84		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp87 = load <8 x float> addrspace(1)* %arrayidx86		; <<8 x float>> [#uses=1]
	%tmp88 = shufflevector <8 x float> %tmp87, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call89 = call float @__dotf4(<4 x float> %tmp83, <4 x float> %tmp88)		; <float> [#uses=1]
	%add90 = fadd float %call78, %call89		; <float> [#uses=1]
	%tmp91 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp92 = insertelement <8 x float> %tmp91, float %add90, i32 2		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp92, <8 x float>* %acc
	%tmp93 = load i32* %index2		; <i32> [#uses=1]
	%tmp94 = load i32* %step		; <i32> [#uses=1]
	%add95 = add i32 %tmp93, %tmp94		; <i32> [#uses=1]
	store i32 %add95, i32* %index2
	%tmp96 = load i32* %index1		; <i32> [#uses=1]
	%tmp97 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx98 = getelementptr <8 x float> addrspace(1)* %tmp97, i32 %tmp96		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp99 = load <8 x float> addrspace(1)* %arrayidx98		; <<8 x float>> [#uses=1]
	%tmp100 = shufflevector <8 x float> %tmp99, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp101 = load i32* %index2		; <i32> [#uses=1]
	%tmp102 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx103 = getelementptr <8 x float> addrspace(1)* %tmp102, i32 %tmp101		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp104 = load <8 x float> addrspace(1)* %arrayidx103		; <<8 x float>> [#uses=1]
	%tmp105 = shufflevector <8 x float> %tmp104, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call106 = call float @__dotf4(<4 x float> %tmp100, <4 x float> %tmp105)		; <float> [#uses=1]
	%tmp107 = load i32* %index1		; <i32> [#uses=1]
	%tmp108 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx109 = getelementptr <8 x float> addrspace(1)* %tmp108, i32 %tmp107		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp110 = load <8 x float> addrspace(1)* %arrayidx109		; <<8 x float>> [#uses=1]
	%tmp111 = shufflevector <8 x float> %tmp110, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp112 = load i32* %index2		; <i32> [#uses=1]
	%tmp113 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx114 = getelementptr <8 x float> addrspace(1)* %tmp113, i32 %tmp112		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp115 = load <8 x float> addrspace(1)* %arrayidx114		; <<8 x float>> [#uses=1]
	%tmp116 = shufflevector <8 x float> %tmp115, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call117 = call float @__dotf4(<4 x float> %tmp111, <4 x float> %tmp116)		; <float> [#uses=1]
	%add118 = fadd float %call106, %call117		; <float> [#uses=1]
	%tmp119 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp120 = insertelement <8 x float> %tmp119, float %add118, i32 3		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp120, <8 x float>* %acc
	%tmp121 = load i32* %index2		; <i32> [#uses=1]
	%tmp122 = load i32* %step		; <i32> [#uses=1]
	%add123 = add i32 %tmp121, %tmp122		; <i32> [#uses=1]
	store i32 %add123, i32* %index2
	%tmp124 = load i32* %index1		; <i32> [#uses=1]
	%tmp125 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx126 = getelementptr <8 x float> addrspace(1)* %tmp125, i32 %tmp124		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp127 = load <8 x float> addrspace(1)* %arrayidx126		; <<8 x float>> [#uses=1]
	%tmp128 = shufflevector <8 x float> %tmp127, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp129 = load i32* %index2		; <i32> [#uses=1]
	%tmp130 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx131 = getelementptr <8 x float> addrspace(1)* %tmp130, i32 %tmp129		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp132 = load <8 x float> addrspace(1)* %arrayidx131		; <<8 x float>> [#uses=1]
	%tmp133 = shufflevector <8 x float> %tmp132, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call134 = call float @__dotf4(<4 x float> %tmp128, <4 x float> %tmp133)		; <float> [#uses=1]
	%tmp135 = load i32* %index1		; <i32> [#uses=1]
	%tmp136 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx137 = getelementptr <8 x float> addrspace(1)* %tmp136, i32 %tmp135		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp138 = load <8 x float> addrspace(1)* %arrayidx137		; <<8 x float>> [#uses=1]
	%tmp139 = shufflevector <8 x float> %tmp138, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp140 = load i32* %index2		; <i32> [#uses=1]
	%tmp141 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx142 = getelementptr <8 x float> addrspace(1)* %tmp141, i32 %tmp140		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp143 = load <8 x float> addrspace(1)* %arrayidx142		; <<8 x float>> [#uses=1]
	%tmp144 = shufflevector <8 x float> %tmp143, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call145 = call float @__dotf4(<4 x float> %tmp139, <4 x float> %tmp144)		; <float> [#uses=1]
	%add146 = fadd float %call134, %call145		; <float> [#uses=1]
	%tmp147 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp148 = insertelement <8 x float> %tmp147, float %add146, i32 4		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp148, <8 x float>* %acc
	%tmp149 = load i32* %index2		; <i32> [#uses=1]
	%tmp150 = load i32* %step		; <i32> [#uses=1]
	%add151 = add i32 %tmp149, %tmp150		; <i32> [#uses=1]
	store i32 %add151, i32* %index2
	%tmp152 = load i32* %index1		; <i32> [#uses=1]
	%tmp153 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx154 = getelementptr <8 x float> addrspace(1)* %tmp153, i32 %tmp152		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp155 = load <8 x float> addrspace(1)* %arrayidx154		; <<8 x float>> [#uses=1]
	%tmp156 = shufflevector <8 x float> %tmp155, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp157 = load i32* %index2		; <i32> [#uses=1]
	%tmp158 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx159 = getelementptr <8 x float> addrspace(1)* %tmp158, i32 %tmp157		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp160 = load <8 x float> addrspace(1)* %arrayidx159		; <<8 x float>> [#uses=1]
	%tmp161 = shufflevector <8 x float> %tmp160, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call162 = call float @__dotf4(<4 x float> %tmp156, <4 x float> %tmp161)		; <float> [#uses=1]
	%tmp163 = load i32* %index1		; <i32> [#uses=1]
	%tmp164 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx165 = getelementptr <8 x float> addrspace(1)* %tmp164, i32 %tmp163		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp166 = load <8 x float> addrspace(1)* %arrayidx165		; <<8 x float>> [#uses=1]
	%tmp167 = shufflevector <8 x float> %tmp166, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp168 = load i32* %index2		; <i32> [#uses=1]
	%tmp169 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx170 = getelementptr <8 x float> addrspace(1)* %tmp169, i32 %tmp168		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp171 = load <8 x float> addrspace(1)* %arrayidx170		; <<8 x float>> [#uses=1]
	%tmp172 = shufflevector <8 x float> %tmp171, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call173 = call float @__dotf4(<4 x float> %tmp167, <4 x float> %tmp172)		; <float> [#uses=1]
	%add174 = fadd float %call162, %call173		; <float> [#uses=1]
	%tmp175 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp176 = insertelement <8 x float> %tmp175, float %add174, i32 5		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp176, <8 x float>* %acc
	%tmp177 = load i32* %index2		; <i32> [#uses=1]
	%tmp178 = load i32* %step		; <i32> [#uses=1]
	%add179 = add i32 %tmp177, %tmp178		; <i32> [#uses=1]
	store i32 %add179, i32* %index2
	%tmp180 = load i32* %index1		; <i32> [#uses=1]
	%tmp181 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx182 = getelementptr <8 x float> addrspace(1)* %tmp181, i32 %tmp180		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp183 = load <8 x float> addrspace(1)* %arrayidx182		; <<8 x float>> [#uses=1]
	%tmp184 = shufflevector <8 x float> %tmp183, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp185 = load i32* %index2		; <i32> [#uses=1]
	%tmp186 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx187 = getelementptr <8 x float> addrspace(1)* %tmp186, i32 %tmp185		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp188 = load <8 x float> addrspace(1)* %arrayidx187		; <<8 x float>> [#uses=1]
	%tmp189 = shufflevector <8 x float> %tmp188, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call190 = call float @__dotf4(<4 x float> %tmp184, <4 x float> %tmp189)		; <float> [#uses=1]
	%tmp191 = load i32* %index1		; <i32> [#uses=1]
	%tmp192 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx193 = getelementptr <8 x float> addrspace(1)* %tmp192, i32 %tmp191		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp194 = load <8 x float> addrspace(1)* %arrayidx193		; <<8 x float>> [#uses=1]
	%tmp195 = shufflevector <8 x float> %tmp194, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp196 = load i32* %index2		; <i32> [#uses=1]
	%tmp197 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx198 = getelementptr <8 x float> addrspace(1)* %tmp197, i32 %tmp196		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp199 = load <8 x float> addrspace(1)* %arrayidx198		; <<8 x float>> [#uses=1]
	%tmp200 = shufflevector <8 x float> %tmp199, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call201 = call float @__dotf4(<4 x float> %tmp195, <4 x float> %tmp200)		; <float> [#uses=1]
	%add202 = fadd float %call190, %call201		; <float> [#uses=1]
	%tmp203 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp204 = insertelement <8 x float> %tmp203, float %add202, i32 6		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp204, <8 x float>* %acc
	%tmp205 = load i32* %index2		; <i32> [#uses=1]
	%tmp206 = load i32* %step		; <i32> [#uses=1]
	%add207 = add i32 %tmp205, %tmp206		; <i32> [#uses=1]
	store i32 %add207, i32* %index2
	%tmp208 = load i32* %index1		; <i32> [#uses=1]
	%tmp209 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx210 = getelementptr <8 x float> addrspace(1)* %tmp209, i32 %tmp208		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp211 = load <8 x float> addrspace(1)* %arrayidx210		; <<8 x float>> [#uses=1]
	%tmp212 = shufflevector <8 x float> %tmp211, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp213 = load i32* %index2		; <i32> [#uses=1]
	%tmp214 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx215 = getelementptr <8 x float> addrspace(1)* %tmp214, i32 %tmp213		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp216 = load <8 x float> addrspace(1)* %arrayidx215		; <<8 x float>> [#uses=1]
	%tmp217 = shufflevector <8 x float> %tmp216, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call218 = call float @__dotf4(<4 x float> %tmp212, <4 x float> %tmp217)		; <float> [#uses=1]
	%tmp219 = load i32* %index1		; <i32> [#uses=1]
	%tmp220 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx221 = getelementptr <8 x float> addrspace(1)* %tmp220, i32 %tmp219		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp222 = load <8 x float> addrspace(1)* %arrayidx221		; <<8 x float>> [#uses=1]
	%tmp223 = shufflevector <8 x float> %tmp222, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp224 = load i32* %index2		; <i32> [#uses=1]
	%tmp225 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx226 = getelementptr <8 x float> addrspace(1)* %tmp225, i32 %tmp224		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp227 = load <8 x float> addrspace(1)* %arrayidx226		; <<8 x float>> [#uses=1]
	%tmp228 = shufflevector <8 x float> %tmp227, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call229 = call float @__dotf4(<4 x float> %tmp223, <4 x float> %tmp228)		; <float> [#uses=1]
	%add230 = fadd float %call218, %call229		; <float> [#uses=1]
	%tmp231 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp232 = insertelement <8 x float> %tmp231, float %add230, i32 7		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp232, <8 x float>* %acc
	%tmp233 = load i32* %k1		; <i32> [#uses=1]
	store i32 %tmp233, i32* %idx
	%tmp234 = load i32* %idx		; <i32> [#uses=1]
	%arrayidx235 = getelementptr <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 0), i32 %tmp234		; <<8 x float> addrspace(3)*> [#uses=1]
	%tmp236 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp236, <8 x float> addrspace(3)* %arrayidx235
	call void @barrier(i32 2)
	%tmp237 = load i32* %i		; <i32> [#uses=1]
	store i32 %tmp237, i32* %k2
	%tmp238 = load i32* %k2		; <i32> [#uses=1]
	store i32 %tmp238, i32* %index2
	%tmp239 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 0)		; <<8 x float>> [#uses=1]
	%tmp240 = shufflevector <8 x float> %tmp239, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp241 = load i32* %index2		; <i32> [#uses=1]
	%tmp242 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx243 = getelementptr <8 x float> addrspace(1)* %tmp242, i32 %tmp241		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp244 = load <8 x float> addrspace(1)* %arrayidx243		; <<8 x float>> [#uses=1]
	%tmp245 = shufflevector <8 x float> %tmp244, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call246 = call float @__dotf4(<4 x float> %tmp240, <4 x float> %tmp245)		; <float> [#uses=1]
	%tmp247 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 0)		; <<8 x float>> [#uses=1]
	%tmp248 = shufflevector <8 x float> %tmp247, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp249 = load i32* %index2		; <i32> [#uses=1]
	%tmp250 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx251 = getelementptr <8 x float> addrspace(1)* %tmp250, i32 %tmp249		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp252 = load <8 x float> addrspace(1)* %arrayidx251		; <<8 x float>> [#uses=1]
	%tmp253 = shufflevector <8 x float> %tmp252, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call254 = call float @__dotf4(<4 x float> %tmp248, <4 x float> %tmp253)		; <float> [#uses=1]
	%add255 = fadd float %call246, %call254		; <float> [#uses=1]
	%tmp256 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp257 = insertelement <8 x float> %tmp256, float %add255, i32 0		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp257, <8 x float>* %acc
	%tmp258 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 1)		; <<8 x float>> [#uses=1]
	%tmp259 = shufflevector <8 x float> %tmp258, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp260 = load i32* %index2		; <i32> [#uses=1]
	%tmp261 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx262 = getelementptr <8 x float> addrspace(1)* %tmp261, i32 %tmp260		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp263 = load <8 x float> addrspace(1)* %arrayidx262		; <<8 x float>> [#uses=1]
	%tmp264 = shufflevector <8 x float> %tmp263, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call265 = call float @__dotf4(<4 x float> %tmp259, <4 x float> %tmp264)		; <float> [#uses=1]
	%tmp266 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 1)		; <<8 x float>> [#uses=1]
	%tmp267 = shufflevector <8 x float> %tmp266, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp268 = load i32* %index2		; <i32> [#uses=1]
	%tmp269 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx270 = getelementptr <8 x float> addrspace(1)* %tmp269, i32 %tmp268		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp271 = load <8 x float> addrspace(1)* %arrayidx270		; <<8 x float>> [#uses=1]
	%tmp272 = shufflevector <8 x float> %tmp271, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call273 = call float @__dotf4(<4 x float> %tmp267, <4 x float> %tmp272)		; <float> [#uses=1]
	%add274 = fadd float %call265, %call273		; <float> [#uses=1]
	%tmp275 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp276 = insertelement <8 x float> %tmp275, float %add274, i32 1		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp276, <8 x float>* %acc
	%tmp277 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 2)		; <<8 x float>> [#uses=1]
	%tmp278 = shufflevector <8 x float> %tmp277, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp279 = load i32* %index2		; <i32> [#uses=1]
	%tmp280 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx281 = getelementptr <8 x float> addrspace(1)* %tmp280, i32 %tmp279		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp282 = load <8 x float> addrspace(1)* %arrayidx281		; <<8 x float>> [#uses=1]
	%tmp283 = shufflevector <8 x float> %tmp282, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call284 = call float @__dotf4(<4 x float> %tmp278, <4 x float> %tmp283)		; <float> [#uses=1]
	%tmp285 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 2)		; <<8 x float>> [#uses=1]
	%tmp286 = shufflevector <8 x float> %tmp285, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp287 = load i32* %index2		; <i32> [#uses=1]
	%tmp288 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx289 = getelementptr <8 x float> addrspace(1)* %tmp288, i32 %tmp287		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp290 = load <8 x float> addrspace(1)* %arrayidx289		; <<8 x float>> [#uses=1]
	%tmp291 = shufflevector <8 x float> %tmp290, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call292 = call float @__dotf4(<4 x float> %tmp286, <4 x float> %tmp291)		; <float> [#uses=1]
	%add293 = fadd float %call284, %call292		; <float> [#uses=1]
	%tmp294 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp295 = insertelement <8 x float> %tmp294, float %add293, i32 2		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp295, <8 x float>* %acc
	%tmp296 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 3)		; <<8 x float>> [#uses=1]
	%tmp297 = shufflevector <8 x float> %tmp296, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp298 = load i32* %index2		; <i32> [#uses=1]
	%tmp299 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx300 = getelementptr <8 x float> addrspace(1)* %tmp299, i32 %tmp298		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp301 = load <8 x float> addrspace(1)* %arrayidx300		; <<8 x float>> [#uses=1]
	%tmp302 = shufflevector <8 x float> %tmp301, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call303 = call float @__dotf4(<4 x float> %tmp297, <4 x float> %tmp302)		; <float> [#uses=1]
	%tmp304 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 3)		; <<8 x float>> [#uses=1]
	%tmp305 = shufflevector <8 x float> %tmp304, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp306 = load i32* %index2		; <i32> [#uses=1]
	%tmp307 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx308 = getelementptr <8 x float> addrspace(1)* %tmp307, i32 %tmp306		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp309 = load <8 x float> addrspace(1)* %arrayidx308		; <<8 x float>> [#uses=1]
	%tmp310 = shufflevector <8 x float> %tmp309, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call311 = call float @__dotf4(<4 x float> %tmp305, <4 x float> %tmp310)		; <float> [#uses=1]
	%add312 = fadd float %call303, %call311		; <float> [#uses=1]
	%tmp313 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp314 = insertelement <8 x float> %tmp313, float %add312, i32 3		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp314, <8 x float>* %acc
	%tmp315 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 4)		; <<8 x float>> [#uses=1]
	%tmp316 = shufflevector <8 x float> %tmp315, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp317 = load i32* %index2		; <i32> [#uses=1]
	%tmp318 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx319 = getelementptr <8 x float> addrspace(1)* %tmp318, i32 %tmp317		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp320 = load <8 x float> addrspace(1)* %arrayidx319		; <<8 x float>> [#uses=1]
	%tmp321 = shufflevector <8 x float> %tmp320, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call322 = call float @__dotf4(<4 x float> %tmp316, <4 x float> %tmp321)		; <float> [#uses=1]
	%tmp323 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 4)		; <<8 x float>> [#uses=1]
	%tmp324 = shufflevector <8 x float> %tmp323, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp325 = load i32* %index2		; <i32> [#uses=1]
	%tmp326 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx327 = getelementptr <8 x float> addrspace(1)* %tmp326, i32 %tmp325		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp328 = load <8 x float> addrspace(1)* %arrayidx327		; <<8 x float>> [#uses=1]
	%tmp329 = shufflevector <8 x float> %tmp328, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call330 = call float @__dotf4(<4 x float> %tmp324, <4 x float> %tmp329)		; <float> [#uses=1]
	%add331 = fadd float %call322, %call330		; <float> [#uses=1]
	%tmp332 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp333 = insertelement <8 x float> %tmp332, float %add331, i32 4		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp333, <8 x float>* %acc
	%tmp334 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 5)		; <<8 x float>> [#uses=1]
	%tmp335 = shufflevector <8 x float> %tmp334, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp336 = load i32* %index2		; <i32> [#uses=1]
	%tmp337 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx338 = getelementptr <8 x float> addrspace(1)* %tmp337, i32 %tmp336		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp339 = load <8 x float> addrspace(1)* %arrayidx338		; <<8 x float>> [#uses=1]
	%tmp340 = shufflevector <8 x float> %tmp339, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call341 = call float @__dotf4(<4 x float> %tmp335, <4 x float> %tmp340)		; <float> [#uses=1]
	%tmp342 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 5)		; <<8 x float>> [#uses=1]
	%tmp343 = shufflevector <8 x float> %tmp342, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp344 = load i32* %index2		; <i32> [#uses=1]
	%tmp345 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx346 = getelementptr <8 x float> addrspace(1)* %tmp345, i32 %tmp344		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp347 = load <8 x float> addrspace(1)* %arrayidx346		; <<8 x float>> [#uses=1]
	%tmp348 = shufflevector <8 x float> %tmp347, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call349 = call float @__dotf4(<4 x float> %tmp343, <4 x float> %tmp348)		; <float> [#uses=1]
	%add350 = fadd float %call341, %call349		; <float> [#uses=1]
	%tmp351 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp352 = insertelement <8 x float> %tmp351, float %add350, i32 5		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp352, <8 x float>* %acc
	%tmp353 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 6)		; <<8 x float>> [#uses=1]
	%tmp354 = shufflevector <8 x float> %tmp353, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp355 = load i32* %index2		; <i32> [#uses=1]
	%tmp356 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx357 = getelementptr <8 x float> addrspace(1)* %tmp356, i32 %tmp355		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp358 = load <8 x float> addrspace(1)* %arrayidx357		; <<8 x float>> [#uses=1]
	%tmp359 = shufflevector <8 x float> %tmp358, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call360 = call float @__dotf4(<4 x float> %tmp354, <4 x float> %tmp359)		; <float> [#uses=1]
	%tmp361 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 6)		; <<8 x float>> [#uses=1]
	%tmp362 = shufflevector <8 x float> %tmp361, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp363 = load i32* %index2		; <i32> [#uses=1]
	%tmp364 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx365 = getelementptr <8 x float> addrspace(1)* %tmp364, i32 %tmp363		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp366 = load <8 x float> addrspace(1)* %arrayidx365		; <<8 x float>> [#uses=1]
	%tmp367 = shufflevector <8 x float> %tmp366, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call368 = call float @__dotf4(<4 x float> %tmp362, <4 x float> %tmp367)		; <float> [#uses=1]
	%add369 = fadd float %call360, %call368		; <float> [#uses=1]
	%tmp370 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp371 = insertelement <8 x float> %tmp370, float %add369, i32 6		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp371, <8 x float>* %acc
	%tmp372 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 7)		; <<8 x float>> [#uses=1]
	%tmp373 = shufflevector <8 x float> %tmp372, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%tmp374 = load i32* %index2		; <i32> [#uses=1]
	%tmp375 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx376 = getelementptr <8 x float> addrspace(1)* %tmp375, i32 %tmp374		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp377 = load <8 x float> addrspace(1)* %arrayidx376		; <<8 x float>> [#uses=1]
	%tmp378 = shufflevector <8 x float> %tmp377, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>		; <<4 x float>> [#uses=1]
	%call379 = call float @__dotf4(<4 x float> %tmp373, <4 x float> %tmp378)		; <float> [#uses=1]
	%tmp380 = load <8 x float> addrspace(3)* getelementptr ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 7)		; <<8 x float>> [#uses=1]
	%tmp381 = shufflevector <8 x float> %tmp380, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%tmp382 = load i32* %index2		; <i32> [#uses=1]
	%tmp383 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx384 = getelementptr <8 x float> addrspace(1)* %tmp383, i32 %tmp382		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp385 = load <8 x float> addrspace(1)* %arrayidx384		; <<8 x float>> [#uses=1]
	%tmp386 = shufflevector <8 x float> %tmp385, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>		; <<4 x float>> [#uses=1]
	%call387 = call float @__dotf4(<4 x float> %tmp381, <4 x float> %tmp386)		; <float> [#uses=1]
	%add388 = fadd float %call379, %call387		; <float> [#uses=1]
	%tmp389 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	%tmp390 = insertelement <8 x float> %tmp389, float %add388, i32 7		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp390, <8 x float>* %acc
	%tmp391 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul392 = mul i32 %tmp391, 8		; <i32> [#uses=1]
	%tmp393 = load i32* %k2		; <i32> [#uses=1]
	%add394 = add i32 %mul392, %tmp393		; <i32> [#uses=1]
	%tmp395 = load i32* %step		; <i32> [#uses=1]
	%mul396 = mul i32 %add394, %tmp395		; <i32> [#uses=1]
	%tmp397 = load i32* %groupIdx		; <i32> [#uses=1]
	%add398 = add i32 %mul396, %tmp397		; <i32> [#uses=1]
	store i32 %add398, i32* %idx
	%tmp399 = load i32* %idx		; <i32> [#uses=1]
	%tmp400 = load <8 x float> addrspace(1)** %output.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx401 = getelementptr <8 x float> addrspace(1)* %tmp400, i32 %tmp399		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp402 = load <8 x float>* %acc		; <<8 x float>> [#uses=1]
	store <8 x float> %tmp402, <8 x float> addrspace(1)* %arrayidx401
	ret void
}

declare float @__dotf4(<4 x float>, <4 x float>)

define void @DCT_CPU(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%input.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%dct.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=4]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%k1 = alloca i32, align 4		; <i32*> [#uses=10]
	%k2 = alloca i32, align 4		; <i32*> [#uses=4]
	%n1 = alloca i32, align 4		; <i32*> [#uses=6]
	%n2 = alloca i32, align 4		; <i32*> [#uses=11]
	%inter = alloca [64 x float], align 4		; <[64 x float]*> [#uses=66]
	%step = alloca i32, align 4		; <i32*> [#uses=3]
	%inputIndex = alloca i32, align 4		; <i32*> [#uses=5]
	%dctIndex = alloca i32, align 4		; <i32*> [#uses=9]
	%interIndex = alloca i32, align 4		; <i32*> [#uses=9]
	%outputIndex = alloca i32, align 4		; <i32*> [#uses=6]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store float addrspace(1)* %input, float addrspace(1)** %input.addr
	store float addrspace(1)* %dct, float addrspace(1)** %dct.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %groupIdx
	%call1 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %groupIdy
	%.array = getelementptr [64 x float]* %inter, i32 0, i32 0		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array
	%.array2 = getelementptr [64 x float]* %inter, i32 0, i32 1		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array2
	%.array3 = getelementptr [64 x float]* %inter, i32 0, i32 2		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array3
	%.array4 = getelementptr [64 x float]* %inter, i32 0, i32 3		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array4
	%.array5 = getelementptr [64 x float]* %inter, i32 0, i32 4		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array5
	%.array6 = getelementptr [64 x float]* %inter, i32 0, i32 5		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array6
	%.array7 = getelementptr [64 x float]* %inter, i32 0, i32 6		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array7
	%.array8 = getelementptr [64 x float]* %inter, i32 0, i32 7		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array8
	%.array9 = getelementptr [64 x float]* %inter, i32 0, i32 8		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array9
	%.array10 = getelementptr [64 x float]* %inter, i32 0, i32 9		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array10
	%.array11 = getelementptr [64 x float]* %inter, i32 0, i32 10		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array11
	%.array12 = getelementptr [64 x float]* %inter, i32 0, i32 11		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array12
	%.array13 = getelementptr [64 x float]* %inter, i32 0, i32 12		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array13
	%.array14 = getelementptr [64 x float]* %inter, i32 0, i32 13		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array14
	%.array15 = getelementptr [64 x float]* %inter, i32 0, i32 14		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array15
	%.array16 = getelementptr [64 x float]* %inter, i32 0, i32 15		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array16
	%.array17 = getelementptr [64 x float]* %inter, i32 0, i32 16		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array17
	%.array18 = getelementptr [64 x float]* %inter, i32 0, i32 17		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array18
	%.array19 = getelementptr [64 x float]* %inter, i32 0, i32 18		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array19
	%.array20 = getelementptr [64 x float]* %inter, i32 0, i32 19		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array20
	%.array21 = getelementptr [64 x float]* %inter, i32 0, i32 20		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array21
	%.array22 = getelementptr [64 x float]* %inter, i32 0, i32 21		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array22
	%.array23 = getelementptr [64 x float]* %inter, i32 0, i32 22		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array23
	%.array24 = getelementptr [64 x float]* %inter, i32 0, i32 23		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array24
	%.array25 = getelementptr [64 x float]* %inter, i32 0, i32 24		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array25
	%.array26 = getelementptr [64 x float]* %inter, i32 0, i32 25		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array26
	%.array27 = getelementptr [64 x float]* %inter, i32 0, i32 26		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array27
	%.array28 = getelementptr [64 x float]* %inter, i32 0, i32 27		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array28
	%.array29 = getelementptr [64 x float]* %inter, i32 0, i32 28		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array29
	%.array30 = getelementptr [64 x float]* %inter, i32 0, i32 29		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array30
	%.array31 = getelementptr [64 x float]* %inter, i32 0, i32 30		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array31
	%.array32 = getelementptr [64 x float]* %inter, i32 0, i32 31		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array32
	%.array33 = getelementptr [64 x float]* %inter, i32 0, i32 32		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array33
	%.array34 = getelementptr [64 x float]* %inter, i32 0, i32 33		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array34
	%.array35 = getelementptr [64 x float]* %inter, i32 0, i32 34		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array35
	%.array36 = getelementptr [64 x float]* %inter, i32 0, i32 35		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array36
	%.array37 = getelementptr [64 x float]* %inter, i32 0, i32 36		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array37
	%.array38 = getelementptr [64 x float]* %inter, i32 0, i32 37		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array38
	%.array39 = getelementptr [64 x float]* %inter, i32 0, i32 38		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array39
	%.array40 = getelementptr [64 x float]* %inter, i32 0, i32 39		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array40
	%.array41 = getelementptr [64 x float]* %inter, i32 0, i32 40		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array41
	%.array42 = getelementptr [64 x float]* %inter, i32 0, i32 41		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array42
	%.array43 = getelementptr [64 x float]* %inter, i32 0, i32 42		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array43
	%.array44 = getelementptr [64 x float]* %inter, i32 0, i32 43		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array44
	%.array45 = getelementptr [64 x float]* %inter, i32 0, i32 44		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array45
	%.array46 = getelementptr [64 x float]* %inter, i32 0, i32 45		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array46
	%.array47 = getelementptr [64 x float]* %inter, i32 0, i32 46		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array47
	%.array48 = getelementptr [64 x float]* %inter, i32 0, i32 47		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array48
	%.array49 = getelementptr [64 x float]* %inter, i32 0, i32 48		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array49
	%.array50 = getelementptr [64 x float]* %inter, i32 0, i32 49		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array50
	%.array51 = getelementptr [64 x float]* %inter, i32 0, i32 50		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array51
	%.array52 = getelementptr [64 x float]* %inter, i32 0, i32 51		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array52
	%.array53 = getelementptr [64 x float]* %inter, i32 0, i32 52		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array53
	%.array54 = getelementptr [64 x float]* %inter, i32 0, i32 53		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array54
	%.array55 = getelementptr [64 x float]* %inter, i32 0, i32 54		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array55
	%.array56 = getelementptr [64 x float]* %inter, i32 0, i32 55		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array56
	%.array57 = getelementptr [64 x float]* %inter, i32 0, i32 56		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array57
	%.array58 = getelementptr [64 x float]* %inter, i32 0, i32 57		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array58
	%.array59 = getelementptr [64 x float]* %inter, i32 0, i32 58		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array59
	%.array60 = getelementptr [64 x float]* %inter, i32 0, i32 59		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array60
	%.array61 = getelementptr [64 x float]* %inter, i32 0, i32 60		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array61
	%.array62 = getelementptr [64 x float]* %inter, i32 0, i32 61		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array62
	%.array63 = getelementptr [64 x float]* %inter, i32 0, i32 62		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array63
	%.array64 = getelementptr [64 x float]* %inter, i32 0, i32 63		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array64
	%tmp = load i32* %width.addr		; <i32> [#uses=1]
	store i32 %tmp, i32* %step
	store i32 0, i32* %inputIndex
	store i32 0, i32* %dctIndex
	store i32 0, i32* %interIndex
	store i32 0, i32* %outputIndex
	%tmp69 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul = mul i32 %tmp69, 8		; <i32> [#uses=1]
	%tmp70 = load i32* %width.addr		; <i32> [#uses=1]
	%mul71 = mul i32 %mul, %tmp70		; <i32> [#uses=1]
	%tmp72 = load i32* %groupIdx		; <i32> [#uses=1]
	%mul73 = mul i32 %tmp72, 8		; <i32> [#uses=1]
	%add = add i32 %mul71, %mul73		; <i32> [#uses=1]
	store i32 %add, i32* %inputIndex
	store i32 0, i32* %n2
	br label %for.cond

for.cond:		; preds = %for.inc113, %entry
	%tmp74 = load i32* %n2		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp74, 8		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end116

for.body:		; preds = %for.cond
	store i32 0, i32* %dctIndex
	store i32 0, i32* %interIndex
	store i32 0, i32* %k1
	br label %for.cond75

for.cond75:		; preds = %for.inc106, %for.body
	%tmp76 = load i32* %k1		; <i32> [#uses=1]
	%cmp77 = icmp ult i32 %tmp76, 8		; <i1> [#uses=1]
	br i1 %cmp77, label %for.body78, label %for.end109

for.body78:		; preds = %for.cond75
	store i32 0, i32* %n1
	br label %for.cond79

for.cond79:		; preds = %for.inc, %for.body78
	%tmp80 = load i32* %n1		; <i32> [#uses=1]
	%cmp81 = icmp ult i32 %tmp80, 8		; <i1> [#uses=1]
	br i1 %cmp81, label %for.body82, label %for.end

for.body82:		; preds = %for.cond79
	%tmp83 = load i32* %interIndex		; <i32> [#uses=1]
	%tmp84 = load i32* %n2		; <i32> [#uses=1]
	%add85 = add i32 %tmp83, %tmp84		; <i32> [#uses=1]
	%arraydecay = getelementptr [64 x float]* %inter, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx = getelementptr float* %arraydecay, i32 %add85		; <float*> [#uses=2]
	%tmp86 = load float* %arrayidx		; <float> [#uses=1]
	%tmp87 = load i32* %dctIndex		; <i32> [#uses=1]
	%tmp88 = load i32* %n1		; <i32> [#uses=1]
	%add89 = add i32 %tmp87, %tmp88		; <i32> [#uses=1]
	%tmp90 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx91 = getelementptr float addrspace(1)* %tmp90, i32 %add89		; <float addrspace(1)*> [#uses=1]
	%tmp92 = load float addrspace(1)* %arrayidx91		; <float> [#uses=1]
	%tmp93 = load i32* %inputIndex		; <i32> [#uses=1]
	%tmp94 = load i32* %n1		; <i32> [#uses=1]
	%add95 = add i32 %tmp93, %tmp94		; <i32> [#uses=1]
	%tmp96 = load float addrspace(1)** %input.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx97 = getelementptr float addrspace(1)* %tmp96, i32 %add95		; <float addrspace(1)*> [#uses=1]
	%tmp98 = load float addrspace(1)* %arrayidx97		; <float> [#uses=1]
	%mul99 = fmul float %tmp92, %tmp98		; <float> [#uses=1]
	%add100 = fadd float %tmp86, %mul99		; <float> [#uses=1]
	store float %add100, float* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body82
	%tmp101 = load i32* %n1		; <i32> [#uses=1]
	%inc = add i32 %tmp101, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %n1
	br label %for.cond79

for.end:		; preds = %for.cond79
	%tmp102 = load i32* %dctIndex		; <i32> [#uses=1]
	%add103 = add i32 %tmp102, 8		; <i32> [#uses=1]
	store i32 %add103, i32* %dctIndex
	%tmp104 = load i32* %interIndex		; <i32> [#uses=1]
	%add105 = add i32 %tmp104, 8		; <i32> [#uses=1]
	store i32 %add105, i32* %interIndex
	br label %for.inc106

for.inc106:		; preds = %for.end
	%tmp107 = load i32* %k1		; <i32> [#uses=1]
	%inc108 = add i32 %tmp107, 1		; <i32> [#uses=1]
	store i32 %inc108, i32* %k1
	br label %for.cond75

for.end109:		; preds = %for.cond75
	%tmp110 = load i32* %inputIndex		; <i32> [#uses=1]
	%tmp111 = load i32* %step		; <i32> [#uses=1]
	%add112 = add i32 %tmp110, %tmp111		; <i32> [#uses=1]
	store i32 %add112, i32* %inputIndex
	br label %for.inc113

for.inc113:		; preds = %for.end109
	%tmp114 = load i32* %n2		; <i32> [#uses=1]
	%inc115 = add i32 %tmp114, 1		; <i32> [#uses=1]
	store i32 %inc115, i32* %n2
	br label %for.cond

for.end116:		; preds = %for.cond
	store i32 0, i32* %dctIndex
	%tmp117 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul118 = mul i32 %tmp117, 8		; <i32> [#uses=1]
	%tmp119 = load i32* %width.addr		; <i32> [#uses=1]
	%mul120 = mul i32 %mul118, %tmp119		; <i32> [#uses=1]
	%tmp121 = load i32* %groupIdx		; <i32> [#uses=1]
	%mul122 = mul i32 %tmp121, 8		; <i32> [#uses=1]
	%add123 = add i32 %mul120, %mul122		; <i32> [#uses=1]
	store i32 %add123, i32* %outputIndex
	store i32 0, i32* %k2
	br label %for.cond124

for.cond124:		; preds = %for.inc176, %for.end116
	%tmp125 = load i32* %k2		; <i32> [#uses=1]
	%cmp126 = icmp ult i32 %tmp125, 8		; <i1> [#uses=1]
	br i1 %cmp126, label %for.body127, label %for.end179

for.body127:		; preds = %for.cond124
	store i32 0, i32* %interIndex
	store i32 0, i32* %k1
	br label %for.cond128

for.cond128:		; preds = %for.inc167, %for.body127
	%tmp129 = load i32* %k1		; <i32> [#uses=1]
	%cmp130 = icmp ult i32 %tmp129, 8		; <i1> [#uses=1]
	br i1 %cmp130, label %for.body131, label %for.end170

for.body131:		; preds = %for.cond128
	%tmp132 = load i32* %outputIndex		; <i32> [#uses=1]
	%tmp133 = load i32* %k1		; <i32> [#uses=1]
	%add134 = add i32 %tmp132, %tmp133		; <i32> [#uses=1]
	%tmp135 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx136 = getelementptr float addrspace(1)* %tmp135, i32 %add134		; <float addrspace(1)*> [#uses=1]
	store float 0.000000e+000, float addrspace(1)* %arrayidx136
	store i32 0, i32* %n2
	br label %for.cond137

for.cond137:		; preds = %for.inc161, %for.body131
	%tmp138 = load i32* %n2		; <i32> [#uses=1]
	%cmp139 = icmp ult i32 %tmp138, 8		; <i1> [#uses=1]
	br i1 %cmp139, label %for.body140, label %for.end164

for.body140:		; preds = %for.cond137
	%tmp141 = load i32* %outputIndex		; <i32> [#uses=1]
	%tmp142 = load i32* %k1		; <i32> [#uses=1]
	%add143 = add i32 %tmp141, %tmp142		; <i32> [#uses=1]
	%tmp144 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx145 = getelementptr float addrspace(1)* %tmp144, i32 %add143		; <float addrspace(1)*> [#uses=2]
	%tmp146 = load float addrspace(1)* %arrayidx145		; <float> [#uses=1]
	%tmp147 = load i32* %dctIndex		; <i32> [#uses=1]
	%tmp148 = load i32* %n2		; <i32> [#uses=1]
	%add149 = add i32 %tmp147, %tmp148		; <i32> [#uses=1]
	%tmp150 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx151 = getelementptr float addrspace(1)* %tmp150, i32 %add149		; <float addrspace(1)*> [#uses=1]
	%tmp152 = load float addrspace(1)* %arrayidx151		; <float> [#uses=1]
	%tmp153 = load i32* %interIndex		; <i32> [#uses=1]
	%tmp154 = load i32* %n2		; <i32> [#uses=1]
	%add155 = add i32 %tmp153, %tmp154		; <i32> [#uses=1]
	%arraydecay156 = getelementptr [64 x float]* %inter, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx157 = getelementptr float* %arraydecay156, i32 %add155		; <float*> [#uses=1]
	%tmp158 = load float* %arrayidx157		; <float> [#uses=1]
	%mul159 = fmul float %tmp152, %tmp158		; <float> [#uses=1]
	%add160 = fadd float %tmp146, %mul159		; <float> [#uses=1]
	store float %add160, float addrspace(1)* %arrayidx145
	br label %for.inc161

for.inc161:		; preds = %for.body140
	%tmp162 = load i32* %n2		; <i32> [#uses=1]
	%inc163 = add i32 %tmp162, 1		; <i32> [#uses=1]
	store i32 %inc163, i32* %n2
	br label %for.cond137

for.end164:		; preds = %for.cond137
	%tmp165 = load i32* %interIndex		; <i32> [#uses=1]
	%add166 = add i32 %tmp165, 8		; <i32> [#uses=1]
	store i32 %add166, i32* %interIndex
	br label %for.inc167

for.inc167:		; preds = %for.end164
	%tmp168 = load i32* %k1		; <i32> [#uses=1]
	%inc169 = add i32 %tmp168, 1		; <i32> [#uses=1]
	store i32 %inc169, i32* %k1
	br label %for.cond128

for.end170:		; preds = %for.cond128
	%tmp171 = load i32* %dctIndex		; <i32> [#uses=1]
	%add172 = add i32 %tmp171, 8		; <i32> [#uses=1]
	store i32 %add172, i32* %dctIndex
	%tmp173 = load i32* %outputIndex		; <i32> [#uses=1]
	%tmp174 = load i32* %step		; <i32> [#uses=1]
	%add175 = add i32 %tmp173, %tmp174		; <i32> [#uses=1]
	store i32 %add175, i32* %outputIndex
	br label %for.inc176

for.inc176:		; preds = %for.end170
	%tmp177 = load i32* %k2		; <i32> [#uses=1]
	%inc178 = add i32 %tmp177, 1		; <i32> [#uses=1]
	store i32 %inc178, i32* %k2
	br label %for.cond124

for.end179:		; preds = %for.cond124
	ret void
}

define void @DCT_CPU_VECTOR(float addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%input.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=2]
	%dct.addr = alloca <8 x float> addrspace(1)*		; <<8 x float> addrspace(1)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=5]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%k1 = alloca i32, align 4		; <i32*> [#uses=11]
	%k2 = alloca i32, align 4		; <i32*> [#uses=5]
	%n1 = alloca i32, align 4		; <i32*> [#uses=0]
	%n2 = alloca i32, align 4		; <i32*> [#uses=5]
	%acc = alloca [64 x float], align 4		; <[64 x float]*> [#uses=2]
	%inter = alloca [8 x <8 x float>], align 32		; <[8 x <8 x float>]*> [#uses=2]
	%temp = alloca <8 x float>, align 32		; <<8 x float>*> [#uses=18]
	%step = alloca i32, align 4		; <i32*> [#uses=2]
	%inputIndex = alloca i32, align 4		; <i32*> [#uses=5]
	%dctIndex = alloca i32, align 4		; <i32*> [#uses=1]
	%interIndex = alloca i32, align 4		; <i32*> [#uses=5]
	%outputIndex = alloca i32, align 4		; <i32*> [#uses=5]
	%i = alloca i32, align 4		; <i32*> [#uses=6]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
	store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
	store i32 %width, i32* %width.addr
	%call = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %groupIdx
	%call1 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %groupIdy
	%tmp = load i32* %width.addr		; <i32> [#uses=1]
	%div = udiv i32 %tmp, 8		; <i32> [#uses=1]
	store i32 %div, i32* %step
	store i32 0, i32* %inputIndex
	store i32 0, i32* %dctIndex
	store i32 0, i32* %interIndex
	store i32 0, i32* %outputIndex
	%tmp6 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul = mul i32 %tmp6, 8		; <i32> [#uses=1]
	%tmp7 = load i32* %width.addr		; <i32> [#uses=1]
	%mul8 = mul i32 %mul, %tmp7		; <i32> [#uses=1]
	%div9 = udiv i32 %mul8, 8		; <i32> [#uses=1]
	%tmp10 = load i32* %groupIdx		; <i32> [#uses=1]
	%add = add i32 %div9, %tmp10		; <i32> [#uses=1]
	store i32 %add, i32* %inputIndex
	store i32 0, i32* %n2
	br label %for.cond

for.cond:		; preds = %for.inc57, %entry
	%tmp11 = load i32* %n2		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp11, 8		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end60

for.body:		; preds = %for.cond
	store i32 0, i32* %interIndex
	store i32 0, i32* %k1
	br label %for.cond12

for.cond12:		; preds = %for.inc, %for.body
	%tmp13 = load i32* %k1		; <i32> [#uses=1]
	%cmp14 = icmp ult i32 %tmp13, 8		; <i1> [#uses=1]
	br i1 %cmp14, label %for.body15, label %for.end

for.body15:		; preds = %for.cond12
	%tmp16 = load i32* %k1		; <i32> [#uses=1]
	%tmp17 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr <8 x float> addrspace(1)* %tmp17, i32 %tmp16		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp18 = load <8 x float> addrspace(1)* %arrayidx		; <<8 x float>> [#uses=1]
	%tmp19 = load i32* %inputIndex		; <i32> [#uses=1]
	%tmp20 = load <8 x float> addrspace(1)** %input.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx21 = getelementptr <8 x float> addrspace(1)* %tmp20, i32 %tmp19		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp22 = load <8 x float> addrspace(1)* %arrayidx21		; <<8 x float>> [#uses=1]
	%mul23 = fmul <8 x float> %tmp18, %tmp22		; <<8 x float>> [#uses=1]
	store <8 x float> %mul23, <8 x float>* %temp
	%tmp24 = load i32* %interIndex		; <i32> [#uses=1]
	%tmp25 = load i32* %n2		; <i32> [#uses=1]
	%add26 = add i32 %tmp24, %tmp25		; <i32> [#uses=1]
	%arraydecay = getelementptr [64 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx27 = getelementptr float* %arraydecay, i32 %add26		; <float*> [#uses=1]
	%tmp28 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp29 = extractelement <8 x float> %tmp28, i32 0		; <float> [#uses=1]
	%tmp30 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp31 = extractelement <8 x float> %tmp30, i32 1		; <float> [#uses=1]
	%add32 = fadd float %tmp29, %tmp31		; <float> [#uses=1]
	%tmp33 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp34 = extractelement <8 x float> %tmp33, i32 2		; <float> [#uses=1]
	%add35 = fadd float %add32, %tmp34		; <float> [#uses=1]
	%tmp36 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp37 = extractelement <8 x float> %tmp36, i32 3		; <float> [#uses=1]
	%add38 = fadd float %add35, %tmp37		; <float> [#uses=1]
	%tmp39 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp40 = extractelement <8 x float> %tmp39, i32 4		; <float> [#uses=1]
	%add41 = fadd float %add38, %tmp40		; <float> [#uses=1]
	%tmp42 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp43 = extractelement <8 x float> %tmp42, i32 5		; <float> [#uses=1]
	%add44 = fadd float %add41, %tmp43		; <float> [#uses=1]
	%tmp45 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp46 = extractelement <8 x float> %tmp45, i32 6		; <float> [#uses=1]
	%add47 = fadd float %add44, %tmp46		; <float> [#uses=1]
	%tmp48 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp49 = extractelement <8 x float> %tmp48, i32 7		; <float> [#uses=1]
	%add50 = fadd float %add47, %tmp49		; <float> [#uses=1]
	store float %add50, float* %arrayidx27
	%tmp51 = load i32* %interIndex		; <i32> [#uses=1]
	%add52 = add i32 %tmp51, 8		; <i32> [#uses=1]
	store i32 %add52, i32* %interIndex
	br label %for.inc

for.inc:		; preds = %for.body15
	%tmp53 = load i32* %k1		; <i32> [#uses=1]
	%inc = add i32 %tmp53, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %k1
	br label %for.cond12

for.end:		; preds = %for.cond12
	%tmp54 = load i32* %inputIndex		; <i32> [#uses=1]
	%tmp55 = load i32* %step		; <i32> [#uses=1]
	%add56 = add i32 %tmp54, %tmp55		; <i32> [#uses=1]
	store i32 %add56, i32* %inputIndex
	br label %for.inc57

for.inc57:		; preds = %for.end
	%tmp58 = load i32* %n2		; <i32> [#uses=1]
	%inc59 = add i32 %tmp58, 1		; <i32> [#uses=1]
	store i32 %inc59, i32* %n2
	br label %for.cond

for.end60:		; preds = %for.cond
	store i32 0, i32* %i
	br label %for.cond62

for.cond62:		; preds = %for.inc73, %for.end60
	%tmp63 = load i32* %i		; <i32> [#uses=1]
	%cmp64 = icmp slt i32 %tmp63, 8		; <i1> [#uses=1]
	br i1 %cmp64, label %for.body65, label %for.end76

for.body65:		; preds = %for.cond62
	%tmp66 = load i32* %i		; <i32> [#uses=1]
	%arraydecay67 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 0		; <<8 x float>*> [#uses=1]
	%arrayidx68 = getelementptr <8 x float>* %arraydecay67, i32 %tmp66		; <<8 x float>*> [#uses=1]
	%tmp69 = load i32* %i		; <i32> [#uses=1]
	%arraydecay70 = getelementptr [64 x float]* %acc, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx71 = getelementptr float* %arraydecay70, i32 0		; <float*> [#uses=1]
	%call72 = call <8 x float> @__vload8_f8(i32 %tmp69, float* %arrayidx71)		; <<8 x float>> [#uses=1]
	store <8 x float> %call72, <8 x float>* %arrayidx68
	br label %for.inc73

for.inc73:		; preds = %for.body65
	%tmp74 = load i32* %i		; <i32> [#uses=1]
	%inc75 = add i32 %tmp74, 1		; <i32> [#uses=1]
	store i32 %inc75, i32* %i
	br label %for.cond62

for.end76:		; preds = %for.cond62
	%tmp77 = load i32* %groupIdy		; <i32> [#uses=1]
	%mul78 = mul i32 %tmp77, 8		; <i32> [#uses=1]
	%tmp79 = load i32* %width.addr		; <i32> [#uses=1]
	%mul80 = mul i32 %mul78, %tmp79		; <i32> [#uses=1]
	%tmp81 = load i32* %groupIdx		; <i32> [#uses=1]
	%mul82 = mul i32 %tmp81, 8		; <i32> [#uses=1]
	%add83 = add i32 %mul80, %mul82		; <i32> [#uses=1]
	store i32 %add83, i32* %outputIndex
	store i32 0, i32* %k2
	br label %for.cond84

for.cond84:		; preds = %for.inc136, %for.end76
	%tmp85 = load i32* %k2		; <i32> [#uses=1]
	%cmp86 = icmp ult i32 %tmp85, 8		; <i1> [#uses=1]
	br i1 %cmp86, label %for.body87, label %for.end139

for.body87:		; preds = %for.cond84
	store i32 0, i32* %k1
	br label %for.cond88

for.cond88:		; preds = %for.inc129, %for.body87
	%tmp89 = load i32* %k1		; <i32> [#uses=1]
	%cmp90 = icmp ult i32 %tmp89, 8		; <i1> [#uses=1]
	br i1 %cmp90, label %for.body91, label %for.end132

for.body91:		; preds = %for.cond88
	%tmp92 = load i32* %k2		; <i32> [#uses=1]
	%tmp93 = load <8 x float> addrspace(1)** %dct.addr		; <<8 x float> addrspace(1)*> [#uses=1]
	%arrayidx94 = getelementptr <8 x float> addrspace(1)* %tmp93, i32 %tmp92		; <<8 x float> addrspace(1)*> [#uses=1]
	%tmp95 = load <8 x float> addrspace(1)* %arrayidx94		; <<8 x float>> [#uses=1]
	%tmp96 = load i32* %k1		; <i32> [#uses=1]
	%arraydecay97 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 0		; <<8 x float>*> [#uses=1]
	%arrayidx98 = getelementptr <8 x float>* %arraydecay97, i32 %tmp96		; <<8 x float>*> [#uses=1]
	%tmp99 = load <8 x float>* %arrayidx98		; <<8 x float>> [#uses=1]
	%mul100 = fmul <8 x float> %tmp95, %tmp99		; <<8 x float>> [#uses=1]
	store <8 x float> %mul100, <8 x float>* %temp
	%tmp101 = load i32* %outputIndex		; <i32> [#uses=1]
	%tmp102 = load i32* %k1		; <i32> [#uses=1]
	%add103 = add i32 %tmp101, %tmp102		; <i32> [#uses=1]
	%tmp104 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx105 = getelementptr float addrspace(1)* %tmp104, i32 %add103		; <float addrspace(1)*> [#uses=1]
	%tmp106 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp107 = extractelement <8 x float> %tmp106, i32 0		; <float> [#uses=1]
	%tmp108 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp109 = extractelement <8 x float> %tmp108, i32 1		; <float> [#uses=1]
	%add110 = fadd float %tmp107, %tmp109		; <float> [#uses=1]
	%tmp111 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp112 = extractelement <8 x float> %tmp111, i32 2		; <float> [#uses=1]
	%add113 = fadd float %add110, %tmp112		; <float> [#uses=1]
	%tmp114 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp115 = extractelement <8 x float> %tmp114, i32 3		; <float> [#uses=1]
	%add116 = fadd float %add113, %tmp115		; <float> [#uses=1]
	%tmp117 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp118 = extractelement <8 x float> %tmp117, i32 4		; <float> [#uses=1]
	%add119 = fadd float %add116, %tmp118		; <float> [#uses=1]
	%tmp120 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp121 = extractelement <8 x float> %tmp120, i32 5		; <float> [#uses=1]
	%add122 = fadd float %add119, %tmp121		; <float> [#uses=1]
	%tmp123 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp124 = extractelement <8 x float> %tmp123, i32 6		; <float> [#uses=1]
	%add125 = fadd float %add122, %tmp124		; <float> [#uses=1]
	%tmp126 = load <8 x float>* %temp		; <<8 x float>> [#uses=1]
	%tmp127 = extractelement <8 x float> %tmp126, i32 7		; <float> [#uses=1]
	%add128 = fadd float %add125, %tmp127		; <float> [#uses=1]
	store float %add128, float addrspace(1)* %arrayidx105
	br label %for.inc129

for.inc129:		; preds = %for.body91
	%tmp130 = load i32* %k1		; <i32> [#uses=1]
	%inc131 = add i32 %tmp130, 1		; <i32> [#uses=1]
	store i32 %inc131, i32* %k1
	br label %for.cond88

for.end132:		; preds = %for.cond88
	%tmp133 = load i32* %outputIndex		; <i32> [#uses=1]
	%tmp134 = load i32* %width.addr		; <i32> [#uses=1]
	%add135 = add i32 %tmp133, %tmp134		; <i32> [#uses=1]
	store i32 %add135, i32* %outputIndex
	br label %for.inc136

for.inc136:		; preds = %for.end132
	%tmp137 = load i32* %k2		; <i32> [#uses=1]
	%inc138 = add i32 %tmp137, 1		; <i32> [#uses=1]
	store i32 %inc138, i32* %k2
	br label %for.cond84

for.end139:		; preds = %for.cond84
	ret void
}

declare <8 x float> @__vload8_f8(i32, float*)
