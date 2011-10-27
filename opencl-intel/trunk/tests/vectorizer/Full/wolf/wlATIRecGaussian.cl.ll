; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIRecGaussian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_transpose_kernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_transpose_kernel_parameters = appending global [167 x i8] c"uchar4 __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(3))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[167 x i8]*> [#uses=1]
@opencl_RecursiveGaussian_kernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_RecursiveGaussian_kernel_parameters = appending global [219 x i8] c"uchar4 const __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *, int const, int const, float const, float const, float const, float const, float const, float const, float const, float const\00", section "llvm.metadata" ; <[219 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(3)*, i32, i32, i32)* @transpose_kernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_transpose_kernel_locals to i8*), i8* getelementptr inbounds ([167 x i8]* @opencl_transpose_kernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i32, i32, float, float, float, float, float, float, float, float)* @RecursiveGaussian_kernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_RecursiveGaussian_kernel_locals to i8*), i8* getelementptr inbounds ([219 x i8]* @opencl_RecursiveGaussian_kernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @transpose_kernel(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(3)* %block, i32 %width, i32 %height, i32 %blockSize) nounwind {
entry:
  %output.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=2]
  %input.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=2]
  %block.addr = alloca <4 x i8> addrspace(3)*, align 4 ; <<4 x i8> addrspace(3)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=2]
  %blockSize.addr = alloca i32, align 4           ; <i32*> [#uses=3]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=3]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=3]
  %localIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %localIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=2]
  %targetIndex = alloca i32, align 4              ; <i32*> [#uses=2]
  store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
  store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
  store <4 x i8> addrspace(3)* %block, <4 x i8> addrspace(3)** %block.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %blockSize, i32* %blockSize.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalIdx
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdy
  %call2 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %localIdx
  %call3 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call3, i32* %localIdy
  %tmp = load i32* %globalIdy                     ; <i32> [#uses=1]
  %tmp4 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp4                      ; <i32> [#uses=1]
  %tmp5 = load i32* %globalIdx                    ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp5                      ; <i32> [#uses=1]
  %tmp6 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %tmp6, i32 %add ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp7 = load <4 x i8> addrspace(1)* %arrayidx   ; <<4 x i8>> [#uses=1]
  %tmp8 = load i32* %localIdy                     ; <i32> [#uses=1]
  %tmp9 = load i32* %blockSize.addr               ; <i32> [#uses=1]
  %mul10 = mul i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  %tmp11 = load i32* %localIdx                    ; <i32> [#uses=1]
  %add12 = add i32 %mul10, %tmp11                 ; <i32> [#uses=1]
  %tmp13 = load <4 x i8> addrspace(3)** %block.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx14 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp13, i32 %add12 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp7, <4 x i8> addrspace(3)* %arrayidx14
  call void @barrier(i32 1)
  %tmp16 = load i32* %localIdy                    ; <i32> [#uses=1]
  %tmp17 = load i32* %blockSize.addr              ; <i32> [#uses=1]
  %mul18 = mul i32 %tmp16, %tmp17                 ; <i32> [#uses=1]
  %tmp19 = load i32* %localIdx                    ; <i32> [#uses=1]
  %add20 = add i32 %mul18, %tmp19                 ; <i32> [#uses=1]
  store i32 %add20, i32* %sourceIndex
  %tmp22 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %tmp23 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %tmp24 = load i32* %height.addr                 ; <i32> [#uses=1]
  %mul25 = mul i32 %tmp23, %tmp24                 ; <i32> [#uses=1]
  %add26 = add i32 %tmp22, %mul25                 ; <i32> [#uses=1]
  store i32 %add26, i32* %targetIndex
  %tmp27 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp28 = load <4 x i8> addrspace(3)** %block.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp28, i32 %tmp27 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp30 = load <4 x i8> addrspace(3)* %arrayidx29 ; <<4 x i8>> [#uses=1]
  %tmp31 = load i32* %targetIndex                 ; <i32> [#uses=1]
  %tmp32 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx33 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp32, i32 %tmp31 ; <<4 x i8> addrspace(1)*> [#uses=1]
  store <4 x i8> %tmp30, <4 x i8> addrspace(1)* %arrayidx33
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

; CHECK: ret
define void @RecursiveGaussian_kernel(<4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %output, i32 %width, i32 %height, float %a0, float %a1, float %a2, float %a3, float %b1, float %b2, float %coefp, float %coefn) nounwind {
entry:
  %input.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=9]
  %output.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=7]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=3]
  %a0.addr = alloca float, align 4                ; <float*> [#uses=2]
  %a1.addr = alloca float, align 4                ; <float*> [#uses=2]
  %a2.addr = alloca float, align 4                ; <float*> [#uses=2]
  %a3.addr = alloca float, align 4                ; <float*> [#uses=2]
  %b1.addr = alloca float, align 4                ; <float*> [#uses=3]
  %b2.addr = alloca float, align 4                ; <float*> [#uses=3]
  %coefp.addr = alloca float, align 4             ; <float*> [#uses=1]
  %coefn.addr = alloca float, align 4             ; <float*> [#uses=1]
  %x = alloca i32, align 4                        ; <i32*> [#uses=4]
  %xp = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %yp = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %yb = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %y = alloca i32, align 4                        ; <i32*> [#uses=5]
  %pos = alloca i32, align 4                      ; <i32*> [#uses=6]
  %xc = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %yc = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=6]
  %.compoundliteral62 = alloca <4 x i8>, align 4  ; <<4 x i8>*> [#uses=2]
  %xn = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %.compoundliteral88 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %xa = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral91 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %yn = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %.compoundliteral94 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %ya = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %.compoundliteral97 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %y100 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %pos109 = alloca i32, align 4                   ; <i32*> [#uses=10]
  %xc116 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %.compoundliteral117 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %yc148 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=5]
  %.compoundliteral177 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral209 = alloca <4 x i8>, align 4 ; <<4 x i8>*> [#uses=2]
  store <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)** %input.addr
  store <4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)** %output.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store float %a0, float* %a0.addr
  store float %a1, float* %a1.addr
  store float %a2, float* %a2.addr
  store float %a3, float* %a3.addr
  store float %b1, float* %b1.addr
  store float %b2, float* %b2.addr
  store float %coefp, float* %coefp.addr
  store float %coefn, float* %coefn.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %x
  %tmp = load i32* %x                             ; <i32> [#uses=1]
  %tmp1 = load i32* %width.addr                   ; <i32> [#uses=1]
  %cmp = icmp uge i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end232

if.end:                                           ; preds = %entry
  store <4 x float> zeroinitializer, <4 x float>* %xp
  store <4 x float> zeroinitializer, <4 x float>* %yp
  store <4 x float> zeroinitializer, <4 x float>* %yb
  store i32 0, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %tmp6 = load i32* %y                            ; <i32> [#uses=1]
  %tmp7 = load i32* %height.addr                  ; <i32> [#uses=1]
  %cmp8 = icmp slt i32 %tmp6, %tmp7               ; <i1> [#uses=1]
  br i1 %cmp8, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp10 = load i32* %x                           ; <i32> [#uses=1]
  %tmp11 = load i32* %y                           ; <i32> [#uses=1]
  %tmp12 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp11, %tmp12                   ; <i32> [#uses=1]
  %add = add i32 %tmp10, %mul                     ; <i32> [#uses=1]
  store i32 %add, i32* %pos
  %tmp14 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp15 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %tmp15, i32 %tmp14 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp16 = load <4 x i8> addrspace(1)* %arrayidx  ; <<4 x i8>> [#uses=1]
  %tmp17 = extractelement <4 x i8> %tmp16, i32 0  ; <i8> [#uses=1]
  %conv = uitofp i8 %tmp17 to float               ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %conv, i32 0 ; <<4 x float>> [#uses=1]
  %tmp18 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp19 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp19, i32 %tmp18 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp21 = load <4 x i8> addrspace(1)* %arrayidx20 ; <<4 x i8>> [#uses=1]
  %tmp22 = extractelement <4 x i8> %tmp21, i32 1  ; <i8> [#uses=1]
  %conv23 = uitofp i8 %tmp22 to float             ; <float> [#uses=1]
  %vecinit24 = insertelement <4 x float> %vecinit, float %conv23, i32 1 ; <<4 x float>> [#uses=1]
  %tmp25 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp26 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx27 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp26, i32 %tmp25 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp28 = load <4 x i8> addrspace(1)* %arrayidx27 ; <<4 x i8>> [#uses=1]
  %tmp29 = extractelement <4 x i8> %tmp28, i32 2  ; <i8> [#uses=1]
  %conv30 = uitofp i8 %tmp29 to float             ; <float> [#uses=1]
  %vecinit31 = insertelement <4 x float> %vecinit24, float %conv30, i32 2 ; <<4 x float>> [#uses=1]
  %tmp32 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp33 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp33, i32 %tmp32 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp35 = load <4 x i8> addrspace(1)* %arrayidx34 ; <<4 x i8>> [#uses=1]
  %tmp36 = extractelement <4 x i8> %tmp35, i32 3  ; <i8> [#uses=1]
  %conv37 = uitofp i8 %tmp36 to float             ; <float> [#uses=1]
  %vecinit38 = insertelement <4 x float> %vecinit31, float %conv37, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit38, <4 x float>* %.compoundliteral
  %tmp39 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp39, <4 x float>* %xc
  %tmp41 = load float* %a0.addr                   ; <float> [#uses=1]
  %tmp42 = insertelement <4 x float> undef, float %tmp41, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp42, <4 x float> %tmp42, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp43 = load <4 x float>* %xc                  ; <<4 x float>> [#uses=1]
  %mul44 = fmul <4 x float> %splat, %tmp43        ; <<4 x float>> [#uses=1]
  %tmp45 = load float* %a1.addr                   ; <float> [#uses=1]
  %tmp46 = insertelement <4 x float> undef, float %tmp45, i32 0 ; <<4 x float>> [#uses=2]
  %splat47 = shufflevector <4 x float> %tmp46, <4 x float> %tmp46, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp48 = load <4 x float>* %xp                  ; <<4 x float>> [#uses=1]
  %mul49 = fmul <4 x float> %splat47, %tmp48      ; <<4 x float>> [#uses=1]
  %add50 = fadd <4 x float> %mul44, %mul49        ; <<4 x float>> [#uses=1]
  %tmp51 = load float* %b1.addr                   ; <float> [#uses=1]
  %tmp52 = insertelement <4 x float> undef, float %tmp51, i32 0 ; <<4 x float>> [#uses=2]
  %splat53 = shufflevector <4 x float> %tmp52, <4 x float> %tmp52, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp54 = load <4 x float>* %yp                  ; <<4 x float>> [#uses=1]
  %mul55 = fmul <4 x float> %splat53, %tmp54      ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %add50, %mul55          ; <<4 x float>> [#uses=1]
  %tmp56 = load float* %b2.addr                   ; <float> [#uses=1]
  %tmp57 = insertelement <4 x float> undef, float %tmp56, i32 0 ; <<4 x float>> [#uses=2]
  %splat58 = shufflevector <4 x float> %tmp57, <4 x float> %tmp57, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp59 = load <4 x float>* %yb                  ; <<4 x float>> [#uses=1]
  %mul60 = fmul <4 x float> %splat58, %tmp59      ; <<4 x float>> [#uses=1]
  %sub61 = fsub <4 x float> %sub, %mul60          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub61, <4 x float>* %yc
  %tmp63 = load <4 x float>* %yc                  ; <<4 x float>> [#uses=1]
  %tmp64 = extractelement <4 x float> %tmp63, i32 0 ; <float> [#uses=1]
  %conv65 = fptoui float %tmp64 to i8             ; <i8> [#uses=1]
  %vecinit66 = insertelement <4 x i8> undef, i8 %conv65, i32 0 ; <<4 x i8>> [#uses=1]
  %tmp67 = load <4 x float>* %yc                  ; <<4 x float>> [#uses=1]
  %tmp68 = extractelement <4 x float> %tmp67, i32 1 ; <float> [#uses=1]
  %conv69 = fptoui float %tmp68 to i8             ; <i8> [#uses=1]
  %vecinit70 = insertelement <4 x i8> %vecinit66, i8 %conv69, i32 1 ; <<4 x i8>> [#uses=1]
  %tmp71 = load <4 x float>* %yc                  ; <<4 x float>> [#uses=1]
  %tmp72 = extractelement <4 x float> %tmp71, i32 2 ; <float> [#uses=1]
  %conv73 = fptoui float %tmp72 to i8             ; <i8> [#uses=1]
  %vecinit74 = insertelement <4 x i8> %vecinit70, i8 %conv73, i32 2 ; <<4 x i8>> [#uses=1]
  %tmp75 = load <4 x float>* %yc                  ; <<4 x float>> [#uses=1]
  %tmp76 = extractelement <4 x float> %tmp75, i32 3 ; <float> [#uses=1]
  %conv77 = fptoui float %tmp76 to i8             ; <i8> [#uses=1]
  %vecinit78 = insertelement <4 x i8> %vecinit74, i8 %conv77, i32 3 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %vecinit78, <4 x i8>* %.compoundliteral62
  %tmp79 = load <4 x i8>* %.compoundliteral62     ; <<4 x i8>> [#uses=1]
  %tmp80 = load i32* %pos                         ; <i32> [#uses=1]
  %tmp81 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx82 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp81, i32 %tmp80 ; <<4 x i8> addrspace(1)*> [#uses=1]
  store <4 x i8> %tmp79, <4 x i8> addrspace(1)* %arrayidx82
  %tmp83 = load <4 x float>* %xc                  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp83, <4 x float>* %xp
  %tmp84 = load <4 x float>* %yp                  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp84, <4 x float>* %yb
  %tmp85 = load <4 x float>* %yc                  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp85, <4 x float>* %yp
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp86 = load i32* %y                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp86, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %y
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @barrier(i32 2)
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral88
  %tmp89 = load <4 x float>* %.compoundliteral88  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp89, <4 x float>* %xn
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral91
  %tmp92 = load <4 x float>* %.compoundliteral91  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp92, <4 x float>* %xa
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral94
  %tmp95 = load <4 x float>* %.compoundliteral94  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp95, <4 x float>* %yn
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral97
  %tmp98 = load <4 x float>* %.compoundliteral97  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp98, <4 x float>* %ya
  %tmp101 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub102 = sub i32 %tmp101, 1                    ; <i32> [#uses=1]
  store i32 %sub102, i32* %y100
  br label %for.cond103

for.cond103:                                      ; preds = %for.inc230, %for.end
  %tmp104 = load i32* %y100                       ; <i32> [#uses=1]
  %cmp105 = icmp sgt i32 %tmp104, -1              ; <i1> [#uses=1]
  br i1 %cmp105, label %for.body107, label %for.end232

for.body107:                                      ; preds = %for.cond103
  %tmp110 = load i32* %x                          ; <i32> [#uses=1]
  %tmp111 = load i32* %y100                       ; <i32> [#uses=1]
  %tmp112 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul113 = mul i32 %tmp111, %tmp112              ; <i32> [#uses=1]
  %add114 = add i32 %tmp110, %mul113              ; <i32> [#uses=1]
  store i32 %add114, i32* %pos109
  %tmp118 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp119 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx120 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp119, i32 %tmp118 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp121 = load <4 x i8> addrspace(1)* %arrayidx120 ; <<4 x i8>> [#uses=1]
  %tmp122 = extractelement <4 x i8> %tmp121, i32 0 ; <i8> [#uses=1]
  %conv123 = uitofp i8 %tmp122 to float           ; <float> [#uses=1]
  %vecinit124 = insertelement <4 x float> undef, float %conv123, i32 0 ; <<4 x float>> [#uses=1]
  %tmp125 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp126 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx127 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp126, i32 %tmp125 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp128 = load <4 x i8> addrspace(1)* %arrayidx127 ; <<4 x i8>> [#uses=1]
  %tmp129 = extractelement <4 x i8> %tmp128, i32 1 ; <i8> [#uses=1]
  %conv130 = uitofp i8 %tmp129 to float           ; <float> [#uses=1]
  %vecinit131 = insertelement <4 x float> %vecinit124, float %conv130, i32 1 ; <<4 x float>> [#uses=1]
  %tmp132 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp133 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp133, i32 %tmp132 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp135 = load <4 x i8> addrspace(1)* %arrayidx134 ; <<4 x i8>> [#uses=1]
  %tmp136 = extractelement <4 x i8> %tmp135, i32 2 ; <i8> [#uses=1]
  %conv137 = uitofp i8 %tmp136 to float           ; <float> [#uses=1]
  %vecinit138 = insertelement <4 x float> %vecinit131, float %conv137, i32 2 ; <<4 x float>> [#uses=1]
  %tmp139 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp140 = load <4 x i8> addrspace(1)** %input.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx141 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp140, i32 %tmp139 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp142 = load <4 x i8> addrspace(1)* %arrayidx141 ; <<4 x i8>> [#uses=1]
  %tmp143 = extractelement <4 x i8> %tmp142, i32 3 ; <i8> [#uses=1]
  %conv144 = uitofp i8 %tmp143 to float           ; <float> [#uses=1]
  %vecinit145 = insertelement <4 x float> %vecinit138, float %conv144, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit145, <4 x float>* %.compoundliteral117
  %tmp146 = load <4 x float>* %.compoundliteral117 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp146, <4 x float>* %xc116
  %tmp149 = load float* %a2.addr                  ; <float> [#uses=1]
  %tmp150 = insertelement <4 x float> undef, float %tmp149, i32 0 ; <<4 x float>> [#uses=2]
  %splat151 = shufflevector <4 x float> %tmp150, <4 x float> %tmp150, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp152 = load <4 x float>* %xn                 ; <<4 x float>> [#uses=1]
  %mul153 = fmul <4 x float> %splat151, %tmp152   ; <<4 x float>> [#uses=1]
  %tmp154 = load float* %a3.addr                  ; <float> [#uses=1]
  %tmp155 = insertelement <4 x float> undef, float %tmp154, i32 0 ; <<4 x float>> [#uses=2]
  %splat156 = shufflevector <4 x float> %tmp155, <4 x float> %tmp155, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp157 = load <4 x float>* %xa                 ; <<4 x float>> [#uses=1]
  %mul158 = fmul <4 x float> %splat156, %tmp157   ; <<4 x float>> [#uses=1]
  %add159 = fadd <4 x float> %mul153, %mul158     ; <<4 x float>> [#uses=1]
  %tmp160 = load float* %b1.addr                  ; <float> [#uses=1]
  %tmp161 = insertelement <4 x float> undef, float %tmp160, i32 0 ; <<4 x float>> [#uses=2]
  %splat162 = shufflevector <4 x float> %tmp161, <4 x float> %tmp161, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp163 = load <4 x float>* %yn                 ; <<4 x float>> [#uses=1]
  %mul164 = fmul <4 x float> %splat162, %tmp163   ; <<4 x float>> [#uses=1]
  %sub165 = fsub <4 x float> %add159, %mul164     ; <<4 x float>> [#uses=1]
  %tmp166 = load float* %b2.addr                  ; <float> [#uses=1]
  %tmp167 = insertelement <4 x float> undef, float %tmp166, i32 0 ; <<4 x float>> [#uses=2]
  %splat168 = shufflevector <4 x float> %tmp167, <4 x float> %tmp167, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp169 = load <4 x float>* %ya                 ; <<4 x float>> [#uses=1]
  %mul170 = fmul <4 x float> %splat168, %tmp169   ; <<4 x float>> [#uses=1]
  %sub171 = fsub <4 x float> %sub165, %mul170     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub171, <4 x float>* %yc148
  %tmp172 = load <4 x float>* %xn                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp172, <4 x float>* %xa
  %tmp173 = load <4 x float>* %xc116              ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp173, <4 x float>* %xn
  %tmp174 = load <4 x float>* %yn                 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp174, <4 x float>* %ya
  %tmp175 = load <4 x float>* %yc148              ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp175, <4 x float>* %yn
  %tmp178 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp179 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx180 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp179, i32 %tmp178 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp181 = load <4 x i8> addrspace(1)* %arrayidx180 ; <<4 x i8>> [#uses=1]
  %tmp182 = extractelement <4 x i8> %tmp181, i32 0 ; <i8> [#uses=1]
  %conv183 = uitofp i8 %tmp182 to float           ; <float> [#uses=1]
  %vecinit184 = insertelement <4 x float> undef, float %conv183, i32 0 ; <<4 x float>> [#uses=1]
  %tmp185 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp186 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx187 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp186, i32 %tmp185 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp188 = load <4 x i8> addrspace(1)* %arrayidx187 ; <<4 x i8>> [#uses=1]
  %tmp189 = extractelement <4 x i8> %tmp188, i32 1 ; <i8> [#uses=1]
  %conv190 = uitofp i8 %tmp189 to float           ; <float> [#uses=1]
  %vecinit191 = insertelement <4 x float> %vecinit184, float %conv190, i32 1 ; <<4 x float>> [#uses=1]
  %tmp192 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp193 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx194 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp193, i32 %tmp192 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp195 = load <4 x i8> addrspace(1)* %arrayidx194 ; <<4 x i8>> [#uses=1]
  %tmp196 = extractelement <4 x i8> %tmp195, i32 2 ; <i8> [#uses=1]
  %conv197 = uitofp i8 %tmp196 to float           ; <float> [#uses=1]
  %vecinit198 = insertelement <4 x float> %vecinit191, float %conv197, i32 2 ; <<4 x float>> [#uses=1]
  %tmp199 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp200 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx201 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp200, i32 %tmp199 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp202 = load <4 x i8> addrspace(1)* %arrayidx201 ; <<4 x i8>> [#uses=1]
  %tmp203 = extractelement <4 x i8> %tmp202, i32 3 ; <i8> [#uses=1]
  %conv204 = uitofp i8 %tmp203 to float           ; <float> [#uses=1]
  %vecinit205 = insertelement <4 x float> %vecinit198, float %conv204, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit205, <4 x float>* %.compoundliteral177
  %tmp206 = load <4 x float>* %.compoundliteral177 ; <<4 x float>> [#uses=1]
  %tmp207 = load <4 x float>* %yc148              ; <<4 x float>> [#uses=1]
  %add208 = fadd <4 x float> %tmp206, %tmp207     ; <<4 x float>> [#uses=1]
  store <4 x float> %add208, <4 x float>* %temp
  %tmp210 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %tmp211 = extractelement <4 x float> %tmp210, i32 0 ; <float> [#uses=1]
  %conv212 = fptoui float %tmp211 to i8           ; <i8> [#uses=1]
  %vecinit213 = insertelement <4 x i8> undef, i8 %conv212, i32 0 ; <<4 x i8>> [#uses=1]
  %tmp214 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %tmp215 = extractelement <4 x float> %tmp214, i32 1 ; <float> [#uses=1]
  %conv216 = fptoui float %tmp215 to i8           ; <i8> [#uses=1]
  %vecinit217 = insertelement <4 x i8> %vecinit213, i8 %conv216, i32 1 ; <<4 x i8>> [#uses=1]
  %tmp218 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %tmp219 = extractelement <4 x float> %tmp218, i32 2 ; <float> [#uses=1]
  %conv220 = fptoui float %tmp219 to i8           ; <i8> [#uses=1]
  %vecinit221 = insertelement <4 x i8> %vecinit217, i8 %conv220, i32 2 ; <<4 x i8>> [#uses=1]
  %tmp222 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %tmp223 = extractelement <4 x float> %tmp222, i32 3 ; <float> [#uses=1]
  %conv224 = fptoui float %tmp223 to i8           ; <i8> [#uses=1]
  %vecinit225 = insertelement <4 x i8> %vecinit221, i8 %conv224, i32 3 ; <<4 x i8>> [#uses=1]
  store <4 x i8> %vecinit225, <4 x i8>* %.compoundliteral209
  %tmp226 = load <4 x i8>* %.compoundliteral209   ; <<4 x i8>> [#uses=1]
  %tmp227 = load i32* %pos109                     ; <i32> [#uses=1]
  %tmp228 = load <4 x i8> addrspace(1)** %output.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx229 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp228, i32 %tmp227 ; <<4 x i8> addrspace(1)*> [#uses=1]
  store <4 x i8> %tmp226, <4 x i8> addrspace(1)* %arrayidx229
  br label %for.inc230

for.inc230:                                       ; preds = %for.body107
  %tmp231 = load i32* %y100                       ; <i32> [#uses=1]
  %dec = add nsw i32 %tmp231, -1                  ; <i32> [#uses=1]
  store i32 %dec, i32* %y100
  br label %for.cond103

for.end232:                                       ; preds = %if.then, %for.cond103
  ret void
}
