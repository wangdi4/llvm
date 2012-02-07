; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlMersenneTwister.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"1002\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @lshift128
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %LeafBlock37, %LeafBlock
; CHECK: phi-split-bb1:                                    ; preds = %LeafBlock39, %phi-split-bb
; CHECK: phi-split-bb2:                                    ; preds = %LeafBlock41, %phi-split-bb1
; CHECK: phi-split-bb3:                                    ; preds = %LeafBlock47, %phi-split-bb2
; CHECK: phi-split-bb4:                                    ; preds = %LeafBlock49, %phi-split-bb3
; CHECK: phi-split-bb5:                                    ; preds = %LeafBlock53, %phi-split-bb4
; CHECK: phi-split-bb6:                                    ; preds = %sw.bb128, %sw.bb139
; CHECK: phi-split-bb10:                                   ; preds = %sw.bb110, %sw.bb117
; CHECK: phi-split-bb15:                                   ; preds = %phi-split-bb6, %phi-split-bb10
; CHECK: phi-split-bb20:                                   ; preds = %sw.bb96, %sw.bb103
; CHECK: phi-split-bb25:                                   ; preds = %sw.bb, %sw.bb90
; CHECK: phi-split-bb30:                                   ; preds = %phi-split-bb20, %phi-split-bb25
; CHECK: phi-split-bb35:                                   ; preds = %phi-split-bb15, %phi-split-bb30
; CHECK: phi-split-bb40:                                   ; preds = %for.end, %for.end328.loopexit
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

define void @gaussianRand(<4 x i32> addrspace(1)* %seedArray, i32 %width, i32 %mulFactor, <4 x float> addrspace(1)* %gaussianRand, ...) nounwind {
entry:
  %temp = alloca [8 x <4 x i32>], align 16
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 1) nounwind
  %mul = mul i32 %call1, %width
  %add = add i32 %mul, %call
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %seedArray, i32 %add
  %tmp5 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %shr = lshr <4 x i32> %tmp5, <i32 30, i32 30, i32 30, i32 30>
  %xor = xor <4 x i32> %tmp5, %shr
  %mul49 = mul <4 x i32> %xor, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add51 = add <4 x i32> %mul49, <i32 1, i32 1, i32 1, i32 1>
  %shr57 = lshr <4 x i32> %add51, <i32 30, i32 30, i32 30, i32 30>
  %xor58 = xor <4 x i32> %add51, %shr57
  %mul59 = mul <4 x i32> %xor58, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add61 = add <4 x i32> %mul59, <i32 2, i32 2, i32 2, i32 2>
  %shr67 = lshr <4 x i32> %add61, <i32 30, i32 30, i32 30, i32 30>
  %xor68 = xor <4 x i32> %add61, %shr67
  %mul69 = mul <4 x i32> %xor68, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add71 = add <4 x i32> %mul69, <i32 3, i32 3, i32 3, i32 3>
  %shr77 = lshr <4 x i32> %add71, <i32 30, i32 30, i32 30, i32 30>
  %xor78 = xor <4 x i32> %add71, %shr77
  %mul79 = mul <4 x i32> %xor78, <i32 1812433253, i32 1812433253, i32 1812433253, i32 1812433253>
  %add81 = add <4 x i32> %mul79, <i32 4, i32 4, i32 4, i32 4>
  %cmp27 = icmp eq i32 %mulFactor, 0
  br i1 %cmp27, label %for.end328, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %arraydecay = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0
  %arrayidx99 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 1
  %arrayidx106 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 2
  %arrayidx113 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 3
  %arrayidx120 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 4
  %arrayidx131 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 5
  %arrayidx142 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 6
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %sw.epilog
  %storemerge132 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %sw.epilog ]
  %tmp1521731 = phi <4 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %tmp15218, %sw.epilog ]
  %tmp2531930 = phi <4 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %tmp25320, %sw.epilog ]
  %tmp2372129 = phi <4 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %tmp23722, %sw.epilog ]
  %tmp2422328 = phi <4 x i32> [ zeroinitializer, %for.body.lr.ph ], [ %tmp24224, %sw.epilog ]
  br label %NodeBlock61

NodeBlock61:                                      ; preds = %for.body
  %Pivot62 = icmp slt i32 %storemerge132, 4
  br i1 %Pivot62, label %NodeBlock45, label %NodeBlock59

NodeBlock59:                                      ; preds = %NodeBlock61
  %Pivot60 = icmp slt i32 %storemerge132, 6
  br i1 %Pivot60, label %NodeBlock51, label %NodeBlock57

NodeBlock57:                                      ; preds = %NodeBlock59
  %Pivot58 = icmp slt i32 %storemerge132, 7
  br i1 %Pivot58, label %LeafBlock53, label %LeafBlock55

