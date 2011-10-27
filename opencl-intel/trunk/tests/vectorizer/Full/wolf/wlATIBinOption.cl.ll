; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIBinOption.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_binomial_options_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_binomial_options_parameters = appending global [186 x i8] c"int, float4 const __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(3))) *, float4 __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[186 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(3)*, <4 x float> addrspace(3)*)* @binomial_options to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_binomial_options_locals to i8*), i8* getelementptr inbounds ([186 x i8]* @opencl_binomial_options_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @binomial_options(i32 %numSteps, <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)* %output, <4 x float> addrspace(3)* %callA, <4 x float> addrspace(3)* %callB) nounwind {
entry:
  %numSteps.addr = alloca i32, align 4            ; <i32*> [#uses=4]
  %randArray.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %callA.addr = alloca <4 x float> addrspace(3)*, align 4 ; <<4 x float> addrspace(3)**> [#uses=9]
  %callB.addr = alloca <4 x float> addrspace(3)*, align 4 ; <<4 x float> addrspace(3)**> [#uses=4]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=15]
  %bid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %inRand = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=7]
  %s = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %x = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=2]
  %optionYears = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %dt = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %vsdt = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %rdt = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %rInv = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %u = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %d = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %pu = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %pd = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %puByr = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %pdByr = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %profit = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=9]
  %j = alloca i32, align 4                        ; <i32*> [#uses=6]
  store i32 %numSteps, i32* %numSteps.addr
  store <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)** %randArray.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store <4 x float> addrspace(3)* %callA, <4 x float> addrspace(3)** %callA.addr
  store <4 x float> addrspace(3)* %callB, <4 x float> addrspace(3)** %callB.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %call1 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %bid
  %tmp = load i32* %bid                           ; <i32> [#uses=1]
  %tmp2 = load <4 x float> addrspace(1)** %randArray.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp2, i32 %tmp ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp3 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %inRand
  %tmp5 = load <4 x float>* %inRand               ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp5 ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %sub, <float 5.000000e+000, float 5.000000e+000, float 5.000000e+000, float 5.000000e+000> ; <<4 x float>> [#uses=1]
  %tmp6 = load <4 x float>* %inRand               ; <<4 x float>> [#uses=1]
  %mul7 = fmul <4 x float> %tmp6, <float 3.000000e+001, float 3.000000e+001, float 3.000000e+001, float 3.000000e+001> ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %mul, %mul7             ; <<4 x float>> [#uses=1]
  store <4 x float> %add, <4 x float>* %s
  %tmp9 = load <4 x float>* %inRand               ; <<4 x float>> [#uses=1]
  %sub10 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp9 ; <<4 x float>> [#uses=1]
  %mul11 = fmul <4 x float> %sub10, <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000> ; <<4 x float>> [#uses=1]
  %tmp12 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul13 = fmul <4 x float> %tmp12, <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002> ; <<4 x float>> [#uses=1]
  %add14 = fadd <4 x float> %mul11, %mul13        ; <<4 x float>> [#uses=1]
  store <4 x float> %add14, <4 x float>* %x
  %tmp16 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub17 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp16 ; <<4 x float>> [#uses=1]
  %mul18 = fmul <4 x float> %sub17, <float 2.500000e-001, float 2.500000e-001, float 2.500000e-001, float 2.500000e-001> ; <<4 x float>> [#uses=1]
  %tmp19 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul20 = fmul <4 x float> %tmp19, <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001> ; <<4 x float>> [#uses=1]
  %add21 = fadd <4 x float> %mul18, %mul20        ; <<4 x float>> [#uses=1]
  store <4 x float> %add21, <4 x float>* %optionYears
  %tmp23 = load <4 x float>* %optionYears         ; <<4 x float>> [#uses=1]
  %tmp24 = load i32* %numSteps.addr               ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp24 to float              ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %conv          ; <float> [#uses=1]
  %tmp25 = insertelement <4 x float> undef, float %div, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp25, <4 x float> %tmp25, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul26 = fmul <4 x float> %tmp23, %splat        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul26, <4 x float>* %dt
  %tmp28 = load <4 x float>* %dt                  ; <<4 x float>> [#uses=1]
  %call29 = call <4 x float> @_Z4sqrtU8__vector4f(<4 x float> %tmp28) ; <<4 x float>> [#uses=1]
  %mul30 = fmul <4 x float> <float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000>, %call29 ; <<4 x float>> [#uses=1]
  store <4 x float> %mul30, <4 x float>* %vsdt
  %tmp32 = load <4 x float>* %dt                  ; <<4 x float>> [#uses=1]
  %mul33 = fmul <4 x float> <float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000>, %tmp32 ; <<4 x float>> [#uses=1]
  store <4 x float> %mul33, <4 x float>* %rdt
  %tmp35 = load <4 x float>* %rdt                 ; <<4 x float>> [#uses=1]
  %call36 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %tmp35) ; <<4 x float>> [#uses=1]
  store <4 x float> %call36, <4 x float>* %r
  %tmp38 = load <4 x float>* %r                   ; <<4 x float>> [#uses=3]
  %cmp39 = fcmp oeq <4 x float> zeroinitializer, %tmp38 ; <<4 x i1>> [#uses=1]
  %sel40 = select <4 x i1> %cmp39, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp38 ; <<4 x float>> [#uses=0]
  %div41 = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp38 ; <<4 x float>> [#uses=1]
  store <4 x float> %div41, <4 x float>* %rInv
  %tmp43 = load <4 x float>* %vsdt                ; <<4 x float>> [#uses=1]
  %call44 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %tmp43) ; <<4 x float>> [#uses=1]
  store <4 x float> %call44, <4 x float>* %u
  %tmp46 = load <4 x float>* %u                   ; <<4 x float>> [#uses=3]
  %cmp47 = fcmp oeq <4 x float> zeroinitializer, %tmp46 ; <<4 x i1>> [#uses=1]
  %sel48 = select <4 x i1> %cmp47, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp46 ; <<4 x float>> [#uses=0]
  %div49 = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp46 ; <<4 x float>> [#uses=1]
  store <4 x float> %div49, <4 x float>* %d
  %tmp51 = load <4 x float>* %r                   ; <<4 x float>> [#uses=1]
  %tmp52 = load <4 x float>* %d                   ; <<4 x float>> [#uses=1]
  %sub53 = fsub <4 x float> %tmp51, %tmp52        ; <<4 x float>> [#uses=1]
  %tmp54 = load <4 x float>* %u                   ; <<4 x float>> [#uses=1]
  %tmp55 = load <4 x float>* %d                   ; <<4 x float>> [#uses=1]
  %sub56 = fsub <4 x float> %tmp54, %tmp55        ; <<4 x float>> [#uses=3]
  %cmp57 = fcmp oeq <4 x float> zeroinitializer, %sub56 ; <<4 x i1>> [#uses=1]
  %sel58 = select <4 x i1> %cmp57, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %sub56 ; <<4 x float>> [#uses=0]
  %div59 = fdiv <4 x float> %sub53, %sub56        ; <<4 x float>> [#uses=1]
  store <4 x float> %div59, <4 x float>* %pu
  %tmp61 = load <4 x float>* %pu                  ; <<4 x float>> [#uses=1]
  %sub62 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp61 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub62, <4 x float>* %pd
  %tmp64 = load <4 x float>* %pu                  ; <<4 x float>> [#uses=1]
  %tmp65 = load <4 x float>* %rInv                ; <<4 x float>> [#uses=1]
  %mul66 = fmul <4 x float> %tmp64, %tmp65        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul66, <4 x float>* %puByr
  %tmp68 = load <4 x float>* %pd                  ; <<4 x float>> [#uses=1]
  %tmp69 = load <4 x float>* %rInv                ; <<4 x float>> [#uses=1]
  %mul70 = fmul <4 x float> %tmp68, %tmp69        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul70, <4 x float>* %pdByr
  %tmp72 = load <4 x float>* %s                   ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %vsdt                ; <<4 x float>> [#uses=1]
  %tmp74 = load i32* %tid                         ; <i32> [#uses=1]
  %conv75 = uitofp i32 %tmp74 to float            ; <float> [#uses=1]
  %mul76 = fmul float 2.000000e+000, %conv75      ; <float> [#uses=1]
  %tmp77 = load i32* %numSteps.addr               ; <i32> [#uses=1]
  %conv78 = sitofp i32 %tmp77 to float            ; <float> [#uses=1]
  %sub79 = fsub float %mul76, %conv78             ; <float> [#uses=1]
  %tmp80 = insertelement <4 x float> undef, float %sub79, i32 0 ; <<4 x float>> [#uses=2]
  %splat81 = shufflevector <4 x float> %tmp80, <4 x float> %tmp80, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul82 = fmul <4 x float> %tmp73, %splat81      ; <<4 x float>> [#uses=1]
  %call83 = call <4 x float> @_Z3expU8__vector4f(<4 x float> %mul82) ; <<4 x float>> [#uses=1]
  %mul84 = fmul <4 x float> %tmp72, %call83       ; <<4 x float>> [#uses=1]
  %tmp85 = load <4 x float>* %x                   ; <<4 x float>> [#uses=1]
  %sub86 = fsub <4 x float> %mul84, %tmp85        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub86, <4 x float>* %profit
  %tmp87 = load <4 x float>* %profit              ; <<4 x float>> [#uses=1]
  %tmp88 = extractelement <4 x float> %tmp87, i32 0 ; <float> [#uses=1]
  %cmp89 = fcmp ogt float %tmp88, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp89, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp91 = load <4 x float>* %profit              ; <<4 x float>> [#uses=1]
  %tmp92 = extractelement <4 x float> %tmp91, i32 0 ; <float> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi float [ %tmp92, %cond.true ], [ 0.000000e+000, %cond.false ] ; <float> [#uses=1]
  %tmp93 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp94 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds <4 x float> addrspace(3)* %tmp94, i32 %tmp93 ; <<4 x float> addrspace(3)*> [#uses=2]
  %tmp96 = load <4 x float> addrspace(3)* %arrayidx95 ; <<4 x float>> [#uses=1]
  %tmp97 = insertelement <4 x float> %tmp96, float %cond, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp97, <4 x float> addrspace(3)* %arrayidx95
  %tmp98 = load <4 x float>* %profit              ; <<4 x float>> [#uses=1]
  %tmp99 = extractelement <4 x float> %tmp98, i32 1 ; <float> [#uses=1]
  %cmp100 = fcmp ogt float %tmp99, 0.000000e+000  ; <i1> [#uses=1]
  br i1 %cmp100, label %cond.true102, label %cond.false105

cond.true102:                                     ; preds = %cond.end
  %tmp103 = load <4 x float>* %profit             ; <<4 x float>> [#uses=1]
  %tmp104 = extractelement <4 x float> %tmp103, i32 1 ; <float> [#uses=1]
  br label %cond.end106

cond.false105:                                    ; preds = %cond.end
  br label %cond.end106

cond.end106:                                      ; preds = %cond.false105, %cond.true102
  %cond107 = phi float [ %tmp104, %cond.true102 ], [ 0.000000e+000, %cond.false105 ] ; <float> [#uses=1]
  %tmp108 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp109 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx110 = getelementptr inbounds <4 x float> addrspace(3)* %tmp109, i32 %tmp108 ; <<4 x float> addrspace(3)*> [#uses=2]
  %tmp111 = load <4 x float> addrspace(3)* %arrayidx110 ; <<4 x float>> [#uses=1]
  %tmp112 = insertelement <4 x float> %tmp111, float %cond107, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112, <4 x float> addrspace(3)* %arrayidx110
  %tmp113 = load <4 x float>* %profit             ; <<4 x float>> [#uses=1]
  %tmp114 = extractelement <4 x float> %tmp113, i32 2 ; <float> [#uses=1]
  %cmp115 = fcmp ogt float %tmp114, 0.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp115, label %cond.true117, label %cond.false120

cond.true117:                                     ; preds = %cond.end106
  %tmp118 = load <4 x float>* %profit             ; <<4 x float>> [#uses=1]
  %tmp119 = extractelement <4 x float> %tmp118, i32 2 ; <float> [#uses=1]
  br label %cond.end121

cond.false120:                                    ; preds = %cond.end106
  br label %cond.end121

cond.end121:                                      ; preds = %cond.false120, %cond.true117
  %cond122 = phi float [ %tmp119, %cond.true117 ], [ 0.000000e+000, %cond.false120 ] ; <float> [#uses=1]
  %tmp123 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp124 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx125 = getelementptr inbounds <4 x float> addrspace(3)* %tmp124, i32 %tmp123 ; <<4 x float> addrspace(3)*> [#uses=2]
  %tmp126 = load <4 x float> addrspace(3)* %arrayidx125 ; <<4 x float>> [#uses=1]
  %tmp127 = insertelement <4 x float> %tmp126, float %cond122, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp127, <4 x float> addrspace(3)* %arrayidx125
  %tmp128 = load <4 x float>* %profit             ; <<4 x float>> [#uses=1]
  %tmp129 = extractelement <4 x float> %tmp128, i32 3 ; <float> [#uses=1]
  %cmp130 = fcmp ogt float %tmp129, 0.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp130, label %cond.true132, label %cond.false135

cond.true132:                                     ; preds = %cond.end121
  %tmp133 = load <4 x float>* %profit             ; <<4 x float>> [#uses=1]
  %tmp134 = extractelement <4 x float> %tmp133, i32 3 ; <float> [#uses=1]
  br label %cond.end136

cond.false135:                                    ; preds = %cond.end121
  br label %cond.end136

cond.end136:                                      ; preds = %cond.false135, %cond.true132
  %cond137 = phi float [ %tmp134, %cond.true132 ], [ 0.000000e+000, %cond.false135 ] ; <float> [#uses=1]
  %tmp138 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp139 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx140 = getelementptr inbounds <4 x float> addrspace(3)* %tmp139, i32 %tmp138 ; <<4 x float> addrspace(3)*> [#uses=2]
  %tmp141 = load <4 x float> addrspace(3)* %arrayidx140 ; <<4 x float>> [#uses=1]
  %tmp142 = insertelement <4 x float> %tmp141, float %cond137, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp142, <4 x float> addrspace(3)* %arrayidx140
  call void @barrier(i32 1)
  %tmp144 = load i32* %numSteps.addr              ; <i32> [#uses=1]
  store i32 %tmp144, i32* %j
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %cond.end136
  %tmp145 = load i32* %j                          ; <i32> [#uses=1]
  %cmp146 = icmp sgt i32 %tmp145, 0               ; <i1> [#uses=1]
  br i1 %cmp146, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp148 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp149 = load i32* %j                          ; <i32> [#uses=1]
  %cmp150 = icmp ult i32 %tmp148, %tmp149         ; <i1> [#uses=1]
  br i1 %cmp150, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp152 = load <4 x float>* %puByr              ; <<4 x float>> [#uses=1]
  %tmp153 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp154 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx155 = getelementptr inbounds <4 x float> addrspace(3)* %tmp154, i32 %tmp153 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp156 = load <4 x float> addrspace(3)* %arrayidx155 ; <<4 x float>> [#uses=1]
  %mul157 = fmul <4 x float> %tmp152, %tmp156     ; <<4 x float>> [#uses=1]
  %tmp158 = load <4 x float>* %pdByr              ; <<4 x float>> [#uses=1]
  %tmp159 = load i32* %tid                        ; <i32> [#uses=1]
  %add160 = add i32 %tmp159, 1                    ; <i32> [#uses=1]
  %tmp161 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx162 = getelementptr inbounds <4 x float> addrspace(3)* %tmp161, i32 %add160 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp163 = load <4 x float> addrspace(3)* %arrayidx162 ; <<4 x float>> [#uses=1]
  %mul164 = fmul <4 x float> %tmp158, %tmp163     ; <<4 x float>> [#uses=1]
  %add165 = fadd <4 x float> %mul157, %mul164     ; <<4 x float>> [#uses=1]
  %tmp166 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp167 = load <4 x float> addrspace(3)** %callB.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx168 = getelementptr inbounds <4 x float> addrspace(3)* %tmp167, i32 %tmp166 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %add165, <4 x float> addrspace(3)* %arrayidx168
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  call void @barrier(i32 1)
  %tmp169 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp170 = load i32* %j                          ; <i32> [#uses=1]
  %sub171 = sub i32 %tmp170, 1                    ; <i32> [#uses=1]
  %cmp172 = icmp ult i32 %tmp169, %sub171         ; <i1> [#uses=1]
  br i1 %cmp172, label %if.then174, label %if.end192

if.then174:                                       ; preds = %if.end
  %tmp175 = load <4 x float>* %puByr              ; <<4 x float>> [#uses=1]
  %tmp176 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp177 = load <4 x float> addrspace(3)** %callB.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx178 = getelementptr inbounds <4 x float> addrspace(3)* %tmp177, i32 %tmp176 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp179 = load <4 x float> addrspace(3)* %arrayidx178 ; <<4 x float>> [#uses=1]
  %mul180 = fmul <4 x float> %tmp175, %tmp179     ; <<4 x float>> [#uses=1]
  %tmp181 = load <4 x float>* %pdByr              ; <<4 x float>> [#uses=1]
  %tmp182 = load i32* %tid                        ; <i32> [#uses=1]
  %add183 = add i32 %tmp182, 1                    ; <i32> [#uses=1]
  %tmp184 = load <4 x float> addrspace(3)** %callB.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx185 = getelementptr inbounds <4 x float> addrspace(3)* %tmp184, i32 %add183 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp186 = load <4 x float> addrspace(3)* %arrayidx185 ; <<4 x float>> [#uses=1]
  %mul187 = fmul <4 x float> %tmp181, %tmp186     ; <<4 x float>> [#uses=1]
  %add188 = fadd <4 x float> %mul180, %mul187     ; <<4 x float>> [#uses=1]
  %tmp189 = load i32* %tid                        ; <i32> [#uses=1]
  %tmp190 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx191 = getelementptr inbounds <4 x float> addrspace(3)* %tmp190, i32 %tmp189 ; <<4 x float> addrspace(3)*> [#uses=1]
  store <4 x float> %add188, <4 x float> addrspace(3)* %arrayidx191
  br label %if.end192

if.end192:                                        ; preds = %if.then174, %if.end
  call void @barrier(i32 1)
  br label %for.inc

for.inc:                                          ; preds = %if.end192
  %tmp193 = load i32* %j                          ; <i32> [#uses=1]
  %sub194 = sub i32 %tmp193, 2                    ; <i32> [#uses=1]
  store i32 %sub194, i32* %j
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp195 = load i32* %tid                        ; <i32> [#uses=1]
  %cmp196 = icmp eq i32 %tmp195, 0                ; <i1> [#uses=1]
  br i1 %cmp196, label %if.then198, label %if.end205

if.then198:                                       ; preds = %for.end
  %tmp199 = load <4 x float> addrspace(3)** %callA.addr ; <<4 x float> addrspace(3)*> [#uses=1]
  %arrayidx200 = getelementptr inbounds <4 x float> addrspace(3)* %tmp199, i32 0 ; <<4 x float> addrspace(3)*> [#uses=1]
  %tmp201 = load <4 x float> addrspace(3)* %arrayidx200 ; <<4 x float>> [#uses=1]
  %tmp202 = load i32* %bid                        ; <i32> [#uses=1]
  %tmp203 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx204 = getelementptr inbounds <4 x float> addrspace(1)* %tmp203, i32 %tmp202 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp201, <4 x float> addrspace(1)* %arrayidx204
  br label %if.end205

if.end205:                                        ; preds = %if.then198, %for.end
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare <4 x float> @_Z4sqrtU8__vector4f(<4 x float>)

declare <4 x float> @_Z3expU8__vector4f(<4 x float>)

declare void @barrier(i32)
