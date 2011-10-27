; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIDwtHaar1D.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_dwtHaar1D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_dwtHaar1D_parameters = appending global [189 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(3))) *, uint, uint, uint\00", section "llvm.metadata" ; <[189 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i32, i32)* @dwtHaar1D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_dwtHaar1D_locals to i8*), i8* getelementptr inbounds ([189 x i8]* @opencl_dwtHaar1D_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @dwtHaar1D(float addrspace(1)* %inSignal, float addrspace(1)* %coefsSignal, float addrspace(1)* %AverageSignal, float addrspace(3)* %sharedArray, i32 %tLevels, i32 %signalLength, i32 %levelsDone) nounwind {
entry:
  %inSignal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %coefsSignal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %AverageSignal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %sharedArray.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=9]
  %tLevels.addr = alloca i32, align 4             ; <i32*> [#uses=3]
  %signalLength.addr = alloca i32, align 4        ; <i32*> [#uses=4]
  %levelsDone.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %localId = alloca i32, align 4                  ; <i32*> [#uses=13]
  %groupId = alloca i32, align 4                  ; <i32*> [#uses=5]
  %globalId = alloca i32, align 4                 ; <i32*> [#uses=1]
  %localSize = alloca i32, align 4                ; <i32*> [#uses=3]
  %mLevels = alloca i32, align 4                  ; <i32*> [#uses=1]
  %levels = alloca i32, align 4                   ; <i32*> [#uses=3]
  %activeThreads = alloca i32, align 4            ; <i32*> [#uses=5]
  %midOutPos = alloca i32, align 4                ; <i32*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  %data0 = alloca float, align 4                  ; <float*> [#uses=3]
  %data1 = alloca float, align 4                  ; <float*> [#uses=3]
  %globalPos = alloca i32, align 4                ; <i32*> [#uses=2]
  store float addrspace(1)* %inSignal, float addrspace(1)** %inSignal.addr
  store float addrspace(1)* %coefsSignal, float addrspace(1)** %coefsSignal.addr
  store float addrspace(1)* %AverageSignal, float addrspace(1)** %AverageSignal.addr
  store float addrspace(3)* %sharedArray, float addrspace(3)** %sharedArray.addr
  store i32 %tLevels, i32* %tLevels.addr
  store i32 %signalLength, i32* %signalLength.addr
  store i32 %levelsDone, i32* %levelsDone.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %localId
  %call1 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %groupId
  %call2 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call2, i32* %globalId
  %call3 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call3, i32* %localSize
  %tmp = load i32* %groupId                       ; <i32> [#uses=1]
  %tmp4 = load i32* %localSize                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp4                      ; <i32> [#uses=1]
  %mul5 = mul i32 %mul, 2                         ; <i32> [#uses=1]
  %tmp6 = load i32* %localId                      ; <i32> [#uses=1]
  %mul7 = mul i32 %tmp6, 2                        ; <i32> [#uses=1]
  %add = add i32 %mul5, %mul7                     ; <i32> [#uses=1]
  %tmp8 = load float addrspace(1)** %inSignal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp8, i32 %add ; <float addrspace(1)*> [#uses=1]
  %tmp9 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  %tmp10 = load i32* %localId                     ; <i32> [#uses=1]
  %mul11 = mul i32 %tmp10, 2                      ; <i32> [#uses=1]
  %tmp12 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx13 = getelementptr inbounds float addrspace(3)* %tmp12, i32 %mul11 ; <float addrspace(3)*> [#uses=1]
  store float %tmp9, float addrspace(3)* %arrayidx13
  %tmp14 = load i32* %groupId                     ; <i32> [#uses=1]
  %tmp15 = load i32* %localSize                   ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp14, %tmp15                 ; <i32> [#uses=1]
  %mul17 = mul i32 %mul16, 2                      ; <i32> [#uses=1]
  %tmp18 = load i32* %localId                     ; <i32> [#uses=1]
  %mul19 = mul i32 %tmp18, 2                      ; <i32> [#uses=1]
  %add20 = add i32 %mul17, %mul19                 ; <i32> [#uses=1]
  %add21 = add i32 %add20, 1                      ; <i32> [#uses=1]
  %tmp22 = load float addrspace(1)** %inSignal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %tmp22, i32 %add21 ; <float addrspace(1)*> [#uses=1]
  %tmp24 = load float addrspace(1)* %arrayidx23   ; <float> [#uses=1]
  %tmp25 = load i32* %localId                     ; <i32> [#uses=1]
  %mul26 = mul i32 %tmp25, 2                      ; <i32> [#uses=1]
  %add27 = add i32 %mul26, 1                      ; <i32> [#uses=1]
  %tmp28 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(3)* %tmp28, i32 %add27 ; <float addrspace(3)*> [#uses=1]
  store float %tmp24, float addrspace(3)* %arrayidx29
  %tmp30 = load i32* %levelsDone.addr             ; <i32> [#uses=1]
  %cmp = icmp eq i32 0, %tmp30                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp31 = load i32* %signalLength.addr           ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp31 to float              ; <float> [#uses=1]
  %call32 = call float @_Z4sqrtf(float %conv)     ; <float> [#uses=3]
  %tmp33 = load i32* %localId                     ; <i32> [#uses=1]
  %mul34 = mul i32 %tmp33, 2                      ; <i32> [#uses=1]
  %tmp35 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds float addrspace(3)* %tmp35, i32 %mul34 ; <float addrspace(3)*> [#uses=2]
  %tmp37 = load float addrspace(3)* %arrayidx36   ; <float> [#uses=1]
  %cmp38 = fcmp oeq float 0.000000e+000, %call32  ; <i1> [#uses=1]
  %sel = select i1 %cmp38, float 1.000000e+000, float %call32 ; <float> [#uses=0]
  %div = fdiv float %tmp37, %call32               ; <float> [#uses=1]
  store float %div, float addrspace(3)* %arrayidx36
  %tmp39 = load i32* %signalLength.addr           ; <i32> [#uses=1]
  %conv40 = uitofp i32 %tmp39 to float            ; <float> [#uses=1]
  %call41 = call float @_Z4sqrtf(float %conv40)   ; <float> [#uses=3]
  %tmp42 = load i32* %localId                     ; <i32> [#uses=1]
  %mul43 = mul i32 %tmp42, 2                      ; <i32> [#uses=1]
  %add44 = add i32 %mul43, 1                      ; <i32> [#uses=1]
  %tmp45 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float addrspace(3)* %tmp45, i32 %add44 ; <float addrspace(3)*> [#uses=2]
  %tmp47 = load float addrspace(3)* %arrayidx46   ; <float> [#uses=1]
  %cmp48 = fcmp oeq float 0.000000e+000, %call41  ; <i1> [#uses=1]
  %sel49 = select i1 %cmp48, float 1.000000e+000, float %call41 ; <float> [#uses=0]
  %div50 = fdiv float %tmp47, %call41             ; <float> [#uses=1]
  store float %div50, float addrspace(3)* %arrayidx46
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @barrier(i32 1)
  store i32 9, i32* %mLevels
  %tmp53 = load i32* %tLevels.addr                ; <i32> [#uses=1]
  %cmp54 = icmp ugt i32 %tmp53, 9                 ; <i1> [#uses=1]
  br i1 %cmp54, label %cond.true, label %cond.false

cond.true:                                        ; preds = %if.end
  br label %cond.end

cond.false:                                       ; preds = %if.end
  %tmp56 = load i32* %tLevels.addr                ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ 9, %cond.true ], [ %tmp56, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond, i32* %levels
  %tmp58 = load i32* %levels                      ; <i32> [#uses=1]
  %shl = shl i32 1, %tmp58                        ; <i32> [#uses=1]
  %div59 = sdiv i32 %shl, 2                       ; <i32> [#uses=1]
  store i32 %div59, i32* %activeThreads
  %tmp61 = load i32* %signalLength.addr           ; <i32> [#uses=1]
  %div62 = udiv i32 %tmp61, 2                     ; <i32> [#uses=1]
  store i32 %div62, i32* %midOutPos
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %cond.end
  %tmp64 = load i32* %i                           ; <i32> [#uses=1]
  %tmp65 = load i32* %levels                      ; <i32> [#uses=1]
  %cmp66 = icmp ult i32 %tmp64, %tmp65            ; <i1> [#uses=1]
  br i1 %cmp66, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp68 = load i32* %localId                     ; <i32> [#uses=1]
  %tmp69 = load i32* %activeThreads               ; <i32> [#uses=1]
  %cmp70 = icmp ult i32 %tmp68, %tmp69            ; <i1> [#uses=1]
  br i1 %cmp70, label %if.then72, label %if.end114

if.then72:                                        ; preds = %for.body
  %tmp74 = load i32* %localId                     ; <i32> [#uses=1]
  %mul75 = mul i32 2, %tmp74                      ; <i32> [#uses=1]
  %tmp76 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(3)* %tmp76, i32 %mul75 ; <float addrspace(3)*> [#uses=1]
  %tmp78 = load float addrspace(3)* %arrayidx77   ; <float> [#uses=1]
  store float %tmp78, float* %data0
  %tmp80 = load i32* %localId                     ; <i32> [#uses=1]
  %mul81 = mul i32 2, %tmp80                      ; <i32> [#uses=1]
  %add82 = add i32 %mul81, 1                      ; <i32> [#uses=1]
  %tmp83 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(3)* %tmp83, i32 %add82 ; <float addrspace(3)*> [#uses=1]
  %tmp85 = load float addrspace(3)* %arrayidx84   ; <float> [#uses=1]
  store float %tmp85, float* %data1
  %tmp86 = load float* %data0                     ; <float> [#uses=1]
  %tmp87 = load float* %data1                     ; <float> [#uses=1]
  %add88 = fadd float %tmp86, %tmp87              ; <float> [#uses=1]
  %call89 = call float @_Z4sqrtf(float 2.000000e+000) ; <float> [#uses=3]
  %cmp90 = fcmp oeq float 0.000000e+000, %call89  ; <i1> [#uses=1]
  %sel91 = select i1 %cmp90, float 1.000000e+000, float %call89 ; <float> [#uses=0]
  %div92 = fdiv float %add88, %call89             ; <float> [#uses=1]
  %tmp93 = load i32* %localId                     ; <i32> [#uses=1]
  %tmp94 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds float addrspace(3)* %tmp94, i32 %tmp93 ; <float addrspace(3)*> [#uses=1]
  store float %div92, float addrspace(3)* %arrayidx95
  %tmp97 = load i32* %midOutPos                   ; <i32> [#uses=1]
  %tmp98 = load i32* %groupId                     ; <i32> [#uses=1]
  %tmp99 = load i32* %activeThreads               ; <i32> [#uses=1]
  %mul100 = mul i32 %tmp98, %tmp99                ; <i32> [#uses=1]
  %add101 = add i32 %tmp97, %mul100               ; <i32> [#uses=1]
  %tmp102 = load i32* %localId                    ; <i32> [#uses=1]
  %add103 = add i32 %add101, %tmp102              ; <i32> [#uses=1]
  store i32 %add103, i32* %globalPos
  %tmp104 = load float* %data0                    ; <float> [#uses=1]
  %tmp105 = load float* %data1                    ; <float> [#uses=1]
  %sub = fsub float %tmp104, %tmp105              ; <float> [#uses=1]
  %call106 = call float @_Z4sqrtf(float 2.000000e+000) ; <float> [#uses=3]
  %cmp107 = fcmp oeq float 0.000000e+000, %call106 ; <i1> [#uses=1]
  %sel108 = select i1 %cmp107, float 1.000000e+000, float %call106 ; <float> [#uses=0]
  %div109 = fdiv float %sub, %call106             ; <float> [#uses=1]
  %tmp110 = load i32* %globalPos                  ; <i32> [#uses=1]
  %tmp111 = load float addrspace(1)** %coefsSignal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx112 = getelementptr inbounds float addrspace(1)* %tmp111, i32 %tmp110 ; <float addrspace(1)*> [#uses=1]
  store float %div109, float addrspace(1)* %arrayidx112
  %tmp113 = load i32* %midOutPos                  ; <i32> [#uses=1]
  %shr = lshr i32 %tmp113, 1                      ; <i32> [#uses=1]
  store i32 %shr, i32* %midOutPos
  br label %if.end114

if.end114:                                        ; preds = %if.then72, %for.body
  %tmp115 = load i32* %activeThreads              ; <i32> [#uses=1]
  %shr116 = lshr i32 %tmp115, 1                   ; <i32> [#uses=1]
  store i32 %shr116, i32* %activeThreads
  call void @barrier(i32 1)
  br label %for.inc

for.inc:                                          ; preds = %if.end114
  %tmp117 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add i32 %tmp117, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp118 = load i32* %localId                    ; <i32> [#uses=1]
  %cmp119 = icmp eq i32 0, %tmp118                ; <i1> [#uses=1]
  br i1 %cmp119, label %if.then121, label %if.end128

if.then121:                                       ; preds = %for.end
  %tmp122 = load float addrspace(3)** %sharedArray.addr ; <float addrspace(3)*> [#uses=1]
  %arrayidx123 = getelementptr inbounds float addrspace(3)* %tmp122, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp124 = load float addrspace(3)* %arrayidx123 ; <float> [#uses=1]
  %tmp125 = load i32* %groupId                    ; <i32> [#uses=1]
  %tmp126 = load float addrspace(1)** %AverageSignal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx127 = getelementptr inbounds float addrspace(1)* %tmp126, i32 %tmp125 ; <float addrspace(1)*> [#uses=1]
  store float %tmp124, float addrspace(1)* %arrayidx127
  br label %if.end128

if.end128:                                        ; preds = %if.then121, %for.end
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare float @_Z4sqrtf(float)

declare void @barrier(i32)
