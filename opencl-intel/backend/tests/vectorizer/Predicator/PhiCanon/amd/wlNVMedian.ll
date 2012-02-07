; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlNVMedian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [7 x i8] c"229000\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @ckMedian
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %if.then46, %if.else58
; CHECK: phi-split-bb1:                                    ; preds = %if.then202, %if.else219
; CHECK: phi-split-bb2:                                    ; preds = %if.end181, %phi-split-bb1
; CHECK: phi-split-bb3:                                    ; preds = %if.else140, %phi-split-bb2
; CHECK: phi-split-bb4:                                    ; preds = %if.then117, %if.else134
; CHECK: phi-split-bb5:                                    ; preds = %if.end100, %phi-split-bb4
; CHECK: ret

define void @ckMedian(<4 x i8> addrspace(1)* %uc4Source, i32 addrspace(1)* %uiDest, <4 x i8> addrspace(3)* %uc4LocalData, i32 %iLocalPixPitch, i32 %uiImageWidth, i32 %uiDevImageHeight, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 1) nounwind
  %sub = add i32 %call1, -1
  %call2 = call i32 @get_global_size(i32 0) nounwind
  %call3 = call i32 @__mul24_1i32(i32 %sub, i32 %call2) nounwind
  %add = add i32 %call3, %call
  %call6 = call i32 @get_local_id(i32 1) nounwind
  %call8 = call i32 @__mul24_1i32(i32 %call6, i32 %iLocalPixPitch) nounwind
  %call9 = call i32 @get_local_id(i32 0) nounwind
  %add10 = add i32 %call8, 1
  %add11 = add i32 %add10, %call9
  %cmp = icmp sgt i32 %sub, -1
  %cmp15 = icmp slt i32 %sub, %uiDevImageHeight
  %or.cond = and i1 %cmp, %cmp15
  %cmp19 = icmp slt i32 %call, %uiImageWidth
  %or.cond1 = and i1 %or.cond, %cmp19
  %arrayidx = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add11
  br i1 %or.cond1, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx24 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %add
  %tmp25 = load <4 x i8> addrspace(1)* %arrayidx24, align 4
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %storemerge6 = phi <4 x i8> [ %tmp25, %if.then ], [ zeroinitializer, %entry ]
  store <4 x i8> %storemerge6, <4 x i8> addrspace(3)* %arrayidx, align 4
  %call29 = call i32 @get_local_id(i32 1) nounwind
  %cmp30 = icmp ult i32 %call29, 2
  br i1 %cmp30, label %if.then31, label %if.end63

if.then31:                                        ; preds = %if.end
  %call33 = call i32 @get_local_size(i32 1) nounwind
  %call35 = call i32 @__mul24_1i32(i32 %call33, i32 %iLocalPixPitch) nounwind
  %add36 = add i32 %call35, %add11
  %call38 = call i32 @get_local_size(i32 1) nounwind
  %add39 = add i32 %call38, %sub
  %cmp41 = icmp ult i32 %add39, %uiDevImageHeight
  %or.cond2 = and i1 %cmp41, %cmp19
  %arrayidx49 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add36
  br i1 %or.cond2, label %if.then46, label %if.else58

if.then46:                                        ; preds = %if.then31
  %call51 = call i32 @get_local_size(i32 1) nounwind
  %call52 = call i32 @get_global_size(i32 0) nounwind
  %call53 = call i32 @__mul24_1u32(i32 %call51, i32 %call52) nounwind
  %add54 = add i32 %call53, %add
  %arrayidx56 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %add54
  %tmp57 = load <4 x i8> addrspace(1)* %arrayidx56, align 4
  store <4 x i8> %tmp57, <4 x i8> addrspace(3)* %arrayidx49, align 4
  br label %if.end63

if.else58:                                        ; preds = %if.then31
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx49, align 4
  br label %if.end63

if.end63:                                         ; preds = %if.then46, %if.else58, %if.end
  %call64 = call i32 @get_local_id(i32 0) nounwind
  %call65 = call i32 @get_local_size(i32 0) nounwind
  %sub66 = add i32 %call65, -1
  %cmp67 = icmp eq i32 %call64, %sub66
  br i1 %cmp67, label %if.then68, label %if.else140

