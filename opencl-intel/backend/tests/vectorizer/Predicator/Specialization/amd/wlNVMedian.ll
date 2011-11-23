; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlNVMedian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"
	type { i8*, i8*, i8*, i8*, i32 }		; type %0
@sgv = internal constant [7 x i8] c"229000\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, <4 x i8> addrspace(3)*, i32, i32, i32, ...)* @ckMedian to i8*), i8* getelementptr ([7 x i8]* @sgv, i32 0, i32 0), i8* getelementptr ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"		; <[1 x %0]*> [#uses=0]

; CHECK: @ckMedian
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}

define void @ckMedian(<4 x i8> addrspace(1)* %uc4Source, i32 addrspace(1)* %uiDest, <4 x i8> addrspace(3)* %uc4LocalData, i32 %iLocalPixPitch, i32 %uiImageWidth, i32 %uiDevImageHeight, ...) nounwind {
entry:
	%uc4Source.addr = alloca <4 x i8> addrspace(1)*		; <<4 x i8> addrspace(1)**> [#uses=7]
	%uiDest.addr = alloca i32 addrspace(1)*		; <i32 addrspace(1)**> [#uses=2]
	%uc4LocalData.addr = alloca <4 x i8> addrspace(3)*		; <<4 x i8> addrspace(3)**> [#uses=40]
	%iLocalPixPitch.addr = alloca i32		; <i32*> [#uses=10]
	%uiImageWidth.addr = alloca i32		; <i32*> [#uses=6]
	%uiDevImageHeight.addr = alloca i32		; <i32*> [#uses=8]
	%iImagePosX = alloca i32, align 4		; <i32*> [#uses=5]
	%iDevYPrime = alloca i32, align 4		; <i32*> [#uses=16]
	%iDevGMEMOffset = alloca i32, align 4		; <i32*> [#uses=4]
	%iLocalPixOffset = alloca i32, align 4		; <i32*> [#uses=59]
	%fMedianEstimate = alloca [3 x float], align 4		; <[3 x float]*> [#uses=42]
	%fMinBound = alloca [3 x float], align 4		; <[3 x float]*> [#uses=9]
	%fMaxBound = alloca [3 x float], align 4		; <[3 x float]*> [#uses=9]
	%iSearch = alloca i32, align 4		; <i32*> [#uses=4]
	%uiHighCount = alloca [3 x i32], align 4		; <[3 x i32]*> [#uses=33]
	%uiPackedPix = alloca i32, align 4		; <i32*> [#uses=6]
	store <4 x i8> addrspace(1)* %uc4Source, <4 x i8> addrspace(1)** %uc4Source.addr
	store i32 addrspace(1)* %uiDest, i32 addrspace(1)** %uiDest.addr
	store <4 x i8> addrspace(3)* %uc4LocalData, <4 x i8> addrspace(3)** %uc4LocalData.addr
	store i32 %iLocalPixPitch, i32* %iLocalPixPitch.addr
	store i32 %uiImageWidth, i32* %uiImageWidth.addr
	store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
	%call = call i32 @get_global_id(i32 0)		; <i32> [#uses=1]
	store i32 %call, i32* %iImagePosX
	%call1 = call i32 @get_global_id(i32 1)		; <i32> [#uses=1]
	%sub = sub i32 %call1, 1		; <i32> [#uses=1]
	store i32 %sub, i32* %iDevYPrime
	%tmp = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call2 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call3 = call i32 @__mul24_1i32(i32 %tmp, i32 %call2)		; <i32> [#uses=1]
	%tmp4 = load i32* %iImagePosX		; <i32> [#uses=1]
	%add = add i32 %call3, %tmp4		; <i32> [#uses=1]
	store i32 %add, i32* %iDevGMEMOffset
	%call6 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%tmp7 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call8 = call i32 @__mul24_1i32(i32 %call6, i32 %tmp7)		; <i32> [#uses=1]
	%call9 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	%add10 = add i32 %call8, %call9		; <i32> [#uses=1]
	%add11 = add i32 %add10, 1		; <i32> [#uses=1]
	store i32 %add11, i32* %iLocalPixOffset
	%tmp12 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%cmp = icmp sge i32 %tmp12, 0		; <i1> [#uses=1]
	br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:		; preds = %entry
	%tmp13 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%tmp14 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp15 = icmp slt i32 %tmp13, %tmp14		; <i1> [#uses=1]
	br i1 %cmp15, label %land.lhs.true16, label %if.else

land.lhs.true16:		; preds = %land.lhs.true
	%tmp17 = load i32* %iImagePosX		; <i32> [#uses=1]
	%tmp18 = load i32* %uiImageWidth.addr		; <i32> [#uses=1]
	%cmp19 = icmp slt i32 %tmp17, %tmp18		; <i1> [#uses=1]
	br i1 %cmp19, label %if.then, label %if.else

if.then:		; preds = %land.lhs.true16
	%tmp20 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp21 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx = getelementptr <4 x i8> addrspace(3)* %tmp21, i32 %tmp20		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp22 = load i32* %iDevGMEMOffset		; <i32> [#uses=1]
	%tmp23 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx24 = getelementptr <4 x i8> addrspace(1)* %tmp23, i32 %tmp22		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp25 = load <4 x i8> addrspace(1)* %arrayidx24		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp25, <4 x i8> addrspace(3)* %arrayidx
	br label %if.end

if.else:		; preds = %land.lhs.true16, %land.lhs.true, %entry
	%tmp26 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp27 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx28 = getelementptr <4 x i8> addrspace(3)* %tmp27, i32 %tmp26		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx28
	br label %if.end

if.end:		; preds = %if.else, %if.then
	%call29 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%cmp30 = icmp ult i32 %call29, 2		; <i1> [#uses=1]
	br i1 %cmp30, label %if.then31, label %if.end63

if.then31:		; preds = %if.end
	%tmp32 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%call33 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%tmp34 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call35 = call i32 @__mul24_1i32(i32 %call33, i32 %tmp34)		; <i32> [#uses=1]
	%add36 = add i32 %tmp32, %call35		; <i32> [#uses=1]
	store i32 %add36, i32* %iLocalPixOffset
	%tmp37 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call38 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%add39 = add i32 %tmp37, %call38		; <i32> [#uses=1]
	%tmp40 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp41 = icmp ult i32 %add39, %tmp40		; <i1> [#uses=1]
	br i1 %cmp41, label %land.lhs.true42, label %if.else58

land.lhs.true42:		; preds = %if.then31
	%tmp43 = load i32* %iImagePosX		; <i32> [#uses=1]
	%tmp44 = load i32* %uiImageWidth.addr		; <i32> [#uses=1]
	%cmp45 = icmp slt i32 %tmp43, %tmp44		; <i1> [#uses=1]
	br i1 %cmp45, label %if.then46, label %if.else58

if.then46:		; preds = %land.lhs.true42
	%tmp47 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp48 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx49 = getelementptr <4 x i8> addrspace(3)* %tmp48, i32 %tmp47		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp50 = load i32* %iDevGMEMOffset		; <i32> [#uses=1]
	%call51 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%call52 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call53 = call i32 @__mul24_1u32(i32 %call51, i32 %call52)		; <i32> [#uses=1]
	%add54 = add i32 %tmp50, %call53		; <i32> [#uses=1]
	%tmp55 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx56 = getelementptr <4 x i8> addrspace(1)* %tmp55, i32 %add54		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp57 = load <4 x i8> addrspace(1)* %arrayidx56		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp57, <4 x i8> addrspace(3)* %arrayidx49
	br label %if.end62

if.else58:		; preds = %land.lhs.true42, %if.then31
	%tmp59 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp60 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx61 = getelementptr <4 x i8> addrspace(3)* %tmp60, i32 %tmp59		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx61
	br label %if.end62

if.end62:		; preds = %if.else58, %if.then46
	br label %if.end63

if.end63:		; preds = %if.end62, %if.end
	%call64 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	%call65 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%sub66 = sub i32 %call65, 1		; <i32> [#uses=1]
	%cmp67 = icmp eq i32 %call64, %sub66		; <i1> [#uses=1]
	br i1 %cmp67, label %if.then68, label %if.else140

if.then68:		; preds = %if.end63
	%call69 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%tmp70 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call71 = call i32 @__mul24_1i32(i32 %call69, i32 %tmp70)		; <i32> [#uses=1]
	store i32 %call71, i32* %iLocalPixOffset
	%tmp72 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%cmp73 = icmp sge i32 %tmp72, 0		; <i1> [#uses=1]
	br i1 %cmp73, label %land.lhs.true74, label %if.else96

land.lhs.true74:		; preds = %if.then68
	%tmp75 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%tmp76 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp77 = icmp slt i32 %tmp75, %tmp76		; <i1> [#uses=1]
	br i1 %cmp77, label %land.lhs.true78, label %if.else96

land.lhs.true78:		; preds = %land.lhs.true74
	%call79 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%cmp80 = icmp ugt i32 %call79, 0		; <i1> [#uses=1]
	br i1 %cmp80, label %if.then81, label %if.else96

if.then81:		; preds = %land.lhs.true78
	%tmp82 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp83 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx84 = getelementptr <4 x i8> addrspace(3)* %tmp83, i32 %tmp82		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp85 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call86 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call87 = call i32 @__mul24_1i32(i32 %tmp85, i32 %call86)		; <i32> [#uses=1]
	%call88 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%call89 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call90 = call i32 @__mul24_1u32(i32 %call88, i32 %call89)		; <i32> [#uses=1]
	%add91 = add i32 %call87, %call90		; <i32> [#uses=1]
	%sub92 = sub i32 %add91, 1		; <i32> [#uses=1]
	%tmp93 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx94 = getelementptr <4 x i8> addrspace(1)* %tmp93, i32 %sub92		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp95 = load <4 x i8> addrspace(1)* %arrayidx94		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp95, <4 x i8> addrspace(3)* %arrayidx84
	br label %if.end100

if.else96:		; preds = %land.lhs.true78, %land.lhs.true74, %if.then68
	%tmp97 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp98 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx99 = getelementptr <4 x i8> addrspace(3)* %tmp98, i32 %tmp97		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx99
	br label %if.end100

if.end100:		; preds = %if.else96, %if.then81
	%call101 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%cmp102 = icmp ult i32 %call101, 2		; <i1> [#uses=1]
	br i1 %cmp102, label %if.then103, label %if.end139

if.then103:		; preds = %if.end100
	%tmp104 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%call105 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%tmp106 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call107 = call i32 @__mul24_1i32(i32 %call105, i32 %tmp106)		; <i32> [#uses=1]
	%add108 = add i32 %tmp104, %call107		; <i32> [#uses=1]
	store i32 %add108, i32* %iLocalPixOffset
	%tmp109 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call110 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%add111 = add i32 %tmp109, %call110		; <i32> [#uses=1]
	%tmp112 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp113 = icmp ult i32 %add111, %tmp112		; <i1> [#uses=1]
	br i1 %cmp113, label %land.lhs.true114, label %if.else134

land.lhs.true114:		; preds = %if.then103
	%call115 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%cmp116 = icmp ugt i32 %call115, 0		; <i1> [#uses=1]
	br i1 %cmp116, label %if.then117, label %if.else134

if.then117:		; preds = %land.lhs.true114
	%tmp118 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp119 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx120 = getelementptr <4 x i8> addrspace(3)* %tmp119, i32 %tmp118		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp121 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call122 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%add123 = add i32 %tmp121, %call122		; <i32> [#uses=1]
	%call124 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call125 = call i32 @__mul24_1i32(i32 %add123, i32 %call124)		; <i32> [#uses=1]
	%call126 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%call127 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call128 = call i32 @__mul24_1u32(i32 %call126, i32 %call127)		; <i32> [#uses=1]
	%add129 = add i32 %call125, %call128		; <i32> [#uses=1]
	%sub130 = sub i32 %add129, 1		; <i32> [#uses=1]
	%tmp131 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx132 = getelementptr <4 x i8> addrspace(1)* %tmp131, i32 %sub130		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp133 = load <4 x i8> addrspace(1)* %arrayidx132		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp133, <4 x i8> addrspace(3)* %arrayidx120
	br label %if.end138

if.else134:		; preds = %land.lhs.true114, %if.then103
	%tmp135 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp136 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx137 = getelementptr <4 x i8> addrspace(3)* %tmp136, i32 %tmp135		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx137
	br label %if.end138

if.end138:		; preds = %if.else134, %if.then117
	br label %if.end139

if.end139:		; preds = %if.end138, %if.end100
	br label %if.end226

if.else140:		; preds = %if.end63
	%call141 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	%cmp142 = icmp eq i32 %call141, 0		; <i1> [#uses=1]
	br i1 %cmp142, label %if.then143, label %if.end225

if.then143:		; preds = %if.else140
	%call144 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%add145 = add i32 %call144, 1		; <i32> [#uses=1]
	%tmp146 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call147 = call i32 @__mul24_1i32(i32 %add145, i32 %tmp146)		; <i32> [#uses=1]
	%sub148 = sub i32 %call147, 1		; <i32> [#uses=1]
	store i32 %sub148, i32* %iLocalPixOffset
	%tmp149 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%cmp150 = icmp sge i32 %tmp149, 0		; <i1> [#uses=1]
	br i1 %cmp150, label %land.lhs.true151, label %if.else177

land.lhs.true151:		; preds = %if.then143
	%tmp152 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%tmp153 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp154 = icmp slt i32 %tmp152, %tmp153		; <i1> [#uses=1]
	br i1 %cmp154, label %land.lhs.true155, label %if.else177

land.lhs.true155:		; preds = %land.lhs.true151
	%call156 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%add157 = add i32 %call156, 1		; <i32> [#uses=1]
	%call158 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call159 = call i32 @__mul24_1i32(i32 %add157, i32 %call158)		; <i32> [#uses=1]
	%tmp160 = load i32* %uiImageWidth.addr		; <i32> [#uses=1]
	%cmp161 = icmp slt i32 %call159, %tmp160		; <i1> [#uses=1]
	br i1 %cmp161, label %if.then162, label %if.else177

if.then162:		; preds = %land.lhs.true155
	%tmp163 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp164 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx165 = getelementptr <4 x i8> addrspace(3)* %tmp164, i32 %tmp163		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp166 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call167 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call168 = call i32 @__mul24_1i32(i32 %tmp166, i32 %call167)		; <i32> [#uses=1]
	%call169 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%add170 = add i32 %call169, 1		; <i32> [#uses=1]
	%call171 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call172 = call i32 @__mul24_1u32(i32 %add170, i32 %call171)		; <i32> [#uses=1]
	%add173 = add i32 %call168, %call172		; <i32> [#uses=1]
	%tmp174 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx175 = getelementptr <4 x i8> addrspace(1)* %tmp174, i32 %add173		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp176 = load <4 x i8> addrspace(1)* %arrayidx175		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp176, <4 x i8> addrspace(3)* %arrayidx165
	br label %if.end181

if.else177:		; preds = %land.lhs.true155, %land.lhs.true151, %if.then143
	%tmp178 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp179 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx180 = getelementptr <4 x i8> addrspace(3)* %tmp179, i32 %tmp178		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx180
	br label %if.end181

if.end181:		; preds = %if.else177, %if.then162
	%call182 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%cmp183 = icmp ult i32 %call182, 2		; <i1> [#uses=1]
	br i1 %cmp183, label %if.then184, label %if.end224

if.then184:		; preds = %if.end181
	%tmp185 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%call186 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%tmp187 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call188 = call i32 @__mul24_1i32(i32 %call186, i32 %tmp187)		; <i32> [#uses=1]
	%add189 = add i32 %tmp185, %call188		; <i32> [#uses=1]
	store i32 %add189, i32* %iLocalPixOffset
	%tmp190 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call191 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%add192 = add i32 %tmp190, %call191		; <i32> [#uses=1]
	%tmp193 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp194 = icmp ult i32 %add192, %tmp193		; <i1> [#uses=1]
	br i1 %cmp194, label %land.lhs.true195, label %if.else219

land.lhs.true195:		; preds = %if.then184
	%call196 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%add197 = add i32 %call196, 1		; <i32> [#uses=1]
	%call198 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call199 = call i32 @__mul24_1u32(i32 %add197, i32 %call198)		; <i32> [#uses=1]
	%tmp200 = load i32* %uiImageWidth.addr		; <i32> [#uses=1]
	%cmp201 = icmp ult i32 %call199, %tmp200		; <i1> [#uses=1]
	br i1 %cmp201, label %if.then202, label %if.else219

if.then202:		; preds = %land.lhs.true195
	%tmp203 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp204 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx205 = getelementptr <4 x i8> addrspace(3)* %tmp204, i32 %tmp203		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp206 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%call207 = call i32 @get_local_size(i32 1)		; <i32> [#uses=1]
	%add208 = add i32 %tmp206, %call207		; <i32> [#uses=1]
	%call209 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%call210 = call i32 @__mul24_1i32(i32 %add208, i32 %call209)		; <i32> [#uses=1]
	%call211 = call i32 @get_group_id(i32 0)		; <i32> [#uses=1]
	%add212 = add i32 %call211, 1		; <i32> [#uses=1]
	%call213 = call i32 @get_local_size(i32 0)		; <i32> [#uses=1]
	%call214 = call i32 @__mul24_1u32(i32 %add212, i32 %call213)		; <i32> [#uses=1]
	%add215 = add i32 %call210, %call214		; <i32> [#uses=1]
	%tmp216 = load <4 x i8> addrspace(1)** %uc4Source.addr		; <<4 x i8> addrspace(1)*> [#uses=1]
	%arrayidx217 = getelementptr <4 x i8> addrspace(1)* %tmp216, i32 %add215		; <<4 x i8> addrspace(1)*> [#uses=1]
	%tmp218 = load <4 x i8> addrspace(1)* %arrayidx217		; <<4 x i8>> [#uses=1]
	store <4 x i8> %tmp218, <4 x i8> addrspace(3)* %arrayidx205
	br label %if.end223

if.else219:		; preds = %land.lhs.true195, %if.then184
	%tmp220 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp221 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx222 = getelementptr <4 x i8> addrspace(3)* %tmp221, i32 %tmp220		; <<4 x i8> addrspace(3)*> [#uses=1]
	store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx222
	br label %if.end223

if.end223:		; preds = %if.else219, %if.then202
	br label %if.end224

if.end224:		; preds = %if.end223, %if.end181
	br label %if.end225

if.end225:		; preds = %if.end224, %if.else140
	br label %if.end226

if.end226:		; preds = %if.end225, %if.end139
	call void @barrier(i32 1)
	%.array = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	store float 1.280000e+002, float* %.array
	%.array228 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 1		; <float*> [#uses=1]
	store float 1.280000e+002, float* %.array228
	%.array229 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 2		; <float*> [#uses=1]
	store float 1.280000e+002, float* %.array229
	%.array231 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array231
	%.array232 = getelementptr [3 x float]* %fMinBound, i32 0, i32 1		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array232
	%.array233 = getelementptr [3 x float]* %fMinBound, i32 0, i32 2		; <float*> [#uses=1]
	store float 0.000000e+000, float* %.array233
	%.array235 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	store float 2.550000e+002, float* %.array235
	%.array236 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 1		; <float*> [#uses=1]
	store float 2.550000e+002, float* %.array236
	%.array237 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 2		; <float*> [#uses=1]
	store float 2.550000e+002, float* %.array237
	store i32 0, i32* %iSearch
	br label %for.cond

for.cond:		; preds = %for.inc, %if.end226
	%tmp239 = load i32* %iSearch		; <i32> [#uses=1]
	%cmp240 = icmp slt i32 %tmp239, 8		; <i1> [#uses=1]
	br i1 %cmp240, label %for.body, label %for.end

for.body:		; preds = %for.cond
	%.array242 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	store i32 0, i32* %.array242
	%.array243 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 1		; <i32*> [#uses=1]
	store i32 0, i32* %.array243
	%.array244 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 2		; <i32*> [#uses=1]
	store i32 0, i32* %.array244
	%call245 = call i32 @get_local_id(i32 1)		; <i32> [#uses=1]
	%tmp246 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%call247 = call i32 @__mul24_1i32(i32 %call245, i32 %tmp246)		; <i32> [#uses=1]
	%call248 = call i32 @get_local_id(i32 0)		; <i32> [#uses=1]
	%add249 = add i32 %call247, %call248		; <i32> [#uses=1]
	store i32 %add249, i32* %iLocalPixOffset
	%arraydecay = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx250 = getelementptr i32* %arraydecay, i32 0		; <i32*> [#uses=2]
	%tmp251 = load i32* %arrayidx250		; <i32> [#uses=1]
	%arraydecay252 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx253 = getelementptr float* %arraydecay252, i32 0		; <float*> [#uses=1]
	%tmp254 = load float* %arrayidx253		; <float> [#uses=1]
	%tmp255 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp256 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx257 = getelementptr <4 x i8> addrspace(3)* %tmp256, i32 %tmp255		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp258 = load <4 x i8> addrspace(3)* %arrayidx257		; <<4 x i8>> [#uses=1]
	%tmp259 = extractelement <4 x i8> %tmp258, i32 0		; <i8> [#uses=1]
	%conv = uitofp i8 %tmp259 to float		; <float> [#uses=1]
	%cmp260 = fcmp olt float %tmp254, %conv		; <i1> [#uses=1]
	%conv261 = zext i1 %cmp260 to i32		; <i32> [#uses=1]
	%add262 = add i32 %tmp251, %conv261		; <i32> [#uses=1]
	store i32 %add262, i32* %arrayidx250
	%arraydecay263 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx264 = getelementptr i32* %arraydecay263, i32 1		; <i32*> [#uses=2]
	%tmp265 = load i32* %arrayidx264		; <i32> [#uses=1]
	%arraydecay266 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx267 = getelementptr float* %arraydecay266, i32 1		; <float*> [#uses=1]
	%tmp268 = load float* %arrayidx267		; <float> [#uses=1]
	%tmp269 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp270 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx271 = getelementptr <4 x i8> addrspace(3)* %tmp270, i32 %tmp269		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp272 = load <4 x i8> addrspace(3)* %arrayidx271		; <<4 x i8>> [#uses=1]
	%tmp273 = extractelement <4 x i8> %tmp272, i32 1		; <i8> [#uses=1]
	%conv274 = uitofp i8 %tmp273 to float		; <float> [#uses=1]
	%cmp275 = fcmp olt float %tmp268, %conv274		; <i1> [#uses=1]
	%conv276 = zext i1 %cmp275 to i32		; <i32> [#uses=1]
	%add277 = add i32 %tmp265, %conv276		; <i32> [#uses=1]
	store i32 %add277, i32* %arrayidx264
	%arraydecay278 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx279 = getelementptr i32* %arraydecay278, i32 2		; <i32*> [#uses=2]
	%tmp280 = load i32* %arrayidx279		; <i32> [#uses=1]
	%arraydecay281 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx282 = getelementptr float* %arraydecay281, i32 2		; <float*> [#uses=1]
	%tmp283 = load float* %arrayidx282		; <float> [#uses=1]
	%tmp284 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc = add i32 %tmp284, 1		; <i32> [#uses=1]
	store i32 %inc, i32* %iLocalPixOffset
	%tmp285 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx286 = getelementptr <4 x i8> addrspace(3)* %tmp285, i32 %tmp284		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp287 = load <4 x i8> addrspace(3)* %arrayidx286		; <<4 x i8>> [#uses=1]
	%tmp288 = extractelement <4 x i8> %tmp287, i32 2		; <i8> [#uses=1]
	%conv289 = uitofp i8 %tmp288 to float		; <float> [#uses=1]
	%cmp290 = fcmp olt float %tmp283, %conv289		; <i1> [#uses=1]
	%conv291 = zext i1 %cmp290 to i32		; <i32> [#uses=1]
	%add292 = add i32 %tmp280, %conv291		; <i32> [#uses=1]
	store i32 %add292, i32* %arrayidx279
	%arraydecay293 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx294 = getelementptr i32* %arraydecay293, i32 0		; <i32*> [#uses=2]
	%tmp295 = load i32* %arrayidx294		; <i32> [#uses=1]
	%arraydecay296 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx297 = getelementptr float* %arraydecay296, i32 0		; <float*> [#uses=1]
	%tmp298 = load float* %arrayidx297		; <float> [#uses=1]
	%tmp299 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp300 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx301 = getelementptr <4 x i8> addrspace(3)* %tmp300, i32 %tmp299		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp302 = load <4 x i8> addrspace(3)* %arrayidx301		; <<4 x i8>> [#uses=1]
	%tmp303 = extractelement <4 x i8> %tmp302, i32 0		; <i8> [#uses=1]
	%conv304 = uitofp i8 %tmp303 to float		; <float> [#uses=1]
	%cmp305 = fcmp olt float %tmp298, %conv304		; <i1> [#uses=1]
	%conv306 = zext i1 %cmp305 to i32		; <i32> [#uses=1]
	%add307 = add i32 %tmp295, %conv306		; <i32> [#uses=1]
	store i32 %add307, i32* %arrayidx294
	%arraydecay308 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx309 = getelementptr i32* %arraydecay308, i32 1		; <i32*> [#uses=2]
	%tmp310 = load i32* %arrayidx309		; <i32> [#uses=1]
	%arraydecay311 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx312 = getelementptr float* %arraydecay311, i32 1		; <float*> [#uses=1]
	%tmp313 = load float* %arrayidx312		; <float> [#uses=1]
	%tmp314 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp315 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx316 = getelementptr <4 x i8> addrspace(3)* %tmp315, i32 %tmp314		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp317 = load <4 x i8> addrspace(3)* %arrayidx316		; <<4 x i8>> [#uses=1]
	%tmp318 = extractelement <4 x i8> %tmp317, i32 1		; <i8> [#uses=1]
	%conv319 = uitofp i8 %tmp318 to float		; <float> [#uses=1]
	%cmp320 = fcmp olt float %tmp313, %conv319		; <i1> [#uses=1]
	%conv321 = zext i1 %cmp320 to i32		; <i32> [#uses=1]
	%add322 = add i32 %tmp310, %conv321		; <i32> [#uses=1]
	store i32 %add322, i32* %arrayidx309
	%arraydecay323 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx324 = getelementptr i32* %arraydecay323, i32 2		; <i32*> [#uses=2]
	%tmp325 = load i32* %arrayidx324		; <i32> [#uses=1]
	%arraydecay326 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx327 = getelementptr float* %arraydecay326, i32 2		; <float*> [#uses=1]
	%tmp328 = load float* %arrayidx327		; <float> [#uses=1]
	%tmp329 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc330 = add i32 %tmp329, 1		; <i32> [#uses=1]
	store i32 %inc330, i32* %iLocalPixOffset
	%tmp331 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx332 = getelementptr <4 x i8> addrspace(3)* %tmp331, i32 %tmp329		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp333 = load <4 x i8> addrspace(3)* %arrayidx332		; <<4 x i8>> [#uses=1]
	%tmp334 = extractelement <4 x i8> %tmp333, i32 2		; <i8> [#uses=1]
	%conv335 = uitofp i8 %tmp334 to float		; <float> [#uses=1]
	%cmp336 = fcmp olt float %tmp328, %conv335		; <i1> [#uses=1]
	%conv337 = zext i1 %cmp336 to i32		; <i32> [#uses=1]
	%add338 = add i32 %tmp325, %conv337		; <i32> [#uses=1]
	store i32 %add338, i32* %arrayidx324
	%arraydecay339 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx340 = getelementptr i32* %arraydecay339, i32 0		; <i32*> [#uses=2]
	%tmp341 = load i32* %arrayidx340		; <i32> [#uses=1]
	%arraydecay342 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx343 = getelementptr float* %arraydecay342, i32 0		; <float*> [#uses=1]
	%tmp344 = load float* %arrayidx343		; <float> [#uses=1]
	%tmp345 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp346 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx347 = getelementptr <4 x i8> addrspace(3)* %tmp346, i32 %tmp345		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp348 = load <4 x i8> addrspace(3)* %arrayidx347		; <<4 x i8>> [#uses=1]
	%tmp349 = extractelement <4 x i8> %tmp348, i32 0		; <i8> [#uses=1]
	%conv350 = uitofp i8 %tmp349 to float		; <float> [#uses=1]
	%cmp351 = fcmp olt float %tmp344, %conv350		; <i1> [#uses=1]
	%conv352 = zext i1 %cmp351 to i32		; <i32> [#uses=1]
	%add353 = add i32 %tmp341, %conv352		; <i32> [#uses=1]
	store i32 %add353, i32* %arrayidx340
	%arraydecay354 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx355 = getelementptr i32* %arraydecay354, i32 1		; <i32*> [#uses=2]
	%tmp356 = load i32* %arrayidx355		; <i32> [#uses=1]
	%arraydecay357 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx358 = getelementptr float* %arraydecay357, i32 1		; <float*> [#uses=1]
	%tmp359 = load float* %arrayidx358		; <float> [#uses=1]
	%tmp360 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp361 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx362 = getelementptr <4 x i8> addrspace(3)* %tmp361, i32 %tmp360		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp363 = load <4 x i8> addrspace(3)* %arrayidx362		; <<4 x i8>> [#uses=1]
	%tmp364 = extractelement <4 x i8> %tmp363, i32 1		; <i8> [#uses=1]
	%conv365 = uitofp i8 %tmp364 to float		; <float> [#uses=1]
	%cmp366 = fcmp olt float %tmp359, %conv365		; <i1> [#uses=1]
	%conv367 = zext i1 %cmp366 to i32		; <i32> [#uses=1]
	%add368 = add i32 %tmp356, %conv367		; <i32> [#uses=1]
	store i32 %add368, i32* %arrayidx355
	%arraydecay369 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx370 = getelementptr i32* %arraydecay369, i32 2		; <i32*> [#uses=2]
	%tmp371 = load i32* %arrayidx370		; <i32> [#uses=1]
	%arraydecay372 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx373 = getelementptr float* %arraydecay372, i32 2		; <float*> [#uses=1]
	%tmp374 = load float* %arrayidx373		; <float> [#uses=1]
	%tmp375 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp376 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx377 = getelementptr <4 x i8> addrspace(3)* %tmp376, i32 %tmp375		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp378 = load <4 x i8> addrspace(3)* %arrayidx377		; <<4 x i8>> [#uses=1]
	%tmp379 = extractelement <4 x i8> %tmp378, i32 2		; <i8> [#uses=1]
	%conv380 = uitofp i8 %tmp379 to float		; <float> [#uses=1]
	%cmp381 = fcmp olt float %tmp374, %conv380		; <i1> [#uses=1]
	%conv382 = zext i1 %cmp381 to i32		; <i32> [#uses=1]
	%add383 = add i32 %tmp371, %conv382		; <i32> [#uses=1]
	store i32 %add383, i32* %arrayidx370
	%tmp384 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp385 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%sub386 = sub i32 %tmp385, 2		; <i32> [#uses=1]
	%add387 = add i32 %tmp384, %sub386		; <i32> [#uses=1]
	store i32 %add387, i32* %iLocalPixOffset
	%arraydecay388 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx389 = getelementptr i32* %arraydecay388, i32 0		; <i32*> [#uses=2]
	%tmp390 = load i32* %arrayidx389		; <i32> [#uses=1]
	%arraydecay391 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx392 = getelementptr float* %arraydecay391, i32 0		; <float*> [#uses=1]
	%tmp393 = load float* %arrayidx392		; <float> [#uses=1]
	%tmp394 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp395 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx396 = getelementptr <4 x i8> addrspace(3)* %tmp395, i32 %tmp394		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp397 = load <4 x i8> addrspace(3)* %arrayidx396		; <<4 x i8>> [#uses=1]
	%tmp398 = extractelement <4 x i8> %tmp397, i32 0		; <i8> [#uses=1]
	%conv399 = uitofp i8 %tmp398 to float		; <float> [#uses=1]
	%cmp400 = fcmp olt float %tmp393, %conv399		; <i1> [#uses=1]
	%conv401 = zext i1 %cmp400 to i32		; <i32> [#uses=1]
	%add402 = add i32 %tmp390, %conv401		; <i32> [#uses=1]
	store i32 %add402, i32* %arrayidx389
	%arraydecay403 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx404 = getelementptr i32* %arraydecay403, i32 1		; <i32*> [#uses=2]
	%tmp405 = load i32* %arrayidx404		; <i32> [#uses=1]
	%arraydecay406 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx407 = getelementptr float* %arraydecay406, i32 1		; <float*> [#uses=1]
	%tmp408 = load float* %arrayidx407		; <float> [#uses=1]
	%tmp409 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp410 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx411 = getelementptr <4 x i8> addrspace(3)* %tmp410, i32 %tmp409		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp412 = load <4 x i8> addrspace(3)* %arrayidx411		; <<4 x i8>> [#uses=1]
	%tmp413 = extractelement <4 x i8> %tmp412, i32 1		; <i8> [#uses=1]
	%conv414 = uitofp i8 %tmp413 to float		; <float> [#uses=1]
	%cmp415 = fcmp olt float %tmp408, %conv414		; <i1> [#uses=1]
	%conv416 = zext i1 %cmp415 to i32		; <i32> [#uses=1]
	%add417 = add i32 %tmp405, %conv416		; <i32> [#uses=1]
	store i32 %add417, i32* %arrayidx404
	%arraydecay418 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx419 = getelementptr i32* %arraydecay418, i32 2		; <i32*> [#uses=2]
	%tmp420 = load i32* %arrayidx419		; <i32> [#uses=1]
	%arraydecay421 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx422 = getelementptr float* %arraydecay421, i32 2		; <float*> [#uses=1]
	%tmp423 = load float* %arrayidx422		; <float> [#uses=1]
	%tmp424 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc425 = add i32 %tmp424, 1		; <i32> [#uses=1]
	store i32 %inc425, i32* %iLocalPixOffset
	%tmp426 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx427 = getelementptr <4 x i8> addrspace(3)* %tmp426, i32 %tmp424		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp428 = load <4 x i8> addrspace(3)* %arrayidx427		; <<4 x i8>> [#uses=1]
	%tmp429 = extractelement <4 x i8> %tmp428, i32 2		; <i8> [#uses=1]
	%conv430 = uitofp i8 %tmp429 to float		; <float> [#uses=1]
	%cmp431 = fcmp olt float %tmp423, %conv430		; <i1> [#uses=1]
	%conv432 = zext i1 %cmp431 to i32		; <i32> [#uses=1]
	%add433 = add i32 %tmp420, %conv432		; <i32> [#uses=1]
	store i32 %add433, i32* %arrayidx419
	%arraydecay434 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx435 = getelementptr i32* %arraydecay434, i32 0		; <i32*> [#uses=2]
	%tmp436 = load i32* %arrayidx435		; <i32> [#uses=1]
	%arraydecay437 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx438 = getelementptr float* %arraydecay437, i32 0		; <float*> [#uses=1]
	%tmp439 = load float* %arrayidx438		; <float> [#uses=1]
	%tmp440 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp441 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx442 = getelementptr <4 x i8> addrspace(3)* %tmp441, i32 %tmp440		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp443 = load <4 x i8> addrspace(3)* %arrayidx442		; <<4 x i8>> [#uses=1]
	%tmp444 = extractelement <4 x i8> %tmp443, i32 0		; <i8> [#uses=1]
	%conv445 = uitofp i8 %tmp444 to float		; <float> [#uses=1]
	%cmp446 = fcmp olt float %tmp439, %conv445		; <i1> [#uses=1]
	%conv447 = zext i1 %cmp446 to i32		; <i32> [#uses=1]
	%add448 = add i32 %tmp436, %conv447		; <i32> [#uses=1]
	store i32 %add448, i32* %arrayidx435
	%arraydecay449 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx450 = getelementptr i32* %arraydecay449, i32 1		; <i32*> [#uses=2]
	%tmp451 = load i32* %arrayidx450		; <i32> [#uses=1]
	%arraydecay452 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx453 = getelementptr float* %arraydecay452, i32 1		; <float*> [#uses=1]
	%tmp454 = load float* %arrayidx453		; <float> [#uses=1]
	%tmp455 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp456 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx457 = getelementptr <4 x i8> addrspace(3)* %tmp456, i32 %tmp455		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp458 = load <4 x i8> addrspace(3)* %arrayidx457		; <<4 x i8>> [#uses=1]
	%tmp459 = extractelement <4 x i8> %tmp458, i32 1		; <i8> [#uses=1]
	%conv460 = uitofp i8 %tmp459 to float		; <float> [#uses=1]
	%cmp461 = fcmp olt float %tmp454, %conv460		; <i1> [#uses=1]
	%conv462 = zext i1 %cmp461 to i32		; <i32> [#uses=1]
	%add463 = add i32 %tmp451, %conv462		; <i32> [#uses=1]
	store i32 %add463, i32* %arrayidx450
	%arraydecay464 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx465 = getelementptr i32* %arraydecay464, i32 2		; <i32*> [#uses=2]
	%tmp466 = load i32* %arrayidx465		; <i32> [#uses=1]
	%arraydecay467 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx468 = getelementptr float* %arraydecay467, i32 2		; <float*> [#uses=1]
	%tmp469 = load float* %arrayidx468		; <float> [#uses=1]
	%tmp470 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc471 = add i32 %tmp470, 1		; <i32> [#uses=1]
	store i32 %inc471, i32* %iLocalPixOffset
	%tmp472 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx473 = getelementptr <4 x i8> addrspace(3)* %tmp472, i32 %tmp470		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp474 = load <4 x i8> addrspace(3)* %arrayidx473		; <<4 x i8>> [#uses=1]
	%tmp475 = extractelement <4 x i8> %tmp474, i32 2		; <i8> [#uses=1]
	%conv476 = uitofp i8 %tmp475 to float		; <float> [#uses=1]
	%cmp477 = fcmp olt float %tmp469, %conv476		; <i1> [#uses=1]
	%conv478 = zext i1 %cmp477 to i32		; <i32> [#uses=1]
	%add479 = add i32 %tmp466, %conv478		; <i32> [#uses=1]
	store i32 %add479, i32* %arrayidx465
	%arraydecay480 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx481 = getelementptr i32* %arraydecay480, i32 0		; <i32*> [#uses=2]
	%tmp482 = load i32* %arrayidx481		; <i32> [#uses=1]
	%arraydecay483 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx484 = getelementptr float* %arraydecay483, i32 0		; <float*> [#uses=1]
	%tmp485 = load float* %arrayidx484		; <float> [#uses=1]
	%tmp486 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp487 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx488 = getelementptr <4 x i8> addrspace(3)* %tmp487, i32 %tmp486		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp489 = load <4 x i8> addrspace(3)* %arrayidx488		; <<4 x i8>> [#uses=1]
	%tmp490 = extractelement <4 x i8> %tmp489, i32 0		; <i8> [#uses=1]
	%conv491 = uitofp i8 %tmp490 to float		; <float> [#uses=1]
	%cmp492 = fcmp olt float %tmp485, %conv491		; <i1> [#uses=1]
	%conv493 = zext i1 %cmp492 to i32		; <i32> [#uses=1]
	%add494 = add i32 %tmp482, %conv493		; <i32> [#uses=1]
	store i32 %add494, i32* %arrayidx481
	%arraydecay495 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx496 = getelementptr i32* %arraydecay495, i32 1		; <i32*> [#uses=2]
	%tmp497 = load i32* %arrayidx496		; <i32> [#uses=1]
	%arraydecay498 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx499 = getelementptr float* %arraydecay498, i32 1		; <float*> [#uses=1]
	%tmp500 = load float* %arrayidx499		; <float> [#uses=1]
	%tmp501 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp502 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx503 = getelementptr <4 x i8> addrspace(3)* %tmp502, i32 %tmp501		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp504 = load <4 x i8> addrspace(3)* %arrayidx503		; <<4 x i8>> [#uses=1]
	%tmp505 = extractelement <4 x i8> %tmp504, i32 1		; <i8> [#uses=1]
	%conv506 = uitofp i8 %tmp505 to float		; <float> [#uses=1]
	%cmp507 = fcmp olt float %tmp500, %conv506		; <i1> [#uses=1]
	%conv508 = zext i1 %cmp507 to i32		; <i32> [#uses=1]
	%add509 = add i32 %tmp497, %conv508		; <i32> [#uses=1]
	store i32 %add509, i32* %arrayidx496
	%arraydecay510 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx511 = getelementptr i32* %arraydecay510, i32 2		; <i32*> [#uses=2]
	%tmp512 = load i32* %arrayidx511		; <i32> [#uses=1]
	%arraydecay513 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx514 = getelementptr float* %arraydecay513, i32 2		; <float*> [#uses=1]
	%tmp515 = load float* %arrayidx514		; <float> [#uses=1]
	%tmp516 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp517 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx518 = getelementptr <4 x i8> addrspace(3)* %tmp517, i32 %tmp516		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp519 = load <4 x i8> addrspace(3)* %arrayidx518		; <<4 x i8>> [#uses=1]
	%tmp520 = extractelement <4 x i8> %tmp519, i32 2		; <i8> [#uses=1]
	%conv521 = uitofp i8 %tmp520 to float		; <float> [#uses=1]
	%cmp522 = fcmp olt float %tmp515, %conv521		; <i1> [#uses=1]
	%conv523 = zext i1 %cmp522 to i32		; <i32> [#uses=1]
	%add524 = add i32 %tmp512, %conv523		; <i32> [#uses=1]
	store i32 %add524, i32* %arrayidx511
	%tmp525 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp526 = load i32* %iLocalPixPitch.addr		; <i32> [#uses=1]
	%sub527 = sub i32 %tmp526, 2		; <i32> [#uses=1]
	%add528 = add i32 %tmp525, %sub527		; <i32> [#uses=1]
	store i32 %add528, i32* %iLocalPixOffset
	%arraydecay529 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx530 = getelementptr i32* %arraydecay529, i32 0		; <i32*> [#uses=2]
	%tmp531 = load i32* %arrayidx530		; <i32> [#uses=1]
	%arraydecay532 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx533 = getelementptr float* %arraydecay532, i32 0		; <float*> [#uses=1]
	%tmp534 = load float* %arrayidx533		; <float> [#uses=1]
	%tmp535 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp536 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx537 = getelementptr <4 x i8> addrspace(3)* %tmp536, i32 %tmp535		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp538 = load <4 x i8> addrspace(3)* %arrayidx537		; <<4 x i8>> [#uses=1]
	%tmp539 = extractelement <4 x i8> %tmp538, i32 0		; <i8> [#uses=1]
	%conv540 = uitofp i8 %tmp539 to float		; <float> [#uses=1]
	%cmp541 = fcmp olt float %tmp534, %conv540		; <i1> [#uses=1]
	%conv542 = zext i1 %cmp541 to i32		; <i32> [#uses=1]
	%add543 = add i32 %tmp531, %conv542		; <i32> [#uses=1]
	store i32 %add543, i32* %arrayidx530
	%arraydecay544 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx545 = getelementptr i32* %arraydecay544, i32 1		; <i32*> [#uses=2]
	%tmp546 = load i32* %arrayidx545		; <i32> [#uses=1]
	%arraydecay547 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx548 = getelementptr float* %arraydecay547, i32 1		; <float*> [#uses=1]
	%tmp549 = load float* %arrayidx548		; <float> [#uses=1]
	%tmp550 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp551 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx552 = getelementptr <4 x i8> addrspace(3)* %tmp551, i32 %tmp550		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp553 = load <4 x i8> addrspace(3)* %arrayidx552		; <<4 x i8>> [#uses=1]
	%tmp554 = extractelement <4 x i8> %tmp553, i32 1		; <i8> [#uses=1]
	%conv555 = uitofp i8 %tmp554 to float		; <float> [#uses=1]
	%cmp556 = fcmp olt float %tmp549, %conv555		; <i1> [#uses=1]
	%conv557 = zext i1 %cmp556 to i32		; <i32> [#uses=1]
	%add558 = add i32 %tmp546, %conv557		; <i32> [#uses=1]
	store i32 %add558, i32* %arrayidx545
	%arraydecay559 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx560 = getelementptr i32* %arraydecay559, i32 2		; <i32*> [#uses=2]
	%tmp561 = load i32* %arrayidx560		; <i32> [#uses=1]
	%arraydecay562 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx563 = getelementptr float* %arraydecay562, i32 2		; <float*> [#uses=1]
	%tmp564 = load float* %arrayidx563		; <float> [#uses=1]
	%tmp565 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc566 = add i32 %tmp565, 1		; <i32> [#uses=1]
	store i32 %inc566, i32* %iLocalPixOffset
	%tmp567 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx568 = getelementptr <4 x i8> addrspace(3)* %tmp567, i32 %tmp565		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp569 = load <4 x i8> addrspace(3)* %arrayidx568		; <<4 x i8>> [#uses=1]
	%tmp570 = extractelement <4 x i8> %tmp569, i32 2		; <i8> [#uses=1]
	%conv571 = uitofp i8 %tmp570 to float		; <float> [#uses=1]
	%cmp572 = fcmp olt float %tmp564, %conv571		; <i1> [#uses=1]
	%conv573 = zext i1 %cmp572 to i32		; <i32> [#uses=1]
	%add574 = add i32 %tmp561, %conv573		; <i32> [#uses=1]
	store i32 %add574, i32* %arrayidx560
	%arraydecay575 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx576 = getelementptr i32* %arraydecay575, i32 0		; <i32*> [#uses=2]
	%tmp577 = load i32* %arrayidx576		; <i32> [#uses=1]
	%arraydecay578 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx579 = getelementptr float* %arraydecay578, i32 0		; <float*> [#uses=1]
	%tmp580 = load float* %arrayidx579		; <float> [#uses=1]
	%tmp581 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp582 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx583 = getelementptr <4 x i8> addrspace(3)* %tmp582, i32 %tmp581		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp584 = load <4 x i8> addrspace(3)* %arrayidx583		; <<4 x i8>> [#uses=1]
	%tmp585 = extractelement <4 x i8> %tmp584, i32 0		; <i8> [#uses=1]
	%conv586 = uitofp i8 %tmp585 to float		; <float> [#uses=1]
	%cmp587 = fcmp olt float %tmp580, %conv586		; <i1> [#uses=1]
	%conv588 = zext i1 %cmp587 to i32		; <i32> [#uses=1]
	%add589 = add i32 %tmp577, %conv588		; <i32> [#uses=1]
	store i32 %add589, i32* %arrayidx576
	%arraydecay590 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx591 = getelementptr i32* %arraydecay590, i32 1		; <i32*> [#uses=2]
	%tmp592 = load i32* %arrayidx591		; <i32> [#uses=1]
	%arraydecay593 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx594 = getelementptr float* %arraydecay593, i32 1		; <float*> [#uses=1]
	%tmp595 = load float* %arrayidx594		; <float> [#uses=1]
	%tmp596 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp597 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx598 = getelementptr <4 x i8> addrspace(3)* %tmp597, i32 %tmp596		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp599 = load <4 x i8> addrspace(3)* %arrayidx598		; <<4 x i8>> [#uses=1]
	%tmp600 = extractelement <4 x i8> %tmp599, i32 1		; <i8> [#uses=1]
	%conv601 = uitofp i8 %tmp600 to float		; <float> [#uses=1]
	%cmp602 = fcmp olt float %tmp595, %conv601		; <i1> [#uses=1]
	%conv603 = zext i1 %cmp602 to i32		; <i32> [#uses=1]
	%add604 = add i32 %tmp592, %conv603		; <i32> [#uses=1]
	store i32 %add604, i32* %arrayidx591
	%arraydecay605 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx606 = getelementptr i32* %arraydecay605, i32 2		; <i32*> [#uses=2]
	%tmp607 = load i32* %arrayidx606		; <i32> [#uses=1]
	%arraydecay608 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx609 = getelementptr float* %arraydecay608, i32 2		; <float*> [#uses=1]
	%tmp610 = load float* %arrayidx609		; <float> [#uses=1]
	%tmp611 = load i32* %iLocalPixOffset		; <i32> [#uses=2]
	%inc612 = add i32 %tmp611, 1		; <i32> [#uses=1]
	store i32 %inc612, i32* %iLocalPixOffset
	%tmp613 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx614 = getelementptr <4 x i8> addrspace(3)* %tmp613, i32 %tmp611		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp615 = load <4 x i8> addrspace(3)* %arrayidx614		; <<4 x i8>> [#uses=1]
	%tmp616 = extractelement <4 x i8> %tmp615, i32 2		; <i8> [#uses=1]
	%conv617 = uitofp i8 %tmp616 to float		; <float> [#uses=1]
	%cmp618 = fcmp olt float %tmp610, %conv617		; <i1> [#uses=1]
	%conv619 = zext i1 %cmp618 to i32		; <i32> [#uses=1]
	%add620 = add i32 %tmp607, %conv619		; <i32> [#uses=1]
	store i32 %add620, i32* %arrayidx606
	%arraydecay621 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx622 = getelementptr i32* %arraydecay621, i32 0		; <i32*> [#uses=2]
	%tmp623 = load i32* %arrayidx622		; <i32> [#uses=1]
	%arraydecay624 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx625 = getelementptr float* %arraydecay624, i32 0		; <float*> [#uses=1]
	%tmp626 = load float* %arrayidx625		; <float> [#uses=1]
	%tmp627 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp628 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx629 = getelementptr <4 x i8> addrspace(3)* %tmp628, i32 %tmp627		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp630 = load <4 x i8> addrspace(3)* %arrayidx629		; <<4 x i8>> [#uses=1]
	%tmp631 = extractelement <4 x i8> %tmp630, i32 0		; <i8> [#uses=1]
	%conv632 = uitofp i8 %tmp631 to float		; <float> [#uses=1]
	%cmp633 = fcmp olt float %tmp626, %conv632		; <i1> [#uses=1]
	%conv634 = zext i1 %cmp633 to i32		; <i32> [#uses=1]
	%add635 = add i32 %tmp623, %conv634		; <i32> [#uses=1]
	store i32 %add635, i32* %arrayidx622
	%arraydecay636 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx637 = getelementptr i32* %arraydecay636, i32 1		; <i32*> [#uses=2]
	%tmp638 = load i32* %arrayidx637		; <i32> [#uses=1]
	%arraydecay639 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx640 = getelementptr float* %arraydecay639, i32 1		; <float*> [#uses=1]
	%tmp641 = load float* %arrayidx640		; <float> [#uses=1]
	%tmp642 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp643 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx644 = getelementptr <4 x i8> addrspace(3)* %tmp643, i32 %tmp642		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp645 = load <4 x i8> addrspace(3)* %arrayidx644		; <<4 x i8>> [#uses=1]
	%tmp646 = extractelement <4 x i8> %tmp645, i32 1		; <i8> [#uses=1]
	%conv647 = uitofp i8 %tmp646 to float		; <float> [#uses=1]
	%cmp648 = fcmp olt float %tmp641, %conv647		; <i1> [#uses=1]
	%conv649 = zext i1 %cmp648 to i32		; <i32> [#uses=1]
	%add650 = add i32 %tmp638, %conv649		; <i32> [#uses=1]
	store i32 %add650, i32* %arrayidx637
	%arraydecay651 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx652 = getelementptr i32* %arraydecay651, i32 2		; <i32*> [#uses=2]
	%tmp653 = load i32* %arrayidx652		; <i32> [#uses=1]
	%arraydecay654 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx655 = getelementptr float* %arraydecay654, i32 2		; <float*> [#uses=1]
	%tmp656 = load float* %arrayidx655		; <float> [#uses=1]
	%tmp657 = load i32* %iLocalPixOffset		; <i32> [#uses=1]
	%tmp658 = load <4 x i8> addrspace(3)** %uc4LocalData.addr		; <<4 x i8> addrspace(3)*> [#uses=1]
	%arrayidx659 = getelementptr <4 x i8> addrspace(3)* %tmp658, i32 %tmp657		; <<4 x i8> addrspace(3)*> [#uses=1]
	%tmp660 = load <4 x i8> addrspace(3)* %arrayidx659		; <<4 x i8>> [#uses=1]
	%tmp661 = extractelement <4 x i8> %tmp660, i32 2		; <i8> [#uses=1]
	%conv662 = uitofp i8 %tmp661 to float		; <float> [#uses=1]
	%cmp663 = fcmp olt float %tmp656, %conv662		; <i1> [#uses=1]
	%conv664 = zext i1 %cmp663 to i32		; <i32> [#uses=1]
	%add665 = add i32 %tmp653, %conv664		; <i32> [#uses=1]
	store i32 %add665, i32* %arrayidx652
	%arraydecay666 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx667 = getelementptr i32* %arraydecay666, i32 0		; <i32*> [#uses=1]
	%tmp668 = load i32* %arrayidx667		; <i32> [#uses=1]
	%cmp669 = icmp ugt i32 %tmp668, 4		; <i1> [#uses=1]
	br i1 %cmp669, label %if.then671, label %if.else677

if.then671:		; preds = %for.body
	%arraydecay672 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx673 = getelementptr float* %arraydecay672, i32 0		; <float*> [#uses=1]
	%arraydecay674 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx675 = getelementptr float* %arraydecay674, i32 0		; <float*> [#uses=1]
	%tmp676 = load float* %arrayidx675		; <float> [#uses=1]
	store float %tmp676, float* %arrayidx673
	br label %if.end683

if.else677:		; preds = %for.body
	%arraydecay678 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx679 = getelementptr float* %arraydecay678, i32 0		; <float*> [#uses=1]
	%arraydecay680 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx681 = getelementptr float* %arraydecay680, i32 0		; <float*> [#uses=1]
	%tmp682 = load float* %arrayidx681		; <float> [#uses=1]
	store float %tmp682, float* %arrayidx679
	br label %if.end683

if.end683:		; preds = %if.else677, %if.then671
	%arraydecay684 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx685 = getelementptr i32* %arraydecay684, i32 1		; <i32*> [#uses=1]
	%tmp686 = load i32* %arrayidx685		; <i32> [#uses=1]
	%cmp687 = icmp ugt i32 %tmp686, 4		; <i1> [#uses=1]
	br i1 %cmp687, label %if.then689, label %if.else695

if.then689:		; preds = %if.end683
	%arraydecay690 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx691 = getelementptr float* %arraydecay690, i32 1		; <float*> [#uses=1]
	%arraydecay692 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx693 = getelementptr float* %arraydecay692, i32 1		; <float*> [#uses=1]
	%tmp694 = load float* %arrayidx693		; <float> [#uses=1]
	store float %tmp694, float* %arrayidx691
	br label %if.end701

if.else695:		; preds = %if.end683
	%arraydecay696 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx697 = getelementptr float* %arraydecay696, i32 1		; <float*> [#uses=1]
	%arraydecay698 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx699 = getelementptr float* %arraydecay698, i32 1		; <float*> [#uses=1]
	%tmp700 = load float* %arrayidx699		; <float> [#uses=1]
	store float %tmp700, float* %arrayidx697
	br label %if.end701

if.end701:		; preds = %if.else695, %if.then689
	%arraydecay702 = getelementptr [3 x i32]* %uiHighCount, i32 0, i32 0		; <i32*> [#uses=1]
	%arrayidx703 = getelementptr i32* %arraydecay702, i32 2		; <i32*> [#uses=1]
	%tmp704 = load i32* %arrayidx703		; <i32> [#uses=1]
	%cmp705 = icmp ugt i32 %tmp704, 4		; <i1> [#uses=1]
	br i1 %cmp705, label %if.then707, label %if.else713

if.then707:		; preds = %if.end701
	%arraydecay708 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx709 = getelementptr float* %arraydecay708, i32 2		; <float*> [#uses=1]
	%arraydecay710 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx711 = getelementptr float* %arraydecay710, i32 2		; <float*> [#uses=1]
	%tmp712 = load float* %arrayidx711		; <float> [#uses=1]
	store float %tmp712, float* %arrayidx709
	br label %if.end719

if.else713:		; preds = %if.end701
	%arraydecay714 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx715 = getelementptr float* %arraydecay714, i32 2		; <float*> [#uses=1]
	%arraydecay716 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx717 = getelementptr float* %arraydecay716, i32 2		; <float*> [#uses=1]
	%tmp718 = load float* %arrayidx717		; <float> [#uses=1]
	store float %tmp718, float* %arrayidx715
	br label %if.end719

if.end719:		; preds = %if.else713, %if.then707
	%arraydecay720 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx721 = getelementptr float* %arraydecay720, i32 0		; <float*> [#uses=1]
	%arraydecay722 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx723 = getelementptr float* %arraydecay722, i32 0		; <float*> [#uses=1]
	%tmp724 = load float* %arrayidx723		; <float> [#uses=1]
	%arraydecay725 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx726 = getelementptr float* %arraydecay725, i32 0		; <float*> [#uses=1]
	%tmp727 = load float* %arrayidx726		; <float> [#uses=1]
	%add728 = fadd float %tmp724, %tmp727		; <float> [#uses=1]
	%mul = fmul float %add728, 5.000000e-001		; <float> [#uses=1]
	store float %mul, float* %arrayidx721
	%arraydecay729 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx730 = getelementptr float* %arraydecay729, i32 1		; <float*> [#uses=1]
	%arraydecay731 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx732 = getelementptr float* %arraydecay731, i32 1		; <float*> [#uses=1]
	%tmp733 = load float* %arrayidx732		; <float> [#uses=1]
	%arraydecay734 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx735 = getelementptr float* %arraydecay734, i32 1		; <float*> [#uses=1]
	%tmp736 = load float* %arrayidx735		; <float> [#uses=1]
	%add737 = fadd float %tmp733, %tmp736		; <float> [#uses=1]
	%mul738 = fmul float %add737, 5.000000e-001		; <float> [#uses=1]
	store float %mul738, float* %arrayidx730
	%arraydecay739 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx740 = getelementptr float* %arraydecay739, i32 2		; <float*> [#uses=1]
	%arraydecay741 = getelementptr [3 x float]* %fMaxBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx742 = getelementptr float* %arraydecay741, i32 2		; <float*> [#uses=1]
	%tmp743 = load float* %arrayidx742		; <float> [#uses=1]
	%arraydecay744 = getelementptr [3 x float]* %fMinBound, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx745 = getelementptr float* %arraydecay744, i32 2		; <float*> [#uses=1]
	%tmp746 = load float* %arrayidx745		; <float> [#uses=1]
	%add747 = fadd float %tmp743, %tmp746		; <float> [#uses=1]
	%mul748 = fmul float %add747, 5.000000e-001		; <float> [#uses=1]
	store float %mul748, float* %arrayidx740
	br label %for.inc

for.inc:		; preds = %if.end719
	%tmp749 = load i32* %iSearch		; <i32> [#uses=1]
	%inc750 = add i32 %tmp749, 1		; <i32> [#uses=1]
	store i32 %inc750, i32* %iSearch
	br label %for.cond

for.end:		; preds = %for.cond
	%arraydecay752 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx753 = getelementptr float* %arraydecay752, i32 0		; <float*> [#uses=1]
	%tmp754 = load float* %arrayidx753		; <float> [#uses=1]
	%conv755 = fptoui float %tmp754 to i32		; <i32> [#uses=1]
	%and = and i32 255, %conv755		; <i32> [#uses=1]
	store i32 %and, i32* %uiPackedPix
	%tmp756 = load i32* %uiPackedPix		; <i32> [#uses=1]
	%arraydecay757 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx758 = getelementptr float* %arraydecay757, i32 1		; <float*> [#uses=1]
	%tmp759 = load float* %arrayidx758		; <float> [#uses=1]
	%conv760 = fptoui float %tmp759 to i32		; <i32> [#uses=1]
	%shl = shl i32 %conv760, 8		; <i32> [#uses=1]
	%and761 = and i32 65280, %shl		; <i32> [#uses=1]
	%or = or i32 %tmp756, %and761		; <i32> [#uses=1]
	store i32 %or, i32* %uiPackedPix
	%tmp762 = load i32* %uiPackedPix		; <i32> [#uses=1]
	%arraydecay763 = getelementptr [3 x float]* %fMedianEstimate, i32 0, i32 0		; <float*> [#uses=1]
	%arrayidx764 = getelementptr float* %arraydecay763, i32 2		; <float*> [#uses=1]
	%tmp765 = load float* %arrayidx764		; <float> [#uses=1]
	%conv766 = fptoui float %tmp765 to i32		; <i32> [#uses=1]
	%shl767 = shl i32 %conv766, 16		; <i32> [#uses=1]
	%and768 = and i32 16711680, %shl767		; <i32> [#uses=1]
	%or769 = or i32 %tmp762, %and768		; <i32> [#uses=1]
	store i32 %or769, i32* %uiPackedPix
	%tmp770 = load i32* %iDevYPrime		; <i32> [#uses=1]
	%tmp771 = load i32* %uiDevImageHeight.addr		; <i32> [#uses=1]
	%cmp772 = icmp slt i32 %tmp770, %tmp771		; <i1> [#uses=1]
	br i1 %cmp772, label %land.lhs.true774, label %if.end786

land.lhs.true774:		; preds = %for.end
	%tmp775 = load i32* %iImagePosX		; <i32> [#uses=1]
	%tmp776 = load i32* %uiImageWidth.addr		; <i32> [#uses=1]
	%cmp777 = icmp slt i32 %tmp775, %tmp776		; <i1> [#uses=1]
	br i1 %cmp777, label %if.then779, label %if.end786

if.then779:		; preds = %land.lhs.true774
	%tmp780 = load i32* %iDevGMEMOffset		; <i32> [#uses=1]
	%call781 = call i32 @get_global_size(i32 0)		; <i32> [#uses=1]
	%add782 = add i32 %tmp780, %call781		; <i32> [#uses=1]
	%tmp783 = load i32 addrspace(1)** %uiDest.addr		; <i32 addrspace(1)*> [#uses=1]
	%arrayidx784 = getelementptr i32 addrspace(1)* %tmp783, i32 %add782		; <i32 addrspace(1)*> [#uses=1]
	%tmp785 = load i32* %uiPackedPix		; <i32> [#uses=1]
	store i32 %tmp785, i32 addrspace(1)* %arrayidx784
	br label %if.end786

if.end786:		; preds = %if.then779, %land.lhs.true774, %for.end
	ret void
}

declare i32 @get_global_id(i32)

declare i32 @__mul24_1i32(i32, i32)

declare i32 @get_global_size(i32)

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

declare i32 @__mul24_1u32(i32, i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)
