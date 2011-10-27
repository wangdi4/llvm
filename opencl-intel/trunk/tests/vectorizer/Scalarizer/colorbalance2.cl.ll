; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\colorbalance2.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type { <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float>, <4 x float> }

@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [101 x i8] c"float4 const, float4 const, float4 const, uint const, kernelArgs __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[101 x i8]*> [#uses=1]
@opencl_colorbalance2_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_colorbalance2_GPU_parameters = appending global [135 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[135 x i8]*> [#uses=1]
@opencl_colorbalance2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_colorbalance2_parameters = appending global [147 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[147 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float>, <4 x float>, <4 x float>, i32, %struct.anon addrspace(1)*)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([101 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, %struct.anon addrspace(2)*)* @colorbalance2_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_colorbalance2_GPU_locals to i8*), i8* getelementptr inbounds ([135 x i8]* @opencl_colorbalance2_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, %struct.anon addrspace(2)*)* @colorbalance2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_colorbalance2_locals to i8*), i8* getelementptr inbounds ([147 x i8]* @opencl_colorbalance2_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @SetGammaCorrected(<4 x float> %color, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %color.addr = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=5]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=10]
  %kOne = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %.compoundliteral2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %EPSILON = alloca float, align 4                ; <float*> [#uses=3]
  %flag1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=5]
  %B = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=7]
  %C = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=5]
  %D = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=10]
  %T1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=6]
  %T2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %T3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=6]
  %outColor = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=4]
  store <4 x float> %color, <4 x float>* %color.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %kOne
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral2
  %tmp3 = load <4 x float>* %.compoundliteral2    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %kZero
  store float 0x3EE4F8B580000000, float* %EPSILON
  %tmp6 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp7 = getelementptr inbounds %struct.anon addrspace(2)* %tmp6, i32 0, i32 3 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp8 = load <4 x float> addrspace(2)* %tmp7    ; <<4 x float>> [#uses=1]
  %tmp9 = load <4 x float>* %kOne                 ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp8, <4 x float> %tmp9) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %flag1
  %tmp10 = load float* %EPSILON                   ; <float> [#uses=1]
  %tmp11 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %call12 = call <4 x float> @_Z4stepfDv4_f(float %tmp10, <4 x float> %tmp11) ; <<4 x float>> [#uses=1]
  %tmp13 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %tmp14 = load float* %EPSILON                   ; <float> [#uses=1]
  %tmp15 = insertelement <4 x float> undef, float %tmp14, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp15, <4 x float> %tmp15, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp13, %splat          ; <<4 x float>> [#uses=1]
  %tmp16 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp17 = getelementptr inbounds %struct.anon addrspace(2)* %tmp16, i32 0, i32 4 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp18 = load <4 x float> addrspace(2)* %tmp17  ; <<4 x float>> [#uses=1]
  %call19 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %add, <4 x float> %tmp18) ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %call12, %call19        ; <<4 x float>> [#uses=1]
  %tmp20 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %mul21 = fmul <4 x float> %tmp20, %mul          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul21, <4 x float>* %flag1
  %tmp23 = load <4 x float>* %kZero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp23, <4 x float>* %B
  %tmp26 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %tmp27 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp28 = getelementptr inbounds %struct.anon addrspace(2)* %tmp27, i32 0, i32 4 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp29 = load <4 x float> addrspace(2)* %tmp28  ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %tmp29 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp29 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp26, %tmp29          ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %B
  %tmp30 = load <4 x float>* %B                   ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp30 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %C
  %tmp31 = load <4 x float>* %C                   ; <<4 x float>> [#uses=1]
  %tmp32 = load <4 x float>* %C                   ; <<4 x float>> [#uses=1]
  %mul33 = fmul <4 x float> %tmp31, %tmp32        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul33, <4 x float>* %D
  %tmp34 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp35 = getelementptr inbounds %struct.anon addrspace(2)* %tmp34, i32 0, i32 4 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp36 = load <4 x float> addrspace(2)* %tmp35  ; <<4 x float>> [#uses=1]
  %tmp37 = load <4 x float>* %D                   ; <<4 x float>> [#uses=1]
  %mul38 = fmul <4 x float> %tmp37, %tmp36        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul38, <4 x float>* %D
  %tmp39 = load <4 x float>* %B                   ; <<4 x float>> [#uses=1]
  %tmp40 = load <4 x float>* %D                   ; <<4 x float>> [#uses=1]
  %mul41 = fmul <4 x float> %tmp40, %tmp39        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul41, <4 x float>* %D
  %tmp42 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp43 = getelementptr inbounds %struct.anon addrspace(2)* %tmp42, i32 0, i32 6 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp44 = load <4 x float> addrspace(2)* %tmp43  ; <<4 x float>> [#uses=1]
  %tmp45 = load <4 x float>* %D                   ; <<4 x float>> [#uses=1]
  %mul46 = fmul <4 x float> %tmp45, %tmp44        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul46, <4 x float>* %D
  %tmp48 = load <4 x float>* %B                   ; <<4 x float>> [#uses=1]
  %sub49 = fsub <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, %tmp48 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub49, <4 x float>* %T1
  %tmp50 = load <4 x float>* %C                   ; <<4 x float>> [#uses=1]
  %tmp51 = load <4 x float>* %T1                  ; <<4 x float>> [#uses=1]
  %add52 = fadd <4 x float> %tmp51, %tmp50        ; <<4 x float>> [#uses=1]
  store <4 x float> %add52, <4 x float>* %T1
  %tmp54 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp55 = getelementptr inbounds %struct.anon addrspace(2)* %tmp54, i32 0, i32 7 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp56 = load <4 x float> addrspace(2)* %tmp55  ; <<4 x float>> [#uses=1]
  %tmp57 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp58 = getelementptr inbounds %struct.anon addrspace(2)* %tmp57, i32 0, i32 4 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp59 = load <4 x float> addrspace(2)* %tmp58  ; <<4 x float>> [#uses=1]
  %mul60 = fmul <4 x float> %tmp56, %tmp59        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul60, <4 x float>* %T2
  %tmp61 = load <4 x float>* %C                   ; <<4 x float>> [#uses=1]
  %tmp62 = load <4 x float>* %T2                  ; <<4 x float>> [#uses=1]
  %mul63 = fmul <4 x float> %tmp62, %tmp61        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul63, <4 x float>* %T2
  %tmp64 = load <4 x float>* %T2                  ; <<4 x float>> [#uses=1]
  %tmp65 = load <4 x float>* %T1                  ; <<4 x float>> [#uses=1]
  %sub66 = fsub <4 x float> %tmp65, %tmp64        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub66, <4 x float>* %T1
  %tmp68 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp69 = getelementptr inbounds %struct.anon addrspace(2)* %tmp68, i32 0, i32 5 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp70 = load <4 x float> addrspace(2)* %tmp69  ; <<4 x float>> [#uses=1]
  %tmp71 = load <4 x float>* %T1                  ; <<4 x float>> [#uses=1]
  %mul72 = fmul <4 x float> %tmp70, %tmp71        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul72, <4 x float>* %T3
  %tmp73 = load <4 x float>* %B                   ; <<4 x float>> [#uses=1]
  %tmp74 = load <4 x float>* %T3                  ; <<4 x float>> [#uses=1]
  %mul75 = fmul <4 x float> %tmp74, %tmp73        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul75, <4 x float>* %T3
  %tmp76 = load <4 x float>* %B                   ; <<4 x float>> [#uses=1]
  %tmp77 = load <4 x float>* %T3                  ; <<4 x float>> [#uses=1]
  %mul78 = fmul <4 x float> %tmp77, %tmp76        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul78, <4 x float>* %T3
  %tmp79 = load <4 x float>* %T3                  ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %D                   ; <<4 x float>> [#uses=1]
  %add81 = fadd <4 x float> %tmp80, %tmp79        ; <<4 x float>> [#uses=1]
  store <4 x float> %add81, <4 x float>* %D
  %tmp83 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %tmp84 = load <4 x float>* %D                   ; <<4 x float>> [#uses=1]
  %mul85 = fmul <4 x float> %tmp83, %tmp84        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul85, <4 x float>* %outColor
  %tmp86 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %sub87 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp86 ; <<4 x float>> [#uses=1]
  %tmp88 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %call89 = call <4 x float> @_Z4fabsDv4_f(<4 x float> %tmp88) ; <<4 x float>> [#uses=1]
  %tmp90 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp91 = getelementptr inbounds %struct.anon addrspace(2)* %tmp90, i32 0, i32 3 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp92 = load <4 x float> addrspace(2)* %tmp91  ; <<4 x float>> [#uses=1]
  %call93 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %call89, <4 x float> %tmp92) ; <<4 x float>> [#uses=1]
  %mul94 = fmul <4 x float> %sub87, %call93       ; <<4 x float>> [#uses=1]
  %tmp95 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %add96 = fadd <4 x float> %tmp95, %mul94        ; <<4 x float>> [#uses=1]
  store <4 x float> %add96, <4 x float>* %outColor
  %tmp97 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp97, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z4stepDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z4stepfDv4_f(float, <4 x float>)

declare <4 x float> @_Z3powDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z4fabsDv4_f(<4 x float>)

; CHECK: ret
define <4 x float> @SetGamma(<4 x float> %color, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %color.addr = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=5]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=11]
  %ratio = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %flag1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %outColor = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=6]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=10]
  %temp2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  store <4 x float> %color, <4 x float>* %color.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load <4 x float>* %color.addr            ; <<4 x float>> [#uses=1]
  %tmp1 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp2 = getelementptr inbounds %struct.anon addrspace(2)* %tmp1, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp3 = load <4 x float> addrspace(2)* %tmp2    ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp, %tmp3             ; <<4 x float>> [#uses=1]
  %tmp4 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp5 = getelementptr inbounds %struct.anon addrspace(2)* %tmp4, i32 0, i32 8 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp6 = load <4 x float> addrspace(2)* %tmp5    ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %tmp6 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp6 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %sub, %tmp6             ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %ratio
  %tmp8 = load <4 x float>* %color.addr           ; <<4 x float>> [#uses=1]
  %tmp9 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp10 = getelementptr inbounds %struct.anon addrspace(2)* %tmp9, i32 0, i32 2 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp11 = load <4 x float> addrspace(2)* %tmp10  ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp8, <4 x float> %tmp11) ; <<4 x float>> [#uses=1]
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp14 = load <4 x float> addrspace(2)* %tmp13  ; <<4 x float>> [#uses=1]
  %tmp15 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %call16 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp14, <4 x float> %tmp15) ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %call, %call16          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %flag1
  %tmp18 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %tmp19 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp20 = getelementptr inbounds %struct.anon addrspace(2)* %tmp19, i32 0, i32 9 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp21 = load <4 x float> addrspace(2)* %tmp20  ; <<4 x float>> [#uses=1]
  %mul22 = fmul <4 x float> %tmp18, %tmp21        ; <<4 x float>> [#uses=1]
  %tmp23 = load <4 x float>* %ratio               ; <<4 x float>> [#uses=1]
  %mul24 = fmul <4 x float> %mul22, %tmp23        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul24, <4 x float>* %outColor
  %tmp27 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp28 = getelementptr inbounds %struct.anon addrspace(2)* %tmp27, i32 0, i32 9 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp29 = load <4 x float> addrspace(2)* %tmp28  ; <<4 x float>> [#uses=1]
  %sub30 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp29 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub30, <4 x float>* %temp2
  %tmp31 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp32 = getelementptr inbounds %struct.anon addrspace(2)* %tmp31, i32 0, i32 10 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp33 = load <4 x float> addrspace(2)* %tmp32  ; <<4 x float>> [#uses=1]
  %tmp34 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %mul35 = fmul <4 x float> %tmp33, %tmp34        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul35, <4 x float>* %temp
  %tmp36 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %tmp37 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call38 = call <4 x float> @SetGammaCorrected(<4 x float> %tmp36, %struct.anon addrspace(2)* %tmp37) ; <<4 x float>> [#uses=1]
  %tmp39 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %mul40 = fmul <4 x float> %tmp39, %call38       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul40, <4 x float>* %temp
  %tmp41 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %tmp42 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp42, %tmp41          ; <<4 x float>> [#uses=1]
  store <4 x float> %add, <4 x float>* %outColor
  %tmp43 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %tmp44 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %mul45 = fmul <4 x float> %tmp43, %tmp44        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul45, <4 x float>* %temp
  %tmp46 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp47 = getelementptr inbounds %struct.anon addrspace(2)* %tmp46, i32 0, i32 10 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp48 = load <4 x float> addrspace(2)* %tmp47  ; <<4 x float>> [#uses=1]
  %sub49 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp48 ; <<4 x float>> [#uses=1]
  %tmp50 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %mul51 = fmul <4 x float> %tmp50, %sub49        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul51, <4 x float>* %temp
  %tmp52 = load <4 x float>* %ratio               ; <<4 x float>> [#uses=1]
  %tmp53 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call54 = call <4 x float> @SetGammaCorrected(<4 x float> %tmp52, %struct.anon addrspace(2)* %tmp53) ; <<4 x float>> [#uses=1]
  %tmp55 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %mul56 = fmul <4 x float> %tmp55, %call54       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul56, <4 x float>* %temp
  %tmp57 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %tmp58 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %add59 = fadd <4 x float> %tmp58, %tmp57        ; <<4 x float>> [#uses=1]
  store <4 x float> %add59, <4 x float>* %outColor
  %tmp60 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp60, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define <4 x float> @doChannel(<4 x float> %color, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %color.addr = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=5]
  %kOne = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %EPSILON = alloca float, align 4                ; <float*> [#uses=2]
  %outColor = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=8]
  %flag1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  store <4 x float> %color, <4 x float>* %color.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %kOne
  store float 0x3EE4F8B580000000, float* %EPSILON
  %tmp4 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp5 = getelementptr inbounds %struct.anon addrspace(2)* %tmp4, i32 0, i32 2 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp6 = load <4 x float> addrspace(2)* %tmp5    ; <<4 x float>> [#uses=1]
  %tmp7 = load <4 x float>* %color.addr           ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp6, <4 x float> %tmp7) ; <<4 x float>> [#uses=1]
  %tmp8 = load <4 x float>* %color.addr           ; <<4 x float>> [#uses=1]
  %tmp9 = load <4 x float>* %kOne                 ; <<4 x float>> [#uses=1]
  %call10 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp8, <4 x float> %tmp9) ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %call, %call10          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %flag1
  %tmp11 = load <4 x float>* %kOne                ; <<4 x float>> [#uses=1]
  %tmp12 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp11, %tmp12          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %outColor
  %tmp13 = load <4 x float>* %color.addr          ; <<4 x float>> [#uses=1]
  %tmp14 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call15 = call <4 x float> @SetGamma(<4 x float> %tmp13, %struct.anon addrspace(2)* %tmp14) ; <<4 x float>> [#uses=1]
  %tmp16 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %mul17 = fmul <4 x float> %tmp16, %call15       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul17, <4 x float>* %outColor
  %tmp18 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon addrspace(2)* %tmp18, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp20 = load <4 x float> addrspace(2)* %tmp19  ; <<4 x float>> [#uses=1]
  %tmp21 = load float* %EPSILON                   ; <float> [#uses=1]
  %tmp22 = insertelement <4 x float> undef, float %tmp21, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp22, <4 x float> %tmp22, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp20, %splat          ; <<4 x float>> [#uses=1]
  %tmp23 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp24 = getelementptr inbounds %struct.anon addrspace(2)* %tmp23, i32 0, i32 2 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp25 = load <4 x float> addrspace(2)* %tmp24  ; <<4 x float>> [#uses=1]
  %call26 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %add, <4 x float> %tmp25) ; <<4 x float>> [#uses=1]
  %tmp27 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %mul28 = fmul <4 x float> %tmp27, %call26       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul28, <4 x float>* %outColor
  %tmp29 = load <4 x float>* %flag1               ; <<4 x float>> [#uses=1]
  %tmp30 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %add31 = fadd <4 x float> %tmp30, %tmp29        ; <<4 x float>> [#uses=1]
  store <4 x float> %add31, <4 x float>* %outColor
  %tmp32 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define void @evaluateDependents(<4 x float> %shadowBalance, <4 x float> %midtoneBalance, <4 x float> %hilightBalance, i32 %preserveLuminosity, %struct.anon addrspace(1)* %pArgs) nounwind {
entry:
  %shadowBalance.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %midtoneBalance.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %hilightBalance.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %preserveLuminosity.addr = alloca i32, align 4  ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=65]
  %kOne = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=5]
  %.compoundliteral2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %kMaxConst = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=5]
  %.compoundliteral5 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %EPSILON = alloca float, align 4                ; <float*> [#uses=10]
  %middle = alloca float, align 4                 ; <float*> [#uses=2]
  %shadow = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=9]
  %midtone = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=9]
  %hilight = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=9]
  %lower = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=7]
  %.compoundliteral21 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tmp2 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %.compoundliteral31 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tmp344 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral45 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %upper = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=7]
  %.compoundliteral59 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tmp4 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %.compoundliteral70 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tmp5 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %.compoundliteral84 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %tolerance = alloca float, align 4              ; <float*> [#uses=7]
  %flagGamma = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=8]
  %stepTmp = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %.compoundliteral280 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> %shadowBalance, <4 x float>* %shadowBalance.addr
  store <4 x float> %midtoneBalance, <4 x float>* %midtoneBalance.addr
  store <4 x float> %hilightBalance, <4 x float>* %hilightBalance.addr
  store i32 %preserveLuminosity, i32* %preserveLuminosity.addr
  store %struct.anon addrspace(1)* %pArgs, %struct.anon addrspace(1)** %pArgs.addr
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %kOne
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral2
  %tmp3 = load <4 x float>* %.compoundliteral2    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %kZero
  store <4 x float> <float 0x3F70101020000000, float 0x3F70101020000000, float 0x3F70101020000000, float 0x3F70101020000000>, <4 x float>* %.compoundliteral5
  %tmp6 = load <4 x float>* %.compoundliteral5    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp6, <4 x float>* %kMaxConst
  store float 0x3EE4F8B580000000, float* %EPSILON
  %tmp10 = load <4 x float>* %shadowBalance.addr  ; <<4 x float>> [#uses=1]
  %tmp11 = load <4 x float>* %kMaxConst           ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp10, %tmp11          ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %shadow
  %tmp13 = load <4 x float>* %midtoneBalance.addr ; <<4 x float>> [#uses=1]
  %tmp14 = load <4 x float>* %kMaxConst           ; <<4 x float>> [#uses=1]
  %mul15 = fmul <4 x float> %tmp13, %tmp14        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul15, <4 x float>* %midtone
  %tmp17 = load <4 x float>* %hilightBalance.addr ; <<4 x float>> [#uses=1]
  %tmp18 = load <4 x float>* %kMaxConst           ; <<4 x float>> [#uses=1]
  %mul19 = fmul <4 x float> %tmp17, %tmp18        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul19, <4 x float>* %hilight
  %tmp22 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp23 = extractelement <4 x float> %tmp22, i32 2 ; <float> [#uses=0]
  %0 = shufflevector <4 x float> %tmp22, <4 x float> undef, <4 x i32> <i32 2, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp24 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp25 = extractelement <4 x float> %tmp24, i32 2 ; <float> [#uses=0]
  %1 = shufflevector <4 x float> %tmp22, <4 x float> %tmp24, <4 x i32> <i32 2, i32 6, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp26 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp27 = extractelement <4 x float> %tmp26, i32 2 ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> %1, float %tmp27, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit28 = insertelement <4 x float> %vecinit, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit28, <4 x float>* %.compoundliteral21
  %tmp29 = load <4 x float>* %.compoundliteral21  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp29, <4 x float>* %lower
  %tmp32 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp33 = extractelement <4 x float> %tmp32, i32 1 ; <float> [#uses=0]
  %2 = shufflevector <4 x float> %tmp32, <4 x float> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp34 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp35 = extractelement <4 x float> %tmp34, i32 1 ; <float> [#uses=0]
  %3 = shufflevector <4 x float> %tmp32, <4 x float> %tmp34, <4 x i32> <i32 1, i32 5, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp36 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp37 = extractelement <4 x float> %tmp36, i32 1 ; <float> [#uses=1]
  %vecinit38 = insertelement <4 x float> %3, float %tmp37, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit39 = insertelement <4 x float> %vecinit38, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit39, <4 x float>* %.compoundliteral31
  %tmp40 = load <4 x float>* %.compoundliteral31  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp40, <4 x float>* %tmp2
  %tmp41 = load <4 x float>* %lower               ; <<4 x float>> [#uses=1]
  %tmp42 = load <4 x float>* %tmp2                ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z3minDv4_fS_(<4 x float> %tmp41, <4 x float> %tmp42) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %lower
  %tmp46 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp47 = extractelement <4 x float> %tmp46, i32 0 ; <float> [#uses=0]
  %4 = shufflevector <4 x float> %tmp46, <4 x float> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp48 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp49 = extractelement <4 x float> %tmp48, i32 0 ; <float> [#uses=0]
  %5 = shufflevector <4 x float> %tmp46, <4 x float> %tmp48, <4 x i32> <i32 0, i32 4, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp50 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp51 = extractelement <4 x float> %tmp50, i32 0 ; <float> [#uses=1]
  %vecinit52 = insertelement <4 x float> %5, float %tmp51, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit53 = insertelement <4 x float> %vecinit52, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit53, <4 x float>* %.compoundliteral45
  %tmp54 = load <4 x float>* %.compoundliteral45  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %tmp344
  %tmp55 = load <4 x float>* %lower               ; <<4 x float>> [#uses=1]
  %tmp56 = load <4 x float>* %tmp344              ; <<4 x float>> [#uses=1]
  %call57 = call <4 x float> @_Z3minDv4_fS_(<4 x float> %tmp55, <4 x float> %tmp56) ; <<4 x float>> [#uses=1]
  store <4 x float> %call57, <4 x float>* %lower
  %tmp60 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp61 = extractelement <4 x float> %tmp60, i32 2 ; <float> [#uses=0]
  %6 = shufflevector <4 x float> %tmp60, <4 x float> undef, <4 x i32> <i32 2, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp62 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp63 = extractelement <4 x float> %tmp62, i32 2 ; <float> [#uses=0]
  %7 = shufflevector <4 x float> %tmp60, <4 x float> %tmp62, <4 x i32> <i32 2, i32 6, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp64 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp65 = extractelement <4 x float> %tmp64, i32 2 ; <float> [#uses=1]
  %vecinit66 = insertelement <4 x float> %7, float %tmp65, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit67 = insertelement <4 x float> %vecinit66, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit67, <4 x float>* %.compoundliteral59
  %tmp68 = load <4 x float>* %.compoundliteral59  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68, <4 x float>* %upper
  %tmp71 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp72 = extractelement <4 x float> %tmp71, i32 1 ; <float> [#uses=0]
  %8 = shufflevector <4 x float> %tmp71, <4 x float> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp73 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp74 = extractelement <4 x float> %tmp73, i32 1 ; <float> [#uses=0]
  %9 = shufflevector <4 x float> %tmp71, <4 x float> %tmp73, <4 x i32> <i32 1, i32 5, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp75 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp76 = extractelement <4 x float> %tmp75, i32 1 ; <float> [#uses=1]
  %vecinit77 = insertelement <4 x float> %9, float %tmp76, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit78 = insertelement <4 x float> %vecinit77, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit78, <4 x float>* %.compoundliteral70
  %tmp79 = load <4 x float>* %.compoundliteral70  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp79, <4 x float>* %tmp4
  %tmp80 = load <4 x float>* %upper               ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %tmp4                ; <<4 x float>> [#uses=1]
  %call82 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp80, <4 x float> %tmp81) ; <<4 x float>> [#uses=1]
  store <4 x float> %call82, <4 x float>* %upper
  %tmp85 = load <4 x float>* %shadow              ; <<4 x float>> [#uses=3]
  %tmp86 = extractelement <4 x float> %tmp85, i32 0 ; <float> [#uses=0]
  %10 = shufflevector <4 x float> %tmp85, <4 x float> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp87 = load <4 x float>* %midtone             ; <<4 x float>> [#uses=2]
  %tmp88 = extractelement <4 x float> %tmp87, i32 0 ; <float> [#uses=0]
  %11 = shufflevector <4 x float> %tmp85, <4 x float> %tmp87, <4 x i32> <i32 0, i32 4, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp89 = load <4 x float>* %hilight             ; <<4 x float>> [#uses=1]
  %tmp90 = extractelement <4 x float> %tmp89, i32 0 ; <float> [#uses=1]
  %vecinit91 = insertelement <4 x float> %11, float %tmp90, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit92 = insertelement <4 x float> %vecinit91, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit92, <4 x float>* %.compoundliteral84
  %tmp93 = load <4 x float>* %.compoundliteral84  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp93, <4 x float>* %tmp5
  %tmp94 = load <4 x float>* %upper               ; <<4 x float>> [#uses=1]
  %tmp95 = load <4 x float>* %tmp5                ; <<4 x float>> [#uses=1]
  %call96 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp94, <4 x float> %tmp95) ; <<4 x float>> [#uses=1]
  store <4 x float> %call96, <4 x float>* %upper
  %tmp97 = load <4 x float>* %lower               ; <<4 x float>> [#uses=1]
  %tmp98 = extractelement <4 x float> %tmp97, i32 1 ; <float> [#uses=1]
  %tmp99 = load <4 x float>* %upper               ; <<4 x float>> [#uses=1]
  %tmp100 = extractelement <4 x float> %tmp99, i32 1 ; <float> [#uses=1]
  %add = fadd float %tmp98, %tmp100               ; <float> [#uses=1]
  %div = fdiv float %add, 2.000000e+000           ; <float> [#uses=1]
  store float %div, float* %middle
  %tmp101 = load i32* %preserveLuminosity.addr    ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp101, 0                ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %tmp102 = load <4 x float>* %upper              ; <<4 x float>> [#uses=1]
  %tmp103 = shufflevector <4 x float> %tmp102, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp104 = load <4 x float>* %shadow             ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp103, %tmp104        ; <<4 x float>> [#uses=1]
  %tmp105 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp106 = getelementptr inbounds %struct.anon addrspace(1)* %tmp105, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub, <4 x float> addrspace(1)* %tmp106
  %tmp107 = load <4 x float>* %midtone            ; <<4 x float>> [#uses=1]
  %tmp108 = load float* %middle                   ; <float> [#uses=1]
  %tmp109 = insertelement <4 x float> undef, float %tmp108, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp109, <4 x float> %tmp109, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %sub110 = fsub <4 x float> %tmp107, %splat      ; <<4 x float>> [#uses=1]
  %tmp111 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp112 = getelementptr inbounds %struct.anon addrspace(1)* %tmp111, i32 0, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub110, <4 x float> addrspace(1)* %tmp112
  %tmp113 = load <4 x float>* %hilight            ; <<4 x float>> [#uses=1]
  %sub114 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp113 ; <<4 x float>> [#uses=1]
  %tmp115 = load <4 x float>* %lower              ; <<4 x float>> [#uses=1]
  %tmp116 = shufflevector <4 x float> %tmp115, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  %add117 = fadd <4 x float> %sub114, %tmp116     ; <<4 x float>> [#uses=1]
  %tmp118 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp119 = getelementptr inbounds %struct.anon addrspace(1)* %tmp118, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %add117, <4 x float> addrspace(1)* %tmp119
  br label %if.end

if.else:                                          ; preds = %entry
  %tmp120 = load <4 x float>* %shadow             ; <<4 x float>> [#uses=1]
  %mul121 = fmul <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, %tmp120 ; <<4 x float>> [#uses=1]
  %tmp122 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp123 = getelementptr inbounds %struct.anon addrspace(1)* %tmp122, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %mul121, <4 x float> addrspace(1)* %tmp123
  %tmp124 = load <4 x float>* %hilight            ; <<4 x float>> [#uses=1]
  %mul125 = fmul <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, %tmp124 ; <<4 x float>> [#uses=1]
  %tmp126 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp127 = getelementptr inbounds %struct.anon addrspace(1)* %tmp126, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %mul125, <4 x float> addrspace(1)* %tmp127
  %tmp128 = load <4 x float>* %midtone            ; <<4 x float>> [#uses=1]
  %tmp129 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp130 = getelementptr inbounds %struct.anon addrspace(1)* %tmp129, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp131 = load <4 x float> addrspace(1)* %tmp130 ; <<4 x float>> [#uses=1]
  %tmp132 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp133 = getelementptr inbounds %struct.anon addrspace(1)* %tmp132, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp134 = load <4 x float> addrspace(1)* %tmp133 ; <<4 x float>> [#uses=1]
  %add135 = fadd <4 x float> %tmp131, %tmp134     ; <<4 x float>> [#uses=1]
  %div136 = fdiv <4 x float> %add135, <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000> ; <<4 x float>> [#uses=1]
  %sub137 = fsub <4 x float> %tmp128, %div136     ; <<4 x float>> [#uses=1]
  %tmp138 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp139 = getelementptr inbounds %struct.anon addrspace(1)* %tmp138, i32 0, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub137, <4 x float> addrspace(1)* %tmp139
  %tmp140 = load <4 x float>* %kZero              ; <<4 x float>> [#uses=1]
  %tmp141 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp142 = getelementptr inbounds %struct.anon addrspace(1)* %tmp141, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp143 = load <4 x float> addrspace(1)* %tmp142 ; <<4 x float>> [#uses=1]
  %call144 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp140, <4 x float> %tmp143) ; <<4 x float>> [#uses=1]
  %tmp145 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp146 = getelementptr inbounds %struct.anon addrspace(1)* %tmp145, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call144, <4 x float> addrspace(1)* %tmp146
  %tmp147 = load <4 x float>* %kOne               ; <<4 x float>> [#uses=1]
  %tmp148 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp149 = getelementptr inbounds %struct.anon addrspace(1)* %tmp148, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp150 = load <4 x float> addrspace(1)* %tmp149 ; <<4 x float>> [#uses=1]
  %add151 = fadd <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp150 ; <<4 x float>> [#uses=1]
  %call152 = call <4 x float> @_Z3minDv4_fS_(<4 x float> %tmp147, <4 x float> %add151) ; <<4 x float>> [#uses=1]
  %tmp153 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp154 = getelementptr inbounds %struct.anon addrspace(1)* %tmp153, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call152, <4 x float> addrspace(1)* %tmp154
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp155 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp156 = getelementptr inbounds %struct.anon addrspace(1)* %tmp155, i32 0, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp157 = load <4 x float> addrspace(1)* %tmp156 ; <<4 x float>> [#uses=1]
  %tmp158 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp159 = getelementptr inbounds %struct.anon addrspace(1)* %tmp158, i32 0, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp160 = load <4 x float> addrspace(1)* %tmp159 ; <<4 x float>> [#uses=1]
  %sub161 = fsub <4 x float> %tmp157, %tmp160     ; <<4 x float>> [#uses=1]
  %tmp162 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp163 = getelementptr inbounds %struct.anon addrspace(1)* %tmp162, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub161, <4 x float> addrspace(1)* %tmp163
  %tmp164 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp165 = getelementptr inbounds %struct.anon addrspace(1)* %tmp164, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp166 = load <4 x float> addrspace(1)* %tmp165 ; <<4 x float>> [#uses=1]
  %tmp167 = extractelement <4 x float> %tmp166, i32 0 ; <float> [#uses=1]
  %tmp168 = load float* %EPSILON                  ; <float> [#uses=1]
  %cmp = fcmp olt float %tmp167, %tmp168          ; <i1> [#uses=1]
  br i1 %cmp, label %if.then169, label %if.end175

if.then169:                                       ; preds = %if.end
  %tmp170 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp171 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp172 = getelementptr inbounds %struct.anon addrspace(1)* %tmp171, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp173 = load <4 x float> addrspace(1)* %tmp172 ; <<4 x float>> [#uses=1]
  %tmp174 = insertelement <4 x float> %tmp173, float %tmp170, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp174, <4 x float> addrspace(1)* %tmp172
  br label %if.end175

if.end175:                                        ; preds = %if.then169, %if.end
  %tmp176 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp177 = getelementptr inbounds %struct.anon addrspace(1)* %tmp176, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp178 = load <4 x float> addrspace(1)* %tmp177 ; <<4 x float>> [#uses=1]
  %tmp179 = extractelement <4 x float> %tmp178, i32 1 ; <float> [#uses=1]
  %tmp180 = load float* %EPSILON                  ; <float> [#uses=1]
  %cmp181 = fcmp olt float %tmp179, %tmp180       ; <i1> [#uses=1]
  br i1 %cmp181, label %if.then182, label %if.end188

if.then182:                                       ; preds = %if.end175
  %tmp183 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp184 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp185 = getelementptr inbounds %struct.anon addrspace(1)* %tmp184, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp186 = load <4 x float> addrspace(1)* %tmp185 ; <<4 x float>> [#uses=1]
  %tmp187 = insertelement <4 x float> %tmp186, float %tmp183, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp187, <4 x float> addrspace(1)* %tmp185
  br label %if.end188

if.end188:                                        ; preds = %if.then182, %if.end175
  %tmp189 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp190 = getelementptr inbounds %struct.anon addrspace(1)* %tmp189, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp191 = load <4 x float> addrspace(1)* %tmp190 ; <<4 x float>> [#uses=1]
  %tmp192 = extractelement <4 x float> %tmp191, i32 2 ; <float> [#uses=1]
  %tmp193 = load float* %EPSILON                  ; <float> [#uses=1]
  %cmp194 = fcmp olt float %tmp192, %tmp193       ; <i1> [#uses=1]
  br i1 %cmp194, label %if.then195, label %if.end201

if.then195:                                       ; preds = %if.end188
  %tmp196 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp197 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp198 = getelementptr inbounds %struct.anon addrspace(1)* %tmp197, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp199 = load <4 x float> addrspace(1)* %tmp198 ; <<4 x float>> [#uses=1]
  %tmp200 = insertelement <4 x float> %tmp199, float %tmp196, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp200, <4 x float> addrspace(1)* %tmp198
  br label %if.end201

if.end201:                                        ; preds = %if.then195, %if.end188
  %tmp202 = load <4 x float>* %kZero              ; <<4 x float>> [#uses=1]
  %tmp203 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp204 = getelementptr inbounds %struct.anon addrspace(1)* %tmp203, i32 0, i32 10 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp202, <4 x float> addrspace(1)* %tmp204
  %tmp205 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp206 = getelementptr inbounds %struct.anon addrspace(1)* %tmp205, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp207 = load <4 x float> addrspace(1)* %tmp206 ; <<4 x float>> [#uses=1]
  %tmp208 = extractelement <4 x float> %tmp207, i32 0 ; <float> [#uses=1]
  %cmp209 = fcmp oeq float %tmp208, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp209, label %if.then210, label %if.end215

if.then210:                                       ; preds = %if.end201
  %tmp211 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp212 = getelementptr inbounds %struct.anon addrspace(1)* %tmp211, i32 0, i32 10 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp213 = load <4 x float> addrspace(1)* %tmp212 ; <<4 x float>> [#uses=1]
  %tmp214 = insertelement <4 x float> %tmp213, float 1.000000e+000, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp214, <4 x float> addrspace(1)* %tmp212
  br label %if.end215

if.end215:                                        ; preds = %if.then210, %if.end201
  %tmp216 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp217 = getelementptr inbounds %struct.anon addrspace(1)* %tmp216, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp218 = load <4 x float> addrspace(1)* %tmp217 ; <<4 x float>> [#uses=1]
  %tmp219 = extractelement <4 x float> %tmp218, i32 1 ; <float> [#uses=1]
  %cmp220 = fcmp oeq float %tmp219, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp220, label %if.then221, label %if.end226

if.then221:                                       ; preds = %if.end215
  %tmp222 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp223 = getelementptr inbounds %struct.anon addrspace(1)* %tmp222, i32 0, i32 10 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp224 = load <4 x float> addrspace(1)* %tmp223 ; <<4 x float>> [#uses=1]
  %tmp225 = insertelement <4 x float> %tmp224, float 1.000000e+000, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp225, <4 x float> addrspace(1)* %tmp223
  br label %if.end226

if.end226:                                        ; preds = %if.then221, %if.end215
  %tmp227 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp228 = getelementptr inbounds %struct.anon addrspace(1)* %tmp227, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp229 = load <4 x float> addrspace(1)* %tmp228 ; <<4 x float>> [#uses=1]
  %tmp230 = extractelement <4 x float> %tmp229, i32 2 ; <float> [#uses=1]
  %cmp231 = fcmp oeq float %tmp230, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp231, label %if.then232, label %if.end237

if.then232:                                       ; preds = %if.end226
  %tmp233 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp234 = getelementptr inbounds %struct.anon addrspace(1)* %tmp233, i32 0, i32 10 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp235 = load <4 x float> addrspace(1)* %tmp234 ; <<4 x float>> [#uses=1]
  %tmp236 = insertelement <4 x float> %tmp235, float 1.000000e+000, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp236, <4 x float> addrspace(1)* %tmp234
  br label %if.end237

if.end237:                                        ; preds = %if.then232, %if.end226
  store float 0x3F8EB851E0000000, float* %tolerance
  %tmp239 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp240 = getelementptr inbounds %struct.anon addrspace(1)* %tmp239, i32 0, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp241 = load <4 x float> addrspace(1)* %tmp240 ; <<4 x float>> [#uses=1]
  %tmp242 = load <4 x float>* %kMaxConst          ; <<4 x float>> [#uses=1]
  %mul243 = fmul <4 x float> %tmp242, <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002> ; <<4 x float>> [#uses=3]
  %cmp244 = fcmp oeq <4 x float> zeroinitializer, %mul243 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp244, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %mul243 ; <<4 x float>> [#uses=0]
  %div245 = fdiv <4 x float> %tmp241, %mul243     ; <<4 x float>> [#uses=1]
  %mul246 = fmul <4 x float> %div245, <float 0xBFE62DE000000000, float 0xBFE62DE000000000, float 0xBFE62DE000000000, float 0xBFE62DE000000000> ; <<4 x float>> [#uses=1]
  %call247 = call <4 x float> @_Z3expDv4_f(<4 x float> %mul246) ; <<4 x float>> [#uses=1]
  %tmp248 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp249 = getelementptr inbounds %struct.anon addrspace(1)* %tmp248, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call247, <4 x float> addrspace(1)* %tmp249
  %tmp251 = load <4 x float>* %kZero              ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp251, <4 x float>* %flagGamma
  %tmp252 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp253 = getelementptr inbounds %struct.anon addrspace(1)* %tmp252, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp254 = load <4 x float> addrspace(1)* %tmp253 ; <<4 x float>> [#uses=1]
  %tmp255 = extractelement <4 x float> %tmp254, i32 0 ; <float> [#uses=1]
  %cmp256 = fcmp oeq float %tmp255, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp256, label %if.then257, label %if.end260

if.then257:                                       ; preds = %if.end237
  %tmp258 = load <4 x float>* %flagGamma          ; <<4 x float>> [#uses=1]
  %tmp259 = insertelement <4 x float> %tmp258, float 1.000000e+000, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp259, <4 x float>* %flagGamma
  br label %if.end260

if.end260:                                        ; preds = %if.then257, %if.end237
  %tmp261 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp262 = getelementptr inbounds %struct.anon addrspace(1)* %tmp261, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp263 = load <4 x float> addrspace(1)* %tmp262 ; <<4 x float>> [#uses=1]
  %tmp264 = extractelement <4 x float> %tmp263, i32 1 ; <float> [#uses=1]
  %cmp265 = fcmp oeq float %tmp264, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp265, label %if.then266, label %if.end269

if.then266:                                       ; preds = %if.end260
  %tmp267 = load <4 x float>* %flagGamma          ; <<4 x float>> [#uses=1]
  %tmp268 = insertelement <4 x float> %tmp267, float 1.000000e+000, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp268, <4 x float>* %flagGamma
  br label %if.end269

if.end269:                                        ; preds = %if.then266, %if.end260
  %tmp270 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp271 = getelementptr inbounds %struct.anon addrspace(1)* %tmp270, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp272 = load <4 x float> addrspace(1)* %tmp271 ; <<4 x float>> [#uses=1]
  %tmp273 = extractelement <4 x float> %tmp272, i32 2 ; <float> [#uses=1]
  %cmp274 = fcmp oeq float %tmp273, 1.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp274, label %if.then275, label %if.end278

if.then275:                                       ; preds = %if.end269
  %tmp276 = load <4 x float>* %flagGamma          ; <<4 x float>> [#uses=1]
  %tmp277 = insertelement <4 x float> %tmp276, float 1.000000e+000, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp277, <4 x float>* %flagGamma
  br label %if.end278

if.end278:                                        ; preds = %if.then275, %if.end269
  store <4 x float> <float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000>, <4 x float>* %.compoundliteral280
  %tmp281 = load <4 x float>* %.compoundliteral280 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp281, <4 x float>* %stepTmp
  %tmp282 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp283 = getelementptr inbounds %struct.anon addrspace(1)* %tmp282, i32 0, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp284 = load <4 x float> addrspace(1)* %tmp283 ; <<4 x float>> [#uses=1]
  %tmp285 = load <4 x float>* %stepTmp            ; <<4 x float>> [#uses=1]
  %call286 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %tmp284, <4 x float> %tmp285) ; <<4 x float>> [#uses=1]
  %tmp287 = load <4 x float>* %flagGamma          ; <<4 x float>> [#uses=1]
  %add288 = fadd <4 x float> %call286, %tmp287    ; <<4 x float>> [#uses=1]
  %tmp289 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp290 = getelementptr inbounds %struct.anon addrspace(1)* %tmp289, i32 0, i32 9 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %add288, <4 x float> addrspace(1)* %tmp290
  %tmp291 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp292 = getelementptr inbounds %struct.anon addrspace(1)* %tmp291, i32 0, i32 9 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp293 = load <4 x float> addrspace(1)* %tmp292 ; <<4 x float>> [#uses=1]
  %tmp294 = load <4 x float>* %kZero              ; <<4 x float>> [#uses=1]
  %tmp295 = load <4 x float>* %kOne               ; <<4 x float>> [#uses=1]
  %call296 = call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %tmp293, <4 x float> %tmp294, <4 x float> %tmp295) ; <<4 x float>> [#uses=1]
  %tmp297 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp298 = getelementptr inbounds %struct.anon addrspace(1)* %tmp297, i32 0, i32 9 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call296, <4 x float> addrspace(1)* %tmp298
  %tmp299 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp300 = getelementptr inbounds %struct.anon addrspace(1)* %tmp299, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp301 = load <4 x float> addrspace(1)* %tmp300 ; <<4 x float>> [#uses=1]
  %tmp302 = extractelement <4 x float> %tmp301, i32 0 ; <float> [#uses=1]
  %tmp303 = load float* %tolerance                ; <float> [#uses=1]
  %add304 = fadd float 1.000000e+000, %tmp303     ; <float> [#uses=1]
  %cmp305 = fcmp ole float %tmp302, %add304       ; <i1> [#uses=1]
  br i1 %cmp305, label %land.lhs.true, label %if.else319

land.lhs.true:                                    ; preds = %if.end278
  %tmp306 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp307 = getelementptr inbounds %struct.anon addrspace(1)* %tmp306, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp308 = load <4 x float> addrspace(1)* %tmp307 ; <<4 x float>> [#uses=1]
  %tmp309 = extractelement <4 x float> %tmp308, i32 0 ; <float> [#uses=1]
  %tmp310 = load float* %tolerance                ; <float> [#uses=1]
  %sub311 = fsub float 1.000000e+000, %tmp310     ; <float> [#uses=1]
  %cmp312 = fcmp oge float %tmp309, %sub311       ; <i1> [#uses=1]
  br i1 %cmp312, label %if.then313, label %if.else319

if.then313:                                       ; preds = %land.lhs.true
  %tmp314 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp315 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp316 = getelementptr inbounds %struct.anon addrspace(1)* %tmp315, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp317 = load <4 x float> addrspace(1)* %tmp316 ; <<4 x float>> [#uses=1]
  %tmp318 = insertelement <4 x float> %tmp317, float %tmp314, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp318, <4 x float> addrspace(1)* %tmp316
  br label %if.end342

if.else319:                                       ; preds = %land.lhs.true, %if.end278
  %tmp320 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp321 = getelementptr inbounds %struct.anon addrspace(1)* %tmp320, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp322 = load <4 x float> addrspace(1)* %tmp321 ; <<4 x float>> [#uses=1]
  %tmp323 = extractelement <4 x float> %tmp322, i32 0 ; <float> [#uses=3]
  %cmp324 = fcmp oeq float 0.000000e+000, %tmp323 ; <i1> [#uses=1]
  %sel325 = select i1 %cmp324, float 1.000000e+000, float %tmp323 ; <float> [#uses=0]
  %div326 = fdiv float 1.000000e+000, %tmp323     ; <float> [#uses=1]
  %tmp327 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp328 = getelementptr inbounds %struct.anon addrspace(1)* %tmp327, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp329 = load <4 x float> addrspace(1)* %tmp328 ; <<4 x float>> [#uses=1]
  %tmp330 = extractelement <4 x float> %tmp329, i32 0 ; <float> [#uses=1]
  %sub331 = fsub float %tmp330, 1.000000e+000     ; <float> [#uses=3]
  %cmp332 = fcmp oeq float 0.000000e+000, %sub331 ; <i1> [#uses=1]
  %sel333 = select i1 %cmp332, float 1.000000e+000, float %sub331 ; <float> [#uses=0]
  %div334 = fdiv float %div326, %sub331           ; <float> [#uses=1]
  %add335 = fadd float 1.000000e+000, %div334     ; <float> [#uses=1]
  %mul336 = fmul float 0x3FE62DE000000000, %add335 ; <float> [#uses=1]
  %call337 = call float @_Z3expf(float %mul336)   ; <float> [#uses=1]
  %tmp338 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp339 = getelementptr inbounds %struct.anon addrspace(1)* %tmp338, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp340 = load <4 x float> addrspace(1)* %tmp339 ; <<4 x float>> [#uses=1]
  %tmp341 = insertelement <4 x float> %tmp340, float %call337, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp341, <4 x float> addrspace(1)* %tmp339
  br label %if.end342

if.end342:                                        ; preds = %if.else319, %if.then313
  %tmp343 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp345 = getelementptr inbounds %struct.anon addrspace(1)* %tmp343, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp346 = load <4 x float> addrspace(1)* %tmp345 ; <<4 x float>> [#uses=1]
  %tmp347 = extractelement <4 x float> %tmp346, i32 1 ; <float> [#uses=1]
  %tmp348 = load float* %tolerance                ; <float> [#uses=1]
  %add349 = fadd float 1.000000e+000, %tmp348     ; <float> [#uses=1]
  %cmp350 = fcmp ole float %tmp347, %add349       ; <i1> [#uses=1]
  br i1 %cmp350, label %land.lhs.true351, label %if.else365

land.lhs.true351:                                 ; preds = %if.end342
  %tmp352 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp353 = getelementptr inbounds %struct.anon addrspace(1)* %tmp352, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp354 = load <4 x float> addrspace(1)* %tmp353 ; <<4 x float>> [#uses=1]
  %tmp355 = extractelement <4 x float> %tmp354, i32 1 ; <float> [#uses=1]
  %tmp356 = load float* %tolerance                ; <float> [#uses=1]
  %sub357 = fsub float 1.000000e+000, %tmp356     ; <float> [#uses=1]
  %cmp358 = fcmp oge float %tmp355, %sub357       ; <i1> [#uses=1]
  br i1 %cmp358, label %if.then359, label %if.else365

if.then359:                                       ; preds = %land.lhs.true351
  %tmp360 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp361 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp362 = getelementptr inbounds %struct.anon addrspace(1)* %tmp361, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp363 = load <4 x float> addrspace(1)* %tmp362 ; <<4 x float>> [#uses=1]
  %tmp364 = insertelement <4 x float> %tmp363, float %tmp360, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp364, <4 x float> addrspace(1)* %tmp362
  br label %if.end388

if.else365:                                       ; preds = %land.lhs.true351, %if.end342
  %tmp366 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp367 = getelementptr inbounds %struct.anon addrspace(1)* %tmp366, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp368 = load <4 x float> addrspace(1)* %tmp367 ; <<4 x float>> [#uses=1]
  %tmp369 = extractelement <4 x float> %tmp368, i32 1 ; <float> [#uses=3]
  %cmp370 = fcmp oeq float 0.000000e+000, %tmp369 ; <i1> [#uses=1]
  %sel371 = select i1 %cmp370, float 1.000000e+000, float %tmp369 ; <float> [#uses=0]
  %div372 = fdiv float 1.000000e+000, %tmp369     ; <float> [#uses=1]
  %tmp373 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp374 = getelementptr inbounds %struct.anon addrspace(1)* %tmp373, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp375 = load <4 x float> addrspace(1)* %tmp374 ; <<4 x float>> [#uses=1]
  %tmp376 = extractelement <4 x float> %tmp375, i32 1 ; <float> [#uses=1]
  %sub377 = fsub float %tmp376, 1.000000e+000     ; <float> [#uses=3]
  %cmp378 = fcmp oeq float 0.000000e+000, %sub377 ; <i1> [#uses=1]
  %sel379 = select i1 %cmp378, float 1.000000e+000, float %sub377 ; <float> [#uses=0]
  %div380 = fdiv float %div372, %sub377           ; <float> [#uses=1]
  %add381 = fadd float 1.000000e+000, %div380     ; <float> [#uses=1]
  %mul382 = fmul float 0x3FE62DE000000000, %add381 ; <float> [#uses=1]
  %call383 = call float @_Z3expf(float %mul382)   ; <float> [#uses=1]
  %tmp384 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp385 = getelementptr inbounds %struct.anon addrspace(1)* %tmp384, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp386 = load <4 x float> addrspace(1)* %tmp385 ; <<4 x float>> [#uses=1]
  %tmp387 = insertelement <4 x float> %tmp386, float %call383, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp387, <4 x float> addrspace(1)* %tmp385
  br label %if.end388

if.end388:                                        ; preds = %if.else365, %if.then359
  %tmp389 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp390 = getelementptr inbounds %struct.anon addrspace(1)* %tmp389, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp391 = load <4 x float> addrspace(1)* %tmp390 ; <<4 x float>> [#uses=1]
  %tmp392 = extractelement <4 x float> %tmp391, i32 2 ; <float> [#uses=1]
  %tmp393 = load float* %tolerance                ; <float> [#uses=1]
  %add394 = fadd float 1.000000e+000, %tmp393     ; <float> [#uses=1]
  %cmp395 = fcmp ole float %tmp392, %add394       ; <i1> [#uses=1]
  br i1 %cmp395, label %land.lhs.true396, label %if.else410

land.lhs.true396:                                 ; preds = %if.end388
  %tmp397 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp398 = getelementptr inbounds %struct.anon addrspace(1)* %tmp397, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp399 = load <4 x float> addrspace(1)* %tmp398 ; <<4 x float>> [#uses=1]
  %tmp400 = extractelement <4 x float> %tmp399, i32 2 ; <float> [#uses=1]
  %tmp401 = load float* %tolerance                ; <float> [#uses=1]
  %sub402 = fsub float 1.000000e+000, %tmp401     ; <float> [#uses=1]
  %cmp403 = fcmp oge float %tmp400, %sub402       ; <i1> [#uses=1]
  br i1 %cmp403, label %if.then404, label %if.else410

if.then404:                                       ; preds = %land.lhs.true396
  %tmp405 = load float* %EPSILON                  ; <float> [#uses=1]
  %tmp406 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp407 = getelementptr inbounds %struct.anon addrspace(1)* %tmp406, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp408 = load <4 x float> addrspace(1)* %tmp407 ; <<4 x float>> [#uses=1]
  %tmp409 = insertelement <4 x float> %tmp408, float %tmp405, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp409, <4 x float> addrspace(1)* %tmp407
  br label %if.end433

if.else410:                                       ; preds = %land.lhs.true396, %if.end388
  %tmp411 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp412 = getelementptr inbounds %struct.anon addrspace(1)* %tmp411, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp413 = load <4 x float> addrspace(1)* %tmp412 ; <<4 x float>> [#uses=1]
  %tmp414 = extractelement <4 x float> %tmp413, i32 2 ; <float> [#uses=3]
  %cmp415 = fcmp oeq float 0.000000e+000, %tmp414 ; <i1> [#uses=1]
  %sel416 = select i1 %cmp415, float 1.000000e+000, float %tmp414 ; <float> [#uses=0]
  %div417 = fdiv float 1.000000e+000, %tmp414     ; <float> [#uses=1]
  %tmp418 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp419 = getelementptr inbounds %struct.anon addrspace(1)* %tmp418, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp420 = load <4 x float> addrspace(1)* %tmp419 ; <<4 x float>> [#uses=1]
  %tmp421 = extractelement <4 x float> %tmp420, i32 2 ; <float> [#uses=1]
  %sub422 = fsub float %tmp421, 1.000000e+000     ; <float> [#uses=3]
  %cmp423 = fcmp oeq float 0.000000e+000, %sub422 ; <i1> [#uses=1]
  %sel424 = select i1 %cmp423, float 1.000000e+000, float %sub422 ; <float> [#uses=0]
  %div425 = fdiv float %div417, %sub422           ; <float> [#uses=1]
  %add426 = fadd float 1.000000e+000, %div425     ; <float> [#uses=1]
  %mul427 = fmul float 0x3FE62DE000000000, %add426 ; <float> [#uses=1]
  %call428 = call float @_Z3expf(float %mul427)   ; <float> [#uses=1]
  %tmp429 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp430 = getelementptr inbounds %struct.anon addrspace(1)* %tmp429, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=2]
  %tmp431 = load <4 x float> addrspace(1)* %tmp430 ; <<4 x float>> [#uses=1]
  %tmp432 = insertelement <4 x float> %tmp431, float %call428, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp432, <4 x float> addrspace(1)* %tmp430
  br label %if.end433

if.end433:                                        ; preds = %if.else410, %if.then404
  %tmp434 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp435 = getelementptr inbounds %struct.anon addrspace(1)* %tmp434, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp436 = load <4 x float> addrspace(1)* %tmp435 ; <<4 x float>> [#uses=1]
  %call437 = call <4 x float> @_Z4fabsDv4_f(<4 x float> %tmp436) ; <<4 x float>> [#uses=1]
  %tmp438 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp439 = getelementptr inbounds %struct.anon addrspace(1)* %tmp438, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp440 = load <4 x float> addrspace(1)* %tmp439 ; <<4 x float>> [#uses=1]
  %call441 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %call437, <4 x float> %tmp440) ; <<4 x float>> [#uses=1]
  %tmp442 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp443 = getelementptr inbounds %struct.anon addrspace(1)* %tmp442, i32 0, i32 5 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call441, <4 x float> addrspace(1)* %tmp443
  %tmp444 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp445 = getelementptr inbounds %struct.anon addrspace(1)* %tmp444, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp446 = load <4 x float> addrspace(1)* %tmp445 ; <<4 x float>> [#uses=3]
  %cmp447 = fcmp oeq <4 x float> zeroinitializer, %tmp446 ; <<4 x i1>> [#uses=1]
  %sel448 = select <4 x i1> %cmp447, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp446 ; <<4 x float>> [#uses=0]
  %div449 = fdiv <4 x float> <float 0x3FE62DE000000000, float 0x3FE62DE000000000, float 0x3FE62DE000000000, float 0x3FE62DE000000000>, %tmp446 ; <<4 x float>> [#uses=1]
  %call450 = call <4 x float> @_Z3expDv4_f(<4 x float> %div449) ; <<4 x float>> [#uses=1]
  %tmp451 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp452 = getelementptr inbounds %struct.anon addrspace(1)* %tmp451, i32 0, i32 6 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call450, <4 x float> addrspace(1)* %tmp452
  %tmp453 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp454 = getelementptr inbounds %struct.anon addrspace(1)* %tmp453, i32 0, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp455 = load <4 x float> addrspace(1)* %tmp454 ; <<4 x float>> [#uses=1]
  %tmp456 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp457 = getelementptr inbounds %struct.anon addrspace(1)* %tmp456, i32 0, i32 5 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp458 = load <4 x float> addrspace(1)* %tmp457 ; <<4 x float>> [#uses=1]
  %mul459 = fmul <4 x float> %tmp455, %tmp458     ; <<4 x float>> [#uses=1]
  %tmp460 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp461 = getelementptr inbounds %struct.anon addrspace(1)* %tmp460, i32 0, i32 4 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp462 = load <4 x float> addrspace(1)* %tmp461 ; <<4 x float>> [#uses=3]
  %cmp463 = fcmp oeq <4 x float> zeroinitializer, %tmp462 ; <<4 x i1>> [#uses=1]
  %sel464 = select <4 x i1> %cmp463, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp462 ; <<4 x float>> [#uses=0]
  %div465 = fdiv <4 x float> %mul459, %tmp462     ; <<4 x float>> [#uses=1]
  %tmp466 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp467 = getelementptr inbounds %struct.anon addrspace(1)* %tmp466, i32 0, i32 7 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div465, <4 x float> addrspace(1)* %tmp467
  ret void
}

declare <4 x float> @_Z3minDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z3maxDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z3expDv4_f(<4 x float>)

declare <4 x float> @_Z5clampDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>)

declare float @_Z3expf(float)

; CHECK: ret
define <4 x float> @evPix(<4 x float> %colorIn, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %colorIn.addr = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %kOne = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %.compoundliteral2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %vOutputColor = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=5]
  %mycolor = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %outColor = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  store <4 x float> %colorIn, <4 x float>* %colorIn.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %kOne
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral2
  %tmp3 = load <4 x float>* %.compoundliteral2    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %kZero
  %tmp6 = load <4 x float>* %colorIn.addr         ; <<4 x float>> [#uses=1]
  %tmp7 = load <4 x float>* %kZero                ; <<4 x float>> [#uses=1]
  %tmp8 = load <4 x float>* %kOne                 ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %tmp6, <4 x float> %tmp7, <4 x float> %tmp8) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %mycolor
  %tmp10 = load <4 x float>* %mycolor             ; <<4 x float>> [#uses=1]
  %tmp11 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call12 = call <4 x float> @doChannel(<4 x float> %tmp10, %struct.anon addrspace(2)* %tmp11) ; <<4 x float>> [#uses=1]
  store <4 x float> %call12, <4 x float>* %outColor
  %tmp13 = load <4 x float>* %outColor            ; <<4 x float>> [#uses=1]
  %tmp14 = shufflevector <4 x float> %tmp13, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %tmp15 = load <4 x float>* %vOutputColor        ; <<4 x float>> [#uses=1]
  %tmp16 = shufflevector <3 x float> %tmp14, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp17 = shufflevector <4 x float> %tmp15, <4 x float> %tmp16, <4 x i32> <i32 4, i32 5, i32 6, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp17, <4 x float>* %vOutputColor
  %tmp18 = load <4 x float>* %mycolor             ; <<4 x float>> [#uses=1]
  %tmp19 = extractelement <4 x float> %tmp18, i32 3 ; <float> [#uses=1]
  %tmp20 = load <4 x float>* %vOutputColor        ; <<4 x float>> [#uses=1]
  %tmp21 = insertelement <4 x float> %tmp20, float %tmp19, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp21, <4 x float>* %vOutputColor
  %tmp22 = load <4 x float>* %vOutputColor        ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp22, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define void @colorbalance2_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %gid0_curPix = alloca i32, align 4              ; <i32*> [#uses=3]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_curPix
  %tmp = load i32* %gid0_curPix                   ; <i32> [#uses=1]
  %tmp1 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp1, i32 %tmp ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp2 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp3 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call4 = call <4 x float> @evPix(<4 x float> %tmp2, %struct.anon addrspace(2)* %tmp3) ; <<4 x float>> [#uses=1]
  %tmp5 = load i32* %gid0_curPix                  ; <i32> [#uses=1]
  %tmp6 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx7 = getelementptr inbounds <4 x float> addrspace(1)* %tmp6, i32 %tmp5 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call4, <4 x float> addrspace(1)* %arrayidx7
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @colorbalance2(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %pixelCountPerGlobalID, %struct.anon addrspace(2)* %args) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pixelCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %args.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=7]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %pixelCountPerGlobalID, i32* %pixelCountPerGlobalID.addr
  store %struct.anon addrspace(2)* %args, %struct.anon addrspace(2)** %args.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %pixelCountPerGlobalID.addr    ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %index
  %tmp3 = load i32* %index                        ; <i32> [#uses=1]
  %tmp4 = load i32* %pixelCountPerGlobalID.addr   ; <i32> [#uses=1]
  %add = add i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  store i32 %add, i32* %lastIndex
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp5 = load i32* %index                        ; <i32> [#uses=1]
  %tmp6 = load i32* %lastIndex                    ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp5, %tmp6                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp7 = load i32* %index                        ; <i32> [#uses=1]
  %tmp8 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp8, i32 %tmp7 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp9 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp10 = load %struct.anon addrspace(2)** %args.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call11 = call <4 x float> @evPix(<4 x float> %tmp9, %struct.anon addrspace(2)* %tmp10) ; <<4 x float>> [#uses=1]
  %tmp12 = load i32* %index                       ; <i32> [#uses=1]
  %tmp13 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx14 = getelementptr inbounds <4 x float> addrspace(1)* %tmp13, i32 %tmp12 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call11, <4 x float> addrspace(1)* %arrayidx14
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp15 = load i32* %index                       ; <i32> [#uses=1]
  %inc = add i32 %tmp15, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
