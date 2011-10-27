; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIPrefixSum.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"2290\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, ...)* @prefixSum to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @prefixSum

define void @prefixSum(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(3)* %block, i32 %length, ...) nounwind {
entry:
	%output.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%input.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=3]
	%block.addr = alloca float addrspace(3)*		; <float addrspace(3)**> [#uses=12]
	%length.addr = alloca i32		; <i32*> [#uses=4]
	%tid = alloca i32, align 4		; <i32*> [#uses=16]
	%offset = alloca i32, align 4		; <i32*> [#uses=9]
	%d = alloca i32, align 4		; <i32*> [#uses=5]
	%ai = alloca i32, align 4		; <i32*> [#uses=2]
	%bi = alloca i32, align 4		; <i32*> [#uses=2]
	%d58 = alloca i32, align 4		; <i32*> [#uses=5]
	%ai71 = alloca i32, align 4		; <i32*> [#uses=3]
	%bi79 = alloca i32, align 4		; <i32*> [#uses=3]
	%t = alloca float, align 4		; <float*> [#uses=2]
	store float addrspace(1)* %output, float addrspace(1)** %output.addr
	store float addrspace(1)* %input, float addrspace(1)** %input.addr
	store float addrspace(3)* %block, float addrspace(3)** %block.addr
	store i32 %length, i32* %length.addr
	%call = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %tid
	store i32 1, i32* %offset
	%tmp = load i32* %tid		; <i32> [#uses=1]
	%mul = mul i32 2, %tmp		; <i32> [#uses=1]
	%tmp1 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr float addrspace(3)* %tmp1, i32 %mul		; <float addrspace(3)*> [#uses=1]
	%tmp2 = load i32* %tid		; <i32> [#uses=1]
	%mul3 = mul i32 2, %tmp2		; <i32> [#uses=1]
	%tmp4 = load float addrspace(1)** %input.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx5 = getelementptr float addrspace(1)* %tmp4, i32 %mul3		; <float addrspace(1)*> [#uses=1]
	%tmp6 = load float addrspace(1)* %arrayidx5		; <float> [#uses=1]
	store float %tmp6, float addrspace(3)* %arrayidx
	%tmp7 = load i32* %tid		; <i32> [#uses=1]
	%mul8 = mul i32 2, %tmp7		; <i32> [#uses=1]
	%add = add i32 %mul8, 1		; <i32> [#uses=1]
	%tmp9 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx10 = getelementptr float addrspace(3)* %tmp9, i32 %add		; <float addrspace(3)*> [#uses=1]
	%tmp11 = load i32* %tid		; <i32> [#uses=1]
	%mul12 = mul i32 2, %tmp11		; <i32> [#uses=1]
	%add13 = add i32 %mul12, 1		; <i32> [#uses=1]
	%tmp14 = load float addrspace(1)** %input.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx15 = getelementptr float addrspace(1)* %tmp14, i32 %add13		; <float addrspace(1)*> [#uses=1]
	%tmp16 = load float addrspace(1)* %arrayidx15		; <float> [#uses=1]
	store float %tmp16, float addrspace(3)* %arrayidx10
	%tmp18 = load i32* %length.addr		; <i32> [#uses=1]
	%shr = lshr i32 %tmp18, 1		; <i32> [#uses=1]
	store i32 %shr, i32* %d
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp19 = load i32* %d		; <i32> [#uses=1]
	%cmp = icmp sgt i32 %tmp19, 0		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	call void @barrier(i32 1)
	%tmp20 = load i32* %tid		; <i32> [#uses=1]
	%tmp21 = load i32* %d		; <i32> [#uses=1]
	%cmp22 = icmp slt i32 %tmp20, %tmp21		; <i1> [#uses=1]
	br i1 %cmp22, label %if.then, label %if.end

if.then:		; preds = %for.body
	%tmp24 = load i32* %offset		; <i32> [#uses=1]
	%tmp25 = load i32* %tid		; <i32> [#uses=1]
	%mul26 = mul i32 2, %tmp25		; <i32> [#uses=1]
	%add27 = add i32 %mul26, 1		; <i32> [#uses=1]
	%mul28 = mul i32 %tmp24, %add27		; <i32> [#uses=1]
	%sub = sub i32 %mul28, 1		; <i32> [#uses=1]
	store i32 %sub, i32* %ai
	%tmp30 = load i32* %offset		; <i32> [#uses=1]
	%tmp31 = load i32* %tid		; <i32> [#uses=1]
	%mul32 = mul i32 2, %tmp31		; <i32> [#uses=1]
	%add33 = add i32 %mul32, 2		; <i32> [#uses=1]
	%mul34 = mul i32 %tmp30, %add33		; <i32> [#uses=1]
	%sub35 = sub i32 %mul34, 1		; <i32> [#uses=1]
	store i32 %sub35, i32* %bi
	%tmp36 = load i32* %bi		; <i32> [#uses=1]
	%tmp37 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx38 = getelementptr float addrspace(3)* %tmp37, i32 %tmp36		; <float addrspace(3)*> [#uses=2]
	%tmp39 = load float addrspace(3)* %arrayidx38		; <float> [#uses=1]
	%tmp40 = load i32* %ai		; <i32> [#uses=1]
	%tmp41 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx42 = getelementptr float addrspace(3)* %tmp41, i32 %tmp40		; <float addrspace(3)*> [#uses=1]
	%tmp43 = load float addrspace(3)* %arrayidx42		; <float> [#uses=1]
	%add44 = fadd float %tmp39, %tmp43		; <float> [#uses=1]
	store float %add44, float addrspace(3)* %arrayidx38
	br label %if.end

if.end:		; preds = %if.then, %for.body
	%tmp45 = load i32* %offset		; <i32> [#uses=1]
	%mul46 = mul i32 %tmp45, 2		; <i32> [#uses=1]
	store i32 %mul46, i32* %offset
	br label %for.inc

for.inc:		; preds = %if.end
	%tmp47 = load i32* %d		; <i32> [#uses=1]
	%shr48 = ashr i32 %tmp47, 1		; <i32> [#uses=1]
	store i32 %shr48, i32* %d
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp49 = load i32* %tid		; <i32> [#uses=1]
	%cmp50 = icmp eq i32 %tmp49, 0		; <i1> [#uses=1]
	br i1 %cmp50, label %if.then51, label %if.end56

if.then51:		; preds = %for.end
	%tmp52 = load i32* %length.addr		; <i32> [#uses=1]
	%sub53 = sub i32 %tmp52, 1		; <i32> [#uses=1]
	%tmp54 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx55 = getelementptr float addrspace(3)* %tmp54, i32 %sub53		; <float addrspace(3)*> [#uses=1]
	store float 0.000000e+000, float addrspace(3)* %arrayidx55
	br label %if.end56

if.end56:		; preds = %if.then51, %for.end
	store i32 1, i32* %d58
	br label %for.cond59

for.cond59:		; preds = %for.inc105, %if.end56
	%tmp60 = load i32* %d58		; <i32> [#uses=1]
	%tmp61 = load i32* %length.addr		; <i32> [#uses=1]
	%cmp62 = icmp ult i32 %tmp60, %tmp61		; <i1> [#uses=1]
	br i1 %cmp62, label %for.body63, label %for.end108

for.body63:		; preds = %for.cond59
	%tmp64 = load i32* %offset		; <i32> [#uses=1]
	%shr65 = ashr i32 %tmp64, 1		; <i32> [#uses=1]
	store i32 %shr65, i32* %offset
	call void @barrier(i32 1)
	%tmp66 = load i32* %tid		; <i32> [#uses=1]
	%tmp67 = load i32* %d58		; <i32> [#uses=1]
	%cmp68 = icmp slt i32 %tmp66, %tmp67		; <i1> [#uses=1]
	br i1 %cmp68, label %if.then69, label %if.end104

if.then69:		; preds = %for.body63
	%tmp72 = load i32* %offset		; <i32> [#uses=1]
	%tmp73 = load i32* %tid		; <i32> [#uses=1]
	%mul74 = mul i32 2, %tmp73		; <i32> [#uses=1]
	%add75 = add i32 %mul74, 1		; <i32> [#uses=1]
	%mul76 = mul i32 %tmp72, %add75		; <i32> [#uses=1]
	%sub77 = sub i32 %mul76, 1		; <i32> [#uses=1]
	store i32 %sub77, i32* %ai71
	%tmp80 = load i32* %offset		; <i32> [#uses=1]
	%tmp81 = load i32* %tid		; <i32> [#uses=1]
	%mul82 = mul i32 2, %tmp81		; <i32> [#uses=1]
	%add83 = add i32 %mul82, 2		; <i32> [#uses=1]
	%mul84 = mul i32 %tmp80, %add83		; <i32> [#uses=1]
	%sub85 = sub i32 %mul84, 1		; <i32> [#uses=1]
	store i32 %sub85, i32* %bi79
	%tmp87 = load i32* %ai71		; <i32> [#uses=1]
	%tmp88 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx89 = getelementptr float addrspace(3)* %tmp88, i32 %tmp87		; <float addrspace(3)*> [#uses=1]
	%tmp90 = load float addrspace(3)* %arrayidx89		; <float> [#uses=1]
	store float %tmp90, float* %t
	%tmp91 = load i32* %ai71		; <i32> [#uses=1]
	%tmp92 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx93 = getelementptr float addrspace(3)* %tmp92, i32 %tmp91		; <float addrspace(3)*> [#uses=1]
	%tmp94 = load i32* %bi79		; <i32> [#uses=1]
	%tmp95 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx96 = getelementptr float addrspace(3)* %tmp95, i32 %tmp94		; <float addrspace(3)*> [#uses=1]
	%tmp97 = load float addrspace(3)* %arrayidx96		; <float> [#uses=1]
	store float %tmp97, float addrspace(3)* %arrayidx93
	%tmp98 = load i32* %bi79		; <i32> [#uses=1]
	%tmp99 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx100 = getelementptr float addrspace(3)* %tmp99, i32 %tmp98		; <float addrspace(3)*> [#uses=2]
	%tmp101 = load float addrspace(3)* %arrayidx100		; <float> [#uses=1]
	%tmp102 = load float* %t		; <float> [#uses=1]
	%add103 = fadd float %tmp101, %tmp102		; <float> [#uses=1]
	store float %add103, float addrspace(3)* %arrayidx100
	br label %if.end104

if.end104:		; preds = %if.then69, %for.body63
	br label %for.inc105

for.inc105:		; preds = %if.end104
	%tmp106 = load i32* %d58		; <i32> [#uses=1]
	%mul107 = mul i32 %tmp106, 2		; <i32> [#uses=1]
	store i32 %mul107, i32* %d58
	br label %for.cond59

for.end108:		; preds = %for.cond59
	call void @barrier(i32 1)
	%tmp109 = load i32* %tid		; <i32> [#uses=1]
	%mul110 = mul i32 2, %tmp109		; <i32> [#uses=1]
	%tmp111 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx112 = getelementptr float addrspace(1)* %tmp111, i32 %mul110		; <float addrspace(1)*> [#uses=1]
	%tmp113 = load i32* %tid		; <i32> [#uses=1]
	%mul114 = mul i32 2, %tmp113		; <i32> [#uses=1]
	%tmp115 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx116 = getelementptr float addrspace(3)* %tmp115, i32 %mul114		; <float addrspace(3)*> [#uses=1]
	%tmp117 = load float addrspace(3)* %arrayidx116		; <float> [#uses=1]
	store float %tmp117, float addrspace(1)* %arrayidx112
	%tmp118 = load i32* %tid		; <i32> [#uses=1]
	%mul119 = mul i32 2, %tmp118		; <i32> [#uses=1]
	%add120 = add i32 %mul119, 1		; <i32> [#uses=1]
	%tmp121 = load float addrspace(1)** %output.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx122 = getelementptr float addrspace(1)* %tmp121, i32 %add120		; <float addrspace(1)*> [#uses=1]
	%tmp123 = load i32* %tid		; <i32> [#uses=1]
	%mul124 = mul i32 2, %tmp123		; <i32> [#uses=1]
	%add125 = add i32 %mul124, 1		; <i32> [#uses=1]
	%tmp126 = load float addrspace(3)** %block.addr		; <float addrspace(3)*> [#uses=1]
	%arrayidx127 = getelementptr float addrspace(3)* %tmp126, i32 %add125		; <float addrspace(3)*> [#uses=1]
	%tmp128 = load float addrspace(3)* %arrayidx127		; <float> [#uses=1]
	store float %tmp128, float addrspace(1)* %arrayidx122
	ret void
}

declare i32 @get_local_id(i32)

declare void @barrier(i32)
