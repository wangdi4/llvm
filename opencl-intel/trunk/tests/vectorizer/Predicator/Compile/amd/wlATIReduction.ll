; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIReduction.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [4 x i8] c"229\00"		; <[4 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, <4 x i32> addrspace(3)*, ...)* @reduce to i8*), i8* getelementptr ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @reduce
define void @reduce(<4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)* %output, <4 x i32> addrspace(3)* %sdata, ...) nounwind {
entry:
	%input.addr = alloca <4 x i32> addrspace(1)*		; <<4 x i32> addrspace(1)**> [#uses=2]
	%output.addr = alloca <4 x i32> addrspace(1)*		; <<4 x i32> addrspace(1)**> [#uses=2]
	%sdata.addr = alloca <4 x i32> addrspace(3)*		; <<4 x i32> addrspace(3)**> [#uses=5]
	%tid = alloca i32, align 4		; <i32*> [#uses=6]
	%bid = alloca i32, align 4		; <i32*> [#uses=2]
	%gid = alloca i32, align 4		; <i32*> [#uses=2]
	%localSize = alloca i32, align 4		; <i32*> [#uses=2]
	%s = alloca i32, align 4		; <i32*> [#uses=6]
	store <4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)** %input.addr
	store <4 x i32> addrspace(1)* %output, <4 x i32> addrspace(1)** %output.addr
	store <4 x i32> addrspace(3)* %sdata, <4 x i32> addrspace(3)** %sdata.addr
	%call = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %tid
	%call1 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %bid
	%call2 = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %gid
	%call3 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	store i32 %call3, i32* %localSize
	%tmp = load i32* %tid		; <i32> [#uses=1]
	%tmp4 = load <4 x i32> addrspace(3)** %sdata.addr		; <<4 x i32> addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr <4 x i32> addrspace(3)* %tmp4, i32 %tmp		; <<4 x i32> addrspace(3)*> [#uses=1]
	%tmp5 = load i32* %gid		; <i32> [#uses=1]
	%tmp6 = load <4 x i32> addrspace(1)** %input.addr		; <<4 x i32> addrspace(1)*> [#uses=1]
	%arrayidx7 = getelementptr <4 x i32> addrspace(1)* %tmp6, i32 %tmp5		; <<4 x i32> addrspace(1)*> [#uses=1]
	%tmp8 = load <4 x i32> addrspace(1)* %arrayidx7		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp8, <4 x i32> addrspace(3)* %arrayidx
	call void @barrier(i32 1)
	%tmp10 = load i32* %localSize		; <i32> [#uses=1]
	%div = udiv i32 %tmp10, 2		; <i32> [#uses=1]
	store i32 %div, i32* %s
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp11 = load i32* %s		; <i32> [#uses=1]
	%cmp = icmp ugt i32 %tmp11, 0		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp12 = load i32* %tid		; <i32> [#uses=1]
	%tmp13 = load i32* %s		; <i32> [#uses=1]
	%cmp14 = icmp ult i32 %tmp12, %tmp13		; <i1> [#uses=1]
	br i1 %cmp14, label %if.then, label %if.end

if.then:		; preds = %for.body
	%tmp15 = load i32* %tid		; <i32> [#uses=1]
	%tmp16 = load <4 x i32> addrspace(3)** %sdata.addr		; <<4 x i32> addrspace(3)*> [#uses=1]
	%arrayidx17 = getelementptr <4 x i32> addrspace(3)* %tmp16, i32 %tmp15		; <<4 x i32> addrspace(3)*> [#uses=2]
	%tmp18 = load <4 x i32> addrspace(3)* %arrayidx17		; <<4 x i32>> [#uses=1]
	%tmp19 = load i32* %tid		; <i32> [#uses=1]
	%tmp20 = load i32* %s		; <i32> [#uses=1]
	%add = add i32 %tmp19, %tmp20		; <i32> [#uses=1]
	%tmp21 = load <4 x i32> addrspace(3)** %sdata.addr		; <<4 x i32> addrspace(3)*> [#uses=1]
	%arrayidx22 = getelementptr <4 x i32> addrspace(3)* %tmp21, i32 %add		; <<4 x i32> addrspace(3)*> [#uses=1]
	%tmp23 = load <4 x i32> addrspace(3)* %arrayidx22		; <<4 x i32>> [#uses=1]
	%add24 = add <4 x i32> %tmp18, %tmp23		; <<4 x i32>> [#uses=1]
	store <4 x i32> %add24, <4 x i32> addrspace(3)* %arrayidx17
	br label %if.end

if.end:		; preds = %if.then, %for.body
	call void @barrier(i32 1)
	br label %for.inc

for.inc:		; preds = %if.end
	%tmp25 = load i32* %s		; <i32> [#uses=1]
	%shr = lshr i32 %tmp25, 1		; <i32> [#uses=1]
	store i32 %shr, i32* %s
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp26 = load i32* %tid		; <i32> [#uses=1]
	%cmp27 = icmp eq i32 %tmp26, 0		; <i1> [#uses=1]
	br i1 %cmp27, label %if.then28, label %if.end35

if.then28:		; preds = %for.end
	%tmp29 = load i32* %bid		; <i32> [#uses=1]
	%tmp30 = load <4 x i32> addrspace(1)** %output.addr		; <<4 x i32> addrspace(1)*> [#uses=1]
	%arrayidx31 = getelementptr <4 x i32> addrspace(1)* %tmp30, i32 %tmp29		; <<4 x i32> addrspace(1)*> [#uses=1]
	%tmp32 = load <4 x i32> addrspace(3)** %sdata.addr		; <<4 x i32> addrspace(3)*> [#uses=1]
	%arrayidx33 = getelementptr <4 x i32> addrspace(3)* %tmp32, i32 0		; <<4 x i32> addrspace(3)*> [#uses=1]
	%tmp34 = load <4 x i32> addrspace(3)* %arrayidx33		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp34, <4 x i32> addrspace(1)* %arrayidx31
	br label %if.end35

if.end35:		; preds = %if.then28, %for.end
	ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)
