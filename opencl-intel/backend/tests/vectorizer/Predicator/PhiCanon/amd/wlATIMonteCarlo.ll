; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIMonteCarlo.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [7 x i8] c"200222\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @lshift128
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %LeafBlock29, %LeafBlock
; CHECK: phi-split-bb1:                                    ; preds = %LeafBlock31, %phi-split-bb
; CHECK: phi-split-bb2:                                    ; preds = %sw.bb87, %sw.bb94
; CHECK: phi-split-bb6:                                    ; preds = %sw.bb, %sw.bb82
; CHECK: phi-split-bb11:                                   ; preds = %phi-split-bb2, %phi-split-bb6
; CHECK: ret

; CHECK: @calPriceVega
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %LeafBlock24, %LeafBlock
; CHECK: phi-split-bb1:                                    ; preds = %LeafBlock26, %phi-split-bb
; CHECK: phi-split-bb2:                                    ; preds = %sw.bb87.i, %sw.bb94.i
; CHECK: phi-split-bb6:                                    ; preds = %sw.bb.i, %sw.bb82.i
; CHECK: phi-split-bb11:                                   ; preds = %phi-split-bb2, %phi-split-bb6
; CHECK: ret

define void @lshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
  %tmp3 = extractelement <4 x i32> %input, i32 0
  %and = and i32 %shift, 31
  %shl = shl i32 %tmp3, %and
  %tmp6 = insertelement <4 x i32> undef, i32 %shl, i32 0
  %tmp8 = extractelement <4 x i32> %input, i32 1
  %shl11 = shl i32 %tmp8, %and
  %0 = sub i32 0, %shift
  %and15 = and i32 %0, 31
  %shr = lshr i32 %tmp3, %and15
  %or = or i32 %shl11, %shr
  %tmp17 = insertelement <4 x i32> %tmp6, i32 %or, i32 1
  %tmp19 = extractelement <4 x i32> %input, i32 2
  %shl22 = shl i32 %tmp19, %and
  %shr27 = lshr i32 %tmp8, %and15
  %or28 = or i32 %shl22, %shr27
  %tmp30 = insertelement <4 x i32> %tmp17, i32 %or28, i32 2
  %tmp32 = extractelement <4 x i32> %input, i32 3
  %shl35 = shl i32 %tmp32, %and
  %shr40 = lshr i32 %tmp19, %and15
  %or41 = or i32 %shl35, %shr40
  %tmp43 = insertelement <4 x i32> %tmp30, i32 %or41, i32 3
  store <4 x i32> %tmp43, <4 x i32>* %output, align 16
  ret void
}

define void @rshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
  %tmp3 = extractelement <4 x i32> %input, i32 3
  %and = and i32 %shift, 31
  %shr = lshr i32 %tmp3, %and
  %tmp6 = insertelement <4 x i32> undef, i32 %shr, i32 3
  %tmp8 = extractelement <4 x i32> %input, i32 2
  %shr11 = lshr i32 %tmp8, %and
  %0 = sub i32 0, %shift
  %and15 = and i32 %0, 31
  %shl = shl i32 %tmp3, %and15
  %or = or i32 %shr11, %shl
  %tmp17 = insertelement <4 x i32> %tmp6, i32 %or, i32 2
  %tmp19 = extractelement <4 x i32> %input, i32 1
  %shr22 = lshr i32 %tmp19, %and
  %shl27 = shl i32 %tmp8, %and15
  %or28 = or i32 %shr22, %shl27
  %tmp30 = insertelement <4 x i32> %tmp17, i32 %or28, i32 1
  %tmp32 = extractelement <4 x i32> %input, i32 0
  %shr35 = lshr i32 %tmp32, %and
  %shl40 = shl i32 %tmp19, %and15
  %or41 = or i32 %shr35, %shl40
  %tmp43 = insertelement <4 x i32> %tmp30, i32 %or41, i32 0
  store <4 x i32> %tmp43, <4 x i32>* %output, align 16
  ret void
}