if.then68:                                        ; preds = %if.end63
  %call69 = call i32 @get_local_id(i32 1) nounwind
  %call71 = call i32 @__mul24_1i32(i32 %call69, i32 %iLocalPixPitch) nounwind
  br i1 %or.cond, label %land.lhs.true78, label %if.else96

land.lhs.true78:                                  ; preds = %if.then68
  %call79 = call i32 @get_group_id(i32 0) nounwind
  %cmp80 = icmp eq i32 %call79, 0
  br i1 %cmp80, label %if.else96, label %if.then81

if.then81:                                        ; preds = %land.lhs.true78
  %arrayidx84 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %call71
  %call86 = call i32 @get_global_size(i32 0) nounwind
  %call87 = call i32 @__mul24_1i32(i32 %sub, i32 %call86) nounwind
  %call88 = call i32 @get_group_id(i32 0) nounwind
  %call89 = call i32 @get_local_size(i32 0) nounwind
  %call90 = call i32 @__mul24_1u32(i32 %call88, i32 %call89) nounwind
  %add91 = add i32 %call87, -1
  %sub92 = add i32 %add91, %call90
  %arrayidx94 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %sub92
  %tmp95 = load <4 x i8> addrspace(1)* %arrayidx94, align 4
  store <4 x i8> %tmp95, <4 x i8> addrspace(3)* %arrayidx84, align 4
  br label %if.end100

if.else96:                                        ; preds = %land.lhs.true78, %if.then68
  %arrayidx99 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %call71
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx99, align 4
  br label %if.end100

if.end100:                                        ; preds = %if.else96, %if.then81
  %call101 = call i32 @get_local_id(i32 1) nounwind
  %cmp102 = icmp ult i32 %call101, 2
  br i1 %cmp102, label %if.then103, label %if.end226

if.then103:                                       ; preds = %if.end100
  %call105 = call i32 @get_local_size(i32 1) nounwind
  %call107 = call i32 @__mul24_1i32(i32 %call105, i32 %iLocalPixPitch) nounwind
  %add108 = add i32 %call107, %call71
  %call110 = call i32 @get_local_size(i32 1) nounwind
  %add111 = add i32 %call110, %sub
  %cmp113 = icmp ult i32 %add111, %uiDevImageHeight
  br i1 %cmp113, label %land.lhs.true114, label %if.else134

land.lhs.true114:                                 ; preds = %if.then103
  %call115 = call i32 @get_group_id(i32 0) nounwind
  %cmp116 = icmp eq i32 %call115, 0
  br i1 %cmp116, label %if.else134, label %if.then117

if.then117:                                       ; preds = %land.lhs.true114
  %arrayidx120 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add108
  %call122 = call i32 @get_local_size(i32 1) nounwind
  %add123 = add i32 %call122, %sub
  %call124 = call i32 @get_global_size(i32 0) nounwind
  %call125 = call i32 @__mul24_1i32(i32 %add123, i32 %call124) nounwind
  %call126 = call i32 @get_group_id(i32 0) nounwind
  %call127 = call i32 @get_local_size(i32 0) nounwind
  %call128 = call i32 @__mul24_1u32(i32 %call126, i32 %call127) nounwind
  %add129 = add i32 %call125, -1
  %sub130 = add i32 %add129, %call128
  %arrayidx132 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %sub130
  %tmp133 = load <4 x i8> addrspace(1)* %arrayidx132, align 4
  store <4 x i8> %tmp133, <4 x i8> addrspace(3)* %arrayidx120, align 4
  br label %if.end226

if.else134:                                       ; preds = %land.lhs.true114, %if.then103
  %arrayidx137 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add108
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx137, align 4
  br label %if.end226

if.else140:                                       ; preds = %if.end63
  %call141 = call i32 @get_local_id(i32 0) nounwind
  %cmp142 = icmp eq i32 %call141, 0
  br i1 %cmp142, label %if.then143, label %if.end226

if.then143:                                       ; preds = %if.else140
  %call144 = call i32 @get_local_id(i32 1) nounwind
  %add145 = add i32 %call144, 1
  %call147 = call i32 @__mul24_1i32(i32 %add145, i32 %iLocalPixPitch) nounwind
  %sub148 = add i32 %call147, -1
  br i1 %or.cond, label %land.lhs.true155, label %if.else177

