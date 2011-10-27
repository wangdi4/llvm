; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIMonteCarlo.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_calPriceVega_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_calPriceVega_parameters = appending global [184 x i8] c"float4 __attribute__((address_space(1))) *, int, int, uint4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[184 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, i32, i32, <4 x i32> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @calPriceVega to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_calPriceVega_locals to i8*), i8* getelementptr inbounds ([184 x i8]* @opencl_calPriceVega_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @lshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
  %input.addr = alloca <4 x i32>, align 16        ; <<4 x i32>*> [#uses=8]
  %shift.addr = alloca i32, align 4               ; <i32*> [#uses=6]
  %output.addr = alloca <4 x i32>*, align 4       ; <<4 x i32>**> [#uses=2]
  %invshift = alloca i32, align 4                 ; <i32*> [#uses=4]
  %temp = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=9]
  store <4 x i32> %input, <4 x i32>* %input.addr
  store i32 %shift, i32* %shift.addr
  store <4 x i32>* %output, <4 x i32>** %output.addr
  %tmp = load i32* %shift.addr                    ; <i32> [#uses=1]
  %sub = sub i32 32, %tmp                         ; <i32> [#uses=1]
  store i32 %sub, i32* %invshift
  %tmp2 = load <4 x i32>* %input.addr             ; <<4 x i32>> [#uses=1]
  %tmp3 = extractelement <4 x i32> %tmp2, i32 0   ; <i32> [#uses=1]
  %tmp4 = load i32* %shift.addr                   ; <i32> [#uses=1]
  %shl = shl i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  %tmp5 = load <4 x i32>* %temp                   ; <<4 x i32>> [#uses=1]
  %tmp6 = insertelement <4 x i32> %tmp5, i32 %shl, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp6, <4 x i32>* %temp
  %tmp7 = load <4 x i32>* %input.addr             ; <<4 x i32>> [#uses=1]
  %tmp8 = extractelement <4 x i32> %tmp7, i32 1   ; <i32> [#uses=1]
  %tmp9 = load i32* %shift.addr                   ; <i32> [#uses=1]
  %shl10 = shl i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  %tmp11 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp12 = extractelement <4 x i32> %tmp11, i32 0 ; <i32> [#uses=1]
  %tmp13 = load i32* %invshift                    ; <i32> [#uses=1]
  %shr = lshr i32 %tmp12, %tmp13                  ; <i32> [#uses=1]
  %or = or i32 %shl10, %shr                       ; <i32> [#uses=1]
  %tmp14 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp15 = insertelement <4 x i32> %tmp14, i32 %or, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp15, <4 x i32>* %temp
  %tmp16 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp17 = extractelement <4 x i32> %tmp16, i32 2 ; <i32> [#uses=1]
  %tmp18 = load i32* %shift.addr                  ; <i32> [#uses=1]
  %shl19 = shl i32 %tmp17, %tmp18                 ; <i32> [#uses=1]
  %tmp20 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp21 = extractelement <4 x i32> %tmp20, i32 1 ; <i32> [#uses=1]
  %tmp22 = load i32* %invshift                    ; <i32> [#uses=1]
  %shr23 = lshr i32 %tmp21, %tmp22                ; <i32> [#uses=1]
  %or24 = or i32 %shl19, %shr23                   ; <i32> [#uses=1]
  %tmp25 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp26 = insertelement <4 x i32> %tmp25, i32 %or24, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp26, <4 x i32>* %temp
  %tmp27 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp28 = extractelement <4 x i32> %tmp27, i32 3 ; <i32> [#uses=1]
  %tmp29 = load i32* %shift.addr                  ; <i32> [#uses=1]
  %shl30 = shl i32 %tmp28, %tmp29                 ; <i32> [#uses=1]
  %tmp31 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp32 = extractelement <4 x i32> %tmp31, i32 2 ; <i32> [#uses=1]
  %tmp33 = load i32* %invshift                    ; <i32> [#uses=1]
  %shr34 = lshr i32 %tmp32, %tmp33                ; <i32> [#uses=1]
  %or35 = or i32 %shl30, %shr34                   ; <i32> [#uses=1]
  %tmp36 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp37 = insertelement <4 x i32> %tmp36, i32 %or35, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp37, <4 x i32>* %temp
  %tmp38 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp39 = load <4 x i32>** %output.addr          ; <<4 x i32>*> [#uses=1]
  store <4 x i32> %tmp38, <4 x i32>* %tmp39
  ret void
}

; CHECK: ret
define void @rshift128(<4 x i32> %input, i32 %shift, <4 x i32>* %output) nounwind {
entry:
  %input.addr = alloca <4 x i32>, align 16        ; <<4 x i32>*> [#uses=8]
  %shift.addr = alloca i32, align 4               ; <i32*> [#uses=6]
  %output.addr = alloca <4 x i32>*, align 4       ; <<4 x i32>**> [#uses=2]
  %invshift = alloca i32, align 4                 ; <i32*> [#uses=4]
  %temp = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=9]
  store <4 x i32> %input, <4 x i32>* %input.addr
  store i32 %shift, i32* %shift.addr
  store <4 x i32>* %output, <4 x i32>** %output.addr
  %tmp = load i32* %shift.addr                    ; <i32> [#uses=1]
  %sub = sub i32 32, %tmp                         ; <i32> [#uses=1]
  store i32 %sub, i32* %invshift
  %tmp2 = load <4 x i32>* %input.addr             ; <<4 x i32>> [#uses=1]
  %tmp3 = extractelement <4 x i32> %tmp2, i32 3   ; <i32> [#uses=1]
  %tmp4 = load i32* %shift.addr                   ; <i32> [#uses=1]
  %shr = lshr i32 %tmp3, %tmp4                    ; <i32> [#uses=1]
  %tmp5 = load <4 x i32>* %temp                   ; <<4 x i32>> [#uses=1]
  %tmp6 = insertelement <4 x i32> %tmp5, i32 %shr, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp6, <4 x i32>* %temp
  %tmp7 = load <4 x i32>* %input.addr             ; <<4 x i32>> [#uses=1]
  %tmp8 = extractelement <4 x i32> %tmp7, i32 2   ; <i32> [#uses=1]
  %tmp9 = load i32* %shift.addr                   ; <i32> [#uses=1]
  %shr10 = lshr i32 %tmp8, %tmp9                  ; <i32> [#uses=1]
  %tmp11 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp12 = extractelement <4 x i32> %tmp11, i32 3 ; <i32> [#uses=1]
  %tmp13 = load i32* %invshift                    ; <i32> [#uses=1]
  %shl = shl i32 %tmp12, %tmp13                   ; <i32> [#uses=1]
  %or = or i32 %shr10, %shl                       ; <i32> [#uses=1]
  %tmp14 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp15 = insertelement <4 x i32> %tmp14, i32 %or, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp15, <4 x i32>* %temp
  %tmp16 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp17 = extractelement <4 x i32> %tmp16, i32 1 ; <i32> [#uses=1]
  %tmp18 = load i32* %shift.addr                  ; <i32> [#uses=1]
  %shr19 = lshr i32 %tmp17, %tmp18                ; <i32> [#uses=1]
  %tmp20 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp21 = extractelement <4 x i32> %tmp20, i32 2 ; <i32> [#uses=1]
  %tmp22 = load i32* %invshift                    ; <i32> [#uses=1]
  %shl23 = shl i32 %tmp21, %tmp22                 ; <i32> [#uses=1]
  %or24 = or i32 %shr19, %shl23                   ; <i32> [#uses=1]
  %tmp25 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp26 = insertelement <4 x i32> %tmp25, i32 %or24, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp26, <4 x i32>* %temp
  %tmp27 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp28 = extractelement <4 x i32> %tmp27, i32 0 ; <i32> [#uses=1]
  %tmp29 = load i32* %shift.addr                  ; <i32> [#uses=1]
  %shr30 = lshr i32 %tmp28, %tmp29                ; <i32> [#uses=1]
  %tmp31 = load <4 x i32>* %input.addr            ; <<4 x i32>> [#uses=1]
  %tmp32 = extractelement <4 x i32> %tmp31, i32 1 ; <i32> [#uses=1]
  %tmp33 = load i32* %invshift                    ; <i32> [#uses=1]
  %shl34 = shl i32 %tmp32, %tmp33                 ; <i32> [#uses=1]
  %or35 = or i32 %shr30, %shl34                   ; <i32> [#uses=1]
  %tmp36 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp37 = insertelement <4 x i32> %tmp36, i32 %or35, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp37, <4 x i32>* %temp
  %tmp38 = load <4 x i32>* %temp                  ; <<4 x i32>> [#uses=1]
  %tmp39 = load <4 x i32>** %output.addr          ; <<4 x i32>*> [#uses=1]
  store <4 x i32> %tmp38, <4 x i32>* %tmp39
  ret void
}

; CHECK: ret
define void @generateRand(<4 x i32> %seed, <4 x float>* %gaussianRand1, <4 x float>* %gaussianRand2, <4 x i32>* %nextRand) nounwind {
entry:
  %seed.addr = alloca <4 x i32>, align 16         ; <<4 x i32>*> [#uses=2]
  %gaussianRand1.addr = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %gaussianRand2.addr = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %nextRand.addr = alloca <4 x i32>*, align 4     ; <<4 x i32>**> [#uses=2]
  %mulFactor = alloca i32, align 4                ; <i32*> [#uses=2]
  %temp = alloca [8 x <4 x i32>], align 16        ; <[8 x <4 x i32>]*> [#uses=16]
  %state1 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=5]
  %state2 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=5]
  %.compoundliteral = alloca <4 x i32>, align 16  ; <<4 x i32>*> [#uses=2]
  %state3 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=6]
  %.compoundliteral4 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %state4 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=7]
  %.compoundliteral7 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %state5 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=4]
  %.compoundliteral10 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %stateMask = alloca i32, align 4                ; <i32*> [#uses=5]
  %thirty = alloca i32, align 4                   ; <i32*> [#uses=5]
  %mask4 = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=5]
  %.compoundliteral15 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %thirty4 = alloca <4 x i32>, align 16           ; <<4 x i32>*> [#uses=5]
  %.compoundliteral25 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %one4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %.compoundliteral36 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %two4 = alloca <4 x i32>, align 16              ; <<4 x i32>*> [#uses=2]
  %.compoundliteral39 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %three4 = alloca <4 x i32>, align 16            ; <<4 x i32>*> [#uses=2]
  %.compoundliteral42 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %four4 = alloca <4 x i32>, align 16             ; <<4 x i32>*> [#uses=2]
  %.compoundliteral45 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %r1 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=6]
  %.compoundliteral48 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %r2 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=12]
  %.compoundliteral51 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %a = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=10]
  %.compoundliteral54 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %b = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=9]
  %.compoundliteral57 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %e = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=6]
  %.compoundliteral60 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %f = alloca <4 x i32>, align 16                 ; <<4 x i32>*> [#uses=6]
  %.compoundliteral63 = alloca <4 x i32>, align 16 ; <<4 x i32>*> [#uses=2]
  %thirteen = alloca i32, align 4                 ; <i32*> [#uses=5]
  %fifteen = alloca i32, align 4                  ; <i32*> [#uses=5]
  %shift = alloca i32, align 4                    ; <i32*> [#uses=3]
  %mask11 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask12 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask13 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %mask14 = alloca i32, align 4                   ; <i32*> [#uses=2]
  %one = alloca float, align 4                    ; <float*> [#uses=3]
  %intMax = alloca float, align 4                 ; <float*> [#uses=3]
  %PI = alloca float, align 4                     ; <float*> [#uses=2]
  %two = alloca float, align 4                    ; <float*> [#uses=3]
  %r = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %phi = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %temp1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %temp2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=10]
  %tmpValue1 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %tmpValue2 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  store <4 x i32> %seed, <4 x i32>* %seed.addr
  store <4 x float>* %gaussianRand1, <4 x float>** %gaussianRand1.addr
  store <4 x float>* %gaussianRand2, <4 x float>** %gaussianRand2.addr
  store <4 x i32>* %nextRand, <4 x i32>** %nextRand.addr
  store i32 4, i32* %mulFactor
  %tmp = load <4 x i32>* %seed.addr               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp, <4 x i32>* %state1
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral
  %tmp2 = load <4 x i32>* %.compoundliteral       ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp2, <4 x i32>* %state2
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral4
  %tmp5 = load <4 x i32>* %.compoundliteral4      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp5, <4 x i32>* %state3
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral7
  %tmp8 = load <4 x i32>* %.compoundliteral7      ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp8, <4 x i32>* %state4
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral10
  %tmp11 = load <4 x i32>* %.compoundliteral10    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp11, <4 x i32>* %state5
  store i32 1812433253, i32* %stateMask
  store i32 30, i32* %thirty
  %tmp16 = load i32* %stateMask                   ; <i32> [#uses=1]
  %vecinit = insertelement <4 x i32> undef, i32 %tmp16, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp17 = load i32* %stateMask                   ; <i32> [#uses=1]
  %vecinit18 = insertelement <4 x i32> %vecinit, i32 %tmp17, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp19 = load i32* %stateMask                   ; <i32> [#uses=1]
  %vecinit20 = insertelement <4 x i32> %vecinit18, i32 %tmp19, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp21 = load i32* %stateMask                   ; <i32> [#uses=1]
  %vecinit22 = insertelement <4 x i32> %vecinit20, i32 %tmp21, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit22, <4 x i32>* %.compoundliteral15
  %tmp23 = load <4 x i32>* %.compoundliteral15    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp23, <4 x i32>* %mask4
  %tmp26 = load i32* %thirty                      ; <i32> [#uses=1]
  %vecinit27 = insertelement <4 x i32> undef, i32 %tmp26, i32 0 ; <<4 x i32>> [#uses=1]
  %tmp28 = load i32* %thirty                      ; <i32> [#uses=1]
  %vecinit29 = insertelement <4 x i32> %vecinit27, i32 %tmp28, i32 1 ; <<4 x i32>> [#uses=1]
  %tmp30 = load i32* %thirty                      ; <i32> [#uses=1]
  %vecinit31 = insertelement <4 x i32> %vecinit29, i32 %tmp30, i32 2 ; <<4 x i32>> [#uses=1]
  %tmp32 = load i32* %thirty                      ; <i32> [#uses=1]
  %vecinit33 = insertelement <4 x i32> %vecinit31, i32 %tmp32, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %vecinit33, <4 x i32>* %.compoundliteral25
  %tmp34 = load <4 x i32>* %.compoundliteral25    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp34, <4 x i32>* %thirty4
  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32>* %.compoundliteral36
  %tmp37 = load <4 x i32>* %.compoundliteral36    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp37, <4 x i32>* %one4
  store <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32>* %.compoundliteral39
  %tmp40 = load <4 x i32>* %.compoundliteral39    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp40, <4 x i32>* %two4
  store <4 x i32> <i32 3, i32 3, i32 3, i32 3>, <4 x i32>* %.compoundliteral42
  %tmp43 = load <4 x i32>* %.compoundliteral42    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp43, <4 x i32>* %three4
  store <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32>* %.compoundliteral45
  %tmp46 = load <4 x i32>* %.compoundliteral45    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp46, <4 x i32>* %four4
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral48
  %tmp49 = load <4 x i32>* %.compoundliteral48    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp49, <4 x i32>* %r1
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral51
  %tmp52 = load <4 x i32>* %.compoundliteral51    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp52, <4 x i32>* %r2
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral54
  %tmp55 = load <4 x i32>* %.compoundliteral54    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp55, <4 x i32>* %a
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral57
  %tmp58 = load <4 x i32>* %.compoundliteral57    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp58, <4 x i32>* %b
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral60
  %tmp61 = load <4 x i32>* %.compoundliteral60    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp61, <4 x i32>* %e
  store <4 x i32> zeroinitializer, <4 x i32>* %.compoundliteral63
  %tmp64 = load <4 x i32>* %.compoundliteral63    ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp64, <4 x i32>* %f
  store i32 13, i32* %thirteen
  store i32 15, i32* %fifteen
  store i32 24, i32* %shift
  store i32 -33605633, i32* %mask11
  store i32 -276873347, i32* %mask12
  store i32 -8946819, i32* %mask13
  store i32 2146958127, i32* %mask14
  store float 1.000000e+000, float* %one
  store float 0x41F0000000000000, float* %intMax
  store float 0x400921FB60000000, float* %PI
  store float 2.000000e+000, float* %two
  %tmp80 = load <4 x i32>* %mask4                 ; <<4 x i32>> [#uses=1]
  %tmp81 = load <4 x i32>* %state1                ; <<4 x i32>> [#uses=1]
  %tmp82 = load <4 x i32>* %state1                ; <<4 x i32>> [#uses=1]
  %tmp83 = load <4 x i32>* %thirty4               ; <<4 x i32>> [#uses=1]
  %shr = lshr <4 x i32> %tmp82, %tmp83            ; <<4 x i32>> [#uses=1]
  %xor = xor <4 x i32> %tmp81, %shr               ; <<4 x i32>> [#uses=1]
  %mul = mul <4 x i32> %tmp80, %xor               ; <<4 x i32>> [#uses=1]
  %tmp84 = load <4 x i32>* %one4                  ; <<4 x i32>> [#uses=1]
  %add = add <4 x i32> %mul, %tmp84               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add, <4 x i32>* %state2
  %tmp85 = load <4 x i32>* %mask4                 ; <<4 x i32>> [#uses=1]
  %tmp86 = load <4 x i32>* %state2                ; <<4 x i32>> [#uses=1]
  %tmp87 = load <4 x i32>* %state2                ; <<4 x i32>> [#uses=1]
  %tmp88 = load <4 x i32>* %thirty4               ; <<4 x i32>> [#uses=1]
  %shr89 = lshr <4 x i32> %tmp87, %tmp88          ; <<4 x i32>> [#uses=1]
  %xor90 = xor <4 x i32> %tmp86, %shr89           ; <<4 x i32>> [#uses=1]
  %mul91 = mul <4 x i32> %tmp85, %xor90           ; <<4 x i32>> [#uses=1]
  %tmp92 = load <4 x i32>* %two4                  ; <<4 x i32>> [#uses=1]
  %add93 = add <4 x i32> %mul91, %tmp92           ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add93, <4 x i32>* %state3
  %tmp94 = load <4 x i32>* %mask4                 ; <<4 x i32>> [#uses=1]
  %tmp95 = load <4 x i32>* %state3                ; <<4 x i32>> [#uses=1]
  %tmp96 = load <4 x i32>* %state3                ; <<4 x i32>> [#uses=1]
  %tmp97 = load <4 x i32>* %thirty4               ; <<4 x i32>> [#uses=1]
  %shr98 = lshr <4 x i32> %tmp96, %tmp97          ; <<4 x i32>> [#uses=1]
  %xor99 = xor <4 x i32> %tmp95, %shr98           ; <<4 x i32>> [#uses=1]
  %mul100 = mul <4 x i32> %tmp94, %xor99          ; <<4 x i32>> [#uses=1]
  %tmp101 = load <4 x i32>* %three4               ; <<4 x i32>> [#uses=1]
  %add102 = add <4 x i32> %mul100, %tmp101        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add102, <4 x i32>* %state4
  %tmp103 = load <4 x i32>* %mask4                ; <<4 x i32>> [#uses=1]
  %tmp104 = load <4 x i32>* %state4               ; <<4 x i32>> [#uses=1]
  %tmp105 = load <4 x i32>* %state4               ; <<4 x i32>> [#uses=1]
  %tmp106 = load <4 x i32>* %thirty4              ; <<4 x i32>> [#uses=1]
  %shr107 = lshr <4 x i32> %tmp105, %tmp106       ; <<4 x i32>> [#uses=1]
  %xor108 = xor <4 x i32> %tmp104, %shr107        ; <<4 x i32>> [#uses=1]
  %mul109 = mul <4 x i32> %tmp103, %xor108        ; <<4 x i32>> [#uses=1]
  %tmp110 = load <4 x i32>* %four4                ; <<4 x i32>> [#uses=1]
  %add111 = add <4 x i32> %mul109, %tmp110        ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add111, <4 x i32>* %state5
  store i32 0, i32* %i
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp113 = load i32* %i                          ; <i32> [#uses=1]
  %tmp114 = load i32* %mulFactor                  ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp113, %tmp114            ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp115 = load i32* %i                          ; <i32> [#uses=1]
  switch i32 %tmp115, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb120
    i32 2, label %sw.bb125
    i32 3, label %sw.bb132
  ]

sw.bb:                                            ; preds = %for.body
  %tmp116 = load <4 x i32>* %state4               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp116, <4 x i32>* %r1
  %tmp117 = load <4 x i32>* %state5               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp117, <4 x i32>* %r2
  %tmp118 = load <4 x i32>* %state1               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp118, <4 x i32>* %a
  %tmp119 = load <4 x i32>* %state3               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp119, <4 x i32>* %b
  br label %sw.epilog

sw.bb120:                                         ; preds = %for.body
  %tmp121 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp121, <4 x i32>* %r1
  %arraydecay = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i32>* %arraydecay, i32 0 ; <<4 x i32>*> [#uses=1]
  %tmp122 = load <4 x i32>* %arrayidx             ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp122, <4 x i32>* %r2
  %tmp123 = load <4 x i32>* %state2               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp123, <4 x i32>* %a
  %tmp124 = load <4 x i32>* %state4               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp124, <4 x i32>* %b
  br label %sw.epilog

sw.bb125:                                         ; preds = %for.body
  %tmp126 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp126, <4 x i32>* %r1
  %arraydecay127 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx128 = getelementptr inbounds <4 x i32>* %arraydecay127, i32 1 ; <<4 x i32>*> [#uses=1]
  %tmp129 = load <4 x i32>* %arrayidx128          ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp129, <4 x i32>* %r2
  %tmp130 = load <4 x i32>* %state3               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp130, <4 x i32>* %a
  %tmp131 = load <4 x i32>* %state5               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp131, <4 x i32>* %b
  br label %sw.epilog

sw.bb132:                                         ; preds = %for.body
  %tmp133 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp133, <4 x i32>* %r1
  %arraydecay134 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx135 = getelementptr inbounds <4 x i32>* %arraydecay134, i32 2 ; <<4 x i32>*> [#uses=1]
  %tmp136 = load <4 x i32>* %arrayidx135          ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp136, <4 x i32>* %r2
  %tmp137 = load <4 x i32>* %state4               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp137, <4 x i32>* %a
  %tmp138 = load <4 x i32>* %state1               ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp138, <4 x i32>* %b
  br label %sw.epilog

sw.default:                                       ; preds = %for.body
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb132, %sw.bb125, %sw.bb120, %sw.bb
  %tmp139 = load <4 x i32>* %a                    ; <<4 x i32>> [#uses=1]
  %tmp140 = load i32* %shift                      ; <i32> [#uses=1]
  call void @lshift128(<4 x i32> %tmp139, i32 %tmp140, <4 x i32>* %e)
  %tmp141 = load <4 x i32>* %r1                   ; <<4 x i32>> [#uses=1]
  %tmp142 = load i32* %shift                      ; <i32> [#uses=1]
  call void @rshift128(<4 x i32> %tmp141, i32 %tmp142, <4 x i32>* %f)
  %tmp143 = load <4 x i32>* %a                    ; <<4 x i32>> [#uses=1]
  %tmp144 = extractelement <4 x i32> %tmp143, i32 0 ; <i32> [#uses=1]
  %tmp145 = load <4 x i32>* %e                    ; <<4 x i32>> [#uses=1]
  %tmp146 = extractelement <4 x i32> %tmp145, i32 0 ; <i32> [#uses=1]
  %xor147 = xor i32 %tmp144, %tmp146              ; <i32> [#uses=1]
  %tmp148 = load <4 x i32>* %b                    ; <<4 x i32>> [#uses=1]
  %tmp149 = extractelement <4 x i32> %tmp148, i32 0 ; <i32> [#uses=1]
  %tmp150 = load i32* %thirteen                   ; <i32> [#uses=1]
  %shr151 = lshr i32 %tmp149, %tmp150             ; <i32> [#uses=1]
  %tmp152 = load i32* %mask11                     ; <i32> [#uses=1]
  %and = and i32 %shr151, %tmp152                 ; <i32> [#uses=1]
  %xor153 = xor i32 %xor147, %and                 ; <i32> [#uses=1]
  %tmp154 = load <4 x i32>* %f                    ; <<4 x i32>> [#uses=1]
  %tmp155 = extractelement <4 x i32> %tmp154, i32 0 ; <i32> [#uses=1]
  %xor156 = xor i32 %xor153, %tmp155              ; <i32> [#uses=1]
  %tmp157 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  %tmp158 = extractelement <4 x i32> %tmp157, i32 0 ; <i32> [#uses=1]
  %tmp159 = load i32* %fifteen                    ; <i32> [#uses=1]
  %shl = shl i32 %tmp158, %tmp159                 ; <i32> [#uses=1]
  %xor160 = xor i32 %xor156, %shl                 ; <i32> [#uses=1]
  %tmp161 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay162 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx163 = getelementptr inbounds <4 x i32>* %arraydecay162, i32 %tmp161 ; <<4 x i32>*> [#uses=2]
  %tmp164 = load <4 x i32>* %arrayidx163          ; <<4 x i32>> [#uses=1]
  %tmp165 = insertelement <4 x i32> %tmp164, i32 %xor160, i32 0 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp165, <4 x i32>* %arrayidx163
  %tmp166 = load <4 x i32>* %a                    ; <<4 x i32>> [#uses=1]
  %tmp167 = extractelement <4 x i32> %tmp166, i32 1 ; <i32> [#uses=1]
  %tmp168 = load <4 x i32>* %e                    ; <<4 x i32>> [#uses=1]
  %tmp169 = extractelement <4 x i32> %tmp168, i32 1 ; <i32> [#uses=1]
  %xor170 = xor i32 %tmp167, %tmp169              ; <i32> [#uses=1]
  %tmp171 = load <4 x i32>* %b                    ; <<4 x i32>> [#uses=1]
  %tmp172 = extractelement <4 x i32> %tmp171, i32 1 ; <i32> [#uses=1]
  %tmp173 = load i32* %thirteen                   ; <i32> [#uses=1]
  %shr174 = lshr i32 %tmp172, %tmp173             ; <i32> [#uses=1]
  %tmp175 = load i32* %mask12                     ; <i32> [#uses=1]
  %and176 = and i32 %shr174, %tmp175              ; <i32> [#uses=1]
  %xor177 = xor i32 %xor170, %and176              ; <i32> [#uses=1]
  %tmp178 = load <4 x i32>* %f                    ; <<4 x i32>> [#uses=1]
  %tmp179 = extractelement <4 x i32> %tmp178, i32 1 ; <i32> [#uses=1]
  %xor180 = xor i32 %xor177, %tmp179              ; <i32> [#uses=1]
  %tmp181 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  %tmp182 = extractelement <4 x i32> %tmp181, i32 1 ; <i32> [#uses=1]
  %tmp183 = load i32* %fifteen                    ; <i32> [#uses=1]
  %shl184 = shl i32 %tmp182, %tmp183              ; <i32> [#uses=1]
  %xor185 = xor i32 %xor180, %shl184              ; <i32> [#uses=1]
  %tmp186 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay187 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx188 = getelementptr inbounds <4 x i32>* %arraydecay187, i32 %tmp186 ; <<4 x i32>*> [#uses=2]
  %tmp189 = load <4 x i32>* %arrayidx188          ; <<4 x i32>> [#uses=1]
  %tmp190 = insertelement <4 x i32> %tmp189, i32 %xor185, i32 1 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp190, <4 x i32>* %arrayidx188
  %tmp191 = load <4 x i32>* %a                    ; <<4 x i32>> [#uses=1]
  %tmp192 = extractelement <4 x i32> %tmp191, i32 2 ; <i32> [#uses=1]
  %tmp193 = load <4 x i32>* %e                    ; <<4 x i32>> [#uses=1]
  %tmp194 = extractelement <4 x i32> %tmp193, i32 2 ; <i32> [#uses=1]
  %xor195 = xor i32 %tmp192, %tmp194              ; <i32> [#uses=1]
  %tmp196 = load <4 x i32>* %b                    ; <<4 x i32>> [#uses=1]
  %tmp197 = extractelement <4 x i32> %tmp196, i32 2 ; <i32> [#uses=1]
  %tmp198 = load i32* %thirteen                   ; <i32> [#uses=1]
  %shr199 = lshr i32 %tmp197, %tmp198             ; <i32> [#uses=1]
  %tmp200 = load i32* %mask13                     ; <i32> [#uses=1]
  %and201 = and i32 %shr199, %tmp200              ; <i32> [#uses=1]
  %xor202 = xor i32 %xor195, %and201              ; <i32> [#uses=1]
  %tmp203 = load <4 x i32>* %f                    ; <<4 x i32>> [#uses=1]
  %tmp204 = extractelement <4 x i32> %tmp203, i32 2 ; <i32> [#uses=1]
  %xor205 = xor i32 %xor202, %tmp204              ; <i32> [#uses=1]
  %tmp206 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  %tmp207 = extractelement <4 x i32> %tmp206, i32 2 ; <i32> [#uses=1]
  %tmp208 = load i32* %fifteen                    ; <i32> [#uses=1]
  %shl209 = shl i32 %tmp207, %tmp208              ; <i32> [#uses=1]
  %xor210 = xor i32 %xor205, %shl209              ; <i32> [#uses=1]
  %tmp211 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay212 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx213 = getelementptr inbounds <4 x i32>* %arraydecay212, i32 %tmp211 ; <<4 x i32>*> [#uses=2]
  %tmp214 = load <4 x i32>* %arrayidx213          ; <<4 x i32>> [#uses=1]
  %tmp215 = insertelement <4 x i32> %tmp214, i32 %xor210, i32 2 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp215, <4 x i32>* %arrayidx213
  %tmp216 = load <4 x i32>* %a                    ; <<4 x i32>> [#uses=1]
  %tmp217 = extractelement <4 x i32> %tmp216, i32 3 ; <i32> [#uses=1]
  %tmp218 = load <4 x i32>* %e                    ; <<4 x i32>> [#uses=1]
  %tmp219 = extractelement <4 x i32> %tmp218, i32 3 ; <i32> [#uses=1]
  %xor220 = xor i32 %tmp217, %tmp219              ; <i32> [#uses=1]
  %tmp221 = load <4 x i32>* %b                    ; <<4 x i32>> [#uses=1]
  %tmp222 = extractelement <4 x i32> %tmp221, i32 3 ; <i32> [#uses=1]
  %tmp223 = load i32* %thirteen                   ; <i32> [#uses=1]
  %shr224 = lshr i32 %tmp222, %tmp223             ; <i32> [#uses=1]
  %tmp225 = load i32* %mask14                     ; <i32> [#uses=1]
  %and226 = and i32 %shr224, %tmp225              ; <i32> [#uses=1]
  %xor227 = xor i32 %xor220, %and226              ; <i32> [#uses=1]
  %tmp228 = load <4 x i32>* %f                    ; <<4 x i32>> [#uses=1]
  %tmp229 = extractelement <4 x i32> %tmp228, i32 3 ; <i32> [#uses=1]
  %xor230 = xor i32 %xor227, %tmp229              ; <i32> [#uses=1]
  %tmp231 = load <4 x i32>* %r2                   ; <<4 x i32>> [#uses=1]
  %tmp232 = extractelement <4 x i32> %tmp231, i32 3 ; <i32> [#uses=1]
  %tmp233 = load i32* %fifteen                    ; <i32> [#uses=1]
  %shl234 = shl i32 %tmp232, %tmp233              ; <i32> [#uses=1]
  %xor235 = xor i32 %xor230, %shl234              ; <i32> [#uses=1]
  %tmp236 = load i32* %i                          ; <i32> [#uses=1]
  %arraydecay237 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx238 = getelementptr inbounds <4 x i32>* %arraydecay237, i32 %tmp236 ; <<4 x i32>*> [#uses=2]
  %tmp239 = load <4 x i32>* %arrayidx238          ; <<4 x i32>> [#uses=1]
  %tmp240 = insertelement <4 x i32> %tmp239, i32 %xor235, i32 3 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp240, <4 x i32>* %arrayidx238
  br label %for.inc

for.inc:                                          ; preds = %sw.epilog
  %tmp241 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add i32 %tmp241, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arraydecay243 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx244 = getelementptr inbounds <4 x i32>* %arraydecay243, i32 0 ; <<4 x i32>*> [#uses=1]
  %tmp245 = load <4 x i32>* %arrayidx244          ; <<4 x i32>> [#uses=1]
  %tmp246 = extractelement <4 x i32> %tmp245, i32 0 ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp246 to float             ; <float> [#uses=1]
  %vecinit247 = insertelement <4 x float> undef, float %conv, i32 0 ; <<4 x float>> [#uses=1]
  %arraydecay248 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx249 = getelementptr inbounds <4 x i32>* %arraydecay248, i32 0 ; <<4 x i32>*> [#uses=1]
  %tmp250 = load <4 x i32>* %arrayidx249          ; <<4 x i32>> [#uses=1]
  %tmp251 = extractelement <4 x i32> %tmp250, i32 1 ; <i32> [#uses=1]
  %conv252 = uitofp i32 %tmp251 to float          ; <float> [#uses=1]
  %vecinit253 = insertelement <4 x float> %vecinit247, float %conv252, i32 1 ; <<4 x float>> [#uses=1]
  %arraydecay254 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx255 = getelementptr inbounds <4 x i32>* %arraydecay254, i32 0 ; <<4 x i32>*> [#uses=1]
  %tmp256 = load <4 x i32>* %arrayidx255          ; <<4 x i32>> [#uses=1]
  %tmp257 = extractelement <4 x i32> %tmp256, i32 2 ; <i32> [#uses=1]
  %conv258 = uitofp i32 %tmp257 to float          ; <float> [#uses=1]
  %vecinit259 = insertelement <4 x float> %vecinit253, float %conv258, i32 2 ; <<4 x float>> [#uses=1]
  %arraydecay260 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx261 = getelementptr inbounds <4 x i32>* %arraydecay260, i32 0 ; <<4 x i32>*> [#uses=1]
  %tmp262 = load <4 x i32>* %arrayidx261          ; <<4 x i32>> [#uses=1]
  %tmp263 = extractelement <4 x i32> %tmp262, i32 3 ; <i32> [#uses=1]
  %conv264 = uitofp i32 %tmp263 to float          ; <float> [#uses=1]
  %vecinit265 = insertelement <4 x float> %vecinit259, float %conv264, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit265, <4 x float>* %tmpValue1
  %tmp266 = load <4 x float>* %tmpValue1          ; <<4 x float>> [#uses=1]
  %tmp267 = load float* %one                      ; <float> [#uses=1]
  %tmp268 = insertelement <4 x float> undef, float %tmp267, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp268, <4 x float> %tmp268, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul269 = fmul <4 x float> %tmp266, %splat      ; <<4 x float>> [#uses=1]
  %tmp270 = load float* %intMax                   ; <float> [#uses=1]
  %tmp271 = insertelement <4 x float> undef, float %tmp270, i32 0 ; <<4 x float>> [#uses=2]
  %splat272 = shufflevector <4 x float> %tmp271, <4 x float> %tmp271, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp273 = fcmp oeq <4 x float> zeroinitializer, %splat272 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp273, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat272 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %mul269, %splat272      ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %temp1
  %arraydecay275 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx276 = getelementptr inbounds <4 x i32>* %arraydecay275, i32 1 ; <<4 x i32>*> [#uses=1]
  %tmp277 = load <4 x i32>* %arrayidx276          ; <<4 x i32>> [#uses=1]
  %tmp278 = extractelement <4 x i32> %tmp277, i32 0 ; <i32> [#uses=1]
  %conv279 = uitofp i32 %tmp278 to float          ; <float> [#uses=1]
  %vecinit280 = insertelement <4 x float> undef, float %conv279, i32 0 ; <<4 x float>> [#uses=1]
  %arraydecay281 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx282 = getelementptr inbounds <4 x i32>* %arraydecay281, i32 1 ; <<4 x i32>*> [#uses=1]
  %tmp283 = load <4 x i32>* %arrayidx282          ; <<4 x i32>> [#uses=1]
  %tmp284 = extractelement <4 x i32> %tmp283, i32 1 ; <i32> [#uses=1]
  %conv285 = uitofp i32 %tmp284 to float          ; <float> [#uses=1]
  %vecinit286 = insertelement <4 x float> %vecinit280, float %conv285, i32 1 ; <<4 x float>> [#uses=1]
  %arraydecay287 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx288 = getelementptr inbounds <4 x i32>* %arraydecay287, i32 1 ; <<4 x i32>*> [#uses=1]
  %tmp289 = load <4 x i32>* %arrayidx288          ; <<4 x i32>> [#uses=1]
  %tmp290 = extractelement <4 x i32> %tmp289, i32 2 ; <i32> [#uses=1]
  %conv291 = uitofp i32 %tmp290 to float          ; <float> [#uses=1]
  %vecinit292 = insertelement <4 x float> %vecinit286, float %conv291, i32 2 ; <<4 x float>> [#uses=1]
  %arraydecay293 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx294 = getelementptr inbounds <4 x i32>* %arraydecay293, i32 1 ; <<4 x i32>*> [#uses=1]
  %tmp295 = load <4 x i32>* %arrayidx294          ; <<4 x i32>> [#uses=1]
  %tmp296 = extractelement <4 x i32> %tmp295, i32 3 ; <i32> [#uses=1]
  %conv297 = uitofp i32 %tmp296 to float          ; <float> [#uses=1]
  %vecinit298 = insertelement <4 x float> %vecinit292, float %conv297, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit298, <4 x float>* %tmpValue2
  %tmp299 = load <4 x float>* %tmpValue2          ; <<4 x float>> [#uses=1]
  %tmp300 = load float* %one                      ; <float> [#uses=1]
  %tmp301 = insertelement <4 x float> undef, float %tmp300, i32 0 ; <<4 x float>> [#uses=2]
  %splat302 = shufflevector <4 x float> %tmp301, <4 x float> %tmp301, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul303 = fmul <4 x float> %tmp299, %splat302   ; <<4 x float>> [#uses=1]
  %tmp304 = load float* %intMax                   ; <float> [#uses=1]
  %tmp305 = insertelement <4 x float> undef, float %tmp304, i32 0 ; <<4 x float>> [#uses=2]
  %splat306 = shufflevector <4 x float> %tmp305, <4 x float> %tmp305, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp307 = fcmp oeq <4 x float> zeroinitializer, %splat306 ; <<4 x i1>> [#uses=1]
  %sel308 = select <4 x i1> %cmp307, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat306 ; <<4 x float>> [#uses=0]
  %div309 = fdiv <4 x float> %mul303, %splat306   ; <<4 x float>> [#uses=1]
  store <4 x float> %div309, <4 x float>* %temp2
  %tmp310 = load float* %two                      ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp310       ; <float> [#uses=1]
  %tmp311 = insertelement <4 x float> undef, float %neg, i32 0 ; <<4 x float>> [#uses=2]
  %splat312 = shufflevector <4 x float> %tmp311, <4 x float> %tmp311, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp313 = load <4 x float>* %temp1              ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z3logDv4_f(<4 x float> %tmp313) ; <<4 x float>> [#uses=1]
  %mul314 = fmul <4 x float> %splat312, %call     ; <<4 x float>> [#uses=1]
  %call315 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %mul314) ; <<4 x float>> [#uses=1]
  store <4 x float> %call315, <4 x float>* %r
  %tmp316 = load float* %two                      ; <float> [#uses=1]
  %tmp317 = load float* %PI                       ; <float> [#uses=1]
  %mul318 = fmul float %tmp316, %tmp317           ; <float> [#uses=1]
  %tmp319 = insertelement <4 x float> undef, float %mul318, i32 0 ; <<4 x float>> [#uses=2]
  %splat320 = shufflevector <4 x float> %tmp319, <4 x float> %tmp319, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp321 = load <4 x float>* %temp2              ; <<4 x float>> [#uses=1]
  %mul322 = fmul <4 x float> %splat320, %tmp321   ; <<4 x float>> [#uses=1]
  store <4 x float> %mul322, <4 x float>* %phi
  %tmp323 = load <4 x float>* %r                  ; <<4 x float>> [#uses=1]
  %tmp324 = load <4 x float>* %phi                ; <<4 x float>> [#uses=1]
  %call325 = call <4 x float> @_Z3cosDv4_f(<4 x float> %tmp324) ; <<4 x float>> [#uses=1]
  %mul326 = fmul <4 x float> %tmp323, %call325    ; <<4 x float>> [#uses=1]
  %tmp327 = load <4 x float>** %gaussianRand1.addr ; <<4 x float>*> [#uses=1]
  store <4 x float> %mul326, <4 x float>* %tmp327
  %tmp328 = load <4 x float>* %r                  ; <<4 x float>> [#uses=1]
  %tmp329 = load <4 x float>* %phi                ; <<4 x float>> [#uses=1]
  %call330 = call <4 x float> @_Z3sinDv4_f(<4 x float> %tmp329) ; <<4 x float>> [#uses=1]
  %mul331 = fmul <4 x float> %tmp328, %call330    ; <<4 x float>> [#uses=1]
  %tmp332 = load <4 x float>** %gaussianRand2.addr ; <<4 x float>*> [#uses=1]
  store <4 x float> %mul331, <4 x float>* %tmp332
  %arraydecay333 = getelementptr inbounds [8 x <4 x i32>]* %temp, i32 0, i32 0 ; <<4 x i32>*> [#uses=1]
  %arrayidx334 = getelementptr inbounds <4 x i32>* %arraydecay333, i32 2 ; <<4 x i32>*> [#uses=1]
  %tmp335 = load <4 x i32>* %arrayidx334          ; <<4 x i32>> [#uses=1]
  %tmp336 = load <4 x i32>** %nextRand.addr       ; <<4 x i32>*> [#uses=1]
  store <4 x i32> %tmp335, <4 x i32>* %tmp336
  ret void
}

declare <4 x float> @_Z4sqrtDv4_f(<4 x float>)

declare <4 x float> @_Z3logDv4_f(<4 x float>)

declare <4 x float> @_Z3cosDv4_f(<4 x float>)

declare <4 x float> @_Z3sinDv4_f(<4 x float>)

; CHECK: ret
define void @calOutputs(<4 x float> %strikePrice, <4 x float> %meanDeriv1, <4 x float> %meanDeriv2, <4 x float> %meanPrice1, <4 x float> %meanPrice2, <4 x float>* %pathDeriv1, <4 x float>* %pathDeriv2, <4 x float>* %priceVec1, <4 x float>* %priceVec2) nounwind {
entry:
  %strikePrice.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=3]
  %meanDeriv1.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %meanDeriv2.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %meanPrice1.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %meanPrice2.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %pathDeriv1.addr = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %pathDeriv2.addr = alloca <4 x float>*, align 4 ; <<4 x float>**> [#uses=2]
  %priceVec1.addr = alloca <4 x float>*, align 4  ; <<4 x float>**> [#uses=2]
  %priceVec2.addr = alloca <4 x float>*, align 4  ; <<4 x float>**> [#uses=2]
  %temp1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=10]
  %temp2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=10]
  %temp3 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=10]
  %temp4 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=10]
  %tempDiff1 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=9]
  %tempDiff2 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=9]
  store <4 x float> %strikePrice, <4 x float>* %strikePrice.addr
  store <4 x float> %meanDeriv1, <4 x float>* %meanDeriv1.addr
  store <4 x float> %meanDeriv2, <4 x float>* %meanDeriv2.addr
  store <4 x float> %meanPrice1, <4 x float>* %meanPrice1.addr
  store <4 x float> %meanPrice2, <4 x float>* %meanPrice2.addr
  store <4 x float>* %pathDeriv1, <4 x float>** %pathDeriv1.addr
  store <4 x float>* %pathDeriv2, <4 x float>** %pathDeriv2.addr
  store <4 x float>* %priceVec1, <4 x float>** %priceVec1.addr
  store <4 x float>* %priceVec2, <4 x float>** %priceVec2.addr
  store <4 x float> zeroinitializer, <4 x float>* %temp1
  store <4 x float> zeroinitializer, <4 x float>* %temp2
  store <4 x float> zeroinitializer, <4 x float>* %temp3
  store <4 x float> zeroinitializer, <4 x float>* %temp4
  %tmp = load <4 x float>* %meanPrice1.addr       ; <<4 x float>> [#uses=1]
  %tmp1 = load <4 x float>* %strikePrice.addr     ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp, %tmp1             ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %tempDiff1
  %tmp3 = load <4 x float>* %meanPrice2.addr      ; <<4 x float>> [#uses=1]
  %tmp4 = load <4 x float>* %strikePrice.addr     ; <<4 x float>> [#uses=1]
  %sub5 = fsub <4 x float> %tmp3, %tmp4           ; <<4 x float>> [#uses=1]
  store <4 x float> %sub5, <4 x float>* %tempDiff2
  %tmp6 = load <4 x float>* %tempDiff1            ; <<4 x float>> [#uses=1]
  %tmp7 = extractelement <4 x float> %tmp6, i32 0 ; <float> [#uses=1]
  %cmp = fcmp ogt float %tmp7, 0.000000e+000      ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp8 = load <4 x float>* %temp1                ; <<4 x float>> [#uses=1]
  %tmp9 = insertelement <4 x float> %tmp8, float 1.000000e+000, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp9, <4 x float>* %temp1
  %tmp10 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp11 = extractelement <4 x float> %tmp10, i32 0 ; <float> [#uses=1]
  %tmp12 = load <4 x float>* %temp3               ; <<4 x float>> [#uses=1]
  %tmp13 = insertelement <4 x float> %tmp12, float %tmp11, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp13, <4 x float>* %temp3
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp14 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp15 = extractelement <4 x float> %tmp14, i32 1 ; <float> [#uses=1]
  %cmp16 = fcmp ogt float %tmp15, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp16, label %if.then17, label %if.end24

if.then17:                                        ; preds = %if.end
  %tmp18 = load <4 x float>* %temp1               ; <<4 x float>> [#uses=1]
  %tmp19 = insertelement <4 x float> %tmp18, float 1.000000e+000, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp19, <4 x float>* %temp1
  %tmp20 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp21 = extractelement <4 x float> %tmp20, i32 1 ; <float> [#uses=1]
  %tmp22 = load <4 x float>* %temp3               ; <<4 x float>> [#uses=1]
  %tmp23 = insertelement <4 x float> %tmp22, float %tmp21, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp23, <4 x float>* %temp3
  br label %if.end24

if.end24:                                         ; preds = %if.then17, %if.end
  %tmp25 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp26 = extractelement <4 x float> %tmp25, i32 2 ; <float> [#uses=1]
  %cmp27 = fcmp ogt float %tmp26, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp27, label %if.then28, label %if.end35

if.then28:                                        ; preds = %if.end24
  %tmp29 = load <4 x float>* %temp1               ; <<4 x float>> [#uses=1]
  %tmp30 = insertelement <4 x float> %tmp29, float 1.000000e+000, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp30, <4 x float>* %temp1
  %tmp31 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp32 = extractelement <4 x float> %tmp31, i32 2 ; <float> [#uses=1]
  %tmp33 = load <4 x float>* %temp3               ; <<4 x float>> [#uses=1]
  %tmp34 = insertelement <4 x float> %tmp33, float %tmp32, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp34, <4 x float>* %temp3
  br label %if.end35

if.end35:                                         ; preds = %if.then28, %if.end24
  %tmp36 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp37 = extractelement <4 x float> %tmp36, i32 3 ; <float> [#uses=1]
  %cmp38 = fcmp ogt float %tmp37, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp38, label %if.then39, label %if.end46

if.then39:                                        ; preds = %if.end35
  %tmp40 = load <4 x float>* %temp1               ; <<4 x float>> [#uses=1]
  %tmp41 = insertelement <4 x float> %tmp40, float 1.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %temp1
  %tmp42 = load <4 x float>* %tempDiff1           ; <<4 x float>> [#uses=1]
  %tmp43 = extractelement <4 x float> %tmp42, i32 3 ; <float> [#uses=1]
  %tmp44 = load <4 x float>* %temp3               ; <<4 x float>> [#uses=1]
  %tmp45 = insertelement <4 x float> %tmp44, float %tmp43, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp45, <4 x float>* %temp3
  br label %if.end46

if.end46:                                         ; preds = %if.then39, %if.end35
  %tmp47 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp48 = extractelement <4 x float> %tmp47, i32 0 ; <float> [#uses=1]
  %cmp49 = fcmp ogt float %tmp48, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp49, label %if.then50, label %if.end57

if.then50:                                        ; preds = %if.end46
  %tmp51 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %tmp52 = insertelement <4 x float> %tmp51, float 1.000000e+000, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp52, <4 x float>* %temp2
  %tmp53 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp54 = extractelement <4 x float> %tmp53, i32 0 ; <float> [#uses=1]
  %tmp55 = load <4 x float>* %temp4               ; <<4 x float>> [#uses=1]
  %tmp56 = insertelement <4 x float> %tmp55, float %tmp54, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %temp4
  br label %if.end57

if.end57:                                         ; preds = %if.then50, %if.end46
  %tmp58 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp59 = extractelement <4 x float> %tmp58, i32 1 ; <float> [#uses=1]
  %cmp60 = fcmp ogt float %tmp59, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp60, label %if.then61, label %if.end68

if.then61:                                        ; preds = %if.end57
  %tmp62 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %tmp63 = insertelement <4 x float> %tmp62, float 1.000000e+000, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp63, <4 x float>* %temp2
  %tmp64 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp65 = extractelement <4 x float> %tmp64, i32 1 ; <float> [#uses=1]
  %tmp66 = load <4 x float>* %temp4               ; <<4 x float>> [#uses=1]
  %tmp67 = insertelement <4 x float> %tmp66, float %tmp65, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp67, <4 x float>* %temp4
  br label %if.end68

if.end68:                                         ; preds = %if.then61, %if.end57
  %tmp69 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp70 = extractelement <4 x float> %tmp69, i32 2 ; <float> [#uses=1]
  %cmp71 = fcmp ogt float %tmp70, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp71, label %if.then72, label %if.end79

if.then72:                                        ; preds = %if.end68
  %tmp73 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %tmp74 = insertelement <4 x float> %tmp73, float 1.000000e+000, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp74, <4 x float>* %temp2
  %tmp75 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp76 = extractelement <4 x float> %tmp75, i32 2 ; <float> [#uses=1]
  %tmp77 = load <4 x float>* %temp4               ; <<4 x float>> [#uses=1]
  %tmp78 = insertelement <4 x float> %tmp77, float %tmp76, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp78, <4 x float>* %temp4
  br label %if.end79

if.end79:                                         ; preds = %if.then72, %if.end68
  %tmp80 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp81 = extractelement <4 x float> %tmp80, i32 3 ; <float> [#uses=1]
  %cmp82 = fcmp ogt float %tmp81, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp82, label %if.then83, label %if.end90

if.then83:                                        ; preds = %if.end79
  %tmp84 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %tmp85 = insertelement <4 x float> %tmp84, float 1.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp85, <4 x float>* %temp2
  %tmp86 = load <4 x float>* %tempDiff2           ; <<4 x float>> [#uses=1]
  %tmp87 = extractelement <4 x float> %tmp86, i32 3 ; <float> [#uses=1]
  %tmp88 = load <4 x float>* %temp4               ; <<4 x float>> [#uses=1]
  %tmp89 = insertelement <4 x float> %tmp88, float %tmp87, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp89, <4 x float>* %temp4
  br label %if.end90

if.end90:                                         ; preds = %if.then83, %if.end79
  %tmp91 = load <4 x float>* %meanDeriv1.addr     ; <<4 x float>> [#uses=1]
  %tmp92 = load <4 x float>* %temp1               ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp91, %tmp92          ; <<4 x float>> [#uses=1]
  %tmp93 = load <4 x float>** %pathDeriv1.addr    ; <<4 x float>*> [#uses=1]
  store <4 x float> %mul, <4 x float>* %tmp93
  %tmp94 = load <4 x float>* %meanDeriv2.addr     ; <<4 x float>> [#uses=1]
  %tmp95 = load <4 x float>* %temp2               ; <<4 x float>> [#uses=1]
  %mul96 = fmul <4 x float> %tmp94, %tmp95        ; <<4 x float>> [#uses=1]
  %tmp97 = load <4 x float>** %pathDeriv2.addr    ; <<4 x float>*> [#uses=1]
  store <4 x float> %mul96, <4 x float>* %tmp97
  %tmp98 = load <4 x float>* %temp3               ; <<4 x float>> [#uses=1]
  %tmp99 = load <4 x float>** %priceVec1.addr     ; <<4 x float>*> [#uses=1]
  store <4 x float> %tmp98, <4 x float>* %tmp99
  %tmp100 = load <4 x float>* %temp4              ; <<4 x float>> [#uses=1]
  %tmp101 = load <4 x float>** %priceVec2.addr    ; <<4 x float>*> [#uses=1]
  store <4 x float> %tmp100, <4 x float>* %tmp101
  ret void
}

; CHECK: ret
define void @calPriceVega(<4 x float> addrspace(1)* %attrib, i32 %noOfSum, i32 %width, <4 x i32> addrspace(1)* %randArray, <4 x float> addrspace(1)* %priceSamples, <4 x float> addrspace(1)* %pathDeriv) nounwind {
entry:
  %attrib.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=8]
  %noOfSum.addr = alloca i32, align 4             ; <i32*> [#uses=6]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=6]
  %randArray.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %priceSamples.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=3]
  %pathDeriv.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=3]
  %strikePrice = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %initPrice = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=7]
  %sigma = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=3]
  %timeStep = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %xPos = alloca i32, align 4                     ; <i32*> [#uses=6]
  %yPos = alloca i32, align 4                     ; <i32*> [#uses=6]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=12]
  %price1 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %price2 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %pathDeriv1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %pathDeriv2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %trajPrice1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=6]
  %trajPrice2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=6]
  %sumPrice1 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=4]
  %sumPrice2 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=4]
  %meanPrice1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %meanPrice2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %sumDeriv1 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=4]
  %sumDeriv2 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=4]
  %meanDeriv1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %meanDeriv2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %finalRandf1 = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %finalRandf2 = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %nextRand = alloca <4 x i32>, align 16          ; <<4 x i32>*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %tempRand = alloca <4 x i32>, align 16          ; <<4 x i32>*> [#uses=2]
  store <4 x float> addrspace(1)* %attrib, <4 x float> addrspace(1)** %attrib.addr
  store i32 %noOfSum, i32* %noOfSum.addr
  store i32 %width, i32* %width.addr
  store <4 x i32> addrspace(1)* %randArray, <4 x i32> addrspace(1)** %randArray.addr
  store <4 x float> addrspace(1)* %priceSamples, <4 x float> addrspace(1)** %priceSamples.addr
  store <4 x float> addrspace(1)* %pathDeriv, <4 x float> addrspace(1)** %pathDeriv.addr
  %tmp = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1, <4 x float>* %strikePrice
  %tmp3 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx4 = getelementptr inbounds <4 x float> addrspace(1)* %tmp3, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp5 = load <4 x float> addrspace(1)* %arrayidx4 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp5, <4 x float>* %c1
  %tmp7 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx8 = getelementptr inbounds <4 x float> addrspace(1)* %tmp7, i32 2 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp9 = load <4 x float> addrspace(1)* %arrayidx8 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp9, <4 x float>* %c2
  %tmp11 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds <4 x float> addrspace(1)* %tmp11, i32 3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp13 = load <4 x float> addrspace(1)* %arrayidx12 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp13, <4 x float>* %c3
  %tmp15 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx16 = getelementptr inbounds <4 x float> addrspace(1)* %tmp15, i32 4 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp17 = load <4 x float> addrspace(1)* %arrayidx16 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp17, <4 x float>* %initPrice
  %tmp19 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds <4 x float> addrspace(1)* %tmp19, i32 5 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp21 = load <4 x float> addrspace(1)* %arrayidx20 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp21, <4 x float>* %sigma
  %tmp23 = load <4 x float> addrspace(1)** %attrib.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds <4 x float> addrspace(1)* %tmp23, i32 6 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp25 = load <4 x float> addrspace(1)* %arrayidx24 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %timeStep
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %xPos
  %call28 = call i32 @get_global_id(i32 1)        ; <i32> [#uses=1]
  store i32 %call28, i32* %yPos
  store <4 x float> zeroinitializer, <4 x float>* %temp
  store <4 x float> zeroinitializer, <4 x float>* %price1
  store <4 x float> zeroinitializer, <4 x float>* %price2
  store <4 x float> zeroinitializer, <4 x float>* %pathDeriv1
  store <4 x float> zeroinitializer, <4 x float>* %pathDeriv2
  %tmp35 = load <4 x float>* %initPrice           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp35, <4 x float>* %trajPrice1
  %tmp37 = load <4 x float>* %initPrice           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp37, <4 x float>* %trajPrice2
  %tmp39 = load <4 x float>* %initPrice           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp39, <4 x float>* %sumPrice1
  %tmp41 = load <4 x float>* %initPrice           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %sumPrice2
  %tmp43 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp43, <4 x float>* %meanPrice1
  %tmp45 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp45, <4 x float>* %meanPrice2
  %tmp47 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp47, <4 x float>* %sumDeriv1
  %tmp49 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp49, <4 x float>* %sumDeriv2
  %tmp51 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp51, <4 x float>* %meanDeriv1
  %tmp53 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp53, <4 x float>* %meanDeriv2
  %tmp55 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp55, <4 x float>* %finalRandf1
  %tmp57 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp57, <4 x float>* %finalRandf2
  %tmp59 = load i32* %yPos                        ; <i32> [#uses=1]
  %tmp60 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp59, %tmp60                   ; <i32> [#uses=1]
  %tmp61 = load i32* %xPos                        ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp61                     ; <i32> [#uses=1]
  %tmp62 = load <4 x i32> addrspace(1)** %randArray.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx63 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp62, i32 %add ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp64 = load <4 x i32> addrspace(1)* %arrayidx63 ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp64, <4 x i32>* %nextRand
  store i32 1, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp66 = load i32* %i                           ; <i32> [#uses=1]
  %tmp67 = load i32* %noOfSum.addr                ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp66, %tmp67              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp69 = load <4 x i32>* %nextRand              ; <<4 x i32>> [#uses=1]
  store <4 x i32> %tmp69, <4 x i32>* %tempRand
  %tmp70 = load <4 x i32>* %tempRand              ; <<4 x i32>> [#uses=1]
  call void @generateRand(<4 x i32> %tmp70, <4 x float>* %finalRandf1, <4 x float>* %finalRandf2, <4 x i32>* %nextRand)
  %tmp71 = load <4 x float>* %trajPrice1          ; <<4 x float>> [#uses=1]
  %tmp72 = load <4 x float>* %c1                  ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %c2                  ; <<4 x float>> [#uses=1]
  %tmp74 = load <4 x float>* %finalRandf1         ; <<4 x float>> [#uses=1]
  %mul75 = fmul <4 x float> %tmp73, %tmp74        ; <<4 x float>> [#uses=1]
  %add76 = fadd <4 x float> %tmp72, %mul75        ; <<4 x float>> [#uses=1]
  %call77 = call <4 x float> @_Z3expDv4_f(<4 x float> %add76) ; <<4 x float>> [#uses=1]
  %mul78 = fmul <4 x float> %tmp71, %call77       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul78, <4 x float>* %trajPrice1
  %tmp79 = load <4 x float>* %trajPrice2          ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %c1                  ; <<4 x float>> [#uses=1]
  %tmp81 = load <4 x float>* %c2                  ; <<4 x float>> [#uses=1]
  %tmp82 = load <4 x float>* %finalRandf2         ; <<4 x float>> [#uses=1]
  %mul83 = fmul <4 x float> %tmp81, %tmp82        ; <<4 x float>> [#uses=1]
  %add84 = fadd <4 x float> %tmp80, %mul83        ; <<4 x float>> [#uses=1]
  %call85 = call <4 x float> @_Z3expDv4_f(<4 x float> %add84) ; <<4 x float>> [#uses=1]
  %mul86 = fmul <4 x float> %tmp79, %call85       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul86, <4 x float>* %trajPrice2
  %tmp87 = load <4 x float>* %sumPrice1           ; <<4 x float>> [#uses=1]
  %tmp88 = load <4 x float>* %trajPrice1          ; <<4 x float>> [#uses=1]
  %add89 = fadd <4 x float> %tmp87, %tmp88        ; <<4 x float>> [#uses=1]
  store <4 x float> %add89, <4 x float>* %sumPrice1
  %tmp90 = load <4 x float>* %sumPrice2           ; <<4 x float>> [#uses=1]
  %tmp91 = load <4 x float>* %trajPrice2          ; <<4 x float>> [#uses=1]
  %add92 = fadd <4 x float> %tmp90, %tmp91        ; <<4 x float>> [#uses=1]
  store <4 x float> %add92, <4 x float>* %sumPrice2
  %tmp93 = load <4 x float>* %c3                  ; <<4 x float>> [#uses=1]
  %tmp94 = load <4 x float>* %timeStep            ; <<4 x float>> [#uses=1]
  %mul95 = fmul <4 x float> %tmp93, %tmp94        ; <<4 x float>> [#uses=1]
  %tmp96 = load i32* %i                           ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp96 to float              ; <float> [#uses=1]
  %tmp97 = insertelement <4 x float> undef, float %conv, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp97, <4 x float> %tmp97, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul98 = fmul <4 x float> %mul95, %splat        ; <<4 x float>> [#uses=1]
  store <4 x float> %mul98, <4 x float>* %temp
  %tmp99 = load <4 x float>* %sumDeriv1           ; <<4 x float>> [#uses=1]
  %tmp100 = load <4 x float>* %trajPrice1         ; <<4 x float>> [#uses=1]
  %tmp101 = load <4 x float>* %trajPrice1         ; <<4 x float>> [#uses=1]
  %tmp102 = load <4 x float>* %initPrice          ; <<4 x float>> [#uses=3]
  %cmp103 = fcmp oeq <4 x float> zeroinitializer, %tmp102 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp103, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp102 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp101, %tmp102        ; <<4 x float>> [#uses=1]
  %call104 = call <4 x float> @_Z3logDv4_f(<4 x float> %div) ; <<4 x float>> [#uses=1]
  %tmp105 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %call104, %tmp105       ; <<4 x float>> [#uses=1]
  %tmp106 = load <4 x float>* %sigma              ; <<4 x float>> [#uses=3]
  %cmp107 = fcmp oeq <4 x float> zeroinitializer, %tmp106 ; <<4 x i1>> [#uses=1]
  %sel108 = select <4 x i1> %cmp107, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp106 ; <<4 x float>> [#uses=0]
  %div109 = fdiv <4 x float> %sub, %tmp106        ; <<4 x float>> [#uses=1]
  %mul110 = fmul <4 x float> %tmp100, %div109     ; <<4 x float>> [#uses=1]
  %add111 = fadd <4 x float> %tmp99, %mul110      ; <<4 x float>> [#uses=1]
  store <4 x float> %add111, <4 x float>* %sumDeriv1
  %tmp112 = load <4 x float>* %sumDeriv2          ; <<4 x float>> [#uses=1]
  %tmp113 = load <4 x float>* %trajPrice2         ; <<4 x float>> [#uses=1]
  %tmp114 = load <4 x float>* %trajPrice2         ; <<4 x float>> [#uses=1]
  %tmp115 = load <4 x float>* %initPrice          ; <<4 x float>> [#uses=3]
  %cmp116 = fcmp oeq <4 x float> zeroinitializer, %tmp115 ; <<4 x i1>> [#uses=1]
  %sel117 = select <4 x i1> %cmp116, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp115 ; <<4 x float>> [#uses=0]
  %div118 = fdiv <4 x float> %tmp114, %tmp115     ; <<4 x float>> [#uses=1]
  %call119 = call <4 x float> @_Z3logDv4_f(<4 x float> %div118) ; <<4 x float>> [#uses=1]
  %tmp120 = load <4 x float>* %temp               ; <<4 x float>> [#uses=1]
  %sub121 = fsub <4 x float> %call119, %tmp120    ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %sigma              ; <<4 x float>> [#uses=3]
  %cmp123 = fcmp oeq <4 x float> zeroinitializer, %tmp122 ; <<4 x i1>> [#uses=1]
  %sel124 = select <4 x i1> %cmp123, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp122 ; <<4 x float>> [#uses=0]
  %div125 = fdiv <4 x float> %sub121, %tmp122     ; <<4 x float>> [#uses=1]
  %mul126 = fmul <4 x float> %tmp113, %div125     ; <<4 x float>> [#uses=1]
  %add127 = fadd <4 x float> %tmp112, %mul126     ; <<4 x float>> [#uses=1]
  store <4 x float> %add127, <4 x float>* %sumDeriv2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp128 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp128, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp129 = load <4 x float>* %sumPrice1          ; <<4 x float>> [#uses=1]
  %tmp130 = load i32* %noOfSum.addr               ; <i32> [#uses=1]
  %conv131 = sitofp i32 %tmp130 to float          ; <float> [#uses=1]
  %tmp132 = insertelement <4 x float> undef, float %conv131, i32 0 ; <<4 x float>> [#uses=2]
  %splat133 = shufflevector <4 x float> %tmp132, <4 x float> %tmp132, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp134 = fcmp oeq <4 x float> zeroinitializer, %splat133 ; <<4 x i1>> [#uses=1]
  %sel135 = select <4 x i1> %cmp134, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat133 ; <<4 x float>> [#uses=0]
  %div136 = fdiv <4 x float> %tmp129, %splat133   ; <<4 x float>> [#uses=1]
  store <4 x float> %div136, <4 x float>* %meanPrice1
  %tmp137 = load <4 x float>* %sumPrice2          ; <<4 x float>> [#uses=1]
  %tmp138 = load i32* %noOfSum.addr               ; <i32> [#uses=1]
  %conv139 = sitofp i32 %tmp138 to float          ; <float> [#uses=1]
  %tmp140 = insertelement <4 x float> undef, float %conv139, i32 0 ; <<4 x float>> [#uses=2]
  %splat141 = shufflevector <4 x float> %tmp140, <4 x float> %tmp140, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp142 = fcmp oeq <4 x float> zeroinitializer, %splat141 ; <<4 x i1>> [#uses=1]
  %sel143 = select <4 x i1> %cmp142, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat141 ; <<4 x float>> [#uses=0]
  %div144 = fdiv <4 x float> %tmp137, %splat141   ; <<4 x float>> [#uses=1]
  store <4 x float> %div144, <4 x float>* %meanPrice2
  %tmp145 = load <4 x float>* %sumDeriv1          ; <<4 x float>> [#uses=1]
  %tmp146 = load i32* %noOfSum.addr               ; <i32> [#uses=1]
  %conv147 = sitofp i32 %tmp146 to float          ; <float> [#uses=1]
  %tmp148 = insertelement <4 x float> undef, float %conv147, i32 0 ; <<4 x float>> [#uses=2]
  %splat149 = shufflevector <4 x float> %tmp148, <4 x float> %tmp148, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp150 = fcmp oeq <4 x float> zeroinitializer, %splat149 ; <<4 x i1>> [#uses=1]
  %sel151 = select <4 x i1> %cmp150, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat149 ; <<4 x float>> [#uses=0]
  %div152 = fdiv <4 x float> %tmp145, %splat149   ; <<4 x float>> [#uses=1]
  store <4 x float> %div152, <4 x float>* %meanDeriv1
  %tmp153 = load <4 x float>* %sumDeriv2          ; <<4 x float>> [#uses=1]
  %tmp154 = load i32* %noOfSum.addr               ; <i32> [#uses=1]
  %conv155 = sitofp i32 %tmp154 to float          ; <float> [#uses=1]
  %tmp156 = insertelement <4 x float> undef, float %conv155, i32 0 ; <<4 x float>> [#uses=2]
  %splat157 = shufflevector <4 x float> %tmp156, <4 x float> %tmp156, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp158 = fcmp oeq <4 x float> zeroinitializer, %splat157 ; <<4 x i1>> [#uses=1]
  %sel159 = select <4 x i1> %cmp158, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat157 ; <<4 x float>> [#uses=0]
  %div160 = fdiv <4 x float> %tmp153, %splat157   ; <<4 x float>> [#uses=1]
  store <4 x float> %div160, <4 x float>* %meanDeriv2
  %tmp161 = load <4 x float>* %strikePrice        ; <<4 x float>> [#uses=1]
  %tmp162 = load <4 x float>* %meanDeriv1         ; <<4 x float>> [#uses=1]
  %tmp163 = load <4 x float>* %meanDeriv2         ; <<4 x float>> [#uses=1]
  %tmp164 = load <4 x float>* %meanPrice1         ; <<4 x float>> [#uses=1]
  %tmp165 = load <4 x float>* %meanPrice2         ; <<4 x float>> [#uses=1]
  call void @calOutputs(<4 x float> %tmp161, <4 x float> %tmp162, <4 x float> %tmp163, <4 x float> %tmp164, <4 x float> %tmp165, <4 x float>* %pathDeriv1, <4 x float>* %pathDeriv2, <4 x float>* %price1, <4 x float>* %price2)
  %tmp166 = load <4 x float>* %price1             ; <<4 x float>> [#uses=1]
  %tmp167 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp168 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul169 = mul i32 %tmp167, %tmp168              ; <i32> [#uses=1]
  %tmp170 = load i32* %xPos                       ; <i32> [#uses=1]
  %add171 = add i32 %mul169, %tmp170              ; <i32> [#uses=1]
  %mul172 = mul i32 %add171, 2                    ; <i32> [#uses=1]
  %tmp173 = load <4 x float> addrspace(1)** %priceSamples.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx174 = getelementptr inbounds <4 x float> addrspace(1)* %tmp173, i32 %mul172 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp166, <4 x float> addrspace(1)* %arrayidx174
  %tmp175 = load <4 x float>* %price2             ; <<4 x float>> [#uses=1]
  %tmp176 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp177 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul178 = mul i32 %tmp176, %tmp177              ; <i32> [#uses=1]
  %tmp179 = load i32* %xPos                       ; <i32> [#uses=1]
  %add180 = add i32 %mul178, %tmp179              ; <i32> [#uses=1]
  %mul181 = mul i32 %add180, 2                    ; <i32> [#uses=1]
  %add182 = add i32 %mul181, 1                    ; <i32> [#uses=1]
  %tmp183 = load <4 x float> addrspace(1)** %priceSamples.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx184 = getelementptr inbounds <4 x float> addrspace(1)* %tmp183, i32 %add182 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp175, <4 x float> addrspace(1)* %arrayidx184
  %tmp185 = load <4 x float>* %pathDeriv1         ; <<4 x float>> [#uses=1]
  %tmp186 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp187 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul188 = mul i32 %tmp186, %tmp187              ; <i32> [#uses=1]
  %tmp189 = load i32* %xPos                       ; <i32> [#uses=1]
  %add190 = add i32 %mul188, %tmp189              ; <i32> [#uses=1]
  %mul191 = mul i32 %add190, 2                    ; <i32> [#uses=1]
  %tmp192 = load <4 x float> addrspace(1)** %pathDeriv.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx193 = getelementptr inbounds <4 x float> addrspace(1)* %tmp192, i32 %mul191 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp185, <4 x float> addrspace(1)* %arrayidx193
  %tmp194 = load <4 x float>* %pathDeriv2         ; <<4 x float>> [#uses=1]
  %tmp195 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp196 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul197 = mul i32 %tmp195, %tmp196              ; <i32> [#uses=1]
  %tmp198 = load i32* %xPos                       ; <i32> [#uses=1]
  %add199 = add i32 %mul197, %tmp198              ; <i32> [#uses=1]
  %mul200 = mul i32 %add199, 2                    ; <i32> [#uses=1]
  %add201 = add i32 %mul200, 1                    ; <i32> [#uses=1]
  %tmp202 = load <4 x float> addrspace(1)** %pathDeriv.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx203 = getelementptr inbounds <4 x float> addrspace(1)* %tmp202, i32 %add201 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp194, <4 x float> addrspace(1)* %arrayidx203
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z3expDv4_f(<4 x float>)
