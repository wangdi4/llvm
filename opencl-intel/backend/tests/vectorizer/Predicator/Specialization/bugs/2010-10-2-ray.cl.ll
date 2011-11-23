; RUN: llvm-as %s -o %t.bc
; RUN: opt  -predicate -specialize %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'ray.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

@PI = addrspace(1) constant float 0x400921FB60000000, align 4 ; <float addrspace(1)*> [#uses=0]
@g_renderWidth = addrspace(1) global float 5.120000e+02, align 4 ; <float addrspace(1)*> [#uses=1]
@g_renderHeight = addrspace(1) global float 5.120000e+02, align 4 ; <float addrspace(1)*> [#uses=1]
@SPECULAR_EXPONENT = addrspace(1) global float 5.000000e+01, align 4 ; <float addrspace(1)*> [#uses=2]
@g_maxRayShots = addrspace(1) global i32 4, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_numberOfSpheres = addrspace(1) global i32 35, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_numberOfSphereParameters = addrspace(1) global i32 13, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_pSphereArray = common global float addrspace(1)* null, align 4 ; <float addrspace(1)**> [#uses=1]
@g_viewPlaneDistance = common addrspace(1) global float 0.000000e+00, align 4 ; <float addrspace(1)*> [#uses=1]
@g_lightPos = common addrspace(1) global <4 x float> zeroinitializer, align 16 ; <<4 x float> addrspace(1)*> [#uses=1]

declare float @_Z3sinf(float)

declare float @_Z3powff(float, float)

declare float @_Z3cosf(float)

declare float @_Z3dotU8__vector4fS_(<4 x float>, <4 x float>)

declare float @_Z4sqrtf(float)

; CHECK: @evaluatePixel
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                 ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                 ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                 ; preds = %header{{[0-9]*}}
; CHECK: ret

define <4 x float> @evaluatePixel(float addrspace(1)* %pSphereArray, <2 x i32> %pixel, i32 %numberOfSpheres, i32 %numberOfSphereParameters, i32 %maxRayShots, i32 %renderWidth, i32 %renderHeight, float %viewPlaneDistance, <4 x float> %lightPos) nounwind {
BB0:
  %scalar200 = extractelement <4 x float> %lightPos, i32 0 ; <float> [#uses=1]
  %scalar201 = extractelement <4 x float> %lightPos, i32 1 ; <float> [#uses=1]
  %scalar202 = extractelement <4 x float> %lightPos, i32 2 ; <float> [#uses=1]
  %scalar203 = extractelement <4 x float> %lightPos, i32 3 ; <float> [#uses=1]
  %scalar = extractelement <2 x i32> %pixel, i32 0 ; <i32> [#uses=1]
  %scalar132 = extractelement <2 x i32> %pixel, i32 1 ; <i32> [#uses=1]
  %conv = sitofp i32 %scalar to float             ; <float> [#uses=1]
  %mul = fmul float %conv, 2.000000e+00           ; <float> [#uses=1]
  %conv9 = sitofp i32 %renderWidth to float       ; <float> [#uses=1]
  %div = fdiv float %mul, %conv9                  ; <float> [#uses=1]
  %sub = fadd float %div, -1.000000e+00           ; <float> [#uses=1]
  %conv12 = sitofp i32 %scalar132 to float        ; <float> [#uses=1]
  %mul13 = fmul float %conv12, -2.000000e+00      ; <float> [#uses=1]
  %conv15 = sitofp i32 %renderHeight to float     ; <float> [#uses=1]
  %div18 = fdiv float %mul13, %conv15             ; <float> [#uses=1]
  %add = fadd float %div18, 1.000000e+00          ; <float> [#uses=1]
  %neg = fsub float -0.000000e+00, %viewPlaneDistance ; <float> [#uses=1]
  %mul1.i = mul i32 %numberOfSphereParameters, %numberOfSpheres ; <i32> [#uses=3]
  %cmp2.i = icmp eq i32 %mul1.i, 0                ; <i1> [#uses=2]
  br label %BB0314

BB0314:                                           ; preds = %BB0343, %BB0
  %dir.0.ph133 = phi float [ %sub, %BB0 ], [ %scalar300, %BB0343 ] ; <float> [#uses=1]
  %dir.0.ph134 = phi float [ %add, %BB0 ], [ %scalar301, %BB0343 ] ; <float> [#uses=1]
  %dir.0.ph135 = phi float [ %neg, %BB0 ], [ %scalar302, %BB0343 ] ; <float> [#uses=1]
  %dir.0.ph136 = phi float [ 0.000000e+00, %BB0 ], [ %scalar303, %BB0343 ] ; <float> [#uses=1]
  %origin.0.ph137 = phi float [ 0.000000e+00, %BB0 ], [ %add78.i188, %BB0343 ] ; <float> [#uses=2]
  %origin.0.ph138 = phi float [ 0.000000e+00, %BB0 ], [ %add78.i189, %BB0343 ] ; <float> [#uses=2]
  %origin.0.ph139 = phi float [ 0.000000e+00, %BB0 ], [ %add78.i190, %BB0343 ] ; <float> [#uses=2]
  %origin.0.ph140 = phi float [ 0.000000e+00, %BB0 ], [ %add78.i191, %BB0343 ] ; <float> [#uses=2]
  %dst.0.ph141 = phi float [ 0.000000e+00, %BB0 ], [ %scalar296, %BB0343 ] ; <float> [#uses=1]
  %dst.0.ph142 = phi float [ 0.000000e+00, %BB0 ], [ %scalar297, %BB0343 ] ; <float> [#uses=1]
  %dst.0.ph143 = phi float [ 0.000000e+00, %BB0 ], [ %scalar298, %BB0343 ] ; <float> [#uses=1]
  %dst.0.ph144 = phi float [ 0.000000e+00, %BB0 ], [ %scalar299, %BB0343 ] ; <float> [#uses=1]
  %sphereNum.3.ph = phi i32 [ undef, %BB0 ], [ %sphereNum.0, %BB0343 ] ; <i32> [#uses=1]
  %rayShots.0.ph = phi i32 [ %maxRayShots, %BB0 ], [ %dec, %BB0343 ] ; <i32> [#uses=1]
  %colorScale.0.ph145 = phi float [ 1.000000e+00, %BB0 ], [ %mul305284, %BB0343 ] ; <float> [#uses=2]
  %colorScale.0.ph146 = phi float [ 1.000000e+00, %BB0 ], [ %mul305285, %BB0343 ] ; <float> [#uses=2]
  %colorScale.0.ph147 = phi float [ 1.000000e+00, %BB0 ], [ %mul305286, %BB0343 ] ; <float> [#uses=2]
  %colorScale.0.ph148 = phi float [ 0.000000e+00, %BB0 ], [ %mul305287, %BB0343 ] ; <float> [#uses=2]
  br label %BB0315

BB0315:                                           ; preds = %BB0314, %BB0342
  %dir.0.ph69149 = phi float [ %scalar180, %BB0342 ], [ %dir.0.ph133, %BB0314 ] ; <float> [#uses=1]
  %dir.0.ph69150 = phi float [ %scalar181, %BB0342 ], [ %dir.0.ph134, %BB0314 ] ; <float> [#uses=1]
  %dir.0.ph69151 = phi float [ %scalar182, %BB0342 ], [ %dir.0.ph135, %BB0314 ] ; <float> [#uses=1]
  %dir.0.ph69152 = phi float [ %scalar183, %BB0342 ], [ %dir.0.ph136, %BB0314 ] ; <float> [#uses=1]
  %dst.0.ph70153 = phi float [ %scalar296, %BB0342 ], [ %dst.0.ph141, %BB0314 ] ; <float> [#uses=2]
  %dst.0.ph70154 = phi float [ %scalar297, %BB0342 ], [ %dst.0.ph142, %BB0314 ] ; <float> [#uses=2]
  %dst.0.ph70155 = phi float [ %scalar298, %BB0342 ], [ %dst.0.ph143, %BB0314 ] ; <float> [#uses=2]
  %dst.0.ph70156 = phi float [ %scalar299, %BB0342 ], [ %dst.0.ph144, %BB0314 ] ; <float> [#uses=2]
  %sphereNum.3.ph71 = phi i32 [ %sphereNum.0, %BB0342 ], [ %sphereNum.3.ph, %BB0314 ] ; <i32> [#uses=1]
  %rayShots.0.ph72 = phi i32 [ 0, %BB0342 ], [ %rayShots.0.ph, %BB0314 ] ; <i32> [#uses=1]
  %temp.vect288 = insertelement <4 x float> undef, float %dst.0.ph70153, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect289 = insertelement <4 x float> %temp.vect288, float %dst.0.ph70154, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect290 = insertelement <4 x float> %temp.vect289, float %dst.0.ph70155, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect291 = insertelement <4 x float> %temp.vect290, float %dst.0.ph70156, i32 3 ; <<4 x float>> [#uses=1]
  br label %BB0316

BB0316:                                           ; preds = %BB0315, %BB0327
  %dir.0.ph75157 = phi float [ %scalar180, %BB0327 ], [ %dir.0.ph69149, %BB0315 ] ; <float> [#uses=2]
  %dir.0.ph75158 = phi float [ %scalar181, %BB0327 ], [ %dir.0.ph69150, %BB0315 ] ; <float> [#uses=2]
  %dir.0.ph75159 = phi float [ %scalar182, %BB0327 ], [ %dir.0.ph69151, %BB0315 ] ; <float> [#uses=2]
  %dir.0.ph75160 = phi float [ %scalar183, %BB0327 ], [ %dir.0.ph69152, %BB0315 ] ; <float> [#uses=2]
  %sphereNum.3.ph76 = phi i32 [ %sphereNum.0, %BB0327 ], [ %sphereNum.3.ph71, %BB0315 ] ; <i32> [#uses=1]
  %rayShots.0.ph77 = phi i32 [ 0, %BB0327 ], [ %rayShots.0.ph72, %BB0315 ] ; <i32> [#uses=3]
  %temp.vect168 = insertelement <4 x float> undef, float %dir.0.ph75157, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect169 = insertelement <4 x float> %temp.vect168, float %dir.0.ph75158, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect170 = insertelement <4 x float> %temp.vect169, float %dir.0.ph75159, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect171 = insertelement <4 x float> %temp.vect170, float %dir.0.ph75160, i32 3 ; <<4 x float>> [#uses=1]
  br i1 %cmp2.i, label %BB0317, label %BB0320

BB0317:                                           ; preds = %BB0316
  br label %BB0318

BB0318:                                           ; preds = %BB0317, %BB0319
  %dir.0.us161 = phi float [ %scalar292, %BB0319 ], [ %dir.0.ph75157, %BB0317 ] ; <float> [#uses=1]
  %dir.0.us162 = phi float [ %scalar293, %BB0319 ], [ %dir.0.ph75158, %BB0317 ] ; <float> [#uses=1]
  %dir.0.us163 = phi float [ %scalar294, %BB0319 ], [ %dir.0.ph75159, %BB0317 ] ; <float> [#uses=1]
  %dir.0.us164 = phi float [ %scalar295, %BB0319 ], [ %dir.0.ph75160, %BB0317 ] ; <float> [#uses=1]
  %rayShots.0.us = phi i32 [ 0, %BB0319 ], [ %rayShots.0.ph77, %BB0317 ] ; <i32> [#uses=1]
  %temp.vect = insertelement <4 x float> undef, float %dir.0.us161, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect165 = insertelement <4 x float> %temp.vect, float %dir.0.us162, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect166 = insertelement <4 x float> %temp.vect165, float %dir.0.us163, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect167 = insertelement <4 x float> %temp.vect166, float %dir.0.us164, i32 3 ; <<4 x float>> [#uses=1]
  %cmp53.us = icmp sgt i32 %rayShots.0.us, 0      ; <i1> [#uses=1]
  br i1 %cmp53.us, label %BB0319, label %BB0344

BB0319:                                           ; preds = %BB0318
  %call.us = tail call <4 x float> @_Z9normalizeU8__vector4f(<4 x float> %temp.vect167) nounwind ; <<4 x float>> [#uses=4]
  %scalar292 = extractelement <4 x float> %call.us, i32 0 ; <float> [#uses=1]
  %scalar293 = extractelement <4 x float> %call.us, i32 1 ; <float> [#uses=1]
  %scalar294 = extractelement <4 x float> %call.us, i32 2 ; <float> [#uses=1]
  %scalar295 = extractelement <4 x float> %call.us, i32 3 ; <float> [#uses=1]
  br label %BB0318

BB0320:                                           ; preds = %BB0316
  %cmp53 = icmp sgt i32 %rayShots.0.ph77, 0       ; <i1> [#uses=1]
  br i1 %cmp53, label %BB0321, label %BB0345

BB0321:                                           ; preds = %BB0320
  %call = tail call <4 x float> @_Z9normalizeU8__vector4f(<4 x float> %temp.vect171) nounwind ; <<4 x float>> [#uses=7]
  %scalar180 = extractelement <4 x float> %call, i32 0 ; <float> [#uses=4]
  %scalar181 = extractelement <4 x float> %call, i32 1 ; <float> [#uses=4]
  %scalar182 = extractelement <4 x float> %call, i32 2 ; <float> [#uses=4]
  %scalar183 = extractelement <4 x float> %call, i32 3 ; <float> [#uses=4]
  br label %BB0322

BB0322:                                           ; preds = %BB0321, %BB0326
  %sphereNum.1 = phi i32 [ %sphereNum.0, %BB0326 ], [ %sphereNum.3.ph76, %BB0321 ] ; <i32> [#uses=2]
  %t.1 = phi float [ %t.0, %BB0326 ], [ 9.999900e+04, %BB0321 ] ; <float> [#uses=3]
  %hit.1 = phi i32 [ %hit.0, %BB0326 ], [ 0, %BB0321 ] ; <i32> [#uses=2]
  %indvar.i = phi i32 [ %indvar.next.i, %BB0326 ], [ 0, %BB0321 ] ; <i32> [#uses=2]
  %storemerge3.i = mul i32 %indvar.i, %numberOfSphereParameters ; <i32> [#uses=7]
  %add71.i = add i32 %storemerge3.i, %numberOfSphereParameters ; <i32> [#uses=1]
  %tmp113 = add i32 %storemerge3.i, 4             ; <i32> [#uses=1]
  %arrayidx30.i = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp113 ; <float addrspace(1)*> [#uses=1]
  %tmp114 = add i32 %storemerge3.i, 3             ; <i32> [#uses=1]
  %arrayidx23.i = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp114 ; <float addrspace(1)*> [#uses=1]
  %tmp115 = add i32 %storemerge3.i, 2             ; <i32> [#uses=1]
  %arrayidx17.i = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp115 ; <float addrspace(1)*> [#uses=1]
  %tmp117 = add i32 %storemerge3.i, 1             ; <i32> [#uses=1]
  %arrayidx11.i = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp117 ; <float addrspace(1)*> [#uses=1]
  %arrayidx.i = getelementptr float addrspace(1)* %pSphereArray, i32 %storemerge3.i ; <float addrspace(1)*> [#uses=1]
  %tmp8.i = load float addrspace(1)* %arrayidx.i  ; <float> [#uses=1]
  %tmp12.i = load float addrspace(1)* %arrayidx11.i ; <float> [#uses=1]
  %tmp18.i = load float addrspace(1)* %arrayidx17.i ; <float> [#uses=1]
  %tmp24.i = load float addrspace(1)* %arrayidx23.i ; <float> [#uses=1]
  %tmp31.i = load float addrspace(1)* %arrayidx30.i ; <float> [#uses=2]
  %sub.i172 = fsub float %origin.0.ph137, %tmp8.i ; <float> [#uses=1]
  %sub.i173 = fsub float %origin.0.ph138, %tmp12.i ; <float> [#uses=1]
  %sub.i174 = fsub float %origin.0.ph139, %tmp18.i ; <float> [#uses=1]
  %sub.i175 = fsub float %origin.0.ph140, %tmp24.i ; <float> [#uses=1]
  %temp.vect176 = insertelement <4 x float> undef, float %sub.i172, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect177 = insertelement <4 x float> %temp.vect176, float %sub.i173, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect178 = insertelement <4 x float> %temp.vect177, float %sub.i174, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect179 = insertelement <4 x float> %temp.vect178, float %sub.i175, i32 3 ; <<4 x float>> [#uses=3]
  %call.i = tail call float @_Z3dotU8__vector4fS_(<4 x float> %temp.vect179, <4 x float> %call) nounwind ; <float> [#uses=3]
  %call38.i = tail call float @_Z3dotU8__vector4fS_(<4 x float> %temp.vect179, <4 x float> %temp.vect179) nounwind ; <float> [#uses=1]
  %mul41.i = fmul float %tmp31.i, %tmp31.i        ; <float> [#uses=1]
  %sub42.i = fsub float %call38.i, %mul41.i       ; <float> [#uses=1]
  %mul45.i = fmul float %call.i, %call.i          ; <float> [#uses=1]
  %sub47.i = fsub float %mul45.i, %sub42.i        ; <float> [#uses=2]
  %cmp49.i = fcmp ogt float %sub47.i, 0.000000e+00 ; <i1> [#uses=1]
  br i1 %cmp49.i, label %BB0323, label %BB0326

BB0323:                                           ; preds = %BB0322
  %neg.i = fsub float -0.000000e+00, %call.i      ; <float> [#uses=1]
  %call52.i = tail call float @_Z4sqrtf(float %sub47.i) nounwind ; <float> [#uses=1]
  %sub53.i = fsub float %neg.i, %call52.i         ; <float> [#uses=3]
  %conv.i = fpext float %sub53.i to double        ; <double> [#uses=1]
  %cmp55.i = fcmp ogt double %conv.i, 0.000000e+00 ; <i1> [#uses=1]
  %cmp60.i = fcmp olt float %sub53.i, %t.1        ; <i1> [#uses=1]
  %or.cond = and i1 %cmp55.i, %cmp60.i            ; <i1> [#uses=1]
  br i1 %or.cond, label %BB0324, label %BB0325

BB0324:                                           ; preds = %BB0323
  br label %BB0325

BB0325:                                           ; preds = %BB0323, %BB0324
  %new_phi306 = phi i32 [ 1, %BB0324 ], [ %hit.1, %BB0323 ] ; <i32> [#uses=1]
  %new_phi305 = phi float [ %sub53.i, %BB0324 ], [ %t.1, %BB0323 ] ; <float> [#uses=1]
  %new_phi = phi i32 [ %storemerge3.i, %BB0324 ], [ %sphereNum.1, %BB0323 ] ; <i32> [#uses=1]
  br label %BB0326

BB0326:                                           ; preds = %BB0325, %BB0322
  %sphereNum.0 = phi i32 [ %sphereNum.1, %BB0322 ], [ %new_phi, %BB0325 ] ; <i32> [#uses=18]
  %t.0 = phi float [ %t.1, %BB0322 ], [ %new_phi305, %BB0325 ] ; <float> [#uses=5]
  %hit.0 = phi i32 [ %hit.1, %BB0322 ], [ %new_phi306, %BB0325 ] ; <i32> [#uses=2]
  %cmp.i = icmp ult i32 %add71.i, %mul1.i         ; <i1> [#uses=1]
  %indvar.next.i = add i32 %indvar.i, 1           ; <i32> [#uses=1]
  br i1 %cmp.i, label %BB0322, label %BB0327

BB0327:                                           ; preds = %BB0326
  %mul77.i184 = fmul float %t.0, %scalar180       ; <float> [#uses=1]
  %mul77.i185 = fmul float %t.0, %scalar181       ; <float> [#uses=1]
  %mul77.i186 = fmul float %t.0, %scalar182       ; <float> [#uses=1]
  %mul77.i187 = fmul float %t.0, %scalar183       ; <float> [#uses=1]
  %add78.i188 = fadd float %mul77.i184, %origin.0.ph137 ; <float> [#uses=4]
  %add78.i189 = fadd float %mul77.i185, %origin.0.ph138 ; <float> [#uses=4]
  %add78.i190 = fadd float %mul77.i186, %origin.0.ph139 ; <float> [#uses=4]
  %add78.i191 = fadd float %mul77.i187, %origin.0.ph140 ; <float> [#uses=4]
  %cmp62 = icmp eq i32 %hit.0, 0                  ; <i1> [#uses=1]
  br i1 %cmp62, label %BB0316, label %BB0328

BB0328:                                           ; preds = %BB0327
  %arrayidx = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %sphereNum.0 ; <float addrspace(1)*> [#uses=1]
  %tmp67 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %add70 = add nsw i32 %sphereNum.0, 1            ; <i32> [#uses=1]
  %arrayidx72 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add70 ; <float addrspace(1)*> [#uses=1]
  %tmp73 = load float addrspace(1)* %arrayidx72   ; <float> [#uses=1]
  %add76 = add nsw i32 %sphereNum.0, 2            ; <i32> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add76 ; <float addrspace(1)*> [#uses=1]
  %tmp79 = load float addrspace(1)* %arrayidx78   ; <float> [#uses=1]
  %add82 = add nsw i32 %sphereNum.0, 3            ; <i32> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add82 ; <float addrspace(1)*> [#uses=1]
  %tmp85 = load float addrspace(1)* %arrayidx84   ; <float> [#uses=1]
  %add89 = add nsw i32 %sphereNum.0, 4            ; <i32> [#uses=1]
  %arrayidx91 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add89 ; <float addrspace(1)*> [#uses=1]
  %tmp92 = load float addrspace(1)* %arrayidx91   ; <float> [#uses=4]
  %add95 = add nsw i32 %sphereNum.0, 5            ; <i32> [#uses=1]
  %arrayidx97 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add95 ; <float addrspace(1)*> [#uses=1]
  %tmp98 = load float addrspace(1)* %arrayidx97   ; <float> [#uses=2]
  %add101 = add nsw i32 %sphereNum.0, 6           ; <i32> [#uses=1]
  %arrayidx103 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add101 ; <float addrspace(1)*> [#uses=1]
  %tmp104 = load float addrspace(1)* %arrayidx103 ; <float> [#uses=2]
  %add107 = add nsw i32 %sphereNum.0, 7           ; <i32> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add107 ; <float addrspace(1)*> [#uses=1]
  %tmp110 = load float addrspace(1)* %arrayidx109 ; <float> [#uses=2]
  %add113 = add nsw i32 %sphereNum.0, 8           ; <i32> [#uses=1]
  %arrayidx115 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add113 ; <float addrspace(1)*> [#uses=1]
  %tmp116 = load float addrspace(1)* %arrayidx115 ; <float> [#uses=2]
  %add121 = add nsw i32 %sphereNum.0, 9           ; <i32> [#uses=1]
  %arrayidx123 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add121 ; <float addrspace(1)*> [#uses=1]
  %tmp124 = load float addrspace(1)* %arrayidx123 ; <float> [#uses=1]
  %add127 = add nsw i32 %sphereNum.0, 10          ; <i32> [#uses=1]
  %arrayidx129 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add127 ; <float addrspace(1)*> [#uses=1]
  %tmp130 = load float addrspace(1)* %arrayidx129 ; <float> [#uses=1]
  %add133 = add nsw i32 %sphereNum.0, 11          ; <i32> [#uses=1]
  %arrayidx135 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add133 ; <float addrspace(1)*> [#uses=1]
  %tmp136 = load float addrspace(1)* %arrayidx135 ; <float> [#uses=1]
  %add139 = add nsw i32 %sphereNum.0, 12          ; <i32> [#uses=1]
  %arrayidx141 = getelementptr inbounds float addrspace(1)* %pSphereArray, i32 %add139 ; <float addrspace(1)*> [#uses=1]
  %tmp142 = load float addrspace(1)* %arrayidx141 ; <float> [#uses=5]
  %sub147192 = fsub float %add78.i188, %tmp67     ; <float> [#uses=1]
  %sub147193 = fsub float %add78.i189, %tmp73     ; <float> [#uses=1]
  %sub147194 = fsub float %add78.i190, %tmp79     ; <float> [#uses=1]
  %sub147195 = fsub float %add78.i191, %tmp85     ; <float> [#uses=1]
  %div153196 = fdiv float %sub147192, %tmp92      ; <float> [#uses=3]
  %div153197 = fdiv float %sub147193, %tmp92      ; <float> [#uses=3]
  %div153198 = fdiv float %sub147194, %tmp92      ; <float> [#uses=3]
  %div153199 = fdiv float %sub147195, %tmp92      ; <float> [#uses=3]
  %temp.vect228 = insertelement <4 x float> undef, float %div153196, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect229 = insertelement <4 x float> %temp.vect228, float %div153197, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect230 = insertelement <4 x float> %temp.vect229, float %div153198, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect231 = insertelement <4 x float> %temp.vect230, float %div153199, i32 3 ; <<4 x float>> [#uses=4]
  %sub156204 = fsub float %scalar200, %add78.i188 ; <float> [#uses=2]
  %sub156205 = fsub float %scalar201, %add78.i189 ; <float> [#uses=2]
  %sub156206 = fsub float %scalar202, %add78.i190 ; <float> [#uses=2]
  %sub156207 = fsub float %scalar203, %add78.i191 ; <float> [#uses=2]
  %temp.vect208 = insertelement <4 x float> undef, float %sub156204, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect209 = insertelement <4 x float> %temp.vect208, float %sub156205, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect210 = insertelement <4 x float> %temp.vect209, float %sub156206, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect211 = insertelement <4 x float> %temp.vect210, float %sub156207, i32 3 ; <<4 x float>> [#uses=1]
  %call158 = tail call float @_Z6lengthU8__vector4f(<4 x float> %temp.vect211) nounwind ; <float> [#uses=5]
  %div165212 = fdiv float %sub156204, %call158    ; <float> [#uses=2]
  %div165213 = fdiv float %sub156205, %call158    ; <float> [#uses=2]
  %div165214 = fdiv float %sub156206, %call158    ; <float> [#uses=2]
  %div165215 = fdiv float %sub156207, %call158    ; <float> [#uses=2]
  %temp.vect224 = insertelement <4 x float> undef, float %div165212, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect225 = insertelement <4 x float> %temp.vect224, float %div165213, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect226 = insertelement <4 x float> %temp.vect225, float %div165214, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect227 = insertelement <4 x float> %temp.vect226, float %div165215, i32 3 ; <<4 x float>> [#uses=2]
  br i1 %cmp2.i, label %BB0338, label %BB0329

BB0329:                                           ; preds = %BB0328
  br label %BB0330

BB0330:                                           ; preds = %BB0329, %BB0334
  %t.3 = phi float [ %t.2, %BB0334 ], [ 9.999900e+04, %BB0329 ] ; <float> [#uses=3]
  %shadowTest.2 = phi i32 [ %shadowTest.1, %BB0334 ], [ 0, %BB0329 ] ; <i32> [#uses=2]
  %indvar.i5 = phi i32 [ %indvar.next.i46, %BB0334 ], [ 0, %BB0329 ] ; <i32> [#uses=2]
  %tmp = mul i32 %indvar.i5, %numberOfSphereParameters ; <i32> [#uses=6]
  %add71.i16 = add i32 %tmp, %numberOfSphereParameters ; <i32> [#uses=1]
  %tmp102 = add i32 %tmp, 4                       ; <i32> [#uses=1]
  %arrayidx30.i15 = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp102 ; <float addrspace(1)*> [#uses=1]
  %tmp103 = add i32 %tmp, 3                       ; <i32> [#uses=1]
  %arrayidx23.i13 = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp103 ; <float addrspace(1)*> [#uses=1]
  %tmp105 = add i32 %tmp, 2                       ; <i32> [#uses=1]
  %arrayidx17.i11 = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp105 ; <float addrspace(1)*> [#uses=1]
  %tmp106 = add i32 %tmp, 1                       ; <i32> [#uses=1]
  %arrayidx11.i9 = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp106 ; <float addrspace(1)*> [#uses=1]
  %arrayidx.i7 = getelementptr float addrspace(1)* %pSphereArray, i32 %tmp ; <float addrspace(1)*> [#uses=1]
  %tmp8.i17 = load float addrspace(1)* %arrayidx.i7 ; <float> [#uses=1]
  %tmp12.i19 = load float addrspace(1)* %arrayidx11.i9 ; <float> [#uses=1]
  %tmp18.i21 = load float addrspace(1)* %arrayidx17.i11 ; <float> [#uses=1]
  %tmp24.i23 = load float addrspace(1)* %arrayidx23.i13 ; <float> [#uses=1]
  %tmp31.i25 = load float addrspace(1)* %arrayidx30.i15 ; <float> [#uses=2]
  %sub.i26216 = fsub float %add78.i188, %tmp8.i17 ; <float> [#uses=1]
  %sub.i26217 = fsub float %add78.i189, %tmp12.i19 ; <float> [#uses=1]
  %sub.i26218 = fsub float %add78.i190, %tmp18.i21 ; <float> [#uses=1]
  %sub.i26219 = fsub float %add78.i191, %tmp24.i23 ; <float> [#uses=1]
  %temp.vect220 = insertelement <4 x float> undef, float %sub.i26216, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect221 = insertelement <4 x float> %temp.vect220, float %sub.i26217, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect222 = insertelement <4 x float> %temp.vect221, float %sub.i26218, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect223 = insertelement <4 x float> %temp.vect222, float %sub.i26219, i32 3 ; <<4 x float>> [#uses=3]
  %call.i27 = tail call float @_Z3dotU8__vector4fS_(<4 x float> %temp.vect223, <4 x float> %temp.vect227) nounwind ; <float> [#uses=3]
  %call38.i28 = tail call float @_Z3dotU8__vector4fS_(<4 x float> %temp.vect223, <4 x float> %temp.vect223) nounwind ; <float> [#uses=1]
  %mul41.i29 = fmul float %tmp31.i25, %tmp31.i25  ; <float> [#uses=1]
  %sub42.i30 = fsub float %call38.i28, %mul41.i29 ; <float> [#uses=1]
  %mul45.i31 = fmul float %call.i27, %call.i27    ; <float> [#uses=1]
  %sub47.i32 = fsub float %mul45.i31, %sub42.i30  ; <float> [#uses=2]
  %cmp49.i33 = fcmp ogt float %sub47.i32, 0.000000e+00 ; <i1> [#uses=1]
  br i1 %cmp49.i33, label %BB0331, label %BB0334

BB0331:                                           ; preds = %BB0330
  %neg.i35 = fsub float -0.000000e+00, %call.i27  ; <float> [#uses=1]
  %call52.i36 = tail call float @_Z4sqrtf(float %sub47.i32) nounwind ; <float> [#uses=1]
  %sub53.i37 = fsub float %neg.i35, %call52.i36   ; <float> [#uses=3]
  %conv.i38 = fpext float %sub53.i37 to double    ; <double> [#uses=1]
  %cmp55.i39 = fcmp ogt double %conv.i38, 0.000000e+00 ; <i1> [#uses=1]
  %cmp60.i42 = fcmp olt float %sub53.i37, %t.3    ; <i1> [#uses=1]
  %or.cond67 = and i1 %cmp55.i39, %cmp60.i42      ; <i1> [#uses=1]
  br i1 %or.cond67, label %BB0332, label %BB0333

BB0332:                                           ; preds = %BB0331
  br label %BB0333

BB0333:                                           ; preds = %BB0331, %BB0332
  %new_phi309 = phi i32 [ 1, %BB0332 ], [ %shadowTest.2, %BB0331 ] ; <i32> [#uses=1]
  %new_phi308 = phi float [ %sub53.i37, %BB0332 ], [ %t.3, %BB0331 ] ; <float> [#uses=1]
  br label %BB0334

BB0334:                                           ; preds = %BB0333, %BB0330
  %t.2 = phi float [ %t.3, %BB0330 ], [ %new_phi308, %BB0333 ] ; <float> [#uses=2]
  %shadowTest.1 = phi i32 [ %shadowTest.2, %BB0330 ], [ %new_phi309, %BB0333 ] ; <i32> [#uses=3]
  %cmp.i45 = icmp ult i32 %add71.i16, %mul1.i     ; <i1> [#uses=1]
  %indvar.next.i46 = add i32 %indvar.i5, 1        ; <i32> [#uses=1]
  br i1 %cmp.i45, label %BB0330, label %BB0335

BB0335:                                           ; preds = %BB0334
  %cmp172 = icmp eq i32 %shadowTest.1, 0          ; <i1> [#uses=1]
  br i1 %cmp172, label %BB0338, label %BB0336

BB0336:                                           ; preds = %BB0335
  %cmp177 = fcmp olt float %t.2, %call158         ; <i1> [#uses=1]
  br i1 %cmp177, label %BB0337, label %BB0339

BB0337:                                           ; preds = %BB0336
  br label %BB0340

BB0338:                                           ; preds = %BB0335, %BB0328
  %new_phi311 = phi i32 [ 1, %BB0328 ], [ 1, %BB0335 ] ; <i32> [#uses=1]
  br label %BB0339

BB0339:                                           ; preds = %BB0336, %BB0338
  %new_phi313 = phi i32 [ %new_phi311, %BB0338 ], [ %shadowTest.1, %BB0336 ] ; <i32> [#uses=1]
  br label %BB0340

BB0340:                                           ; preds = %BB0339, %BB0337
  %shadowTest.0 = phi i32 [ 0, %BB0337 ], [ %new_phi313, %BB0339 ] ; <i32> [#uses=1]
  %call183 = tail call float @_Z3dotU8__vector4fS_(<4 x float> %temp.vect227, <4 x float> %temp.vect231) nounwind ; <float> [#uses=2]
  %mul186 = fmul float %call183, 2.000000e+00     ; <float> [#uses=4]
  %mul190232 = fmul float %mul186, %div153196     ; <float> [#uses=1]
  %mul190233 = fmul float %mul186, %div153197     ; <float> [#uses=1]
  %mul190234 = fmul float %mul186, %div153198     ; <float> [#uses=1]
  %mul190235 = fmul float %mul186, %div153199     ; <float> [#uses=1]
  %sub191236 = fsub float %div165212, %mul190232  ; <float> [#uses=1]
  %sub191237 = fsub float %div165213, %mul190233  ; <float> [#uses=1]
  %sub191238 = fsub float %div165214, %mul190234  ; <float> [#uses=1]
  %sub191239 = fsub float %div165215, %mul190235  ; <float> [#uses=1]
  %temp.vect240 = insertelement <4 x float> undef, float %sub191236, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect241 = insertelement <4 x float> %temp.vect240, float %sub191237, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect242 = insertelement <4 x float> %temp.vect241, float %sub191238, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect243 = insertelement <4 x float> %temp.vect242, float %sub191239, i32 3 ; <<4 x float>> [#uses=1]
  %call194 = tail call float @_Z3dotU8__vector4fS_(<4 x float> %call, <4 x float> %temp.vect243) nounwind ; <float> [#uses=1]
  %call196 = tail call float @_Z3maxff(float %call183, float 0.000000e+00) nounwind ; <float> [#uses=1]
  %call198 = tail call float @_Z3maxff(float %call194, float 0.000000e+00) nounwind ; <float> [#uses=1]
  %tmp199 = load float addrspace(1)* @SPECULAR_EXPONENT ; <float> [#uses=1]
  %call200 = tail call float @_Z3powff(float %call198, float %tmp199) nounwind ; <float> [#uses=1]
  %cmp202 = icmp eq i32 %sphereNum.0, 13          ; <i1> [#uses=1]
  br i1 %cmp202, label %BB0341, label %BB0342

BB0341:                                           ; preds = %BB0340
  %call208 = tail call float @_Z3dotU8__vector4fS_(<4 x float> <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <4 x float> %temp.vect231) nounwind ; <float> [#uses=1]
  %neg209 = fsub float -0.000000e+00, %call208    ; <float> [#uses=1]
  %call210 = tail call float @_Z4acosf(float %neg209) nounwind ; <float> [#uses=2]
  %call215 = tail call float @_Z3dotU8__vector4fS_(<4 x float> <float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0.000000e+00>, <4 x float> %temp.vect231) nounwind ; <float> [#uses=1]
  %call217 = tail call float @_Z3sinf(float %call210) nounwind ; <float> [#uses=1]
  %div220 = fdiv float %call215, %call217         ; <float> [#uses=1]
  %call221 = tail call float @_Z4acosf(float %div220) nounwind ; <float> [#uses=1]
  %div226 = fdiv float %call221, 0x401921FB60000000 ; <float> [#uses=1]
  %div232 = fdiv float %call210, 0x400921FB60000000 ; <float> [#uses=1]
  %mul237 = fmul float %div226, 2.000000e+03      ; <float> [#uses=1]
  %call238 = tail call float @_Z5floorf(float %mul237) nounwind ; <float> [#uses=1]
  %mul241 = fmul float %div232, 2.000000e+03      ; <float> [#uses=1]
  %call242 = tail call float @_Z5floorf(float %mul241) nounwind ; <float> [#uses=1]
  %add243 = fadd float %call238, %call242         ; <float> [#uses=1]
  %call244 = tail call float @_Z4fmodff(float %add243, float 2.000000e+00) nounwind ; <float> [#uses=1]
  %cmp245 = fcmp oeq float %call244, 0.000000e+00 ; <i1> [#uses=1]
  %cond = select i1 %cmp245, float 5.000000e-01, float 1.000000e+00 ; <float> [#uses=4]
  %mul250244 = fmul float %tmp98, %cond           ; <float> [#uses=1]
  %mul250245 = fmul float %tmp104, %cond          ; <float> [#uses=1]
  %mul250246 = fmul float %tmp110, %cond          ; <float> [#uses=1]
  %mul250247 = fmul float %tmp116, %cond          ; <float> [#uses=1]
  br label %BB0342

BB0342:                                           ; preds = %BB0341, %BB0340
  %sphereColor.0248 = phi float [ %mul250244, %BB0341 ], [ %tmp98, %BB0340 ] ; <float> [#uses=2]
  %sphereColor.0249 = phi float [ %mul250245, %BB0341 ], [ %tmp104, %BB0340 ] ; <float> [#uses=2]
  %sphereColor.0250 = phi float [ %mul250246, %BB0341 ], [ %tmp110, %BB0340 ] ; <float> [#uses=2]
  %sphereColor.0251 = phi float [ %mul250247, %BB0341 ], [ %tmp116, %BB0340 ] ; <float> [#uses=2]
  %conv255 = sitofp i32 %shadowTest.0 to float    ; <float> [#uses=1]
  %mul259 = fmul float %call196, %tmp130          ; <float> [#uses=1]
  %mul263 = fmul float %call200, %tmp136          ; <float> [#uses=1]
  %add264 = fadd float %mul259, %mul263           ; <float> [#uses=1]
  %mul265 = fmul float %conv255, %add264          ; <float> [#uses=1]
  %add266 = fadd float %tmp124, %mul265           ; <float> [#uses=4]
  %mul271252 = fmul float %colorScale.0.ph145, %add266 ; <float> [#uses=1]
  %mul271253 = fmul float %colorScale.0.ph146, %add266 ; <float> [#uses=1]
  %mul271254 = fmul float %colorScale.0.ph147, %add266 ; <float> [#uses=1]
  %mul271255 = fmul float %colorScale.0.ph148, %add266 ; <float> [#uses=1]
  %mul273256 = fmul float %mul271252, %sphereColor.0248 ; <float> [#uses=1]
  %mul273257 = fmul float %mul271253, %sphereColor.0249 ; <float> [#uses=1]
  %mul273258 = fmul float %mul271254, %sphereColor.0250 ; <float> [#uses=1]
  %mul273259 = fmul float %mul271255, %sphereColor.0251 ; <float> [#uses=1]
  %add275260 = fadd float %dst.0.ph70153, %mul273256 ; <float> [#uses=1]
  %add275261 = fadd float %dst.0.ph70154, %mul273257 ; <float> [#uses=1]
  %add275262 = fadd float %dst.0.ph70155, %mul273258 ; <float> [#uses=1]
  %add275263 = fadd float %dst.0.ph70156, %mul273259 ; <float> [#uses=1]
  %temp.vect264 = insertelement <4 x float> undef, float %add275260, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect265 = insertelement <4 x float> %temp.vect264, float %add275261, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect266 = insertelement <4 x float> %temp.vect265, float %add275262, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect267 = insertelement <4 x float> %temp.vect266, float %add275263, i32 3 ; <<4 x float>> [#uses=1]
  %call277 = tail call <4 x float> @_Z5clampU8__vector4fff(<4 x float> %temp.vect267, float 0.000000e+00, float 1.000000e+00) nounwind ; <<4 x float>> [#uses=4]
  %scalar296 = extractelement <4 x float> %call277, i32 0 ; <float> [#uses=2]
  %scalar297 = extractelement <4 x float> %call277, i32 1 ; <float> [#uses=2]
  %scalar298 = extractelement <4 x float> %call277, i32 2 ; <float> [#uses=2]
  %scalar299 = extractelement <4 x float> %call277, i32 3 ; <float> [#uses=2]
  %cmp280 = fcmp ogt float %tmp142, 0.000000e+00  ; <i1> [#uses=1]
  br i1 %cmp280, label %BB0343, label %BB0315

BB0343:                                           ; preds = %BB0342
  %call286 = tail call float @_Z3dotU8__vector4fS_(<4 x float> %call, <4 x float> %temp.vect231) nounwind ; <float> [#uses=1]
  %mul287 = fmul float %call286, 2.000000e+00     ; <float> [#uses=4]
  %mul291268 = fmul float %mul287, %div153196     ; <float> [#uses=1]
  %mul291269 = fmul float %mul287, %div153197     ; <float> [#uses=1]
  %mul291270 = fmul float %mul287, %div153198     ; <float> [#uses=1]
  %mul291271 = fmul float %mul287, %div153199     ; <float> [#uses=1]
  %sub292272 = fsub float %scalar180, %mul291268  ; <float> [#uses=1]
  %sub292273 = fsub float %scalar181, %mul291269  ; <float> [#uses=1]
  %sub292274 = fsub float %scalar182, %mul291270  ; <float> [#uses=1]
  %sub292275 = fsub float %scalar183, %mul291271  ; <float> [#uses=1]
  %temp.vect276 = insertelement <4 x float> undef, float %sub292272, i32 0 ; <<4 x float>> [#uses=1]
  %temp.vect277 = insertelement <4 x float> %temp.vect276, float %sub292273, i32 1 ; <<4 x float>> [#uses=1]
  %temp.vect278 = insertelement <4 x float> %temp.vect277, float %sub292274, i32 2 ; <<4 x float>> [#uses=1]
  %temp.vect279 = insertelement <4 x float> %temp.vect278, float %sub292275, i32 3 ; <<4 x float>> [#uses=1]
  %call294 = tail call <4 x float> @_Z9normalizeU8__vector4f(<4 x float> %temp.vect279) nounwind ; <<4 x float>> [#uses=4]
  %scalar300 = extractelement <4 x float> %call294, i32 0 ; <float> [#uses=1]
  %scalar301 = extractelement <4 x float> %call294, i32 1 ; <float> [#uses=1]
  %scalar302 = extractelement <4 x float> %call294, i32 2 ; <float> [#uses=1]
  %scalar303 = extractelement <4 x float> %call294, i32 3 ; <float> [#uses=1]
  %dec = add nsw i32 %rayShots.0.ph77, -1         ; <i32> [#uses=1]
  %mul303280 = fmul float %tmp142, %sphereColor.0248 ; <float> [#uses=1]
  %mul303281 = fmul float %tmp142, %sphereColor.0249 ; <float> [#uses=1]
  %mul303282 = fmul float %tmp142, %sphereColor.0250 ; <float> [#uses=1]
  %mul303283 = fmul float %tmp142, %sphereColor.0251 ; <float> [#uses=1]
  %mul305284 = fmul float %colorScale.0.ph145, %mul303280 ; <float> [#uses=1]
  %mul305285 = fmul float %colorScale.0.ph146, %mul303281 ; <float> [#uses=1]
  %mul305286 = fmul float %colorScale.0.ph147, %mul303282 ; <float> [#uses=1]
  %mul305287 = fmul float %colorScale.0.ph148, %mul303283 ; <float> [#uses=1]
  br label %BB0314

BB0344:                                           ; preds = %BB0318
  br label %BB0346

BB0345:                                           ; preds = %BB0320
  br label %BB0346

BB0346:                                           ; preds = %BB0345, %BB0344
  ret <4 x float> %temp.vect291
}

declare <4 x float> @_Z9normalizeU8__vector4f(<4 x float>)

declare float @_Z6lengthU8__vector4f(<4 x float>)

declare float @_Z3maxff(float, float)

declare float @_Z4acosf(float)

declare float @_Z4fmodff(float, float)

declare float @_Z5floorf(float)

declare <4 x float> @_Z5clampU8__vector4fff(<4 x float>, float, float)

declare i32 @get_global_size(i32)

declare i32 @get_global_id(i32)
