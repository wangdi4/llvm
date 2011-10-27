; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIRadixSort.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [5 x i8] c"1209\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [6 x i8] c"11092\00"		; <[6 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [2 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*, ...)* @histogram to i8*), i8* getelementptr ([5 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*, i32 addrspace(1)*, ...)* @permute to i8*), i8* getelementptr ([6 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv3 to i8*), i32 0 }], section "llvm.metadata"		; <[2 x %0]*> [#uses=0]

; CHECK: @histogram

define void @histogram(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %buckets, i32 %shiftCount, i16 addrspace(3)* %sharedArray, ...) nounwind {
entry:
	%unsortedData.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%buckets.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%shiftCount.addr = alloca i32		; <i32*> [#uses=2]
	%sharedArray.addr = alloca i16 addrspace(3)*		; <i16 addrspace(3)**> [#uses=4]
	%localId = alloca i32, align 4		; <i32*> [#uses=5]
	%globalId = alloca i32, align 4		; <i32*> [#uses=2]
	%groupId = alloca i32, align 4		; <i32*> [#uses=2]
	%i = alloca i32, align 4		; <i32*> [#uses=5]
	%i8 = alloca i32, align 4		; <i32*> [#uses=5]
	%value = alloca i32, align 4		; <i32*> [#uses=4]
	%i37 = alloca i32, align 4		; <i32*> [#uses=6]
	%bucketPos = alloca i32, align 4		; <i32*> [#uses=2]
	store i32 addrspace(1)* %unsortedData, i32 addrspace(1)** %unsortedData.addr
	store i32 addrspace(1)* %buckets, i32 addrspace(1)** %buckets.addr
	store i32 %shiftCount, i32* %shiftCount.addr
	store i16 addrspace(3)* %sharedArray, i16 addrspace(3)** %sharedArray.addr
	%call = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %localId
	%call1 = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %globalId
	%call2 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %groupId
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp = load i32* %i		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp, 256		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp3 = load i32* %localId		; <i32> [#uses=1]
	%mul = mul i32 %tmp3, 256		; <i32> [#uses=1]
	%tmp4 = load i32* %i		; <i32> [#uses=1]
	%add = add i32 %mul, %tmp4		; <i32> [#uses=1]
	%tmp5 = load i16 addrspace(3)** %sharedArray.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr i16 addrspace(3)* %tmp5, i32 %add		; <i16 addrspace(3)*> [#uses=1]
	store i16 0, i16 addrspace(3)* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp6 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp6, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	call void @barrier(i32 1)
	store i32 0, i32* %i8
	br label %for.cond9

for.cond9:		; preds = %for.inc32, %for.end
	%tmp10 = load i32* %i8		; <i32> [#uses=1]
	%cmp11 = icmp slt i32 %tmp10, 256		; <i1> [#uses=1]
	br i1 %cmp11, label %for.body12, label %for.end35

for.body12:		; preds = %for.cond9
	%tmp14 = load i32* %globalId		; <i32> [#uses=1]
	%mul15 = mul i32 %tmp14, 256		; <i32> [#uses=1]
	%tmp16 = load i32* %i8		; <i32> [#uses=1]
	%add17 = add i32 %mul15, %tmp16		; <i32> [#uses=1]
	%tmp18 = load i32 addrspace(1)** %unsortedData.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx19 = getelementptr i32 addrspace(1)* %tmp18, i32 %add17		; <i32 addrspace(1)*> [#uses=1]
	%tmp20 = load i32 addrspace(1)* %arrayidx19		; <i32> [#uses=1]
	store i32 %tmp20, i32* %value
	%tmp21 = load i32* %value		; <i32> [#uses=1]
	%tmp22 = load i32* %shiftCount.addr		; <i32> [#uses=1]
	%and = and i32 %tmp22, 31		; <i32> [#uses=1]
	%shr = lshr i32 %tmp21, %and		; <i32> [#uses=1]
	%and23 = and i32 %shr, 255		; <i32> [#uses=1]
	store i32 %and23, i32* %value
	%tmp24 = load i32* %localId		; <i32> [#uses=1]
	%mul25 = mul i32 %tmp24, 256		; <i32> [#uses=1]
	%tmp26 = load i32* %value		; <i32> [#uses=1]
	%add27 = add i32 %mul25, %tmp26		; <i32> [#uses=1]
	%tmp28 = load i16 addrspace(3)** %sharedArray.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx29 = getelementptr i16 addrspace(3)* %tmp28, i32 %add27		; <i16 addrspace(3)*> [#uses=2]
	%tmp30 = load i16 addrspace(3)* %arrayidx29		; <i16> [#uses=1]
	%inc31 = add i16 %tmp30, 1		; <i16> [#uses=1]
	store i16 %inc31, i16 addrspace(3)* %arrayidx29
	br label %for.inc32

for.inc32:		; preds = %for.body12
	%tmp33 = load i32* %i8		; <i32> [#uses=1]
	%inc34 = add i32 %tmp33, 1		; <i32> [#uses=1]
	store i32 %inc34, i32* %i8
	br label %for.cond9

for.end35:		; preds = %for.cond9
	call void @barrier(i32 1)
	store i32 0, i32* %i37
	br label %for.cond38

for.cond38:		; preds = %for.inc61, %for.end35
	%tmp39 = load i32* %i37		; <i32> [#uses=1]
	%cmp40 = icmp slt i32 %tmp39, 256		; <i1> [#uses=1]
	br i1 %cmp40, label %for.body41, label %for.end64

for.body41:		; preds = %for.cond38
	%tmp43 = load i32* %groupId		; <i32> [#uses=1]
	%mul44 = mul i32 %tmp43, 256		; <i32> [#uses=1]
	%mul45 = mul i32 %mul44, 16		; <i32> [#uses=1]
	%tmp46 = load i32* %localId		; <i32> [#uses=1]
	%mul47 = mul i32 %tmp46, 256		; <i32> [#uses=1]
	%add48 = add i32 %mul45, %mul47		; <i32> [#uses=1]
	%tmp49 = load i32* %i37		; <i32> [#uses=1]
	%add50 = add i32 %add48, %tmp49		; <i32> [#uses=1]
	store i32 %add50, i32* %bucketPos
	%tmp51 = load i32* %bucketPos		; <i32> [#uses=1]
	%tmp52 = load i32 addrspace(1)** %buckets.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx53 = getelementptr i32 addrspace(1)* %tmp52, i32 %tmp51		; <i32 addrspace(1)*> [#uses=1]
	%tmp54 = load i32* %localId		; <i32> [#uses=1]
	%mul55 = mul i32 %tmp54, 256		; <i32> [#uses=1]
	%tmp56 = load i32* %i37		; <i32> [#uses=1]
	%add57 = add i32 %mul55, %tmp56		; <i32> [#uses=1]
	%tmp58 = load i16 addrspace(3)** %sharedArray.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx59 = getelementptr i16 addrspace(3)* %tmp58, i32 %add57		; <i16 addrspace(3)*> [#uses=1]
	%tmp60 = load i16 addrspace(3)* %arrayidx59		; <i16> [#uses=1]
	%conv = zext i16 %tmp60 to i32		; <i32> [#uses=1]
	store i32 %conv, i32 addrspace(1)* %arrayidx53
	br label %for.inc61

for.inc61:		; preds = %for.body41
	%tmp62 = load i32* %i37		; <i32> [#uses=1]
	%inc63 = add i32 %tmp62, 1		; <i32> [#uses=1]
	store i32 %inc63, i32* %i37
	br label %for.cond38

for.end64:		; preds = %for.cond38
	ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)

define void @permute(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %prescanedBuckets, i32 %shiftCount, i16 addrspace(3)* %sharedBuckets, i32 addrspace(1)* %sortedData, ...) nounwind {
entry:
	%unsortedData.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=3]
	%prescanedBuckets.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%shiftCount.addr = alloca i32		; <i32*> [#uses=2]
	%sharedBuckets.addr = alloca i16 addrspace(3)*		; <i16 addrspace(3)**> [#uses=4]
	%sortedData.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%groupId = alloca i32, align 4		; <i32*> [#uses=2]
	%localId = alloca i32, align 4		; <i32*> [#uses=5]
	%globalId = alloca i32, align 4		; <i32*> [#uses=3]
	%i = alloca i32, align 4		; <i32*> [#uses=6]
	%bucketPos = alloca i32, align 4		; <i32*> [#uses=2]
	%i21 = alloca i32, align 4		; <i32*> [#uses=6]
	%value = alloca i32, align 4		; <i32*> [#uses=5]
	%index = alloca i32, align 4		; <i32*> [#uses=3]
	store i32 addrspace(1)* %unsortedData, i32 addrspace(1)** %unsortedData.addr
	store i32 addrspace(1)* %prescanedBuckets, i32 addrspace(1)** %prescanedBuckets.addr
	store i32 %shiftCount, i32* %shiftCount.addr
	store i16 addrspace(3)* %sharedBuckets, i16 addrspace(3)** %sharedBuckets.addr
	store i32 addrspace(1)* %sortedData, i32 addrspace(1)** %sortedData.addr
	%call = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %groupId
	%call1 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call1, i32* %localId
	%call2 = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %globalId
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp = load i32* %i		; <i32> [#uses=1]
	%cmp = icmp slt i32 %tmp, 256		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp4 = load i32* %groupId		; <i32> [#uses=1]
	%mul = mul i32 %tmp4, 256		; <i32> [#uses=1]
	%mul5 = mul i32 %mul, 16		; <i32> [#uses=1]
	%tmp6 = load i32* %localId		; <i32> [#uses=1]
	%mul7 = mul i32 %tmp6, 256		; <i32> [#uses=1]
	%add = add i32 %mul5, %mul7		; <i32> [#uses=1]
	%tmp8 = load i32* %i		; <i32> [#uses=1]
	%add9 = add i32 %add, %tmp8		; <i32> [#uses=1]
	store i32 %add9, i32* %bucketPos
	%tmp10 = load i32* %localId		; <i32> [#uses=1]
	%mul11 = mul i32 %tmp10, 256		; <i32> [#uses=1]
	%tmp12 = load i32* %i		; <i32> [#uses=1]
	%add13 = add i32 %mul11, %tmp12		; <i32> [#uses=1]
	%tmp14 = load i16 addrspace(3)** %sharedBuckets.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr i16 addrspace(3)* %tmp14, i32 %add13		; <i16 addrspace(3)*> [#uses=1]
	%tmp15 = load i32* %bucketPos		; <i32> [#uses=1]
	%tmp16 = load i32 addrspace(1)** %prescanedBuckets.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx17 = getelementptr i32 addrspace(1)* %tmp16, i32 %tmp15		; <i32 addrspace(1)*> [#uses=1]
	%tmp18 = load i32 addrspace(1)* %arrayidx17		; <i32> [#uses=1]
	%conv = trunc i32 %tmp18 to i16		; <i16> [#uses=1]
	store i16 %conv, i16 addrspace(3)* %arrayidx
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp19 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp19, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	call void @barrier(i32 1)
	store i32 0, i32* %i21
	br label %for.cond22

for.cond22:		; preds = %for.inc66, %for.end
	%tmp23 = load i32* %i21		; <i32> [#uses=1]
	%cmp24 = icmp slt i32 %tmp23, 256		; <i1> [#uses=1]
	br i1 %cmp24, label %for.body26, label %for.end69

for.body26:		; preds = %for.cond22
	%tmp28 = load i32* %globalId		; <i32> [#uses=1]
	%mul29 = mul i32 %tmp28, 256		; <i32> [#uses=1]
	%tmp30 = load i32* %i21		; <i32> [#uses=1]
	%add31 = add i32 %mul29, %tmp30		; <i32> [#uses=1]
	%tmp32 = load i32 addrspace(1)** %unsortedData.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx33 = getelementptr i32 addrspace(1)* %tmp32, i32 %add31		; <i32 addrspace(1)*> [#uses=1]
	%tmp34 = load i32 addrspace(1)* %arrayidx33		; <i32> [#uses=1]
	store i32 %tmp34, i32* %value
	%tmp35 = load i32* %value		; <i32> [#uses=1]
	%tmp36 = load i32* %shiftCount.addr		; <i32> [#uses=1]
	%and = and i32 %tmp36, 31		; <i32> [#uses=1]
	%shr = lshr i32 %tmp35, %and		; <i32> [#uses=1]
	%and37 = and i32 %shr, 255		; <i32> [#uses=1]
	store i32 %and37, i32* %value
	%tmp39 = load i32* %localId		; <i32> [#uses=1]
	%mul40 = mul i32 %tmp39, 256		; <i32> [#uses=1]
	%tmp41 = load i32* %value		; <i32> [#uses=1]
	%add42 = add i32 %mul40, %tmp41		; <i32> [#uses=1]
	%tmp43 = load i16 addrspace(3)** %sharedBuckets.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx44 = getelementptr i16 addrspace(3)* %tmp43, i32 %add42		; <i16 addrspace(3)*> [#uses=1]
	%tmp45 = load i16 addrspace(3)* %arrayidx44		; <i16> [#uses=1]
	%conv46 = zext i16 %tmp45 to i32		; <i32> [#uses=1]
	store i32 %conv46, i32* %index
	%tmp47 = load i32* %index		; <i32> [#uses=1]
	%tmp48 = load i32 addrspace(1)** %sortedData.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx49 = getelementptr i32 addrspace(1)* %tmp48, i32 %tmp47		; <i32 addrspace(1)*> [#uses=1]
	%tmp50 = load i32* %globalId		; <i32> [#uses=1]
	%mul51 = mul i32 %tmp50, 256		; <i32> [#uses=1]
	%tmp52 = load i32* %i21		; <i32> [#uses=1]
	%add53 = add i32 %mul51, %tmp52		; <i32> [#uses=1]
	%tmp54 = load i32 addrspace(1)** %unsortedData.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx55 = getelementptr i32 addrspace(1)* %tmp54, i32 %add53		; <i32 addrspace(1)*> [#uses=1]
	%tmp56 = load i32 addrspace(1)* %arrayidx55		; <i32> [#uses=1]
	store i32 %tmp56, i32 addrspace(1)* %arrayidx49
	%tmp57 = load i32* %localId		; <i32> [#uses=1]
	%mul58 = mul i32 %tmp57, 256		; <i32> [#uses=1]
	%tmp59 = load i32* %value		; <i32> [#uses=1]
	%add60 = add i32 %mul58, %tmp59		; <i32> [#uses=1]
	%tmp61 = load i16 addrspace(3)** %sharedBuckets.addr		; <i16 addrspace(3)*> [#uses=1]
	%arrayidx62 = getelementptr i16 addrspace(3)* %tmp61, i32 %add60		; <i16 addrspace(3)*> [#uses=1]
	%tmp63 = load i32* %index		; <i32> [#uses=1]
	%add64 = add i32 %tmp63, 1		; <i32> [#uses=1]
	%conv65 = trunc i32 %add64 to i16		; <i16> [#uses=1]
	store i16 %conv65, i16 addrspace(3)* %arrayidx62
	br label %for.inc66

for.inc66:		; preds = %for.body26
	%tmp67 = load i32* %i21		; <i32> [#uses=1]
	%inc68 = add i32 %tmp67, 1		; <i32> [#uses=1]
	store i32 %inc68, i32* %i21
	br label %for.cond22

for.end69:		; preds = %for.cond22
	ret void
}
