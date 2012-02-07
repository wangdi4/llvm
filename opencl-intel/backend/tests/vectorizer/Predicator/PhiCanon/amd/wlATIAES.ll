; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIAES.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [9 x i8] c"22229900\00"		; <[9 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [9 x i8] c"22229900\00"		; <[9 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @shiftRows
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @AESEncrypt
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @AESDecrypt
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define <4 x i8> @shiftRows(<4 x i8> %row, i32 %j) nounwind readnone {
entry:
  %cmp2 = icmp eq i32 %j, 0
  br i1 %cmp2, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge4 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %tmp513 = phi <4 x i8> [ %tmp5, %for.body ], [ %row, %for.body.preheader ]
  %tmp5 = shufflevector <4 x i8> %tmp513, <4 x i8> undef, <4 x i32> <i32 1, i32 2, i32 3, i32 0>
  %inc = add i32 %storemerge4, 1
  %cmp = icmp ult i32 %inc, %j
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %tmp51.lcssa = phi <4 x i8> [ %row, %entry ], [ %tmp5, %for.end.loopexit ]
  ret <4 x i8> %tmp51.lcssa
}

define void @AESEncrypt(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %roundKey, i8 addrspace(1)* %SBox, <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)* %block1, i32 %width, i32 %rounds, ...) nounwind {
entry:
  %galiosCoeff = alloca [4 x <4 x i8>], align 4
  %call = call i32 @get_group_id(i32 0) nounwind
  %call1 = call i32 @get_group_id(i32 1) nounwind
  %call2 = call i32 @get_local_id(i32 0) nounwind
  %call3 = call i32 @get_local_id(i32 1) nounwind
  %mul = mul i32 %call1, %width
  %add1 = shl i32 %call, 2
  %div2 = add i32 %mul, %add1
  %mul6 = and i32 %div2, -4
  %add8 = add i32 %mul6, %call3
  %arraydecay = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0
  store <4 x i8> <i8 2, i8 0, i8 0, i8 0>, <4 x i8>* %arraydecay, align 4
  %arrayidx14 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 1
  store <4 x i8> <i8 3, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx14, align 4
  %arrayidx18 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 2
  store <4 x i8> <i8 1, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx18, align 4
  %arrayidx22 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 3
  store <4 x i8> <i8 1, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx22, align 4
  %arrayidx27 = getelementptr <4 x i8> addrspace(3)* %block0, i32 %call3
  %arrayidx30 = getelementptr <4 x i8> addrspace(1)* %input, i32 %add8
  %tmp31 = load <4 x i8> addrspace(1)* %arrayidx30, align 4
  store <4 x i8> %tmp31, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %arrayidx38 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %call3
  %tmp39 = load <4 x i8> addrspace(1)* %arrayidx38, align 4
  %xor = xor <4 x i8> %tmp31, %tmp39
  store <4 x i8> %xor, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %cmp2.i = icmp eq i32 %call3, 0
  %0 = sub i32 0, %call3
  %rem = and i32 %0, 3
  %arrayidx78 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 %rem
  %arrayidx618 = getelementptr <4 x i8> addrspace(3)* %block1, i32 %call3
  br label %for.cond

for.cond:                                         ; preds = %for.end615, %entry
  %tmp50 = phi <4 x i8> [ %xor639, %for.end615 ], [ %xor, %entry ]
  %storemerge = phi i32 [ %inc642, %for.end615 ], [ 1, %entry ]
  %cmp = icmp ult i32 %storemerge, %rounds
  %tmp1.i = extractelement <4 x i8> %tmp50, i32 0
  %idxprom.i = zext i8 %tmp1.i to i32
  %arrayidx.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom.i
  %tmp3.i = load i8 addrspace(1)* %arrayidx.i, align 1
  %1 = insertelement <4 x i8> undef, i8 %tmp3.i, i32 0
  %tmp5.i = extractelement <4 x i8> %tmp50, i32 1
  %idxprom7.i = zext i8 %tmp5.i to i32
  %arrayidx8.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom7.i
  %tmp9.i = load i8 addrspace(1)* %arrayidx8.i, align 1
  %2 = insertelement <4 x i8> %1, i8 %tmp9.i, i32 1
  %tmp11.i = extractelement <4 x i8> %tmp50, i32 2
  %idxprom13.i = zext i8 %tmp11.i to i32
  %arrayidx14.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom13.i
  %tmp15.i = load i8 addrspace(1)* %arrayidx14.i, align 1
  %3 = insertelement <4 x i8> %2, i8 %tmp15.i, i32 2
  %tmp17.i = extractelement <4 x i8> %tmp50, i32 3
  %idxprom19.i = zext i8 %tmp17.i to i32
  %arrayidx20.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom19.i
  %tmp21.i = load i8 addrspace(1)* %arrayidx20.i, align 1
  %4 = insertelement <4 x i8> %3, i8 %tmp21.i, i32 3
  store <4 x i8> %4, <4 x i8> addrspace(3)* %arrayidx27, align 4
  br i1 %cmp2.i, label %shiftRows.exit, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %for.cond
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i.preheader, %for.body.i
  %storemerge4.i = phi i32 [ %inc.i, %for.body.i ], [ 0, %for.body.i.preheader ]
  %tmp513.i = phi <4 x i8> [ %tmp5.i32, %for.body.i ], [ %4, %for.body.i.preheader ]
  %tmp5.i32 = shufflevector <4 x i8> %tmp513.i, <4 x i8> undef, <4 x i32> <i32 1, i32 2, i32 3, i32 0>
  %inc.i = add i32 %storemerge4.i, 1
  %cmp.i = icmp ult i32 %inc.i, %call3
  br i1 %cmp.i, label %for.body.i, label %shiftRows.exit.loopexit

shiftRows.exit.loopexit:                          ; preds = %for.body.i
  br label %shiftRows.exit

shiftRows.exit:                                   ; preds = %shiftRows.exit.loopexit, %for.cond
  %tmp51.lcssa.i = phi <4 x i8> [ %4, %for.cond ], [ %tmp5.i32, %shiftRows.exit.loopexit ]
  store <4 x i8> %tmp51.lcssa.i, <4 x i8> addrspace(3)* %arrayidx27, align 4
  br i1 %cmp, label %for.body, label %for.end643

for.body:                                         ; preds = %shiftRows.exit
  call void @barrier(i32 1) nounwind
  %tmp71 = load <4 x i8> addrspace(3)* %block0, align 4
  %tmp72 = extractelement <4 x i8> %tmp71, i32 0
  %tmp79 = load <4 x i8>* %arrayidx78, align 4
  %tmp80 = extractelement <4 x i8> %tmp79, i32 0
  %and = and i8 %tmp80, 1
  %cmp87 = icmp eq i8 %and, 0
  %xor9331 = select i1 %cmp87, i8 0, i8 %tmp72
  %shl = shl i8 %tmp72, 1
  %cmp105 = icmp sgt i8 %tmp72, -1
  %xor110 = xor i8 %shl, 27
  %xor57664 = select i1 %cmp105, i8 %shl, i8 %xor110
  %and.1 = and i8 %tmp80, 2
  %cmp87.1 = icmp eq i8 %and.1, 0
  %xor9331.1 = select i1 %cmp87.1, i8 0, i8 %xor57664
  %tmp58748.xor9331.1 = xor i8 %xor9331.1, %xor9331
  %shl.1 = shl i8 %xor57664, 1
  %cmp105.1 = icmp sgt i8 %xor57664, -1
  %xor110.1 = xor i8 %shl.1, 27
  %xor57664.1 = select i1 %cmp105.1, i8 %shl.1, i8 %xor110.1
  %and.2 = and i8 %tmp80, 4
  %cmp87.2 = icmp eq i8 %and.2, 0
  %xor9331.2 = select i1 %cmp87.2, i8 0, i8 %xor57664.1
  %tmp58748.xor9331.2 = xor i8 %xor9331.2, %tmp58748.xor9331.1
  %shl.2 = shl i8 %xor57664.1, 1
  %cmp105.2 = icmp sgt i8 %xor57664.1, -1
  %xor110.2 = xor i8 %shl.2, 27
  %xor57664.2 = select i1 %cmp105.2, i8 %shl.2, i8 %xor110.2
  %and.3 = and i8 %tmp80, 8
  %cmp87.3 = icmp eq i8 %and.3, 0
  %xor9331.3 = select i1 %cmp87.3, i8 0, i8 %xor57664.2
  %tmp58748.xor9331.3 = xor i8 %xor9331.3, %tmp58748.xor9331.2
  %shl.3 = shl i8 %xor57664.2, 1
  %cmp105.3 = icmp sgt i8 %xor57664.2, -1
  %xor110.3 = xor i8 %shl.3, 27
  %xor57664.3 = select i1 %cmp105.3, i8 %shl.3, i8 %xor110.3
  %and.4 = and i8 %tmp80, 16
  %cmp87.4 = icmp eq i8 %and.4, 0
  %xor9331.4 = select i1 %cmp87.4, i8 0, i8 %xor57664.3
  %tmp58748.xor9331.4 = xor i8 %xor9331.4, %tmp58748.xor9331.3
  %shl.4 = shl i8 %xor57664.3, 1
  %cmp105.4 = icmp sgt i8 %xor57664.3, -1
  %xor110.4 = xor i8 %shl.4, 27
  %xor57664.4 = select i1 %cmp105.4, i8 %shl.4, i8 %xor110.4
  %and.5 = and i8 %tmp80, 32
  %cmp87.5 = icmp eq i8 %and.5, 0
  %xor9331.5 = select i1 %cmp87.5, i8 0, i8 %xor57664.4
  %tmp58748.xor9331.5 = xor i8 %xor9331.5, %tmp58748.xor9331.4
  %shl.5 = shl i8 %xor57664.4, 1
  %cmp105.5 = icmp sgt i8 %xor57664.4, -1
  %xor110.5 = xor i8 %shl.5, 27
  %xor57664.5 = select i1 %cmp105.5, i8 %shl.5, i8 %xor110.5
  %and.6 = and i8 %tmp80, 64
  %cmp87.6 = icmp eq i8 %and.6, 0
  %xor9331.6 = select i1 %cmp87.6, i8 0, i8 %xor57664.5
  %tmp58748.xor9331.6 = xor i8 %xor9331.6, %tmp58748.xor9331.5
  %shl.6 = shl i8 %xor57664.5, 1
  %cmp105.6 = icmp sgt i8 %xor57664.5, -1
  %xor110.6 = xor i8 %shl.6, 27
  %xor57664.6 = select i1 %cmp105.6, i8 %shl.6, i8 %xor110.6
  %cmp87.7 = icmp sgt i8 %tmp80, -1
  %xor9331.7 = select i1 %cmp87.7, i8 0, i8 %xor57664.6
  %tmp58748.xor9331.7 = xor i8 %xor9331.7, %tmp58748.xor9331.6
  %tmp121 = extractelement <4 x i8> %tmp71, i32 1
  %shl161 = shl i8 %tmp121, 1
  %cmp165 = icmp sgt i8 %tmp121, -1
  %xor170 = xor i8 %shl161, 27
  %xor57662 = select i1 %cmp165, i8 %shl161, i8 %xor170
  %xor15028.1 = select i1 %cmp87.1, i8 0, i8 %xor57662
  %shl161.1 = shl i8 %xor57662, 1
  %cmp165.1 = icmp sgt i8 %xor57662, -1
  %xor170.1 = xor i8 %shl161.1, 27
  %xor57662.1 = select i1 %cmp165.1, i8 %shl161.1, i8 %xor170.1
  %xor15028.2 = select i1 %cmp87.2, i8 0, i8 %xor57662.1
  %shl161.2 = shl i8 %xor57662.1, 1
  %cmp165.2 = icmp sgt i8 %xor57662.1, -1
  %xor170.2 = xor i8 %shl161.2, 27
  %xor57662.2 = select i1 %cmp165.2, i8 %shl161.2, i8 %xor170.2
  %xor15028.3 = select i1 %cmp87.3, i8 0, i8 %xor57662.2
  %shl161.3 = shl i8 %xor57662.2, 1
  %cmp165.3 = icmp sgt i8 %xor57662.2, -1
  %xor170.3 = xor i8 %shl161.3, 27
  %xor57662.3 = select i1 %cmp165.3, i8 %shl161.3, i8 %xor170.3
  %xor15028.4 = select i1 %cmp87.4, i8 0, i8 %xor57662.3
  %shl161.4 = shl i8 %xor57662.3, 1
  %cmp165.4 = icmp sgt i8 %xor57662.3, -1
  %xor170.4 = xor i8 %shl161.4, 27
  %xor57662.4 = select i1 %cmp165.4, i8 %shl161.4, i8 %xor170.4
  %xor15028.5 = select i1 %cmp87.5, i8 0, i8 %xor57662.4
  %shl161.5 = shl i8 %xor57662.4, 1
  %cmp165.5 = icmp sgt i8 %xor57662.4, -1
  %xor170.5 = xor i8 %shl161.5, 27
  %xor57662.5 = select i1 %cmp165.5, i8 %shl161.5, i8 %xor170.5
  %xor15028.6 = select i1 %cmp87.6, i8 0, i8 %xor57662.5
  %shl161.6 = shl i8 %xor57662.5, 1
  %cmp165.6 = icmp sgt i8 %xor57662.5, -1
  %xor170.6 = xor i8 %shl161.6, 27
  %xor57662.6 = select i1 %cmp165.6, i8 %shl161.6, i8 %xor170.6
  %xor15028.7 = select i1 %cmp87.7, i8 0, i8 %xor57662.6
  %tmp185 = extractelement <4 x i8> %tmp71, i32 2
  %xor21425 = select i1 %cmp87, i8 0, i8 %tmp185
  %shl225 = shl i8 %tmp185, 1
  %cmp229 = icmp sgt i8 %tmp185, -1
  %xor234 = xor i8 %shl225, 27
  %xor57660 = select i1 %cmp229, i8 %shl225, i8 %xor234
  %xor21425.1 = select i1 %cmp87.1, i8 0, i8 %xor57660
  %tmp58744.xor21425.1 = xor i8 %xor21425.1, %xor21425
  %shl225.1 = shl i8 %xor57660, 1
  %cmp229.1 = icmp sgt i8 %xor57660, -1
  %xor234.1 = xor i8 %shl225.1, 27
  %xor57660.1 = select i1 %cmp229.1, i8 %shl225.1, i8 %xor234.1
  %xor21425.2 = select i1 %cmp87.2, i8 0, i8 %xor57660.1
  %tmp58744.xor21425.2 = xor i8 %xor21425.2, %tmp58744.xor21425.1
  %shl225.2 = shl i8 %xor57660.1, 1
  %cmp229.2 = icmp sgt i8 %xor57660.1, -1
  %xor234.2 = xor i8 %shl225.2, 27
  %xor57660.2 = select i1 %cmp229.2, i8 %shl225.2, i8 %xor234.2
  %xor21425.3 = select i1 %cmp87.3, i8 0, i8 %xor57660.2
  %tmp58744.xor21425.3 = xor i8 %xor21425.3, %tmp58744.xor21425.2
  %shl225.3 = shl i8 %xor57660.2, 1
  %cmp229.3 = icmp sgt i8 %xor57660.2, -1
  %xor234.3 = xor i8 %shl225.3, 27
  %xor57660.3 = select i1 %cmp229.3, i8 %shl225.3, i8 %xor234.3
  %xor21425.4 = select i1 %cmp87.4, i8 0, i8 %xor57660.3
  %tmp58744.xor21425.4 = xor i8 %xor21425.4, %tmp58744.xor21425.3
  %shl225.4 = shl i8 %xor57660.3, 1
  %cmp229.4 = icmp sgt i8 %xor57660.3, -1
  %xor234.4 = xor i8 %shl225.4, 27
  %xor57660.4 = select i1 %cmp229.4, i8 %shl225.4, i8 %xor234.4
  %xor21425.5 = select i1 %cmp87.5, i8 0, i8 %xor57660.4
  %tmp58744.xor21425.5 = xor i8 %xor21425.5, %tmp58744.xor21425.4
  %shl225.5 = shl i8 %xor57660.4, 1
  %cmp229.5 = icmp sgt i8 %xor57660.4, -1
  %xor234.5 = xor i8 %shl225.5, 27
  %xor57660.5 = select i1 %cmp229.5, i8 %shl225.5, i8 %xor234.5
  %xor21425.6 = select i1 %cmp87.6, i8 0, i8 %xor57660.5
  %tmp58744.xor21425.6 = xor i8 %xor21425.6, %tmp58744.xor21425.5
  %shl225.6 = shl i8 %xor57660.5, 1
  %cmp229.6 = icmp sgt i8 %xor57660.5, -1
  %xor234.6 = xor i8 %shl225.6, 27
  %xor57660.6 = select i1 %cmp229.6, i8 %shl225.6, i8 %xor234.6
  %xor21425.7 = select i1 %cmp87.7, i8 0, i8 %xor57660.6
  %tmp58744.xor21425.7 = xor i8 %xor21425.7, %tmp58744.xor21425.6
  %xor15028 = select i1 %cmp87, i8 0, i8 %tmp121
  %tmp58746.xor15028.1 = xor i8 %xor15028.1, %xor15028
  %tmp58746.xor15028.2 = xor i8 %xor15028.2, %tmp58746.xor15028.1
  %tmp58746.xor15028.3 = xor i8 %xor15028.3, %tmp58746.xor15028.2
  %tmp58746.xor15028.4 = xor i8 %xor15028.4, %tmp58746.xor15028.3
  %tmp58746.xor15028.5 = xor i8 %xor15028.5, %tmp58746.xor15028.4
  %tmp58746.xor15028.6 = xor i8 %xor15028.6, %tmp58746.xor15028.5
  %tmp249 = extractelement <4 x i8> %tmp71, i32 3
  %xor27822 = select i1 %cmp87, i8 0, i8 %tmp249
  %shl289 = shl i8 %tmp249, 1
  %cmp293 = icmp sgt i8 %tmp249, -1
  %xor298 = xor i8 %shl289, 27
  %xor57658 = select i1 %cmp293, i8 %shl289, i8 %xor298
  %xor27822.1 = select i1 %cmp87.1, i8 0, i8 %xor57658
  %tmp58742.xor27822.1 = xor i8 %xor27822.1, %xor27822
  %shl289.1 = shl i8 %xor57658, 1
  %cmp293.1 = icmp sgt i8 %xor57658, -1
  %xor298.1 = xor i8 %shl289.1, 27
  %xor57658.1 = select i1 %cmp293.1, i8 %shl289.1, i8 %xor298.1
  %xor27822.2 = select i1 %cmp87.2, i8 0, i8 %xor57658.1
  %tmp58742.xor27822.2 = xor i8 %xor27822.2, %tmp58742.xor27822.1
  %shl289.2 = shl i8 %xor57658.1, 1
  %cmp293.2 = icmp sgt i8 %xor57658.1, -1
  %xor298.2 = xor i8 %shl289.2, 27
  %xor57658.2 = select i1 %cmp293.2, i8 %shl289.2, i8 %xor298.2
  %xor27822.3 = select i1 %cmp87.3, i8 0, i8 %xor57658.2
  %tmp58742.xor27822.3 = xor i8 %xor27822.3, %tmp58742.xor27822.2
  %shl289.3 = shl i8 %xor57658.2, 1
  %cmp293.3 = icmp sgt i8 %xor57658.2, -1
  %xor298.3 = xor i8 %shl289.3, 27
  %xor57658.3 = select i1 %cmp293.3, i8 %shl289.3, i8 %xor298.3
  %xor27822.4 = select i1 %cmp87.4, i8 0, i8 %xor57658.3
  %tmp58742.xor27822.4 = xor i8 %xor27822.4, %tmp58742.xor27822.3
  %shl289.4 = shl i8 %xor57658.3, 1
  %cmp293.4 = icmp sgt i8 %xor57658.3, -1
  %xor298.4 = xor i8 %shl289.4, 27
  %xor57658.4 = select i1 %cmp293.4, i8 %shl289.4, i8 %xor298.4
  %xor27822.5 = select i1 %cmp87.5, i8 0, i8 %xor57658.4
  %tmp58742.xor27822.5 = xor i8 %xor27822.5, %tmp58742.xor27822.4
  %shl289.5 = shl i8 %xor57658.4, 1
  %cmp293.5 = icmp sgt i8 %xor57658.4, -1
  %xor298.5 = xor i8 %shl289.5, 27
  %xor57658.5 = select i1 %cmp293.5, i8 %shl289.5, i8 %xor298.5
  %xor27822.6 = select i1 %cmp87.6, i8 0, i8 %xor57658.5
  %tmp58742.xor27822.6 = xor i8 %xor27822.6, %tmp58742.xor27822.5
  %shl289.6 = shl i8 %xor57658.5, 1
  %cmp293.6 = icmp sgt i8 %xor57658.5, -1
  %xor298.6 = xor i8 %shl289.6, 27
  %xor57658.6 = select i1 %cmp293.6, i8 %shl289.6, i8 %xor298.6
  %xor27822.7 = select i1 %cmp87.7, i8 0, i8 %xor57658.6
  %tmp58742.xor27822.7 = xor i8 %xor27822.7, %tmp58742.xor27822.6
  %tmp58746.xor15028.7 = xor i8 %xor15028.7, %tmp58746.xor15028.6
  br label %for.body315

for.body315:                                      ; preds = %for.body, %for.body315
  %storemerge7113 = phi i32 [ 1, %for.body ], [ %inc614, %for.body315 ]
  %tmp62073112 = phi i8 [ %tmp58748.xor9331.7, %for.body ], [ %xor5923, %for.body315 ]
  %tmp62174111 = phi i8 [ %tmp58746.xor15028.7, %for.body ], [ %xor5984, %for.body315 ]
  %tmp62275110 = phi i8 [ %tmp58744.xor21425.7, %for.body ], [ %xor6045, %for.body315 ]
  %tmp62376109 = phi i8 [ %tmp58742.xor27822.7, %for.body ], [ %xor6106, %for.body315 ]
  %arrayidx322 = getelementptr <4 x i8> addrspace(3)* %block0, i32 %storemerge7113
  %tmp323 = load <4 x i8> addrspace(3)* %arrayidx322, align 4
  %tmp324 = extractelement <4 x i8> %tmp323, i32 0
  %sub329 = sub i32 %storemerge7113, %call3
  %rem333 = and i32 %sub329, 3
  %arrayidx335 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 %rem333
  %tmp336 = load <4 x i8>* %arrayidx335, align 4
  %tmp337 = extractelement <4 x i8> %tmp336, i32 0
  %and347 = and i8 %tmp337, 1
  %cmp348 = icmp eq i8 %and347, 0
  %xor35519 = select i1 %cmp348, i8 0, i8 %tmp324
  %shl366 = shl i8 %tmp324, 1
  %cmp370 = icmp sgt i8 %tmp324, -1
  %xor375 = xor i8 %shl366, 27
  %xor57656 = select i1 %cmp370, i8 %shl366, i8 %xor375
  %and347.1 = and i8 %tmp337, 2
  %cmp348.1 = icmp eq i8 %and347.1, 0
  %xor35519.1 = select i1 %cmp348.1, i8 0, i8 %xor57656
  %tmp58740.xor35519.1 = xor i8 %xor35519.1, %xor35519
  %shl366.1 = shl i8 %xor57656, 1
  %cmp370.1 = icmp sgt i8 %xor57656, -1
  %xor375.1 = xor i8 %shl366.1, 27
  %xor57656.1 = select i1 %cmp370.1, i8 %shl366.1, i8 %xor375.1
  %and347.2 = and i8 %tmp337, 4
  %cmp348.2 = icmp eq i8 %and347.2, 0
  %xor35519.2 = select i1 %cmp348.2, i8 0, i8 %xor57656.1
  %tmp58740.xor35519.2 = xor i8 %xor35519.2, %tmp58740.xor35519.1
  %shl366.2 = shl i8 %xor57656.1, 1
  %cmp370.2 = icmp sgt i8 %xor57656.1, -1
  %xor375.2 = xor i8 %shl366.2, 27
  %xor57656.2 = select i1 %cmp370.2, i8 %shl366.2, i8 %xor375.2
  %and347.3 = and i8 %tmp337, 8
  %cmp348.3 = icmp eq i8 %and347.3, 0
  %xor35519.3 = select i1 %cmp348.3, i8 0, i8 %xor57656.2
  %tmp58740.xor35519.3 = xor i8 %xor35519.3, %tmp58740.xor35519.2
  %shl366.3 = shl i8 %xor57656.2, 1
  %cmp370.3 = icmp sgt i8 %xor57656.2, -1
  %xor375.3 = xor i8 %shl366.3, 27
  %xor57656.3 = select i1 %cmp370.3, i8 %shl366.3, i8 %xor375.3
  %and347.4 = and i8 %tmp337, 16
  %cmp348.4 = icmp eq i8 %and347.4, 0
  %xor35519.4 = select i1 %cmp348.4, i8 0, i8 %xor57656.3
  %tmp58740.xor35519.4 = xor i8 %xor35519.4, %tmp58740.xor35519.3
  %shl366.4 = shl i8 %xor57656.3, 1
  %cmp370.4 = icmp sgt i8 %xor57656.3, -1
  %xor375.4 = xor i8 %shl366.4, 27
  %xor57656.4 = select i1 %cmp370.4, i8 %shl366.4, i8 %xor375.4
  %and347.5 = and i8 %tmp337, 32
  %cmp348.5 = icmp eq i8 %and347.5, 0
  %xor35519.5 = select i1 %cmp348.5, i8 0, i8 %xor57656.4
  %tmp58740.xor35519.5 = xor i8 %xor35519.5, %tmp58740.xor35519.4
  %shl366.5 = shl i8 %xor57656.4, 1
  %cmp370.5 = icmp sgt i8 %xor57656.4, -1
  %xor375.5 = xor i8 %shl366.5, 27
  %xor57656.5 = select i1 %cmp370.5, i8 %shl366.5, i8 %xor375.5
  %and347.6 = and i8 %tmp337, 64
  %cmp348.6 = icmp eq i8 %and347.6, 0
  %xor35519.6 = select i1 %cmp348.6, i8 0, i8 %xor57656.5
  %tmp58740.xor35519.6 = xor i8 %xor35519.6, %tmp58740.xor35519.5
  %shl366.6 = shl i8 %xor57656.5, 1
  %cmp370.6 = icmp sgt i8 %xor57656.5, -1
  %xor375.6 = xor i8 %shl366.6, 27
  %xor57656.6 = select i1 %cmp370.6, i8 %shl366.6, i8 %xor375.6
  %cmp348.7 = icmp sgt i8 %tmp337, -1
  %xor35519.7 = select i1 %cmp348.7, i8 0, i8 %xor57656.6
  %tmp58740.xor35519.7 = xor i8 %xor35519.7, %tmp58740.xor35519.6
  %tmp391 = extractelement <4 x i8> %tmp323, i32 1
  %shl433 = shl i8 %tmp391, 1
  %cmp437 = icmp sgt i8 %tmp391, -1
  %xor442 = xor i8 %shl433, 27
  %xor57654 = select i1 %cmp437, i8 %shl433, i8 %xor442
  %xor42216.1 = select i1 %cmp348.1, i8 0, i8 %xor57654
  %shl433.1 = shl i8 %xor57654, 1
  %cmp437.1 = icmp sgt i8 %xor57654, -1
  %xor442.1 = xor i8 %shl433.1, 27
  %xor57654.1 = select i1 %cmp437.1, i8 %shl433.1, i8 %xor442.1
  %xor42216.2 = select i1 %cmp348.2, i8 0, i8 %xor57654.1
  %shl433.2 = shl i8 %xor57654.1, 1
  %cmp437.2 = icmp sgt i8 %xor57654.1, -1
  %xor442.2 = xor i8 %shl433.2, 27
  %xor57654.2 = select i1 %cmp437.2, i8 %shl433.2, i8 %xor442.2
  %xor42216.3 = select i1 %cmp348.3, i8 0, i8 %xor57654.2
  %shl433.3 = shl i8 %xor57654.2, 1
  %cmp437.3 = icmp sgt i8 %xor57654.2, -1
  %xor442.3 = xor i8 %shl433.3, 27
  %xor57654.3 = select i1 %cmp437.3, i8 %shl433.3, i8 %xor442.3
  %xor42216.4 = select i1 %cmp348.4, i8 0, i8 %xor57654.3
  %shl433.4 = shl i8 %xor57654.3, 1
  %cmp437.4 = icmp sgt i8 %xor57654.3, -1
  %xor442.4 = xor i8 %shl433.4, 27
  %xor57654.4 = select i1 %cmp437.4, i8 %shl433.4, i8 %xor442.4
  %xor42216.5 = select i1 %cmp348.5, i8 0, i8 %xor57654.4
  %shl433.5 = shl i8 %xor57654.4, 1
  %cmp437.5 = icmp sgt i8 %xor57654.4, -1
  %xor442.5 = xor i8 %shl433.5, 27
  %xor57654.5 = select i1 %cmp437.5, i8 %shl433.5, i8 %xor442.5
  %xor42216.6 = select i1 %cmp348.6, i8 0, i8 %xor57654.5
  %shl433.6 = shl i8 %xor57654.5, 1
  %cmp437.6 = icmp sgt i8 %xor57654.5, -1
  %xor442.6 = xor i8 %shl433.6, 27
  %xor57654.6 = select i1 %cmp437.6, i8 %shl433.6, i8 %xor442.6
  %xor42216.7 = select i1 %cmp348.7, i8 0, i8 %xor57654.6
  %tmp458 = extractelement <4 x i8> %tmp323, i32 2
  %xor48913 = select i1 %cmp348, i8 0, i8 %tmp458
  %shl500 = shl i8 %tmp458, 1
  %cmp504 = icmp sgt i8 %tmp458, -1
  %xor509 = xor i8 %shl500, 27
  %xor57652 = select i1 %cmp504, i8 %shl500, i8 %xor509
  %xor48913.1 = select i1 %cmp348.1, i8 0, i8 %xor57652
  %tmp58736.xor48913.1 = xor i8 %xor48913.1, %xor48913
  %shl500.1 = shl i8 %xor57652, 1
  %cmp504.1 = icmp sgt i8 %xor57652, -1
  %xor509.1 = xor i8 %shl500.1, 27
  %xor57652.1 = select i1 %cmp504.1, i8 %shl500.1, i8 %xor509.1
  %xor48913.2 = select i1 %cmp348.2, i8 0, i8 %xor57652.1
  %tmp58736.xor48913.2 = xor i8 %xor48913.2, %tmp58736.xor48913.1
  %shl500.2 = shl i8 %xor57652.1, 1
  %cmp504.2 = icmp sgt i8 %xor57652.1, -1
  %xor509.2 = xor i8 %shl500.2, 27
  %xor57652.2 = select i1 %cmp504.2, i8 %shl500.2, i8 %xor509.2
  %xor48913.3 = select i1 %cmp348.3, i8 0, i8 %xor57652.2
  %tmp58736.xor48913.3 = xor i8 %xor48913.3, %tmp58736.xor48913.2
  %shl500.3 = shl i8 %xor57652.2, 1
  %cmp504.3 = icmp sgt i8 %xor57652.2, -1
  %xor509.3 = xor i8 %shl500.3, 27
  %xor57652.3 = select i1 %cmp504.3, i8 %shl500.3, i8 %xor509.3
  %xor48913.4 = select i1 %cmp348.4, i8 0, i8 %xor57652.3
  %tmp58736.xor48913.4 = xor i8 %xor48913.4, %tmp58736.xor48913.3
  %shl500.4 = shl i8 %xor57652.3, 1
  %cmp504.4 = icmp sgt i8 %xor57652.3, -1
  %xor509.4 = xor i8 %shl500.4, 27
  %xor57652.4 = select i1 %cmp504.4, i8 %shl500.4, i8 %xor509.4
  %xor48913.5 = select i1 %cmp348.5, i8 0, i8 %xor57652.4
  %tmp58736.xor48913.5 = xor i8 %xor48913.5, %tmp58736.xor48913.4
  %shl500.5 = shl i8 %xor57652.4, 1
  %cmp504.5 = icmp sgt i8 %xor57652.4, -1
  %xor509.5 = xor i8 %shl500.5, 27
  %xor57652.5 = select i1 %cmp504.5, i8 %shl500.5, i8 %xor509.5
  %xor48913.6 = select i1 %cmp348.6, i8 0, i8 %xor57652.5
  %tmp58736.xor48913.6 = xor i8 %xor48913.6, %tmp58736.xor48913.5
  %shl500.6 = shl i8 %xor57652.5, 1
  %cmp504.6 = icmp sgt i8 %xor57652.5, -1
  %xor509.6 = xor i8 %shl500.6, 27
  %xor57652.6 = select i1 %cmp504.6, i8 %shl500.6, i8 %xor509.6
  %xor48913.7 = select i1 %cmp348.7, i8 0, i8 %xor57652.6
  %tmp58736.xor48913.7 = xor i8 %xor48913.7, %tmp58736.xor48913.6
  %xor42216 = select i1 %cmp348, i8 0, i8 %tmp391
  %tmp58738.xor42216.1 = xor i8 %xor42216.1, %xor42216
  %tmp58738.xor42216.2 = xor i8 %xor42216.2, %tmp58738.xor42216.1
  %tmp58738.xor42216.3 = xor i8 %xor42216.3, %tmp58738.xor42216.2
  %tmp58738.xor42216.4 = xor i8 %xor42216.4, %tmp58738.xor42216.3
  %tmp58738.xor42216.5 = xor i8 %xor42216.5, %tmp58738.xor42216.4
  %tmp58738.xor42216.6 = xor i8 %xor42216.6, %tmp58738.xor42216.5
  %tmp525 = extractelement <4 x i8> %tmp323, i32 3
  %xor55610 = select i1 %cmp348, i8 0, i8 %tmp525
  %shl567 = shl i8 %tmp525, 1
  %cmp571 = icmp sgt i8 %tmp525, -1
  %xor576 = xor i8 %shl567, 27
  %xor57650 = select i1 %cmp571, i8 %shl567, i8 %xor576
  %xor55610.1 = select i1 %cmp348.1, i8 0, i8 %xor57650
  %tmp58734.xor55610.1 = xor i8 %xor55610.1, %xor55610
  %shl567.1 = shl i8 %xor57650, 1
  %cmp571.1 = icmp sgt i8 %xor57650, -1
  %xor576.1 = xor i8 %shl567.1, 27
  %xor57650.1 = select i1 %cmp571.1, i8 %shl567.1, i8 %xor576.1
  %xor55610.2 = select i1 %cmp348.2, i8 0, i8 %xor57650.1
  %tmp58734.xor55610.2 = xor i8 %xor55610.2, %tmp58734.xor55610.1
  %shl567.2 = shl i8 %xor57650.1, 1
  %cmp571.2 = icmp sgt i8 %xor57650.1, -1
  %xor576.2 = xor i8 %shl567.2, 27
  %xor57650.2 = select i1 %cmp571.2, i8 %shl567.2, i8 %xor576.2
  %xor55610.3 = select i1 %cmp348.3, i8 0, i8 %xor57650.2
  %tmp58734.xor55610.3 = xor i8 %xor55610.3, %tmp58734.xor55610.2
  %shl567.3 = shl i8 %xor57650.2, 1
  %cmp571.3 = icmp sgt i8 %xor57650.2, -1
  %xor576.3 = xor i8 %shl567.3, 27
  %xor57650.3 = select i1 %cmp571.3, i8 %shl567.3, i8 %xor576.3
  %xor55610.4 = select i1 %cmp348.4, i8 0, i8 %xor57650.3
  %tmp58734.xor55610.4 = xor i8 %xor55610.4, %tmp58734.xor55610.3
  %shl567.4 = shl i8 %xor57650.3, 1
  %cmp571.4 = icmp sgt i8 %xor57650.3, -1
  %xor576.4 = xor i8 %shl567.4, 27
  %xor57650.4 = select i1 %cmp571.4, i8 %shl567.4, i8 %xor576.4
  %xor55610.5 = select i1 %cmp348.5, i8 0, i8 %xor57650.4
  %tmp58734.xor55610.5 = xor i8 %xor55610.5, %tmp58734.xor55610.4
  %shl567.5 = shl i8 %xor57650.4, 1
  %cmp571.5 = icmp sgt i8 %xor57650.4, -1
  %xor576.5 = xor i8 %shl567.5, 27
  %xor57650.5 = select i1 %cmp571.5, i8 %shl567.5, i8 %xor576.5
  %xor55610.6 = select i1 %cmp348.6, i8 0, i8 %xor57650.5
  %tmp58734.xor55610.6 = xor i8 %xor55610.6, %tmp58734.xor55610.5
  %shl567.6 = shl i8 %xor57650.5, 1
  %cmp571.6 = icmp sgt i8 %xor57650.5, -1
  %xor576.6 = xor i8 %shl567.6, 27
  %xor57650.6 = select i1 %cmp571.6, i8 %shl567.6, i8 %xor576.6
  %xor55610.7 = select i1 %cmp348.7, i8 0, i8 %xor57650.6
  %tmp58734.xor55610.7 = xor i8 %xor55610.7, %tmp58734.xor55610.6
  %tmp58738.xor42216.7 = xor i8 %xor42216.7, %tmp58738.xor42216.6
  %xor5923 = xor i8 %tmp58740.xor35519.7, %tmp62073112
  %xor5984 = xor i8 %tmp58738.xor42216.7, %tmp62174111
  %xor6045 = xor i8 %tmp58736.xor48913.7, %tmp62275110
  %xor6106 = xor i8 %tmp58734.xor55610.7, %tmp62376109
  %inc614 = add i32 %storemerge7113, 1
  %cmp313 = icmp ult i32 %inc614, 4
  br i1 %cmp313, label %for.body315, label %for.end615

for.end615:                                       ; preds = %for.body315
  %5 = insertelement <4 x i8> undef, i8 %xor5923, i32 0
  %6 = insertelement <4 x i8> %5, i8 %xor5984, i32 1
  %7 = insertelement <4 x i8> %6, i8 %xor6045, i32 2
  %8 = insertelement <4 x i8> %7, i8 %xor6106, i32 3
  store <4 x i8> %8, <4 x i8> addrspace(3)* %arrayidx618, align 4
  call void @barrier(i32 1) nounwind
  %tmp631 = load <4 x i8> addrspace(3)* %arrayidx618, align 4
  %mul633 = shl i32 %storemerge, 2
  %add635 = add i32 %mul633, %call3
  %arrayidx637 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %add635
  %tmp638 = load <4 x i8> addrspace(1)* %arrayidx637, align 4
  %xor639 = xor <4 x i8> %tmp631, %tmp638
  store <4 x i8> %xor639, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %inc642 = add i32 %storemerge, 1
  br label %for.cond

for.end643:                                       ; preds = %shiftRows.exit
  %arrayidx664 = getelementptr <4 x i8> addrspace(1)* %output, i32 %add8
  %mul670 = shl i32 %rounds, 2
  %add672 = add i32 %call3, %mul670
  %arrayidx674 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %add672
  %tmp675 = load <4 x i8> addrspace(1)* %arrayidx674, align 4
  %xor676 = xor <4 x i8> %tmp51.lcssa.i, %tmp675
  store <4 x i8> %xor676, <4 x i8> addrspace(1)* %arrayidx664, align 4
  ret void
}

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

define <4 x i8> @shiftRowsInv(<4 x i8> %row, i32 %j) nounwind readnone {
entry:
  %cmp2 = icmp eq i32 %j, 0
  br i1 %cmp2, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge4 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %tmp513 = phi <4 x i8> [ %tmp5, %for.body ], [ %row, %for.body.preheader ]
  %tmp5 = shufflevector <4 x i8> %tmp513, <4 x i8> undef, <4 x i32> <i32 3, i32 0, i32 1, i32 2>
  %inc = add i32 %storemerge4, 1
  %cmp = icmp ult i32 %inc, %j
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %tmp51.lcssa = phi <4 x i8> [ %row, %entry ], [ %tmp5, %for.end.loopexit ]
  ret <4 x i8> %tmp51.lcssa
}

define void @AESDecrypt(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %roundKey, i8 addrspace(1)* %SBox, <4 x i8> addrspace(3)* %block0, <4 x i8> addrspace(3)* %block1, i32 %width, i32 %rounds, ...) nounwind {
entry:
  %galiosCoeff = alloca [4 x <4 x i8>], align 4
  %call = call i32 @get_group_id(i32 0) nounwind
  %call1 = call i32 @get_group_id(i32 1) nounwind
  %call2 = call i32 @get_local_id(i32 0) nounwind
  %call3 = call i32 @get_local_id(i32 1) nounwind
  %mul = mul i32 %call1, %width
  %add1 = shl i32 %call, 2
  %div2 = add i32 %mul, %add1
  %mul6 = and i32 %div2, -4
  %add8 = add i32 %mul6, %call3
  %arraydecay = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 0
  store <4 x i8> <i8 14, i8 0, i8 0, i8 0>, <4 x i8>* %arraydecay, align 4
  %arrayidx14 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 1
  store <4 x i8> <i8 11, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx14, align 4
  %arrayidx18 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 2
  store <4 x i8> <i8 13, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx18, align 4
  %arrayidx22 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 3
  store <4 x i8> <i8 9, i8 0, i8 0, i8 0>, <4 x i8>* %arrayidx22, align 4
  %arrayidx27 = getelementptr <4 x i8> addrspace(3)* %block0, i32 %call3
  %arrayidx30 = getelementptr <4 x i8> addrspace(1)* %input, i32 %add8
  %tmp31 = load <4 x i8> addrspace(1)* %arrayidx30, align 4
  store <4 x i8> %tmp31, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %mul37 = shl i32 %rounds, 2
  %add39 = add i32 %call3, %mul37
  %arrayidx41 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %add39
  %tmp42 = load <4 x i8> addrspace(1)* %arrayidx41, align 4
  %xor = xor <4 x i8> %tmp31, %tmp42
  %cmp2.i = icmp eq i32 %call3, 0
  %arrayidx66 = getelementptr <4 x i8> addrspace(3)* %block1, i32 %call3
  %0 = sub i32 0, %call3
  %rem = and i32 %0, 3
  %arrayidx97 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 %rem
  br label %for.cond

for.cond:                                         ; preds = %for.end634, %entry
  %storemerge77 = phi <4 x i8> [ %xor, %entry ], [ %8, %for.end634 ]
  %storemerge.in = phi i32 [ %rounds, %entry ], [ %storemerge, %for.end634 ]
  store <4 x i8> %storemerge77, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %storemerge = add i32 %storemerge.in, -1
  %cmp = icmp eq i32 %storemerge, 0
  br i1 %cmp2.i, label %shiftRowsInv.exit, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %for.cond
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i.preheader, %for.body.i
  %storemerge4.i = phi i32 [ %inc.i, %for.body.i ], [ 0, %for.body.i.preheader ]
  %tmp513.i = phi <4 x i8> [ %tmp5.i, %for.body.i ], [ %storemerge77, %for.body.i.preheader ]
  %tmp5.i = shufflevector <4 x i8> %tmp513.i, <4 x i8> undef, <4 x i32> <i32 3, i32 0, i32 1, i32 2>
  %inc.i = add i32 %storemerge4.i, 1
  %cmp.i = icmp ult i32 %inc.i, %call3
  br i1 %cmp.i, label %for.body.i, label %shiftRowsInv.exit.loopexit

shiftRowsInv.exit.loopexit:                       ; preds = %for.body.i
  br label %shiftRowsInv.exit

shiftRowsInv.exit:                                ; preds = %shiftRowsInv.exit.loopexit, %for.cond
  %tmp51.lcssa.i = phi <4 x i8> [ %storemerge77, %for.cond ], [ %tmp5.i, %shiftRowsInv.exit.loopexit ]
  store <4 x i8> %tmp51.lcssa.i, <4 x i8> addrspace(3)* %arrayidx27, align 4
  %tmp1.i = extractelement <4 x i8> %tmp51.lcssa.i, i32 0
  %idxprom.i = zext i8 %tmp1.i to i32
  %arrayidx.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom.i
  %tmp3.i = load i8 addrspace(1)* %arrayidx.i, align 1
  %1 = insertelement <4 x i8> undef, i8 %tmp3.i, i32 0
  %tmp5.i32 = extractelement <4 x i8> %tmp51.lcssa.i, i32 1
  %idxprom7.i = zext i8 %tmp5.i32 to i32
  %arrayidx8.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom7.i
  %tmp9.i = load i8 addrspace(1)* %arrayidx8.i, align 1
  %2 = insertelement <4 x i8> %1, i8 %tmp9.i, i32 1
  %tmp11.i = extractelement <4 x i8> %tmp51.lcssa.i, i32 2
  %idxprom13.i = zext i8 %tmp11.i to i32
  %arrayidx14.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom13.i
  %tmp15.i = load i8 addrspace(1)* %arrayidx14.i, align 1
  %3 = insertelement <4 x i8> %2, i8 %tmp15.i, i32 2
  %tmp17.i = extractelement <4 x i8> %tmp51.lcssa.i, i32 3
  %idxprom19.i = zext i8 %tmp17.i to i32
  %arrayidx20.i = getelementptr i8 addrspace(1)* %SBox, i32 %idxprom19.i
  %tmp21.i = load i8 addrspace(1)* %arrayidx20.i, align 1
  %4 = insertelement <4 x i8> %3, i8 %tmp21.i, i32 3
  store <4 x i8> %4, <4 x i8> addrspace(3)* %arrayidx27, align 4
  br i1 %cmp, label %for.end646, label %for.body

for.body:                                         ; preds = %shiftRowsInv.exit
  call void @barrier(i32 1) nounwind
  %tmp70 = load <4 x i8> addrspace(3)* %arrayidx27, align 4
  %mul72 = shl i32 %storemerge, 2
  %add74 = add i32 %mul72, %call3
  %arrayidx76 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %add74
  %tmp77 = load <4 x i8> addrspace(1)* %arrayidx76, align 4
  %xor78 = xor <4 x i8> %tmp70, %tmp77
  store <4 x i8> %xor78, <4 x i8> addrspace(3)* %arrayidx66, align 4
  call void @barrier(i32 1) nounwind
  %tmp89 = load <4 x i8> addrspace(3)* %block1, align 4
  %tmp90 = extractelement <4 x i8> %tmp89, i32 0
  %tmp98 = load <4 x i8>* %arrayidx97, align 4
  %tmp99 = extractelement <4 x i8> %tmp98, i32 0
  %and = and i8 %tmp99, 1
  %cmp106 = icmp eq i8 %and, 0
  %xor11231 = select i1 %cmp106, i8 0, i8 %tmp90
  %shl = shl i8 %tmp90, 1
  %cmp124 = icmp sgt i8 %tmp90, -1
  %xor129 = xor i8 %shl, 27
  %xor59564 = select i1 %cmp124, i8 %shl, i8 %xor129
  %and.1 = and i8 %tmp99, 2
  %cmp106.1 = icmp eq i8 %and.1, 0
  %xor11231.1 = select i1 %cmp106.1, i8 0, i8 %xor59564
  %tmp60648.xor11231.1 = xor i8 %xor11231.1, %xor11231
  %shl.1 = shl i8 %xor59564, 1
  %cmp124.1 = icmp sgt i8 %xor59564, -1
  %xor129.1 = xor i8 %shl.1, 27
  %xor59564.1 = select i1 %cmp124.1, i8 %shl.1, i8 %xor129.1
  %and.2 = and i8 %tmp99, 4
  %cmp106.2 = icmp eq i8 %and.2, 0
  %xor11231.2 = select i1 %cmp106.2, i8 0, i8 %xor59564.1
  %tmp60648.xor11231.2 = xor i8 %xor11231.2, %tmp60648.xor11231.1
  %shl.2 = shl i8 %xor59564.1, 1
  %cmp124.2 = icmp sgt i8 %xor59564.1, -1
  %xor129.2 = xor i8 %shl.2, 27
  %xor59564.2 = select i1 %cmp124.2, i8 %shl.2, i8 %xor129.2
  %and.3 = and i8 %tmp99, 8
  %cmp106.3 = icmp eq i8 %and.3, 0
  %xor11231.3 = select i1 %cmp106.3, i8 0, i8 %xor59564.2
  %tmp60648.xor11231.3 = xor i8 %xor11231.3, %tmp60648.xor11231.2
  %shl.3 = shl i8 %xor59564.2, 1
  %cmp124.3 = icmp sgt i8 %xor59564.2, -1
  %xor129.3 = xor i8 %shl.3, 27
  %xor59564.3 = select i1 %cmp124.3, i8 %shl.3, i8 %xor129.3
  %and.4 = and i8 %tmp99, 16
  %cmp106.4 = icmp eq i8 %and.4, 0
  %xor11231.4 = select i1 %cmp106.4, i8 0, i8 %xor59564.3
  %tmp60648.xor11231.4 = xor i8 %xor11231.4, %tmp60648.xor11231.3
  %shl.4 = shl i8 %xor59564.3, 1
  %cmp124.4 = icmp sgt i8 %xor59564.3, -1
  %xor129.4 = xor i8 %shl.4, 27
  %xor59564.4 = select i1 %cmp124.4, i8 %shl.4, i8 %xor129.4
  %and.5 = and i8 %tmp99, 32
  %cmp106.5 = icmp eq i8 %and.5, 0
  %xor11231.5 = select i1 %cmp106.5, i8 0, i8 %xor59564.4
  %tmp60648.xor11231.5 = xor i8 %xor11231.5, %tmp60648.xor11231.4
  %shl.5 = shl i8 %xor59564.4, 1
  %cmp124.5 = icmp sgt i8 %xor59564.4, -1
  %xor129.5 = xor i8 %shl.5, 27
  %xor59564.5 = select i1 %cmp124.5, i8 %shl.5, i8 %xor129.5
  %and.6 = and i8 %tmp99, 64
  %cmp106.6 = icmp eq i8 %and.6, 0
  %xor11231.6 = select i1 %cmp106.6, i8 0, i8 %xor59564.5
  %tmp60648.xor11231.6 = xor i8 %xor11231.6, %tmp60648.xor11231.5
  %shl.6 = shl i8 %xor59564.5, 1
  %cmp124.6 = icmp sgt i8 %xor59564.5, -1
  %xor129.6 = xor i8 %shl.6, 27
  %xor59564.6 = select i1 %cmp124.6, i8 %shl.6, i8 %xor129.6
  %cmp106.7 = icmp sgt i8 %tmp99, -1
  %xor11231.7 = select i1 %cmp106.7, i8 0, i8 %xor59564.6
  %tmp60648.xor11231.7 = xor i8 %xor11231.7, %tmp60648.xor11231.6
  %tmp140 = extractelement <4 x i8> %tmp89, i32 1
  %shl180 = shl i8 %tmp140, 1
  %cmp184 = icmp sgt i8 %tmp140, -1
  %xor189 = xor i8 %shl180, 27
  %xor59562 = select i1 %cmp184, i8 %shl180, i8 %xor189
  %xor16928.1 = select i1 %cmp106.1, i8 0, i8 %xor59562
  %shl180.1 = shl i8 %xor59562, 1
  %cmp184.1 = icmp sgt i8 %xor59562, -1
  %xor189.1 = xor i8 %shl180.1, 27
  %xor59562.1 = select i1 %cmp184.1, i8 %shl180.1, i8 %xor189.1
  %xor16928.2 = select i1 %cmp106.2, i8 0, i8 %xor59562.1
  %shl180.2 = shl i8 %xor59562.1, 1
  %cmp184.2 = icmp sgt i8 %xor59562.1, -1
  %xor189.2 = xor i8 %shl180.2, 27
  %xor59562.2 = select i1 %cmp184.2, i8 %shl180.2, i8 %xor189.2
  %xor16928.3 = select i1 %cmp106.3, i8 0, i8 %xor59562.2
  %shl180.3 = shl i8 %xor59562.2, 1
  %cmp184.3 = icmp sgt i8 %xor59562.2, -1
  %xor189.3 = xor i8 %shl180.3, 27
  %xor59562.3 = select i1 %cmp184.3, i8 %shl180.3, i8 %xor189.3
  %xor16928.4 = select i1 %cmp106.4, i8 0, i8 %xor59562.3
  %shl180.4 = shl i8 %xor59562.3, 1
  %cmp184.4 = icmp sgt i8 %xor59562.3, -1
  %xor189.4 = xor i8 %shl180.4, 27
  %xor59562.4 = select i1 %cmp184.4, i8 %shl180.4, i8 %xor189.4
  %xor16928.5 = select i1 %cmp106.5, i8 0, i8 %xor59562.4
  %shl180.5 = shl i8 %xor59562.4, 1
  %cmp184.5 = icmp sgt i8 %xor59562.4, -1
  %xor189.5 = xor i8 %shl180.5, 27
  %xor59562.5 = select i1 %cmp184.5, i8 %shl180.5, i8 %xor189.5
  %xor16928.6 = select i1 %cmp106.6, i8 0, i8 %xor59562.5
  %shl180.6 = shl i8 %xor59562.5, 1
  %cmp184.6 = icmp sgt i8 %xor59562.5, -1
  %xor189.6 = xor i8 %shl180.6, 27
  %xor59562.6 = select i1 %cmp184.6, i8 %shl180.6, i8 %xor189.6
  %xor16928.7 = select i1 %cmp106.7, i8 0, i8 %xor59562.6
  %tmp204 = extractelement <4 x i8> %tmp89, i32 2
  %xor23325 = select i1 %cmp106, i8 0, i8 %tmp204
  %shl244 = shl i8 %tmp204, 1
  %cmp248 = icmp sgt i8 %tmp204, -1
  %xor253 = xor i8 %shl244, 27
  %xor59560 = select i1 %cmp248, i8 %shl244, i8 %xor253
  %xor23325.1 = select i1 %cmp106.1, i8 0, i8 %xor59560
  %tmp60644.xor23325.1 = xor i8 %xor23325.1, %xor23325
  %shl244.1 = shl i8 %xor59560, 1
  %cmp248.1 = icmp sgt i8 %xor59560, -1
  %xor253.1 = xor i8 %shl244.1, 27
  %xor59560.1 = select i1 %cmp248.1, i8 %shl244.1, i8 %xor253.1
  %xor23325.2 = select i1 %cmp106.2, i8 0, i8 %xor59560.1
  %tmp60644.xor23325.2 = xor i8 %xor23325.2, %tmp60644.xor23325.1
  %shl244.2 = shl i8 %xor59560.1, 1
  %cmp248.2 = icmp sgt i8 %xor59560.1, -1
  %xor253.2 = xor i8 %shl244.2, 27
  %xor59560.2 = select i1 %cmp248.2, i8 %shl244.2, i8 %xor253.2
  %xor23325.3 = select i1 %cmp106.3, i8 0, i8 %xor59560.2
  %tmp60644.xor23325.3 = xor i8 %xor23325.3, %tmp60644.xor23325.2
  %shl244.3 = shl i8 %xor59560.2, 1
  %cmp248.3 = icmp sgt i8 %xor59560.2, -1
  %xor253.3 = xor i8 %shl244.3, 27
  %xor59560.3 = select i1 %cmp248.3, i8 %shl244.3, i8 %xor253.3
  %xor23325.4 = select i1 %cmp106.4, i8 0, i8 %xor59560.3
  %tmp60644.xor23325.4 = xor i8 %xor23325.4, %tmp60644.xor23325.3
  %shl244.4 = shl i8 %xor59560.3, 1
  %cmp248.4 = icmp sgt i8 %xor59560.3, -1
  %xor253.4 = xor i8 %shl244.4, 27
  %xor59560.4 = select i1 %cmp248.4, i8 %shl244.4, i8 %xor253.4
  %xor23325.5 = select i1 %cmp106.5, i8 0, i8 %xor59560.4
  %tmp60644.xor23325.5 = xor i8 %xor23325.5, %tmp60644.xor23325.4
  %shl244.5 = shl i8 %xor59560.4, 1
  %cmp248.5 = icmp sgt i8 %xor59560.4, -1
  %xor253.5 = xor i8 %shl244.5, 27
  %xor59560.5 = select i1 %cmp248.5, i8 %shl244.5, i8 %xor253.5
  %xor23325.6 = select i1 %cmp106.6, i8 0, i8 %xor59560.5
  %tmp60644.xor23325.6 = xor i8 %xor23325.6, %tmp60644.xor23325.5
  %shl244.6 = shl i8 %xor59560.5, 1
  %cmp248.6 = icmp sgt i8 %xor59560.5, -1
  %xor253.6 = xor i8 %shl244.6, 27
  %xor59560.6 = select i1 %cmp248.6, i8 %shl244.6, i8 %xor253.6
  %xor23325.7 = select i1 %cmp106.7, i8 0, i8 %xor59560.6
  %tmp60644.xor23325.7 = xor i8 %xor23325.7, %tmp60644.xor23325.6
  %xor16928 = select i1 %cmp106, i8 0, i8 %tmp140
  %tmp60646.xor16928.1 = xor i8 %xor16928.1, %xor16928
  %tmp60646.xor16928.2 = xor i8 %xor16928.2, %tmp60646.xor16928.1
  %tmp60646.xor16928.3 = xor i8 %xor16928.3, %tmp60646.xor16928.2
  %tmp60646.xor16928.4 = xor i8 %xor16928.4, %tmp60646.xor16928.3
  %tmp60646.xor16928.5 = xor i8 %xor16928.5, %tmp60646.xor16928.4
  %tmp60646.xor16928.6 = xor i8 %xor16928.6, %tmp60646.xor16928.5
  %tmp268 = extractelement <4 x i8> %tmp89, i32 3
  %xor29722 = select i1 %cmp106, i8 0, i8 %tmp268
  %shl308 = shl i8 %tmp268, 1
  %cmp312 = icmp sgt i8 %tmp268, -1
  %xor317 = xor i8 %shl308, 27
  %xor59558 = select i1 %cmp312, i8 %shl308, i8 %xor317
  %xor29722.1 = select i1 %cmp106.1, i8 0, i8 %xor59558
  %tmp60642.xor29722.1 = xor i8 %xor29722.1, %xor29722
  %shl308.1 = shl i8 %xor59558, 1
  %cmp312.1 = icmp sgt i8 %xor59558, -1
  %xor317.1 = xor i8 %shl308.1, 27
  %xor59558.1 = select i1 %cmp312.1, i8 %shl308.1, i8 %xor317.1
  %xor29722.2 = select i1 %cmp106.2, i8 0, i8 %xor59558.1
  %tmp60642.xor29722.2 = xor i8 %xor29722.2, %tmp60642.xor29722.1
  %shl308.2 = shl i8 %xor59558.1, 1
  %cmp312.2 = icmp sgt i8 %xor59558.1, -1
  %xor317.2 = xor i8 %shl308.2, 27
  %xor59558.2 = select i1 %cmp312.2, i8 %shl308.2, i8 %xor317.2
  %xor29722.3 = select i1 %cmp106.3, i8 0, i8 %xor59558.2
  %tmp60642.xor29722.3 = xor i8 %xor29722.3, %tmp60642.xor29722.2
  %shl308.3 = shl i8 %xor59558.2, 1
  %cmp312.3 = icmp sgt i8 %xor59558.2, -1
  %xor317.3 = xor i8 %shl308.3, 27
  %xor59558.3 = select i1 %cmp312.3, i8 %shl308.3, i8 %xor317.3
  %xor29722.4 = select i1 %cmp106.4, i8 0, i8 %xor59558.3
  %tmp60642.xor29722.4 = xor i8 %xor29722.4, %tmp60642.xor29722.3
  %shl308.4 = shl i8 %xor59558.3, 1
  %cmp312.4 = icmp sgt i8 %xor59558.3, -1
  %xor317.4 = xor i8 %shl308.4, 27
  %xor59558.4 = select i1 %cmp312.4, i8 %shl308.4, i8 %xor317.4
  %xor29722.5 = select i1 %cmp106.5, i8 0, i8 %xor59558.4
  %tmp60642.xor29722.5 = xor i8 %xor29722.5, %tmp60642.xor29722.4
  %shl308.5 = shl i8 %xor59558.4, 1
  %cmp312.5 = icmp sgt i8 %xor59558.4, -1
  %xor317.5 = xor i8 %shl308.5, 27
  %xor59558.5 = select i1 %cmp312.5, i8 %shl308.5, i8 %xor317.5
  %xor29722.6 = select i1 %cmp106.6, i8 0, i8 %xor59558.5
  %tmp60642.xor29722.6 = xor i8 %xor29722.6, %tmp60642.xor29722.5
  %shl308.6 = shl i8 %xor59558.5, 1
  %cmp312.6 = icmp sgt i8 %xor59558.5, -1
  %xor317.6 = xor i8 %shl308.6, 27
  %xor59558.6 = select i1 %cmp312.6, i8 %shl308.6, i8 %xor317.6
  %xor29722.7 = select i1 %cmp106.7, i8 0, i8 %xor59558.6
  %tmp60642.xor29722.7 = xor i8 %xor29722.7, %tmp60642.xor29722.6
  %tmp60646.xor16928.7 = xor i8 %xor16928.7, %tmp60646.xor16928.6
  br label %for.body334

for.body334:                                      ; preds = %for.body, %for.body334
  %storemerge7114 = phi i32 [ 1, %for.body ], [ %inc633, %for.body334 ]
  %tmp63973113 = phi i8 [ %tmp60648.xor11231.7, %for.body ], [ %xor6113, %for.body334 ]
  %tmp64074112 = phi i8 [ %tmp60646.xor16928.7, %for.body ], [ %xor6174, %for.body334 ]
  %tmp64175111 = phi i8 [ %tmp60644.xor23325.7, %for.body ], [ %xor6235, %for.body334 ]
  %tmp64276110 = phi i8 [ %tmp60642.xor29722.7, %for.body ], [ %xor6296, %for.body334 ]
  %arrayidx341 = getelementptr <4 x i8> addrspace(3)* %block1, i32 %storemerge7114
  %tmp342 = load <4 x i8> addrspace(3)* %arrayidx341, align 4
  %tmp343 = extractelement <4 x i8> %tmp342, i32 0
  %sub348 = sub i32 %storemerge7114, %call3
  %rem352 = and i32 %sub348, 3
  %arrayidx354 = getelementptr [4 x <4 x i8>]* %galiosCoeff, i32 0, i32 %rem352
  %tmp355 = load <4 x i8>* %arrayidx354, align 4
  %tmp356 = extractelement <4 x i8> %tmp355, i32 0
  %and366 = and i8 %tmp356, 1
  %cmp367 = icmp eq i8 %and366, 0
  %xor37419 = select i1 %cmp367, i8 0, i8 %tmp343
  %shl385 = shl i8 %tmp343, 1
  %cmp389 = icmp sgt i8 %tmp343, -1
  %xor394 = xor i8 %shl385, 27
  %xor59556 = select i1 %cmp389, i8 %shl385, i8 %xor394
  %and366.1 = and i8 %tmp356, 2
  %cmp367.1 = icmp eq i8 %and366.1, 0
  %xor37419.1 = select i1 %cmp367.1, i8 0, i8 %xor59556
  %tmp60640.xor37419.1 = xor i8 %xor37419.1, %xor37419
  %shl385.1 = shl i8 %xor59556, 1
  %cmp389.1 = icmp sgt i8 %xor59556, -1
  %xor394.1 = xor i8 %shl385.1, 27
  %xor59556.1 = select i1 %cmp389.1, i8 %shl385.1, i8 %xor394.1
  %and366.2 = and i8 %tmp356, 4
  %cmp367.2 = icmp eq i8 %and366.2, 0
  %xor37419.2 = select i1 %cmp367.2, i8 0, i8 %xor59556.1
  %tmp60640.xor37419.2 = xor i8 %xor37419.2, %tmp60640.xor37419.1
  %shl385.2 = shl i8 %xor59556.1, 1
  %cmp389.2 = icmp sgt i8 %xor59556.1, -1
  %xor394.2 = xor i8 %shl385.2, 27
  %xor59556.2 = select i1 %cmp389.2, i8 %shl385.2, i8 %xor394.2
  %and366.3 = and i8 %tmp356, 8
  %cmp367.3 = icmp eq i8 %and366.3, 0
  %xor37419.3 = select i1 %cmp367.3, i8 0, i8 %xor59556.2
  %tmp60640.xor37419.3 = xor i8 %xor37419.3, %tmp60640.xor37419.2
  %shl385.3 = shl i8 %xor59556.2, 1
  %cmp389.3 = icmp sgt i8 %xor59556.2, -1
  %xor394.3 = xor i8 %shl385.3, 27
  %xor59556.3 = select i1 %cmp389.3, i8 %shl385.3, i8 %xor394.3
  %and366.4 = and i8 %tmp356, 16
  %cmp367.4 = icmp eq i8 %and366.4, 0
  %xor37419.4 = select i1 %cmp367.4, i8 0, i8 %xor59556.3
  %tmp60640.xor37419.4 = xor i8 %xor37419.4, %tmp60640.xor37419.3
  %shl385.4 = shl i8 %xor59556.3, 1
  %cmp389.4 = icmp sgt i8 %xor59556.3, -1
  %xor394.4 = xor i8 %shl385.4, 27
  %xor59556.4 = select i1 %cmp389.4, i8 %shl385.4, i8 %xor394.4
  %and366.5 = and i8 %tmp356, 32
  %cmp367.5 = icmp eq i8 %and366.5, 0
  %xor37419.5 = select i1 %cmp367.5, i8 0, i8 %xor59556.4
  %tmp60640.xor37419.5 = xor i8 %xor37419.5, %tmp60640.xor37419.4
  %shl385.5 = shl i8 %xor59556.4, 1
  %cmp389.5 = icmp sgt i8 %xor59556.4, -1
  %xor394.5 = xor i8 %shl385.5, 27
  %xor59556.5 = select i1 %cmp389.5, i8 %shl385.5, i8 %xor394.5
  %and366.6 = and i8 %tmp356, 64
  %cmp367.6 = icmp eq i8 %and366.6, 0
  %xor37419.6 = select i1 %cmp367.6, i8 0, i8 %xor59556.5
  %tmp60640.xor37419.6 = xor i8 %xor37419.6, %tmp60640.xor37419.5
  %shl385.6 = shl i8 %xor59556.5, 1
  %cmp389.6 = icmp sgt i8 %xor59556.5, -1
  %xor394.6 = xor i8 %shl385.6, 27
  %xor59556.6 = select i1 %cmp389.6, i8 %shl385.6, i8 %xor394.6
  %cmp367.7 = icmp sgt i8 %tmp356, -1
  %xor37419.7 = select i1 %cmp367.7, i8 0, i8 %xor59556.6
  %tmp60640.xor37419.7 = xor i8 %xor37419.7, %tmp60640.xor37419.6
  %tmp410 = extractelement <4 x i8> %tmp342, i32 1
  %shl452 = shl i8 %tmp410, 1
  %cmp456 = icmp sgt i8 %tmp410, -1
  %xor461 = xor i8 %shl452, 27
  %xor59554 = select i1 %cmp456, i8 %shl452, i8 %xor461
  %xor44116.1 = select i1 %cmp367.1, i8 0, i8 %xor59554
  %shl452.1 = shl i8 %xor59554, 1
  %cmp456.1 = icmp sgt i8 %xor59554, -1
  %xor461.1 = xor i8 %shl452.1, 27
  %xor59554.1 = select i1 %cmp456.1, i8 %shl452.1, i8 %xor461.1
  %xor44116.2 = select i1 %cmp367.2, i8 0, i8 %xor59554.1
  %shl452.2 = shl i8 %xor59554.1, 1
  %cmp456.2 = icmp sgt i8 %xor59554.1, -1
  %xor461.2 = xor i8 %shl452.2, 27
  %xor59554.2 = select i1 %cmp456.2, i8 %shl452.2, i8 %xor461.2
  %xor44116.3 = select i1 %cmp367.3, i8 0, i8 %xor59554.2
  %shl452.3 = shl i8 %xor59554.2, 1
  %cmp456.3 = icmp sgt i8 %xor59554.2, -1
  %xor461.3 = xor i8 %shl452.3, 27
  %xor59554.3 = select i1 %cmp456.3, i8 %shl452.3, i8 %xor461.3
  %xor44116.4 = select i1 %cmp367.4, i8 0, i8 %xor59554.3
  %shl452.4 = shl i8 %xor59554.3, 1
  %cmp456.4 = icmp sgt i8 %xor59554.3, -1
  %xor461.4 = xor i8 %shl452.4, 27
  %xor59554.4 = select i1 %cmp456.4, i8 %shl452.4, i8 %xor461.4
  %xor44116.5 = select i1 %cmp367.5, i8 0, i8 %xor59554.4
  %shl452.5 = shl i8 %xor59554.4, 1
  %cmp456.5 = icmp sgt i8 %xor59554.4, -1
  %xor461.5 = xor i8 %shl452.5, 27
  %xor59554.5 = select i1 %cmp456.5, i8 %shl452.5, i8 %xor461.5
  %xor44116.6 = select i1 %cmp367.6, i8 0, i8 %xor59554.5
  %shl452.6 = shl i8 %xor59554.5, 1
  %cmp456.6 = icmp sgt i8 %xor59554.5, -1
  %xor461.6 = xor i8 %shl452.6, 27
  %xor59554.6 = select i1 %cmp456.6, i8 %shl452.6, i8 %xor461.6
  %xor44116.7 = select i1 %cmp367.7, i8 0, i8 %xor59554.6
  %tmp477 = extractelement <4 x i8> %tmp342, i32 2
  %xor50813 = select i1 %cmp367, i8 0, i8 %tmp477
  %shl519 = shl i8 %tmp477, 1
  %cmp523 = icmp sgt i8 %tmp477, -1
  %xor528 = xor i8 %shl519, 27
  %xor59552 = select i1 %cmp523, i8 %shl519, i8 %xor528
  %xor50813.1 = select i1 %cmp367.1, i8 0, i8 %xor59552
  %tmp60636.xor50813.1 = xor i8 %xor50813.1, %xor50813
  %shl519.1 = shl i8 %xor59552, 1
  %cmp523.1 = icmp sgt i8 %xor59552, -1
  %xor528.1 = xor i8 %shl519.1, 27
  %xor59552.1 = select i1 %cmp523.1, i8 %shl519.1, i8 %xor528.1
  %xor50813.2 = select i1 %cmp367.2, i8 0, i8 %xor59552.1
  %tmp60636.xor50813.2 = xor i8 %xor50813.2, %tmp60636.xor50813.1
  %shl519.2 = shl i8 %xor59552.1, 1
  %cmp523.2 = icmp sgt i8 %xor59552.1, -1
  %xor528.2 = xor i8 %shl519.2, 27
  %xor59552.2 = select i1 %cmp523.2, i8 %shl519.2, i8 %xor528.2
  %xor50813.3 = select i1 %cmp367.3, i8 0, i8 %xor59552.2
  %tmp60636.xor50813.3 = xor i8 %xor50813.3, %tmp60636.xor50813.2
  %shl519.3 = shl i8 %xor59552.2, 1
  %cmp523.3 = icmp sgt i8 %xor59552.2, -1
  %xor528.3 = xor i8 %shl519.3, 27
  %xor59552.3 = select i1 %cmp523.3, i8 %shl519.3, i8 %xor528.3
  %xor50813.4 = select i1 %cmp367.4, i8 0, i8 %xor59552.3
  %tmp60636.xor50813.4 = xor i8 %xor50813.4, %tmp60636.xor50813.3
  %shl519.4 = shl i8 %xor59552.3, 1
  %cmp523.4 = icmp sgt i8 %xor59552.3, -1
  %xor528.4 = xor i8 %shl519.4, 27
  %xor59552.4 = select i1 %cmp523.4, i8 %shl519.4, i8 %xor528.4
  %xor50813.5 = select i1 %cmp367.5, i8 0, i8 %xor59552.4
  %tmp60636.xor50813.5 = xor i8 %xor50813.5, %tmp60636.xor50813.4
  %shl519.5 = shl i8 %xor59552.4, 1
  %cmp523.5 = icmp sgt i8 %xor59552.4, -1
  %xor528.5 = xor i8 %shl519.5, 27
  %xor59552.5 = select i1 %cmp523.5, i8 %shl519.5, i8 %xor528.5
  %xor50813.6 = select i1 %cmp367.6, i8 0, i8 %xor59552.5
  %tmp60636.xor50813.6 = xor i8 %xor50813.6, %tmp60636.xor50813.5
  %shl519.6 = shl i8 %xor59552.5, 1
  %cmp523.6 = icmp sgt i8 %xor59552.5, -1
  %xor528.6 = xor i8 %shl519.6, 27
  %xor59552.6 = select i1 %cmp523.6, i8 %shl519.6, i8 %xor528.6
  %xor50813.7 = select i1 %cmp367.7, i8 0, i8 %xor59552.6
  %tmp60636.xor50813.7 = xor i8 %xor50813.7, %tmp60636.xor50813.6
  %xor44116 = select i1 %cmp367, i8 0, i8 %tmp410
  %tmp60638.xor44116.1 = xor i8 %xor44116.1, %xor44116
  %tmp60638.xor44116.2 = xor i8 %xor44116.2, %tmp60638.xor44116.1
  %tmp60638.xor44116.3 = xor i8 %xor44116.3, %tmp60638.xor44116.2
  %tmp60638.xor44116.4 = xor i8 %xor44116.4, %tmp60638.xor44116.3
  %tmp60638.xor44116.5 = xor i8 %xor44116.5, %tmp60638.xor44116.4
  %tmp60638.xor44116.6 = xor i8 %xor44116.6, %tmp60638.xor44116.5
  %tmp544 = extractelement <4 x i8> %tmp342, i32 3
  %xor57510 = select i1 %cmp367, i8 0, i8 %tmp544
  %shl586 = shl i8 %tmp544, 1
  %cmp590 = icmp sgt i8 %tmp544, -1
  %xor595 = xor i8 %shl586, 27
  %xor59550 = select i1 %cmp590, i8 %shl586, i8 %xor595
  %xor57510.1 = select i1 %cmp367.1, i8 0, i8 %xor59550
  %tmp60634.xor57510.1 = xor i8 %xor57510.1, %xor57510
  %shl586.1 = shl i8 %xor59550, 1
  %cmp590.1 = icmp sgt i8 %xor59550, -1
  %xor595.1 = xor i8 %shl586.1, 27
  %xor59550.1 = select i1 %cmp590.1, i8 %shl586.1, i8 %xor595.1
  %xor57510.2 = select i1 %cmp367.2, i8 0, i8 %xor59550.1
  %tmp60634.xor57510.2 = xor i8 %xor57510.2, %tmp60634.xor57510.1
  %shl586.2 = shl i8 %xor59550.1, 1
  %cmp590.2 = icmp sgt i8 %xor59550.1, -1
  %xor595.2 = xor i8 %shl586.2, 27
  %xor59550.2 = select i1 %cmp590.2, i8 %shl586.2, i8 %xor595.2
  %xor57510.3 = select i1 %cmp367.3, i8 0, i8 %xor59550.2
  %tmp60634.xor57510.3 = xor i8 %xor57510.3, %tmp60634.xor57510.2
  %shl586.3 = shl i8 %xor59550.2, 1
  %cmp590.3 = icmp sgt i8 %xor59550.2, -1
  %xor595.3 = xor i8 %shl586.3, 27
  %xor59550.3 = select i1 %cmp590.3, i8 %shl586.3, i8 %xor595.3
  %xor57510.4 = select i1 %cmp367.4, i8 0, i8 %xor59550.3
  %tmp60634.xor57510.4 = xor i8 %xor57510.4, %tmp60634.xor57510.3
  %shl586.4 = shl i8 %xor59550.3, 1
  %cmp590.4 = icmp sgt i8 %xor59550.3, -1
  %xor595.4 = xor i8 %shl586.4, 27
  %xor59550.4 = select i1 %cmp590.4, i8 %shl586.4, i8 %xor595.4
  %xor57510.5 = select i1 %cmp367.5, i8 0, i8 %xor59550.4
  %tmp60634.xor57510.5 = xor i8 %xor57510.5, %tmp60634.xor57510.4
  %shl586.5 = shl i8 %xor59550.4, 1
  %cmp590.5 = icmp sgt i8 %xor59550.4, -1
  %xor595.5 = xor i8 %shl586.5, 27
  %xor59550.5 = select i1 %cmp590.5, i8 %shl586.5, i8 %xor595.5
  %xor57510.6 = select i1 %cmp367.6, i8 0, i8 %xor59550.5
  %tmp60634.xor57510.6 = xor i8 %xor57510.6, %tmp60634.xor57510.5
  %shl586.6 = shl i8 %xor59550.5, 1
  %cmp590.6 = icmp sgt i8 %xor59550.5, -1
  %xor595.6 = xor i8 %shl586.6, 27
  %xor59550.6 = select i1 %cmp590.6, i8 %shl586.6, i8 %xor595.6
  %xor57510.7 = select i1 %cmp367.7, i8 0, i8 %xor59550.6
  %tmp60634.xor57510.7 = xor i8 %xor57510.7, %tmp60634.xor57510.6
  %tmp60638.xor44116.7 = xor i8 %xor44116.7, %tmp60638.xor44116.6
  %xor6113 = xor i8 %tmp60640.xor37419.7, %tmp63973113
  %xor6174 = xor i8 %tmp60638.xor44116.7, %tmp64074112
  %xor6235 = xor i8 %tmp60636.xor50813.7, %tmp64175111
  %xor6296 = xor i8 %tmp60634.xor57510.7, %tmp64276110
  %inc633 = add i32 %storemerge7114, 1
  %cmp332 = icmp ult i32 %inc633, 4
  br i1 %cmp332, label %for.body334, label %for.end634

for.end634:                                       ; preds = %for.body334
  %5 = insertelement <4 x i8> undef, i8 %xor6113, i32 0
  %6 = insertelement <4 x i8> %5, i8 %xor6174, i32 1
  %7 = insertelement <4 x i8> %6, i8 %xor6235, i32 2
  %8 = insertelement <4 x i8> %7, i8 %xor6296, i32 3
  br label %for.cond

for.end646:                                       ; preds = %shiftRowsInv.exit
  %arrayidx667 = getelementptr <4 x i8> addrspace(1)* %output, i32 %add8
  %arrayidx674 = getelementptr <4 x i8> addrspace(1)* %roundKey, i32 %call3
  %tmp675 = load <4 x i8> addrspace(1)* %arrayidx674, align 4
  %xor676 = xor <4 x i8> %4, %tmp675
  store <4 x i8> %xor676, <4 x i8> addrspace(1)* %arrayidx667, align 4
  ret void
}