LeafBlock55:                                      ; preds = %NodeBlock57
  %SwitchLeaf56 = icmp eq i32 %storemerge132, 7
  br i1 %SwitchLeaf56, label %sw.bb139, label %NewDefault

LeafBlock53:                                      ; preds = %NodeBlock57
  %SwitchLeaf54 = icmp eq i32 %storemerge132, 6
  br i1 %SwitchLeaf54, label %sw.bb128, label %NewDefault

NodeBlock51:                                      ; preds = %NodeBlock59
  %Pivot52 = icmp slt i32 %storemerge132, 5
  br i1 %Pivot52, label %LeafBlock47, label %LeafBlock49

LeafBlock49:                                      ; preds = %NodeBlock51
  %SwitchLeaf50 = icmp eq i32 %storemerge132, 5
  br i1 %SwitchLeaf50, label %sw.bb117, label %NewDefault

LeafBlock47:                                      ; preds = %NodeBlock51
  %SwitchLeaf48 = icmp eq i32 %storemerge132, 4
  br i1 %SwitchLeaf48, label %sw.bb110, label %NewDefault

NodeBlock45:                                      ; preds = %NodeBlock61
  %Pivot46 = icmp slt i32 %storemerge132, 2
  br i1 %Pivot46, label %NodeBlock, label %NodeBlock43

NodeBlock43:                                      ; preds = %NodeBlock45
  %Pivot44 = icmp slt i32 %storemerge132, 3
  br i1 %Pivot44, label %LeafBlock39, label %LeafBlock41

LeafBlock41:                                      ; preds = %NodeBlock43
  %SwitchLeaf42 = icmp eq i32 %storemerge132, 3
  br i1 %SwitchLeaf42, label %sw.bb103, label %NewDefault

LeafBlock39:                                      ; preds = %NodeBlock43
  %SwitchLeaf40 = icmp eq i32 %storemerge132, 2
  br i1 %SwitchLeaf40, label %sw.bb96, label %NewDefault

NodeBlock:                                        ; preds = %NodeBlock45
  %Pivot = icmp slt i32 %storemerge132, 1
  br i1 %Pivot, label %LeafBlock, label %LeafBlock37

LeafBlock37:                                      ; preds = %NodeBlock
  %SwitchLeaf38 = icmp eq i32 %storemerge132, 1
  br i1 %SwitchLeaf38, label %sw.bb90, label %NewDefault

LeafBlock:                                        ; preds = %NodeBlock
  %SwitchLeaf = icmp eq i32 %storemerge132, 0
  br i1 %SwitchLeaf, label %sw.bb, label %NewDefault

sw.bb:                                            ; preds = %LeafBlock
  br label %sw.epilog

sw.bb90:                                          ; preds = %LeafBlock37
  %tmp93 = load <4 x i32>* %arraydecay, align 16
  br label %sw.epilog

sw.bb96:                                          ; preds = %LeafBlock39
  %tmp100 = load <4 x i32>* %arrayidx99, align 16
  br label %sw.epilog

sw.bb103:                                         ; preds = %LeafBlock41
  %tmp107 = load <4 x i32>* %arrayidx106, align 16
  br label %sw.epilog

sw.bb110:                                         ; preds = %LeafBlock47
  %tmp114 = load <4 x i32>* %arrayidx113, align 16
  br label %sw.epilog

sw.bb117:                                         ; preds = %LeafBlock49
  %tmp121 = load <4 x i32>* %arrayidx120, align 16
  %tmp124 = load <4 x i32>* %arraydecay, align 16
  %tmp127 = load <4 x i32>* %arrayidx106, align 16
  br label %sw.epilog

sw.bb128:                                         ; preds = %LeafBlock53
  %tmp132 = load <4 x i32>* %arrayidx131, align 16
  %tmp135 = load <4 x i32>* %arrayidx99, align 16
  %tmp138 = load <4 x i32>* %arrayidx113, align 16
  br label %sw.epilog

sw.bb139:                                         ; preds = %LeafBlock55
  %tmp143 = load <4 x i32>* %arrayidx142, align 16
  %tmp146 = load <4 x i32>* %arrayidx106, align 16
  %tmp149 = load <4 x i32>* %arrayidx120, align 16
  br label %sw.epilog

NewDefault:                                       ; preds = %LeafBlock55, %LeafBlock53, %LeafBlock49, %LeafBlock47, %LeafBlock41, %LeafBlock39, %LeafBlock37, %LeafBlock
  br label %sw.epilog

