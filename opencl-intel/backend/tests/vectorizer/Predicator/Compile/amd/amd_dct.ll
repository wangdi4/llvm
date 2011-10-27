; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_dct.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [8 x i8] c"2222000\00"		; <[8 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, i32, ...)* @DCT to i8*), i8* getelementptr ([8 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: getIdx

define i32 @getIdx(i32 %blockIdx, i32 %blockIdy, i32 %localIdx, i32 %localIdy, i32 %blockWidth, i32 %globalWidth) nounwind {
entry:
	%retval = alloca i32		; <i32*> [#uses=2]
	%blockIdx.addr = alloca i32		; <i32*> [#uses=2]
	%blockIdy.addr = alloca i32		; <i32*> [#uses=2]
	%localIdx.addr = alloca i32		; <i32*> [#uses=2]
	%localIdy.addr = alloca i32		; <i32*> [#uses=2]
	%blockWidth.addr = alloca i32		; <i32*> [#uses=3]
	%globalWidth.addr = alloca i32		; <i32*> [#uses=2]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=2]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=2]
	store i32 %blockIdx, i32* %blockIdx.addr
	store i32 %blockIdy, i32* %blockIdy.addr
	store i32 %localIdx, i32* %localIdx.addr
	store i32 %localIdy, i32* %localIdy.addr
	store i32 %blockWidth, i32* %blockWidth.addr
	store i32 %globalWidth, i32* %globalWidth.addr
	%tmp = load i32* %blockIdx.addr		; <i32> [#uses=1]
	%tmp1 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp1		; <i32> [#uses=1]
	%tmp2 = load i32* %localIdx.addr		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp2		; <i32> [#uses=1]
	store i32 %add, i32* %globalIdx
	%tmp4 = load i32* %blockIdy.addr		; <i32> [#uses=1]
	%tmp5 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul6 = mul i32 %tmp4, %tmp5		; <i32> [#uses=1]
	%tmp7 = load i32* %localIdy.addr		; <i32> [#uses=1]
	%add8 = add i32 %mul6, %tmp7		; <i32> [#uses=1]
	store i32 %add8, i32* %globalIdy
	%tmp9 = load i32* %globalIdy		; <i32> [#uses=1]
	%tmp10 = load i32* %globalWidth.addr		; <i32> [#uses=1]
	%mul11 = mul i32 %tmp9, %tmp10		; <i32> [#uses=1]
	%tmp12 = load i32* %globalIdx		; <i32> [#uses=1]
	%add13 = add i32 %mul11, %tmp12		; <i32> [#uses=1]
	store i32 %add13, i32* %retval
	%0 = load i32* %retval		; <i32> [#uses=1]
	ret i32 %0
}

define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, float addrspace(1)* %inter, i32 %width, i32 %blockWidth, i32 %inverse, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%input.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=2]
	%dct.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%inter.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=4]
	%blockWidth.addr = alloca i32		; <i32*> [#uses=9]
	%inverse.addr = alloca i32		; <i32*> [#uses=3]
	%globalIdx = alloca i32, align 4		; <i32*> [#uses=2]
	%globalIdy = alloca i32, align 4		; <i32*> [#uses=2]
	%groupIdx = alloca i32, align 4		; <i32*> [#uses=3]
	%groupIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	%j = alloca i32, align 4		; <i32*> [#uses=4]
	%k = alloca i32, align 4		; <i32*> [#uses=14]
	%idx = alloca i32, align 4		; <i32*> [#uses=3]
	%acc = alloca float, align 4		; <float*> [#uses=8]
	%index1 = alloca i32, align 4		; <i32*> [#uses=2]
	%index2 = alloca i32, align 4		; <i32*> [#uses=2]
	%index152 = alloca i32, align 4		; <i32*> [#uses=2]
	%index261 = alloca i32, align 4		; <i32*> [#uses=2]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store float addrspace(1)* %input, float addrspace(1)** %input.addr
	store float addrspace(1)* %dct, float addrspace(1)** %dct.addr
	store float addrspace(1)* %inter, float addrspace(1)** %inter.addr
	store i32 %width, i32* %width.addr
	store i32 %blockWidth, i32* %blockWidth.addr
	store i32 %inverse, i32* %inverse.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %globalIdx
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalIdy
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupIdx
	%call3 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %groupIdy
	%call4 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call4, i32* %i
	%call5 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call5, i32* %j
	%tmp = load i32* %globalIdy		; <i32> [#uses=1]
	%tmp6 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp6		; <i32> [#uses=1]
	%tmp7 = load i32* %globalIdx		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp7		; <i32> [#uses=1]
	store i32 %add, i32* %idx
	store float 0.000000e+000, float* %acc
	store i32 0, i32* %k
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp9 = load i32* %k		; <i32> [#uses=1]
	%tmp10 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp9, %tmp10		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp12 = load i32* %inverse.addr		; <i32> [#uses=1]
	%tobool = icmp ne i32 %tmp12, 0		; <i1> [#uses=1]
	br i1 %tobool, label %cond.true, label %cond.false

cond.true:		; preds = %for.body
	%tmp13 = load i32* %i		; <i32> [#uses=1]
	%tmp14 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul15 = mul i32 %tmp13, %tmp14		; <i32> [#uses=1]
	%tmp16 = load i32* %k		; <i32> [#uses=1]
	%add17 = add i32 %mul15, %tmp16		; <i32> [#uses=1]
	br label %cond.end

cond.false:		; preds = %for.body
	%tmp18 = load i32* %k		; <i32> [#uses=1]
	%tmp19 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul20 = mul i32 %tmp18, %tmp19		; <i32> [#uses=1]
	%tmp21 = load i32* %i		; <i32> [#uses=1]
	%add22 = add i32 %mul20, %tmp21		; <i32> [#uses=1]
	br label %cond.end

cond.end:		; preds = %cond.false, %cond.true
	%cond = phi i32 [ %add17, %cond.true ], [ %add22, %cond.false ]		; <i32> [#uses=1]
	store i32 %cond, i32* %index1
	%tmp24 = load i32* %groupIdx		; <i32> [#uses=1]
	%tmp25 = load i32* %groupIdy		; <i32> [#uses=1]
	%tmp26 = load i32* %j		; <i32> [#uses=1]
	%tmp27 = load i32* %k		; <i32> [#uses=1]
	%tmp28 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%tmp29 = load i32* %width.addr		; <i32> [#uses=1]
	%call30 = call i32 @getIdx(i32 %tmp24, i32 %tmp25, i32 %tmp26, i32 %tmp27, i32 %tmp28, i32 %tmp29)		; <i32> [#uses=1]
	store i32 %call30, i32* %index2
	%tmp31 = load float* %acc		; <float> [#uses=1]
	%tmp32 = load i32* %index1		; <i32> [#uses=1]
	%tmp33 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr float addrspace(1)* %tmp33, i32 %tmp32		; <float addrspace(1)*> [#uses=1]
	%tmp34 = load float addrspace(1)* %arrayidx		; <float> [#uses=1]
	%tmp35 = load i32* %index2		; <i32> [#uses=1]
	%tmp36 = load float addrspace(1)** %input.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx37 = getelementptr float addrspace(1)* %tmp36, i32 %tmp35		; <float addrspace(1)*> [#uses=1]
	%tmp38 = load float addrspace(1)* %arrayidx37		; <float> [#uses=1]
	%mul39 = fmul float %tmp34, %tmp38		; <float> [#uses=1]
	%add40 = fadd float %tmp31, %mul39		; <float> [#uses=1]
	store float %add40, float* %acc
	br label %for.inc

for.inc:		; preds = %cond.end
	%tmp41 = load i32* %k		; <i32> [#uses=1]
	%inc = add i32 %tmp41, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %k
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp42 = load i32* %idx		; <i32> [#uses=1]
	%tmp43 = load float addrspace(1)** %inter.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx44 = getelementptr float addrspace(1)* %tmp43, i32 %tmp42		; <float addrspace(1)*> [#uses=1]
	%tmp45 = load float* %acc		; <float> [#uses=1]
	store float %tmp45, float addrspace(1)* %arrayidx44
	call void @barrier(i32 1)
	store float 0.000000e+000, float* %acc
	store i32 0, i32* %k
	br label %for.cond46

for.cond46:		; preds = %for.inc89, %for.end
	%tmp47 = load i32* %k		; <i32> [#uses=1]
	%tmp48 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%cmp49 = icmp ult i32 %tmp47, %tmp48		; <i1> [#uses=1]
	br i1 %cmp49, label %for.body50, label %for.end92

for.body50:		; preds = %for.cond46
	%tmp53 = load i32* %groupIdx		; <i32> [#uses=1]
	%tmp54 = load i32* %groupIdy		; <i32> [#uses=1]
	%tmp55 = load i32* %k		; <i32> [#uses=1]
	%tmp56 = load i32* %i		; <i32> [#uses=1]
	%tmp57 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%tmp58 = load i32* %width.addr		; <i32> [#uses=1]
	%call59 = call i32 @getIdx(i32 %tmp53, i32 %tmp54, i32 %tmp55, i32 %tmp56, i32 %tmp57, i32 %tmp58)		; <i32> [#uses=1]
	store i32 %call59, i32* %index152
	%tmp62 = load i32* %inverse.addr		; <i32> [#uses=1]
	%tobool63 = icmp ne i32 %tmp62, 0		; <i1> [#uses=1]
	br i1 %tobool63, label %cond.true64, label %cond.false70

cond.true64:		; preds = %for.body50
	%tmp65 = load i32* %j		; <i32> [#uses=1]
	%tmp66 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul67 = mul i32 %tmp65, %tmp66		; <i32> [#uses=1]
	%tmp68 = load i32* %k		; <i32> [#uses=1]
	%add69 = add i32 %mul67, %tmp68		; <i32> [#uses=1]
	br label %cond.end76

cond.false70:		; preds = %for.body50
	%tmp71 = load i32* %k		; <i32> [#uses=1]
	%tmp72 = load i32* %blockWidth.addr		; <i32> [#uses=1]
	%mul73 = mul i32 %tmp71, %tmp72		; <i32> [#uses=1]
	%tmp74 = load i32* %j		; <i32> [#uses=1]
	%add75 = add i32 %mul73, %tmp74		; <i32> [#uses=1]
	br label %cond.end76

cond.end76:		; preds = %cond.false70, %cond.true64
	%cond77 = phi i32 [ %add69, %cond.true64 ], [ %add75, %cond.false70 ]		; <i32> [#uses=1]
	store i32 %cond77, i32* %index261
	%tmp78 = load float* %acc		; <float> [#uses=1]
	%tmp79 = load i32* %index152		; <i32> [#uses=1]
	%tmp80 = load float addrspace(1)** %inter.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx81 = getelementptr float addrspace(1)* %tmp80, i32 %tmp79		; <float addrspace(1)*> [#uses=1]
	%tmp82 = load float addrspace(1)* %arrayidx81		; <float> [#uses=1]
	%tmp83 = load i32* %index261		; <i32> [#uses=1]
	%tmp84 = load float addrspace(1)** %dct.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx85 = getelementptr float addrspace(1)* %tmp84, i32 %tmp83		; <float addrspace(1)*> [#uses=1]
	%tmp86 = load float addrspace(1)* %arrayidx85		; <float> [#uses=1]
	%mul87 = fmul float %tmp82, %tmp86		; <float> [#uses=1]
	%add88 = fadd float %tmp78, %mul87		; <float> [#uses=1]
	store float %add88, float* %acc
	br label %for.inc89

for.inc89:		; preds = %cond.end76
	%tmp90 = load i32* %k		; <i32> [#uses=1]
	%inc91 = add i32 %tmp90, 1		; <i32> [#uses=1]
	store i32 %inc91, i32* %k
	br label %for.cond46

for.end92:		; preds = %for.cond46
	%tmp93 = load i32* %idx		; <i32> [#uses=1]
	%tmp94 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx95 = getelementptr float addrspace(1)* %tmp94, i32 %tmp93		; <float addrspace(1)*> [#uses=1]
	%tmp96 = load float* %acc		; <float> [#uses=1]
	store float %tmp96, float addrspace(1)* %arrayidx95
	ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)
