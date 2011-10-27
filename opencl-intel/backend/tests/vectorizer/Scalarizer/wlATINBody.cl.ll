; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATINBody.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_nbody_sim_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_nbody_sim_parameters = appending global [238 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int, float, float, float4 __attribute__((address_space(3))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[238 x i8]*> [#uses=1]
@opencl_IntegrateSystemNonVectorized_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_IntegrateSystemNonVectorized_parameters = appending global [638 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float, float, float, int, int, float4 __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[638 x i8]*> [#uses=1]
@opencl_IntegrateSystemVectorized_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_IntegrateSystemVectorized_parameters = appending global [651 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float, float, float, int, int, float4 __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[651 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, float, float, <4 x float> addrspace(3)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @nbody_sim to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_nbody_sim_locals to i8*), i8* getelementptr inbounds ([238 x i8]* @opencl_nbody_sim_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, float, float, i32, i32, <4 x float> addrspace(1)*, i32)* @IntegrateSystemNonVectorized to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_IntegrateSystemNonVectorized_locals to i8*), i8* getelementptr inbounds ([638 x i8]* @opencl_IntegrateSystemNonVectorized_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, float, float, i32, i32, <4 x float> addrspace(1)*, i32)* @IntegrateSystemVectorized to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_IntegrateSystemVectorized_locals to i8*), i8* getelementptr inbounds ([651 x i8]* @opencl_IntegrateSystemVectorized_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @nbody_sim(<4 x float> addrspace(1)* %pos, <4 x float> addrspace(1)* %vel, i32 %numBodies, float %deltaTime, float %epsSqr, <4 x float> addrspace(3)* %localPos, <4 x float> addrspace(1)* %updatedPos, <4 x float> addrspace(1)* %updatedVel) nounwind {
entry:
  %pos.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=3]
  %vel.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %numBodies.addr = alloca i32, align 4           ; <i32*> [#uses=2]
  %deltaTime.addr = alloca float, align 4         ; <float*> [#uses=5]
  %epsSqr.addr = alloca float, align 4            ; <float*> [#uses=2]
  %localPos.addr = alloca <4 x float> addrspace(3)*, align 4 ; <<4 x float> addrspace(3)**> [#uses=4]
  %updatedPos.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %updatedVel.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=5]
  %localSize = alloca i32, align 4                ; <i32*> [#uses=4]
  %numTiles = alloca i32, align 4                 ; <i32*> [#uses=2]
  %myPos = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %acc = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=5]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=2]
  %j = alloca i32, align 4                        ; <i32*> [#uses=6]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=8]
  %distSqr = alloca float, align 4                ; <float*> [#uses=2]
  %invDist = alloca float, align 4                ; <float*> [#uses=4]
  %invDistCube = alloca float, align 4            ; <float*> [#uses=2]
  %s = alloca float, align 4                      ; <float*> [#uses=2]
  %oldVel = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %newPos = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  %newVel = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %pos, <4 x float> addrspace(1)** %pos.addr
  store <4 x float> addrspace(1)* %vel, <4 x float> addrspace(1)** %vel.addr
  store i32 %numBodies, i32* %numBodies.addr
  store float %deltaTime, float* %deltaTime.addr
  store float %epsSqr, float* %epsSqr.addr
  store <4 x float> addrspace(3)* %localPos, <4 x float> addrspace(3)** %localPos.addr
  store <4 x float> addrspace(1)* %updatedPos, <4 x float> addrspace(1)** %updatedPos.addr
  store <4 x float> addrspace(1)* %updatedVel, <4 x float> addrspace(1)** %updatedVel.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid
  %call2 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call2, i32* %localSize
  %tmp = load i32* %numBodies.addr                ; <i32> [#uses=1]
  %tmp3 = load i32* %localSize                    ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp3                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp3         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %numTiles
  %tmp5 = load i32* %gid                          ; <i32> [#uses=1]
  %tmp6 = load <4 x float> addrspace(1)** %pos.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp6, i32 %tmp5 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp7 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp7, <4 x float>* %myPos
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp9 = load <4 x float>* %.compoundliteral     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp9, <4 x float>* %acc
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc84, %entry
  %tmp11 = load i32* %i                           ; <i32> [#uses=1]
  %tmp12 = load i32* %numTiles                    ; <i32> [#uses=1]
  %cmp13 = icmp ult i32 %tmp11, %tmp12            ; <i1> [#uses=1]
  br i1 %cmp13, label %for.body, label %for.end87

for.body:                                         ; preds = %for.cond
  %tmp15 = load i32* %i                           ; <i32> [#uses=1]
  %tmp16 = load i32* %localSize                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp15, %tmp16                   ; <i32> [#uses=1]
  %tmp17 = load i32* %tid                         ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp17                     ; <i32> [#uses=1]
  store i32 %add, i32* %idx
  %tmp18 = load i32* %idx                         ; <i32> [#uses=1]
  %tmp19 = load <4 x float> addrspace(1)** %pos.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds <4 x float> addrspace(1)* %tmp19, i32 %tmp18 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp21 = load <4 x float> addrspace(1)* %arrayidx20 ; <<4 x float>> [#uses=1]
  %tmp22 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp23 = load <4 x float> addrspace(3)** %localPos.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <4 x float> addrspace(3)* %tmp23, i32 %tmp22 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %tmp21, <4 x float> addrspace(3)* %arrayidx24
  call void @barrier(i32 1)
  store i32 0, i32* %j
  br label %for.cond26

for.cond26:                                       ; preds = %for.inc, %for.body
  %tmp27 = load i32* %j                           ; <i32> [#uses=1]
  %tmp28 = load i32* %localSize                   ; <i32> [#uses=1]
  %cmp29 = icmp ult i32 %tmp27, %tmp28            ; <i1> [#uses=1]
  br i1 %cmp29, label %for.body30, label %for.end

for.body30:                                       ; preds = %for.cond26
  %tmp32 = load i32* %j                           ; <i32> [#uses=1]
  %tmp33 = load <4 x float> addrspace(3)** %localPos.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds <4 x float> addrspace(3)* %tmp33, i32 %tmp32 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp35 = load <4 x float> addrspace(3)* %arrayidx34 ; <<4 x float>> [#uses=1]
  %tmp36 = load <4 x float>* %myPos               ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp35, %tmp36          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %r
  %tmp38 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp39 = extractelement <4 x float> %tmp38, i32 0 ; <float> [#uses=1]
  %tmp40 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp41 = extractelement <4 x float> %tmp40, i32 0 ; <float> [#uses=1]
  %mul42 = fmul float %tmp39, %tmp41              ; <float> [#uses=1]
  %tmp43 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp44 = extractelement <4 x float> %tmp43, i32 1 ; <float> [#uses=1]
  %tmp45 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp46 = extractelement <4 x float> %tmp45, i32 1 ; <float> [#uses=1]
  %mul47 = fmul float %tmp44, %tmp46              ; <float> [#uses=1]
  %add48 = fadd float %mul42, %mul47              ; <float> [#uses=1]
  %tmp49 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp50 = extractelement <4 x float> %tmp49, i32 2 ; <float> [#uses=1]
  %tmp51 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp52 = extractelement <4 x float> %tmp51, i32 2 ; <float> [#uses=1]
  %mul53 = fmul float %tmp50, %tmp52              ; <float> [#uses=1]
  %add54 = fadd float %add48, %mul53              ; <float> [#uses=1]
  store float %add54, float* %distSqr
  %tmp56 = load float* %distSqr                   ; <float> [#uses=1]
  %tmp57 = load float* %epsSqr.addr               ; <float> [#uses=1]
  %add58 = fadd float %tmp56, %tmp57              ; <float> [#uses=1]
  %call59 = call float @_Z4sqrtf(float %add58)    ; <float> [#uses=3]
  %cmp60 = fcmp oeq float 0.000000e+000, %call59  ; <i1> [#uses=1]
  %sel61 = select i1 %cmp60, float 1.000000e+000, float %call59 ; <float> [#uses=0]
  %div62 = fdiv float 1.000000e+000, %call59      ; <float> [#uses=1]
  store float %div62, float* %invDist
  %tmp64 = load float* %invDist                   ; <float> [#uses=1]
  %tmp65 = load float* %invDist                   ; <float> [#uses=1]
  %mul66 = fmul float %tmp64, %tmp65              ; <float> [#uses=1]
  %tmp67 = load float* %invDist                   ; <float> [#uses=1]
  %mul68 = fmul float %mul66, %tmp67              ; <float> [#uses=1]
  store float %mul68, float* %invDistCube
  %tmp70 = load i32* %j                           ; <i32> [#uses=1]
  %tmp71 = load <4 x float> addrspace(3)** %localPos.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx72 = getelementptr inbounds <4 x float> addrspace(3)* %tmp71, i32 %tmp70 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp73 = load <4 x float> addrspace(3)* %arrayidx72 ; <<4 x float>> [#uses=1]
  %tmp74 = extractelement <4 x float> %tmp73, i32 3 ; <float> [#uses=1]
  %tmp75 = load float* %invDistCube               ; <float> [#uses=1]
  %mul76 = fmul float %tmp74, %tmp75              ; <float> [#uses=1]
  store float %mul76, float* %s
  %tmp77 = load float* %s                         ; <float> [#uses=1]
  %tmp78 = insertelement <4 x float> undef, float %tmp77, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp78, <4 x float> %tmp78, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %mul80 = fmul <4 x float> %splat, %tmp79        ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %acc                 ; <<4 x float>> [#uses=1]
  %add82 = fadd <4 x float> %tmp81, %mul80        ; <<4 x float>> [#uses=1]
  store <4 x float> %add82, <4 x float>* %acc
  br label %for.inc

for.inc:                                          ; preds = %for.body30
  %tmp83 = load i32* %j                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp83, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond26

for.end:                                          ; preds = %for.cond26
  call void @barrier(i32 1)
  br label %for.inc84

for.inc84:                                        ; preds = %for.end
  %tmp85 = load i32* %i                           ; <i32> [#uses=1]
  %inc86 = add nsw i32 %tmp85, 1                  ; <i32> [#uses=1]
  store i32 %inc86, i32* %i
  br label %for.cond

for.end87:                                        ; preds = %for.cond
  %tmp89 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp90 = load <4 x float> addrspace(1)** %vel.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds <4 x float> addrspace(1)* %tmp90, i32 %tmp89 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp92 = load <4 x float> addrspace(1)* %arrayidx91 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp92, <4 x float>* %oldVel
  %tmp94 = load <4 x float>* %myPos               ; <<4 x float>> [#uses=1]
  %tmp95 = load <4 x float>* %oldVel              ; <<4 x float>> [#uses=1]
  %tmp96 = load float* %deltaTime.addr            ; <float> [#uses=1]
  %tmp97 = insertelement <4 x float> undef, float %tmp96, i32 0 ; <<4 x float>> [#uses=2]
  %splat98 = shufflevector <4 x float> %tmp97, <4 x float> %tmp97, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul99 = fmul <4 x float> %tmp95, %splat98      ; <<4 x float>> [#uses=1]
  %add100 = fadd <4 x float> %tmp94, %mul99       ; <<4 x float>> [#uses=1]
  %tmp101 = load <4 x float>* %acc                ; <<4 x float>> [#uses=1]
  %mul102 = fmul <4 x float> %tmp101, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001> ; <<4 x float>> [#uses=1]
  %tmp103 = load float* %deltaTime.addr           ; <float> [#uses=1]
  %tmp104 = insertelement <4 x float> undef, float %tmp103, i32 0 ; <<4 x float>> [#uses=2]
  %splat105 = shufflevector <4 x float> %tmp104, <4 x float> %tmp104, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul106 = fmul <4 x float> %mul102, %splat105   ; <<4 x float>> [#uses=1]
  %tmp107 = load float* %deltaTime.addr           ; <float> [#uses=1]
  %tmp108 = insertelement <4 x float> undef, float %tmp107, i32 0 ; <<4 x float>> [#uses=2]
  %splat109 = shufflevector <4 x float> %tmp108, <4 x float> %tmp108, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul110 = fmul <4 x float> %mul106, %splat109   ; <<4 x float>> [#uses=1]
  %add111 = fadd <4 x float> %add100, %mul110     ; <<4 x float>> [#uses=1]
  store <4 x float> %add111, <4 x float>* %newPos
  %tmp112 = load <4 x float>* %myPos              ; <<4 x float>> [#uses=1]
  %tmp113 = extractelement <4 x float> %tmp112, i32 3 ; <float> [#uses=1]
  %tmp114 = load <4 x float>* %newPos             ; <<4 x float>> [#uses=1]
  %tmp115 = insertelement <4 x float> %tmp114, float %tmp113, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp115, <4 x float>* %newPos
  %tmp117 = load <4 x float>* %oldVel             ; <<4 x float>> [#uses=1]
  %tmp118 = load <4 x float>* %acc                ; <<4 x float>> [#uses=1]
  %tmp119 = load float* %deltaTime.addr           ; <float> [#uses=1]
  %tmp120 = insertelement <4 x float> undef, float %tmp119, i32 0 ; <<4 x float>> [#uses=2]
  %splat121 = shufflevector <4 x float> %tmp120, <4 x float> %tmp120, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul122 = fmul <4 x float> %tmp118, %splat121   ; <<4 x float>> [#uses=1]
  %add123 = fadd <4 x float> %tmp117, %mul122     ; <<4 x float>> [#uses=1]
  store <4 x float> %add123, <4 x float>* %newVel
  %tmp124 = load <4 x float>* %newPos             ; <<4 x float>> [#uses=1]
  %tmp125 = load i32* %gid                        ; <i32> [#uses=1]
  %tmp126 = load <4 x float> addrspace(1)** %updatedPos.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx127 = getelementptr inbounds <4 x float> addrspace(1)* %tmp126, i32 %tmp125 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp124, <4 x float> addrspace(1)* %arrayidx127
  %tmp128 = load <4 x float>* %newVel             ; <<4 x float>> [#uses=1]
  %tmp129 = load i32* %gid                        ; <i32> [#uses=1]
  %tmp130 = load <4 x float> addrspace(1)** %updatedVel.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds <4 x float> addrspace(1)* %tmp130, i32 %tmp129 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp128, <4 x float> addrspace(1)* %arrayidx131
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)

declare float @_Z4sqrtf(float)

; CHECK: ret
define void @IntegrateSystemNonVectorized(float addrspace(1)* %output_position_x, float addrspace(1)* %output_position_y, float addrspace(1)* %output_position_z, float addrspace(1)* %mass, float addrspace(1)* %output_velocity_x, float addrspace(1)* %output_velocity_y, float addrspace(1)* %output_velocity_z, float addrspace(1)* %input_position_x, float addrspace(1)* %input_position_y, float addrspace(1)* %input_position_z, float addrspace(1)* %input_velocity_x, float addrspace(1)* %input_velocity_y, float addrspace(1)* %input_velocity_z, float %time_delta, float %damping, float %softening_squared, i32 %body_count, i32 %body_count_per_group, <4 x float> addrspace(1)* %output_position, i32 %start_index) nounwind {
entry:
  %output_position_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %mass.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %output_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input_position_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_position_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_position_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %time_delta.addr = alloca float, align 4        ; <float*> [#uses=7]
  %damping.addr = alloca float, align 4           ; <float*> [#uses=4]
  %softening_squared.addr = alloca float, align 4 ; <float*> [#uses=2]
  %body_count.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %body_count_per_group.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %output_position.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %start_index.addr = alloca i32, align 4         ; <i32*> [#uses=3]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %position_x = alloca float, align 4             ; <float*> [#uses=6]
  %position_y = alloca float, align 4             ; <float*> [#uses=6]
  %position_z = alloca float, align 4             ; <float*> [#uses=6]
  %m = alloca float, align 4                      ; <float*> [#uses=3]
  %current_x1 = alloca float, align 4             ; <float*> [#uses=2]
  %current_y1 = alloca float, align 4             ; <float*> [#uses=2]
  %current_z1 = alloca float, align 4             ; <float*> [#uses=2]
  %current_mass1 = alloca float, align 4          ; <float*> [#uses=1]
  %current_x2 = alloca float, align 4             ; <float*> [#uses=0]
  %current_y2 = alloca float, align 4             ; <float*> [#uses=0]
  %current_z2 = alloca float, align 4             ; <float*> [#uses=0]
  %current_mass2 = alloca float, align 4          ; <float*> [#uses=0]
  %velocity_x = alloca float, align 4             ; <float*> [#uses=7]
  %velocity_y = alloca float, align 4             ; <float*> [#uses=7]
  %velocity_z = alloca float, align 4             ; <float*> [#uses=7]
  %zero = alloca float, align 4                   ; <float*> [#uses=7]
  %i = alloca i32, align 4                        ; <i32*> [#uses=8]
  %j = alloca i32, align 4                        ; <i32*> [#uses=0]
  %k = alloca i32, align 4                        ; <i32*> [#uses=14]
  %l = alloca i32, align 4                        ; <i32*> [#uses=6]
  %inner_loop_count = alloca i32, align 4         ; <i32*> [#uses=2]
  %outer_loop_count = alloca i32, align 4         ; <i32*> [#uses=3]
  %start = alloca i32, align 4                    ; <i32*> [#uses=2]
  %offset = alloca i32, align 4                   ; <i32*> [#uses=2]
  %final_ax = alloca float, align 4               ; <float*> [#uses=3]
  %final_ay = alloca float, align 4               ; <float*> [#uses=3]
  %final_az = alloca float, align 4               ; <float*> [#uses=3]
  %acceleration_x1 = alloca float, align 4        ; <float*> [#uses=4]
  %acceleration_y1 = alloca float, align 4        ; <float*> [#uses=4]
  %acceleration_z1 = alloca float, align 4        ; <float*> [#uses=4]
  %dx = alloca float, align 4                     ; <float*> [#uses=4]
  %dy = alloca float, align 4                     ; <float*> [#uses=4]
  %dz = alloca float, align 4                     ; <float*> [#uses=4]
  %mi = alloca float, align 4                     ; <float*> [#uses=2]
  %distance_squared = alloca float, align 4       ; <float*> [#uses=4]
  %inverse_distance = alloca float, align 4       ; <float*> [#uses=4]
  %s = alloca float, align 4                      ; <float*> [#uses=4]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store float addrspace(1)* %output_position_x, float addrspace(1)** %output_position_x.addr
  store float addrspace(1)* %output_position_y, float addrspace(1)** %output_position_y.addr
  store float addrspace(1)* %output_position_z, float addrspace(1)** %output_position_z.addr
  store float addrspace(1)* %mass, float addrspace(1)** %mass.addr
  store float addrspace(1)* %output_velocity_x, float addrspace(1)** %output_velocity_x.addr
  store float addrspace(1)* %output_velocity_y, float addrspace(1)** %output_velocity_y.addr
  store float addrspace(1)* %output_velocity_z, float addrspace(1)** %output_velocity_z.addr
  store float addrspace(1)* %input_position_x, float addrspace(1)** %input_position_x.addr
  store float addrspace(1)* %input_position_y, float addrspace(1)** %input_position_y.addr
  store float addrspace(1)* %input_position_z, float addrspace(1)** %input_position_z.addr
  store float addrspace(1)* %input_velocity_x, float addrspace(1)** %input_velocity_x.addr
  store float addrspace(1)* %input_velocity_y, float addrspace(1)** %input_velocity_y.addr
  store float addrspace(1)* %input_velocity_z, float addrspace(1)** %input_velocity_z.addr
  store float %time_delta, float* %time_delta.addr
  store float %damping, float* %damping.addr
  store float %softening_squared, float* %softening_squared.addr
  store i32 %body_count, i32* %body_count.addr
  store i32 %body_count_per_group, i32* %body_count_per_group.addr
  store <4 x float> addrspace(1)* %output_position, <4 x float> addrspace(1)** %output_position.addr
  store i32 %start_index, i32* %start_index.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %index
  store float 0.000000e+000, float* %zero
  %tmp = load i32* %body_count.addr               ; <i32> [#uses=1]
  store i32 %tmp, i32* %inner_loop_count
  %tmp2 = load i32* %body_count_per_group.addr    ; <i32> [#uses=1]
  store i32 %tmp2, i32* %outer_loop_count
  %tmp4 = load i32* %index                        ; <i32> [#uses=1]
  %tmp5 = load i32* %outer_loop_count             ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  %tmp6 = load i32* %start_index.addr             ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp6                  ; <i32> [#uses=1]
  store i32 %add, i32* %start
  %tmp8 = load i32* %index                        ; <i32> [#uses=1]
  %tmp9 = load i32* %body_count_per_group.addr    ; <i32> [#uses=1]
  %mul10 = mul i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  %tmp11 = load i32* %start_index.addr            ; <i32> [#uses=1]
  %add12 = add nsw i32 %mul10, %tmp11             ; <i32> [#uses=1]
  store i32 %add12, i32* %offset
  store i32 0, i32* %l
  br label %for.cond

for.cond:                                         ; preds = %for.inc212, %entry
  %tmp13 = load i32* %l                           ; <i32> [#uses=1]
  %tmp14 = load i32* %outer_loop_count            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp13, %tmp14              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end215

for.body:                                         ; preds = %for.cond
  %tmp15 = load i32* %l                           ; <i32> [#uses=1]
  %tmp16 = load i32* %start                       ; <i32> [#uses=1]
  %add17 = add nsw i32 %tmp15, %tmp16             ; <i32> [#uses=1]
  store i32 %add17, i32* %k
  %tmp18 = load i32* %k                           ; <i32> [#uses=1]
  %tmp19 = load float addrspace(1)** %input_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp19, i32 %tmp18 ; <float addrspace(1)*> [#uses=1]
  %tmp20 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  store float %tmp20, float* %position_x
  %tmp21 = load i32* %k                           ; <i32> [#uses=1]
  %tmp22 = load float addrspace(1)** %input_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %tmp22, i32 %tmp21 ; <float addrspace(1)*> [#uses=1]
  %tmp24 = load float addrspace(1)* %arrayidx23   ; <float> [#uses=1]
  store float %tmp24, float* %position_y
  %tmp25 = load i32* %k                           ; <i32> [#uses=1]
  %tmp26 = load float addrspace(1)** %input_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx27 = getelementptr inbounds float addrspace(1)* %tmp26, i32 %tmp25 ; <float addrspace(1)*> [#uses=1]
  %tmp28 = load float addrspace(1)* %arrayidx27   ; <float> [#uses=1]
  store float %tmp28, float* %position_z
  %tmp29 = load i32* %k                           ; <i32> [#uses=1]
  %tmp30 = load float addrspace(1)** %mass.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx31 = getelementptr inbounds float addrspace(1)* %tmp30, i32 %tmp29 ; <float addrspace(1)*> [#uses=1]
  %tmp32 = load float addrspace(1)* %arrayidx31   ; <float> [#uses=1]
  store float %tmp32, float* %m
  %tmp34 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp34, float* %final_ax
  %tmp36 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp36, float* %final_ay
  %tmp38 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp38, float* %final_az
  %tmp39 = load float* %position_x                ; <float> [#uses=1]
  store float %tmp39, float* %current_x1
  %tmp40 = load float* %position_y                ; <float> [#uses=1]
  store float %tmp40, float* %current_y1
  %tmp41 = load float* %position_z                ; <float> [#uses=1]
  store float %tmp41, float* %current_z1
  %tmp42 = load float* %m                         ; <float> [#uses=1]
  store float %tmp42, float* %current_mass1
  %tmp44 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp44, float* %acceleration_x1
  %tmp46 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp46, float* %acceleration_y1
  %tmp48 = load float* %zero                      ; <float> [#uses=1]
  store float %tmp48, float* %acceleration_z1
  store i32 0, i32* %i
  br label %for.cond49

for.cond49:                                       ; preds = %for.inc, %for.body
  %tmp50 = load i32* %i                           ; <i32> [#uses=1]
  %tmp51 = load i32* %inner_loop_count            ; <i32> [#uses=1]
  %cmp52 = icmp slt i32 %tmp50, %tmp51            ; <i1> [#uses=1]
  br i1 %cmp52, label %for.body53, label %for.end

for.body53:                                       ; preds = %for.cond49
  %tmp55 = load i32* %i                           ; <i32> [#uses=1]
  %tmp56 = load float addrspace(1)** %input_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx57 = getelementptr inbounds float addrspace(1)* %tmp56, i32 %tmp55 ; <float addrspace(1)*> [#uses=1]
  %tmp58 = load float addrspace(1)* %arrayidx57   ; <float> [#uses=1]
  %tmp59 = load float* %current_x1                ; <float> [#uses=1]
  %sub = fsub float %tmp58, %tmp59                ; <float> [#uses=1]
  store float %sub, float* %dx
  %tmp61 = load i32* %i                           ; <i32> [#uses=1]
  %tmp62 = load float addrspace(1)** %input_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx63 = getelementptr inbounds float addrspace(1)* %tmp62, i32 %tmp61 ; <float addrspace(1)*> [#uses=1]
  %tmp64 = load float addrspace(1)* %arrayidx63   ; <float> [#uses=1]
  %tmp65 = load float* %current_y1                ; <float> [#uses=1]
  %sub66 = fsub float %tmp64, %tmp65              ; <float> [#uses=1]
  store float %sub66, float* %dy
  %tmp68 = load i32* %i                           ; <i32> [#uses=1]
  %tmp69 = load float addrspace(1)** %input_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds float addrspace(1)* %tmp69, i32 %tmp68 ; <float addrspace(1)*> [#uses=1]
  %tmp71 = load float addrspace(1)* %arrayidx70   ; <float> [#uses=1]
  %tmp72 = load float* %current_z1                ; <float> [#uses=1]
  %sub73 = fsub float %tmp71, %tmp72              ; <float> [#uses=1]
  store float %sub73, float* %dz
  %tmp75 = load i32* %i                           ; <i32> [#uses=1]
  %tmp76 = load float addrspace(1)** %mass.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(1)* %tmp76, i32 %tmp75 ; <float addrspace(1)*> [#uses=1]
  %tmp78 = load float addrspace(1)* %arrayidx77   ; <float> [#uses=1]
  store float %tmp78, float* %mi
  %tmp80 = load float* %dx                        ; <float> [#uses=1]
  %tmp81 = load float* %dx                        ; <float> [#uses=1]
  %mul82 = fmul float %tmp80, %tmp81              ; <float> [#uses=1]
  %tmp83 = load float* %dy                        ; <float> [#uses=1]
  %tmp84 = load float* %dy                        ; <float> [#uses=1]
  %mul85 = fmul float %tmp83, %tmp84              ; <float> [#uses=1]
  %add86 = fadd float %mul82, %mul85              ; <float> [#uses=1]
  %tmp87 = load float* %dz                        ; <float> [#uses=1]
  %tmp88 = load float* %dz                        ; <float> [#uses=1]
  %mul89 = fmul float %tmp87, %tmp88              ; <float> [#uses=1]
  %add90 = fadd float %add86, %mul89              ; <float> [#uses=1]
  store float %add90, float* %distance_squared
  %tmp91 = load float* %softening_squared.addr    ; <float> [#uses=1]
  %tmp92 = load float* %distance_squared          ; <float> [#uses=1]
  %add93 = fadd float %tmp92, %tmp91              ; <float> [#uses=1]
  store float %add93, float* %distance_squared
  %tmp95 = load float* %distance_squared          ; <float> [#uses=1]
  %call96 = call float @_Z12native_rsqrtf(float %tmp95) ; <float> [#uses=1]
  store float %call96, float* %inverse_distance
  %tmp98 = load float* %mi                        ; <float> [#uses=1]
  %tmp99 = load float* %inverse_distance          ; <float> [#uses=1]
  %mul100 = fmul float %tmp98, %tmp99             ; <float> [#uses=1]
  %tmp101 = load float* %inverse_distance         ; <float> [#uses=1]
  %tmp102 = load float* %inverse_distance         ; <float> [#uses=1]
  %mul103 = fmul float %tmp101, %tmp102           ; <float> [#uses=1]
  %mul104 = fmul float %mul100, %mul103           ; <float> [#uses=1]
  store float %mul104, float* %s
  %tmp105 = load float* %dx                       ; <float> [#uses=1]
  %tmp106 = load float* %s                        ; <float> [#uses=1]
  %mul107 = fmul float %tmp105, %tmp106           ; <float> [#uses=1]
  %tmp108 = load float* %acceleration_x1          ; <float> [#uses=1]
  %add109 = fadd float %tmp108, %mul107           ; <float> [#uses=1]
  store float %add109, float* %acceleration_x1
  %tmp110 = load float* %dy                       ; <float> [#uses=1]
  %tmp111 = load float* %s                        ; <float> [#uses=1]
  %mul112 = fmul float %tmp110, %tmp111           ; <float> [#uses=1]
  %tmp113 = load float* %acceleration_y1          ; <float> [#uses=1]
  %add114 = fadd float %tmp113, %mul112           ; <float> [#uses=1]
  store float %add114, float* %acceleration_y1
  %tmp115 = load float* %dz                       ; <float> [#uses=1]
  %tmp116 = load float* %s                        ; <float> [#uses=1]
  %mul117 = fmul float %tmp115, %tmp116           ; <float> [#uses=1]
  %tmp118 = load float* %acceleration_z1          ; <float> [#uses=1]
  %add119 = fadd float %tmp118, %mul117           ; <float> [#uses=1]
  store float %add119, float* %acceleration_z1
  br label %for.inc

for.inc:                                          ; preds = %for.body53
  %tmp120 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp120, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond49

for.end:                                          ; preds = %for.cond49
  %tmp121 = load float* %acceleration_x1          ; <float> [#uses=1]
  store float %tmp121, float* %final_ax
  %tmp122 = load float* %acceleration_y1          ; <float> [#uses=1]
  store float %tmp122, float* %final_ay
  %tmp123 = load float* %acceleration_z1          ; <float> [#uses=1]
  store float %tmp123, float* %final_az
  %tmp124 = load i32* %k                          ; <i32> [#uses=1]
  %tmp125 = load float addrspace(1)** %input_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds float addrspace(1)* %tmp125, i32 %tmp124 ; <float addrspace(1)*> [#uses=1]
  %tmp127 = load float addrspace(1)* %arrayidx126 ; <float> [#uses=1]
  store float %tmp127, float* %velocity_x
  %tmp128 = load i32* %k                          ; <i32> [#uses=1]
  %tmp129 = load float addrspace(1)** %input_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds float addrspace(1)* %tmp129, i32 %tmp128 ; <float addrspace(1)*> [#uses=1]
  %tmp131 = load float addrspace(1)* %arrayidx130 ; <float> [#uses=1]
  store float %tmp131, float* %velocity_y
  %tmp132 = load i32* %k                          ; <i32> [#uses=1]
  %tmp133 = load float addrspace(1)** %input_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds float addrspace(1)* %tmp133, i32 %tmp132 ; <float addrspace(1)*> [#uses=1]
  %tmp135 = load float addrspace(1)* %arrayidx134 ; <float> [#uses=1]
  store float %tmp135, float* %velocity_z
  %tmp136 = load float* %final_ax                 ; <float> [#uses=1]
  %tmp137 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul138 = fmul float %tmp136, %tmp137           ; <float> [#uses=1]
  %tmp139 = load float* %velocity_x               ; <float> [#uses=1]
  %add140 = fadd float %tmp139, %mul138           ; <float> [#uses=1]
  store float %add140, float* %velocity_x
  %tmp141 = load float* %final_ay                 ; <float> [#uses=1]
  %tmp142 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul143 = fmul float %tmp141, %tmp142           ; <float> [#uses=1]
  %tmp144 = load float* %velocity_y               ; <float> [#uses=1]
  %add145 = fadd float %tmp144, %mul143           ; <float> [#uses=1]
  store float %add145, float* %velocity_y
  %tmp146 = load float* %final_az                 ; <float> [#uses=1]
  %tmp147 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul148 = fmul float %tmp146, %tmp147           ; <float> [#uses=1]
  %tmp149 = load float* %velocity_z               ; <float> [#uses=1]
  %add150 = fadd float %tmp149, %mul148           ; <float> [#uses=1]
  store float %add150, float* %velocity_z
  %tmp151 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp152 = load float* %velocity_x               ; <float> [#uses=1]
  %mul153 = fmul float %tmp152, %tmp151           ; <float> [#uses=1]
  store float %mul153, float* %velocity_x
  %tmp154 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp155 = load float* %velocity_y               ; <float> [#uses=1]
  %mul156 = fmul float %tmp155, %tmp154           ; <float> [#uses=1]
  store float %mul156, float* %velocity_y
  %tmp157 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp158 = load float* %velocity_z               ; <float> [#uses=1]
  %mul159 = fmul float %tmp158, %tmp157           ; <float> [#uses=1]
  store float %mul159, float* %velocity_z
  %tmp160 = load float* %velocity_x               ; <float> [#uses=1]
  %tmp161 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul162 = fmul float %tmp160, %tmp161           ; <float> [#uses=1]
  %tmp163 = load float* %position_x               ; <float> [#uses=1]
  %add164 = fadd float %tmp163, %mul162           ; <float> [#uses=1]
  store float %add164, float* %position_x
  %tmp165 = load float* %velocity_y               ; <float> [#uses=1]
  %tmp166 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul167 = fmul float %tmp165, %tmp166           ; <float> [#uses=1]
  %tmp168 = load float* %position_y               ; <float> [#uses=1]
  %add169 = fadd float %tmp168, %mul167           ; <float> [#uses=1]
  store float %add169, float* %position_y
  %tmp170 = load float* %velocity_z               ; <float> [#uses=1]
  %tmp171 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul172 = fmul float %tmp170, %tmp171           ; <float> [#uses=1]
  %tmp173 = load float* %position_z               ; <float> [#uses=1]
  %add174 = fadd float %tmp173, %mul172           ; <float> [#uses=1]
  store float %add174, float* %position_z
  %tmp175 = load float* %position_x               ; <float> [#uses=1]
  %tmp176 = load i32* %k                          ; <i32> [#uses=1]
  %tmp177 = load float addrspace(1)** %output_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx178 = getelementptr inbounds float addrspace(1)* %tmp177, i32 %tmp176 ; <float addrspace(1)*> [#uses=1]
  store float %tmp175, float addrspace(1)* %arrayidx178
  %tmp179 = load float* %position_y               ; <float> [#uses=1]
  %tmp180 = load i32* %k                          ; <i32> [#uses=1]
  %tmp181 = load float addrspace(1)** %output_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx182 = getelementptr inbounds float addrspace(1)* %tmp181, i32 %tmp180 ; <float addrspace(1)*> [#uses=1]
  store float %tmp179, float addrspace(1)* %arrayidx182
  %tmp183 = load float* %position_z               ; <float> [#uses=1]
  %tmp184 = load i32* %k                          ; <i32> [#uses=1]
  %tmp185 = load float addrspace(1)** %output_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx186 = getelementptr inbounds float addrspace(1)* %tmp185, i32 %tmp184 ; <float addrspace(1)*> [#uses=1]
  store float %tmp183, float addrspace(1)* %arrayidx186
  %tmp187 = load float* %velocity_x               ; <float> [#uses=1]
  %tmp188 = load i32* %k                          ; <i32> [#uses=1]
  %tmp189 = load float addrspace(1)** %output_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx190 = getelementptr inbounds float addrspace(1)* %tmp189, i32 %tmp188 ; <float addrspace(1)*> [#uses=1]
  store float %tmp187, float addrspace(1)* %arrayidx190
  %tmp191 = load float* %velocity_y               ; <float> [#uses=1]
  %tmp192 = load i32* %k                          ; <i32> [#uses=1]
  %tmp193 = load float addrspace(1)** %output_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx194 = getelementptr inbounds float addrspace(1)* %tmp193, i32 %tmp192 ; <float addrspace(1)*> [#uses=1]
  store float %tmp191, float addrspace(1)* %arrayidx194
  %tmp195 = load float* %velocity_z               ; <float> [#uses=1]
  %tmp196 = load i32* %k                          ; <i32> [#uses=1]
  %tmp197 = load float addrspace(1)** %output_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx198 = getelementptr inbounds float addrspace(1)* %tmp197, i32 %tmp196 ; <float addrspace(1)*> [#uses=1]
  store float %tmp195, float addrspace(1)* %arrayidx198
  %tmp199 = load float* %position_x               ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %tmp199, i32 0 ; <<4 x float>> [#uses=1]
  %tmp200 = load float* %position_y               ; <float> [#uses=1]
  %vecinit201 = insertelement <4 x float> %vecinit, float %tmp200, i32 1 ; <<4 x float>> [#uses=1]
  %tmp202 = load float* %position_z               ; <float> [#uses=1]
  %vecinit203 = insertelement <4 x float> %vecinit201, float %tmp202, i32 2 ; <<4 x float>> [#uses=1]
  %tmp204 = load float* %m                        ; <float> [#uses=1]
  %vecinit205 = insertelement <4 x float> %vecinit203, float %tmp204, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit205, <4 x float>* %.compoundliteral
  %tmp206 = load <4 x float>* %.compoundliteral   ; <<4 x float>> [#uses=1]
  %tmp207 = load i32* %l                          ; <i32> [#uses=1]
  %tmp208 = load i32* %offset                     ; <i32> [#uses=1]
  %add209 = add nsw i32 %tmp207, %tmp208          ; <i32> [#uses=1]
  %tmp210 = load <4 x float> addrspace(1)** %output_position.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx211 = getelementptr inbounds <4 x float> addrspace(1)* %tmp210, i32 %add209 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp206, <4 x float> addrspace(1)* %arrayidx211
  br label %for.inc212

for.inc212:                                       ; preds = %for.end
  %tmp213 = load i32* %l                          ; <i32> [#uses=1]
  %inc214 = add nsw i32 %tmp213, 1                ; <i32> [#uses=1]
  store i32 %inc214, i32* %l
  br label %for.cond

for.end215:                                       ; preds = %for.cond
  ret void
}

declare float @_Z12native_rsqrtf(float)

; CHECK: ret
define void @IntegrateSystemVectorized(<4 x float> addrspace(1)* %output_position_x, <4 x float> addrspace(1)* %output_position_y, <4 x float> addrspace(1)* %output_position_z, <4 x float> addrspace(1)* %mass, <4 x float> addrspace(1)* %output_velocity_x, <4 x float> addrspace(1)* %output_velocity_y, <4 x float> addrspace(1)* %output_velocity_z, <4 x float> addrspace(1)* %input_position_x, <4 x float> addrspace(1)* %input_position_y, <4 x float> addrspace(1)* %input_position_z, <4 x float> addrspace(1)* %input_velocity_x, <4 x float> addrspace(1)* %input_velocity_y, <4 x float> addrspace(1)* %input_velocity_z, float %time_delta, float %damping, float %softening_squared, i32 %body_count, i32 %body_count_per_group, <4 x float> addrspace(1)* %output_position, i32 %start_index) nounwind {
entry:
  %output_position_x.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output_position_y.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output_position_z.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %mass.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=4]
  %output_velocity_x.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output_velocity_y.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output_velocity_z.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %input_position_x.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %input_position_y.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %input_position_z.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %input_velocity_x.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %input_velocity_y.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %input_velocity_z.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %time_delta.addr = alloca float, align 4        ; <float*> [#uses=7]
  %damping.addr = alloca float, align 4           ; <float*> [#uses=4]
  %softening_squared.addr = alloca float, align 4 ; <float*> [#uses=5]
  %body_count.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %body_count_per_group.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %output_position.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=5]
  %start_index.addr = alloca i32, align 4         ; <i32*> [#uses=3]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %position_x = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=12]
  %position_y = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=12]
  %position_z = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=12]
  %m = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=9]
  %current_x1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_y1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_z1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_mass1 = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %current_x2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_y2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_z2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %current_mass2 = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %velocity_x = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=7]
  %velocity_y = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=7]
  %velocity_z = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=7]
  %zero = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=16]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=22]
  %j = alloca i32, align 4                        ; <i32*> [#uses=0]
  %k = alloca i32, align 4                        ; <i32*> [#uses=14]
  %l = alloca i32, align 4                        ; <i32*> [#uses=9]
  %inner_loop_count = alloca i32, align 4         ; <i32*> [#uses=3]
  %outer_loop_count = alloca i32, align 4         ; <i32*> [#uses=3]
  %start = alloca i32, align 4                    ; <i32*> [#uses=2]
  %offset = alloca i32, align 4                   ; <i32*> [#uses=5]
  %final_ax = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=10]
  %final_ay = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=10]
  %final_az = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=10]
  %acceleration_x1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %acceleration_y1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %acceleration_z1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %acceleration_x2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %acceleration_y2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %acceleration_z2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=14]
  %dx = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=8]
  %dy = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=8]
  %dz = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=8]
  %mi = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %distance_squared = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=8]
  %inverse_distance = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=8]
  %s = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=8]
  %dx312 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %dy320 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %dz328 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %mi336 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %distance_squared342 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=8]
  %inverse_distance360 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=8]
  %s364 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=8]
  %.compoundliteral620 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral638 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral657 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral676 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %output_position_x, <4 x float> addrspace(1)** %output_position_x.addr
  store <4 x float> addrspace(1)* %output_position_y, <4 x float> addrspace(1)** %output_position_y.addr
  store <4 x float> addrspace(1)* %output_position_z, <4 x float> addrspace(1)** %output_position_z.addr
  store <4 x float> addrspace(1)* %mass, <4 x float> addrspace(1)** %mass.addr
  store <4 x float> addrspace(1)* %output_velocity_x, <4 x float> addrspace(1)** %output_velocity_x.addr
  store <4 x float> addrspace(1)* %output_velocity_y, <4 x float> addrspace(1)** %output_velocity_y.addr
  store <4 x float> addrspace(1)* %output_velocity_z, <4 x float> addrspace(1)** %output_velocity_z.addr
  store <4 x float> addrspace(1)* %input_position_x, <4 x float> addrspace(1)** %input_position_x.addr
  store <4 x float> addrspace(1)* %input_position_y, <4 x float> addrspace(1)** %input_position_y.addr
  store <4 x float> addrspace(1)* %input_position_z, <4 x float> addrspace(1)** %input_position_z.addr
  store <4 x float> addrspace(1)* %input_velocity_x, <4 x float> addrspace(1)** %input_velocity_x.addr
  store <4 x float> addrspace(1)* %input_velocity_y, <4 x float> addrspace(1)** %input_velocity_y.addr
  store <4 x float> addrspace(1)* %input_velocity_z, <4 x float> addrspace(1)** %input_velocity_z.addr
  store float %time_delta, float* %time_delta.addr
  store float %damping, float* %damping.addr
  store float %softening_squared, float* %softening_squared.addr
  store i32 %body_count, i32* %body_count.addr
  store i32 %body_count_per_group, i32* %body_count_per_group.addr
  store <4 x float> addrspace(1)* %output_position, <4 x float> addrspace(1)** %output_position.addr
  store i32 %start_index, i32* %start_index.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %index
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %zero
  %tmp6 = load i32* %body_count.addr              ; <i32> [#uses=1]
  %shr = ashr i32 %tmp6, 2                        ; <i32> [#uses=1]
  store i32 %shr, i32* %inner_loop_count
  %tmp8 = load i32* %body_count_per_group.addr    ; <i32> [#uses=1]
  %shr9 = ashr i32 %tmp8, 2                       ; <i32> [#uses=1]
  store i32 %shr9, i32* %outer_loop_count
  %tmp11 = load i32* %index                       ; <i32> [#uses=1]
  %tmp12 = load i32* %outer_loop_count            ; <i32> [#uses=1]
  %mul = mul i32 %tmp11, %tmp12                   ; <i32> [#uses=1]
  %tmp13 = load i32* %start_index.addr            ; <i32> [#uses=1]
  %shr14 = ashr i32 %tmp13, 2                     ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %shr14                 ; <i32> [#uses=1]
  store i32 %add, i32* %start
  %tmp16 = load i32* %index                       ; <i32> [#uses=1]
  %tmp17 = load i32* %body_count_per_group.addr   ; <i32> [#uses=1]
  %mul18 = mul i32 %tmp16, %tmp17                 ; <i32> [#uses=1]
  %tmp19 = load i32* %start_index.addr            ; <i32> [#uses=1]
  %add20 = add nsw i32 %mul18, %tmp19             ; <i32> [#uses=1]
  store i32 %add20, i32* %offset
  store i32 0, i32* %l
  br label %for.cond

for.cond:                                         ; preds = %for.inc695, %entry
  %tmp21 = load i32* %l                           ; <i32> [#uses=1]
  %tmp22 = load i32* %outer_loop_count            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp21, %tmp22              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end698

for.body:                                         ; preds = %for.cond
  %tmp23 = load i32* %l                           ; <i32> [#uses=1]
  %tmp24 = load i32* %start                       ; <i32> [#uses=1]
  %add25 = add nsw i32 %tmp23, %tmp24             ; <i32> [#uses=1]
  store i32 %add25, i32* %k
  %tmp26 = load i32* %k                           ; <i32> [#uses=1]
  %tmp27 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp27, i32 %tmp26 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp28 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp28, <4 x float>* %position_x
  %tmp29 = load i32* %k                           ; <i32> [#uses=1]
  %tmp30 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx31 = getelementptr inbounds <4 x float> addrspace(1)* %tmp30, i32 %tmp29 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp32 = load <4 x float> addrspace(1)* %arrayidx31 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %position_y
  %tmp33 = load i32* %k                           ; <i32> [#uses=1]
  %tmp34 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx35 = getelementptr inbounds <4 x float> addrspace(1)* %tmp34, i32 %tmp33 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp36 = load <4 x float> addrspace(1)* %arrayidx35 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36, <4 x float>* %position_z
  %tmp37 = load i32* %k                           ; <i32> [#uses=1]
  %tmp38 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds <4 x float> addrspace(1)* %tmp38, i32 %tmp37 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp40 = load <4 x float> addrspace(1)* %arrayidx39 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp40, <4 x float>* %m
  %tmp42 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %final_ax
  %tmp44 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp44, <4 x float>* %final_ay
  %tmp46 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46, <4 x float>* %final_az
  %tmp47 = load <4 x float>* %position_x          ; <<4 x float>> [#uses=1]
  %tmp48 = shufflevector <4 x float> %tmp47, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp48, <4 x float>* %current_x1
  %tmp49 = load <4 x float>* %position_y          ; <<4 x float>> [#uses=1]
  %tmp50 = shufflevector <4 x float> %tmp49, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp50, <4 x float>* %current_y1
  %tmp51 = load <4 x float>* %position_z          ; <<4 x float>> [#uses=1]
  %tmp52 = shufflevector <4 x float> %tmp51, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp52, <4 x float>* %current_z1
  %tmp53 = load <4 x float>* %m                   ; <<4 x float>> [#uses=1]
  %tmp54 = shufflevector <4 x float> %tmp53, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %current_mass1
  %tmp55 = load <4 x float>* %position_x          ; <<4 x float>> [#uses=1]
  %tmp56 = shufflevector <4 x float> %tmp55, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %current_x2
  %tmp57 = load <4 x float>* %position_y          ; <<4 x float>> [#uses=1]
  %tmp58 = shufflevector <4 x float> %tmp57, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp58, <4 x float>* %current_y2
  %tmp59 = load <4 x float>* %position_z          ; <<4 x float>> [#uses=1]
  %tmp60 = shufflevector <4 x float> %tmp59, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp60, <4 x float>* %current_z2
  %tmp61 = load <4 x float>* %m                   ; <<4 x float>> [#uses=1]
  %tmp62 = shufflevector <4 x float> %tmp61, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp62, <4 x float>* %current_mass2
  %tmp64 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64, <4 x float>* %acceleration_x1
  %tmp66 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp66, <4 x float>* %acceleration_y1
  %tmp68 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68, <4 x float>* %acceleration_z1
  %tmp70 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp70, <4 x float>* %acceleration_x2
  %tmp72 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp72, <4 x float>* %acceleration_y2
  %tmp74 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp74, <4 x float>* %acceleration_z2
  store i32 0, i32* %i
  br label %for.cond75

for.cond75:                                       ; preds = %for.inc, %for.body
  %tmp76 = load i32* %i                           ; <i32> [#uses=1]
  %tmp77 = load i32* %inner_loop_count            ; <i32> [#uses=1]
  %cmp78 = icmp slt i32 %tmp76, %tmp77            ; <i1> [#uses=1]
  br i1 %cmp78, label %for.body79, label %for.end

for.body79:                                       ; preds = %for.cond75
  %tmp81 = load i32* %i                           ; <i32> [#uses=1]
  %tmp82 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds <4 x float> addrspace(1)* %tmp82, i32 %tmp81 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp84 = load <4 x float> addrspace(1)* %arrayidx83 ; <<4 x float>> [#uses=1]
  %tmp85 = load <4 x float>* %current_x1          ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp84, %tmp85          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %dx
  %tmp87 = load i32* %i                           ; <i32> [#uses=1]
  %tmp88 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx89 = getelementptr inbounds <4 x float> addrspace(1)* %tmp88, i32 %tmp87 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp90 = load <4 x float> addrspace(1)* %arrayidx89 ; <<4 x float>> [#uses=1]
  %tmp91 = load <4 x float>* %current_y1          ; <<4 x float>> [#uses=1]
  %sub92 = fsub <4 x float> %tmp90, %tmp91        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub92, <4 x float>* %dy
  %tmp94 = load i32* %i                           ; <i32> [#uses=1]
  %tmp95 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx96 = getelementptr inbounds <4 x float> addrspace(1)* %tmp95, i32 %tmp94 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp97 = load <4 x float> addrspace(1)* %arrayidx96 ; <<4 x float>> [#uses=1]
  %tmp98 = load <4 x float>* %current_z1          ; <<4 x float>> [#uses=1]
  %sub99 = fsub <4 x float> %tmp97, %tmp98        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub99, <4 x float>* %dz
  %tmp101 = load i32* %i                          ; <i32> [#uses=1]
  %tmp102 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds <4 x float> addrspace(1)* %tmp102, i32 %tmp101 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp104 = load <4 x float> addrspace(1)* %arrayidx103 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp104, <4 x float>* %mi
  %tmp106 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %tmp107 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %mul108 = fmul <4 x float> %tmp106, %tmp107     ; <<4 x float>> [#uses=1]
  %tmp109 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %tmp110 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %mul111 = fmul <4 x float> %tmp109, %tmp110     ; <<4 x float>> [#uses=1]
  %add112 = fadd <4 x float> %mul108, %mul111     ; <<4 x float>> [#uses=1]
  %tmp113 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %tmp114 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %mul115 = fmul <4 x float> %tmp113, %tmp114     ; <<4 x float>> [#uses=1]
  %add116 = fadd <4 x float> %add112, %mul115     ; <<4 x float>> [#uses=1]
  store <4 x float> %add116, <4 x float>* %distance_squared
  %tmp117 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp118 = insertelement <4 x float> undef, float %tmp117, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp118, <4 x float> %tmp118, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp119 = load <4 x float>* %distance_squared   ; <<4 x float>> [#uses=1]
  %add120 = fadd <4 x float> %tmp119, %splat      ; <<4 x float>> [#uses=1]
  store <4 x float> %add120, <4 x float>* %distance_squared
  %tmp122 = load <4 x float>* %distance_squared   ; <<4 x float>> [#uses=1]
  %call123 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %tmp122) ; <<4 x float>> [#uses=1]
  store <4 x float> %call123, <4 x float>* %inverse_distance
  %tmp125 = load <4 x float>* %mi                 ; <<4 x float>> [#uses=1]
  %tmp126 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %mul127 = fmul <4 x float> %tmp125, %tmp126     ; <<4 x float>> [#uses=1]
  %tmp128 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %mul130 = fmul <4 x float> %tmp128, %tmp129     ; <<4 x float>> [#uses=1]
  %mul131 = fmul <4 x float> %mul127, %mul130     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul131, <4 x float>* %s
  %tmp132 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %tmp133 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul134 = fmul <4 x float> %tmp132, %tmp133     ; <<4 x float>> [#uses=1]
  %tmp135 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %add136 = fadd <4 x float> %tmp135, %mul134     ; <<4 x float>> [#uses=1]
  store <4 x float> %add136, <4 x float>* %acceleration_x1
  %tmp137 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %tmp138 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul139 = fmul <4 x float> %tmp137, %tmp138     ; <<4 x float>> [#uses=1]
  %tmp140 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %add141 = fadd <4 x float> %tmp140, %mul139     ; <<4 x float>> [#uses=1]
  store <4 x float> %add141, <4 x float>* %acceleration_y1
  %tmp142 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %tmp143 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul144 = fmul <4 x float> %tmp142, %tmp143     ; <<4 x float>> [#uses=1]
  %tmp145 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %add146 = fadd <4 x float> %tmp145, %mul144     ; <<4 x float>> [#uses=1]
  store <4 x float> %add146, <4 x float>* %acceleration_z1
  %tmp147 = load i32* %i                          ; <i32> [#uses=1]
  %tmp148 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx149 = getelementptr inbounds <4 x float> addrspace(1)* %tmp148, i32 %tmp147 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp150 = load <4 x float> addrspace(1)* %arrayidx149 ; <<4 x float>> [#uses=1]
  %tmp151 = load <4 x float>* %current_x2         ; <<4 x float>> [#uses=1]
  %sub152 = fsub <4 x float> %tmp150, %tmp151     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub152, <4 x float>* %dx
  %tmp153 = load i32* %i                          ; <i32> [#uses=1]
  %tmp154 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx155 = getelementptr inbounds <4 x float> addrspace(1)* %tmp154, i32 %tmp153 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp156 = load <4 x float> addrspace(1)* %arrayidx155 ; <<4 x float>> [#uses=1]
  %tmp157 = load <4 x float>* %current_y2         ; <<4 x float>> [#uses=1]
  %sub158 = fsub <4 x float> %tmp156, %tmp157     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub158, <4 x float>* %dy
  %tmp159 = load i32* %i                          ; <i32> [#uses=1]
  %tmp160 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx161 = getelementptr inbounds <4 x float> addrspace(1)* %tmp160, i32 %tmp159 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp162 = load <4 x float> addrspace(1)* %arrayidx161 ; <<4 x float>> [#uses=1]
  %tmp163 = load <4 x float>* %current_z2         ; <<4 x float>> [#uses=1]
  %sub164 = fsub <4 x float> %tmp162, %tmp163     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub164, <4 x float>* %dz
  %tmp165 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %tmp166 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %mul167 = fmul <4 x float> %tmp165, %tmp166     ; <<4 x float>> [#uses=1]
  %tmp168 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %tmp169 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %mul170 = fmul <4 x float> %tmp168, %tmp169     ; <<4 x float>> [#uses=1]
  %add171 = fadd <4 x float> %mul167, %mul170     ; <<4 x float>> [#uses=1]
  %tmp172 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %tmp173 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %mul174 = fmul <4 x float> %tmp172, %tmp173     ; <<4 x float>> [#uses=1]
  %add175 = fadd <4 x float> %add171, %mul174     ; <<4 x float>> [#uses=1]
  store <4 x float> %add175, <4 x float>* %distance_squared
  %tmp176 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp177 = insertelement <4 x float> undef, float %tmp176, i32 0 ; <<4 x float>> [#uses=2]
  %splat178 = shufflevector <4 x float> %tmp177, <4 x float> %tmp177, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp179 = load <4 x float>* %distance_squared   ; <<4 x float>> [#uses=1]
  %add180 = fadd <4 x float> %tmp179, %splat178   ; <<4 x float>> [#uses=1]
  store <4 x float> %add180, <4 x float>* %distance_squared
  %tmp181 = load <4 x float>* %distance_squared   ; <<4 x float>> [#uses=1]
  %call182 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %tmp181) ; <<4 x float>> [#uses=1]
  store <4 x float> %call182, <4 x float>* %inverse_distance
  %tmp183 = load <4 x float>* %mi                 ; <<4 x float>> [#uses=1]
  %tmp184 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %mul185 = fmul <4 x float> %tmp183, %tmp184     ; <<4 x float>> [#uses=1]
  %tmp186 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %tmp187 = load <4 x float>* %inverse_distance   ; <<4 x float>> [#uses=1]
  %mul188 = fmul <4 x float> %tmp186, %tmp187     ; <<4 x float>> [#uses=1]
  %mul189 = fmul <4 x float> %mul185, %mul188     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul189, <4 x float>* %s
  %tmp190 = load <4 x float>* %dx                 ; <<4 x float>> [#uses=1]
  %tmp191 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul192 = fmul <4 x float> %tmp190, %tmp191     ; <<4 x float>> [#uses=1]
  %tmp193 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %add194 = fadd <4 x float> %tmp193, %mul192     ; <<4 x float>> [#uses=1]
  store <4 x float> %add194, <4 x float>* %acceleration_x2
  %tmp195 = load <4 x float>* %dy                 ; <<4 x float>> [#uses=1]
  %tmp196 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul197 = fmul <4 x float> %tmp195, %tmp196     ; <<4 x float>> [#uses=1]
  %tmp198 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %add199 = fadd <4 x float> %tmp198, %mul197     ; <<4 x float>> [#uses=1]
  store <4 x float> %add199, <4 x float>* %acceleration_y2
  %tmp200 = load <4 x float>* %dz                 ; <<4 x float>> [#uses=1]
  %tmp201 = load <4 x float>* %s                  ; <<4 x float>> [#uses=1]
  %mul202 = fmul <4 x float> %tmp200, %tmp201     ; <<4 x float>> [#uses=1]
  %tmp203 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %add204 = fadd <4 x float> %tmp203, %mul202     ; <<4 x float>> [#uses=1]
  store <4 x float> %add204, <4 x float>* %acceleration_z2
  br label %for.inc

for.inc:                                          ; preds = %for.body79
  %tmp205 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp205, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond75

for.end:                                          ; preds = %for.cond75
  %tmp206 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp207 = extractelement <4 x float> %tmp206, i32 0 ; <float> [#uses=1]
  %tmp208 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp209 = extractelement <4 x float> %tmp208, i32 1 ; <float> [#uses=1]
  %add210 = fadd float %tmp207, %tmp209           ; <float> [#uses=1]
  %tmp211 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp212 = extractelement <4 x float> %tmp211, i32 2 ; <float> [#uses=1]
  %add213 = fadd float %add210, %tmp212           ; <float> [#uses=1]
  %tmp214 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp215 = extractelement <4 x float> %tmp214, i32 3 ; <float> [#uses=1]
  %add216 = fadd float %add213, %tmp215           ; <float> [#uses=1]
  %tmp217 = load <4 x float>* %final_ax           ; <<4 x float>> [#uses=1]
  %tmp218 = insertelement <4 x float> %tmp217, float %add216, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp218, <4 x float>* %final_ax
  %tmp219 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp220 = extractelement <4 x float> %tmp219, i32 0 ; <float> [#uses=1]
  %tmp221 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp222 = extractelement <4 x float> %tmp221, i32 1 ; <float> [#uses=1]
  %add223 = fadd float %tmp220, %tmp222           ; <float> [#uses=1]
  %tmp224 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp225 = extractelement <4 x float> %tmp224, i32 2 ; <float> [#uses=1]
  %add226 = fadd float %add223, %tmp225           ; <float> [#uses=1]
  %tmp227 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp228 = extractelement <4 x float> %tmp227, i32 3 ; <float> [#uses=1]
  %add229 = fadd float %add226, %tmp228           ; <float> [#uses=1]
  %tmp230 = load <4 x float>* %final_ay           ; <<4 x float>> [#uses=1]
  %tmp231 = insertelement <4 x float> %tmp230, float %add229, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231, <4 x float>* %final_ay
  %tmp232 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp233 = extractelement <4 x float> %tmp232, i32 0 ; <float> [#uses=1]
  %tmp234 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp235 = extractelement <4 x float> %tmp234, i32 1 ; <float> [#uses=1]
  %add236 = fadd float %tmp233, %tmp235           ; <float> [#uses=1]
  %tmp237 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp238 = extractelement <4 x float> %tmp237, i32 2 ; <float> [#uses=1]
  %add239 = fadd float %add236, %tmp238           ; <float> [#uses=1]
  %tmp240 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp241 = extractelement <4 x float> %tmp240, i32 3 ; <float> [#uses=1]
  %add242 = fadd float %add239, %tmp241           ; <float> [#uses=1]
  %tmp243 = load <4 x float>* %final_az           ; <<4 x float>> [#uses=1]
  %tmp244 = insertelement <4 x float> %tmp243, float %add242, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp244, <4 x float>* %final_az
  %tmp245 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp246 = extractelement <4 x float> %tmp245, i32 0 ; <float> [#uses=1]
  %tmp247 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp248 = extractelement <4 x float> %tmp247, i32 1 ; <float> [#uses=1]
  %add249 = fadd float %tmp246, %tmp248           ; <float> [#uses=1]
  %tmp250 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp251 = extractelement <4 x float> %tmp250, i32 2 ; <float> [#uses=1]
  %add252 = fadd float %add249, %tmp251           ; <float> [#uses=1]
  %tmp253 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp254 = extractelement <4 x float> %tmp253, i32 3 ; <float> [#uses=1]
  %add255 = fadd float %add252, %tmp254           ; <float> [#uses=1]
  %tmp256 = load <4 x float>* %final_ax           ; <<4 x float>> [#uses=1]
  %tmp257 = insertelement <4 x float> %tmp256, float %add255, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp257, <4 x float>* %final_ax
  %tmp258 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp259 = extractelement <4 x float> %tmp258, i32 0 ; <float> [#uses=1]
  %tmp260 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp261 = extractelement <4 x float> %tmp260, i32 1 ; <float> [#uses=1]
  %add262 = fadd float %tmp259, %tmp261           ; <float> [#uses=1]
  %tmp263 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp264 = extractelement <4 x float> %tmp263, i32 2 ; <float> [#uses=1]
  %add265 = fadd float %add262, %tmp264           ; <float> [#uses=1]
  %tmp266 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp267 = extractelement <4 x float> %tmp266, i32 3 ; <float> [#uses=1]
  %add268 = fadd float %add265, %tmp267           ; <float> [#uses=1]
  %tmp269 = load <4 x float>* %final_ay           ; <<4 x float>> [#uses=1]
  %tmp270 = insertelement <4 x float> %tmp269, float %add268, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp270, <4 x float>* %final_ay
  %tmp271 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp272 = extractelement <4 x float> %tmp271, i32 0 ; <float> [#uses=1]
  %tmp273 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp274 = extractelement <4 x float> %tmp273, i32 1 ; <float> [#uses=1]
  %add275 = fadd float %tmp272, %tmp274           ; <float> [#uses=1]
  %tmp276 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp277 = extractelement <4 x float> %tmp276, i32 2 ; <float> [#uses=1]
  %add278 = fadd float %add275, %tmp277           ; <float> [#uses=1]
  %tmp279 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp280 = extractelement <4 x float> %tmp279, i32 3 ; <float> [#uses=1]
  %add281 = fadd float %add278, %tmp280           ; <float> [#uses=1]
  %tmp282 = load <4 x float>* %final_az           ; <<4 x float>> [#uses=1]
  %tmp283 = insertelement <4 x float> %tmp282, float %add281, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp283, <4 x float>* %final_az
  %tmp284 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %tmp285 = shufflevector <4 x float> %tmp284, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp285, <4 x float>* %current_x1
  %tmp286 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %tmp287 = shufflevector <4 x float> %tmp286, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp287, <4 x float>* %current_y1
  %tmp288 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp289 = shufflevector <4 x float> %tmp288, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp289, <4 x float>* %current_z1
  %tmp290 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp291 = shufflevector <4 x float> %tmp290, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp291, <4 x float>* %current_mass1
  %tmp292 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %tmp293 = shufflevector <4 x float> %tmp292, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp293, <4 x float>* %current_x2
  %tmp294 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %tmp295 = shufflevector <4 x float> %tmp294, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp295, <4 x float>* %current_y2
  %tmp296 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp297 = shufflevector <4 x float> %tmp296, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp297, <4 x float>* %current_z2
  %tmp298 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp299 = shufflevector <4 x float> %tmp298, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp299, <4 x float>* %current_mass2
  %tmp300 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp300, <4 x float>* %acceleration_x1
  %tmp301 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp301, <4 x float>* %acceleration_y1
  %tmp302 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp302, <4 x float>* %acceleration_z1
  %tmp303 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp303, <4 x float>* %acceleration_x2
  %tmp304 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp304, <4 x float>* %acceleration_y2
  %tmp305 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp305, <4 x float>* %acceleration_z2
  store i32 0, i32* %i
  br label %for.cond306

for.cond306:                                      ; preds = %for.inc445, %for.end
  %tmp307 = load i32* %i                          ; <i32> [#uses=1]
  %tmp308 = load i32* %inner_loop_count           ; <i32> [#uses=1]
  %cmp309 = icmp slt i32 %tmp307, %tmp308         ; <i1> [#uses=1]
  br i1 %cmp309, label %for.body310, label %for.end448

for.body310:                                      ; preds = %for.cond306
  %tmp313 = load i32* %i                          ; <i32> [#uses=1]
  %tmp314 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx315 = getelementptr inbounds <4 x float> addrspace(1)* %tmp314, i32 %tmp313 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp316 = load <4 x float> addrspace(1)* %arrayidx315 ; <<4 x float>> [#uses=1]
  %tmp317 = load <4 x float>* %current_x1         ; <<4 x float>> [#uses=1]
  %sub318 = fsub <4 x float> %tmp316, %tmp317     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub318, <4 x float>* %dx312
  %tmp321 = load i32* %i                          ; <i32> [#uses=1]
  %tmp322 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx323 = getelementptr inbounds <4 x float> addrspace(1)* %tmp322, i32 %tmp321 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp324 = load <4 x float> addrspace(1)* %arrayidx323 ; <<4 x float>> [#uses=1]
  %tmp325 = load <4 x float>* %current_y1         ; <<4 x float>> [#uses=1]
  %sub326 = fsub <4 x float> %tmp324, %tmp325     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub326, <4 x float>* %dy320
  %tmp329 = load i32* %i                          ; <i32> [#uses=1]
  %tmp330 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx331 = getelementptr inbounds <4 x float> addrspace(1)* %tmp330, i32 %tmp329 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp332 = load <4 x float> addrspace(1)* %arrayidx331 ; <<4 x float>> [#uses=1]
  %tmp333 = load <4 x float>* %current_z1         ; <<4 x float>> [#uses=1]
  %sub334 = fsub <4 x float> %tmp332, %tmp333     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub334, <4 x float>* %dz328
  %tmp337 = load i32* %i                          ; <i32> [#uses=1]
  %tmp338 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx339 = getelementptr inbounds <4 x float> addrspace(1)* %tmp338, i32 %tmp337 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp340 = load <4 x float> addrspace(1)* %arrayidx339 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp340, <4 x float>* %mi336
  %tmp343 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %tmp344 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %mul345 = fmul <4 x float> %tmp343, %tmp344     ; <<4 x float>> [#uses=1]
  %tmp346 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %tmp347 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %mul348 = fmul <4 x float> %tmp346, %tmp347     ; <<4 x float>> [#uses=1]
  %add349 = fadd <4 x float> %mul345, %mul348     ; <<4 x float>> [#uses=1]
  %tmp350 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %tmp351 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %mul352 = fmul <4 x float> %tmp350, %tmp351     ; <<4 x float>> [#uses=1]
  %add353 = fadd <4 x float> %add349, %mul352     ; <<4 x float>> [#uses=1]
  store <4 x float> %add353, <4 x float>* %distance_squared342
  %tmp354 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp355 = insertelement <4 x float> undef, float %tmp354, i32 0 ; <<4 x float>> [#uses=2]
  %splat356 = shufflevector <4 x float> %tmp355, <4 x float> %tmp355, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp357 = load <4 x float>* %distance_squared342 ; <<4 x float>> [#uses=1]
  %add358 = fadd <4 x float> %tmp357, %splat356   ; <<4 x float>> [#uses=1]
  store <4 x float> %add358, <4 x float>* %distance_squared342
  %tmp361 = load <4 x float>* %distance_squared342 ; <<4 x float>> [#uses=1]
  %call362 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %tmp361) ; <<4 x float>> [#uses=1]
  store <4 x float> %call362, <4 x float>* %inverse_distance360
  %tmp365 = load <4 x float>* %mi336              ; <<4 x float>> [#uses=1]
  %tmp366 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %mul367 = fmul <4 x float> %tmp365, %tmp366     ; <<4 x float>> [#uses=1]
  %tmp368 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %tmp369 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %mul370 = fmul <4 x float> %tmp368, %tmp369     ; <<4 x float>> [#uses=1]
  %mul371 = fmul <4 x float> %mul367, %mul370     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul371, <4 x float>* %s364
  %tmp372 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %tmp373 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul374 = fmul <4 x float> %tmp372, %tmp373     ; <<4 x float>> [#uses=1]
  %tmp375 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %add376 = fadd <4 x float> %tmp375, %mul374     ; <<4 x float>> [#uses=1]
  store <4 x float> %add376, <4 x float>* %acceleration_x1
  %tmp377 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %tmp378 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul379 = fmul <4 x float> %tmp377, %tmp378     ; <<4 x float>> [#uses=1]
  %tmp380 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %add381 = fadd <4 x float> %tmp380, %mul379     ; <<4 x float>> [#uses=1]
  store <4 x float> %add381, <4 x float>* %acceleration_y1
  %tmp382 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %tmp383 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul384 = fmul <4 x float> %tmp382, %tmp383     ; <<4 x float>> [#uses=1]
  %tmp385 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %add386 = fadd <4 x float> %tmp385, %mul384     ; <<4 x float>> [#uses=1]
  store <4 x float> %add386, <4 x float>* %acceleration_z1
  %tmp387 = load i32* %i                          ; <i32> [#uses=1]
  %tmp388 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx389 = getelementptr inbounds <4 x float> addrspace(1)* %tmp388, i32 %tmp387 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp390 = load <4 x float> addrspace(1)* %arrayidx389 ; <<4 x float>> [#uses=1]
  %tmp391 = load <4 x float>* %current_x2         ; <<4 x float>> [#uses=1]
  %sub392 = fsub <4 x float> %tmp390, %tmp391     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub392, <4 x float>* %dx312
  %tmp393 = load i32* %i                          ; <i32> [#uses=1]
  %tmp394 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx395 = getelementptr inbounds <4 x float> addrspace(1)* %tmp394, i32 %tmp393 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp396 = load <4 x float> addrspace(1)* %arrayidx395 ; <<4 x float>> [#uses=1]
  %tmp397 = load <4 x float>* %current_y2         ; <<4 x float>> [#uses=1]
  %sub398 = fsub <4 x float> %tmp396, %tmp397     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub398, <4 x float>* %dy320
  %tmp399 = load i32* %i                          ; <i32> [#uses=1]
  %tmp400 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx401 = getelementptr inbounds <4 x float> addrspace(1)* %tmp400, i32 %tmp399 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp402 = load <4 x float> addrspace(1)* %arrayidx401 ; <<4 x float>> [#uses=1]
  %tmp403 = load <4 x float>* %current_z2         ; <<4 x float>> [#uses=1]
  %sub404 = fsub <4 x float> %tmp402, %tmp403     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub404, <4 x float>* %dz328
  %tmp405 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %tmp406 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %mul407 = fmul <4 x float> %tmp405, %tmp406     ; <<4 x float>> [#uses=1]
  %tmp408 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %tmp409 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %mul410 = fmul <4 x float> %tmp408, %tmp409     ; <<4 x float>> [#uses=1]
  %add411 = fadd <4 x float> %mul407, %mul410     ; <<4 x float>> [#uses=1]
  %tmp412 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %tmp413 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %mul414 = fmul <4 x float> %tmp412, %tmp413     ; <<4 x float>> [#uses=1]
  %add415 = fadd <4 x float> %add411, %mul414     ; <<4 x float>> [#uses=1]
  store <4 x float> %add415, <4 x float>* %distance_squared342
  %tmp416 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp417 = insertelement <4 x float> undef, float %tmp416, i32 0 ; <<4 x float>> [#uses=2]
  %splat418 = shufflevector <4 x float> %tmp417, <4 x float> %tmp417, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp419 = load <4 x float>* %distance_squared342 ; <<4 x float>> [#uses=1]
  %add420 = fadd <4 x float> %tmp419, %splat418   ; <<4 x float>> [#uses=1]
  store <4 x float> %add420, <4 x float>* %distance_squared342
  %tmp421 = load <4 x float>* %distance_squared342 ; <<4 x float>> [#uses=1]
  %call422 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %tmp421) ; <<4 x float>> [#uses=1]
  store <4 x float> %call422, <4 x float>* %inverse_distance360
  %tmp423 = load <4 x float>* %mi336              ; <<4 x float>> [#uses=1]
  %tmp424 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %mul425 = fmul <4 x float> %tmp423, %tmp424     ; <<4 x float>> [#uses=1]
  %tmp426 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %tmp427 = load <4 x float>* %inverse_distance360 ; <<4 x float>> [#uses=1]
  %mul428 = fmul <4 x float> %tmp426, %tmp427     ; <<4 x float>> [#uses=1]
  %mul429 = fmul <4 x float> %mul425, %mul428     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul429, <4 x float>* %s364
  %tmp430 = load <4 x float>* %dx312              ; <<4 x float>> [#uses=1]
  %tmp431 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul432 = fmul <4 x float> %tmp430, %tmp431     ; <<4 x float>> [#uses=1]
  %tmp433 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %add434 = fadd <4 x float> %tmp433, %mul432     ; <<4 x float>> [#uses=1]
  store <4 x float> %add434, <4 x float>* %acceleration_x2
  %tmp435 = load <4 x float>* %dy320              ; <<4 x float>> [#uses=1]
  %tmp436 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul437 = fmul <4 x float> %tmp435, %tmp436     ; <<4 x float>> [#uses=1]
  %tmp438 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %add439 = fadd <4 x float> %tmp438, %mul437     ; <<4 x float>> [#uses=1]
  store <4 x float> %add439, <4 x float>* %acceleration_y2
  %tmp440 = load <4 x float>* %dz328              ; <<4 x float>> [#uses=1]
  %tmp441 = load <4 x float>* %s364               ; <<4 x float>> [#uses=1]
  %mul442 = fmul <4 x float> %tmp440, %tmp441     ; <<4 x float>> [#uses=1]
  %tmp443 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %add444 = fadd <4 x float> %tmp443, %mul442     ; <<4 x float>> [#uses=1]
  store <4 x float> %add444, <4 x float>* %acceleration_z2
  br label %for.inc445

for.inc445:                                       ; preds = %for.body310
  %tmp446 = load i32* %i                          ; <i32> [#uses=1]
  %inc447 = add nsw i32 %tmp446, 1                ; <i32> [#uses=1]
  store i32 %inc447, i32* %i
  br label %for.cond306

for.end448:                                       ; preds = %for.cond306
  %tmp449 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp450 = extractelement <4 x float> %tmp449, i32 0 ; <float> [#uses=1]
  %tmp451 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp452 = extractelement <4 x float> %tmp451, i32 1 ; <float> [#uses=1]
  %add453 = fadd float %tmp450, %tmp452           ; <float> [#uses=1]
  %tmp454 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp455 = extractelement <4 x float> %tmp454, i32 2 ; <float> [#uses=1]
  %add456 = fadd float %add453, %tmp455           ; <float> [#uses=1]
  %tmp457 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp458 = extractelement <4 x float> %tmp457, i32 3 ; <float> [#uses=1]
  %add459 = fadd float %add456, %tmp458           ; <float> [#uses=1]
  %tmp460 = load <4 x float>* %final_ax           ; <<4 x float>> [#uses=1]
  %tmp461 = insertelement <4 x float> %tmp460, float %add459, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp461, <4 x float>* %final_ax
  %tmp462 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp463 = extractelement <4 x float> %tmp462, i32 0 ; <float> [#uses=1]
  %tmp464 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp465 = extractelement <4 x float> %tmp464, i32 1 ; <float> [#uses=1]
  %add466 = fadd float %tmp463, %tmp465           ; <float> [#uses=1]
  %tmp467 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp468 = extractelement <4 x float> %tmp467, i32 2 ; <float> [#uses=1]
  %add469 = fadd float %add466, %tmp468           ; <float> [#uses=1]
  %tmp470 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp471 = extractelement <4 x float> %tmp470, i32 3 ; <float> [#uses=1]
  %add472 = fadd float %add469, %tmp471           ; <float> [#uses=1]
  %tmp473 = load <4 x float>* %final_ay           ; <<4 x float>> [#uses=1]
  %tmp474 = insertelement <4 x float> %tmp473, float %add472, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp474, <4 x float>* %final_ay
  %tmp475 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp476 = extractelement <4 x float> %tmp475, i32 0 ; <float> [#uses=1]
  %tmp477 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp478 = extractelement <4 x float> %tmp477, i32 1 ; <float> [#uses=1]
  %add479 = fadd float %tmp476, %tmp478           ; <float> [#uses=1]
  %tmp480 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp481 = extractelement <4 x float> %tmp480, i32 2 ; <float> [#uses=1]
  %add482 = fadd float %add479, %tmp481           ; <float> [#uses=1]
  %tmp483 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp484 = extractelement <4 x float> %tmp483, i32 3 ; <float> [#uses=1]
  %add485 = fadd float %add482, %tmp484           ; <float> [#uses=1]
  %tmp486 = load <4 x float>* %final_az           ; <<4 x float>> [#uses=1]
  %tmp487 = insertelement <4 x float> %tmp486, float %add485, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp487, <4 x float>* %final_az
  %tmp488 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp489 = extractelement <4 x float> %tmp488, i32 0 ; <float> [#uses=1]
  %tmp490 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp491 = extractelement <4 x float> %tmp490, i32 1 ; <float> [#uses=1]
  %add492 = fadd float %tmp489, %tmp491           ; <float> [#uses=1]
  %tmp493 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp494 = extractelement <4 x float> %tmp493, i32 2 ; <float> [#uses=1]
  %add495 = fadd float %add492, %tmp494           ; <float> [#uses=1]
  %tmp496 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %tmp497 = extractelement <4 x float> %tmp496, i32 3 ; <float> [#uses=1]
  %add498 = fadd float %add495, %tmp497           ; <float> [#uses=1]
  %tmp499 = load <4 x float>* %final_ax           ; <<4 x float>> [#uses=1]
  %tmp500 = insertelement <4 x float> %tmp499, float %add498, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp500, <4 x float>* %final_ax
  %tmp501 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp502 = extractelement <4 x float> %tmp501, i32 0 ; <float> [#uses=1]
  %tmp503 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp504 = extractelement <4 x float> %tmp503, i32 1 ; <float> [#uses=1]
  %add505 = fadd float %tmp502, %tmp504           ; <float> [#uses=1]
  %tmp506 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp507 = extractelement <4 x float> %tmp506, i32 2 ; <float> [#uses=1]
  %add508 = fadd float %add505, %tmp507           ; <float> [#uses=1]
  %tmp509 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %tmp510 = extractelement <4 x float> %tmp509, i32 3 ; <float> [#uses=1]
  %add511 = fadd float %add508, %tmp510           ; <float> [#uses=1]
  %tmp512 = load <4 x float>* %final_ay           ; <<4 x float>> [#uses=1]
  %tmp513 = insertelement <4 x float> %tmp512, float %add511, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp513, <4 x float>* %final_ay
  %tmp514 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp515 = extractelement <4 x float> %tmp514, i32 0 ; <float> [#uses=1]
  %tmp516 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp517 = extractelement <4 x float> %tmp516, i32 1 ; <float> [#uses=1]
  %add518 = fadd float %tmp515, %tmp517           ; <float> [#uses=1]
  %tmp519 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp520 = extractelement <4 x float> %tmp519, i32 2 ; <float> [#uses=1]
  %add521 = fadd float %add518, %tmp520           ; <float> [#uses=1]
  %tmp522 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %tmp523 = extractelement <4 x float> %tmp522, i32 3 ; <float> [#uses=1]
  %add524 = fadd float %add521, %tmp523           ; <float> [#uses=1]
  %tmp525 = load <4 x float>* %final_az           ; <<4 x float>> [#uses=1]
  %tmp526 = insertelement <4 x float> %tmp525, float %add524, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp526, <4 x float>* %final_az
  %tmp527 = load i32* %k                          ; <i32> [#uses=1]
  %tmp528 = load <4 x float> addrspace(1)** %input_velocity_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx529 = getelementptr inbounds <4 x float> addrspace(1)* %tmp528, i32 %tmp527 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp530 = load <4 x float> addrspace(1)* %arrayidx529 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp530, <4 x float>* %velocity_x
  %tmp531 = load i32* %k                          ; <i32> [#uses=1]
  %tmp532 = load <4 x float> addrspace(1)** %input_velocity_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx533 = getelementptr inbounds <4 x float> addrspace(1)* %tmp532, i32 %tmp531 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp534 = load <4 x float> addrspace(1)* %arrayidx533 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp534, <4 x float>* %velocity_y
  %tmp535 = load i32* %k                          ; <i32> [#uses=1]
  %tmp536 = load <4 x float> addrspace(1)** %input_velocity_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx537 = getelementptr inbounds <4 x float> addrspace(1)* %tmp536, i32 %tmp535 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp538 = load <4 x float> addrspace(1)* %arrayidx537 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp538, <4 x float>* %velocity_z
  %tmp539 = load <4 x float>* %final_ax           ; <<4 x float>> [#uses=1]
  %tmp540 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp541 = insertelement <4 x float> undef, float %tmp540, i32 0 ; <<4 x float>> [#uses=2]
  %splat542 = shufflevector <4 x float> %tmp541, <4 x float> %tmp541, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul543 = fmul <4 x float> %tmp539, %splat542   ; <<4 x float>> [#uses=1]
  %tmp544 = load <4 x float>* %velocity_x         ; <<4 x float>> [#uses=1]
  %add545 = fadd <4 x float> %tmp544, %mul543     ; <<4 x float>> [#uses=1]
  store <4 x float> %add545, <4 x float>* %velocity_x
  %tmp546 = load <4 x float>* %final_ay           ; <<4 x float>> [#uses=1]
  %tmp547 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp548 = insertelement <4 x float> undef, float %tmp547, i32 0 ; <<4 x float>> [#uses=2]
  %splat549 = shufflevector <4 x float> %tmp548, <4 x float> %tmp548, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul550 = fmul <4 x float> %tmp546, %splat549   ; <<4 x float>> [#uses=1]
  %tmp551 = load <4 x float>* %velocity_y         ; <<4 x float>> [#uses=1]
  %add552 = fadd <4 x float> %tmp551, %mul550     ; <<4 x float>> [#uses=1]
  store <4 x float> %add552, <4 x float>* %velocity_y
  %tmp553 = load <4 x float>* %final_az           ; <<4 x float>> [#uses=1]
  %tmp554 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp555 = insertelement <4 x float> undef, float %tmp554, i32 0 ; <<4 x float>> [#uses=2]
  %splat556 = shufflevector <4 x float> %tmp555, <4 x float> %tmp555, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul557 = fmul <4 x float> %tmp553, %splat556   ; <<4 x float>> [#uses=1]
  %tmp558 = load <4 x float>* %velocity_z         ; <<4 x float>> [#uses=1]
  %add559 = fadd <4 x float> %tmp558, %mul557     ; <<4 x float>> [#uses=1]
  store <4 x float> %add559, <4 x float>* %velocity_z
  %tmp560 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp561 = insertelement <4 x float> undef, float %tmp560, i32 0 ; <<4 x float>> [#uses=2]
  %splat562 = shufflevector <4 x float> %tmp561, <4 x float> %tmp561, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp563 = load <4 x float>* %velocity_x         ; <<4 x float>> [#uses=1]
  %mul564 = fmul <4 x float> %tmp563, %splat562   ; <<4 x float>> [#uses=1]
  store <4 x float> %mul564, <4 x float>* %velocity_x
  %tmp565 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp566 = insertelement <4 x float> undef, float %tmp565, i32 0 ; <<4 x float>> [#uses=2]
  %splat567 = shufflevector <4 x float> %tmp566, <4 x float> %tmp566, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp568 = load <4 x float>* %velocity_y         ; <<4 x float>> [#uses=1]
  %mul569 = fmul <4 x float> %tmp568, %splat567   ; <<4 x float>> [#uses=1]
  store <4 x float> %mul569, <4 x float>* %velocity_y
  %tmp570 = load float* %damping.addr             ; <float> [#uses=1]
  %tmp571 = insertelement <4 x float> undef, float %tmp570, i32 0 ; <<4 x float>> [#uses=2]
  %splat572 = shufflevector <4 x float> %tmp571, <4 x float> %tmp571, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp573 = load <4 x float>* %velocity_z         ; <<4 x float>> [#uses=1]
  %mul574 = fmul <4 x float> %tmp573, %splat572   ; <<4 x float>> [#uses=1]
  store <4 x float> %mul574, <4 x float>* %velocity_z
  %tmp575 = load <4 x float>* %velocity_x         ; <<4 x float>> [#uses=1]
  %tmp576 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp577 = insertelement <4 x float> undef, float %tmp576, i32 0 ; <<4 x float>> [#uses=2]
  %splat578 = shufflevector <4 x float> %tmp577, <4 x float> %tmp577, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul579 = fmul <4 x float> %tmp575, %splat578   ; <<4 x float>> [#uses=1]
  %tmp580 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %add581 = fadd <4 x float> %tmp580, %mul579     ; <<4 x float>> [#uses=1]
  store <4 x float> %add581, <4 x float>* %position_x
  %tmp582 = load <4 x float>* %velocity_y         ; <<4 x float>> [#uses=1]
  %tmp583 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp584 = insertelement <4 x float> undef, float %tmp583, i32 0 ; <<4 x float>> [#uses=2]
  %splat585 = shufflevector <4 x float> %tmp584, <4 x float> %tmp584, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul586 = fmul <4 x float> %tmp582, %splat585   ; <<4 x float>> [#uses=1]
  %tmp587 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %add588 = fadd <4 x float> %tmp587, %mul586     ; <<4 x float>> [#uses=1]
  store <4 x float> %add588, <4 x float>* %position_y
  %tmp589 = load <4 x float>* %velocity_z         ; <<4 x float>> [#uses=1]
  %tmp590 = load float* %time_delta.addr          ; <float> [#uses=1]
  %tmp591 = insertelement <4 x float> undef, float %tmp590, i32 0 ; <<4 x float>> [#uses=2]
  %splat592 = shufflevector <4 x float> %tmp591, <4 x float> %tmp591, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul593 = fmul <4 x float> %tmp589, %splat592   ; <<4 x float>> [#uses=1]
  %tmp594 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %add595 = fadd <4 x float> %tmp594, %mul593     ; <<4 x float>> [#uses=1]
  store <4 x float> %add595, <4 x float>* %position_z
  %tmp596 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %tmp597 = load i32* %k                          ; <i32> [#uses=1]
  %tmp598 = load <4 x float> addrspace(1)** %output_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx599 = getelementptr inbounds <4 x float> addrspace(1)* %tmp598, i32 %tmp597 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp596, <4 x float> addrspace(1)* %arrayidx599
  %tmp600 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %tmp601 = load i32* %k                          ; <i32> [#uses=1]
  %tmp602 = load <4 x float> addrspace(1)** %output_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx603 = getelementptr inbounds <4 x float> addrspace(1)* %tmp602, i32 %tmp601 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp600, <4 x float> addrspace(1)* %arrayidx603
  %tmp604 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp605 = load i32* %k                          ; <i32> [#uses=1]
  %tmp606 = load <4 x float> addrspace(1)** %output_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx607 = getelementptr inbounds <4 x float> addrspace(1)* %tmp606, i32 %tmp605 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp604, <4 x float> addrspace(1)* %arrayidx607
  %tmp608 = load <4 x float>* %velocity_x         ; <<4 x float>> [#uses=1]
  %tmp609 = load i32* %k                          ; <i32> [#uses=1]
  %tmp610 = load <4 x float> addrspace(1)** %output_velocity_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx611 = getelementptr inbounds <4 x float> addrspace(1)* %tmp610, i32 %tmp609 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp608, <4 x float> addrspace(1)* %arrayidx611
  %tmp612 = load <4 x float>* %velocity_y         ; <<4 x float>> [#uses=1]
  %tmp613 = load i32* %k                          ; <i32> [#uses=1]
  %tmp614 = load <4 x float> addrspace(1)** %output_velocity_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx615 = getelementptr inbounds <4 x float> addrspace(1)* %tmp614, i32 %tmp613 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp612, <4 x float> addrspace(1)* %arrayidx615
  %tmp616 = load <4 x float>* %velocity_z         ; <<4 x float>> [#uses=1]
  %tmp617 = load i32* %k                          ; <i32> [#uses=1]
  %tmp618 = load <4 x float> addrspace(1)** %output_velocity_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx619 = getelementptr inbounds <4 x float> addrspace(1)* %tmp618, i32 %tmp617 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp616, <4 x float> addrspace(1)* %arrayidx619
  %tmp621 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=3]
  %tmp622 = extractelement <4 x float> %tmp621, i32 0 ; <float> [#uses=0]
  %0 = shufflevector <4 x float> %tmp621, <4 x float> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp623 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=2]
  %tmp624 = extractelement <4 x float> %tmp623, i32 0 ; <float> [#uses=0]
  %1 = shufflevector <4 x float> %tmp621, <4 x float> %tmp623, <4 x i32> <i32 0, i32 4, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp625 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp626 = extractelement <4 x float> %tmp625, i32 0 ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> %1, float %tmp626, i32 2 ; <<4 x float>> [#uses=1]
  %tmp627 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp628 = extractelement <4 x float> %tmp627, i32 0 ; <float> [#uses=1]
  %vecinit629 = insertelement <4 x float> %vecinit, float %tmp628, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit629, <4 x float>* %.compoundliteral620
  %tmp630 = load <4 x float>* %.compoundliteral620 ; <<4 x float>> [#uses=1]
  %tmp631 = load i32* %l                          ; <i32> [#uses=1]
  %mul632 = mul i32 4, %tmp631                    ; <i32> [#uses=1]
  %add633 = add nsw i32 %mul632, 0                ; <i32> [#uses=1]
  %tmp634 = load i32* %offset                     ; <i32> [#uses=1]
  %add635 = add nsw i32 %add633, %tmp634          ; <i32> [#uses=1]
  %tmp636 = load <4 x float> addrspace(1)** %output_position.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx637 = getelementptr inbounds <4 x float> addrspace(1)* %tmp636, i32 %add635 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp630, <4 x float> addrspace(1)* %arrayidx637
  %tmp639 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=3]
  %tmp640 = extractelement <4 x float> %tmp639, i32 1 ; <float> [#uses=0]
  %2 = shufflevector <4 x float> %tmp639, <4 x float> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp641 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=2]
  %tmp642 = extractelement <4 x float> %tmp641, i32 1 ; <float> [#uses=0]
  %3 = shufflevector <4 x float> %tmp639, <4 x float> %tmp641, <4 x i32> <i32 1, i32 5, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp643 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp644 = extractelement <4 x float> %tmp643, i32 1 ; <float> [#uses=1]
  %vecinit645 = insertelement <4 x float> %3, float %tmp644, i32 2 ; <<4 x float>> [#uses=1]
  %tmp646 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp647 = extractelement <4 x float> %tmp646, i32 1 ; <float> [#uses=1]
  %vecinit648 = insertelement <4 x float> %vecinit645, float %tmp647, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit648, <4 x float>* %.compoundliteral638
  %tmp649 = load <4 x float>* %.compoundliteral638 ; <<4 x float>> [#uses=1]
  %tmp650 = load i32* %l                          ; <i32> [#uses=1]
  %mul651 = mul i32 4, %tmp650                    ; <i32> [#uses=1]
  %add652 = add nsw i32 %mul651, 1                ; <i32> [#uses=1]
  %tmp653 = load i32* %offset                     ; <i32> [#uses=1]
  %add654 = add nsw i32 %add652, %tmp653          ; <i32> [#uses=1]
  %tmp655 = load <4 x float> addrspace(1)** %output_position.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx656 = getelementptr inbounds <4 x float> addrspace(1)* %tmp655, i32 %add654 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp649, <4 x float> addrspace(1)* %arrayidx656
  %tmp658 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=3]
  %tmp659 = extractelement <4 x float> %tmp658, i32 2 ; <float> [#uses=0]
  %4 = shufflevector <4 x float> %tmp658, <4 x float> undef, <4 x i32> <i32 2, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp660 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=2]
  %tmp661 = extractelement <4 x float> %tmp660, i32 2 ; <float> [#uses=0]
  %5 = shufflevector <4 x float> %tmp658, <4 x float> %tmp660, <4 x i32> <i32 2, i32 6, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp662 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp663 = extractelement <4 x float> %tmp662, i32 2 ; <float> [#uses=1]
  %vecinit664 = insertelement <4 x float> %5, float %tmp663, i32 2 ; <<4 x float>> [#uses=1]
  %tmp665 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp666 = extractelement <4 x float> %tmp665, i32 2 ; <float> [#uses=1]
  %vecinit667 = insertelement <4 x float> %vecinit664, float %tmp666, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit667, <4 x float>* %.compoundliteral657
  %tmp668 = load <4 x float>* %.compoundliteral657 ; <<4 x float>> [#uses=1]
  %tmp669 = load i32* %l                          ; <i32> [#uses=1]
  %mul670 = mul i32 4, %tmp669                    ; <i32> [#uses=1]
  %add671 = add nsw i32 %mul670, 2                ; <i32> [#uses=1]
  %tmp672 = load i32* %offset                     ; <i32> [#uses=1]
  %add673 = add nsw i32 %add671, %tmp672          ; <i32> [#uses=1]
  %tmp674 = load <4 x float> addrspace(1)** %output_position.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx675 = getelementptr inbounds <4 x float> addrspace(1)* %tmp674, i32 %add673 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp668, <4 x float> addrspace(1)* %arrayidx675
  %tmp677 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=3]
  %tmp678 = extractelement <4 x float> %tmp677, i32 3 ; <float> [#uses=0]
  %6 = shufflevector <4 x float> %tmp677, <4 x float> undef, <4 x i32> <i32 3, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp679 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=2]
  %tmp680 = extractelement <4 x float> %tmp679, i32 3 ; <float> [#uses=0]
  %7 = shufflevector <4 x float> %tmp677, <4 x float> %tmp679, <4 x i32> <i32 3, i32 7, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp681 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %tmp682 = extractelement <4 x float> %tmp681, i32 3 ; <float> [#uses=1]
  %vecinit683 = insertelement <4 x float> %7, float %tmp682, i32 2 ; <<4 x float>> [#uses=1]
  %tmp684 = load <4 x float>* %m                  ; <<4 x float>> [#uses=1]
  %tmp685 = extractelement <4 x float> %tmp684, i32 3 ; <float> [#uses=1]
  %vecinit686 = insertelement <4 x float> %vecinit683, float %tmp685, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit686, <4 x float>* %.compoundliteral676
  %tmp687 = load <4 x float>* %.compoundliteral676 ; <<4 x float>> [#uses=1]
  %tmp688 = load i32* %l                          ; <i32> [#uses=1]
  %mul689 = mul i32 4, %tmp688                    ; <i32> [#uses=1]
  %add690 = add nsw i32 %mul689, 3                ; <i32> [#uses=1]
  %tmp691 = load i32* %offset                     ; <i32> [#uses=1]
  %add692 = add nsw i32 %add690, %tmp691          ; <i32> [#uses=1]
  %tmp693 = load <4 x float> addrspace(1)** %output_position.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx694 = getelementptr inbounds <4 x float> addrspace(1)* %tmp693, i32 %add692 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp687, <4 x float> addrspace(1)* %arrayidx694
  br label %for.inc695

for.inc695:                                       ; preds = %for.end448
  %tmp696 = load i32* %l                          ; <i32> [#uses=1]
  %inc697 = add nsw i32 %tmp696, 1                ; <i32> [#uses=1]
  store i32 %inc697, i32* %l
  br label %for.cond

for.end698:                                       ; preds = %for.cond
  ret void
}

declare <4 x float> @_Z12native_rsqrtDv4_f(<4 x float>)