define void @generateRand(<4 x i32> %seed, <4 x float>* %gaussianRand1, <4 x float>* %gaussianRand2, <4 x i32>* %nextRand) nounwind {
entry:
  %temp = alloca [8 x <4 x i32>], align 16
  %shr = lshr <4 x i32> %seed, <i32 30, i32 30, i32 30, i32 30>
  %xor = xor <4 x i32> %shr, %seed
  %mul = mul <4 x i32> %xor, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add = add <4 x i32> %mul, <i32 1, i32 1, i32 1, i32 1>
  %shr49 = lshr <4 x i32> %add, <i32 30, i32 30, i32 30, i32 30>
  %xor50 = xor <4 x i32> %add, %shr49
  %mul51 = mul <4 x i32> %xor50, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add53 = add <4 x i32> %mul51, <i32 2, i32 2, i32 2, i32 2>
  %shr59 = lshr <4 x i32> %add53, <i32 30, i32 30, i32 30, i32 30>
  %xor60 = xor <4 x i32> %add53, %shr59
  %mul61 = mul <4 x i32> %xor60, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add63 = add <4 x i32> %mul61, <i32 3, i32 3, i32 3, i32 3>
  %shr69 = lshr <4 x i32> %add63, <i32 30, i32 30, i32 30, i32 30>
  %xor70 = xor <4 x i32> %add63, %shr69
  %mul71 = mul <4 x i32> %xor70, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add73 = add <4 x i32> %mul71, <i32 4, i32 4, i32 4, i32 4>
  %arraydecay = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0
  %arrayidx90 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 1
  %arrayidx97 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 2
  br label %for.body

for.body:                                         ; preds = %entry, %sw.epilog
  %storemerge28 = phi i32 [ 0, %entry ], [ %inc, %sw.epilog ]
  %tmp1031627 = phi <4 x i32> [ zeroinitializer, %entry ], [ %tmp10317, %sw.epilog ]
  %tmp2041826 = phi <4 x i32> [ zeroinitializer, %entry ], [ %tmp20419, %sw.epilog ]
  %tmp1882025 = phi <4 x i32> [ zeroinitializer, %entry ], [ %tmp18821, %sw.epilog ]
  %tmp1932224 = phi <4 x i32> [ zeroinitializer, %entry ], [ %tmp19323, %sw.epilog ]
  br label %NodeBlock37

NodeBlock37:                                      ; preds = %for.body
  %Pivot38 = icmp slt i32 %storemerge28, 2
  br i1 %Pivot38, label %NodeBlock, label %NodeBlock35

NodeBlock35:                                      ; preds = %NodeBlock37
  %Pivot36 = icmp slt i32 %storemerge28, 3
  br i1 %Pivot36, label %LeafBlock31, label %LeafBlock33

LeafBlock33:                                      ; preds = %NodeBlock35
  %SwitchLeaf34 = icmp eq i32 %storemerge28, 3
  br i1 %SwitchLeaf34, label %sw.bb94, label %NewDefault

LeafBlock31:                                      ; preds = %NodeBlock35
  %SwitchLeaf32 = icmp eq i32 %storemerge28, 2
  br i1 %SwitchLeaf32, label %sw.bb87, label %NewDefault

NodeBlock:                                        ; preds = %NodeBlock37
  %Pivot = icmp slt i32 %storemerge28, 1
  br i1 %Pivot, label %LeafBlock, label %LeafBlock29

LeafBlock29:                                      ; preds = %NodeBlock
  %SwitchLeaf30 = icmp eq i32 %storemerge28, 1
  br i1 %SwitchLeaf30, label %sw.bb82, label %NewDefault

LeafBlock:                                        ; preds = %NodeBlock
  %SwitchLeaf = icmp eq i32 %storemerge28, 0
  br i1 %SwitchLeaf, label %sw.bb, label %NewDefault

sw.bb:                                            ; preds = %LeafBlock
  br label %sw.epilog

sw.bb82:                                          ; preds = %LeafBlock29
  %tmp84 = load <4 x i32>* %arraydecay, align 16
  br label %sw.epilog

sw.bb87:                                          ; preds = %LeafBlock31
  %tmp91 = load <4 x i32>* %arrayidx90, align 16
  br label %sw.epilog

sw.bb94:                                          ; preds = %LeafBlock33
  %tmp98 = load <4 x i32>* %arrayidx97, align 16
  br label %sw.epilog

NewDefault:                                       ; preds = %LeafBlock33, %LeafBlock31, %LeafBlock29, %LeafBlock
  br label %sw.epilog

sw.epilog:                                        ; preds = %NewDefault, %sw.bb94, %sw.bb87, %sw.bb82, %sw.bb
  %tmp19323 = phi <4 x i32> [ %seed, %sw.bb94 ], [ %add73, %sw.bb87 ], [ %add63, %sw.bb82 ], [ %add53, %sw.bb ], [ %tmp1932224, %NewDefault ]
  %tmp18821 = phi <4 x i32> [ %add63, %sw.bb94 ], [ %add53, %sw.bb87 ], [ %add, %sw.bb82 ], [ %seed, %sw.bb ], [ %tmp1882025, %NewDefault ]
  %tmp20419 = phi <4 x i32> [ %tmp98, %sw.bb94 ], [ %tmp91, %sw.bb87 ], [ %tmp84, %sw.bb82 ], [ %add73, %sw.bb ], [ %tmp2041826, %NewDefault ]
  %tmp10317 = phi <4 x i32> [ %tmp2041826, %sw.bb94 ], [ %tmp2041826, %sw.bb87 ], [ %tmp2041826, %sw.bb82 ], [ %add63, %sw.bb ], [ %tmp1031627, %NewDefault ]
  %tmp3.i = extractelement <4 x i32> %tmp18821, i32 0
  %shl.i = shl i32 %tmp3.i, 24
  %tmp8.i = extractelement <4 x i32> %tmp18821, i32 1
  %shl11.i = shl i32 %tmp8.i, 24
  %shr.i = lshr i32 %tmp3.i, 8
  %or.i = or i32 %shl11.i, %shr.i
  %tmp19.i = extractelement <4 x i32> %tmp18821, i32 2
  %shl22.i = shl i32 %tmp19.i, 24
  %shr27.i = lshr i32 %tmp8.i, 8
  %or28.i = or i32 %shl22.i, %shr27.i
  %tmp32.i = extractelement <4 x i32> %tmp18821, i32 3
  %shl35.i = shl i32 %tmp32.i, 24
  %shr40.i = lshr i32 %tmp19.i, 8
  %or41.i = or i32 %shl35.i, %shr40.i
  %tmp3.i1 = extractelement <4 x i32> %tmp10317, i32 3
  %shr.i3 = lshr i32 %tmp3.i1, 24
  %tmp8.i5 = extractelement <4 x i32> %tmp10317, i32 2
  %shr11.i = lshr i32 %tmp8.i5, 24
  %shl.i7 = shl i32 %tmp3.i1, 8
  %or.i8 = or i32 %shr11.i, %shl.i7
  %tmp19.i10 = extractelement <4 x i32> %tmp10317, i32 1
  %shr22.i = lshr i32 %tmp19.i10, 24
  %shl27.i = shl i32 %tmp8.i5, 8
  %or28.i11 = or i32 %shr22.i, %shl27.i
  %tmp32.i13 = extractelement <4 x i32> %tmp10317, i32 0
  %shr35.i = lshr i32 %tmp32.i13, 24
  %shl40.i = shl i32 %tmp19.i10, 8
  %or41.i14 = or i32 %shr35.i, %shl40.i
  %arrayidx107 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 %storemerge28
  %tmp114 = extractelement <4 x i32> %tmp19323, i32 0
  %shr117 = lshr i32 %tmp114, 13
  %and119 = and i32 %shr117, 473087
  %tmp125 = extractelement <4 x i32> %tmp20419, i32 0
  %shl = shl i32 %tmp125, 15
  %xor112 = xor i32 %and119, %tmp3.i
  %xor120 = xor i32 %xor112, %shl.i
  %xor123 = xor i32 %xor120, %shl
  %xor128 = xor i32 %xor123, %or41.i14
  %tmp130 = insertelement <4 x i32> undef, i32 %xor128, i32 0
  %tmp140 = extractelement <4 x i32> %tmp19323, i32 1
  %shr143 = lshr i32 %tmp140, 13
  %and145 = and i32 %shr143, 475005
  %tmp151 = extractelement <4 x i32> %tmp20419, i32 1
  %shl154 = shl i32 %tmp151, 15
  %xor138 = xor i32 %and145, %tmp8.i
  %xor146 = xor i32 %xor138, %shl154
  %xor149 = xor i32 %xor146, %or.i
  %xor155 = xor i32 %xor149, %or28.i11
  %tmp157 = insertelement <4 x i32> %tmp130, i32 %xor155, i32 1
  %tmp167 = extractelement <4 x i32> %tmp19323, i32 2
  %shr170 = lshr i32 %tmp167, 13
  %and172 = and i32 %shr170, 490365
  %tmp178 = extractelement <4 x i32> %tmp20419, i32 2
  %shl181 = shl i32 %tmp178, 15
  %xor165 = xor i32 %and172, %tmp19.i
  %xor173 = xor i32 %xor165, %shl181
  %xor176 = xor i32 %xor173, %or28.i
  %xor182 = xor i32 %xor176, %or.i8
  %tmp184 = insertelement <4 x i32> %tmp157, i32 %xor182, i32 2
  %tmp194 = extractelement <4 x i32> %tmp19323, i32 3
  %shr197 = lshr i32 %tmp194, 13
  %and199 = and i32 %shr197, 523055
  %tmp205 = extractelement <4 x i32> %tmp20419, i32 3
  %shl208 = shl i32 %tmp205, 15
  %xor192 = xor i32 %and199, %tmp32.i
  %xor200 = xor i32 %xor192, %shl208
  %xor203 = xor i32 %xor200, %or41.i
  %xor209 = xor i32 %xor203, %shr.i3
  %tmp211 = insertelement <4 x i32> %tmp184, i32 %xor209, i32 3
  store <4 x i32> %tmp211, <4 x i32>* %arrayidx107, align 16
  %inc = add i32 %storemerge28, 1
  %cmp = icmp ult i32 %inc, 4
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %sw.epilog
  %tmp216 = load <4 x i32>* %arraydecay, align 16
  %tmp217 = extractelement <4 x i32> %tmp216, i32 0
  %conv = uitofp i32 %tmp217 to float
  %0 = insertelement <4 x float> undef, float %conv, i32 0
  %tmp221 = extractelement <4 x i32> %tmp216, i32 1
  %conv222 = uitofp i32 %tmp221 to float
  %1 = insertelement <4 x float> %0, float %conv222, i32 1
  %tmp226 = extractelement <4 x i32> %tmp216, i32 2
  %conv227 = uitofp i32 %tmp226 to float
  %2 = insertelement <4 x float> %1, float %conv227, i32 2
  %tmp231 = extractelement <4 x i32> %tmp216, i32 3
  %conv232 = uitofp i32 %tmp231 to float
  %3 = insertelement <4 x float> %2, float %conv232, i32 3
  %div = fdiv <4 x float> %3, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %tmp244 = load <4 x i32>* %arrayidx90, align 16
  %tmp245 = extractelement <4 x i32> %tmp244, i32 0
  %conv246 = uitofp i32 %tmp245 to float
  %4 = insertelement <4 x float> undef, float %conv246, i32 0
  %tmp250 = extractelement <4 x i32> %tmp244, i32 1
  %conv251 = uitofp i32 %tmp250 to float
  %5 = insertelement <4 x float> %4, float %conv251, i32 1
  %tmp255 = extractelement <4 x i32> %tmp244, i32 2
  %conv256 = uitofp i32 %tmp255 to float
  %6 = insertelement <4 x float> %5, float %conv256, i32 2
  %tmp260 = extractelement <4 x i32> %tmp244, i32 3
  %conv261 = uitofp i32 %tmp260 to float
  %7 = insertelement <4 x float> %6, float %conv261, i32 3
  %div270 = fdiv <4 x float> %7, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %call = call <4 x float> @__logf4(<4 x float> %div) nounwind
  %mul275 = fmul <4 x float> %call, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
  %call276 = call <4 x float> @__sqrtf4(<4 x float> %mul275) nounwind
  %mul283 = fmul <4 x float> %div270, <float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000>
  %call287 = call <4 x float> @__cosf4(<4 x float> %mul283) nounwind
  %mul288 = fmul <4 x float> %call276, %call287
  store <4 x float> %mul288, <4 x float>* %gaussianRand1, align 16
  %call292 = call <4 x float> @__sinf4(<4 x float> %mul283) nounwind
  %mul293 = fmul <4 x float> %call276, %call292
  store <4 x float> %mul293, <4 x float>* %gaussianRand2, align 16
  %tmp297 = load <4 x i32>* %arrayidx97, align 16
  store <4 x i32> %tmp297, <4 x i32>* %nextRand, align 16
  ret void
}

