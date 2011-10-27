; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlMonteCarlo_scalar2.cl'
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
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=10]
  %sample = alloca [512 x float], align 4         ; <[512 x float]*> [#uses=0]
  %temp = alloca [512 x float], align 4           ; <[512 x float]*> [#uses=0]
  %optionid = alloca i32, align 4                 ; <i32*> [#uses=8]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %S = alloca float, align 4                      ; <float*> [#uses=4]
  %X = alloca float, align 4                      ; <float*> [#uses=4]
  %V = alloca float, align 4                      ; <float*> [#uses=4]
  %MuByT = alloca float, align 4                  ; <float*> [#uses=4]
  %VBySqrtT = alloca float, align 4               ; <float*> [#uses=4]
  %sum = alloca float, align 4                    ; <float*> [#uses=8]
  %sum2 = alloca float, align 4                   ; <float*> [#uses=8]
  %boundary12 = alloca i32, align 4               ; <i32*> [#uses=3]
  %boundary23 = alloca i32, align 4               ; <i32*> [#uses=3]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=15]
  %n = alloca i32, align 4                        ; <i32*> [#uses=0]
  %pathNinv = alloca float, align 4               ; <float*> [#uses=4]
  %index = alloca i32, align 4                    ; <i32*> [#uses=1]
  %c1 = alloca float, align 4                     ; <float*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=2]
  %c4 = alloca float, align 4                     ; <float*> [#uses=2]
  %c5 = alloca float, align 4                     ; <float*> [#uses=2]
  %c6 = alloca float, align 4                     ; <float*> [#uses=2]
  %c7 = alloca float, align 4                     ; <float*> [#uses=2]
  %c8 = alloca float, align 4                     ; <float*> [#uses=2]
  %c9 = alloca float, align 4                     ; <float*> [#uses=2]
  %p = alloca float, align 4                      ; <float*> [#uses=2]
  %z = alloca float, align 4                      ; <float*> [#uses=11]
  %t = alloca float, align 4                      ; <float*> [#uses=2]
  %callValue = alloca float, align 4              ; <float*> [#uses=6]
  %a1 = alloca float, align 4                     ; <float*> [#uses=2]
  %a2 = alloca float, align 4                     ; <float*> [#uses=2]
  %a3 = alloca float, align 4                     ; <float*> [#uses=2]
  %a4 = alloca float, align 4                     ; <float*> [#uses=2]
  %b1 = alloca float, align 4                     ; <float*> [#uses=2]
  %b2 = alloca float, align 4                     ; <float*> [#uses=2]
  %b3 = alloca float, align 4                     ; <float*> [#uses=2]
  %b4 = alloca float, align 4                     ; <float*> [#uses=2]
  %p163 = alloca float, align 4                   ; <float*> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=4]
  %z173 = alloca float, align 4                   ; <float*> [#uses=8]
  %sample178 = alloca float, align 4              ; <float*> [#uses=2]
  %callValue214 = alloca float, align 4           ; <float*> [#uses=6]
  %index240 = alloca i32, align 4                 ; <i32*> [#uses=1]
  %c1242 = alloca float, align 4                  ; <float*> [#uses=2]
  %c2244 = alloca float, align 4                  ; <float*> [#uses=2]
  %c3246 = alloca float, align 4                  ; <float*> [#uses=2]
  %c4248 = alloca float, align 4                  ; <float*> [#uses=2]
  %c5250 = alloca float, align 4                  ; <float*> [#uses=2]
  %c6252 = alloca float, align 4                  ; <float*> [#uses=2]
  %c7254 = alloca float, align 4                  ; <float*> [#uses=2]
  %c8256 = alloca float, align 4                  ; <float*> [#uses=2]
  %c9258 = alloca float, align 4                  ; <float*> [#uses=2]
  %p267 = alloca float, align 4                   ; <float*> [#uses=2]
  %z274 = alloca float, align 4                   ; <float*> [#uses=9]
  %t281 = alloca float, align 4                   ; <float*> [#uses=2]
  %callValue316 = alloca float, align 4           ; <float*> [#uses=6]
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
  %tmp62 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add = add nsw i32 %tmp62, 1                    ; <i32> [#uses=1]
  %conv63 = sitofp i32 %add to float              ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv63    ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv63 ; <float> [#uses=0]
  %div64 = fdiv float 1.000000e+000, %conv63      ; <float> [#uses=1]
  store float %div64, float* %pathNinv
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
  store i32 0, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp75 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp76 = load i32* %boundary12                  ; <i32> [#uses=1]
  %cmp77 = icmp slt i32 %tmp75, %tmp76            ; <i1> [#uses=1]
  br i1 %cmp77, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp82 = load i32* %pos                         ; <i32> [#uses=1]
  %add83 = add nsw i32 %tmp82, 1                  ; <i32> [#uses=1]
  %conv84 = sitofp i32 %add83 to float            ; <float> [#uses=1]
  %tmp85 = load float* %pathNinv                  ; <float> [#uses=1]
  %mul86 = fmul float %conv84, %tmp85             ; <float> [#uses=1]
  store float %mul86, float* %p
  %tmp87 = load float* %p                         ; <float> [#uses=1]
  %call88 = call float @_Z3logf(float %tmp87)     ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %call88       ; <float> [#uses=1]
  %call89 = call float @_Z3logf(float %neg)       ; <float> [#uses=1]
  store float %call89, float* %z
  %tmp90 = load float* %c1                        ; <float> [#uses=1]
  %tmp91 = load float* %z                         ; <float> [#uses=1]
  %tmp92 = load float* %c2                        ; <float> [#uses=1]
  %tmp93 = load float* %z                         ; <float> [#uses=1]
  %tmp94 = load float* %c3                        ; <float> [#uses=1]
  %tmp95 = load float* %z                         ; <float> [#uses=1]
  %tmp96 = load float* %c4                        ; <float> [#uses=1]
  %tmp97 = load float* %z                         ; <float> [#uses=1]
  %tmp98 = load float* %c5                        ; <float> [#uses=1]
  %tmp99 = load float* %z                         ; <float> [#uses=1]
  %tmp100 = load float* %c6                       ; <float> [#uses=1]
  %tmp101 = load float* %z                        ; <float> [#uses=1]
  %tmp102 = load float* %c7                       ; <float> [#uses=1]
  %tmp103 = load float* %z                        ; <float> [#uses=1]
  %tmp104 = load float* %c8                       ; <float> [#uses=1]
  %tmp105 = load float* %z                        ; <float> [#uses=1]
  %tmp106 = load float* %c9                       ; <float> [#uses=1]
  %mul107 = fmul float %tmp105, %tmp106           ; <float> [#uses=1]
  %add108 = fadd float %tmp104, %mul107           ; <float> [#uses=1]
  %mul109 = fmul float %tmp103, %add108           ; <float> [#uses=1]
  %add110 = fadd float %tmp102, %mul109           ; <float> [#uses=1]
  %mul111 = fmul float %tmp101, %add110           ; <float> [#uses=1]
  %add112 = fadd float %tmp100, %mul111           ; <float> [#uses=1]
  %mul113 = fmul float %tmp99, %add112            ; <float> [#uses=1]
  %add114 = fadd float %tmp98, %mul113            ; <float> [#uses=1]
  %mul115 = fmul float %tmp97, %add114            ; <float> [#uses=1]
  %add116 = fadd float %tmp96, %mul115            ; <float> [#uses=1]
  %mul117 = fmul float %tmp95, %add116            ; <float> [#uses=1]
  %add118 = fadd float %tmp94, %mul117            ; <float> [#uses=1]
  %mul119 = fmul float %tmp93, %add118            ; <float> [#uses=1]
  %add120 = fadd float %tmp92, %mul119            ; <float> [#uses=1]
  %mul121 = fmul float %tmp91, %add120            ; <float> [#uses=1]
  %add122 = fadd float %tmp90, %mul121            ; <float> [#uses=1]
  store float %add122, float* %z
  %tmp123 = load float* %z                        ; <float> [#uses=1]
  %neg124 = fsub float -0.000000e+000, %tmp123    ; <float> [#uses=1]
  store float %neg124, float* %t
  %tmp126 = load float* %S                        ; <float> [#uses=1]
  %tmp127 = load float* %t                        ; <float> [#uses=1]
  %tmp128 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul129 = fmul float %tmp127, %tmp128           ; <float> [#uses=1]
  %tmp130 = load float* %MuByT                    ; <float> [#uses=1]
  %add131 = fadd float %mul129, %tmp130           ; <float> [#uses=1]
  %call132 = call float @_Z3expf(float %add131)   ; <float> [#uses=1]
  %mul133 = fmul float %tmp126, %call132          ; <float> [#uses=1]
  %tmp134 = load float* %X                        ; <float> [#uses=1]
  %sub135 = fsub float %mul133, %tmp134           ; <float> [#uses=1]
  store float %sub135, float* %callValue
  %tmp136 = load float* %callValue                ; <float> [#uses=1]
  %call137 = call float @_Z3maxff(float %tmp136, float 0.000000e+000) ; <float> [#uses=1]
  store float %call137, float* %callValue
  %tmp138 = load float* %callValue                ; <float> [#uses=1]
  %tmp139 = load float* %sum                      ; <float> [#uses=1]
  %add140 = fadd float %tmp139, %tmp138           ; <float> [#uses=1]
  store float %add140, float* %sum
  %tmp141 = load float* %callValue                ; <float> [#uses=1]
  %tmp142 = load float* %callValue                ; <float> [#uses=1]
  %mul143 = fmul float %tmp141, %tmp142           ; <float> [#uses=1]
  %tmp144 = load float* %sum2                     ; <float> [#uses=1]
  %add145 = fadd float %tmp144, %mul143           ; <float> [#uses=1]
  store float %add145, float* %sum2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp146 = load i32* %pos                        ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp146, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store float 0x40040D9320000000, float* %a1
  store float 0xC0329D70A0000000, float* %a2
  store float 0x4044B212C0000000, float* %a3
  store float 0xC03970E960000000, float* %a4
  store float 0xC020F27000000000, float* %b1
  store float 0x40371557A0000000, float* %b2
  store float 0xC0350FEF00000000, float* %b3
  store float 0x40090BF020000000, float* %b4
  %tmp155 = load i32* %boundary12                 ; <i32> [#uses=1]
  store i32 %tmp155, i32* %pos
  br label %for.cond156

for.cond156:                                      ; preds = %for.inc235, %for.end
  %tmp157 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp158 = load i32* %boundary23                 ; <i32> [#uses=1]
  %cmp159 = icmp slt i32 %tmp157, %tmp158         ; <i1> [#uses=1]
  br i1 %cmp159, label %for.body161, label %for.end238

for.body161:                                      ; preds = %for.cond156
  %tmp164 = load i32* %pos                        ; <i32> [#uses=1]
  %add165 = add nsw i32 %tmp164, 1                ; <i32> [#uses=1]
  %conv166 = sitofp i32 %add165 to float          ; <float> [#uses=1]
  %tmp167 = load float* %pathNinv                 ; <float> [#uses=1]
  %mul168 = fmul float %conv166, %tmp167          ; <float> [#uses=1]
  store float %mul168, float* %p163
  %tmp170 = load float* %p163                     ; <float> [#uses=1]
  %sub171 = fsub float %tmp170, 5.000000e-001     ; <float> [#uses=1]
  store float %sub171, float* %y
  %tmp174 = load float* %y                        ; <float> [#uses=1]
  %tmp175 = load float* %y                        ; <float> [#uses=1]
  %mul176 = fmul float %tmp174, %tmp175           ; <float> [#uses=1]
  store float %mul176, float* %z173
  %tmp179 = load float* %y                        ; <float> [#uses=1]
  %tmp180 = load float* %a4                       ; <float> [#uses=1]
  %tmp181 = load float* %z173                     ; <float> [#uses=1]
  %mul182 = fmul float %tmp180, %tmp181           ; <float> [#uses=1]
  %tmp183 = load float* %a3                       ; <float> [#uses=1]
  %add184 = fadd float %mul182, %tmp183           ; <float> [#uses=1]
  %tmp185 = load float* %z173                     ; <float> [#uses=1]
  %mul186 = fmul float %add184, %tmp185           ; <float> [#uses=1]
  %tmp187 = load float* %a2                       ; <float> [#uses=1]
  %add188 = fadd float %mul186, %tmp187           ; <float> [#uses=1]
  %tmp189 = load float* %z173                     ; <float> [#uses=1]
  %mul190 = fmul float %add188, %tmp189           ; <float> [#uses=1]
  %tmp191 = load float* %a1                       ; <float> [#uses=1]
  %add192 = fadd float %mul190, %tmp191           ; <float> [#uses=1]
  %mul193 = fmul float %tmp179, %add192           ; <float> [#uses=1]
  %tmp194 = load float* %b4                       ; <float> [#uses=1]
  %tmp195 = load float* %z173                     ; <float> [#uses=1]
  %mul196 = fmul float %tmp194, %tmp195           ; <float> [#uses=1]
  %tmp197 = load float* %b3                       ; <float> [#uses=1]
  %add198 = fadd float %mul196, %tmp197           ; <float> [#uses=1]
  %tmp199 = load float* %z173                     ; <float> [#uses=1]
  %mul200 = fmul float %add198, %tmp199           ; <float> [#uses=1]
  %tmp201 = load float* %b2                       ; <float> [#uses=1]
  %add202 = fadd float %mul200, %tmp201           ; <float> [#uses=1]
  %tmp203 = load float* %z173                     ; <float> [#uses=1]
  %mul204 = fmul float %add202, %tmp203           ; <float> [#uses=1]
  %tmp205 = load float* %b1                       ; <float> [#uses=1]
  %add206 = fadd float %mul204, %tmp205           ; <float> [#uses=1]
  %tmp207 = load float* %z173                     ; <float> [#uses=1]
  %mul208 = fmul float %add206, %tmp207           ; <float> [#uses=1]
  %add209 = fadd float %mul208, 1.000000e+000     ; <float> [#uses=3]
  %cmp210 = fcmp oeq float 0.000000e+000, %add209 ; <i1> [#uses=1]
  %sel211 = select i1 %cmp210, float 1.000000e+000, float %add209 ; <float> [#uses=0]
  %div212 = fdiv float %mul193, %add209           ; <float> [#uses=1]
  store float %div212, float* %sample178
  %tmp215 = load float* %S                        ; <float> [#uses=1]
  %tmp216 = load float* %sample178                ; <float> [#uses=1]
  %tmp217 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul218 = fmul float %tmp216, %tmp217           ; <float> [#uses=1]
  %tmp219 = load float* %MuByT                    ; <float> [#uses=1]
  %add220 = fadd float %mul218, %tmp219           ; <float> [#uses=1]
  %call221 = call float @_Z3expf(float %add220)   ; <float> [#uses=1]
  %mul222 = fmul float %tmp215, %call221          ; <float> [#uses=1]
  %tmp223 = load float* %X                        ; <float> [#uses=1]
  %sub224 = fsub float %mul222, %tmp223           ; <float> [#uses=1]
  store float %sub224, float* %callValue214
  %tmp225 = load float* %callValue214             ; <float> [#uses=1]
  %call226 = call float @_Z3maxff(float %tmp225, float 0.000000e+000) ; <float> [#uses=1]
  store float %call226, float* %callValue214
  %tmp227 = load float* %callValue214             ; <float> [#uses=1]
  %tmp228 = load float* %sum                      ; <float> [#uses=1]
  %add229 = fadd float %tmp228, %tmp227           ; <float> [#uses=1]
  store float %add229, float* %sum
  %tmp230 = load float* %callValue214             ; <float> [#uses=1]
  %tmp231 = load float* %callValue214             ; <float> [#uses=1]
  %mul232 = fmul float %tmp230, %tmp231           ; <float> [#uses=1]
  %tmp233 = load float* %sum2                     ; <float> [#uses=1]
  %add234 = fadd float %tmp233, %mul232           ; <float> [#uses=1]
  store float %add234, float* %sum2
  br label %for.inc235

for.inc235:                                       ; preds = %for.body161
  %tmp236 = load i32* %pos                        ; <i32> [#uses=1]
  %inc237 = add nsw i32 %tmp236, 1                ; <i32> [#uses=1]
  store i32 %inc237, i32* %pos
  br label %for.cond156

for.end238:                                       ; preds = %for.cond156
  store i32 0, i32* %index240
  store float 0x3FD59932C0000000, float* %c1242
  store float 0x3FEF3CC6C0000000, float* %c2244
  store float 0x3FC4950720000000, float* %c3246
  store float 0x3F9C4EAD80000000, float* %c4248
  store float 0x3F6F7643E0000000, float* %c5250
  store float 0x3F39E62EA0000000, float* %c6252
  store float 0x3F00DEB200000000, float* %c7254
  store float 0x3E9361D580000000, float* %c8256
  store float 0x3E9A93C500000000, float* %c9258
  %tmp259 = load i32* %boundary23                 ; <i32> [#uses=1]
  store i32 %tmp259, i32* %pos
  br label %for.cond260

for.cond260:                                      ; preds = %for.inc337, %for.end238
  %tmp261 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp262 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %cmp263 = icmp slt i32 %tmp261, %tmp262         ; <i1> [#uses=1]
  br i1 %cmp263, label %for.body265, label %for.end340

for.body265:                                      ; preds = %for.cond260
  %tmp268 = load i32* %pos                        ; <i32> [#uses=1]
  %add269 = add nsw i32 %tmp268, 1                ; <i32> [#uses=1]
  %conv270 = sitofp i32 %add269 to float          ; <float> [#uses=1]
  %tmp271 = load float* %pathNinv                 ; <float> [#uses=1]
  %mul272 = fmul float %conv270, %tmp271          ; <float> [#uses=1]
  store float %mul272, float* %p267
  %tmp275 = load float* %p267                     ; <float> [#uses=1]
  %sub276 = fsub float 1.000000e+000, %tmp275     ; <float> [#uses=1]
  %call277 = call float @_Z3logf(float %sub276)   ; <float> [#uses=1]
  %neg278 = fsub float -0.000000e+000, %call277   ; <float> [#uses=1]
  %call279 = call float @_Z3logf(float %neg278)   ; <float> [#uses=1]
  store float %call279, float* %z274
  %tmp282 = load float* %c1242                    ; <float> [#uses=1]
  %tmp283 = load float* %z274                     ; <float> [#uses=1]
  %tmp284 = load float* %c2244                    ; <float> [#uses=1]
  %tmp285 = load float* %z274                     ; <float> [#uses=1]
  %tmp286 = load float* %c3246                    ; <float> [#uses=1]
  %tmp287 = load float* %z274                     ; <float> [#uses=1]
  %tmp288 = load float* %c4248                    ; <float> [#uses=1]
  %tmp289 = load float* %z274                     ; <float> [#uses=1]
  %tmp290 = load float* %c5250                    ; <float> [#uses=1]
  %tmp291 = load float* %z274                     ; <float> [#uses=1]
  %tmp292 = load float* %c6252                    ; <float> [#uses=1]
  %tmp293 = load float* %z274                     ; <float> [#uses=1]
  %tmp294 = load float* %c7254                    ; <float> [#uses=1]
  %tmp295 = load float* %z274                     ; <float> [#uses=1]
  %tmp296 = load float* %c8256                    ; <float> [#uses=1]
  %tmp297 = load float* %z274                     ; <float> [#uses=1]
  %tmp298 = load float* %c9258                    ; <float> [#uses=1]
  %mul299 = fmul float %tmp297, %tmp298           ; <float> [#uses=1]
  %add300 = fadd float %tmp296, %mul299           ; <float> [#uses=1]
  %mul301 = fmul float %tmp295, %add300           ; <float> [#uses=1]
  %add302 = fadd float %tmp294, %mul301           ; <float> [#uses=1]
  %mul303 = fmul float %tmp293, %add302           ; <float> [#uses=1]
  %add304 = fadd float %tmp292, %mul303           ; <float> [#uses=1]
  %mul305 = fmul float %tmp291, %add304           ; <float> [#uses=1]
  %add306 = fadd float %tmp290, %mul305           ; <float> [#uses=1]
  %mul307 = fmul float %tmp289, %add306           ; <float> [#uses=1]
  %add308 = fadd float %tmp288, %mul307           ; <float> [#uses=1]
  %mul309 = fmul float %tmp287, %add308           ; <float> [#uses=1]
  %add310 = fadd float %tmp286, %mul309           ; <float> [#uses=1]
  %mul311 = fmul float %tmp285, %add310           ; <float> [#uses=1]
  %add312 = fadd float %tmp284, %mul311           ; <float> [#uses=1]
  %mul313 = fmul float %tmp283, %add312           ; <float> [#uses=1]
  %add314 = fadd float %tmp282, %mul313           ; <float> [#uses=1]
  store float %add314, float* %t281
  %tmp317 = load float* %S                        ; <float> [#uses=1]
  %tmp318 = load float* %t281                     ; <float> [#uses=1]
  %tmp319 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %mul320 = fmul float %tmp318, %tmp319           ; <float> [#uses=1]
  %tmp321 = load float* %MuByT                    ; <float> [#uses=1]
  %add322 = fadd float %mul320, %tmp321           ; <float> [#uses=1]
  %call323 = call float @_Z3expf(float %add322)   ; <float> [#uses=1]
  %mul324 = fmul float %tmp317, %call323          ; <float> [#uses=1]
  %tmp325 = load float* %X                        ; <float> [#uses=1]
  %sub326 = fsub float %mul324, %tmp325           ; <float> [#uses=1]
  store float %sub326, float* %callValue316
  %tmp327 = load float* %callValue316             ; <float> [#uses=1]
  %call328 = call float @_Z3maxff(float %tmp327, float 0.000000e+000) ; <float> [#uses=1]
  store float %call328, float* %callValue316
  %tmp329 = load float* %callValue316             ; <float> [#uses=1]
  %tmp330 = load float* %sum                      ; <float> [#uses=1]
  %add331 = fadd float %tmp330, %tmp329           ; <float> [#uses=1]
  store float %add331, float* %sum
  %tmp332 = load float* %callValue316             ; <float> [#uses=1]
  %tmp333 = load float* %callValue316             ; <float> [#uses=1]
  %mul334 = fmul float %tmp332, %tmp333           ; <float> [#uses=1]
  %tmp335 = load float* %sum2                     ; <float> [#uses=1]
  %add336 = fadd float %tmp335, %mul334           ; <float> [#uses=1]
  store float %add336, float* %sum2
  br label %for.inc337

for.inc337:                                       ; preds = %for.body265
  %tmp338 = load i32* %pos                        ; <i32> [#uses=1]
  %inc339 = add nsw i32 %tmp338, 1                ; <i32> [#uses=1]
  store i32 %inc339, i32* %pos
  br label %for.cond260

for.end340:                                       ; preds = %for.cond260
  %tmp342 = load float* %sum                      ; <float> [#uses=1]
  store float %tmp342, float* %totalsum
  %tmp344 = load float* %sum2                     ; <float> [#uses=1]
  store float %tmp344, float* %totalsum2
  %tmp346 = load float* %R                        ; <float> [#uses=1]
  %neg347 = fsub float -0.000000e+000, %tmp346    ; <float> [#uses=1]
  %tmp348 = load float* %T                        ; <float> [#uses=1]
  %mul349 = fmul float %neg347, %tmp348           ; <float> [#uses=1]
  %call350 = call float @_Z3expf(float %mul349)   ; <float> [#uses=1]
  store float %call350, float* %ExpRT
  %tmp351 = load float* %ExpRT                    ; <float> [#uses=1]
  %tmp352 = load float* %totalsum                 ; <float> [#uses=1]
  %mul353 = fmul float %tmp351, %tmp352           ; <float> [#uses=1]
  %tmp354 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv355 = sitofp i32 %tmp354 to float          ; <float> [#uses=3]
  %cmp356 = fcmp oeq float 0.000000e+000, %conv355 ; <i1> [#uses=1]
  %sel357 = select i1 %cmp356, float 1.000000e+000, float %conv355 ; <float> [#uses=0]
  %div358 = fdiv float %mul353, %conv355          ; <float> [#uses=1]
  %tmp359 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp360 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx361 = getelementptr inbounds %struct.anon addrspace(1)* %tmp360, i32 %tmp359 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp362 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx361, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %div358, float addrspace(1)* %tmp362
  %tmp364 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv365 = sitofp i32 %tmp364 to float          ; <float> [#uses=1]
  %tmp366 = load float* %totalsum2                ; <float> [#uses=1]
  %mul367 = fmul float %conv365, %tmp366          ; <float> [#uses=1]
  %tmp368 = load float* %totalsum                 ; <float> [#uses=1]
  %tmp369 = load float* %totalsum                 ; <float> [#uses=1]
  %mul370 = fmul float %tmp368, %tmp369           ; <float> [#uses=1]
  %sub371 = fsub float %mul367, %mul370           ; <float> [#uses=1]
  %tmp372 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv373 = sitofp i32 %tmp372 to float          ; <float> [#uses=1]
  %tmp374 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub375 = sub i32 %tmp374, 1                    ; <i32> [#uses=1]
  %conv376 = sitofp i32 %sub375 to float          ; <float> [#uses=1]
  %mul377 = fmul float %conv373, %conv376         ; <float> [#uses=3]
  %cmp378 = fcmp oeq float 0.000000e+000, %mul377 ; <i1> [#uses=1]
  %sel379 = select i1 %cmp378, float 1.000000e+000, float %mul377 ; <float> [#uses=0]
  %div380 = fdiv float %sub371, %mul377           ; <float> [#uses=1]
  %call381 = call float @_Z4sqrtf(float %div380)  ; <float> [#uses=1]
  store float %call381, float* %stdDev
  %tmp382 = load float* %ExpRT                    ; <float> [#uses=1]
  %conv383 = fpext float %tmp382 to double        ; <double> [#uses=1]
  %mul384 = fmul double %conv383, 1.960000e+000   ; <double> [#uses=1]
  %tmp385 = load float* %stdDev                   ; <float> [#uses=1]
  %conv386 = fpext float %tmp385 to double        ; <double> [#uses=1]
  %mul387 = fmul double %mul384, %conv386         ; <double> [#uses=1]
  %tmp388 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv389 = sitofp i32 %tmp388 to float          ; <float> [#uses=1]
  %call390 = call float @_Z4sqrtf(float %conv389) ; <float> [#uses=1]
  %conv391 = fpext float %call390 to double       ; <double> [#uses=3]
  %cmp392 = fcmp oeq double 0.000000e+000, %conv391 ; <i1> [#uses=1]
  %sel393 = select i1 %cmp392, double 1.000000e+000, double %conv391 ; <double> [#uses=0]
  %div394 = fdiv double %mul387, %conv391         ; <double> [#uses=1]
  %conv395 = fptrunc double %div394 to float      ; <float> [#uses=1]
  %tmp396 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp397 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx398 = getelementptr inbounds %struct.anon addrspace(1)* %tmp397, i32 %tmp396 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp399 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx398, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %conv395, float addrspace(1)* %tmp399
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
