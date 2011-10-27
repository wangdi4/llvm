; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIFastWalsh.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [3 x i8] c"20\00"		; <[3 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (float addrspace(1)*, i32, ...)* @fastWalshTransform to i8*), i8* getelementptr ([3 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @fastWalshTransform

define void @fastWalshTransform(float addrspace(1)* %tArray, i32 %step, ...) nounwind {
entry:
	%tArray.addr = alloca float addrspace(1)*		; <float addrspace(1)**> [#uses=5]
	%step.addr = alloca i32		; <i32*> [#uses=5]
	%tid = alloca i32, align 4		; <i32*> [#uses=3]
	%group = alloca i32, align 4		; <i32*> [#uses=2]
	%pair = alloca i32, align 4		; <i32*> [#uses=4]
	%match = alloca i32, align 4		; <i32*> [#uses=3]
	%T1 = alloca float, align 4		; <float*> [#uses=3]
	%T2 = alloca float, align 4		; <float*> [#uses=3]
	store float addrspace(1)* %tArray, float addrspace(1)** %tArray.addr
	store i32 %step, i32* %step.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %tid
	%tmp = load i32* %tid		; <i32> [#uses=1]
	%tmp1 = load i32* %step.addr		; <i32> [#uses=2]
	%cmp = icmp ne i32 %tmp1, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp, i32 %tmp1, i32 1		; <i32> [#uses=1]
	%rem = urem i32 %tmp, %nonzero		; <i32> [#uses=1]
	store i32 %rem, i32* %group
	%tmp3 = load i32* %step.addr		; <i32> [#uses=1]
	%mul = mul i32 2, %tmp3		; <i32> [#uses=1]
	%tmp4 = load i32* %tid		; <i32> [#uses=1]
	%tmp5 = load i32* %step.addr		; <i32> [#uses=2]
	%cmp6 = icmp ne i32 %tmp5, 0		; <i1> [#uses=1]
	%nonzero7 = select i1 %cmp6, i32 %tmp5, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp4, %nonzero7		; <i32> [#uses=1]
	%mul8 = mul i32 %mul, %div		; <i32> [#uses=1]
	%tmp9 = load i32* %group		; <i32> [#uses=1]
	%add = add i32 %mul8, %tmp9		; <i32> [#uses=1]
	store i32 %add, i32* %pair
	%tmp11 = load i32* %pair		; <i32> [#uses=1]
	%tmp12 = load i32* %step.addr		; <i32> [#uses=1]
	%add13 = add i32 %tmp11, %tmp12		; <i32> [#uses=1]
	store i32 %add13, i32* %match
	%tmp15 = load i32* %pair		; <i32> [#uses=1]
	%tmp16 = load float addrspace(1)** %tArray.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr float addrspace(1)* %tmp16, i32 %tmp15		; <float addrspace(1)*> [#uses=1]
	%tmp17 = load float addrspace(1)* %arrayidx		; <float> [#uses=1]
	store float %tmp17, float* %T1
	%tmp19 = load i32* %match		; <i32> [#uses=1]
	%tmp20 = load float addrspace(1)** %tArray.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx21 = getelementptr float addrspace(1)* %tmp20, i32 %tmp19		; <float addrspace(1)*> [#uses=1]
	%tmp22 = load float addrspace(1)* %arrayidx21		; <float> [#uses=1]
	store float %tmp22, float* %T2
	%tmp23 = load i32* %pair		; <i32> [#uses=1]
	%tmp24 = load float addrspace(1)** %tArray.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx25 = getelementptr float addrspace(1)* %tmp24, i32 %tmp23		; <float addrspace(1)*> [#uses=1]
	%tmp26 = load float* %T1		; <float> [#uses=1]
	%tmp27 = load float* %T2		; <float> [#uses=1]
	%add28 =fadd float %tmp26, %tmp27		; <float> [#uses=1]
	store float %add28, float addrspace(1)* %arrayidx25
	%tmp29 = load i32* %match		; <i32> [#uses=1]
	%tmp30 = load float addrspace(1)** %tArray.addr		; <float addrspace(1)*> [#uses=1]
	%arrayidx31 = getelementptr float addrspace(1)* %tmp30, i32 %tmp29		; <float addrspace(1)*> [#uses=1]
	%tmp32 = load float* %T1		; <float> [#uses=1]
	%tmp33 = load float* %T2		; <float> [#uses=1]
	%sub = fsub float %tmp32, %tmp33		; <float> [#uses=1]
	store float %sub, float addrspace(1)* %arrayidx31
	ret void
}

declare i32 @get_global_id(i32)
