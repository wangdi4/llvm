; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIFFT.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_kfft_local_lds = internal addrspace(3) global [2176 x float] zeroinitializer, align 4 ; <[2176 x float] addrspace(3)*> [#uses=2]
@opencl_kfft_locals = appending global [2 x i8*] [i8* bitcast ([2176 x float] addrspace(3)* @opencl_kfft_local_lds to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_kfft_parameters = appending global [85 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[85 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @kfft to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] [i32 1, i32 64, i32 1, i32 1], i8* bitcast ([2 x i8*]* @opencl_kfft_locals to i8*), i8* getelementptr inbounds ([85 x i8]* @opencl_kfft_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define float @k_sincos(i32 %i, float* %cretp) nounwind alwaysinline {
entry:
  %retval = alloca float, align 4                 ; <float*> [#uses=2]
  %i.addr = alloca i32, align 4                   ; <i32*> [#uses=5]
  %cretp.addr = alloca float*, align 4            ; <float**> [#uses=2]
  %x = alloca float, align 4                      ; <float*> [#uses=3]
  store i32 %i, i32* %i.addr
  store float* %cretp, float** %cretp.addr
  %tmp = load i32* %i.addr                        ; <i32> [#uses=1]
  %cmp = icmp sgt i32 %tmp, 512                   ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp1 = load i32* %i.addr                       ; <i32> [#uses=1]
  %sub = sub i32 %tmp1, 1024                      ; <i32> [#uses=1]
  store i32 %sub, i32* %i.addr
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp3 = load i32* %i.addr                       ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp3 to float               ; <float> [#uses=1]
  %mul = fmul float %conv, 0xBF7921FB60000000     ; <float> [#uses=1]
  store float %mul, float* %x
  %tmp4 = load float* %x                          ; <float> [#uses=1]
  %call = call float @_Z10native_cosf(float %tmp4) ; <float> [#uses=1]
  %tmp5 = load float** %cretp.addr                ; <float*> [#uses=1]
  store float %call, float* %tmp5
  %tmp6 = load float* %x                          ; <float> [#uses=1]
  %call7 = call float @_Z10native_sinf(float %tmp6) ; <float> [#uses=1]
  store float %call7, float* %retval
  %0 = load float* %retval                        ; <float> [#uses=1]
  ret float %0
}

declare float @_Z10native_cosf(float)

declare float @_Z10native_sinf(float)

; CHECK: ret
define <4 x float> @k_sincos4(<4 x i32> %i, <4 x float>* %cretp) nounwind alwaysinline {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %i.addr = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=5]
  %cretp.addr = alloca <4 x float>*, align 4      ; <<4 x float>**> [#uses=2]
  %x = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  store <4 x i32> %i, <4 x i32>* %i.addr
  store <4 x float>* %cretp, <4 x float>** %cretp.addr
  %tmp = load <4 x i32>* %i.addr                  ; <<4 x i32>> [#uses=1]
  %cmp = icmp sgt <4 x i32> %tmp, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext = sext <4 x i1> %cmp to <4 x i32>         ; <<4 x i32>> [#uses=1]
  %and = and <4 x i32> %sext, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1 = load <4 x i32>* %i.addr                 ; <<4 x i32>> [#uses=1]
  %sub = sub <4 x i32> %tmp1, %and                ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub, <4 x i32>* %i.addr
  %tmp3 = load <4 x i32>* %i.addr                 ; <<4 x i32>> [#uses=1]
  %call = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3) ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %call, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul, <4 x float>* %x
  %tmp4 = load <4 x float>* %x                    ; <<4 x float>> [#uses=1]
  %call5 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4) ; <<4 x float>> [#uses=1]
  %tmp6 = load <4 x float>** %cretp.addr          ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5, <4 x float>* %tmp6
  %tmp7 = load <4 x float>* %x                    ; <<4 x float>> [#uses=1]
  %call8 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7) ; <<4 x float>> [#uses=1]
  store <4 x float> %call8, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32>)

declare <4 x float> @_Z10native_cosU8__vector4f(<4 x float>)

declare <4 x float> @_Z10native_sinU8__vector4f(<4 x float>)

; CHECK: ret
define void @kfft_pass1(i32 %me, float addrspace(1)* %gr, float addrspace(1)* %gi, float addrspace(3)* %lds) nounwind alwaysinline {
entry:
  %retval.i342 = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %i.addr.i343 = alloca <4 x i32>, align 16       ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i344 = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %x.i345 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %retval.i324 = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %i.addr.i325 = alloca <4 x i32>, align 16       ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i326 = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %x.i327 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %retval.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %i.addr.i = alloca <4 x i32>, align 16          ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i = alloca <4 x float>*, align 4    ; <<4 x float>**> [#uses=2]
  %x.i = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %me.addr = alloca i32, align 4                  ; <i32*> [#uses=6]
  %gr.addr = alloca float addrspace(1)*, align 4  ; <float addrspace(1)**> [#uses=2]
  %gi.addr = alloca float addrspace(1)*, align 4  ; <float addrspace(1)**> [#uses=2]
  %lds.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=2]
  %gp = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %lp = alloca float addrspace(3)*, align 4       ; <float addrspace(3)**> [#uses=47]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=11]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=4]
  %.compoundliteral = alloca <4 x i32>, align 16  ; <<4 x i32>*> [#uses=2]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r134 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %s3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %__r158 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %me.addr
  store float addrspace(1)* %gr, float addrspace(1)** %gr.addr
  store float addrspace(1)* %gi, float addrspace(1)** %gi.addr
  store float addrspace(3)* %lds, float addrspace(3)** %lds.addr
  %tmp = load float addrspace(1)** %gr.addr       ; <float addrspace(1)*> [#uses=1]
  %tmp1 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shl = shl i32 %tmp1, 2                         ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(1)* %tmp, i32 %shl ; <float addrspace(1)*> [#uses=1]
  %0 = bitcast float addrspace(1)* %add.ptr to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %0, <4 x float> addrspace(1)** %gp
  %tmp3 = load <4 x float> addrspace(1)** %gp     ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp3, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp4 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp4, <4 x float>* %zr0
  %tmp6 = load <4 x float> addrspace(1)** %gp     ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx7 = getelementptr inbounds <4 x float> addrspace(1)* %tmp6, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp8 = load <4 x float> addrspace(1)* %arrayidx7 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp8, <4 x float>* %zr1
  %tmp10 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp12 = load <4 x float> addrspace(1)* %arrayidx11 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp12, <4 x float>* %zr2
  %tmp14 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx15 = getelementptr inbounds <4 x float> addrspace(1)* %tmp14, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp16 = load <4 x float> addrspace(1)* %arrayidx15 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp16, <4 x float>* %zr3
  %tmp17 = load float addrspace(1)** %gi.addr     ; <float addrspace(1)*> [#uses=1]
  %tmp18 = load i32* %me.addr                     ; <i32> [#uses=1]
  %shl19 = shl i32 %tmp18, 2                      ; <i32> [#uses=1]
  %add.ptr20 = getelementptr inbounds float addrspace(1)* %tmp17, i32 %shl19 ; <float addrspace(1)*> [#uses=1]
  %1 = bitcast float addrspace(1)* %add.ptr20 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %1, <4 x float> addrspace(1)** %gp
  %tmp22 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds <4 x float> addrspace(1)* %tmp22, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp24 = load <4 x float> addrspace(1)* %arrayidx23 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp24, <4 x float>* %zi0
  %tmp26 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx27 = getelementptr inbounds <4 x float> addrspace(1)* %tmp26, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp28 = load <4 x float> addrspace(1)* %arrayidx27 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp28, <4 x float>* %zi1
  %tmp30 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx31 = getelementptr inbounds <4 x float> addrspace(1)* %tmp30, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp32 = load <4 x float> addrspace(1)* %arrayidx31 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %zi2
  %tmp34 = load <4 x float> addrspace(1)** %gp    ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx35 = getelementptr inbounds <4 x float> addrspace(1)* %tmp34, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp36 = load <4 x float> addrspace(1)* %arrayidx35 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36, <4 x float>* %zi3
  br label %do.body

do.body:                                          ; preds = %entry
  %tmp38 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp39 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp38, %tmp39          ; <<4 x float>> [#uses=1]
  store <4 x float> %add, <4 x float>* %ar0
  %tmp41 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp42 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %add43 = fadd <4 x float> %tmp41, %tmp42        ; <<4 x float>> [#uses=1]
  store <4 x float> %add43, <4 x float>* %ar2
  %tmp45 = load <4 x float>* %ar0                 ; <<4 x float>> [#uses=1]
  %tmp46 = load <4 x float>* %ar2                 ; <<4 x float>> [#uses=1]
  %add47 = fadd <4 x float> %tmp45, %tmp46        ; <<4 x float>> [#uses=1]
  store <4 x float> %add47, <4 x float>* %br0
  %tmp49 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp50 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp49, %tmp50          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %br1
  %tmp52 = load <4 x float>* %ar0                 ; <<4 x float>> [#uses=1]
  %tmp53 = load <4 x float>* %ar2                 ; <<4 x float>> [#uses=1]
  %sub54 = fsub <4 x float> %tmp52, %tmp53        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub54, <4 x float>* %br2
  %tmp56 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp57 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %sub58 = fsub <4 x float> %tmp56, %tmp57        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub58, <4 x float>* %br3
  %tmp60 = load <4 x float>* %zi0                 ; <<4 x float>> [#uses=1]
  %tmp61 = load <4 x float>* %zi2                 ; <<4 x float>> [#uses=1]
  %add62 = fadd <4 x float> %tmp60, %tmp61        ; <<4 x float>> [#uses=1]
  store <4 x float> %add62, <4 x float>* %ai0
  %tmp64 = load <4 x float>* %zi1                 ; <<4 x float>> [#uses=1]
  %tmp65 = load <4 x float>* %zi3                 ; <<4 x float>> [#uses=1]
  %add66 = fadd <4 x float> %tmp64, %tmp65        ; <<4 x float>> [#uses=1]
  store <4 x float> %add66, <4 x float>* %ai2
  %tmp68 = load <4 x float>* %ai0                 ; <<4 x float>> [#uses=1]
  %tmp69 = load <4 x float>* %ai2                 ; <<4 x float>> [#uses=1]
  %add70 = fadd <4 x float> %tmp68, %tmp69        ; <<4 x float>> [#uses=1]
  store <4 x float> %add70, <4 x float>* %bi0
  %tmp72 = load <4 x float>* %zi0                 ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %zi2                 ; <<4 x float>> [#uses=1]
  %sub74 = fsub <4 x float> %tmp72, %tmp73        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub74, <4 x float>* %bi1
  %tmp76 = load <4 x float>* %ai0                 ; <<4 x float>> [#uses=1]
  %tmp77 = load <4 x float>* %ai2                 ; <<4 x float>> [#uses=1]
  %sub78 = fsub <4 x float> %tmp76, %tmp77        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub78, <4 x float>* %bi2
  %tmp80 = load <4 x float>* %zi1                 ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %zi3                 ; <<4 x float>> [#uses=1]
  %sub82 = fsub <4 x float> %tmp80, %tmp81        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub82, <4 x float>* %bi3
  %tmp83 = load <4 x float>* %br0                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp83, <4 x float>* %zr0
  %tmp84 = load <4 x float>* %bi0                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp84, <4 x float>* %zi0
  %tmp85 = load <4 x float>* %br1                 ; <<4 x float>> [#uses=1]
  %tmp86 = load <4 x float>* %bi3                 ; <<4 x float>> [#uses=1]
  %add87 = fadd <4 x float> %tmp85, %tmp86        ; <<4 x float>> [#uses=1]
  store <4 x float> %add87, <4 x float>* %zr1
  %tmp88 = load <4 x float>* %bi1                 ; <<4 x float>> [#uses=1]
  %tmp89 = load <4 x float>* %br3                 ; <<4 x float>> [#uses=1]
  %sub90 = fsub <4 x float> %tmp88, %tmp89        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub90, <4 x float>* %zi1
  %tmp91 = load <4 x float>* %br1                 ; <<4 x float>> [#uses=1]
  %tmp92 = load <4 x float>* %bi3                 ; <<4 x float>> [#uses=1]
  %sub93 = fsub <4 x float> %tmp91, %tmp92        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub93, <4 x float>* %zr3
  %tmp94 = load <4 x float>* %br3                 ; <<4 x float>> [#uses=1]
  %tmp95 = load <4 x float>* %bi1                 ; <<4 x float>> [#uses=1]
  %add96 = fadd <4 x float> %tmp94, %tmp95        ; <<4 x float>> [#uses=1]
  store <4 x float> %add96, <4 x float>* %zi3
  %tmp97 = load <4 x float>* %br2                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp97, <4 x float>* %zr2
  %tmp98 = load <4 x float>* %bi2                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp98, <4 x float>* %zi2
  br label %do.end

do.end:                                           ; preds = %do.body
  %tmp100 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl101 = shl i32 %tmp100, 2                    ; <i32> [#uses=1]
  %tmp102 = insertelement <4 x i32> undef, i32 %shl101, i32 0 ; <<4 x i32>> [#uses=2]
  %splat = shufflevector <4 x i32> %tmp102, <4 x i32> %tmp102, <4 x i32> zeroinitializer ; <<4 x i32>> [#uses=1]
  store <4 x i32> <i32 0, i32 1, i32 2, i32 3>, <4 x i32>* %.compoundliteral
  %tmp103 = load <4 x i32>* %.compoundliteral     ; <<4 x i32>> [#uses=1]
  %add104 = add nsw <4 x i32> %splat, %tmp103     ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add104, <4 x i32>* %tbase
  br label %do.body105

do.body105:                                       ; preds = %do.end
  %tmp108 = load <4 x i32>* %tbase                ; <<4 x i32>> [#uses=1]
  %mul = mul <4 x i32> %tmp108, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul, <4 x i32>* %i.addr.i
  store <4 x float>* %c1, <4 x float>** %cretp.addr.i
  %tmp.i = load <4 x i32>* %i.addr.i              ; <<4 x i32>> [#uses=1]
  %cmp.i = icmp sgt <4 x i32> %tmp.i, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i = sext <4 x i1> %cmp.i to <4 x i32>     ; <<4 x i32>> [#uses=1]
  %and.i = and <4 x i32> %sext.i, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i = load <4 x i32>* %i.addr.i             ; <<4 x i32>> [#uses=1]
  %sub.i = sub <4 x i32> %tmp1.i, %and.i          ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i, <4 x i32>* %i.addr.i
  %tmp3.i = load <4 x i32>* %i.addr.i             ; <<4 x i32>> [#uses=1]
  %call.i = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i) nounwind ; <<4 x float>> [#uses=1]
  %mul.i = fmul <4 x float> %call.i, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i, <4 x float>* %x.i
  %tmp4.i = load <4 x float>* %x.i                ; <<4 x float>> [#uses=1]
  %call5.i = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i = load <4 x float>** %cretp.addr.i      ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i, <4 x float>* %tmp6.i
  %tmp7.i = load <4 x float>* %x.i                ; <<4 x float>> [#uses=1]
  %call8.i = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i, <4 x float>* %retval.i
  %2 = load <4 x float>* %retval.i                ; <<4 x float>> [#uses=1]
  store <4 x float> %2, <4 x float>* %s1
  br label %do.body109

do.body109:                                       ; preds = %do.body105
  %tmp111 = load <4 x float>* %c1                 ; <<4 x float>> [#uses=1]
  %tmp112 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul113 = fmul <4 x float> %tmp111, %tmp112     ; <<4 x float>> [#uses=1]
  %tmp114 = load <4 x float>* %s1                 ; <<4 x float>> [#uses=1]
  %tmp115 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul116 = fmul <4 x float> %tmp114, %tmp115     ; <<4 x float>> [#uses=1]
  %sub117 = fsub <4 x float> %mul113, %mul116     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub117, <4 x float>* %__r
  %tmp118 = load <4 x float>* %c1                 ; <<4 x float>> [#uses=1]
  %tmp119 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul120 = fmul <4 x float> %tmp118, %tmp119     ; <<4 x float>> [#uses=1]
  %tmp121 = load <4 x float>* %s1                 ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul123 = fmul <4 x float> %tmp121, %tmp122     ; <<4 x float>> [#uses=1]
  %add124 = fadd <4 x float> %mul120, %mul123     ; <<4 x float>> [#uses=1]
  store <4 x float> %add124, <4 x float>* %zi1
  %tmp125 = load <4 x float>* %__r                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp125, <4 x float>* %zr1
  br label %do.end126

do.end126:                                        ; preds = %do.body109
  %tmp129 = load <4 x i32>* %tbase                ; <<4 x i32>> [#uses=1]
  %mul130 = mul <4 x i32> %tmp129, <i32 2, i32 2, i32 2, i32 2> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul130, <4 x i32>* %i.addr.i343
  store <4 x float>* %c2, <4 x float>** %cretp.addr.i344
  %tmp.i346 = load <4 x i32>* %i.addr.i343        ; <<4 x i32>> [#uses=1]
  %cmp.i347 = icmp sgt <4 x i32> %tmp.i346, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i348 = sext <4 x i1> %cmp.i347 to <4 x i32> ; <<4 x i32>> [#uses=1]
  %and.i349 = and <4 x i32> %sext.i348, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i350 = load <4 x i32>* %i.addr.i343       ; <<4 x i32>> [#uses=1]
  %sub.i351 = sub <4 x i32> %tmp1.i350, %and.i349 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i351, <4 x i32>* %i.addr.i343
  %tmp3.i352 = load <4 x i32>* %i.addr.i343       ; <<4 x i32>> [#uses=1]
  %call.i353 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i352) nounwind ; <<4 x float>> [#uses=1]
  %mul.i354 = fmul <4 x float> %call.i353, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i354, <4 x float>* %x.i345
  %tmp4.i355 = load <4 x float>* %x.i345          ; <<4 x float>> [#uses=1]
  %call5.i356 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i355) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i357 = load <4 x float>** %cretp.addr.i344 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i356, <4 x float>* %tmp6.i357
  %tmp7.i358 = load <4 x float>* %x.i345          ; <<4 x float>> [#uses=1]
  %call8.i359 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i358) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i359, <4 x float>* %retval.i342
  %3 = load <4 x float>* %retval.i342             ; <<4 x float>> [#uses=1]
  store <4 x float> %3, <4 x float>* %s2
  br label %do.body132

do.body132:                                       ; preds = %do.end126
  %tmp135 = load <4 x float>* %c2                 ; <<4 x float>> [#uses=1]
  %tmp136 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul137 = fmul <4 x float> %tmp135, %tmp136     ; <<4 x float>> [#uses=1]
  %tmp138 = load <4 x float>* %s2                 ; <<4 x float>> [#uses=1]
  %tmp139 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul140 = fmul <4 x float> %tmp138, %tmp139     ; <<4 x float>> [#uses=1]
  %sub141 = fsub <4 x float> %mul137, %mul140     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub141, <4 x float>* %__r134
  %tmp142 = load <4 x float>* %c2                 ; <<4 x float>> [#uses=1]
  %tmp143 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul144 = fmul <4 x float> %tmp142, %tmp143     ; <<4 x float>> [#uses=1]
  %tmp145 = load <4 x float>* %s2                 ; <<4 x float>> [#uses=1]
  %tmp146 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul147 = fmul <4 x float> %tmp145, %tmp146     ; <<4 x float>> [#uses=1]
  %add148 = fadd <4 x float> %mul144, %mul147     ; <<4 x float>> [#uses=1]
  store <4 x float> %add148, <4 x float>* %zi2
  %tmp149 = load <4 x float>* %__r134             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp149, <4 x float>* %zr2
  br label %do.end150

do.end150:                                        ; preds = %do.body132
  %tmp153 = load <4 x i32>* %tbase                ; <<4 x i32>> [#uses=1]
  %mul154 = mul <4 x i32> %tmp153, <i32 3, i32 3, i32 3, i32 3> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul154, <4 x i32>* %i.addr.i325
  store <4 x float>* %c3, <4 x float>** %cretp.addr.i326
  %tmp.i328 = load <4 x i32>* %i.addr.i325        ; <<4 x i32>> [#uses=1]
  %cmp.i329 = icmp sgt <4 x i32> %tmp.i328, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i330 = sext <4 x i1> %cmp.i329 to <4 x i32> ; <<4 x i32>> [#uses=1]
  %and.i331 = and <4 x i32> %sext.i330, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i332 = load <4 x i32>* %i.addr.i325       ; <<4 x i32>> [#uses=1]
  %sub.i333 = sub <4 x i32> %tmp1.i332, %and.i331 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i333, <4 x i32>* %i.addr.i325
  %tmp3.i334 = load <4 x i32>* %i.addr.i325       ; <<4 x i32>> [#uses=1]
  %call.i335 = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i334) nounwind ; <<4 x float>> [#uses=1]
  %mul.i336 = fmul <4 x float> %call.i335, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i336, <4 x float>* %x.i327
  %tmp4.i337 = load <4 x float>* %x.i327          ; <<4 x float>> [#uses=1]
  %call5.i338 = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i337) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i339 = load <4 x float>** %cretp.addr.i326 ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i338, <4 x float>* %tmp6.i339
  %tmp7.i340 = load <4 x float>* %x.i327          ; <<4 x float>> [#uses=1]
  %call8.i341 = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i340) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i341, <4 x float>* %retval.i324
  %4 = load <4 x float>* %retval.i324             ; <<4 x float>> [#uses=1]
  store <4 x float> %4, <4 x float>* %s3
  br label %do.body156

do.body156:                                       ; preds = %do.end150
  %tmp159 = load <4 x float>* %c3                 ; <<4 x float>> [#uses=1]
  %tmp160 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul161 = fmul <4 x float> %tmp159, %tmp160     ; <<4 x float>> [#uses=1]
  %tmp162 = load <4 x float>* %s3                 ; <<4 x float>> [#uses=1]
  %tmp163 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul164 = fmul <4 x float> %tmp162, %tmp163     ; <<4 x float>> [#uses=1]
  %sub165 = fsub <4 x float> %mul161, %mul164     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub165, <4 x float>* %__r158
  %tmp166 = load <4 x float>* %c3                 ; <<4 x float>> [#uses=1]
  %tmp167 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul168 = fmul <4 x float> %tmp166, %tmp167     ; <<4 x float>> [#uses=1]
  %tmp169 = load <4 x float>* %s3                 ; <<4 x float>> [#uses=1]
  %tmp170 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul171 = fmul <4 x float> %tmp169, %tmp170     ; <<4 x float>> [#uses=1]
  %add172 = fadd <4 x float> %mul168, %mul171     ; <<4 x float>> [#uses=1]
  store <4 x float> %add172, <4 x float>* %zi3
  %tmp173 = load <4 x float>* %__r158             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173, <4 x float>* %zr3
  br label %do.end174

do.end174:                                        ; preds = %do.body156
  br label %do.end175

do.end175:                                        ; preds = %do.end174
  %tmp176 = load float addrspace(3)** %lds.addr   ; <float addrspace(3)*> [#uses=1]
  %tmp177 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl178 = shl i32 %tmp177, 2                    ; <i32> [#uses=1]
  %tmp179 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shr = lshr i32 %tmp179, 3                      ; <i32> [#uses=1]
  %add180 = add i32 %shl178, %shr                 ; <i32> [#uses=1]
  %add.ptr181 = getelementptr inbounds float addrspace(3)* %tmp176, i32 %add180 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr181, float addrspace(3)** %lp
  %tmp182 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp183 = extractelement <4 x float> %tmp182, i32 0 ; <float> [#uses=1]
  %tmp184 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx185 = getelementptr inbounds float addrspace(3)* %tmp184, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp183, float addrspace(3)* %arrayidx185
  %tmp186 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp187 = extractelement <4 x float> %tmp186, i32 1 ; <float> [#uses=1]
  %tmp188 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx189 = getelementptr inbounds float addrspace(3)* %tmp188, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp187, float addrspace(3)* %arrayidx189
  %tmp190 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp191 = extractelement <4 x float> %tmp190, i32 2 ; <float> [#uses=1]
  %tmp192 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx193 = getelementptr inbounds float addrspace(3)* %tmp192, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp191, float addrspace(3)* %arrayidx193
  %tmp194 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp195 = extractelement <4 x float> %tmp194, i32 3 ; <float> [#uses=1]
  %tmp196 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx197 = getelementptr inbounds float addrspace(3)* %tmp196, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp195, float addrspace(3)* %arrayidx197
  %tmp198 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr199 = getelementptr inbounds float addrspace(3)* %tmp198, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr199, float addrspace(3)** %lp
  %tmp200 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp201 = extractelement <4 x float> %tmp200, i32 0 ; <float> [#uses=1]
  %tmp202 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx203 = getelementptr inbounds float addrspace(3)* %tmp202, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp201, float addrspace(3)* %arrayidx203
  %tmp204 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp205 = extractelement <4 x float> %tmp204, i32 1 ; <float> [#uses=1]
  %tmp206 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx207 = getelementptr inbounds float addrspace(3)* %tmp206, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp205, float addrspace(3)* %arrayidx207
  %tmp208 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp209 = extractelement <4 x float> %tmp208, i32 2 ; <float> [#uses=1]
  %tmp210 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx211 = getelementptr inbounds float addrspace(3)* %tmp210, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp209, float addrspace(3)* %arrayidx211
  %tmp212 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp213 = extractelement <4 x float> %tmp212, i32 3 ; <float> [#uses=1]
  %tmp214 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx215 = getelementptr inbounds float addrspace(3)* %tmp214, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp213, float addrspace(3)* %arrayidx215
  %tmp216 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr217 = getelementptr inbounds float addrspace(3)* %tmp216, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr217, float addrspace(3)** %lp
  %tmp218 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp219 = extractelement <4 x float> %tmp218, i32 0 ; <float> [#uses=1]
  %tmp220 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx221 = getelementptr inbounds float addrspace(3)* %tmp220, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp219, float addrspace(3)* %arrayidx221
  %tmp222 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp223 = extractelement <4 x float> %tmp222, i32 1 ; <float> [#uses=1]
  %tmp224 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx225 = getelementptr inbounds float addrspace(3)* %tmp224, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp223, float addrspace(3)* %arrayidx225
  %tmp226 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp227 = extractelement <4 x float> %tmp226, i32 2 ; <float> [#uses=1]
  %tmp228 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx229 = getelementptr inbounds float addrspace(3)* %tmp228, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp227, float addrspace(3)* %arrayidx229
  %tmp230 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp231 = extractelement <4 x float> %tmp230, i32 3 ; <float> [#uses=1]
  %tmp232 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx233 = getelementptr inbounds float addrspace(3)* %tmp232, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp231, float addrspace(3)* %arrayidx233
  %tmp234 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr235 = getelementptr inbounds float addrspace(3)* %tmp234, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr235, float addrspace(3)** %lp
  %tmp236 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp237 = extractelement <4 x float> %tmp236, i32 0 ; <float> [#uses=1]
  %tmp238 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx239 = getelementptr inbounds float addrspace(3)* %tmp238, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp237, float addrspace(3)* %arrayidx239
  %tmp240 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp241 = extractelement <4 x float> %tmp240, i32 1 ; <float> [#uses=1]
  %tmp242 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx243 = getelementptr inbounds float addrspace(3)* %tmp242, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp241, float addrspace(3)* %arrayidx243
  %tmp244 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp245 = extractelement <4 x float> %tmp244, i32 2 ; <float> [#uses=1]
  %tmp246 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx247 = getelementptr inbounds float addrspace(3)* %tmp246, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp245, float addrspace(3)* %arrayidx247
  %tmp248 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp249 = extractelement <4 x float> %tmp248, i32 3 ; <float> [#uses=1]
  %tmp250 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx251 = getelementptr inbounds float addrspace(3)* %tmp250, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp249, float addrspace(3)* %arrayidx251
  %tmp252 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr253 = getelementptr inbounds float addrspace(3)* %tmp252, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr253, float addrspace(3)** %lp
  %tmp254 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp255 = extractelement <4 x float> %tmp254, i32 0 ; <float> [#uses=1]
  %tmp256 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx257 = getelementptr inbounds float addrspace(3)* %tmp256, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp255, float addrspace(3)* %arrayidx257
  %tmp258 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp259 = extractelement <4 x float> %tmp258, i32 1 ; <float> [#uses=1]
  %tmp260 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx261 = getelementptr inbounds float addrspace(3)* %tmp260, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp259, float addrspace(3)* %arrayidx261
  %tmp262 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp263 = extractelement <4 x float> %tmp262, i32 2 ; <float> [#uses=1]
  %tmp264 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx265 = getelementptr inbounds float addrspace(3)* %tmp264, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp263, float addrspace(3)* %arrayidx265
  %tmp266 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp267 = extractelement <4 x float> %tmp266, i32 3 ; <float> [#uses=1]
  %tmp268 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx269 = getelementptr inbounds float addrspace(3)* %tmp268, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp267, float addrspace(3)* %arrayidx269
  %tmp270 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr271 = getelementptr inbounds float addrspace(3)* %tmp270, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr271, float addrspace(3)** %lp
  %tmp272 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp273 = extractelement <4 x float> %tmp272, i32 0 ; <float> [#uses=1]
  %tmp274 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx275 = getelementptr inbounds float addrspace(3)* %tmp274, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp273, float addrspace(3)* %arrayidx275
  %tmp276 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp277 = extractelement <4 x float> %tmp276, i32 1 ; <float> [#uses=1]
  %tmp278 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx279 = getelementptr inbounds float addrspace(3)* %tmp278, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp277, float addrspace(3)* %arrayidx279
  %tmp280 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp281 = extractelement <4 x float> %tmp280, i32 2 ; <float> [#uses=1]
  %tmp282 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx283 = getelementptr inbounds float addrspace(3)* %tmp282, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp281, float addrspace(3)* %arrayidx283
  %tmp284 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp285 = extractelement <4 x float> %tmp284, i32 3 ; <float> [#uses=1]
  %tmp286 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx287 = getelementptr inbounds float addrspace(3)* %tmp286, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp285, float addrspace(3)* %arrayidx287
  %tmp288 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr289 = getelementptr inbounds float addrspace(3)* %tmp288, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr289, float addrspace(3)** %lp
  %tmp290 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp291 = extractelement <4 x float> %tmp290, i32 0 ; <float> [#uses=1]
  %tmp292 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx293 = getelementptr inbounds float addrspace(3)* %tmp292, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp291, float addrspace(3)* %arrayidx293
  %tmp294 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp295 = extractelement <4 x float> %tmp294, i32 1 ; <float> [#uses=1]
  %tmp296 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx297 = getelementptr inbounds float addrspace(3)* %tmp296, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp295, float addrspace(3)* %arrayidx297
  %tmp298 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp299 = extractelement <4 x float> %tmp298, i32 2 ; <float> [#uses=1]
  %tmp300 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx301 = getelementptr inbounds float addrspace(3)* %tmp300, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp299, float addrspace(3)* %arrayidx301
  %tmp302 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp303 = extractelement <4 x float> %tmp302, i32 3 ; <float> [#uses=1]
  %tmp304 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx305 = getelementptr inbounds float addrspace(3)* %tmp304, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp303, float addrspace(3)* %arrayidx305
  %tmp306 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr307 = getelementptr inbounds float addrspace(3)* %tmp306, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr307, float addrspace(3)** %lp
  %tmp308 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp309 = extractelement <4 x float> %tmp308, i32 0 ; <float> [#uses=1]
  %tmp310 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx311 = getelementptr inbounds float addrspace(3)* %tmp310, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp309, float addrspace(3)* %arrayidx311
  %tmp312 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp313 = extractelement <4 x float> %tmp312, i32 1 ; <float> [#uses=1]
  %tmp314 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx315 = getelementptr inbounds float addrspace(3)* %tmp314, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp313, float addrspace(3)* %arrayidx315
  %tmp316 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp317 = extractelement <4 x float> %tmp316, i32 2 ; <float> [#uses=1]
  %tmp318 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx319 = getelementptr inbounds float addrspace(3)* %tmp318, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp317, float addrspace(3)* %arrayidx319
  %tmp320 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp321 = extractelement <4 x float> %tmp320, i32 3 ; <float> [#uses=1]
  %tmp322 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx323 = getelementptr inbounds float addrspace(3)* %tmp322, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp321, float addrspace(3)* %arrayidx323
  call void @barrier(i32 1)
  ret void
}

declare void @barrier(i32)

; CHECK: ret
define void @kfft_pass2(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
entry:
  %retval.i510 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i511 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i512 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i513 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i492 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i493 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i494 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i495 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i = alloca float, align 4               ; <float*> [#uses=2]
  %i.addr.i = alloca i32, align 4                 ; <i32*> [#uses=5]
  %cretp.addr.i = alloca float*, align 4          ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %me.addr = alloca i32, align 4                  ; <i32*> [#uses=6]
  %lds.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 4       ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r285 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r317 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %me.addr
  store float addrspace(3)* %lds, float addrspace(3)** %lds.addr
  %tmp = load float addrspace(3)** %lds.addr      ; <float addrspace(3)*> [#uses=1]
  %tmp1 = load i32* %me.addr                      ; <i32> [#uses=1]
  %tmp2 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shr = lshr i32 %tmp2, 5                        ; <i32> [#uses=1]
  %add = add i32 %tmp1, %shr                      ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(3)* %tmp, i32 %add ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr, float addrspace(3)** %lp
  %tmp7 = load float addrspace(3)** %lp           ; <float addrspace(3)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(3)* %tmp7, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8 = load float addrspace(3)* %arrayidx      ; <float> [#uses=1]
  %tmp9 = load <4 x float>* %zr0                  ; <<4 x float>> [#uses=1]
  %tmp10 = insertelement <4 x float> %tmp9, float %tmp8, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10, <4 x float>* %zr0
  %tmp11 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds float addrspace(3)* %tmp11, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp13 = load float addrspace(3)* %arrayidx12   ; <float> [#uses=1]
  %tmp14 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp15 = insertelement <4 x float> %tmp14, float %tmp13, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15, <4 x float>* %zr1
  %tmp16 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx17 = getelementptr inbounds float addrspace(3)* %tmp16, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp18 = load float addrspace(3)* %arrayidx17   ; <float> [#uses=1]
  %tmp19 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp20 = insertelement <4 x float> %tmp19, float %tmp18, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20, <4 x float>* %zr2
  %tmp21 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds float addrspace(3)* %tmp21, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp23 = load float addrspace(3)* %arrayidx22   ; <float> [#uses=1]
  %tmp24 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp25 = insertelement <4 x float> %tmp24, float %tmp23, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %zr3
  %tmp26 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr27 = getelementptr inbounds float addrspace(3)* %tmp26, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27, float addrspace(3)** %lp
  %tmp28 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(3)* %tmp28, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30 = load float addrspace(3)* %arrayidx29   ; <float> [#uses=1]
  %tmp31 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp32 = insertelement <4 x float> %tmp31, float %tmp30, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %zr0
  %tmp33 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(3)* %tmp33, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp35 = load float addrspace(3)* %arrayidx34   ; <float> [#uses=1]
  %tmp36 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp37 = insertelement <4 x float> %tmp36, float %tmp35, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37, <4 x float>* %zr1
  %tmp38 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds float addrspace(3)* %tmp38, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp40 = load float addrspace(3)* %arrayidx39   ; <float> [#uses=1]
  %tmp41 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp42 = insertelement <4 x float> %tmp41, float %tmp40, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %zr2
  %tmp43 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds float addrspace(3)* %tmp43, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp45 = load float addrspace(3)* %arrayidx44   ; <float> [#uses=1]
  %tmp46 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp47 = insertelement <4 x float> %tmp46, float %tmp45, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %zr3
  %tmp48 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr49 = getelementptr inbounds float addrspace(3)* %tmp48, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49, float addrspace(3)** %lp
  %tmp50 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds float addrspace(3)* %tmp50, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52 = load float addrspace(3)* %arrayidx51   ; <float> [#uses=1]
  %tmp53 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp54 = insertelement <4 x float> %tmp53, float %tmp52, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %zr0
  %tmp55 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(3)* %tmp55, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp57 = load float addrspace(3)* %arrayidx56   ; <float> [#uses=1]
  %tmp58 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp59 = insertelement <4 x float> %tmp58, float %tmp57, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59, <4 x float>* %zr1
  %tmp60 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx61 = getelementptr inbounds float addrspace(3)* %tmp60, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp62 = load float addrspace(3)* %arrayidx61   ; <float> [#uses=1]
  %tmp63 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp64 = insertelement <4 x float> %tmp63, float %tmp62, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64, <4 x float>* %zr2
  %tmp65 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float addrspace(3)* %tmp65, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp67 = load float addrspace(3)* %arrayidx66   ; <float> [#uses=1]
  %tmp68 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp69 = insertelement <4 x float> %tmp68, float %tmp67, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69, <4 x float>* %zr3
  %tmp70 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr71 = getelementptr inbounds float addrspace(3)* %tmp70, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71, float addrspace(3)** %lp
  %tmp72 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds float addrspace(3)* %tmp72, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74 = load float addrspace(3)* %arrayidx73   ; <float> [#uses=1]
  %tmp75 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp76 = insertelement <4 x float> %tmp75, float %tmp74, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76, <4 x float>* %zr0
  %tmp77 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(3)* %tmp77, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp79 = load float addrspace(3)* %arrayidx78   ; <float> [#uses=1]
  %tmp80 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp81 = insertelement <4 x float> %tmp80, float %tmp79, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81, <4 x float>* %zr1
  %tmp82 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds float addrspace(3)* %tmp82, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp84 = load float addrspace(3)* %arrayidx83   ; <float> [#uses=1]
  %tmp85 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp86 = insertelement <4 x float> %tmp85, float %tmp84, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86, <4 x float>* %zr2
  %tmp87 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds float addrspace(3)* %tmp87, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp89 = load float addrspace(3)* %arrayidx88   ; <float> [#uses=1]
  %tmp90 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp91 = insertelement <4 x float> %tmp90, float %tmp89, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91, <4 x float>* %zr3
  %tmp92 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr93 = getelementptr inbounds float addrspace(3)* %tmp92, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93, float addrspace(3)** %lp
  %tmp98 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds float addrspace(3)* %tmp98, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100 = load float addrspace(3)* %arrayidx99  ; <float> [#uses=1]
  %tmp101 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp102 = insertelement <4 x float> %tmp101, float %tmp100, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102, <4 x float>* %zi0
  %tmp103 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx104 = getelementptr inbounds float addrspace(3)* %tmp103, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp105 = load float addrspace(3)* %arrayidx104 ; <float> [#uses=1]
  %tmp106 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp107 = insertelement <4 x float> %tmp106, float %tmp105, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107, <4 x float>* %zi1
  %tmp108 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(3)* %tmp108, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp110 = load float addrspace(3)* %arrayidx109 ; <float> [#uses=1]
  %tmp111 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp112 = insertelement <4 x float> %tmp111, float %tmp110, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112, <4 x float>* %zi2
  %tmp113 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds float addrspace(3)* %tmp113, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp115 = load float addrspace(3)* %arrayidx114 ; <float> [#uses=1]
  %tmp116 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp117 = insertelement <4 x float> %tmp116, float %tmp115, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117, <4 x float>* %zi3
  %tmp118 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr119 = getelementptr inbounds float addrspace(3)* %tmp118, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119, float addrspace(3)** %lp
  %tmp120 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds float addrspace(3)* %tmp120, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122 = load float addrspace(3)* %arrayidx121 ; <float> [#uses=1]
  %tmp123 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp124 = insertelement <4 x float> %tmp123, float %tmp122, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124, <4 x float>* %zi0
  %tmp125 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds float addrspace(3)* %tmp125, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp127 = load float addrspace(3)* %arrayidx126 ; <float> [#uses=1]
  %tmp128 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp129 = insertelement <4 x float> %tmp128, float %tmp127, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129, <4 x float>* %zi1
  %tmp130 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds float addrspace(3)* %tmp130, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp132 = load float addrspace(3)* %arrayidx131 ; <float> [#uses=1]
  %tmp133 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp134 = insertelement <4 x float> %tmp133, float %tmp132, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134, <4 x float>* %zi2
  %tmp135 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx136 = getelementptr inbounds float addrspace(3)* %tmp135, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp137 = load float addrspace(3)* %arrayidx136 ; <float> [#uses=1]
  %tmp138 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp139 = insertelement <4 x float> %tmp138, float %tmp137, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139, <4 x float>* %zi3
  %tmp140 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr141 = getelementptr inbounds float addrspace(3)* %tmp140, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141, float addrspace(3)** %lp
  %tmp142 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx143 = getelementptr inbounds float addrspace(3)* %tmp142, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144 = load float addrspace(3)* %arrayidx143 ; <float> [#uses=1]
  %tmp145 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp146 = insertelement <4 x float> %tmp145, float %tmp144, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146, <4 x float>* %zi0
  %tmp147 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx148 = getelementptr inbounds float addrspace(3)* %tmp147, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp149 = load float addrspace(3)* %arrayidx148 ; <float> [#uses=1]
  %tmp150 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp151 = insertelement <4 x float> %tmp150, float %tmp149, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151, <4 x float>* %zi1
  %tmp152 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx153 = getelementptr inbounds float addrspace(3)* %tmp152, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp154 = load float addrspace(3)* %arrayidx153 ; <float> [#uses=1]
  %tmp155 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp156 = insertelement <4 x float> %tmp155, float %tmp154, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156, <4 x float>* %zi2
  %tmp157 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx158 = getelementptr inbounds float addrspace(3)* %tmp157, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp159 = load float addrspace(3)* %arrayidx158 ; <float> [#uses=1]
  %tmp160 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp161 = insertelement <4 x float> %tmp160, float %tmp159, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161, <4 x float>* %zi3
  %tmp162 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr163 = getelementptr inbounds float addrspace(3)* %tmp162, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163, float addrspace(3)** %lp
  %tmp164 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx165 = getelementptr inbounds float addrspace(3)* %tmp164, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166 = load float addrspace(3)* %arrayidx165 ; <float> [#uses=1]
  %tmp167 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp168 = insertelement <4 x float> %tmp167, float %tmp166, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168, <4 x float>* %zi0
  %tmp169 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx170 = getelementptr inbounds float addrspace(3)* %tmp169, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp171 = load float addrspace(3)* %arrayidx170 ; <float> [#uses=1]
  %tmp172 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp173 = insertelement <4 x float> %tmp172, float %tmp171, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173, <4 x float>* %zi1
  %tmp174 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(3)* %tmp174, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp176 = load float addrspace(3)* %arrayidx175 ; <float> [#uses=1]
  %tmp177 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp178 = insertelement <4 x float> %tmp177, float %tmp176, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178, <4 x float>* %zi2
  %tmp179 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx180 = getelementptr inbounds float addrspace(3)* %tmp179, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp181 = load float addrspace(3)* %arrayidx180 ; <float> [#uses=1]
  %tmp182 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp183 = insertelement <4 x float> %tmp182, float %tmp181, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183, <4 x float>* %zi3
  br label %do.body

do.body:                                          ; preds = %entry
  %tmp185 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp186 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %add187 = fadd <4 x float> %tmp185, %tmp186     ; <<4 x float>> [#uses=1]
  store <4 x float> %add187, <4 x float>* %ar0
  %tmp189 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp190 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %add191 = fadd <4 x float> %tmp189, %tmp190     ; <<4 x float>> [#uses=1]
  store <4 x float> %add191, <4 x float>* %ar2
  %tmp193 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp194 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %add195 = fadd <4 x float> %tmp193, %tmp194     ; <<4 x float>> [#uses=1]
  store <4 x float> %add195, <4 x float>* %br0
  %tmp197 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp198 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp197, %tmp198        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %br1
  %tmp200 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp201 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %sub202 = fsub <4 x float> %tmp200, %tmp201     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202, <4 x float>* %br2
  %tmp204 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp205 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %sub206 = fsub <4 x float> %tmp204, %tmp205     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206, <4 x float>* %br3
  %tmp208 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp209 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %add210 = fadd <4 x float> %tmp208, %tmp209     ; <<4 x float>> [#uses=1]
  store <4 x float> %add210, <4 x float>* %ai0
  %tmp212 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp213 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %add214 = fadd <4 x float> %tmp212, %tmp213     ; <<4 x float>> [#uses=1]
  store <4 x float> %add214, <4 x float>* %ai2
  %tmp216 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp217 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %add218 = fadd <4 x float> %tmp216, %tmp217     ; <<4 x float>> [#uses=1]
  store <4 x float> %add218, <4 x float>* %bi0
  %tmp220 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp221 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %sub222 = fsub <4 x float> %tmp220, %tmp221     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222, <4 x float>* %bi1
  %tmp224 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp225 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %sub226 = fsub <4 x float> %tmp224, %tmp225     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226, <4 x float>* %bi2
  %tmp228 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp229 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %sub230 = fsub <4 x float> %tmp228, %tmp229     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230, <4 x float>* %bi3
  %tmp231 = load <4 x float>* %br0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231, <4 x float>* %zr0
  %tmp232 = load <4 x float>* %bi0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232, <4 x float>* %zi0
  %tmp233 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp234 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %add235 = fadd <4 x float> %tmp233, %tmp234     ; <<4 x float>> [#uses=1]
  store <4 x float> %add235, <4 x float>* %zr1
  %tmp236 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %tmp237 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %sub238 = fsub <4 x float> %tmp236, %tmp237     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238, <4 x float>* %zi1
  %tmp239 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp240 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %sub241 = fsub <4 x float> %tmp239, %tmp240     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241, <4 x float>* %zr3
  %tmp242 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %tmp243 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %add244 = fadd <4 x float> %tmp242, %tmp243     ; <<4 x float>> [#uses=1]
  store <4 x float> %add244, <4 x float>* %zi3
  %tmp245 = load <4 x float>* %br2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245, <4 x float>* %zr2
  %tmp246 = load <4 x float>* %bi2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246, <4 x float>* %zi2
  br label %do.end

do.end:                                           ; preds = %do.body
  %tmp248 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl = shl i32 %tmp248, 2                       ; <i32> [#uses=1]
  store i32 %shl, i32* %tbase
  br label %do.body249

do.body249:                                       ; preds = %do.end
  %tmp252 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul = mul i32 %tmp252, 1                       ; <i32> [#uses=1]
  store i32 %mul, i32* %i.addr.i
  store float* %c1, float** %cretp.addr.i
  %tmp.i = load i32* %i.addr.i                    ; <i32> [#uses=1]
  %cmp.i = icmp sgt i32 %tmp.i, 512               ; <i1> [#uses=1]
  br i1 %cmp.i, label %if.then.i, label %k_sincos.exit

if.then.i:                                        ; preds = %do.body249
  %tmp1.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %sub.i = sub i32 %tmp1.i, 1024                  ; <i32> [#uses=1]
  store i32 %sub.i, i32* %i.addr.i
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %do.body249, %if.then.i
  %tmp3.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %conv.i = sitofp i32 %tmp3.i to float           ; <float> [#uses=1]
  %mul.i = fmul float %conv.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i, float* %x.i
  %tmp4.i = load float* %x.i                      ; <float> [#uses=1]
  %call.i = call float @_Z10native_cosf(float %tmp4.i) nounwind ; <float> [#uses=1]
  %tmp5.i = load float** %cretp.addr.i            ; <float*> [#uses=1]
  store float %call.i, float* %tmp5.i
  %tmp6.i = load float* %x.i                      ; <float> [#uses=1]
  %call7.i = call float @_Z10native_sinf(float %tmp6.i) nounwind ; <float> [#uses=1]
  store float %call7.i, float* %retval.i
  %0 = load float* %retval.i                      ; <float> [#uses=1]
  store float %0, float* %s1
  br label %do.body253

do.body253:                                       ; preds = %k_sincos.exit
  %tmp255 = load float* %c1                       ; <float> [#uses=1]
  %tmp256 = insertelement <4 x float> undef, float %tmp255, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp256, <4 x float> %tmp256, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp257 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul258 = fmul <4 x float> %splat, %tmp257      ; <<4 x float>> [#uses=1]
  %tmp259 = load float* %s1                       ; <float> [#uses=1]
  %tmp260 = insertelement <4 x float> undef, float %tmp259, i32 0 ; <<4 x float>> [#uses=2]
  %splat261 = shufflevector <4 x float> %tmp260, <4 x float> %tmp260, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp262 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul263 = fmul <4 x float> %splat261, %tmp262   ; <<4 x float>> [#uses=1]
  %sub264 = fsub <4 x float> %mul258, %mul263     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub264, <4 x float>* %__r
  %tmp265 = load float* %c1                       ; <float> [#uses=1]
  %tmp266 = insertelement <4 x float> undef, float %tmp265, i32 0 ; <<4 x float>> [#uses=2]
  %splat267 = shufflevector <4 x float> %tmp266, <4 x float> %tmp266, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp268 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul269 = fmul <4 x float> %splat267, %tmp268   ; <<4 x float>> [#uses=1]
  %tmp270 = load float* %s1                       ; <float> [#uses=1]
  %tmp271 = insertelement <4 x float> undef, float %tmp270, i32 0 ; <<4 x float>> [#uses=2]
  %splat272 = shufflevector <4 x float> %tmp271, <4 x float> %tmp271, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp273 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul274 = fmul <4 x float> %splat272, %tmp273   ; <<4 x float>> [#uses=1]
  %add275 = fadd <4 x float> %mul269, %mul274     ; <<4 x float>> [#uses=1]
  store <4 x float> %add275, <4 x float>* %zi1
  %tmp276 = load <4 x float>* %__r                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp276, <4 x float>* %zr1
  br label %do.end277

do.end277:                                        ; preds = %do.body253
  %tmp280 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul281 = mul i32 %tmp280, 2                    ; <i32> [#uses=1]
  store i32 %mul281, i32* %i.addr.i511
  store float* %c2, float** %cretp.addr.i512
  %tmp.i514 = load i32* %i.addr.i511              ; <i32> [#uses=1]
  %cmp.i515 = icmp sgt i32 %tmp.i514, 512         ; <i1> [#uses=1]
  br i1 %cmp.i515, label %if.then.i518, label %k_sincos.exit527

if.then.i518:                                     ; preds = %do.end277
  %tmp1.i516 = load i32* %i.addr.i511             ; <i32> [#uses=1]
  %sub.i517 = sub i32 %tmp1.i516, 1024            ; <i32> [#uses=1]
  store i32 %sub.i517, i32* %i.addr.i511
  br label %k_sincos.exit527

k_sincos.exit527:                                 ; preds = %do.end277, %if.then.i518
  %tmp3.i519 = load i32* %i.addr.i511             ; <i32> [#uses=1]
  %conv.i520 = sitofp i32 %tmp3.i519 to float     ; <float> [#uses=1]
  %mul.i521 = fmul float %conv.i520, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i521, float* %x.i513
  %tmp4.i522 = load float* %x.i513                ; <float> [#uses=1]
  %call.i523 = call float @_Z10native_cosf(float %tmp4.i522) nounwind ; <float> [#uses=1]
  %tmp5.i524 = load float** %cretp.addr.i512      ; <float*> [#uses=1]
  store float %call.i523, float* %tmp5.i524
  %tmp6.i525 = load float* %x.i513                ; <float> [#uses=1]
  %call7.i526 = call float @_Z10native_sinf(float %tmp6.i525) nounwind ; <float> [#uses=1]
  store float %call7.i526, float* %retval.i510
  %1 = load float* %retval.i510                   ; <float> [#uses=1]
  store float %1, float* %s2
  br label %do.body283

do.body283:                                       ; preds = %k_sincos.exit527
  %tmp286 = load float* %c2                       ; <float> [#uses=1]
  %tmp287 = insertelement <4 x float> undef, float %tmp286, i32 0 ; <<4 x float>> [#uses=2]
  %splat288 = shufflevector <4 x float> %tmp287, <4 x float> %tmp287, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp289 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul290 = fmul <4 x float> %splat288, %tmp289   ; <<4 x float>> [#uses=1]
  %tmp291 = load float* %s2                       ; <float> [#uses=1]
  %tmp292 = insertelement <4 x float> undef, float %tmp291, i32 0 ; <<4 x float>> [#uses=2]
  %splat293 = shufflevector <4 x float> %tmp292, <4 x float> %tmp292, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp294 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul295 = fmul <4 x float> %splat293, %tmp294   ; <<4 x float>> [#uses=1]
  %sub296 = fsub <4 x float> %mul290, %mul295     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub296, <4 x float>* %__r285
  %tmp297 = load float* %c2                       ; <float> [#uses=1]
  %tmp298 = insertelement <4 x float> undef, float %tmp297, i32 0 ; <<4 x float>> [#uses=2]
  %splat299 = shufflevector <4 x float> %tmp298, <4 x float> %tmp298, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp300 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul301 = fmul <4 x float> %splat299, %tmp300   ; <<4 x float>> [#uses=1]
  %tmp302 = load float* %s2                       ; <float> [#uses=1]
  %tmp303 = insertelement <4 x float> undef, float %tmp302, i32 0 ; <<4 x float>> [#uses=2]
  %splat304 = shufflevector <4 x float> %tmp303, <4 x float> %tmp303, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp305 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul306 = fmul <4 x float> %splat304, %tmp305   ; <<4 x float>> [#uses=1]
  %add307 = fadd <4 x float> %mul301, %mul306     ; <<4 x float>> [#uses=1]
  store <4 x float> %add307, <4 x float>* %zi2
  %tmp308 = load <4 x float>* %__r285             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp308, <4 x float>* %zr2
  br label %do.end309

do.end309:                                        ; preds = %do.body283
  %tmp312 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul313 = mul i32 %tmp312, 3                    ; <i32> [#uses=1]
  store i32 %mul313, i32* %i.addr.i493
  store float* %c3, float** %cretp.addr.i494
  %tmp.i496 = load i32* %i.addr.i493              ; <i32> [#uses=1]
  %cmp.i497 = icmp sgt i32 %tmp.i496, 512         ; <i1> [#uses=1]
  br i1 %cmp.i497, label %if.then.i500, label %k_sincos.exit509

if.then.i500:                                     ; preds = %do.end309
  %tmp1.i498 = load i32* %i.addr.i493             ; <i32> [#uses=1]
  %sub.i499 = sub i32 %tmp1.i498, 1024            ; <i32> [#uses=1]
  store i32 %sub.i499, i32* %i.addr.i493
  br label %k_sincos.exit509

k_sincos.exit509:                                 ; preds = %do.end309, %if.then.i500
  %tmp3.i501 = load i32* %i.addr.i493             ; <i32> [#uses=1]
  %conv.i502 = sitofp i32 %tmp3.i501 to float     ; <float> [#uses=1]
  %mul.i503 = fmul float %conv.i502, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i503, float* %x.i495
  %tmp4.i504 = load float* %x.i495                ; <float> [#uses=1]
  %call.i505 = call float @_Z10native_cosf(float %tmp4.i504) nounwind ; <float> [#uses=1]
  %tmp5.i506 = load float** %cretp.addr.i494      ; <float*> [#uses=1]
  store float %call.i505, float* %tmp5.i506
  %tmp6.i507 = load float* %x.i495                ; <float> [#uses=1]
  %call7.i508 = call float @_Z10native_sinf(float %tmp6.i507) nounwind ; <float> [#uses=1]
  store float %call7.i508, float* %retval.i492
  %2 = load float* %retval.i492                   ; <float> [#uses=1]
  store float %2, float* %s3
  br label %do.body315

do.body315:                                       ; preds = %k_sincos.exit509
  %tmp318 = load float* %c3                       ; <float> [#uses=1]
  %tmp319 = insertelement <4 x float> undef, float %tmp318, i32 0 ; <<4 x float>> [#uses=2]
  %splat320 = shufflevector <4 x float> %tmp319, <4 x float> %tmp319, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp321 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul322 = fmul <4 x float> %splat320, %tmp321   ; <<4 x float>> [#uses=1]
  %tmp323 = load float* %s3                       ; <float> [#uses=1]
  %tmp324 = insertelement <4 x float> undef, float %tmp323, i32 0 ; <<4 x float>> [#uses=2]
  %splat325 = shufflevector <4 x float> %tmp324, <4 x float> %tmp324, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp326 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul327 = fmul <4 x float> %splat325, %tmp326   ; <<4 x float>> [#uses=1]
  %sub328 = fsub <4 x float> %mul322, %mul327     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub328, <4 x float>* %__r317
  %tmp329 = load float* %c3                       ; <float> [#uses=1]
  %tmp330 = insertelement <4 x float> undef, float %tmp329, i32 0 ; <<4 x float>> [#uses=2]
  %splat331 = shufflevector <4 x float> %tmp330, <4 x float> %tmp330, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp332 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul333 = fmul <4 x float> %splat331, %tmp332   ; <<4 x float>> [#uses=1]
  %tmp334 = load float* %s3                       ; <float> [#uses=1]
  %tmp335 = insertelement <4 x float> undef, float %tmp334, i32 0 ; <<4 x float>> [#uses=2]
  %splat336 = shufflevector <4 x float> %tmp335, <4 x float> %tmp335, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp337 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul338 = fmul <4 x float> %splat336, %tmp337   ; <<4 x float>> [#uses=1]
  %add339 = fadd <4 x float> %mul333, %mul338     ; <<4 x float>> [#uses=1]
  store <4 x float> %add339, <4 x float>* %zi3
  %tmp340 = load <4 x float>* %__r317             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp340, <4 x float>* %zr3
  br label %do.end341

do.end341:                                        ; preds = %do.body315
  br label %do.end342

do.end342:                                        ; preds = %do.end341
  call void @barrier(i32 1)
  %tmp343 = load float addrspace(3)** %lds.addr   ; <float addrspace(3)*> [#uses=1]
  %tmp344 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl345 = shl i32 %tmp344, 2                    ; <i32> [#uses=1]
  %tmp346 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shr347 = lshr i32 %tmp346, 3                   ; <i32> [#uses=1]
  %add348 = add i32 %shl345, %shr347              ; <i32> [#uses=1]
  %add.ptr349 = getelementptr inbounds float addrspace(3)* %tmp343, i32 %add348 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr349, float addrspace(3)** %lp
  %tmp350 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp351 = extractelement <4 x float> %tmp350, i32 0 ; <float> [#uses=1]
  %tmp352 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx353 = getelementptr inbounds float addrspace(3)* %tmp352, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp351, float addrspace(3)* %arrayidx353
  %tmp354 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp355 = extractelement <4 x float> %tmp354, i32 0 ; <float> [#uses=1]
  %tmp356 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx357 = getelementptr inbounds float addrspace(3)* %tmp356, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp355, float addrspace(3)* %arrayidx357
  %tmp358 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp359 = extractelement <4 x float> %tmp358, i32 0 ; <float> [#uses=1]
  %tmp360 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx361 = getelementptr inbounds float addrspace(3)* %tmp360, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp359, float addrspace(3)* %arrayidx361
  %tmp362 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp363 = extractelement <4 x float> %tmp362, i32 0 ; <float> [#uses=1]
  %tmp364 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx365 = getelementptr inbounds float addrspace(3)* %tmp364, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp363, float addrspace(3)* %arrayidx365
  %tmp366 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr367 = getelementptr inbounds float addrspace(3)* %tmp366, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr367, float addrspace(3)** %lp
  %tmp368 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp369 = extractelement <4 x float> %tmp368, i32 1 ; <float> [#uses=1]
  %tmp370 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx371 = getelementptr inbounds float addrspace(3)* %tmp370, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp369, float addrspace(3)* %arrayidx371
  %tmp372 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp373 = extractelement <4 x float> %tmp372, i32 1 ; <float> [#uses=1]
  %tmp374 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx375 = getelementptr inbounds float addrspace(3)* %tmp374, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp373, float addrspace(3)* %arrayidx375
  %tmp376 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp377 = extractelement <4 x float> %tmp376, i32 1 ; <float> [#uses=1]
  %tmp378 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx379 = getelementptr inbounds float addrspace(3)* %tmp378, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp377, float addrspace(3)* %arrayidx379
  %tmp380 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp381 = extractelement <4 x float> %tmp380, i32 1 ; <float> [#uses=1]
  %tmp382 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx383 = getelementptr inbounds float addrspace(3)* %tmp382, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp381, float addrspace(3)* %arrayidx383
  %tmp384 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr385 = getelementptr inbounds float addrspace(3)* %tmp384, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr385, float addrspace(3)** %lp
  %tmp386 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp387 = extractelement <4 x float> %tmp386, i32 2 ; <float> [#uses=1]
  %tmp388 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx389 = getelementptr inbounds float addrspace(3)* %tmp388, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp387, float addrspace(3)* %arrayidx389
  %tmp390 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp391 = extractelement <4 x float> %tmp390, i32 2 ; <float> [#uses=1]
  %tmp392 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx393 = getelementptr inbounds float addrspace(3)* %tmp392, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp391, float addrspace(3)* %arrayidx393
  %tmp394 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp395 = extractelement <4 x float> %tmp394, i32 2 ; <float> [#uses=1]
  %tmp396 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx397 = getelementptr inbounds float addrspace(3)* %tmp396, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp395, float addrspace(3)* %arrayidx397
  %tmp398 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp399 = extractelement <4 x float> %tmp398, i32 2 ; <float> [#uses=1]
  %tmp400 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx401 = getelementptr inbounds float addrspace(3)* %tmp400, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp399, float addrspace(3)* %arrayidx401
  %tmp402 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr403 = getelementptr inbounds float addrspace(3)* %tmp402, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr403, float addrspace(3)** %lp
  %tmp404 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp405 = extractelement <4 x float> %tmp404, i32 3 ; <float> [#uses=1]
  %tmp406 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx407 = getelementptr inbounds float addrspace(3)* %tmp406, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp405, float addrspace(3)* %arrayidx407
  %tmp408 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp409 = extractelement <4 x float> %tmp408, i32 3 ; <float> [#uses=1]
  %tmp410 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx411 = getelementptr inbounds float addrspace(3)* %tmp410, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp409, float addrspace(3)* %arrayidx411
  %tmp412 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp413 = extractelement <4 x float> %tmp412, i32 3 ; <float> [#uses=1]
  %tmp414 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx415 = getelementptr inbounds float addrspace(3)* %tmp414, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp413, float addrspace(3)* %arrayidx415
  %tmp416 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp417 = extractelement <4 x float> %tmp416, i32 3 ; <float> [#uses=1]
  %tmp418 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx419 = getelementptr inbounds float addrspace(3)* %tmp418, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp417, float addrspace(3)* %arrayidx419
  %tmp420 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr421 = getelementptr inbounds float addrspace(3)* %tmp420, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr421, float addrspace(3)** %lp
  %tmp422 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp423 = extractelement <4 x float> %tmp422, i32 0 ; <float> [#uses=1]
  %tmp424 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx425 = getelementptr inbounds float addrspace(3)* %tmp424, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp423, float addrspace(3)* %arrayidx425
  %tmp426 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp427 = extractelement <4 x float> %tmp426, i32 0 ; <float> [#uses=1]
  %tmp428 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx429 = getelementptr inbounds float addrspace(3)* %tmp428, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp427, float addrspace(3)* %arrayidx429
  %tmp430 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp431 = extractelement <4 x float> %tmp430, i32 0 ; <float> [#uses=1]
  %tmp432 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx433 = getelementptr inbounds float addrspace(3)* %tmp432, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp431, float addrspace(3)* %arrayidx433
  %tmp434 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp435 = extractelement <4 x float> %tmp434, i32 0 ; <float> [#uses=1]
  %tmp436 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx437 = getelementptr inbounds float addrspace(3)* %tmp436, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp435, float addrspace(3)* %arrayidx437
  %tmp438 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr439 = getelementptr inbounds float addrspace(3)* %tmp438, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr439, float addrspace(3)** %lp
  %tmp440 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp441 = extractelement <4 x float> %tmp440, i32 1 ; <float> [#uses=1]
  %tmp442 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx443 = getelementptr inbounds float addrspace(3)* %tmp442, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp441, float addrspace(3)* %arrayidx443
  %tmp444 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp445 = extractelement <4 x float> %tmp444, i32 1 ; <float> [#uses=1]
  %tmp446 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx447 = getelementptr inbounds float addrspace(3)* %tmp446, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp445, float addrspace(3)* %arrayidx447
  %tmp448 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp449 = extractelement <4 x float> %tmp448, i32 1 ; <float> [#uses=1]
  %tmp450 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx451 = getelementptr inbounds float addrspace(3)* %tmp450, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp449, float addrspace(3)* %arrayidx451
  %tmp452 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp453 = extractelement <4 x float> %tmp452, i32 1 ; <float> [#uses=1]
  %tmp454 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx455 = getelementptr inbounds float addrspace(3)* %tmp454, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp453, float addrspace(3)* %arrayidx455
  %tmp456 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr457 = getelementptr inbounds float addrspace(3)* %tmp456, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr457, float addrspace(3)** %lp
  %tmp458 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp459 = extractelement <4 x float> %tmp458, i32 2 ; <float> [#uses=1]
  %tmp460 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx461 = getelementptr inbounds float addrspace(3)* %tmp460, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp459, float addrspace(3)* %arrayidx461
  %tmp462 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp463 = extractelement <4 x float> %tmp462, i32 2 ; <float> [#uses=1]
  %tmp464 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx465 = getelementptr inbounds float addrspace(3)* %tmp464, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp463, float addrspace(3)* %arrayidx465
  %tmp466 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp467 = extractelement <4 x float> %tmp466, i32 2 ; <float> [#uses=1]
  %tmp468 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx469 = getelementptr inbounds float addrspace(3)* %tmp468, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp467, float addrspace(3)* %arrayidx469
  %tmp470 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp471 = extractelement <4 x float> %tmp470, i32 2 ; <float> [#uses=1]
  %tmp472 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx473 = getelementptr inbounds float addrspace(3)* %tmp472, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp471, float addrspace(3)* %arrayidx473
  %tmp474 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr475 = getelementptr inbounds float addrspace(3)* %tmp474, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr475, float addrspace(3)** %lp
  %tmp476 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp477 = extractelement <4 x float> %tmp476, i32 3 ; <float> [#uses=1]
  %tmp478 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx479 = getelementptr inbounds float addrspace(3)* %tmp478, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp477, float addrspace(3)* %arrayidx479
  %tmp480 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp481 = extractelement <4 x float> %tmp480, i32 3 ; <float> [#uses=1]
  %tmp482 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx483 = getelementptr inbounds float addrspace(3)* %tmp482, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp481, float addrspace(3)* %arrayidx483
  %tmp484 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp485 = extractelement <4 x float> %tmp484, i32 3 ; <float> [#uses=1]
  %tmp486 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx487 = getelementptr inbounds float addrspace(3)* %tmp486, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp485, float addrspace(3)* %arrayidx487
  %tmp488 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp489 = extractelement <4 x float> %tmp488, i32 3 ; <float> [#uses=1]
  %tmp490 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx491 = getelementptr inbounds float addrspace(3)* %tmp490, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp489, float addrspace(3)* %arrayidx491
  call void @barrier(i32 1)
  ret void
}

; CHECK: ret
define void @kfft_pass3(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
entry:
  %retval.i507 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i508 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i509 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i510 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i489 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i490 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i491 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i492 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i = alloca float, align 4               ; <float*> [#uses=2]
  %i.addr.i = alloca i32, align 4                 ; <i32*> [#uses=5]
  %cretp.addr.i = alloca float*, align 4          ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %me.addr = alloca i32, align 4                  ; <i32*> [#uses=5]
  %lds.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 4       ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r286 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r318 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %me.addr
  store float addrspace(3)* %lds, float addrspace(3)** %lds.addr
  %tmp = load float addrspace(3)** %lds.addr      ; <float addrspace(3)*> [#uses=1]
  %tmp1 = load i32* %me.addr                      ; <i32> [#uses=1]
  %tmp2 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shr = lshr i32 %tmp2, 5                        ; <i32> [#uses=1]
  %add = add i32 %tmp1, %shr                      ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(3)* %tmp, i32 %add ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr, float addrspace(3)** %lp
  %tmp7 = load float addrspace(3)** %lp           ; <float addrspace(3)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(3)* %tmp7, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8 = load float addrspace(3)* %arrayidx      ; <float> [#uses=1]
  %tmp9 = load <4 x float>* %zr0                  ; <<4 x float>> [#uses=1]
  %tmp10 = insertelement <4 x float> %tmp9, float %tmp8, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10, <4 x float>* %zr0
  %tmp11 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds float addrspace(3)* %tmp11, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp13 = load float addrspace(3)* %arrayidx12   ; <float> [#uses=1]
  %tmp14 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp15 = insertelement <4 x float> %tmp14, float %tmp13, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15, <4 x float>* %zr1
  %tmp16 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx17 = getelementptr inbounds float addrspace(3)* %tmp16, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp18 = load float addrspace(3)* %arrayidx17   ; <float> [#uses=1]
  %tmp19 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp20 = insertelement <4 x float> %tmp19, float %tmp18, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20, <4 x float>* %zr2
  %tmp21 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds float addrspace(3)* %tmp21, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp23 = load float addrspace(3)* %arrayidx22   ; <float> [#uses=1]
  %tmp24 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp25 = insertelement <4 x float> %tmp24, float %tmp23, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %zr3
  %tmp26 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr27 = getelementptr inbounds float addrspace(3)* %tmp26, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27, float addrspace(3)** %lp
  %tmp28 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(3)* %tmp28, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30 = load float addrspace(3)* %arrayidx29   ; <float> [#uses=1]
  %tmp31 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp32 = insertelement <4 x float> %tmp31, float %tmp30, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %zr0
  %tmp33 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(3)* %tmp33, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp35 = load float addrspace(3)* %arrayidx34   ; <float> [#uses=1]
  %tmp36 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp37 = insertelement <4 x float> %tmp36, float %tmp35, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37, <4 x float>* %zr1
  %tmp38 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds float addrspace(3)* %tmp38, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp40 = load float addrspace(3)* %arrayidx39   ; <float> [#uses=1]
  %tmp41 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp42 = insertelement <4 x float> %tmp41, float %tmp40, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %zr2
  %tmp43 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds float addrspace(3)* %tmp43, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp45 = load float addrspace(3)* %arrayidx44   ; <float> [#uses=1]
  %tmp46 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp47 = insertelement <4 x float> %tmp46, float %tmp45, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %zr3
  %tmp48 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr49 = getelementptr inbounds float addrspace(3)* %tmp48, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49, float addrspace(3)** %lp
  %tmp50 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds float addrspace(3)* %tmp50, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52 = load float addrspace(3)* %arrayidx51   ; <float> [#uses=1]
  %tmp53 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp54 = insertelement <4 x float> %tmp53, float %tmp52, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %zr0
  %tmp55 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(3)* %tmp55, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp57 = load float addrspace(3)* %arrayidx56   ; <float> [#uses=1]
  %tmp58 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp59 = insertelement <4 x float> %tmp58, float %tmp57, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59, <4 x float>* %zr1
  %tmp60 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx61 = getelementptr inbounds float addrspace(3)* %tmp60, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp62 = load float addrspace(3)* %arrayidx61   ; <float> [#uses=1]
  %tmp63 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp64 = insertelement <4 x float> %tmp63, float %tmp62, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64, <4 x float>* %zr2
  %tmp65 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float addrspace(3)* %tmp65, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp67 = load float addrspace(3)* %arrayidx66   ; <float> [#uses=1]
  %tmp68 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp69 = insertelement <4 x float> %tmp68, float %tmp67, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69, <4 x float>* %zr3
  %tmp70 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr71 = getelementptr inbounds float addrspace(3)* %tmp70, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71, float addrspace(3)** %lp
  %tmp72 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds float addrspace(3)* %tmp72, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74 = load float addrspace(3)* %arrayidx73   ; <float> [#uses=1]
  %tmp75 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp76 = insertelement <4 x float> %tmp75, float %tmp74, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76, <4 x float>* %zr0
  %tmp77 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(3)* %tmp77, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp79 = load float addrspace(3)* %arrayidx78   ; <float> [#uses=1]
  %tmp80 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp81 = insertelement <4 x float> %tmp80, float %tmp79, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81, <4 x float>* %zr1
  %tmp82 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds float addrspace(3)* %tmp82, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp84 = load float addrspace(3)* %arrayidx83   ; <float> [#uses=1]
  %tmp85 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp86 = insertelement <4 x float> %tmp85, float %tmp84, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86, <4 x float>* %zr2
  %tmp87 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds float addrspace(3)* %tmp87, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp89 = load float addrspace(3)* %arrayidx88   ; <float> [#uses=1]
  %tmp90 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp91 = insertelement <4 x float> %tmp90, float %tmp89, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91, <4 x float>* %zr3
  %tmp92 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr93 = getelementptr inbounds float addrspace(3)* %tmp92, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93, float addrspace(3)** %lp
  %tmp98 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds float addrspace(3)* %tmp98, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100 = load float addrspace(3)* %arrayidx99  ; <float> [#uses=1]
  %tmp101 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp102 = insertelement <4 x float> %tmp101, float %tmp100, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102, <4 x float>* %zi0
  %tmp103 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx104 = getelementptr inbounds float addrspace(3)* %tmp103, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp105 = load float addrspace(3)* %arrayidx104 ; <float> [#uses=1]
  %tmp106 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp107 = insertelement <4 x float> %tmp106, float %tmp105, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107, <4 x float>* %zi1
  %tmp108 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(3)* %tmp108, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp110 = load float addrspace(3)* %arrayidx109 ; <float> [#uses=1]
  %tmp111 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp112 = insertelement <4 x float> %tmp111, float %tmp110, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112, <4 x float>* %zi2
  %tmp113 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds float addrspace(3)* %tmp113, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp115 = load float addrspace(3)* %arrayidx114 ; <float> [#uses=1]
  %tmp116 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp117 = insertelement <4 x float> %tmp116, float %tmp115, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117, <4 x float>* %zi3
  %tmp118 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr119 = getelementptr inbounds float addrspace(3)* %tmp118, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119, float addrspace(3)** %lp
  %tmp120 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds float addrspace(3)* %tmp120, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122 = load float addrspace(3)* %arrayidx121 ; <float> [#uses=1]
  %tmp123 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp124 = insertelement <4 x float> %tmp123, float %tmp122, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124, <4 x float>* %zi0
  %tmp125 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds float addrspace(3)* %tmp125, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp127 = load float addrspace(3)* %arrayidx126 ; <float> [#uses=1]
  %tmp128 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp129 = insertelement <4 x float> %tmp128, float %tmp127, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129, <4 x float>* %zi1
  %tmp130 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds float addrspace(3)* %tmp130, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp132 = load float addrspace(3)* %arrayidx131 ; <float> [#uses=1]
  %tmp133 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp134 = insertelement <4 x float> %tmp133, float %tmp132, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134, <4 x float>* %zi2
  %tmp135 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx136 = getelementptr inbounds float addrspace(3)* %tmp135, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp137 = load float addrspace(3)* %arrayidx136 ; <float> [#uses=1]
  %tmp138 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp139 = insertelement <4 x float> %tmp138, float %tmp137, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139, <4 x float>* %zi3
  %tmp140 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr141 = getelementptr inbounds float addrspace(3)* %tmp140, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141, float addrspace(3)** %lp
  %tmp142 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx143 = getelementptr inbounds float addrspace(3)* %tmp142, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144 = load float addrspace(3)* %arrayidx143 ; <float> [#uses=1]
  %tmp145 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp146 = insertelement <4 x float> %tmp145, float %tmp144, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146, <4 x float>* %zi0
  %tmp147 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx148 = getelementptr inbounds float addrspace(3)* %tmp147, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp149 = load float addrspace(3)* %arrayidx148 ; <float> [#uses=1]
  %tmp150 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp151 = insertelement <4 x float> %tmp150, float %tmp149, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151, <4 x float>* %zi1
  %tmp152 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx153 = getelementptr inbounds float addrspace(3)* %tmp152, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp154 = load float addrspace(3)* %arrayidx153 ; <float> [#uses=1]
  %tmp155 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp156 = insertelement <4 x float> %tmp155, float %tmp154, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156, <4 x float>* %zi2
  %tmp157 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx158 = getelementptr inbounds float addrspace(3)* %tmp157, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp159 = load float addrspace(3)* %arrayidx158 ; <float> [#uses=1]
  %tmp160 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp161 = insertelement <4 x float> %tmp160, float %tmp159, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161, <4 x float>* %zi3
  %tmp162 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr163 = getelementptr inbounds float addrspace(3)* %tmp162, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163, float addrspace(3)** %lp
  %tmp164 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx165 = getelementptr inbounds float addrspace(3)* %tmp164, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166 = load float addrspace(3)* %arrayidx165 ; <float> [#uses=1]
  %tmp167 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp168 = insertelement <4 x float> %tmp167, float %tmp166, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168, <4 x float>* %zi0
  %tmp169 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx170 = getelementptr inbounds float addrspace(3)* %tmp169, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp171 = load float addrspace(3)* %arrayidx170 ; <float> [#uses=1]
  %tmp172 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp173 = insertelement <4 x float> %tmp172, float %tmp171, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173, <4 x float>* %zi1
  %tmp174 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(3)* %tmp174, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp176 = load float addrspace(3)* %arrayidx175 ; <float> [#uses=1]
  %tmp177 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp178 = insertelement <4 x float> %tmp177, float %tmp176, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178, <4 x float>* %zi2
  %tmp179 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx180 = getelementptr inbounds float addrspace(3)* %tmp179, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp181 = load float addrspace(3)* %arrayidx180 ; <float> [#uses=1]
  %tmp182 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp183 = insertelement <4 x float> %tmp182, float %tmp181, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183, <4 x float>* %zi3
  br label %do.body

do.body:                                          ; preds = %entry
  %tmp185 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp186 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %add187 = fadd <4 x float> %tmp185, %tmp186     ; <<4 x float>> [#uses=1]
  store <4 x float> %add187, <4 x float>* %ar0
  %tmp189 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp190 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %add191 = fadd <4 x float> %tmp189, %tmp190     ; <<4 x float>> [#uses=1]
  store <4 x float> %add191, <4 x float>* %ar2
  %tmp193 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp194 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %add195 = fadd <4 x float> %tmp193, %tmp194     ; <<4 x float>> [#uses=1]
  store <4 x float> %add195, <4 x float>* %br0
  %tmp197 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp198 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp197, %tmp198        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %br1
  %tmp200 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp201 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %sub202 = fsub <4 x float> %tmp200, %tmp201     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202, <4 x float>* %br2
  %tmp204 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp205 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %sub206 = fsub <4 x float> %tmp204, %tmp205     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206, <4 x float>* %br3
  %tmp208 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp209 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %add210 = fadd <4 x float> %tmp208, %tmp209     ; <<4 x float>> [#uses=1]
  store <4 x float> %add210, <4 x float>* %ai0
  %tmp212 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp213 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %add214 = fadd <4 x float> %tmp212, %tmp213     ; <<4 x float>> [#uses=1]
  store <4 x float> %add214, <4 x float>* %ai2
  %tmp216 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp217 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %add218 = fadd <4 x float> %tmp216, %tmp217     ; <<4 x float>> [#uses=1]
  store <4 x float> %add218, <4 x float>* %bi0
  %tmp220 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp221 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %sub222 = fsub <4 x float> %tmp220, %tmp221     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222, <4 x float>* %bi1
  %tmp224 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp225 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %sub226 = fsub <4 x float> %tmp224, %tmp225     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226, <4 x float>* %bi2
  %tmp228 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp229 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %sub230 = fsub <4 x float> %tmp228, %tmp229     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230, <4 x float>* %bi3
  %tmp231 = load <4 x float>* %br0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231, <4 x float>* %zr0
  %tmp232 = load <4 x float>* %bi0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232, <4 x float>* %zi0
  %tmp233 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp234 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %add235 = fadd <4 x float> %tmp233, %tmp234     ; <<4 x float>> [#uses=1]
  store <4 x float> %add235, <4 x float>* %zr1
  %tmp236 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %tmp237 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %sub238 = fsub <4 x float> %tmp236, %tmp237     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238, <4 x float>* %zi1
  %tmp239 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp240 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %sub241 = fsub <4 x float> %tmp239, %tmp240     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241, <4 x float>* %zr3
  %tmp242 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %tmp243 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %add244 = fadd <4 x float> %tmp242, %tmp243     ; <<4 x float>> [#uses=1]
  store <4 x float> %add244, <4 x float>* %zi3
  %tmp245 = load <4 x float>* %br2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245, <4 x float>* %zr2
  %tmp246 = load <4 x float>* %bi2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246, <4 x float>* %zi2
  br label %do.end

do.end:                                           ; preds = %do.body
  %tmp248 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shr249 = lshr i32 %tmp248, 2                   ; <i32> [#uses=1]
  %shl = shl i32 %shr249, 4                       ; <i32> [#uses=1]
  store i32 %shl, i32* %tbase
  br label %do.body250

do.body250:                                       ; preds = %do.end
  %tmp253 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul = mul i32 %tmp253, 1                       ; <i32> [#uses=1]
  store i32 %mul, i32* %i.addr.i
  store float* %c1, float** %cretp.addr.i
  %tmp.i = load i32* %i.addr.i                    ; <i32> [#uses=1]
  %cmp.i = icmp sgt i32 %tmp.i, 512               ; <i1> [#uses=1]
  br i1 %cmp.i, label %if.then.i, label %k_sincos.exit

if.then.i:                                        ; preds = %do.body250
  %tmp1.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %sub.i = sub i32 %tmp1.i, 1024                  ; <i32> [#uses=1]
  store i32 %sub.i, i32* %i.addr.i
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %do.body250, %if.then.i
  %tmp3.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %conv.i = sitofp i32 %tmp3.i to float           ; <float> [#uses=1]
  %mul.i = fmul float %conv.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i, float* %x.i
  %tmp4.i = load float* %x.i                      ; <float> [#uses=1]
  %call.i = call float @_Z10native_cosf(float %tmp4.i) nounwind ; <float> [#uses=1]
  %tmp5.i = load float** %cretp.addr.i            ; <float*> [#uses=1]
  store float %call.i, float* %tmp5.i
  %tmp6.i = load float* %x.i                      ; <float> [#uses=1]
  %call7.i = call float @_Z10native_sinf(float %tmp6.i) nounwind ; <float> [#uses=1]
  store float %call7.i, float* %retval.i
  %0 = load float* %retval.i                      ; <float> [#uses=1]
  store float %0, float* %s1
  br label %do.body254

do.body254:                                       ; preds = %k_sincos.exit
  %tmp256 = load float* %c1                       ; <float> [#uses=1]
  %tmp257 = insertelement <4 x float> undef, float %tmp256, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp257, <4 x float> %tmp257, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp258 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul259 = fmul <4 x float> %splat, %tmp258      ; <<4 x float>> [#uses=1]
  %tmp260 = load float* %s1                       ; <float> [#uses=1]
  %tmp261 = insertelement <4 x float> undef, float %tmp260, i32 0 ; <<4 x float>> [#uses=2]
  %splat262 = shufflevector <4 x float> %tmp261, <4 x float> %tmp261, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp263 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul264 = fmul <4 x float> %splat262, %tmp263   ; <<4 x float>> [#uses=1]
  %sub265 = fsub <4 x float> %mul259, %mul264     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub265, <4 x float>* %__r
  %tmp266 = load float* %c1                       ; <float> [#uses=1]
  %tmp267 = insertelement <4 x float> undef, float %tmp266, i32 0 ; <<4 x float>> [#uses=2]
  %splat268 = shufflevector <4 x float> %tmp267, <4 x float> %tmp267, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp269 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul270 = fmul <4 x float> %splat268, %tmp269   ; <<4 x float>> [#uses=1]
  %tmp271 = load float* %s1                       ; <float> [#uses=1]
  %tmp272 = insertelement <4 x float> undef, float %tmp271, i32 0 ; <<4 x float>> [#uses=2]
  %splat273 = shufflevector <4 x float> %tmp272, <4 x float> %tmp272, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp274 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul275 = fmul <4 x float> %splat273, %tmp274   ; <<4 x float>> [#uses=1]
  %add276 = fadd <4 x float> %mul270, %mul275     ; <<4 x float>> [#uses=1]
  store <4 x float> %add276, <4 x float>* %zi1
  %tmp277 = load <4 x float>* %__r                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp277, <4 x float>* %zr1
  br label %do.end278

do.end278:                                        ; preds = %do.body254
  %tmp281 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul282 = mul i32 %tmp281, 2                    ; <i32> [#uses=1]
  store i32 %mul282, i32* %i.addr.i508
  store float* %c2, float** %cretp.addr.i509
  %tmp.i511 = load i32* %i.addr.i508              ; <i32> [#uses=1]
  %cmp.i512 = icmp sgt i32 %tmp.i511, 512         ; <i1> [#uses=1]
  br i1 %cmp.i512, label %if.then.i515, label %k_sincos.exit524

if.then.i515:                                     ; preds = %do.end278
  %tmp1.i513 = load i32* %i.addr.i508             ; <i32> [#uses=1]
  %sub.i514 = sub i32 %tmp1.i513, 1024            ; <i32> [#uses=1]
  store i32 %sub.i514, i32* %i.addr.i508
  br label %k_sincos.exit524

k_sincos.exit524:                                 ; preds = %do.end278, %if.then.i515
  %tmp3.i516 = load i32* %i.addr.i508             ; <i32> [#uses=1]
  %conv.i517 = sitofp i32 %tmp3.i516 to float     ; <float> [#uses=1]
  %mul.i518 = fmul float %conv.i517, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i518, float* %x.i510
  %tmp4.i519 = load float* %x.i510                ; <float> [#uses=1]
  %call.i520 = call float @_Z10native_cosf(float %tmp4.i519) nounwind ; <float> [#uses=1]
  %tmp5.i521 = load float** %cretp.addr.i509      ; <float*> [#uses=1]
  store float %call.i520, float* %tmp5.i521
  %tmp6.i522 = load float* %x.i510                ; <float> [#uses=1]
  %call7.i523 = call float @_Z10native_sinf(float %tmp6.i522) nounwind ; <float> [#uses=1]
  store float %call7.i523, float* %retval.i507
  %1 = load float* %retval.i507                   ; <float> [#uses=1]
  store float %1, float* %s2
  br label %do.body284

do.body284:                                       ; preds = %k_sincos.exit524
  %tmp287 = load float* %c2                       ; <float> [#uses=1]
  %tmp288 = insertelement <4 x float> undef, float %tmp287, i32 0 ; <<4 x float>> [#uses=2]
  %splat289 = shufflevector <4 x float> %tmp288, <4 x float> %tmp288, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp290 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul291 = fmul <4 x float> %splat289, %tmp290   ; <<4 x float>> [#uses=1]
  %tmp292 = load float* %s2                       ; <float> [#uses=1]
  %tmp293 = insertelement <4 x float> undef, float %tmp292, i32 0 ; <<4 x float>> [#uses=2]
  %splat294 = shufflevector <4 x float> %tmp293, <4 x float> %tmp293, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp295 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul296 = fmul <4 x float> %splat294, %tmp295   ; <<4 x float>> [#uses=1]
  %sub297 = fsub <4 x float> %mul291, %mul296     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub297, <4 x float>* %__r286
  %tmp298 = load float* %c2                       ; <float> [#uses=1]
  %tmp299 = insertelement <4 x float> undef, float %tmp298, i32 0 ; <<4 x float>> [#uses=2]
  %splat300 = shufflevector <4 x float> %tmp299, <4 x float> %tmp299, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp301 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul302 = fmul <4 x float> %splat300, %tmp301   ; <<4 x float>> [#uses=1]
  %tmp303 = load float* %s2                       ; <float> [#uses=1]
  %tmp304 = insertelement <4 x float> undef, float %tmp303, i32 0 ; <<4 x float>> [#uses=2]
  %splat305 = shufflevector <4 x float> %tmp304, <4 x float> %tmp304, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp306 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul307 = fmul <4 x float> %splat305, %tmp306   ; <<4 x float>> [#uses=1]
  %add308 = fadd <4 x float> %mul302, %mul307     ; <<4 x float>> [#uses=1]
  store <4 x float> %add308, <4 x float>* %zi2
  %tmp309 = load <4 x float>* %__r286             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp309, <4 x float>* %zr2
  br label %do.end310

do.end310:                                        ; preds = %do.body284
  %tmp313 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul314 = mul i32 %tmp313, 3                    ; <i32> [#uses=1]
  store i32 %mul314, i32* %i.addr.i490
  store float* %c3, float** %cretp.addr.i491
  %tmp.i493 = load i32* %i.addr.i490              ; <i32> [#uses=1]
  %cmp.i494 = icmp sgt i32 %tmp.i493, 512         ; <i1> [#uses=1]
  br i1 %cmp.i494, label %if.then.i497, label %k_sincos.exit506

if.then.i497:                                     ; preds = %do.end310
  %tmp1.i495 = load i32* %i.addr.i490             ; <i32> [#uses=1]
  %sub.i496 = sub i32 %tmp1.i495, 1024            ; <i32> [#uses=1]
  store i32 %sub.i496, i32* %i.addr.i490
  br label %k_sincos.exit506

k_sincos.exit506:                                 ; preds = %do.end310, %if.then.i497
  %tmp3.i498 = load i32* %i.addr.i490             ; <i32> [#uses=1]
  %conv.i499 = sitofp i32 %tmp3.i498 to float     ; <float> [#uses=1]
  %mul.i500 = fmul float %conv.i499, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i500, float* %x.i492
  %tmp4.i501 = load float* %x.i492                ; <float> [#uses=1]
  %call.i502 = call float @_Z10native_cosf(float %tmp4.i501) nounwind ; <float> [#uses=1]
  %tmp5.i503 = load float** %cretp.addr.i491      ; <float*> [#uses=1]
  store float %call.i502, float* %tmp5.i503
  %tmp6.i504 = load float* %x.i492                ; <float> [#uses=1]
  %call7.i505 = call float @_Z10native_sinf(float %tmp6.i504) nounwind ; <float> [#uses=1]
  store float %call7.i505, float* %retval.i489
  %2 = load float* %retval.i489                   ; <float> [#uses=1]
  store float %2, float* %s3
  br label %do.body316

do.body316:                                       ; preds = %k_sincos.exit506
  %tmp319 = load float* %c3                       ; <float> [#uses=1]
  %tmp320 = insertelement <4 x float> undef, float %tmp319, i32 0 ; <<4 x float>> [#uses=2]
  %splat321 = shufflevector <4 x float> %tmp320, <4 x float> %tmp320, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp322 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul323 = fmul <4 x float> %splat321, %tmp322   ; <<4 x float>> [#uses=1]
  %tmp324 = load float* %s3                       ; <float> [#uses=1]
  %tmp325 = insertelement <4 x float> undef, float %tmp324, i32 0 ; <<4 x float>> [#uses=2]
  %splat326 = shufflevector <4 x float> %tmp325, <4 x float> %tmp325, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp327 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul328 = fmul <4 x float> %splat326, %tmp327   ; <<4 x float>> [#uses=1]
  %sub329 = fsub <4 x float> %mul323, %mul328     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub329, <4 x float>* %__r318
  %tmp330 = load float* %c3                       ; <float> [#uses=1]
  %tmp331 = insertelement <4 x float> undef, float %tmp330, i32 0 ; <<4 x float>> [#uses=2]
  %splat332 = shufflevector <4 x float> %tmp331, <4 x float> %tmp331, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp333 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul334 = fmul <4 x float> %splat332, %tmp333   ; <<4 x float>> [#uses=1]
  %tmp335 = load float* %s3                       ; <float> [#uses=1]
  %tmp336 = insertelement <4 x float> undef, float %tmp335, i32 0 ; <<4 x float>> [#uses=2]
  %splat337 = shufflevector <4 x float> %tmp336, <4 x float> %tmp336, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp338 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul339 = fmul <4 x float> %splat337, %tmp338   ; <<4 x float>> [#uses=1]
  %add340 = fadd <4 x float> %mul334, %mul339     ; <<4 x float>> [#uses=1]
  store <4 x float> %add340, <4 x float>* %zi3
  %tmp341 = load <4 x float>* %__r318             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp341, <4 x float>* %zr3
  br label %do.end342

do.end342:                                        ; preds = %do.body316
  br label %do.end343

do.end343:                                        ; preds = %do.end342
  call void @barrier(i32 1)
  %tmp344 = load float addrspace(3)** %lds.addr   ; <float addrspace(3)*> [#uses=1]
  %tmp345 = load i32* %me.addr                    ; <i32> [#uses=1]
  %add.ptr346 = getelementptr inbounds float addrspace(3)* %tmp344, i32 %tmp345 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr346, float addrspace(3)** %lp
  %tmp347 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp348 = extractelement <4 x float> %tmp347, i32 0 ; <float> [#uses=1]
  %tmp349 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx350 = getelementptr inbounds float addrspace(3)* %tmp349, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp348, float addrspace(3)* %arrayidx350
  %tmp351 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp352 = extractelement <4 x float> %tmp351, i32 1 ; <float> [#uses=1]
  %tmp353 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx354 = getelementptr inbounds float addrspace(3)* %tmp353, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp352, float addrspace(3)* %arrayidx354
  %tmp355 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp356 = extractelement <4 x float> %tmp355, i32 2 ; <float> [#uses=1]
  %tmp357 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx358 = getelementptr inbounds float addrspace(3)* %tmp357, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp356, float addrspace(3)* %arrayidx358
  %tmp359 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp360 = extractelement <4 x float> %tmp359, i32 3 ; <float> [#uses=1]
  %tmp361 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx362 = getelementptr inbounds float addrspace(3)* %tmp361, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp360, float addrspace(3)* %arrayidx362
  %tmp363 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr364 = getelementptr inbounds float addrspace(3)* %tmp363, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr364, float addrspace(3)** %lp
  %tmp365 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp366 = extractelement <4 x float> %tmp365, i32 0 ; <float> [#uses=1]
  %tmp367 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx368 = getelementptr inbounds float addrspace(3)* %tmp367, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp366, float addrspace(3)* %arrayidx368
  %tmp369 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp370 = extractelement <4 x float> %tmp369, i32 1 ; <float> [#uses=1]
  %tmp371 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx372 = getelementptr inbounds float addrspace(3)* %tmp371, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp370, float addrspace(3)* %arrayidx372
  %tmp373 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp374 = extractelement <4 x float> %tmp373, i32 2 ; <float> [#uses=1]
  %tmp375 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx376 = getelementptr inbounds float addrspace(3)* %tmp375, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp374, float addrspace(3)* %arrayidx376
  %tmp377 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp378 = extractelement <4 x float> %tmp377, i32 3 ; <float> [#uses=1]
  %tmp379 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx380 = getelementptr inbounds float addrspace(3)* %tmp379, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp378, float addrspace(3)* %arrayidx380
  %tmp381 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr382 = getelementptr inbounds float addrspace(3)* %tmp381, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr382, float addrspace(3)** %lp
  %tmp383 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp384 = extractelement <4 x float> %tmp383, i32 0 ; <float> [#uses=1]
  %tmp385 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx386 = getelementptr inbounds float addrspace(3)* %tmp385, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp384, float addrspace(3)* %arrayidx386
  %tmp387 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp388 = extractelement <4 x float> %tmp387, i32 1 ; <float> [#uses=1]
  %tmp389 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx390 = getelementptr inbounds float addrspace(3)* %tmp389, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp388, float addrspace(3)* %arrayidx390
  %tmp391 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp392 = extractelement <4 x float> %tmp391, i32 2 ; <float> [#uses=1]
  %tmp393 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx394 = getelementptr inbounds float addrspace(3)* %tmp393, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp392, float addrspace(3)* %arrayidx394
  %tmp395 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp396 = extractelement <4 x float> %tmp395, i32 3 ; <float> [#uses=1]
  %tmp397 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx398 = getelementptr inbounds float addrspace(3)* %tmp397, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp396, float addrspace(3)* %arrayidx398
  %tmp399 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr400 = getelementptr inbounds float addrspace(3)* %tmp399, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr400, float addrspace(3)** %lp
  %tmp401 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp402 = extractelement <4 x float> %tmp401, i32 0 ; <float> [#uses=1]
  %tmp403 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx404 = getelementptr inbounds float addrspace(3)* %tmp403, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp402, float addrspace(3)* %arrayidx404
  %tmp405 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp406 = extractelement <4 x float> %tmp405, i32 1 ; <float> [#uses=1]
  %tmp407 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx408 = getelementptr inbounds float addrspace(3)* %tmp407, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp406, float addrspace(3)* %arrayidx408
  %tmp409 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp410 = extractelement <4 x float> %tmp409, i32 2 ; <float> [#uses=1]
  %tmp411 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx412 = getelementptr inbounds float addrspace(3)* %tmp411, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp410, float addrspace(3)* %arrayidx412
  %tmp413 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp414 = extractelement <4 x float> %tmp413, i32 3 ; <float> [#uses=1]
  %tmp415 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx416 = getelementptr inbounds float addrspace(3)* %tmp415, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp414, float addrspace(3)* %arrayidx416
  %tmp417 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr418 = getelementptr inbounds float addrspace(3)* %tmp417, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr418, float addrspace(3)** %lp
  %tmp419 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp420 = extractelement <4 x float> %tmp419, i32 0 ; <float> [#uses=1]
  %tmp421 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx422 = getelementptr inbounds float addrspace(3)* %tmp421, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp420, float addrspace(3)* %arrayidx422
  %tmp423 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp424 = extractelement <4 x float> %tmp423, i32 1 ; <float> [#uses=1]
  %tmp425 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx426 = getelementptr inbounds float addrspace(3)* %tmp425, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp424, float addrspace(3)* %arrayidx426
  %tmp427 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp428 = extractelement <4 x float> %tmp427, i32 2 ; <float> [#uses=1]
  %tmp429 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx430 = getelementptr inbounds float addrspace(3)* %tmp429, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp428, float addrspace(3)* %arrayidx430
  %tmp431 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp432 = extractelement <4 x float> %tmp431, i32 3 ; <float> [#uses=1]
  %tmp433 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx434 = getelementptr inbounds float addrspace(3)* %tmp433, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp432, float addrspace(3)* %arrayidx434
  %tmp435 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr436 = getelementptr inbounds float addrspace(3)* %tmp435, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr436, float addrspace(3)** %lp
  %tmp437 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp438 = extractelement <4 x float> %tmp437, i32 0 ; <float> [#uses=1]
  %tmp439 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx440 = getelementptr inbounds float addrspace(3)* %tmp439, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp438, float addrspace(3)* %arrayidx440
  %tmp441 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp442 = extractelement <4 x float> %tmp441, i32 1 ; <float> [#uses=1]
  %tmp443 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx444 = getelementptr inbounds float addrspace(3)* %tmp443, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp442, float addrspace(3)* %arrayidx444
  %tmp445 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp446 = extractelement <4 x float> %tmp445, i32 2 ; <float> [#uses=1]
  %tmp447 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx448 = getelementptr inbounds float addrspace(3)* %tmp447, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp446, float addrspace(3)* %arrayidx448
  %tmp449 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp450 = extractelement <4 x float> %tmp449, i32 3 ; <float> [#uses=1]
  %tmp451 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx452 = getelementptr inbounds float addrspace(3)* %tmp451, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp450, float addrspace(3)* %arrayidx452
  %tmp453 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr454 = getelementptr inbounds float addrspace(3)* %tmp453, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr454, float addrspace(3)** %lp
  %tmp455 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp456 = extractelement <4 x float> %tmp455, i32 0 ; <float> [#uses=1]
  %tmp457 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx458 = getelementptr inbounds float addrspace(3)* %tmp457, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp456, float addrspace(3)* %arrayidx458
  %tmp459 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp460 = extractelement <4 x float> %tmp459, i32 1 ; <float> [#uses=1]
  %tmp461 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx462 = getelementptr inbounds float addrspace(3)* %tmp461, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp460, float addrspace(3)* %arrayidx462
  %tmp463 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp464 = extractelement <4 x float> %tmp463, i32 2 ; <float> [#uses=1]
  %tmp465 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx466 = getelementptr inbounds float addrspace(3)* %tmp465, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp464, float addrspace(3)* %arrayidx466
  %tmp467 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp468 = extractelement <4 x float> %tmp467, i32 3 ; <float> [#uses=1]
  %tmp469 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx470 = getelementptr inbounds float addrspace(3)* %tmp469, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp468, float addrspace(3)* %arrayidx470
  %tmp471 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr472 = getelementptr inbounds float addrspace(3)* %tmp471, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr472, float addrspace(3)** %lp
  %tmp473 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp474 = extractelement <4 x float> %tmp473, i32 0 ; <float> [#uses=1]
  %tmp475 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx476 = getelementptr inbounds float addrspace(3)* %tmp475, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp474, float addrspace(3)* %arrayidx476
  %tmp477 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp478 = extractelement <4 x float> %tmp477, i32 1 ; <float> [#uses=1]
  %tmp479 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx480 = getelementptr inbounds float addrspace(3)* %tmp479, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp478, float addrspace(3)* %arrayidx480
  %tmp481 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp482 = extractelement <4 x float> %tmp481, i32 2 ; <float> [#uses=1]
  %tmp483 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx484 = getelementptr inbounds float addrspace(3)* %tmp483, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp482, float addrspace(3)* %arrayidx484
  %tmp485 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp486 = extractelement <4 x float> %tmp485, i32 3 ; <float> [#uses=1]
  %tmp487 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx488 = getelementptr inbounds float addrspace(3)* %tmp487, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp486, float addrspace(3)* %arrayidx488
  call void @barrier(i32 1)
  ret void
}

; CHECK: ret
define void @kfft_pass4(i32 %me, float addrspace(3)* %lds) nounwind alwaysinline {
entry:
  %retval.i513 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i514 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i515 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i516 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i495 = alloca float, align 4            ; <float*> [#uses=2]
  %i.addr.i496 = alloca i32, align 4              ; <i32*> [#uses=5]
  %cretp.addr.i497 = alloca float*, align 4       ; <float**> [#uses=2]
  %x.i498 = alloca float, align 4                 ; <float*> [#uses=3]
  %retval.i = alloca float, align 4               ; <float*> [#uses=2]
  %i.addr.i = alloca i32, align 4                 ; <i32*> [#uses=5]
  %cretp.addr.i = alloca float*, align 4          ; <float**> [#uses=2]
  %x.i = alloca float, align 4                    ; <float*> [#uses=3]
  %me.addr = alloca i32, align 4                  ; <i32*> [#uses=6]
  %lds.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp = alloca float addrspace(3)*, align 4       ; <float addrspace(3)**> [#uses=94]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=15]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %tbase = alloca i32, align 4                    ; <i32*> [#uses=4]
  %c1 = alloca float, align 4                     ; <float*> [#uses=3]
  %s1 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=3]
  %s2 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r292 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=3]
  %s3 = alloca float, align 4                     ; <float*> [#uses=3]
  %__r324 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store i32 %me, i32* %me.addr
  store float addrspace(3)* %lds, float addrspace(3)** %lds.addr
  %tmp = load float addrspace(3)** %lds.addr      ; <float addrspace(3)*> [#uses=1]
  %tmp1 = load i32* %me.addr                      ; <i32> [#uses=1]
  %and = and i32 %tmp1, 3                         ; <i32> [#uses=1]
  %tmp2 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shr = lshr i32 %tmp2, 2                        ; <i32> [#uses=1]
  %and3 = and i32 %shr, 3                         ; <i32> [#uses=1]
  %mul = mul i32 %and3, 264                       ; <i32> [#uses=1]
  %add = add i32 %and, %mul                       ; <i32> [#uses=1]
  %tmp4 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shr5 = lshr i32 %tmp4, 4                       ; <i32> [#uses=1]
  %shl = shl i32 %shr5, 2                         ; <i32> [#uses=1]
  %add6 = add i32 %add, %shl                      ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(3)* %tmp, i32 %add6 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr, float addrspace(3)** %lp
  %tmp11 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(3)* %tmp11, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp12 = load float addrspace(3)* %arrayidx     ; <float> [#uses=1]
  %tmp13 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp14 = insertelement <4 x float> %tmp13, float %tmp12, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp14, <4 x float>* %zr0
  %tmp15 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx16 = getelementptr inbounds float addrspace(3)* %tmp15, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp17 = load float addrspace(3)* %arrayidx16   ; <float> [#uses=1]
  %tmp18 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp19 = insertelement <4 x float> %tmp18, float %tmp17, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp19, <4 x float>* %zr0
  %tmp20 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx21 = getelementptr inbounds float addrspace(3)* %tmp20, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp22 = load float addrspace(3)* %arrayidx21   ; <float> [#uses=1]
  %tmp23 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp24 = insertelement <4 x float> %tmp23, float %tmp22, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp24, <4 x float>* %zr0
  %tmp25 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx26 = getelementptr inbounds float addrspace(3)* %tmp25, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp27 = load float addrspace(3)* %arrayidx26   ; <float> [#uses=1]
  %tmp28 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp29 = insertelement <4 x float> %tmp28, float %tmp27, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp29, <4 x float>* %zr0
  %tmp30 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr31 = getelementptr inbounds float addrspace(3)* %tmp30, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr31, float addrspace(3)** %lp
  %tmp32 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx33 = getelementptr inbounds float addrspace(3)* %tmp32, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp34 = load float addrspace(3)* %arrayidx33   ; <float> [#uses=1]
  %tmp35 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp36 = insertelement <4 x float> %tmp35, float %tmp34, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36, <4 x float>* %zr1
  %tmp37 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx38 = getelementptr inbounds float addrspace(3)* %tmp37, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp39 = load float addrspace(3)* %arrayidx38   ; <float> [#uses=1]
  %tmp40 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp41 = insertelement <4 x float> %tmp40, float %tmp39, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %zr1
  %tmp42 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx43 = getelementptr inbounds float addrspace(3)* %tmp42, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp44 = load float addrspace(3)* %arrayidx43   ; <float> [#uses=1]
  %tmp45 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp46 = insertelement <4 x float> %tmp45, float %tmp44, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46, <4 x float>* %zr1
  %tmp47 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx48 = getelementptr inbounds float addrspace(3)* %tmp47, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp49 = load float addrspace(3)* %arrayidx48   ; <float> [#uses=1]
  %tmp50 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp51 = insertelement <4 x float> %tmp50, float %tmp49, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp51, <4 x float>* %zr1
  %tmp52 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr53 = getelementptr inbounds float addrspace(3)* %tmp52, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr53, float addrspace(3)** %lp
  %tmp54 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds float addrspace(3)* %tmp54, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp56 = load float addrspace(3)* %arrayidx55   ; <float> [#uses=1]
  %tmp57 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp58 = insertelement <4 x float> %tmp57, float %tmp56, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp58, <4 x float>* %zr2
  %tmp59 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds float addrspace(3)* %tmp59, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp61 = load float addrspace(3)* %arrayidx60   ; <float> [#uses=1]
  %tmp62 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp63 = insertelement <4 x float> %tmp62, float %tmp61, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp63, <4 x float>* %zr2
  %tmp64 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds float addrspace(3)* %tmp64, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp66 = load float addrspace(3)* %arrayidx65   ; <float> [#uses=1]
  %tmp67 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp68 = insertelement <4 x float> %tmp67, float %tmp66, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68, <4 x float>* %zr2
  %tmp69 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds float addrspace(3)* %tmp69, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp71 = load float addrspace(3)* %arrayidx70   ; <float> [#uses=1]
  %tmp72 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp73 = insertelement <4 x float> %tmp72, float %tmp71, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp73, <4 x float>* %zr2
  %tmp74 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr75 = getelementptr inbounds float addrspace(3)* %tmp74, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr75, float addrspace(3)** %lp
  %tmp76 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(3)* %tmp76, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp78 = load float addrspace(3)* %arrayidx77   ; <float> [#uses=1]
  %tmp79 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp80 = insertelement <4 x float> %tmp79, float %tmp78, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp80, <4 x float>* %zr3
  %tmp81 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx82 = getelementptr inbounds float addrspace(3)* %tmp81, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp83 = load float addrspace(3)* %arrayidx82   ; <float> [#uses=1]
  %tmp84 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp85 = insertelement <4 x float> %tmp84, float %tmp83, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp85, <4 x float>* %zr3
  %tmp86 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx87 = getelementptr inbounds float addrspace(3)* %tmp86, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp88 = load float addrspace(3)* %arrayidx87   ; <float> [#uses=1]
  %tmp89 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp90 = insertelement <4 x float> %tmp89, float %tmp88, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp90, <4 x float>* %zr3
  %tmp91 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx92 = getelementptr inbounds float addrspace(3)* %tmp91, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp93 = load float addrspace(3)* %arrayidx92   ; <float> [#uses=1]
  %tmp94 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp95 = insertelement <4 x float> %tmp94, float %tmp93, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp95, <4 x float>* %zr3
  %tmp96 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr97 = getelementptr inbounds float addrspace(3)* %tmp96, i32 1008 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr97, float addrspace(3)** %lp
  %tmp102 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds float addrspace(3)* %tmp102, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp104 = load float addrspace(3)* %arrayidx103 ; <float> [#uses=1]
  %tmp105 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp106 = insertelement <4 x float> %tmp105, float %tmp104, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp106, <4 x float>* %zi0
  %tmp107 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds float addrspace(3)* %tmp107, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp109 = load float addrspace(3)* %arrayidx108 ; <float> [#uses=1]
  %tmp110 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp111 = insertelement <4 x float> %tmp110, float %tmp109, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp111, <4 x float>* %zi0
  %tmp112 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx113 = getelementptr inbounds float addrspace(3)* %tmp112, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp114 = load float addrspace(3)* %arrayidx113 ; <float> [#uses=1]
  %tmp115 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp116 = insertelement <4 x float> %tmp115, float %tmp114, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp116, <4 x float>* %zi0
  %tmp117 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx118 = getelementptr inbounds float addrspace(3)* %tmp117, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp119 = load float addrspace(3)* %arrayidx118 ; <float> [#uses=1]
  %tmp120 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp121 = insertelement <4 x float> %tmp120, float %tmp119, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp121, <4 x float>* %zi0
  %tmp122 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr123 = getelementptr inbounds float addrspace(3)* %tmp122, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr123, float addrspace(3)** %lp
  %tmp124 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx125 = getelementptr inbounds float addrspace(3)* %tmp124, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp126 = load float addrspace(3)* %arrayidx125 ; <float> [#uses=1]
  %tmp127 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp128 = insertelement <4 x float> %tmp127, float %tmp126, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp128, <4 x float>* %zi1
  %tmp129 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds float addrspace(3)* %tmp129, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp131 = load float addrspace(3)* %arrayidx130 ; <float> [#uses=1]
  %tmp132 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp133 = insertelement <4 x float> %tmp132, float %tmp131, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp133, <4 x float>* %zi1
  %tmp134 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx135 = getelementptr inbounds float addrspace(3)* %tmp134, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp136 = load float addrspace(3)* %arrayidx135 ; <float> [#uses=1]
  %tmp137 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp138 = insertelement <4 x float> %tmp137, float %tmp136, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp138, <4 x float>* %zi1
  %tmp139 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx140 = getelementptr inbounds float addrspace(3)* %tmp139, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp141 = load float addrspace(3)* %arrayidx140 ; <float> [#uses=1]
  %tmp142 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp143 = insertelement <4 x float> %tmp142, float %tmp141, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp143, <4 x float>* %zi1
  %tmp144 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr145 = getelementptr inbounds float addrspace(3)* %tmp144, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr145, float addrspace(3)** %lp
  %tmp146 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx147 = getelementptr inbounds float addrspace(3)* %tmp146, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp148 = load float addrspace(3)* %arrayidx147 ; <float> [#uses=1]
  %tmp149 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp150 = insertelement <4 x float> %tmp149, float %tmp148, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp150, <4 x float>* %zi2
  %tmp151 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx152 = getelementptr inbounds float addrspace(3)* %tmp151, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp153 = load float addrspace(3)* %arrayidx152 ; <float> [#uses=1]
  %tmp154 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp155 = insertelement <4 x float> %tmp154, float %tmp153, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp155, <4 x float>* %zi2
  %tmp156 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx157 = getelementptr inbounds float addrspace(3)* %tmp156, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp158 = load float addrspace(3)* %arrayidx157 ; <float> [#uses=1]
  %tmp159 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp160 = insertelement <4 x float> %tmp159, float %tmp158, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp160, <4 x float>* %zi2
  %tmp161 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx162 = getelementptr inbounds float addrspace(3)* %tmp161, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp163 = load float addrspace(3)* %arrayidx162 ; <float> [#uses=1]
  %tmp164 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp165 = insertelement <4 x float> %tmp164, float %tmp163, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp165, <4 x float>* %zi2
  %tmp166 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr167 = getelementptr inbounds float addrspace(3)* %tmp166, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr167, float addrspace(3)** %lp
  %tmp168 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx169 = getelementptr inbounds float addrspace(3)* %tmp168, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp170 = load float addrspace(3)* %arrayidx169 ; <float> [#uses=1]
  %tmp171 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp172 = insertelement <4 x float> %tmp171, float %tmp170, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp172, <4 x float>* %zi3
  %tmp173 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx174 = getelementptr inbounds float addrspace(3)* %tmp173, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp175 = load float addrspace(3)* %arrayidx174 ; <float> [#uses=1]
  %tmp176 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp177 = insertelement <4 x float> %tmp176, float %tmp175, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp177, <4 x float>* %zi3
  %tmp178 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx179 = getelementptr inbounds float addrspace(3)* %tmp178, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp180 = load float addrspace(3)* %arrayidx179 ; <float> [#uses=1]
  %tmp181 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp182 = insertelement <4 x float> %tmp181, float %tmp180, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp182, <4 x float>* %zi3
  %tmp183 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx184 = getelementptr inbounds float addrspace(3)* %tmp183, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp185 = load float addrspace(3)* %arrayidx184 ; <float> [#uses=1]
  %tmp186 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp187 = insertelement <4 x float> %tmp186, float %tmp185, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp187, <4 x float>* %zi3
  br label %do.body

do.body:                                          ; preds = %entry
  %tmp189 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp190 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %add191 = fadd <4 x float> %tmp189, %tmp190     ; <<4 x float>> [#uses=1]
  store <4 x float> %add191, <4 x float>* %ar0
  %tmp193 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp194 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %add195 = fadd <4 x float> %tmp193, %tmp194     ; <<4 x float>> [#uses=1]
  store <4 x float> %add195, <4 x float>* %ar2
  %tmp197 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp198 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %add199 = fadd <4 x float> %tmp197, %tmp198     ; <<4 x float>> [#uses=1]
  store <4 x float> %add199, <4 x float>* %br0
  %tmp201 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp202 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp201, %tmp202        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %br1
  %tmp204 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp205 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %sub206 = fsub <4 x float> %tmp204, %tmp205     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206, <4 x float>* %br2
  %tmp208 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp209 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %sub210 = fsub <4 x float> %tmp208, %tmp209     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub210, <4 x float>* %br3
  %tmp212 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp213 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %add214 = fadd <4 x float> %tmp212, %tmp213     ; <<4 x float>> [#uses=1]
  store <4 x float> %add214, <4 x float>* %ai0
  %tmp216 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp217 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %add218 = fadd <4 x float> %tmp216, %tmp217     ; <<4 x float>> [#uses=1]
  store <4 x float> %add218, <4 x float>* %ai2
  %tmp220 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp221 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %add222 = fadd <4 x float> %tmp220, %tmp221     ; <<4 x float>> [#uses=1]
  store <4 x float> %add222, <4 x float>* %bi0
  %tmp224 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp225 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %sub226 = fsub <4 x float> %tmp224, %tmp225     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226, <4 x float>* %bi1
  %tmp228 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp229 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %sub230 = fsub <4 x float> %tmp228, %tmp229     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230, <4 x float>* %bi2
  %tmp232 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp233 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %sub234 = fsub <4 x float> %tmp232, %tmp233     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub234, <4 x float>* %bi3
  %tmp235 = load <4 x float>* %br0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp235, <4 x float>* %zr0
  %tmp236 = load <4 x float>* %bi0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp236, <4 x float>* %zi0
  %tmp237 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp238 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %add239 = fadd <4 x float> %tmp237, %tmp238     ; <<4 x float>> [#uses=1]
  store <4 x float> %add239, <4 x float>* %zr1
  %tmp240 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %tmp241 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %sub242 = fsub <4 x float> %tmp240, %tmp241     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub242, <4 x float>* %zi1
  %tmp243 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp244 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %sub245 = fsub <4 x float> %tmp243, %tmp244     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub245, <4 x float>* %zr3
  %tmp246 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %tmp247 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %add248 = fadd <4 x float> %tmp246, %tmp247     ; <<4 x float>> [#uses=1]
  store <4 x float> %add248, <4 x float>* %zi3
  %tmp249 = load <4 x float>* %br2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp249, <4 x float>* %zr2
  %tmp250 = load <4 x float>* %bi2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp250, <4 x float>* %zi2
  br label %do.end

do.end:                                           ; preds = %do.body
  %tmp252 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shr253 = lshr i32 %tmp252, 4                   ; <i32> [#uses=1]
  %shl254 = shl i32 %shr253, 6                    ; <i32> [#uses=1]
  store i32 %shl254, i32* %tbase
  br label %do.body255

do.body255:                                       ; preds = %do.end
  %tmp258 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul259 = mul i32 %tmp258, 1                    ; <i32> [#uses=1]
  store i32 %mul259, i32* %i.addr.i
  store float* %c1, float** %cretp.addr.i
  %tmp.i = load i32* %i.addr.i                    ; <i32> [#uses=1]
  %cmp.i = icmp sgt i32 %tmp.i, 512               ; <i1> [#uses=1]
  br i1 %cmp.i, label %if.then.i, label %k_sincos.exit

if.then.i:                                        ; preds = %do.body255
  %tmp1.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %sub.i = sub i32 %tmp1.i, 1024                  ; <i32> [#uses=1]
  store i32 %sub.i, i32* %i.addr.i
  br label %k_sincos.exit

k_sincos.exit:                                    ; preds = %do.body255, %if.then.i
  %tmp3.i = load i32* %i.addr.i                   ; <i32> [#uses=1]
  %conv.i = sitofp i32 %tmp3.i to float           ; <float> [#uses=1]
  %mul.i = fmul float %conv.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i, float* %x.i
  %tmp4.i = load float* %x.i                      ; <float> [#uses=1]
  %call.i = call float @_Z10native_cosf(float %tmp4.i) nounwind ; <float> [#uses=1]
  %tmp5.i = load float** %cretp.addr.i            ; <float*> [#uses=1]
  store float %call.i, float* %tmp5.i
  %tmp6.i = load float* %x.i                      ; <float> [#uses=1]
  %call7.i = call float @_Z10native_sinf(float %tmp6.i) nounwind ; <float> [#uses=1]
  store float %call7.i, float* %retval.i
  %0 = load float* %retval.i                      ; <float> [#uses=1]
  store float %0, float* %s1
  br label %do.body260

do.body260:                                       ; preds = %k_sincos.exit
  %tmp262 = load float* %c1                       ; <float> [#uses=1]
  %tmp263 = insertelement <4 x float> undef, float %tmp262, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp263, <4 x float> %tmp263, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp264 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul265 = fmul <4 x float> %splat, %tmp264      ; <<4 x float>> [#uses=1]
  %tmp266 = load float* %s1                       ; <float> [#uses=1]
  %tmp267 = insertelement <4 x float> undef, float %tmp266, i32 0 ; <<4 x float>> [#uses=2]
  %splat268 = shufflevector <4 x float> %tmp267, <4 x float> %tmp267, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp269 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul270 = fmul <4 x float> %splat268, %tmp269   ; <<4 x float>> [#uses=1]
  %sub271 = fsub <4 x float> %mul265, %mul270     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub271, <4 x float>* %__r
  %tmp272 = load float* %c1                       ; <float> [#uses=1]
  %tmp273 = insertelement <4 x float> undef, float %tmp272, i32 0 ; <<4 x float>> [#uses=2]
  %splat274 = shufflevector <4 x float> %tmp273, <4 x float> %tmp273, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp275 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %mul276 = fmul <4 x float> %splat274, %tmp275   ; <<4 x float>> [#uses=1]
  %tmp277 = load float* %s1                       ; <float> [#uses=1]
  %tmp278 = insertelement <4 x float> undef, float %tmp277, i32 0 ; <<4 x float>> [#uses=2]
  %splat279 = shufflevector <4 x float> %tmp278, <4 x float> %tmp278, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp280 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %mul281 = fmul <4 x float> %splat279, %tmp280   ; <<4 x float>> [#uses=1]
  %add282 = fadd <4 x float> %mul276, %mul281     ; <<4 x float>> [#uses=1]
  store <4 x float> %add282, <4 x float>* %zi1
  %tmp283 = load <4 x float>* %__r                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp283, <4 x float>* %zr1
  br label %do.end284

do.end284:                                        ; preds = %do.body260
  %tmp287 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul288 = mul i32 %tmp287, 2                    ; <i32> [#uses=1]
  store i32 %mul288, i32* %i.addr.i514
  store float* %c2, float** %cretp.addr.i515
  %tmp.i517 = load i32* %i.addr.i514              ; <i32> [#uses=1]
  %cmp.i518 = icmp sgt i32 %tmp.i517, 512         ; <i1> [#uses=1]
  br i1 %cmp.i518, label %if.then.i521, label %k_sincos.exit530

if.then.i521:                                     ; preds = %do.end284
  %tmp1.i519 = load i32* %i.addr.i514             ; <i32> [#uses=1]
  %sub.i520 = sub i32 %tmp1.i519, 1024            ; <i32> [#uses=1]
  store i32 %sub.i520, i32* %i.addr.i514
  br label %k_sincos.exit530

k_sincos.exit530:                                 ; preds = %do.end284, %if.then.i521
  %tmp3.i522 = load i32* %i.addr.i514             ; <i32> [#uses=1]
  %conv.i523 = sitofp i32 %tmp3.i522 to float     ; <float> [#uses=1]
  %mul.i524 = fmul float %conv.i523, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i524, float* %x.i516
  %tmp4.i525 = load float* %x.i516                ; <float> [#uses=1]
  %call.i526 = call float @_Z10native_cosf(float %tmp4.i525) nounwind ; <float> [#uses=1]
  %tmp5.i527 = load float** %cretp.addr.i515      ; <float*> [#uses=1]
  store float %call.i526, float* %tmp5.i527
  %tmp6.i528 = load float* %x.i516                ; <float> [#uses=1]
  %call7.i529 = call float @_Z10native_sinf(float %tmp6.i528) nounwind ; <float> [#uses=1]
  store float %call7.i529, float* %retval.i513
  %1 = load float* %retval.i513                   ; <float> [#uses=1]
  store float %1, float* %s2
  br label %do.body290

do.body290:                                       ; preds = %k_sincos.exit530
  %tmp293 = load float* %c2                       ; <float> [#uses=1]
  %tmp294 = insertelement <4 x float> undef, float %tmp293, i32 0 ; <<4 x float>> [#uses=2]
  %splat295 = shufflevector <4 x float> %tmp294, <4 x float> %tmp294, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp296 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul297 = fmul <4 x float> %splat295, %tmp296   ; <<4 x float>> [#uses=1]
  %tmp298 = load float* %s2                       ; <float> [#uses=1]
  %tmp299 = insertelement <4 x float> undef, float %tmp298, i32 0 ; <<4 x float>> [#uses=2]
  %splat300 = shufflevector <4 x float> %tmp299, <4 x float> %tmp299, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp301 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul302 = fmul <4 x float> %splat300, %tmp301   ; <<4 x float>> [#uses=1]
  %sub303 = fsub <4 x float> %mul297, %mul302     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub303, <4 x float>* %__r292
  %tmp304 = load float* %c2                       ; <float> [#uses=1]
  %tmp305 = insertelement <4 x float> undef, float %tmp304, i32 0 ; <<4 x float>> [#uses=2]
  %splat306 = shufflevector <4 x float> %tmp305, <4 x float> %tmp305, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp307 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %mul308 = fmul <4 x float> %splat306, %tmp307   ; <<4 x float>> [#uses=1]
  %tmp309 = load float* %s2                       ; <float> [#uses=1]
  %tmp310 = insertelement <4 x float> undef, float %tmp309, i32 0 ; <<4 x float>> [#uses=2]
  %splat311 = shufflevector <4 x float> %tmp310, <4 x float> %tmp310, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp312 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %mul313 = fmul <4 x float> %splat311, %tmp312   ; <<4 x float>> [#uses=1]
  %add314 = fadd <4 x float> %mul308, %mul313     ; <<4 x float>> [#uses=1]
  store <4 x float> %add314, <4 x float>* %zi2
  %tmp315 = load <4 x float>* %__r292             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp315, <4 x float>* %zr2
  br label %do.end316

do.end316:                                        ; preds = %do.body290
  %tmp319 = load i32* %tbase                      ; <i32> [#uses=1]
  %mul320 = mul i32 %tmp319, 3                    ; <i32> [#uses=1]
  store i32 %mul320, i32* %i.addr.i496
  store float* %c3, float** %cretp.addr.i497
  %tmp.i499 = load i32* %i.addr.i496              ; <i32> [#uses=1]
  %cmp.i500 = icmp sgt i32 %tmp.i499, 512         ; <i1> [#uses=1]
  br i1 %cmp.i500, label %if.then.i503, label %k_sincos.exit512

if.then.i503:                                     ; preds = %do.end316
  %tmp1.i501 = load i32* %i.addr.i496             ; <i32> [#uses=1]
  %sub.i502 = sub i32 %tmp1.i501, 1024            ; <i32> [#uses=1]
  store i32 %sub.i502, i32* %i.addr.i496
  br label %k_sincos.exit512

k_sincos.exit512:                                 ; preds = %do.end316, %if.then.i503
  %tmp3.i504 = load i32* %i.addr.i496             ; <i32> [#uses=1]
  %conv.i505 = sitofp i32 %tmp3.i504 to float     ; <float> [#uses=1]
  %mul.i506 = fmul float %conv.i505, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i506, float* %x.i498
  %tmp4.i507 = load float* %x.i498                ; <float> [#uses=1]
  %call.i508 = call float @_Z10native_cosf(float %tmp4.i507) nounwind ; <float> [#uses=1]
  %tmp5.i509 = load float** %cretp.addr.i497      ; <float*> [#uses=1]
  store float %call.i508, float* %tmp5.i509
  %tmp6.i510 = load float* %x.i498                ; <float> [#uses=1]
  %call7.i511 = call float @_Z10native_sinf(float %tmp6.i510) nounwind ; <float> [#uses=1]
  store float %call7.i511, float* %retval.i495
  %2 = load float* %retval.i495                   ; <float> [#uses=1]
  store float %2, float* %s3
  br label %do.body322

do.body322:                                       ; preds = %k_sincos.exit512
  %tmp325 = load float* %c3                       ; <float> [#uses=1]
  %tmp326 = insertelement <4 x float> undef, float %tmp325, i32 0 ; <<4 x float>> [#uses=2]
  %splat327 = shufflevector <4 x float> %tmp326, <4 x float> %tmp326, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp328 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul329 = fmul <4 x float> %splat327, %tmp328   ; <<4 x float>> [#uses=1]
  %tmp330 = load float* %s3                       ; <float> [#uses=1]
  %tmp331 = insertelement <4 x float> undef, float %tmp330, i32 0 ; <<4 x float>> [#uses=2]
  %splat332 = shufflevector <4 x float> %tmp331, <4 x float> %tmp331, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp333 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul334 = fmul <4 x float> %splat332, %tmp333   ; <<4 x float>> [#uses=1]
  %sub335 = fsub <4 x float> %mul329, %mul334     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub335, <4 x float>* %__r324
  %tmp336 = load float* %c3                       ; <float> [#uses=1]
  %tmp337 = insertelement <4 x float> undef, float %tmp336, i32 0 ; <<4 x float>> [#uses=2]
  %splat338 = shufflevector <4 x float> %tmp337, <4 x float> %tmp337, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp339 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %mul340 = fmul <4 x float> %splat338, %tmp339   ; <<4 x float>> [#uses=1]
  %tmp341 = load float* %s3                       ; <float> [#uses=1]
  %tmp342 = insertelement <4 x float> undef, float %tmp341, i32 0 ; <<4 x float>> [#uses=2]
  %splat343 = shufflevector <4 x float> %tmp342, <4 x float> %tmp342, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp344 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %mul345 = fmul <4 x float> %splat343, %tmp344   ; <<4 x float>> [#uses=1]
  %add346 = fadd <4 x float> %mul340, %mul345     ; <<4 x float>> [#uses=1]
  store <4 x float> %add346, <4 x float>* %zi3
  %tmp347 = load <4 x float>* %__r324             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp347, <4 x float>* %zr3
  br label %do.end348

do.end348:                                        ; preds = %do.body322
  br label %do.end349

do.end349:                                        ; preds = %do.end348
  call void @barrier(i32 1)
  %tmp350 = load float addrspace(3)** %lds.addr   ; <float addrspace(3)*> [#uses=1]
  %tmp351 = load i32* %me.addr                    ; <i32> [#uses=1]
  %add.ptr352 = getelementptr inbounds float addrspace(3)* %tmp350, i32 %tmp351 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr352, float addrspace(3)** %lp
  %tmp353 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp354 = extractelement <4 x float> %tmp353, i32 0 ; <float> [#uses=1]
  %tmp355 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx356 = getelementptr inbounds float addrspace(3)* %tmp355, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp354, float addrspace(3)* %arrayidx356
  %tmp357 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp358 = extractelement <4 x float> %tmp357, i32 1 ; <float> [#uses=1]
  %tmp359 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx360 = getelementptr inbounds float addrspace(3)* %tmp359, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp358, float addrspace(3)* %arrayidx360
  %tmp361 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp362 = extractelement <4 x float> %tmp361, i32 2 ; <float> [#uses=1]
  %tmp363 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx364 = getelementptr inbounds float addrspace(3)* %tmp363, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp362, float addrspace(3)* %arrayidx364
  %tmp365 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp366 = extractelement <4 x float> %tmp365, i32 3 ; <float> [#uses=1]
  %tmp367 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx368 = getelementptr inbounds float addrspace(3)* %tmp367, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp366, float addrspace(3)* %arrayidx368
  %tmp369 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr370 = getelementptr inbounds float addrspace(3)* %tmp369, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr370, float addrspace(3)** %lp
  %tmp371 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp372 = extractelement <4 x float> %tmp371, i32 0 ; <float> [#uses=1]
  %tmp373 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx374 = getelementptr inbounds float addrspace(3)* %tmp373, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp372, float addrspace(3)* %arrayidx374
  %tmp375 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp376 = extractelement <4 x float> %tmp375, i32 1 ; <float> [#uses=1]
  %tmp377 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx378 = getelementptr inbounds float addrspace(3)* %tmp377, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp376, float addrspace(3)* %arrayidx378
  %tmp379 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp380 = extractelement <4 x float> %tmp379, i32 2 ; <float> [#uses=1]
  %tmp381 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx382 = getelementptr inbounds float addrspace(3)* %tmp381, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp380, float addrspace(3)* %arrayidx382
  %tmp383 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp384 = extractelement <4 x float> %tmp383, i32 3 ; <float> [#uses=1]
  %tmp385 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx386 = getelementptr inbounds float addrspace(3)* %tmp385, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp384, float addrspace(3)* %arrayidx386
  %tmp387 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr388 = getelementptr inbounds float addrspace(3)* %tmp387, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr388, float addrspace(3)** %lp
  %tmp389 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp390 = extractelement <4 x float> %tmp389, i32 0 ; <float> [#uses=1]
  %tmp391 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx392 = getelementptr inbounds float addrspace(3)* %tmp391, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp390, float addrspace(3)* %arrayidx392
  %tmp393 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp394 = extractelement <4 x float> %tmp393, i32 1 ; <float> [#uses=1]
  %tmp395 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx396 = getelementptr inbounds float addrspace(3)* %tmp395, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp394, float addrspace(3)* %arrayidx396
  %tmp397 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp398 = extractelement <4 x float> %tmp397, i32 2 ; <float> [#uses=1]
  %tmp399 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx400 = getelementptr inbounds float addrspace(3)* %tmp399, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp398, float addrspace(3)* %arrayidx400
  %tmp401 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp402 = extractelement <4 x float> %tmp401, i32 3 ; <float> [#uses=1]
  %tmp403 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx404 = getelementptr inbounds float addrspace(3)* %tmp403, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp402, float addrspace(3)* %arrayidx404
  %tmp405 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr406 = getelementptr inbounds float addrspace(3)* %tmp405, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr406, float addrspace(3)** %lp
  %tmp407 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp408 = extractelement <4 x float> %tmp407, i32 0 ; <float> [#uses=1]
  %tmp409 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx410 = getelementptr inbounds float addrspace(3)* %tmp409, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp408, float addrspace(3)* %arrayidx410
  %tmp411 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp412 = extractelement <4 x float> %tmp411, i32 1 ; <float> [#uses=1]
  %tmp413 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx414 = getelementptr inbounds float addrspace(3)* %tmp413, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp412, float addrspace(3)* %arrayidx414
  %tmp415 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp416 = extractelement <4 x float> %tmp415, i32 2 ; <float> [#uses=1]
  %tmp417 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx418 = getelementptr inbounds float addrspace(3)* %tmp417, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp416, float addrspace(3)* %arrayidx418
  %tmp419 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp420 = extractelement <4 x float> %tmp419, i32 3 ; <float> [#uses=1]
  %tmp421 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx422 = getelementptr inbounds float addrspace(3)* %tmp421, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp420, float addrspace(3)* %arrayidx422
  %tmp423 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr424 = getelementptr inbounds float addrspace(3)* %tmp423, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr424, float addrspace(3)** %lp
  %tmp425 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp426 = extractelement <4 x float> %tmp425, i32 0 ; <float> [#uses=1]
  %tmp427 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx428 = getelementptr inbounds float addrspace(3)* %tmp427, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp426, float addrspace(3)* %arrayidx428
  %tmp429 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp430 = extractelement <4 x float> %tmp429, i32 1 ; <float> [#uses=1]
  %tmp431 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx432 = getelementptr inbounds float addrspace(3)* %tmp431, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp430, float addrspace(3)* %arrayidx432
  %tmp433 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp434 = extractelement <4 x float> %tmp433, i32 2 ; <float> [#uses=1]
  %tmp435 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx436 = getelementptr inbounds float addrspace(3)* %tmp435, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp434, float addrspace(3)* %arrayidx436
  %tmp437 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp438 = extractelement <4 x float> %tmp437, i32 3 ; <float> [#uses=1]
  %tmp439 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx440 = getelementptr inbounds float addrspace(3)* %tmp439, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp438, float addrspace(3)* %arrayidx440
  %tmp441 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr442 = getelementptr inbounds float addrspace(3)* %tmp441, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr442, float addrspace(3)** %lp
  %tmp443 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp444 = extractelement <4 x float> %tmp443, i32 0 ; <float> [#uses=1]
  %tmp445 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx446 = getelementptr inbounds float addrspace(3)* %tmp445, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp444, float addrspace(3)* %arrayidx446
  %tmp447 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp448 = extractelement <4 x float> %tmp447, i32 1 ; <float> [#uses=1]
  %tmp449 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx450 = getelementptr inbounds float addrspace(3)* %tmp449, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp448, float addrspace(3)* %arrayidx450
  %tmp451 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp452 = extractelement <4 x float> %tmp451, i32 2 ; <float> [#uses=1]
  %tmp453 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx454 = getelementptr inbounds float addrspace(3)* %tmp453, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp452, float addrspace(3)* %arrayidx454
  %tmp455 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp456 = extractelement <4 x float> %tmp455, i32 3 ; <float> [#uses=1]
  %tmp457 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx458 = getelementptr inbounds float addrspace(3)* %tmp457, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp456, float addrspace(3)* %arrayidx458
  %tmp459 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr460 = getelementptr inbounds float addrspace(3)* %tmp459, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr460, float addrspace(3)** %lp
  %tmp461 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp462 = extractelement <4 x float> %tmp461, i32 0 ; <float> [#uses=1]
  %tmp463 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx464 = getelementptr inbounds float addrspace(3)* %tmp463, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp462, float addrspace(3)* %arrayidx464
  %tmp465 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp466 = extractelement <4 x float> %tmp465, i32 1 ; <float> [#uses=1]
  %tmp467 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx468 = getelementptr inbounds float addrspace(3)* %tmp467, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp466, float addrspace(3)* %arrayidx468
  %tmp469 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp470 = extractelement <4 x float> %tmp469, i32 2 ; <float> [#uses=1]
  %tmp471 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx472 = getelementptr inbounds float addrspace(3)* %tmp471, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp470, float addrspace(3)* %arrayidx472
  %tmp473 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp474 = extractelement <4 x float> %tmp473, i32 3 ; <float> [#uses=1]
  %tmp475 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx476 = getelementptr inbounds float addrspace(3)* %tmp475, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp474, float addrspace(3)* %arrayidx476
  %tmp477 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr478 = getelementptr inbounds float addrspace(3)* %tmp477, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr478, float addrspace(3)** %lp
  %tmp479 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp480 = extractelement <4 x float> %tmp479, i32 0 ; <float> [#uses=1]
  %tmp481 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx482 = getelementptr inbounds float addrspace(3)* %tmp481, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp480, float addrspace(3)* %arrayidx482
  %tmp483 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp484 = extractelement <4 x float> %tmp483, i32 1 ; <float> [#uses=1]
  %tmp485 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx486 = getelementptr inbounds float addrspace(3)* %tmp485, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp484, float addrspace(3)* %arrayidx486
  %tmp487 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp488 = extractelement <4 x float> %tmp487, i32 2 ; <float> [#uses=1]
  %tmp489 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx490 = getelementptr inbounds float addrspace(3)* %tmp489, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp488, float addrspace(3)* %arrayidx490
  %tmp491 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp492 = extractelement <4 x float> %tmp491, i32 3 ; <float> [#uses=1]
  %tmp493 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx494 = getelementptr inbounds float addrspace(3)* %tmp493, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp492, float addrspace(3)* %arrayidx494
  call void @barrier(i32 1)
  ret void
}

; CHECK: ret
define void @kfft_pass5(i32 %me, float addrspace(3)* %lds, float addrspace(1)* %gr, float addrspace(1)* %gi) nounwind alwaysinline {
entry:
  %me.addr = alloca i32, align 4                  ; <i32*> [#uses=5]
  %lds.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=2]
  %gr.addr = alloca float addrspace(1)*, align 4  ; <float addrspace(1)**> [#uses=2]
  %gi.addr = alloca float addrspace(1)*, align 4  ; <float addrspace(1)**> [#uses=2]
  %lp = alloca float addrspace(3)*, align 4       ; <float addrspace(3)**> [#uses=47]
  %zr0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zr3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %zi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %ar0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ar2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %br2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %br3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %ai2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi0 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %bi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %bi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %gp = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  store i32 %me, i32* %me.addr
  store float addrspace(3)* %lds, float addrspace(3)** %lds.addr
  store float addrspace(1)* %gr, float addrspace(1)** %gr.addr
  store float addrspace(1)* %gi, float addrspace(1)** %gi.addr
  %tmp = load float addrspace(3)** %lds.addr      ; <float addrspace(3)*> [#uses=1]
  %tmp1 = load i32* %me.addr                      ; <i32> [#uses=1]
  %and = and i32 %tmp1, 15                        ; <i32> [#uses=1]
  %tmp2 = load i32* %me.addr                      ; <i32> [#uses=1]
  %shr = lshr i32 %tmp2, 4                        ; <i32> [#uses=1]
  %mul = mul i32 %shr, 272                        ; <i32> [#uses=1]
  %add = add i32 %and, %mul                       ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(3)* %tmp, i32 %add ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr, float addrspace(3)** %lp
  %tmp7 = load float addrspace(3)** %lp           ; <float addrspace(3)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(3)* %tmp7, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8 = load float addrspace(3)* %arrayidx      ; <float> [#uses=1]
  %tmp9 = load <4 x float>* %zr0                  ; <<4 x float>> [#uses=1]
  %tmp10 = insertelement <4 x float> %tmp9, float %tmp8, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10, <4 x float>* %zr0
  %tmp11 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds float addrspace(3)* %tmp11, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp13 = load float addrspace(3)* %arrayidx12   ; <float> [#uses=1]
  %tmp14 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp15 = insertelement <4 x float> %tmp14, float %tmp13, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15, <4 x float>* %zr0
  %tmp16 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx17 = getelementptr inbounds float addrspace(3)* %tmp16, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp18 = load float addrspace(3)* %arrayidx17   ; <float> [#uses=1]
  %tmp19 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp20 = insertelement <4 x float> %tmp19, float %tmp18, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20, <4 x float>* %zr0
  %tmp21 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds float addrspace(3)* %tmp21, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp23 = load float addrspace(3)* %arrayidx22   ; <float> [#uses=1]
  %tmp24 = load <4 x float>* %zr0                 ; <<4 x float>> [#uses=1]
  %tmp25 = insertelement <4 x float> %tmp24, float %tmp23, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %zr0
  %tmp26 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr27 = getelementptr inbounds float addrspace(3)* %tmp26, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27, float addrspace(3)** %lp
  %tmp28 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(3)* %tmp28, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30 = load float addrspace(3)* %arrayidx29   ; <float> [#uses=1]
  %tmp31 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp32 = insertelement <4 x float> %tmp31, float %tmp30, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32, <4 x float>* %zr1
  %tmp33 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(3)* %tmp33, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp35 = load float addrspace(3)* %arrayidx34   ; <float> [#uses=1]
  %tmp36 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp37 = insertelement <4 x float> %tmp36, float %tmp35, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37, <4 x float>* %zr1
  %tmp38 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx39 = getelementptr inbounds float addrspace(3)* %tmp38, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp40 = load float addrspace(3)* %arrayidx39   ; <float> [#uses=1]
  %tmp41 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp42 = insertelement <4 x float> %tmp41, float %tmp40, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %zr1
  %tmp43 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx44 = getelementptr inbounds float addrspace(3)* %tmp43, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp45 = load float addrspace(3)* %arrayidx44   ; <float> [#uses=1]
  %tmp46 = load <4 x float>* %zr1                 ; <<4 x float>> [#uses=1]
  %tmp47 = insertelement <4 x float> %tmp46, float %tmp45, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %zr1
  %tmp48 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr49 = getelementptr inbounds float addrspace(3)* %tmp48, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49, float addrspace(3)** %lp
  %tmp50 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds float addrspace(3)* %tmp50, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52 = load float addrspace(3)* %arrayidx51   ; <float> [#uses=1]
  %tmp53 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp54 = insertelement <4 x float> %tmp53, float %tmp52, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %zr2
  %tmp55 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(3)* %tmp55, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp57 = load float addrspace(3)* %arrayidx56   ; <float> [#uses=1]
  %tmp58 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp59 = insertelement <4 x float> %tmp58, float %tmp57, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59, <4 x float>* %zr2
  %tmp60 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx61 = getelementptr inbounds float addrspace(3)* %tmp60, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp62 = load float addrspace(3)* %arrayidx61   ; <float> [#uses=1]
  %tmp63 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp64 = insertelement <4 x float> %tmp63, float %tmp62, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64, <4 x float>* %zr2
  %tmp65 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float addrspace(3)* %tmp65, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp67 = load float addrspace(3)* %arrayidx66   ; <float> [#uses=1]
  %tmp68 = load <4 x float>* %zr2                 ; <<4 x float>> [#uses=1]
  %tmp69 = insertelement <4 x float> %tmp68, float %tmp67, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69, <4 x float>* %zr2
  %tmp70 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr71 = getelementptr inbounds float addrspace(3)* %tmp70, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71, float addrspace(3)** %lp
  %tmp72 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds float addrspace(3)* %tmp72, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74 = load float addrspace(3)* %arrayidx73   ; <float> [#uses=1]
  %tmp75 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp76 = insertelement <4 x float> %tmp75, float %tmp74, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76, <4 x float>* %zr3
  %tmp77 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(3)* %tmp77, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp79 = load float addrspace(3)* %arrayidx78   ; <float> [#uses=1]
  %tmp80 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp81 = insertelement <4 x float> %tmp80, float %tmp79, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81, <4 x float>* %zr3
  %tmp82 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds float addrspace(3)* %tmp82, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp84 = load float addrspace(3)* %arrayidx83   ; <float> [#uses=1]
  %tmp85 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp86 = insertelement <4 x float> %tmp85, float %tmp84, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86, <4 x float>* %zr3
  %tmp87 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds float addrspace(3)* %tmp87, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp89 = load float addrspace(3)* %arrayidx88   ; <float> [#uses=1]
  %tmp90 = load <4 x float>* %zr3                 ; <<4 x float>> [#uses=1]
  %tmp91 = insertelement <4 x float> %tmp90, float %tmp89, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91, <4 x float>* %zr3
  %tmp92 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %add.ptr93 = getelementptr inbounds float addrspace(3)* %tmp92, i32 1040 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93, float addrspace(3)** %lp
  %tmp98 = load float addrspace(3)** %lp          ; <float addrspace(3)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds float addrspace(3)* %tmp98, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100 = load float addrspace(3)* %arrayidx99  ; <float> [#uses=1]
  %tmp101 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp102 = insertelement <4 x float> %tmp101, float %tmp100, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102, <4 x float>* %zi0
  %tmp103 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx104 = getelementptr inbounds float addrspace(3)* %tmp103, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp105 = load float addrspace(3)* %arrayidx104 ; <float> [#uses=1]
  %tmp106 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp107 = insertelement <4 x float> %tmp106, float %tmp105, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107, <4 x float>* %zi0
  %tmp108 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(3)* %tmp108, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp110 = load float addrspace(3)* %arrayidx109 ; <float> [#uses=1]
  %tmp111 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp112 = insertelement <4 x float> %tmp111, float %tmp110, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112, <4 x float>* %zi0
  %tmp113 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds float addrspace(3)* %tmp113, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp115 = load float addrspace(3)* %arrayidx114 ; <float> [#uses=1]
  %tmp116 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp117 = insertelement <4 x float> %tmp116, float %tmp115, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117, <4 x float>* %zi0
  %tmp118 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr119 = getelementptr inbounds float addrspace(3)* %tmp118, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119, float addrspace(3)** %lp
  %tmp120 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds float addrspace(3)* %tmp120, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122 = load float addrspace(3)* %arrayidx121 ; <float> [#uses=1]
  %tmp123 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp124 = insertelement <4 x float> %tmp123, float %tmp122, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124, <4 x float>* %zi1
  %tmp125 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds float addrspace(3)* %tmp125, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp127 = load float addrspace(3)* %arrayidx126 ; <float> [#uses=1]
  %tmp128 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp129 = insertelement <4 x float> %tmp128, float %tmp127, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129, <4 x float>* %zi1
  %tmp130 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds float addrspace(3)* %tmp130, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp132 = load float addrspace(3)* %arrayidx131 ; <float> [#uses=1]
  %tmp133 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp134 = insertelement <4 x float> %tmp133, float %tmp132, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134, <4 x float>* %zi1
  %tmp135 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx136 = getelementptr inbounds float addrspace(3)* %tmp135, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp137 = load float addrspace(3)* %arrayidx136 ; <float> [#uses=1]
  %tmp138 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp139 = insertelement <4 x float> %tmp138, float %tmp137, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139, <4 x float>* %zi1
  %tmp140 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr141 = getelementptr inbounds float addrspace(3)* %tmp140, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141, float addrspace(3)** %lp
  %tmp142 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx143 = getelementptr inbounds float addrspace(3)* %tmp142, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144 = load float addrspace(3)* %arrayidx143 ; <float> [#uses=1]
  %tmp145 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp146 = insertelement <4 x float> %tmp145, float %tmp144, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146, <4 x float>* %zi2
  %tmp147 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx148 = getelementptr inbounds float addrspace(3)* %tmp147, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp149 = load float addrspace(3)* %arrayidx148 ; <float> [#uses=1]
  %tmp150 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp151 = insertelement <4 x float> %tmp150, float %tmp149, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151, <4 x float>* %zi2
  %tmp152 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx153 = getelementptr inbounds float addrspace(3)* %tmp152, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp154 = load float addrspace(3)* %arrayidx153 ; <float> [#uses=1]
  %tmp155 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp156 = insertelement <4 x float> %tmp155, float %tmp154, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156, <4 x float>* %zi2
  %tmp157 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx158 = getelementptr inbounds float addrspace(3)* %tmp157, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp159 = load float addrspace(3)* %arrayidx158 ; <float> [#uses=1]
  %tmp160 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp161 = insertelement <4 x float> %tmp160, float %tmp159, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161, <4 x float>* %zi2
  %tmp162 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %add.ptr163 = getelementptr inbounds float addrspace(3)* %tmp162, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163, float addrspace(3)** %lp
  %tmp164 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx165 = getelementptr inbounds float addrspace(3)* %tmp164, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166 = load float addrspace(3)* %arrayidx165 ; <float> [#uses=1]
  %tmp167 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp168 = insertelement <4 x float> %tmp167, float %tmp166, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168, <4 x float>* %zi3
  %tmp169 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx170 = getelementptr inbounds float addrspace(3)* %tmp169, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp171 = load float addrspace(3)* %arrayidx170 ; <float> [#uses=1]
  %tmp172 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp173 = insertelement <4 x float> %tmp172, float %tmp171, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173, <4 x float>* %zi3
  %tmp174 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(3)* %tmp174, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp176 = load float addrspace(3)* %arrayidx175 ; <float> [#uses=1]
  %tmp177 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp178 = insertelement <4 x float> %tmp177, float %tmp176, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178, <4 x float>* %zi3
  %tmp179 = load float addrspace(3)** %lp         ; <float addrspace(3)*> [#uses=1]
  %arrayidx180 = getelementptr inbounds float addrspace(3)* %tmp179, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp181 = load float addrspace(3)* %arrayidx180 ; <float> [#uses=1]
  %tmp182 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp183 = insertelement <4 x float> %tmp182, float %tmp181, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183, <4 x float>* %zi3
  br label %do.body

do.body:                                          ; preds = %entry
  %tmp185 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp186 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %add187 = fadd <4 x float> %tmp185, %tmp186     ; <<4 x float>> [#uses=1]
  store <4 x float> %add187, <4 x float>* %ar0
  %tmp189 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp190 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %add191 = fadd <4 x float> %tmp189, %tmp190     ; <<4 x float>> [#uses=1]
  store <4 x float> %add191, <4 x float>* %ar2
  %tmp193 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp194 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %add195 = fadd <4 x float> %tmp193, %tmp194     ; <<4 x float>> [#uses=1]
  store <4 x float> %add195, <4 x float>* %br0
  %tmp197 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp198 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp197, %tmp198        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %br1
  %tmp200 = load <4 x float>* %ar0                ; <<4 x float>> [#uses=1]
  %tmp201 = load <4 x float>* %ar2                ; <<4 x float>> [#uses=1]
  %sub202 = fsub <4 x float> %tmp200, %tmp201     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202, <4 x float>* %br2
  %tmp204 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp205 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %sub206 = fsub <4 x float> %tmp204, %tmp205     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206, <4 x float>* %br3
  %tmp208 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp209 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %add210 = fadd <4 x float> %tmp208, %tmp209     ; <<4 x float>> [#uses=1]
  store <4 x float> %add210, <4 x float>* %ai0
  %tmp212 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp213 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %add214 = fadd <4 x float> %tmp212, %tmp213     ; <<4 x float>> [#uses=1]
  store <4 x float> %add214, <4 x float>* %ai2
  %tmp216 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp217 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %add218 = fadd <4 x float> %tmp216, %tmp217     ; <<4 x float>> [#uses=1]
  store <4 x float> %add218, <4 x float>* %bi0
  %tmp220 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp221 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %sub222 = fsub <4 x float> %tmp220, %tmp221     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222, <4 x float>* %bi1
  %tmp224 = load <4 x float>* %ai0                ; <<4 x float>> [#uses=1]
  %tmp225 = load <4 x float>* %ai2                ; <<4 x float>> [#uses=1]
  %sub226 = fsub <4 x float> %tmp224, %tmp225     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226, <4 x float>* %bi2
  %tmp228 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp229 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %sub230 = fsub <4 x float> %tmp228, %tmp229     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230, <4 x float>* %bi3
  %tmp231 = load <4 x float>* %br0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231, <4 x float>* %zr0
  %tmp232 = load <4 x float>* %bi0                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232, <4 x float>* %zi0
  %tmp233 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp234 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %add235 = fadd <4 x float> %tmp233, %tmp234     ; <<4 x float>> [#uses=1]
  store <4 x float> %add235, <4 x float>* %zr1
  %tmp236 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %tmp237 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %sub238 = fsub <4 x float> %tmp236, %tmp237     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238, <4 x float>* %zi1
  %tmp239 = load <4 x float>* %br1                ; <<4 x float>> [#uses=1]
  %tmp240 = load <4 x float>* %bi3                ; <<4 x float>> [#uses=1]
  %sub241 = fsub <4 x float> %tmp239, %tmp240     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241, <4 x float>* %zr3
  %tmp242 = load <4 x float>* %br3                ; <<4 x float>> [#uses=1]
  %tmp243 = load <4 x float>* %bi1                ; <<4 x float>> [#uses=1]
  %add244 = fadd <4 x float> %tmp242, %tmp243     ; <<4 x float>> [#uses=1]
  store <4 x float> %add244, <4 x float>* %zi3
  %tmp245 = load <4 x float>* %br2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245, <4 x float>* %zr2
  %tmp246 = load <4 x float>* %bi2                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246, <4 x float>* %zi2
  br label %do.end

do.end:                                           ; preds = %do.body
  %tmp248 = load float addrspace(1)** %gr.addr    ; <float addrspace(1)*> [#uses=1]
  %tmp249 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl = shl i32 %tmp249, 2                       ; <i32> [#uses=1]
  %add.ptr250 = getelementptr inbounds float addrspace(1)* %tmp248, i32 %shl ; <float addrspace(1)*> [#uses=1]
  %0 = bitcast float addrspace(1)* %add.ptr250 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %0, <4 x float> addrspace(1)** %gp
  %tmp251 = load <4 x float>* %zr0                ; <<4 x float>> [#uses=1]
  %tmp252 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx253 = getelementptr inbounds <4 x float> addrspace(1)* %tmp252, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp251, <4 x float> addrspace(1)* %arrayidx253
  %tmp254 = load <4 x float>* %zr1                ; <<4 x float>> [#uses=1]
  %tmp255 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx256 = getelementptr inbounds <4 x float> addrspace(1)* %tmp255, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp254, <4 x float> addrspace(1)* %arrayidx256
  %tmp257 = load <4 x float>* %zr2                ; <<4 x float>> [#uses=1]
  %tmp258 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx259 = getelementptr inbounds <4 x float> addrspace(1)* %tmp258, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp257, <4 x float> addrspace(1)* %arrayidx259
  %tmp260 = load <4 x float>* %zr3                ; <<4 x float>> [#uses=1]
  %tmp261 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx262 = getelementptr inbounds <4 x float> addrspace(1)* %tmp261, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp260, <4 x float> addrspace(1)* %arrayidx262
  %tmp263 = load float addrspace(1)** %gi.addr    ; <float addrspace(1)*> [#uses=1]
  %tmp264 = load i32* %me.addr                    ; <i32> [#uses=1]
  %shl265 = shl i32 %tmp264, 2                    ; <i32> [#uses=1]
  %add.ptr266 = getelementptr inbounds float addrspace(1)* %tmp263, i32 %shl265 ; <float addrspace(1)*> [#uses=1]
  %1 = bitcast float addrspace(1)* %add.ptr266 to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %1, <4 x float> addrspace(1)** %gp
  %tmp267 = load <4 x float>* %zi0                ; <<4 x float>> [#uses=1]
  %tmp268 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx269 = getelementptr inbounds <4 x float> addrspace(1)* %tmp268, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp267, <4 x float> addrspace(1)* %arrayidx269
  %tmp270 = load <4 x float>* %zi1                ; <<4 x float>> [#uses=1]
  %tmp271 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx272 = getelementptr inbounds <4 x float> addrspace(1)* %tmp271, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp270, <4 x float> addrspace(1)* %arrayidx272
  %tmp273 = load <4 x float>* %zi2                ; <<4 x float>> [#uses=1]
  %tmp274 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx275 = getelementptr inbounds <4 x float> addrspace(1)* %tmp274, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp273, <4 x float> addrspace(1)* %arrayidx275
  %tmp276 = load <4 x float>* %zi3                ; <<4 x float>> [#uses=1]
  %tmp277 = load <4 x float> addrspace(1)** %gp   ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx278 = getelementptr inbounds <4 x float> addrspace(1)* %tmp277, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp276, <4 x float> addrspace(1)* %arrayidx278
  ret void
}

; CHECK: ret
define void @kfft(float addrspace(1)* %greal, float addrspace(1)* %gimag) nounwind {
entry:
  %retval.i510.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i511.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i512.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i513.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i492.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i493.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i494.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i495.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i.i810 = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i.i811 = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i.i812 = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i.i813 = alloca float, align 4               ; <float*> [#uses=3]
  %me.addr.i814 = alloca i32, align 4             ; <i32*> [#uses=6]
  %lds.addr.i815 = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp.i816 = alloca float addrspace(3)*, align 4  ; <float addrspace(3)**> [#uses=94]
  %zr0.i817 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zr1.i818 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr2.i819 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr3.i820 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi0.i821 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zi1.i822 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi2.i823 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi3.i824 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %ar0.i825 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ar2.i826 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br0.i827 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br1.i828 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br2.i829 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br3.i830 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai0.i831 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai2.i832 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi0.i833 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi1.i834 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi2.i835 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi3.i836 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %tbase.i837 = alloca i32, align 4               ; <i32*> [#uses=4]
  %c1.i838 = alloca float, align 4                ; <float*> [#uses=3]
  %s1.i839 = alloca float, align 4                ; <float*> [#uses=3]
  %__r.i840 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c2.i841 = alloca float, align 4                ; <float*> [#uses=3]
  %s2.i842 = alloca float, align 4                ; <float*> [#uses=3]
  %__r285.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i843 = alloca float, align 4                ; <float*> [#uses=3]
  %s3.i844 = alloca float, align 4                ; <float*> [#uses=3]
  %__r317.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %retval.i507.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i508.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i509.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i510.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i489.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i490.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i491.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i492.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i.i396 = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i.i397 = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i.i398 = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i.i399 = alloca float, align 4               ; <float*> [#uses=3]
  %me.addr.i400 = alloca i32, align 4             ; <i32*> [#uses=5]
  %lds.addr.i401 = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp.i402 = alloca float addrspace(3)*, align 4  ; <float addrspace(3)**> [#uses=94]
  %zr0.i403 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zr1.i404 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr2.i405 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr3.i406 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi0.i407 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zi1.i408 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi2.i409 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi3.i410 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %ar0.i411 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ar2.i412 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br0.i413 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br1.i414 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br2.i415 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br3.i416 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai0.i417 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai2.i418 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi0.i419 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi1.i420 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi2.i421 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi3.i422 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %tbase.i423 = alloca i32, align 4               ; <i32*> [#uses=4]
  %c1.i424 = alloca float, align 4                ; <float*> [#uses=3]
  %s1.i425 = alloca float, align 4                ; <float*> [#uses=3]
  %__r.i426 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c2.i427 = alloca float, align 4                ; <float*> [#uses=3]
  %s2.i428 = alloca float, align 4                ; <float*> [#uses=3]
  %__r286.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i429 = alloca float, align 4                ; <float*> [#uses=3]
  %s3.i430 = alloca float, align 4                ; <float*> [#uses=3]
  %__r318.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %retval.i513.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i514.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i515.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i516.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i495.i = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i496.i = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i497.i = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i498.i = alloca float, align 4               ; <float*> [#uses=3]
  %retval.i.i162 = alloca float, align 4          ; <float*> [#uses=2]
  %i.addr.i.i163 = alloca i32, align 4            ; <i32*> [#uses=5]
  %cretp.addr.i.i164 = alloca float*, align 4     ; <float**> [#uses=2]
  %x.i.i165 = alloca float, align 4               ; <float*> [#uses=3]
  %me.addr.i166 = alloca i32, align 4             ; <i32*> [#uses=6]
  %lds.addr.i167 = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %lp.i168 = alloca float addrspace(3)*, align 4  ; <float addrspace(3)**> [#uses=94]
  %zr0.i169 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zr1.i170 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr2.i171 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zr3.i172 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi0.i173 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=15]
  %zi1.i174 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi2.i175 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %zi3.i176 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=18]
  %ar0.i177 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ar2.i178 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br0.i179 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br1.i180 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %br2.i181 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %br3.i182 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai0.i183 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ai2.i184 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi0.i185 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi1.i186 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %bi2.i187 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %bi3.i188 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %tbase.i189 = alloca i32, align 4               ; <i32*> [#uses=4]
  %c1.i190 = alloca float, align 4                ; <float*> [#uses=3]
  %s1.i191 = alloca float, align 4                ; <float*> [#uses=3]
  %__r.i192 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c2.i193 = alloca float, align 4                ; <float*> [#uses=3]
  %s2.i194 = alloca float, align 4                ; <float*> [#uses=3]
  %__r292.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i195 = alloca float, align 4                ; <float*> [#uses=3]
  %s3.i196 = alloca float, align 4                ; <float*> [#uses=3]
  %__r324.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %me.addr.i17 = alloca i32, align 4              ; <i32*> [#uses=5]
  %lds.addr.i18 = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=2]
  %gr.addr.i19 = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %gi.addr.i20 = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %lp.i21 = alloca float addrspace(3)*, align 4   ; <float addrspace(3)**> [#uses=47]
  %zr0.i22 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zr1.i23 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zr2.i24 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zr3.i25 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zi0.i26 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zi1.i27 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zi2.i28 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %zi3.i29 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=12]
  %ar0.i30 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ar2.i31 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br0.i32 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br1.i33 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %br2.i34 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %br3.i35 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai0.i36 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %ai2.i37 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi0.i38 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi1.i39 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %bi2.i40 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %bi3.i41 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %gp.i42 = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %retval.i342.i = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %i.addr.i343.i = alloca <4 x i32>, align 16     ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i344.i = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %x.i345.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %retval.i324.i = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %i.addr.i325.i = alloca <4 x i32>, align 16     ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i326.i = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %x.i327.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %retval.i.i = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  %i.addr.i.i = alloca <4 x i32>, align 16        ; <<4 x i32>*> [#uses=5]
  %cretp.addr.i.i = alloca <4 x float>*, align 4  ; <<4 x float>**> [#uses=2]
  %x.i.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %me.addr.i = alloca i32, align 4                ; <i32*> [#uses=6]
  %gr.addr.i = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %gi.addr.i = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %lds.addr.i = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=2]
  %gp.i = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %lp.i = alloca float addrspace(3)*, align 4     ; <float addrspace(3)**> [#uses=47]
  %zr0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %zr1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zr2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zr3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=8]
  %zi1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %zi3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=11]
  %ar0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ar2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %br0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %br1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %br2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %br3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ai0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %ai2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %bi0.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %bi1.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %bi2.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %bi3.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %tbase.i = alloca <4 x i32>, align 16           ; <<4 x i32>*> [#uses=4]
  %.compoundliteral.i = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %c1.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s1.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r.i = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %c2.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s2.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r134.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %c3.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %s3.i = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=3]
  %__r158.i = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %greal.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %gimag.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %gr = alloca float addrspace(1)*, align 4       ; <float addrspace(1)**> [#uses=3]
  %gi = alloca float addrspace(1)*, align 4       ; <float addrspace(1)**> [#uses=3]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %me = alloca i32, align 4                       ; <i32*> [#uses=6]
  %dg = alloca i32, align 4                       ; <i32*> [#uses=3]
  store float addrspace(1)* %greal, float addrspace(1)** %greal.addr
  store float addrspace(1)* %gimag, float addrspace(1)** %gimag.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %and = and i32 %tmp, 63                         ; <i32> [#uses=1]
  store i32 %and, i32* %me
  %tmp2 = load i32* %gid                          ; <i32> [#uses=1]
  %shr = lshr i32 %tmp2, 6                        ; <i32> [#uses=1]
  %mul = mul i32 %shr, 1024                       ; <i32> [#uses=1]
  store i32 %mul, i32* %dg
  %tmp3 = load float addrspace(1)** %greal.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp4 = load i32* %dg                           ; <i32> [#uses=1]
  %add.ptr = getelementptr inbounds float addrspace(1)* %tmp3, i32 %tmp4 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr, float addrspace(1)** %gr
  %tmp5 = load float addrspace(1)** %gimag.addr   ; <float addrspace(1)*> [#uses=1]
  %tmp6 = load i32* %dg                           ; <i32> [#uses=1]
  %add.ptr7 = getelementptr inbounds float addrspace(1)* %tmp5, i32 %tmp6 ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %add.ptr7, float addrspace(1)** %gi
  %tmp8 = load i32* %me                           ; <i32> [#uses=1]
  %tmp9 = load float addrspace(1)** %gr           ; <float addrspace(1)*> [#uses=1]
  %tmp10 = load float addrspace(1)** %gi          ; <float addrspace(1)*> [#uses=1]
  store i32 %tmp8, i32* %me.addr.i
  store float addrspace(1)* %tmp9, float addrspace(1)** %gr.addr.i
  store float addrspace(1)* %tmp10, float addrspace(1)** %gi.addr.i
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %lds.addr.i
  %tmp.i = load float addrspace(1)** %gr.addr.i   ; <float addrspace(1)*> [#uses=1]
  %tmp1.i = load i32* %me.addr.i                  ; <i32> [#uses=1]
  %shl.i = shl i32 %tmp1.i, 2                     ; <i32> [#uses=1]
  %add.ptr.i = getelementptr inbounds float addrspace(1)* %tmp.i, i32 %shl.i ; <float addrspace(1)*> [#uses=1]
  %0 = bitcast float addrspace(1)* %add.ptr.i to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %0, <4 x float> addrspace(1)** %gp.i
  %tmp3.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp3.i, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp4.i = load <4 x float> addrspace(1)* %arrayidx.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp4.i, <4 x float>* %zr0.i
  %tmp6.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx7.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp6.i, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp8.i = load <4 x float> addrspace(1)* %arrayidx7.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp8.i, <4 x float>* %zr1.i
  %tmp10.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx11.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp10.i, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp12.i = load <4 x float> addrspace(1)* %arrayidx11.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp12.i, <4 x float>* %zr2.i
  %tmp14.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx15.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp14.i, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp16.i = load <4 x float> addrspace(1)* %arrayidx15.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp16.i, <4 x float>* %zr3.i
  %tmp17.i = load float addrspace(1)** %gi.addr.i ; <float addrspace(1)*> [#uses=1]
  %tmp18.i = load i32* %me.addr.i                 ; <i32> [#uses=1]
  %shl19.i = shl i32 %tmp18.i, 2                  ; <i32> [#uses=1]
  %add.ptr20.i = getelementptr inbounds float addrspace(1)* %tmp17.i, i32 %shl19.i ; <float addrspace(1)*> [#uses=1]
  %1 = bitcast float addrspace(1)* %add.ptr20.i to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %1, <4 x float> addrspace(1)** %gp.i
  %tmp22.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx23.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp22.i, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp24.i = load <4 x float> addrspace(1)* %arrayidx23.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp24.i, <4 x float>* %zi0.i
  %tmp26.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx27.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp26.i, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp28.i = load <4 x float> addrspace(1)* %arrayidx27.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp28.i, <4 x float>* %zi1.i
  %tmp30.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx31.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp30.i, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp32.i = load <4 x float> addrspace(1)* %arrayidx31.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32.i, <4 x float>* %zi2.i
  %tmp34.i = load <4 x float> addrspace(1)** %gp.i ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx35.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp34.i, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp36.i = load <4 x float> addrspace(1)* %arrayidx35.i ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36.i, <4 x float>* %zi3.i
  %tmp38.i = load <4 x float>* %zr0.i             ; <<4 x float>> [#uses=1]
  %tmp39.i = load <4 x float>* %zr2.i             ; <<4 x float>> [#uses=1]
  %add.i = fadd <4 x float> %tmp38.i, %tmp39.i    ; <<4 x float>> [#uses=1]
  store <4 x float> %add.i, <4 x float>* %ar0.i
  %tmp41.i = load <4 x float>* %zr1.i             ; <<4 x float>> [#uses=1]
  %tmp42.i = load <4 x float>* %zr3.i             ; <<4 x float>> [#uses=1]
  %add43.i = fadd <4 x float> %tmp41.i, %tmp42.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add43.i, <4 x float>* %ar2.i
  %tmp45.i = load <4 x float>* %ar0.i             ; <<4 x float>> [#uses=1]
  %tmp46.i = load <4 x float>* %ar2.i             ; <<4 x float>> [#uses=1]
  %add47.i = fadd <4 x float> %tmp45.i, %tmp46.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add47.i, <4 x float>* %br0.i
  %tmp49.i = load <4 x float>* %zr0.i             ; <<4 x float>> [#uses=1]
  %tmp50.i = load <4 x float>* %zr2.i             ; <<4 x float>> [#uses=1]
  %sub.i = fsub <4 x float> %tmp49.i, %tmp50.i    ; <<4 x float>> [#uses=1]
  store <4 x float> %sub.i, <4 x float>* %br1.i
  %tmp52.i = load <4 x float>* %ar0.i             ; <<4 x float>> [#uses=1]
  %tmp53.i = load <4 x float>* %ar2.i             ; <<4 x float>> [#uses=1]
  %sub54.i = fsub <4 x float> %tmp52.i, %tmp53.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub54.i, <4 x float>* %br2.i
  %tmp56.i = load <4 x float>* %zr1.i             ; <<4 x float>> [#uses=1]
  %tmp57.i = load <4 x float>* %zr3.i             ; <<4 x float>> [#uses=1]
  %sub58.i = fsub <4 x float> %tmp56.i, %tmp57.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub58.i, <4 x float>* %br3.i
  %tmp60.i = load <4 x float>* %zi0.i             ; <<4 x float>> [#uses=1]
  %tmp61.i = load <4 x float>* %zi2.i             ; <<4 x float>> [#uses=1]
  %add62.i = fadd <4 x float> %tmp60.i, %tmp61.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add62.i, <4 x float>* %ai0.i
  %tmp64.i = load <4 x float>* %zi1.i             ; <<4 x float>> [#uses=1]
  %tmp65.i = load <4 x float>* %zi3.i             ; <<4 x float>> [#uses=1]
  %add66.i = fadd <4 x float> %tmp64.i, %tmp65.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add66.i, <4 x float>* %ai2.i
  %tmp68.i = load <4 x float>* %ai0.i             ; <<4 x float>> [#uses=1]
  %tmp69.i = load <4 x float>* %ai2.i             ; <<4 x float>> [#uses=1]
  %add70.i = fadd <4 x float> %tmp68.i, %tmp69.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add70.i, <4 x float>* %bi0.i
  %tmp72.i = load <4 x float>* %zi0.i             ; <<4 x float>> [#uses=1]
  %tmp73.i = load <4 x float>* %zi2.i             ; <<4 x float>> [#uses=1]
  %sub74.i = fsub <4 x float> %tmp72.i, %tmp73.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub74.i, <4 x float>* %bi1.i
  %tmp76.i = load <4 x float>* %ai0.i             ; <<4 x float>> [#uses=1]
  %tmp77.i = load <4 x float>* %ai2.i             ; <<4 x float>> [#uses=1]
  %sub78.i = fsub <4 x float> %tmp76.i, %tmp77.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub78.i, <4 x float>* %bi2.i
  %tmp80.i = load <4 x float>* %zi1.i             ; <<4 x float>> [#uses=1]
  %tmp81.i = load <4 x float>* %zi3.i             ; <<4 x float>> [#uses=1]
  %sub82.i = fsub <4 x float> %tmp80.i, %tmp81.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub82.i, <4 x float>* %bi3.i
  %tmp83.i = load <4 x float>* %br0.i             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp83.i, <4 x float>* %zr0.i
  %tmp84.i = load <4 x float>* %bi0.i             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp84.i, <4 x float>* %zi0.i
  %tmp85.i = load <4 x float>* %br1.i             ; <<4 x float>> [#uses=1]
  %tmp86.i = load <4 x float>* %bi3.i             ; <<4 x float>> [#uses=1]
  %add87.i = fadd <4 x float> %tmp85.i, %tmp86.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add87.i, <4 x float>* %zr1.i
  %tmp88.i = load <4 x float>* %bi1.i             ; <<4 x float>> [#uses=1]
  %tmp89.i = load <4 x float>* %br3.i             ; <<4 x float>> [#uses=1]
  %sub90.i = fsub <4 x float> %tmp88.i, %tmp89.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub90.i, <4 x float>* %zi1.i
  %tmp91.i = load <4 x float>* %br1.i             ; <<4 x float>> [#uses=1]
  %tmp92.i = load <4 x float>* %bi3.i             ; <<4 x float>> [#uses=1]
  %sub93.i = fsub <4 x float> %tmp91.i, %tmp92.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %sub93.i, <4 x float>* %zr3.i
  %tmp94.i = load <4 x float>* %br3.i             ; <<4 x float>> [#uses=1]
  %tmp95.i = load <4 x float>* %bi1.i             ; <<4 x float>> [#uses=1]
  %add96.i = fadd <4 x float> %tmp94.i, %tmp95.i  ; <<4 x float>> [#uses=1]
  store <4 x float> %add96.i, <4 x float>* %zi3.i
  %tmp97.i = load <4 x float>* %br2.i             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp97.i, <4 x float>* %zr2.i
  %tmp98.i = load <4 x float>* %bi2.i             ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp98.i, <4 x float>* %zi2.i
  %tmp100.i = load i32* %me.addr.i                ; <i32> [#uses=1]
  %shl101.i = shl i32 %tmp100.i, 2                ; <i32> [#uses=1]
  %tmp102.i = insertelement <4 x i32> undef, i32 %shl101.i, i32 0 ; <<4 x i32>> [#uses=2]
  %splat.i = shufflevector <4 x i32> %tmp102.i, <4 x i32> %tmp102.i, <4 x i32> zeroinitializer ; <<4 x i32>> [#uses=1]
  store <4 x i32> <i32 0, i32 1, i32 2, i32 3>, <4 x i32>* %.compoundliteral.i
  %tmp103.i = load <4 x i32>* %.compoundliteral.i ; <<4 x i32>> [#uses=1]
  %add104.i = add nsw <4 x i32> %splat.i, %tmp103.i ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add104.i, <4 x i32>* %tbase.i
  %tmp108.i = load <4 x i32>* %tbase.i            ; <<4 x i32>> [#uses=1]
  %mul.i = mul <4 x i32> %tmp108.i, <i32 1, i32 1, i32 1, i32 1> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul.i, <4 x i32>* %i.addr.i.i
  store <4 x float>* %c1.i, <4 x float>** %cretp.addr.i.i
  %tmp.i.i = load <4 x i32>* %i.addr.i.i          ; <<4 x i32>> [#uses=1]
  %cmp.i.i = icmp sgt <4 x i32> %tmp.i.i, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i.i = sext <4 x i1> %cmp.i.i to <4 x i32> ; <<4 x i32>> [#uses=1]
  %and.i.i = and <4 x i32> %sext.i.i, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i.i = load <4 x i32>* %i.addr.i.i         ; <<4 x i32>> [#uses=1]
  %sub.i.i = sub <4 x i32> %tmp1.i.i, %and.i.i    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i.i, <4 x i32>* %i.addr.i.i
  %tmp3.i.i = load <4 x i32>* %i.addr.i.i         ; <<4 x i32>> [#uses=1]
  %call.i.i = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i.i) nounwind ; <<4 x float>> [#uses=1]
  %mul.i.i = fmul <4 x float> %call.i.i, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i.i, <4 x float>* %x.i.i
  %tmp4.i.i = load <4 x float>* %x.i.i            ; <<4 x float>> [#uses=1]
  %call5.i.i = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i.i) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i.i = load <4 x float>** %cretp.addr.i.i  ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i.i, <4 x float>* %tmp6.i.i
  %tmp7.i.i = load <4 x float>* %x.i.i            ; <<4 x float>> [#uses=1]
  %call8.i.i = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i.i) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i.i, <4 x float>* %retval.i.i
  %2 = load <4 x float>* %retval.i.i              ; <<4 x float>> [#uses=1]
  store <4 x float> %2, <4 x float>* %s1.i
  %tmp111.i = load <4 x float>* %c1.i             ; <<4 x float>> [#uses=1]
  %tmp112.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %mul113.i = fmul <4 x float> %tmp111.i, %tmp112.i ; <<4 x float>> [#uses=1]
  %tmp114.i = load <4 x float>* %s1.i             ; <<4 x float>> [#uses=1]
  %tmp115.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %mul116.i = fmul <4 x float> %tmp114.i, %tmp115.i ; <<4 x float>> [#uses=1]
  %sub117.i = fsub <4 x float> %mul113.i, %mul116.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub117.i, <4 x float>* %__r.i
  %tmp118.i = load <4 x float>* %c1.i             ; <<4 x float>> [#uses=1]
  %tmp119.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %mul120.i = fmul <4 x float> %tmp118.i, %tmp119.i ; <<4 x float>> [#uses=1]
  %tmp121.i = load <4 x float>* %s1.i             ; <<4 x float>> [#uses=1]
  %tmp122.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %mul123.i = fmul <4 x float> %tmp121.i, %tmp122.i ; <<4 x float>> [#uses=1]
  %add124.i = fadd <4 x float> %mul120.i, %mul123.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add124.i, <4 x float>* %zi1.i
  %tmp125.i = load <4 x float>* %__r.i            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp125.i, <4 x float>* %zr1.i
  %tmp129.i = load <4 x i32>* %tbase.i            ; <<4 x i32>> [#uses=1]
  %mul130.i = mul <4 x i32> %tmp129.i, <i32 2, i32 2, i32 2, i32 2> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul130.i, <4 x i32>* %i.addr.i343.i
  store <4 x float>* %c2.i, <4 x float>** %cretp.addr.i344.i
  %tmp.i346.i = load <4 x i32>* %i.addr.i343.i    ; <<4 x i32>> [#uses=1]
  %cmp.i347.i = icmp sgt <4 x i32> %tmp.i346.i, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i348.i = sext <4 x i1> %cmp.i347.i to <4 x i32> ; <<4 x i32>> [#uses=1]
  %and.i349.i = and <4 x i32> %sext.i348.i, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i350.i = load <4 x i32>* %i.addr.i343.i   ; <<4 x i32>> [#uses=1]
  %sub.i351.i = sub <4 x i32> %tmp1.i350.i, %and.i349.i ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i351.i, <4 x i32>* %i.addr.i343.i
  %tmp3.i352.i = load <4 x i32>* %i.addr.i343.i   ; <<4 x i32>> [#uses=1]
  %call.i353.i = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i352.i) nounwind ; <<4 x float>> [#uses=1]
  %mul.i354.i = fmul <4 x float> %call.i353.i, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i354.i, <4 x float>* %x.i345.i
  %tmp4.i355.i = load <4 x float>* %x.i345.i      ; <<4 x float>> [#uses=1]
  %call5.i356.i = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i355.i) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i357.i = load <4 x float>** %cretp.addr.i344.i ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i356.i, <4 x float>* %tmp6.i357.i
  %tmp7.i358.i = load <4 x float>* %x.i345.i      ; <<4 x float>> [#uses=1]
  %call8.i359.i = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i358.i) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i359.i, <4 x float>* %retval.i342.i
  %3 = load <4 x float>* %retval.i342.i           ; <<4 x float>> [#uses=1]
  store <4 x float> %3, <4 x float>* %s2.i
  %tmp135.i = load <4 x float>* %c2.i             ; <<4 x float>> [#uses=1]
  %tmp136.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %mul137.i = fmul <4 x float> %tmp135.i, %tmp136.i ; <<4 x float>> [#uses=1]
  %tmp138.i = load <4 x float>* %s2.i             ; <<4 x float>> [#uses=1]
  %tmp139.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %mul140.i = fmul <4 x float> %tmp138.i, %tmp139.i ; <<4 x float>> [#uses=1]
  %sub141.i = fsub <4 x float> %mul137.i, %mul140.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub141.i, <4 x float>* %__r134.i
  %tmp142.i = load <4 x float>* %c2.i             ; <<4 x float>> [#uses=1]
  %tmp143.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %mul144.i = fmul <4 x float> %tmp142.i, %tmp143.i ; <<4 x float>> [#uses=1]
  %tmp145.i = load <4 x float>* %s2.i             ; <<4 x float>> [#uses=1]
  %tmp146.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %mul147.i = fmul <4 x float> %tmp145.i, %tmp146.i ; <<4 x float>> [#uses=1]
  %add148.i = fadd <4 x float> %mul144.i, %mul147.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add148.i, <4 x float>* %zi2.i
  %tmp149.i = load <4 x float>* %__r134.i         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp149.i, <4 x float>* %zr2.i
  %tmp153.i = load <4 x i32>* %tbase.i            ; <<4 x i32>> [#uses=1]
  %mul154.i = mul <4 x i32> %tmp153.i, <i32 3, i32 3, i32 3, i32 3> ; <<4 x i32>> [#uses=1]
  store <4 x i32> %mul154.i, <4 x i32>* %i.addr.i325.i
  store <4 x float>* %c3.i, <4 x float>** %cretp.addr.i326.i
  %tmp.i328.i = load <4 x i32>* %i.addr.i325.i    ; <<4 x i32>> [#uses=1]
  %cmp.i329.i = icmp sgt <4 x i32> %tmp.i328.i, <i32 512, i32 512, i32 512, i32 512> ; <<4 x i1>> [#uses=1]
  %sext.i330.i = sext <4 x i1> %cmp.i329.i to <4 x i32> ; <<4 x i32>> [#uses=1]
  %and.i331.i = and <4 x i32> %sext.i330.i, <i32 1024, i32 1024, i32 1024, i32 1024> ; <<4 x i32>> [#uses=1]
  %tmp1.i332.i = load <4 x i32>* %i.addr.i325.i   ; <<4 x i32>> [#uses=1]
  %sub.i333.i = sub <4 x i32> %tmp1.i332.i, %and.i331.i ; <<4 x i32>> [#uses=1]
  store <4 x i32> %sub.i333.i, <4 x i32>* %i.addr.i325.i
  %tmp3.i334.i = load <4 x i32>* %i.addr.i325.i   ; <<4 x i32>> [#uses=1]
  %call.i335.i = call <4 x float> @_Z14convert_float4U8__vector4i(<4 x i32> %tmp3.i334.i) nounwind ; <<4 x float>> [#uses=1]
  %mul.i336.i = fmul <4 x float> %call.i335.i, <float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000, float 0xBF7921FB60000000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul.i336.i, <4 x float>* %x.i327.i
  %tmp4.i337.i = load <4 x float>* %x.i327.i      ; <<4 x float>> [#uses=1]
  %call5.i338.i = call <4 x float> @_Z10native_cosU8__vector4f(<4 x float> %tmp4.i337.i) nounwind ; <<4 x float>> [#uses=1]
  %tmp6.i339.i = load <4 x float>** %cretp.addr.i326.i ; <<4 x float>*> [#uses=1]
  store <4 x float> %call5.i338.i, <4 x float>* %tmp6.i339.i
  %tmp7.i340.i = load <4 x float>* %x.i327.i      ; <<4 x float>> [#uses=1]
  %call8.i341.i = call <4 x float> @_Z10native_sinU8__vector4f(<4 x float> %tmp7.i340.i) nounwind ; <<4 x float>> [#uses=1]
  store <4 x float> %call8.i341.i, <4 x float>* %retval.i324.i
  %4 = load <4 x float>* %retval.i324.i           ; <<4 x float>> [#uses=1]
  store <4 x float> %4, <4 x float>* %s3.i
  %tmp159.i = load <4 x float>* %c3.i             ; <<4 x float>> [#uses=1]
  %tmp160.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %mul161.i = fmul <4 x float> %tmp159.i, %tmp160.i ; <<4 x float>> [#uses=1]
  %tmp162.i = load <4 x float>* %s3.i             ; <<4 x float>> [#uses=1]
  %tmp163.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %mul164.i = fmul <4 x float> %tmp162.i, %tmp163.i ; <<4 x float>> [#uses=1]
  %sub165.i = fsub <4 x float> %mul161.i, %mul164.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub165.i, <4 x float>* %__r158.i
  %tmp166.i = load <4 x float>* %c3.i             ; <<4 x float>> [#uses=1]
  %tmp167.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %mul168.i = fmul <4 x float> %tmp166.i, %tmp167.i ; <<4 x float>> [#uses=1]
  %tmp169.i = load <4 x float>* %s3.i             ; <<4 x float>> [#uses=1]
  %tmp170.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %mul171.i = fmul <4 x float> %tmp169.i, %tmp170.i ; <<4 x float>> [#uses=1]
  %add172.i = fadd <4 x float> %mul168.i, %mul171.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add172.i, <4 x float>* %zi3.i
  %tmp173.i = load <4 x float>* %__r158.i         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173.i, <4 x float>* %zr3.i
  %tmp176.i = load float addrspace(3)** %lds.addr.i ; <float addrspace(3)*> [#uses=1]
  %tmp177.i = load i32* %me.addr.i                ; <i32> [#uses=1]
  %shl178.i = shl i32 %tmp177.i, 2                ; <i32> [#uses=1]
  %tmp179.i = load i32* %me.addr.i                ; <i32> [#uses=1]
  %shr.i = lshr i32 %tmp179.i, 3                  ; <i32> [#uses=1]
  %add180.i = add i32 %shl178.i, %shr.i           ; <i32> [#uses=1]
  %add.ptr181.i = getelementptr inbounds float addrspace(3)* %tmp176.i, i32 %add180.i ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr181.i, float addrspace(3)** %lp.i
  %tmp182.i = load <4 x float>* %zr0.i            ; <<4 x float>> [#uses=1]
  %tmp183.i = extractelement <4 x float> %tmp182.i, i32 0 ; <float> [#uses=1]
  %tmp184.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx185.i = getelementptr inbounds float addrspace(3)* %tmp184.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp183.i, float addrspace(3)* %arrayidx185.i
  %tmp186.i = load <4 x float>* %zr0.i            ; <<4 x float>> [#uses=1]
  %tmp187.i = extractelement <4 x float> %tmp186.i, i32 1 ; <float> [#uses=1]
  %tmp188.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx189.i = getelementptr inbounds float addrspace(3)* %tmp188.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp187.i, float addrspace(3)* %arrayidx189.i
  %tmp190.i = load <4 x float>* %zr0.i            ; <<4 x float>> [#uses=1]
  %tmp191.i = extractelement <4 x float> %tmp190.i, i32 2 ; <float> [#uses=1]
  %tmp192.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx193.i = getelementptr inbounds float addrspace(3)* %tmp192.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp191.i, float addrspace(3)* %arrayidx193.i
  %tmp194.i = load <4 x float>* %zr0.i            ; <<4 x float>> [#uses=1]
  %tmp195.i = extractelement <4 x float> %tmp194.i, i32 3 ; <float> [#uses=1]
  %tmp196.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx197.i = getelementptr inbounds float addrspace(3)* %tmp196.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp195.i, float addrspace(3)* %arrayidx197.i
  %tmp198.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr199.i = getelementptr inbounds float addrspace(3)* %tmp198.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr199.i, float addrspace(3)** %lp.i
  %tmp200.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %tmp201.i = extractelement <4 x float> %tmp200.i, i32 0 ; <float> [#uses=1]
  %tmp202.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx203.i = getelementptr inbounds float addrspace(3)* %tmp202.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp201.i, float addrspace(3)* %arrayidx203.i
  %tmp204.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %tmp205.i = extractelement <4 x float> %tmp204.i, i32 1 ; <float> [#uses=1]
  %tmp206.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx207.i = getelementptr inbounds float addrspace(3)* %tmp206.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp205.i, float addrspace(3)* %arrayidx207.i
  %tmp208.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %tmp209.i = extractelement <4 x float> %tmp208.i, i32 2 ; <float> [#uses=1]
  %tmp210.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx211.i = getelementptr inbounds float addrspace(3)* %tmp210.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp209.i, float addrspace(3)* %arrayidx211.i
  %tmp212.i = load <4 x float>* %zr1.i            ; <<4 x float>> [#uses=1]
  %tmp213.i = extractelement <4 x float> %tmp212.i, i32 3 ; <float> [#uses=1]
  %tmp214.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx215.i = getelementptr inbounds float addrspace(3)* %tmp214.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp213.i, float addrspace(3)* %arrayidx215.i
  %tmp216.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr217.i = getelementptr inbounds float addrspace(3)* %tmp216.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr217.i, float addrspace(3)** %lp.i
  %tmp218.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %tmp219.i = extractelement <4 x float> %tmp218.i, i32 0 ; <float> [#uses=1]
  %tmp220.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx221.i = getelementptr inbounds float addrspace(3)* %tmp220.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp219.i, float addrspace(3)* %arrayidx221.i
  %tmp222.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %tmp223.i = extractelement <4 x float> %tmp222.i, i32 1 ; <float> [#uses=1]
  %tmp224.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx225.i = getelementptr inbounds float addrspace(3)* %tmp224.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp223.i, float addrspace(3)* %arrayidx225.i
  %tmp226.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %tmp227.i = extractelement <4 x float> %tmp226.i, i32 2 ; <float> [#uses=1]
  %tmp228.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx229.i = getelementptr inbounds float addrspace(3)* %tmp228.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp227.i, float addrspace(3)* %arrayidx229.i
  %tmp230.i = load <4 x float>* %zr2.i            ; <<4 x float>> [#uses=1]
  %tmp231.i = extractelement <4 x float> %tmp230.i, i32 3 ; <float> [#uses=1]
  %tmp232.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx233.i = getelementptr inbounds float addrspace(3)* %tmp232.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp231.i, float addrspace(3)* %arrayidx233.i
  %tmp234.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr235.i = getelementptr inbounds float addrspace(3)* %tmp234.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr235.i, float addrspace(3)** %lp.i
  %tmp236.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %tmp237.i = extractelement <4 x float> %tmp236.i, i32 0 ; <float> [#uses=1]
  %tmp238.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx239.i = getelementptr inbounds float addrspace(3)* %tmp238.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp237.i, float addrspace(3)* %arrayidx239.i
  %tmp240.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %tmp241.i = extractelement <4 x float> %tmp240.i, i32 1 ; <float> [#uses=1]
  %tmp242.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx243.i = getelementptr inbounds float addrspace(3)* %tmp242.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp241.i, float addrspace(3)* %arrayidx243.i
  %tmp244.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %tmp245.i = extractelement <4 x float> %tmp244.i, i32 2 ; <float> [#uses=1]
  %tmp246.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx247.i = getelementptr inbounds float addrspace(3)* %tmp246.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp245.i, float addrspace(3)* %arrayidx247.i
  %tmp248.i = load <4 x float>* %zr3.i            ; <<4 x float>> [#uses=1]
  %tmp249.i = extractelement <4 x float> %tmp248.i, i32 3 ; <float> [#uses=1]
  %tmp250.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx251.i = getelementptr inbounds float addrspace(3)* %tmp250.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp249.i, float addrspace(3)* %arrayidx251.i
  %tmp252.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr253.i = getelementptr inbounds float addrspace(3)* %tmp252.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr253.i, float addrspace(3)** %lp.i
  %tmp254.i = load <4 x float>* %zi0.i            ; <<4 x float>> [#uses=1]
  %tmp255.i = extractelement <4 x float> %tmp254.i, i32 0 ; <float> [#uses=1]
  %tmp256.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx257.i = getelementptr inbounds float addrspace(3)* %tmp256.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp255.i, float addrspace(3)* %arrayidx257.i
  %tmp258.i = load <4 x float>* %zi0.i            ; <<4 x float>> [#uses=1]
  %tmp259.i = extractelement <4 x float> %tmp258.i, i32 1 ; <float> [#uses=1]
  %tmp260.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx261.i = getelementptr inbounds float addrspace(3)* %tmp260.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp259.i, float addrspace(3)* %arrayidx261.i
  %tmp262.i = load <4 x float>* %zi0.i            ; <<4 x float>> [#uses=1]
  %tmp263.i = extractelement <4 x float> %tmp262.i, i32 2 ; <float> [#uses=1]
  %tmp264.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx265.i = getelementptr inbounds float addrspace(3)* %tmp264.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp263.i, float addrspace(3)* %arrayidx265.i
  %tmp266.i = load <4 x float>* %zi0.i            ; <<4 x float>> [#uses=1]
  %tmp267.i = extractelement <4 x float> %tmp266.i, i32 3 ; <float> [#uses=1]
  %tmp268.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx269.i = getelementptr inbounds float addrspace(3)* %tmp268.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp267.i, float addrspace(3)* %arrayidx269.i
  %tmp270.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr271.i = getelementptr inbounds float addrspace(3)* %tmp270.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr271.i, float addrspace(3)** %lp.i
  %tmp272.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %tmp273.i = extractelement <4 x float> %tmp272.i, i32 0 ; <float> [#uses=1]
  %tmp274.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx275.i = getelementptr inbounds float addrspace(3)* %tmp274.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp273.i, float addrspace(3)* %arrayidx275.i
  %tmp276.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %tmp277.i = extractelement <4 x float> %tmp276.i, i32 1 ; <float> [#uses=1]
  %tmp278.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx279.i = getelementptr inbounds float addrspace(3)* %tmp278.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp277.i, float addrspace(3)* %arrayidx279.i
  %tmp280.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %tmp281.i = extractelement <4 x float> %tmp280.i, i32 2 ; <float> [#uses=1]
  %tmp282.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx283.i = getelementptr inbounds float addrspace(3)* %tmp282.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp281.i, float addrspace(3)* %arrayidx283.i
  %tmp284.i = load <4 x float>* %zi1.i            ; <<4 x float>> [#uses=1]
  %tmp285.i = extractelement <4 x float> %tmp284.i, i32 3 ; <float> [#uses=1]
  %tmp286.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx287.i = getelementptr inbounds float addrspace(3)* %tmp286.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp285.i, float addrspace(3)* %arrayidx287.i
  %tmp288.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr289.i = getelementptr inbounds float addrspace(3)* %tmp288.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr289.i, float addrspace(3)** %lp.i
  %tmp290.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %tmp291.i = extractelement <4 x float> %tmp290.i, i32 0 ; <float> [#uses=1]
  %tmp292.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx293.i = getelementptr inbounds float addrspace(3)* %tmp292.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp291.i, float addrspace(3)* %arrayidx293.i
  %tmp294.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %tmp295.i = extractelement <4 x float> %tmp294.i, i32 1 ; <float> [#uses=1]
  %tmp296.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx297.i = getelementptr inbounds float addrspace(3)* %tmp296.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp295.i, float addrspace(3)* %arrayidx297.i
  %tmp298.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %tmp299.i = extractelement <4 x float> %tmp298.i, i32 2 ; <float> [#uses=1]
  %tmp300.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx301.i = getelementptr inbounds float addrspace(3)* %tmp300.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp299.i, float addrspace(3)* %arrayidx301.i
  %tmp302.i = load <4 x float>* %zi2.i            ; <<4 x float>> [#uses=1]
  %tmp303.i = extractelement <4 x float> %tmp302.i, i32 3 ; <float> [#uses=1]
  %tmp304.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx305.i = getelementptr inbounds float addrspace(3)* %tmp304.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp303.i, float addrspace(3)* %arrayidx305.i
  %tmp306.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %add.ptr307.i = getelementptr inbounds float addrspace(3)* %tmp306.i, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr307.i, float addrspace(3)** %lp.i
  %tmp308.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %tmp309.i = extractelement <4 x float> %tmp308.i, i32 0 ; <float> [#uses=1]
  %tmp310.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx311.i = getelementptr inbounds float addrspace(3)* %tmp310.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp309.i, float addrspace(3)* %arrayidx311.i
  %tmp312.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %tmp313.i = extractelement <4 x float> %tmp312.i, i32 1 ; <float> [#uses=1]
  %tmp314.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx315.i = getelementptr inbounds float addrspace(3)* %tmp314.i, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp313.i, float addrspace(3)* %arrayidx315.i
  %tmp316.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %tmp317.i = extractelement <4 x float> %tmp316.i, i32 2 ; <float> [#uses=1]
  %tmp318.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx319.i = getelementptr inbounds float addrspace(3)* %tmp318.i, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp317.i, float addrspace(3)* %arrayidx319.i
  %tmp320.i = load <4 x float>* %zi3.i            ; <<4 x float>> [#uses=1]
  %tmp321.i = extractelement <4 x float> %tmp320.i, i32 3 ; <float> [#uses=1]
  %tmp322.i = load float addrspace(3)** %lp.i     ; <float addrspace(3)*> [#uses=1]
  %arrayidx323.i = getelementptr inbounds float addrspace(3)* %tmp322.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp321.i, float addrspace(3)* %arrayidx323.i
  call void @barrier(i32 1) nounwind
  %tmp11 = load i32* %me                          ; <i32> [#uses=1]
  store i32 %tmp11, i32* %me.addr.i814
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %lds.addr.i815
  %tmp.i845 = load float addrspace(3)** %lds.addr.i815 ; <float addrspace(3)*> [#uses=1]
  %tmp1.i846 = load i32* %me.addr.i814            ; <i32> [#uses=1]
  %tmp2.i847 = load i32* %me.addr.i814            ; <i32> [#uses=1]
  %shr.i848 = lshr i32 %tmp2.i847, 5              ; <i32> [#uses=1]
  %add.i849 = add i32 %tmp1.i846, %shr.i848       ; <i32> [#uses=1]
  %add.ptr.i850 = getelementptr inbounds float addrspace(3)* %tmp.i845, i32 %add.i849 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr.i850, float addrspace(3)** %lp.i816
  %tmp7.i851 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx.i852 = getelementptr inbounds float addrspace(3)* %tmp7.i851, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8.i853 = load float addrspace(3)* %arrayidx.i852 ; <float> [#uses=1]
  %tmp9.i854 = load <4 x float>* %zr0.i817        ; <<4 x float>> [#uses=1]
  %tmp10.i855 = insertelement <4 x float> %tmp9.i854, float %tmp8.i853, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10.i855, <4 x float>* %zr0.i817
  %tmp11.i856 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx12.i857 = getelementptr inbounds float addrspace(3)* %tmp11.i856, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp13.i858 = load float addrspace(3)* %arrayidx12.i857 ; <float> [#uses=1]
  %tmp14.i859 = load <4 x float>* %zr1.i818       ; <<4 x float>> [#uses=1]
  %tmp15.i860 = insertelement <4 x float> %tmp14.i859, float %tmp13.i858, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15.i860, <4 x float>* %zr1.i818
  %tmp16.i861 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx17.i862 = getelementptr inbounds float addrspace(3)* %tmp16.i861, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp18.i863 = load float addrspace(3)* %arrayidx17.i862 ; <float> [#uses=1]
  %tmp19.i864 = load <4 x float>* %zr2.i819       ; <<4 x float>> [#uses=1]
  %tmp20.i865 = insertelement <4 x float> %tmp19.i864, float %tmp18.i863, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20.i865, <4 x float>* %zr2.i819
  %tmp21.i866 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx22.i867 = getelementptr inbounds float addrspace(3)* %tmp21.i866, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp23.i868 = load float addrspace(3)* %arrayidx22.i867 ; <float> [#uses=1]
  %tmp24.i869 = load <4 x float>* %zr3.i820       ; <<4 x float>> [#uses=1]
  %tmp25.i870 = insertelement <4 x float> %tmp24.i869, float %tmp23.i868, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25.i870, <4 x float>* %zr3.i820
  %tmp26.i871 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr27.i872 = getelementptr inbounds float addrspace(3)* %tmp26.i871, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27.i872, float addrspace(3)** %lp.i816
  %tmp28.i873 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx29.i874 = getelementptr inbounds float addrspace(3)* %tmp28.i873, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30.i875 = load float addrspace(3)* %arrayidx29.i874 ; <float> [#uses=1]
  %tmp31.i876 = load <4 x float>* %zr0.i817       ; <<4 x float>> [#uses=1]
  %tmp32.i877 = insertelement <4 x float> %tmp31.i876, float %tmp30.i875, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32.i877, <4 x float>* %zr0.i817
  %tmp33.i878 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx34.i879 = getelementptr inbounds float addrspace(3)* %tmp33.i878, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp35.i880 = load float addrspace(3)* %arrayidx34.i879 ; <float> [#uses=1]
  %tmp36.i881 = load <4 x float>* %zr1.i818       ; <<4 x float>> [#uses=1]
  %tmp37.i882 = insertelement <4 x float> %tmp36.i881, float %tmp35.i880, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37.i882, <4 x float>* %zr1.i818
  %tmp38.i883 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx39.i884 = getelementptr inbounds float addrspace(3)* %tmp38.i883, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp40.i885 = load float addrspace(3)* %arrayidx39.i884 ; <float> [#uses=1]
  %tmp41.i886 = load <4 x float>* %zr2.i819       ; <<4 x float>> [#uses=1]
  %tmp42.i887 = insertelement <4 x float> %tmp41.i886, float %tmp40.i885, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42.i887, <4 x float>* %zr2.i819
  %tmp43.i888 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx44.i889 = getelementptr inbounds float addrspace(3)* %tmp43.i888, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp45.i890 = load float addrspace(3)* %arrayidx44.i889 ; <float> [#uses=1]
  %tmp46.i891 = load <4 x float>* %zr3.i820       ; <<4 x float>> [#uses=1]
  %tmp47.i892 = insertelement <4 x float> %tmp46.i891, float %tmp45.i890, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47.i892, <4 x float>* %zr3.i820
  %tmp48.i893 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr49.i894 = getelementptr inbounds float addrspace(3)* %tmp48.i893, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49.i894, float addrspace(3)** %lp.i816
  %tmp50.i895 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx51.i896 = getelementptr inbounds float addrspace(3)* %tmp50.i895, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52.i897 = load float addrspace(3)* %arrayidx51.i896 ; <float> [#uses=1]
  %tmp53.i898 = load <4 x float>* %zr0.i817       ; <<4 x float>> [#uses=1]
  %tmp54.i899 = insertelement <4 x float> %tmp53.i898, float %tmp52.i897, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54.i899, <4 x float>* %zr0.i817
  %tmp55.i900 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx56.i901 = getelementptr inbounds float addrspace(3)* %tmp55.i900, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp57.i902 = load float addrspace(3)* %arrayidx56.i901 ; <float> [#uses=1]
  %tmp58.i903 = load <4 x float>* %zr1.i818       ; <<4 x float>> [#uses=1]
  %tmp59.i904 = insertelement <4 x float> %tmp58.i903, float %tmp57.i902, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59.i904, <4 x float>* %zr1.i818
  %tmp60.i905 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx61.i906 = getelementptr inbounds float addrspace(3)* %tmp60.i905, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp62.i907 = load float addrspace(3)* %arrayidx61.i906 ; <float> [#uses=1]
  %tmp63.i908 = load <4 x float>* %zr2.i819       ; <<4 x float>> [#uses=1]
  %tmp64.i909 = insertelement <4 x float> %tmp63.i908, float %tmp62.i907, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64.i909, <4 x float>* %zr2.i819
  %tmp65.i910 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx66.i911 = getelementptr inbounds float addrspace(3)* %tmp65.i910, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp67.i912 = load float addrspace(3)* %arrayidx66.i911 ; <float> [#uses=1]
  %tmp68.i913 = load <4 x float>* %zr3.i820       ; <<4 x float>> [#uses=1]
  %tmp69.i914 = insertelement <4 x float> %tmp68.i913, float %tmp67.i912, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69.i914, <4 x float>* %zr3.i820
  %tmp70.i915 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr71.i916 = getelementptr inbounds float addrspace(3)* %tmp70.i915, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71.i916, float addrspace(3)** %lp.i816
  %tmp72.i917 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx73.i918 = getelementptr inbounds float addrspace(3)* %tmp72.i917, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74.i919 = load float addrspace(3)* %arrayidx73.i918 ; <float> [#uses=1]
  %tmp75.i920 = load <4 x float>* %zr0.i817       ; <<4 x float>> [#uses=1]
  %tmp76.i921 = insertelement <4 x float> %tmp75.i920, float %tmp74.i919, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76.i921, <4 x float>* %zr0.i817
  %tmp77.i922 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx78.i923 = getelementptr inbounds float addrspace(3)* %tmp77.i922, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp79.i924 = load float addrspace(3)* %arrayidx78.i923 ; <float> [#uses=1]
  %tmp80.i925 = load <4 x float>* %zr1.i818       ; <<4 x float>> [#uses=1]
  %tmp81.i926 = insertelement <4 x float> %tmp80.i925, float %tmp79.i924, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81.i926, <4 x float>* %zr1.i818
  %tmp82.i927 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx83.i928 = getelementptr inbounds float addrspace(3)* %tmp82.i927, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp84.i929 = load float addrspace(3)* %arrayidx83.i928 ; <float> [#uses=1]
  %tmp85.i930 = load <4 x float>* %zr2.i819       ; <<4 x float>> [#uses=1]
  %tmp86.i931 = insertelement <4 x float> %tmp85.i930, float %tmp84.i929, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86.i931, <4 x float>* %zr2.i819
  %tmp87.i932 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx88.i933 = getelementptr inbounds float addrspace(3)* %tmp87.i932, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp89.i934 = load float addrspace(3)* %arrayidx88.i933 ; <float> [#uses=1]
  %tmp90.i935 = load <4 x float>* %zr3.i820       ; <<4 x float>> [#uses=1]
  %tmp91.i936 = insertelement <4 x float> %tmp90.i935, float %tmp89.i934, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91.i936, <4 x float>* %zr3.i820
  %tmp92.i937 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr93.i938 = getelementptr inbounds float addrspace(3)* %tmp92.i937, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93.i938, float addrspace(3)** %lp.i816
  %tmp98.i939 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx99.i940 = getelementptr inbounds float addrspace(3)* %tmp98.i939, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100.i941 = load float addrspace(3)* %arrayidx99.i940 ; <float> [#uses=1]
  %tmp101.i942 = load <4 x float>* %zi0.i821      ; <<4 x float>> [#uses=1]
  %tmp102.i943 = insertelement <4 x float> %tmp101.i942, float %tmp100.i941, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102.i943, <4 x float>* %zi0.i821
  %tmp103.i944 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx104.i945 = getelementptr inbounds float addrspace(3)* %tmp103.i944, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp105.i946 = load float addrspace(3)* %arrayidx104.i945 ; <float> [#uses=1]
  %tmp106.i947 = load <4 x float>* %zi1.i822      ; <<4 x float>> [#uses=1]
  %tmp107.i948 = insertelement <4 x float> %tmp106.i947, float %tmp105.i946, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107.i948, <4 x float>* %zi1.i822
  %tmp108.i949 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx109.i950 = getelementptr inbounds float addrspace(3)* %tmp108.i949, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp110.i951 = load float addrspace(3)* %arrayidx109.i950 ; <float> [#uses=1]
  %tmp111.i952 = load <4 x float>* %zi2.i823      ; <<4 x float>> [#uses=1]
  %tmp112.i953 = insertelement <4 x float> %tmp111.i952, float %tmp110.i951, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112.i953, <4 x float>* %zi2.i823
  %tmp113.i954 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx114.i955 = getelementptr inbounds float addrspace(3)* %tmp113.i954, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp115.i956 = load float addrspace(3)* %arrayidx114.i955 ; <float> [#uses=1]
  %tmp116.i957 = load <4 x float>* %zi3.i824      ; <<4 x float>> [#uses=1]
  %tmp117.i958 = insertelement <4 x float> %tmp116.i957, float %tmp115.i956, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117.i958, <4 x float>* %zi3.i824
  %tmp118.i959 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr119.i960 = getelementptr inbounds float addrspace(3)* %tmp118.i959, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119.i960, float addrspace(3)** %lp.i816
  %tmp120.i961 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx121.i962 = getelementptr inbounds float addrspace(3)* %tmp120.i961, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122.i963 = load float addrspace(3)* %arrayidx121.i962 ; <float> [#uses=1]
  %tmp123.i964 = load <4 x float>* %zi0.i821      ; <<4 x float>> [#uses=1]
  %tmp124.i965 = insertelement <4 x float> %tmp123.i964, float %tmp122.i963, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124.i965, <4 x float>* %zi0.i821
  %tmp125.i966 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx126.i967 = getelementptr inbounds float addrspace(3)* %tmp125.i966, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp127.i968 = load float addrspace(3)* %arrayidx126.i967 ; <float> [#uses=1]
  %tmp128.i969 = load <4 x float>* %zi1.i822      ; <<4 x float>> [#uses=1]
  %tmp129.i970 = insertelement <4 x float> %tmp128.i969, float %tmp127.i968, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129.i970, <4 x float>* %zi1.i822
  %tmp130.i971 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx131.i972 = getelementptr inbounds float addrspace(3)* %tmp130.i971, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp132.i973 = load float addrspace(3)* %arrayidx131.i972 ; <float> [#uses=1]
  %tmp133.i974 = load <4 x float>* %zi2.i823      ; <<4 x float>> [#uses=1]
  %tmp134.i975 = insertelement <4 x float> %tmp133.i974, float %tmp132.i973, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134.i975, <4 x float>* %zi2.i823
  %tmp135.i976 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx136.i977 = getelementptr inbounds float addrspace(3)* %tmp135.i976, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp137.i978 = load float addrspace(3)* %arrayidx136.i977 ; <float> [#uses=1]
  %tmp138.i979 = load <4 x float>* %zi3.i824      ; <<4 x float>> [#uses=1]
  %tmp139.i980 = insertelement <4 x float> %tmp138.i979, float %tmp137.i978, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139.i980, <4 x float>* %zi3.i824
  %tmp140.i981 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr141.i982 = getelementptr inbounds float addrspace(3)* %tmp140.i981, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141.i982, float addrspace(3)** %lp.i816
  %tmp142.i983 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx143.i984 = getelementptr inbounds float addrspace(3)* %tmp142.i983, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144.i985 = load float addrspace(3)* %arrayidx143.i984 ; <float> [#uses=1]
  %tmp145.i986 = load <4 x float>* %zi0.i821      ; <<4 x float>> [#uses=1]
  %tmp146.i987 = insertelement <4 x float> %tmp145.i986, float %tmp144.i985, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146.i987, <4 x float>* %zi0.i821
  %tmp147.i988 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx148.i989 = getelementptr inbounds float addrspace(3)* %tmp147.i988, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp149.i990 = load float addrspace(3)* %arrayidx148.i989 ; <float> [#uses=1]
  %tmp150.i991 = load <4 x float>* %zi1.i822      ; <<4 x float>> [#uses=1]
  %tmp151.i992 = insertelement <4 x float> %tmp150.i991, float %tmp149.i990, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151.i992, <4 x float>* %zi1.i822
  %tmp152.i993 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx153.i994 = getelementptr inbounds float addrspace(3)* %tmp152.i993, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp154.i995 = load float addrspace(3)* %arrayidx153.i994 ; <float> [#uses=1]
  %tmp155.i996 = load <4 x float>* %zi2.i823      ; <<4 x float>> [#uses=1]
  %tmp156.i997 = insertelement <4 x float> %tmp155.i996, float %tmp154.i995, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156.i997, <4 x float>* %zi2.i823
  %tmp157.i998 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx158.i999 = getelementptr inbounds float addrspace(3)* %tmp157.i998, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp159.i1000 = load float addrspace(3)* %arrayidx158.i999 ; <float> [#uses=1]
  %tmp160.i1001 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp161.i1002 = insertelement <4 x float> %tmp160.i1001, float %tmp159.i1000, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161.i1002, <4 x float>* %zi3.i824
  %tmp162.i1003 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr163.i1004 = getelementptr inbounds float addrspace(3)* %tmp162.i1003, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163.i1004, float addrspace(3)** %lp.i816
  %tmp164.i1005 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx165.i1006 = getelementptr inbounds float addrspace(3)* %tmp164.i1005, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166.i1007 = load float addrspace(3)* %arrayidx165.i1006 ; <float> [#uses=1]
  %tmp167.i1008 = load <4 x float>* %zi0.i821     ; <<4 x float>> [#uses=1]
  %tmp168.i1009 = insertelement <4 x float> %tmp167.i1008, float %tmp166.i1007, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168.i1009, <4 x float>* %zi0.i821
  %tmp169.i1010 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx170.i1011 = getelementptr inbounds float addrspace(3)* %tmp169.i1010, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp171.i1012 = load float addrspace(3)* %arrayidx170.i1011 ; <float> [#uses=1]
  %tmp172.i1013 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp173.i1014 = insertelement <4 x float> %tmp172.i1013, float %tmp171.i1012, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173.i1014, <4 x float>* %zi1.i822
  %tmp174.i1015 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx175.i1016 = getelementptr inbounds float addrspace(3)* %tmp174.i1015, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp176.i1017 = load float addrspace(3)* %arrayidx175.i1016 ; <float> [#uses=1]
  %tmp177.i1018 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %tmp178.i1019 = insertelement <4 x float> %tmp177.i1018, float %tmp176.i1017, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178.i1019, <4 x float>* %zi2.i823
  %tmp179.i1020 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx180.i1021 = getelementptr inbounds float addrspace(3)* %tmp179.i1020, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp181.i1022 = load float addrspace(3)* %arrayidx180.i1021 ; <float> [#uses=1]
  %tmp182.i1023 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp183.i1024 = insertelement <4 x float> %tmp182.i1023, float %tmp181.i1022, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183.i1024, <4 x float>* %zi3.i824
  %tmp185.i1025 = load <4 x float>* %zr0.i817     ; <<4 x float>> [#uses=1]
  %tmp186.i1026 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %add187.i1027 = fadd <4 x float> %tmp185.i1025, %tmp186.i1026 ; <<4 x float>> [#uses=1]
  store <4 x float> %add187.i1027, <4 x float>* %ar0.i825
  %tmp189.i1028 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp190.i1029 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %add191.i1030 = fadd <4 x float> %tmp189.i1028, %tmp190.i1029 ; <<4 x float>> [#uses=1]
  store <4 x float> %add191.i1030, <4 x float>* %ar2.i826
  %tmp193.i1031 = load <4 x float>* %ar0.i825     ; <<4 x float>> [#uses=1]
  %tmp194.i1032 = load <4 x float>* %ar2.i826     ; <<4 x float>> [#uses=1]
  %add195.i1033 = fadd <4 x float> %tmp193.i1031, %tmp194.i1032 ; <<4 x float>> [#uses=1]
  store <4 x float> %add195.i1033, <4 x float>* %br0.i827
  %tmp197.i1034 = load <4 x float>* %zr0.i817     ; <<4 x float>> [#uses=1]
  %tmp198.i1035 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %sub.i1036 = fsub <4 x float> %tmp197.i1034, %tmp198.i1035 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub.i1036, <4 x float>* %br1.i828
  %tmp200.i1037 = load <4 x float>* %ar0.i825     ; <<4 x float>> [#uses=1]
  %tmp201.i1038 = load <4 x float>* %ar2.i826     ; <<4 x float>> [#uses=1]
  %sub202.i1039 = fsub <4 x float> %tmp200.i1037, %tmp201.i1038 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202.i1039, <4 x float>* %br2.i829
  %tmp204.i1040 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp205.i1041 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %sub206.i1042 = fsub <4 x float> %tmp204.i1040, %tmp205.i1041 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206.i1042, <4 x float>* %br3.i830
  %tmp208.i1043 = load <4 x float>* %zi0.i821     ; <<4 x float>> [#uses=1]
  %tmp209.i1044 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %add210.i1045 = fadd <4 x float> %tmp208.i1043, %tmp209.i1044 ; <<4 x float>> [#uses=1]
  store <4 x float> %add210.i1045, <4 x float>* %ai0.i831
  %tmp212.i1046 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp213.i1047 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %add214.i1048 = fadd <4 x float> %tmp212.i1046, %tmp213.i1047 ; <<4 x float>> [#uses=1]
  store <4 x float> %add214.i1048, <4 x float>* %ai2.i832
  %tmp216.i1049 = load <4 x float>* %ai0.i831     ; <<4 x float>> [#uses=1]
  %tmp217.i1050 = load <4 x float>* %ai2.i832     ; <<4 x float>> [#uses=1]
  %add218.i1051 = fadd <4 x float> %tmp216.i1049, %tmp217.i1050 ; <<4 x float>> [#uses=1]
  store <4 x float> %add218.i1051, <4 x float>* %bi0.i833
  %tmp220.i1052 = load <4 x float>* %zi0.i821     ; <<4 x float>> [#uses=1]
  %tmp221.i1053 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %sub222.i1054 = fsub <4 x float> %tmp220.i1052, %tmp221.i1053 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222.i1054, <4 x float>* %bi1.i834
  %tmp224.i1055 = load <4 x float>* %ai0.i831     ; <<4 x float>> [#uses=1]
  %tmp225.i1056 = load <4 x float>* %ai2.i832     ; <<4 x float>> [#uses=1]
  %sub226.i1057 = fsub <4 x float> %tmp224.i1055, %tmp225.i1056 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226.i1057, <4 x float>* %bi2.i835
  %tmp228.i1058 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp229.i1059 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %sub230.i1060 = fsub <4 x float> %tmp228.i1058, %tmp229.i1059 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230.i1060, <4 x float>* %bi3.i836
  %tmp231.i1061 = load <4 x float>* %br0.i827     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231.i1061, <4 x float>* %zr0.i817
  %tmp232.i1062 = load <4 x float>* %bi0.i833     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232.i1062, <4 x float>* %zi0.i821
  %tmp233.i1063 = load <4 x float>* %br1.i828     ; <<4 x float>> [#uses=1]
  %tmp234.i1064 = load <4 x float>* %bi3.i836     ; <<4 x float>> [#uses=1]
  %add235.i1065 = fadd <4 x float> %tmp233.i1063, %tmp234.i1064 ; <<4 x float>> [#uses=1]
  store <4 x float> %add235.i1065, <4 x float>* %zr1.i818
  %tmp236.i1066 = load <4 x float>* %bi1.i834     ; <<4 x float>> [#uses=1]
  %tmp237.i1067 = load <4 x float>* %br3.i830     ; <<4 x float>> [#uses=1]
  %sub238.i1068 = fsub <4 x float> %tmp236.i1066, %tmp237.i1067 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238.i1068, <4 x float>* %zi1.i822
  %tmp239.i1069 = load <4 x float>* %br1.i828     ; <<4 x float>> [#uses=1]
  %tmp240.i1070 = load <4 x float>* %bi3.i836     ; <<4 x float>> [#uses=1]
  %sub241.i1071 = fsub <4 x float> %tmp239.i1069, %tmp240.i1070 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241.i1071, <4 x float>* %zr3.i820
  %tmp242.i1072 = load <4 x float>* %br3.i830     ; <<4 x float>> [#uses=1]
  %tmp243.i1073 = load <4 x float>* %bi1.i834     ; <<4 x float>> [#uses=1]
  %add244.i1074 = fadd <4 x float> %tmp242.i1072, %tmp243.i1073 ; <<4 x float>> [#uses=1]
  store <4 x float> %add244.i1074, <4 x float>* %zi3.i824
  %tmp245.i1075 = load <4 x float>* %br2.i829     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245.i1075, <4 x float>* %zr2.i819
  %tmp246.i1076 = load <4 x float>* %bi2.i835     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246.i1076, <4 x float>* %zi2.i823
  %tmp248.i1077 = load i32* %me.addr.i814         ; <i32> [#uses=1]
  %shl.i1078 = shl i32 %tmp248.i1077, 2           ; <i32> [#uses=1]
  store i32 %shl.i1078, i32* %tbase.i837
  %tmp252.i1079 = load i32* %tbase.i837           ; <i32> [#uses=1]
  %mul.i1080 = mul i32 %tmp252.i1079, 1           ; <i32> [#uses=1]
  store i32 %mul.i1080, i32* %i.addr.i.i811
  store float* %c1.i838, float** %cretp.addr.i.i812
  %tmp.i.i1081 = load i32* %i.addr.i.i811         ; <i32> [#uses=1]
  %cmp.i.i1082 = icmp sgt i32 %tmp.i.i1081, 512   ; <i1> [#uses=1]
  br i1 %cmp.i.i1082, label %if.then.i.i1085, label %k_sincos.exit.i1094

if.then.i.i1085:                                  ; preds = %entry
  %tmp1.i.i1083 = load i32* %i.addr.i.i811        ; <i32> [#uses=1]
  %sub.i.i1084 = sub i32 %tmp1.i.i1083, 1024      ; <i32> [#uses=1]
  store i32 %sub.i.i1084, i32* %i.addr.i.i811
  br label %k_sincos.exit.i1094

k_sincos.exit.i1094:                              ; preds = %if.then.i.i1085, %entry
  %tmp3.i.i1086 = load i32* %i.addr.i.i811        ; <i32> [#uses=1]
  %conv.i.i1087 = sitofp i32 %tmp3.i.i1086 to float ; <float> [#uses=1]
  %mul.i.i1088 = fmul float %conv.i.i1087, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i.i1088, float* %x.i.i813
  %tmp4.i.i1089 = load float* %x.i.i813           ; <float> [#uses=1]
  %call.i.i1090 = call float @_Z10native_cosf(float %tmp4.i.i1089) nounwind ; <float> [#uses=1]
  %tmp5.i.i1091 = load float** %cretp.addr.i.i812 ; <float*> [#uses=1]
  store float %call.i.i1090, float* %tmp5.i.i1091
  %tmp6.i.i1092 = load float* %x.i.i813           ; <float> [#uses=1]
  %call7.i.i1093 = call float @_Z10native_sinf(float %tmp6.i.i1092) nounwind ; <float> [#uses=1]
  store float %call7.i.i1093, float* %retval.i.i810
  %5 = load float* %retval.i.i810                 ; <float> [#uses=1]
  store float %5, float* %s1.i839
  %tmp255.i1095 = load float* %c1.i838            ; <float> [#uses=1]
  %tmp256.i1096 = insertelement <4 x float> undef, float %tmp255.i1095, i32 0 ; <<4 x float>> [#uses=2]
  %splat.i1097 = shufflevector <4 x float> %tmp256.i1096, <4 x float> %tmp256.i1096, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp257.i1098 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %mul258.i = fmul <4 x float> %splat.i1097, %tmp257.i1098 ; <<4 x float>> [#uses=1]
  %tmp259.i1099 = load float* %s1.i839            ; <float> [#uses=1]
  %tmp260.i1100 = insertelement <4 x float> undef, float %tmp259.i1099, i32 0 ; <<4 x float>> [#uses=2]
  %splat261.i = shufflevector <4 x float> %tmp260.i1100, <4 x float> %tmp260.i1100, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp262.i1101 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %mul263.i = fmul <4 x float> %splat261.i, %tmp262.i1101 ; <<4 x float>> [#uses=1]
  %sub264.i = fsub <4 x float> %mul258.i, %mul263.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub264.i, <4 x float>* %__r.i840
  %tmp265.i = load float* %c1.i838                ; <float> [#uses=1]
  %tmp266.i1102 = insertelement <4 x float> undef, float %tmp265.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat267.i = shufflevector <4 x float> %tmp266.i1102, <4 x float> %tmp266.i1102, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp268.i1103 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %mul269.i = fmul <4 x float> %splat267.i, %tmp268.i1103 ; <<4 x float>> [#uses=1]
  %tmp270.i1104 = load float* %s1.i839            ; <float> [#uses=1]
  %tmp271.i1105 = insertelement <4 x float> undef, float %tmp270.i1104, i32 0 ; <<4 x float>> [#uses=2]
  %splat272.i = shufflevector <4 x float> %tmp271.i1105, <4 x float> %tmp271.i1105, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp273.i1106 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %mul274.i = fmul <4 x float> %splat272.i, %tmp273.i1106 ; <<4 x float>> [#uses=1]
  %add275.i = fadd <4 x float> %mul269.i, %mul274.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add275.i, <4 x float>* %zi1.i822
  %tmp276.i1107 = load <4 x float>* %__r.i840     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp276.i1107, <4 x float>* %zr1.i818
  %tmp280.i1108 = load i32* %tbase.i837           ; <i32> [#uses=1]
  %mul281.i1109 = mul i32 %tmp280.i1108, 2        ; <i32> [#uses=1]
  store i32 %mul281.i1109, i32* %i.addr.i511.i
  store float* %c2.i841, float** %cretp.addr.i512.i
  %tmp.i514.i = load i32* %i.addr.i511.i          ; <i32> [#uses=1]
  %cmp.i515.i = icmp sgt i32 %tmp.i514.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i515.i, label %if.then.i518.i, label %k_sincos.exit527.i

if.then.i518.i:                                   ; preds = %k_sincos.exit.i1094
  %tmp1.i516.i = load i32* %i.addr.i511.i         ; <i32> [#uses=1]
  %sub.i517.i = sub i32 %tmp1.i516.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i517.i, i32* %i.addr.i511.i
  br label %k_sincos.exit527.i

k_sincos.exit527.i:                               ; preds = %if.then.i518.i, %k_sincos.exit.i1094
  %tmp3.i519.i = load i32* %i.addr.i511.i         ; <i32> [#uses=1]
  %conv.i520.i = sitofp i32 %tmp3.i519.i to float ; <float> [#uses=1]
  %mul.i521.i = fmul float %conv.i520.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i521.i, float* %x.i513.i
  %tmp4.i522.i = load float* %x.i513.i            ; <float> [#uses=1]
  %call.i523.i = call float @_Z10native_cosf(float %tmp4.i522.i) nounwind ; <float> [#uses=1]
  %tmp5.i524.i = load float** %cretp.addr.i512.i  ; <float*> [#uses=1]
  store float %call.i523.i, float* %tmp5.i524.i
  %tmp6.i525.i = load float* %x.i513.i            ; <float> [#uses=1]
  %call7.i526.i = call float @_Z10native_sinf(float %tmp6.i525.i) nounwind ; <float> [#uses=1]
  store float %call7.i526.i, float* %retval.i510.i
  %6 = load float* %retval.i510.i                 ; <float> [#uses=1]
  store float %6, float* %s2.i842
  %tmp286.i1110 = load float* %c2.i841            ; <float> [#uses=1]
  %tmp287.i1111 = insertelement <4 x float> undef, float %tmp286.i1110, i32 0 ; <<4 x float>> [#uses=2]
  %splat288.i = shufflevector <4 x float> %tmp287.i1111, <4 x float> %tmp287.i1111, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp289.i = load <4 x float>* %zr2.i819         ; <<4 x float>> [#uses=1]
  %mul290.i = fmul <4 x float> %splat288.i, %tmp289.i ; <<4 x float>> [#uses=1]
  %tmp291.i1112 = load float* %s2.i842            ; <float> [#uses=1]
  %tmp292.i1113 = insertelement <4 x float> undef, float %tmp291.i1112, i32 0 ; <<4 x float>> [#uses=2]
  %splat293.i = shufflevector <4 x float> %tmp292.i1113, <4 x float> %tmp292.i1113, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp294.i1114 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %mul295.i = fmul <4 x float> %splat293.i, %tmp294.i1114 ; <<4 x float>> [#uses=1]
  %sub296.i = fsub <4 x float> %mul290.i, %mul295.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub296.i, <4 x float>* %__r285.i
  %tmp297.i = load float* %c2.i841                ; <float> [#uses=1]
  %tmp298.i1115 = insertelement <4 x float> undef, float %tmp297.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat299.i = shufflevector <4 x float> %tmp298.i1115, <4 x float> %tmp298.i1115, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp300.i1116 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %mul301.i = fmul <4 x float> %splat299.i, %tmp300.i1116 ; <<4 x float>> [#uses=1]
  %tmp302.i1117 = load float* %s2.i842            ; <float> [#uses=1]
  %tmp303.i1118 = insertelement <4 x float> undef, float %tmp302.i1117, i32 0 ; <<4 x float>> [#uses=2]
  %splat304.i = shufflevector <4 x float> %tmp303.i1118, <4 x float> %tmp303.i1118, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp305.i1119 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %mul306.i = fmul <4 x float> %splat304.i, %tmp305.i1119 ; <<4 x float>> [#uses=1]
  %add307.i = fadd <4 x float> %mul301.i, %mul306.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add307.i, <4 x float>* %zi2.i823
  %tmp308.i1120 = load <4 x float>* %__r285.i     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp308.i1120, <4 x float>* %zr2.i819
  %tmp312.i1121 = load i32* %tbase.i837           ; <i32> [#uses=1]
  %mul313.i1122 = mul i32 %tmp312.i1121, 3        ; <i32> [#uses=1]
  store i32 %mul313.i1122, i32* %i.addr.i493.i
  store float* %c3.i843, float** %cretp.addr.i494.i
  %tmp.i496.i = load i32* %i.addr.i493.i          ; <i32> [#uses=1]
  %cmp.i497.i = icmp sgt i32 %tmp.i496.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i497.i, label %if.then.i500.i, label %kfft_pass2.exit

if.then.i500.i:                                   ; preds = %k_sincos.exit527.i
  %tmp1.i498.i = load i32* %i.addr.i493.i         ; <i32> [#uses=1]
  %sub.i499.i = sub i32 %tmp1.i498.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i499.i, i32* %i.addr.i493.i
  br label %kfft_pass2.exit

kfft_pass2.exit:                                  ; preds = %k_sincos.exit527.i, %if.then.i500.i
  %tmp3.i501.i = load i32* %i.addr.i493.i         ; <i32> [#uses=1]
  %conv.i502.i = sitofp i32 %tmp3.i501.i to float ; <float> [#uses=1]
  %mul.i503.i = fmul float %conv.i502.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i503.i, float* %x.i495.i
  %tmp4.i504.i = load float* %x.i495.i            ; <float> [#uses=1]
  %call.i505.i = call float @_Z10native_cosf(float %tmp4.i504.i) nounwind ; <float> [#uses=1]
  %tmp5.i506.i = load float** %cretp.addr.i494.i  ; <float*> [#uses=1]
  store float %call.i505.i, float* %tmp5.i506.i
  %tmp6.i507.i = load float* %x.i495.i            ; <float> [#uses=1]
  %call7.i508.i = call float @_Z10native_sinf(float %tmp6.i507.i) nounwind ; <float> [#uses=1]
  store float %call7.i508.i, float* %retval.i492.i
  %7 = load float* %retval.i492.i                 ; <float> [#uses=1]
  store float %7, float* %s3.i844
  %tmp318.i1123 = load float* %c3.i843            ; <float> [#uses=1]
  %tmp319.i1124 = insertelement <4 x float> undef, float %tmp318.i1123, i32 0 ; <<4 x float>> [#uses=2]
  %splat320.i = shufflevector <4 x float> %tmp319.i1124, <4 x float> %tmp319.i1124, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp321.i1125 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %mul322.i = fmul <4 x float> %splat320.i, %tmp321.i1125 ; <<4 x float>> [#uses=1]
  %tmp323.i = load float* %s3.i844                ; <float> [#uses=1]
  %tmp324.i1126 = insertelement <4 x float> undef, float %tmp323.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat325.i = shufflevector <4 x float> %tmp324.i1126, <4 x float> %tmp324.i1126, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp326.i1127 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %mul327.i = fmul <4 x float> %splat325.i, %tmp326.i1127 ; <<4 x float>> [#uses=1]
  %sub328.i = fsub <4 x float> %mul322.i, %mul327.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub328.i, <4 x float>* %__r317.i
  %tmp329.i = load float* %c3.i843                ; <float> [#uses=1]
  %tmp330.i1128 = insertelement <4 x float> undef, float %tmp329.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat331.i = shufflevector <4 x float> %tmp330.i1128, <4 x float> %tmp330.i1128, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp332.i = load <4 x float>* %zi3.i824         ; <<4 x float>> [#uses=1]
  %mul333.i = fmul <4 x float> %splat331.i, %tmp332.i ; <<4 x float>> [#uses=1]
  %tmp334.i = load float* %s3.i844                ; <float> [#uses=1]
  %tmp335.i1129 = insertelement <4 x float> undef, float %tmp334.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat336.i = shufflevector <4 x float> %tmp335.i1129, <4 x float> %tmp335.i1129, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp337.i1130 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %mul338.i = fmul <4 x float> %splat336.i, %tmp337.i1130 ; <<4 x float>> [#uses=1]
  %add339.i = fadd <4 x float> %mul333.i, %mul338.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add339.i, <4 x float>* %zi3.i824
  %tmp340.i = load <4 x float>* %__r317.i         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp340.i, <4 x float>* %zr3.i820
  call void @barrier(i32 1) nounwind
  %tmp343.i = load float addrspace(3)** %lds.addr.i815 ; <float addrspace(3)*> [#uses=1]
  %tmp344.i1131 = load i32* %me.addr.i814         ; <i32> [#uses=1]
  %shl345.i = shl i32 %tmp344.i1131, 2            ; <i32> [#uses=1]
  %tmp346.i = load i32* %me.addr.i814             ; <i32> [#uses=1]
  %shr347.i = lshr i32 %tmp346.i, 3               ; <i32> [#uses=1]
  %add348.i = add i32 %shl345.i, %shr347.i        ; <i32> [#uses=1]
  %add.ptr349.i = getelementptr inbounds float addrspace(3)* %tmp343.i, i32 %add348.i ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr349.i, float addrspace(3)** %lp.i816
  %tmp350.i1132 = load <4 x float>* %zr0.i817     ; <<4 x float>> [#uses=1]
  %tmp351.i1133 = extractelement <4 x float> %tmp350.i1132, i32 0 ; <float> [#uses=1]
  %tmp352.i1134 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx353.i = getelementptr inbounds float addrspace(3)* %tmp352.i1134, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp351.i1133, float addrspace(3)* %arrayidx353.i
  %tmp354.i1135 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp355.i1136 = extractelement <4 x float> %tmp354.i1135, i32 0 ; <float> [#uses=1]
  %tmp356.i1137 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx357.i = getelementptr inbounds float addrspace(3)* %tmp356.i1137, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp355.i1136, float addrspace(3)* %arrayidx357.i
  %tmp358.i1138 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %tmp359.i1139 = extractelement <4 x float> %tmp358.i1138, i32 0 ; <float> [#uses=1]
  %tmp360.i1140 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx361.i = getelementptr inbounds float addrspace(3)* %tmp360.i1140, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp359.i1139, float addrspace(3)* %arrayidx361.i
  %tmp362.i1141 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %tmp363.i1142 = extractelement <4 x float> %tmp362.i1141, i32 0 ; <float> [#uses=1]
  %tmp364.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx365.i = getelementptr inbounds float addrspace(3)* %tmp364.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp363.i1142, float addrspace(3)* %arrayidx365.i
  %tmp366.i1143 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr367.i = getelementptr inbounds float addrspace(3)* %tmp366.i1143, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr367.i, float addrspace(3)** %lp.i816
  %tmp368.i = load <4 x float>* %zr0.i817         ; <<4 x float>> [#uses=1]
  %tmp369.i1144 = extractelement <4 x float> %tmp368.i, i32 1 ; <float> [#uses=1]
  %tmp370.i1145 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx371.i = getelementptr inbounds float addrspace(3)* %tmp370.i1145, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp369.i1144, float addrspace(3)* %arrayidx371.i
  %tmp372.i1146 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp373.i1147 = extractelement <4 x float> %tmp372.i1146, i32 1 ; <float> [#uses=1]
  %tmp374.i1148 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx375.i = getelementptr inbounds float addrspace(3)* %tmp374.i1148, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp373.i1147, float addrspace(3)* %arrayidx375.i
  %tmp376.i1149 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %tmp377.i1150 = extractelement <4 x float> %tmp376.i1149, i32 1 ; <float> [#uses=1]
  %tmp378.i1151 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx379.i = getelementptr inbounds float addrspace(3)* %tmp378.i1151, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp377.i1150, float addrspace(3)* %arrayidx379.i
  %tmp380.i1152 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %tmp381.i1153 = extractelement <4 x float> %tmp380.i1152, i32 1 ; <float> [#uses=1]
  %tmp382.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx383.i = getelementptr inbounds float addrspace(3)* %tmp382.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp381.i1153, float addrspace(3)* %arrayidx383.i
  %tmp384.i1154 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr385.i = getelementptr inbounds float addrspace(3)* %tmp384.i1154, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr385.i, float addrspace(3)** %lp.i816
  %tmp386.i = load <4 x float>* %zr0.i817         ; <<4 x float>> [#uses=1]
  %tmp387.i1155 = extractelement <4 x float> %tmp386.i, i32 2 ; <float> [#uses=1]
  %tmp388.i1156 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx389.i = getelementptr inbounds float addrspace(3)* %tmp388.i1156, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp387.i1155, float addrspace(3)* %arrayidx389.i
  %tmp390.i1157 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp391.i1158 = extractelement <4 x float> %tmp390.i1157, i32 2 ; <float> [#uses=1]
  %tmp392.i1159 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx393.i = getelementptr inbounds float addrspace(3)* %tmp392.i1159, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp391.i1158, float addrspace(3)* %arrayidx393.i
  %tmp394.i1160 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %tmp395.i1161 = extractelement <4 x float> %tmp394.i1160, i32 2 ; <float> [#uses=1]
  %tmp396.i1162 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx397.i = getelementptr inbounds float addrspace(3)* %tmp396.i1162, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp395.i1161, float addrspace(3)* %arrayidx397.i
  %tmp398.i1163 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %tmp399.i1164 = extractelement <4 x float> %tmp398.i1163, i32 2 ; <float> [#uses=1]
  %tmp400.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx401.i = getelementptr inbounds float addrspace(3)* %tmp400.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp399.i1164, float addrspace(3)* %arrayidx401.i
  %tmp402.i1165 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr403.i = getelementptr inbounds float addrspace(3)* %tmp402.i1165, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr403.i, float addrspace(3)** %lp.i816
  %tmp404.i = load <4 x float>* %zr0.i817         ; <<4 x float>> [#uses=1]
  %tmp405.i1166 = extractelement <4 x float> %tmp404.i, i32 3 ; <float> [#uses=1]
  %tmp406.i1167 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx407.i = getelementptr inbounds float addrspace(3)* %tmp406.i1167, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp405.i1166, float addrspace(3)* %arrayidx407.i
  %tmp408.i1168 = load <4 x float>* %zr1.i818     ; <<4 x float>> [#uses=1]
  %tmp409.i1169 = extractelement <4 x float> %tmp408.i1168, i32 3 ; <float> [#uses=1]
  %tmp410.i1170 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx411.i = getelementptr inbounds float addrspace(3)* %tmp410.i1170, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp409.i1169, float addrspace(3)* %arrayidx411.i
  %tmp412.i1171 = load <4 x float>* %zr2.i819     ; <<4 x float>> [#uses=1]
  %tmp413.i1172 = extractelement <4 x float> %tmp412.i1171, i32 3 ; <float> [#uses=1]
  %tmp414.i1173 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx415.i = getelementptr inbounds float addrspace(3)* %tmp414.i1173, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp413.i1172, float addrspace(3)* %arrayidx415.i
  %tmp416.i1174 = load <4 x float>* %zr3.i820     ; <<4 x float>> [#uses=1]
  %tmp417.i1175 = extractelement <4 x float> %tmp416.i1174, i32 3 ; <float> [#uses=1]
  %tmp418.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx419.i = getelementptr inbounds float addrspace(3)* %tmp418.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp417.i1175, float addrspace(3)* %arrayidx419.i
  %tmp420.i1176 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr421.i = getelementptr inbounds float addrspace(3)* %tmp420.i1176, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr421.i, float addrspace(3)** %lp.i816
  %tmp422.i = load <4 x float>* %zi0.i821         ; <<4 x float>> [#uses=1]
  %tmp423.i1177 = extractelement <4 x float> %tmp422.i, i32 0 ; <float> [#uses=1]
  %tmp424.i1178 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx425.i = getelementptr inbounds float addrspace(3)* %tmp424.i1178, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp423.i1177, float addrspace(3)* %arrayidx425.i
  %tmp426.i1179 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp427.i1180 = extractelement <4 x float> %tmp426.i1179, i32 0 ; <float> [#uses=1]
  %tmp428.i1181 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx429.i = getelementptr inbounds float addrspace(3)* %tmp428.i1181, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp427.i1180, float addrspace(3)* %arrayidx429.i
  %tmp430.i1182 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %tmp431.i1183 = extractelement <4 x float> %tmp430.i1182, i32 0 ; <float> [#uses=1]
  %tmp432.i1184 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx433.i = getelementptr inbounds float addrspace(3)* %tmp432.i1184, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp431.i1183, float addrspace(3)* %arrayidx433.i
  %tmp434.i1185 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp435.i1186 = extractelement <4 x float> %tmp434.i1185, i32 0 ; <float> [#uses=1]
  %tmp436.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx437.i = getelementptr inbounds float addrspace(3)* %tmp436.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp435.i1186, float addrspace(3)* %arrayidx437.i
  %tmp438.i1187 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr439.i = getelementptr inbounds float addrspace(3)* %tmp438.i1187, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr439.i, float addrspace(3)** %lp.i816
  %tmp440.i = load <4 x float>* %zi0.i821         ; <<4 x float>> [#uses=1]
  %tmp441.i1188 = extractelement <4 x float> %tmp440.i, i32 1 ; <float> [#uses=1]
  %tmp442.i1189 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx443.i = getelementptr inbounds float addrspace(3)* %tmp442.i1189, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp441.i1188, float addrspace(3)* %arrayidx443.i
  %tmp444.i1190 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp445.i1191 = extractelement <4 x float> %tmp444.i1190, i32 1 ; <float> [#uses=1]
  %tmp446.i1192 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx447.i = getelementptr inbounds float addrspace(3)* %tmp446.i1192, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp445.i1191, float addrspace(3)* %arrayidx447.i
  %tmp448.i1193 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %tmp449.i1194 = extractelement <4 x float> %tmp448.i1193, i32 1 ; <float> [#uses=1]
  %tmp450.i1195 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx451.i = getelementptr inbounds float addrspace(3)* %tmp450.i1195, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp449.i1194, float addrspace(3)* %arrayidx451.i
  %tmp452.i1196 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp453.i1197 = extractelement <4 x float> %tmp452.i1196, i32 1 ; <float> [#uses=1]
  %tmp454.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx455.i = getelementptr inbounds float addrspace(3)* %tmp454.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp453.i1197, float addrspace(3)* %arrayidx455.i
  %tmp456.i1198 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr457.i = getelementptr inbounds float addrspace(3)* %tmp456.i1198, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr457.i, float addrspace(3)** %lp.i816
  %tmp458.i = load <4 x float>* %zi0.i821         ; <<4 x float>> [#uses=1]
  %tmp459.i1199 = extractelement <4 x float> %tmp458.i, i32 2 ; <float> [#uses=1]
  %tmp460.i1200 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx461.i = getelementptr inbounds float addrspace(3)* %tmp460.i1200, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp459.i1199, float addrspace(3)* %arrayidx461.i
  %tmp462.i1201 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp463.i1202 = extractelement <4 x float> %tmp462.i1201, i32 2 ; <float> [#uses=1]
  %tmp464.i1203 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx465.i = getelementptr inbounds float addrspace(3)* %tmp464.i1203, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp463.i1202, float addrspace(3)* %arrayidx465.i
  %tmp466.i1204 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %tmp467.i1205 = extractelement <4 x float> %tmp466.i1204, i32 2 ; <float> [#uses=1]
  %tmp468.i1206 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx469.i = getelementptr inbounds float addrspace(3)* %tmp468.i1206, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp467.i1205, float addrspace(3)* %arrayidx469.i
  %tmp470.i1207 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp471.i1208 = extractelement <4 x float> %tmp470.i1207, i32 2 ; <float> [#uses=1]
  %tmp472.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx473.i = getelementptr inbounds float addrspace(3)* %tmp472.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp471.i1208, float addrspace(3)* %arrayidx473.i
  %tmp474.i1209 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %add.ptr475.i = getelementptr inbounds float addrspace(3)* %tmp474.i1209, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr475.i, float addrspace(3)** %lp.i816
  %tmp476.i = load <4 x float>* %zi0.i821         ; <<4 x float>> [#uses=1]
  %tmp477.i1210 = extractelement <4 x float> %tmp476.i, i32 3 ; <float> [#uses=1]
  %tmp478.i1211 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx479.i = getelementptr inbounds float addrspace(3)* %tmp478.i1211, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp477.i1210, float addrspace(3)* %arrayidx479.i
  %tmp480.i1212 = load <4 x float>* %zi1.i822     ; <<4 x float>> [#uses=1]
  %tmp481.i1213 = extractelement <4 x float> %tmp480.i1212, i32 3 ; <float> [#uses=1]
  %tmp482.i1214 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx483.i = getelementptr inbounds float addrspace(3)* %tmp482.i1214, i32 1 ; <float addrspace(3)*> [#uses=1]
  store float %tmp481.i1213, float addrspace(3)* %arrayidx483.i
  %tmp484.i1215 = load <4 x float>* %zi2.i823     ; <<4 x float>> [#uses=1]
  %tmp485.i1216 = extractelement <4 x float> %tmp484.i1215, i32 3 ; <float> [#uses=1]
  %tmp486.i1217 = load float addrspace(3)** %lp.i816 ; <float addrspace(3)*> [#uses=1]
  %arrayidx487.i = getelementptr inbounds float addrspace(3)* %tmp486.i1217, i32 2 ; <float addrspace(3)*> [#uses=1]
  store float %tmp485.i1216, float addrspace(3)* %arrayidx487.i
  %tmp488.i1218 = load <4 x float>* %zi3.i824     ; <<4 x float>> [#uses=1]
  %tmp489.i1219 = extractelement <4 x float> %tmp488.i1218, i32 3 ; <float> [#uses=1]
  %tmp490.i = load float addrspace(3)** %lp.i816  ; <float addrspace(3)*> [#uses=1]
  %arrayidx491.i = getelementptr inbounds float addrspace(3)* %tmp490.i, i32 3 ; <float addrspace(3)*> [#uses=1]
  store float %tmp489.i1219, float addrspace(3)* %arrayidx491.i
  call void @barrier(i32 1) nounwind
  %tmp12 = load i32* %me                          ; <i32> [#uses=1]
  store i32 %tmp12, i32* %me.addr.i400
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %lds.addr.i401
  %tmp.i431 = load float addrspace(3)** %lds.addr.i401 ; <float addrspace(3)*> [#uses=1]
  %tmp1.i432 = load i32* %me.addr.i400            ; <i32> [#uses=1]
  %tmp2.i433 = load i32* %me.addr.i400            ; <i32> [#uses=1]
  %shr.i434 = lshr i32 %tmp2.i433, 5              ; <i32> [#uses=1]
  %add.i435 = add i32 %tmp1.i432, %shr.i434       ; <i32> [#uses=1]
  %add.ptr.i436 = getelementptr inbounds float addrspace(3)* %tmp.i431, i32 %add.i435 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr.i436, float addrspace(3)** %lp.i402
  %tmp7.i437 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx.i438 = getelementptr inbounds float addrspace(3)* %tmp7.i437, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8.i439 = load float addrspace(3)* %arrayidx.i438 ; <float> [#uses=1]
  %tmp9.i440 = load <4 x float>* %zr0.i403        ; <<4 x float>> [#uses=1]
  %tmp10.i441 = insertelement <4 x float> %tmp9.i440, float %tmp8.i439, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10.i441, <4 x float>* %zr0.i403
  %tmp11.i442 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx12.i443 = getelementptr inbounds float addrspace(3)* %tmp11.i442, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp13.i444 = load float addrspace(3)* %arrayidx12.i443 ; <float> [#uses=1]
  %tmp14.i445 = load <4 x float>* %zr1.i404       ; <<4 x float>> [#uses=1]
  %tmp15.i446 = insertelement <4 x float> %tmp14.i445, float %tmp13.i444, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15.i446, <4 x float>* %zr1.i404
  %tmp16.i447 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx17.i448 = getelementptr inbounds float addrspace(3)* %tmp16.i447, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp18.i449 = load float addrspace(3)* %arrayidx17.i448 ; <float> [#uses=1]
  %tmp19.i450 = load <4 x float>* %zr2.i405       ; <<4 x float>> [#uses=1]
  %tmp20.i451 = insertelement <4 x float> %tmp19.i450, float %tmp18.i449, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20.i451, <4 x float>* %zr2.i405
  %tmp21.i452 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx22.i453 = getelementptr inbounds float addrspace(3)* %tmp21.i452, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp23.i454 = load float addrspace(3)* %arrayidx22.i453 ; <float> [#uses=1]
  %tmp24.i455 = load <4 x float>* %zr3.i406       ; <<4 x float>> [#uses=1]
  %tmp25.i456 = insertelement <4 x float> %tmp24.i455, float %tmp23.i454, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25.i456, <4 x float>* %zr3.i406
  %tmp26.i457 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr27.i458 = getelementptr inbounds float addrspace(3)* %tmp26.i457, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27.i458, float addrspace(3)** %lp.i402
  %tmp28.i459 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx29.i460 = getelementptr inbounds float addrspace(3)* %tmp28.i459, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30.i461 = load float addrspace(3)* %arrayidx29.i460 ; <float> [#uses=1]
  %tmp31.i462 = load <4 x float>* %zr0.i403       ; <<4 x float>> [#uses=1]
  %tmp32.i463 = insertelement <4 x float> %tmp31.i462, float %tmp30.i461, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32.i463, <4 x float>* %zr0.i403
  %tmp33.i464 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx34.i465 = getelementptr inbounds float addrspace(3)* %tmp33.i464, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp35.i466 = load float addrspace(3)* %arrayidx34.i465 ; <float> [#uses=1]
  %tmp36.i467 = load <4 x float>* %zr1.i404       ; <<4 x float>> [#uses=1]
  %tmp37.i468 = insertelement <4 x float> %tmp36.i467, float %tmp35.i466, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37.i468, <4 x float>* %zr1.i404
  %tmp38.i469 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx39.i470 = getelementptr inbounds float addrspace(3)* %tmp38.i469, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp40.i471 = load float addrspace(3)* %arrayidx39.i470 ; <float> [#uses=1]
  %tmp41.i472 = load <4 x float>* %zr2.i405       ; <<4 x float>> [#uses=1]
  %tmp42.i473 = insertelement <4 x float> %tmp41.i472, float %tmp40.i471, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42.i473, <4 x float>* %zr2.i405
  %tmp43.i474 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx44.i475 = getelementptr inbounds float addrspace(3)* %tmp43.i474, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp45.i476 = load float addrspace(3)* %arrayidx44.i475 ; <float> [#uses=1]
  %tmp46.i477 = load <4 x float>* %zr3.i406       ; <<4 x float>> [#uses=1]
  %tmp47.i478 = insertelement <4 x float> %tmp46.i477, float %tmp45.i476, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47.i478, <4 x float>* %zr3.i406
  %tmp48.i479 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr49.i480 = getelementptr inbounds float addrspace(3)* %tmp48.i479, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49.i480, float addrspace(3)** %lp.i402
  %tmp50.i481 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx51.i482 = getelementptr inbounds float addrspace(3)* %tmp50.i481, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52.i483 = load float addrspace(3)* %arrayidx51.i482 ; <float> [#uses=1]
  %tmp53.i484 = load <4 x float>* %zr0.i403       ; <<4 x float>> [#uses=1]
  %tmp54.i485 = insertelement <4 x float> %tmp53.i484, float %tmp52.i483, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54.i485, <4 x float>* %zr0.i403
  %tmp55.i486 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx56.i487 = getelementptr inbounds float addrspace(3)* %tmp55.i486, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp57.i488 = load float addrspace(3)* %arrayidx56.i487 ; <float> [#uses=1]
  %tmp58.i489 = load <4 x float>* %zr1.i404       ; <<4 x float>> [#uses=1]
  %tmp59.i490 = insertelement <4 x float> %tmp58.i489, float %tmp57.i488, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59.i490, <4 x float>* %zr1.i404
  %tmp60.i491 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx61.i492 = getelementptr inbounds float addrspace(3)* %tmp60.i491, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp62.i493 = load float addrspace(3)* %arrayidx61.i492 ; <float> [#uses=1]
  %tmp63.i494 = load <4 x float>* %zr2.i405       ; <<4 x float>> [#uses=1]
  %tmp64.i495 = insertelement <4 x float> %tmp63.i494, float %tmp62.i493, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64.i495, <4 x float>* %zr2.i405
  %tmp65.i496 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx66.i497 = getelementptr inbounds float addrspace(3)* %tmp65.i496, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp67.i498 = load float addrspace(3)* %arrayidx66.i497 ; <float> [#uses=1]
  %tmp68.i499 = load <4 x float>* %zr3.i406       ; <<4 x float>> [#uses=1]
  %tmp69.i500 = insertelement <4 x float> %tmp68.i499, float %tmp67.i498, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69.i500, <4 x float>* %zr3.i406
  %tmp70.i501 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr71.i502 = getelementptr inbounds float addrspace(3)* %tmp70.i501, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71.i502, float addrspace(3)** %lp.i402
  %tmp72.i503 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx73.i504 = getelementptr inbounds float addrspace(3)* %tmp72.i503, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74.i505 = load float addrspace(3)* %arrayidx73.i504 ; <float> [#uses=1]
  %tmp75.i506 = load <4 x float>* %zr0.i403       ; <<4 x float>> [#uses=1]
  %tmp76.i507 = insertelement <4 x float> %tmp75.i506, float %tmp74.i505, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76.i507, <4 x float>* %zr0.i403
  %tmp77.i508 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx78.i509 = getelementptr inbounds float addrspace(3)* %tmp77.i508, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp79.i510 = load float addrspace(3)* %arrayidx78.i509 ; <float> [#uses=1]
  %tmp80.i511 = load <4 x float>* %zr1.i404       ; <<4 x float>> [#uses=1]
  %tmp81.i512 = insertelement <4 x float> %tmp80.i511, float %tmp79.i510, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81.i512, <4 x float>* %zr1.i404
  %tmp82.i513 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx83.i514 = getelementptr inbounds float addrspace(3)* %tmp82.i513, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp84.i515 = load float addrspace(3)* %arrayidx83.i514 ; <float> [#uses=1]
  %tmp85.i516 = load <4 x float>* %zr2.i405       ; <<4 x float>> [#uses=1]
  %tmp86.i517 = insertelement <4 x float> %tmp85.i516, float %tmp84.i515, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86.i517, <4 x float>* %zr2.i405
  %tmp87.i518 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx88.i519 = getelementptr inbounds float addrspace(3)* %tmp87.i518, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp89.i520 = load float addrspace(3)* %arrayidx88.i519 ; <float> [#uses=1]
  %tmp90.i521 = load <4 x float>* %zr3.i406       ; <<4 x float>> [#uses=1]
  %tmp91.i522 = insertelement <4 x float> %tmp90.i521, float %tmp89.i520, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91.i522, <4 x float>* %zr3.i406
  %tmp92.i523 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr93.i524 = getelementptr inbounds float addrspace(3)* %tmp92.i523, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93.i524, float addrspace(3)** %lp.i402
  %tmp98.i525 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx99.i526 = getelementptr inbounds float addrspace(3)* %tmp98.i525, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100.i527 = load float addrspace(3)* %arrayidx99.i526 ; <float> [#uses=1]
  %tmp101.i528 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp102.i529 = insertelement <4 x float> %tmp101.i528, float %tmp100.i527, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102.i529, <4 x float>* %zi0.i407
  %tmp103.i530 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx104.i531 = getelementptr inbounds float addrspace(3)* %tmp103.i530, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp105.i532 = load float addrspace(3)* %arrayidx104.i531 ; <float> [#uses=1]
  %tmp106.i533 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp107.i534 = insertelement <4 x float> %tmp106.i533, float %tmp105.i532, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107.i534, <4 x float>* %zi1.i408
  %tmp108.i535 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx109.i536 = getelementptr inbounds float addrspace(3)* %tmp108.i535, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp110.i537 = load float addrspace(3)* %arrayidx109.i536 ; <float> [#uses=1]
  %tmp111.i538 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp112.i539 = insertelement <4 x float> %tmp111.i538, float %tmp110.i537, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112.i539, <4 x float>* %zi2.i409
  %tmp113.i540 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx114.i541 = getelementptr inbounds float addrspace(3)* %tmp113.i540, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp115.i542 = load float addrspace(3)* %arrayidx114.i541 ; <float> [#uses=1]
  %tmp116.i543 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp117.i544 = insertelement <4 x float> %tmp116.i543, float %tmp115.i542, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117.i544, <4 x float>* %zi3.i410
  %tmp118.i545 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr119.i546 = getelementptr inbounds float addrspace(3)* %tmp118.i545, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119.i546, float addrspace(3)** %lp.i402
  %tmp120.i547 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx121.i548 = getelementptr inbounds float addrspace(3)* %tmp120.i547, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122.i549 = load float addrspace(3)* %arrayidx121.i548 ; <float> [#uses=1]
  %tmp123.i550 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp124.i551 = insertelement <4 x float> %tmp123.i550, float %tmp122.i549, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124.i551, <4 x float>* %zi0.i407
  %tmp125.i552 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx126.i553 = getelementptr inbounds float addrspace(3)* %tmp125.i552, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp127.i554 = load float addrspace(3)* %arrayidx126.i553 ; <float> [#uses=1]
  %tmp128.i555 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp129.i556 = insertelement <4 x float> %tmp128.i555, float %tmp127.i554, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129.i556, <4 x float>* %zi1.i408
  %tmp130.i557 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx131.i558 = getelementptr inbounds float addrspace(3)* %tmp130.i557, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp132.i559 = load float addrspace(3)* %arrayidx131.i558 ; <float> [#uses=1]
  %tmp133.i560 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp134.i561 = insertelement <4 x float> %tmp133.i560, float %tmp132.i559, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134.i561, <4 x float>* %zi2.i409
  %tmp135.i562 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx136.i563 = getelementptr inbounds float addrspace(3)* %tmp135.i562, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp137.i564 = load float addrspace(3)* %arrayidx136.i563 ; <float> [#uses=1]
  %tmp138.i565 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp139.i566 = insertelement <4 x float> %tmp138.i565, float %tmp137.i564, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139.i566, <4 x float>* %zi3.i410
  %tmp140.i567 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr141.i568 = getelementptr inbounds float addrspace(3)* %tmp140.i567, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141.i568, float addrspace(3)** %lp.i402
  %tmp142.i569 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx143.i570 = getelementptr inbounds float addrspace(3)* %tmp142.i569, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144.i571 = load float addrspace(3)* %arrayidx143.i570 ; <float> [#uses=1]
  %tmp145.i572 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp146.i573 = insertelement <4 x float> %tmp145.i572, float %tmp144.i571, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146.i573, <4 x float>* %zi0.i407
  %tmp147.i574 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx148.i575 = getelementptr inbounds float addrspace(3)* %tmp147.i574, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp149.i576 = load float addrspace(3)* %arrayidx148.i575 ; <float> [#uses=1]
  %tmp150.i577 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp151.i578 = insertelement <4 x float> %tmp150.i577, float %tmp149.i576, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151.i578, <4 x float>* %zi1.i408
  %tmp152.i579 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx153.i580 = getelementptr inbounds float addrspace(3)* %tmp152.i579, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp154.i581 = load float addrspace(3)* %arrayidx153.i580 ; <float> [#uses=1]
  %tmp155.i582 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp156.i583 = insertelement <4 x float> %tmp155.i582, float %tmp154.i581, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156.i583, <4 x float>* %zi2.i409
  %tmp157.i584 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx158.i585 = getelementptr inbounds float addrspace(3)* %tmp157.i584, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp159.i586 = load float addrspace(3)* %arrayidx158.i585 ; <float> [#uses=1]
  %tmp160.i587 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp161.i588 = insertelement <4 x float> %tmp160.i587, float %tmp159.i586, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161.i588, <4 x float>* %zi3.i410
  %tmp162.i589 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr163.i590 = getelementptr inbounds float addrspace(3)* %tmp162.i589, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163.i590, float addrspace(3)** %lp.i402
  %tmp164.i591 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx165.i592 = getelementptr inbounds float addrspace(3)* %tmp164.i591, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166.i593 = load float addrspace(3)* %arrayidx165.i592 ; <float> [#uses=1]
  %tmp167.i594 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp168.i595 = insertelement <4 x float> %tmp167.i594, float %tmp166.i593, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168.i595, <4 x float>* %zi0.i407
  %tmp169.i596 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx170.i597 = getelementptr inbounds float addrspace(3)* %tmp169.i596, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp171.i598 = load float addrspace(3)* %arrayidx170.i597 ; <float> [#uses=1]
  %tmp172.i599 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp173.i600 = insertelement <4 x float> %tmp172.i599, float %tmp171.i598, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173.i600, <4 x float>* %zi1.i408
  %tmp174.i601 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx175.i602 = getelementptr inbounds float addrspace(3)* %tmp174.i601, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp176.i603 = load float addrspace(3)* %arrayidx175.i602 ; <float> [#uses=1]
  %tmp177.i604 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp178.i605 = insertelement <4 x float> %tmp177.i604, float %tmp176.i603, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178.i605, <4 x float>* %zi2.i409
  %tmp179.i606 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx180.i607 = getelementptr inbounds float addrspace(3)* %tmp179.i606, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp181.i608 = load float addrspace(3)* %arrayidx180.i607 ; <float> [#uses=1]
  %tmp182.i609 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp183.i610 = insertelement <4 x float> %tmp182.i609, float %tmp181.i608, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183.i610, <4 x float>* %zi3.i410
  %tmp185.i611 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp186.i612 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %add187.i613 = fadd <4 x float> %tmp185.i611, %tmp186.i612 ; <<4 x float>> [#uses=1]
  store <4 x float> %add187.i613, <4 x float>* %ar0.i411
  %tmp189.i614 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp190.i615 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %add191.i616 = fadd <4 x float> %tmp189.i614, %tmp190.i615 ; <<4 x float>> [#uses=1]
  store <4 x float> %add191.i616, <4 x float>* %ar2.i412
  %tmp193.i617 = load <4 x float>* %ar0.i411      ; <<4 x float>> [#uses=1]
  %tmp194.i618 = load <4 x float>* %ar2.i412      ; <<4 x float>> [#uses=1]
  %add195.i619 = fadd <4 x float> %tmp193.i617, %tmp194.i618 ; <<4 x float>> [#uses=1]
  store <4 x float> %add195.i619, <4 x float>* %br0.i413
  %tmp197.i620 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp198.i621 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %sub.i622 = fsub <4 x float> %tmp197.i620, %tmp198.i621 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub.i622, <4 x float>* %br1.i414
  %tmp200.i623 = load <4 x float>* %ar0.i411      ; <<4 x float>> [#uses=1]
  %tmp201.i624 = load <4 x float>* %ar2.i412      ; <<4 x float>> [#uses=1]
  %sub202.i625 = fsub <4 x float> %tmp200.i623, %tmp201.i624 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202.i625, <4 x float>* %br2.i415
  %tmp204.i626 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp205.i627 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %sub206.i628 = fsub <4 x float> %tmp204.i626, %tmp205.i627 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206.i628, <4 x float>* %br3.i416
  %tmp208.i629 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp209.i630 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %add210.i631 = fadd <4 x float> %tmp208.i629, %tmp209.i630 ; <<4 x float>> [#uses=1]
  store <4 x float> %add210.i631, <4 x float>* %ai0.i417
  %tmp212.i632 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp213.i633 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %add214.i634 = fadd <4 x float> %tmp212.i632, %tmp213.i633 ; <<4 x float>> [#uses=1]
  store <4 x float> %add214.i634, <4 x float>* %ai2.i418
  %tmp216.i635 = load <4 x float>* %ai0.i417      ; <<4 x float>> [#uses=1]
  %tmp217.i636 = load <4 x float>* %ai2.i418      ; <<4 x float>> [#uses=1]
  %add218.i637 = fadd <4 x float> %tmp216.i635, %tmp217.i636 ; <<4 x float>> [#uses=1]
  store <4 x float> %add218.i637, <4 x float>* %bi0.i419
  %tmp220.i638 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp221.i639 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %sub222.i640 = fsub <4 x float> %tmp220.i638, %tmp221.i639 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222.i640, <4 x float>* %bi1.i420
  %tmp224.i641 = load <4 x float>* %ai0.i417      ; <<4 x float>> [#uses=1]
  %tmp225.i642 = load <4 x float>* %ai2.i418      ; <<4 x float>> [#uses=1]
  %sub226.i643 = fsub <4 x float> %tmp224.i641, %tmp225.i642 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226.i643, <4 x float>* %bi2.i421
  %tmp228.i644 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp229.i645 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %sub230.i646 = fsub <4 x float> %tmp228.i644, %tmp229.i645 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230.i646, <4 x float>* %bi3.i422
  %tmp231.i647 = load <4 x float>* %br0.i413      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231.i647, <4 x float>* %zr0.i403
  %tmp232.i648 = load <4 x float>* %bi0.i419      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232.i648, <4 x float>* %zi0.i407
  %tmp233.i649 = load <4 x float>* %br1.i414      ; <<4 x float>> [#uses=1]
  %tmp234.i650 = load <4 x float>* %bi3.i422      ; <<4 x float>> [#uses=1]
  %add235.i651 = fadd <4 x float> %tmp233.i649, %tmp234.i650 ; <<4 x float>> [#uses=1]
  store <4 x float> %add235.i651, <4 x float>* %zr1.i404
  %tmp236.i652 = load <4 x float>* %bi1.i420      ; <<4 x float>> [#uses=1]
  %tmp237.i653 = load <4 x float>* %br3.i416      ; <<4 x float>> [#uses=1]
  %sub238.i654 = fsub <4 x float> %tmp236.i652, %tmp237.i653 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238.i654, <4 x float>* %zi1.i408
  %tmp239.i655 = load <4 x float>* %br1.i414      ; <<4 x float>> [#uses=1]
  %tmp240.i656 = load <4 x float>* %bi3.i422      ; <<4 x float>> [#uses=1]
  %sub241.i657 = fsub <4 x float> %tmp239.i655, %tmp240.i656 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241.i657, <4 x float>* %zr3.i406
  %tmp242.i658 = load <4 x float>* %br3.i416      ; <<4 x float>> [#uses=1]
  %tmp243.i659 = load <4 x float>* %bi1.i420      ; <<4 x float>> [#uses=1]
  %add244.i660 = fadd <4 x float> %tmp242.i658, %tmp243.i659 ; <<4 x float>> [#uses=1]
  store <4 x float> %add244.i660, <4 x float>* %zi3.i410
  %tmp245.i661 = load <4 x float>* %br2.i415      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245.i661, <4 x float>* %zr2.i405
  %tmp246.i662 = load <4 x float>* %bi2.i421      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246.i662, <4 x float>* %zi2.i409
  %tmp248.i663 = load i32* %me.addr.i400          ; <i32> [#uses=1]
  %shr249.i = lshr i32 %tmp248.i663, 2            ; <i32> [#uses=1]
  %shl.i664 = shl i32 %shr249.i, 4                ; <i32> [#uses=1]
  store i32 %shl.i664, i32* %tbase.i423
  %tmp253.i = load i32* %tbase.i423               ; <i32> [#uses=1]
  %mul.i665 = mul i32 %tmp253.i, 1                ; <i32> [#uses=1]
  store i32 %mul.i665, i32* %i.addr.i.i397
  store float* %c1.i424, float** %cretp.addr.i.i398
  %tmp.i.i666 = load i32* %i.addr.i.i397          ; <i32> [#uses=1]
  %cmp.i.i667 = icmp sgt i32 %tmp.i.i666, 512     ; <i1> [#uses=1]
  br i1 %cmp.i.i667, label %if.then.i.i670, label %k_sincos.exit.i679

if.then.i.i670:                                   ; preds = %kfft_pass2.exit
  %tmp1.i.i668 = load i32* %i.addr.i.i397         ; <i32> [#uses=1]
  %sub.i.i669 = sub i32 %tmp1.i.i668, 1024        ; <i32> [#uses=1]
  store i32 %sub.i.i669, i32* %i.addr.i.i397
  br label %k_sincos.exit.i679

k_sincos.exit.i679:                               ; preds = %if.then.i.i670, %kfft_pass2.exit
  %tmp3.i.i671 = load i32* %i.addr.i.i397         ; <i32> [#uses=1]
  %conv.i.i672 = sitofp i32 %tmp3.i.i671 to float ; <float> [#uses=1]
  %mul.i.i673 = fmul float %conv.i.i672, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i.i673, float* %x.i.i399
  %tmp4.i.i674 = load float* %x.i.i399            ; <float> [#uses=1]
  %call.i.i675 = call float @_Z10native_cosf(float %tmp4.i.i674) nounwind ; <float> [#uses=1]
  %tmp5.i.i676 = load float** %cretp.addr.i.i398  ; <float*> [#uses=1]
  store float %call.i.i675, float* %tmp5.i.i676
  %tmp6.i.i677 = load float* %x.i.i399            ; <float> [#uses=1]
  %call7.i.i678 = call float @_Z10native_sinf(float %tmp6.i.i677) nounwind ; <float> [#uses=1]
  store float %call7.i.i678, float* %retval.i.i396
  %8 = load float* %retval.i.i396                 ; <float> [#uses=1]
  store float %8, float* %s1.i425
  %tmp256.i680 = load float* %c1.i424             ; <float> [#uses=1]
  %tmp257.i681 = insertelement <4 x float> undef, float %tmp256.i680, i32 0 ; <<4 x float>> [#uses=2]
  %splat.i682 = shufflevector <4 x float> %tmp257.i681, <4 x float> %tmp257.i681, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp258.i683 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %mul259.i684 = fmul <4 x float> %splat.i682, %tmp258.i683 ; <<4 x float>> [#uses=1]
  %tmp260.i685 = load float* %s1.i425             ; <float> [#uses=1]
  %tmp261.i686 = insertelement <4 x float> undef, float %tmp260.i685, i32 0 ; <<4 x float>> [#uses=2]
  %splat262.i = shufflevector <4 x float> %tmp261.i686, <4 x float> %tmp261.i686, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp263.i687 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %mul264.i = fmul <4 x float> %splat262.i, %tmp263.i687 ; <<4 x float>> [#uses=1]
  %sub265.i = fsub <4 x float> %mul259.i684, %mul264.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub265.i, <4 x float>* %__r.i426
  %tmp266.i688 = load float* %c1.i424             ; <float> [#uses=1]
  %tmp267.i689 = insertelement <4 x float> undef, float %tmp266.i688, i32 0 ; <<4 x float>> [#uses=2]
  %splat268.i690 = shufflevector <4 x float> %tmp267.i689, <4 x float> %tmp267.i689, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp269.i691 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %mul270.i692 = fmul <4 x float> %splat268.i690, %tmp269.i691 ; <<4 x float>> [#uses=1]
  %tmp271.i693 = load float* %s1.i425             ; <float> [#uses=1]
  %tmp272.i694 = insertelement <4 x float> undef, float %tmp271.i693, i32 0 ; <<4 x float>> [#uses=2]
  %splat273.i = shufflevector <4 x float> %tmp272.i694, <4 x float> %tmp272.i694, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp274.i695 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %mul275.i = fmul <4 x float> %splat273.i, %tmp274.i695 ; <<4 x float>> [#uses=1]
  %add276.i = fadd <4 x float> %mul270.i692, %mul275.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add276.i, <4 x float>* %zi1.i408
  %tmp277.i696 = load <4 x float>* %__r.i426      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp277.i696, <4 x float>* %zr1.i404
  %tmp281.i697 = load i32* %tbase.i423            ; <i32> [#uses=1]
  %mul282.i = mul i32 %tmp281.i697, 2             ; <i32> [#uses=1]
  store i32 %mul282.i, i32* %i.addr.i508.i
  store float* %c2.i427, float** %cretp.addr.i509.i
  %tmp.i511.i = load i32* %i.addr.i508.i          ; <i32> [#uses=1]
  %cmp.i512.i = icmp sgt i32 %tmp.i511.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i512.i, label %if.then.i515.i, label %k_sincos.exit524.i

if.then.i515.i:                                   ; preds = %k_sincos.exit.i679
  %tmp1.i513.i = load i32* %i.addr.i508.i         ; <i32> [#uses=1]
  %sub.i514.i = sub i32 %tmp1.i513.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i514.i, i32* %i.addr.i508.i
  br label %k_sincos.exit524.i

k_sincos.exit524.i:                               ; preds = %if.then.i515.i, %k_sincos.exit.i679
  %tmp3.i516.i = load i32* %i.addr.i508.i         ; <i32> [#uses=1]
  %conv.i517.i = sitofp i32 %tmp3.i516.i to float ; <float> [#uses=1]
  %mul.i518.i = fmul float %conv.i517.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i518.i, float* %x.i510.i
  %tmp4.i519.i = load float* %x.i510.i            ; <float> [#uses=1]
  %call.i520.i = call float @_Z10native_cosf(float %tmp4.i519.i) nounwind ; <float> [#uses=1]
  %tmp5.i521.i = load float** %cretp.addr.i509.i  ; <float*> [#uses=1]
  store float %call.i520.i, float* %tmp5.i521.i
  %tmp6.i522.i = load float* %x.i510.i            ; <float> [#uses=1]
  %call7.i523.i = call float @_Z10native_sinf(float %tmp6.i522.i) nounwind ; <float> [#uses=1]
  store float %call7.i523.i, float* %retval.i507.i
  %9 = load float* %retval.i507.i                 ; <float> [#uses=1]
  store float %9, float* %s2.i428
  %tmp287.i698 = load float* %c2.i427             ; <float> [#uses=1]
  %tmp288.i699 = insertelement <4 x float> undef, float %tmp287.i698, i32 0 ; <<4 x float>> [#uses=2]
  %splat289.i = shufflevector <4 x float> %tmp288.i699, <4 x float> %tmp288.i699, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp290.i700 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %mul291.i = fmul <4 x float> %splat289.i, %tmp290.i700 ; <<4 x float>> [#uses=1]
  %tmp292.i701 = load float* %s2.i428             ; <float> [#uses=1]
  %tmp293.i702 = insertelement <4 x float> undef, float %tmp292.i701, i32 0 ; <<4 x float>> [#uses=2]
  %splat294.i = shufflevector <4 x float> %tmp293.i702, <4 x float> %tmp293.i702, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp295.i703 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %mul296.i = fmul <4 x float> %splat294.i, %tmp295.i703 ; <<4 x float>> [#uses=1]
  %sub297.i = fsub <4 x float> %mul291.i, %mul296.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub297.i, <4 x float>* %__r286.i
  %tmp298.i704 = load float* %c2.i427             ; <float> [#uses=1]
  %tmp299.i705 = insertelement <4 x float> undef, float %tmp298.i704, i32 0 ; <<4 x float>> [#uses=2]
  %splat300.i706 = shufflevector <4 x float> %tmp299.i705, <4 x float> %tmp299.i705, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp301.i707 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %mul302.i708 = fmul <4 x float> %splat300.i706, %tmp301.i707 ; <<4 x float>> [#uses=1]
  %tmp303.i709 = load float* %s2.i428             ; <float> [#uses=1]
  %tmp304.i710 = insertelement <4 x float> undef, float %tmp303.i709, i32 0 ; <<4 x float>> [#uses=2]
  %splat305.i = shufflevector <4 x float> %tmp304.i710, <4 x float> %tmp304.i710, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp306.i711 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %mul307.i = fmul <4 x float> %splat305.i, %tmp306.i711 ; <<4 x float>> [#uses=1]
  %add308.i = fadd <4 x float> %mul302.i708, %mul307.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add308.i, <4 x float>* %zi2.i409
  %tmp309.i712 = load <4 x float>* %__r286.i      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp309.i712, <4 x float>* %zr2.i405
  %tmp313.i713 = load i32* %tbase.i423            ; <i32> [#uses=1]
  %mul314.i = mul i32 %tmp313.i713, 3             ; <i32> [#uses=1]
  store i32 %mul314.i, i32* %i.addr.i490.i
  store float* %c3.i429, float** %cretp.addr.i491.i
  %tmp.i493.i = load i32* %i.addr.i490.i          ; <i32> [#uses=1]
  %cmp.i494.i = icmp sgt i32 %tmp.i493.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i494.i, label %if.then.i497.i, label %kfft_pass3.exit

if.then.i497.i:                                   ; preds = %k_sincos.exit524.i
  %tmp1.i495.i = load i32* %i.addr.i490.i         ; <i32> [#uses=1]
  %sub.i496.i = sub i32 %tmp1.i495.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i496.i, i32* %i.addr.i490.i
  br label %kfft_pass3.exit

kfft_pass3.exit:                                  ; preds = %k_sincos.exit524.i, %if.then.i497.i
  %tmp3.i498.i = load i32* %i.addr.i490.i         ; <i32> [#uses=1]
  %conv.i499.i = sitofp i32 %tmp3.i498.i to float ; <float> [#uses=1]
  %mul.i500.i = fmul float %conv.i499.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i500.i, float* %x.i492.i
  %tmp4.i501.i = load float* %x.i492.i            ; <float> [#uses=1]
  %call.i502.i = call float @_Z10native_cosf(float %tmp4.i501.i) nounwind ; <float> [#uses=1]
  %tmp5.i503.i = load float** %cretp.addr.i491.i  ; <float*> [#uses=1]
  store float %call.i502.i, float* %tmp5.i503.i
  %tmp6.i504.i = load float* %x.i492.i            ; <float> [#uses=1]
  %call7.i505.i = call float @_Z10native_sinf(float %tmp6.i504.i) nounwind ; <float> [#uses=1]
  store float %call7.i505.i, float* %retval.i489.i
  %10 = load float* %retval.i489.i                ; <float> [#uses=1]
  store float %10, float* %s3.i430
  %tmp319.i714 = load float* %c3.i429             ; <float> [#uses=1]
  %tmp320.i715 = insertelement <4 x float> undef, float %tmp319.i714, i32 0 ; <<4 x float>> [#uses=2]
  %splat321.i = shufflevector <4 x float> %tmp320.i715, <4 x float> %tmp320.i715, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp322.i716 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %mul323.i = fmul <4 x float> %splat321.i, %tmp322.i716 ; <<4 x float>> [#uses=1]
  %tmp324.i = load float* %s3.i430                ; <float> [#uses=1]
  %tmp325.i717 = insertelement <4 x float> undef, float %tmp324.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat326.i = shufflevector <4 x float> %tmp325.i717, <4 x float> %tmp325.i717, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp327.i = load <4 x float>* %zi3.i410         ; <<4 x float>> [#uses=1]
  %mul328.i = fmul <4 x float> %splat326.i, %tmp327.i ; <<4 x float>> [#uses=1]
  %sub329.i = fsub <4 x float> %mul323.i, %mul328.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub329.i, <4 x float>* %__r318.i
  %tmp330.i718 = load float* %c3.i429             ; <float> [#uses=1]
  %tmp331.i719 = insertelement <4 x float> undef, float %tmp330.i718, i32 0 ; <<4 x float>> [#uses=2]
  %splat332.i720 = shufflevector <4 x float> %tmp331.i719, <4 x float> %tmp331.i719, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp333.i721 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %mul334.i722 = fmul <4 x float> %splat332.i720, %tmp333.i721 ; <<4 x float>> [#uses=1]
  %tmp335.i = load float* %s3.i430                ; <float> [#uses=1]
  %tmp336.i723 = insertelement <4 x float> undef, float %tmp335.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat337.i = shufflevector <4 x float> %tmp336.i723, <4 x float> %tmp336.i723, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp338.i = load <4 x float>* %zr3.i406         ; <<4 x float>> [#uses=1]
  %mul339.i = fmul <4 x float> %splat337.i, %tmp338.i ; <<4 x float>> [#uses=1]
  %add340.i = fadd <4 x float> %mul334.i722, %mul339.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add340.i, <4 x float>* %zi3.i410
  %tmp341.i724 = load <4 x float>* %__r318.i      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp341.i724, <4 x float>* %zr3.i406
  call void @barrier(i32 1) nounwind
  %tmp344.i725 = load float addrspace(3)** %lds.addr.i401 ; <float addrspace(3)*> [#uses=1]
  %tmp345.i = load i32* %me.addr.i400             ; <i32> [#uses=1]
  %add.ptr346.i = getelementptr inbounds float addrspace(3)* %tmp344.i725, i32 %tmp345.i ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr346.i, float addrspace(3)** %lp.i402
  %tmp347.i726 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp348.i = extractelement <4 x float> %tmp347.i726, i32 0 ; <float> [#uses=1]
  %tmp349.i = load float addrspace(3)** %lp.i402  ; <float addrspace(3)*> [#uses=1]
  %arrayidx350.i = getelementptr inbounds float addrspace(3)* %tmp349.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp348.i, float addrspace(3)* %arrayidx350.i
  %tmp351.i727 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp352.i = extractelement <4 x float> %tmp351.i727, i32 1 ; <float> [#uses=1]
  %tmp353.i728 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx354.i = getelementptr inbounds float addrspace(3)* %tmp353.i728, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp352.i, float addrspace(3)* %arrayidx354.i
  %tmp355.i729 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp356.i = extractelement <4 x float> %tmp355.i729, i32 2 ; <float> [#uses=1]
  %tmp357.i730 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx358.i = getelementptr inbounds float addrspace(3)* %tmp357.i730, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp356.i, float addrspace(3)* %arrayidx358.i
  %tmp359.i731 = load <4 x float>* %zr0.i403      ; <<4 x float>> [#uses=1]
  %tmp360.i = extractelement <4 x float> %tmp359.i731, i32 3 ; <float> [#uses=1]
  %tmp361.i732 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx362.i = getelementptr inbounds float addrspace(3)* %tmp361.i732, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp360.i, float addrspace(3)* %arrayidx362.i
  %tmp363.i733 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr364.i = getelementptr inbounds float addrspace(3)* %tmp363.i733, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr364.i, float addrspace(3)** %lp.i402
  %tmp365.i734 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp366.i735 = extractelement <4 x float> %tmp365.i734, i32 0 ; <float> [#uses=1]
  %tmp367.i736 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx368.i737 = getelementptr inbounds float addrspace(3)* %tmp367.i736, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp366.i735, float addrspace(3)* %arrayidx368.i737
  %tmp369.i738 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp370.i = extractelement <4 x float> %tmp369.i738, i32 1 ; <float> [#uses=1]
  %tmp371.i739 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx372.i = getelementptr inbounds float addrspace(3)* %tmp371.i739, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp370.i, float addrspace(3)* %arrayidx372.i
  %tmp373.i740 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp374.i = extractelement <4 x float> %tmp373.i740, i32 2 ; <float> [#uses=1]
  %tmp375.i741 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx376.i = getelementptr inbounds float addrspace(3)* %tmp375.i741, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp374.i, float addrspace(3)* %arrayidx376.i
  %tmp377.i742 = load <4 x float>* %zr1.i404      ; <<4 x float>> [#uses=1]
  %tmp378.i = extractelement <4 x float> %tmp377.i742, i32 3 ; <float> [#uses=1]
  %tmp379.i743 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx380.i = getelementptr inbounds float addrspace(3)* %tmp379.i743, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp378.i, float addrspace(3)* %arrayidx380.i
  %tmp381.i744 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr382.i = getelementptr inbounds float addrspace(3)* %tmp381.i744, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr382.i, float addrspace(3)** %lp.i402
  %tmp383.i745 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %tmp384.i746 = extractelement <4 x float> %tmp383.i745, i32 0 ; <float> [#uses=1]
  %tmp385.i747 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx386.i748 = getelementptr inbounds float addrspace(3)* %tmp385.i747, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp384.i746, float addrspace(3)* %arrayidx386.i748
  %tmp387.i749 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %tmp388.i = extractelement <4 x float> %tmp387.i749, i32 1 ; <float> [#uses=1]
  %tmp389.i750 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx390.i = getelementptr inbounds float addrspace(3)* %tmp389.i750, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp388.i, float addrspace(3)* %arrayidx390.i
  %tmp391.i751 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %tmp392.i = extractelement <4 x float> %tmp391.i751, i32 2 ; <float> [#uses=1]
  %tmp393.i752 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx394.i = getelementptr inbounds float addrspace(3)* %tmp393.i752, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp392.i, float addrspace(3)* %arrayidx394.i
  %tmp395.i753 = load <4 x float>* %zr2.i405      ; <<4 x float>> [#uses=1]
  %tmp396.i = extractelement <4 x float> %tmp395.i753, i32 3 ; <float> [#uses=1]
  %tmp397.i754 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx398.i = getelementptr inbounds float addrspace(3)* %tmp397.i754, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp396.i, float addrspace(3)* %arrayidx398.i
  %tmp399.i755 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr400.i = getelementptr inbounds float addrspace(3)* %tmp399.i755, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr400.i, float addrspace(3)** %lp.i402
  %tmp401.i756 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %tmp402.i757 = extractelement <4 x float> %tmp401.i756, i32 0 ; <float> [#uses=1]
  %tmp403.i758 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx404.i759 = getelementptr inbounds float addrspace(3)* %tmp403.i758, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp402.i757, float addrspace(3)* %arrayidx404.i759
  %tmp405.i760 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %tmp406.i = extractelement <4 x float> %tmp405.i760, i32 1 ; <float> [#uses=1]
  %tmp407.i761 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx408.i = getelementptr inbounds float addrspace(3)* %tmp407.i761, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp406.i, float addrspace(3)* %arrayidx408.i
  %tmp409.i762 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %tmp410.i = extractelement <4 x float> %tmp409.i762, i32 2 ; <float> [#uses=1]
  %tmp411.i763 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx412.i = getelementptr inbounds float addrspace(3)* %tmp411.i763, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp410.i, float addrspace(3)* %arrayidx412.i
  %tmp413.i764 = load <4 x float>* %zr3.i406      ; <<4 x float>> [#uses=1]
  %tmp414.i = extractelement <4 x float> %tmp413.i764, i32 3 ; <float> [#uses=1]
  %tmp415.i765 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx416.i = getelementptr inbounds float addrspace(3)* %tmp415.i765, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp414.i, float addrspace(3)* %arrayidx416.i
  %tmp417.i766 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr418.i = getelementptr inbounds float addrspace(3)* %tmp417.i766, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr418.i, float addrspace(3)** %lp.i402
  %tmp419.i767 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp420.i768 = extractelement <4 x float> %tmp419.i767, i32 0 ; <float> [#uses=1]
  %tmp421.i769 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx422.i770 = getelementptr inbounds float addrspace(3)* %tmp421.i769, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp420.i768, float addrspace(3)* %arrayidx422.i770
  %tmp423.i771 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp424.i = extractelement <4 x float> %tmp423.i771, i32 1 ; <float> [#uses=1]
  %tmp425.i772 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx426.i = getelementptr inbounds float addrspace(3)* %tmp425.i772, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp424.i, float addrspace(3)* %arrayidx426.i
  %tmp427.i773 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp428.i = extractelement <4 x float> %tmp427.i773, i32 2 ; <float> [#uses=1]
  %tmp429.i774 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx430.i = getelementptr inbounds float addrspace(3)* %tmp429.i774, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp428.i, float addrspace(3)* %arrayidx430.i
  %tmp431.i775 = load <4 x float>* %zi0.i407      ; <<4 x float>> [#uses=1]
  %tmp432.i = extractelement <4 x float> %tmp431.i775, i32 3 ; <float> [#uses=1]
  %tmp433.i776 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx434.i = getelementptr inbounds float addrspace(3)* %tmp433.i776, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp432.i, float addrspace(3)* %arrayidx434.i
  %tmp435.i777 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr436.i = getelementptr inbounds float addrspace(3)* %tmp435.i777, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr436.i, float addrspace(3)** %lp.i402
  %tmp437.i778 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp438.i779 = extractelement <4 x float> %tmp437.i778, i32 0 ; <float> [#uses=1]
  %tmp439.i780 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx440.i781 = getelementptr inbounds float addrspace(3)* %tmp439.i780, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp438.i779, float addrspace(3)* %arrayidx440.i781
  %tmp441.i782 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp442.i = extractelement <4 x float> %tmp441.i782, i32 1 ; <float> [#uses=1]
  %tmp443.i783 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx444.i = getelementptr inbounds float addrspace(3)* %tmp443.i783, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp442.i, float addrspace(3)* %arrayidx444.i
  %tmp445.i784 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp446.i = extractelement <4 x float> %tmp445.i784, i32 2 ; <float> [#uses=1]
  %tmp447.i785 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx448.i = getelementptr inbounds float addrspace(3)* %tmp447.i785, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp446.i, float addrspace(3)* %arrayidx448.i
  %tmp449.i786 = load <4 x float>* %zi1.i408      ; <<4 x float>> [#uses=1]
  %tmp450.i = extractelement <4 x float> %tmp449.i786, i32 3 ; <float> [#uses=1]
  %tmp451.i787 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx452.i = getelementptr inbounds float addrspace(3)* %tmp451.i787, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp450.i, float addrspace(3)* %arrayidx452.i
  %tmp453.i788 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr454.i = getelementptr inbounds float addrspace(3)* %tmp453.i788, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr454.i, float addrspace(3)** %lp.i402
  %tmp455.i789 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp456.i790 = extractelement <4 x float> %tmp455.i789, i32 0 ; <float> [#uses=1]
  %tmp457.i791 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx458.i792 = getelementptr inbounds float addrspace(3)* %tmp457.i791, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp456.i790, float addrspace(3)* %arrayidx458.i792
  %tmp459.i793 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp460.i = extractelement <4 x float> %tmp459.i793, i32 1 ; <float> [#uses=1]
  %tmp461.i794 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx462.i = getelementptr inbounds float addrspace(3)* %tmp461.i794, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp460.i, float addrspace(3)* %arrayidx462.i
  %tmp463.i795 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp464.i = extractelement <4 x float> %tmp463.i795, i32 2 ; <float> [#uses=1]
  %tmp465.i796 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx466.i = getelementptr inbounds float addrspace(3)* %tmp465.i796, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp464.i, float addrspace(3)* %arrayidx466.i
  %tmp467.i797 = load <4 x float>* %zi2.i409      ; <<4 x float>> [#uses=1]
  %tmp468.i = extractelement <4 x float> %tmp467.i797, i32 3 ; <float> [#uses=1]
  %tmp469.i798 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx470.i = getelementptr inbounds float addrspace(3)* %tmp469.i798, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp468.i, float addrspace(3)* %arrayidx470.i
  %tmp471.i799 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %add.ptr472.i = getelementptr inbounds float addrspace(3)* %tmp471.i799, i32 264 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr472.i, float addrspace(3)** %lp.i402
  %tmp473.i800 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp474.i801 = extractelement <4 x float> %tmp473.i800, i32 0 ; <float> [#uses=1]
  %tmp475.i802 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx476.i803 = getelementptr inbounds float addrspace(3)* %tmp475.i802, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp474.i801, float addrspace(3)* %arrayidx476.i803
  %tmp477.i804 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp478.i = extractelement <4 x float> %tmp477.i804, i32 1 ; <float> [#uses=1]
  %tmp479.i805 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx480.i = getelementptr inbounds float addrspace(3)* %tmp479.i805, i32 66 ; <float addrspace(3)*> [#uses=1]
  store float %tmp478.i, float addrspace(3)* %arrayidx480.i
  %tmp481.i806 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp482.i = extractelement <4 x float> %tmp481.i806, i32 2 ; <float> [#uses=1]
  %tmp483.i807 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx484.i = getelementptr inbounds float addrspace(3)* %tmp483.i807, i32 132 ; <float addrspace(3)*> [#uses=1]
  store float %tmp482.i, float addrspace(3)* %arrayidx484.i
  %tmp485.i808 = load <4 x float>* %zi3.i410      ; <<4 x float>> [#uses=1]
  %tmp486.i = extractelement <4 x float> %tmp485.i808, i32 3 ; <float> [#uses=1]
  %tmp487.i809 = load float addrspace(3)** %lp.i402 ; <float addrspace(3)*> [#uses=1]
  %arrayidx488.i = getelementptr inbounds float addrspace(3)* %tmp487.i809, i32 198 ; <float addrspace(3)*> [#uses=1]
  store float %tmp486.i, float addrspace(3)* %arrayidx488.i
  call void @barrier(i32 1) nounwind
  %tmp13 = load i32* %me                          ; <i32> [#uses=1]
  store i32 %tmp13, i32* %me.addr.i166
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %lds.addr.i167
  %tmp.i197 = load float addrspace(3)** %lds.addr.i167 ; <float addrspace(3)*> [#uses=1]
  %tmp1.i198 = load i32* %me.addr.i166            ; <i32> [#uses=1]
  %and.i199 = and i32 %tmp1.i198, 3               ; <i32> [#uses=1]
  %tmp2.i200 = load i32* %me.addr.i166            ; <i32> [#uses=1]
  %shr.i201 = lshr i32 %tmp2.i200, 2              ; <i32> [#uses=1]
  %and3.i = and i32 %shr.i201, 3                  ; <i32> [#uses=1]
  %mul.i202 = mul i32 %and3.i, 264                ; <i32> [#uses=1]
  %add.i203 = add i32 %and.i199, %mul.i202        ; <i32> [#uses=1]
  %tmp4.i204 = load i32* %me.addr.i166            ; <i32> [#uses=1]
  %shr5.i = lshr i32 %tmp4.i204, 4                ; <i32> [#uses=1]
  %shl.i205 = shl i32 %shr5.i, 2                  ; <i32> [#uses=1]
  %add6.i = add i32 %add.i203, %shl.i205          ; <i32> [#uses=1]
  %add.ptr.i206 = getelementptr inbounds float addrspace(3)* %tmp.i197, i32 %add6.i ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr.i206, float addrspace(3)** %lp.i168
  %tmp11.i207 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx.i208 = getelementptr inbounds float addrspace(3)* %tmp11.i207, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp12.i209 = load float addrspace(3)* %arrayidx.i208 ; <float> [#uses=1]
  %tmp13.i210 = load <4 x float>* %zr0.i169       ; <<4 x float>> [#uses=1]
  %tmp14.i211 = insertelement <4 x float> %tmp13.i210, float %tmp12.i209, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp14.i211, <4 x float>* %zr0.i169
  %tmp15.i212 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx16.i = getelementptr inbounds float addrspace(3)* %tmp15.i212, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp17.i213 = load float addrspace(3)* %arrayidx16.i ; <float> [#uses=1]
  %tmp18.i214 = load <4 x float>* %zr0.i169       ; <<4 x float>> [#uses=1]
  %tmp19.i215 = insertelement <4 x float> %tmp18.i214, float %tmp17.i213, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp19.i215, <4 x float>* %zr0.i169
  %tmp20.i216 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx21.i = getelementptr inbounds float addrspace(3)* %tmp20.i216, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp22.i217 = load float addrspace(3)* %arrayidx21.i ; <float> [#uses=1]
  %tmp23.i218 = load <4 x float>* %zr0.i169       ; <<4 x float>> [#uses=1]
  %tmp24.i219 = insertelement <4 x float> %tmp23.i218, float %tmp22.i217, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp24.i219, <4 x float>* %zr0.i169
  %tmp25.i220 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx26.i = getelementptr inbounds float addrspace(3)* %tmp25.i220, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp27.i = load float addrspace(3)* %arrayidx26.i ; <float> [#uses=1]
  %tmp28.i221 = load <4 x float>* %zr0.i169       ; <<4 x float>> [#uses=1]
  %tmp29.i = insertelement <4 x float> %tmp28.i221, float %tmp27.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp29.i, <4 x float>* %zr0.i169
  %tmp30.i222 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr31.i = getelementptr inbounds float addrspace(3)* %tmp30.i222, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr31.i, float addrspace(3)** %lp.i168
  %tmp32.i223 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx33.i = getelementptr inbounds float addrspace(3)* %tmp32.i223, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp34.i224 = load float addrspace(3)* %arrayidx33.i ; <float> [#uses=1]
  %tmp35.i225 = load <4 x float>* %zr1.i170       ; <<4 x float>> [#uses=1]
  %tmp36.i226 = insertelement <4 x float> %tmp35.i225, float %tmp34.i224, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36.i226, <4 x float>* %zr1.i170
  %tmp37.i227 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx38.i = getelementptr inbounds float addrspace(3)* %tmp37.i227, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp39.i228 = load float addrspace(3)* %arrayidx38.i ; <float> [#uses=1]
  %tmp40.i229 = load <4 x float>* %zr1.i170       ; <<4 x float>> [#uses=1]
  %tmp41.i230 = insertelement <4 x float> %tmp40.i229, float %tmp39.i228, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41.i230, <4 x float>* %zr1.i170
  %tmp42.i231 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx43.i = getelementptr inbounds float addrspace(3)* %tmp42.i231, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp44.i = load float addrspace(3)* %arrayidx43.i ; <float> [#uses=1]
  %tmp45.i232 = load <4 x float>* %zr1.i170       ; <<4 x float>> [#uses=1]
  %tmp46.i233 = insertelement <4 x float> %tmp45.i232, float %tmp44.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46.i233, <4 x float>* %zr1.i170
  %tmp47.i234 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx48.i = getelementptr inbounds float addrspace(3)* %tmp47.i234, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp49.i235 = load float addrspace(3)* %arrayidx48.i ; <float> [#uses=1]
  %tmp50.i236 = load <4 x float>* %zr1.i170       ; <<4 x float>> [#uses=1]
  %tmp51.i = insertelement <4 x float> %tmp50.i236, float %tmp49.i235, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp51.i, <4 x float>* %zr1.i170
  %tmp52.i237 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr53.i = getelementptr inbounds float addrspace(3)* %tmp52.i237, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr53.i, float addrspace(3)** %lp.i168
  %tmp54.i238 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx55.i = getelementptr inbounds float addrspace(3)* %tmp54.i238, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp56.i239 = load float addrspace(3)* %arrayidx55.i ; <float> [#uses=1]
  %tmp57.i240 = load <4 x float>* %zr2.i171       ; <<4 x float>> [#uses=1]
  %tmp58.i241 = insertelement <4 x float> %tmp57.i240, float %tmp56.i239, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp58.i241, <4 x float>* %zr2.i171
  %tmp59.i242 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx60.i = getelementptr inbounds float addrspace(3)* %tmp59.i242, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp61.i243 = load float addrspace(3)* %arrayidx60.i ; <float> [#uses=1]
  %tmp62.i244 = load <4 x float>* %zr2.i171       ; <<4 x float>> [#uses=1]
  %tmp63.i245 = insertelement <4 x float> %tmp62.i244, float %tmp61.i243, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp63.i245, <4 x float>* %zr2.i171
  %tmp64.i246 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx65.i = getelementptr inbounds float addrspace(3)* %tmp64.i246, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp66.i = load float addrspace(3)* %arrayidx65.i ; <float> [#uses=1]
  %tmp67.i247 = load <4 x float>* %zr2.i171       ; <<4 x float>> [#uses=1]
  %tmp68.i248 = insertelement <4 x float> %tmp67.i247, float %tmp66.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp68.i248, <4 x float>* %zr2.i171
  %tmp69.i249 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx70.i = getelementptr inbounds float addrspace(3)* %tmp69.i249, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp71.i = load float addrspace(3)* %arrayidx70.i ; <float> [#uses=1]
  %tmp72.i250 = load <4 x float>* %zr2.i171       ; <<4 x float>> [#uses=1]
  %tmp73.i251 = insertelement <4 x float> %tmp72.i250, float %tmp71.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp73.i251, <4 x float>* %zr2.i171
  %tmp74.i252 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr75.i = getelementptr inbounds float addrspace(3)* %tmp74.i252, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr75.i, float addrspace(3)** %lp.i168
  %tmp76.i253 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx77.i = getelementptr inbounds float addrspace(3)* %tmp76.i253, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp78.i = load float addrspace(3)* %arrayidx77.i ; <float> [#uses=1]
  %tmp79.i254 = load <4 x float>* %zr3.i172       ; <<4 x float>> [#uses=1]
  %tmp80.i255 = insertelement <4 x float> %tmp79.i254, float %tmp78.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp80.i255, <4 x float>* %zr3.i172
  %tmp81.i256 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx82.i = getelementptr inbounds float addrspace(3)* %tmp81.i256, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp83.i257 = load float addrspace(3)* %arrayidx82.i ; <float> [#uses=1]
  %tmp84.i258 = load <4 x float>* %zr3.i172       ; <<4 x float>> [#uses=1]
  %tmp85.i259 = insertelement <4 x float> %tmp84.i258, float %tmp83.i257, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp85.i259, <4 x float>* %zr3.i172
  %tmp86.i260 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx87.i = getelementptr inbounds float addrspace(3)* %tmp86.i260, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp88.i261 = load float addrspace(3)* %arrayidx87.i ; <float> [#uses=1]
  %tmp89.i262 = load <4 x float>* %zr3.i172       ; <<4 x float>> [#uses=1]
  %tmp90.i263 = insertelement <4 x float> %tmp89.i262, float %tmp88.i261, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp90.i263, <4 x float>* %zr3.i172
  %tmp91.i264 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx92.i = getelementptr inbounds float addrspace(3)* %tmp91.i264, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp93.i = load float addrspace(3)* %arrayidx92.i ; <float> [#uses=1]
  %tmp94.i265 = load <4 x float>* %zr3.i172       ; <<4 x float>> [#uses=1]
  %tmp95.i266 = insertelement <4 x float> %tmp94.i265, float %tmp93.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp95.i266, <4 x float>* %zr3.i172
  %tmp96.i = load float addrspace(3)** %lp.i168   ; <float addrspace(3)*> [#uses=1]
  %add.ptr97.i = getelementptr inbounds float addrspace(3)* %tmp96.i, i32 1008 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr97.i, float addrspace(3)** %lp.i168
  %tmp102.i267 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx103.i = getelementptr inbounds float addrspace(3)* %tmp102.i267, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp104.i = load float addrspace(3)* %arrayidx103.i ; <float> [#uses=1]
  %tmp105.i268 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp106.i269 = insertelement <4 x float> %tmp105.i268, float %tmp104.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp106.i269, <4 x float>* %zi0.i173
  %tmp107.i270 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx108.i = getelementptr inbounds float addrspace(3)* %tmp107.i270, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp109.i = load float addrspace(3)* %arrayidx108.i ; <float> [#uses=1]
  %tmp110.i271 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp111.i272 = insertelement <4 x float> %tmp110.i271, float %tmp109.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp111.i272, <4 x float>* %zi0.i173
  %tmp112.i273 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx113.i = getelementptr inbounds float addrspace(3)* %tmp112.i273, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp114.i274 = load float addrspace(3)* %arrayidx113.i ; <float> [#uses=1]
  %tmp115.i275 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp116.i276 = insertelement <4 x float> %tmp115.i275, float %tmp114.i274, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp116.i276, <4 x float>* %zi0.i173
  %tmp117.i277 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx118.i = getelementptr inbounds float addrspace(3)* %tmp117.i277, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp119.i278 = load float addrspace(3)* %arrayidx118.i ; <float> [#uses=1]
  %tmp120.i279 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp121.i280 = insertelement <4 x float> %tmp120.i279, float %tmp119.i278, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp121.i280, <4 x float>* %zi0.i173
  %tmp122.i281 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr123.i = getelementptr inbounds float addrspace(3)* %tmp122.i281, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr123.i, float addrspace(3)** %lp.i168
  %tmp124.i282 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx125.i = getelementptr inbounds float addrspace(3)* %tmp124.i282, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp126.i = load float addrspace(3)* %arrayidx125.i ; <float> [#uses=1]
  %tmp127.i283 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp128.i284 = insertelement <4 x float> %tmp127.i283, float %tmp126.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp128.i284, <4 x float>* %zi1.i174
  %tmp129.i285 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx130.i = getelementptr inbounds float addrspace(3)* %tmp129.i285, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp131.i = load float addrspace(3)* %arrayidx130.i ; <float> [#uses=1]
  %tmp132.i286 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp133.i287 = insertelement <4 x float> %tmp132.i286, float %tmp131.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp133.i287, <4 x float>* %zi1.i174
  %tmp134.i288 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx135.i = getelementptr inbounds float addrspace(3)* %tmp134.i288, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp136.i289 = load float addrspace(3)* %arrayidx135.i ; <float> [#uses=1]
  %tmp137.i290 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp138.i291 = insertelement <4 x float> %tmp137.i290, float %tmp136.i289, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp138.i291, <4 x float>* %zi1.i174
  %tmp139.i292 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx140.i = getelementptr inbounds float addrspace(3)* %tmp139.i292, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp141.i = load float addrspace(3)* %arrayidx140.i ; <float> [#uses=1]
  %tmp142.i293 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp143.i294 = insertelement <4 x float> %tmp142.i293, float %tmp141.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp143.i294, <4 x float>* %zi1.i174
  %tmp144.i295 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr145.i = getelementptr inbounds float addrspace(3)* %tmp144.i295, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr145.i, float addrspace(3)** %lp.i168
  %tmp146.i296 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx147.i = getelementptr inbounds float addrspace(3)* %tmp146.i296, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp148.i = load float addrspace(3)* %arrayidx147.i ; <float> [#uses=1]
  %tmp149.i297 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %tmp150.i298 = insertelement <4 x float> %tmp149.i297, float %tmp148.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp150.i298, <4 x float>* %zi2.i175
  %tmp151.i299 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx152.i = getelementptr inbounds float addrspace(3)* %tmp151.i299, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp153.i300 = load float addrspace(3)* %arrayidx152.i ; <float> [#uses=1]
  %tmp154.i301 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %tmp155.i302 = insertelement <4 x float> %tmp154.i301, float %tmp153.i300, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp155.i302, <4 x float>* %zi2.i175
  %tmp156.i303 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx157.i = getelementptr inbounds float addrspace(3)* %tmp156.i303, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp158.i = load float addrspace(3)* %arrayidx157.i ; <float> [#uses=1]
  %tmp159.i304 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %tmp160.i305 = insertelement <4 x float> %tmp159.i304, float %tmp158.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp160.i305, <4 x float>* %zi2.i175
  %tmp161.i306 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx162.i = getelementptr inbounds float addrspace(3)* %tmp161.i306, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp163.i307 = load float addrspace(3)* %arrayidx162.i ; <float> [#uses=1]
  %tmp164.i308 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %tmp165.i = insertelement <4 x float> %tmp164.i308, float %tmp163.i307, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp165.i, <4 x float>* %zi2.i175
  %tmp166.i309 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %add.ptr167.i = getelementptr inbounds float addrspace(3)* %tmp166.i309, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr167.i, float addrspace(3)** %lp.i168
  %tmp168.i310 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx169.i = getelementptr inbounds float addrspace(3)* %tmp168.i310, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp170.i311 = load float addrspace(3)* %arrayidx169.i ; <float> [#uses=1]
  %tmp171.i312 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %tmp172.i313 = insertelement <4 x float> %tmp171.i312, float %tmp170.i311, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp172.i313, <4 x float>* %zi3.i176
  %tmp173.i314 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx174.i = getelementptr inbounds float addrspace(3)* %tmp173.i314, i32 66 ; <float addrspace(3)*> [#uses=1]
  %tmp175.i = load float addrspace(3)* %arrayidx174.i ; <float> [#uses=1]
  %tmp176.i315 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %tmp177.i316 = insertelement <4 x float> %tmp176.i315, float %tmp175.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp177.i316, <4 x float>* %zi3.i176
  %tmp178.i317 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx179.i = getelementptr inbounds float addrspace(3)* %tmp178.i317, i32 132 ; <float addrspace(3)*> [#uses=1]
  %tmp180.i = load float addrspace(3)* %arrayidx179.i ; <float> [#uses=1]
  %tmp181.i318 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %tmp182.i319 = insertelement <4 x float> %tmp181.i318, float %tmp180.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp182.i319, <4 x float>* %zi3.i176
  %tmp183.i320 = load float addrspace(3)** %lp.i168 ; <float addrspace(3)*> [#uses=1]
  %arrayidx184.i = getelementptr inbounds float addrspace(3)* %tmp183.i320, i32 198 ; <float addrspace(3)*> [#uses=1]
  %tmp185.i321 = load float addrspace(3)* %arrayidx184.i ; <float> [#uses=1]
  %tmp186.i322 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %tmp187.i323 = insertelement <4 x float> %tmp186.i322, float %tmp185.i321, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp187.i323, <4 x float>* %zi3.i176
  %tmp189.i324 = load <4 x float>* %zr0.i169      ; <<4 x float>> [#uses=1]
  %tmp190.i325 = load <4 x float>* %zr2.i171      ; <<4 x float>> [#uses=1]
  %add191.i326 = fadd <4 x float> %tmp189.i324, %tmp190.i325 ; <<4 x float>> [#uses=1]
  store <4 x float> %add191.i326, <4 x float>* %ar0.i177
  %tmp193.i327 = load <4 x float>* %zr1.i170      ; <<4 x float>> [#uses=1]
  %tmp194.i328 = load <4 x float>* %zr3.i172      ; <<4 x float>> [#uses=1]
  %add195.i329 = fadd <4 x float> %tmp193.i327, %tmp194.i328 ; <<4 x float>> [#uses=1]
  store <4 x float> %add195.i329, <4 x float>* %ar2.i178
  %tmp197.i330 = load <4 x float>* %ar0.i177      ; <<4 x float>> [#uses=1]
  %tmp198.i331 = load <4 x float>* %ar2.i178      ; <<4 x float>> [#uses=1]
  %add199.i = fadd <4 x float> %tmp197.i330, %tmp198.i331 ; <<4 x float>> [#uses=1]
  store <4 x float> %add199.i, <4 x float>* %br0.i179
  %tmp201.i332 = load <4 x float>* %zr0.i169      ; <<4 x float>> [#uses=1]
  %tmp202.i333 = load <4 x float>* %zr2.i171      ; <<4 x float>> [#uses=1]
  %sub.i334 = fsub <4 x float> %tmp201.i332, %tmp202.i333 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub.i334, <4 x float>* %br1.i180
  %tmp204.i335 = load <4 x float>* %ar0.i177      ; <<4 x float>> [#uses=1]
  %tmp205.i336 = load <4 x float>* %ar2.i178      ; <<4 x float>> [#uses=1]
  %sub206.i337 = fsub <4 x float> %tmp204.i335, %tmp205.i336 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206.i337, <4 x float>* %br2.i181
  %tmp208.i338 = load <4 x float>* %zr1.i170      ; <<4 x float>> [#uses=1]
  %tmp209.i339 = load <4 x float>* %zr3.i172      ; <<4 x float>> [#uses=1]
  %sub210.i = fsub <4 x float> %tmp208.i338, %tmp209.i339 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub210.i, <4 x float>* %br3.i182
  %tmp212.i340 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp213.i341 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %add214.i342 = fadd <4 x float> %tmp212.i340, %tmp213.i341 ; <<4 x float>> [#uses=1]
  store <4 x float> %add214.i342, <4 x float>* %ai0.i183
  %tmp216.i343 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp217.i344 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %add218.i345 = fadd <4 x float> %tmp216.i343, %tmp217.i344 ; <<4 x float>> [#uses=1]
  store <4 x float> %add218.i345, <4 x float>* %ai2.i184
  %tmp220.i346 = load <4 x float>* %ai0.i183      ; <<4 x float>> [#uses=1]
  %tmp221.i347 = load <4 x float>* %ai2.i184      ; <<4 x float>> [#uses=1]
  %add222.i = fadd <4 x float> %tmp220.i346, %tmp221.i347 ; <<4 x float>> [#uses=1]
  store <4 x float> %add222.i, <4 x float>* %bi0.i185
  %tmp224.i348 = load <4 x float>* %zi0.i173      ; <<4 x float>> [#uses=1]
  %tmp225.i349 = load <4 x float>* %zi2.i175      ; <<4 x float>> [#uses=1]
  %sub226.i350 = fsub <4 x float> %tmp224.i348, %tmp225.i349 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226.i350, <4 x float>* %bi1.i186
  %tmp228.i351 = load <4 x float>* %ai0.i183      ; <<4 x float>> [#uses=1]
  %tmp229.i352 = load <4 x float>* %ai2.i184      ; <<4 x float>> [#uses=1]
  %sub230.i353 = fsub <4 x float> %tmp228.i351, %tmp229.i352 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230.i353, <4 x float>* %bi2.i187
  %tmp232.i354 = load <4 x float>* %zi1.i174      ; <<4 x float>> [#uses=1]
  %tmp233.i355 = load <4 x float>* %zi3.i176      ; <<4 x float>> [#uses=1]
  %sub234.i = fsub <4 x float> %tmp232.i354, %tmp233.i355 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub234.i, <4 x float>* %bi3.i188
  %tmp235.i = load <4 x float>* %br0.i179         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp235.i, <4 x float>* %zr0.i169
  %tmp236.i356 = load <4 x float>* %bi0.i185      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp236.i356, <4 x float>* %zi0.i173
  %tmp237.i357 = load <4 x float>* %br1.i180      ; <<4 x float>> [#uses=1]
  %tmp238.i358 = load <4 x float>* %bi3.i188      ; <<4 x float>> [#uses=1]
  %add239.i = fadd <4 x float> %tmp237.i357, %tmp238.i358 ; <<4 x float>> [#uses=1]
  store <4 x float> %add239.i, <4 x float>* %zr1.i170
  %tmp240.i359 = load <4 x float>* %bi1.i186      ; <<4 x float>> [#uses=1]
  %tmp241.i360 = load <4 x float>* %br3.i182      ; <<4 x float>> [#uses=1]
  %sub242.i = fsub <4 x float> %tmp240.i359, %tmp241.i360 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub242.i, <4 x float>* %zi1.i174
  %tmp243.i361 = load <4 x float>* %br1.i180      ; <<4 x float>> [#uses=1]
  %tmp244.i362 = load <4 x float>* %bi3.i188      ; <<4 x float>> [#uses=1]
  %sub245.i = fsub <4 x float> %tmp243.i361, %tmp244.i362 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub245.i, <4 x float>* %zr3.i172
  %tmp246.i363 = load <4 x float>* %br3.i182      ; <<4 x float>> [#uses=1]
  %tmp247.i = load <4 x float>* %bi1.i186         ; <<4 x float>> [#uses=1]
  %add248.i = fadd <4 x float> %tmp246.i363, %tmp247.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add248.i, <4 x float>* %zi3.i176
  %tmp249.i364 = load <4 x float>* %br2.i181      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp249.i364, <4 x float>* %zr2.i171
  %tmp250.i365 = load <4 x float>* %bi2.i187      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp250.i365, <4 x float>* %zi2.i175
  %tmp252.i366 = load i32* %me.addr.i166          ; <i32> [#uses=1]
  %shr253.i = lshr i32 %tmp252.i366, 4            ; <i32> [#uses=1]
  %shl254.i = shl i32 %shr253.i, 6                ; <i32> [#uses=1]
  store i32 %shl254.i, i32* %tbase.i189
  %tmp258.i367 = load i32* %tbase.i189            ; <i32> [#uses=1]
  %mul259.i = mul i32 %tmp258.i367, 1             ; <i32> [#uses=1]
  store i32 %mul259.i, i32* %i.addr.i.i163
  store float* %c1.i190, float** %cretp.addr.i.i164
  %tmp.i.i368 = load i32* %i.addr.i.i163          ; <i32> [#uses=1]
  %cmp.i.i369 = icmp sgt i32 %tmp.i.i368, 512     ; <i1> [#uses=1]
  br i1 %cmp.i.i369, label %if.then.i.i, label %k_sincos.exit.i

if.then.i.i:                                      ; preds = %kfft_pass3.exit
  %tmp1.i.i370 = load i32* %i.addr.i.i163         ; <i32> [#uses=1]
  %sub.i.i371 = sub i32 %tmp1.i.i370, 1024        ; <i32> [#uses=1]
  store i32 %sub.i.i371, i32* %i.addr.i.i163
  br label %k_sincos.exit.i

k_sincos.exit.i:                                  ; preds = %if.then.i.i, %kfft_pass3.exit
  %tmp3.i.i372 = load i32* %i.addr.i.i163         ; <i32> [#uses=1]
  %conv.i.i = sitofp i32 %tmp3.i.i372 to float    ; <float> [#uses=1]
  %mul.i.i373 = fmul float %conv.i.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i.i373, float* %x.i.i165
  %tmp4.i.i374 = load float* %x.i.i165            ; <float> [#uses=1]
  %call.i.i375 = call float @_Z10native_cosf(float %tmp4.i.i374) nounwind ; <float> [#uses=1]
  %tmp5.i.i = load float** %cretp.addr.i.i164     ; <float*> [#uses=1]
  store float %call.i.i375, float* %tmp5.i.i
  %tmp6.i.i376 = load float* %x.i.i165            ; <float> [#uses=1]
  %call7.i.i = call float @_Z10native_sinf(float %tmp6.i.i376) nounwind ; <float> [#uses=1]
  store float %call7.i.i, float* %retval.i.i162
  %11 = load float* %retval.i.i162                ; <float> [#uses=1]
  store float %11, float* %s1.i191
  %tmp262.i377 = load float* %c1.i190             ; <float> [#uses=1]
  %tmp263.i378 = insertelement <4 x float> undef, float %tmp262.i377, i32 0 ; <<4 x float>> [#uses=2]
  %splat.i379 = shufflevector <4 x float> %tmp263.i378, <4 x float> %tmp263.i378, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp264.i380 = load <4 x float>* %zr1.i170      ; <<4 x float>> [#uses=1]
  %mul265.i = fmul <4 x float> %splat.i379, %tmp264.i380 ; <<4 x float>> [#uses=1]
  %tmp266.i381 = load float* %s1.i191             ; <float> [#uses=1]
  %tmp267.i382 = insertelement <4 x float> undef, float %tmp266.i381, i32 0 ; <<4 x float>> [#uses=2]
  %splat268.i = shufflevector <4 x float> %tmp267.i382, <4 x float> %tmp267.i382, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp269.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %mul270.i = fmul <4 x float> %splat268.i, %tmp269.i ; <<4 x float>> [#uses=1]
  %sub271.i = fsub <4 x float> %mul265.i, %mul270.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub271.i, <4 x float>* %__r.i192
  %tmp272.i383 = load float* %c1.i190             ; <float> [#uses=1]
  %tmp273.i384 = insertelement <4 x float> undef, float %tmp272.i383, i32 0 ; <<4 x float>> [#uses=2]
  %splat274.i = shufflevector <4 x float> %tmp273.i384, <4 x float> %tmp273.i384, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp275.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %mul276.i = fmul <4 x float> %splat274.i, %tmp275.i ; <<4 x float>> [#uses=1]
  %tmp277.i385 = load float* %s1.i191             ; <float> [#uses=1]
  %tmp278.i386 = insertelement <4 x float> undef, float %tmp277.i385, i32 0 ; <<4 x float>> [#uses=2]
  %splat279.i = shufflevector <4 x float> %tmp278.i386, <4 x float> %tmp278.i386, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp280.i387 = load <4 x float>* %zr1.i170      ; <<4 x float>> [#uses=1]
  %mul281.i = fmul <4 x float> %splat279.i, %tmp280.i387 ; <<4 x float>> [#uses=1]
  %add282.i = fadd <4 x float> %mul276.i, %mul281.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add282.i, <4 x float>* %zi1.i174
  %tmp283.i = load <4 x float>* %__r.i192         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp283.i, <4 x float>* %zr1.i170
  %tmp287.i = load i32* %tbase.i189               ; <i32> [#uses=1]
  %mul288.i = mul i32 %tmp287.i, 2                ; <i32> [#uses=1]
  store i32 %mul288.i, i32* %i.addr.i514.i
  store float* %c2.i193, float** %cretp.addr.i515.i
  %tmp.i517.i = load i32* %i.addr.i514.i          ; <i32> [#uses=1]
  %cmp.i518.i = icmp sgt i32 %tmp.i517.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i518.i, label %if.then.i521.i, label %k_sincos.exit530.i

if.then.i521.i:                                   ; preds = %k_sincos.exit.i
  %tmp1.i519.i = load i32* %i.addr.i514.i         ; <i32> [#uses=1]
  %sub.i520.i = sub i32 %tmp1.i519.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i520.i, i32* %i.addr.i514.i
  br label %k_sincos.exit530.i

k_sincos.exit530.i:                               ; preds = %if.then.i521.i, %k_sincos.exit.i
  %tmp3.i522.i = load i32* %i.addr.i514.i         ; <i32> [#uses=1]
  %conv.i523.i = sitofp i32 %tmp3.i522.i to float ; <float> [#uses=1]
  %mul.i524.i = fmul float %conv.i523.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i524.i, float* %x.i516.i
  %tmp4.i525.i = load float* %x.i516.i            ; <float> [#uses=1]
  %call.i526.i = call float @_Z10native_cosf(float %tmp4.i525.i) nounwind ; <float> [#uses=1]
  %tmp5.i527.i = load float** %cretp.addr.i515.i  ; <float*> [#uses=1]
  store float %call.i526.i, float* %tmp5.i527.i
  %tmp6.i528.i = load float* %x.i516.i            ; <float> [#uses=1]
  %call7.i529.i = call float @_Z10native_sinf(float %tmp6.i528.i) nounwind ; <float> [#uses=1]
  store float %call7.i529.i, float* %retval.i513.i
  %12 = load float* %retval.i513.i                ; <float> [#uses=1]
  store float %12, float* %s2.i194
  %tmp293.i = load float* %c2.i193                ; <float> [#uses=1]
  %tmp294.i388 = insertelement <4 x float> undef, float %tmp293.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat295.i = shufflevector <4 x float> %tmp294.i388, <4 x float> %tmp294.i388, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp296.i389 = load <4 x float>* %zr2.i171      ; <<4 x float>> [#uses=1]
  %mul297.i = fmul <4 x float> %splat295.i, %tmp296.i389 ; <<4 x float>> [#uses=1]
  %tmp298.i390 = load float* %s2.i194             ; <float> [#uses=1]
  %tmp299.i391 = insertelement <4 x float> undef, float %tmp298.i390, i32 0 ; <<4 x float>> [#uses=2]
  %splat300.i = shufflevector <4 x float> %tmp299.i391, <4 x float> %tmp299.i391, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp301.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %mul302.i = fmul <4 x float> %splat300.i, %tmp301.i ; <<4 x float>> [#uses=1]
  %sub303.i = fsub <4 x float> %mul297.i, %mul302.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub303.i, <4 x float>* %__r292.i
  %tmp304.i392 = load float* %c2.i193             ; <float> [#uses=1]
  %tmp305.i = insertelement <4 x float> undef, float %tmp304.i392, i32 0 ; <<4 x float>> [#uses=2]
  %splat306.i = shufflevector <4 x float> %tmp305.i, <4 x float> %tmp305.i, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp307.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %mul308.i = fmul <4 x float> %splat306.i, %tmp307.i ; <<4 x float>> [#uses=1]
  %tmp309.i393 = load float* %s2.i194             ; <float> [#uses=1]
  %tmp310.i394 = insertelement <4 x float> undef, float %tmp309.i393, i32 0 ; <<4 x float>> [#uses=2]
  %splat311.i = shufflevector <4 x float> %tmp310.i394, <4 x float> %tmp310.i394, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp312.i395 = load <4 x float>* %zr2.i171      ; <<4 x float>> [#uses=1]
  %mul313.i = fmul <4 x float> %splat311.i, %tmp312.i395 ; <<4 x float>> [#uses=1]
  %add314.i = fadd <4 x float> %mul308.i, %mul313.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add314.i, <4 x float>* %zi2.i175
  %tmp315.i = load <4 x float>* %__r292.i         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp315.i, <4 x float>* %zr2.i171
  %tmp319.i = load i32* %tbase.i189               ; <i32> [#uses=1]
  %mul320.i = mul i32 %tmp319.i, 3                ; <i32> [#uses=1]
  store i32 %mul320.i, i32* %i.addr.i496.i
  store float* %c3.i195, float** %cretp.addr.i497.i
  %tmp.i499.i = load i32* %i.addr.i496.i          ; <i32> [#uses=1]
  %cmp.i500.i = icmp sgt i32 %tmp.i499.i, 512     ; <i1> [#uses=1]
  br i1 %cmp.i500.i, label %if.then.i503.i, label %kfft_pass4.exit

if.then.i503.i:                                   ; preds = %k_sincos.exit530.i
  %tmp1.i501.i = load i32* %i.addr.i496.i         ; <i32> [#uses=1]
  %sub.i502.i = sub i32 %tmp1.i501.i, 1024        ; <i32> [#uses=1]
  store i32 %sub.i502.i, i32* %i.addr.i496.i
  br label %kfft_pass4.exit

kfft_pass4.exit:                                  ; preds = %k_sincos.exit530.i, %if.then.i503.i
  %tmp3.i504.i = load i32* %i.addr.i496.i         ; <i32> [#uses=1]
  %conv.i505.i = sitofp i32 %tmp3.i504.i to float ; <float> [#uses=1]
  %mul.i506.i = fmul float %conv.i505.i, 0xBF7921FB60000000 ; <float> [#uses=1]
  store float %mul.i506.i, float* %x.i498.i
  %tmp4.i507.i = load float* %x.i498.i            ; <float> [#uses=1]
  %call.i508.i = call float @_Z10native_cosf(float %tmp4.i507.i) nounwind ; <float> [#uses=1]
  %tmp5.i509.i = load float** %cretp.addr.i497.i  ; <float*> [#uses=1]
  store float %call.i508.i, float* %tmp5.i509.i
  %tmp6.i510.i = load float* %x.i498.i            ; <float> [#uses=1]
  %call7.i511.i = call float @_Z10native_sinf(float %tmp6.i510.i) nounwind ; <float> [#uses=1]
  store float %call7.i511.i, float* %retval.i495.i
  %13 = load float* %retval.i495.i                ; <float> [#uses=1]
  store float %13, float* %s3.i196
  %tmp325.i = load float* %c3.i195                ; <float> [#uses=1]
  %tmp326.i = insertelement <4 x float> undef, float %tmp325.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat327.i = shufflevector <4 x float> %tmp326.i, <4 x float> %tmp326.i, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp328.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %mul329.i = fmul <4 x float> %splat327.i, %tmp328.i ; <<4 x float>> [#uses=1]
  %tmp330.i = load float* %s3.i196                ; <float> [#uses=1]
  %tmp331.i = insertelement <4 x float> undef, float %tmp330.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat332.i = shufflevector <4 x float> %tmp331.i, <4 x float> %tmp331.i, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp333.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %mul334.i = fmul <4 x float> %splat332.i, %tmp333.i ; <<4 x float>> [#uses=1]
  %sub335.i = fsub <4 x float> %mul329.i, %mul334.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub335.i, <4 x float>* %__r324.i
  %tmp336.i = load float* %c3.i195                ; <float> [#uses=1]
  %tmp337.i = insertelement <4 x float> undef, float %tmp336.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat338.i = shufflevector <4 x float> %tmp337.i, <4 x float> %tmp337.i, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp339.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %mul340.i = fmul <4 x float> %splat338.i, %tmp339.i ; <<4 x float>> [#uses=1]
  %tmp341.i = load float* %s3.i196                ; <float> [#uses=1]
  %tmp342.i = insertelement <4 x float> undef, float %tmp341.i, i32 0 ; <<4 x float>> [#uses=2]
  %splat343.i = shufflevector <4 x float> %tmp342.i, <4 x float> %tmp342.i, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp344.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %mul345.i = fmul <4 x float> %splat343.i, %tmp344.i ; <<4 x float>> [#uses=1]
  %add346.i = fadd <4 x float> %mul340.i, %mul345.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add346.i, <4 x float>* %zi3.i176
  %tmp347.i = load <4 x float>* %__r324.i         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp347.i, <4 x float>* %zr3.i172
  call void @barrier(i32 1) nounwind
  %tmp350.i = load float addrspace(3)** %lds.addr.i167 ; <float addrspace(3)*> [#uses=1]
  %tmp351.i = load i32* %me.addr.i166             ; <i32> [#uses=1]
  %add.ptr352.i = getelementptr inbounds float addrspace(3)* %tmp350.i, i32 %tmp351.i ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr352.i, float addrspace(3)** %lp.i168
  %tmp353.i = load <4 x float>* %zr0.i169         ; <<4 x float>> [#uses=1]
  %tmp354.i = extractelement <4 x float> %tmp353.i, i32 0 ; <float> [#uses=1]
  %tmp355.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx356.i = getelementptr inbounds float addrspace(3)* %tmp355.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp354.i, float addrspace(3)* %arrayidx356.i
  %tmp357.i = load <4 x float>* %zr0.i169         ; <<4 x float>> [#uses=1]
  %tmp358.i = extractelement <4 x float> %tmp357.i, i32 1 ; <float> [#uses=1]
  %tmp359.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx360.i = getelementptr inbounds float addrspace(3)* %tmp359.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp358.i, float addrspace(3)* %arrayidx360.i
  %tmp361.i = load <4 x float>* %zr0.i169         ; <<4 x float>> [#uses=1]
  %tmp362.i = extractelement <4 x float> %tmp361.i, i32 2 ; <float> [#uses=1]
  %tmp363.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx364.i = getelementptr inbounds float addrspace(3)* %tmp363.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp362.i, float addrspace(3)* %arrayidx364.i
  %tmp365.i = load <4 x float>* %zr0.i169         ; <<4 x float>> [#uses=1]
  %tmp366.i = extractelement <4 x float> %tmp365.i, i32 3 ; <float> [#uses=1]
  %tmp367.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx368.i = getelementptr inbounds float addrspace(3)* %tmp367.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp366.i, float addrspace(3)* %arrayidx368.i
  %tmp369.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr370.i = getelementptr inbounds float addrspace(3)* %tmp369.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr370.i, float addrspace(3)** %lp.i168
  %tmp371.i = load <4 x float>* %zr1.i170         ; <<4 x float>> [#uses=1]
  %tmp372.i = extractelement <4 x float> %tmp371.i, i32 0 ; <float> [#uses=1]
  %tmp373.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx374.i = getelementptr inbounds float addrspace(3)* %tmp373.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp372.i, float addrspace(3)* %arrayidx374.i
  %tmp375.i = load <4 x float>* %zr1.i170         ; <<4 x float>> [#uses=1]
  %tmp376.i = extractelement <4 x float> %tmp375.i, i32 1 ; <float> [#uses=1]
  %tmp377.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx378.i = getelementptr inbounds float addrspace(3)* %tmp377.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp376.i, float addrspace(3)* %arrayidx378.i
  %tmp379.i = load <4 x float>* %zr1.i170         ; <<4 x float>> [#uses=1]
  %tmp380.i = extractelement <4 x float> %tmp379.i, i32 2 ; <float> [#uses=1]
  %tmp381.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx382.i = getelementptr inbounds float addrspace(3)* %tmp381.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp380.i, float addrspace(3)* %arrayidx382.i
  %tmp383.i = load <4 x float>* %zr1.i170         ; <<4 x float>> [#uses=1]
  %tmp384.i = extractelement <4 x float> %tmp383.i, i32 3 ; <float> [#uses=1]
  %tmp385.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx386.i = getelementptr inbounds float addrspace(3)* %tmp385.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp384.i, float addrspace(3)* %arrayidx386.i
  %tmp387.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr388.i = getelementptr inbounds float addrspace(3)* %tmp387.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr388.i, float addrspace(3)** %lp.i168
  %tmp389.i = load <4 x float>* %zr2.i171         ; <<4 x float>> [#uses=1]
  %tmp390.i = extractelement <4 x float> %tmp389.i, i32 0 ; <float> [#uses=1]
  %tmp391.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx392.i = getelementptr inbounds float addrspace(3)* %tmp391.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp390.i, float addrspace(3)* %arrayidx392.i
  %tmp393.i = load <4 x float>* %zr2.i171         ; <<4 x float>> [#uses=1]
  %tmp394.i = extractelement <4 x float> %tmp393.i, i32 1 ; <float> [#uses=1]
  %tmp395.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx396.i = getelementptr inbounds float addrspace(3)* %tmp395.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp394.i, float addrspace(3)* %arrayidx396.i
  %tmp397.i = load <4 x float>* %zr2.i171         ; <<4 x float>> [#uses=1]
  %tmp398.i = extractelement <4 x float> %tmp397.i, i32 2 ; <float> [#uses=1]
  %tmp399.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx400.i = getelementptr inbounds float addrspace(3)* %tmp399.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp398.i, float addrspace(3)* %arrayidx400.i
  %tmp401.i = load <4 x float>* %zr2.i171         ; <<4 x float>> [#uses=1]
  %tmp402.i = extractelement <4 x float> %tmp401.i, i32 3 ; <float> [#uses=1]
  %tmp403.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx404.i = getelementptr inbounds float addrspace(3)* %tmp403.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp402.i, float addrspace(3)* %arrayidx404.i
  %tmp405.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr406.i = getelementptr inbounds float addrspace(3)* %tmp405.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr406.i, float addrspace(3)** %lp.i168
  %tmp407.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %tmp408.i = extractelement <4 x float> %tmp407.i, i32 0 ; <float> [#uses=1]
  %tmp409.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx410.i = getelementptr inbounds float addrspace(3)* %tmp409.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp408.i, float addrspace(3)* %arrayidx410.i
  %tmp411.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %tmp412.i = extractelement <4 x float> %tmp411.i, i32 1 ; <float> [#uses=1]
  %tmp413.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx414.i = getelementptr inbounds float addrspace(3)* %tmp413.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp412.i, float addrspace(3)* %arrayidx414.i
  %tmp415.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %tmp416.i = extractelement <4 x float> %tmp415.i, i32 2 ; <float> [#uses=1]
  %tmp417.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx418.i = getelementptr inbounds float addrspace(3)* %tmp417.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp416.i, float addrspace(3)* %arrayidx418.i
  %tmp419.i = load <4 x float>* %zr3.i172         ; <<4 x float>> [#uses=1]
  %tmp420.i = extractelement <4 x float> %tmp419.i, i32 3 ; <float> [#uses=1]
  %tmp421.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx422.i = getelementptr inbounds float addrspace(3)* %tmp421.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp420.i, float addrspace(3)* %arrayidx422.i
  %tmp423.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr424.i = getelementptr inbounds float addrspace(3)* %tmp423.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr424.i, float addrspace(3)** %lp.i168
  %tmp425.i = load <4 x float>* %zi0.i173         ; <<4 x float>> [#uses=1]
  %tmp426.i = extractelement <4 x float> %tmp425.i, i32 0 ; <float> [#uses=1]
  %tmp427.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx428.i = getelementptr inbounds float addrspace(3)* %tmp427.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp426.i, float addrspace(3)* %arrayidx428.i
  %tmp429.i = load <4 x float>* %zi0.i173         ; <<4 x float>> [#uses=1]
  %tmp430.i = extractelement <4 x float> %tmp429.i, i32 1 ; <float> [#uses=1]
  %tmp431.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx432.i = getelementptr inbounds float addrspace(3)* %tmp431.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp430.i, float addrspace(3)* %arrayidx432.i
  %tmp433.i = load <4 x float>* %zi0.i173         ; <<4 x float>> [#uses=1]
  %tmp434.i = extractelement <4 x float> %tmp433.i, i32 2 ; <float> [#uses=1]
  %tmp435.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx436.i = getelementptr inbounds float addrspace(3)* %tmp435.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp434.i, float addrspace(3)* %arrayidx436.i
  %tmp437.i = load <4 x float>* %zi0.i173         ; <<4 x float>> [#uses=1]
  %tmp438.i = extractelement <4 x float> %tmp437.i, i32 3 ; <float> [#uses=1]
  %tmp439.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx440.i = getelementptr inbounds float addrspace(3)* %tmp439.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp438.i, float addrspace(3)* %arrayidx440.i
  %tmp441.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr442.i = getelementptr inbounds float addrspace(3)* %tmp441.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr442.i, float addrspace(3)** %lp.i168
  %tmp443.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %tmp444.i = extractelement <4 x float> %tmp443.i, i32 0 ; <float> [#uses=1]
  %tmp445.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx446.i = getelementptr inbounds float addrspace(3)* %tmp445.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp444.i, float addrspace(3)* %arrayidx446.i
  %tmp447.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %tmp448.i = extractelement <4 x float> %tmp447.i, i32 1 ; <float> [#uses=1]
  %tmp449.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx450.i = getelementptr inbounds float addrspace(3)* %tmp449.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp448.i, float addrspace(3)* %arrayidx450.i
  %tmp451.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %tmp452.i = extractelement <4 x float> %tmp451.i, i32 2 ; <float> [#uses=1]
  %tmp453.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx454.i = getelementptr inbounds float addrspace(3)* %tmp453.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp452.i, float addrspace(3)* %arrayidx454.i
  %tmp455.i = load <4 x float>* %zi1.i174         ; <<4 x float>> [#uses=1]
  %tmp456.i = extractelement <4 x float> %tmp455.i, i32 3 ; <float> [#uses=1]
  %tmp457.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx458.i = getelementptr inbounds float addrspace(3)* %tmp457.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp456.i, float addrspace(3)* %arrayidx458.i
  %tmp459.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr460.i = getelementptr inbounds float addrspace(3)* %tmp459.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr460.i, float addrspace(3)** %lp.i168
  %tmp461.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %tmp462.i = extractelement <4 x float> %tmp461.i, i32 0 ; <float> [#uses=1]
  %tmp463.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx464.i = getelementptr inbounds float addrspace(3)* %tmp463.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp462.i, float addrspace(3)* %arrayidx464.i
  %tmp465.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %tmp466.i = extractelement <4 x float> %tmp465.i, i32 1 ; <float> [#uses=1]
  %tmp467.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx468.i = getelementptr inbounds float addrspace(3)* %tmp467.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp466.i, float addrspace(3)* %arrayidx468.i
  %tmp469.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %tmp470.i = extractelement <4 x float> %tmp469.i, i32 2 ; <float> [#uses=1]
  %tmp471.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx472.i = getelementptr inbounds float addrspace(3)* %tmp471.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp470.i, float addrspace(3)* %arrayidx472.i
  %tmp473.i = load <4 x float>* %zi2.i175         ; <<4 x float>> [#uses=1]
  %tmp474.i = extractelement <4 x float> %tmp473.i, i32 3 ; <float> [#uses=1]
  %tmp475.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx476.i = getelementptr inbounds float addrspace(3)* %tmp475.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp474.i, float addrspace(3)* %arrayidx476.i
  %tmp477.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %add.ptr478.i = getelementptr inbounds float addrspace(3)* %tmp477.i, i32 272 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr478.i, float addrspace(3)** %lp.i168
  %tmp479.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %tmp480.i = extractelement <4 x float> %tmp479.i, i32 0 ; <float> [#uses=1]
  %tmp481.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx482.i = getelementptr inbounds float addrspace(3)* %tmp481.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  store float %tmp480.i, float addrspace(3)* %arrayidx482.i
  %tmp483.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %tmp484.i = extractelement <4 x float> %tmp483.i, i32 1 ; <float> [#uses=1]
  %tmp485.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx486.i = getelementptr inbounds float addrspace(3)* %tmp485.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  store float %tmp484.i, float addrspace(3)* %arrayidx486.i
  %tmp487.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %tmp488.i = extractelement <4 x float> %tmp487.i, i32 2 ; <float> [#uses=1]
  %tmp489.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx490.i = getelementptr inbounds float addrspace(3)* %tmp489.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  store float %tmp488.i, float addrspace(3)* %arrayidx490.i
  %tmp491.i = load <4 x float>* %zi3.i176         ; <<4 x float>> [#uses=1]
  %tmp492.i = extractelement <4 x float> %tmp491.i, i32 3 ; <float> [#uses=1]
  %tmp493.i = load float addrspace(3)** %lp.i168  ; <float addrspace(3)*> [#uses=1]
  %arrayidx494.i = getelementptr inbounds float addrspace(3)* %tmp493.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  store float %tmp492.i, float addrspace(3)* %arrayidx494.i
  call void @barrier(i32 1) nounwind
  %tmp14 = load i32* %me                          ; <i32> [#uses=1]
  %tmp15 = load float addrspace(1)** %gr          ; <float addrspace(1)*> [#uses=1]
  %tmp16 = load float addrspace(1)** %gi          ; <float addrspace(1)*> [#uses=1]
  store i32 %tmp14, i32* %me.addr.i17
  store float addrspace(3)* getelementptr inbounds ([2176 x float] addrspace(3)* @opencl_kfft_local_lds, i32 0, i32 0), float addrspace(3)** %lds.addr.i18
  store float addrspace(1)* %tmp15, float addrspace(1)** %gr.addr.i19
  store float addrspace(1)* %tmp16, float addrspace(1)** %gi.addr.i20
  %tmp.i43 = load float addrspace(3)** %lds.addr.i18 ; <float addrspace(3)*> [#uses=1]
  %tmp1.i44 = load i32* %me.addr.i17              ; <i32> [#uses=1]
  %and.i = and i32 %tmp1.i44, 15                  ; <i32> [#uses=1]
  %tmp2.i = load i32* %me.addr.i17                ; <i32> [#uses=1]
  %shr.i45 = lshr i32 %tmp2.i, 4                  ; <i32> [#uses=1]
  %mul.i46 = mul i32 %shr.i45, 272                ; <i32> [#uses=1]
  %add.i47 = add i32 %and.i, %mul.i46             ; <i32> [#uses=1]
  %add.ptr.i48 = getelementptr inbounds float addrspace(3)* %tmp.i43, i32 %add.i47 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr.i48, float addrspace(3)** %lp.i21
  %tmp7.i = load float addrspace(3)** %lp.i21     ; <float addrspace(3)*> [#uses=1]
  %arrayidx.i49 = getelementptr inbounds float addrspace(3)* %tmp7.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp8.i50 = load float addrspace(3)* %arrayidx.i49 ; <float> [#uses=1]
  %tmp9.i = load <4 x float>* %zr0.i22            ; <<4 x float>> [#uses=1]
  %tmp10.i51 = insertelement <4 x float> %tmp9.i, float %tmp8.i50, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10.i51, <4 x float>* %zr0.i22
  %tmp11.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx12.i = getelementptr inbounds float addrspace(3)* %tmp11.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp13.i = load float addrspace(3)* %arrayidx12.i ; <float> [#uses=1]
  %tmp14.i52 = load <4 x float>* %zr0.i22         ; <<4 x float>> [#uses=1]
  %tmp15.i = insertelement <4 x float> %tmp14.i52, float %tmp13.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp15.i, <4 x float>* %zr0.i22
  %tmp16.i53 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx17.i = getelementptr inbounds float addrspace(3)* %tmp16.i53, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp18.i54 = load float addrspace(3)* %arrayidx17.i ; <float> [#uses=1]
  %tmp19.i = load <4 x float>* %zr0.i22           ; <<4 x float>> [#uses=1]
  %tmp20.i = insertelement <4 x float> %tmp19.i, float %tmp18.i54, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp20.i, <4 x float>* %zr0.i22
  %tmp21.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx22.i = getelementptr inbounds float addrspace(3)* %tmp21.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp23.i = load float addrspace(3)* %arrayidx22.i ; <float> [#uses=1]
  %tmp24.i55 = load <4 x float>* %zr0.i22         ; <<4 x float>> [#uses=1]
  %tmp25.i = insertelement <4 x float> %tmp24.i55, float %tmp23.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25.i, <4 x float>* %zr0.i22
  %tmp26.i56 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %add.ptr27.i = getelementptr inbounds float addrspace(3)* %tmp26.i56, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr27.i, float addrspace(3)** %lp.i21
  %tmp28.i57 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx29.i = getelementptr inbounds float addrspace(3)* %tmp28.i57, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp30.i58 = load float addrspace(3)* %arrayidx29.i ; <float> [#uses=1]
  %tmp31.i = load <4 x float>* %zr1.i23           ; <<4 x float>> [#uses=1]
  %tmp32.i59 = insertelement <4 x float> %tmp31.i, float %tmp30.i58, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp32.i59, <4 x float>* %zr1.i23
  %tmp33.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx34.i = getelementptr inbounds float addrspace(3)* %tmp33.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp35.i = load float addrspace(3)* %arrayidx34.i ; <float> [#uses=1]
  %tmp36.i60 = load <4 x float>* %zr1.i23         ; <<4 x float>> [#uses=1]
  %tmp37.i = insertelement <4 x float> %tmp36.i60, float %tmp35.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37.i, <4 x float>* %zr1.i23
  %tmp38.i61 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx39.i = getelementptr inbounds float addrspace(3)* %tmp38.i61, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp40.i = load float addrspace(3)* %arrayidx39.i ; <float> [#uses=1]
  %tmp41.i62 = load <4 x float>* %zr1.i23         ; <<4 x float>> [#uses=1]
  %tmp42.i63 = insertelement <4 x float> %tmp41.i62, float %tmp40.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42.i63, <4 x float>* %zr1.i23
  %tmp43.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx44.i = getelementptr inbounds float addrspace(3)* %tmp43.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp45.i64 = load float addrspace(3)* %arrayidx44.i ; <float> [#uses=1]
  %tmp46.i65 = load <4 x float>* %zr1.i23         ; <<4 x float>> [#uses=1]
  %tmp47.i = insertelement <4 x float> %tmp46.i65, float %tmp45.i64, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47.i, <4 x float>* %zr1.i23
  %tmp48.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %add.ptr49.i = getelementptr inbounds float addrspace(3)* %tmp48.i, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr49.i, float addrspace(3)** %lp.i21
  %tmp50.i66 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx51.i = getelementptr inbounds float addrspace(3)* %tmp50.i66, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp52.i67 = load float addrspace(3)* %arrayidx51.i ; <float> [#uses=1]
  %tmp53.i68 = load <4 x float>* %zr2.i24         ; <<4 x float>> [#uses=1]
  %tmp54.i = insertelement <4 x float> %tmp53.i68, float %tmp52.i67, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54.i, <4 x float>* %zr2.i24
  %tmp55.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx56.i = getelementptr inbounds float addrspace(3)* %tmp55.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp57.i69 = load float addrspace(3)* %arrayidx56.i ; <float> [#uses=1]
  %tmp58.i = load <4 x float>* %zr2.i24           ; <<4 x float>> [#uses=1]
  %tmp59.i = insertelement <4 x float> %tmp58.i, float %tmp57.i69, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59.i, <4 x float>* %zr2.i24
  %tmp60.i70 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx61.i = getelementptr inbounds float addrspace(3)* %tmp60.i70, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp62.i = load float addrspace(3)* %arrayidx61.i ; <float> [#uses=1]
  %tmp63.i = load <4 x float>* %zr2.i24           ; <<4 x float>> [#uses=1]
  %tmp64.i71 = insertelement <4 x float> %tmp63.i, float %tmp62.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp64.i71, <4 x float>* %zr2.i24
  %tmp65.i72 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx66.i = getelementptr inbounds float addrspace(3)* %tmp65.i72, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp67.i = load float addrspace(3)* %arrayidx66.i ; <float> [#uses=1]
  %tmp68.i73 = load <4 x float>* %zr2.i24         ; <<4 x float>> [#uses=1]
  %tmp69.i74 = insertelement <4 x float> %tmp68.i73, float %tmp67.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp69.i74, <4 x float>* %zr2.i24
  %tmp70.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %add.ptr71.i = getelementptr inbounds float addrspace(3)* %tmp70.i, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr71.i, float addrspace(3)** %lp.i21
  %tmp72.i75 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx73.i = getelementptr inbounds float addrspace(3)* %tmp72.i75, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp74.i = load float addrspace(3)* %arrayidx73.i ; <float> [#uses=1]
  %tmp75.i = load <4 x float>* %zr3.i25           ; <<4 x float>> [#uses=1]
  %tmp76.i76 = insertelement <4 x float> %tmp75.i, float %tmp74.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp76.i76, <4 x float>* %zr3.i25
  %tmp77.i77 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx78.i = getelementptr inbounds float addrspace(3)* %tmp77.i77, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp79.i = load float addrspace(3)* %arrayidx78.i ; <float> [#uses=1]
  %tmp80.i78 = load <4 x float>* %zr3.i25         ; <<4 x float>> [#uses=1]
  %tmp81.i79 = insertelement <4 x float> %tmp80.i78, float %tmp79.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81.i79, <4 x float>* %zr3.i25
  %tmp82.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx83.i = getelementptr inbounds float addrspace(3)* %tmp82.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp84.i80 = load float addrspace(3)* %arrayidx83.i ; <float> [#uses=1]
  %tmp85.i81 = load <4 x float>* %zr3.i25         ; <<4 x float>> [#uses=1]
  %tmp86.i82 = insertelement <4 x float> %tmp85.i81, float %tmp84.i80, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86.i82, <4 x float>* %zr3.i25
  %tmp87.i = load float addrspace(3)** %lp.i21    ; <float addrspace(3)*> [#uses=1]
  %arrayidx88.i = getelementptr inbounds float addrspace(3)* %tmp87.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp89.i83 = load float addrspace(3)* %arrayidx88.i ; <float> [#uses=1]
  %tmp90.i = load <4 x float>* %zr3.i25           ; <<4 x float>> [#uses=1]
  %tmp91.i84 = insertelement <4 x float> %tmp90.i, float %tmp89.i83, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp91.i84, <4 x float>* %zr3.i25
  %tmp92.i85 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %add.ptr93.i = getelementptr inbounds float addrspace(3)* %tmp92.i85, i32 1040 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr93.i, float addrspace(3)** %lp.i21
  %tmp98.i86 = load float addrspace(3)** %lp.i21  ; <float addrspace(3)*> [#uses=1]
  %arrayidx99.i = getelementptr inbounds float addrspace(3)* %tmp98.i86, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp100.i87 = load float addrspace(3)* %arrayidx99.i ; <float> [#uses=1]
  %tmp101.i = load <4 x float>* %zi0.i26          ; <<4 x float>> [#uses=1]
  %tmp102.i88 = insertelement <4 x float> %tmp101.i, float %tmp100.i87, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp102.i88, <4 x float>* %zi0.i26
  %tmp103.i89 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx104.i = getelementptr inbounds float addrspace(3)* %tmp103.i89, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp105.i = load float addrspace(3)* %arrayidx104.i ; <float> [#uses=1]
  %tmp106.i = load <4 x float>* %zi0.i26          ; <<4 x float>> [#uses=1]
  %tmp107.i = insertelement <4 x float> %tmp106.i, float %tmp105.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107.i, <4 x float>* %zi0.i26
  %tmp108.i90 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx109.i = getelementptr inbounds float addrspace(3)* %tmp108.i90, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp110.i = load float addrspace(3)* %arrayidx109.i ; <float> [#uses=1]
  %tmp111.i91 = load <4 x float>* %zi0.i26        ; <<4 x float>> [#uses=1]
  %tmp112.i92 = insertelement <4 x float> %tmp111.i91, float %tmp110.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112.i92, <4 x float>* %zi0.i26
  %tmp113.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx114.i = getelementptr inbounds float addrspace(3)* %tmp113.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp115.i93 = load float addrspace(3)* %arrayidx114.i ; <float> [#uses=1]
  %tmp116.i = load <4 x float>* %zi0.i26          ; <<4 x float>> [#uses=1]
  %tmp117.i = insertelement <4 x float> %tmp116.i, float %tmp115.i93, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117.i, <4 x float>* %zi0.i26
  %tmp118.i94 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %add.ptr119.i = getelementptr inbounds float addrspace(3)* %tmp118.i94, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr119.i, float addrspace(3)** %lp.i21
  %tmp120.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx121.i = getelementptr inbounds float addrspace(3)* %tmp120.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp122.i95 = load float addrspace(3)* %arrayidx121.i ; <float> [#uses=1]
  %tmp123.i = load <4 x float>* %zi1.i27          ; <<4 x float>> [#uses=1]
  %tmp124.i = insertelement <4 x float> %tmp123.i, float %tmp122.i95, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp124.i, <4 x float>* %zi1.i27
  %tmp125.i96 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx126.i = getelementptr inbounds float addrspace(3)* %tmp125.i96, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp127.i = load float addrspace(3)* %arrayidx126.i ; <float> [#uses=1]
  %tmp128.i = load <4 x float>* %zi1.i27          ; <<4 x float>> [#uses=1]
  %tmp129.i97 = insertelement <4 x float> %tmp128.i, float %tmp127.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129.i97, <4 x float>* %zi1.i27
  %tmp130.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx131.i = getelementptr inbounds float addrspace(3)* %tmp130.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp132.i = load float addrspace(3)* %arrayidx131.i ; <float> [#uses=1]
  %tmp133.i = load <4 x float>* %zi1.i27          ; <<4 x float>> [#uses=1]
  %tmp134.i = insertelement <4 x float> %tmp133.i, float %tmp132.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134.i, <4 x float>* %zi1.i27
  %tmp135.i98 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx136.i = getelementptr inbounds float addrspace(3)* %tmp135.i98, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp137.i = load float addrspace(3)* %arrayidx136.i ; <float> [#uses=1]
  %tmp138.i99 = load <4 x float>* %zi1.i27        ; <<4 x float>> [#uses=1]
  %tmp139.i100 = insertelement <4 x float> %tmp138.i99, float %tmp137.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp139.i100, <4 x float>* %zi1.i27
  %tmp140.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %add.ptr141.i = getelementptr inbounds float addrspace(3)* %tmp140.i, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr141.i, float addrspace(3)** %lp.i21
  %tmp142.i101 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx143.i = getelementptr inbounds float addrspace(3)* %tmp142.i101, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp144.i = load float addrspace(3)* %arrayidx143.i ; <float> [#uses=1]
  %tmp145.i102 = load <4 x float>* %zi2.i28       ; <<4 x float>> [#uses=1]
  %tmp146.i103 = insertelement <4 x float> %tmp145.i102, float %tmp144.i, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146.i103, <4 x float>* %zi2.i28
  %tmp147.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx148.i = getelementptr inbounds float addrspace(3)* %tmp147.i, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp149.i104 = load float addrspace(3)* %arrayidx148.i ; <float> [#uses=1]
  %tmp150.i = load <4 x float>* %zi2.i28          ; <<4 x float>> [#uses=1]
  %tmp151.i = insertelement <4 x float> %tmp150.i, float %tmp149.i104, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp151.i, <4 x float>* %zi2.i28
  %tmp152.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx153.i = getelementptr inbounds float addrspace(3)* %tmp152.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp154.i = load float addrspace(3)* %arrayidx153.i ; <float> [#uses=1]
  %tmp155.i = load <4 x float>* %zi2.i28          ; <<4 x float>> [#uses=1]
  %tmp156.i = insertelement <4 x float> %tmp155.i, float %tmp154.i, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp156.i, <4 x float>* %zi2.i28
  %tmp157.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx158.i = getelementptr inbounds float addrspace(3)* %tmp157.i, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp159.i105 = load float addrspace(3)* %arrayidx158.i ; <float> [#uses=1]
  %tmp160.i106 = load <4 x float>* %zi2.i28       ; <<4 x float>> [#uses=1]
  %tmp161.i = insertelement <4 x float> %tmp160.i106, float %tmp159.i105, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161.i, <4 x float>* %zi2.i28
  %tmp162.i107 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %add.ptr163.i = getelementptr inbounds float addrspace(3)* %tmp162.i107, i32 16 ; <float addrspace(3)*> [#uses=1]
  store float addrspace(3)* %add.ptr163.i, float addrspace(3)** %lp.i21
  %tmp164.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx165.i = getelementptr inbounds float addrspace(3)* %tmp164.i, i32 0 ; <float addrspace(3)*> [#uses=1]
  %tmp166.i108 = load float addrspace(3)* %arrayidx165.i ; <float> [#uses=1]
  %tmp167.i109 = load <4 x float>* %zi3.i29       ; <<4 x float>> [#uses=1]
  %tmp168.i = insertelement <4 x float> %tmp167.i109, float %tmp166.i108, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp168.i, <4 x float>* %zi3.i29
  %tmp169.i110 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx170.i = getelementptr inbounds float addrspace(3)* %tmp169.i110, i32 68 ; <float addrspace(3)*> [#uses=1]
  %tmp171.i = load float addrspace(3)* %arrayidx170.i ; <float> [#uses=1]
  %tmp172.i = load <4 x float>* %zi3.i29          ; <<4 x float>> [#uses=1]
  %tmp173.i111 = insertelement <4 x float> %tmp172.i, float %tmp171.i, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173.i111, <4 x float>* %zi3.i29
  %tmp174.i = load float addrspace(3)** %lp.i21   ; <float addrspace(3)*> [#uses=1]
  %arrayidx175.i = getelementptr inbounds float addrspace(3)* %tmp174.i, i32 136 ; <float addrspace(3)*> [#uses=1]
  %tmp176.i112 = load float addrspace(3)* %arrayidx175.i ; <float> [#uses=1]
  %tmp177.i113 = load <4 x float>* %zi3.i29       ; <<4 x float>> [#uses=1]
  %tmp178.i = insertelement <4 x float> %tmp177.i113, float %tmp176.i112, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp178.i, <4 x float>* %zi3.i29
  %tmp179.i114 = load float addrspace(3)** %lp.i21 ; <float addrspace(3)*> [#uses=1]
  %arrayidx180.i = getelementptr inbounds float addrspace(3)* %tmp179.i114, i32 204 ; <float addrspace(3)*> [#uses=1]
  %tmp181.i = load float addrspace(3)* %arrayidx180.i ; <float> [#uses=1]
  %tmp182.i115 = load <4 x float>* %zi3.i29       ; <<4 x float>> [#uses=1]
  %tmp183.i116 = insertelement <4 x float> %tmp182.i115, float %tmp181.i, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp183.i116, <4 x float>* %zi3.i29
  %tmp185.i = load <4 x float>* %zr0.i22          ; <<4 x float>> [#uses=1]
  %tmp186.i117 = load <4 x float>* %zr2.i24       ; <<4 x float>> [#uses=1]
  %add187.i = fadd <4 x float> %tmp185.i, %tmp186.i117 ; <<4 x float>> [#uses=1]
  store <4 x float> %add187.i, <4 x float>* %ar0.i30
  %tmp189.i = load <4 x float>* %zr1.i23          ; <<4 x float>> [#uses=1]
  %tmp190.i118 = load <4 x float>* %zr3.i25       ; <<4 x float>> [#uses=1]
  %add191.i = fadd <4 x float> %tmp189.i, %tmp190.i118 ; <<4 x float>> [#uses=1]
  store <4 x float> %add191.i, <4 x float>* %ar2.i31
  %tmp193.i = load <4 x float>* %ar0.i30          ; <<4 x float>> [#uses=1]
  %tmp194.i119 = load <4 x float>* %ar2.i31       ; <<4 x float>> [#uses=1]
  %add195.i = fadd <4 x float> %tmp193.i, %tmp194.i119 ; <<4 x float>> [#uses=1]
  store <4 x float> %add195.i, <4 x float>* %br0.i32
  %tmp197.i = load <4 x float>* %zr0.i22          ; <<4 x float>> [#uses=1]
  %tmp198.i120 = load <4 x float>* %zr2.i24       ; <<4 x float>> [#uses=1]
  %sub.i121 = fsub <4 x float> %tmp197.i, %tmp198.i120 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub.i121, <4 x float>* %br1.i33
  %tmp200.i122 = load <4 x float>* %ar0.i30       ; <<4 x float>> [#uses=1]
  %tmp201.i123 = load <4 x float>* %ar2.i31       ; <<4 x float>> [#uses=1]
  %sub202.i = fsub <4 x float> %tmp200.i122, %tmp201.i123 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub202.i, <4 x float>* %br2.i34
  %tmp204.i124 = load <4 x float>* %zr1.i23       ; <<4 x float>> [#uses=1]
  %tmp205.i125 = load <4 x float>* %zr3.i25       ; <<4 x float>> [#uses=1]
  %sub206.i = fsub <4 x float> %tmp204.i124, %tmp205.i125 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub206.i, <4 x float>* %br3.i35
  %tmp208.i126 = load <4 x float>* %zi0.i26       ; <<4 x float>> [#uses=1]
  %tmp209.i127 = load <4 x float>* %zi2.i28       ; <<4 x float>> [#uses=1]
  %add210.i = fadd <4 x float> %tmp208.i126, %tmp209.i127 ; <<4 x float>> [#uses=1]
  store <4 x float> %add210.i, <4 x float>* %ai0.i36
  %tmp212.i128 = load <4 x float>* %zi1.i27       ; <<4 x float>> [#uses=1]
  %tmp213.i129 = load <4 x float>* %zi3.i29       ; <<4 x float>> [#uses=1]
  %add214.i = fadd <4 x float> %tmp212.i128, %tmp213.i129 ; <<4 x float>> [#uses=1]
  store <4 x float> %add214.i, <4 x float>* %ai2.i37
  %tmp216.i130 = load <4 x float>* %ai0.i36       ; <<4 x float>> [#uses=1]
  %tmp217.i = load <4 x float>* %ai2.i37          ; <<4 x float>> [#uses=1]
  %add218.i = fadd <4 x float> %tmp216.i130, %tmp217.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add218.i, <4 x float>* %bi0.i38
  %tmp220.i131 = load <4 x float>* %zi0.i26       ; <<4 x float>> [#uses=1]
  %tmp221.i = load <4 x float>* %zi2.i28          ; <<4 x float>> [#uses=1]
  %sub222.i = fsub <4 x float> %tmp220.i131, %tmp221.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub222.i, <4 x float>* %bi1.i39
  %tmp224.i132 = load <4 x float>* %ai0.i36       ; <<4 x float>> [#uses=1]
  %tmp225.i = load <4 x float>* %ai2.i37          ; <<4 x float>> [#uses=1]
  %sub226.i = fsub <4 x float> %tmp224.i132, %tmp225.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub226.i, <4 x float>* %bi2.i40
  %tmp228.i133 = load <4 x float>* %zi1.i27       ; <<4 x float>> [#uses=1]
  %tmp229.i = load <4 x float>* %zi3.i29          ; <<4 x float>> [#uses=1]
  %sub230.i = fsub <4 x float> %tmp228.i133, %tmp229.i ; <<4 x float>> [#uses=1]
  store <4 x float> %sub230.i, <4 x float>* %bi3.i41
  %tmp231.i134 = load <4 x float>* %br0.i32       ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp231.i134, <4 x float>* %zr0.i22
  %tmp232.i135 = load <4 x float>* %bi0.i38       ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp232.i135, <4 x float>* %zi0.i26
  %tmp233.i = load <4 x float>* %br1.i33          ; <<4 x float>> [#uses=1]
  %tmp234.i136 = load <4 x float>* %bi3.i41       ; <<4 x float>> [#uses=1]
  %add235.i = fadd <4 x float> %tmp233.i, %tmp234.i136 ; <<4 x float>> [#uses=1]
  store <4 x float> %add235.i, <4 x float>* %zr1.i23
  %tmp236.i137 = load <4 x float>* %bi1.i39       ; <<4 x float>> [#uses=1]
  %tmp237.i138 = load <4 x float>* %br3.i35       ; <<4 x float>> [#uses=1]
  %sub238.i = fsub <4 x float> %tmp236.i137, %tmp237.i138 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub238.i, <4 x float>* %zi1.i27
  %tmp239.i = load <4 x float>* %br1.i33          ; <<4 x float>> [#uses=1]
  %tmp240.i139 = load <4 x float>* %bi3.i41       ; <<4 x float>> [#uses=1]
  %sub241.i = fsub <4 x float> %tmp239.i, %tmp240.i139 ; <<4 x float>> [#uses=1]
  store <4 x float> %sub241.i, <4 x float>* %zr3.i25
  %tmp242.i140 = load <4 x float>* %br3.i35       ; <<4 x float>> [#uses=1]
  %tmp243.i = load <4 x float>* %bi1.i39          ; <<4 x float>> [#uses=1]
  %add244.i = fadd <4 x float> %tmp242.i140, %tmp243.i ; <<4 x float>> [#uses=1]
  store <4 x float> %add244.i, <4 x float>* %zi3.i29
  %tmp245.i141 = load <4 x float>* %br2.i34       ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245.i141, <4 x float>* %zr2.i24
  %tmp246.i142 = load <4 x float>* %bi2.i40       ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp246.i142, <4 x float>* %zi2.i28
  %tmp248.i143 = load float addrspace(1)** %gr.addr.i19 ; <float addrspace(1)*> [#uses=1]
  %tmp249.i144 = load i32* %me.addr.i17           ; <i32> [#uses=1]
  %shl.i145 = shl i32 %tmp249.i144, 2             ; <i32> [#uses=1]
  %add.ptr250.i = getelementptr inbounds float addrspace(1)* %tmp248.i143, i32 %shl.i145 ; <float addrspace(1)*> [#uses=1]
  %14 = bitcast float addrspace(1)* %add.ptr250.i to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %14, <4 x float> addrspace(1)** %gp.i42
  %tmp251.i = load <4 x float>* %zr0.i22          ; <<4 x float>> [#uses=1]
  %tmp252.i146 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx253.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp252.i146, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp251.i, <4 x float> addrspace(1)* %arrayidx253.i
  %tmp254.i147 = load <4 x float>* %zr1.i23       ; <<4 x float>> [#uses=1]
  %tmp255.i148 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx256.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp255.i148, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp254.i147, <4 x float> addrspace(1)* %arrayidx256.i
  %tmp257.i = load <4 x float>* %zr2.i24          ; <<4 x float>> [#uses=1]
  %tmp258.i149 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx259.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp258.i149, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp257.i, <4 x float> addrspace(1)* %arrayidx259.i
  %tmp260.i150 = load <4 x float>* %zr3.i25       ; <<4 x float>> [#uses=1]
  %tmp261.i = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx262.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp261.i, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp260.i150, <4 x float> addrspace(1)* %arrayidx262.i
  %tmp263.i151 = load float addrspace(1)** %gi.addr.i20 ; <float addrspace(1)*> [#uses=1]
  %tmp264.i152 = load i32* %me.addr.i17           ; <i32> [#uses=1]
  %shl265.i = shl i32 %tmp264.i152, 2             ; <i32> [#uses=1]
  %add.ptr266.i = getelementptr inbounds float addrspace(1)* %tmp263.i151, i32 %shl265.i ; <float addrspace(1)*> [#uses=1]
  %15 = bitcast float addrspace(1)* %add.ptr266.i to <4 x float> addrspace(1)* ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> addrspace(1)* %15, <4 x float> addrspace(1)** %gp.i42
  %tmp267.i153 = load <4 x float>* %zi0.i26       ; <<4 x float>> [#uses=1]
  %tmp268.i154 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx269.i155 = getelementptr inbounds <4 x float> addrspace(1)* %tmp268.i154, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp267.i153, <4 x float> addrspace(1)* %arrayidx269.i155
  %tmp270.i156 = load <4 x float>* %zi1.i27       ; <<4 x float>> [#uses=1]
  %tmp271.i = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx272.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp271.i, i32 64 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp270.i156, <4 x float> addrspace(1)* %arrayidx272.i
  %tmp273.i157 = load <4 x float>* %zi2.i28       ; <<4 x float>> [#uses=1]
  %tmp274.i158 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx275.i159 = getelementptr inbounds <4 x float> addrspace(1)* %tmp274.i158, i32 128 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp273.i157, <4 x float> addrspace(1)* %arrayidx275.i159
  %tmp276.i160 = load <4 x float>* %zi3.i29       ; <<4 x float>> [#uses=1]
  %tmp277.i161 = load <4 x float> addrspace(1)** %gp.i42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx278.i = getelementptr inbounds <4 x float> addrspace(1)* %tmp277.i161, i32 192 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp276.i160, <4 x float> addrspace(1)* %arrayidx278.i
  ret void
}

declare i32 @get_global_id(i32)