sw.epilog:                                        ; preds = %NewDefault, %sw.bb139, %sw.bb128, %sw.bb117, %sw.bb110, %sw.bb103, %sw.bb96, %sw.bb90, %sw.bb
  %tmp24224 = phi <4 x i32> [ %tmp149, %sw.bb139 ], [ %tmp138, %sw.bb128 ], [ %tmp127, %sw.bb117 ], [ %add51, %sw.bb110 ], [ %tmp5, %sw.bb103 ], [ %add81, %sw.bb96 ], [ %add71, %sw.bb90 ], [ %add61, %sw.bb ], [ %tmp2422328, %NewDefault ]
  %tmp23722 = phi <4 x i32> [ %tmp146, %sw.bb139 ], [ %tmp135, %sw.bb128 ], [ %tmp124, %sw.bb117 ], [ %add81, %sw.bb110 ], [ %add71, %sw.bb103 ], [ %add61, %sw.bb96 ], [ %add51, %sw.bb90 ], [ %tmp5, %sw.bb ], [ %tmp2372129, %NewDefault ]
  %tmp25320 = phi <4 x i32> [ %tmp143, %sw.bb139 ], [ %tmp132, %sw.bb128 ], [ %tmp121, %sw.bb117 ], [ %tmp114, %sw.bb110 ], [ %tmp107, %sw.bb103 ], [ %tmp100, %sw.bb96 ], [ %tmp93, %sw.bb90 ], [ %add81, %sw.bb ], [ %tmp2531930, %NewDefault ]
  %tmp15218 = phi <4 x i32> [ %tmp2531930, %sw.bb139 ], [ %tmp2531930, %sw.bb128 ], [ %tmp2531930, %sw.bb117 ], [ %tmp2531930, %sw.bb110 ], [ %tmp2531930, %sw.bb103 ], [ %tmp2531930, %sw.bb96 ], [ %tmp2531930, %sw.bb90 ], [ %add71, %sw.bb ], [ %tmp1521731, %NewDefault ]
  %tmp3.i = extractelement <4 x i32> %tmp23722, i32 0
  %shl.i = shl i32 %tmp3.i, 24
  %tmp8.i = extractelement <4 x i32> %tmp23722, i32 1
  %shl11.i = shl i32 %tmp8.i, 24
  %shr.i = lshr i32 %tmp3.i, 8
  %or.i = or i32 %shl11.i, %shr.i
  %tmp19.i = extractelement <4 x i32> %tmp23722, i32 2
  %shl22.i = shl i32 %tmp19.i, 24
  %shr27.i = lshr i32 %tmp8.i, 8
  %or28.i = or i32 %shl22.i, %shr27.i
  %tmp32.i = extractelement <4 x i32> %tmp23722, i32 3
  %shl35.i = shl i32 %tmp32.i, 24
  %shr40.i = lshr i32 %tmp19.i, 8
  %or41.i = or i32 %shl35.i, %shr40.i
  %tmp3.i2 = extractelement <4 x i32> %tmp15218, i32 3
  %shr.i4 = lshr i32 %tmp3.i2, 24
  %tmp8.i6 = extractelement <4 x i32> %tmp15218, i32 2
  %shr11.i = lshr i32 %tmp8.i6, 24
  %shl.i8 = shl i32 %tmp3.i2, 8
  %or.i9 = or i32 %shr11.i, %shl.i8
  %tmp19.i11 = extractelement <4 x i32> %tmp15218, i32 1
  %shr22.i = lshr i32 %tmp19.i11, 24
  %shl27.i = shl i32 %tmp8.i6, 8
  %or28.i12 = or i32 %shr22.i, %shl27.i
  %tmp32.i14 = extractelement <4 x i32> %tmp15218, i32 0
  %shr35.i = lshr i32 %tmp32.i14, 24
  %shl40.i = shl i32 %tmp19.i11, 8
  %or41.i15 = or i32 %shr35.i, %shl40.i
  %arrayidx156 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 %storemerge132
  %tmp163 = extractelement <4 x i32> %tmp24224, i32 0
  %shr166 = lshr i32 %tmp163, 13
  %and168 = and i32 %shr166, 473087
  %tmp174 = extractelement <4 x i32> %tmp25320, i32 0
  %shl = shl i32 %tmp174, 15
  %xor161 = xor i32 %and168, %tmp3.i
  %xor169 = xor i32 %xor161, %shl.i
  %xor172 = xor i32 %xor169, %shl
  %xor177 = xor i32 %xor172, %or41.i15
  %tmp179 = insertelement <4 x i32> undef, i32 %xor177, i32 0
  %tmp189 = extractelement <4 x i32> %tmp24224, i32 1
  %shr192 = lshr i32 %tmp189, 13
  %and194 = and i32 %shr192, 475005
  %tmp200 = extractelement <4 x i32> %tmp25320, i32 1
  %shl203 = shl i32 %tmp200, 15
  %xor187 = xor i32 %and194, %tmp8.i
  %xor195 = xor i32 %xor187, %shl203
  %xor198 = xor i32 %xor195, %or.i
  %xor204 = xor i32 %xor198, %or28.i12
  %tmp206 = insertelement <4 x i32> %tmp179, i32 %xor204, i32 1
  %tmp216 = extractelement <4 x i32> %tmp24224, i32 2
  %shr219 = lshr i32 %tmp216, 13
  %and221 = and i32 %shr219, 490365
  %tmp227 = extractelement <4 x i32> %tmp25320, i32 2
  %shl230 = shl i32 %tmp227, 15
  %xor214 = xor i32 %and221, %tmp19.i
  %xor222 = xor i32 %xor214, %shl230
  %xor225 = xor i32 %xor222, %or28.i
  %xor231 = xor i32 %xor225, %or.i9
  %tmp233 = insertelement <4 x i32> %tmp206, i32 %xor231, i32 2
  %tmp243 = extractelement <4 x i32> %tmp24224, i32 3
  %shr246 = lshr i32 %tmp243, 13
  %and248 = and i32 %shr246, 523055
  %tmp254 = extractelement <4 x i32> %tmp25320, i32 3
  %shl257 = shl i32 %tmp254, 15
  %xor241 = xor i32 %and248, %tmp32.i
  %xor249 = xor i32 %xor241, %shl257
  %xor252 = xor i32 %xor249, %or41.i
  %xor258 = xor i32 %xor252, %shr.i4
  %tmp260 = insertelement <4 x i32> %tmp233, i32 %xor258, i32 3
  store <4 x i32> %tmp260, <4 x i32>* %arrayidx156, align 16
  %inc = add i32 %storemerge132, 1
  %cmp = icmp ult i32 %inc, %mulFactor
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %sw.epilog
  %mul268 = mul i32 %add, %mulFactor
  %div = lshr i32 %mulFactor, 1
  %cmp27225 = icmp eq i32 %div, 0
  br i1 %cmp27225, label %for.end328, label %for.body273.lr.ph

