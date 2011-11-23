; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_bitonic.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [6 x i8] c"20000\00"		; <[6 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, i32, i32, i32, i32, ...)* @bitonicSort to i8*), i8* getelementptr ([6 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @bitonicSort
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @bitonicSort(i32 addrspace(1)* %theArray, i32 %stage, i32 %passOfStage, i32 %width, i32 %direction, ...) nounwind {
entry:
	%theArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=7]
	%stage.addr = alloca i32		; <i32*> [#uses=3]
	%passOfStage.addr = alloca i32		; <i32*> [#uses=2]
	%width.addr = alloca i32		; <i32*> [#uses=1]
	%direction.addr = alloca i32		; <i32*> [#uses=2]
	%sortIncreasing = alloca i32, align 4		; <i32*> [#uses=4]
	%threadId = alloca i32, align 4		; <i32*> [#uses=4]
	%pairDistance = alloca i32, align 4		; <i32*> [#uses=5]
	%blockWidth = alloca i32, align 4		; <i32*> [#uses=2]
	%leftId = alloca i32, align 4		; <i32*> [#uses=5]
	%rightId = alloca i32, align 4		; <i32*> [#uses=4]
	%leftElement = alloca i32, align 4		; <i32*> [#uses=4]
	%rightElement = alloca i32, align 4		; <i32*> [#uses=4]
	%sameDirectionBlockWidth = alloca i32, align 4		; <i32*> [#uses=2]
	%greater = alloca i32, align 4		; <i32*> [#uses=4]
	%lesser = alloca i32, align 4		; <i32*> [#uses=4]
	store i32 addrspace(1)* %theArray, i32 addrspace(1)** %theArray.addr
	store i32 %stage, i32* %stage.addr
	store i32 %passOfStage, i32* %passOfStage.addr
	store i32 %width, i32* %width.addr
	store i32 %direction, i32* %direction.addr
	%tmp = load i32* %direction.addr		; <i32> [#uses=1]
	store i32 %tmp, i32* %sortIncreasing
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %threadId
	%tmp3 = load i32* %stage.addr		; <i32> [#uses=1]
	%tmp4 = load i32* %passOfStage.addr		; <i32> [#uses=1]
	%sub = sub i32 %tmp3, %tmp4		; <i32> [#uses=1]
	%and = and i32 %sub, 31		; <i32> [#uses=1]
	%shl = shl i32 1, %and		; <i32> [#uses=1]
	store i32 %shl, i32* %pairDistance
	%tmp6 = load i32* %pairDistance		; <i32> [#uses=1]
	%mul = mul i32 2, %tmp6		; <i32> [#uses=1]
	store i32 %mul, i32* %blockWidth
	%tmp8 = load i32* %threadId		; <i32> [#uses=1]
	%tmp9 = load i32* %pairDistance		; <i32> [#uses=2]
	%cmp = icmp ne i32 %tmp9, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp, i32 %tmp9, i32 1		; <i32> [#uses=1]
	%rem = urem i32 %tmp8, %nonzero		; <i32> [#uses=1]
	%tmp10 = load i32* %threadId		; <i32> [#uses=1]
	%tmp11 = load i32* %pairDistance		; <i32> [#uses=2]
	%cmp12 = icmp ne i32 %tmp11, 0		; <i1> [#uses=1]
	%nonzero13 = select i1 %cmp12, i32 %tmp11, i32 1		; <i32> [#uses=1]
	%div = udiv i32 %tmp10, %nonzero13		; <i32> [#uses=1]
	%tmp14 = load i32* %blockWidth		; <i32> [#uses=1]
	%mul15 = mul i32 %div, %tmp14		; <i32> [#uses=1]
	%add = add i32 %rem, %mul15		; <i32> [#uses=1]
	store i32 %add, i32* %leftId
	%tmp17 = load i32* %leftId		; <i32> [#uses=1]
	%tmp18 = load i32* %pairDistance		; <i32> [#uses=1]
	%add19 = add i32 %tmp17, %tmp18		; <i32> [#uses=1]
	store i32 %add19, i32* %rightId
	%tmp21 = load i32* %leftId		; <i32> [#uses=1]
	%tmp22 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i32 addrspace(1)* %tmp22, i32 %tmp21		; <i32 addrspace(1)*> [#uses=1]
	%tmp23 = load i32 addrspace(1)* %arrayidx		; <i32> [#uses=1]
	store i32 %tmp23, i32* %leftElement
	%tmp25 = load i32* %rightId		; <i32> [#uses=1]
	%tmp26 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx27 = getelementptr i32 addrspace(1)* %tmp26, i32 %tmp25		; <i32 addrspace(1)*> [#uses=1]
	%tmp28 = load i32 addrspace(1)* %arrayidx27		; <i32> [#uses=1]
	store i32 %tmp28, i32* %rightElement
	%tmp30 = load i32* %stage.addr		; <i32> [#uses=1]
	%and31 = and i32 %tmp30, 31		; <i32> [#uses=1]
	%shl32 = shl i32 1, %and31		; <i32> [#uses=1]
	store i32 %shl32, i32* %sameDirectionBlockWidth
	%tmp33 = load i32* %threadId		; <i32> [#uses=1]
	%tmp34 = load i32* %sameDirectionBlockWidth		; <i32> [#uses=2]
	%cmp35 = icmp ne i32 %tmp34, 0		; <i1> [#uses=1]
	%nonzero36 = select i1 %cmp35, i32 %tmp34, i32 1		; <i32> [#uses=1]
	%div37 = udiv i32 %tmp33, %nonzero36		; <i32> [#uses=1]
	%rem38 = urem i32 %div37, 2		; <i32> [#uses=1]
	%cmp39 = icmp eq i32 %rem38, 1		; <i1> [#uses=1]
	br i1 %cmp39, label %if.then, label %if.end

if.then:		; preds = %entry
	%tmp40 = load i32* %sortIncreasing		; <i32> [#uses=1]
	%sub41 = sub i32 1, %tmp40		; <i32> [#uses=1]
	store i32 %sub41, i32* %sortIncreasing
	br label %if.end

if.end:		; preds = %if.then, %entry
	%tmp44 = load i32* %leftElement		; <i32> [#uses=1]
	%tmp45 = load i32* %rightElement		; <i32> [#uses=1]
	%cmp46 = icmp ugt i32 %tmp44, %tmp45		; <i1> [#uses=1]
	br i1 %cmp46, label %if.then47, label %if.else

if.then47:		; preds = %if.end
	%tmp48 = load i32* %leftElement		; <i32> [#uses=1]
	store i32 %tmp48, i32* %greater
	%tmp49 = load i32* %rightElement		; <i32> [#uses=1]
	store i32 %tmp49, i32* %lesser
	br label %if.end52

if.else:		; preds = %if.end
	%tmp50 = load i32* %rightElement		; <i32> [#uses=1]
	store i32 %tmp50, i32* %greater
	%tmp51 = load i32* %leftElement		; <i32> [#uses=1]
	store i32 %tmp51, i32* %lesser
	br label %if.end52

if.end52:		; preds = %if.else, %if.then47
	%tmp53 = load i32* %sortIncreasing		; <i32> [#uses=1]
	%tobool = icmp ne i32 %tmp53, 0		; <i1> [#uses=1]
	br i1 %tobool, label %if.then54, label %if.else63

if.then54:		; preds = %if.end52
	%tmp55 = load i32* %leftId		; <i32> [#uses=1]
	%tmp56 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx57 = getelementptr i32 addrspace(1)* %tmp56, i32 %tmp55		; <i32 addrspace(1)*> [#uses=1]
	%tmp58 = load i32* %lesser		; <i32> [#uses=1]
	store i32 %tmp58, i32 addrspace(1)* %arrayidx57
	%tmp59 = load i32* %rightId		; <i32> [#uses=1]
	%tmp60 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx61 = getelementptr i32 addrspace(1)* %tmp60, i32 %tmp59		; <i32 addrspace(1)*> [#uses=1]
	%tmp62 = load i32* %greater		; <i32> [#uses=1]
	store i32 %tmp62, i32 addrspace(1)* %arrayidx61
	br label %if.end72

if.else63:		; preds = %if.end52
	%tmp64 = load i32* %leftId		; <i32> [#uses=1]
	%tmp65 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx66 = getelementptr i32 addrspace(1)* %tmp65, i32 %tmp64		; <i32 addrspace(1)*> [#uses=1]
	%tmp67 = load i32* %greater		; <i32> [#uses=1]
	store i32 %tmp67, i32 addrspace(1)* %arrayidx66
	%tmp68 = load i32* %rightId		; <i32> [#uses=1]
	%tmp69 = load i32 addrspace(1)** %theArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx70 = getelementptr i32 addrspace(1)* %tmp69, i32 %tmp68		; <i32 addrspace(1)*> [#uses=1]
	%tmp71 = load i32* %lesser		; <i32> [#uses=1]
	store i32 %tmp71, i32 addrspace(1)* %arrayidx70
	br label %if.end72

if.end72:		; preds = %if.else63, %if.then54
	ret void
}

declare i32 @get_global_id(i32)
