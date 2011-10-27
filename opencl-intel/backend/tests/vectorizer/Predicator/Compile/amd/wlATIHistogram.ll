; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIHistogram.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"1920\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, i8 addrspace(3)*, i32 addrspace(1)*, i32, ...)* @histogram to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @histogram

define void @histogram(i32 addrspace(1)* %data, i8 addrspace(3)* %sharedArray, i32 addrspace(1)* %binResult, i32 %BIN_SIZE, ...) nounwind {
entry:
	%data.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%sharedArray.addr = alloca i8 addrspace(3)*		; <i8 addrspace(3)**> [#uses=4]
	%binResult.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%BIN_SIZE.addr = alloca i32		; <i32*> [#uses=9]
	%localId = alloca i32, align 4		; <i32*> [#uses=5]
	%globalId = alloca i32, align 4		; <i32*> [#uses=2]
	%groupId = alloca i32, align 4		; <i32*> [#uses=2]
	%GROUP_SIZE = alloca i32, align 4		; <i32*> [#uses=5]
	%i = alloca i32, align 4		; <i32*> [#uses=5]
	%i11 = alloca i32, align 4		; <i32*> [#uses=5]
	%value = alloca i32, align 4		; <i32*> [#uses=2]
	%i40 = alloca i32, align 4		; <i32*> [#uses=6]
	%binCount = alloca i32, align 4		; <i32*> [#uses=4]
	%j = alloca i32, align 4		; <i32*> [#uses=5]
	store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr
	store i8 addrspace(3)* %sharedArray, i8 addrspace(3)** %sharedArray.addr
	store i32 addrspace(1)* %binResult, i32 addrspace(1)** %binResult.addr
	store i32 %BIN_SIZE, i32* %BIN_SIZE.addr
	%call = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %localId
	%call1 = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalId
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupId
	%call3 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	store i32 %call3, i32* %GROUP_SIZE
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp = load i32* %i		; <i32> [#uses=1]
	%tmp4 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp, %tmp4		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp5 = load i32* %localId		; <i32> [#uses=1]
	%tmp6 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp5, %tmp6		; <i32> [#uses=1]
	%tmp7 = load i32* %i		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp7		; <i32> [#uses=1]
	%tmp8 = load i8 addrspace(3)** %sharedArray.addr		; <i8 addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr i8 addrspace(3)* %tmp8, i32 %add		; <i8 addrspace(3)*> [#uses=1]
	store i8 0, i8 addrspace(3)* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp9 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp9, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	call void @barrier(i32 1)
	store i32 0, i32* %i11
	br label %for.cond12

for.cond12:		; preds = %for.inc35, %for.end
	%tmp13 = load i32* %i11		; <i32> [#uses=1]
	%tmp14 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%cmp15 = icmp ult i32 %tmp13, %tmp14		; <i1> [#uses=1]
	br i1 %cmp15, label %for.body16, label %for.end38

for.body16:		; preds = %for.cond12
	%tmp18 = load i32* %globalId		; <i32> [#uses=1]
	%tmp19 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%mul20 = mul i32 %tmp18, %tmp19		; <i32> [#uses=1]
	%tmp21 = load i32* %i11		; <i32> [#uses=1]
	%add22 = add i32 %mul20, %tmp21		; <i32> [#uses=1]
	%tmp23 = load i32 addrspace(1)** %data.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx24 = getelementptr i32 addrspace(1)* %tmp23, i32 %add22		; <i32 addrspace(1)*> [#uses=1]
	%tmp25 = load i32 addrspace(1)* %arrayidx24		; <i32> [#uses=1]
	store i32 %tmp25, i32* %value
	%tmp26 = load i32* %localId		; <i32> [#uses=1]
	%tmp27 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%mul28 = mul i32 %tmp26, %tmp27		; <i32> [#uses=1]
	%tmp29 = load i32* %value		; <i32> [#uses=1]
	%add30 = add i32 %mul28, %tmp29		; <i32> [#uses=1]
	%tmp31 = load i8 addrspace(3)** %sharedArray.addr		; <i8 addrspace(3)*> [#uses=1]
	%arrayidx32 = getelementptr i8 addrspace(3)* %tmp31, i32 %add30		; <i8 addrspace(3)*> [#uses=2]
	%tmp33 = load i8 addrspace(3)* %arrayidx32		; <i8> [#uses=1]
	%inc34 = add i8 %tmp33, 1		; <i8> [#uses=1]
	store i8 %inc34, i8 addrspace(3)* %arrayidx32
	br label %for.inc35

for.inc35:		; preds = %for.body16
	%tmp36 = load i32* %i11		; <i32> [#uses=1]
	%inc37 = add i32 %tmp36, 1		; <i32> [#uses=1]
	store i32 %inc37, i32* %i11
	br label %for.cond12

for.end38:		; preds = %for.cond12
	call void @barrier(i32 1)
	store i32 0, i32* %i40
	br label %for.cond41

for.cond41:		; preds = %for.inc85, %for.end38
	%tmp42 = load i32* %i40		; <i32> [#uses=1]
	%tmp43 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%tmp44 = load i32* %GROUP_SIZE		; <i32> [#uses=2]
	%cmp45 = icmp ne i32 %tmp44, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp45, i32 %tmp44, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp43, %nonzero		; <i32> [#uses=1]
	%cmp46 = icmp ult i32 %tmp42, %div		; <i1> [#uses=1]
	br i1 %cmp46, label %for.body47, label %for.end88

for.body47:		; preds = %for.cond41
	store i32 0, i32* %binCount
	store i32 0, i32* %j
	br label %for.cond50

for.cond50:		; preds = %for.inc69, %for.body47
	%tmp51 = load i32* %j		; <i32> [#uses=1]
	%tmp52 = load i32* %GROUP_SIZE		; <i32> [#uses=1]
	%cmp53 = icmp ult i32 %tmp51, %tmp52		; <i1> [#uses=1]
	br i1 %cmp53, label %for.body54, label %for.end72

for.body54:		; preds = %for.cond50
	%tmp55 = load i32* %binCount		; <i32> [#uses=1]
	%tmp56 = load i32* %j		; <i32> [#uses=1]
	%tmp57 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%mul58 = mul i32 %tmp56, %tmp57		; <i32> [#uses=1]
	%tmp59 = load i32* %i40		; <i32> [#uses=1]
	%tmp60 = load i32* %GROUP_SIZE		; <i32> [#uses=1]
	%mul61 = mul i32 %tmp59, %tmp60		; <i32> [#uses=1]
	%add62 = add i32 %mul58, %mul61		; <i32> [#uses=1]
	%tmp63 = load i32* %localId		; <i32> [#uses=1]
	%add64 = add i32 %add62, %tmp63		; <i32> [#uses=1]
	%tmp65 = load i8 addrspace(3)** %sharedArray.addr		; <i8 addrspace(3)*> [#uses=1]
	%arrayidx66 = getelementptr i8 addrspace(3)* %tmp65, i32 %add64		; <i8 addrspace(3)*> [#uses=1]
	%tmp67 = load i8 addrspace(3)* %arrayidx66		; <i8> [#uses=1]
	%conv = zext i8 %tmp67 to i32		; <i32> [#uses=1]
	%add68 = add i32 %tmp55, %conv		; <i32> [#uses=1]
	store i32 %add68, i32* %binCount
	br label %for.inc69

for.inc69:		; preds = %for.body54
	%tmp70 = load i32* %j		; <i32> [#uses=1]
	%inc71 = add i32 %tmp70, 1		; <i32> [#uses=1]
	store i32 %inc71, i32* %j
	br label %for.cond50

for.end72:		; preds = %for.cond50
	%tmp73 = load i32* %groupId		; <i32> [#uses=1]
	%tmp74 = load i32* %BIN_SIZE.addr		; <i32> [#uses=1]
	%mul75 = mul i32 %tmp73, %tmp74		; <i32> [#uses=1]
	%tmp76 = load i32* %i40		; <i32> [#uses=1]
	%tmp77 = load i32* %GROUP_SIZE		; <i32> [#uses=1]
	%mul78 = mul i32 %tmp76, %tmp77		; <i32> [#uses=1]
	%add79 = add i32 %mul75, %mul78		; <i32> [#uses=1]
	%tmp80 = load i32* %localId		; <i32> [#uses=1]
	%add81 = add i32 %add79, %tmp80		; <i32> [#uses=1]
	%tmp82 = load i32 addrspace(1)** %binResult.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx83 = getelementptr i32 addrspace(1)* %tmp82, i32 %add81		; <i32 addrspace(1)*> [#uses=1]
	%tmp84 = load i32* %binCount		; <i32> [#uses=1]
	store i32 %tmp84, i32 addrspace(1)* %arrayidx83
	br label %for.inc85

for.inc85:		; preds = %for.end72
	%tmp86 = load i32* %i40		; <i32> [#uses=1]
	%inc87 = add i32 %tmp86, 1		; <i32> [#uses=1]
	store i32 %inc87, i32* %i40
	br label %for.cond41

for.end88:		; preds = %for.cond41
	ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)
