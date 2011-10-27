; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIEigenValue.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_calNumEigenValueInterval_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_calNumEigenValueInterval_parameters = appending global [182 x i8] c"uint __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[182 x i8]*> [#uses=1]
@opencl_recalculateEigenIntervals_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_recalculateEigenIntervals_parameters = appending global [238 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const, float const\00", section "llvm.metadata" ; <[238 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @calNumEigenValueInterval to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_calNumEigenValueInterval_locals to i8*), i8* getelementptr inbounds ([182 x i8]* @opencl_calNumEigenValueInterval_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, float)* @recalculateEigenIntervals to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_recalculateEigenIntervals_locals to i8*), i8* getelementptr inbounds ([238 x i8]* @opencl_recalculateEigenIntervals_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define float @calNumEigenValuesLessThan(float addrspace(1)* %diagonal, float addrspace(1)* %offDiagonal, i32 %width, float %x) nounwind {
entry:
  %retval = alloca float, align 4                 ; <float*> [#uses=2]
  %diagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %offDiagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %x.addr = alloca float, align 4                 ; <float*> [#uses=3]
  %count = alloca i32, align 4                    ; <i32*> [#uses=6]
  %prev_diff = alloca float, align 4              ; <float*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=7]
  %diff = alloca float, align 4                   ; <float*> [#uses=3]
  store float addrspace(1)* %diagonal, float addrspace(1)** %diagonal.addr
  store float addrspace(1)* %offDiagonal, float addrspace(1)** %offDiagonal.addr
  store i32 %width, i32* %width.addr
  store float %x, float* %x.addr
  store i32 0, i32* %count
  %tmp = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp1 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  %tmp2 = load float* %x.addr                     ; <float> [#uses=1]
  %sub = fsub float %tmp1, %tmp2                  ; <float> [#uses=1]
  store float %sub, float* %prev_diff
  %tmp3 = load float* %prev_diff                  ; <float> [#uses=1]
  %cmp = fcmp olt float %tmp3, 0.000000e+000      ; <i1> [#uses=1]
  %cond = select i1 %cmp, i32 1, i32 0            ; <i32> [#uses=1]
  %tmp4 = load i32* %count                        ; <i32> [#uses=1]
  %add = add i32 %tmp4, %cond                     ; <i32> [#uses=1]
  store i32 %add, i32* %count
  store i32 1, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %tmp7 = load i32* %width.addr                   ; <i32> [#uses=1]
  %cmp8 = icmp ult i32 %tmp6, %tmp7               ; <i1> [#uses=1]
  br i1 %cmp8, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %tmp11 = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds float addrspace(1)* %tmp11, i32 %tmp10 ; <float addrspace(1)*> [#uses=1]
  %tmp13 = load float addrspace(1)* %arrayidx12   ; <float> [#uses=1]
  %tmp14 = load float* %x.addr                    ; <float> [#uses=1]
  %sub15 = fsub float %tmp13, %tmp14              ; <float> [#uses=1]
  %tmp16 = load i32* %i                           ; <i32> [#uses=1]
  %sub17 = sub i32 %tmp16, 1                      ; <i32> [#uses=1]
  %tmp18 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds float addrspace(1)* %tmp18, i32 %sub17 ; <float addrspace(1)*> [#uses=1]
  %tmp20 = load float addrspace(1)* %arrayidx19   ; <float> [#uses=1]
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %sub22 = sub i32 %tmp21, 1                      ; <i32> [#uses=1]
  %tmp23 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds float addrspace(1)* %tmp23, i32 %sub22 ; <float addrspace(1)*> [#uses=1]
  %tmp25 = load float addrspace(1)* %arrayidx24   ; <float> [#uses=1]
  %mul = fmul float %tmp20, %tmp25                ; <float> [#uses=1]
  %tmp26 = load float* %prev_diff                 ; <float> [#uses=3]
  %cmp27 = fcmp oeq float 0.000000e+000, %tmp26   ; <i1> [#uses=1]
  %sel = select i1 %cmp27, float 1.000000e+000, float %tmp26 ; <float> [#uses=0]
  %div = fdiv float %mul, %tmp26                  ; <float> [#uses=1]
  %sub28 = fsub float %sub15, %div                ; <float> [#uses=1]
  store float %sub28, float* %diff
  %tmp29 = load float* %diff                      ; <float> [#uses=1]
  %cmp30 = fcmp olt float %tmp29, 0.000000e+000   ; <i1> [#uses=1]
  %cond31 = select i1 %cmp30, i32 1, i32 0        ; <i32> [#uses=1]
  %tmp32 = load i32* %count                       ; <i32> [#uses=1]
  %add33 = add i32 %tmp32, %cond31                ; <i32> [#uses=1]
  store i32 %add33, i32* %count
  %tmp34 = load float* %diff                      ; <float> [#uses=1]
  store float %tmp34, float* %prev_diff
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp35 = load i32* %i                           ; <i32> [#uses=1]
  %add36 = add i32 %tmp35, 1                      ; <i32> [#uses=1]
  store i32 %add36, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp37 = load i32* %count                       ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp37 to float              ; <float> [#uses=1]
  store float %conv, float* %retval
  %0 = load float* %retval                        ; <float> [#uses=1]
  ret float %0
}

define void @calNumEigenValueInterval(i32 addrspace(1)* %numEigenIntervals, float addrspace(1)* %eigenIntervals, float addrspace(1)* %diagonal, float addrspace(1)* %offDiagonal, i32 %width) nounwind {
entry:
  %numEigenIntervals.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %eigenIntervals.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %diagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %offDiagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=3]
  %threadId = alloca i32, align 4                 ; <i32*> [#uses=3]
  %lowerId = alloca i32, align 4                  ; <i32*> [#uses=3]
  %upperId = alloca i32, align 4                  ; <i32*> [#uses=2]
  %lowerLimit = alloca float, align 4             ; <float*> [#uses=2]
  %upperLimit = alloca float, align 4             ; <float*> [#uses=2]
  %numEigenValuesLessThanLowerLimit = alloca i32, align 4 ; <i32*> [#uses=2]
  %numEigenValuesLessThanUpperLimit = alloca i32, align 4 ; <i32*> [#uses=2]
  store i32 addrspace(1)* %numEigenIntervals, i32 addrspace(1)** %numEigenIntervals.addr
  store float addrspace(1)* %eigenIntervals, float addrspace(1)** %eigenIntervals.addr
  store float addrspace(1)* %diagonal, float addrspace(1)** %diagonal.addr
  store float addrspace(1)* %offDiagonal, float addrspace(1)** %offDiagonal.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %threadId
  %tmp = load i32* %threadId                      ; <i32> [#uses=1]
  %mul = mul i32 2, %tmp                          ; <i32> [#uses=1]
  store i32 %mul, i32* %lowerId
  %tmp2 = load i32* %lowerId                      ; <i32> [#uses=1]
  %add = add i32 %tmp2, 1                         ; <i32> [#uses=1]
  store i32 %add, i32* %upperId
  %tmp4 = load i32* %lowerId                      ; <i32> [#uses=1]
  %tmp5 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp5, i32 %tmp4 ; <float addrspace(1)*> [#uses=1]
  %tmp6 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  store float %tmp6, float* %lowerLimit
  %tmp8 = load i32* %upperId                      ; <i32> [#uses=1]
  %tmp9 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx10 = getelementptr inbounds float addrspace(1)* %tmp9, i32 %tmp8 ; <float addrspace(1)*> [#uses=1]
  %tmp11 = load float addrspace(1)* %arrayidx10   ; <float> [#uses=1]
  store float %tmp11, float* %upperLimit
  %tmp13 = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp14 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp16 = load float* %lowerLimit                ; <float> [#uses=1]
  %call17 = call float @calNumEigenValuesLessThan(float addrspace(1)* %tmp13, float addrspace(1)* %tmp14, i32 %tmp15, float %tmp16) ; <float> [#uses=1]
  %conv = fptoui float %call17 to i32             ; <i32> [#uses=1]
  store i32 %conv, i32* %numEigenValuesLessThanLowerLimit
  %tmp19 = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp20 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp21 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp22 = load float* %upperLimit                ; <float> [#uses=1]
  %call23 = call float @calNumEigenValuesLessThan(float addrspace(1)* %tmp19, float addrspace(1)* %tmp20, i32 %tmp21, float %tmp22) ; <float> [#uses=1]
  %conv24 = fptoui float %call23 to i32           ; <i32> [#uses=1]
  store i32 %conv24, i32* %numEigenValuesLessThanUpperLimit
  %tmp25 = load i32* %numEigenValuesLessThanUpperLimit ; <i32> [#uses=1]
  %tmp26 = load i32* %numEigenValuesLessThanLowerLimit ; <i32> [#uses=1]
  %sub = sub i32 %tmp25, %tmp26                   ; <i32> [#uses=1]
  %tmp27 = load i32* %threadId                    ; <i32> [#uses=1]
  %tmp28 = load i32 addrspace(1)** %numEigenIntervals.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds i32 addrspace(1)* %tmp28, i32 %tmp27 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %sub, i32 addrspace(1)* %arrayidx29
  ret void
}

declare i32 @get_global_id(i32)

define void @recalculateEigenIntervals(float addrspace(1)* %newEigenIntervals, float addrspace(1)* %eigenIntervals, i32 addrspace(1)* %numEigenIntervals, float addrspace(1)* %diagonal, float addrspace(1)* %offDiagonal, i32 %width, float %tolerance) nounwind {
entry:
  %newEigenIntervals.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=10]
  %eigenIntervals.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=13]
  %numEigenIntervals.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=5]
  %diagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %offDiagonal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=3]
  %tolerance.addr = alloca float, align 4         ; <float*> [#uses=2]
  %threadId = alloca i32, align 4                 ; <i32*> [#uses=3]
  %currentIndex = alloca i32, align 4             ; <i32*> [#uses=5]
  %lowerId = alloca i32, align 4                  ; <i32*> [#uses=7]
  %upperId = alloca i32, align 4                  ; <i32*> [#uses=5]
  %index = alloca i32, align 4                    ; <i32*> [#uses=8]
  %lId = alloca i32, align 4                      ; <i32*> [#uses=9]
  %uId = alloca i32, align 4                      ; <i32*> [#uses=6]
  %midValue = alloca float, align 4               ; <float*> [#uses=4]
  %n = alloca float, align 4                      ; <float*> [#uses=2]
  %divisionWidth = alloca float, align 4          ; <float*> [#uses=3]
  store float addrspace(1)* %newEigenIntervals, float addrspace(1)** %newEigenIntervals.addr
  store float addrspace(1)* %eigenIntervals, float addrspace(1)** %eigenIntervals.addr
  store i32 addrspace(1)* %numEigenIntervals, i32 addrspace(1)** %numEigenIntervals.addr
  store float addrspace(1)* %diagonal, float addrspace(1)** %diagonal.addr
  store float addrspace(1)* %offDiagonal, float addrspace(1)** %offDiagonal.addr
  store i32 %width, i32* %width.addr
  store float %tolerance, float* %tolerance.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %threadId
  %tmp = load i32* %threadId                      ; <i32> [#uses=1]
  store i32 %tmp, i32* %currentIndex
  %tmp2 = load i32* %threadId                     ; <i32> [#uses=1]
  %mul = mul i32 2, %tmp2                         ; <i32> [#uses=1]
  store i32 %mul, i32* %lowerId
  %tmp4 = load i32* %lowerId                      ; <i32> [#uses=1]
  %add = add i32 %tmp4, 1                         ; <i32> [#uses=1]
  store i32 %add, i32* %upperId
  store i32 0, i32* %index
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %tmp6 = load i32* %currentIndex                 ; <i32> [#uses=1]
  %tmp7 = load i32* %index                        ; <i32> [#uses=1]
  %tmp8 = load i32 addrspace(1)** %numEigenIntervals.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp8, i32 %tmp7 ; <i32 addrspace(1)*> [#uses=1]
  %tmp9 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  %cmp = icmp uge i32 %tmp6, %tmp9                ; <i1> [#uses=1]
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %tmp10 = load i32* %index                       ; <i32> [#uses=1]
  %tmp11 = load i32 addrspace(1)** %numEigenIntervals.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds i32 addrspace(1)* %tmp11, i32 %tmp10 ; <i32 addrspace(1)*> [#uses=1]
  %tmp13 = load i32 addrspace(1)* %arrayidx12     ; <i32> [#uses=1]
  %tmp14 = load i32* %currentIndex                ; <i32> [#uses=1]
  %sub = sub i32 %tmp14, %tmp13                   ; <i32> [#uses=1]
  store i32 %sub, i32* %currentIndex
  %tmp15 = load i32* %index                       ; <i32> [#uses=1]
  %inc = add i32 %tmp15, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %tmp17 = load i32* %index                       ; <i32> [#uses=1]
  %mul18 = mul i32 2, %tmp17                      ; <i32> [#uses=1]
  store i32 %mul18, i32* %lId
  %tmp20 = load i32* %lId                         ; <i32> [#uses=1]
  %add21 = add i32 %tmp20, 1                      ; <i32> [#uses=1]
  store i32 %add21, i32* %uId
  %tmp22 = load i32* %index                       ; <i32> [#uses=1]
  %tmp23 = load i32 addrspace(1)** %numEigenIntervals.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds i32 addrspace(1)* %tmp23, i32 %tmp22 ; <i32 addrspace(1)*> [#uses=1]
  %tmp25 = load i32 addrspace(1)* %arrayidx24     ; <i32> [#uses=1]
  %cmp26 = icmp eq i32 %tmp25, 1                  ; <i1> [#uses=1]
  br i1 %cmp26, label %if.then, label %if.else105

if.then:                                          ; preds = %while.end
  %tmp28 = load i32* %uId                         ; <i32> [#uses=1]
  %tmp29 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds float addrspace(1)* %tmp29, i32 %tmp28 ; <float addrspace(1)*> [#uses=1]
  %tmp31 = load float addrspace(1)* %arrayidx30   ; <float> [#uses=1]
  %tmp32 = load i32* %lId                         ; <i32> [#uses=1]
  %tmp33 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 %tmp32 ; <float addrspace(1)*> [#uses=1]
  %tmp35 = load float addrspace(1)* %arrayidx34   ; <float> [#uses=1]
  %add36 = fadd float %tmp31, %tmp35              ; <float> [#uses=1]
  %div = fdiv float %add36, 2.000000e+000         ; <float> [#uses=1]
  store float %div, float* %midValue
  %tmp38 = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp39 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp40 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp41 = load float* %midValue                  ; <float> [#uses=1]
  %call42 = call float @calNumEigenValuesLessThan(float addrspace(1)* %tmp38, float addrspace(1)* %tmp39, i32 %tmp40, float %tmp41) ; <float> [#uses=1]
  %tmp43 = load float addrspace(1)** %diagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp44 = load float addrspace(1)** %offDiagonal.addr ; <float addrspace(1)*> [#uses=1]
  %tmp45 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp46 = load i32* %lId                         ; <i32> [#uses=1]
  %tmp47 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx48 = getelementptr inbounds float addrspace(1)* %tmp47, i32 %tmp46 ; <float addrspace(1)*> [#uses=1]
  %tmp49 = load float addrspace(1)* %arrayidx48   ; <float> [#uses=1]
  %call50 = call float @calNumEigenValuesLessThan(float addrspace(1)* %tmp43, float addrspace(1)* %tmp44, i32 %tmp45, float %tmp49) ; <float> [#uses=1]
  %sub51 = fsub float %call42, %call50            ; <float> [#uses=1]
  store float %sub51, float* %n
  %tmp52 = load i32* %uId                         ; <i32> [#uses=1]
  %tmp53 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx54 = getelementptr inbounds float addrspace(1)* %tmp53, i32 %tmp52 ; <float addrspace(1)*> [#uses=1]
  %tmp55 = load float addrspace(1)* %arrayidx54   ; <float> [#uses=1]
  %tmp56 = load i32* %lId                         ; <i32> [#uses=1]
  %tmp57 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx58 = getelementptr inbounds float addrspace(1)* %tmp57, i32 %tmp56 ; <float addrspace(1)*> [#uses=1]
  %tmp59 = load float addrspace(1)* %arrayidx58   ; <float> [#uses=1]
  %sub60 = fsub float %tmp55, %tmp59              ; <float> [#uses=1]
  %tmp61 = load float* %tolerance.addr            ; <float> [#uses=1]
  %cmp62 = fcmp olt float %sub60, %tmp61          ; <i1> [#uses=1]
  br i1 %cmp62, label %if.then63, label %if.else

if.then63:                                        ; preds = %if.then
  %tmp64 = load i32* %lId                         ; <i32> [#uses=1]
  %tmp65 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float addrspace(1)* %tmp65, i32 %tmp64 ; <float addrspace(1)*> [#uses=1]
  %tmp67 = load float addrspace(1)* %arrayidx66   ; <float> [#uses=1]
  %tmp68 = load i32* %lowerId                     ; <i32> [#uses=1]
  %tmp69 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds float addrspace(1)* %tmp69, i32 %tmp68 ; <float addrspace(1)*> [#uses=1]
  store float %tmp67, float addrspace(1)* %arrayidx70
  %tmp71 = load i32* %uId                         ; <i32> [#uses=1]
  %tmp72 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds float addrspace(1)* %tmp72, i32 %tmp71 ; <float addrspace(1)*> [#uses=1]
  %tmp74 = load float addrspace(1)* %arrayidx73   ; <float> [#uses=1]
  %tmp75 = load i32* %upperId                     ; <i32> [#uses=1]
  %tmp76 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(1)* %tmp76, i32 %tmp75 ; <float addrspace(1)*> [#uses=1]
  store float %tmp74, float addrspace(1)* %arrayidx77
  br label %if.end104

if.else:                                          ; preds = %if.then
  %tmp78 = load float* %n                         ; <float> [#uses=1]
  %cmp79 = fcmp oeq float %tmp78, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp79, label %if.then80, label %if.else92

if.then80:                                        ; preds = %if.else
  %tmp81 = load float* %midValue                  ; <float> [#uses=1]
  %tmp82 = load i32* %lowerId                     ; <i32> [#uses=1]
  %tmp83 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %tmp83, i32 %tmp82 ; <float addrspace(1)*> [#uses=1]
  store float %tmp81, float addrspace(1)* %arrayidx84
  %tmp85 = load i32* %uId                         ; <i32> [#uses=1]
  %tmp86 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx87 = getelementptr inbounds float addrspace(1)* %tmp86, i32 %tmp85 ; <float addrspace(1)*> [#uses=1]
  %tmp88 = load float addrspace(1)* %arrayidx87   ; <float> [#uses=1]
  %tmp89 = load i32* %upperId                     ; <i32> [#uses=1]
  %tmp90 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds float addrspace(1)* %tmp90, i32 %tmp89 ; <float addrspace(1)*> [#uses=1]
  store float %tmp88, float addrspace(1)* %arrayidx91
  br label %if.end

if.else92:                                        ; preds = %if.else
  %tmp93 = load i32* %lId                         ; <i32> [#uses=1]
  %tmp94 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds float addrspace(1)* %tmp94, i32 %tmp93 ; <float addrspace(1)*> [#uses=1]
  %tmp96 = load float addrspace(1)* %arrayidx95   ; <float> [#uses=1]
  %tmp97 = load i32* %lowerId                     ; <i32> [#uses=1]
  %tmp98 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds float addrspace(1)* %tmp98, i32 %tmp97 ; <float addrspace(1)*> [#uses=1]
  store float %tmp96, float addrspace(1)* %arrayidx99
  %tmp100 = load float* %midValue                 ; <float> [#uses=1]
  %tmp101 = load i32* %upperId                    ; <i32> [#uses=1]
  %tmp102 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds float addrspace(1)* %tmp102, i32 %tmp101 ; <float addrspace(1)*> [#uses=1]
  store float %tmp100, float addrspace(1)* %arrayidx103
  br label %if.end

if.end:                                           ; preds = %if.else92, %if.then80
  br label %if.end104

if.end104:                                        ; preds = %if.end, %if.then63
  br label %if.end143

if.else105:                                       ; preds = %while.end
  %tmp107 = load i32* %uId                        ; <i32> [#uses=1]
  %tmp108 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(1)* %tmp108, i32 %tmp107 ; <float addrspace(1)*> [#uses=1]
  %tmp110 = load float addrspace(1)* %arrayidx109 ; <float> [#uses=1]
  %tmp111 = load i32* %lId                        ; <i32> [#uses=1]
  %tmp112 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx113 = getelementptr inbounds float addrspace(1)* %tmp112, i32 %tmp111 ; <float addrspace(1)*> [#uses=1]
  %tmp114 = load float addrspace(1)* %arrayidx113 ; <float> [#uses=1]
  %sub115 = fsub float %tmp110, %tmp114           ; <float> [#uses=1]
  %tmp116 = load i32* %index                      ; <i32> [#uses=1]
  %tmp117 = load i32 addrspace(1)** %numEigenIntervals.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx118 = getelementptr inbounds i32 addrspace(1)* %tmp117, i32 %tmp116 ; <i32 addrspace(1)*> [#uses=1]
  %tmp119 = load i32 addrspace(1)* %arrayidx118   ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp119 to float             ; <float> [#uses=3]
  %cmp120 = fcmp oeq float 0.000000e+000, %conv   ; <i1> [#uses=1]
  %sel = select i1 %cmp120, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div121 = fdiv float %sub115, %conv             ; <float> [#uses=1]
  store float %div121, float* %divisionWidth
  %tmp122 = load i32* %lId                        ; <i32> [#uses=1]
  %tmp123 = load float addrspace(1)** %eigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx124 = getelementptr inbounds float addrspace(1)* %tmp123, i32 %tmp122 ; <float addrspace(1)*> [#uses=1]
  %tmp125 = load float addrspace(1)* %arrayidx124 ; <float> [#uses=1]
  %tmp126 = load float* %divisionWidth            ; <float> [#uses=1]
  %tmp127 = load i32* %currentIndex               ; <i32> [#uses=1]
  %conv128 = uitofp i32 %tmp127 to float          ; <float> [#uses=1]
  %mul129 = fmul float %tmp126, %conv128          ; <float> [#uses=1]
  %add130 = fadd float %tmp125, %mul129           ; <float> [#uses=1]
  %tmp131 = load i32* %lowerId                    ; <i32> [#uses=1]
  %tmp132 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx133 = getelementptr inbounds float addrspace(1)* %tmp132, i32 %tmp131 ; <float addrspace(1)*> [#uses=1]
  store float %add130, float addrspace(1)* %arrayidx133
  %tmp134 = load i32* %lowerId                    ; <i32> [#uses=1]
  %tmp135 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx136 = getelementptr inbounds float addrspace(1)* %tmp135, i32 %tmp134 ; <float addrspace(1)*> [#uses=1]
  %tmp137 = load float addrspace(1)* %arrayidx136 ; <float> [#uses=1]
  %tmp138 = load float* %divisionWidth            ; <float> [#uses=1]
  %add139 = fadd float %tmp137, %tmp138           ; <float> [#uses=1]
  %tmp140 = load i32* %upperId                    ; <i32> [#uses=1]
  %tmp141 = load float addrspace(1)** %newEigenIntervals.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx142 = getelementptr inbounds float addrspace(1)* %tmp141, i32 %tmp140 ; <float addrspace(1)*> [#uses=1]
  store float %add139, float addrspace(1)* %arrayidx142
  br label %if.end143

if.end143:                                        ; preds = %if.else105, %if.end104
  ret void
}
