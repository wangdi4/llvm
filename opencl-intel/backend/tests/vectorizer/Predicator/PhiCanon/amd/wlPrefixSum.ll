; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlPrefixSum.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"1220\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [4 x i8] c"210\00"		; <[4 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [2 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, ...)* @prefixSumStep1 to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, ...)* @prefixSumStep2 to i8*), i8* getelementptr ([4 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv3 to i8*), i32 0 }], section "llvm.metadata"		; <[2 x %0]*> [#uses=0]

;CHECK: @prefixSumStep1 
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond23.preheader, %for.end57.loopexit
; CHECK: ret

define void @prefixSumStep1(i32 addrspace(1)* %puiInputArray, i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)* %puiTmpArray, i32 %szElementsPerItem, ...) nounwind {
entry:
	%puiInputArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%puiOutputArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%puiTmpArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%szElementsPerItem.addr = alloca i32		; <i32*> [#uses=6]
	%gid = alloca i32, align 4		; <i32*> [#uses=4]
	%offset = alloca i32, align 4		; <i32*> [#uses=6]
	%shiftedInputArray = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=2]
	%shiftedOutputArray = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=5]
	%i = alloca i32, align 4		; <i32*> [#uses=6]
	%szPairs = alloca i32, align 4		; <i32*> [#uses=2]
	%h = alloca i32, align 4		; <i32*> [#uses=5]
	%index = alloca i32, align 4		; <i32*> [#uses=5]
	%p = alloca i32, align 4		; <i32*> [#uses=4]
	store i32 addrspace(1)* %puiInputArray, i32 addrspace(1)** %puiInputArray.addr
	store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
	store i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)** %puiTmpArray.addr
	store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %gid
	store i32 1, i32* %offset
	%tmp = load i32 addrspace(1)** %puiInputArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%tmp1 = load i32* %gid		; <i32> [#uses=1]
	%tmp2 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp1, %tmp2		; <i32> [#uses=1]
	%add.ptr = getelementptr i32 addrspace(1)* %tmp, i32 %mul		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedInputArray
	%tmp4 = load i32 addrspace(1)** %puiOutputArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%tmp5 = load i32* %gid		; <i32> [#uses=1]
	%tmp6 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%mul7 = mul i32 %tmp5, %tmp6		; <i32> [#uses=1]
	%add.ptr8 = getelementptr i32 addrspace(1)* %tmp4, i32 %mul7		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %add.ptr8, i32 addrspace(1)** %shiftedOutputArray
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp10 = load i32* %i		; <i32> [#uses=1]
	%tmp11 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp10, %tmp11		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp12 = load i32* %i		; <i32> [#uses=1]
	%tmp13 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i32 addrspace(1)* %tmp13, i32 %tmp12		; <i32 addrspace(1)*> [#uses=1]
	%tmp14 = load i32* %i		; <i32> [#uses=1]
	%tmp15 = load i32 addrspace(1)** %shiftedInputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx16 = getelementptr i32 addrspace(1)* %tmp15, i32 %tmp14		; <i32 addrspace(1)*> [#uses=1]
	%tmp17 = load i32 addrspace(1)* %arrayidx16		; <i32> [#uses=1]
	store i32 %tmp17, i32 addrspace(1)* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp18 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp18, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp20 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%shr = lshr i32 %tmp20, 1		; <i32> [#uses=1]
	store i32 %shr, i32* %szPairs
	%tmp22 = load i32* %szPairs		; <i32> [#uses=1]
	store i32 %tmp22, i32* %h
	br label %for.cond23

for.cond23:		; preds = %for.inc54, %for.end
	%tmp24 = load i32* %h		; <i32> [#uses=1]
	%cmp25 = icmp ugt i32 %tmp24, 0		; <i1> [#uses=1]
	br i1 %cmp25, label %for.body26, label %for.end57

for.body26:		; preds = %for.cond23
	%tmp28 = load i32* %offset		; <i32> [#uses=1]
	%sub = sub i32 %tmp28, 1		; <i32> [#uses=1]
	store i32 %sub, i32* %index
	store i32 0, i32* %p
	br label %for.cond30

for.cond30:		; preds = %for.inc48, %for.body26
	%tmp31 = load i32* %p		; <i32> [#uses=1]
	%tmp32 = load i32* %h		; <i32> [#uses=1]
	%cmp33 = icmp ult i32 %tmp31, %tmp32		; <i1> [#uses=1]
	br i1 %cmp33, label %for.body34, label %for.end51

for.body34:		; preds = %for.cond30
	%tmp35 = load i32* %index		; <i32> [#uses=1]
	%tmp36 = load i32* %offset		; <i32> [#uses=1]
	%add = add i32 %tmp35, %tmp36		; <i32> [#uses=1]
	%tmp37 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx38 = getelementptr i32 addrspace(1)* %tmp37, i32 %add		; <i32 addrspace(1)*> [#uses=2]
	%tmp39 = load i32 addrspace(1)* %arrayidx38		; <i32> [#uses=1]
	%tmp40 = load i32* %index		; <i32> [#uses=1]
	%tmp41 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx42 = getelementptr i32 addrspace(1)* %tmp41, i32 %tmp40		; <i32 addrspace(1)*> [#uses=1]
	%tmp43 = load i32 addrspace(1)* %arrayidx42		; <i32> [#uses=1]
	%add44 = add i32 %tmp39, %tmp43		; <i32> [#uses=1]
	store i32 %add44, i32 addrspace(1)* %arrayidx38
	%tmp45 = load i32* %index		; <i32> [#uses=1]
	%tmp46 = load i32* %offset		; <i32> [#uses=1]
	%shl = shl i32 %tmp46, 1		; <i32> [#uses=1]
	%add47 = add i32 %tmp45, %shl		; <i32> [#uses=1]
	store i32 %add47, i32* %index
	br label %for.inc48

for.inc48:		; preds = %for.body34
	%tmp49 = load i32* %p		; <i32> [#uses=1]
	%inc50 = add i32 %tmp49, 1		; <i32> [#uses=1]
	store i32 %inc50, i32* %p
	br label %for.cond30

for.end51:		; preds = %for.cond30
	%tmp52 = load i32* %offset		; <i32> [#uses=1]
	%shl53 = shl i32 %tmp52, 1		; <i32> [#uses=1]
	store i32 %shl53, i32* %offset
	br label %for.inc54

for.inc54:		; preds = %for.end51
	%tmp55 = load i32* %h		; <i32> [#uses=1]
	%shr56 = lshr i32 %tmp55, 1		; <i32> [#uses=1]
	store i32 %shr56, i32* %h
	br label %for.cond23

for.end57:		; preds = %for.cond23
	%tmp58 = load i32* %gid		; <i32> [#uses=1]
	%tmp59 = load i32 addrspace(1)** %puiTmpArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx60 = getelementptr i32 addrspace(1)* %tmp59, i32 %tmp58		; <i32 addrspace(1)*> [#uses=1]
	%tmp61 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%sub62 = sub i32 %tmp61, 1		; <i32> [#uses=1]
	%tmp63 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx64 = getelementptr i32 addrspace(1)* %tmp63, i32 %sub62		; <i32 addrspace(1)*> [#uses=1]
	%tmp65 = load i32 addrspace(1)* %arrayidx64		; <i32> [#uses=1]
	store i32 %tmp65, i32 addrspace(1)* %arrayidx60
	ret void
}

declare i32 @get_global_id(i32)

define void @prefixSumStep2(i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)* %puiValueToAddArray, i32 %szElementsPerItem, ...) nounwind {
entry:
	%puiOutputArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%puiValueToAddArray.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%szElementsPerItem.addr = alloca i32		; <i32*> [#uses=6]
	%gid = alloca i32, align 4		; <i32*> [#uses=3]
	%offset = alloca i32, align 4		; <i32*> [#uses=7]
	%shiftedOutputArray = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=7]
	%shiftedValueToAddArray = alloca i32 addrspace(1)*, align 4		; <i32 addrspace(1)**> [#uses=2]
	%szPairs = alloca i32, align 4		; <i32*> [#uses=2]
	%nextValue = alloca i32, align 4		; <i32*> [#uses=3]
	%h = alloca i32, align 4		; <i32*> [#uses=5]
	%index = alloca i32, align 4		; <i32*> [#uses=7]
	%p = alloca i32, align 4		; <i32*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=5]
	store i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)** %puiOutputArray.addr
	store i32 addrspace(1)* %puiValueToAddArray, i32 addrspace(1)** %puiValueToAddArray.addr
	store i32 %szElementsPerItem, i32* %szElementsPerItem.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %gid
	%tmp = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	store i32 %tmp, i32* %offset
	%tmp2 = load i32 addrspace(1)** %puiOutputArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%tmp3 = load i32* %gid		; <i32> [#uses=1]
	%tmp4 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp3, %tmp4		; <i32> [#uses=1]
	%add.ptr = getelementptr i32 addrspace(1)* %tmp2, i32 %mul		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %add.ptr, i32 addrspace(1)** %shiftedOutputArray
	%tmp6 = load i32 addrspace(1)** %puiValueToAddArray.addr		; <i32 addrspace(1)*> [#uses=1]
	%tmp7 = load i32* %gid		; <i32> [#uses=1]
	%add.ptr8 = getelementptr i32 addrspace(1)* %tmp6, i32 %tmp7		; <i32 addrspace(1)*> [#uses=1]
	store i32 addrspace(1)* %add.ptr8, i32 addrspace(1)** %shiftedValueToAddArray
	%tmp10 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%shr = lshr i32 %tmp10, 1		; <i32> [#uses=1]
	store i32 %shr, i32* %szPairs
	store i32 0, i32* %nextValue
	%tmp12 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%sub = sub i32 %tmp12, 1		; <i32> [#uses=1]
	%tmp13 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx = getelementptr i32 addrspace(1)* %tmp13, i32 %sub		; <i32 addrspace(1)*> [#uses=1]
	store i32 0, i32 addrspace(1)* %arrayidx
	store i32 1, i32* %h
	br label %for.cond

for.cond:		; preds = %for.inc52, %entry
	%tmp15 = load i32* %h		; <i32> [#uses=1]
	%tmp16 = load i32* %szPairs		; <i32> [#uses=1]
	%cmp = icmp ule i32 %tmp15, %tmp16		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end55

for.body:		; preds = %for.cond
	%tmp17 = load i32* %offset		; <i32> [#uses=1]
	%shr18 = lshr i32 %tmp17, 1		; <i32> [#uses=1]
	store i32 %shr18, i32* %offset
	%tmp20 = load i32* %offset		; <i32> [#uses=1]
	%sub21 = sub i32 %tmp20, 1		; <i32> [#uses=1]
	store i32 %sub21, i32* %index
	store i32 0, i32* %p
	br label %for.cond23

for.cond23:		; preds = %for.inc, %for.body
	%tmp24 = load i32* %p		; <i32> [#uses=1]
	%tmp25 = load i32* %h		; <i32> [#uses=1]
	%cmp26 = icmp ult i32 %tmp24, %tmp25		; <i1> [#uses=1]
	br i1 %cmp26, label %for.body27, label %for.end

for.body27:		; preds = %for.cond23
	%tmp28 = load i32* %index		; <i32> [#uses=1]
	%tmp29 = load i32* %offset		; <i32> [#uses=1]
	%add = add i32 %tmp28, %tmp29		; <i32> [#uses=1]
	%tmp30 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx31 = getelementptr i32 addrspace(1)* %tmp30, i32 %add		; <i32 addrspace(1)*> [#uses=1]
	%tmp32 = load i32 addrspace(1)* %arrayidx31		; <i32> [#uses=1]
	store i32 %tmp32, i32* %nextValue
	%tmp33 = load i32* %index		; <i32> [#uses=1]
	%tmp34 = load i32* %offset		; <i32> [#uses=1]
	%add35 = add i32 %tmp33, %tmp34		; <i32> [#uses=1]
	%tmp36 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx37 = getelementptr i32 addrspace(1)* %tmp36, i32 %add35		; <i32 addrspace(1)*> [#uses=2]
	%tmp38 = load i32 addrspace(1)* %arrayidx37		; <i32> [#uses=1]
	%tmp39 = load i32* %index		; <i32> [#uses=1]
	%tmp40 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx41 = getelementptr i32 addrspace(1)* %tmp40, i32 %tmp39		; <i32 addrspace(1)*> [#uses=1]
	%tmp42 = load i32 addrspace(1)* %arrayidx41		; <i32> [#uses=1]
	%add43 = add i32 %tmp38, %tmp42		; <i32> [#uses=1]
	store i32 %add43, i32 addrspace(1)* %arrayidx37
	%tmp44 = load i32* %index		; <i32> [#uses=1]
	%tmp45 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx46 = getelementptr i32 addrspace(1)* %tmp45, i32 %tmp44		; <i32 addrspace(1)*> [#uses=1]
	%tmp47 = load i32* %nextValue		; <i32> [#uses=1]
	store i32 %tmp47, i32 addrspace(1)* %arrayidx46
	%tmp48 = load i32* %index		; <i32> [#uses=1]
	%tmp49 = load i32* %offset		; <i32> [#uses=1]
	%shl = shl i32 %tmp49, 1		; <i32> [#uses=1]
	%add50 = add i32 %tmp48, %shl		; <i32> [#uses=1]
	store i32 %add50, i32* %index
	br label %for.inc

for.inc:		; preds = %for.body27
	%tmp51 = load i32* %p		; <i32> [#uses=1]
	%inc = add i32 %tmp51, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %p
	br label %for.cond23

for.end:		; preds = %for.cond23
	br label %for.inc52

for.inc52:		; preds = %for.end
	%tmp53 = load i32* %h		; <i32> [#uses=1]
	%shl54 = shl i32 %tmp53, 1		; <i32> [#uses=1]
	store i32 %shl54, i32* %h
	br label %for.cond

for.end55:		; preds = %for.cond
	store i32 0, i32* %i
	br label %for.cond57

for.cond57:		; preds = %for.inc70, %for.end55
	%tmp58 = load i32* %i		; <i32> [#uses=1]
	%tmp59 = load i32* %szElementsPerItem.addr		; <i32> [#uses=1]
	%cmp60 = icmp ult i32 %tmp58, %tmp59		; <i1> [#uses=1]
	br i1 %cmp60, label %for.body61, label %for.end73

for.body61:		; preds = %for.cond57
	%tmp62 = load i32* %i		; <i32> [#uses=1]
	%tmp63 = load i32 addrspace(1)** %shiftedOutputArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx64 = getelementptr i32 addrspace(1)* %tmp63, i32 %tmp62		; <i32 addrspace(1)*> [#uses=2]
	%tmp65 = load i32 addrspace(1)* %arrayidx64		; <i32> [#uses=1]
	%tmp66 = load i32 addrspace(1)** %shiftedValueToAddArray		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx67 = getelementptr i32 addrspace(1)* %tmp66, i32 0		; <i32 addrspace(1)*> [#uses=1]
	%tmp68 = load i32 addrspace(1)* %arrayidx67		; <i32> [#uses=1]
	%add69 = add i32 %tmp65, %tmp68		; <i32> [#uses=1]
	store i32 %add69, i32 addrspace(1)* %arrayidx64
	br label %for.inc70

for.inc70:		; preds = %for.body61
	%tmp71 = load i32* %i		; <i32> [#uses=1]
	%inc72 = add i32 %tmp71, 1		; <i32> [#uses=1]
	store i32 %inc72, i32* %i
	br label %for.cond57

for.end73:		; preds = %for.cond57
	ret void
}
