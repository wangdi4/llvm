; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlMonteCarlo_vector.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%0 = type { float, float, float, float, float }
%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type { float, float }

@opencl_wlMonteCarloKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlMonteCarloKernel_parameters = appending global [109 x i8] c"TOptionValue __attribute__((address_space(1))) *, TOptionData __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[109 x i8]*> [#uses=1]
@opencl_wlMonteCarloKernel_PrecalculatedSamples_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlMonteCarloKernel_PrecalculatedSamples_parameters = appending global [153 x i8] c"TOptionValue __attribute__((address_space(1))) *, TOptionData __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[153 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct.anon addrspace(1)*, %0 addrspace(1)*, i32)* @wlMonteCarloKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlMonteCarloKernel_locals to i8*), i8* getelementptr inbounds ([109 x i8]* @opencl_wlMonteCarloKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct.anon addrspace(1)*, %0 addrspace(1)*, <4 x float> addrspace(1)*, i32)* @wlMonteCarloKernel_PrecalculatedSamples to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlMonteCarloKernel_PrecalculatedSamples_locals to i8*), i8* getelementptr inbounds ([153 x i8]* @opencl_wlMonteCarloKernel_PrecalculatedSamples_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @NormalDistributionBatch1(<4 x float>* %pDist, i32 %i, i32 %j, i32 %pathN, i32 %threadId) nounwind {
entry:
  %pDist.addr = alloca <4 x float>*, align 4      ; <<4 x float>**> [#uses=3]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=6]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=5]
  %threadId.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %pos4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %.compoundliteral = alloca <4 x i32>, align 16  ; <<4 x i32>*> [#uses=2]
  %pos4pl1 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=7]
  %pathNinv = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %.compoundliteral14 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=5]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral31 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral34 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral37 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral40 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c5 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral43 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c6 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral46 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c7 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral49 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c8 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral52 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c9 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral55 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=4]
  %p = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %y = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=0]
  %z = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=22]
  store <4 x float>* %pDist, <4 x float>** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  store i32 %threadId, i32* %threadId.addr
  %tmp = load i32* %i.addr                        ; <i32> [#uses=1]
  %vecinit = insertelement <4 x i32> undef, i32 %tmp, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp1 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add = add i32 %tmp1, 1                         ; <i32> [#uses=1]
  %vecinit2 = insertelement <4 x i32> %vecinit, i32 %add, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp3 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add4 = add i32 %tmp3, 2                        ; <i32> [#uses=1]
  %vecinit5 = insertelement <4 x i32> %vecinit2, i32 %add4, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp6 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add7 = add i32 %tmp6, 3                        ; <i32> [#uses=1]
  %vecinit8 = insertelement <4 x i32> %vecinit5, i32 %add7, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit8, <4 x i32>* %.compoundliteral
  %tmp9 = load <4 x i32>* %.compoundliteral       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp9, <4 x i32>* %pos4
  %tmp11 = load <4 x i32>* %pos4                  ; <<4 x i32>> [#uses=1]
  %add12 = add nsw <4 x i32> %tmp11, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  %call = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %add12) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %pos4pl1
  %tmp15 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add16 = add i32 %tmp15, 1                      ; <i32> [#uses=1]
  %vecinit17 = insertelement <4 x i32> undef, i32 %add16, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp18 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add19 = add i32 %tmp18, 1                      ; <i32> [#uses=1]
  %vecinit20 = insertelement <4 x i32> %vecinit17, i32 %add19, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp21 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add22 = add i32 %tmp21, 1                      ; <i32> [#uses=1]
  %vecinit23 = insertelement <4 x i32> %vecinit20, i32 %add22, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp24 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add25 = add i32 %tmp24, 1                      ; <i32> [#uses=1]
  %vecinit26 = insertelement <4 x i32> %vecinit23, i32 %add25, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit26, <4 x i32>* %.compoundliteral14
  %tmp27 = load <4 x i32>* %.compoundliteral14    ; <<4 x i32>> [#uses=1]
  %call28 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %tmp27) ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %call28 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %call28 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %call28 ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %pathNinv
  store i32 0, i32* %index
  store <4 x float> <float 0x3FD59932C0000000, float 0x3FD59932C0000000, float 0x3FD59932C0000000, float 0x3FD59932C0000000>, <4 x float>* %.compoundliteral31
  %tmp32 = load <4 x float>* %.compoundliteral31  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %c1
  store <4 x float> <float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000>, <4 x float>* %.compoundliteral34
  %tmp35 = load <4 x float>* %.compoundliteral34  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp35, <4 x float>* %c2
  store <4 x float> <float 0x3FC4950720000000, float 0x3FC4950720000000, float 0x3FC4950720000000, float 0x3FC4950720000000>, <4 x float>* %.compoundliteral37
  %tmp38 = load <4 x float>* %.compoundliteral37  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp38, <4 x float>* %c3
  store <4 x float> <float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000>, <4 x float>* %.compoundliteral40
  %tmp41 = load <4 x float>* %.compoundliteral40  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %c4
  store <4 x float> <float 0x3F6F7643E0000000, float 0x3F6F7643E0000000, float 0x3F6F7643E0000000, float 0x3F6F7643E0000000>, <4 x float>* %.compoundliteral43
  %tmp44 = load <4 x float>* %.compoundliteral43  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp44, <4 x float>* %c5
  store <4 x float> <float 0x3F39E62EA0000000, float 0x3F39E62EA0000000, float 0x3F39E62EA0000000, float 0x3F39E62EA0000000>, <4 x float>* %.compoundliteral46
  %tmp47 = load <4 x float>* %.compoundliteral46  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %c6
  store <4 x float> <float 0x3F00DEB200000000, float 0x3F00DEB200000000, float 0x3F00DEB200000000, float 0x3F00DEB200000000>, <4 x float>* %.compoundliteral49
  %tmp50 = load <4 x float>* %.compoundliteral49  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp50, <4 x float>* %c7
  store <4 x float> <float 0x3E9361D580000000, float 0x3E9361D580000000, float 0x3E9361D580000000, float 0x3E9361D580000000>, <4 x float>* %.compoundliteral52
  %tmp53 = load <4 x float>* %.compoundliteral52  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp53, <4 x float>* %c8
  store <4 x float> <float 0x3E9A93C500000000, float 0x3E9A93C500000000, float 0x3E9A93C500000000, float 0x3E9A93C500000000>, <4 x float>* %.compoundliteral55
  %tmp56 = load <4 x float>* %.compoundliteral55  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %c9
  %tmp58 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp58, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp59 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp60 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp61 = icmp ule i32 %tmp59, %tmp60            ; <i1> [#uses=1]
  br i1 %cmp61, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp65 = load <4 x float>* %pos4pl1             ; <<4 x float>> [#uses=1]
  %tmp66 = load <4 x float>* %pathNinv            ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp65, %tmp66          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %p
  %tmp67 = load <4 x float>* %p                   ; <<4 x float>> [#uses=1]
  %call68 = call <4 x float> @_Z3logDv4_f(<4 x float> %tmp67) ; <<4 x float>> [#uses=1]
  %neg = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %call68 ; <<4 x float>> [#uses=1]
  %call69 = call <4 x float> @_Z3logDv4_f(<4 x float> %neg) ; <<4 x float>> [#uses=1]
  store <4 x float> %call69, <4 x float>* %z
  %tmp70 = load <4 x float>* %c1                  ; <<4 x float>> [#uses=1]
  %tmp71 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp72 = load <4 x float>* %c2                  ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp74 = load <4 x float>* %c3                  ; <<4 x float>> [#uses=1]
  %tmp75 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %c4                  ; <<4 x float>> [#uses=1]
  %tmp77 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp78 = load <4 x float>* %c5                  ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %c6                  ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp82 = load <4 x float>* %c7                  ; <<4 x float>> [#uses=1]
  %tmp83 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp84 = load <4 x float>* %c8                  ; <<4 x float>> [#uses=1]
  %tmp85 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp86 = load <4 x float>* %c9                  ; <<4 x float>> [#uses=1]
  %mul87 = fmul <4 x float> %tmp85, %tmp86        ; <<4 x float>> [#uses=1]
  %add88 = fadd <4 x float> %tmp84, %mul87        ; <<4 x float>> [#uses=1]
  %mul89 = fmul <4 x float> %tmp83, %add88        ; <<4 x float>> [#uses=1]
  %add90 = fadd <4 x float> %tmp82, %mul89        ; <<4 x float>> [#uses=1]
  %mul91 = fmul <4 x float> %tmp81, %add90        ; <<4 x float>> [#uses=1]
  %add92 = fadd <4 x float> %tmp80, %mul91        ; <<4 x float>> [#uses=1]
  %mul93 = fmul <4 x float> %tmp79, %add92        ; <<4 x float>> [#uses=1]
  %add94 = fadd <4 x float> %tmp78, %mul93        ; <<4 x float>> [#uses=1]
  %mul95 = fmul <4 x float> %tmp77, %add94        ; <<4 x float>> [#uses=1]
  %add96 = fadd <4 x float> %tmp76, %mul95        ; <<4 x float>> [#uses=1]
  %mul97 = fmul <4 x float> %tmp75, %add96        ; <<4 x float>> [#uses=1]
  %add98 = fadd <4 x float> %tmp74, %mul97        ; <<4 x float>> [#uses=1]
  %mul99 = fmul <4 x float> %tmp73, %add98        ; <<4 x float>> [#uses=1]
  %add100 = fadd <4 x float> %tmp72, %mul99       ; <<4 x float>> [#uses=1]
  %mul101 = fmul <4 x float> %tmp71, %add100      ; <<4 x float>> [#uses=1]
  %add102 = fadd <4 x float> %tmp70, %mul101      ; <<4 x float>> [#uses=1]
  store <4 x float> %add102, <4 x float>* %z
  %tmp103 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %neg104 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp103 ; <<4 x float>> [#uses=1]
  %tmp105 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp105, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp106 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp106, i32 %tmp105 ; <<4 x float>*> [#uses=1]
  store <4 x float> %neg104, <4 x float>* %arrayidx
  %tmp107 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add108 = fadd <4 x float> %tmp107, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add108, <4 x float>* %pos4pl1
  %tmp109 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %tmp110 = load <4 x float>* %pathNinv           ; <<4 x float>> [#uses=1]
  %mul111 = fmul <4 x float> %tmp109, %tmp110     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul111, <4 x float>* %p
  %tmp112 = load <4 x float>* %p                  ; <<4 x float>> [#uses=1]
  %call113 = call <4 x float> @_Z3logDv4_f(<4 x float> %tmp112) ; <<4 x float>> [#uses=1]
  %neg114 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %call113 ; <<4 x float>> [#uses=1]
  %call115 = call <4 x float> @_Z3logDv4_f(<4 x float> %neg114) ; <<4 x float>> [#uses=1]
  store <4 x float> %call115, <4 x float>* %z
  %tmp116 = load <4 x float>* %c1                 ; <<4 x float>> [#uses=1]
  %tmp117 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp118 = load <4 x float>* %c2                 ; <<4 x float>> [#uses=1]
  %tmp119 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp120 = load <4 x float>* %c3                 ; <<4 x float>> [#uses=1]
  %tmp121 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %c4                 ; <<4 x float>> [#uses=1]
  %tmp123 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp124 = load <4 x float>* %c5                 ; <<4 x float>> [#uses=1]
  %tmp125 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp126 = load <4 x float>* %c6                 ; <<4 x float>> [#uses=1]
  %tmp127 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp128 = load <4 x float>* %c7                 ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp130 = load <4 x float>* %c8                 ; <<4 x float>> [#uses=1]
  %tmp131 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp132 = load <4 x float>* %c9                 ; <<4 x float>> [#uses=1]
  %mul133 = fmul <4 x float> %tmp131, %tmp132     ; <<4 x float>> [#uses=1]
  %add134 = fadd <4 x float> %tmp130, %mul133     ; <<4 x float>> [#uses=1]
  %mul135 = fmul <4 x float> %tmp129, %add134     ; <<4 x float>> [#uses=1]
  %add136 = fadd <4 x float> %tmp128, %mul135     ; <<4 x float>> [#uses=1]
  %mul137 = fmul <4 x float> %tmp127, %add136     ; <<4 x float>> [#uses=1]
  %add138 = fadd <4 x float> %tmp126, %mul137     ; <<4 x float>> [#uses=1]
  %mul139 = fmul <4 x float> %tmp125, %add138     ; <<4 x float>> [#uses=1]
  %add140 = fadd <4 x float> %tmp124, %mul139     ; <<4 x float>> [#uses=1]
  %mul141 = fmul <4 x float> %tmp123, %add140     ; <<4 x float>> [#uses=1]
  %add142 = fadd <4 x float> %tmp122, %mul141     ; <<4 x float>> [#uses=1]
  %mul143 = fmul <4 x float> %tmp121, %add142     ; <<4 x float>> [#uses=1]
  %add144 = fadd <4 x float> %tmp120, %mul143     ; <<4 x float>> [#uses=1]
  %mul145 = fmul <4 x float> %tmp119, %add144     ; <<4 x float>> [#uses=1]
  %add146 = fadd <4 x float> %tmp118, %mul145     ; <<4 x float>> [#uses=1]
  %mul147 = fmul <4 x float> %tmp117, %add146     ; <<4 x float>> [#uses=1]
  %add148 = fadd <4 x float> %tmp116, %mul147     ; <<4 x float>> [#uses=1]
  store <4 x float> %add148, <4 x float>* %z
  %tmp149 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %neg150 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp149 ; <<4 x float>> [#uses=1]
  %tmp151 = load i32* %index                      ; <i32> [#uses=2]
  %inc152 = add nsw i32 %tmp151, 1                ; <i32> [#uses=1]
  store i32 %inc152, i32* %index
  %tmp153 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx154 = getelementptr inbounds <4 x float>* %tmp153, i32 %tmp151 ; <<4 x float>*> [#uses=1]
  store <4 x float> %neg150, <4 x float>* %arrayidx154
  %tmp155 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add156 = fadd <4 x float> %tmp155, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add156, <4 x float>* %pos4pl1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp157 = load i32* %pos                        ; <i32> [#uses=1]
  %add158 = add nsw i32 %tmp157, 8                ; <i32> [#uses=1]
  store i32 %add158, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare <4 x float> @_Z14convert_float4Dv4_i(<4 x i32>)

declare <4 x float> @_Z3logDv4_f(<4 x float>)

; CHECK: ret
define void @NormalDistributionBatch2(<4 x float>* %pDist, i32 %i, i32 %j, i32 %pathN, i32 %threadId) nounwind {
entry:
  %pDist.addr = alloca <4 x float>*, align 4      ; <<4 x float>**> [#uses=3]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=6]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=5]
  %threadId.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %pos4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %.compoundliteral = alloca <4 x i32>, align 16  ; <<4 x i32>*> [#uses=2]
  %pos4pl1 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=7]
  %pathNinv = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %.compoundliteral14 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=5]
  %a1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral31 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %a2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral34 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %a3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral37 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %a4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral40 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %b1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral43 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %b2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral46 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %b3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral49 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %b4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral52 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=4]
  %p = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %y = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=8]
  %z = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=16]
  store <4 x float>* %pDist, <4 x float>** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  store i32 %threadId, i32* %threadId.addr
  %tmp = load i32* %i.addr                        ; <i32> [#uses=1]
  %vecinit = insertelement <4 x i32> undef, i32 %tmp, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp1 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add = add i32 %tmp1, 1                         ; <i32> [#uses=1]
  %vecinit2 = insertelement <4 x i32> %vecinit, i32 %add, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp3 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add4 = add i32 %tmp3, 2                        ; <i32> [#uses=1]
  %vecinit5 = insertelement <4 x i32> %vecinit2, i32 %add4, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp6 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add7 = add i32 %tmp6, 3                        ; <i32> [#uses=1]
  %vecinit8 = insertelement <4 x i32> %vecinit5, i32 %add7, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit8, <4 x i32>* %.compoundliteral
  %tmp9 = load <4 x i32>* %.compoundliteral       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp9, <4 x i32>* %pos4
  %tmp11 = load <4 x i32>* %pos4                  ; <<4 x i32>> [#uses=1]
  %add12 = add nsw <4 x i32> %tmp11, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  %call = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %add12) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %pos4pl1
  %tmp15 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add16 = add i32 %tmp15, 1                      ; <i32> [#uses=1]
  %vecinit17 = insertelement <4 x i32> undef, i32 %add16, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp18 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add19 = add i32 %tmp18, 1                      ; <i32> [#uses=1]
  %vecinit20 = insertelement <4 x i32> %vecinit17, i32 %add19, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp21 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add22 = add i32 %tmp21, 1                      ; <i32> [#uses=1]
  %vecinit23 = insertelement <4 x i32> %vecinit20, i32 %add22, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp24 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add25 = add i32 %tmp24, 1                      ; <i32> [#uses=1]
  %vecinit26 = insertelement <4 x i32> %vecinit23, i32 %add25, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit26, <4 x i32>* %.compoundliteral14
  %tmp27 = load <4 x i32>* %.compoundliteral14    ; <<4 x i32>> [#uses=1]
  %call28 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %tmp27) ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %call28 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %call28 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %call28 ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %pathNinv
  store i32 0, i32* %index
  store <4 x float> <float 0x40040D9320000000, float 0x40040D9320000000, float 0x40040D9320000000, float 0x40040D9320000000>, <4 x float>* %.compoundliteral31
  %tmp32 = load <4 x float>* %.compoundliteral31  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %a1
  store <4 x float> <float 0xC0329D70A0000000, float 0xC0329D70A0000000, float 0xC0329D70A0000000, float 0xC0329D70A0000000>, <4 x float>* %.compoundliteral34
  %tmp35 = load <4 x float>* %.compoundliteral34  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp35, <4 x float>* %a2
  store <4 x float> <float 0x4044B212C0000000, float 0x4044B212C0000000, float 0x4044B212C0000000, float 0x4044B212C0000000>, <4 x float>* %.compoundliteral37
  %tmp38 = load <4 x float>* %.compoundliteral37  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp38, <4 x float>* %a3
  store <4 x float> <float 0xC03970E960000000, float 0xC03970E960000000, float 0xC03970E960000000, float 0xC03970E960000000>, <4 x float>* %.compoundliteral40
  %tmp41 = load <4 x float>* %.compoundliteral40  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %a4
  store <4 x float> <float 0xC020F27000000000, float 0xC020F27000000000, float 0xC020F27000000000, float 0xC020F27000000000>, <4 x float>* %.compoundliteral43
  %tmp44 = load <4 x float>* %.compoundliteral43  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp44, <4 x float>* %b1
  store <4 x float> <float 0x40371557A0000000, float 0x40371557A0000000, float 0x40371557A0000000, float 0x40371557A0000000>, <4 x float>* %.compoundliteral46
  %tmp47 = load <4 x float>* %.compoundliteral46  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %b2
  store <4 x float> <float 0xC0350FEF00000000, float 0xC0350FEF00000000, float 0xC0350FEF00000000, float 0xC0350FEF00000000>, <4 x float>* %.compoundliteral49
  %tmp50 = load <4 x float>* %.compoundliteral49  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp50, <4 x float>* %b3
  store <4 x float> <float 0x40090BF020000000, float 0x40090BF020000000, float 0x40090BF020000000, float 0x40090BF020000000>, <4 x float>* %.compoundliteral52
  %tmp53 = load <4 x float>* %.compoundliteral52  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp53, <4 x float>* %b4
  %tmp55 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp55, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp56 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp57 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp58 = icmp ule i32 %tmp56, %tmp57            ; <i1> [#uses=1]
  br i1 %cmp58, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp62 = load <4 x float>* %pos4pl1             ; <<4 x float>> [#uses=1]
  %tmp63 = load <4 x float>* %pathNinv            ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp62, %tmp63          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %p
  %tmp64 = load <4 x float>* %p                   ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp64, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001> ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %y
  %tmp65 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp66 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %mul67 = fmul <4 x float> %tmp65, %tmp66        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul67, <4 x float>* %z
  %tmp68 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp69 = load <4 x float>* %a4                  ; <<4 x float>> [#uses=1]
  %tmp70 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul71 = fmul <4 x float> %tmp69, %tmp70        ; <<4 x float>> [#uses=1]
  %tmp72 = load <4 x float>* %a3                  ; <<4 x float>> [#uses=1]
  %add73 = fadd <4 x float> %mul71, %tmp72        ; <<4 x float>> [#uses=1]
  %tmp74 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul75 = fmul <4 x float> %add73, %tmp74        ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %a2                  ; <<4 x float>> [#uses=1]
  %add77 = fadd <4 x float> %mul75, %tmp76        ; <<4 x float>> [#uses=1]
  %tmp78 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul79 = fmul <4 x float> %add77, %tmp78        ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %a1                  ; <<4 x float>> [#uses=1]
  %add81 = fadd <4 x float> %mul79, %tmp80        ; <<4 x float>> [#uses=1]
  %mul82 = fmul <4 x float> %tmp68, %add81        ; <<4 x float>> [#uses=1]
  %tmp83 = load <4 x float>* %b4                  ; <<4 x float>> [#uses=1]
  %tmp84 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul85 = fmul <4 x float> %tmp83, %tmp84        ; <<4 x float>> [#uses=1]
  %tmp86 = load <4 x float>* %b3                  ; <<4 x float>> [#uses=1]
  %add87 = fadd <4 x float> %mul85, %tmp86        ; <<4 x float>> [#uses=1]
  %tmp88 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul89 = fmul <4 x float> %add87, %tmp88        ; <<4 x float>> [#uses=1]
  %tmp90 = load <4 x float>* %b2                  ; <<4 x float>> [#uses=1]
  %add91 = fadd <4 x float> %mul89, %tmp90        ; <<4 x float>> [#uses=1]
  %tmp92 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul93 = fmul <4 x float> %add91, %tmp92        ; <<4 x float>> [#uses=1]
  %tmp94 = load <4 x float>* %b1                  ; <<4 x float>> [#uses=1]
  %add95 = fadd <4 x float> %mul93, %tmp94        ; <<4 x float>> [#uses=1]
  %tmp96 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %mul97 = fmul <4 x float> %add95, %tmp96        ; <<4 x float>> [#uses=1]
  %add98 = fadd <4 x float> %mul97, <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000> ; <<4 x float>> [#uses=3]
  %cmp99 = fcmp oeq <4 x float> zeroinitializer, %add98 ; <<4 x i1>> [#uses=1]
  %sel100 = select <4 x i1> %cmp99, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %add98 ; <<4 x float>> [#uses=0]
  %div101 = fdiv <4 x float> %mul82, %add98       ; <<4 x float>> [#uses=1]
  %tmp102 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp102, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp103 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp103, i32 %tmp102 ; <<4 x float>*> [#uses=1]
  store <4 x float> %div101, <4 x float>* %arrayidx
  %tmp104 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add105 = fadd <4 x float> %tmp104, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add105, <4 x float>* %pos4pl1
  %tmp106 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %tmp107 = load <4 x float>* %pathNinv           ; <<4 x float>> [#uses=1]
  %mul108 = fmul <4 x float> %tmp106, %tmp107     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul108, <4 x float>* %p
  %tmp109 = load <4 x float>* %p                  ; <<4 x float>> [#uses=1]
  %sub110 = fsub <4 x float> %tmp109, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001> ; <<4 x float>> [#uses=1]
  store <4 x float> %sub110, <4 x float>* %y
  %tmp111 = load <4 x float>* %y                  ; <<4 x float>> [#uses=1]
  %tmp112 = load <4 x float>* %y                  ; <<4 x float>> [#uses=1]
  %mul113 = fmul <4 x float> %tmp111, %tmp112     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul113, <4 x float>* %z
  %tmp114 = load <4 x float>* %y                  ; <<4 x float>> [#uses=1]
  %tmp115 = load <4 x float>* %a4                 ; <<4 x float>> [#uses=1]
  %tmp116 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul117 = fmul <4 x float> %tmp115, %tmp116     ; <<4 x float>> [#uses=1]
  %tmp118 = load <4 x float>* %a3                 ; <<4 x float>> [#uses=1]
  %add119 = fadd <4 x float> %mul117, %tmp118     ; <<4 x float>> [#uses=1]
  %tmp120 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul121 = fmul <4 x float> %add119, %tmp120     ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %a2                 ; <<4 x float>> [#uses=1]
  %add123 = fadd <4 x float> %mul121, %tmp122     ; <<4 x float>> [#uses=1]
  %tmp124 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul125 = fmul <4 x float> %add123, %tmp124     ; <<4 x float>> [#uses=1]
  %tmp126 = load <4 x float>* %a1                 ; <<4 x float>> [#uses=1]
  %add127 = fadd <4 x float> %mul125, %tmp126     ; <<4 x float>> [#uses=1]
  %mul128 = fmul <4 x float> %tmp114, %add127     ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %b4                 ; <<4 x float>> [#uses=1]
  %tmp130 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul131 = fmul <4 x float> %tmp129, %tmp130     ; <<4 x float>> [#uses=1]
  %tmp132 = load <4 x float>* %b3                 ; <<4 x float>> [#uses=1]
  %add133 = fadd <4 x float> %mul131, %tmp132     ; <<4 x float>> [#uses=1]
  %tmp134 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul135 = fmul <4 x float> %add133, %tmp134     ; <<4 x float>> [#uses=1]
  %tmp136 = load <4 x float>* %b2                 ; <<4 x float>> [#uses=1]
  %add137 = fadd <4 x float> %mul135, %tmp136     ; <<4 x float>> [#uses=1]
  %tmp138 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul139 = fmul <4 x float> %add137, %tmp138     ; <<4 x float>> [#uses=1]
  %tmp140 = load <4 x float>* %b1                 ; <<4 x float>> [#uses=1]
  %add141 = fadd <4 x float> %mul139, %tmp140     ; <<4 x float>> [#uses=1]
  %tmp142 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %mul143 = fmul <4 x float> %add141, %tmp142     ; <<4 x float>> [#uses=1]
  %add144 = fadd <4 x float> %mul143, <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000> ; <<4 x float>> [#uses=3]
  %cmp145 = fcmp oeq <4 x float> zeroinitializer, %add144 ; <<4 x i1>> [#uses=1]
  %sel146 = select <4 x i1> %cmp145, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %add144 ; <<4 x float>> [#uses=0]
  %div147 = fdiv <4 x float> %mul128, %add144     ; <<4 x float>> [#uses=1]
  %tmp148 = load i32* %index                      ; <i32> [#uses=2]
  %inc149 = add nsw i32 %tmp148, 1                ; <i32> [#uses=1]
  store i32 %inc149, i32* %index
  %tmp150 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx151 = getelementptr inbounds <4 x float>* %tmp150, i32 %tmp148 ; <<4 x float>*> [#uses=1]
  store <4 x float> %div147, <4 x float>* %arrayidx151
  %tmp152 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add153 = fadd <4 x float> %tmp152, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add153, <4 x float>* %pos4pl1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp154 = load i32* %pos                        ; <i32> [#uses=1]
  %add155 = add nsw i32 %tmp154, 8                ; <i32> [#uses=1]
  store i32 %add155, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @NormalDistributionBatch3(<4 x float>* %pDist, i32 %i, i32 %j, i32 %pathN, i32 %threadId, i32 %optionid) nounwind {
entry:
  %pDist.addr = alloca <4 x float>*, align 4      ; <<4 x float>**> [#uses=3]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=6]
  %j.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=5]
  %threadId.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %optionid.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %pos4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %.compoundliteral = alloca <4 x i32>, align 16  ; <<4 x i32>*> [#uses=2]
  %pos4pl1 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=7]
  %pathNinv = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %.compoundliteral14 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=5]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral31 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral34 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral37 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral40 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c5 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral43 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c6 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral46 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c7 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral49 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c8 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral52 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %c9 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral55 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=4]
  %p = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %y = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=0]
  %z = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=18]
  store <4 x float>* %pDist, <4 x float>** %pDist.addr
  store i32 %i, i32* %i.addr
  store i32 %j, i32* %j.addr
  store i32 %pathN, i32* %pathN.addr
  store i32 %threadId, i32* %threadId.addr
  store i32 %optionid, i32* %optionid.addr
  %tmp = load i32* %i.addr                        ; <i32> [#uses=1]
  %vecinit = insertelement <4 x i32> undef, i32 %tmp, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp1 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add = add i32 %tmp1, 1                         ; <i32> [#uses=1]
  %vecinit2 = insertelement <4 x i32> %vecinit, i32 %add, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp3 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add4 = add i32 %tmp3, 2                        ; <i32> [#uses=1]
  %vecinit5 = insertelement <4 x i32> %vecinit2, i32 %add4, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp6 = load i32* %i.addr                       ; <i32> [#uses=1]
  %add7 = add i32 %tmp6, 3                        ; <i32> [#uses=1]
  %vecinit8 = insertelement <4 x i32> %vecinit5, i32 %add7, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit8, <4 x i32>* %.compoundliteral
  %tmp9 = load <4 x i32>* %.compoundliteral       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp9, <4 x i32>* %pos4
  %tmp11 = load <4 x i32>* %pos4                  ; <<4 x i32>> [#uses=1]
  %add12 = add nsw <4 x i32> %tmp11, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  %call = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %add12) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %pos4pl1
  %tmp15 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add16 = add i32 %tmp15, 1                      ; <i32> [#uses=1]
  %vecinit17 = insertelement <4 x i32> undef, i32 %add16, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp18 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add19 = add i32 %tmp18, 1                      ; <i32> [#uses=1]
  %vecinit20 = insertelement <4 x i32> %vecinit17, i32 %add19, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp21 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add22 = add i32 %tmp21, 1                      ; <i32> [#uses=1]
  %vecinit23 = insertelement <4 x i32> %vecinit20, i32 %add22, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp24 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %add25 = add i32 %tmp24, 1                      ; <i32> [#uses=1]
  %vecinit26 = insertelement <4 x i32> %vecinit23, i32 %add25, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit26, <4 x i32>* %.compoundliteral14
  %tmp27 = load <4 x i32>* %.compoundliteral14    ; <<4 x i32>> [#uses=1]
  %call28 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %tmp27) ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %call28 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %call28 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %call28 ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %pathNinv
  store i32 0, i32* %index
  store <4 x float> <float 0x3FD59932C0000000, float 0x3FD59932C0000000, float 0x3FD59932C0000000, float 0x3FD59932C0000000>, <4 x float>* %.compoundliteral31
  %tmp32 = load <4 x float>* %.compoundliteral31  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %c1
  store <4 x float> <float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000, float 0x3FEF3CC6C0000000>, <4 x float>* %.compoundliteral34
  %tmp35 = load <4 x float>* %.compoundliteral34  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp35, <4 x float>* %c2
  store <4 x float> <float 0x3FC4950720000000, float 0x3FC4950720000000, float 0x3FC4950720000000, float 0x3FC4950720000000>, <4 x float>* %.compoundliteral37
  %tmp38 = load <4 x float>* %.compoundliteral37  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp38, <4 x float>* %c3
  store <4 x float> <float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000, float 0x3F9C4EAD80000000>, <4 x float>* %.compoundliteral40
  %tmp41 = load <4 x float>* %.compoundliteral40  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %c4
  store <4 x float> <float 0x3F6F7643E0000000, float 0x3F6F7643E0000000, float 0x3F6F7643E0000000, float 0x3F6F7643E0000000>, <4 x float>* %.compoundliteral43
  %tmp44 = load <4 x float>* %.compoundliteral43  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp44, <4 x float>* %c5
  store <4 x float> <float 0x3F39E62EA0000000, float 0x3F39E62EA0000000, float 0x3F39E62EA0000000, float 0x3F39E62EA0000000>, <4 x float>* %.compoundliteral46
  %tmp47 = load <4 x float>* %.compoundliteral46  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %c6
  store <4 x float> <float 0x3F00DEB200000000, float 0x3F00DEB200000000, float 0x3F00DEB200000000, float 0x3F00DEB200000000>, <4 x float>* %.compoundliteral49
  %tmp50 = load <4 x float>* %.compoundliteral49  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp50, <4 x float>* %c7
  store <4 x float> <float 0x3E9361D580000000, float 0x3E9361D580000000, float 0x3E9361D580000000, float 0x3E9361D580000000>, <4 x float>* %.compoundliteral52
  %tmp53 = load <4 x float>* %.compoundliteral52  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp53, <4 x float>* %c8
  store <4 x float> <float 0x3E9A93C500000000, float 0x3E9A93C500000000, float 0x3E9A93C500000000, float 0x3E9A93C500000000>, <4 x float>* %.compoundliteral55
  %tmp56 = load <4 x float>* %.compoundliteral55  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %c9
  %tmp58 = load i32* %i.addr                      ; <i32> [#uses=1]
  store i32 %tmp58, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp59 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp60 = load i32* %j.addr                      ; <i32> [#uses=1]
  %cmp61 = icmp ule i32 %tmp59, %tmp60            ; <i1> [#uses=1]
  br i1 %cmp61, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp65 = load <4 x float>* %pos4pl1             ; <<4 x float>> [#uses=1]
  %tmp66 = load <4 x float>* %pathNinv            ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp65, %tmp66          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %p
  %tmp67 = load <4 x float>* %p                   ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp67 ; <<4 x float>> [#uses=1]
  %call68 = call <4 x float> @_Z3logDv4_f(<4 x float> %sub) ; <<4 x float>> [#uses=1]
  %neg = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %call68 ; <<4 x float>> [#uses=1]
  %call69 = call <4 x float> @_Z3logDv4_f(<4 x float> %neg) ; <<4 x float>> [#uses=1]
  store <4 x float> %call69, <4 x float>* %z
  %tmp70 = load <4 x float>* %c1                  ; <<4 x float>> [#uses=1]
  %tmp71 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp72 = load <4 x float>* %c2                  ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp74 = load <4 x float>* %c3                  ; <<4 x float>> [#uses=1]
  %tmp75 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %c4                  ; <<4 x float>> [#uses=1]
  %tmp77 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp78 = load <4 x float>* %c5                  ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %c6                  ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp82 = load <4 x float>* %c7                  ; <<4 x float>> [#uses=1]
  %tmp83 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp84 = load <4 x float>* %c8                  ; <<4 x float>> [#uses=1]
  %tmp85 = load <4 x float>* %z                   ; <<4 x float>> [#uses=1]
  %tmp86 = load <4 x float>* %c9                  ; <<4 x float>> [#uses=1]
  %mul87 = fmul <4 x float> %tmp85, %tmp86        ; <<4 x float>> [#uses=1]
  %add88 = fadd <4 x float> %tmp84, %mul87        ; <<4 x float>> [#uses=1]
  %mul89 = fmul <4 x float> %tmp83, %add88        ; <<4 x float>> [#uses=1]
  %add90 = fadd <4 x float> %tmp82, %mul89        ; <<4 x float>> [#uses=1]
  %mul91 = fmul <4 x float> %tmp81, %add90        ; <<4 x float>> [#uses=1]
  %add92 = fadd <4 x float> %tmp80, %mul91        ; <<4 x float>> [#uses=1]
  %mul93 = fmul <4 x float> %tmp79, %add92        ; <<4 x float>> [#uses=1]
  %add94 = fadd <4 x float> %tmp78, %mul93        ; <<4 x float>> [#uses=1]
  %mul95 = fmul <4 x float> %tmp77, %add94        ; <<4 x float>> [#uses=1]
  %add96 = fadd <4 x float> %tmp76, %mul95        ; <<4 x float>> [#uses=1]
  %mul97 = fmul <4 x float> %tmp75, %add96        ; <<4 x float>> [#uses=1]
  %add98 = fadd <4 x float> %tmp74, %mul97        ; <<4 x float>> [#uses=1]
  %mul99 = fmul <4 x float> %tmp73, %add98        ; <<4 x float>> [#uses=1]
  %add100 = fadd <4 x float> %tmp72, %mul99       ; <<4 x float>> [#uses=1]
  %mul101 = fmul <4 x float> %tmp71, %add100      ; <<4 x float>> [#uses=1]
  %add102 = fadd <4 x float> %tmp70, %mul101      ; <<4 x float>> [#uses=1]
  %tmp103 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp103, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp104 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp104, i32 %tmp103 ; <<4 x float>*> [#uses=1]
  store <4 x float> %add102, <4 x float>* %arrayidx
  %tmp105 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add106 = fadd <4 x float> %tmp105, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add106, <4 x float>* %pos4pl1
  %tmp107 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %tmp108 = load <4 x float>* %pathNinv           ; <<4 x float>> [#uses=1]
  %mul109 = fmul <4 x float> %tmp107, %tmp108     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul109, <4 x float>* %p
  %tmp110 = load <4 x float>* %p                  ; <<4 x float>> [#uses=1]
  %sub111 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp110 ; <<4 x float>> [#uses=1]
  %call112 = call <4 x float> @_Z3logDv4_f(<4 x float> %sub111) ; <<4 x float>> [#uses=1]
  %neg113 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %call112 ; <<4 x float>> [#uses=1]
  %call114 = call <4 x float> @_Z3logDv4_f(<4 x float> %neg113) ; <<4 x float>> [#uses=1]
  store <4 x float> %call114, <4 x float>* %z
  %tmp115 = load <4 x float>* %c1                 ; <<4 x float>> [#uses=1]
  %tmp116 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp117 = load <4 x float>* %c2                 ; <<4 x float>> [#uses=1]
  %tmp118 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp119 = load <4 x float>* %c3                 ; <<4 x float>> [#uses=1]
  %tmp120 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp121 = load <4 x float>* %c4                 ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp123 = load <4 x float>* %c5                 ; <<4 x float>> [#uses=1]
  %tmp124 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp125 = load <4 x float>* %c6                 ; <<4 x float>> [#uses=1]
  %tmp126 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp127 = load <4 x float>* %c7                 ; <<4 x float>> [#uses=1]
  %tmp128 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %c8                 ; <<4 x float>> [#uses=1]
  %tmp130 = load <4 x float>* %z                  ; <<4 x float>> [#uses=1]
  %tmp131 = load <4 x float>* %c9                 ; <<4 x float>> [#uses=1]
  %mul132 = fmul <4 x float> %tmp130, %tmp131     ; <<4 x float>> [#uses=1]
  %add133 = fadd <4 x float> %tmp129, %mul132     ; <<4 x float>> [#uses=1]
  %mul134 = fmul <4 x float> %tmp128, %add133     ; <<4 x float>> [#uses=1]
  %add135 = fadd <4 x float> %tmp127, %mul134     ; <<4 x float>> [#uses=1]
  %mul136 = fmul <4 x float> %tmp126, %add135     ; <<4 x float>> [#uses=1]
  %add137 = fadd <4 x float> %tmp125, %mul136     ; <<4 x float>> [#uses=1]
  %mul138 = fmul <4 x float> %tmp124, %add137     ; <<4 x float>> [#uses=1]
  %add139 = fadd <4 x float> %tmp123, %mul138     ; <<4 x float>> [#uses=1]
  %mul140 = fmul <4 x float> %tmp122, %add139     ; <<4 x float>> [#uses=1]
  %add141 = fadd <4 x float> %tmp121, %mul140     ; <<4 x float>> [#uses=1]
  %mul142 = fmul <4 x float> %tmp120, %add141     ; <<4 x float>> [#uses=1]
  %add143 = fadd <4 x float> %tmp119, %mul142     ; <<4 x float>> [#uses=1]
  %mul144 = fmul <4 x float> %tmp118, %add143     ; <<4 x float>> [#uses=1]
  %add145 = fadd <4 x float> %tmp117, %mul144     ; <<4 x float>> [#uses=1]
  %mul146 = fmul <4 x float> %tmp116, %add145     ; <<4 x float>> [#uses=1]
  %add147 = fadd <4 x float> %tmp115, %mul146     ; <<4 x float>> [#uses=1]
  %tmp148 = load i32* %index                      ; <i32> [#uses=2]
  %inc149 = add nsw i32 %tmp148, 1                ; <i32> [#uses=1]
  store i32 %inc149, i32* %index
  %tmp150 = load <4 x float>** %pDist.addr        ; <<4 x float>*> [#uses=1]
  %arrayidx151 = getelementptr inbounds <4 x float>* %tmp150, i32 %tmp148 ; <<4 x float>*> [#uses=1]
  store <4 x float> %add147, <4 x float>* %arrayidx151
  %tmp152 = load <4 x float>* %pos4pl1            ; <<4 x float>> [#uses=1]
  %add153 = fadd <4 x float> %tmp152, <float 4.000000e+000, float 4.000000e+000, float 4.000000e+000, float 4.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %add153, <4 x float>* %pos4pl1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp154 = load i32* %pos                        ; <i32> [#uses=1]
  %add155 = add nsw i32 %tmp154, 8                ; <i32> [#uses=1]
  store i32 %add155, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @vsExp(i32 %n, <4 x float>* %input, <4 x float>* %output, i32 %threadId) nounwind {
entry:
  %n.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %input.addr = alloca <4 x float>*, align 4      ; <<4 x float>**> [#uses=2]
  %output.addr = alloca <4 x float>*, align 4     ; <<4 x float>**> [#uses=2]
  %threadId.addr = alloca i32, align 4            ; <i32*> [#uses=1]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store i32 %n, i32* %n.addr
  store <4 x float>* %input, <4 x float>** %input.addr
  store <4 x float>* %output, <4 x float>** %output.addr
  store i32 %threadId, i32* %threadId.addr
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %tmp1 = load i32* %n.addr                       ; <i32> [#uses=1]
  %shr = ashr i32 %tmp1, 2                        ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp, %shr                  ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32* %i                            ; <i32> [#uses=1]
  %tmp3 = load <4 x float>** %input.addr          ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp3, i32 %tmp2 ; <<4 x float>*> [#uses=1]
  %tmp4 = load <4 x float>* %arrayidx             ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z3expDv4_f(<4 x float> %tmp4) ; <<4 x float>> [#uses=1]
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  %tmp6 = load <4 x float>** %output.addr         ; <<4 x float>*> [#uses=1]
  %arrayidx7 = getelementptr inbounds <4 x float>* %tmp6, i32 %tmp5 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call, <4 x float>* %arrayidx7
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp8 = load i32* %i                            ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp8, 1                     ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare <4 x float> @_Z3expDv4_f(<4 x float>)

; CHECK: ret
define void @wlMonteCarloKernel(%struct.anon addrspace(1)* %pOptionValue, %0 addrspace(1)* %pOptionData, i32 %pathN) nounwind {
entry:
  %pOptionValue.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=3]
  %pOptionData.addr = alloca %0 addrspace(1)*, align 4 ; <%0 addrspace(1)**> [#uses=6]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=16]
  %sample = alloca [128 x <4 x float>], align 16  ; <[128 x <4 x float>]*> [#uses=17]
  %temp = alloca [128 x <4 x float>], align 16    ; <[128 x <4 x float>]*> [#uses=24]
  %temp2 = alloca [128 x <4 x float>], align 16   ; <[128 x <4 x float>]*> [#uses=0]
  %optionid = alloca i32, align 4                 ; <i32*> [#uses=10]
  %threadId = alloca i32, align 4                 ; <i32*> [#uses=9]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %S = alloca float, align 4                      ; <float*> [#uses=10]
  %X = alloca float, align 4                      ; <float*> [#uses=10]
  %V = alloca float, align 4                      ; <float*> [#uses=4]
  %MuByT = alloca float, align 4                  ; <float*> [#uses=12]
  %VBySqrtT = alloca float, align 4               ; <float*> [#uses=12]
  %sum = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=23]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %sum2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=23]
  %.compoundliteral48 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %boundary12 = alloca i32, align 4               ; <i32*> [#uses=4]
  %boundary23 = alloca i32, align 4               ; <i32*> [#uses=4]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=33]
  %n = alloca i32, align 4                        ; <i32*> [#uses=20]
  %i = alloca i32, align 4                        ; <i32*> [#uses=10]
  %i120 = alloca i32, align 4                     ; <i32*> [#uses=8]
  %callValue = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=12]
  %.compoundliteral141 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral168 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i200 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i233 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue241 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=6]
  %.compoundliteral256 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i291 = alloca i32, align 4                     ; <i32*> [#uses=18]
  %i379 = alloca i32, align 4                     ; <i32*> [#uses=8]
  %callValue386 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=12]
  %.compoundliteral400 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral425 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i459 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i489 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue497 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=6]
  %.compoundliteral512 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i549 = alloca i32, align 4                     ; <i32*> [#uses=10]
  %i597 = alloca i32, align 4                     ; <i32*> [#uses=8]
  %callValue604 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=12]
  %.compoundliteral619 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral646 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i681 = alloca i32, align 4                     ; <i32*> [#uses=6]
  %i714 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %callValue722 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=6]
  %.compoundliteral737 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %totalsum = alloca float, align 4               ; <float*> [#uses=4]
  %totalsum2 = alloca float, align 4              ; <float*> [#uses=2]
  %ExpRT = alloca float, align 4                  ; <float*> [#uses=3]
  %stdDev = alloca float, align 4                 ; <float*> [#uses=2]
  store %struct.anon addrspace(1)* %pOptionValue, %struct.anon addrspace(1)** %pOptionValue.addr
  store %0 addrspace(1)* %pOptionData, %0 addrspace(1)** %pOptionData.addr
  store i32 %pathN, i32* %pathN.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %optionid
  %call1 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %threadId
  %tmp = load i32* %optionid                      ; <i32> [#uses=1]
  %tmp2 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds %0 addrspace(1)* %tmp2, i32 %tmp ; <%0 addrspace(1)*> [#uses=1]
  %tmp3 = getelementptr inbounds %0 addrspace(1)* %arrayidx, i32 0, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp4 = load float addrspace(1)* %tmp3          ; <float> [#uses=1]
  store float %tmp4, float* %T
  %tmp6 = load i32* %optionid                     ; <i32> [#uses=1]
  %tmp7 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx8 = getelementptr inbounds %0 addrspace(1)* %tmp7, i32 %tmp6 ; <%0 addrspace(1)*> [#uses=1]
  %tmp9 = getelementptr inbounds %0 addrspace(1)* %arrayidx8, i32 0, i32 3 ; <float addrspace(1)*> [#uses=1]
  %tmp10 = load float addrspace(1)* %tmp9         ; <float> [#uses=1]
  store float %tmp10, float* %R
  %tmp12 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp13 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx14 = getelementptr inbounds %0 addrspace(1)* %tmp13, i32 %tmp12 ; <%0 addrspace(1)*> [#uses=1]
  %tmp15 = getelementptr inbounds %0 addrspace(1)* %arrayidx14, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp16 = load float addrspace(1)* %tmp15        ; <float> [#uses=1]
  store float %tmp16, float* %S
  %tmp18 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp19 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds %0 addrspace(1)* %tmp19, i32 %tmp18 ; <%0 addrspace(1)*> [#uses=1]
  %tmp21 = getelementptr inbounds %0 addrspace(1)* %arrayidx20, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp22 = load float addrspace(1)* %tmp21        ; <float> [#uses=1]
  store float %tmp22, float* %X
  %tmp24 = load i32* %optionid                    ; <i32> [#uses=1]
  %tmp25 = load %0 addrspace(1)** %pOptionData.addr ; <%0 addrspace(1)*> [#uses=1]
  %arrayidx26 = getelementptr inbounds %0 addrspace(1)* %tmp25, i32 %tmp24 ; <%0 addrspace(1)*> [#uses=1]
  %tmp27 = getelementptr inbounds %0 addrspace(1)* %arrayidx26, i32 0, i32 4 ; <float addrspace(1)*> [#uses=1]
  %tmp28 = load float addrspace(1)* %tmp27        ; <float> [#uses=1]
  store float %tmp28, float* %V
  %tmp30 = load float* %R                         ; <float> [#uses=1]
  %conv = fpext float %tmp30 to double            ; <double> [#uses=1]
  %tmp31 = load float* %V                         ; <float> [#uses=1]
  %conv32 = fpext float %tmp31 to double          ; <double> [#uses=1]
  %mul = fmul double 5.000000e-001, %conv32       ; <double> [#uses=1]
  %tmp33 = load float* %V                         ; <float> [#uses=1]
  %conv34 = fpext float %tmp33 to double          ; <double> [#uses=1]
  %mul35 = fmul double %mul, %conv34              ; <double> [#uses=1]
  %sub = fsub double %conv, %mul35                ; <double> [#uses=1]
  %tmp36 = load float* %T                         ; <float> [#uses=1]
  %conv37 = fpext float %tmp36 to double          ; <double> [#uses=1]
  %mul38 = fmul double %sub, %conv37              ; <double> [#uses=1]
  %conv39 = fptrunc double %mul38 to float        ; <float> [#uses=1]
  store float %conv39, float* %MuByT
  %tmp41 = load float* %V                         ; <float> [#uses=1]
  %tmp42 = load float* %T                         ; <float> [#uses=1]
  %call43 = call float @_Z4sqrtf(float %tmp42)    ; <float> [#uses=1]
  %mul44 = fmul float %tmp41, %call43             ; <float> [#uses=1]
  store float %mul44, float* %VBySqrtT
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp46 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46, <4 x float>* %sum
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral48
  %tmp49 = load <4 x float>* %.compoundliteral48  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp49, <4 x float>* %sum2
  %tmp51 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv52 = sitofp i32 %tmp51 to double           ; <double> [#uses=1]
  %mul53 = fmul double 8.000000e-002, %conv52     ; <double> [#uses=1]
  %conv54 = fptosi double %mul53 to i32           ; <i32> [#uses=1]
  %div = sdiv i32 %conv54, 4                      ; <i32> [#uses=1]
  %mul55 = mul i32 4, %div                        ; <i32> [#uses=1]
  store i32 %mul55, i32* %boundary12
  %tmp57 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %conv58 = sitofp i32 %tmp57 to double           ; <double> [#uses=1]
  %mul59 = fmul double 9.200000e-001, %conv58     ; <double> [#uses=1]
  %conv60 = fptosi double %mul59 to i32           ; <i32> [#uses=1]
  %div61 = sdiv i32 %conv60, 4                    ; <i32> [#uses=1]
  %mul62 = mul i32 4, %div61                      ; <i32> [#uses=1]
  store i32 %mul62, i32* %boundary23
  store i32 0, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp65 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp66 = load i32* %boundary12                  ; <i32> [#uses=1]
  %sub67 = sub i32 %tmp66, 512                    ; <i32> [#uses=1]
  %add = add nsw i32 %sub67, 1                    ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp65, %add                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end184

for.body:                                         ; preds = %for.cond
  %arraydecay = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp69 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp70 = load i32* %pos                         ; <i32> [#uses=1]
  %add71 = add nsw i32 %tmp70, 512                ; <i32> [#uses=1]
  %sub72 = sub i32 %add71, 1                      ; <i32> [#uses=1]
  %tmp73 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %tmp74 = load i32* %threadId                    ; <i32> [#uses=1]
  call void @NormalDistributionBatch1(<4 x float>* %arraydecay, i32 %tmp69, i32 %sub72, i32 %tmp73, i32 %tmp74)
  store i32 0, i32* %i
  br label %for.cond76

for.cond76:                                       ; preds = %for.body80, %for.body
  %tmp77 = load i32* %i                           ; <i32> [#uses=1]
  %cmp78 = icmp slt i32 %tmp77, 512               ; <i1> [#uses=1]
  br i1 %cmp78, label %for.body80, label %for.end

for.body80:                                       ; preds = %for.cond76
  %tmp81 = load i32* %i                           ; <i32> [#uses=1]
  %shr = ashr i32 %tmp81, 2                       ; <i32> [#uses=1]
  %arraydecay82 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx83 = getelementptr inbounds <4 x float>* %arraydecay82, i32 %shr ; <<4 x float>*> [#uses=1]
  %tmp84 = load <4 x float>* %arrayidx83          ; <<4 x float>> [#uses=1]
  %tmp85 = load float* %VBySqrtT                  ; <float> [#uses=1]
  %tmp86 = insertelement <4 x float> undef, float %tmp85, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp86, <4 x float> %tmp86, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul87 = fmul <4 x float> %tmp84, %splat        ; <<4 x float>> [#uses=1]
  %tmp88 = load float* %MuByT                     ; <float> [#uses=1]
  %tmp89 = insertelement <4 x float> undef, float %tmp88, i32 0 ; <<4 x float>> [#uses=2]
  %splat90 = shufflevector <4 x float> %tmp89, <4 x float> %tmp89, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add91 = fadd <4 x float> %mul87, %splat90      ; <<4 x float>> [#uses=1]
  %call92 = call <4 x float> @_Z3expDv4_f(<4 x float> %add91) ; <<4 x float>> [#uses=1]
  %tmp93 = load i32* %i                           ; <i32> [#uses=1]
  %shr94 = ashr i32 %tmp93, 2                     ; <i32> [#uses=1]
  %arraydecay95 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx96 = getelementptr inbounds <4 x float>* %arraydecay95, i32 %shr94 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call92, <4 x float>* %arrayidx96
  %tmp97 = load i32* %i                           ; <i32> [#uses=1]
  %add98 = add nsw i32 %tmp97, 4                  ; <i32> [#uses=1]
  store i32 %add98, i32* %i
  %tmp99 = load i32* %i                           ; <i32> [#uses=1]
  %shr100 = ashr i32 %tmp99, 2                    ; <i32> [#uses=1]
  %arraydecay101 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx102 = getelementptr inbounds <4 x float>* %arraydecay101, i32 %shr100 ; <<4 x float>*> [#uses=1]
  %tmp103 = load <4 x float>* %arrayidx102        ; <<4 x float>> [#uses=1]
  %tmp104 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp105 = insertelement <4 x float> undef, float %tmp104, i32 0 ; <<4 x float>> [#uses=2]
  %splat106 = shufflevector <4 x float> %tmp105, <4 x float> %tmp105, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul107 = fmul <4 x float> %tmp103, %splat106   ; <<4 x float>> [#uses=1]
  %tmp108 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp109 = insertelement <4 x float> undef, float %tmp108, i32 0 ; <<4 x float>> [#uses=2]
  %splat110 = shufflevector <4 x float> %tmp109, <4 x float> %tmp109, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add111 = fadd <4 x float> %mul107, %splat110   ; <<4 x float>> [#uses=1]
  %call112 = call <4 x float> @_Z3expDv4_f(<4 x float> %add111) ; <<4 x float>> [#uses=1]
  %tmp113 = load i32* %i                          ; <i32> [#uses=1]
  %shr114 = ashr i32 %tmp113, 2                   ; <i32> [#uses=1]
  %arraydecay115 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx116 = getelementptr inbounds <4 x float>* %arraydecay115, i32 %shr114 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call112, <4 x float>* %arrayidx116
  %tmp117 = load i32* %i                          ; <i32> [#uses=1]
  %add118 = add nsw i32 %tmp117, 4                ; <i32> [#uses=1]
  store i32 %add118, i32* %i
  br label %for.cond76

for.end:                                          ; preds = %for.cond76
  store i32 0, i32* %i120
  br label %for.cond121

for.cond121:                                      ; preds = %for.body125, %for.end
  %tmp122 = load i32* %i120                       ; <i32> [#uses=1]
  %cmp123 = icmp slt i32 %tmp122, 512             ; <i1> [#uses=1]
  br i1 %cmp123, label %for.body125, label %for.end181

for.body125:                                      ; preds = %for.cond121
  %tmp127 = load float* %S                        ; <float> [#uses=1]
  %tmp128 = insertelement <4 x float> undef, float %tmp127, i32 0 ; <<4 x float>> [#uses=2]
  %splat129 = shufflevector <4 x float> %tmp128, <4 x float> %tmp128, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp130 = load i32* %i120                       ; <i32> [#uses=1]
  %shr131 = ashr i32 %tmp130, 2                   ; <i32> [#uses=1]
  %arraydecay132 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx133 = getelementptr inbounds <4 x float>* %arraydecay132, i32 %shr131 ; <<4 x float>*> [#uses=1]
  %tmp134 = load <4 x float>* %arrayidx133        ; <<4 x float>> [#uses=1]
  %mul135 = fmul <4 x float> %splat129, %tmp134   ; <<4 x float>> [#uses=1]
  %tmp136 = load float* %X                        ; <float> [#uses=1]
  %tmp137 = insertelement <4 x float> undef, float %tmp136, i32 0 ; <<4 x float>> [#uses=2]
  %splat138 = shufflevector <4 x float> %tmp137, <4 x float> %tmp137, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub139 = fsub <4 x float> %mul135, %splat138   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub139, <4 x float>* %callValue
  %tmp140 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral141
  %tmp142 = load <4 x float>* %.compoundliteral141 ; <<4 x float>> [#uses=1]
  %call143 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp140, <4 x float> %tmp142) ; <<4 x float>> [#uses=1]
  store <4 x float> %call143, <4 x float>* %callValue
  %tmp144 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %tmp145 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add146 = fadd <4 x float> %tmp145, %tmp144     ; <<4 x float>> [#uses=1]
  store <4 x float> %add146, <4 x float>* %sum
  %tmp147 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %tmp148 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %mul149 = fmul <4 x float> %tmp147, %tmp148     ; <<4 x float>> [#uses=1]
  %tmp150 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add151 = fadd <4 x float> %tmp150, %mul149     ; <<4 x float>> [#uses=1]
  store <4 x float> %add151, <4 x float>* %sum2
  %tmp152 = load i32* %i120                       ; <i32> [#uses=1]
  %add153 = add nsw i32 %tmp152, 4                ; <i32> [#uses=1]
  store i32 %add153, i32* %i120
  %tmp154 = load float* %S                        ; <float> [#uses=1]
  %tmp155 = insertelement <4 x float> undef, float %tmp154, i32 0 ; <<4 x float>> [#uses=2]
  %splat156 = shufflevector <4 x float> %tmp155, <4 x float> %tmp155, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp157 = load i32* %i120                       ; <i32> [#uses=1]
  %shr158 = ashr i32 %tmp157, 2                   ; <i32> [#uses=1]
  %arraydecay159 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx160 = getelementptr inbounds <4 x float>* %arraydecay159, i32 %shr158 ; <<4 x float>*> [#uses=1]
  %tmp161 = load <4 x float>* %arrayidx160        ; <<4 x float>> [#uses=1]
  %mul162 = fmul <4 x float> %splat156, %tmp161   ; <<4 x float>> [#uses=1]
  %tmp163 = load float* %X                        ; <float> [#uses=1]
  %tmp164 = insertelement <4 x float> undef, float %tmp163, i32 0 ; <<4 x float>> [#uses=2]
  %splat165 = shufflevector <4 x float> %tmp164, <4 x float> %tmp164, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub166 = fsub <4 x float> %mul162, %splat165   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub166, <4 x float>* %callValue
  %tmp167 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral168
  %tmp169 = load <4 x float>* %.compoundliteral168 ; <<4 x float>> [#uses=1]
  %call170 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp167, <4 x float> %tmp169) ; <<4 x float>> [#uses=1]
  store <4 x float> %call170, <4 x float>* %callValue
  %tmp171 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %tmp172 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add173 = fadd <4 x float> %tmp172, %tmp171     ; <<4 x float>> [#uses=1]
  store <4 x float> %add173, <4 x float>* %sum
  %tmp174 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %tmp175 = load <4 x float>* %callValue          ; <<4 x float>> [#uses=1]
  %mul176 = fmul <4 x float> %tmp174, %tmp175     ; <<4 x float>> [#uses=1]
  %tmp177 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add178 = fadd <4 x float> %tmp177, %mul176     ; <<4 x float>> [#uses=1]
  store <4 x float> %add178, <4 x float>* %sum2
  %tmp179 = load i32* %i120                       ; <i32> [#uses=1]
  %add180 = add nsw i32 %tmp179, 4                ; <i32> [#uses=1]
  store i32 %add180, i32* %i120
  br label %for.cond121

for.end181:                                       ; preds = %for.cond121
  br label %for.inc

for.inc:                                          ; preds = %for.end181
  %tmp182 = load i32* %pos                        ; <i32> [#uses=1]
  %add183 = add nsw i32 %tmp182, 512              ; <i32> [#uses=1]
  store i32 %add183, i32* %pos
  br label %for.cond

for.end184:                                       ; preds = %for.cond
  %tmp185 = load i32* %boundary12                 ; <i32> [#uses=1]
  %tmp186 = load i32* %pos                        ; <i32> [#uses=1]
  %sub187 = sub i32 %tmp185, %tmp186              ; <i32> [#uses=1]
  store i32 %sub187, i32* %n
  %tmp188 = load i32* %n                          ; <i32> [#uses=1]
  %cmp189 = icmp sgt i32 %tmp188, 0               ; <i1> [#uses=1]
  br i1 %cmp189, label %if.then, label %if.end

if.then:                                          ; preds = %for.end184
  %arraydecay191 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp192 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp193 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp194 = load i32* %n                          ; <i32> [#uses=1]
  %add195 = add nsw i32 %tmp193, %tmp194          ; <i32> [#uses=1]
  %sub196 = sub i32 %add195, 1                    ; <i32> [#uses=1]
  %tmp197 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp198 = load i32* %threadId                   ; <i32> [#uses=1]
  call void @NormalDistributionBatch1(<4 x float>* %arraydecay191, i32 %tmp192, i32 %sub196, i32 %tmp197, i32 %tmp198)
  store i32 0, i32* %i200
  br label %for.cond201

for.cond201:                                      ; preds = %for.inc224, %if.then
  %tmp202 = load i32* %i200                       ; <i32> [#uses=1]
  %tmp203 = load i32* %n                          ; <i32> [#uses=1]
  %cmp204 = icmp slt i32 %tmp202, %tmp203         ; <i1> [#uses=1]
  br i1 %cmp204, label %for.body206, label %for.end227

for.body206:                                      ; preds = %for.cond201
  %tmp207 = load i32* %i200                       ; <i32> [#uses=1]
  %shr208 = ashr i32 %tmp207, 2                   ; <i32> [#uses=1]
  %arraydecay209 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx210 = getelementptr inbounds <4 x float>* %arraydecay209, i32 %shr208 ; <<4 x float>*> [#uses=1]
  %tmp211 = load <4 x float>* %arrayidx210        ; <<4 x float>> [#uses=1]
  %tmp212 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp213 = insertelement <4 x float> undef, float %tmp212, i32 0 ; <<4 x float>> [#uses=2]
  %splat214 = shufflevector <4 x float> %tmp213, <4 x float> %tmp213, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul215 = fmul <4 x float> %tmp211, %splat214   ; <<4 x float>> [#uses=1]
  %tmp216 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp217 = insertelement <4 x float> undef, float %tmp216, i32 0 ; <<4 x float>> [#uses=2]
  %splat218 = shufflevector <4 x float> %tmp217, <4 x float> %tmp217, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add219 = fadd <4 x float> %mul215, %splat218   ; <<4 x float>> [#uses=1]
  %tmp220 = load i32* %i200                       ; <i32> [#uses=1]
  %shr221 = ashr i32 %tmp220, 2                   ; <i32> [#uses=1]
  %arraydecay222 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx223 = getelementptr inbounds <4 x float>* %arraydecay222, i32 %shr221 ; <<4 x float>*> [#uses=1]
  store <4 x float> %add219, <4 x float>* %arrayidx223
  br label %for.inc224

for.inc224:                                       ; preds = %for.body206
  %tmp225 = load i32* %i200                       ; <i32> [#uses=1]
  %add226 = add nsw i32 %tmp225, 4                ; <i32> [#uses=1]
  store i32 %add226, i32* %i200
  br label %for.cond201

for.end227:                                       ; preds = %for.cond201
  %tmp228 = load i32* %n                          ; <i32> [#uses=1]
  %arraydecay229 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arraydecay230 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp231 = load i32* %threadId                   ; <i32> [#uses=1]
  call void @vsExp(i32 %tmp228, <4 x float>* %arraydecay229, <4 x float>* %arraydecay230, i32 %tmp231)
  store i32 0, i32* %i233
  br label %for.cond234

for.cond234:                                      ; preds = %for.inc267, %for.end227
  %tmp235 = load i32* %i233                       ; <i32> [#uses=1]
  %tmp236 = load i32* %n                          ; <i32> [#uses=1]
  %cmp237 = icmp slt i32 %tmp235, %tmp236         ; <i1> [#uses=1]
  br i1 %cmp237, label %for.body239, label %for.end270

for.body239:                                      ; preds = %for.cond234
  %tmp242 = load float* %S                        ; <float> [#uses=1]
  %tmp243 = insertelement <4 x float> undef, float %tmp242, i32 0 ; <<4 x float>> [#uses=2]
  %splat244 = shufflevector <4 x float> %tmp243, <4 x float> %tmp243, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp245 = load i32* %i233                       ; <i32> [#uses=1]
  %shr246 = ashr i32 %tmp245, 2                   ; <i32> [#uses=1]
  %arraydecay247 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx248 = getelementptr inbounds <4 x float>* %arraydecay247, i32 %shr246 ; <<4 x float>*> [#uses=1]
  %tmp249 = load <4 x float>* %arrayidx248        ; <<4 x float>> [#uses=1]
  %mul250 = fmul <4 x float> %splat244, %tmp249   ; <<4 x float>> [#uses=1]
  %tmp251 = load float* %X                        ; <float> [#uses=1]
  %tmp252 = insertelement <4 x float> undef, float %tmp251, i32 0 ; <<4 x float>> [#uses=2]
  %splat253 = shufflevector <4 x float> %tmp252, <4 x float> %tmp252, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub254 = fsub <4 x float> %mul250, %splat253   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub254, <4 x float>* %callValue241
  %tmp255 = load <4 x float>* %callValue241       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral256
  %tmp257 = load <4 x float>* %.compoundliteral256 ; <<4 x float>> [#uses=1]
  %call258 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp255, <4 x float> %tmp257) ; <<4 x float>> [#uses=1]
  store <4 x float> %call258, <4 x float>* %callValue241
  %tmp259 = load <4 x float>* %callValue241       ; <<4 x float>> [#uses=1]
  %tmp260 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add261 = fadd <4 x float> %tmp260, %tmp259     ; <<4 x float>> [#uses=1]
  store <4 x float> %add261, <4 x float>* %sum
  %tmp262 = load <4 x float>* %callValue241       ; <<4 x float>> [#uses=1]
  %tmp263 = load <4 x float>* %callValue241       ; <<4 x float>> [#uses=1]
  %mul264 = fmul <4 x float> %tmp262, %tmp263     ; <<4 x float>> [#uses=1]
  %tmp265 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add266 = fadd <4 x float> %tmp265, %mul264     ; <<4 x float>> [#uses=1]
  store <4 x float> %add266, <4 x float>* %sum2
  br label %for.inc267

for.inc267:                                       ; preds = %for.body239
  %tmp268 = load i32* %i233                       ; <i32> [#uses=1]
  %add269 = add nsw i32 %tmp268, 4                ; <i32> [#uses=1]
  store i32 %add269, i32* %i233
  br label %for.cond234

for.end270:                                       ; preds = %for.cond234
  %tmp271 = load i32* %n                          ; <i32> [#uses=1]
  %tmp272 = load i32* %pos                        ; <i32> [#uses=1]
  %add273 = add nsw i32 %tmp272, %tmp271          ; <i32> [#uses=1]
  store i32 %add273, i32* %pos
  br label %if.end

if.end:                                           ; preds = %for.end270, %for.end184
  %tmp274 = load i32* %boundary12                 ; <i32> [#uses=1]
  store i32 %tmp274, i32* %pos
  br label %for.cond275

for.cond275:                                      ; preds = %for.inc439, %if.end
  %tmp276 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp277 = load i32* %boundary23                 ; <i32> [#uses=1]
  %sub278 = sub i32 %tmp277, 512                  ; <i32> [#uses=1]
  %add279 = add nsw i32 %sub278, 1                ; <i32> [#uses=1]
  %cmp280 = icmp slt i32 %tmp276, %add279         ; <i1> [#uses=1]
  br i1 %cmp280, label %for.body282, label %for.end442

for.body282:                                      ; preds = %for.cond275
  %arraydecay283 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp284 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp285 = load i32* %pos                        ; <i32> [#uses=1]
  %add286 = add nsw i32 %tmp285, 512              ; <i32> [#uses=1]
  %sub287 = sub i32 %add286, 1                    ; <i32> [#uses=1]
  %tmp288 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp289 = load i32* %threadId                   ; <i32> [#uses=1]
  call void @NormalDistributionBatch2(<4 x float>* %arraydecay283, i32 %tmp284, i32 %sub287, i32 %tmp288, i32 %tmp289)
  store i32 0, i32* %i291
  br label %for.cond292

for.cond292:                                      ; preds = %for.body296, %for.body282
  %tmp293 = load i32* %i291                       ; <i32> [#uses=1]
  %cmp294 = icmp slt i32 %tmp293, 512             ; <i1> [#uses=1]
  br i1 %cmp294, label %for.body296, label %for.end377

for.body296:                                      ; preds = %for.cond292
  %tmp297 = load i32* %i291                       ; <i32> [#uses=1]
  %shr298 = ashr i32 %tmp297, 2                   ; <i32> [#uses=1]
  %arraydecay299 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx300 = getelementptr inbounds <4 x float>* %arraydecay299, i32 %shr298 ; <<4 x float>*> [#uses=1]
  %tmp301 = load <4 x float>* %arrayidx300        ; <<4 x float>> [#uses=1]
  %tmp302 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp303 = insertelement <4 x float> undef, float %tmp302, i32 0 ; <<4 x float>> [#uses=2]
  %splat304 = shufflevector <4 x float> %tmp303, <4 x float> %tmp303, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul305 = fmul <4 x float> %tmp301, %splat304   ; <<4 x float>> [#uses=1]
  %tmp306 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp307 = insertelement <4 x float> undef, float %tmp306, i32 0 ; <<4 x float>> [#uses=2]
  %splat308 = shufflevector <4 x float> %tmp307, <4 x float> %tmp307, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add309 = fadd <4 x float> %mul305, %splat308   ; <<4 x float>> [#uses=1]
  %call310 = call <4 x float> @_Z3expDv4_f(<4 x float> %add309) ; <<4 x float>> [#uses=1]
  %tmp311 = load i32* %i291                       ; <i32> [#uses=1]
  %shr312 = ashr i32 %tmp311, 2                   ; <i32> [#uses=1]
  %arraydecay313 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx314 = getelementptr inbounds <4 x float>* %arraydecay313, i32 %shr312 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call310, <4 x float>* %arrayidx314
  %tmp315 = load i32* %i291                       ; <i32> [#uses=1]
  %add316 = add nsw i32 %tmp315, 4                ; <i32> [#uses=1]
  store i32 %add316, i32* %i291
  %tmp317 = load i32* %i291                       ; <i32> [#uses=1]
  %shr318 = ashr i32 %tmp317, 2                   ; <i32> [#uses=1]
  %arraydecay319 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx320 = getelementptr inbounds <4 x float>* %arraydecay319, i32 %shr318 ; <<4 x float>*> [#uses=1]
  %tmp321 = load <4 x float>* %arrayidx320        ; <<4 x float>> [#uses=1]
  %tmp322 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp323 = insertelement <4 x float> undef, float %tmp322, i32 0 ; <<4 x float>> [#uses=2]
  %splat324 = shufflevector <4 x float> %tmp323, <4 x float> %tmp323, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul325 = fmul <4 x float> %tmp321, %splat324   ; <<4 x float>> [#uses=1]
  %tmp326 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp327 = insertelement <4 x float> undef, float %tmp326, i32 0 ; <<4 x float>> [#uses=2]
  %splat328 = shufflevector <4 x float> %tmp327, <4 x float> %tmp327, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add329 = fadd <4 x float> %mul325, %splat328   ; <<4 x float>> [#uses=1]
  %call330 = call <4 x float> @_Z3expDv4_f(<4 x float> %add329) ; <<4 x float>> [#uses=1]
  %tmp331 = load i32* %i291                       ; <i32> [#uses=1]
  %shr332 = ashr i32 %tmp331, 2                   ; <i32> [#uses=1]
  %arraydecay333 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx334 = getelementptr inbounds <4 x float>* %arraydecay333, i32 %shr332 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call330, <4 x float>* %arrayidx334
  %tmp335 = load i32* %i291                       ; <i32> [#uses=1]
  %add336 = add nsw i32 %tmp335, 4                ; <i32> [#uses=1]
  store i32 %add336, i32* %i291
  %tmp337 = load i32* %i291                       ; <i32> [#uses=1]
  %shr338 = ashr i32 %tmp337, 2                   ; <i32> [#uses=1]
  %arraydecay339 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx340 = getelementptr inbounds <4 x float>* %arraydecay339, i32 %shr338 ; <<4 x float>*> [#uses=1]
  %tmp341 = load <4 x float>* %arrayidx340        ; <<4 x float>> [#uses=1]
  %tmp342 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp343 = insertelement <4 x float> undef, float %tmp342, i32 0 ; <<4 x float>> [#uses=2]
  %splat344 = shufflevector <4 x float> %tmp343, <4 x float> %tmp343, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul345 = fmul <4 x float> %tmp341, %splat344   ; <<4 x float>> [#uses=1]
  %tmp346 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp347 = insertelement <4 x float> undef, float %tmp346, i32 0 ; <<4 x float>> [#uses=2]
  %splat348 = shufflevector <4 x float> %tmp347, <4 x float> %tmp347, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add349 = fadd <4 x float> %mul345, %splat348   ; <<4 x float>> [#uses=1]
  %call350 = call <4 x float> @_Z3expDv4_f(<4 x float> %add349) ; <<4 x float>> [#uses=1]
  %tmp351 = load i32* %i291                       ; <i32> [#uses=1]
  %shr352 = ashr i32 %tmp351, 2                   ; <i32> [#uses=1]
  %arraydecay353 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx354 = getelementptr inbounds <4 x float>* %arraydecay353, i32 %shr352 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call350, <4 x float>* %arrayidx354
  %tmp355 = load i32* %i291                       ; <i32> [#uses=1]
  %add356 = add nsw i32 %tmp355, 4                ; <i32> [#uses=1]
  store i32 %add356, i32* %i291
  %tmp357 = load i32* %i291                       ; <i32> [#uses=1]
  %shr358 = ashr i32 %tmp357, 2                   ; <i32> [#uses=1]
  %arraydecay359 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx360 = getelementptr inbounds <4 x float>* %arraydecay359, i32 %shr358 ; <<4 x float>*> [#uses=1]
  %tmp361 = load <4 x float>* %arrayidx360        ; <<4 x float>> [#uses=1]
  %tmp362 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp363 = insertelement <4 x float> undef, float %tmp362, i32 0 ; <<4 x float>> [#uses=2]
  %splat364 = shufflevector <4 x float> %tmp363, <4 x float> %tmp363, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul365 = fmul <4 x float> %tmp361, %splat364   ; <<4 x float>> [#uses=1]
  %tmp366 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp367 = insertelement <4 x float> undef, float %tmp366, i32 0 ; <<4 x float>> [#uses=2]
  %splat368 = shufflevector <4 x float> %tmp367, <4 x float> %tmp367, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add369 = fadd <4 x float> %mul365, %splat368   ; <<4 x float>> [#uses=1]
  %call370 = call <4 x float> @_Z3expDv4_f(<4 x float> %add369) ; <<4 x float>> [#uses=1]
  %tmp371 = load i32* %i291                       ; <i32> [#uses=1]
  %shr372 = ashr i32 %tmp371, 2                   ; <i32> [#uses=1]
  %arraydecay373 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx374 = getelementptr inbounds <4 x float>* %arraydecay373, i32 %shr372 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call370, <4 x float>* %arrayidx374
  %tmp375 = load i32* %i291                       ; <i32> [#uses=1]
  %add376 = add nsw i32 %tmp375, 4                ; <i32> [#uses=1]
  store i32 %add376, i32* %i291
  br label %for.cond292

for.end377:                                       ; preds = %for.cond292
  store i32 0, i32* %i379
  br label %for.cond380

for.cond380:                                      ; preds = %for.body384, %for.end377
  %tmp381 = load i32* %i379                       ; <i32> [#uses=1]
  %cmp382 = icmp slt i32 %tmp381, 128             ; <i1> [#uses=1]
  br i1 %cmp382, label %for.body384, label %for.end438

for.body384:                                      ; preds = %for.cond380
  %tmp387 = load float* %S                        ; <float> [#uses=1]
  %tmp388 = insertelement <4 x float> undef, float %tmp387, i32 0 ; <<4 x float>> [#uses=2]
  %splat389 = shufflevector <4 x float> %tmp388, <4 x float> %tmp388, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp390 = load i32* %i379                       ; <i32> [#uses=1]
  %arraydecay391 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx392 = getelementptr inbounds <4 x float>* %arraydecay391, i32 %tmp390 ; <<4 x float>*> [#uses=1]
  %tmp393 = load <4 x float>* %arrayidx392        ; <<4 x float>> [#uses=1]
  %mul394 = fmul <4 x float> %splat389, %tmp393   ; <<4 x float>> [#uses=1]
  %tmp395 = load float* %X                        ; <float> [#uses=1]
  %tmp396 = insertelement <4 x float> undef, float %tmp395, i32 0 ; <<4 x float>> [#uses=2]
  %splat397 = shufflevector <4 x float> %tmp396, <4 x float> %tmp396, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub398 = fsub <4 x float> %mul394, %splat397   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub398, <4 x float>* %callValue386
  %tmp399 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral400
  %tmp401 = load <4 x float>* %.compoundliteral400 ; <<4 x float>> [#uses=1]
  %call402 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp399, <4 x float> %tmp401) ; <<4 x float>> [#uses=1]
  store <4 x float> %call402, <4 x float>* %callValue386
  %tmp403 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %tmp404 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add405 = fadd <4 x float> %tmp404, %tmp403     ; <<4 x float>> [#uses=1]
  store <4 x float> %add405, <4 x float>* %sum
  %tmp406 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %tmp407 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %mul408 = fmul <4 x float> %tmp406, %tmp407     ; <<4 x float>> [#uses=1]
  %tmp409 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add410 = fadd <4 x float> %tmp409, %mul408     ; <<4 x float>> [#uses=1]
  store <4 x float> %add410, <4 x float>* %sum2
  %tmp411 = load i32* %i379                       ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp411, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i379
  %tmp412 = load float* %S                        ; <float> [#uses=1]
  %tmp413 = insertelement <4 x float> undef, float %tmp412, i32 0 ; <<4 x float>> [#uses=2]
  %splat414 = shufflevector <4 x float> %tmp413, <4 x float> %tmp413, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp415 = load i32* %i379                       ; <i32> [#uses=1]
  %arraydecay416 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx417 = getelementptr inbounds <4 x float>* %arraydecay416, i32 %tmp415 ; <<4 x float>*> [#uses=1]
  %tmp418 = load <4 x float>* %arrayidx417        ; <<4 x float>> [#uses=1]
  %mul419 = fmul <4 x float> %splat414, %tmp418   ; <<4 x float>> [#uses=1]
  %tmp420 = load float* %X                        ; <float> [#uses=1]
  %tmp421 = insertelement <4 x float> undef, float %tmp420, i32 0 ; <<4 x float>> [#uses=2]
  %splat422 = shufflevector <4 x float> %tmp421, <4 x float> %tmp421, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub423 = fsub <4 x float> %mul419, %splat422   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub423, <4 x float>* %callValue386
  %tmp424 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral425
  %tmp426 = load <4 x float>* %.compoundliteral425 ; <<4 x float>> [#uses=1]
  %call427 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp424, <4 x float> %tmp426) ; <<4 x float>> [#uses=1]
  store <4 x float> %call427, <4 x float>* %callValue386
  %tmp428 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %tmp429 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add430 = fadd <4 x float> %tmp429, %tmp428     ; <<4 x float>> [#uses=1]
  store <4 x float> %add430, <4 x float>* %sum
  %tmp431 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %tmp432 = load <4 x float>* %callValue386       ; <<4 x float>> [#uses=1]
  %mul433 = fmul <4 x float> %tmp431, %tmp432     ; <<4 x float>> [#uses=1]
  %tmp434 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add435 = fadd <4 x float> %tmp434, %mul433     ; <<4 x float>> [#uses=1]
  store <4 x float> %add435, <4 x float>* %sum2
  %tmp436 = load i32* %i379                       ; <i32> [#uses=1]
  %inc437 = add nsw i32 %tmp436, 1                ; <i32> [#uses=1]
  store i32 %inc437, i32* %i379
  br label %for.cond380

for.end438:                                       ; preds = %for.cond380
  br label %for.inc439

for.inc439:                                       ; preds = %for.end438
  %tmp440 = load i32* %pos                        ; <i32> [#uses=1]
  %add441 = add nsw i32 %tmp440, 512              ; <i32> [#uses=1]
  store i32 %add441, i32* %pos
  br label %for.cond275

for.end442:                                       ; preds = %for.cond275
  %tmp443 = load i32* %boundary23                 ; <i32> [#uses=1]
  %tmp444 = load i32* %pos                        ; <i32> [#uses=1]
  %sub445 = sub i32 %tmp443, %tmp444              ; <i32> [#uses=1]
  store i32 %sub445, i32* %n
  %tmp446 = load i32* %n                          ; <i32> [#uses=1]
  %cmp447 = icmp sgt i32 %tmp446, 0               ; <i1> [#uses=1]
  br i1 %cmp447, label %if.then449, label %if.end530

if.then449:                                       ; preds = %for.end442
  %arraydecay450 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp451 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp452 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp453 = load i32* %n                          ; <i32> [#uses=1]
  %add454 = add nsw i32 %tmp452, %tmp453          ; <i32> [#uses=1]
  %sub455 = sub i32 %add454, 1                    ; <i32> [#uses=1]
  %tmp456 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp457 = load i32* %threadId                   ; <i32> [#uses=1]
  call void @NormalDistributionBatch2(<4 x float>* %arraydecay450, i32 %tmp451, i32 %sub455, i32 %tmp456, i32 %tmp457)
  store i32 0, i32* %i459
  br label %for.cond460

for.cond460:                                      ; preds = %for.inc484, %if.then449
  %tmp461 = load i32* %i459                       ; <i32> [#uses=1]
  %tmp462 = load i32* %n                          ; <i32> [#uses=1]
  %cmp463 = icmp slt i32 %tmp461, %tmp462         ; <i1> [#uses=1]
  br i1 %cmp463, label %for.body465, label %for.end487

for.body465:                                      ; preds = %for.cond460
  %tmp466 = load i32* %i459                       ; <i32> [#uses=1]
  %shr467 = ashr i32 %tmp466, 2                   ; <i32> [#uses=1]
  %arraydecay468 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx469 = getelementptr inbounds <4 x float>* %arraydecay468, i32 %shr467 ; <<4 x float>*> [#uses=1]
  %tmp470 = load <4 x float>* %arrayidx469        ; <<4 x float>> [#uses=1]
  %tmp471 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp472 = insertelement <4 x float> undef, float %tmp471, i32 0 ; <<4 x float>> [#uses=2]
  %splat473 = shufflevector <4 x float> %tmp472, <4 x float> %tmp472, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul474 = fmul <4 x float> %tmp470, %splat473   ; <<4 x float>> [#uses=1]
  %tmp475 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp476 = insertelement <4 x float> undef, float %tmp475, i32 0 ; <<4 x float>> [#uses=2]
  %splat477 = shufflevector <4 x float> %tmp476, <4 x float> %tmp476, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add478 = fadd <4 x float> %mul474, %splat477   ; <<4 x float>> [#uses=1]
  %call479 = call <4 x float> @_Z3expDv4_f(<4 x float> %add478) ; <<4 x float>> [#uses=1]
  %tmp480 = load i32* %i459                       ; <i32> [#uses=1]
  %shr481 = ashr i32 %tmp480, 2                   ; <i32> [#uses=1]
  %arraydecay482 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx483 = getelementptr inbounds <4 x float>* %arraydecay482, i32 %shr481 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call479, <4 x float>* %arrayidx483
  br label %for.inc484

for.inc484:                                       ; preds = %for.body465
  %tmp485 = load i32* %i459                       ; <i32> [#uses=1]
  %add486 = add nsw i32 %tmp485, 4                ; <i32> [#uses=1]
  store i32 %add486, i32* %i459
  br label %for.cond460

for.end487:                                       ; preds = %for.cond460
  store i32 0, i32* %i489
  br label %for.cond490

for.cond490:                                      ; preds = %for.inc523, %for.end487
  %tmp491 = load i32* %i489                       ; <i32> [#uses=1]
  %tmp492 = load i32* %n                          ; <i32> [#uses=1]
  %cmp493 = icmp slt i32 %tmp491, %tmp492         ; <i1> [#uses=1]
  br i1 %cmp493, label %for.body495, label %for.end526

for.body495:                                      ; preds = %for.cond490
  %tmp498 = load float* %S                        ; <float> [#uses=1]
  %tmp499 = insertelement <4 x float> undef, float %tmp498, i32 0 ; <<4 x float>> [#uses=2]
  %splat500 = shufflevector <4 x float> %tmp499, <4 x float> %tmp499, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp501 = load i32* %i489                       ; <i32> [#uses=1]
  %shr502 = ashr i32 %tmp501, 2                   ; <i32> [#uses=1]
  %arraydecay503 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx504 = getelementptr inbounds <4 x float>* %arraydecay503, i32 %shr502 ; <<4 x float>*> [#uses=1]
  %tmp505 = load <4 x float>* %arrayidx504        ; <<4 x float>> [#uses=1]
  %mul506 = fmul <4 x float> %splat500, %tmp505   ; <<4 x float>> [#uses=1]
  %tmp507 = load float* %X                        ; <float> [#uses=1]
  %tmp508 = insertelement <4 x float> undef, float %tmp507, i32 0 ; <<4 x float>> [#uses=2]
  %splat509 = shufflevector <4 x float> %tmp508, <4 x float> %tmp508, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub510 = fsub <4 x float> %mul506, %splat509   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub510, <4 x float>* %callValue497
  %tmp511 = load <4 x float>* %callValue497       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral512
  %tmp513 = load <4 x float>* %.compoundliteral512 ; <<4 x float>> [#uses=1]
  %call514 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp511, <4 x float> %tmp513) ; <<4 x float>> [#uses=1]
  store <4 x float> %call514, <4 x float>* %callValue497
  %tmp515 = load <4 x float>* %callValue497       ; <<4 x float>> [#uses=1]
  %tmp516 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add517 = fadd <4 x float> %tmp516, %tmp515     ; <<4 x float>> [#uses=1]
  store <4 x float> %add517, <4 x float>* %sum
  %tmp518 = load <4 x float>* %callValue497       ; <<4 x float>> [#uses=1]
  %tmp519 = load <4 x float>* %callValue497       ; <<4 x float>> [#uses=1]
  %mul520 = fmul <4 x float> %tmp518, %tmp519     ; <<4 x float>> [#uses=1]
  %tmp521 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add522 = fadd <4 x float> %tmp521, %mul520     ; <<4 x float>> [#uses=1]
  store <4 x float> %add522, <4 x float>* %sum2
  br label %for.inc523

for.inc523:                                       ; preds = %for.body495
  %tmp524 = load i32* %i489                       ; <i32> [#uses=1]
  %add525 = add nsw i32 %tmp524, 4                ; <i32> [#uses=1]
  store i32 %add525, i32* %i489
  br label %for.cond490

for.end526:                                       ; preds = %for.cond490
  %tmp527 = load i32* %n                          ; <i32> [#uses=1]
  %tmp528 = load i32* %pos                        ; <i32> [#uses=1]
  %add529 = add nsw i32 %tmp528, %tmp527          ; <i32> [#uses=1]
  store i32 %add529, i32* %pos
  br label %if.end530

if.end530:                                        ; preds = %for.end526, %for.end442
  %tmp531 = load i32* %boundary23                 ; <i32> [#uses=1]
  store i32 %tmp531, i32* %pos
  br label %for.cond532

for.cond532:                                      ; preds = %for.inc660, %if.end530
  %tmp533 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp534 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub535 = sub i32 %tmp534, 512                  ; <i32> [#uses=1]
  %add536 = add nsw i32 %sub535, 1                ; <i32> [#uses=1]
  %cmp537 = icmp slt i32 %tmp533, %add536         ; <i1> [#uses=1]
  br i1 %cmp537, label %for.body539, label %for.end663

for.body539:                                      ; preds = %for.cond532
  %arraydecay540 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp541 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp542 = load i32* %pos                        ; <i32> [#uses=1]
  %add543 = add nsw i32 %tmp542, 512              ; <i32> [#uses=1]
  %sub544 = sub i32 %add543, 1                    ; <i32> [#uses=1]
  %tmp545 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp546 = load i32* %threadId                   ; <i32> [#uses=1]
  %tmp547 = load i32* %optionid                   ; <i32> [#uses=1]
  call void @NormalDistributionBatch3(<4 x float>* %arraydecay540, i32 %tmp541, i32 %sub544, i32 %tmp545, i32 %tmp546, i32 %tmp547)
  store i32 0, i32* %i549
  br label %for.cond550

for.cond550:                                      ; preds = %for.body554, %for.body539
  %tmp551 = load i32* %i549                       ; <i32> [#uses=1]
  %cmp552 = icmp slt i32 %tmp551, 512             ; <i1> [#uses=1]
  br i1 %cmp552, label %for.body554, label %for.end595

for.body554:                                      ; preds = %for.cond550
  %tmp555 = load i32* %i549                       ; <i32> [#uses=1]
  %shr556 = ashr i32 %tmp555, 2                   ; <i32> [#uses=1]
  %arraydecay557 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx558 = getelementptr inbounds <4 x float>* %arraydecay557, i32 %shr556 ; <<4 x float>*> [#uses=1]
  %tmp559 = load <4 x float>* %arrayidx558        ; <<4 x float>> [#uses=1]
  %tmp560 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp561 = insertelement <4 x float> undef, float %tmp560, i32 0 ; <<4 x float>> [#uses=2]
  %splat562 = shufflevector <4 x float> %tmp561, <4 x float> %tmp561, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul563 = fmul <4 x float> %tmp559, %splat562   ; <<4 x float>> [#uses=1]
  %tmp564 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp565 = insertelement <4 x float> undef, float %tmp564, i32 0 ; <<4 x float>> [#uses=2]
  %splat566 = shufflevector <4 x float> %tmp565, <4 x float> %tmp565, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add567 = fadd <4 x float> %mul563, %splat566   ; <<4 x float>> [#uses=1]
  %call568 = call <4 x float> @_Z3expDv4_f(<4 x float> %add567) ; <<4 x float>> [#uses=1]
  %tmp569 = load i32* %i549                       ; <i32> [#uses=1]
  %shr570 = ashr i32 %tmp569, 2                   ; <i32> [#uses=1]
  %arraydecay571 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx572 = getelementptr inbounds <4 x float>* %arraydecay571, i32 %shr570 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call568, <4 x float>* %arrayidx572
  %tmp573 = load i32* %i549                       ; <i32> [#uses=1]
  %add574 = add nsw i32 %tmp573, 4                ; <i32> [#uses=1]
  store i32 %add574, i32* %i549
  %tmp575 = load i32* %i549                       ; <i32> [#uses=1]
  %shr576 = ashr i32 %tmp575, 2                   ; <i32> [#uses=1]
  %arraydecay577 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx578 = getelementptr inbounds <4 x float>* %arraydecay577, i32 %shr576 ; <<4 x float>*> [#uses=1]
  %tmp579 = load <4 x float>* %arrayidx578        ; <<4 x float>> [#uses=1]
  %tmp580 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp581 = insertelement <4 x float> undef, float %tmp580, i32 0 ; <<4 x float>> [#uses=2]
  %splat582 = shufflevector <4 x float> %tmp581, <4 x float> %tmp581, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul583 = fmul <4 x float> %tmp579, %splat582   ; <<4 x float>> [#uses=1]
  %tmp584 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp585 = insertelement <4 x float> undef, float %tmp584, i32 0 ; <<4 x float>> [#uses=2]
  %splat586 = shufflevector <4 x float> %tmp585, <4 x float> %tmp585, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add587 = fadd <4 x float> %mul583, %splat586   ; <<4 x float>> [#uses=1]
  %call588 = call <4 x float> @_Z3expDv4_f(<4 x float> %add587) ; <<4 x float>> [#uses=1]
  %tmp589 = load i32* %i549                       ; <i32> [#uses=1]
  %shr590 = ashr i32 %tmp589, 2                   ; <i32> [#uses=1]
  %arraydecay591 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx592 = getelementptr inbounds <4 x float>* %arraydecay591, i32 %shr590 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call588, <4 x float>* %arrayidx592
  %tmp593 = load i32* %i549                       ; <i32> [#uses=1]
  %add594 = add nsw i32 %tmp593, 4                ; <i32> [#uses=1]
  store i32 %add594, i32* %i549
  br label %for.cond550

for.end595:                                       ; preds = %for.cond550
  store i32 0, i32* %i597
  br label %for.cond598

for.cond598:                                      ; preds = %for.body602, %for.end595
  %tmp599 = load i32* %i597                       ; <i32> [#uses=1]
  %cmp600 = icmp slt i32 %tmp599, 512             ; <i1> [#uses=1]
  br i1 %cmp600, label %for.body602, label %for.end659

for.body602:                                      ; preds = %for.cond598
  %tmp605 = load float* %S                        ; <float> [#uses=1]
  %tmp606 = insertelement <4 x float> undef, float %tmp605, i32 0 ; <<4 x float>> [#uses=2]
  %splat607 = shufflevector <4 x float> %tmp606, <4 x float> %tmp606, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp608 = load i32* %i597                       ; <i32> [#uses=1]
  %shr609 = ashr i32 %tmp608, 2                   ; <i32> [#uses=1]
  %arraydecay610 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx611 = getelementptr inbounds <4 x float>* %arraydecay610, i32 %shr609 ; <<4 x float>*> [#uses=1]
  %tmp612 = load <4 x float>* %arrayidx611        ; <<4 x float>> [#uses=1]
  %mul613 = fmul <4 x float> %splat607, %tmp612   ; <<4 x float>> [#uses=1]
  %tmp614 = load float* %X                        ; <float> [#uses=1]
  %tmp615 = insertelement <4 x float> undef, float %tmp614, i32 0 ; <<4 x float>> [#uses=2]
  %splat616 = shufflevector <4 x float> %tmp615, <4 x float> %tmp615, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub617 = fsub <4 x float> %mul613, %splat616   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub617, <4 x float>* %callValue604
  %tmp618 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral619
  %tmp620 = load <4 x float>* %.compoundliteral619 ; <<4 x float>> [#uses=1]
  %call621 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp618, <4 x float> %tmp620) ; <<4 x float>> [#uses=1]
  store <4 x float> %call621, <4 x float>* %callValue604
  %tmp622 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %tmp623 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add624 = fadd <4 x float> %tmp623, %tmp622     ; <<4 x float>> [#uses=1]
  store <4 x float> %add624, <4 x float>* %sum
  %tmp625 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %tmp626 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %mul627 = fmul <4 x float> %tmp625, %tmp626     ; <<4 x float>> [#uses=1]
  %tmp628 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add629 = fadd <4 x float> %tmp628, %mul627     ; <<4 x float>> [#uses=1]
  store <4 x float> %add629, <4 x float>* %sum2
  %tmp630 = load i32* %i597                       ; <i32> [#uses=1]
  %add631 = add nsw i32 %tmp630, 4                ; <i32> [#uses=1]
  store i32 %add631, i32* %i597
  %tmp632 = load float* %S                        ; <float> [#uses=1]
  %tmp633 = insertelement <4 x float> undef, float %tmp632, i32 0 ; <<4 x float>> [#uses=2]
  %splat634 = shufflevector <4 x float> %tmp633, <4 x float> %tmp633, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp635 = load i32* %i597                       ; <i32> [#uses=1]
  %shr636 = ashr i32 %tmp635, 2                   ; <i32> [#uses=1]
  %arraydecay637 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx638 = getelementptr inbounds <4 x float>* %arraydecay637, i32 %shr636 ; <<4 x float>*> [#uses=1]
  %tmp639 = load <4 x float>* %arrayidx638        ; <<4 x float>> [#uses=1]
  %mul640 = fmul <4 x float> %splat634, %tmp639   ; <<4 x float>> [#uses=1]
  %tmp641 = load float* %X                        ; <float> [#uses=1]
  %tmp642 = insertelement <4 x float> undef, float %tmp641, i32 0 ; <<4 x float>> [#uses=2]
  %splat643 = shufflevector <4 x float> %tmp642, <4 x float> %tmp642, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub644 = fsub <4 x float> %mul640, %splat643   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub644, <4 x float>* %callValue604
  %tmp645 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral646
  %tmp647 = load <4 x float>* %.compoundliteral646 ; <<4 x float>> [#uses=1]
  %call648 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp645, <4 x float> %tmp647) ; <<4 x float>> [#uses=1]
  store <4 x float> %call648, <4 x float>* %callValue604
  %tmp649 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %tmp650 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add651 = fadd <4 x float> %tmp650, %tmp649     ; <<4 x float>> [#uses=1]
  store <4 x float> %add651, <4 x float>* %sum
  %tmp652 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %tmp653 = load <4 x float>* %callValue604       ; <<4 x float>> [#uses=1]
  %mul654 = fmul <4 x float> %tmp652, %tmp653     ; <<4 x float>> [#uses=1]
  %tmp655 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add656 = fadd <4 x float> %tmp655, %mul654     ; <<4 x float>> [#uses=1]
  store <4 x float> %add656, <4 x float>* %sum2
  %tmp657 = load i32* %i597                       ; <i32> [#uses=1]
  %add658 = add nsw i32 %tmp657, 4                ; <i32> [#uses=1]
  store i32 %add658, i32* %i597
  br label %for.cond598

for.end659:                                       ; preds = %for.cond598
  br label %for.inc660

for.inc660:                                       ; preds = %for.end659
  %tmp661 = load i32* %pos                        ; <i32> [#uses=1]
  %add662 = add nsw i32 %tmp661, 512              ; <i32> [#uses=1]
  store i32 %add662, i32* %pos
  br label %for.cond532

for.end663:                                       ; preds = %for.cond532
  %tmp664 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp665 = load i32* %pos                        ; <i32> [#uses=1]
  %sub666 = sub i32 %tmp664, %tmp665              ; <i32> [#uses=1]
  store i32 %sub666, i32* %n
  %tmp667 = load i32* %n                          ; <i32> [#uses=1]
  %cmp668 = icmp sgt i32 %tmp667, 0               ; <i1> [#uses=1]
  br i1 %cmp668, label %if.then670, label %if.end755

if.then670:                                       ; preds = %for.end663
  %arraydecay671 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp672 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp673 = load i32* %pos                        ; <i32> [#uses=1]
  %tmp674 = load i32* %n                          ; <i32> [#uses=1]
  %add675 = add nsw i32 %tmp673, %tmp674          ; <i32> [#uses=1]
  %sub676 = sub i32 %add675, 1                    ; <i32> [#uses=1]
  %tmp677 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %tmp678 = load i32* %threadId                   ; <i32> [#uses=1]
  %tmp679 = load i32* %optionid                   ; <i32> [#uses=1]
  call void @NormalDistributionBatch3(<4 x float>* %arraydecay671, i32 %tmp672, i32 %sub676, i32 %tmp677, i32 %tmp678, i32 %tmp679)
  store i32 0, i32* %i681
  br label %for.cond682

for.cond682:                                      ; preds = %for.inc705, %if.then670
  %tmp683 = load i32* %i681                       ; <i32> [#uses=1]
  %tmp684 = load i32* %n                          ; <i32> [#uses=1]
  %cmp685 = icmp slt i32 %tmp683, %tmp684         ; <i1> [#uses=1]
  br i1 %cmp685, label %for.body687, label %for.end708

for.body687:                                      ; preds = %for.cond682
  %tmp688 = load i32* %i681                       ; <i32> [#uses=1]
  %shr689 = ashr i32 %tmp688, 2                   ; <i32> [#uses=1]
  %arraydecay690 = getelementptr inbounds [128 x <4 x float>]* %sample, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx691 = getelementptr inbounds <4 x float>* %arraydecay690, i32 %shr689 ; <<4 x float>*> [#uses=1]
  %tmp692 = load <4 x float>* %arrayidx691        ; <<4 x float>> [#uses=1]
  %tmp693 = load float* %VBySqrtT                 ; <float> [#uses=1]
  %tmp694 = insertelement <4 x float> undef, float %tmp693, i32 0 ; <<4 x float>> [#uses=2]
  %splat695 = shufflevector <4 x float> %tmp694, <4 x float> %tmp694, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul696 = fmul <4 x float> %tmp692, %splat695   ; <<4 x float>> [#uses=1]
  %tmp697 = load float* %MuByT                    ; <float> [#uses=1]
  %tmp698 = insertelement <4 x float> undef, float %tmp697, i32 0 ; <<4 x float>> [#uses=2]
  %splat699 = shufflevector <4 x float> %tmp698, <4 x float> %tmp698, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add700 = fadd <4 x float> %mul696, %splat699   ; <<4 x float>> [#uses=1]
  %tmp701 = load i32* %i681                       ; <i32> [#uses=1]
  %shr702 = ashr i32 %tmp701, 2                   ; <i32> [#uses=1]
  %arraydecay703 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx704 = getelementptr inbounds <4 x float>* %arraydecay703, i32 %shr702 ; <<4 x float>*> [#uses=1]
  store <4 x float> %add700, <4 x float>* %arrayidx704
  br label %for.inc705

for.inc705:                                       ; preds = %for.body687
  %tmp706 = load i32* %i681                       ; <i32> [#uses=1]
  %add707 = add nsw i32 %tmp706, 4                ; <i32> [#uses=1]
  store i32 %add707, i32* %i681
  br label %for.cond682

for.end708:                                       ; preds = %for.cond682
  %tmp709 = load i32* %n                          ; <i32> [#uses=1]
  %arraydecay710 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arraydecay711 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %tmp712 = load i32* %threadId                   ; <i32> [#uses=1]
  call void @vsExp(i32 %tmp709, <4 x float>* %arraydecay710, <4 x float>* %arraydecay711, i32 %tmp712)
  store i32 0, i32* %i714
  br label %for.cond715

for.cond715:                                      ; preds = %for.inc748, %for.end708
  %tmp716 = load i32* %i714                       ; <i32> [#uses=1]
  %tmp717 = load i32* %n                          ; <i32> [#uses=1]
  %cmp718 = icmp slt i32 %tmp716, %tmp717         ; <i1> [#uses=1]
  br i1 %cmp718, label %for.body720, label %for.end751

for.body720:                                      ; preds = %for.cond715
  %tmp723 = load float* %S                        ; <float> [#uses=1]
  %tmp724 = insertelement <4 x float> undef, float %tmp723, i32 0 ; <<4 x float>> [#uses=2]
  %splat725 = shufflevector <4 x float> %tmp724, <4 x float> %tmp724, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp726 = load i32* %i714                       ; <i32> [#uses=1]
  %shr727 = ashr i32 %tmp726, 2                   ; <i32> [#uses=1]
  %arraydecay728 = getelementptr inbounds [128 x <4 x float>]* %temp, i32 0, i32 0 ; <<4 x float>*> [#uses=1]
  %arrayidx729 = getelementptr inbounds <4 x float>* %arraydecay728, i32 %shr727 ; <<4 x float>*> [#uses=1]
  %tmp730 = load <4 x float>* %arrayidx729        ; <<4 x float>> [#uses=1]
  %mul731 = fmul <4 x float> %splat725, %tmp730   ; <<4 x float>> [#uses=1]
  %tmp732 = load float* %X                        ; <float> [#uses=1]
  %tmp733 = insertelement <4 x float> undef, float %tmp732, i32 0 ; <<4 x float>> [#uses=2]
  %splat734 = shufflevector <4 x float> %tmp733, <4 x float> %tmp733, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub735 = fsub <4 x float> %mul731, %splat734   ; <<4 x float>> [#uses=1]
  store <4 x float> %sub735, <4 x float>* %callValue722
  %tmp736 = load <4 x float>* %callValue722       ; <<4 x float>> [#uses=1]
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral737
  %tmp738 = load <4 x float>* %.compoundliteral737 ; <<4 x float>> [#uses=1]
  %call739 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp736, <4 x float> %tmp738) ; <<4 x float>> [#uses=1]
  store <4 x float> %call739, <4 x float>* %callValue722
  %tmp740 = load <4 x float>* %callValue722       ; <<4 x float>> [#uses=1]
  %tmp741 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %add742 = fadd <4 x float> %tmp741, %tmp740     ; <<4 x float>> [#uses=1]
  store <4 x float> %add742, <4 x float>* %sum
  %tmp743 = load <4 x float>* %callValue722       ; <<4 x float>> [#uses=1]
  %tmp744 = load <4 x float>* %callValue722       ; <<4 x float>> [#uses=1]
  %mul745 = fmul <4 x float> %tmp743, %tmp744     ; <<4 x float>> [#uses=1]
  %tmp746 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %add747 = fadd <4 x float> %tmp746, %mul745     ; <<4 x float>> [#uses=1]
  store <4 x float> %add747, <4 x float>* %sum2
  br label %for.inc748

for.inc748:                                       ; preds = %for.body720
  %tmp749 = load i32* %i714                       ; <i32> [#uses=1]
  %add750 = add nsw i32 %tmp749, 4                ; <i32> [#uses=1]
  store i32 %add750, i32* %i714
  br label %for.cond715

for.end751:                                       ; preds = %for.cond715
  %tmp752 = load i32* %n                          ; <i32> [#uses=1]
  %tmp753 = load i32* %pos                        ; <i32> [#uses=1]
  %add754 = add nsw i32 %tmp753, %tmp752          ; <i32> [#uses=1]
  store i32 %add754, i32* %pos
  br label %if.end755

if.end755:                                        ; preds = %for.end751, %for.end663
  %tmp757 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %tmp758 = extractelement <4 x float> %tmp757, i32 0 ; <float> [#uses=1]
  %tmp759 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %tmp760 = extractelement <4 x float> %tmp759, i32 1 ; <float> [#uses=1]
  %add761 = fadd float %tmp758, %tmp760           ; <float> [#uses=1]
  %tmp762 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %tmp763 = extractelement <4 x float> %tmp762, i32 2 ; <float> [#uses=1]
  %add764 = fadd float %add761, %tmp763           ; <float> [#uses=1]
  %tmp765 = load <4 x float>* %sum                ; <<4 x float>> [#uses=1]
  %tmp766 = extractelement <4 x float> %tmp765, i32 3 ; <float> [#uses=1]
  %add767 = fadd float %add764, %tmp766           ; <float> [#uses=1]
  store float %add767, float* %totalsum
  %tmp769 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp770 = extractelement <4 x float> %tmp769, i32 0 ; <float> [#uses=1]
  %tmp771 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp772 = extractelement <4 x float> %tmp771, i32 1 ; <float> [#uses=1]
  %add773 = fadd float %tmp770, %tmp772           ; <float> [#uses=1]
  %tmp774 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp775 = extractelement <4 x float> %tmp774, i32 2 ; <float> [#uses=1]
  %add776 = fadd float %add773, %tmp775           ; <float> [#uses=1]
  %tmp777 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp778 = extractelement <4 x float> %tmp777, i32 3 ; <float> [#uses=1]
  %add779 = fadd float %add776, %tmp778           ; <float> [#uses=1]
  store float %add779, float* %totalsum2
  %tmp781 = load float* %R                        ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp781       ; <float> [#uses=1]
  %tmp782 = load float* %T                        ; <float> [#uses=1]
  %mul783 = fmul float %neg, %tmp782              ; <float> [#uses=1]
  %call784 = call float @_Z3expf(float %mul783)   ; <float> [#uses=1]
  store float %call784, float* %ExpRT
  %tmp785 = load float* %ExpRT                    ; <float> [#uses=1]
  %tmp786 = load float* %totalsum                 ; <float> [#uses=1]
  %mul787 = fmul float %tmp785, %tmp786           ; <float> [#uses=1]
  %tmp788 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv789 = sitofp i32 %tmp788 to float          ; <float> [#uses=3]
  %cmp790 = fcmp oeq float 0.000000e+000, %conv789 ; <i1> [#uses=1]
  %sel = select i1 %cmp790, float 1.000000e+000, float %conv789 ; <float> [#uses=0]
  %div791 = fdiv float %mul787, %conv789          ; <float> [#uses=1]
  %tmp792 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp793 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx794 = getelementptr inbounds %struct.anon addrspace(1)* %tmp793, i32 %tmp792 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp795 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx794, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %div791, float addrspace(1)* %tmp795
  %tmp797 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv798 = sitofp i32 %tmp797 to float          ; <float> [#uses=1]
  %tmp799 = load float* %totalsum2                ; <float> [#uses=1]
  %mul800 = fmul float %conv798, %tmp799          ; <float> [#uses=1]
  %tmp801 = load float* %totalsum                 ; <float> [#uses=1]
  %tmp802 = load float* %totalsum                 ; <float> [#uses=1]
  %mul803 = fmul float %tmp801, %tmp802           ; <float> [#uses=1]
  %sub804 = fsub float %mul800, %mul803           ; <float> [#uses=1]
  %tmp805 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv806 = sitofp i32 %tmp805 to float          ; <float> [#uses=1]
  %tmp807 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub808 = sub i32 %tmp807, 1                    ; <i32> [#uses=1]
  %conv809 = sitofp i32 %sub808 to float          ; <float> [#uses=1]
  %mul810 = fmul float %conv806, %conv809         ; <float> [#uses=3]
  %cmp811 = fcmp oeq float 0.000000e+000, %mul810 ; <i1> [#uses=1]
  %sel812 = select i1 %cmp811, float 1.000000e+000, float %mul810 ; <float> [#uses=0]
  %div813 = fdiv float %sub804, %mul810           ; <float> [#uses=1]
  %call814 = call float @_Z4sqrtf(float %div813)  ; <float> [#uses=1]
  store float %call814, float* %stdDev
  %tmp815 = load float* %ExpRT                    ; <float> [#uses=1]
  %conv816 = fpext float %tmp815 to double        ; <double> [#uses=1]
  %mul817 = fmul double %conv816, 1.960000e+000   ; <double> [#uses=1]
  %tmp818 = load float* %stdDev                   ; <float> [#uses=1]
  %conv819 = fpext float %tmp818 to double        ; <double> [#uses=1]
  %mul820 = fmul double %mul817, %conv819         ; <double> [#uses=1]
  %tmp821 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv822 = sitofp i32 %tmp821 to float          ; <float> [#uses=1]
  %call823 = call float @_Z4sqrtf(float %conv822) ; <float> [#uses=1]
  %conv824 = fpext float %call823 to double       ; <double> [#uses=3]
  %cmp825 = fcmp oeq double 0.000000e+000, %conv824 ; <i1> [#uses=1]
  %sel826 = select i1 %cmp825, double 1.000000e+000, double %conv824 ; <double> [#uses=0]
  %div827 = fdiv double %mul820, %conv824         ; <double> [#uses=1]
  %conv828 = fptrunc double %div827 to float      ; <float> [#uses=1]
  %tmp829 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp830 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx831 = getelementptr inbounds %struct.anon addrspace(1)* %tmp830, i32 %tmp829 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp832 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx831, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %conv828, float addrspace(1)* %tmp832
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare float @_Z4sqrtf(float)

declare <4 x float> @_Z3maxDv4_fS_(<4 x float>, <4 x float>)

declare float @_Z3expf(float)

; CHECK: ret
define void @wlMonteCarloKernel_PrecalculatedSamples(%struct.anon addrspace(1)* %pOptionValue, %0 addrspace(1)* %pOptionData, <4 x float> addrspace(1)* %pSamples, i32 %pathN) nounwind {
entry:
  %pOptionValue.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=3]
  %pOptionData.addr = alloca %0 addrspace(1)*, align 4 ; <%0 addrspace(1)**> [#uses=6]
  %pSamples.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pathN.addr = alloca i32, align 4               ; <i32*> [#uses=7]
  %optionid = alloca i32, align 4                 ; <i32*> [#uses=8]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %S = alloca float, align 4                      ; <float*> [#uses=2]
  %X = alloca float, align 4                      ; <float*> [#uses=2]
  %V = alloca float, align 4                      ; <float*> [#uses=4]
  %MuByT = alloca float, align 4                  ; <float*> [#uses=2]
  %VBySqrtT = alloca float, align 4               ; <float*> [#uses=2]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=5]
  %zero4 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %sum = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=7]
  %.compoundliteral44 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %sum2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=7]
  %.compoundliteral47 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %callValue = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=6]
  %totalsum = alloca float, align 4               ; <float*> [#uses=4]
  %totalsum2 = alloca float, align 4              ; <float*> [#uses=2]
  %ExpRT = alloca float, align 4                  ; <float*> [#uses=3]
  %stdDev = alloca float, align 4                 ; <float*> [#uses=2]
  store %struct.anon addrspace(1)* %pOptionValue, %struct.anon addrspace(1)** %pOptionValue.addr
  store %0 addrspace(1)* %pOptionData, %0 addrspace(1)** %pOptionData.addr
  store <4 x float> addrspace(1)* %pSamples, <4 x float> addrspace(1)** %pSamples.addr
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
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp42 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %zero4
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral44
  %tmp45 = load <4 x float>* %.compoundliteral44  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp45, <4 x float>* %sum
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral47
  %tmp48 = load <4 x float>* %.compoundliteral47  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp48, <4 x float>* %sum2
  store i32 0, i32* %pos
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp49 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp50 = load i32* %pathN.addr                  ; <i32> [#uses=1]
  %sub51 = sub i32 %tmp50, 3                      ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp49, %sub51              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp53 = load float* %S                         ; <float> [#uses=1]
  %tmp54 = insertelement <4 x float> undef, float %tmp53, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp54, <4 x float> %tmp54, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp55 = load i32* %pos                         ; <i32> [#uses=1]
  %shr = ashr i32 %tmp55, 2                       ; <i32> [#uses=1]
  %tmp56 = load <4 x float> addrspace(1)** %pSamples.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx57 = getelementptr inbounds <4 x float> addrspace(1)* %tmp56, i32 %shr ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp58 = load <4 x float> addrspace(1)* %arrayidx57 ; <<4 x float>> [#uses=1]
  %tmp59 = load float* %VBySqrtT                  ; <float> [#uses=1]
  %tmp60 = insertelement <4 x float> undef, float %tmp59, i32 0 ; <<4 x float>> [#uses=2]
  %splat61 = shufflevector <4 x float> %tmp60, <4 x float> %tmp60, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul62 = fmul <4 x float> %tmp58, %splat61      ; <<4 x float>> [#uses=1]
  %tmp63 = load float* %MuByT                     ; <float> [#uses=1]
  %tmp64 = insertelement <4 x float> undef, float %tmp63, i32 0 ; <<4 x float>> [#uses=2]
  %splat65 = shufflevector <4 x float> %tmp64, <4 x float> %tmp64, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %mul62, %splat65        ; <<4 x float>> [#uses=1]
  %call66 = call <4 x float> @_Z3expDv4_f(<4 x float> %add) ; <<4 x float>> [#uses=1]
  %mul67 = fmul <4 x float> %splat, %call66       ; <<4 x float>> [#uses=1]
  %tmp68 = load float* %X                         ; <float> [#uses=1]
  %tmp69 = insertelement <4 x float> undef, float %tmp68, i32 0 ; <<4 x float>> [#uses=2]
  %splat70 = shufflevector <4 x float> %tmp69, <4 x float> %tmp69, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub71 = fsub <4 x float> %mul67, %splat70      ; <<4 x float>> [#uses=1]
  store <4 x float> %sub71, <4 x float>* %callValue
  %tmp72 = load <4 x float>* %callValue           ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %zero4               ; <<4 x float>> [#uses=1]
  %call74 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp72, <4 x float> %tmp73) ; <<4 x float>> [#uses=1]
  store <4 x float> %call74, <4 x float>* %callValue
  %tmp75 = load <4 x float>* %callValue           ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %add77 = fadd <4 x float> %tmp76, %tmp75        ; <<4 x float>> [#uses=1]
  store <4 x float> %add77, <4 x float>* %sum
  %tmp78 = load <4 x float>* %callValue           ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>* %callValue           ; <<4 x float>> [#uses=1]
  %mul80 = fmul <4 x float> %tmp78, %tmp79        ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %sum2                ; <<4 x float>> [#uses=1]
  %add82 = fadd <4 x float> %tmp81, %mul80        ; <<4 x float>> [#uses=1]
  store <4 x float> %add82, <4 x float>* %sum2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp83 = load i32* %pos                         ; <i32> [#uses=1]
  %add84 = add nsw i32 %tmp83, 4                  ; <i32> [#uses=1]
  store i32 %add84, i32* %pos
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp86 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %tmp87 = extractelement <4 x float> %tmp86, i32 0 ; <float> [#uses=1]
  %tmp88 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %tmp89 = extractelement <4 x float> %tmp88, i32 1 ; <float> [#uses=1]
  %add90 = fadd float %tmp87, %tmp89              ; <float> [#uses=1]
  %tmp91 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %tmp92 = extractelement <4 x float> %tmp91, i32 2 ; <float> [#uses=1]
  %add93 = fadd float %add90, %tmp92              ; <float> [#uses=1]
  %tmp94 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %tmp95 = extractelement <4 x float> %tmp94, i32 3 ; <float> [#uses=1]
  %add96 = fadd float %add93, %tmp95              ; <float> [#uses=1]
  store float %add96, float* %totalsum
  %tmp98 = load <4 x float>* %sum2                ; <<4 x float>> [#uses=1]
  %tmp99 = extractelement <4 x float> %tmp98, i32 0 ; <float> [#uses=1]
  %tmp100 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp101 = extractelement <4 x float> %tmp100, i32 1 ; <float> [#uses=1]
  %add102 = fadd float %tmp99, %tmp101            ; <float> [#uses=1]
  %tmp103 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp104 = extractelement <4 x float> %tmp103, i32 2 ; <float> [#uses=1]
  %add105 = fadd float %add102, %tmp104           ; <float> [#uses=1]
  %tmp106 = load <4 x float>* %sum2               ; <<4 x float>> [#uses=1]
  %tmp107 = extractelement <4 x float> %tmp106, i32 3 ; <float> [#uses=1]
  %add108 = fadd float %add105, %tmp107           ; <float> [#uses=1]
  store float %add108, float* %totalsum2
  %tmp110 = load float* %R                        ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp110       ; <float> [#uses=1]
  %tmp111 = load float* %T                        ; <float> [#uses=1]
  %mul112 = fmul float %neg, %tmp111              ; <float> [#uses=1]
  %call113 = call float @_Z3expf(float %mul112)   ; <float> [#uses=1]
  store float %call113, float* %ExpRT
  %tmp114 = load float* %ExpRT                    ; <float> [#uses=1]
  %tmp115 = load float* %totalsum                 ; <float> [#uses=1]
  %mul116 = fmul float %tmp114, %tmp115           ; <float> [#uses=1]
  %tmp117 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp117 to float             ; <float> [#uses=3]
  %cmp118 = fcmp oeq float 0.000000e+000, %conv   ; <i1> [#uses=1]
  %sel = select i1 %cmp118, float 1.000000e+000, float %conv ; <float> [#uses=0]
  %div = fdiv float %mul116, %conv                ; <float> [#uses=1]
  %tmp119 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp120 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds %struct.anon addrspace(1)* %tmp120, i32 %tmp119 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp122 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx121, i32 0, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %div, float addrspace(1)* %tmp122
  %tmp124 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv125 = sitofp i32 %tmp124 to float          ; <float> [#uses=1]
  %tmp126 = load float* %totalsum2                ; <float> [#uses=1]
  %mul127 = fmul float %conv125, %tmp126          ; <float> [#uses=1]
  %tmp128 = load float* %totalsum                 ; <float> [#uses=1]
  %tmp129 = load float* %totalsum                 ; <float> [#uses=1]
  %mul130 = fmul float %tmp128, %tmp129           ; <float> [#uses=1]
  %sub131 = fsub float %mul127, %mul130           ; <float> [#uses=1]
  %tmp132 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv133 = sitofp i32 %tmp132 to float          ; <float> [#uses=1]
  %tmp134 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %sub135 = sub i32 %tmp134, 1                    ; <i32> [#uses=1]
  %conv136 = sitofp i32 %sub135 to float          ; <float> [#uses=1]
  %mul137 = fmul float %conv133, %conv136         ; <float> [#uses=3]
  %cmp138 = fcmp oeq float 0.000000e+000, %mul137 ; <i1> [#uses=1]
  %sel139 = select i1 %cmp138, float 1.000000e+000, float %mul137 ; <float> [#uses=0]
  %div140 = fdiv float %sub131, %mul137           ; <float> [#uses=1]
  %call141 = call float @_Z4sqrtf(float %div140)  ; <float> [#uses=1]
  store float %call141, float* %stdDev
  %tmp142 = load float* %ExpRT                    ; <float> [#uses=1]
  %conv143 = fpext float %tmp142 to double        ; <double> [#uses=1]
  %mul144 = fmul double %conv143, 1.960000e+000   ; <double> [#uses=1]
  %tmp145 = load float* %stdDev                   ; <float> [#uses=1]
  %conv146 = fpext float %tmp145 to double        ; <double> [#uses=1]
  %mul147 = fmul double %mul144, %conv146         ; <double> [#uses=1]
  %tmp148 = load i32* %pathN.addr                 ; <i32> [#uses=1]
  %conv149 = sitofp i32 %tmp148 to float          ; <float> [#uses=1]
  %call150 = call float @_Z4sqrtf(float %conv149) ; <float> [#uses=1]
  %conv151 = fpext float %call150 to double       ; <double> [#uses=3]
  %cmp152 = fcmp oeq double 0.000000e+000, %conv151 ; <i1> [#uses=1]
  %sel153 = select i1 %cmp152, double 1.000000e+000, double %conv151 ; <double> [#uses=0]
  %div154 = fdiv double %mul147, %conv151         ; <double> [#uses=1]
  %conv155 = fptrunc double %div154 to float      ; <float> [#uses=1]
  %tmp156 = load i32* %optionid                   ; <i32> [#uses=1]
  %tmp157 = load %struct.anon addrspace(1)** %pOptionValue.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %arrayidx158 = getelementptr inbounds %struct.anon addrspace(1)* %tmp157, i32 %tmp156 ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp159 = getelementptr inbounds %struct.anon addrspace(1)* %arrayidx158, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %conv155, float addrspace(1)* %tmp159
  ret void
}