land.lhs.true155:                                 ; preds = %if.then143
  %call156 = call i32 @get_group_id(i32 0) nounwind
  %add157 = add i32 %call156, 1
  %call158 = call i32 @get_local_size(i32 0) nounwind
  %call159 = call i32 @__mul24_1i32(i32 %add157, i32 %call158) nounwind
  %cmp161 = icmp slt i32 %call159, %uiImageWidth
  br i1 %cmp161, label %if.then162, label %if.else177

if.then162:                                       ; preds = %land.lhs.true155
  %arrayidx165 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %sub148
  %call167 = call i32 @get_global_size(i32 0) nounwind
  %call168 = call i32 @__mul24_1i32(i32 %sub, i32 %call167) nounwind
  %call169 = call i32 @get_group_id(i32 0) nounwind
  %add170 = add i32 %call169, 1
  %call171 = call i32 @get_local_size(i32 0) nounwind
  %call172 = call i32 @__mul24_1u32(i32 %add170, i32 %call171) nounwind
  %add173 = add i32 %call172, %call168
  %arrayidx175 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %add173
  %tmp176 = load <4 x i8> addrspace(1)* %arrayidx175, align 4
  store <4 x i8> %tmp176, <4 x i8> addrspace(3)* %arrayidx165, align 4
  br label %if.end181

if.else177:                                       ; preds = %land.lhs.true155, %if.then143
  %arrayidx180 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %sub148
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx180, align 4
  br label %if.end181

if.end181:                                        ; preds = %if.else177, %if.then162
  %call182 = call i32 @get_local_id(i32 1) nounwind
  %cmp183 = icmp ult i32 %call182, 2
  br i1 %cmp183, label %if.then184, label %if.end226

if.then184:                                       ; preds = %if.end181
  %call186 = call i32 @get_local_size(i32 1) nounwind
  %call188 = call i32 @__mul24_1i32(i32 %call186, i32 %iLocalPixPitch) nounwind
  %add189 = add i32 %call188, %sub148
  %call191 = call i32 @get_local_size(i32 1) nounwind
  %add192 = add i32 %call191, %sub
  %cmp194 = icmp ult i32 %add192, %uiDevImageHeight
  br i1 %cmp194, label %land.lhs.true195, label %if.else219

land.lhs.true195:                                 ; preds = %if.then184
  %call196 = call i32 @get_group_id(i32 0) nounwind
  %add197 = add i32 %call196, 1
  %call198 = call i32 @get_local_size(i32 0) nounwind
  %call199 = call i32 @__mul24_1u32(i32 %add197, i32 %call198) nounwind
  %cmp201 = icmp ult i32 %call199, %uiImageWidth
  br i1 %cmp201, label %if.then202, label %if.else219

if.then202:                                       ; preds = %land.lhs.true195
  %arrayidx205 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add189
  %call207 = call i32 @get_local_size(i32 1) nounwind
  %add208 = add i32 %call207, %sub
  %call209 = call i32 @get_global_size(i32 0) nounwind
  %call210 = call i32 @__mul24_1i32(i32 %add208, i32 %call209) nounwind
  %call211 = call i32 @get_group_id(i32 0) nounwind
  %add212 = add i32 %call211, 1
  %call213 = call i32 @get_local_size(i32 0) nounwind
  %call214 = call i32 @__mul24_1u32(i32 %add212, i32 %call213) nounwind
  %add215 = add i32 %call214, %call210
  %arrayidx217 = getelementptr <4 x i8> addrspace(1)* %uc4Source, i32 %add215
  %tmp218 = load <4 x i8> addrspace(1)* %arrayidx217, align 4
  store <4 x i8> %tmp218, <4 x i8> addrspace(3)* %arrayidx205, align 4
  br label %if.end226

if.else219:                                       ; preds = %land.lhs.true195, %if.then184
  %arrayidx222 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add189
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx222, align 4
  br label %if.end226

if.end226:                                        ; preds = %if.else140, %if.then202, %if.else219, %if.end181, %if.end100, %if.else134, %if.then117
  call void @barrier(i32 1) nounwind
  br label %for.body

