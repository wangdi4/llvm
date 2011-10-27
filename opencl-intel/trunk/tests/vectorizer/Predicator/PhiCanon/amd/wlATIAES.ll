; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIAES.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [9 x i8] c"22229900\00"		; <[9 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [9 x i8] c"22229900\00"		; <[9 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [2 x %0] [%0 { i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i8 addrspace(1)*, <4 x i8> addrspace(3)*, <4 x i8> addrspace(3)*, i32, i32, ...)* @AESEncrypt to i8*), i8* getelementptr ([9 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }, %0 { i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i8 addrspace(1)*, <4 x i8> addrspace(3)*, <4 x i8> addrspace(3)*, i32, i32, ...)* @AESDecrypt to i8*), i8* getelementptr ([9 x i8]* @sgv1, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv2, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv3 to i8*), i32 0 }], section "llvm.metadata"		; <[2 x %0]*> [#uses=0]

; CHECK: @shiftRows
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: ret

define <4 x i8> @shiftRows(<4 x i8> %row, i32 %j) nounwind {
entry:
	%retval = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%row.addr = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%j.addr = alloca i32		; <i32*> [#uses=2]
	%r = alloca <4 x i8>, align 4		; <<4 x i8>*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	store <4 x i8> %row, <4 x i8>* %row.addr
	store i32 %j, i32* %j.addr
	%tmp = load <4 x i8>* %row.addr		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp, <4 x i8>* %r
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp2 = load i32* %i		; <i32> [#uses=1]
	%tmp3 = load i32* %j.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp2, %tmp3		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp4 = load <4 x i8>* %r		; <<4 x i8>> [#uses=1]
	%tmp5 = shufflevector <4 x i8> %tmp4, <4 x i8> undef, <4 x i32> <i32 1, i32 2, i32 3, i32 0>		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp5, <4 x i8>* %r
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp6 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp6, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp7 = load <4 x i8>* %r		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp7, <4 x i8>* %retval
	%0 = load <4 x i8>* %retval		; <<4 x i8>> [#uses=1]
	ret <4 x i8> %0
}

define void @AESEncrypt(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %roundKey, i8 addrspace(1)* %SBox, <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)* %block1, i32 %width, i32 %rounds, ...) nounwind {
entry:
	%output.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%input.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%roundKey.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=4]
	%SBox.addr = alloca i8 addrspace(1)*		; <i8 addrspace(1)**> [#uses=3]
	%block0.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=21]
	%block1.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=3]
	%width.addr = alloca i32		; <i32*> [#uses=2]
	%rounds.addr = alloca i32		; <i32*> [#uses=3]
	%blockIdx = alloca i32, align 4		; <i32*> [#uses=2]
	%blockIdy = alloca i32, align 4		; <i32*> [#uses=2]
	%localIdx = alloca i32, align 4		; <i32*> [#uses=1]
	%localIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%globalIndex = alloca i32, align 4		; <i32*> [#uses=3]
	%localIndex = alloca i32, align 4		; <i32*> [#uses=28]
	%galiosCoeff = alloca [4 x <4 x i8>], align 4		; <[4 x <4 x i8>]*> [#uses=12]
	%.compoundliteral = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral15 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral19 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral23 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%r = alloca i32, align 4		; <i32*> [#uses=5]
	%p = alloca i8, align 1		; <i8*> [#uses=33]
	%a = alloca i8, align 1		; <i8*> [#uses=57]
	%b = alloca i8, align 1		; <i8*> [#uses=33]
	%bw = alloca i32, align 4		; <i32*> [#uses=17]
	%x = alloca i8, align 1		; <i8*> [#uses=4]
	%y = alloca i8, align 1		; <i8*> [#uses=4]
	%z = alloca i8, align 1		; <i8*> [#uses=4]
	%w = alloca i8, align 1		; <i8*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet = alloca i8, align 1		; <i8*> [#uses=2]
	%i134 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet154 = alloca i8, align 1		; <i8*> [#uses=2]
	%i198 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet218 = alloca i8, align 1		; <i8*> [#uses=2]
	%i262 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet282 = alloca i8, align 1		; <i8*> [#uses=2]
	%k = alloca i32, align 4		; <i32*> [#uses=12]
	%x1 = alloca i8, align 1		; <i8*> [#uses=2]
	%y1 = alloca i8, align 1		; <i8*> [#uses=2]
	%z1 = alloca i8, align 1		; <i8*> [#uses=2]
	%w1 = alloca i8, align 1		; <i8*> [#uses=2]
	%i339 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet359 = alloca i8, align 1		; <i8*> [#uses=2]
	%i406 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet426 = alloca i8, align 1		; <i8*> [#uses=2]
	%i473 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet493 = alloca i8, align 1		; <i8*> [#uses=2]
	%i540 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet560 = alloca i8, align 1		; <i8*> [#uses=2]
	%.compoundliteral619 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
	store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
	store <4 x i8> addrspace(1)* %roundKey, <4 x i8> addrspace(1)** %roundKey.addr
	store i8 addrspace(1)* %SBox, i8 addrspace(1)** %SBox.addr
	store <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)** %block0.addr
	store <4 x i8> addrspace(3)* %block1, <4 x i8> addrspace(3)** %block1.addr
	store i32 %width, i32* %width.addr
	store i32 %rounds, i32* %rounds.addr
	%call = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %blockIdx
	%call1 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %blockIdy
	%call2 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %localIdx
	%call3 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %localIdy
	%tmp = load i32* %blockIdy		; <i32> [#uses=1]
	%tmp4 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp4		; <i32> [#uses=1]
	%div = udiv i32 %mul, 4		; <i32> [#uses=1]
	%tmp5 = load i32* %blockIdx		; <i32> [#uses=1]
	%add = add i32 %div, %tmp5		; <i32> [#uses=1]
	%mul6 = mul i32 %add, 4		; <i32> [#uses=1]
	%tmp7 = load i32* %localIdy		; <i32> [#uses=1]
	%add8 = add i32 %mul6, %tmp7		; <i32> [#uses=1]
	store i32 %add8, i32* %globalIndex
	%tmp10 = load i32* %localIdy		; <i32> [#uses=1]
	store i32 %tmp10, i32* %localIndex
	%arraydecay = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx = getelementptr <4 x i8>* %arraydecay, i32 0		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 2, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral
	%tmp12 = load <4 x i8>* %.compoundliteral		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp12, <4 x i8>* %arrayidx
	%arraydecay13 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx14 = getelementptr <4 x i8>* %arraydecay13, i32 1		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 3, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral15
	%tmp16 = load <4 x i8>* %.compoundliteral15		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp16, <4 x i8>* %arrayidx14
	%arraydecay17 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx18 = getelementptr <4 x i8>* %arraydecay17, i32 2		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 1, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral19
	%tmp20 = load <4 x i8>* %.compoundliteral19		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp20, <4 x i8>* %arrayidx18
	%arraydecay21 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx22 = getelementptr <4 x i8>* %arraydecay21, i32 3		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 1, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral23
	%tmp24 = load <4 x i8>* %.compoundliteral23		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp24, <4 x i8>* %arrayidx22
	%tmp25 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp26 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx27 = getelementptr <4 x i8> addrspace(3)* %tmp26, i32 %tmp25		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp28 = load i32* %globalIndex		; <i32> [#uses=1]
	%tmp29 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx30 = getelementptr <4 x i8> addrspace(1)* %tmp29, i32 %tmp28		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp31 = load <4 x i8> addrspace(1)* %arrayidx30		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp31, <4 x i8> addrspace(3)* %arrayidx27
	%tmp32 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp33 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx34 = getelementptr <4 x i8> addrspace(3)* %tmp33, i32 %tmp32		; <<4 x i8> addrspace(3)*> [#uses=2]
	%tmp35 = load <4 x i8> addrspace(3)* %arrayidx34		; <<4 x i8>> [#uses=1]
	%tmp36 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp37 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx38 = getelementptr <4 x i8> addrspace(1)* %tmp37, i32 %tmp36		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp39 = load <4 x i8> addrspace(1)* %arrayidx38		; <<4 x i8>> [#uses=1]
	%xor = xor <4 x i8> %tmp35, %tmp39		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor, <4 x i8> addrspace(3)* %arrayidx34
	store i32 1, i32* %r
	br label %for.cond

for.cond:		; preds = %for.inc640, %entry
	%tmp41 = load i32* %r		; <i32> [#uses=1]
	%tmp42 = load i32* %rounds.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp41, %tmp42		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end643

for.body:		; preds = %for.cond
	%tmp43 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp44 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx45 = getelementptr <4 x i8> addrspace(3)* %tmp44, i32 %tmp43		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp46 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%tmp47 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp48 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx49 = getelementptr <4 x i8> addrspace(3)* %tmp48, i32 %tmp47		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp50 = load <4 x i8> addrspace(3)* %arrayidx49		; <<4 x i8>> [#uses=1]
	%call51 = call <4 x i8> @sbox(i8 addrspace(1)* %tmp46, <4 x i8> %tmp50)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call51, <4 x i8> addrspace(3)* %arrayidx45
	%tmp52 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp53 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx54 = getelementptr <4 x i8> addrspace(3)* %tmp53, i32 %tmp52		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp55 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp56 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx57 = getelementptr <4 x i8> addrspace(3)* %tmp56, i32 %tmp55		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp58 = load <4 x i8> addrspace(3)* %arrayidx57		; <<4 x i8>> [#uses=1]
	%tmp59 = load i32* %localIndex		; <i32> [#uses=1]
	%call60 = call <4 x i8> @shiftRows(<4 x i8> %tmp58, i32 %tmp59)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call60, <4 x i8> addrspace(3)* %arrayidx54
	call void @barrier(i32 1)
	store i8 0, i8* %p
	store i8 0, i8* %a
	store i8 0, i8* %b
	store i32 4, i32* %bw
	store i8 0, i8* %p
	%tmp69 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx70 = getelementptr <4 x i8> addrspace(3)* %tmp69, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp71 = load <4 x i8> addrspace(3)* %arrayidx70		; <<4 x i8>> [#uses=1]
	%tmp72 = extractelement <4 x i8> %tmp71, i32 0		; <i8> [#uses=1]
	store i8 %tmp72, i8* %a
	%tmp73 = load i32* %bw		; <i32> [#uses=1]
	%tmp74 = load i32* %localIndex		; <i32> [#uses=1]
	%sub = sub i32 %tmp73, %tmp74		; <i32> [#uses=1]
	%tmp75 = load i32* %bw		; <i32> [#uses=2]
	%cmp76 = icmp ne i32 %tmp75, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp76, i32 %tmp75, i32 1		; <i32> [#uses=1]
	%rem = urem i32 %sub, %nonzero		; <i32> [#uses=1]
	%arraydecay77 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx78 = getelementptr <4 x i8>* %arraydecay77, i32 %rem		; <<4 x i8>*> [#uses=1]
	%tmp79 = load <4 x i8>* %arrayidx78		; <<4 x i8>> [#uses=1]
	%tmp80 = extractelement <4 x i8> %tmp79, i32 0		; <i8> [#uses=1]
	store i8 %tmp80, i8* %b
	store i32 0, i32* %i
	br label %for.cond82

for.cond82:		; preds = %for.inc, %for.body
	%tmp83 = load i32* %i		; <i32> [#uses=1]
	%cmp84 = icmp ult i32 %tmp83, 8		; <i1> [#uses=1]
	br i1 %cmp84, label %for.body85, label %for.end

for.body85:		; preds = %for.cond82
	%tmp86 = load i8* %b		; <i8> [#uses=1]
	%conv = zext i8 %tmp86 to i32		; <i32> [#uses=1]
	%and = and i32 %conv, 1		; <i32> [#uses=1]
	%cmp87 = icmp eq i32 %and, 1		; <i1> [#uses=1]
	br i1 %cmp87, label %if.then, label %if.end

if.then:		; preds = %for.body85
	%tmp89 = load i8* %p		; <i8> [#uses=1]
	%conv90 = zext i8 %tmp89 to i32		; <i32> [#uses=1]
	%tmp91 = load i8* %a		; <i8> [#uses=1]
	%conv92 = zext i8 %tmp91 to i32		; <i32> [#uses=1]
	%xor93 = xor i32 %conv90, %conv92		; <i32> [#uses=1]
	%conv94 = trunc i32 %xor93 to i8		; <i8> [#uses=1]
	store i8 %conv94, i8* %p
	br label %if.end

if.end:		; preds = %if.then, %for.body85
	%tmp96 = load i8* %a		; <i8> [#uses=1]
	%conv97 = zext i8 %tmp96 to i32		; <i32> [#uses=1]
	%and98 = and i32 %conv97, 128		; <i32> [#uses=1]
	%conv99 = trunc i32 %and98 to i8		; <i8> [#uses=1]
	store i8 %conv99, i8* %hiBitSet
	%tmp100 = load i8* %a		; <i8> [#uses=1]
	%conv101 = zext i8 %tmp100 to i32		; <i32> [#uses=1]
	%shl = shl i32 %conv101, 1		; <i32> [#uses=1]
	%conv102 = trunc i32 %shl to i8		; <i8> [#uses=1]
	store i8 %conv102, i8* %a
	%tmp103 = load i8* %hiBitSet		; <i8> [#uses=1]
	%conv104 = zext i8 %tmp103 to i32		; <i32> [#uses=1]
	%cmp105 = icmp eq i32 %conv104, 128		; <i1> [#uses=1]
	br i1 %cmp105, label %if.then107, label %if.end112

if.then107:		; preds = %if.end
	%tmp108 = load i8* %a		; <i8> [#uses=1]
	%conv109 = zext i8 %tmp108 to i32		; <i32> [#uses=1]
	%xor110 = xor i32 %conv109, 27		; <i32> [#uses=1]
	%conv111 = trunc i32 %xor110 to i8		; <i8> [#uses=1]
	store i8 %conv111, i8* %a
	br label %if.end112

if.end112:		; preds = %if.then107, %if.end
	%tmp113 = load i8* %b		; <i8> [#uses=1]
	%conv114 = zext i8 %tmp113 to i32		; <i32> [#uses=1]
	%shr = ashr i32 %conv114, 1		; <i32> [#uses=1]
	%conv115 = trunc i32 %shr to i8		; <i8> [#uses=1]
	store i8 %conv115, i8* %b
	br label %for.inc

for.inc:		; preds = %if.end112
	%tmp116 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp116, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond82

for.end:		; preds = %for.cond82
	%tmp117 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp117, i8* %x
	store i8 0, i8* %p
	%tmp118 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx119 = getelementptr <4 x i8> addrspace(3)* %tmp118, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp120 = load <4 x i8> addrspace(3)* %arrayidx119		; <<4 x i8>> [#uses=1]
	%tmp121 = extractelement <4 x i8> %tmp120, i32 1		; <i8> [#uses=1]
	store i8 %tmp121, i8* %a
	%tmp122 = load i32* %bw		; <i32> [#uses=1]
	%tmp123 = load i32* %localIndex		; <i32> [#uses=1]
	%sub124 = sub i32 %tmp122, %tmp123		; <i32> [#uses=1]
	%tmp125 = load i32* %bw		; <i32> [#uses=2]
	%cmp126 = icmp ne i32 %tmp125, 0		; <i1> [#uses=1]
	%nonzero127 = select i1 %cmp126, i32 %tmp125, i32 1		; <i32> [#uses=1]
	%rem128 = urem i32 %sub124, %nonzero127		; <i32> [#uses=1]
	%arraydecay129 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx130 = getelementptr <4 x i8>* %arraydecay129, i32 %rem128		; <<4 x i8>*> [#uses=1]
	%tmp131 = load <4 x i8>* %arrayidx130		; <<4 x i8>> [#uses=1]
	%tmp132 = extractelement <4 x i8> %tmp131, i32 0		; <i8> [#uses=1]
	store i8 %tmp132, i8* %b
	store i32 0, i32* %i134
	br label %for.cond135

for.cond135:		; preds = %for.inc177, %for.end
	%tmp136 = load i32* %i134		; <i32> [#uses=1]
	%cmp137 = icmp ult i32 %tmp136, 8		; <i1> [#uses=1]
	br i1 %cmp137, label %for.body139, label %for.end180

for.body139:		; preds = %for.cond135
	%tmp140 = load i8* %b		; <i8> [#uses=1]
	%conv141 = zext i8 %tmp140 to i32		; <i32> [#uses=1]
	%and142 = and i32 %conv141, 1		; <i32> [#uses=1]
	%cmp143 = icmp eq i32 %and142, 1		; <i1> [#uses=1]
	br i1 %cmp143, label %if.then145, label %if.end152

if.then145:		; preds = %for.body139
	%tmp146 = load i8* %p		; <i8> [#uses=1]
	%conv147 = zext i8 %tmp146 to i32		; <i32> [#uses=1]
	%tmp148 = load i8* %a		; <i8> [#uses=1]
	%conv149 = zext i8 %tmp148 to i32		; <i32> [#uses=1]
	%xor150 = xor i32 %conv147, %conv149		; <i32> [#uses=1]
	%conv151 = trunc i32 %xor150 to i8		; <i8> [#uses=1]
	store i8 %conv151, i8* %p
	br label %if.end152

if.end152:		; preds = %if.then145, %for.body139
	%tmp155 = load i8* %a		; <i8> [#uses=1]
	%conv156 = zext i8 %tmp155 to i32		; <i32> [#uses=1]
	%and157 = and i32 %conv156, 128		; <i32> [#uses=1]
	%conv158 = trunc i32 %and157 to i8		; <i8> [#uses=1]
	store i8 %conv158, i8* %hiBitSet154
	%tmp159 = load i8* %a		; <i8> [#uses=1]
	%conv160 = zext i8 %tmp159 to i32		; <i32> [#uses=1]
	%shl161 = shl i32 %conv160, 1		; <i32> [#uses=1]
	%conv162 = trunc i32 %shl161 to i8		; <i8> [#uses=1]
	store i8 %conv162, i8* %a
	%tmp163 = load i8* %hiBitSet154		; <i8> [#uses=1]
	%conv164 = zext i8 %tmp163 to i32		; <i32> [#uses=1]
	%cmp165 = icmp eq i32 %conv164, 128		; <i1> [#uses=1]
	br i1 %cmp165, label %if.then167, label %if.end172

if.then167:		; preds = %if.end152
	%tmp168 = load i8* %a		; <i8> [#uses=1]
	%conv169 = zext i8 %tmp168 to i32		; <i32> [#uses=1]
	%xor170 = xor i32 %conv169, 27		; <i32> [#uses=1]
	%conv171 = trunc i32 %xor170 to i8		; <i8> [#uses=1]
	store i8 %conv171, i8* %a
	br label %if.end172

if.end172:		; preds = %if.then167, %if.end152
	%tmp173 = load i8* %b		; <i8> [#uses=1]
	%conv174 = zext i8 %tmp173 to i32		; <i32> [#uses=1]
	%shr175 = ashr i32 %conv174, 1		; <i32> [#uses=1]
	%conv176 = trunc i32 %shr175 to i8		; <i8> [#uses=1]
	store i8 %conv176, i8* %b
	br label %for.inc177

for.inc177:		; preds = %if.end172
	%tmp178 = load i32* %i134		; <i32> [#uses=1]
	%inc179 = add i32 %tmp178, 1		; <i32> [#uses=1]
	store i32 %inc179, i32* %i134
	br label %for.cond135

for.end180:		; preds = %for.cond135
	%tmp181 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp181, i8* %y
	store i8 0, i8* %p
	%tmp182 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx183 = getelementptr <4 x i8> addrspace(3)* %tmp182, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp184 = load <4 x i8> addrspace(3)* %arrayidx183		; <<4 x i8>> [#uses=1]
	%tmp185 = extractelement <4 x i8> %tmp184, i32 2		; <i8> [#uses=1]
	store i8 %tmp185, i8* %a
	%tmp186 = load i32* %bw		; <i32> [#uses=1]
	%tmp187 = load i32* %localIndex		; <i32> [#uses=1]
	%sub188 = sub i32 %tmp186, %tmp187		; <i32> [#uses=1]
	%tmp189 = load i32* %bw		; <i32> [#uses=2]
	%cmp190 = icmp ne i32 %tmp189, 0		; <i1> [#uses=1]
	%nonzero191 = select i1 %cmp190, i32 %tmp189, i32 1		; <i32> [#uses=1]
	%rem192 = urem i32 %sub188, %nonzero191		; <i32> [#uses=1]
	%arraydecay193 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx194 = getelementptr <4 x i8>* %arraydecay193, i32 %rem192		; <<4 x i8>*> [#uses=1]
	%tmp195 = load <4 x i8>* %arrayidx194		; <<4 x i8>> [#uses=1]
	%tmp196 = extractelement <4 x i8> %tmp195, i32 0		; <i8> [#uses=1]
	store i8 %tmp196, i8* %b
	store i32 0, i32* %i198
	br label %for.cond199

for.cond199:		; preds = %for.inc241, %for.end180
	%tmp200 = load i32* %i198		; <i32> [#uses=1]
	%cmp201 = icmp ult i32 %tmp200, 8		; <i1> [#uses=1]
	br i1 %cmp201, label %for.body203, label %for.end244

for.body203:		; preds = %for.cond199
	%tmp204 = load i8* %b		; <i8> [#uses=1]
	%conv205 = zext i8 %tmp204 to i32		; <i32> [#uses=1]
	%and206 = and i32 %conv205, 1		; <i32> [#uses=1]
	%cmp207 = icmp eq i32 %and206, 1		; <i1> [#uses=1]
	br i1 %cmp207, label %if.then209, label %if.end216

if.then209:		; preds = %for.body203
	%tmp210 = load i8* %p		; <i8> [#uses=1]
	%conv211 = zext i8 %tmp210 to i32		; <i32> [#uses=1]
	%tmp212 = load i8* %a		; <i8> [#uses=1]
	%conv213 = zext i8 %tmp212 to i32		; <i32> [#uses=1]
	%xor214 = xor i32 %conv211, %conv213		; <i32> [#uses=1]
	%conv215 = trunc i32 %xor214 to i8		; <i8> [#uses=1]
	store i8 %conv215, i8* %p
	br label %if.end216

if.end216:		; preds = %if.then209, %for.body203
	%tmp219 = load i8* %a		; <i8> [#uses=1]
	%conv220 = zext i8 %tmp219 to i32		; <i32> [#uses=1]
	%and221 = and i32 %conv220, 128		; <i32> [#uses=1]
	%conv222 = trunc i32 %and221 to i8		; <i8> [#uses=1]
	store i8 %conv222, i8* %hiBitSet218
	%tmp223 = load i8* %a		; <i8> [#uses=1]
	%conv224 = zext i8 %tmp223 to i32		; <i32> [#uses=1]
	%shl225 = shl i32 %conv224, 1		; <i32> [#uses=1]
	%conv226 = trunc i32 %shl225 to i8		; <i8> [#uses=1]
	store i8 %conv226, i8* %a
	%tmp227 = load i8* %hiBitSet218		; <i8> [#uses=1]
	%conv228 = zext i8 %tmp227 to i32		; <i32> [#uses=1]
	%cmp229 = icmp eq i32 %conv228, 128		; <i1> [#uses=1]
	br i1 %cmp229, label %if.then231, label %if.end236

if.then231:		; preds = %if.end216
	%tmp232 = load i8* %a		; <i8> [#uses=1]
	%conv233 = zext i8 %tmp232 to i32		; <i32> [#uses=1]
	%xor234 = xor i32 %conv233, 27		; <i32> [#uses=1]
	%conv235 = trunc i32 %xor234 to i8		; <i8> [#uses=1]
	store i8 %conv235, i8* %a
	br label %if.end236

if.end236:		; preds = %if.then231, %if.end216
	%tmp237 = load i8* %b		; <i8> [#uses=1]
	%conv238 = zext i8 %tmp237 to i32		; <i32> [#uses=1]
	%shr239 = ashr i32 %conv238, 1		; <i32> [#uses=1]
	%conv240 = trunc i32 %shr239 to i8		; <i8> [#uses=1]
	store i8 %conv240, i8* %b
	br label %for.inc241

for.inc241:		; preds = %if.end236
	%tmp242 = load i32* %i198		; <i32> [#uses=1]
	%inc243 = add i32 %tmp242, 1		; <i32> [#uses=1]
	store i32 %inc243, i32* %i198
	br label %for.cond199

for.end244:		; preds = %for.cond199
	%tmp245 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp245, i8* %z
	store i8 0, i8* %p
	%tmp246 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx247 = getelementptr <4 x i8> addrspace(3)* %tmp246, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp248 = load <4 x i8> addrspace(3)* %arrayidx247		; <<4 x i8>> [#uses=1]
	%tmp249 = extractelement <4 x i8> %tmp248, i32 3		; <i8> [#uses=1]
	store i8 %tmp249, i8* %a
	%tmp250 = load i32* %bw		; <i32> [#uses=1]
	%tmp251 = load i32* %localIndex		; <i32> [#uses=1]
	%sub252 = sub i32 %tmp250, %tmp251		; <i32> [#uses=1]
	%tmp253 = load i32* %bw		; <i32> [#uses=2]
	%cmp254 = icmp ne i32 %tmp253, 0		; <i1> [#uses=1]
	%nonzero255 = select i1 %cmp254, i32 %tmp253, i32 1		; <i32> [#uses=1]
	%rem256 = urem i32 %sub252, %nonzero255		; <i32> [#uses=1]
	%arraydecay257 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx258 = getelementptr <4 x i8>* %arraydecay257, i32 %rem256		; <<4 x i8>*> [#uses=1]
	%tmp259 = load <4 x i8>* %arrayidx258		; <<4 x i8>> [#uses=1]
	%tmp260 = extractelement <4 x i8> %tmp259, i32 0		; <i8> [#uses=1]
	store i8 %tmp260, i8* %b
	store i32 0, i32* %i262
	br label %for.cond263

for.cond263:		; preds = %for.inc305, %for.end244
	%tmp264 = load i32* %i262		; <i32> [#uses=1]
	%cmp265 = icmp ult i32 %tmp264, 8		; <i1> [#uses=1]
	br i1 %cmp265, label %for.body267, label %for.end308

for.body267:		; preds = %for.cond263
	%tmp268 = load i8* %b		; <i8> [#uses=1]
	%conv269 = zext i8 %tmp268 to i32		; <i32> [#uses=1]
	%and270 = and i32 %conv269, 1		; <i32> [#uses=1]
	%cmp271 = icmp eq i32 %and270, 1		; <i1> [#uses=1]
	br i1 %cmp271, label %if.then273, label %if.end280

if.then273:		; preds = %for.body267
	%tmp274 = load i8* %p		; <i8> [#uses=1]
	%conv275 = zext i8 %tmp274 to i32		; <i32> [#uses=1]
	%tmp276 = load i8* %a		; <i8> [#uses=1]
	%conv277 = zext i8 %tmp276 to i32		; <i32> [#uses=1]
	%xor278 = xor i32 %conv275, %conv277		; <i32> [#uses=1]
	%conv279 = trunc i32 %xor278 to i8		; <i8> [#uses=1]
	store i8 %conv279, i8* %p
	br label %if.end280

if.end280:		; preds = %if.then273, %for.body267
	%tmp283 = load i8* %a		; <i8> [#uses=1]
	%conv284 = zext i8 %tmp283 to i32		; <i32> [#uses=1]
	%and285 = and i32 %conv284, 128		; <i32> [#uses=1]
	%conv286 = trunc i32 %and285 to i8		; <i8> [#uses=1]
	store i8 %conv286, i8* %hiBitSet282
	%tmp287 = load i8* %a		; <i8> [#uses=1]
	%conv288 = zext i8 %tmp287 to i32		; <i32> [#uses=1]
	%shl289 = shl i32 %conv288, 1		; <i32> [#uses=1]
	%conv290 = trunc i32 %shl289 to i8		; <i8> [#uses=1]
	store i8 %conv290, i8* %a
	%tmp291 = load i8* %hiBitSet282		; <i8> [#uses=1]
	%conv292 = zext i8 %tmp291 to i32		; <i32> [#uses=1]
	%cmp293 = icmp eq i32 %conv292, 128		; <i1> [#uses=1]
	br i1 %cmp293, label %if.then295, label %if.end300

if.then295:		; preds = %if.end280
	%tmp296 = load i8* %a		; <i8> [#uses=1]
	%conv297 = zext i8 %tmp296 to i32		; <i32> [#uses=1]
	%xor298 = xor i32 %conv297, 27		; <i32> [#uses=1]
	%conv299 = trunc i32 %xor298 to i8		; <i8> [#uses=1]
	store i8 %conv299, i8* %a
	br label %if.end300

if.end300:		; preds = %if.then295, %if.end280
	%tmp301 = load i8* %b		; <i8> [#uses=1]
	%conv302 = zext i8 %tmp301 to i32		; <i32> [#uses=1]
	%shr303 = ashr i32 %conv302, 1		; <i32> [#uses=1]
	%conv304 = trunc i32 %shr303 to i8		; <i8> [#uses=1]
	store i8 %conv304, i8* %b
	br label %for.inc305

for.inc305:		; preds = %if.end300
	%tmp306 = load i32* %i262		; <i32> [#uses=1]
	%inc307 = add i32 %tmp306, 1		; <i32> [#uses=1]
	store i32 %inc307, i32* %i262
	br label %for.cond263

for.end308:		; preds = %for.cond263
	%tmp309 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp309, i8* %w
	store i32 1, i32* %k
	br label %for.cond311

for.cond311:		; preds = %for.inc612, %for.end308
	%tmp312 = load i32* %k		; <i32> [#uses=1]
	%cmp313 = icmp ult i32 %tmp312, 4		; <i1> [#uses=1]
	br i1 %cmp313, label %for.body315, label %for.end615

for.body315:		; preds = %for.cond311
	store i8 0, i8* %p
	%tmp320 = load i32* %k		; <i32> [#uses=1]
	%tmp321 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx322 = getelementptr <4 x i8> addrspace(3)* %tmp321, i32 %tmp320		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp323 = load <4 x i8> addrspace(3)* %arrayidx322		; <<4 x i8>> [#uses=1]
	%tmp324 = extractelement <4 x i8> %tmp323, i32 0		; <i8> [#uses=1]
	store i8 %tmp324, i8* %a
	%tmp325 = load i32* %k		; <i32> [#uses=1]
	%tmp326 = load i32* %bw		; <i32> [#uses=1]
	%add327 = add i32 %tmp325, %tmp326		; <i32> [#uses=1]
	%tmp328 = load i32* %localIndex		; <i32> [#uses=1]
	%sub329 = sub i32 %add327, %tmp328		; <i32> [#uses=1]
	%tmp330 = load i32* %bw		; <i32> [#uses=2]
	%cmp331 = icmp ne i32 %tmp330, 0		; <i1> [#uses=1]
	%nonzero332 = select i1 %cmp331, i32 %tmp330, i32 1		; <i32> [#uses=1]
	%rem333 = urem i32 %sub329, %nonzero332		; <i32> [#uses=1]
	%arraydecay334 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx335 = getelementptr <4 x i8>* %arraydecay334, i32 %rem333		; <<4 x i8>*> [#uses=1]
	%tmp336 = load <4 x i8>* %arrayidx335		; <<4 x i8>> [#uses=1]
	%tmp337 = extractelement <4 x i8> %tmp336, i32 0		; <i8> [#uses=1]
	store i8 %tmp337, i8* %b
	store i32 0, i32* %i339
	br label %for.cond340

for.cond340:		; preds = %for.inc382, %for.body315
	%tmp341 = load i32* %i339		; <i32> [#uses=1]
	%cmp342 = icmp ult i32 %tmp341, 8		; <i1> [#uses=1]
	br i1 %cmp342, label %for.body344, label %for.end385

for.body344:		; preds = %for.cond340
	%tmp345 = load i8* %b		; <i8> [#uses=1]
	%conv346 = zext i8 %tmp345 to i32		; <i32> [#uses=1]
	%and347 = and i32 %conv346, 1		; <i32> [#uses=1]
	%cmp348 = icmp eq i32 %and347, 1		; <i1> [#uses=1]
	br i1 %cmp348, label %if.then350, label %if.end357

if.then350:		; preds = %for.body344
	%tmp351 = load i8* %p		; <i8> [#uses=1]
	%conv352 = zext i8 %tmp351 to i32		; <i32> [#uses=1]
	%tmp353 = load i8* %a		; <i8> [#uses=1]
	%conv354 = zext i8 %tmp353 to i32		; <i32> [#uses=1]
	%xor355 = xor i32 %conv352, %conv354		; <i32> [#uses=1]
	%conv356 = trunc i32 %xor355 to i8		; <i8> [#uses=1]
	store i8 %conv356, i8* %p
	br label %if.end357

if.end357:		; preds = %if.then350, %for.body344
	%tmp360 = load i8* %a		; <i8> [#uses=1]
	%conv361 = zext i8 %tmp360 to i32		; <i32> [#uses=1]
	%and362 = and i32 %conv361, 128		; <i32> [#uses=1]
	%conv363 = trunc i32 %and362 to i8		; <i8> [#uses=1]
	store i8 %conv363, i8* %hiBitSet359
	%tmp364 = load i8* %a		; <i8> [#uses=1]
	%conv365 = zext i8 %tmp364 to i32		; <i32> [#uses=1]
	%shl366 = shl i32 %conv365, 1		; <i32> [#uses=1]
	%conv367 = trunc i32 %shl366 to i8		; <i8> [#uses=1]
	store i8 %conv367, i8* %a
	%tmp368 = load i8* %hiBitSet359		; <i8> [#uses=1]
	%conv369 = zext i8 %tmp368 to i32		; <i32> [#uses=1]
	%cmp370 = icmp eq i32 %conv369, 128		; <i1> [#uses=1]
	br i1 %cmp370, label %if.then372, label %if.end377

if.then372:		; preds = %if.end357
	%tmp373 = load i8* %a		; <i8> [#uses=1]
	%conv374 = zext i8 %tmp373 to i32		; <i32> [#uses=1]
	%xor375 = xor i32 %conv374, 27		; <i32> [#uses=1]
	%conv376 = trunc i32 %xor375 to i8		; <i8> [#uses=1]
	store i8 %conv376, i8* %a
	br label %if.end377

if.end377:		; preds = %if.then372, %if.end357
	%tmp378 = load i8* %b		; <i8> [#uses=1]
	%conv379 = zext i8 %tmp378 to i32		; <i32> [#uses=1]
	%shr380 = ashr i32 %conv379, 1		; <i32> [#uses=1]
	%conv381 = trunc i32 %shr380 to i8		; <i8> [#uses=1]
	store i8 %conv381, i8* %b
	br label %for.inc382

for.inc382:		; preds = %if.end377
	%tmp383 = load i32* %i339		; <i32> [#uses=1]
	%inc384 = add i32 %tmp383, 1		; <i32> [#uses=1]
	store i32 %inc384, i32* %i339
	br label %for.cond340

for.end385:		; preds = %for.cond340
	%tmp386 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp386, i8* %x1
	store i8 0, i8* %p
	%tmp387 = load i32* %k		; <i32> [#uses=1]
	%tmp388 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx389 = getelementptr <4 x i8> addrspace(3)* %tmp388, i32 %tmp387		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp390 = load <4 x i8> addrspace(3)* %arrayidx389		; <<4 x i8>> [#uses=1]
	%tmp391 = extractelement <4 x i8> %tmp390, i32 1		; <i8> [#uses=1]
	store i8 %tmp391, i8* %a
	%tmp392 = load i32* %k		; <i32> [#uses=1]
	%tmp393 = load i32* %bw		; <i32> [#uses=1]
	%add394 = add i32 %tmp392, %tmp393		; <i32> [#uses=1]
	%tmp395 = load i32* %localIndex		; <i32> [#uses=1]
	%sub396 = sub i32 %add394, %tmp395		; <i32> [#uses=1]
	%tmp397 = load i32* %bw		; <i32> [#uses=2]
	%cmp398 = icmp ne i32 %tmp397, 0		; <i1> [#uses=1]
	%nonzero399 = select i1 %cmp398, i32 %tmp397, i32 1		; <i32> [#uses=1]
	%rem400 = urem i32 %sub396, %nonzero399		; <i32> [#uses=1]
	%arraydecay401 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx402 = getelementptr <4 x i8>* %arraydecay401, i32 %rem400		; <<4 x i8>*> [#uses=1]
	%tmp403 = load <4 x i8>* %arrayidx402		; <<4 x i8>> [#uses=1]
	%tmp404 = extractelement <4 x i8> %tmp403, i32 0		; <i8> [#uses=1]
	store i8 %tmp404, i8* %b
	store i32 0, i32* %i406
	br label %for.cond407

for.cond407:		; preds = %for.inc449, %for.end385
	%tmp408 = load i32* %i406		; <i32> [#uses=1]
	%cmp409 = icmp ult i32 %tmp408, 8		; <i1> [#uses=1]
	br i1 %cmp409, label %for.body411, label %for.end452

for.body411:		; preds = %for.cond407
	%tmp412 = load i8* %b		; <i8> [#uses=1]
	%conv413 = zext i8 %tmp412 to i32		; <i32> [#uses=1]
	%and414 = and i32 %conv413, 1		; <i32> [#uses=1]
	%cmp415 = icmp eq i32 %and414, 1		; <i1> [#uses=1]
	br i1 %cmp415, label %if.then417, label %if.end424

if.then417:		; preds = %for.body411
	%tmp418 = load i8* %p		; <i8> [#uses=1]
	%conv419 = zext i8 %tmp418 to i32		; <i32> [#uses=1]
	%tmp420 = load i8* %a		; <i8> [#uses=1]
	%conv421 = zext i8 %tmp420 to i32		; <i32> [#uses=1]
	%xor422 = xor i32 %conv419, %conv421		; <i32> [#uses=1]
	%conv423 = trunc i32 %xor422 to i8		; <i8> [#uses=1]
	store i8 %conv423, i8* %p
	br label %if.end424

if.end424:		; preds = %if.then417, %for.body411
	%tmp427 = load i8* %a		; <i8> [#uses=1]
	%conv428 = zext i8 %tmp427 to i32		; <i32> [#uses=1]
	%and429 = and i32 %conv428, 128		; <i32> [#uses=1]
	%conv430 = trunc i32 %and429 to i8		; <i8> [#uses=1]
	store i8 %conv430, i8* %hiBitSet426
	%tmp431 = load i8* %a		; <i8> [#uses=1]
	%conv432 = zext i8 %tmp431 to i32		; <i32> [#uses=1]
	%shl433 = shl i32 %conv432, 1		; <i32> [#uses=1]
	%conv434 = trunc i32 %shl433 to i8		; <i8> [#uses=1]
	store i8 %conv434, i8* %a
	%tmp435 = load i8* %hiBitSet426		; <i8> [#uses=1]
	%conv436 = zext i8 %tmp435 to i32		; <i32> [#uses=1]
	%cmp437 = icmp eq i32 %conv436, 128		; <i1> [#uses=1]
	br i1 %cmp437, label %if.then439, label %if.end444

if.then439:		; preds = %if.end424
	%tmp440 = load i8* %a		; <i8> [#uses=1]
	%conv441 = zext i8 %tmp440 to i32		; <i32> [#uses=1]
	%xor442 = xor i32 %conv441, 27		; <i32> [#uses=1]
	%conv443 = trunc i32 %xor442 to i8		; <i8> [#uses=1]
	store i8 %conv443, i8* %a
	br label %if.end444

if.end444:		; preds = %if.then439, %if.end424
	%tmp445 = load i8* %b		; <i8> [#uses=1]
	%conv446 = zext i8 %tmp445 to i32		; <i32> [#uses=1]
	%shr447 = ashr i32 %conv446, 1		; <i32> [#uses=1]
	%conv448 = trunc i32 %shr447 to i8		; <i8> [#uses=1]
	store i8 %conv448, i8* %b
	br label %for.inc449

for.inc449:		; preds = %if.end444
	%tmp450 = load i32* %i406		; <i32> [#uses=1]
	%inc451 = add i32 %tmp450, 1		; <i32> [#uses=1]
	store i32 %inc451, i32* %i406
	br label %for.cond407

for.end452:		; preds = %for.cond407
	%tmp453 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp453, i8* %y1
	store i8 0, i8* %p
	%tmp454 = load i32* %k		; <i32> [#uses=1]
	%tmp455 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx456 = getelementptr <4 x i8> addrspace(3)* %tmp455, i32 %tmp454		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp457 = load <4 x i8> addrspace(3)* %arrayidx456		; <<4 x i8>> [#uses=1]
	%tmp458 = extractelement <4 x i8> %tmp457, i32 2		; <i8> [#uses=1]
	store i8 %tmp458, i8* %a
	%tmp459 = load i32* %k		; <i32> [#uses=1]
	%tmp460 = load i32* %bw		; <i32> [#uses=1]
	%add461 = add i32 %tmp459, %tmp460		; <i32> [#uses=1]
	%tmp462 = load i32* %localIndex		; <i32> [#uses=1]
	%sub463 = sub i32 %add461, %tmp462		; <i32> [#uses=1]
	%tmp464 = load i32* %bw		; <i32> [#uses=2]
	%cmp465 = icmp ne i32 %tmp464, 0		; <i1> [#uses=1]
	%nonzero466 = select i1 %cmp465, i32 %tmp464, i32 1		; <i32> [#uses=1]
	%rem467 = urem i32 %sub463, %nonzero466		; <i32> [#uses=1]
	%arraydecay468 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx469 = getelementptr <4 x i8>* %arraydecay468, i32 %rem467		; <<4 x i8>*> [#uses=1]
	%tmp470 = load <4 x i8>* %arrayidx469		; <<4 x i8>> [#uses=1]
	%tmp471 = extractelement <4 x i8> %tmp470, i32 0		; <i8> [#uses=1]
	store i8 %tmp471, i8* %b
	store i32 0, i32* %i473
	br label %for.cond474

for.cond474:		; preds = %for.inc516, %for.end452
	%tmp475 = load i32* %i473		; <i32> [#uses=1]
	%cmp476 = icmp ult i32 %tmp475, 8		; <i1> [#uses=1]
	br i1 %cmp476, label %for.body478, label %for.end519

for.body478:		; preds = %for.cond474
	%tmp479 = load i8* %b		; <i8> [#uses=1]
	%conv480 = zext i8 %tmp479 to i32		; <i32> [#uses=1]
	%and481 = and i32 %conv480, 1		; <i32> [#uses=1]
	%cmp482 = icmp eq i32 %and481, 1		; <i1> [#uses=1]
	br i1 %cmp482, label %if.then484, label %if.end491

if.then484:		; preds = %for.body478
	%tmp485 = load i8* %p		; <i8> [#uses=1]
	%conv486 = zext i8 %tmp485 to i32		; <i32> [#uses=1]
	%tmp487 = load i8* %a		; <i8> [#uses=1]
	%conv488 = zext i8 %tmp487 to i32		; <i32> [#uses=1]
	%xor489 = xor i32 %conv486, %conv488		; <i32> [#uses=1]
	%conv490 = trunc i32 %xor489 to i8		; <i8> [#uses=1]
	store i8 %conv490, i8* %p
	br label %if.end491

if.end491:		; preds = %if.then484, %for.body478
	%tmp494 = load i8* %a		; <i8> [#uses=1]
	%conv495 = zext i8 %tmp494 to i32		; <i32> [#uses=1]
	%and496 = and i32 %conv495, 128		; <i32> [#uses=1]
	%conv497 = trunc i32 %and496 to i8		; <i8> [#uses=1]
	store i8 %conv497, i8* %hiBitSet493
	%tmp498 = load i8* %a		; <i8> [#uses=1]
	%conv499 = zext i8 %tmp498 to i32		; <i32> [#uses=1]
	%shl500 = shl i32 %conv499, 1		; <i32> [#uses=1]
	%conv501 = trunc i32 %shl500 to i8		; <i8> [#uses=1]
	store i8 %conv501, i8* %a
	%tmp502 = load i8* %hiBitSet493		; <i8> [#uses=1]
	%conv503 = zext i8 %tmp502 to i32		; <i32> [#uses=1]
	%cmp504 = icmp eq i32 %conv503, 128		; <i1> [#uses=1]
	br i1 %cmp504, label %if.then506, label %if.end511

if.then506:		; preds = %if.end491
	%tmp507 = load i8* %a		; <i8> [#uses=1]
	%conv508 = zext i8 %tmp507 to i32		; <i32> [#uses=1]
	%xor509 = xor i32 %conv508, 27		; <i32> [#uses=1]
	%conv510 = trunc i32 %xor509 to i8		; <i8> [#uses=1]
	store i8 %conv510, i8* %a
	br label %if.end511

if.end511:		; preds = %if.then506, %if.end491
	%tmp512 = load i8* %b		; <i8> [#uses=1]
	%conv513 = zext i8 %tmp512 to i32		; <i32> [#uses=1]
	%shr514 = ashr i32 %conv513, 1		; <i32> [#uses=1]
	%conv515 = trunc i32 %shr514 to i8		; <i8> [#uses=1]
	store i8 %conv515, i8* %b
	br label %for.inc516

for.inc516:		; preds = %if.end511
	%tmp517 = load i32* %i473		; <i32> [#uses=1]
	%inc518 = add i32 %tmp517, 1		; <i32> [#uses=1]
	store i32 %inc518, i32* %i473
	br label %for.cond474

for.end519:		; preds = %for.cond474
	%tmp520 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp520, i8* %z1
	store i8 0, i8* %p
	%tmp521 = load i32* %k		; <i32> [#uses=1]
	%tmp522 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx523 = getelementptr <4 x i8> addrspace(3)* %tmp522, i32 %tmp521		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp524 = load <4 x i8> addrspace(3)* %arrayidx523		; <<4 x i8>> [#uses=1]
	%tmp525 = extractelement <4 x i8> %tmp524, i32 3		; <i8> [#uses=1]
	store i8 %tmp525, i8* %a
	%tmp526 = load i32* %k		; <i32> [#uses=1]
	%tmp527 = load i32* %bw		; <i32> [#uses=1]
	%add528 = add i32 %tmp526, %tmp527		; <i32> [#uses=1]
	%tmp529 = load i32* %localIndex		; <i32> [#uses=1]
	%sub530 = sub i32 %add528, %tmp529		; <i32> [#uses=1]
	%tmp531 = load i32* %bw		; <i32> [#uses=2]
	%cmp532 = icmp ne i32 %tmp531, 0		; <i1> [#uses=1]
	%nonzero533 = select i1 %cmp532, i32 %tmp531, i32 1		; <i32> [#uses=1]
	%rem534 = urem i32 %sub530, %nonzero533		; <i32> [#uses=1]
	%arraydecay535 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx536 = getelementptr <4 x i8>* %arraydecay535, i32 %rem534		; <<4 x i8>*> [#uses=1]
	%tmp537 = load <4 x i8>* %arrayidx536		; <<4 x i8>> [#uses=1]
	%tmp538 = extractelement <4 x i8> %tmp537, i32 0		; <i8> [#uses=1]
	store i8 %tmp538, i8* %b
	store i32 0, i32* %i540
	br label %for.cond541

for.cond541:		; preds = %for.inc583, %for.end519
	%tmp542 = load i32* %i540		; <i32> [#uses=1]
	%cmp543 = icmp ult i32 %tmp542, 8		; <i1> [#uses=1]
	br i1 %cmp543, label %for.body545, label %for.end586

for.body545:		; preds = %for.cond541
	%tmp546 = load i8* %b		; <i8> [#uses=1]
	%conv547 = zext i8 %tmp546 to i32		; <i32> [#uses=1]
	%and548 = and i32 %conv547, 1		; <i32> [#uses=1]
	%cmp549 = icmp eq i32 %and548, 1		; <i1> [#uses=1]
	br i1 %cmp549, label %if.then551, label %if.end558

if.then551:		; preds = %for.body545
	%tmp552 = load i8* %p		; <i8> [#uses=1]
	%conv553 = zext i8 %tmp552 to i32		; <i32> [#uses=1]
	%tmp554 = load i8* %a		; <i8> [#uses=1]
	%conv555 = zext i8 %tmp554 to i32		; <i32> [#uses=1]
	%xor556 = xor i32 %conv553, %conv555		; <i32> [#uses=1]
	%conv557 = trunc i32 %xor556 to i8		; <i8> [#uses=1]
	store i8 %conv557, i8* %p
	br label %if.end558

if.end558:		; preds = %if.then551, %for.body545
	%tmp561 = load i8* %a		; <i8> [#uses=1]
	%conv562 = zext i8 %tmp561 to i32		; <i32> [#uses=1]
	%and563 = and i32 %conv562, 128		; <i32> [#uses=1]
	%conv564 = trunc i32 %and563 to i8		; <i8> [#uses=1]
	store i8 %conv564, i8* %hiBitSet560
	%tmp565 = load i8* %a		; <i8> [#uses=1]
	%conv566 = zext i8 %tmp565 to i32		; <i32> [#uses=1]
	%shl567 = shl i32 %conv566, 1		; <i32> [#uses=1]
	%conv568 = trunc i32 %shl567 to i8		; <i8> [#uses=1]
	store i8 %conv568, i8* %a
	%tmp569 = load i8* %hiBitSet560		; <i8> [#uses=1]
	%conv570 = zext i8 %tmp569 to i32		; <i32> [#uses=1]
	%cmp571 = icmp eq i32 %conv570, 128		; <i1> [#uses=1]
	br i1 %cmp571, label %if.then573, label %if.end578

if.then573:		; preds = %if.end558
	%tmp574 = load i8* %a		; <i8> [#uses=1]
	%conv575 = zext i8 %tmp574 to i32		; <i32> [#uses=1]
	%xor576 = xor i32 %conv575, 27		; <i32> [#uses=1]
	%conv577 = trunc i32 %xor576 to i8		; <i8> [#uses=1]
	store i8 %conv577, i8* %a
	br label %if.end578

if.end578:		; preds = %if.then573, %if.end558
	%tmp579 = load i8* %b		; <i8> [#uses=1]
	%conv580 = zext i8 %tmp579 to i32		; <i32> [#uses=1]
	%shr581 = ashr i32 %conv580, 1		; <i32> [#uses=1]
	%conv582 = trunc i32 %shr581 to i8		; <i8> [#uses=1]
	store i8 %conv582, i8* %b
	br label %for.inc583

for.inc583:		; preds = %if.end578
	%tmp584 = load i32* %i540		; <i32> [#uses=1]
	%inc585 = add i32 %tmp584, 1		; <i32> [#uses=1]
	store i32 %inc585, i32* %i540
	br label %for.cond541

for.end586:		; preds = %for.cond541
	%tmp587 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp587, i8* %w1
	%tmp588 = load i8* %x		; <i8> [#uses=1]
	%conv589 = zext i8 %tmp588 to i32		; <i32> [#uses=1]
	%tmp590 = load i8* %x1		; <i8> [#uses=1]
	%conv591 = zext i8 %tmp590 to i32		; <i32> [#uses=1]
	%xor592 = xor i32 %conv589, %conv591		; <i32> [#uses=1]
	%conv593 = trunc i32 %xor592 to i8		; <i8> [#uses=1]
	store i8 %conv593, i8* %x
	%tmp594 = load i8* %y		; <i8> [#uses=1]
	%conv595 = zext i8 %tmp594 to i32		; <i32> [#uses=1]
	%tmp596 = load i8* %y1		; <i8> [#uses=1]
	%conv597 = zext i8 %tmp596 to i32		; <i32> [#uses=1]
	%xor598 = xor i32 %conv595, %conv597		; <i32> [#uses=1]
	%conv599 = trunc i32 %xor598 to i8		; <i8> [#uses=1]
	store i8 %conv599, i8* %y
	%tmp600 = load i8* %z		; <i8> [#uses=1]
	%conv601 = zext i8 %tmp600 to i32		; <i32> [#uses=1]
	%tmp602 = load i8* %z1		; <i8> [#uses=1]
	%conv603 = zext i8 %tmp602 to i32		; <i32> [#uses=1]
	%xor604 = xor i32 %conv601, %conv603		; <i32> [#uses=1]
	%conv605 = trunc i32 %xor604 to i8		; <i8> [#uses=1]
	store i8 %conv605, i8* %z
	%tmp606 = load i8* %w		; <i8> [#uses=1]
	%conv607 = zext i8 %tmp606 to i32		; <i32> [#uses=1]
	%tmp608 = load i8* %w1		; <i8> [#uses=1]
	%conv609 = zext i8 %tmp608 to i32		; <i32> [#uses=1]
	%xor610 = xor i32 %conv607, %conv609		; <i32> [#uses=1]
	%conv611 = trunc i32 %xor610 to i8		; <i8> [#uses=1]
	store i8 %conv611, i8* %w
	br label %for.inc612

for.inc612:		; preds = %for.end586
	%tmp613 = load i32* %k		; <i32> [#uses=1]
	%inc614 = add i32 %tmp613, 1		; <i32> [#uses=1]
	store i32 %inc614, i32* %k
	br label %for.cond311

for.end615:		; preds = %for.cond311
	%tmp616 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp617 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx618 = getelementptr <4 x i8> addrspace(3)* %tmp617, i32 %tmp616		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp620 = load i8* %x		; <i8> [#uses=1]
	%0 = insertelement <4 x i8> undef, i8 %tmp620, i32 0		; <<4 x i8>> [#uses=1]
	%tmp621 = load i8* %y		; <i8> [#uses=1]
	%1 = insertelement <4 x i8> %0, i8 %tmp621, i32 1		; <<4 x i8>> [#uses=1]
	%tmp622 = load i8* %z		; <i8> [#uses=1]
	%2 = insertelement <4 x i8> %1, i8 %tmp622, i32 2		; <<4 x i8>> [#uses=1]
	%tmp623 = load i8* %w		; <i8> [#uses=1]
	%3 = insertelement <4 x i8> %2, i8 %tmp623, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %3, <4 x i8>* %.compoundliteral619
	%tmp624 = load <4 x i8>* %.compoundliteral619		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp624, <4 x i8> addrspace(3)* %arrayidx618
	call void @barrier(i32 1)
	%tmp625 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp626 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx627 = getelementptr <4 x i8> addrspace(3)* %tmp626, i32 %tmp625		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp628 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp629 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx630 = getelementptr <4 x i8> addrspace(3)* %tmp629, i32 %tmp628		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp631 = load <4 x i8> addrspace(3)* %arrayidx630		; <<4 x i8>> [#uses=1]
	%tmp632 = load i32* %r		; <i32> [#uses=1]
	%mul633 = mul i32 %tmp632, 4		; <i32> [#uses=1]
	%tmp634 = load i32* %localIndex		; <i32> [#uses=1]
	%add635 = add i32 %mul633, %tmp634		; <i32> [#uses=1]
	%tmp636 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx637 = getelementptr <4 x i8> addrspace(1)* %tmp636, i32 %add635		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp638 = load <4 x i8> addrspace(1)* %arrayidx637		; <<4 x i8>> [#uses=1]
	%xor639 = xor <4 x i8> %tmp631, %tmp638		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor639, <4 x i8> addrspace(3)* %arrayidx627
	br label %for.inc640

for.inc640:		; preds = %for.end615
	%tmp641 = load i32* %r		; <i32> [#uses=1]
	%inc642 = add i32 %tmp641, 1		; <i32> [#uses=1]
	store i32 %inc642, i32* %r
	br label %for.cond

for.end643:		; preds = %for.cond
	%tmp644 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp645 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx646 = getelementptr <4 x i8> addrspace(3)* %tmp645, i32 %tmp644		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp647 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%tmp648 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp649 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx650 = getelementptr <4 x i8> addrspace(3)* %tmp649, i32 %tmp648		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp651 = load <4 x i8> addrspace(3)* %arrayidx650		; <<4 x i8>> [#uses=1]
	%call652 = call <4 x i8> @sbox(i8 addrspace(1)* %tmp647, <4 x i8> %tmp651)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call652, <4 x i8> addrspace(3)* %arrayidx646
	%tmp653 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp654 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx655 = getelementptr <4 x i8> addrspace(3)* %tmp654, i32 %tmp653		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp656 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp657 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx658 = getelementptr <4 x i8> addrspace(3)* %tmp657, i32 %tmp656		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp659 = load <4 x i8> addrspace(3)* %arrayidx658		; <<4 x i8>> [#uses=1]
	%tmp660 = load i32* %localIndex		; <i32> [#uses=1]
	%call661 = call <4 x i8> @shiftRows(<4 x i8> %tmp659, i32 %tmp660)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call661, <4 x i8> addrspace(3)* %arrayidx655
	%tmp662 = load i32* %globalIndex		; <i32> [#uses=1]
	%tmp663 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx664 = getelementptr <4 x i8> addrspace(1)* %tmp663, i32 %tmp662		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp665 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp666 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx667 = getelementptr <4 x i8> addrspace(3)* %tmp666, i32 %tmp665		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp668 = load <4 x i8> addrspace(3)* %arrayidx667		; <<4 x i8>> [#uses=1]
	%tmp669 = load i32* %rounds.addr		; <i32> [#uses=1]
	%mul670 = mul i32 %tmp669, 4		; <i32> [#uses=1]
	%tmp671 = load i32* %localIndex		; <i32> [#uses=1]
	%add672 = add i32 %mul670, %tmp671		; <i32> [#uses=1]
	%tmp673 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx674 = getelementptr <4 x i8> addrspace(1)* %tmp673, i32 %add672		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp675 = load <4 x i8> addrspace(1)* %arrayidx674		; <<4 x i8>> [#uses=1]
	%xor676 = xor <4 x i8> %tmp668, %tmp675		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor676, <4 x i8> addrspace(1)* %arrayidx664
	ret void
}

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

define available_externally <4 x i8> @sbox(i8 addrspace(1)* %SBox, <4 x i8> %block) nounwind {
entry:
	%retval = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%SBox.addr = alloca i8 addrspace(1)*		; <i8 addrspace(1)**> [#uses=5]
	%block.addr = alloca <4 x i8>		; <<4 x i8>*> [#uses=5]
	%.compoundliteral = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	store i8 addrspace(1)* %SBox, i8 addrspace(1)** %SBox.addr
	store <4 x i8> %block, <4 x i8>* %block.addr
	%tmp = load <4 x i8>* %block.addr		; <<4 x i8>> [#uses=1]
	%tmp1 = extractelement <4 x i8> %tmp, i32 0		; <i8> [#uses=1]
	%tmp2 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%idxprom = zext i8 %tmp1 to i32		; <i32> [#uses=1]
	%arrayidx = getelementptr i8 addrspace(1)* %tmp2, i32 %idxprom		; <i8 addrspace(1)*> [#uses=1]
	%tmp3 = load i8 addrspace(1)* %arrayidx		; <i8> [#uses=1]
	%0 = insertelement <4 x i8> undef, i8 %tmp3, i32 0		; <<4 x i8>> [#uses=1]
	%tmp4 = load <4 x i8>* %block.addr		; <<4 x i8>> [#uses=1]
	%tmp5 = extractelement <4 x i8> %tmp4, i32 1		; <i8> [#uses=1]
	%tmp6 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%idxprom7 = zext i8 %tmp5 to i32		; <i32> [#uses=1]
	%arrayidx8 = getelementptr i8 addrspace(1)* %tmp6, i32 %idxprom7		; <i8 addrspace(1)*> [#uses=1]
	%tmp9 = load i8 addrspace(1)* %arrayidx8		; <i8> [#uses=1]
	%1 = insertelement <4 x i8> %0, i8 %tmp9, i32 1		; <<4 x i8>> [#uses=1]
	%tmp10 = load <4 x i8>* %block.addr		; <<4 x i8>> [#uses=1]
	%tmp11 = extractelement <4 x i8> %tmp10, i32 2		; <i8> [#uses=1]
	%tmp12 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%idxprom13 = zext i8 %tmp11 to i32		; <i32> [#uses=1]
	%arrayidx14 = getelementptr i8 addrspace(1)* %tmp12, i32 %idxprom13		; <i8 addrspace(1)*> [#uses=1]
	%tmp15 = load i8 addrspace(1)* %arrayidx14		; <i8> [#uses=1]
	%2 = insertelement <4 x i8> %1, i8 %tmp15, i32 2		; <<4 x i8>> [#uses=1]
	%tmp16 = load <4 x i8>* %block.addr		; <<4 x i8>> [#uses=1]
	%tmp17 = extractelement <4 x i8> %tmp16, i32 3		; <i8> [#uses=1]
	%tmp18 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%idxprom19 = zext i8 %tmp17 to i32		; <i32> [#uses=1]
	%arrayidx20 = getelementptr i8 addrspace(1)* %tmp18, i32 %idxprom19		; <i8 addrspace(1)*> [#uses=1]
	%tmp21 = load i8 addrspace(1)* %arrayidx20		; <i8> [#uses=1]
	%3 = insertelement <4 x i8> %2, i8 %tmp21, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %3, <4 x i8>* %.compoundliteral
	%tmp22 = load <4 x i8>* %.compoundliteral		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp22, <4 x i8>* %retval
	%4 = load <4 x i8>* %retval		; <<4 x i8>> [#uses=1]
	ret <4 x i8> %4
}

declare void @barrier(i32)

define <4 x i8> @shiftRowsInv(<4 x i8> %row, i32 %j) nounwind {
entry:
	%retval = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%row.addr = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%j.addr = alloca i32		; <i32*> [#uses=2]
	%r = alloca <4 x i8>, align 4		; <<4 x i8>*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	store <4 x i8> %row, <4 x i8>* %row.addr
	store i32 %j, i32* %j.addr
	%tmp = load <4 x i8>* %row.addr		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp, <4 x i8>* %r
	store i32 0, i32* %i
	br label %for.cond

for.cond:		; preds = %for.inc, %entry
	%tmp2 = load i32* %i		; <i32> [#uses=1]
	%tmp3 = load i32* %j.addr		; <i32> [#uses=1]
	%cmp = icmp ult i32 %tmp2, %tmp3		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%tmp4 = load <4 x i8>* %r		; <<4 x i8>> [#uses=1]
	%tmp5 = shufflevector <4 x i8> %tmp4, <4 x i8> undef, <4 x i32> <i32 3, i32 0, i32 1, i32 2>		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp5, <4 x i8>* %r
	br label %for.inc

for.inc:		; preds = %for.body
	%tmp6 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp6, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond

for.end:		; preds = %for.cond
	%tmp7 = load <4 x i8>* %r		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp7, <4 x i8>* %retval
	%0 = load <4 x i8>* %retval		; <<4 x i8>> [#uses=1]
	ret <4 x i8> %0
}

define void @AESDecrypt(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %roundKey, i8 addrspace(1)* %SBox, <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)* %block1, i32 %width, i32 %rounds, ...) nounwind {
entry:
	%output.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%input.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=2]
	%roundKey.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=4]
	%SBox.addr = alloca i8 addrspace(1)*		; <i8 addrspace(1)**> [#uses=3]
	%block0.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=14]
	%block1.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=10]
	%width.addr = alloca i32		; <i32*> [#uses=2]
	%rounds.addr = alloca i32		; <i32*> [#uses=3]
	%blockIdx = alloca i32, align 4		; <i32*> [#uses=2]
	%blockIdy = alloca i32, align 4		; <i32*> [#uses=2]
	%localIdx = alloca i32, align 4		; <i32*> [#uses=1]
	%localIdy = alloca i32, align 4		; <i32*> [#uses=3]
	%globalIndex = alloca i32, align 4		; <i32*> [#uses=3]
	%localIndex = alloca i32, align 4		; <i32*> [#uses=28]
	%galiosCoeff = alloca [4 x <4 x i8>], align 4		; <[4 x <4 x i8>]*> [#uses=12]
	%.compoundliteral = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral15 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral19 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%.compoundliteral23 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	%r = alloca i32, align 4		; <i32*> [#uses=5]
	%p = alloca i8, align 1		; <i8*> [#uses=33]
	%a = alloca i8, align 1		; <i8*> [#uses=57]
	%b = alloca i8, align 1		; <i8*> [#uses=33]
	%bw = alloca i32, align 4		; <i32*> [#uses=17]
	%x = alloca i8, align 1		; <i8*> [#uses=4]
	%y = alloca i8, align 1		; <i8*> [#uses=4]
	%z = alloca i8, align 1		; <i8*> [#uses=4]
	%w = alloca i8, align 1		; <i8*> [#uses=4]
	%i = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet = alloca i8, align 1		; <i8*> [#uses=2]
	%i153 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet173 = alloca i8, align 1		; <i8*> [#uses=2]
	%i217 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet237 = alloca i8, align 1		; <i8*> [#uses=2]
	%i281 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet301 = alloca i8, align 1		; <i8*> [#uses=2]
	%k = alloca i32, align 4		; <i32*> [#uses=12]
	%x1 = alloca i8, align 1		; <i8*> [#uses=2]
	%y1 = alloca i8, align 1		; <i8*> [#uses=2]
	%z1 = alloca i8, align 1		; <i8*> [#uses=2]
	%w1 = alloca i8, align 1		; <i8*> [#uses=2]
	%i358 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet378 = alloca i8, align 1		; <i8*> [#uses=2]
	%i425 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet445 = alloca i8, align 1		; <i8*> [#uses=2]
	%i492 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet512 = alloca i8, align 1		; <i8*> [#uses=2]
	%i559 = alloca i32, align 4		; <i32*> [#uses=4]
	%hiBitSet579 = alloca i8, align 1		; <i8*> [#uses=2]
	%.compoundliteral638 = alloca <4 x i8>		; <<4 x i8>*> [#uses=2]
	store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
	store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
	store <4 x i8> addrspace(1)* %roundKey, <4 x i8> addrspace(1)** %roundKey.addr
	store i8 addrspace(1)* %SBox, i8 addrspace(1)** %SBox.addr
	store <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)** %block0.addr
	store <4 x i8> addrspace(3)* %block1, <4 x i8> addrspace(3)** %block1.addr
	store i32 %width, i32* %width.addr
	store i32 %rounds, i32* %rounds.addr
	%call = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %blockIdx
	%call1 = call i32 @get_group_id(i32 1)		; <i32> [#uses=1]
	store i32 %call1, i32* %blockIdy
	%call2 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	store i32 %call2, i32* %localIdx
	%call3 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	store i32 %call3, i32* %localIdy
	%tmp = load i32* %blockIdy		; <i32> [#uses=1]
	%tmp4 = load i32* %width.addr		; <i32> [#uses=1]
	%mul = mul i32 %tmp, %tmp4		; <i32> [#uses=1]
	%div = udiv i32 %mul, 4		; <i32> [#uses=1]
	%tmp5 = load i32* %blockIdx		; <i32> [#uses=1]
	%add = add i32 %div, %tmp5		; <i32> [#uses=1]
	%mul6 = mul i32 %add, 4		; <i32> [#uses=1]
	%tmp7 = load i32* %localIdy		; <i32> [#uses=1]
	%add8 = add i32 %mul6, %tmp7		; <i32> [#uses=1]
	store i32 %add8, i32* %globalIndex
	%tmp10 = load i32* %localIdy		; <i32> [#uses=1]
	store i32 %tmp10, i32* %localIndex
	%arraydecay = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx = getelementptr <4 x i8>* %arraydecay, i32 0		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 14, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral
	%tmp12 = load <4 x i8>* %.compoundliteral		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp12, <4 x i8>* %arrayidx
	%arraydecay13 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx14 = getelementptr <4 x i8>* %arraydecay13, i32 1		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 11, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral15
	%tmp16 = load <4 x i8>* %.compoundliteral15		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp16, <4 x i8>* %arrayidx14
	%arraydecay17 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx18 = getelementptr <4 x i8>* %arraydecay17, i32 2		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 13, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral19
	%tmp20 = load <4 x i8>* %.compoundliteral19		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp20, <4 x i8>* %arrayidx18
	%arraydecay21 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx22 = getelementptr <4 x i8>* %arraydecay21, i32 3		; <<4 x i8>*> [#uses=1]
	store <4 x i8> <i8 9, i8 0, i8 0, i8 0>, <4 x i8>* %.compoundliteral23
	%tmp24 = load <4 x i8>* %.compoundliteral23		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp24, <4 x i8>* %arrayidx22
	%tmp25 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp26 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx27 = getelementptr <4 x i8> addrspace(3)* %tmp26, i32 %tmp25		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp28 = load i32* %globalIndex		; <i32> [#uses=1]
	%tmp29 = load <4 x i8> addrspace(1)** %input.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx30 = getelementptr <4 x i8> addrspace(1)* %tmp29, i32 %tmp28		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp31 = load <4 x i8> addrspace(1)* %arrayidx30		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp31, <4 x i8> addrspace(3)* %arrayidx27
	%tmp32 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp33 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx34 = getelementptr <4 x i8> addrspace(3)* %tmp33, i32 %tmp32		; <<4 x i8> addrspace(3)*> [#uses=2]
	%tmp35 = load <4 x i8> addrspace(3)* %arrayidx34		; <<4 x i8>> [#uses=1]
	%tmp36 = load i32* %rounds.addr		; <i32> [#uses=1]
	%mul37 = mul i32 4, %tmp36		; <i32> [#uses=1]
	%tmp38 = load i32* %localIndex		; <i32> [#uses=1]
	%add39 = add i32 %mul37, %tmp38		; <i32> [#uses=1]
	%tmp40 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx41 = getelementptr <4 x i8> addrspace(1)* %tmp40, i32 %add39		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp42 = load <4 x i8> addrspace(1)* %arrayidx41		; <<4 x i8>> [#uses=1]
	%xor = xor <4 x i8> %tmp35, %tmp42		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor, <4 x i8> addrspace(3)* %arrayidx34
	%tmp44 = load i32* %rounds.addr		; <i32> [#uses=1]
	%sub = sub i32 %tmp44, 1		; <i32> [#uses=1]
	store i32 %sub, i32* %r
	br label %for.cond

for.cond:		; preds = %for.inc644, %entry
	%tmp45 = load i32* %r		; <i32> [#uses=1]
	%cmp = icmp ugt i32 %tmp45, 0		; <i1> [#uses=1]
	br i1 %cmp, label %for.body, label %for.end646

for.body:		; preds = %for.cond
	%tmp46 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp47 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx48 = getelementptr <4 x i8> addrspace(3)* %tmp47, i32 %tmp46		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp49 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp50 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx51 = getelementptr <4 x i8> addrspace(3)* %tmp50, i32 %tmp49		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp52 = load <4 x i8> addrspace(3)* %arrayidx51		; <<4 x i8>> [#uses=1]
	%tmp53 = load i32* %localIndex		; <i32> [#uses=1]
	%call54 = call <4 x i8> @shiftRowsInv(<4 x i8> %tmp52, i32 %tmp53)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call54, <4 x i8> addrspace(3)* %arrayidx48
	%tmp55 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp56 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx57 = getelementptr <4 x i8> addrspace(3)* %tmp56, i32 %tmp55		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp58 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%tmp59 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp60 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx61 = getelementptr <4 x i8> addrspace(3)* %tmp60, i32 %tmp59		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp62 = load <4 x i8> addrspace(3)* %arrayidx61		; <<4 x i8>> [#uses=1]
	%call63 = call <4 x i8> @sbox(i8 addrspace(1)* %tmp58, <4 x i8> %tmp62)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call63, <4 x i8> addrspace(3)* %arrayidx57
	call void @barrier(i32 1)
	%tmp64 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp65 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx66 = getelementptr <4 x i8> addrspace(3)* %tmp65, i32 %tmp64		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp67 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp68 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx69 = getelementptr <4 x i8> addrspace(3)* %tmp68, i32 %tmp67		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp70 = load <4 x i8> addrspace(3)* %arrayidx69		; <<4 x i8>> [#uses=1]
	%tmp71 = load i32* %r		; <i32> [#uses=1]
	%mul72 = mul i32 %tmp71, 4		; <i32> [#uses=1]
	%tmp73 = load i32* %localIndex		; <i32> [#uses=1]
	%add74 = add i32 %mul72, %tmp73		; <i32> [#uses=1]
	%tmp75 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx76 = getelementptr <4 x i8> addrspace(1)* %tmp75, i32 %add74		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp77 = load <4 x i8> addrspace(1)* %arrayidx76		; <<4 x i8>> [#uses=1]
	%xor78 = xor <4 x i8> %tmp70, %tmp77		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor78, <4 x i8> addrspace(3)* %arrayidx66
	call void @barrier(i32 1)
	store i8 0, i8* %p
	store i8 0, i8* %a
	store i8 0, i8* %b
	store i32 4, i32* %bw
	store i8 0, i8* %p
	%tmp87 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx88 = getelementptr <4 x i8> addrspace(3)* %tmp87, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp89 = load <4 x i8> addrspace(3)* %arrayidx88		; <<4 x i8>> [#uses=1]
	%tmp90 = extractelement <4 x i8> %tmp89, i32 0		; <i8> [#uses=1]
	store i8 %tmp90, i8* %a
	%tmp91 = load i32* %bw		; <i32> [#uses=1]
	%tmp92 = load i32* %localIndex		; <i32> [#uses=1]
	%sub93 = sub i32 %tmp91, %tmp92		; <i32> [#uses=1]
	%tmp94 = load i32* %bw		; <i32> [#uses=2]
	%cmp95 = icmp ne i32 %tmp94, 0		; <i1> [#uses=1]
	%nonzero = select i1 %cmp95, i32 %tmp94, i32 1		; <i32> [#uses=1]
	%rem = urem i32 %sub93, %nonzero		; <i32> [#uses=1]
	%arraydecay96 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx97 = getelementptr <4 x i8>* %arraydecay96, i32 %rem		; <<4 x i8>*> [#uses=1]
	%tmp98 = load <4 x i8>* %arrayidx97		; <<4 x i8>> [#uses=1]
	%tmp99 = extractelement <4 x i8> %tmp98, i32 0		; <i8> [#uses=1]
	store i8 %tmp99, i8* %b
	store i32 0, i32* %i
	br label %for.cond101

for.cond101:		; preds = %for.inc, %for.body
	%tmp102 = load i32* %i		; <i32> [#uses=1]
	%cmp103 = icmp ult i32 %tmp102, 8		; <i1> [#uses=1]
	br i1 %cmp103, label %for.body104, label %for.end

for.body104:		; preds = %for.cond101
	%tmp105 = load i8* %b		; <i8> [#uses=1]
	%conv = zext i8 %tmp105 to i32		; <i32> [#uses=1]
	%and = and i32 %conv, 1		; <i32> [#uses=1]
	%cmp106 = icmp eq i32 %and, 1		; <i1> [#uses=1]
	br i1 %cmp106, label %if.then, label %if.end

if.then:		; preds = %for.body104
	%tmp108 = load i8* %p		; <i8> [#uses=1]
	%conv109 = zext i8 %tmp108 to i32		; <i32> [#uses=1]
	%tmp110 = load i8* %a		; <i8> [#uses=1]
	%conv111 = zext i8 %tmp110 to i32		; <i32> [#uses=1]
	%xor112 = xor i32 %conv109, %conv111		; <i32> [#uses=1]
	%conv113 = trunc i32 %xor112 to i8		; <i8> [#uses=1]
	store i8 %conv113, i8* %p
	br label %if.end

if.end:		; preds = %if.then, %for.body104
	%tmp115 = load i8* %a		; <i8> [#uses=1]
	%conv116 = zext i8 %tmp115 to i32		; <i32> [#uses=1]
	%and117 = and i32 %conv116, 128		; <i32> [#uses=1]
	%conv118 = trunc i32 %and117 to i8		; <i8> [#uses=1]
	store i8 %conv118, i8* %hiBitSet
	%tmp119 = load i8* %a		; <i8> [#uses=1]
	%conv120 = zext i8 %tmp119 to i32		; <i32> [#uses=1]
	%shl = shl i32 %conv120, 1		; <i32> [#uses=1]
	%conv121 = trunc i32 %shl to i8		; <i8> [#uses=1]
	store i8 %conv121, i8* %a
	%tmp122 = load i8* %hiBitSet		; <i8> [#uses=1]
	%conv123 = zext i8 %tmp122 to i32		; <i32> [#uses=1]
	%cmp124 = icmp eq i32 %conv123, 128		; <i1> [#uses=1]
	br i1 %cmp124, label %if.then126, label %if.end131

if.then126:		; preds = %if.end
	%tmp127 = load i8* %a		; <i8> [#uses=1]
	%conv128 = zext i8 %tmp127 to i32		; <i32> [#uses=1]
	%xor129 = xor i32 %conv128, 27		; <i32> [#uses=1]
	%conv130 = trunc i32 %xor129 to i8		; <i8> [#uses=1]
	store i8 %conv130, i8* %a
	br label %if.end131

if.end131:		; preds = %if.then126, %if.end
	%tmp132 = load i8* %b		; <i8> [#uses=1]
	%conv133 = zext i8 %tmp132 to i32		; <i32> [#uses=1]
	%shr = ashr i32 %conv133, 1		; <i32> [#uses=1]
	%conv134 = trunc i32 %shr to i8		; <i8> [#uses=1]
	store i8 %conv134, i8* %b
	br label %for.inc

for.inc:		; preds = %if.end131
	%tmp135 = load i32* %i		; <i32> [#uses=1]
	%inc = add i32 %tmp135, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %i
	br label %for.cond101

for.end:		; preds = %for.cond101
	%tmp136 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp136, i8* %x
	store i8 0, i8* %p
	%tmp137 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx138 = getelementptr <4 x i8> addrspace(3)* %tmp137, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp139 = load <4 x i8> addrspace(3)* %arrayidx138		; <<4 x i8>> [#uses=1]
	%tmp140 = extractelement <4 x i8> %tmp139, i32 1		; <i8> [#uses=1]
	store i8 %tmp140, i8* %a
	%tmp141 = load i32* %bw		; <i32> [#uses=1]
	%tmp142 = load i32* %localIndex		; <i32> [#uses=1]
	%sub143 = sub i32 %tmp141, %tmp142		; <i32> [#uses=1]
	%tmp144 = load i32* %bw		; <i32> [#uses=2]
	%cmp145 = icmp ne i32 %tmp144, 0		; <i1> [#uses=1]
	%nonzero146 = select i1 %cmp145, i32 %tmp144, i32 1		; <i32> [#uses=1]
	%rem147 = urem i32 %sub143, %nonzero146		; <i32> [#uses=1]
	%arraydecay148 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx149 = getelementptr <4 x i8>* %arraydecay148, i32 %rem147		; <<4 x i8>*> [#uses=1]
	%tmp150 = load <4 x i8>* %arrayidx149		; <<4 x i8>> [#uses=1]
	%tmp151 = extractelement <4 x i8> %tmp150, i32 0		; <i8> [#uses=1]
	store i8 %tmp151, i8* %b
	store i32 0, i32* %i153
	br label %for.cond154

for.cond154:		; preds = %for.inc196, %for.end
	%tmp155 = load i32* %i153		; <i32> [#uses=1]
	%cmp156 = icmp ult i32 %tmp155, 8		; <i1> [#uses=1]
	br i1 %cmp156, label %for.body158, label %for.end199

for.body158:		; preds = %for.cond154
	%tmp159 = load i8* %b		; <i8> [#uses=1]
	%conv160 = zext i8 %tmp159 to i32		; <i32> [#uses=1]
	%and161 = and i32 %conv160, 1		; <i32> [#uses=1]
	%cmp162 = icmp eq i32 %and161, 1		; <i1> [#uses=1]
	br i1 %cmp162, label %if.then164, label %if.end171

if.then164:		; preds = %for.body158
	%tmp165 = load i8* %p		; <i8> [#uses=1]
	%conv166 = zext i8 %tmp165 to i32		; <i32> [#uses=1]
	%tmp167 = load i8* %a		; <i8> [#uses=1]
	%conv168 = zext i8 %tmp167 to i32		; <i32> [#uses=1]
	%xor169 = xor i32 %conv166, %conv168		; <i32> [#uses=1]
	%conv170 = trunc i32 %xor169 to i8		; <i8> [#uses=1]
	store i8 %conv170, i8* %p
	br label %if.end171

if.end171:		; preds = %if.then164, %for.body158
	%tmp174 = load i8* %a		; <i8> [#uses=1]
	%conv175 = zext i8 %tmp174 to i32		; <i32> [#uses=1]
	%and176 = and i32 %conv175, 128		; <i32> [#uses=1]
	%conv177 = trunc i32 %and176 to i8		; <i8> [#uses=1]
	store i8 %conv177, i8* %hiBitSet173
	%tmp178 = load i8* %a		; <i8> [#uses=1]
	%conv179 = zext i8 %tmp178 to i32		; <i32> [#uses=1]
	%shl180 = shl i32 %conv179, 1		; <i32> [#uses=1]
	%conv181 = trunc i32 %shl180 to i8		; <i8> [#uses=1]
	store i8 %conv181, i8* %a
	%tmp182 = load i8* %hiBitSet173		; <i8> [#uses=1]
	%conv183 = zext i8 %tmp182 to i32		; <i32> [#uses=1]
	%cmp184 = icmp eq i32 %conv183, 128		; <i1> [#uses=1]
	br i1 %cmp184, label %if.then186, label %if.end191

if.then186:		; preds = %if.end171
	%tmp187 = load i8* %a		; <i8> [#uses=1]
	%conv188 = zext i8 %tmp187 to i32		; <i32> [#uses=1]
	%xor189 = xor i32 %conv188, 27		; <i32> [#uses=1]
	%conv190 = trunc i32 %xor189 to i8		; <i8> [#uses=1]
	store i8 %conv190, i8* %a
	br label %if.end191

if.end191:		; preds = %if.then186, %if.end171
	%tmp192 = load i8* %b		; <i8> [#uses=1]
	%conv193 = zext i8 %tmp192 to i32		; <i32> [#uses=1]
	%shr194 = ashr i32 %conv193, 1		; <i32> [#uses=1]
	%conv195 = trunc i32 %shr194 to i8		; <i8> [#uses=1]
	store i8 %conv195, i8* %b
	br label %for.inc196

for.inc196:		; preds = %if.end191
	%tmp197 = load i32* %i153		; <i32> [#uses=1]
	%inc198 = add i32 %tmp197, 1		; <i32> [#uses=1]
	store i32 %inc198, i32* %i153
	br label %for.cond154

for.end199:		; preds = %for.cond154
	%tmp200 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp200, i8* %y
	store i8 0, i8* %p
	%tmp201 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx202 = getelementptr <4 x i8> addrspace(3)* %tmp201, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp203 = load <4 x i8> addrspace(3)* %arrayidx202		; <<4 x i8>> [#uses=1]
	%tmp204 = extractelement <4 x i8> %tmp203, i32 2		; <i8> [#uses=1]
	store i8 %tmp204, i8* %a
	%tmp205 = load i32* %bw		; <i32> [#uses=1]
	%tmp206 = load i32* %localIndex		; <i32> [#uses=1]
	%sub207 = sub i32 %tmp205, %tmp206		; <i32> [#uses=1]
	%tmp208 = load i32* %bw		; <i32> [#uses=2]
	%cmp209 = icmp ne i32 %tmp208, 0		; <i1> [#uses=1]
	%nonzero210 = select i1 %cmp209, i32 %tmp208, i32 1		; <i32> [#uses=1]
	%rem211 = urem i32 %sub207, %nonzero210		; <i32> [#uses=1]
	%arraydecay212 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx213 = getelementptr <4 x i8>* %arraydecay212, i32 %rem211		; <<4 x i8>*> [#uses=1]
	%tmp214 = load <4 x i8>* %arrayidx213		; <<4 x i8>> [#uses=1]
	%tmp215 = extractelement <4 x i8> %tmp214, i32 0		; <i8> [#uses=1]
	store i8 %tmp215, i8* %b
	store i32 0, i32* %i217
	br label %for.cond218

for.cond218:		; preds = %for.inc260, %for.end199
	%tmp219 = load i32* %i217		; <i32> [#uses=1]
	%cmp220 = icmp ult i32 %tmp219, 8		; <i1> [#uses=1]
	br i1 %cmp220, label %for.body222, label %for.end263

for.body222:		; preds = %for.cond218
	%tmp223 = load i8* %b		; <i8> [#uses=1]
	%conv224 = zext i8 %tmp223 to i32		; <i32> [#uses=1]
	%and225 = and i32 %conv224, 1		; <i32> [#uses=1]
	%cmp226 = icmp eq i32 %and225, 1		; <i1> [#uses=1]
	br i1 %cmp226, label %if.then228, label %if.end235

if.then228:		; preds = %for.body222
	%tmp229 = load i8* %p		; <i8> [#uses=1]
	%conv230 = zext i8 %tmp229 to i32		; <i32> [#uses=1]
	%tmp231 = load i8* %a		; <i8> [#uses=1]
	%conv232 = zext i8 %tmp231 to i32		; <i32> [#uses=1]
	%xor233 = xor i32 %conv230, %conv232		; <i32> [#uses=1]
	%conv234 = trunc i32 %xor233 to i8		; <i8> [#uses=1]
	store i8 %conv234, i8* %p
	br label %if.end235

if.end235:		; preds = %if.then228, %for.body222
	%tmp238 = load i8* %a		; <i8> [#uses=1]
	%conv239 = zext i8 %tmp238 to i32		; <i32> [#uses=1]
	%and240 = and i32 %conv239, 128		; <i32> [#uses=1]
	%conv241 = trunc i32 %and240 to i8		; <i8> [#uses=1]
	store i8 %conv241, i8* %hiBitSet237
	%tmp242 = load i8* %a		; <i8> [#uses=1]
	%conv243 = zext i8 %tmp242 to i32		; <i32> [#uses=1]
	%shl244 = shl i32 %conv243, 1		; <i32> [#uses=1]
	%conv245 = trunc i32 %shl244 to i8		; <i8> [#uses=1]
	store i8 %conv245, i8* %a
	%tmp246 = load i8* %hiBitSet237		; <i8> [#uses=1]
	%conv247 = zext i8 %tmp246 to i32		; <i32> [#uses=1]
	%cmp248 = icmp eq i32 %conv247, 128		; <i1> [#uses=1]
	br i1 %cmp248, label %if.then250, label %if.end255

if.then250:		; preds = %if.end235
	%tmp251 = load i8* %a		; <i8> [#uses=1]
	%conv252 = zext i8 %tmp251 to i32		; <i32> [#uses=1]
	%xor253 = xor i32 %conv252, 27		; <i32> [#uses=1]
	%conv254 = trunc i32 %xor253 to i8		; <i8> [#uses=1]
	store i8 %conv254, i8* %a
	br label %if.end255

if.end255:		; preds = %if.then250, %if.end235
	%tmp256 = load i8* %b		; <i8> [#uses=1]
	%conv257 = zext i8 %tmp256 to i32		; <i32> [#uses=1]
	%shr258 = ashr i32 %conv257, 1		; <i32> [#uses=1]
	%conv259 = trunc i32 %shr258 to i8		; <i8> [#uses=1]
	store i8 %conv259, i8* %b
	br label %for.inc260

for.inc260:		; preds = %if.end255
	%tmp261 = load i32* %i217		; <i32> [#uses=1]
	%inc262 = add i32 %tmp261, 1		; <i32> [#uses=1]
	store i32 %inc262, i32* %i217
	br label %for.cond218

for.end263:		; preds = %for.cond218
	%tmp264 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp264, i8* %z
	store i8 0, i8* %p
	%tmp265 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx266 = getelementptr <4 x i8> addrspace(3)* %tmp265, i32 0		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp267 = load <4 x i8> addrspace(3)* %arrayidx266		; <<4 x i8>> [#uses=1]
	%tmp268 = extractelement <4 x i8> %tmp267, i32 3		; <i8> [#uses=1]
	store i8 %tmp268, i8* %a
	%tmp269 = load i32* %bw		; <i32> [#uses=1]
	%tmp270 = load i32* %localIndex		; <i32> [#uses=1]
	%sub271 = sub i32 %tmp269, %tmp270		; <i32> [#uses=1]
	%tmp272 = load i32* %bw		; <i32> [#uses=2]
	%cmp273 = icmp ne i32 %tmp272, 0		; <i1> [#uses=1]
	%nonzero274 = select i1 %cmp273, i32 %tmp272, i32 1		; <i32> [#uses=1]
	%rem275 = urem i32 %sub271, %nonzero274		; <i32> [#uses=1]
	%arraydecay276 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx277 = getelementptr <4 x i8>* %arraydecay276, i32 %rem275		; <<4 x i8>*> [#uses=1]
	%tmp278 = load <4 x i8>* %arrayidx277		; <<4 x i8>> [#uses=1]
	%tmp279 = extractelement <4 x i8> %tmp278, i32 0		; <i8> [#uses=1]
	store i8 %tmp279, i8* %b
	store i32 0, i32* %i281
	br label %for.cond282

for.cond282:		; preds = %for.inc324, %for.end263
	%tmp283 = load i32* %i281		; <i32> [#uses=1]
	%cmp284 = icmp ult i32 %tmp283, 8		; <i1> [#uses=1]
	br i1 %cmp284, label %for.body286, label %for.end327

for.body286:		; preds = %for.cond282
	%tmp287 = load i8* %b		; <i8> [#uses=1]
	%conv288 = zext i8 %tmp287 to i32		; <i32> [#uses=1]
	%and289 = and i32 %conv288, 1		; <i32> [#uses=1]
	%cmp290 = icmp eq i32 %and289, 1		; <i1> [#uses=1]
	br i1 %cmp290, label %if.then292, label %if.end299

if.then292:		; preds = %for.body286
	%tmp293 = load i8* %p		; <i8> [#uses=1]
	%conv294 = zext i8 %tmp293 to i32		; <i32> [#uses=1]
	%tmp295 = load i8* %a		; <i8> [#uses=1]
	%conv296 = zext i8 %tmp295 to i32		; <i32> [#uses=1]
	%xor297 = xor i32 %conv294, %conv296		; <i32> [#uses=1]
	%conv298 = trunc i32 %xor297 to i8		; <i8> [#uses=1]
	store i8 %conv298, i8* %p
	br label %if.end299

if.end299:		; preds = %if.then292, %for.body286
	%tmp302 = load i8* %a		; <i8> [#uses=1]
	%conv303 = zext i8 %tmp302 to i32		; <i32> [#uses=1]
	%and304 = and i32 %conv303, 128		; <i32> [#uses=1]
	%conv305 = trunc i32 %and304 to i8		; <i8> [#uses=1]
	store i8 %conv305, i8* %hiBitSet301
	%tmp306 = load i8* %a		; <i8> [#uses=1]
	%conv307 = zext i8 %tmp306 to i32		; <i32> [#uses=1]
	%shl308 = shl i32 %conv307, 1		; <i32> [#uses=1]
	%conv309 = trunc i32 %shl308 to i8		; <i8> [#uses=1]
	store i8 %conv309, i8* %a
	%tmp310 = load i8* %hiBitSet301		; <i8> [#uses=1]
	%conv311 = zext i8 %tmp310 to i32		; <i32> [#uses=1]
	%cmp312 = icmp eq i32 %conv311, 128		; <i1> [#uses=1]
	br i1 %cmp312, label %if.then314, label %if.end319

if.then314:		; preds = %if.end299
	%tmp315 = load i8* %a		; <i8> [#uses=1]
	%conv316 = zext i8 %tmp315 to i32		; <i32> [#uses=1]
	%xor317 = xor i32 %conv316, 27		; <i32> [#uses=1]
	%conv318 = trunc i32 %xor317 to i8		; <i8> [#uses=1]
	store i8 %conv318, i8* %a
	br label %if.end319

if.end319:		; preds = %if.then314, %if.end299
	%tmp320 = load i8* %b		; <i8> [#uses=1]
	%conv321 = zext i8 %tmp320 to i32		; <i32> [#uses=1]
	%shr322 = ashr i32 %conv321, 1		; <i32> [#uses=1]
	%conv323 = trunc i32 %shr322 to i8		; <i8> [#uses=1]
	store i8 %conv323, i8* %b
	br label %for.inc324

for.inc324:		; preds = %if.end319
	%tmp325 = load i32* %i281		; <i32> [#uses=1]
	%inc326 = add i32 %tmp325, 1		; <i32> [#uses=1]
	store i32 %inc326, i32* %i281
	br label %for.cond282

for.end327:		; preds = %for.cond282
	%tmp328 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp328, i8* %w
	store i32 1, i32* %k
	br label %for.cond330

for.cond330:		; preds = %for.inc631, %for.end327
	%tmp331 = load i32* %k		; <i32> [#uses=1]
	%cmp332 = icmp ult i32 %tmp331, 4		; <i1> [#uses=1]
	br i1 %cmp332, label %for.body334, label %for.end634

for.body334:		; preds = %for.cond330
	store i8 0, i8* %p
	%tmp339 = load i32* %k		; <i32> [#uses=1]
	%tmp340 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx341 = getelementptr <4 x i8> addrspace(3)* %tmp340, i32 %tmp339		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp342 = load <4 x i8> addrspace(3)* %arrayidx341		; <<4 x i8>> [#uses=1]
	%tmp343 = extractelement <4 x i8> %tmp342, i32 0		; <i8> [#uses=1]
	store i8 %tmp343, i8* %a
	%tmp344 = load i32* %k		; <i32> [#uses=1]
	%tmp345 = load i32* %bw		; <i32> [#uses=1]
	%add346 = add i32 %tmp344, %tmp345		; <i32> [#uses=1]
	%tmp347 = load i32* %localIndex		; <i32> [#uses=1]
	%sub348 = sub i32 %add346, %tmp347		; <i32> [#uses=1]
	%tmp349 = load i32* %bw		; <i32> [#uses=2]
	%cmp350 = icmp ne i32 %tmp349, 0		; <i1> [#uses=1]
	%nonzero351 = select i1 %cmp350, i32 %tmp349, i32 1		; <i32> [#uses=1]
	%rem352 = urem i32 %sub348, %nonzero351		; <i32> [#uses=1]
	%arraydecay353 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx354 = getelementptr <4 x i8>* %arraydecay353, i32 %rem352		; <<4 x i8>*> [#uses=1]
	%tmp355 = load <4 x i8>* %arrayidx354		; <<4 x i8>> [#uses=1]
	%tmp356 = extractelement <4 x i8> %tmp355, i32 0		; <i8> [#uses=1]
	store i8 %tmp356, i8* %b
	store i32 0, i32* %i358
	br label %for.cond359

for.cond359:		; preds = %for.inc401, %for.body334
	%tmp360 = load i32* %i358		; <i32> [#uses=1]
	%cmp361 = icmp ult i32 %tmp360, 8		; <i1> [#uses=1]
	br i1 %cmp361, label %for.body363, label %for.end404

for.body363:		; preds = %for.cond359
	%tmp364 = load i8* %b		; <i8> [#uses=1]
	%conv365 = zext i8 %tmp364 to i32		; <i32> [#uses=1]
	%and366 = and i32 %conv365, 1		; <i32> [#uses=1]
	%cmp367 = icmp eq i32 %and366, 1		; <i1> [#uses=1]
	br i1 %cmp367, label %if.then369, label %if.end376

if.then369:		; preds = %for.body363
	%tmp370 = load i8* %p		; <i8> [#uses=1]
	%conv371 = zext i8 %tmp370 to i32		; <i32> [#uses=1]
	%tmp372 = load i8* %a		; <i8> [#uses=1]
	%conv373 = zext i8 %tmp372 to i32		; <i32> [#uses=1]
	%xor374 = xor i32 %conv371, %conv373		; <i32> [#uses=1]
	%conv375 = trunc i32 %xor374 to i8		; <i8> [#uses=1]
	store i8 %conv375, i8* %p
	br label %if.end376

if.end376:		; preds = %if.then369, %for.body363
	%tmp379 = load i8* %a		; <i8> [#uses=1]
	%conv380 = zext i8 %tmp379 to i32		; <i32> [#uses=1]
	%and381 = and i32 %conv380, 128		; <i32> [#uses=1]
	%conv382 = trunc i32 %and381 to i8		; <i8> [#uses=1]
	store i8 %conv382, i8* %hiBitSet378
	%tmp383 = load i8* %a		; <i8> [#uses=1]
	%conv384 = zext i8 %tmp383 to i32		; <i32> [#uses=1]
	%shl385 = shl i32 %conv384, 1		; <i32> [#uses=1]
	%conv386 = trunc i32 %shl385 to i8		; <i8> [#uses=1]
	store i8 %conv386, i8* %a
	%tmp387 = load i8* %hiBitSet378		; <i8> [#uses=1]
	%conv388 = zext i8 %tmp387 to i32		; <i32> [#uses=1]
	%cmp389 = icmp eq i32 %conv388, 128		; <i1> [#uses=1]
	br i1 %cmp389, label %if.then391, label %if.end396

if.then391:		; preds = %if.end376
	%tmp392 = load i8* %a		; <i8> [#uses=1]
	%conv393 = zext i8 %tmp392 to i32		; <i32> [#uses=1]
	%xor394 = xor i32 %conv393, 27		; <i32> [#uses=1]
	%conv395 = trunc i32 %xor394 to i8		; <i8> [#uses=1]
	store i8 %conv395, i8* %a
	br label %if.end396

if.end396:		; preds = %if.then391, %if.end376
	%tmp397 = load i8* %b		; <i8> [#uses=1]
	%conv398 = zext i8 %tmp397 to i32		; <i32> [#uses=1]
	%shr399 = ashr i32 %conv398, 1		; <i32> [#uses=1]
	%conv400 = trunc i32 %shr399 to i8		; <i8> [#uses=1]
	store i8 %conv400, i8* %b
	br label %for.inc401

for.inc401:		; preds = %if.end396
	%tmp402 = load i32* %i358		; <i32> [#uses=1]
	%inc403 = add i32 %tmp402, 1		; <i32> [#uses=1]
	store i32 %inc403, i32* %i358
	br label %for.cond359

for.end404:		; preds = %for.cond359
	%tmp405 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp405, i8* %x1
	store i8 0, i8* %p
	%tmp406 = load i32* %k		; <i32> [#uses=1]
	%tmp407 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx408 = getelementptr <4 x i8> addrspace(3)* %tmp407, i32 %tmp406		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp409 = load <4 x i8> addrspace(3)* %arrayidx408		; <<4 x i8>> [#uses=1]
	%tmp410 = extractelement <4 x i8> %tmp409, i32 1		; <i8> [#uses=1]
	store i8 %tmp410, i8* %a
	%tmp411 = load i32* %k		; <i32> [#uses=1]
	%tmp412 = load i32* %bw		; <i32> [#uses=1]
	%add413 = add i32 %tmp411, %tmp412		; <i32> [#uses=1]
	%tmp414 = load i32* %localIndex		; <i32> [#uses=1]
	%sub415 = sub i32 %add413, %tmp414		; <i32> [#uses=1]
	%tmp416 = load i32* %bw		; <i32> [#uses=2]
	%cmp417 = icmp ne i32 %tmp416, 0		; <i1> [#uses=1]
	%nonzero418 = select i1 %cmp417, i32 %tmp416, i32 1		; <i32> [#uses=1]
	%rem419 = urem i32 %sub415, %nonzero418		; <i32> [#uses=1]
	%arraydecay420 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx421 = getelementptr <4 x i8>* %arraydecay420, i32 %rem419		; <<4 x i8>*> [#uses=1]
	%tmp422 = load <4 x i8>* %arrayidx421		; <<4 x i8>> [#uses=1]
	%tmp423 = extractelement <4 x i8> %tmp422, i32 0		; <i8> [#uses=1]
	store i8 %tmp423, i8* %b
	store i32 0, i32* %i425
	br label %for.cond426

for.cond426:		; preds = %for.inc468, %for.end404
	%tmp427 = load i32* %i425		; <i32> [#uses=1]
	%cmp428 = icmp ult i32 %tmp427, 8		; <i1> [#uses=1]
	br i1 %cmp428, label %for.body430, label %for.end471

for.body430:		; preds = %for.cond426
	%tmp431 = load i8* %b		; <i8> [#uses=1]
	%conv432 = zext i8 %tmp431 to i32		; <i32> [#uses=1]
	%and433 = and i32 %conv432, 1		; <i32> [#uses=1]
	%cmp434 = icmp eq i32 %and433, 1		; <i1> [#uses=1]
	br i1 %cmp434, label %if.then436, label %if.end443

if.then436:		; preds = %for.body430
	%tmp437 = load i8* %p		; <i8> [#uses=1]
	%conv438 = zext i8 %tmp437 to i32		; <i32> [#uses=1]
	%tmp439 = load i8* %a		; <i8> [#uses=1]
	%conv440 = zext i8 %tmp439 to i32		; <i32> [#uses=1]
	%xor441 = xor i32 %conv438, %conv440		; <i32> [#uses=1]
	%conv442 = trunc i32 %xor441 to i8		; <i8> [#uses=1]
	store i8 %conv442, i8* %p
	br label %if.end443

if.end443:		; preds = %if.then436, %for.body430
	%tmp446 = load i8* %a		; <i8> [#uses=1]
	%conv447 = zext i8 %tmp446 to i32		; <i32> [#uses=1]
	%and448 = and i32 %conv447, 128		; <i32> [#uses=1]
	%conv449 = trunc i32 %and448 to i8		; <i8> [#uses=1]
	store i8 %conv449, i8* %hiBitSet445
	%tmp450 = load i8* %a		; <i8> [#uses=1]
	%conv451 = zext i8 %tmp450 to i32		; <i32> [#uses=1]
	%shl452 = shl i32 %conv451, 1		; <i32> [#uses=1]
	%conv453 = trunc i32 %shl452 to i8		; <i8> [#uses=1]
	store i8 %conv453, i8* %a
	%tmp454 = load i8* %hiBitSet445		; <i8> [#uses=1]
	%conv455 = zext i8 %tmp454 to i32		; <i32> [#uses=1]
	%cmp456 = icmp eq i32 %conv455, 128		; <i1> [#uses=1]
	br i1 %cmp456, label %if.then458, label %if.end463

if.then458:		; preds = %if.end443
	%tmp459 = load i8* %a		; <i8> [#uses=1]
	%conv460 = zext i8 %tmp459 to i32		; <i32> [#uses=1]
	%xor461 = xor i32 %conv460, 27		; <i32> [#uses=1]
	%conv462 = trunc i32 %xor461 to i8		; <i8> [#uses=1]
	store i8 %conv462, i8* %a
	br label %if.end463

if.end463:		; preds = %if.then458, %if.end443
	%tmp464 = load i8* %b		; <i8> [#uses=1]
	%conv465 = zext i8 %tmp464 to i32		; <i32> [#uses=1]
	%shr466 = ashr i32 %conv465, 1		; <i32> [#uses=1]
	%conv467 = trunc i32 %shr466 to i8		; <i8> [#uses=1]
	store i8 %conv467, i8* %b
	br label %for.inc468

for.inc468:		; preds = %if.end463
	%tmp469 = load i32* %i425		; <i32> [#uses=1]
	%inc470 = add i32 %tmp469, 1		; <i32> [#uses=1]
	store i32 %inc470, i32* %i425
	br label %for.cond426

for.end471:		; preds = %for.cond426
	%tmp472 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp472, i8* %y1
	store i8 0, i8* %p
	%tmp473 = load i32* %k		; <i32> [#uses=1]
	%tmp474 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx475 = getelementptr <4 x i8> addrspace(3)* %tmp474, i32 %tmp473		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp476 = load <4 x i8> addrspace(3)* %arrayidx475		; <<4 x i8>> [#uses=1]
	%tmp477 = extractelement <4 x i8> %tmp476, i32 2		; <i8> [#uses=1]
	store i8 %tmp477, i8* %a
	%tmp478 = load i32* %k		; <i32> [#uses=1]
	%tmp479 = load i32* %bw		; <i32> [#uses=1]
	%add480 = add i32 %tmp478, %tmp479		; <i32> [#uses=1]
	%tmp481 = load i32* %localIndex		; <i32> [#uses=1]
	%sub482 = sub i32 %add480, %tmp481		; <i32> [#uses=1]
	%tmp483 = load i32* %bw		; <i32> [#uses=2]
	%cmp484 = icmp ne i32 %tmp483, 0		; <i1> [#uses=1]
	%nonzero485 = select i1 %cmp484, i32 %tmp483, i32 1		; <i32> [#uses=1]
	%rem486 = urem i32 %sub482, %nonzero485		; <i32> [#uses=1]
	%arraydecay487 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx488 = getelementptr <4 x i8>* %arraydecay487, i32 %rem486		; <<4 x i8>*> [#uses=1]
	%tmp489 = load <4 x i8>* %arrayidx488		; <<4 x i8>> [#uses=1]
	%tmp490 = extractelement <4 x i8> %tmp489, i32 0		; <i8> [#uses=1]
	store i8 %tmp490, i8* %b
	store i32 0, i32* %i492
	br label %for.cond493

for.cond493:		; preds = %for.inc535, %for.end471
	%tmp494 = load i32* %i492		; <i32> [#uses=1]
	%cmp495 = icmp ult i32 %tmp494, 8		; <i1> [#uses=1]
	br i1 %cmp495, label %for.body497, label %for.end538

for.body497:		; preds = %for.cond493
	%tmp498 = load i8* %b		; <i8> [#uses=1]
	%conv499 = zext i8 %tmp498 to i32		; <i32> [#uses=1]
	%and500 = and i32 %conv499, 1		; <i32> [#uses=1]
	%cmp501 = icmp eq i32 %and500, 1		; <i1> [#uses=1]
	br i1 %cmp501, label %if.then503, label %if.end510

if.then503:		; preds = %for.body497
	%tmp504 = load i8* %p		; <i8> [#uses=1]
	%conv505 = zext i8 %tmp504 to i32		; <i32> [#uses=1]
	%tmp506 = load i8* %a		; <i8> [#uses=1]
	%conv507 = zext i8 %tmp506 to i32		; <i32> [#uses=1]
	%xor508 = xor i32 %conv505, %conv507		; <i32> [#uses=1]
	%conv509 = trunc i32 %xor508 to i8		; <i8> [#uses=1]
	store i8 %conv509, i8* %p
	br label %if.end510

if.end510:		; preds = %if.then503, %for.body497
	%tmp513 = load i8* %a		; <i8> [#uses=1]
	%conv514 = zext i8 %tmp513 to i32		; <i32> [#uses=1]
	%and515 = and i32 %conv514, 128		; <i32> [#uses=1]
	%conv516 = trunc i32 %and515 to i8		; <i8> [#uses=1]
	store i8 %conv516, i8* %hiBitSet512
	%tmp517 = load i8* %a		; <i8> [#uses=1]
	%conv518 = zext i8 %tmp517 to i32		; <i32> [#uses=1]
	%shl519 = shl i32 %conv518, 1		; <i32> [#uses=1]
	%conv520 = trunc i32 %shl519 to i8		; <i8> [#uses=1]
	store i8 %conv520, i8* %a
	%tmp521 = load i8* %hiBitSet512		; <i8> [#uses=1]
	%conv522 = zext i8 %tmp521 to i32		; <i32> [#uses=1]
	%cmp523 = icmp eq i32 %conv522, 128		; <i1> [#uses=1]
	br i1 %cmp523, label %if.then525, label %if.end530

if.then525:		; preds = %if.end510
	%tmp526 = load i8* %a		; <i8> [#uses=1]
	%conv527 = zext i8 %tmp526 to i32		; <i32> [#uses=1]
	%xor528 = xor i32 %conv527, 27		; <i32> [#uses=1]
	%conv529 = trunc i32 %xor528 to i8		; <i8> [#uses=1]
	store i8 %conv529, i8* %a
	br label %if.end530

if.end530:		; preds = %if.then525, %if.end510
	%tmp531 = load i8* %b		; <i8> [#uses=1]
	%conv532 = zext i8 %tmp531 to i32		; <i32> [#uses=1]
	%shr533 = ashr i32 %conv532, 1		; <i32> [#uses=1]
	%conv534 = trunc i32 %shr533 to i8		; <i8> [#uses=1]
	store i8 %conv534, i8* %b
	br label %for.inc535

for.inc535:		; preds = %if.end530
	%tmp536 = load i32* %i492		; <i32> [#uses=1]
	%inc537 = add i32 %tmp536, 1		; <i32> [#uses=1]
	store i32 %inc537, i32* %i492
	br label %for.cond493

for.end538:		; preds = %for.cond493
	%tmp539 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp539, i8* %z1
	store i8 0, i8* %p
	%tmp540 = load i32* %k		; <i32> [#uses=1]
	%tmp541 = load <4 x i8> addrspace(3)** %block1.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx542 = getelementptr <4 x i8> addrspace(3)* %tmp541, i32 %tmp540		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp543 = load <4 x i8> addrspace(3)* %arrayidx542		; <<4 x i8>> [#uses=1]
	%tmp544 = extractelement <4 x i8> %tmp543, i32 3		; <i8> [#uses=1]
	store i8 %tmp544, i8* %a
	%tmp545 = load i32* %k		; <i32> [#uses=1]
	%tmp546 = load i32* %bw		; <i32> [#uses=1]
	%add547 = add i32 %tmp545, %tmp546		; <i32> [#uses=1]
	%tmp548 = load i32* %localIndex		; <i32> [#uses=1]
	%sub549 = sub i32 %add547, %tmp548		; <i32> [#uses=1]
	%tmp550 = load i32* %bw		; <i32> [#uses=2]
	%cmp551 = icmp ne i32 %tmp550, 0		; <i1> [#uses=1]
	%nonzero552 = select i1 %cmp551, i32 %tmp550, i32 1		; <i32> [#uses=1]
	%rem553 = urem i32 %sub549, %nonzero552		; <i32> [#uses=1]
	%arraydecay554 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0		; <<4 x i8>*> [#uses=1]
	%arrayidx555 = getelementptr <4 x i8>* %arraydecay554, i32 %rem553		; <<4 x i8>*> [#uses=1]
	%tmp556 = load <4 x i8>* %arrayidx555		; <<4 x i8>> [#uses=1]
	%tmp557 = extractelement <4 x i8> %tmp556, i32 0		; <i8> [#uses=1]
	store i8 %tmp557, i8* %b
	store i32 0, i32* %i559
	br label %for.cond560

for.cond560:		; preds = %for.inc602, %for.end538
	%tmp561 = load i32* %i559		; <i32> [#uses=1]
	%cmp562 = icmp ult i32 %tmp561, 8		; <i1> [#uses=1]
	br i1 %cmp562, label %for.body564, label %for.end605

for.body564:		; preds = %for.cond560
	%tmp565 = load i8* %b		; <i8> [#uses=1]
	%conv566 = zext i8 %tmp565 to i32		; <i32> [#uses=1]
	%and567 = and i32 %conv566, 1		; <i32> [#uses=1]
	%cmp568 = icmp eq i32 %and567, 1		; <i1> [#uses=1]
	br i1 %cmp568, label %if.then570, label %if.end577

if.then570:		; preds = %for.body564
	%tmp571 = load i8* %p		; <i8> [#uses=1]
	%conv572 = zext i8 %tmp571 to i32		; <i32> [#uses=1]
	%tmp573 = load i8* %a		; <i8> [#uses=1]
	%conv574 = zext i8 %tmp573 to i32		; <i32> [#uses=1]
	%xor575 = xor i32 %conv572, %conv574		; <i32> [#uses=1]
	%conv576 = trunc i32 %xor575 to i8		; <i8> [#uses=1]
	store i8 %conv576, i8* %p
	br label %if.end577

if.end577:		; preds = %if.then570, %for.body564
	%tmp580 = load i8* %a		; <i8> [#uses=1]
	%conv581 = zext i8 %tmp580 to i32		; <i32> [#uses=1]
	%and582 = and i32 %conv581, 128		; <i32> [#uses=1]
	%conv583 = trunc i32 %and582 to i8		; <i8> [#uses=1]
	store i8 %conv583, i8* %hiBitSet579
	%tmp584 = load i8* %a		; <i8> [#uses=1]
	%conv585 = zext i8 %tmp584 to i32		; <i32> [#uses=1]
	%shl586 = shl i32 %conv585, 1		; <i32> [#uses=1]
	%conv587 = trunc i32 %shl586 to i8		; <i8> [#uses=1]
	store i8 %conv587, i8* %a
	%tmp588 = load i8* %hiBitSet579		; <i8> [#uses=1]
	%conv589 = zext i8 %tmp588 to i32		; <i32> [#uses=1]
	%cmp590 = icmp eq i32 %conv589, 128		; <i1> [#uses=1]
	br i1 %cmp590, label %if.then592, label %if.end597

if.then592:		; preds = %if.end577
	%tmp593 = load i8* %a		; <i8> [#uses=1]
	%conv594 = zext i8 %tmp593 to i32		; <i32> [#uses=1]
	%xor595 = xor i32 %conv594, 27		; <i32> [#uses=1]
	%conv596 = trunc i32 %xor595 to i8		; <i8> [#uses=1]
	store i8 %conv596, i8* %a
	br label %if.end597

if.end597:		; preds = %if.then592, %if.end577
	%tmp598 = load i8* %b		; <i8> [#uses=1]
	%conv599 = zext i8 %tmp598 to i32		; <i32> [#uses=1]
	%shr600 = ashr i32 %conv599, 1		; <i32> [#uses=1]
	%conv601 = trunc i32 %shr600 to i8		; <i8> [#uses=1]
	store i8 %conv601, i8* %b
	br label %for.inc602

for.inc602:		; preds = %if.end597
	%tmp603 = load i32* %i559		; <i32> [#uses=1]
	%inc604 = add i32 %tmp603, 1		; <i32> [#uses=1]
	store i32 %inc604, i32* %i559
	br label %for.cond560

for.end605:		; preds = %for.cond560
	%tmp606 = load i8* %p		; <i8> [#uses=1]
	store i8 %tmp606, i8* %w1
	%tmp607 = load i8* %x		; <i8> [#uses=1]
	%conv608 = zext i8 %tmp607 to i32		; <i32> [#uses=1]
	%tmp609 = load i8* %x1		; <i8> [#uses=1]
	%conv610 = zext i8 %tmp609 to i32		; <i32> [#uses=1]
	%xor611 = xor i32 %conv608, %conv610		; <i32> [#uses=1]
	%conv612 = trunc i32 %xor611 to i8		; <i8> [#uses=1]
	store i8 %conv612, i8* %x
	%tmp613 = load i8* %y		; <i8> [#uses=1]
	%conv614 = zext i8 %tmp613 to i32		; <i32> [#uses=1]
	%tmp615 = load i8* %y1		; <i8> [#uses=1]
	%conv616 = zext i8 %tmp615 to i32		; <i32> [#uses=1]
	%xor617 = xor i32 %conv614, %conv616		; <i32> [#uses=1]
	%conv618 = trunc i32 %xor617 to i8		; <i8> [#uses=1]
	store i8 %conv618, i8* %y
	%tmp619 = load i8* %z		; <i8> [#uses=1]
	%conv620 = zext i8 %tmp619 to i32		; <i32> [#uses=1]
	%tmp621 = load i8* %z1		; <i8> [#uses=1]
	%conv622 = zext i8 %tmp621 to i32		; <i32> [#uses=1]
	%xor623 = xor i32 %conv620, %conv622		; <i32> [#uses=1]
	%conv624 = trunc i32 %xor623 to i8		; <i8> [#uses=1]
	store i8 %conv624, i8* %z
	%tmp625 = load i8* %w		; <i8> [#uses=1]
	%conv626 = zext i8 %tmp625 to i32		; <i32> [#uses=1]
	%tmp627 = load i8* %w1		; <i8> [#uses=1]
	%conv628 = zext i8 %tmp627 to i32		; <i32> [#uses=1]
	%xor629 = xor i32 %conv626, %conv628		; <i32> [#uses=1]
	%conv630 = trunc i32 %xor629 to i8		; <i8> [#uses=1]
	store i8 %conv630, i8* %w
	br label %for.inc631

for.inc631:		; preds = %for.end605
	%tmp632 = load i32* %k		; <i32> [#uses=1]
	%inc633 = add i32 %tmp632, 1		; <i32> [#uses=1]
	store i32 %inc633, i32* %k
	br label %for.cond330

for.end634:		; preds = %for.cond330
	%tmp635 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp636 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx637 = getelementptr <4 x i8> addrspace(3)* %tmp636, i32 %tmp635		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp639 = load i8* %x		; <i8> [#uses=1]
	%0 = insertelement <4 x i8> undef, i8 %tmp639, i32 0		; <<4 x i8>> [#uses=1]
	%tmp640 = load i8* %y		; <i8> [#uses=1]
	%1 = insertelement <4 x i8> %0, i8 %tmp640, i32 1		; <<4 x i8>> [#uses=1]
	%tmp641 = load i8* %z		; <i8> [#uses=1]
	%2 = insertelement <4 x i8> %1, i8 %tmp641, i32 2		; <<4 x i8>> [#uses=1]
	%tmp642 = load i8* %w		; <i8> [#uses=1]
	%3 = insertelement <4 x i8> %2, i8 %tmp642, i32 3		; <<4 x i8>> [#uses=1]
	store <4 x i8> %3, <4 x i8>* %.compoundliteral638
	%tmp643 = load <4 x i8>* %.compoundliteral638		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp643, <4 x i8> addrspace(3)* %arrayidx637
	br label %for.inc644

for.inc644:		; preds = %for.end634
	%tmp645 = load i32* %r		; <i32> [#uses=1]
	%dec = add i32 %tmp645, -1		; <i32> [#uses=1]
	store i32 %dec, i32* %r
	br label %for.cond

for.end646:		; preds = %for.cond
	%tmp647 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp648 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx649 = getelementptr <4 x i8> addrspace(3)* %tmp648, i32 %tmp647		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp650 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp651 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx652 = getelementptr <4 x i8> addrspace(3)* %tmp651, i32 %tmp650		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp653 = load <4 x i8> addrspace(3)* %arrayidx652		; <<4 x i8>> [#uses=1]
	%tmp654 = load i32* %localIndex		; <i32> [#uses=1]
	%call655 = call <4 x i8> @shiftRowsInv(<4 x i8> %tmp653, i32 %tmp654)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call655, <4 x i8> addrspace(3)* %arrayidx649
	%tmp656 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp657 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx658 = getelementptr <4 x i8> addrspace(3)* %tmp657, i32 %tmp656		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp659 = load i8 addrspace(1)** %SBox.addr		; <i8 addrspace(1)*> [#uses=1]
	%tmp660 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp661 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx662 = getelementptr <4 x i8> addrspace(3)* %tmp661, i32 %tmp660		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp663 = load <4 x i8> addrspace(3)* %arrayidx662		; <<4 x i8>> [#uses=1]
	%call664 = call <4 x i8> @sbox(i8 addrspace(1)* %tmp659, <4 x i8> %tmp663)		; <<4 x i8>> [#uses=1]
	store <4 x i8> %call664, <4 x i8> addrspace(3)* %arrayidx658
	%tmp665 = load i32* %globalIndex		; <i32> [#uses=1]
	%tmp666 = load <4 x i8> addrspace(1)** %output.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx667 = getelementptr <4 x i8> addrspace(1)* %tmp666, i32 %tmp665		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp668 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp669 = load <4 x i8> addrspace(3)** %block0.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx670 = getelementptr <4 x i8> addrspace(3)* %tmp669, i32 %tmp668		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp671 = load <4 x i8> addrspace(3)* %arrayidx670		; <<4 x i8>> [#uses=1]
	%tmp672 = load i32* %localIndex		; <i32> [#uses=1]
	%tmp673 = load <4 x i8> addrspace(1)** %roundKey.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx674 = getelementptr <4 x i8> addrspace(1)* %tmp673, i32 %tmp672		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp675 = load <4 x i8> addrspace(1)* %arrayidx674		; <<4 x i8>> [#uses=1]
	%xor676 = xor <4 x i8> %tmp671, %tmp675		; <<4 x i8>> [#uses=1]
	store <4 x i8> %xor676, <4 x i8> addrspace(1)* %arrayidx667
	ret void
}