declare <4 x float> @__sqrtf4(<4 x float>)

declare <4 x float> @__logf4(<4 x float>)

declare <4 x float> @__cosf4(<4 x float>)

declare <4 x float> @__sinf4(<4 x float>)

define void @calOutputs(<4 x float> %strikePrice, <4 x float> %meanDeriv1, <4 x float> %meanDeriv2, <4 x float> %meanPrice1, <4 x float> %meanPrice2, <4 x float>* %pathDeriv1, <4 x float>* %pathDeriv2, <4 x float>* %priceVec1, <4 x float>* %priceVec2) nounwind {
entry:
  %sub = fsub <4 x float> %meanPrice1, %strikePrice
  %sub5 = fsub <4 x float> %meanPrice2, %strikePrice
  %tmp7 = extractelement <4 x float> %sub, i32 0
  %cmp = fcmp ogt float %tmp7, 0.000000e+00
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp13 = shufflevector <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <4 x float> %sub, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp9912 = phi <4 x float> [ %tmp13, %if.then ], [ zeroinitializer, %entry ]
  %tmp934 = phi <4 x float> [ <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, %if.then ], [ zeroinitializer, %entry ]
  %tmp15 = extractelement <4 x float> %sub, i32 1
  %cmp16 = fcmp ogt float %tmp15, 0.000000e+00
  br i1 %cmp16, label %if.then17, label %if.end24

if.then17:                                        ; preds = %if.end
  %tmp19 = insertelement <4 x float> %tmp934, float 1.000000e+00, i32 1
  %tmp23 = shufflevector <4 x float> %tmp9912, <4 x float> %sub, <4 x i32> <i32 0, i32 5, i32 2, i32 3>
  br label %if.end24

if.end24:                                         ; preds = %if.then17, %if.end
  %tmp9911 = phi <4 x float> [ %tmp23, %if.then17 ], [ %tmp9912, %if.end ]
  %tmp933 = phi <4 x float> [ %tmp19, %if.then17 ], [ %tmp934, %if.end ]
  %tmp26 = extractelement <4 x float> %sub, i32 2
  %cmp27 = fcmp ogt float %tmp26, 0.000000e+00
  br i1 %cmp27, label %if.then28, label %if.end35

if.then28:                                        ; preds = %if.end24
  %tmp30 = insertelement <4 x float> %tmp933, float 1.000000e+00, i32 2
  %tmp34 = shufflevector <4 x float> %tmp9911, <4 x float> %sub, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  br label %if.end35

if.end35:                                         ; preds = %if.then28, %if.end24
  %tmp9910 = phi <4 x float> [ %tmp34, %if.then28 ], [ %tmp9911, %if.end24 ]
  %tmp932 = phi <4 x float> [ %tmp30, %if.then28 ], [ %tmp933, %if.end24 ]
  %tmp37 = extractelement <4 x float> %sub, i32 3
  %cmp38 = fcmp ogt float %tmp37, 0.000000e+00
  br i1 %cmp38, label %if.then39, label %if.end46

if.then39:                                        ; preds = %if.end35
  %tmp41 = insertelement <4 x float> %tmp932, float 1.000000e+00, i32 3
  %tmp45 = shufflevector <4 x float> %tmp9910, <4 x float> %sub, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  br label %if.end46

if.end46:                                         ; preds = %if.then39, %if.end35
  %tmp999 = phi <4 x float> [ %tmp45, %if.then39 ], [ %tmp9910, %if.end35 ]
  %tmp931 = phi <4 x float> [ %tmp41, %if.then39 ], [ %tmp932, %if.end35 ]
  %tmp48 = extractelement <4 x float> %sub5, i32 0
  %cmp49 = fcmp ogt float %tmp48, 0.000000e+00
  br i1 %cmp49, label %if.then50, label %if.end57

if.then50:                                        ; preds = %if.end46
  %tmp56 = shufflevector <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <4 x float> %sub5, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
  br label %if.end57

if.end57:                                         ; preds = %if.then50, %if.end46
  %tmp10116 = phi <4 x float> [ %tmp56, %if.then50 ], [ zeroinitializer, %if.end46 ]
  %tmp968 = phi <4 x float> [ <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, %if.then50 ], [ zeroinitializer, %if.end46 ]
  %tmp59 = extractelement <4 x float> %sub5, i32 1
  %cmp60 = fcmp ogt float %tmp59, 0.000000e+00
  br i1 %cmp60, label %if.then61, label %if.end68

if.then61:                                        ; preds = %if.end57
  %tmp63 = insertelement <4 x float> %tmp968, float 1.000000e+00, i32 1
  %tmp67 = shufflevector <4 x float> %tmp10116, <4 x float> %sub5, <4 x i32> <i32 0, i32 5, i32 2, i32 3>
  br label %if.end68

if.end68:                                         ; preds = %if.then61, %if.end57
  %tmp10115 = phi <4 x float> [ %tmp67, %if.then61 ], [ %tmp10116, %if.end57 ]
  %tmp967 = phi <4 x float> [ %tmp63, %if.then61 ], [ %tmp968, %if.end57 ]
  %tmp70 = extractelement <4 x float> %sub5, i32 2
  %cmp71 = fcmp ogt float %tmp70, 0.000000e+00
  br i1 %cmp71, label %if.then72, label %if.end79

if.then72:                                        ; preds = %if.end68
  %tmp74 = insertelement <4 x float> %tmp967, float 1.000000e+00, i32 2
  %tmp78 = shufflevector <4 x float> %tmp10115, <4 x float> %sub5, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  br label %if.end79

if.end79:                                         ; preds = %if.then72, %if.end68
  %tmp10114 = phi <4 x float> [ %tmp78, %if.then72 ], [ %tmp10115, %if.end68 ]
  %tmp966 = phi <4 x float> [ %tmp74, %if.then72 ], [ %tmp967, %if.end68 ]
  %tmp81 = extractelement <4 x float> %sub5, i32 3
  %cmp82 = fcmp ogt float %tmp81, 0.000000e+00
  br i1 %cmp82, label %if.then83, label %if.end90

if.then83:                                        ; preds = %if.end79
  %tmp85 = insertelement <4 x float> %tmp966, float 1.000000e+00, i32 3
  %tmp89 = shufflevector <4 x float> %tmp10114, <4 x float> %sub5, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  br label %if.end90

if.end90:                                         ; preds = %if.then83, %if.end79
  %tmp10113 = phi <4 x float> [ %tmp89, %if.then83 ], [ %tmp10114, %if.end79 ]
  %tmp965 = phi <4 x float> [ %tmp85, %if.then83 ], [ %tmp966, %if.end79 ]
  %mul = fmul <4 x float> %tmp931, %meanDeriv1
  store <4 x float> %mul, <4 x float>* %pathDeriv1, align 16
  %mul97 = fmul <4 x float> %tmp965, %meanDeriv2
  store <4 x float> %mul97, <4 x float>* %pathDeriv2, align 16
  store <4 x float> %tmp999, <4 x float>* %priceVec1, align 16
  store <4 x float> %tmp10113, <4 x float>* %priceVec2, align 16
  ret void
}

