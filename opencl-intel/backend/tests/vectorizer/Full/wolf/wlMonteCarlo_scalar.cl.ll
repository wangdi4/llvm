; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlMonteCarlo_scalar.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%0 = type { float, float, float, float, float }
%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type { float, float }

@opencl_wlMonteCarloKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlMonteCarloKernel_parameters = appending global [109 x i8] c"TOptionValue __attribute__((address_space(1))) *, TOptionData __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[109 x i8]*> [#uses=1]
@opencl_wlMonteCarloKernel_PrecalculatedSamples_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlMonteCarloKernel_PrecalculatedSamples_parameters = appending global [152 x i8] c"TOptionValue __attribute__((address_space(1))) *, TOptionData __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[152 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct.anon addrspace(1)*, %0 addrspace(1)*, i32)* @wlMonteCarloKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlMonteCarloKernel_locals to i8*), i8* getelementptr inbounds ([109 x i8]* @opencl_wlMonteCarloKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct.anon addrspace(1)*, %0 addrspace(1)*, float addrspace(1)*, i32)* @wlMonteCarloKernel_PrecalculatedSamples to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlMonteCarloKernel_PrecalculatedSamples_locals to i8*), i8* getelementptr inbounds ([152 x i8]* @opencl_wlMonteCarloKernel_PrecalculatedSamples_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @NormalDistributionBatch1(float* %pDist, i32 %i, i32 %j, i32 %pathN) nounwind {
entry:
  %pDist.addr = alloca float*, align 4            ; <float**> [#uses=2]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %pathNinv = alloca float, align 4               ; <float*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %c1 = alloca float, align 4                     ; <float*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=2]
  %c4 = alloca float, align 4                     ; <float*> [#uses=2]
  %c5 = alloca float, align 4                     ; <float*> [#uses=2]
  %c6 = alloca float, align 4                     ; <float*> [#uses=2]
  %c7 = alloca float, align 4                     ; <float*> [#uses=2]
  %c8 = alloca float, align 4                     ; <float*> [#uses=2]
  %c9 = alloca float, align 4                     ; <float*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=5]
  %p = alloca float, align 4                      ; <float*> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=0]
  %z = alloca float, align 4                      ; <float*> [#uses=11]
  store float* %pDist, float** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  %tmp = load i32* %pathN.addr                    ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %conv = uitofp i32 %add to float                ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %conv          ; <float> [#uses=1]
  store float %div, float* %pathNinv
  store i32 0, i32* %index
  store float 0x3FD59932C0000000, float* %c1
  store float 0x3FEF3CC6C0000000, float* %c2
  store float 0x3FC4950720000000, float* %c3
  store float 0x3F9C4EAD80000000, float* %c4
  store float 0x3F6F7643E0000000, float* %c5
  store float 0x3F39E62EA0000000, float* %c6
  store float 0x3F00DEB200000000, float* %c7
  store float 0x3E9361D580000000, float* %c8
  store float 0x3E9A93C500000000, float* %c9
  %tmp12 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp12, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp13 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp14 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp15 = icmp ule i32 %tmp13, %tmp14            ; <i1> [#uses=1]
  br i1 %cmp15, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp20 = load i32* %pos                         ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp20, 1                  ; <i32> [#uses=1]
  %conv22 = sitofp i32 %add21 to float            ; <float> [#uses=1]
  %tmp23 = load float* %pathNinv                  ; <float> [#uses=1]
  %mul = fmul float %conv22, %tmp23               ; <float> [#uses=1]
  store float %mul, float* %p
  %tmp24 = load float* %p                         ; <float> [#uses=1]
  %call = call float @_Z3logf(float %tmp24)       ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %call         ; <float> [#uses=1]
  %call25 = call float @_Z3logf(float %neg)       ; <float> [#uses=1]
  store float %call25, float* %z
  %tmp26 = load float* %c1                        ; <float> [#uses=1]
  %tmp27 = load float* %z                         ; <float> [#uses=1]
  %tmp28 = load float* %c2                        ; <float> [#uses=1]
  %tmp29 = load float* %z                         ; <float> [#uses=1]
  %tmp30 = load float* %c3                        ; <float> [#uses=1]
  %tmp31 = load float* %z                         ; <float> [#uses=1]
  %tmp32 = load float* %c4                        ; <float> [#uses=1]
  %tmp33 = load float* %z                         ; <float> [#uses=1]
  %tmp34 = load float* %c5                        ; <float> [#uses=1]
  %tmp35 = load float* %z                         ; <float> [#uses=1]
  %tmp36 = load float* %c6                        ; <float> [#uses=1]
  %tmp37 = load float* %z                         ; <float> [#uses=1]
  %tmp38 = load float* %c7                        ; <float> [#uses=1]
  %tmp39 = load float* %z                         ; <float> [#uses=1]
  %tmp40 = load float* %c8                        ; <float> [#uses=1]
  %tmp41 = load float* %z                         ; <float> [#uses=1]
  %tmp42 = load float* %c9                        ; <float> [#uses=1]
  %mul43 = fmul float %tmp41, %tmp42              ; <float> [#uses=1]
  %add44 = fadd float %tmp40, %mul43              ; <float> [#uses=1]
  %mul45 = fmul float %tmp39, %add44              ; <float> [#uses=1]
  %add46 = fadd float %tmp38, %mul45              ; <float> [#uses=1]
  %mul47 = fmul float %tmp37, %add46              ; <float> [#uses=1]
  %add48 = fadd float %tmp36, %mul47              ; <float> [#uses=1]
  %mul49 = fmul float %tmp35, %add48              ; <float> [#uses=1]
  %add50 = fadd float %tmp34, %mul49              ; <float> [#uses=1]
  %mul51 = fmul float %tmp33, %add50              ; <float> [#uses=1]
  %add52 = fadd float %tmp32, %mul51              ; <float> [#uses=1]
  %mul53 = fmul float %tmp31, %add52              ; <float> [#uses=1]
  %add54 = fadd float %tmp30, %mul53              ; <float> [#uses=1]
  %mul55 = fmul float %tmp29, %add54              ; <float> [#uses=1]
  %add56 = fadd float %tmp28, %mul55              ; <float> [#uses=1]
  %mul57 = fmul float %tmp27, %add56              ; <float> [#uses=1]
  %add58 = fadd float %tmp26, %mul57              ; <float> [#uses=1]
  store float %add58, float* %z
  %tmp59 = load float* %z                         ; <float> [#uses=1]
  %neg60 = fsub float -0.000000e+000, %tmp59      ; <float> [#uses=1]
  %tmp61 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp61, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp62 = load float** %pDist.addr               ; <float*> [#uses=1]
  %arrayidx = getelementptr inbounds float* %tmp62, i32 %tmp61 ; <float*> [#uses=1]
  store float %neg60, float* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp63 = load i32* %pos                         ; <i32> [#uses=1]
  %add64 = add nsw i32 %tmp63, 1                  ; <i32> [#uses=1]
  store i32 %add64, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare float @_Z3logf(float)

; CHECK: ret
define void @NormalDistributionBatch2(float* %pDist, i32 %i, i32 %j, i32 %pathN) nounwind {
entry:
  %pDist.addr = alloca float*, align 4            ; <float**> [#uses=2]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %pathNinv = alloca float, align 4               ; <float*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %a1 = alloca float, align 4                     ; <float*> [#uses=2]
  %a2 = alloca float, align 4                     ; <float*> [#uses=2]
  %a3 = alloca float, align 4                     ; <float*> [#uses=2]
  %a4 = alloca float, align 4                     ; <float*> [#uses=2]
  %b1 = alloca float, align 4                     ; <float*> [#uses=2]
  %b2 = alloca float, align 4                     ; <float*> [#uses=2]
  %b3 = alloca float, align 4                     ; <float*> [#uses=2]
  %b4 = alloca float, align 4                     ; <float*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=5]
  %p = alloca float, align 4                      ; <float*> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=4]
  %z = alloca float, align 4                      ; <float*> [#uses=8]
  store float* %pDist, float** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  %tmp = load i32* %pathN.addr                    ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %conv = uitofp i32 %add to float                ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %conv          ; <float> [#uses=1]
  store float %div, float* %pathNinv
  store i32 0, i32* %index
  store float 0x40040D9320000000, float* %a1
  store float 0xC0329D70A0000000, float* %a2
  store float 0x4044B212C0000000, float* %a3
  store float 0xC03970E960000000, float* %a4
  store float 0xC020F27000000000, float* %b1
  store float 0x40371557A0000000, float* %b2
  store float 0xC0350FEF00000000, float* %b3
  store float 0x40090BF020000000, float* %b4
  %tmp11 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp11, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp12 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp13 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp14 = icmp ule i32 %tmp12, %tmp13            ; <i1> [#uses=1]
  br i1 %cmp14, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp19 = load i32* %pos                         ; <i32> [#uses=1]
  %add20 = add nsw i32 %tmp19, 1                  ; <i32> [#uses=1]
  %conv21 = sitofp i32 %add20 to float            ; <float> [#uses=1]
  %tmp22 = load float* %pathNinv                  ; <float> [#uses=1]
  %mul = fmul float %conv21, %tmp22               ; <float> [#uses=1]
  store float %mul, float* %p
  %tmp23 = load float* %p                         ; <float> [#uses=1]
  %sub = fsub float %tmp23, 5.000000e-001         ; <float> [#uses=1]
  store float %sub, float* %y
  %tmp24 = load float* %y                         ; <float> [#uses=1]
  %tmp25 = load float* %y                         ; <float> [#uses=1]
  %mul26 = fmul float %tmp24, %tmp25              ; <float> [#uses=1]
  store float %mul26, float* %z
  %tmp27 = load float* %y                         ; <float> [#uses=1]
  %tmp28 = load float* %a4                        ; <float> [#uses=1]
  %tmp29 = load float* %z                         ; <float> [#uses=1]
  %mul30 = fmul float %tmp28, %tmp29              ; <float> [#uses=1]
  %tmp31 = load float* %a3                        ; <float> [#uses=1]
  %add32 = fadd float %mul30, %tmp31              ; <float> [#uses=1]
  %tmp33 = load float* %z                         ; <float> [#uses=1]
  %mul34 = fmul float %add32, %tmp33              ; <float> [#uses=1]
  %tmp35 = load float* %a2                        ; <float> [#uses=1]
  %add36 = fadd float %mul34, %tmp35              ; <float> [#uses=1]
  %tmp37 = load float* %z                         ; <float> [#uses=1]
  %mul38 = fmul float %add36, %tmp37              ; <float> [#uses=1]
  %tmp39 = load float* %a1                        ; <float> [#uses=1]
  %add40 = fadd float %mul38, %tmp39              ; <float> [#uses=1]
  %mul41 = fmul float %tmp27, %add40              ; <float> [#uses=1]
  %conv42 = fpext float %mul41 to double          ; <double> [#uses=1]
  %tmp43 = load float* %b4                        ; <float> [#uses=1]
  %tmp44 = load float* %z                         ; <float> [#uses=1]
  %mul45 = fmul float %tmp43, %tmp44              ; <float> [#uses=1]
  %tmp46 = load float* %b3                        ; <float> [#uses=1]
  %add47 = fadd float %mul45, %tmp46              ; <float> [#uses=1]
  %tmp48 = load float* %z                         ; <float> [#uses=1]
  %mul49 = fmul float %add47, %tmp48              ; <float> [#uses=1]
  %tmp50 = load float* %b2                        ; <float> [#uses=1]
  %add51 = fadd float %mul49, %tmp50              ; <float> [#uses=1]
  %tmp52 = load float* %z                         ; <float> [#uses=1]
  %mul53 = fmul float %add51, %tmp52              ; <float> [#uses=1]
  %tmp54 = load float* %b1                        ; <float> [#uses=1]
  %add55 = fadd float %mul53, %tmp54              ; <float> [#uses=1]
  %tmp56 = load float* %z                         ; <float> [#uses=1]
  %mul57 = fmul float %add55, %tmp56              ; <float> [#uses=1]
  %conv58 = fpext float %mul57 to double          ; <double> [#uses=1]
  %add59 = fadd double %conv58, 1.000000e+000     ; <double> [#uses=3]
  %cmp60 = fcmp oeq double 0.000000e+000, %add59  ; <i1> [#uses=1]
  %sel61 = select i1 %cmp60, double 1.000000e+000, double %add59 ; <double> [#uses=0]
  %div62 = fdiv double %conv42, %add59            ; <double> [#uses=1]
  %conv63 = fptrunc double %div62 to float        ; <float> [#uses=1]
  %tmp64 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp64, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp65 = load float** %pDist.addr               ; <float*> [#uses=1]
  %arrayidx = getelementptr inbounds float* %tmp65, i32 %tmp64 ; <float*> [#uses=1]
  store float %conv63, float* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp66 = load i32* %pos                         ; <i32> [#uses=1]
  %add67 = add nsw i32 %tmp66, 1                  ; <i32> [#uses=1]
  store i32 %add67, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @NormalDistributionBatch3(float* %pDist, i32 %i, i32 %j, i32 %pathN) nounwind {
entry:
  %pDist.addr = alloca float*, align 4            ; <float**> [#uses=2]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %pathNinv = alloca float, align 4               ; <float*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %c1 = alloca float, align 4                     ; <float*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=2]
  %c4 = alloca float, align 4                     ; <float*> [#uses=2]
  %c5 = alloca float, align 4                     ; <float*> [#uses=2]
  %c6 = alloca float, align 4                     ; <float*> [#uses=2]
  %c7 = alloca float, align 4                     ; <float*> [#uses=2]
  %c8 = alloca float, align 4                     ; <float*> [#uses=2]
  %c9 = alloca float, align 4                     ; <float*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=5]
  %p = alloca float, align 4                      ; <float*> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=0]
  %z = alloca float, align 4                      ; <float*> [#uses=9]
  store float* %pDist, float** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  %tmp = load i32* %pathN.addr                    ; <i32> [#uses=1]
  %add = add i32 %tmp, 1                          ; <i32> [#uses=1]
  %conv = uitofp i32 %add to float                ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %conv          ; <float> [#uses=1]
  store float %div, float* %pathNinv
  store i32 0, i32* %index
  store float 0x3FD59932C0000000, float* %c1
  store float 0x3FEF3CC6C0000000, float* %c2
  store float 0x3FC4950720000000, float* %c3
  store float 0x3F9C4EAD80000000, float* %c4
  store float 0x3F6F7643E0000000, float* %c5
  store float 0x3F39E62EA0000000, float* %c6
  store float 0x3F00DEB200000000, float* %c7
  store float 0x3E9361D580000000, float* %c8
  store float 0x3E9A93C500000000, float* %c9
  %tmp12 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp12, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp13 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp14 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp15 = icmp ule i32 %tmp13, %tmp14            ; <i1> [#uses=1]
  br i1 %cmp15, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp20 = load i32* %pos                         ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp20, 1                  ; <i32> [#uses=1]
  %conv22 = sitofp i32 %add21 to float            ; <float> [#uses=1]
  %tmp23 = load float* %pathNinv                  ; <float> [#uses=1]
  %mul = fmul float %conv22, %tmp23               ; <float> [#uses=1]
  store float %mul, float* %p
  %tmp24 = load float* %p                         ; <float> [#uses=1]
  %sub = fsub float 1.000000e+000, %tmp24         ; <float> [#uses=1]
  %call = call float @_Z3logf(float %sub)         ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %call         ; <float> [#uses=1]
  %call25 = call float @_Z3logf(float %neg)       ; <float> [#uses=1]
  store float %call25, float* %z
  %tmp26 = load float* %c1                        ; <float> [#uses=1]
  %tmp27 = load float* %z                         ; <float> [#uses=1]
  %tmp28 = load float* %c2                        ; <float> [#uses=1]
  %tmp29 = load float* %z                         ; <float> [#uses=1]
  %tmp30 = load float* %c3                        ; <float> [#uses=1]
  %tmp31 = load float* %z                         ; <float> [#uses=1]
  %tmp32 = load float* %c4                        ; <float> [#uses=1]
  %tmp33 = load float* %z                         ; <float> [#uses=1]
  %tmp34 = load float* %c5                        ; <float> [#uses=1]
  %tmp35 = load float* %z                         ; <float> [#uses=1]
  %tmp36 = load float* %c6                        ; <float> [#uses=1]
  %tmp37 = load float* %z                         ; <float> [#uses=1]
  %tmp38 = load float* %c7                        ; <float> [#uses=1]
  %tmp39 = load float* %z                         ; <float> [#uses=1]
  %tmp40 = load float* %c8                        ; <float> [#uses=1]
  %tmp41 = load float* %z                         ; <float> [#uses=1]
  %tmp42 = load float* %c9                        ; <float> [#uses=1]
  %mul43 = fmul float %tmp41, %tmp42              ; <float> [#uses=1]
  %add44 = fadd float %tmp40, %mul43              ; <float> [#uses=1]
  %mul45 = fmul float %tmp39, %add44              ; <float> [#uses=1]
  %add46 = fadd float %tmp38, %mul45              ; <float> [#uses=1]
  %mul47 = fmul float %tmp37, %add46              ; <float> [#uses=1]
  %add48 = fadd float %tmp36, %mul47              ; <float> [#uses=1]
  %mul49 = fmul float %tmp35, %add48              ; <float> [#uses=1]
  %add50 = fadd float %tmp34, %mul49              ; <float> [#uses=1]
  %mul51 = fmul float %tmp33, %add50              ; <float> [#uses=1]
  %add52 = fadd float %tmp32, %mul51              ; <float> [#uses=1]
  %mul53 = fmul float %tmp31, %add52              ; <float> [#uses=1]
  %add54 = fadd float %tmp30, %mul53              ; <float> [#uses=1]
  %mul55 = fmul float %tmp29, %add54              ; <float> [#uses=1]
  %add56 = fadd float %tmp28, %mul55              ; <float> [#uses=1]
  %mul57 = fmul float %tmp27, %add56              ; <float> [#uses=1]
  %add58 = fadd float %tmp26, %mul57              ; <float> [#uses=1]
  %tmp59 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp59, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp60 = load float** %pDist.addr               ; <float*> [#uses=1]
  %arrayidx = getelementptr inbounds float* %tmp60, i32 %tmp59 ; <float*> [#uses=1]
  store float %add58, float* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp61 = load i32* %pos                         ; <i32> [#uses=1]
  %add62 = add nsw i32 %tmp61, 1                  ; <i32> [#uses=1]
  store i32 %add62, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @wlMonteCarloKernel(%struct.anon addrspace(1)* %pOptionValue, %0 addrspace(1)* %pOptionData, i32 %pathN) nounwind {
entry:
  %pOptionValue.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=3]
  %pOptionData.addr = alloca %0 addrspace(1)*, align 4 ; <%0 addrspace(1)**> [#uses=6]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=16]
  %sample = alloca [512 x float], align 4         ; <[512 x float]*> [#uses=10]
  %temp = alloca [512 x float], align 4           ; <[512 x float]*> [#uses=12]
  %optionid = alloca i32, align 4                 ; <i32*> [#uses=8]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %S = alloca float, align 4                      ; <float*> [#uses=7]
  %X = alloca float, align 4                      ; <float*> [#uses=7]
  %V = alloca float, align 4                      ; <float*> [#uses=4]
  %MuByT = alloca float, align 4                  ; <float*> [#uses=7]
  %VBySqrtT = alloca float, align 4               ; <float*> [#uses=7]
  %sum = alloca float, align 4                    ; <float*> [#uses=14]
  %sum2 = alloca float, align 4                   ; <float*> [#uses=14]
  %boundary12 = alloca i32, align 4               ; <i32*> [#uses=4]
  %boundary23 = alloca i32, align 4               ; <i32*> [#uses=4]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=32]
  %n = alloca i32, align 4                        ; <i32*> [#uses=18]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %i91 = alloca i32, align 4                      ; <i32*> [#uses=5]
  %callValue = alloca float, align 4              ; <float*> [#uses=6]
  %i136 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i160 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue168 = alloca float, align 4           ; <float*> [#uses=6]
  %pathNinv = alloca float, align 4               ; <float*> [#uses=2]
  %a1 = alloca float, align 4                     ; <float*> [#uses=2]
  %a2 = alloca float, align 4                     ; <float*> [#uses=2]
  %a3 = alloca float, align 4                     ; <float*> [#uses=2]
  %a4 = alloca float, align 4                     ; <float*> [#uses=2]
  %b1 = alloca float, align 4                     ; <float*> [#uses=2]
  %b2 = alloca float, align 4                     ; <float*> [#uses=2]
  %b3 = alloca float, align 4                     ; <float*> [#uses=2]
  %b4 = alloca float, align 4                     ; <float*> [#uses=2]
  %i218 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %p = alloca float, align 4                      ; <float*> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=4]
  %z = alloca float, align 4                      ; <float*> [#uses=8]
  %sample240 = alloca float, align 4              ; <float*> [#uses=2]
  %i288 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue295 = alloca float, align 4           ; <float*> [#uses=6]
  %i336 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i360 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue368 = alloca float, align 4           ; <float*> [#uses=6]
  %i411 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i433 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue440 = alloca float, align 4           ; <float*> [#uses=6]
  %i481 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i505 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue513 = alloca float, align 4           ; <float*> [#uses=6]
  %totalsum = alloca float, align 4               ; <float*> [#uses=4]
  %totalsum2 = alloca float, align 4              ; <float*> [#uses=2]
  %ExpRT = alloca float, align 4                  ; <float*> [#uses=3]
  %stdDev = alloca float, align 4                 ; <float*> [#uses=2]
  store %struct.anon addrspace(1)* %pOptionValue, %struct.anon addrspace(1)** %pOptionValue.addr
  store %0 addrspace(1)* %pOptionData, %0 addrspace(1)** %pOptionData.addr
  store i32 %pathN, i32* %pathN.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %optionid
  %tmp = load i32* %optionid                      ; <i32> [#uses=1]
  %tmp1 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds %0 addrspace(1)* %tmp1, i32 %tmp ; <%0 addrspace(1)*> [#uses=1]
  %tmp2 = getelementptr inbounds %0 addrspace(1)* %arrayidx, i32 0, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp3 = load float addrspace(1)* %tmp2          ; <float> [#uses=1]
  store float %tmp3, float* %T
  %tmp5 = load i32* %optionid                     ; <i32> [#uses=1]
  %tmp6 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx7 = getelementptr inbounds %0 addrspace(1)* %tmp6, i32 %tmp5 ; <%0 addrspace(1)*> [#uses=1]
  %tmp8 = getelementptr inbounds %0 addrspace(1)* %arrayidx7, i32 0, i32 3 ; <float addrspace(1)*> [#uses=1]
  %tmp9 = load float addrspace(1)* %tmp8          ; <float> [#uses=1]
  store float %tmp9, float* %R
  %tmp11 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp12 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx13 = getelementptr inbounds %0 addrspace(1)* %tmp12, i32 %tmp11 ; <%0 addrspace(1)*> [#uses=1]
  %tmp14 = getelementptr inbounds %0 addrspace(1)* %arrayidx13, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load float addrspace(1)* %tmp14        ; <float> [#uses=1]
  store float %tmp15, float* %S
  %tmp17 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp18 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds %0 addrspace(1)* %tmp18, i32 %tmp17 ; <%0 addrspace(1)*> [#uses=1]
  %tmp20 = getelementptr inbounds %0 addrspace(1)* %arrayidx19, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp21 = load float addrspace(1)* %tmp20        ; <float> [#uses=1]
  store float %tmp21, float* %X
  %tmp23 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp24 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds %0 addrspace(1)* %tmp24, i32 %tmp23 ; <%0 addrspace(1)*> [#uses=1]
  %tmp26 = getelementptr inbounds %0 addrspace(1)* %arrayidx25, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  %tmp27 = load float addrspace(1)* %tmp26        ; <float> [#uses=1]
  store float %tmp27, float* %V
  %tmp29 = load float* %R                         ; <float> [#uses=1]
  %conv = fpext float %tmp29 to double            ; <double> [#uses=1]
  %tmp30 = load float* %V                         ; <float> [#uses=1]
  %conv31 = fpext float %tmp30 to double          ; <double> [#uses=1]
  %mul = fmul double 5.000000e-001, %conv31       ; <double> [#uses=1]
  %tmp32 = load float* %V                         ; <float> [#uses=1]
  %conv33 = fpext float %tmp32 to double          ; <double> [#uses=1]
  %mul34 = fmul double %mul, %conv33              ; <double> [#uses=1]
  %sub = fsub double %conv, %mul34                ; <double> [#uses=1]
  %tmp35 = load float* %T                         ; <float> [#uses=1]
  %conv36 = fpext float %tmp35 to double          ; <double> [#uses=1]
  %mul37 = fmul double %sub, %conv36              ; <double> [#uses=1]
  %conv38 = fptrunc double %mul37 to float        ; <float> [#uses=1]
  store float %conv38, float* %MuByT
  %tmp40 = load float* %V                         ; <float> [#uses=1]
  %tmp41 = load float* %T                         ; <float> [#uses=1]
  %call42 = call float @_Z4sqrtf(float %tmp41)    ; <float> [#uses=1]
  %mul43 = fmul float %tmp40, %call42             ; <float> [#uses=1]
  store float %mul43, float* %VBySqrtT
  store float 0.000000e+000, float* %sum
  store float 0.000000e+000, float* %sum2
  %tmp47 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv48 = sitofp i32 %tmp47 to double           ; <double> [#uses=1]
  %mul49 = fmul double 8.000000e-002, %conv48     ; <double> [#uses=1]
  %conv50 = fptosi double %mul49 to i32           ; <i32> [#uses=1]
  %div = sdiv i32 %conv50, 1                      ; <i32> [#uses=1]
  %mul51 = mul i32 1, %div                        ; <i32> [#uses=1]
  store i32 %mul51, i32* %boundary12
  %tmp53 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv54 = sitofp i32 %tmp53 to double           ; <double> [#uses=1]
  %mul55 = fmul double 9.200000e-001, %conv54     ; <double> [#uses=1]
  %conv56 = fptosi double %mul55 to i32           ; <i32> [#uses=1]
  %div57 = sdiv i32 %conv56, 1                    ; <i32> [#uses=1]
  %mul58 = mul i32 1, %div57                      ; <i32> [#uses=1]
  store i32 %mul58, i32* %boundary23
  store i32 0, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp61 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp62 = load i32* %boundary12                  ; <i32> [#uses=1]
  %sub63 = sub i32 %tmp62, 512                    ; <i32> [#uses=1]
  %add = add nsw i32 %sub63, 1                    ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp61, %add                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end121

for.body:                                         ; preds = %for.cond
  %arraydecay = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %tmp65 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp66 = load i32* %pos                         ; <i32> [#uses=1]
  %add67 = add nsw i32 %tmp66, 512                ; <i32> [#uses=1]
  %sub68 = sub i32 %add67, 1                      ; <i32> [#uses=1]
  %tmp69 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  call void @NormalDistributionBatch1(float* %arraydecay, i32 %tmp65, i32 %sub68, i32 %tmp69)
  store i32 0, i32* %i
  br label %for.cond71

for.cond71:                                       ; preds = %for.body75, %for.body
  %tmp72 = load i32* %i                           ; <i32> [#uses=1]
  %cmp73 = icmp slt i32 %tmp72, 512               ; <i1> [#uses=1]
  br i1 %cmp73, label %for.body75, label %for.end

for.body75:                                       ; preds = %for.cond71
  %tmp76 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay77 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float* %arraydecay77, i32 %tmp76 ; <float*> [#uses=1]
  %tmp79 = load float* %arrayidx78                ; <float> [#uses=1]
  %tmp80 = load float* %VBySqrtT                  ; <float> [#uses=1]
  %mul81 = fmul float %tmp79, %tmp80              ; <float> [#uses=1]
  %tmp82 = load float* %MuByT                     ; <float> [#uses=1]
  %add83 = fadd float %mul81, %tmp82              ; <float> [#uses=1]
  %call84 = call float @_Z3expf(float %add83)     ; <float> [#uses=1]
  %tmp85 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay86 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx87 = getelementptr inbounds float* %arraydecay86, i32 %tmp85 ; <float*> [#uses=1]
  store float %call84, float* %arrayidx87
  %tmp88 = load i32* %i                           ; <i32> [#uses=1]
  %add89 = add nsw i32 %tmp88, 1                  ; <i32> [#uses=1]
  store i32 %add89, i32* %i
  br label %for.cond71

for.end:                                          ; preds = %for.cond71
  store i32 0, i32* %i91
  br label %for.cond92

for.cond92:                                       ; preds = %for.body96, %for.end
  %tmp93 = load i32* %i91                         ; <i32> [#uses=1]
  %cmp94 = icmp slt i32 %tmp93, 512               ; <i1> [#uses=1]
  br i1 %cmp94, label %for.body96, label %for.end118

for.body96:                                       ; preds = %for.cond92
  %tmp98 = load float* %S                         ; <float> [#uses=1]
  %tmp99 = load i32* %i91                         ; <i32> [#uses=1]
  %arraydecay100 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx101 = getelementptr inbounds float* %arraydecay100, i32 %tmp99 ; <float*> [#uses=1]
  %tmp102 = load float* %arrayidx101              ; <float> [#uses=1]
  %mul103 = fmul float %tmp98, %tmp102            ; <float> [#uses=1]
  %tmp104 = load float* %X                        ; <float> [#uses=1]
  %sub105 = fsub float %mul103, %tmp104           ; <float> [#uses=1]
  store float %sub105, float* %callValue
  %tmp106 = load float* %callValue                ; <float> [#uses=1]
  %call107 = call float @_Z3maxff(float %tmp106, float 0.000000e+000) ; <float> [#uses=1]
  store float %call107, float* %callValue
  %tmp108 = load float* %callValue                ; <float> [#uses=1]
  %tmp109 = load float* %sum                      ; <float> [#uses=1]
  %add110 = fadd float %tmp109, %tmp108           ; <float> [#uses=1]
  store float %add110, float* %sum
  %tmp111 = load float* %callValue                ; <float> [#uses=1]
  %tmp112 = load float* %callValue                ; <float> [#uses=1]
  %mul113 = fmul float %tmp111, %tmp112           ; <float> [#uses=1]
  %tmp114 = load float* %sum2                     ; <float> [#uses=1]
  %add115 = fadd float %tmp114, %mul113           ; <float> [#uses=1]
  store float %add115, float* %sum2
  %tmp116 = load i32* %i91                        ; <i32> [#uses=1]
  %add117 = add nsw i32 %tmp116, 1                ; <i32> [#uses=1]
  store i32 %add117, i32* %i91
  br label %for.cond92

for.end118:                                       ; preds = %for.cond92
  br label %for.inc

for.inc:                                          ; preds = %for.end118
  %tmp119 = load i32* %pos                        ; <i32> [#uses=1]
  %add120 = add nsw i32 %tmp119, 512              ; <i32> [#uses=1]
  store i32 %add120, i32* %pos
  br label %for.cond

for.end121:                                       ; preds = %for.cond
  %tmp122 = load i32* %boundary12                 ; <i32> [#uses=1]
  %tmp123 = load i32* %pos                        ; <i32> [#uses=1]
  %sub124 = sub i32 %tmp122, %tmp123              ; <i32> [#uses=1]
  store i32 %sub124, i32* %n
  %tmp125 = load i32* %n                          ; <i32> [#uses=1]
  %cmp126 = icmp sgt i32 %tmp125, 0               ; <i1> [#uses=1]
  br i1 %cmp126, label %if.then, label %if.end

if.then:                                          ; preds = %for.end121
  %arraydecay128 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %tmp129 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp130 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp131 = load i32* %n                          ; <i32> [#uses=1]
  %add132 = add nsw i32 %tmp130, %tmp131          ; <i32> [#uses=1]
  %sub133 = sub i32 %add132, 1                    ; <i32> [#uses=1]
  %tmp134 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  call void @NormalDistributionBatch1(float* %arraydecay128, i32 %tmp129, i32 %sub133, i32 %tmp134)
  store i32 0, i32* %i136
  br label %for.cond137

for.cond137:                                      ; preds = %for.inc155, %if.then
  %tmp138 = load i32* %i136                       ; <i32> [#uses=1]
  %tmp139 = load i32* %n                          ; <i32> [#uses=1]
  %cmp140 = icmp slt i32 %tmp138, %tmp139         ; <i1> [#uses=1]
  br i1 %cmp140, label %for.body142, label %for.end158

for.body142:                                      ; preds = %for.cond137
  %tmp143 = load i32* %i136                       ; <i32> [#uses=1]
  %arraydecay144 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx145 = getelementptr inbounds float* %arraydecay144, i32 %tmp143 ; <float*> [#uses=1]
  %tmp146 = load float* %arrayidx145              ; <float> [#uses=1]
  %tmp147 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul148 = fmul float %tmp146, %tmp147           ; <float> [#uses=1]
  %tmp149 = load float* %MuByT                    ; <float> [#uses=1]
  %add150 = fadd float %mul148, %tmp149           ; <float> [#uses=1]
  %call151 = call float @_Z3expf(float %add150)   ; <float> [#uses=1]
  %tmp152 = load i32* %i136                       ; <i32> [#uses=1]
  %arraydecay153 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx154 = getelementptr inbounds float* %arraydecay153, i32 %tmp152 ; <float*> [#uses=1]
  store float %call151, float* %arrayidx154
  br label %for.inc155

for.inc155:                                       ; preds = %for.body142
  %tmp156 = load i32* %i136                       ; <i32> [#uses=1]
  %add157 = add nsw i32 %tmp156, 1                ; <i32> [#uses=1]
  store i32 %add157, i32* %i136
  br label %for.cond137

for.end158:                                       ; preds = %for.cond137
  store i32 0, i32* %i160
  br label %for.cond161

for.cond161:                                      ; preds = %for.inc187, %for.end158
  %tmp162 = load i32* %i160                       ; <i32> [#uses=1]
  %tmp163 = load i32* %n                          ; <i32> [#uses=1]
  %cmp164 = icmp slt i32 %tmp162, %tmp163         ; <i1> [#uses=1]
  br i1 %cmp164, label %for.body166, label %for.end190

for.body166:                                      ; preds = %for.cond161
  %tmp169 = load float* %S                        ; <float> [#uses=1]
  %tmp170 = load i32* %i160                       ; <i32> [#uses=1]
  %arraydecay171 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx172 = getelementptr inbounds float* %arraydecay171, i32 %tmp170 ; <float*> [#uses=1]
  %tmp173 = load float* %arrayidx172              ; <float> [#uses=1]
  %mul174 = fmul float %tmp169, %tmp173           ; <float> [#uses=1]
  %tmp175 = load float* %X                        ; <float> [#uses=1]
  %sub176 = fsub float %mul174, %tmp175           ; <float> [#uses=1]
  store float %sub176, float* %callValue168
  %tmp177 = load float* %callValue168             ; <float> [#uses=1]
  %call178 = call float @_Z3maxff(float %tmp177, float 0.000000e+000) ; <float> [#uses=1]
  store float %call178, float* %callValue168
  %tmp179 = load float* %callValue168             ; <float> [#uses=1]
  %tmp180 = load float* %sum                      ; <float> [#uses=1]
  %add181 = fadd float %tmp180, %tmp179           ; <float> [#uses=1]
  store float %add181, float* %sum
  %tmp182 = load float* %callValue168             ; <float> [#uses=1]
  %tmp183 = load float* %callValue168             ; <float> [#uses=1]
  %mul184 = fmul float %tmp182, %tmp183           ; <float> [#uses=1]
  %tmp185 = load float* %sum2                     ; <float> [#uses=1]
  %add186 = fadd float %tmp185, %mul184           ; <float> [#uses=1]
  store float %add186, float* %sum2
  br label %for.inc187

for.inc187:                                       ; preds = %for.body166
  %tmp188 = load i32* %i160                       ; <i32> [#uses=1]
  %add189 = add nsw i32 %tmp188, 1                ; <i32> [#uses=1]
  store i32 %add189, i32* %i160
  br label %for.cond161

for.end190:                                       ; preds = %for.cond161
  %tmp191 = load i32* %n                          ; <i32> [#uses=1]
  %tmp192 = load i32* %pos                        ; <i32> [#uses=1]
  %add193 = add nsw i32 %tmp192, %tmp191          ; <i32> [#uses=1]
  store i32 %add193, i32* %pos
  br label %if.end

if.end:                                           ; preds = %for.end190, %for.end121
  %tmp194 = load i32* %boundary12                 ; <i32> [#uses=1]
  store i32 %tmp194, i32* %pos
  br label %for.cond195

for.cond195:                                      ; preds = %for.inc317, %if.end
  %tmp196 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp197 = load i32* %boundary23                 ; <i32> [#uses=1]
  %sub198 = sub i32 %tmp197, 512                  ; <i32> [#uses=1]
  %add199 = add nsw i32 %sub198, 1                ; <i32> [#uses=1]
  %cmp200 = icmp slt i32 %tmp196, %add199         ; <i1> [#uses=1]
  br i1 %cmp200, label %for.body202, label %for.end320

for.body202:                                      ; preds = %for.cond195
  %tmp204 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %add205 = add nsw i32 %tmp204, 1                ; <i32> [#uses=1]
  %conv206 = sitofp i32 %add205 to float          ; <float> [#uses=3]
  %cmp207 = fcmp oeq float 0.000000e+000, %conv206 ; <i1> [#uses=1]
  %sel = select i1 %cmp207, float 1.000000e+000, float %conv206 ; <float> [#uses=0]
  %div208 = fdiv float 1.000000e+000, %conv206    ; <float> [#uses=1]
  store float %div208, float* %pathNinv
  store float 0x40040D9320000000, float* %a1
  store float 0xC0329D70A0000000, float* %a2
  store float 0x4044B212C0000000, float* %a3
  store float 0xC03970E960000000, float* %a4
  store float 0xC020F27000000000, float* %b1
  store float 0x40371557A0000000, float* %b2
  store float 0xC0350FEF00000000, float* %b3
  store float 0x40090BF020000000, float* %b4
  store i32 0, i32* %i218
  br label %for.cond219

for.cond219:                                      ; preds = %for.body223, %for.body202
  %tmp220 = load i32* %i218                       ; <i32> [#uses=1]
  %cmp221 = icmp slt i32 %tmp220, 512             ; <i1> [#uses=1]
  br i1 %cmp221, label %for.body223, label %for.end286

for.body223:                                      ; preds = %for.cond219
  %tmp225 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp226 = load i32* %i218                       ; <i32> [#uses=1]
  %add227 = add nsw i32 %tmp225, %tmp226          ; <i32> [#uses=1]
  %add228 = add nsw i32 %add227, 1                ; <i32> [#uses=1]
  %conv229 = sitofp i32 %add228 to float          ; <float> [#uses=1]
  %tmp230 = load float* %pathNinv                 ; <float> [#uses=1]
  %mul231 = fmul float %conv229, %tmp230          ; <float> [#uses=1]
  store float %mul231, float* %p
  %tmp233 = load float* %p                        ; <float> [#uses=1]
  %sub234 = fsub float %tmp233, 5.000000e-001     ; <float> [#uses=1]
  store float %sub234, float* %y
  %tmp236 = load float* %y                        ; <float> [#uses=1]
  %tmp237 = load float* %y                        ; <float> [#uses=1]
  %mul238 = fmul float %tmp236, %tmp237           ; <float> [#uses=1]
  store float %mul238, float* %z
  %tmp241 = load float* %y                        ; <float> [#uses=1]
  %tmp242 = load float* %a4                       ; <float> [#uses=1]
  %tmp243 = load float* %z                        ; <float> [#uses=1]
  %mul244 = fmul float %tmp242, %tmp243           ; <float> [#uses=1]
  %tmp245 = load float* %a3                       ; <float> [#uses=1]
  %add246 = fadd float %mul244, %tmp245           ; <float> [#uses=1]
  %tmp247 = load float* %z                        ; <float> [#uses=1]
  %mul248 = fmul float %add246, %tmp247           ; <float> [#uses=1]
  %tmp249 = load float* %a2                       ; <float> [#uses=1]
  %add250 = fadd float %mul248, %tmp249           ; <float> [#uses=1]
  %tmp251 = load float* %z                        ; <float> [#uses=1]
  %mul252 = fmul float %add250, %tmp251           ; <float> [#uses=1]
  %tmp253 = load float* %a1                       ; <float> [#uses=1]
  %add254 = fadd float %mul252, %tmp253           ; <float> [#uses=1]
  %mul255 = fmul float %tmp241, %add254           ; <float> [#uses=1]
  %tmp256 = load float* %b4                       ; <float> [#uses=1]
  %tmp257 = load float* %z                        ; <float> [#uses=1]
  %mul258 = fmul float %tmp256, %tmp257           ; <float> [#uses=1]
  %tmp259 = load float* %b3                       ; <float> [#uses=1]
  %add260 = fadd float %mul258, %tmp259           ; <float> [#uses=1]
  %tmp261 = load float* %z                        ; <float> [#uses=1]
  %mul262 = fmul float %add260, %tmp261           ; <float> [#uses=1]
  %tmp263 = load float* %b2                       ; <float> [#uses=1]
  %add264 = fadd float %mul262, %tmp263           ; <float> [#uses=1]
  %tmp265 = load float* %z                        ; <float> [#uses=1]
  %mul266 = fmul float %add264, %tmp265           ; <float> [#uses=1]
  %tmp267 = load float* %b1                       ; <float> [#uses=1]
  %add268 = fadd float %mul266, %tmp267           ; <float> [#uses=1]
  %tmp269 = load float* %z                        ; <float> [#uses=1]
  %mul270 = fmul float %add268, %tmp269           ; <float> [#uses=1]
  %add271 = fadd float %mul270, 1.000000e+000     ; <float> [#uses=3]
  %cmp272 = fcmp oeq float 0.000000e+000, %add271 ; <i1> [#uses=1]
  %sel273 = select i1 %cmp272, float 1.000000e+000, float %add271 ; <float> [#uses=0]
  %div274 = fdiv float %mul255, %add271           ; <float> [#uses=1]
  store float %div274, float* %sample240
  %tmp275 = load float* %sample240                ; <float> [#uses=1]
  %tmp276 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul277 = fmul float %tmp275, %tmp276           ; <float> [#uses=1]
  %tmp278 = load float* %MuByT                    ; <float> [#uses=1]
  %add279 = fadd float %mul277, %tmp278           ; <float> [#uses=1]
  %call280 = call float @_Z3expf(float %add279)   ; <float> [#uses=1]
  %tmp281 = load i32* %i218                       ; <i32> [#uses=1]
  %arraydecay282 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx283 = getelementptr inbounds float* %arraydecay282, i32 %tmp281 ; <float*> [#uses=1]
  store float %call280, float* %arrayidx283
  %tmp284 = load i32* %i218                       ; <i32> [#uses=1]
  %add285 = add nsw i32 %tmp284, 1                ; <i32> [#uses=1]
  store i32 %add285, i32* %i218
  br label %for.cond219

for.end286:                                       ; preds = %for.cond219
  store i32 0, i32* %i288
  br label %for.cond289

for.cond289:                                      ; preds = %for.body293, %for.end286
  %tmp290 = load i32* %i288                       ; <i32> [#uses=1]
  %cmp291 = icmp slt i32 %tmp290, 512             ; <i1> [#uses=1]
  br i1 %cmp291, label %for.body293, label %for.end316

for.body293:                                      ; preds = %for.cond289
  %tmp296 = load float* %S                        ; <float> [#uses=1]
  %tmp297 = load i32* %i288                       ; <i32> [#uses=1]
  %arraydecay298 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx299 = getelementptr inbounds float* %arraydecay298, i32 %tmp297 ; <float*> [#uses=1]
  %tmp300 = load float* %arrayidx299              ; <float> [#uses=1]
  %mul301 = fmul float %tmp296, %tmp300           ; <float> [#uses=1]
  %tmp302 = load float* %X                        ; <float> [#uses=1]
  %sub303 = fsub float %mul301, %tmp302           ; <float> [#uses=1]
  store float %sub303, float* %callValue295
  %tmp304 = load float* %callValue295             ; <float> [#uses=1]
  %call305 = call float @_Z3maxff(float %tmp304, float 0.000000e+000) ; <float> [#uses=1]
  store float %call305, float* %callValue295
  %tmp306 = load float* %callValue295             ; <float> [#uses=1]
  %tmp307 = load float* %sum                      ; <float> [#uses=1]
  %add308 = fadd float %tmp307, %tmp306           ; <float> [#uses=1]
  store float %add308, float* %sum
  %tmp309 = load float* %callValue295             ; <float> [#uses=1]
  %tmp310 = load float* %callValue295             ; <float> [#uses=1]
  %mul311 = fmul float %tmp309, %tmp310           ; <float> [#uses=1]
  %tmp312 = load float* %sum2                     ; <float> [#uses=1]
  %add313 = fadd float %tmp312, %mul311           ; <float> [#uses=1]
  store float %add313, float* %sum2
  %tmp314 = load i32* %i288                       ; <i32> [#uses=1]
  %add315 = add nsw i32 %tmp314, 1                ; <i32> [#uses=1]
  store i32 %add315, i32* %i288
  br label %for.cond289

for.end316:                                       ; preds = %for.cond289
  br label %for.inc317

for.inc317:                                       ; preds = %for.end316
  %tmp318 = load i32* %pos                        ; <i32> [#uses=1]
  %add319 = add nsw i32 %tmp318, 512              ; <i32> [#uses=1]
  store i32 %add319, i32* %pos
  br label %for.cond195

for.end320:                                       ; preds = %for.cond195
  %tmp321 = load i32* %boundary23                 ; <i32> [#uses=1]
  %tmp322 = load i32* %pos                        ; <i32> [#uses=1]
  %sub323 = sub i32 %tmp321, %tmp322              ; <i32> [#uses=1]
  store i32 %sub323, i32* %n
  %tmp324 = load i32* %n                          ; <i32> [#uses=1]
  %cmp325 = icmp sgt i32 %tmp324, 0               ; <i1> [#uses=1]
  br i1 %cmp325, label %if.then327, label %if.end394

if.then327:                                       ; preds = %for.end320
  %arraydecay328 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %tmp329 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp330 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp331 = load i32* %n                          ; <i32> [#uses=1]
  %add332 = add nsw i32 %tmp330, %tmp331          ; <i32> [#uses=1]
  %sub333 = sub i32 %add332, 1                    ; <i32> [#uses=1]
  %tmp334 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  call void @NormalDistributionBatch2(float* %arraydecay328, i32 %tmp329, i32 %sub333, i32 %tmp334)
  store i32 0, i32* %i336
  br label %for.cond337

for.cond337:                                      ; preds = %for.inc355, %if.then327
  %tmp338 = load i32* %i336                       ; <i32> [#uses=1]
  %tmp339 = load i32* %n                          ; <i32> [#uses=1]
  %cmp340 = icmp slt i32 %tmp338, %tmp339         ; <i1> [#uses=1]
  br i1 %cmp340, label %for.body342, label %for.end358

for.body342:                                      ; preds = %for.cond337
  %tmp343 = load i32* %i336                       ; <i32> [#uses=1]
  %arraydecay344 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx345 = getelementptr inbounds float* %arraydecay344, i32 %tmp343 ; <float*> [#uses=1]
  %tmp346 = load float* %arrayidx345              ; <float> [#uses=1]
  %tmp347 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul348 = fmul float %tmp346, %tmp347           ; <float> [#uses=1]
  %tmp349 = load float* %MuByT                    ; <float> [#uses=1]
  %add350 = fadd float %mul348, %tmp349           ; <float> [#uses=1]
  %call351 = call float @_Z3expf(float %add350)   ; <float> [#uses=1]
  %tmp352 = load i32* %i336                       ; <i32> [#uses=1]
  %arraydecay353 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx354 = getelementptr inbounds float* %arraydecay353, i32 %tmp352 ; <float*> [#uses=1]
  store float %call351, float* %arrayidx354
  br label %for.inc355

for.inc355:                                       ; preds = %for.body342
  %tmp356 = load i32* %i336                       ; <i32> [#uses=1]
  %add357 = add nsw i32 %tmp356, 1                ; <i32> [#uses=1]
  store i32 %add357, i32* %i336
  br label %for.cond337

for.end358:                                       ; preds = %for.cond337
  store i32 0, i32* %i360
  br label %for.cond361

for.cond361:                                      ; preds = %for.inc387, %for.end358
  %tmp362 = load i32* %i360                       ; <i32> [#uses=1]
  %tmp363 = load i32* %n                          ; <i32> [#uses=1]
  %cmp364 = icmp slt i32 %tmp362, %tmp363         ; <i1> [#uses=1]
  br i1 %cmp364, label %for.body366, label %for.end390

for.body366:                                      ; preds = %for.cond361
  %tmp369 = load float* %S                        ; <float> [#uses=1]
  %tmp370 = load i32* %i360                       ; <i32> [#uses=1]
  %arraydecay371 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx372 = getelementptr inbounds float* %arraydecay371, i32 %tmp370 ; <float*> [#uses=1]
  %tmp373 = load float* %arrayidx372              ; <float> [#uses=1]
  %mul374 = fmul float %tmp369, %tmp373           ; <float> [#uses=1]
  %tmp375 = load float* %X                        ; <float> [#uses=1]
  %sub376 = fsub float %mul374, %tmp375           ; <float> [#uses=1]
  store float %sub376, float* %callValue368
  %tmp377 = load float* %callValue368             ; <float> [#uses=1]
  %call378 = call float @_Z3maxff(float %tmp377, float 0.000000e+000) ; <float> [#uses=1]
  store float %call378, float* %callValue368
  %tmp379 = load float* %callValue368             ; <float> [#uses=1]
  %tmp380 = load float* %sum                      ; <float> [#uses=1]
  %add381 = fadd float %tmp380, %tmp379           ; <float> [#uses=1]
  store float %add381, float* %sum
  %tmp382 = load float* %callValue368             ; <float> [#uses=1]
  %tmp383 = load float* %callValue368             ; <float> [#uses=1]
  %mul384 = fmul float %tmp382, %tmp383           ; <float> [#uses=1]
  %tmp385 = load float* %sum2                     ; <float> [#uses=1]
  %add386 = fadd float %tmp385, %mul384           ; <float> [#uses=1]
  store float %add386, float* %sum2
  br label %for.inc387

for.inc387:                                       ; preds = %for.body366
  %tmp388 = load i32* %i360                       ; <i32> [#uses=1]
  %add389 = add nsw i32 %tmp388, 1                ; <i32> [#uses=1]
  store i32 %add389, i32* %i360
  br label %for.cond361

for.end390:                                       ; preds = %for.cond361
  %tmp391 = load i32* %n                          ; <i32> [#uses=1]
  %tmp392 = load i32* %pos                        ; <i32> [#uses=1]
  %add393 = add nsw i32 %tmp392, %tmp391          ; <i32> [#uses=1]
  store i32 %add393, i32* %pos
  br label %if.end394

if.end394:                                        ; preds = %for.end390, %for.end320
  %tmp395 = load i32* %boundary23                 ; <i32> [#uses=1]
  store i32 %tmp395, i32* %pos
  br label %for.cond396

for.cond396:                                      ; preds = %for.inc462, %if.end394
  %tmp397 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp398 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub399 = sub i32 %tmp398, 512                  ; <i32> [#uses=1]
  %add400 = add nsw i32 %sub399, 1                ; <i32> [#uses=1]
  %cmp401 = icmp slt i32 %tmp397, %add400         ; <i1> [#uses=1]
  br i1 %cmp401, label %for.body403, label %for.end465

for.body403:                                      ; preds = %for.cond396
  %arraydecay404 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %tmp405 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp406 = load i32* %pos                        ; <i32> [#uses=1]
  %add407 = add nsw i32 %tmp406, 512              ; <i32> [#uses=1]
  %sub408 = sub i32 %add407, 1                    ; <i32> [#uses=1]
  %tmp409 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  call void @NormalDistributionBatch3(float* %arraydecay404, i32 %tmp405, i32 %sub408, i32 %tmp409)
  store i32 0, i32* %i411
  br label %for.cond412

for.cond412:                                      ; preds = %for.body416, %for.body403
  %tmp413 = load i32* %i411                       ; <i32> [#uses=1]
  %cmp414 = icmp slt i32 %tmp413, 512             ; <i1> [#uses=1]
  br i1 %cmp414, label %for.body416, label %for.end431

for.body416:                                      ; preds = %for.cond412
  %tmp417 = load i32* %i411                       ; <i32> [#uses=1]
  %arraydecay418 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx419 = getelementptr inbounds float* %arraydecay418, i32 %tmp417 ; <float*> [#uses=1]
  %tmp420 = load float* %arrayidx419              ; <float> [#uses=1]
  %tmp421 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul422 = fmul float %tmp420, %tmp421           ; <float> [#uses=1]
  %tmp423 = load float* %MuByT                    ; <float> [#uses=1]
  %add424 = fadd float %mul422, %tmp423           ; <float> [#uses=1]
  %call425 = call float @_Z3expf(float %add424)   ; <float> [#uses=1]
  %tmp426 = load i32* %i411                       ; <i32> [#uses=1]
  %arraydecay427 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx428 = getelementptr inbounds float* %arraydecay427, i32 %tmp426 ; <float*> [#uses=1]
  store float %call425, float* %arrayidx428
  %tmp429 = load i32* %i411                       ; <i32> [#uses=1]
  %add430 = add nsw i32 %tmp429, 1                ; <i32> [#uses=1]
  store i32 %add430, i32* %i411
  br label %for.cond412

for.end431:                                       ; preds = %for.cond412
  store i32 0, i32* %i433
  br label %for.cond434

for.cond434:                                      ; preds = %for.body438, %for.end431
  %tmp435 = load i32* %i433                       ; <i32> [#uses=1]
  %cmp436 = icmp slt i32 %tmp435, 512             ; <i1> [#uses=1]
  br i1 %cmp436, label %for.body438, label %for.end461

for.body438:                                      ; preds = %for.cond434
  %tmp441 = load float* %S                        ; <float> [#uses=1]
  %tmp442 = load i32* %i433                       ; <i32> [#uses=1]
  %arraydecay443 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx444 = getelementptr inbounds float* %arraydecay443, i32 %tmp442 ; <float*> [#uses=1]
  %tmp445 = load float* %arrayidx444              ; <float> [#uses=1]
  %mul446 = fmul float %tmp441, %tmp445           ; <float> [#uses=1]
  %tmp447 = load float* %X                        ; <float> [#uses=1]
  %sub448 = fsub float %mul446, %tmp447           ; <float> [#uses=1]
  store float %sub448, float* %callValue440
  %tmp449 = load float* %callValue440             ; <float> [#uses=1]
  %call450 = call float @_Z3maxff(float %tmp449, float 0.000000e+000) ; <float> [#uses=1]
  store float %call450, float* %callValue440
  %tmp451 = load float* %callValue440             ; <float> [#uses=1]
  %tmp452 = load float* %sum                      ; <float> [#uses=1]
  %add453 = fadd float %tmp452, %tmp451           ; <float> [#uses=1]
  store float %add453, float* %sum
  %tmp454 = load float* %callValue440             ; <float> [#uses=1]
  %tmp455 = load float* %callValue440             ; <float> [#uses=1]
  %mul456 = fmul float %tmp454, %tmp455           ; <float> [#uses=1]
  %tmp457 = load float* %sum2                     ; <float> [#uses=1]
  %add458 = fadd float %tmp457, %mul456           ; <float> [#uses=1]
  store float %add458, float* %sum2
  %tmp459 = load i32* %i433                       ; <i32> [#uses=1]
  %add460 = add nsw i32 %tmp459, 1                ; <i32> [#uses=1]
  store i32 %add460, i32* %i433
  br label %for.cond434

for.end461:                                       ; preds = %for.cond434
  br label %for.inc462

for.inc462:                                       ; preds = %for.end461
  %tmp463 = load i32* %pos                        ; <i32> [#uses=1]
  %add464 = add nsw i32 %tmp463, 512              ; <i32> [#uses=1]
  store i32 %add464, i32* %pos
  br label %for.cond396

for.end465:                                       ; preds = %for.cond396
  %tmp466 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp467 = load i32* %pos                        ; <i32> [#uses=1]
  %sub468 = sub i32 %tmp466, %tmp467              ; <i32> [#uses=1]
  store i32 %sub468, i32* %n
  %tmp469 = load i32* %n                          ; <i32> [#uses=1]
  %cmp470 = icmp sgt i32 %tmp469, 0               ; <i1> [#uses=1]
  br i1 %cmp470, label %if.then472, label %if.end539

if.then472:                                       ; preds = %for.end465
  %arraydecay473 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %tmp474 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp475 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp476 = load i32* %n                          ; <i32> [#uses=1]
  %add477 = add nsw i32 %tmp475, %tmp476          ; <i32> [#uses=1]
  %sub478 = sub i32 %add477, 1                    ; <i32> [#uses=1]
  %tmp479 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  call void @NormalDistributionBatch3(float* %arraydecay473, i32 %tmp474, i32 %sub478, i32 %tmp479)
  store i32 0, i32* %i481
  br label %for.cond482

for.cond482:                                      ; preds = %for.inc500, %if.then472
  %tmp483 = load i32* %i481                       ; <i32> [#uses=1]
  %tmp484 = load i32* %n                          ; <i32> [#uses=1]
  %cmp485 = icmp slt i32 %tmp483, %tmp484         ; <i1> [#uses=1]
  br i1 %cmp485, label %for.body487, label %for.end503

for.body487:                                      ; preds = %for.cond482
  %tmp488 = load i32* %i481                       ; <i32> [#uses=1]
  %arraydecay489 = getelementptr inbounds [512 x float]* %sample, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx490 = getelementptr inbounds float* %arraydecay489, i32 %tmp488 ; <float*> [#uses=1]
  %tmp491 = load float* %arrayidx490              ; <float> [#uses=1]
  %tmp492 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul493 = fmul float %tmp491, %tmp492           ; <float> [#uses=1]
  %tmp494 = load float* %MuByT                    ; <float> [#uses=1]
  %add495 = fadd float %mul493, %tmp494           ; <float> [#uses=1]
  %call496 = call float @_Z3expf(float %add495)   ; <float> [#uses=1]
  %tmp497 = load i32* %i481                       ; <i32> [#uses=1]
  %arraydecay498 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx499 = getelementptr inbounds float* %arraydecay498, i32 %tmp497 ; <float*> [#uses=1]
  store float %call496, float* %arrayidx499
  br label %for.inc500

for.inc500:                                       ; preds = %for.body487
  %tmp501 = load i32* %i481                       ; <i32> [#uses=1]
  %add502 = add nsw i32 %tmp501, 1                ; <i32> [#uses=1]
  store i32 %add502, i32* %i481
  br label %for.cond482

for.end503:                                       ; preds = %for.cond482
  store i32 0, i32* %i505
  br label %for.cond506

for.cond506:                                      ; preds = %for.inc532, %for.end503
  %tmp507 = load i32* %i505                       ; <i32> [#uses=1]
  %tmp508 = load i32* %n                          ; <i32> [#uses=1]
  %cmp509 = icmp slt i32 %tmp507, %tmp508         ; <i1> [#uses=1]
  br i1 %cmp509, label %for.body511, label %for.end535

for.body511:                                      ; preds = %for.cond506
  %tmp514 = load float* %S                        ; <float> [#uses=1]
  %tmp515 = load i32* %i505                       ; <i32> [#uses=1]
  %arraydecay516 = getelementptr inbounds [512 x float]* %temp, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx517 = getelementptr inbounds float* %arraydecay516, i32 %tmp515 ; <float*> [#uses=1]
  %tmp518 = load float* %arrayidx517              ; <float> [#uses=1]
  %mul519 = fmul float %tmp514, %tmp518           ; <float> [#uses=1]
  %tmp520 = load float* %X                        ; <float> [#uses=1]
  %sub521 = fsub float %mul519, %tmp520           ; <float> [#uses=1]
  store float %sub521, float* %callValue513
  %tmp522 = load float* %callValue513             ; <float> [#uses=1]
  %call523 = call float @_Z3maxff(float %tmp522, float 0.000000e+000) ; <float> [#uses=1]
  store float %call523, float* %callValue513
  %tmp524 = load float* %callValue513             ; <float> [#uses=1]
  %tmp525 = load float* %sum                      ; <float> [#uses=1]
  %add526 = fadd float %tmp525, %tmp524           ; <float> [#uses=1]
  store float %add526, float* %sum
  %tmp527 = load float* %callValue513             ; <float> [#uses=1]
  %tmp528 = load float* %callValue513             ; <float> [#uses=1]
  %mul529 = fmul float %tmp527, %tmp528           ; <float> [#uses=1]
  %tmp530 = load float* %sum2                     ; <float> [#uses=1]
  %add531 = fadd float %tmp530, %mul529           ; <float> [#uses=1]
  store float %add531, float* %sum2
  br label %for.inc532

for.inc532:                                       ; preds = %for.body511
  %tmp533 = load i32* %i505                       ; <i32> [#uses=1]
  %add534 = add nsw i32 %tmp533, 1                ; <i32> [#uses=1]
  store i32 %add534, i32* %i505
  br label %for.cond506

for.end535:                                       ; preds = %for.cond506
  %tmp536 = load i32* %n                          ; <i32> [#uses=1]
  %tmp537 = load i32* %pos                        ; <i32> [#uses=1]
  %add538 = add nsw i32 %tmp537, %tmp536          ; <i32> [#uses=1]
  store i32 %add538, i32* %pos
  br label %if.end539

if.end539:                                        ; preds = %for.end535, %for.end465
  %tmp541 = load float* %sum                      ; <float> [#uses=1]
  store float %tmp541, float* %totalsum
  %tmp543 = load float* %sum2                     ; <float> [#uses=1]
  store float %tmp543, float* %totalsum2
  %tmp545 = load float* %R                        ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp545       ; <float> [#uses=1]
  %tmp546 = load float* %T                        ; <float> [#uses=1]
  %mul547 = fmul float %neg, %tmp546              ; <float> [#uses=1]
  %call548 = call float @_Z3expf(float %mul547)   ; <float> [#uses=1]
  store float %call548, float* %ExpRT
  %tmp549 = load float* %ExpRT                    ; <float> [#uses=1]
  %tmp550 = load float* %totalsum                 ; <float> [#uses=1]
  %mul551 = fmul float %tmp549, %tmp550           ; <float> [#uses=1]
  %tmp552 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv553 = sitofp i32 %tmp552 to float          ; <float> [#uses=3]
  %cmp554 = fcmp oeq float 0.000000e+000, %conv553 ; <i1> [#uses=1]
  %sel555 = select i1 %cmp554, float 1.000000e+000, float %conv553 ; <float> [#uses=0]
  %div556 = fdiv float %mul551, %conv553          ; <float> [#uses=1]
  %tmp557 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp558 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx559 = getelementptr inbounds %struct.anon addrspace(1)* %tmp558, i32 %tmp557 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp560 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx559, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %div556, float addrspace(1)* %tmp560
  %tmp562 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv563 = sitofp i32 %tmp562 to float          ; <float> [#uses=1]
  %tmp564 = load float* %totalsum2                ; <float> [#uses=1]
  %mul565 = fmul float %conv563, %tmp564          ; <float> [#uses=1]
  %tmp566 = load float* %totalsum                 ; <float> [#uses=1]
  %tmp567 = load float* %totalsum                 ; <float> [#uses=1]
  %mul568 = fmul float %tmp566, %tmp567           ; <float> [#uses=1]
  %sub569 = fsub float %mul565, %mul568           ; <float> [#uses=1]
  %tmp570 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv571 = sitofp i32 %tmp570 to float          ; <float> [#uses=1]
  %tmp572 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub573 = sub i32 %tmp572, 1                    ; <i32> [#uses=1]
  %conv574 = sitofp i32 %sub573 to float          ; <float> [#uses=1]
  %mul575 = fmul float %conv571, %conv574         ; <float> [#uses=3]
  %cmp576 = fcmp oeq float 0.000000e+000, %mul575 ; <i1> [#uses=1]
  %sel577 = select i1 %cmp576, float 1.000000e+000, float %mul575 ; <float> [#uses=0]
  %div578 = fdiv float %sub569, %mul575           ; <float> [#uses=1]
  %call579 = call float @_Z4sqrtf(float %div578)  ; <float> [#uses=1]
  store float %call579, float* %stdDev
  %tmp580 = load float* %ExpRT                    ; <float> [#uses=1]
  %conv581 = fpext float %tmp580 to double        ; <double> [#uses=1]
  %mul582 = fmul double %conv581, 1.960000e+000   ; <double> [#uses=1]
  %tmp583 = load float* %stdDev                   ; <float> [#uses=1]
  %conv584 = fpext float %tmp583 to double        ; <double> [#uses=1]
  %mul585 = fmul double %mul582, %conv584         ; <double> [#uses=1]
  %tmp586 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv587 = sitofp i32 %tmp586 to float          ; <float> [#uses=1]
  %call588 = call float @_Z4sqrtf(float %conv587) ; <float> [#uses=1]
  %conv589 = fpext float %call588 to double       ; <double> [#uses=3]
  %cmp590 = fcmp oeq double 0.000000e+000, %conv589 ; <i1> [#uses=1]
  %sel591 = select i1 %cmp590, double 1.000000e+000, double %conv589 ; <double> [#uses=0]
  %div592 = fdiv double %mul585, %conv589         ; <double> [#uses=1]
  %conv593 = fptrunc double %div592 to float      ; <float> [#uses=1]
  %tmp594 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp595 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx596 = getelementptr inbounds %struct.anon addrspace(1)* %tmp595, i32 %tmp594 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp597 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx596, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %conv593, float addrspace(1)* %tmp597
  ret void
}

declare i32 @get_global_id(i32)

declare float @_Z4sqrtf(float)

declare float @_Z3expf(float)

declare float @_Z3maxff(float, float)

; CHECK: ret
define void @wlMonteCarloKernel_PrecalculatedSamples(%struct.anon addrspace(1)* %pOptionValue, %0 addrspace(1)* %pOptionData, float addrspace(1)* %pSamples, i32 %pathN) nounwind {
entry:
  %pOptionValue.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=3]
  %pOptionData.addr = alloca %0 addrspace(1)*, align 4 ; <%0 addrspace(1)**> [#uses=6]
  %pSamples.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=7]
  %optionid = alloca i32, align 4                 ; <i32*> [#uses=8]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %S = alloca float, align 4                      ; <float*> [#uses=2]
  %X = alloca float, align 4                      ; <float*> [#uses=2]
  %V = alloca float, align 4                      ; <float*> [#uses=4]
  %MuByT = alloca float, align 4                  ; <float*> [#uses=2]
  %VBySqrtT = alloca float, align 4               ; <float*> [#uses=2]
  %sum = alloca float, align 4                    ; <float*> [#uses=4]
  %sum2 = alloca float, align 4                   ; <float*> [#uses=4]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=5]
  %expVal = alloca float, align 4                 ; <float*> [#uses=2]
  %callValue = alloca float, align 4              ; <float*> [#uses=6]
  %totalsum = alloca float, align 4               ; <float*> [#uses=4]
  %totalsum2 = alloca float, align 4              ; <float*> [#uses=2]
  %ExpRT = alloca float, align 4                  ; <float*> [#uses=3]
  %stdDev = alloca float, align 4                 ; <float*> [#uses=2]
  store %struct.anon addrspace(1)* %pOptionValue, %struct.anon addrspace(1)** %pOptionValue.addr
  store %0 addrspace(1)* %pOptionData, %0 addrspace(1)** %pOptionData.addr
  store float addrspace(1)* %pSamples, float addrspace(1)** %pSamples.addr
  store i32 %pathN, i32* %pathN.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %optionid
  %tmp = load i32* %optionid                      ; <i32> [#uses=1]
  %tmp1 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds %0 addrspace(1)* %tmp1, i32 %tmp ; <%0 addrspace(1)*> [#uses=1]
  %tmp2 = getelementptr inbounds %0 addrspace(1)* %arrayidx, i32 0, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp3 = load float addrspace(1)* %tmp2          ; <float> [#uses=1]
  store float %tmp3, float* %T
  %tmp5 = load i32* %optionid                     ; <i32> [#uses=1]
  %tmp6 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx7 = getelementptr inbounds %0 addrspace(1)* %tmp6, i32 %tmp5 ; <%0 addrspace(1)*> [#uses=1]
  %tmp8 = getelementptr inbounds %0 addrspace(1)* %arrayidx7, i32 0, i32 3 ; <float addrspace(1)*> [#uses=1]
  %tmp9 = load float addrspace(1)* %tmp8          ; <float> [#uses=1]
  store float %tmp9, float* %R
  %tmp11 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp12 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx13 = getelementptr inbounds %0 addrspace(1)* %tmp12, i32 %tmp11 ; <%0 addrspace(1)*> [#uses=1]
  %tmp14 = getelementptr inbounds %0 addrspace(1)* %arrayidx13, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load float addrspace(1)* %tmp14        ; <float> [#uses=1]
  store float %tmp15, float* %S
  %tmp17 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp18 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds %0 addrspace(1)* %tmp18, i32 %tmp17 ; <%0 addrspace(1)*> [#uses=1]
  %tmp20 = getelementptr inbounds %0 addrspace(1)* %arrayidx19, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp21 = load float addrspace(1)* %tmp20        ; <float> [#uses=1]
  store float %tmp21, float* %X
  %tmp23 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp24 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds %0 addrspace(1)* %tmp24, i32 %tmp23 ; <%0 addrspace(1)*> [#uses=1]
  %tmp26 = getelementptr inbounds %0 addrspace(1)* %arrayidx25, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  %tmp27 = load float addrspace(1)* %tmp26        ; <float> [#uses=1]
  store float %tmp27, float* %V
  %tmp29 = load float* %R                         ; <float> [#uses=1]
  %tmp30 = load float* %V                         ; <float> [#uses=1]
  %mul = fmul float 5.000000e-001, %tmp30         ; <float> [#uses=1]
  %tmp31 = load float* %V                         ; <float> [#uses=1]
  %mul32 = fmul float %mul, %tmp31                ; <float> [#uses=1]
  %sub = fsub float %tmp29, %mul32                ; <float> [#uses=1]
  %tmp33 = load float* %T                         ; <float> [#uses=1]
  %mul34 = fmul float %sub, %tmp33                ; <float> [#uses=1]
  store float %mul34, float* %MuByT
  %tmp36 = load float* %V                         ; <float> [#uses=1]
  %tmp37 = load float* %T                         ; <float> [#uses=1]
  %call38 = call float @_Z4sqrtf(float %tmp37)    ; <float> [#uses=1]
  %mul39 = fmul float %tmp36, %call38             ; <float> [#uses=1]
  store float %mul39, float* %VBySqrtT
  store float 0.000000e+000, float* %sum
  store float 0.000000e+000, float* %sum2
  store i32 0, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp43 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp44 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp43, %tmp44              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp46 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp47 = load float addrspace(1)** %pSamples.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx48 = getelementptr inbounds float addrspace(1)* %tmp47, i32 %tmp46 ; <float addrspace(1)*> [#uses=1]
  %tmp49 = load float addrspace(1)* %arrayidx48   ; <float> [#uses=1]
  %tmp50 = load float* %VBySqrtT                  ; <float> [#uses=1]
  %mul51 = fmul float %tmp49, %tmp50              ; <float> [#uses=1]
  %tmp52 = load float* %MuByT                     ; <float> [#uses=1]
  %add = fadd float %mul51, %tmp52                ; <float> [#uses=1]
  %call53 = call float @_Z3expf(float %add)       ; <float> [#uses=1]
  store float %call53, float* %expVal
  %tmp55 = load float* %S                         ; <float> [#uses=1]
  %tmp56 = load float* %expVal                    ; <float> [#uses=1]
  %mul57 = fmul float %tmp55, %tmp56              ; <float> [#uses=1]
  %tmp58 = load float* %X                         ; <float> [#uses=1]
  %sub59 = fsub float %mul57, %tmp58              ; <float> [#uses=1]
  store float %sub59, float* %callValue
  %tmp60 = load float* %callValue                 ; <float> [#uses=1]
  %call61 = call float @_Z3maxff(float %tmp60, float 0.000000e+000) ; <float> [#uses=1]
  store float %call61, float* %callValue
  %tmp62 = load float* %callValue                 ; <float> [#uses=1]
  %tmp63 = load float* %sum                       ; <float> [#uses=1]
  %add64 = fadd float %tmp63, %tmp62              ; <float> [#uses=1]
  store float %add64, float* %sum
  %tmp65 = load float* %callValue                 ; <float> [#uses=1]
  %tmp66 = load float* %callValue                 ; <float> [#uses=1]
  %mul67 = fmul float %tmp65, %tmp66              ; <float> [#uses=1]
  %tmp68 = load float* %sum2                      ; <float> [#uses=1]
  %add69 = fadd float %tmp68, %mul67              ; <float> [#uses=1]
  store float %add69, float* %sum2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp70 = load i32* %pos                         ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp70, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp72 = load float* %sum                       ; <float> [#uses=1]
  store float %tmp72, float* %totalsum
  %tmp74 = load float* %sum2                      ; <float> [#uses=1]
  store float %tmp74, float* %totalsum2
  %tmp76 = load float* %R                         ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp76        ; <float> [#uses=1]
  %tmp77 = load float* %T                         ; <float> [#uses=1]
  %mul78 = fmul float %neg, %tmp77                ; <float> [#uses=1]
  %call79 = call float @_Z3expf(float %mul78)     ; <float> [#uses=1]
  store float %call79, float* %ExpRT
  %tmp80 = load float* %ExpRT                     ; <float> [#uses=1]
  %tmp81 = load float* %totalsum                  ; <float> [#uses=1]
  %mul82 = fmul float %tmp80, %tmp81              ; <float> [#uses=1]
  %tmp83 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp83 to float              ; <float> [#uses=3]
  %cmp84 = fcmp oeq float 0.000000e+000, %conv    ; <i1> [#uses=1]
  %sel = select i1 %cmp84, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float %mul82, %conv                 ; <float> [#uses=1]
  %tmp85 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp86 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx87 = getelementptr inbounds %struct.anon addrspace(1)* %tmp86, i32 %tmp85 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp88 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx87, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %div, float addrspace(1)* %tmp88
  %tmp90 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv91 = sitofp i32 %tmp90 to float            ; <float> [#uses=1]
  %tmp92 = load float* %totalsum2                 ; <float> [#uses=1]
  %mul93 = fmul float %conv91, %tmp92             ; <float> [#uses=1]
  %tmp94 = load float* %totalsum                  ; <float> [#uses=1]
  %tmp95 = load float* %totalsum                  ; <float> [#uses=1]
  %mul96 = fmul float %tmp94, %tmp95              ; <float> [#uses=1]
  %sub97 = fsub float %mul93, %mul96              ; <float> [#uses=1]
  %tmp98 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv99 = sitofp i32 %tmp98 to float            ; <float> [#uses=1]
  %tmp100 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub101 = sub i32 %tmp100, 1                    ; <i32> [#uses=1]
  %conv102 = sitofp i32 %sub101 to float          ; <float> [#uses=1]
  %mul103 = fmul float %conv99, %conv102          ; <float> [#uses=3]
  %cmp104 = fcmp oeq float 0.000000e+000, %mul103 ; <i1> [#uses=1]
  %sel105 = select i1 %cmp104, float 1.000000e+000, float %mul103 ; <float> [#uses=0]
  %div106 = fdiv float %sub97, %mul103            ; <float> [#uses=1]
  %call107 = call float @_Z4sqrtf(float %div106)  ; <float> [#uses=1]
  store float %call107, float* %stdDev
  %tmp108 = load float* %ExpRT                    ; <float> [#uses=1]
  %conv109 = fpext float %tmp108 to double        ; <double> [#uses=1]
  %mul110 = fmul double %conv109, 1.960000e+000   ; <double> [#uses=1]
  %tmp111 = load float* %stdDev                   ; <float> [#uses=1]
  %conv112 = fpext float %tmp111 to double        ; <double> [#uses=1]
  %mul113 = fmul double %mul110, %conv112         ; <double> [#uses=1]
  %tmp114 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv115 = sitofp i32 %tmp114 to float          ; <float> [#uses=1]
  %call116 = call float @_Z4sqrtf(float %conv115) ; <float> [#uses=1]
  %conv117 = fpext float %call116 to double       ; <double> [#uses=3]
  %cmp118 = fcmp oeq double 0.000000e+000, %conv117 ; <i1> [#uses=1]
  %sel119 = select i1 %cmp118, double 1.000000e+000, double %conv117 ; <double> [#uses=0]
  %div120 = fdiv double %mul113, %conv117         ; <double> [#uses=1]
  %conv121 = fptrunc double %div120 to float      ; <float> [#uses=1]
  %tmp122 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp123 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx124 = getelementptr inbounds %struct.anon addrspace(1)* %tmp123, i32 %tmp122 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp125 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx124, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %conv121, float addrspace(1)* %tmp125
  ret void
}