for.body:                                         ; preds = %if.end226, %for.body
  %storemerge9 = phi i32 [ 0, %if.end226 ], [ %inc750, %for.body ]
  %0 = phi float [ 1.280000e+02, %if.end226 ], [ %mul, %for.body ]
  %1 = phi float [ 1.280000e+02, %if.end226 ], [ %mul738, %for.body ]
  %2 = phi float [ 1.280000e+02, %if.end226 ], [ %mul748, %for.body ]
  %3 = phi float [ 0.000000e+00, %if.end226 ], [ %10, %for.body ]
  %4 = phi float [ 0.000000e+00, %if.end226 ], [ %12, %for.body ]
  %5 = phi float [ 0.000000e+00, %if.end226 ], [ %14, %for.body ]
  %6 = phi float [ 2.550000e+02, %if.end226 ], [ %9, %for.body ]
  %7 = phi float [ 2.550000e+02, %if.end226 ], [ %11, %for.body ]
  %8 = phi float [ 2.550000e+02, %if.end226 ], [ %13, %for.body ]
  %call245 = call i32 @get_local_id(i32 1) nounwind
  %call247 = call i32 @__mul24_1i32(i32 %call245, i32 %iLocalPixPitch) nounwind
  %call248 = call i32 @get_local_id(i32 0) nounwind
  %add249 = add i32 %call248, %call247
  %arrayidx257 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add249
  %tmp258 = load <4 x i8> addrspace(3)* %arrayidx257, align 4
  %tmp259 = extractelement <4 x i8> %tmp258, i32 0
  %conv = uitofp i8 %tmp259 to float
  %cmp260 = fcmp olt float %0, %conv
  %conv261 = zext i1 %cmp260 to i32
  %tmp273 = extractelement <4 x i8> %tmp258, i32 1
  %conv274 = uitofp i8 %tmp273 to float
  %cmp275 = fcmp olt float %1, %conv274
  %conv276 = zext i1 %cmp275 to i32
  %inc = add i32 %add249, 1
  %tmp288 = extractelement <4 x i8> %tmp258, i32 2
  %conv289 = uitofp i8 %tmp288 to float
  %cmp290 = fcmp olt float %2, %conv289
  %conv291 = zext i1 %cmp290 to i32
  %arrayidx301 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc
  %tmp302 = load <4 x i8> addrspace(3)* %arrayidx301, align 4
  %tmp303 = extractelement <4 x i8> %tmp302, i32 0
  %conv304 = uitofp i8 %tmp303 to float
  %cmp305 = fcmp olt float %0, %conv304
  %conv306 = zext i1 %cmp305 to i32
  %tmp318 = extractelement <4 x i8> %tmp302, i32 1
  %conv319 = uitofp i8 %tmp318 to float
  %cmp320 = fcmp olt float %1, %conv319
  %conv321 = zext i1 %cmp320 to i32
  %inc330 = add i32 %add249, 2
  %tmp334 = extractelement <4 x i8> %tmp302, i32 2
  %conv335 = uitofp i8 %tmp334 to float
  %cmp336 = fcmp olt float %2, %conv335
  %conv337 = zext i1 %cmp336 to i32
  %arrayidx347 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc330
  %tmp348 = load <4 x i8> addrspace(3)* %arrayidx347, align 4
  %tmp349 = extractelement <4 x i8> %tmp348, i32 0
  %conv350 = uitofp i8 %tmp349 to float
  %cmp351 = fcmp olt float %0, %conv350
  %conv352 = zext i1 %cmp351 to i32
  %tmp364 = extractelement <4 x i8> %tmp348, i32 1
  %conv365 = uitofp i8 %tmp364 to float
  %cmp366 = fcmp olt float %1, %conv365
  %conv367 = zext i1 %cmp366 to i32
  %tmp379 = extractelement <4 x i8> %tmp348, i32 2
  %conv380 = uitofp i8 %tmp379 to float
  %cmp381 = fcmp olt float %2, %conv380
  %conv382 = zext i1 %cmp381 to i32
  %add387 = add i32 %add249, %iLocalPixPitch
  %arrayidx396 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add387
  %tmp397 = load <4 x i8> addrspace(3)* %arrayidx396, align 4
  %tmp398 = extractelement <4 x i8> %tmp397, i32 0
  %conv399 = uitofp i8 %tmp398 to float
  %cmp400 = fcmp olt float %0, %conv399
  %conv401 = zext i1 %cmp400 to i32
  %tmp413 = extractelement <4 x i8> %tmp397, i32 1
  %conv414 = uitofp i8 %tmp413 to float
  %cmp415 = fcmp olt float %1, %conv414
  %conv416 = zext i1 %cmp415 to i32
  %inc425 = add i32 %add387, 1
  %tmp429 = extractelement <4 x i8> %tmp397, i32 2
  %conv430 = uitofp i8 %tmp429 to float
  %cmp431 = fcmp olt float %2, %conv430
  %conv432 = zext i1 %cmp431 to i32
  %arrayidx442 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc425
  %tmp443 = load <4 x i8> addrspace(3)* %arrayidx442, align 4
  %tmp444 = extractelement <4 x i8> %tmp443, i32 0
  %conv445 = uitofp i8 %tmp444 to float
  %cmp446 = fcmp olt float %0, %conv445
  %conv447 = zext i1 %cmp446 to i32
  %tmp459 = extractelement <4 x i8> %tmp443, i32 1
  %conv460 = uitofp i8 %tmp459 to float
  %cmp461 = fcmp olt float %1, %conv460
  %conv462 = zext i1 %cmp461 to i32
  %inc471 = add i32 %add387, 2
  %tmp475 = extractelement <4 x i8> %tmp443, i32 2
  %conv476 = uitofp i8 %tmp475 to float
  %cmp477 = fcmp olt float %2, %conv476
  %conv478 = zext i1 %cmp477 to i32
  %arrayidx488 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc471
  %tmp489 = load <4 x i8> addrspace(3)* %arrayidx488, align 4
  %tmp490 = extractelement <4 x i8> %tmp489, i32 0
  %conv491 = uitofp i8 %tmp490 to float
  %cmp492 = fcmp olt float %0, %conv491
  %conv493 = zext i1 %cmp492 to i32
  %tmp505 = extractelement <4 x i8> %tmp489, i32 1
  %conv506 = uitofp i8 %tmp505 to float
  %cmp507 = fcmp olt float %1, %conv506
  %conv508 = zext i1 %cmp507 to i32
  %tmp520 = extractelement <4 x i8> %tmp489, i32 2
  %conv521 = uitofp i8 %tmp520 to float
  %cmp522 = fcmp olt float %2, %conv521
  %conv523 = zext i1 %cmp522 to i32
  %add528 = add i32 %add387, %iLocalPixPitch
  %arrayidx537 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %add528
  %tmp538 = load <4 x i8> addrspace(3)* %arrayidx537, align 4
  %tmp539 = extractelement <4 x i8> %tmp538, i32 0
  %conv540 = uitofp i8 %tmp539 to float
  %cmp541 = fcmp olt float %0, %conv540
  %conv542 = zext i1 %cmp541 to i32
  %tmp554 = extractelement <4 x i8> %tmp538, i32 1
  %conv555 = uitofp i8 %tmp554 to float
  %cmp556 = fcmp olt float %1, %conv555
  %conv557 = zext i1 %cmp556 to i32
  %inc566 = add i32 %add528, 1
  %tmp570 = extractelement <4 x i8> %tmp538, i32 2
  %conv571 = uitofp i8 %tmp570 to float
  %cmp572 = fcmp olt float %2, %conv571
  %conv573 = zext i1 %cmp572 to i32
  %arrayidx583 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc566
  %tmp584 = load <4 x i8> addrspace(3)* %arrayidx583, align 4
  %tmp585 = extractelement <4 x i8> %tmp584, i32 0
  %conv586 = uitofp i8 %tmp585 to float
  %cmp587 = fcmp olt float %0, %conv586
  %conv588 = zext i1 %cmp587 to i32
  %tmp600 = extractelement <4 x i8> %tmp584, i32 1
  %conv601 = uitofp i8 %tmp600 to float
  %cmp602 = fcmp olt float %1, %conv601
  %conv603 = zext i1 %cmp602 to i32
  %inc612 = add i32 %add528, 2
  %tmp616 = extractelement <4 x i8> %tmp584, i32 2
  %conv617 = uitofp i8 %tmp616 to float
  %cmp618 = fcmp olt float %2, %conv617
  %conv619 = zext i1 %cmp618 to i32
  %arrayidx629 = getelementptr <4 x i8> addrspace(3)* %uc4LocalData, i32 %inc612
  %tmp630 = load <4 x i8> addrspace(3)* %arrayidx629, align 4
  %tmp631 = extractelement <4 x i8> %tmp630, i32 0
  %conv632 = uitofp i8 %tmp631 to float
  %cmp633 = fcmp olt float %0, %conv632
  %conv634 = zext i1 %cmp633 to i32
  %add307 = add i32 %conv306, %conv261
  %add353 = add i32 %add307, %conv352
  %add402 = add i32 %add353, %conv401
  %add448 = add i32 %add402, %conv447
  %add494 = add i32 %add448, %conv493
  %add543 = add i32 %add494, %conv542
  %add589 = add i32 %add543, %conv588
  %add635 = add i32 %add589, %conv634
  %tmp646 = extractelement <4 x i8> %tmp630, i32 1
  %conv647 = uitofp i8 %tmp646 to float
  %cmp648 = fcmp olt float %1, %conv647
  %conv649 = zext i1 %cmp648 to i32
  %add322 = add i32 %conv321, %conv276
  %add368 = add i32 %add322, %conv367
  %add417 = add i32 %add368, %conv416
  %add463 = add i32 %add417, %conv462
  %add509 = add i32 %add463, %conv508
  %add558 = add i32 %add509, %conv557
  %add604 = add i32 %add558, %conv603
  %add650 = add i32 %add604, %conv649
  %tmp661 = extractelement <4 x i8> %tmp630, i32 2
  %conv662 = uitofp i8 %tmp661 to float
  %cmp663 = fcmp olt float %2, %conv662
  %conv664 = zext i1 %cmp663 to i32
  %add338 = add i32 %conv337, %conv291
  %add383 = add i32 %add338, %conv382
  %add433 = add i32 %add383, %conv432
  %add479 = add i32 %add433, %conv478
  %add524 = add i32 %add479, %conv523
  %add574 = add i32 %add524, %conv573
  %add620 = add i32 %add574, %conv619
  %add665 = add i32 %add620, %conv664
  %cmp669 = icmp ugt i32 %add635, 4
  %9 = select i1 %cmp669, float %6, float %0
  %10 = select i1 %cmp669, float %0, float %3
  %cmp687 = icmp ugt i32 %add650, 4
  %11 = select i1 %cmp687, float %7, float %1
  %12 = select i1 %cmp687, float %1, float %4
  %cmp705 = icmp ugt i32 %add665, 4
  %13 = select i1 %cmp705, float %8, float %2
  %14 = select i1 %cmp705, float %2, float %5
  %add728 = fadd float %9, %10
  %mul = fmul float %add728, 5.000000e-01
  %add737 = fadd float %11, %12
  %mul738 = fmul float %add737, 5.000000e-01
  %add747 = fadd float %13, %14
  %mul748 = fmul float %add747, 5.000000e-01
  %inc750 = add i32 %storemerge9, 1
  %cmp240 = icmp slt i32 %inc750, 8
  br i1 %cmp240, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %or.cond5 = and i1 %cmp15, %cmp19
  br i1 %or.cond5, label %if.then779, label %if.end786

if.then779:                                       ; preds = %for.end
  %conv760 = fptoui float %mul738 to i32
  %conv766 = fptoui float %mul748 to i32
  %shl = shl i32 %conv760, 8
  %conv755 = fptoui float %mul to i32
  %shl767 = shl i32 %conv766, 16
  %and761 = and i32 %shl, 65280
  %and = and i32 %conv755, 255
  %and768 = and i32 %shl767, 16711680
  %or = or i32 %and761, %and768
  %or769 = or i32 %or, %and
  %call781 = call i32 @get_global_size(i32 0) nounwind
  %add782 = add i32 %call781, %add
  %arrayidx784 = getelementptr i32 addrspace(1)* %uiDest, i32 %add782
  store i32 %or769, i32 addrspace(1)* %arrayidx784, align 4
  br label %if.end786

if.end786:                                        ; preds = %if.then779, %for.end
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