for.body273.lr.ph:                                ; preds = %for.end
  %arrayidx276.phi.trans.insert = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 0
  %tmp277.pre = load <4 x i32>* %arrayidx276.phi.trans.insert, align 16
  br label %for.body273

for.body273:                                      ; preds = %for.body273, %for.body273.lr.ph
  %tmp277 = phi <4 x i32> [ %tmp277.pre, %for.body273.lr.ph ], [ %tmp287, %for.body273 ]
  %storemerge26 = phi i32 [ 0, %for.body273.lr.ph ], [ %add284, %for.body273 ]
  %call278 = call <4 x float> @__convert_float4_uint4(<4 x i32> %tmp277) nounwind
  %div282 = fdiv <4 x float> %call278, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %add284 = add i32 %storemerge26, 1
  %arrayidx286 = getelementptr [8 x <4 x i32>]* %temp, i32 0, i32 %add284
  %tmp287 = load <4 x i32>* %arrayidx286, align 16
  %call288 = call <4 x float> @__convert_float4_uint4(<4 x i32> %tmp287) nounwind
  %div292 = fdiv <4 x float> %call288, <float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000, float 0x41F0000000000000>
  %call295 = call <4 x float> @__logf4(<4 x float> %div282) nounwind
  %mul296 = fmul <4 x float> %call295, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
  %call297 = call <4 x float> @__sqrtf4(<4 x float> %mul296) nounwind
  %mul302 = fmul <4 x float> %div292, <float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000, float 0x401921FB60000000>
  %mul305 = shl i32 %storemerge26, 1
  %add307 = add i32 %mul305, %mul268
  %arrayidx309 = getelementptr <4 x float> addrspace(1)* %gaussianRand, i32 %add307
  %call312 = call <4 x float> @__cosf4(<4 x float> %mul302) nounwind
  %mul313 = fmul <4 x float> %call297, %call312
  store <4 x float> %mul313, <4 x float> addrspace(1)* %arrayidx309, align 16
  %add318 = add i32 %add307, 1
  %arrayidx320 = getelementptr <4 x float> addrspace(1)* %gaussianRand, i32 %add318
  %call323 = call <4 x float> @__sinf4(<4 x float> %mul302) nounwind
  %mul324 = fmul <4 x float> %call297, %call323
  store <4 x float> %mul324, <4 x float> addrspace(1)* %arrayidx320, align 16
  %cmp272 = icmp ult i32 %add284, %div
  br i1 %cmp272, label %for.body273, label %for.end328.loopexit

for.end328.loopexit:                              ; preds = %for.body273
  br label %for.end328

for.end328:                                       ; preds = %for.end328.loopexit, %entry, %for.end
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @__convert_float4_uint4(<4 x i32>)

declare <4 x float> @__sqrtf4(<4 x float>)

declare <4 x float> @__logf4(<4 x float>)

declare <4 x float> @__cosf4(<4 x float>)

declare <4 x float> @__sinf4(<4 x float>)