define void @calPriceVega(<4 x float> addrspace(1)* %attrib, i32 %noOfSum, i32 %width, <4 x i32> addrspace(1)* %randArray, <4 x float> addrspace(1)* %priceSamples, <4 x float> addrspace(1)* %pathDeriv, ...) nounwind {
entry:
  %temp.i = alloca [8 x <4 x i32>], align 16
  %tmp1 = load <4 x float> addrspace(1)* %attrib, align 16
  %arrayidx4 = getelementptr <4 x float> addrspace(1)* %attrib, i32 1
  %tmp5 = load <4 x float> addrspace(1)* %arrayidx4, align 16
  %arrayidx8 = getelementptr <4 x float> addrspace(1)* %attrib, i32 2
  %tmp9 = load <4 x float> addrspace(1)* %arrayidx8, align 16
  %arrayidx12 = getelementptr <4 x float> addrspace(1)* %attrib, i32 3
  %tmp13 = load <4 x float> addrspace(1)* %arrayidx12, align 16
  %arrayidx16 = getelementptr <4 x float> addrspace(1)* %attrib, i32 4
  %tmp17 = load <4 x float> addrspace(1)* %arrayidx16, align 16
  %arrayidx20 = getelementptr <4 x float> addrspace(1)* %attrib, i32 5
  %tmp21 = load <4 x float> addrspace(1)* %arrayidx20, align 16
  %arrayidx24 = getelementptr <4 x float> addrspace(1)* %attrib, i32 6
  %tmp25 = load <4 x float> addrspace(1)* %arrayidx24, align 16
  %call = call i32 @get_global_id(i32 0) nounwind
  %call28 = call i32 @get_global_id(i32 1) nounwind
  %mul = mul i32 %call28, %width
  %add = add i32 %mul, %call
  %cmp12 = icmp sgt i32 %noOfSum, 1
  br i1 %cmp12, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %arrayidx63 = getelementptr <4 x i32> addrspace(1)* %randArray, i32 %add
  %tmp64 = load <4 x i32> addrspace(1)* %arrayidx63, align 16
  %0 = bitcast [8 x <4 x i32>]* %temp.i to i8*
  %arraydecay.i = getelementptr [8 x <4 x i32>]* %temp.i, i32 0, i32 0
  %arrayidx90.i = getelementptr [8 x <4 x i32>]* %temp.i, i32 0, i32 1
  %arrayidx97.i = getelementptr [8 x <4 x i32>]* %temp.i, i32 0, i32 2
  %mul95 = fmul <4 x float> %tmp13, %tmp25
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %generateRand.exit
  %storemerge20 = phi i32 [ 1, %for.body.lr.ph ], [ %inc, %generateRand.exit ]
  %tmp100519 = phi <4 x float> [ %tmp17, %for.body.lr.ph ], [ %mul78, %generateRand.exit ]
  %tmp110618 = phi <4 x float> [ %tmp17, %for.body.lr.ph ], [ %mul86, %generateRand.exit ]
  %tmp122717 = phi <4 x float> [ %tmp17, %for.body.lr.ph ], [ %add89, %generateRand.exit ]
  %tmp128816 = phi <4 x float> [ %tmp17, %for.body.lr.ph ], [ %add92, %generateRand.exit ]
  %tmp134915 = phi <4 x float> [ zeroinitializer, %for.body.lr.ph ], [ %add108, %generateRand.exit ]
  %tmp1401014 = phi <4 x float> [ zeroinitializer, %for.body.lr.ph ], [ %add120, %generateRand.exit ]
  %tmp297.i1113 = phi <4 x i32> [ %tmp64, %for.body.lr.ph ], [ %tmp297.i, %generateRand.exit ]
  call void @llvm.lifetime.start(i64 -1, i8* %0) nounwind
  %shr.i = lshr <4 x i32> %tmp297.i1113, <i32 30, i32 30, i32 30, i32 30>
  %xor.i = xor <4 x i32> %shr.i, %tmp297.i1113
  %mul.i = mul <4 x i32> %xor.i, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add.i = add <4 x i32> %mul.i, <i32 1, i32 1, i32 1, i32 1>
  %shr49.i = lshr <4 x i32> %add.i, <i32 30, i32 30, i32 30, i32 30>
  %xor50.i = xor <4 x i32> %add.i, %shr49.i
  %mul51.i = mul <4 x i32> %xor50.i, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add53.i = add <4 x i32> %mul51.i, <i32 2, i32 2, i32 2, i32 2>
  %shr59.i = lshr <4 x i32> %add53.i, <i32 30, i32 30, i32 30, i32 30>
  %xor60.i = xor <4 x i32> %add53.i, %shr59.i
  %mul61.i = mul <4 x i32> %xor60.i, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add63.i = add <4 x i32> %mul61.i, <i32 3, i32 3, i32 3, i32 3>
  %shr69.i = lshr <4 x i32> %add63.i, <i32 30, i32 30, i32 30, i32 30>
  %xor70.i = xor <4 x i32> %add63.i, %shr69.i
  %mul71.i = mul <4 x i32> %xor70.i, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add73.i = add <4 x i32> %mul71.i, <i32 4, i32 4, i32 4, i32 4>
  br label %for.body.i

for.body.i:                                       ; preds = %sw.epilog.i, %for.body
  %storemerge28.i = phi i32 [ 0, %for.body ], [ %inc.i, %sw.epilog.i ]
  %tmp1031627.i = phi <4 x i32> [ zeroinitializer, %for.body ], [ %tmp10317.i, %sw.epilog.i ]
  %tmp2041826.i = phi <4 x i32> [ zeroinitializer, %for.body ], [ %tmp20419.i, %sw.epilog.i ]
  %tmp1882025.i = phi <4 x i32> [ zeroinitializer, %for.body ], [ %tmp18821.i, %sw.epilog.i ]
  %tmp1932224.i = phi <4 x i32> [ zeroinitializer, %for.body ], [ %tmp19323.i, %sw.epilog.i ]
  br label %NodeBlock32

NodeBlock32:                                      ; preds = %for.body.i
  %Pivot33 = icmp slt i32 %storemerge28.i, 2
  br i1 %Pivot33, label %NodeBlock, label %NodeBlock30

NodeBlock30:                                      ; preds = %NodeBlock32
  %Pivot31 = icmp slt i32 %storemerge28.i, 3
  br i1 %Pivot31, label %LeafBlock26, label %LeafBlock28

LeafBlock28:                                      ; preds = %NodeBlock30
  %SwitchLeaf29 = icmp eq i32 %storemerge28.i, 3
  br i1 %SwitchLeaf29, label %sw.bb94.i, label %NewDefault

LeafBlock26:                                      ; preds = %NodeBlock30
  %SwitchLeaf27 = icmp eq i32 %storemerge28.i, 2
  br i1 %SwitchLeaf27, label %sw.bb87.i, label %NewDefault

NodeBlock:                                        ; preds = %NodeBlock32
  %Pivot = icmp slt i32 %storemerge28.i, 1
  br i1 %Pivot, label %LeafBlock, label %LeafBlock24

LeafBlock24:                                      ; preds = %NodeBlock
  %SwitchLeaf25 = icmp eq i32 %storemerge28.i, 1
  br i1 %SwitchLeaf25, label %sw.bb82.i, label %NewDefault

LeafBlock:                                        ; preds = %NodeBlock
  %SwitchLeaf = icmp eq i32 %storemerge28.i, 0
  br i1 %SwitchLeaf, label %sw.bb.i, label %NewDefault

sw.bb.i:                                          ; preds = %LeafBlock
  br label %sw.epilog.i

sw.bb82.i:                                        ; preds = %LeafBlock24
  %tmp84.i = load <4 x i32>* %arraydecay.i, align 16
  br label %sw.epilog.i

sw.bb87.i:                                        ; preds = %LeafBlock26
  %tmp91.i = load <4 x i32>* %arrayidx90.i, align 16
  br label %sw.epilog.i

sw.bb94.i:                                        ; preds = %LeafBlock28
  %tmp98.i = load <4 x i32>* %arrayidx97.i, align 16
  br label %sw.epilog.i

NewDefault:                                       ; preds = %LeafBlock28, %LeafBlock26, %LeafBlock24, %LeafBlock
  br label %sw.epilog.i

sw.epilog.i:                                      ; preds = %NewDefault, %sw.bb94.i, %sw.bb87.i, %sw.bb82.i, %sw.bb.i
  %tmp19323.i = phi <4 x i32> [ %tmp297.i1113, %sw.bb94.i ], [ %add73.i, %sw.bb87.i ], [ %add63.i, %sw.bb82.i ], [ %add53.i, %sw.bb.i ], [ %tmp1932224.i, %NewDefault ]
  %tmp18821.i = phi <4 x i32> [ %add63.i, %sw.bb94.i ], [ %add53.i, %sw.bb87.i ], [ %add.i, %sw.bb82.i ], [ %tmp297.i1113, %sw.bb.i ], [ %tmp1882025.i, %NewDefault ]
  %tmp20419.i = phi <4 x i32> [ %tmp98.i, %sw.bb94.i ], [ %tmp91.i, %sw.bb87.i ], [ %tmp84.i, %sw.bb82.i ], [ %add73.i, %sw.bb.i ], [ %tmp2041826.i, %NewDefault ]
  %tmp10317.i = phi <4 x i32> [ %tmp2041826.i, %sw.bb94.i ], [ %tmp2041826.i, %sw.bb87.i ], [ %tmp2041826.i, %sw.bb82.i ], [ %add63.i, %sw.bb.i ], [ %tmp1031627.i, %NewDefault ]
  %tmp3.i.i = extractelement <4 x i32> %tmp18821.i, i32 0
  %shl.i.i = shl i32 %tmp3.i.i, 24
  %tmp8.i.i = extractelement <4 x i32> %tmp18821.i, i32 1
  %shl11.i.i = shl i32 %tmp8.i.i, 24
  %shr.i.i = lshr i32 %tmp3.i.i, 8
  %or.i.i = or i32 %shl11.i.i, %shr.i.i
  %tmp19.i.i = extractelement <4 x i32> %tmp18821.i, i32 2
  %shl22.i.i = shl i32 %tmp19.i.i, 24
  %shr27.i.i = lshr i32 %tmp8.i.i, 8
  %or28.i.i = or i32 %shl22.i.i, %shr27.i.i
  %tmp32.i.i = extractelement <4 x i32> %tmp18821.i, i32 3
  %shl35.i.i = shl i32 %tmp32.i.i, 24
  %shr40.i.i = lshr i32 %tmp19.i.i, 8
  %or41.i.i = or i32 %shl35.i.i, %shr40.i.i
  %tmp3.i1.i = extractelement <4 x i32> %tmp10317.i, i32 3
  %shr.i3.i = lshr i32 %tmp3.i1.i, 24
  %tmp8.i5.i = extractelement <4 x i32> %tmp10317.i, i32 2
  %shr11.i.i = lshr i32 %tmp8.i5.i, 24
  %shl.i7.i = shl i32 %tmp3.i1.i, 8
  %or.i8.i = or i32 %shr11.i.i, %shl.i7.i
  %tmp19.i10.i = extractelement <4 x i32> %tmp10317.i, i32 1
  %shr22.i.i = lshr i32 %tmp19.i10.i, 24
  %shl27.i.i = shl i32 %tmp8.i5.i, 8
  %or28.i11.i = or i32 %shr22.i.i, %shl27.i.i
  %tmp32.i13.i = extractelement <4 x i32> %tmp10317.i, i32 0
  %shr35.i.i = lshr i32 %tmp32.i13.i, 24
  %shl40.i.i = shl i32 %tmp19.i10.i, 8
  %or41.i14.i = or i32 %shr35.i.i, %shl40.i.i
  %arrayidx107.i = getelementptr [8 x <4 x i32>]* %temp.i, i32 0, i32 %storemerge28.i
  %tmp114.i = extractelement <4 x i32> %tmp19323.i, i32 0
  %shr117.i = lshr i32 %tmp114.i, 13
  %and119.i = and i32 %shr117.i, 473087
  %tmp125.i = extractelement <4 x i32> %tmp20419.i, i32 0
  %shl.i = shl i32 %tmp125.i, 15
  %xor112.i = xor i32 %shl.i.i, %tmp3.i.i
  %xor120.i = xor i32 %xor112.i, %and119.i
  %xor123.i = xor i32 %xor120.i, %shl.i
  %xor128.i = xor i32 %xor123.i, %or41.i14.i
  %tmp130.i = insertelement <4 x i32> undef, i32 %xor128.i, i32 0
  %tmp140.i = extractelement <4 x i32> %tmp19323.i, i32 1
  %shr143.i = lshr i32 %tmp140.i, 13
  %and145.i = and i32 %shr143.i, 475005
  %tmp151.i = extractelement <4 x i32> %tmp20419.i, i32 1
  %shl154.i = shl i32 %tmp151.i, 15
  %xor138.i = xor i32 %and145.i, %tmp8.i.i
  %xor146.i = xor i32 %xor138.i, %or.i.i
  %xor149.i = xor i32 %xor146.i, %shl154.i
  %xor155.i = xor i32 %xor149.i, %or28.i11.i
  %tmp157.i = insertelement <4 x i32> %tmp130.i, i32 %xor155.i, i32 1
  %tmp167.i = extractelement <4 x i32> %tmp19323.i, i32 2
  %shr170.i = lshr i32 %tmp167.i, 13
  %and172.i = and i32 %shr170.i, 490365
  %tmp178.i = extractelement <4 x i32> %tmp20419.i, i32 2
  %shl181.i = shl i32 %tmp178.i, 15
  %xor165.i = xor i32 %and172.i, %tmp19.i.i
  %xor173.i = xor i32 %xor165.i, %or28.i.i
  %xor176.i = xor i32 %xor173.i, %shl181.i
  %xor182.i = xor i32 %xor176.i, %or.i8.i
  %tmp184.i = insertelement <4 x i32> %tmp157.i, i32 %xor182.i, i32 2
  %tmp194.i = extractelement <4 x i32> %tmp19323.i, i32 3
  %shr197.i = lshr i32 %tmp194.i, 13
  %and199.i = and i32 %shr197.i, 523055
  %tmp205.i = extractelement <4 x i32> %tmp20419.i, i32 3
  %shl208.i = shl i32 %tmp205.i, 15
  %xor192.i = xor i32 %and199.i, %tmp32.i.i
  %xor200.i = xor i32 %xor192.i, %or41.i.i
  %xor203.i = xor i32 %xor200.i, %shl208.i
  %xor209.i = xor i32 %xor203.i, %shr.i3.i
  %tmp211.i = insertelement <4 x i32> %tmp184.i, i32 %xor209.i, i32 3
  store <4 x i32> %tmp211.i, <4 x i32>* %arrayidx107.i, align 16
  %inc.i = add i32 %storemerge28.i, 1
  %cmp.i = icmp ult i32 %inc.i, 4
  br i1 %cmp.i, label %for.body.i, label %generateRand.exit

generateRand.exit:                                ; preds = %sw.epilog.i
  %tmp216.i = load <4 x i32>* %arraydecay.i, align 16
  %tmp217.i = extractelement <4 x i32> %tmp216.i, i32 0
  %conv.i = uitofp i32 %tmp217.i to float
  %1 = insertelement <4 x float> undef, float %conv.i, i32 0
  %tmp221.i = extractelement <4 x i32> %tmp216.i, i32 1
  %conv222.i = uitofp i32 %tmp221.i to float
  %2 = insertelement <4 x float> %1, float %conv222.i, i32 1
  %tmp226.i = extractelement <4 x i32> %tmp216.i, i32 2
  %conv227.i = uitofp i32 %tmp226.i to float
  %3 = insertelement <4 x float> %2, float %conv227.i, i32 2
  %tmp231.i = extractelement <4 x i32> %tmp216.i, i32 3
  %conv232.i = uitofp i32 %tmp231.i to float
  %4 = insertelement <4 x float> %3, float %conv232.i, i32 3
  %div.i = fdiv <4 x float> %4, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %tmp244.i = load <4 x i32>* %arrayidx90.i, align 16
  %tmp245.i = extractelement <4 x i32> %tmp244.i, i32 0
  %conv246.i = uitofp i32 %tmp245.i to float
  %5 = insertelement <4 x float> undef, float %conv246.i, i32 0
  %tmp250.i = extractelement <4 x i32> %tmp244.i, i32 1
  %conv251.i = uitofp i32 %tmp250.i to float
  %6 = insertelement <4 x float> %5, float %conv251.i, i32 1
  %tmp255.i = extractelement <4 x i32> %tmp244.i, i32 2
  %conv256.i = uitofp i32 %tmp255.i to float
  %7 = insertelement <4 x float> %6, float %conv256.i, i32 2
  %tmp260.i = extractelement <4 x i32> %tmp244.i, i32 3
  %conv261.i = uitofp i32 %tmp260.i to float
  %8 = insertelement <4 x float> %7, float %conv261.i, i32 3
  %div270.i = fdiv <4 x float> %8, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %call.i = call <4 x float> @__logf4(<4 x float> %div.i) nounwind
  %mul275.i = fmul <4 x float> %call.i, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
  %call276.i = call <4 x float> @__sqrtf4(<4 x float> %mul275.i) nounwind
  %mul283.i = fmul <4 x float> %div270.i, <float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000>
  %call287.i = call <4 x float> @__cosf4(<4 x float> %mul283.i) nounwind
  %mul288.i = fmul <4 x float> %call276.i, %call287.i
  %call292.i = call <4 x float> @__sinf4(<4 x float> %mul283.i) nounwind
  %mul293.i = fmul <4 x float> %call276.i, %call292.i
  %tmp297.i = load <4 x i32>* %arrayidx97.i, align 16
  call void @llvm.lifetime.end(i64 -1, i8* %0) nounwind
  %mul75 = fmul <4 x float> %tmp9, %mul288.i
  %add76 = fadd <4 x float> %tmp5, %mul75
  %call77 = call <4 x float> @__expf4(<4 x float> %add76) nounwind
  %mul78 = fmul <4 x float> %tmp100519, %call77
  %mul83 = fmul <4 x float> %tmp9, %mul293.i
  %add84 = fadd <4 x float> %tmp5, %mul83
  %call85 = call <4 x float> @__expf4(<4 x float> %add84) nounwind
  %mul86 = fmul <4 x float> %tmp110618, %call85
  %add89 = fadd <4 x float> %tmp122717, %mul78
  %add92 = fadd <4 x float> %tmp128816, %mul86
  %conv = sitofp i32 %storemerge20 to float
  %tmp97 = insertelement <4 x float> undef, float %conv, i32 0
  %splat = shufflevector <4 x float> %tmp97, <4 x float> undef, <4 x i32> zeroinitializer
  %mul98 = fmul <4 x float> %mul95, %splat
  %div = fdiv <4 x float> %mul78, %tmp17
  %call103 = call <4 x float> @__logf4(<4 x float> %div) nounwind
  %sub = fsub <4 x float> %call103, %mul98
  %div106 = fdiv <4 x float> %sub, %tmp21
  %mul107 = fmul <4 x float> %mul78, %div106
  %add108 = fadd <4 x float> %tmp134915, %mul107
  %div113 = fdiv <4 x float> %mul86, %tmp17
  %call114 = call <4 x float> @__logf4(<4 x float> %div113) nounwind
  %sub116 = fsub <4 x float> %call114, %mul98
  %div118 = fdiv <4 x float> %sub116, %tmp21
  %mul119 = fmul <4 x float> %mul86, %div118
  %add120 = fadd <4 x float> %tmp1401014, %mul119
  %inc = add i32 %storemerge20, 1
  %cmp = icmp slt i32 %inc, %noOfSum
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %generateRand.exit
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %tmp1227.lcssa = phi <4 x float> [ %tmp17, %entry ], [ %add89, %for.end.loopexit ]
  %tmp1288.lcssa = phi <4 x float> [ %tmp17, %entry ], [ %add92, %for.end.loopexit ]
  %tmp1349.lcssa = phi <4 x float> [ zeroinitializer, %entry ], [ %add108, %for.end.loopexit ]
  %tmp14010.lcssa = phi <4 x float> [ zeroinitializer, %entry ], [ %add120, %for.end.loopexit ]
  %conv124 = sitofp i32 %noOfSum to float
  %tmp125 = insertelement <4 x float> undef, float %conv124, i32 0
  %splat126 = shufflevector <4 x float> %tmp125, <4 x float> undef, <4 x i32> zeroinitializer
  %div127 = fdiv <4 x float> %tmp1227.lcssa, %splat126
  %div133 = fdiv <4 x float> %tmp1288.lcssa, %splat126
  %div139 = fdiv <4 x float> %tmp1349.lcssa, %splat126
  %div145 = fdiv <4 x float> %tmp14010.lcssa, %splat126
  %sub.i = fsub <4 x float> %div127, %tmp1
  %sub5.i = fsub <4 x float> %div133, %tmp1
  %tmp7.i = extractelement <4 x float> %sub.i, i32 0
  %cmp.i3 = fcmp ogt float %tmp7.i, 0.000000e+00
  br i1 %cmp.i3, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %for.end
  %tmp13.i = shufflevector <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <4 x float> %sub.i, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %for.end
  %tmp9912.i = phi <4 x float> [ %tmp13.i, %if.then.i ], [ zeroinitializer, %for.end ]
  %tmp934.i = phi <4 x float> [ <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, %if.then.i ], [ zeroinitializer, %for.end ]
  %tmp15.i = extractelement <4 x float> %sub.i, i32 1
  %cmp16.i = fcmp ogt float %tmp15.i, 0.000000e+00
  br i1 %cmp16.i, label %if.then17.i, label %if.end24.i

if.then17.i:                                      ; preds = %if.end.i
  %tmp19.i = insertelement <4 x float> %tmp934.i, float 1.000000e+00, i32 1
  %tmp23.i = shufflevector <4 x float> %tmp9912.i, <4 x float> %sub.i, <4 x i32> <i32 0, i32 5, i32 2, i32 3>
  br label %if.end24.i

if.end24.i:                                       ; preds = %if.then17.i, %if.end.i
  %tmp9911.i = phi <4 x float> [ %tmp23.i, %if.then17.i ], [ %tmp9912.i, %if.end.i ]
  %tmp933.i = phi <4 x float> [ %tmp19.i, %if.then17.i ], [ %tmp934.i, %if.end.i ]
  %tmp26.i = extractelement <4 x float> %sub.i, i32 2
  %cmp27.i = fcmp ogt float %tmp26.i, 0.000000e+00
  br i1 %cmp27.i, label %if.then28.i, label %if.end35.i

if.then28.i:                                      ; preds = %if.end24.i
  %tmp30.i = insertelement <4 x float> %tmp933.i, float 1.000000e+00, i32 2
  %tmp34.i = shufflevector <4 x float> %tmp9911.i, <4 x float> %sub.i, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  br label %if.end35.i

if.end35.i:                                       ; preds = %if.then28.i, %if.end24.i
  %tmp9910.i = phi <4 x float> [ %tmp34.i, %if.then28.i ], [ %tmp9911.i, %if.end24.i ]
  %tmp932.i = phi <4 x float> [ %tmp30.i, %if.then28.i ], [ %tmp933.i, %if.end24.i ]
  %tmp37.i = extractelement <4 x float> %sub.i, i32 3
  %cmp38.i = fcmp ogt float %tmp37.i, 0.000000e+00
  br i1 %cmp38.i, label %if.then39.i, label %if.end46.i

if.then39.i:                                      ; preds = %if.end35.i
  %tmp41.i = insertelement <4 x float> %tmp932.i, float 1.000000e+00, i32 3
  %tmp45.i = shufflevector <4 x float> %tmp9910.i, <4 x float> %sub.i, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  br label %if.end46.i

if.end46.i:                                       ; preds = %if.then39.i, %if.end35.i
  %tmp999.i = phi <4 x float> [ %tmp45.i, %if.then39.i ], [ %tmp9910.i, %if.end35.i ]
  %tmp931.i = phi <4 x float> [ %tmp41.i, %if.then39.i ], [ %tmp932.i, %if.end35.i ]
  %tmp48.i = extractelement <4 x float> %sub5.i, i32 0
  %cmp49.i = fcmp ogt float %tmp48.i, 0.000000e+00
  br i1 %cmp49.i, label %if.then50.i, label %if.end57.i

if.then50.i:                                      ; preds = %if.end46.i
  %tmp56.i = shufflevector <4 x float> <float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <4 x float> %sub5.i, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
  br label %if.end57.i

if.end57.i:                                       ; preds = %if.then50.i, %if.end46.i
  %tmp10116.i = phi <4 x float> [ %tmp56.i, %if.then50.i ], [ zeroinitializer, %if.end46.i ]
  %tmp968.i = phi <4 x float> [ <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, %if.then50.i ], [ zeroinitializer, %if.end46.i ]
  %tmp59.i = extractelement <4 x float> %sub5.i, i32 1
  %cmp60.i = fcmp ogt float %tmp59.i, 0.000000e+00
  br i1 %cmp60.i, label %if.then61.i, label %if.end68.i

if.then61.i:                                      ; preds = %if.end57.i
  %tmp63.i = insertelement <4 x float> %tmp968.i, float 1.000000e+00, i32 1
  %tmp67.i = shufflevector <4 x float> %tmp10116.i, <4 x float> %sub5.i, <4 x i32> <i32 0, i32 5, i32 2, i32 3>
  br label %if.end68.i

if.end68.i:                                       ; preds = %if.then61.i, %if.end57.i
  %tmp10115.i = phi <4 x float> [ %tmp67.i, %if.then61.i ], [ %tmp10116.i, %if.end57.i ]
  %tmp967.i = phi <4 x float> [ %tmp63.i, %if.then61.i ], [ %tmp968.i, %if.end57.i ]
  %tmp70.i = extractelement <4 x float> %sub5.i, i32 2
  %cmp71.i = fcmp ogt float %tmp70.i, 0.000000e+00
  br i1 %cmp71.i, label %if.then72.i, label %if.end79.i

if.then72.i:                                      ; preds = %if.end68.i
  %tmp74.i = insertelement <4 x float> %tmp967.i, float 1.000000e+00, i32 2
  %tmp78.i = shufflevector <4 x float> %tmp10115.i, <4 x float> %sub5.i, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  br label %if.end79.i

if.end79.i:                                       ; preds = %if.then72.i, %if.end68.i
  %tmp10114.i = phi <4 x float> [ %tmp78.i, %if.then72.i ], [ %tmp10115.i, %if.end68.i ]
  %tmp966.i = phi <4 x float> [ %tmp74.i, %if.then72.i ], [ %tmp967.i, %if.end68.i ]
  %tmp81.i = extractelement <4 x float> %sub5.i, i32 3
  %cmp82.i = fcmp ogt float %tmp81.i, 0.000000e+00
  br i1 %cmp82.i, label %if.then83.i, label %calOutputs.exit

if.then83.i:                                      ; preds = %if.end79.i
  %tmp85.i = insertelement <4 x float> %tmp966.i, float 1.000000e+00, i32 3
  %tmp89.i = shufflevector <4 x float> %tmp10114.i, <4 x float> %sub5.i, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  br label %calOutputs.exit

calOutputs.exit:                                  ; preds = %if.end79.i, %if.then83.i
  %tmp10113.i = phi <4 x float> [ %tmp89.i, %if.then83.i ], [ %tmp10114.i, %if.end79.i ]
  %tmp965.i = phi <4 x float> [ %tmp85.i, %if.then83.i ], [ %tmp966.i, %if.end79.i ]
  %mul.i4 = fmul <4 x float> %tmp931.i, %div139
  %mul97.i = fmul <4 x float> %tmp965.i, %div145
  %mul156 = shl i32 %add, 1
  %arrayidx158 = getelementptr <4 x float> addrspace(1)* %priceSamples, i32 %mul156
  store <4 x float> %tmp999.i, <4 x float> addrspace(1)* %arrayidx158, align 16
  %add1661 = or i32 %mul156, 1
  %arrayidx168 = getelementptr <4 x float> addrspace(1)* %priceSamples, i32 %add1661
  store <4 x float> %tmp10113.i, <4 x float> addrspace(1)* %arrayidx168, align 16
  %arrayidx177 = getelementptr <4 x float> addrspace(1)* %pathDeriv, i32 %mul156
  store <4 x float> %mul.i4, <4 x float> addrspace(1)* %arrayidx177, align 16
  %arrayidx187 = getelementptr <4 x float> addrspace(1)* %pathDeriv, i32 %add1661
  store <4 x float> %mul97.i, <4 x float> addrspace(1)* %arrayidx187, align 16
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @__expf4(<4 x float>)

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind

declare void @llvm.lifetime.end(i64, i8* nocapture) nounwind
