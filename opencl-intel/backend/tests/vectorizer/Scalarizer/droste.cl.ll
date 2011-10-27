; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\droste.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque
%struct.anon = type <{ <4 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x float>, <2 x i32>, float, float, float, float, i32, float, float, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, i32, i32, i32, i32, i32 }>

@opencl_getArgsSize_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_getArgsSize_parameters = appending global [41 x i8] c"uint __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[41 x i8]*> [#uses=1]
@opencl_droste2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_droste2D_parameters = appending global [107 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[107 x i8]*> [#uses=1]
@opencl_droste_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_droste_parameters = appending global [119 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[119 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*)* @getArgsSize to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_getArgsSize_locals to i8*), i8* getelementptr inbounds ([41 x i8]* @opencl_getArgsSize_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, %struct.anon addrspace(2)*)* @droste2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_droste2D_locals to i8*), i8* getelementptr inbounds ([107 x i8]* @opencl_droste2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32, %struct.anon addrspace(2)*)* @droste to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_droste_locals to i8*), i8* getelementptr inbounds ([119 x i8]* @opencl_droste_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <2 x float> @complexMult(<2 x float> %a, <2 x float> %b) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %a.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %b.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %a, <2 x float>* %a.addr
  store <2 x float> %b, <2 x float>* %b.addr
  %tmp = load <2 x float>* %a.addr                ; <<2 x float>> [#uses=1]
  %tmp1 = extractelement <2 x float> %tmp, i32 0  ; <float> [#uses=1]
  %tmp2 = load <2 x float>* %b.addr               ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 0 ; <float> [#uses=1]
  %mul = fmul float %tmp1, %tmp3                  ; <float> [#uses=1]
  %tmp4 = load <2 x float>* %a.addr               ; <<2 x float>> [#uses=1]
  %tmp5 = extractelement <2 x float> %tmp4, i32 1 ; <float> [#uses=1]
  %tmp6 = load <2 x float>* %b.addr               ; <<2 x float>> [#uses=1]
  %tmp7 = extractelement <2 x float> %tmp6, i32 1 ; <float> [#uses=1]
  %mul8 = fmul float %tmp5, %tmp7                 ; <float> [#uses=1]
  %sub = fsub float %mul, %mul8                   ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %sub, i32 0 ; <<2 x float>> [#uses=1]
  %tmp9 = load <2 x float>* %a.addr               ; <<2 x float>> [#uses=1]
  %tmp10 = extractelement <2 x float> %tmp9, i32 0 ; <float> [#uses=1]
  %tmp11 = load <2 x float>* %b.addr              ; <<2 x float>> [#uses=1]
  %tmp12 = extractelement <2 x float> %tmp11, i32 1 ; <float> [#uses=1]
  %mul13 = fmul float %tmp10, %tmp12              ; <float> [#uses=1]
  %tmp14 = load <2 x float>* %a.addr              ; <<2 x float>> [#uses=1]
  %tmp15 = extractelement <2 x float> %tmp14, i32 1 ; <float> [#uses=1]
  %tmp16 = load <2 x float>* %b.addr              ; <<2 x float>> [#uses=1]
  %tmp17 = extractelement <2 x float> %tmp16, i32 0 ; <float> [#uses=1]
  %mul18 = fmul float %tmp15, %tmp17              ; <float> [#uses=1]
  %add = fadd float %mul13, %mul18                ; <float> [#uses=1]
  %vecinit19 = insertelement <2 x float> %vecinit, float %add, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit19, <2 x float>* %rval
  %tmp20 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp20, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define float @complexMag(<2 x float> %z) nounwind {
entry:
  %retval = alloca float, align 4                 ; <float*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %call = call float @_Z6lengthDv2_f(<2 x float> %tmp) ; <float> [#uses=1]
  %call1 = call float @_Z3powff(float %call, float 2.000000e+000) ; <float> [#uses=1]
  store float %call1, float* %retval
  %0 = load float* %retval                        ; <float> [#uses=1]
  ret float %0
}

declare float @_Z3powff(float, float)

declare float @_Z6lengthDv2_f(<2 x float>)

; CHECK: ret
define <2 x float> @complexReciprocal(<2 x float> %z) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %tmp1 = extractelement <2 x float> %tmp, i32 0  ; <float> [#uses=1]
  %tmp2 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %call = call float @complexMag(<2 x float> %tmp2) ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %call      ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %call ; <float> [#uses=0]
  %div = fdiv float %tmp1, %call                  ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %div, i32 0 ; <<2 x float>> [#uses=1]
  %tmp3 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp4 = extractelement <2 x float> %tmp3, i32 1 ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp4         ; <float> [#uses=1]
  %tmp5 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %call6 = call float @complexMag(<2 x float> %tmp5) ; <float> [#uses=3]
  %cmp7 = fcmp oeq float 0.000000e+000, %call6    ; <i1> [#uses=1]
  %sel8 = select i1 %cmp7, float 1.000000e+000, float %call6 ; <float> [#uses=0]
  %div9 = fdiv float %neg, %call6                 ; <float> [#uses=1]
  %vecinit10 = insertelement <2 x float> %vecinit, float %div9, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit10, <2 x float>* %rval
  %tmp11 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp11, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define <2 x float> @complexLog(<2 x float> %z) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=4]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %call = call float @_Z6lengthDv2_f(<2 x float> %tmp) ; <float> [#uses=1]
  %call1 = call float @_Z3logf(float %call)       ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %call1, i32 0 ; <<2 x float>> [#uses=1]
  %tmp2 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 1 ; <float> [#uses=1]
  %tmp4 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp5 = extractelement <2 x float> %tmp4, i32 0 ; <float> [#uses=1]
  %call6 = call float @_Z5atan2ff(float %tmp3, float %tmp5) ; <float> [#uses=1]
  %vecinit7 = insertelement <2 x float> %vecinit, float %call6, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit7, <2 x float>* %rval
  %tmp8 = load <2 x float>* %rval                 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp8, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

declare float @_Z3logf(float)

declare float @_Z5atan2ff(float, float)

; CHECK: ret
define <2 x float> @complexExp(<2 x float> %z) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %tmp1 = extractelement <2 x float> %tmp, i32 0  ; <float> [#uses=1]
  %call = call float @_Z3expf(float %tmp1)        ; <float> [#uses=1]
  %tmp2 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 1 ; <float> [#uses=1]
  %call4 = call float @_Z3cosf(float %tmp3)       ; <float> [#uses=1]
  %mul = fmul float %call, %call4                 ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %mul, i32 0 ; <<2 x float>> [#uses=1]
  %tmp5 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp6 = extractelement <2 x float> %tmp5, i32 0 ; <float> [#uses=1]
  %call7 = call float @_Z3expf(float %tmp6)       ; <float> [#uses=1]
  %tmp8 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp9 = extractelement <2 x float> %tmp8, i32 1 ; <float> [#uses=1]
  %call10 = call float @_Z3sinf(float %tmp9)      ; <float> [#uses=1]
  %mul11 = fmul float %call7, %call10             ; <float> [#uses=1]
  %vecinit12 = insertelement <2 x float> %vecinit, float %mul11, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit12, <2 x float>* %rval
  %tmp13 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp13, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

declare float @_Z3expf(float)

declare float @_Z3cosf(float)

declare float @_Z3sinf(float)

; CHECK: ret
define <2 x float> @complexSin(<2 x float> %z) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %tmp1 = extractelement <2 x float> %tmp, i32 0  ; <float> [#uses=1]
  %call = call float @_Z3sinf(float %tmp1)        ; <float> [#uses=1]
  %tmp2 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 1 ; <float> [#uses=1]
  %call4 = call float @_Z4coshf(float %tmp3)      ; <float> [#uses=1]
  %mul = fmul float %call, %call4                 ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %mul, i32 0 ; <<2 x float>> [#uses=1]
  %tmp5 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp6 = extractelement <2 x float> %tmp5, i32 0 ; <float> [#uses=1]
  %call7 = call float @_Z3cosf(float %tmp6)       ; <float> [#uses=1]
  %tmp8 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp9 = extractelement <2 x float> %tmp8, i32 1 ; <float> [#uses=1]
  %call10 = call float @_Z4sinhf(float %tmp9)     ; <float> [#uses=1]
  %mul11 = fmul float %call7, %call10             ; <float> [#uses=1]
  %vecinit12 = insertelement <2 x float> %vecinit, float %mul11, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit12, <2 x float>* %rval
  %tmp13 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp13, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

declare float @_Z4coshf(float)

declare float @_Z4sinhf(float)

; CHECK: ret
define <2 x float> @complexTan(<2 x float> %z) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=7]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %tmp1 = extractelement <2 x float> %tmp, i32 0  ; <float> [#uses=1]
  %mul = fmul float 2.000000e+000, %tmp1          ; <float> [#uses=1]
  %call = call float @_Z3sinf(float %mul)         ; <float> [#uses=1]
  %tmp2 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp3 = extractelement <2 x float> %tmp2, i32 0 ; <float> [#uses=1]
  %mul4 = fmul float 2.000000e+000, %tmp3         ; <float> [#uses=1]
  %call5 = call float @_Z3cosf(float %mul4)       ; <float> [#uses=1]
  %tmp6 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp7 = extractelement <2 x float> %tmp6, i32 1 ; <float> [#uses=1]
  %mul8 = fmul float 2.000000e+000, %tmp7         ; <float> [#uses=1]
  %call9 = call float @_Z4coshf(float %mul8)      ; <float> [#uses=1]
  %add = fadd float %call5, %call9                ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %add       ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %add ; <float> [#uses=0]
  %div = fdiv float %call, %add                   ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %div, i32 0 ; <<2 x float>> [#uses=1]
  %tmp10 = load <2 x float>* %z.addr              ; <<2 x float>> [#uses=1]
  %tmp11 = extractelement <2 x float> %tmp10, i32 1 ; <float> [#uses=1]
  %mul12 = fmul float 2.000000e+000, %tmp11       ; <float> [#uses=1]
  %call13 = call float @_Z4sinhf(float %mul12)    ; <float> [#uses=1]
  %tmp14 = load <2 x float>* %z.addr              ; <<2 x float>> [#uses=1]
  %tmp15 = extractelement <2 x float> %tmp14, i32 0 ; <float> [#uses=1]
  %mul16 = fmul float 2.000000e+000, %tmp15       ; <float> [#uses=1]
  %call17 = call float @_Z3cosf(float %mul16)     ; <float> [#uses=1]
  %tmp18 = load <2 x float>* %z.addr              ; <<2 x float>> [#uses=1]
  %tmp19 = extractelement <2 x float> %tmp18, i32 1 ; <float> [#uses=1]
  %mul20 = fmul float 2.000000e+000, %tmp19       ; <float> [#uses=1]
  %call21 = call float @_Z4coshf(float %mul20)    ; <float> [#uses=1]
  %add22 = fadd float %call17, %call21            ; <float> [#uses=3]
  %cmp23 = fcmp oeq float 0.000000e+000, %add22   ; <i1> [#uses=1]
  %sel24 = select i1 %cmp23, float 1.000000e+000, float %add22 ; <float> [#uses=0]
  %div25 = fdiv float %call13, %add22             ; <float> [#uses=1]
  %vecinit26 = insertelement <2 x float> %vecinit, float %div25, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit26, <2 x float>* %rval
  %tmp27 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp27, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define <2 x float> @polar(float %r, float %a) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %r.addr = alloca float, align 4                 ; <float*> [#uses=3]
  %a.addr = alloca float, align 4                 ; <float*> [#uses=3]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store float %r, float* %r.addr
  store float %a, float* %a.addr
  %tmp = load float* %a.addr                      ; <float> [#uses=1]
  %call = call float @_Z3cosf(float %tmp)         ; <float> [#uses=1]
  %tmp1 = load float* %r.addr                     ; <float> [#uses=1]
  %mul = fmul float %call, %tmp1                  ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %mul, i32 0 ; <<2 x float>> [#uses=1]
  %tmp2 = load float* %a.addr                     ; <float> [#uses=1]
  %call3 = call float @_Z3sinf(float %tmp2)       ; <float> [#uses=1]
  %tmp4 = load float* %r.addr                     ; <float> [#uses=1]
  %mul5 = fmul float %call3, %tmp4                ; <float> [#uses=1]
  %vecinit6 = insertelement <2 x float> %vecinit, float %mul5, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit6, <2 x float>* %rval
  %tmp7 = load <2 x float>* %rval                 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp7, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define <2 x float> @power(<2 x float> %z, i32 %p) nounwind {
entry:
  %retval = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=4]
  %p.addr = alloca i32, align 4                   ; <i32*> [#uses=3]
  %rval = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  store <2 x float> %z, <2 x float>* %z.addr
  store i32 %p, i32* %p.addr
  %tmp = load <2 x float>* %z.addr                ; <<2 x float>> [#uses=1]
  %call = call float @_Z6lengthDv2_f(<2 x float> %tmp) ; <float> [#uses=1]
  %tmp1 = load i32* %p.addr                       ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp1 to float               ; <float> [#uses=1]
  %call2 = call float @_Z3powff(float %call, float %conv) ; <float> [#uses=1]
  %tmp3 = load i32* %p.addr                       ; <i32> [#uses=1]
  %conv4 = sitofp i32 %tmp3 to float              ; <float> [#uses=1]
  %tmp5 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp6 = extractelement <2 x float> %tmp5, i32 1 ; <float> [#uses=1]
  %tmp7 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp8 = extractelement <2 x float> %tmp7, i32 0 ; <float> [#uses=1]
  %call9 = call float @_Z5atan2ff(float %tmp6, float %tmp8) ; <float> [#uses=1]
  %mul = fmul float %conv4, %call9                ; <float> [#uses=1]
  %call10 = call <2 x float> @polar(float %call2, float %mul) ; <<2 x float>> [#uses=1]
  store <2 x float> %call10, <2 x float>* %rval
  %tmp11 = load <2 x float>* %rval                ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp11, <2 x float>* %retval
  %0 = load <2 x float>* %retval                  ; <<2 x float>> [#uses=1]
  ret <2 x float> %0
}

; CHECK: ret
define void @getArgsSize(i32 addrspace(1)* %pSize) nounwind {
entry:
  %pSize.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  store i32 addrspace(1)* %pSize, i32 addrspace(1)** %pSize.addr
  %tmp = load i32 addrspace(1)** %pSize.addr      ; <i32 addrspace(1)*> [#uses=1]
  store i32 220, i32 addrspace(1)* %tmp
  ret void
}

; CHECK: ret
define <4 x float> @renderPixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd, <2 x float> %z, float* %pAlphaRemaining, i32* %pSign, i32 %iteration, <4 x float>* %pColorSoFar, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=1]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=1]
  %z.addr = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=3]
  %pAlphaRemaining.addr = alloca float*, align 4  ; <float**> [#uses=5]
  %pSign.addr = alloca i32*, align 4              ; <i32**> [#uses=5]
  %iteration.addr = alloca i32, align 4           ; <i32*> [#uses=3]
  %pColorSoFar.addr = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=3]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=11]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %d = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=3]
  %radius = alloca float, align 4                 ; <float*> [#uses=3]
  %newSign = alloca i32, align 4                  ; <i32*> [#uses=4]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store <2 x float> %z, <2 x float>* %z.addr
  store float* %pAlphaRemaining, float** %pAlphaRemaining.addr
  store i32* %pSign, i32** %pSign.addr
  store i32 %iteration, i32* %iteration.addr
  store <4 x float>* %pColorSoFar, <4 x float>** %pColorSoFar.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store i32 17, i32* %samplerLinear
  %tmp = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp1 = getelementptr inbounds %struct.anon addrspace(2)* %tmp, i32 0, i32 10 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp2 = load <2 x float> addrspace(2)* %tmp1    ; <<2 x float>> [#uses=1]
  %tmp3 = load <2 x float>* %z.addr               ; <<2 x float>> [#uses=1]
  %tmp4 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp5 = getelementptr inbounds %struct.anon addrspace(2)* %tmp4, i32 0, i32 3 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp6 = load <2 x float> addrspace(2)* %tmp5    ; <<2 x float>> [#uses=1]
  %add = fadd <2 x float> %tmp3, %tmp6            ; <<2 x float>> [#uses=1]
  %mul = fmul <2 x float> %tmp2, %add             ; <<2 x float>> [#uses=1]
  store <2 x float> %mul, <2 x float>* %d
  %tmp7 = load i32** %pSign.addr                  ; <i32*> [#uses=1]
  store i32 0, i32* %tmp7
  %tmp8 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp9 = getelementptr inbounds %struct.anon addrspace(2)* %tmp8, i32 0, i32 38 ; <i32 addrspace(2)*> [#uses=1]
  %tmp10 = load i32 addrspace(2)* %tmp9           ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp10, 0                 ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %tmp11 = load i32* %iteration.addr              ; <i32> [#uses=1]
  %cmp = icmp eq i32 %tmp11, 0                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %entry
  %tmp12 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp13 = load <2 x float>* %d                   ; <<2 x float>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %tmp12, i32 17, <2 x float> %tmp13) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %dst
  %tmp14 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  %tmp15 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  %tmp16 = extractelement <4 x float> %tmp15, i32 3 ; <float> [#uses=1]
  %tmp17 = load float** %pAlphaRemaining.addr     ; <float*> [#uses=1]
  %tmp18 = load float* %tmp17                     ; <float> [#uses=1]
  %mul19 = fmul float %tmp16, %tmp18              ; <float> [#uses=1]
  %tmp20 = insertelement <4 x float> undef, float %mul19, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp20, <4 x float> %tmp20, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul21 = fmul <4 x float> %tmp14, %splat        ; <<4 x float>> [#uses=1]
  %tmp22 = load <4 x float>** %pColorSoFar.addr   ; <<4 x float>*> [#uses=2]
  %tmp23 = load <4 x float>* %tmp22               ; <<4 x float>> [#uses=1]
  %add24 = fadd <4 x float> %tmp23, %mul21        ; <<4 x float>> [#uses=1]
  store <4 x float> %add24, <4 x float>* %tmp22
  %tmp25 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  %tmp26 = extractelement <4 x float> %tmp25, i32 3 ; <float> [#uses=1]
  %conv = fpext float %tmp26 to double            ; <double> [#uses=1]
  %sub = fsub double 1.000000e+000, %conv         ; <double> [#uses=1]
  %tmp27 = load float** %pAlphaRemaining.addr     ; <float*> [#uses=2]
  %tmp28 = load float* %tmp27                     ; <float> [#uses=1]
  %conv29 = fpext float %tmp28 to double          ; <double> [#uses=1]
  %mul30 = fmul double %conv29, %sub              ; <double> [#uses=1]
  %conv31 = fptrunc double %mul30 to float        ; <float> [#uses=1]
  store float %conv31, float* %tmp27
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  %tmp32 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp33 = getelementptr inbounds %struct.anon addrspace(2)* %tmp32, i32 0, i32 38 ; <i32 addrspace(2)*> [#uses=1]
  %tmp34 = load i32 addrspace(2)* %tmp33          ; <i32> [#uses=1]
  %tobool35 = icmp ne i32 %tmp34, 0               ; <i1> [#uses=1]
  br i1 %tobool35, label %if.then36, label %if.else

if.then36:                                        ; preds = %if.end
  %tmp37 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp38 = getelementptr inbounds %struct.anon addrspace(2)* %tmp37, i32 0, i32 39 ; <i32 addrspace(2)*> [#uses=1]
  %tmp39 = load i32 addrspace(2)* %tmp38          ; <i32> [#uses=1]
  %tobool40 = icmp ne i32 %tmp39, 0               ; <i1> [#uses=1]
  br i1 %tobool40, label %land.lhs.true, label %if.end50

land.lhs.true:                                    ; preds = %if.then36
  %tmp41 = load float** %pAlphaRemaining.addr     ; <float*> [#uses=1]
  %tmp42 = load float* %tmp41                     ; <float> [#uses=1]
  %tmp43 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp44 = getelementptr inbounds %struct.anon addrspace(2)* %tmp43, i32 0, i32 35 ; <float addrspace(2)*> [#uses=1]
  %tmp45 = load float addrspace(2)* %tmp44        ; <float> [#uses=1]
  %cmp46 = fcmp ogt float %tmp42, %tmp45          ; <i1> [#uses=1]
  br i1 %cmp46, label %if.then48, label %if.end50

if.then48:                                        ; preds = %land.lhs.true
  %tmp49 = load i32** %pSign.addr                 ; <i32*> [#uses=1]
  store i32 -1, i32* %tmp49
  br label %if.end50

if.end50:                                         ; preds = %if.then48, %land.lhs.true, %if.then36
  %tmp51 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp52 = getelementptr inbounds %struct.anon addrspace(2)* %tmp51, i32 0, i32 39 ; <i32 addrspace(2)*> [#uses=1]
  %tmp53 = load i32 addrspace(2)* %tmp52          ; <i32> [#uses=1]
  %tobool54 = icmp ne i32 %tmp53, 0               ; <i1> [#uses=1]
  br i1 %tobool54, label %if.end65, label %land.lhs.true55

land.lhs.true55:                                  ; preds = %if.end50
  %tmp56 = load float** %pAlphaRemaining.addr     ; <float*> [#uses=1]
  %tmp57 = load float* %tmp56                     ; <float> [#uses=1]
  %tmp58 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp59 = getelementptr inbounds %struct.anon addrspace(2)* %tmp58, i32 0, i32 35 ; <float addrspace(2)*> [#uses=1]
  %tmp60 = load float addrspace(2)* %tmp59        ; <float> [#uses=1]
  %cmp61 = fcmp ogt float %tmp57, %tmp60          ; <i1> [#uses=1]
  br i1 %cmp61, label %if.then63, label %if.end65

if.then63:                                        ; preds = %land.lhs.true55
  %tmp64 = load i32** %pSign.addr                 ; <i32*> [#uses=1]
  store i32 1, i32* %tmp64
  br label %if.end65

if.end65:                                         ; preds = %if.then63, %land.lhs.true55, %if.end50
  br label %if.end98

if.else:                                          ; preds = %if.end
  %tmp66 = load i32* %iteration.addr              ; <i32> [#uses=1]
  %cmp67 = icmp sgt i32 %tmp66, 0                 ; <i1> [#uses=1]
  br i1 %cmp67, label %if.then69, label %if.end74

if.then69:                                        ; preds = %if.else
  %tmp70 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp71 = load <2 x float>* %d                   ; <<2 x float>> [#uses=1]
  %call72 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %tmp70, i32 17, <2 x float> %tmp71) ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>** %pColorSoFar.addr   ; <<4 x float>*> [#uses=1]
  store <4 x float> %call72, <4 x float>* %tmp73
  br label %if.end74

if.end74:                                         ; preds = %if.then69, %if.else
  %tmp76 = load <2 x float>* %z.addr              ; <<2 x float>> [#uses=1]
  %call77 = call float @_Z6lengthDv2_f(<2 x float> %tmp76) ; <float> [#uses=1]
  store float %call77, float* %radius
  store i32 -1, i32* %newSign
  %tmp79 = load float* %radius                    ; <float> [#uses=1]
  %tmp80 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp81 = getelementptr inbounds %struct.anon addrspace(2)* %tmp80, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp82 = load float addrspace(2)* %tmp81        ; <float> [#uses=1]
  %cmp83 = fcmp oge float %tmp79, %tmp82          ; <i1> [#uses=1]
  br i1 %cmp83, label %if.then85, label %if.end95

if.then85:                                        ; preds = %if.end74
  %tmp86 = load float* %radius                    ; <float> [#uses=1]
  %tmp87 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp88 = getelementptr inbounds %struct.anon addrspace(2)* %tmp87, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp89 = load float addrspace(2)* %tmp88        ; <float> [#uses=1]
  %cmp90 = fcmp ogt float %tmp86, %tmp89          ; <i1> [#uses=1]
  br i1 %cmp90, label %if.then92, label %if.else93

if.then92:                                        ; preds = %if.then85
  store i32 1, i32* %newSign
  br label %if.end94

if.else93:                                        ; preds = %if.then85
  store i32 0, i32* %newSign
  br label %if.end94

if.end94:                                         ; preds = %if.else93, %if.then92
  br label %if.end95

if.end95:                                         ; preds = %if.end94, %if.end74
  %tmp96 = load i32* %newSign                     ; <i32> [#uses=1]
  %tmp97 = load i32** %pSign.addr                 ; <i32*> [#uses=1]
  store i32 %tmp96, i32* %tmp97
  br label %if.end98

if.end98:                                         ; preds = %if.end95, %if.end65
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t*, i32, <2 x float>)

; CHECK: ret
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=4]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=56]
  %PI = alloca float, align 4                     ; <float*> [#uses=4]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %I = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=4]
  %negI = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=4]
  %s = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=4]
  %d = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=0]
  %ratio = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=6]
  %radius = alloca float, align 4                 ; <float*> [#uses=0]
  %theta = alloca float, align 4                  ; <float*> [#uses=5]
  %div = alloca float, align 4                    ; <float*> [#uses=2]
  %iteration = alloca i32, align 4                ; <i32*> [#uses=7]
  %sign = alloca i32, align 4                     ; <i32*> [#uses=6]
  %alphaRemaining = alloca float, align 4         ; <float*> [#uses=3]
  %z = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=35]
  %cd = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=2]
  %cm1 = alloca <2 x float>, align 8              ; <<2 x float>*> [#uses=2]
  %ce = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=2]
  %cm2 = alloca <2 x float>, align 8              ; <<2 x float>*> [#uses=2]
  %polar = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=4]
  %div_temp = alloca <2 x float>, align 8         ; <<2 x float>*> [#uses=2]
  %ct_temp = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=2]
  %z_tmp = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=2]
  %alpha = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=3]
  %f = alloca <2 x float>, align 8                ; <<2 x float>*> [#uses=2]
  %beta = alloca <2 x float>, align 8             ; <<2 x float>*> [#uses=2]
  %angle = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=9]
  %p1_tmp = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %r1_tmp = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  %r2r1t = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=2]
  %r1r2t = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=2]
  %colorSoFar = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %r2r1t422 = alloca <2 x float>, align 8         ; <<2 x float>*> [#uses=2]
  %r1r2t446 = alloca <2 x float>, align 8         ; <<2 x float>*> [#uses=2]
  %maxIteration = alloca i32, align 4             ; <i32*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store float 0x400921FB60000000, float* %PI
  store <4 x float> zeroinitializer, <4 x float>* %kZero
  store <2 x float> <float 0.000000e+000, float 1.000000e+000>, <2 x float>* %I
  store <2 x float> <float -0.000000e+000, float -1.000000e+000>, <2 x float>* %negI
  %tmp = load <2 x float>* %outCrd.addr           ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp, <2 x float>* %s
  store i32 0, i32* %sign
  store float 1.000000e+000, float* %alphaRemaining
  %tmp10 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp11 = getelementptr inbounds %struct.anon addrspace(2)* %tmp10, i32 0, i32 7 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp12 = load <2 x float> addrspace(2)* %tmp11  ; <<2 x float>> [#uses=1]
  %tmp13 = extractelement <2 x float> %tmp12, i32 0 ; <float> [#uses=1]
  %tmp14 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp15 = getelementptr inbounds %struct.anon addrspace(2)* %tmp14, i32 0, i32 7 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp16 = load <2 x float> addrspace(2)* %tmp15  ; <<2 x float>> [#uses=1]
  %tmp17 = extractelement <2 x float> %tmp16, i32 1 ; <float> [#uses=1]
  %tmp18 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp19 = getelementptr inbounds %struct.anon addrspace(2)* %tmp18, i32 0, i32 7 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp20 = load <2 x float> addrspace(2)* %tmp19  ; <<2 x float>> [#uses=1]
  %tmp21 = extractelement <2 x float> %tmp20, i32 0 ; <float> [#uses=1]
  %sub = fsub float %tmp17, %tmp21                ; <float> [#uses=1]
  %tmp22 = load <2 x float>* %s                   ; <<2 x float>> [#uses=1]
  %tmp23 = extractelement <2 x float> %tmp22, i32 0 ; <float> [#uses=1]
  %tmp24 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp25 = getelementptr inbounds %struct.anon addrspace(2)* %tmp24, i32 0, i32 4 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp26 = load <2 x float> addrspace(2)* %tmp25  ; <<2 x float>> [#uses=1]
  %tmp27 = extractelement <2 x float> %tmp26, i32 0 ; <float> [#uses=1]
  %sub28 = fsub float %tmp23, %tmp27              ; <float> [#uses=1]
  %tmp29 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp30 = getelementptr inbounds %struct.anon addrspace(2)* %tmp29, i32 0, i32 33 ; <float addrspace(2)*> [#uses=1]
  %tmp31 = load float addrspace(2)* %tmp30        ; <float> [#uses=1]
  %div32 = fdiv float %tmp31, 2.000000e+000       ; <float> [#uses=1]
  %add = fadd float %sub28, %div32                ; <float> [#uses=1]
  %mul = fmul float %sub, %add                    ; <float> [#uses=1]
  %tmp33 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp34 = getelementptr inbounds %struct.anon addrspace(2)* %tmp33, i32 0, i32 33 ; <float addrspace(2)*> [#uses=1]
  %tmp35 = load float addrspace(2)* %tmp34        ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %tmp35     ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %tmp35 ; <float> [#uses=0]
  %div36 = fdiv float %mul, %tmp35                ; <float> [#uses=1]
  %add37 = fadd float %tmp13, %div36              ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %add37, i32 0 ; <<2 x float>> [#uses=1]
  %tmp38 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp39 = getelementptr inbounds %struct.anon addrspace(2)* %tmp38, i32 0, i32 8 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp40 = load <2 x float> addrspace(2)* %tmp39  ; <<2 x float>> [#uses=1]
  %tmp41 = extractelement <2 x float> %tmp40, i32 0 ; <float> [#uses=1]
  %tmp42 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp43 = getelementptr inbounds %struct.anon addrspace(2)* %tmp42, i32 0, i32 8 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp44 = load <2 x float> addrspace(2)* %tmp43  ; <<2 x float>> [#uses=1]
  %tmp45 = extractelement <2 x float> %tmp44, i32 1 ; <float> [#uses=1]
  %tmp46 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp47 = getelementptr inbounds %struct.anon addrspace(2)* %tmp46, i32 0, i32 8 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp48 = load <2 x float> addrspace(2)* %tmp47  ; <<2 x float>> [#uses=1]
  %tmp49 = extractelement <2 x float> %tmp48, i32 0 ; <float> [#uses=1]
  %sub50 = fsub float %tmp45, %tmp49              ; <float> [#uses=1]
  %tmp51 = load <2 x float>* %s                   ; <<2 x float>> [#uses=1]
  %tmp52 = extractelement <2 x float> %tmp51, i32 1 ; <float> [#uses=1]
  %tmp53 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp54 = getelementptr inbounds %struct.anon addrspace(2)* %tmp53, i32 0, i32 4 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp55 = load <2 x float> addrspace(2)* %tmp54  ; <<2 x float>> [#uses=1]
  %tmp56 = extractelement <2 x float> %tmp55, i32 1 ; <float> [#uses=1]
  %sub57 = fsub float %tmp52, %tmp56              ; <float> [#uses=1]
  %tmp58 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp59 = getelementptr inbounds %struct.anon addrspace(2)* %tmp58, i32 0, i32 34 ; <float addrspace(2)*> [#uses=1]
  %tmp60 = load float addrspace(2)* %tmp59        ; <float> [#uses=1]
  %div61 = fdiv float %tmp60, 2.000000e+000       ; <float> [#uses=1]
  %add62 = fadd float %sub57, %div61              ; <float> [#uses=1]
  %mul63 = fmul float %sub50, %add62              ; <float> [#uses=1]
  %tmp64 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp65 = getelementptr inbounds %struct.anon addrspace(2)* %tmp64, i32 0, i32 34 ; <float addrspace(2)*> [#uses=1]
  %tmp66 = load float addrspace(2)* %tmp65        ; <float> [#uses=3]
  %cmp67 = fcmp oeq float 0.000000e+000, %tmp66   ; <i1> [#uses=1]
  %sel68 = select i1 %cmp67, float 1.000000e+000, float %tmp66 ; <float> [#uses=0]
  %div69 = fdiv float %mul63, %tmp66              ; <float> [#uses=1]
  %add70 = fadd float %tmp41, %div69              ; <float> [#uses=1]
  %vecinit71 = insertelement <2 x float> %vecinit, float %add70, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit71, <2 x float>* %z
  %tmp72 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp73 = getelementptr inbounds %struct.anon addrspace(2)* %tmp72, i32 0, i32 40 ; <i32 addrspace(2)*> [#uses=1]
  %tmp74 = load i32 addrspace(2)* %tmp73          ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp74, 0                 ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp76 = load <2 x float>* %z                   ; <<2 x float>> [#uses=1]
  %tmp77 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp78 = getelementptr inbounds %struct.anon addrspace(2)* %tmp77, i32 0, i32 9 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp79 = load <2 x float> addrspace(2)* %tmp78  ; <<2 x float>> [#uses=1]
  %sub80 = fsub <2 x float> %tmp76, %tmp79        ; <<2 x float>> [#uses=1]
  %tmp81 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp82 = getelementptr inbounds %struct.anon addrspace(2)* %tmp81, i32 0, i32 6 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp83 = load <2 x float> addrspace(2)* %tmp82  ; <<2 x float>> [#uses=1]
  %call = call <2 x float> @complexReciprocal(<2 x float> %tmp83) ; <<2 x float>> [#uses=1]
  %call84 = call <2 x float> @complexMult(<2 x float> %sub80, <2 x float> %call) ; <<2 x float>> [#uses=1]
  store <2 x float> %call84, <2 x float>* %cd
  %tmp86 = load <2 x float>* %negI                ; <<2 x float>> [#uses=1]
  %tmp87 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp88 = getelementptr inbounds %struct.anon addrspace(2)* %tmp87, i32 0, i32 5 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp89 = load <2 x float> addrspace(2)* %tmp88  ; <<2 x float>> [#uses=1]
  %call90 = call <2 x float> @complexMult(<2 x float> %tmp86, <2 x float> %tmp89) ; <<2 x float>> [#uses=1]
  store <2 x float> %call90, <2 x float>* %cm1
  %tmp92 = load <2 x float>* %cm1                 ; <<2 x float>> [#uses=1]
  %call93 = call <2 x float> @complexExp(<2 x float> %tmp92) ; <<2 x float>> [#uses=1]
  store <2 x float> %call93, <2 x float>* %ce
  %tmp95 = load <2 x float>* %cd                  ; <<2 x float>> [#uses=1]
  %tmp96 = load <2 x float>* %ce                  ; <<2 x float>> [#uses=1]
  %call97 = call <2 x float> @complexMult(<2 x float> %tmp95, <2 x float> %tmp96) ; <<2 x float>> [#uses=1]
  store <2 x float> %call97, <2 x float>* %cm2
  %tmp98 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp99 = getelementptr inbounds %struct.anon addrspace(2)* %tmp98, i32 0, i32 9 ; <<2 x float> addrspace(2)*> [#uses=1]
  %tmp100 = load <2 x float> addrspace(2)* %tmp99 ; <<2 x float>> [#uses=1]
  %tmp101 = load <2 x float>* %cm2                ; <<2 x float>> [#uses=1]
  %add102 = fadd <2 x float> %tmp100, %tmp101     ; <<2 x float>> [#uses=1]
  store <2 x float> %add102, <2 x float>* %z
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp104 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp105 = getelementptr inbounds %struct.anon addrspace(2)* %tmp104, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp106 = load <4 x float> addrspace(2)* %tmp105 ; <<4 x float>> [#uses=1]
  %tmp107 = extractelement <4 x float> %tmp106, i32 1 ; <float> [#uses=1]
  %vecinit108 = insertelement <2 x float> undef, float %tmp107, i32 0 ; <<2 x float>> [#uses=1]
  %tmp109 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp110 = getelementptr inbounds %struct.anon addrspace(2)* %tmp109, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp111 = load <4 x float> addrspace(2)* %tmp110 ; <<4 x float>> [#uses=1]
  %tmp112 = extractelement <4 x float> %tmp111, i32 2 ; <float> [#uses=1]
  %vecinit113 = insertelement <2 x float> %vecinit108, float %tmp112, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit113, <2 x float>* %polar
  %tmp114 = load <2 x float>* %polar              ; <<2 x float>> [#uses=1]
  %tmp115 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp116 = getelementptr inbounds %struct.anon addrspace(2)* %tmp115, i32 0, i32 33 ; <float addrspace(2)*> [#uses=1]
  %tmp117 = load float addrspace(2)* %tmp116      ; <float> [#uses=1]
  %tmp118 = insertelement <2 x float> undef, float %tmp117, i32 0 ; <<2 x float>> [#uses=2]
  %splat = shufflevector <2 x float> %tmp118, <2 x float> %tmp118, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=1]
  %mul119 = fmul <2 x float> %tmp114, %splat      ; <<2 x float>> [#uses=1]
  %tmp120 = load <2 x float>* %s                  ; <<2 x float>> [#uses=1]
  %tmp121 = extractelement <2 x float> %tmp120, i32 0 ; <float> [#uses=1]
  %tmp122 = insertelement <2 x float> undef, float %tmp121, i32 0 ; <<2 x float>> [#uses=2]
  %splat123 = shufflevector <2 x float> %tmp122, <2 x float> %tmp122, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=3]
  %cmp124 = fcmp oeq <2 x float> zeroinitializer, %splat123 ; <<2 x i1>> [#uses=1]
  %sel125 = select <2 x i1> %cmp124, <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float> %splat123 ; <<2 x float>> [#uses=0]
  %div126 = fdiv <2 x float> %mul119, %splat123   ; <<2 x float>> [#uses=1]
  %div127 = fdiv <2 x float> %div126, <float 1.000000e+002, float 1.000000e+002> ; <<2 x float>> [#uses=1]
  store <2 x float> %div127, <2 x float>* %polar
  %tmp128 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp129 = getelementptr inbounds %struct.anon addrspace(2)* %tmp128, i32 0, i32 36 ; <i32 addrspace(2)*> [#uses=1]
  %tmp130 = load i32 addrspace(2)* %tmp129        ; <i32> [#uses=1]
  %tobool131 = icmp ne i32 %tmp130, 0             ; <i1> [#uses=1]
  br i1 %tobool131, label %if.then132, label %if.else

if.then132:                                       ; preds = %if.end
  %tmp133 = load float* %PI                       ; <float> [#uses=1]
  %div134 = fdiv float %tmp133, 1.800000e+002     ; <float> [#uses=1]
  %tmp135 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp136 = getelementptr inbounds %struct.anon addrspace(2)* %tmp135, i32 0, i32 0 ; <<4 x float> addrspace(2)*> [#uses=1]
  %tmp137 = load <4 x float> addrspace(2)* %tmp136 ; <<4 x float>> [#uses=1]
  %tmp138 = extractelement <4 x float> %tmp137, i32 0 ; <float> [#uses=1]
  %mul139 = fmul float %div134, %tmp138           ; <float> [#uses=1]
  store float %mul139, float* %theta
  %tmp140 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp141 = extractelement <2 x float> %tmp140, i32 0 ; <float> [#uses=1]
  %call142 = call float @_Z3powff(float %tmp141, float 2.000000e+000) ; <float> [#uses=1]
  %add143 = fadd float 1.000000e+000, %call142    ; <float> [#uses=1]
  %tmp144 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp145 = extractelement <2 x float> %tmp144, i32 1 ; <float> [#uses=1]
  %call146 = call float @_Z3powff(float %tmp145, float 2.000000e+000) ; <float> [#uses=1]
  %add147 = fadd float %add143, %call146          ; <float> [#uses=1]
  %tmp148 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp149 = extractelement <2 x float> %tmp148, i32 0 ; <float> [#uses=1]
  %call150 = call float @_Z3powff(float %tmp149, float 2.000000e+000) ; <float> [#uses=1]
  %sub151 = fsub float 1.000000e+000, %call150    ; <float> [#uses=1]
  %tmp152 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp153 = extractelement <2 x float> %tmp152, i32 1 ; <float> [#uses=1]
  %call154 = call float @_Z3powff(float %tmp153, float 2.000000e+000) ; <float> [#uses=1]
  %sub155 = fsub float %sub151, %call154          ; <float> [#uses=1]
  %tmp156 = load float* %theta                    ; <float> [#uses=1]
  %call157 = call float @_Z3cosf(float %tmp156)   ; <float> [#uses=1]
  %mul158 = fmul float %sub155, %call157          ; <float> [#uses=1]
  %add159 = fadd float %add147, %mul158           ; <float> [#uses=1]
  %tmp160 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp161 = extractelement <2 x float> %tmp160, i32 0 ; <float> [#uses=1]
  %mul162 = fmul float 2.000000e+000, %tmp161     ; <float> [#uses=1]
  %tmp163 = load float* %theta                    ; <float> [#uses=1]
  %call164 = call float @_Z3sinf(float %tmp163)   ; <float> [#uses=1]
  %mul165 = fmul float %mul162, %call164          ; <float> [#uses=1]
  %sub166 = fsub float %add159, %mul165           ; <float> [#uses=1]
  %div167 = fdiv float %sub166, 2.000000e+000     ; <float> [#uses=1]
  store float %div167, float* %div
  %tmp168 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp169 = extractelement <2 x float> %tmp168, i32 0 ; <float> [#uses=1]
  %tmp170 = load float* %theta                    ; <float> [#uses=1]
  %call171 = call float @_Z3cosf(float %tmp170)   ; <float> [#uses=1]
  %mul172 = fmul float %tmp169, %call171          ; <float> [#uses=1]
  %conv = fpext float %mul172 to double           ; <double> [#uses=1]
  %tmp173 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp174 = extractelement <2 x float> %tmp173, i32 0 ; <float> [#uses=1]
  %call175 = call float @_Z3powff(float %tmp174, float 2.000000e+000) ; <float> [#uses=1]
  %conv176 = fpext float %call175 to double       ; <double> [#uses=1]
  %sub177 = fsub double 1.000000e+000, %conv176   ; <double> [#uses=1]
  %tmp178 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp179 = extractelement <2 x float> %tmp178, i32 1 ; <float> [#uses=1]
  %call180 = call float @_Z3powff(float %tmp179, float 2.000000e+000) ; <float> [#uses=1]
  %conv181 = fpext float %call180 to double       ; <double> [#uses=1]
  %sub182 = fsub double %sub177, %conv181         ; <double> [#uses=1]
  %tmp183 = load float* %theta                    ; <float> [#uses=1]
  %call184 = call float @_Z3sinf(float %tmp183)   ; <float> [#uses=1]
  %conv185 = fpext float %call184 to double       ; <double> [#uses=1]
  %mul186 = fmul double %sub182, %conv185         ; <double> [#uses=1]
  %div187 = fdiv double %mul186, 2.000000e+000    ; <double> [#uses=1]
  %add188 = fadd double %conv, %div187            ; <double> [#uses=1]
  %conv189 = fptrunc double %add188 to float      ; <float> [#uses=1]
  %tmp190 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp191 = insertelement <2 x float> %tmp190, float %conv189, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp191, <2 x float>* %z
  %tmp193 = load float* %div                      ; <float> [#uses=1]
  %vecinit194 = insertelement <2 x float> undef, float %tmp193, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit195 = insertelement <2 x float> %vecinit194, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit195, <2 x float>* %div_temp
  %tmp196 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp197 = load <2 x float>* %div_temp           ; <<2 x float>> [#uses=1]
  %call198 = call <2 x float> @complexReciprocal(<2 x float> %tmp197) ; <<2 x float>> [#uses=1]
  %call199 = call <2 x float> @complexMult(<2 x float> %tmp196, <2 x float> %call198) ; <<2 x float>> [#uses=1]
  store <2 x float> %call199, <2 x float>* %z
  br label %if.end225

if.else:                                          ; preds = %if.end
  %tmp200 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp201 = getelementptr inbounds %struct.anon addrspace(2)* %tmp200, i32 0, i32 37 ; <i32 addrspace(2)*> [#uses=1]
  %tmp202 = load i32 addrspace(2)* %tmp201        ; <i32> [#uses=1]
  %tobool203 = icmp ne i32 %tmp202, 0             ; <i1> [#uses=1]
  br i1 %tobool203, label %if.then204, label %if.end207

if.then204:                                       ; preds = %if.else
  %tmp205 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %call206 = call <2 x float> @complexSin(<2 x float> %tmp205) ; <<2 x float>> [#uses=1]
  store <2 x float> %call206, <2 x float>* %z
  br label %if.end207

if.end207:                                        ; preds = %if.then204, %if.else
  %tmp208 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp209 = getelementptr inbounds %struct.anon addrspace(2)* %tmp208, i32 0, i32 27 ; <i32 addrspace(2)*> [#uses=1]
  %tmp210 = load i32 addrspace(2)* %tmp209        ; <i32> [#uses=1]
  %cmp211 = icmp eq i32 %tmp210, 1                ; <i1> [#uses=1]
  br i1 %cmp211, label %if.then213, label %if.end224

if.then213:                                       ; preds = %if.end207
  %tmp214 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp215 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp216 = getelementptr inbounds %struct.anon addrspace(2)* %tmp215, i32 0, i32 28 ; <i32 addrspace(2)*> [#uses=1]
  %tmp217 = load i32 addrspace(2)* %tmp216        ; <i32> [#uses=1]
  %call218 = call <2 x float> @power(<2 x float> %tmp214, i32 %tmp217) ; <<2 x float>> [#uses=1]
  store <2 x float> %call218, <2 x float>* %z
  store <2 x float> <float 2.000000e+000, float 0.000000e+000>, <2 x float>* %ct_temp
  %tmp220 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp221 = load <2 x float>* %ct_temp            ; <<2 x float>> [#uses=1]
  %call222 = call <2 x float> @complexMult(<2 x float> %tmp220, <2 x float> %tmp221) ; <<2 x float>> [#uses=1]
  %call223 = call <2 x float> @complexTan(<2 x float> %call222) ; <<2 x float>> [#uses=1]
  store <2 x float> %call223, <2 x float>* %z
  br label %if.end224

if.end224:                                        ; preds = %if.then213, %if.end207
  br label %if.end225

if.end225:                                        ; preds = %if.end224, %if.then132
  %tmp226 = load <2 x float>* %polar              ; <<2 x float>> [#uses=1]
  %tmp227 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %add228 = fadd <2 x float> %tmp227, %tmp226     ; <<2 x float>> [#uses=1]
  store <2 x float> %add228, <2 x float>* %z
  %tmp229 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp230 = getelementptr inbounds %struct.anon addrspace(2)* %tmp229, i32 0, i32 40 ; <i32 addrspace(2)*> [#uses=1]
  %tmp231 = load i32 addrspace(2)* %tmp230        ; <i32> [#uses=1]
  %tobool232 = icmp ne i32 %tmp231, 0             ; <i1> [#uses=1]
  br i1 %tobool232, label %if.then233, label %if.end245

if.then233:                                       ; preds = %if.end225
  %tmp235 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp236 = getelementptr inbounds %struct.anon addrspace(2)* %tmp235, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp237 = load float addrspace(2)* %tmp236      ; <float> [#uses=1]
  %vecinit238 = insertelement <2 x float> undef, float %tmp237, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit239 = insertelement <2 x float> %vecinit238, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit239, <2 x float>* %z_tmp
  %tmp240 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp241 = load <2 x float>* %z_tmp              ; <<2 x float>> [#uses=1]
  %call242 = call <2 x float> @complexReciprocal(<2 x float> %tmp241) ; <<2 x float>> [#uses=1]
  %call243 = call <2 x float> @complexMult(<2 x float> %tmp240, <2 x float> %call242) ; <<2 x float>> [#uses=1]
  %call244 = call <2 x float> @complexLog(<2 x float> %call243) ; <<2 x float>> [#uses=1]
  store <2 x float> %call244, <2 x float>* %z
  br label %if.end245

if.end245:                                        ; preds = %if.then233, %if.end225
  %tmp247 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp248 = getelementptr inbounds %struct.anon addrspace(2)* %tmp247, i32 0, i32 32 ; <float addrspace(2)*> [#uses=1]
  %tmp249 = load float addrspace(2)* %tmp248      ; <float> [#uses=1]
  %tmp250 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp251 = getelementptr inbounds %struct.anon addrspace(2)* %tmp250, i32 0, i32 31 ; <float addrspace(2)*> [#uses=1]
  %tmp252 = load float addrspace(2)* %tmp251      ; <float> [#uses=3]
  %cmp253 = fcmp oeq float 0.000000e+000, %tmp252 ; <i1> [#uses=1]
  %sel254 = select i1 %cmp253, float 1.000000e+000, float %tmp252 ; <float> [#uses=0]
  %div255 = fdiv float %tmp249, %tmp252           ; <float> [#uses=1]
  %tmp256 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp257 = getelementptr inbounds %struct.anon addrspace(2)* %tmp256, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp258 = load float addrspace(2)* %tmp257      ; <float> [#uses=1]
  %tmp259 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp260 = getelementptr inbounds %struct.anon addrspace(2)* %tmp259, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp261 = load float addrspace(2)* %tmp260      ; <float> [#uses=3]
  %cmp262 = fcmp oeq float 0.000000e+000, %tmp261 ; <i1> [#uses=1]
  %sel263 = select i1 %cmp262, float 1.000000e+000, float %tmp261 ; <float> [#uses=0]
  %div264 = fdiv float %tmp258, %tmp261           ; <float> [#uses=1]
  %call265 = call float @_Z3logf(float %div264)   ; <float> [#uses=1]
  %tmp266 = load float* %PI                       ; <float> [#uses=1]
  %mul267 = fmul float 2.000000e+000, %tmp266     ; <float> [#uses=3]
  %cmp268 = fcmp oeq float 0.000000e+000, %mul267 ; <i1> [#uses=1]
  %sel269 = select i1 %cmp268, float 1.000000e+000, float %mul267 ; <float> [#uses=0]
  %div270 = fdiv float %call265, %mul267          ; <float> [#uses=1]
  %mul271 = fmul float %div255, %div270           ; <float> [#uses=1]
  %call272 = call float @_Z4atanf(float %mul271)  ; <float> [#uses=1]
  %vecinit273 = insertelement <2 x float> undef, float %call272, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit274 = insertelement <2 x float> %vecinit273, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit274, <2 x float>* %alpha
  %tmp276 = load <2 x float>* %alpha              ; <<2 x float>> [#uses=1]
  %tmp277 = extractelement <2 x float> %tmp276, i32 0 ; <float> [#uses=1]
  %call278 = call float @_Z3cosf(float %tmp277)   ; <float> [#uses=1]
  %vecinit279 = insertelement <2 x float> undef, float %call278, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit280 = insertelement <2 x float> %vecinit279, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit280, <2 x float>* %f
  %tmp282 = load <2 x float>* %f                  ; <<2 x float>> [#uses=1]
  %tmp283 = load <2 x float>* %alpha              ; <<2 x float>> [#uses=1]
  %tmp284 = load <2 x float>* %I                  ; <<2 x float>> [#uses=1]
  %call285 = call <2 x float> @complexMult(<2 x float> %tmp283, <2 x float> %tmp284) ; <<2 x float>> [#uses=1]
  %call286 = call <2 x float> @complexExp(<2 x float> %call285) ; <<2 x float>> [#uses=1]
  %call287 = call <2 x float> @complexMult(<2 x float> %tmp282, <2 x float> %call286) ; <<2 x float>> [#uses=1]
  store <2 x float> %call287, <2 x float>* %beta
  %tmp289 = load float* %PI                       ; <float> [#uses=1]
  %mul290 = fmul float -2.000000e+000, %tmp289    ; <float> [#uses=1]
  %tmp291 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp292 = getelementptr inbounds %struct.anon addrspace(2)* %tmp291, i32 0, i32 31 ; <float addrspace(2)*> [#uses=1]
  %tmp293 = load float addrspace(2)* %tmp292      ; <float> [#uses=1]
  %mul294 = fmul float %mul290, %tmp293           ; <float> [#uses=1]
  %vecinit295 = insertelement <2 x float> undef, float %mul294, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit296 = insertelement <2 x float> %vecinit295, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit296, <2 x float>* %angle
  %tmp297 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp298 = getelementptr inbounds %struct.anon addrspace(2)* %tmp297, i32 0, i32 32 ; <float addrspace(2)*> [#uses=1]
  %tmp299 = load float addrspace(2)* %tmp298      ; <float> [#uses=1]
  %cmp300 = fcmp ogt float %tmp299, 0.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp300, label %if.then302, label %if.end304

if.then302:                                       ; preds = %if.end245
  %tmp303 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %neg = fsub <2 x float> <float -0.000000e+000, float -0.000000e+000>, %tmp303 ; <<2 x float>> [#uses=1]
  store <2 x float> %neg, <2 x float>* %angle
  br label %if.end304

if.end304:                                        ; preds = %if.then302, %if.end245
  %tmp305 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp306 = getelementptr inbounds %struct.anon addrspace(2)* %tmp305, i32 0, i32 16 ; <i32 addrspace(2)*> [#uses=1]
  %tmp307 = load i32 addrspace(2)* %tmp306        ; <i32> [#uses=1]
  %cmp308 = icmp eq i32 %tmp307, 1                ; <i1> [#uses=1]
  br i1 %cmp308, label %if.then310, label %if.end320

if.then310:                                       ; preds = %if.end304
  %tmp311 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp312 = getelementptr inbounds %struct.anon addrspace(2)* %tmp311, i32 0, i32 15 ; <float addrspace(2)*> [#uses=1]
  %tmp313 = load float addrspace(2)* %tmp312      ; <float> [#uses=1]
  %tmp314 = insertelement <2 x float> undef, float %tmp313, i32 0 ; <<2 x float>> [#uses=2]
  %splat315 = shufflevector <2 x float> %tmp314, <2 x float> %tmp314, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=3]
  %tmp316 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %cmp317 = fcmp oeq <2 x float> zeroinitializer, %splat315 ; <<2 x i1>> [#uses=1]
  %sel318 = select <2 x i1> %cmp317, <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float> %splat315 ; <<2 x float>> [#uses=0]
  %div319 = fdiv <2 x float> %tmp316, %splat315   ; <<2 x float>> [#uses=1]
  store <2 x float> %div319, <2 x float>* %angle
  br label %if.end320

if.end320:                                        ; preds = %if.then310, %if.end304
  %tmp322 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp323 = getelementptr inbounds %struct.anon addrspace(2)* %tmp322, i32 0, i32 31 ; <float addrspace(2)*> [#uses=1]
  %tmp324 = load float addrspace(2)* %tmp323      ; <float> [#uses=1]
  %vecinit325 = insertelement <2 x float> undef, float %tmp324, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit326 = insertelement <2 x float> %vecinit325, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit326, <2 x float>* %p1_tmp
  %tmp327 = load <2 x float>* %p1_tmp             ; <<2 x float>> [#uses=1]
  %tmp328 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %call329 = call <2 x float> @complexMult(<2 x float> %tmp327, <2 x float> %tmp328) ; <<2 x float>> [#uses=1]
  %tmp330 = load <2 x float>* %beta               ; <<2 x float>> [#uses=1]
  %call331 = call <2 x float> @complexReciprocal(<2 x float> %tmp330) ; <<2 x float>> [#uses=1]
  %call332 = call <2 x float> @complexMult(<2 x float> %call329, <2 x float> %call331) ; <<2 x float>> [#uses=1]
  store <2 x float> %call332, <2 x float>* %z
  %tmp334 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp335 = getelementptr inbounds %struct.anon addrspace(2)* %tmp334, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp336 = load float addrspace(2)* %tmp335      ; <float> [#uses=1]
  %vecinit337 = insertelement <2 x float> undef, float %tmp336, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit338 = insertelement <2 x float> %vecinit337, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit338, <2 x float>* %r1_tmp
  %tmp339 = load <2 x float>* %r1_tmp             ; <<2 x float>> [#uses=1]
  %tmp340 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %call341 = call <2 x float> @complexExp(<2 x float> %tmp340) ; <<2 x float>> [#uses=1]
  %call342 = call <2 x float> @complexMult(<2 x float> %tmp339, <2 x float> %call341) ; <<2 x float>> [#uses=1]
  store <2 x float> %call342, <2 x float>* %z
  %tmp343 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp344 = getelementptr inbounds %struct.anon addrspace(2)* %tmp343, i32 0, i32 38 ; <i32 addrspace(2)*> [#uses=1]
  %tmp345 = load i32 addrspace(2)* %tmp344        ; <i32> [#uses=1]
  %tobool346 = icmp ne i32 %tmp345, 0             ; <i1> [#uses=1]
  br i1 %tobool346, label %land.lhs.true, label %if.end408

land.lhs.true:                                    ; preds = %if.end320
  %tmp347 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp348 = getelementptr inbounds %struct.anon addrspace(2)* %tmp347, i32 0, i32 20 ; <i32 addrspace(2)*> [#uses=1]
  %tmp349 = load i32 addrspace(2)* %tmp348        ; <i32> [#uses=1]
  %cmp350 = icmp sgt i32 %tmp349, 0               ; <i1> [#uses=1]
  br i1 %cmp350, label %if.then352, label %if.end408

if.then352:                                       ; preds = %land.lhs.true
  %tmp353 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp354 = getelementptr inbounds %struct.anon addrspace(2)* %tmp353, i32 0, i32 39 ; <i32 addrspace(2)*> [#uses=1]
  %tmp355 = load i32 addrspace(2)* %tmp354        ; <i32> [#uses=1]
  %tobool356 = icmp ne i32 %tmp355, 0             ; <i1> [#uses=1]
  br i1 %tobool356, label %if.end376, label %if.then357

if.then357:                                       ; preds = %if.then352
  %tmp359 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp360 = getelementptr inbounds %struct.anon addrspace(2)* %tmp359, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp361 = load float addrspace(2)* %tmp360      ; <float> [#uses=1]
  %tmp362 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp363 = getelementptr inbounds %struct.anon addrspace(2)* %tmp362, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp364 = load float addrspace(2)* %tmp363      ; <float> [#uses=3]
  %cmp365 = fcmp oeq float 0.000000e+000, %tmp364 ; <i1> [#uses=1]
  %sel366 = select i1 %cmp365, float 1.000000e+000, float %tmp364 ; <float> [#uses=0]
  %div367 = fdiv float %tmp361, %tmp364           ; <float> [#uses=1]
  %vecinit368 = insertelement <2 x float> undef, float %div367, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit369 = insertelement <2 x float> %vecinit368, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit369, <2 x float>* %r2r1t
  %tmp370 = load <2 x float>* %r2r1t              ; <<2 x float>> [#uses=1]
  %tmp371 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %tmp372 = load <2 x float>* %I                  ; <<2 x float>> [#uses=1]
  %call373 = call <2 x float> @complexMult(<2 x float> %tmp371, <2 x float> %tmp372) ; <<2 x float>> [#uses=1]
  %call374 = call <2 x float> @complexExp(<2 x float> %call373) ; <<2 x float>> [#uses=1]
  %call375 = call <2 x float> @complexMult(<2 x float> %tmp370, <2 x float> %call374) ; <<2 x float>> [#uses=1]
  store <2 x float> %call375, <2 x float>* %ratio
  br label %if.end376

if.end376:                                        ; preds = %if.then357, %if.then352
  %tmp377 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp378 = getelementptr inbounds %struct.anon addrspace(2)* %tmp377, i32 0, i32 39 ; <i32 addrspace(2)*> [#uses=1]
  %tmp379 = load i32 addrspace(2)* %tmp378        ; <i32> [#uses=1]
  %tobool380 = icmp ne i32 %tmp379, 0             ; <i1> [#uses=1]
  br i1 %tobool380, label %if.then381, label %if.end400

if.then381:                                       ; preds = %if.end376
  %tmp383 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp384 = getelementptr inbounds %struct.anon addrspace(2)* %tmp383, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp385 = load float addrspace(2)* %tmp384      ; <float> [#uses=1]
  %tmp386 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp387 = getelementptr inbounds %struct.anon addrspace(2)* %tmp386, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp388 = load float addrspace(2)* %tmp387      ; <float> [#uses=3]
  %cmp389 = fcmp oeq float 0.000000e+000, %tmp388 ; <i1> [#uses=1]
  %sel390 = select i1 %cmp389, float 1.000000e+000, float %tmp388 ; <float> [#uses=0]
  %div391 = fdiv float %tmp385, %tmp388           ; <float> [#uses=1]
  %vecinit392 = insertelement <2 x float> undef, float %div391, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit393 = insertelement <2 x float> %vecinit392, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit393, <2 x float>* %r1r2t
  %tmp394 = load <2 x float>* %r1r2t              ; <<2 x float>> [#uses=1]
  %tmp395 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %tmp396 = load <2 x float>* %negI               ; <<2 x float>> [#uses=1]
  %call397 = call <2 x float> @complexMult(<2 x float> %tmp395, <2 x float> %tmp396) ; <<2 x float>> [#uses=1]
  %call398 = call <2 x float> @complexExp(<2 x float> %call397) ; <<2 x float>> [#uses=1]
  %call399 = call <2 x float> @complexMult(<2 x float> %tmp394, <2 x float> %call398) ; <<2 x float>> [#uses=1]
  store <2 x float> %call399, <2 x float>* %ratio
  br label %if.end400

if.end400:                                        ; preds = %if.then381, %if.end376
  %tmp401 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp402 = load <2 x float>* %ratio              ; <<2 x float>> [#uses=1]
  %tmp403 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp404 = getelementptr inbounds %struct.anon addrspace(2)* %tmp403, i32 0, i32 20 ; <i32 addrspace(2)*> [#uses=1]
  %tmp405 = load i32 addrspace(2)* %tmp404        ; <i32> [#uses=1]
  %call406 = call <2 x float> @power(<2 x float> %tmp402, i32 %tmp405) ; <<2 x float>> [#uses=1]
  %call407 = call <2 x float> @complexMult(<2 x float> %tmp401, <2 x float> %call406) ; <<2 x float>> [#uses=1]
  store <2 x float> %call407, <2 x float>* %z
  br label %if.end408

if.end408:                                        ; preds = %if.end400, %land.lhs.true, %if.end320
  %tmp410 = load <4 x float>* %kZero              ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp410, <4 x float>* %colorSoFar
  store i32 0, i32* %iteration
  %tmp411 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp412 = load <2 x float>* %outCrd.addr        ; <<2 x float>> [#uses=1]
  %tmp413 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp414 = load i32* %iteration                  ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp414, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %iteration
  %tmp415 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call416 = call <4 x float> @renderPixel(%struct._image2d_t* %tmp411, <2 x float> %tmp412, <2 x float> %tmp413, float* %alphaRemaining, i32* %sign, i32 %tmp414, <4 x float>* %colorSoFar, %struct.anon addrspace(2)* %tmp415) ; <<4 x float>> [#uses=0]
  %tmp417 = load i32* %sign                       ; <i32> [#uses=1]
  %cmp418 = icmp slt i32 %tmp417, 0               ; <i1> [#uses=1]
  br i1 %cmp418, label %if.then420, label %if.end440

if.then420:                                       ; preds = %if.end408
  %tmp423 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp424 = getelementptr inbounds %struct.anon addrspace(2)* %tmp423, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp425 = load float addrspace(2)* %tmp424      ; <float> [#uses=1]
  %tmp426 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp427 = getelementptr inbounds %struct.anon addrspace(2)* %tmp426, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp428 = load float addrspace(2)* %tmp427      ; <float> [#uses=3]
  %cmp429 = fcmp oeq float 0.000000e+000, %tmp428 ; <i1> [#uses=1]
  %sel430 = select i1 %cmp429, float 1.000000e+000, float %tmp428 ; <float> [#uses=0]
  %div431 = fdiv float %tmp425, %tmp428           ; <float> [#uses=1]
  %vecinit432 = insertelement <2 x float> undef, float %div431, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit433 = insertelement <2 x float> %vecinit432, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit433, <2 x float>* %r2r1t422
  %tmp434 = load <2 x float>* %r2r1t422           ; <<2 x float>> [#uses=1]
  %tmp435 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %tmp436 = load <2 x float>* %I                  ; <<2 x float>> [#uses=1]
  %call437 = call <2 x float> @complexMult(<2 x float> %tmp435, <2 x float> %tmp436) ; <<2 x float>> [#uses=1]
  %call438 = call <2 x float> @complexExp(<2 x float> %call437) ; <<2 x float>> [#uses=1]
  %call439 = call <2 x float> @complexMult(<2 x float> %tmp434, <2 x float> %call438) ; <<2 x float>> [#uses=1]
  store <2 x float> %call439, <2 x float>* %ratio
  br label %if.end440

if.end440:                                        ; preds = %if.then420, %if.end408
  %tmp441 = load i32* %sign                       ; <i32> [#uses=1]
  %cmp442 = icmp sgt i32 %tmp441, 0               ; <i1> [#uses=1]
  br i1 %cmp442, label %if.then444, label %if.end464

if.then444:                                       ; preds = %if.end440
  %tmp447 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp448 = getelementptr inbounds %struct.anon addrspace(2)* %tmp447, i32 0, i32 29 ; <float addrspace(2)*> [#uses=1]
  %tmp449 = load float addrspace(2)* %tmp448      ; <float> [#uses=1]
  %tmp450 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp451 = getelementptr inbounds %struct.anon addrspace(2)* %tmp450, i32 0, i32 30 ; <float addrspace(2)*> [#uses=1]
  %tmp452 = load float addrspace(2)* %tmp451      ; <float> [#uses=3]
  %cmp453 = fcmp oeq float 0.000000e+000, %tmp452 ; <i1> [#uses=1]
  %sel454 = select i1 %cmp453, float 1.000000e+000, float %tmp452 ; <float> [#uses=0]
  %div455 = fdiv float %tmp449, %tmp452           ; <float> [#uses=1]
  %vecinit456 = insertelement <2 x float> undef, float %div455, i32 0 ; <<2 x float>> [#uses=1]
  %vecinit457 = insertelement <2 x float> %vecinit456, float 0.000000e+000, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit457, <2 x float>* %r1r2t446
  %tmp458 = load <2 x float>* %r1r2t446           ; <<2 x float>> [#uses=1]
  %tmp459 = load <2 x float>* %angle              ; <<2 x float>> [#uses=1]
  %tmp460 = load <2 x float>* %negI               ; <<2 x float>> [#uses=1]
  %call461 = call <2 x float> @complexMult(<2 x float> %tmp459, <2 x float> %tmp460) ; <<2 x float>> [#uses=1]
  %call462 = call <2 x float> @complexExp(<2 x float> %call461) ; <<2 x float>> [#uses=1]
  %call463 = call <2 x float> @complexMult(<2 x float> %tmp458, <2 x float> %call462) ; <<2 x float>> [#uses=1]
  store <2 x float> %call463, <2 x float>* %ratio
  br label %if.end464

if.end464:                                        ; preds = %if.then444, %if.end440
  %tmp465 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp466 = getelementptr inbounds %struct.anon addrspace(2)* %tmp465, i32 0, i32 20 ; <i32 addrspace(2)*> [#uses=1]
  %tmp467 = load i32 addrspace(2)* %tmp466        ; <i32> [#uses=1]
  store i32 %tmp467, i32* %iteration
  %tmp469 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp470 = getelementptr inbounds %struct.anon addrspace(2)* %tmp469, i32 0, i32 19 ; <i32 addrspace(2)*> [#uses=1]
  %tmp471 = load i32 addrspace(2)* %tmp470        ; <i32> [#uses=1]
  %tmp472 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp473 = getelementptr inbounds %struct.anon addrspace(2)* %tmp472, i32 0, i32 20 ; <i32 addrspace(2)*> [#uses=1]
  %tmp474 = load i32 addrspace(2)* %tmp473        ; <i32> [#uses=1]
  %add475 = add nsw i32 %tmp471, %tmp474          ; <i32> [#uses=1]
  %sub476 = sub i32 %add475, 1                    ; <i32> [#uses=1]
  store i32 %sub476, i32* %maxIteration
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.end464
  %tmp477 = load i32* %sign                       ; <i32> [#uses=1]
  %cmp478 = icmp ne i32 %tmp477, 0                ; <i1> [#uses=1]
  br i1 %cmp478, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %tmp480 = load i32* %iteration                  ; <i32> [#uses=1]
  %tmp481 = load i32* %maxIteration               ; <i32> [#uses=1]
  %cmp482 = icmp slt i32 %tmp480, %tmp481         ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %0 = phi i1 [ false, %while.cond ], [ %cmp482, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %tmp484 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp485 = load <2 x float>* %ratio              ; <<2 x float>> [#uses=1]
  %call486 = call <2 x float> @complexMult(<2 x float> %tmp484, <2 x float> %tmp485) ; <<2 x float>> [#uses=1]
  store <2 x float> %call486, <2 x float>* %z
  %tmp487 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp488 = load <2 x float>* %outCrd.addr        ; <<2 x float>> [#uses=1]
  %tmp489 = load <2 x float>* %z                  ; <<2 x float>> [#uses=1]
  %tmp490 = load i32* %iteration                  ; <i32> [#uses=2]
  %inc491 = add nsw i32 %tmp490, 1                ; <i32> [#uses=1]
  store i32 %inc491, i32* %iteration
  %tmp492 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call493 = call <4 x float> @renderPixel(%struct._image2d_t* %tmp487, <2 x float> %tmp488, <2 x float> %tmp489, float* %alphaRemaining, i32* %sign, i32 %tmp490, <4 x float>* %colorSoFar, %struct.anon addrspace(2)* %tmp492) ; <<4 x float>> [#uses=0]
  br label %while.cond

while.end:                                        ; preds = %land.end
  %tmp494 = load <4 x float>* %colorSoFar         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp494, <4 x float>* %retval
  %1 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %1
}

declare float @_Z4atanf(float)

; CHECK: ret
define void @droste2D(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %gid0_col = alloca i32, align 4                 ; <i32*> [#uses=3]
  %gid1_row = alloca i32, align 4                 ; <i32*> [#uses=3]
  %imgWidth = alloca i32, align 4                 ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_col
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid1_row
  %call2 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call2, i32* %imgWidth
  %tmp = load i32* %gid1_row                      ; <i32> [#uses=1]
  %tmp3 = load i32* %imgWidth                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp3                      ; <i32> [#uses=1]
  %tmp4 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp4                  ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp6 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp6 to float               ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv, i32 0 ; <<2 x float>> [#uses=1]
  %tmp7 = load i32* %gid1_row                     ; <i32> [#uses=1]
  %conv8 = sitofp i32 %tmp7 to float              ; <float> [#uses=1]
  %vecinit9 = insertelement <2 x float> %vecinit, float %conv8, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit9, <2 x float>* %curCrd
  %tmp10 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp11 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call13 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp10, <2 x float> %tmp11, %struct.anon addrspace(2)* %tmp12) ; <<4 x float>> [#uses=1]
  %tmp14 = load i32* %index                       ; <i32> [#uses=1]
  %tmp15 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp15, i32 %tmp14 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call13, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

; CHECK: ret
define void @droste(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=5]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=1]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=7]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <4 x float> zeroinitializer, <4 x float>* %kZero
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load i32* %row                          ; <i32> [#uses=1]
  %tmp4 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  %tmp5 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp6 = getelementptr inbounds %struct.anon addrspace(2)* %tmp5, i32 0, i32 11 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp7 = load <2 x i32> addrspace(2)* %tmp6      ; <<2 x i32>> [#uses=1]
  %tmp8 = extractelement <2 x i32> %tmp7, i32 1   ; <i32> [#uses=1]
  %call9 = call i32 @_Z3minjj(i32 %add, i32 %tmp8) ; <i32> [#uses=1]
  store i32 %call9, i32* %lastRow
  %tmp11 = load i32* %row                         ; <i32> [#uses=1]
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 11 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp14 = load <2 x i32> addrspace(2)* %tmp13    ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 0 ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp11, %tmp15                 ; <i32> [#uses=1]
  store i32 %mul16, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc45, %entry
  %tmp17 = load i32* %row                         ; <i32> [#uses=1]
  %tmp18 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp17, %tmp18              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end48

for.body:                                         ; preds = %for.cond
  %tmp20 = load i32* %row                         ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp20 to float              ; <float> [#uses=1]
  %tmp21 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp22 = insertelement <2 x float> %tmp21, float %conv, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp22, <2 x float>* %curCrd
  store i32 0, i32* %col
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc, %for.body
  %tmp25 = load i32* %col                         ; <i32> [#uses=1]
  %tmp26 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp27 = getelementptr inbounds %struct.anon addrspace(2)* %tmp26, i32 0, i32 11 ; <<2 x i32> addrspace(2)*> [#uses=1]
  %tmp28 = load <2 x i32> addrspace(2)* %tmp27    ; <<2 x i32>> [#uses=1]
  %tmp29 = extractelement <2 x i32> %tmp28, i32 0 ; <i32> [#uses=1]
  %cmp30 = icmp ult i32 %tmp25, %tmp29            ; <i1> [#uses=1]
  br i1 %cmp30, label %for.body32, label %for.end

for.body32:                                       ; preds = %for.cond24
  %tmp33 = load i32* %col                         ; <i32> [#uses=1]
  %conv34 = uitofp i32 %tmp33 to float            ; <float> [#uses=1]
  %tmp35 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp36 = insertelement <2 x float> %tmp35, float %conv34, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp36, <2 x float>* %curCrd
  %tmp37 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp38 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp39 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call40 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp37, <2 x float> %tmp38, %struct.anon addrspace(2)* %tmp39) ; <<4 x float>> [#uses=1]
  %tmp41 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add i32 %tmp41, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp42 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp42, i32 %tmp41 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call40, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body32
  %tmp43 = load i32* %col                         ; <i32> [#uses=1]
  %inc44 = add i32 %tmp43, 1                      ; <i32> [#uses=1]
  store i32 %inc44, i32* %col
  br label %for.cond24

for.end:                                          ; preds = %for.cond24
  br label %for.inc45

for.inc45:                                        ; preds = %for.end
  %tmp46 = load i32* %row                         ; <i32> [#uses=1]
  %inc47 = add i32 %tmp46, 1                      ; <i32> [#uses=1]
  store i32 %inc47, i32* %row
  br label %for.cond

for.end48:                                        ; preds = %for.cond
  ret void
}

declare i32 @_Z3minjj(i32, i32)
