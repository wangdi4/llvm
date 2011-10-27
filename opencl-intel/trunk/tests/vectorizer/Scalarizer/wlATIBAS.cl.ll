; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIBAS.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_blackScholes_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_blackScholes_parameters = appending global [142 x i8] c"float4 const __attribute__((address_space(1))) *, int, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[142 x i8]*> [#uses=1]
@opencl_blackScholes_scalar_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_blackScholes_scalar_parameters = appending global [139 x i8] c"float const __attribute__((address_space(1))) *, int, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[139 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, i32, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @blackScholes to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_blackScholes_locals to i8*), i8* getelementptr inbounds ([142 x i8]* @opencl_blackScholes_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32, float addrspace(1)*, float addrspace(1)*)* @blackScholes_scalar to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_blackScholes_scalar_locals to i8*), i8* getelementptr inbounds ([139 x i8]* @opencl_blackScholes_scalar_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @phi(<4 x float> %X, <4 x float>* %phi) nounwind {
entry:
  %X.addr = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=8]
  %phi.addr = alloca <4 x float>*, align 4        ; <<4 x float>**> [#uses=2]
  %y = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=9]
  %absX = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %t = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=6]
  %result = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=9]
  %c1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c4 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %c5 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=2]
  %zero = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=5]
  %one = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %two = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %temp4 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %oneBySqrt2pi = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=2]
  store <4 x float> %X, <4 x float>* %X.addr
  store <4 x float>* %phi, <4 x float>** %phi.addr
  store <4 x float> <float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000, float 0x3FD470BF40000000>, <4 x float>* %c1
  store <4 x float> <float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000, float 0xBFD6D1F0E0000000>, <4 x float>* %c2
  store <4 x float> <float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000, float 0x3FFC80EF00000000>, <4 x float>* %c3
  store <4 x float> <float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000, float 0xBFFD23DD40000000>, <4 x float>* %c4
  store <4 x float> <float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000, float 0x3FF548CDE0000000>, <4 x float>* %c5
  store <4 x float> zeroinitializer, <4 x float>* %zero
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %one
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %two
  store <4 x float> <float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000, float 0x3FCDA67120000000>, <4 x float>* %temp4
  store <4 x float> <float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000, float 0x3FD9884540000000>, <4 x float>* %oneBySqrt2pi
  %tmp = load <4 x float>* %X.addr                ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z4fabsDv4_f(<4 x float> %tmp) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %absX
  %tmp1 = load <4 x float>* %one                  ; <<4 x float>> [#uses=1]
  %tmp2 = load <4 x float>* %one                  ; <<4 x float>> [#uses=1]
  %tmp3 = load <4 x float>* %temp4                ; <<4 x float>> [#uses=1]
  %tmp4 = load <4 x float>* %absX                 ; <<4 x float>> [#uses=1]
  %mul = fmul <4 x float> %tmp3, %tmp4            ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp2, %mul             ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %add ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %add ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp1, %add             ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %t
  %tmp5 = load <4 x float>* %one                  ; <<4 x float>> [#uses=1]
  %tmp6 = load <4 x float>* %oneBySqrt2pi         ; <<4 x float>> [#uses=1]
  %tmp7 = load <4 x float>* %X.addr               ; <<4 x float>> [#uses=1]
  %neg = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp7 ; <<4 x float>> [#uses=1]
  %tmp8 = load <4 x float>* %X.addr               ; <<4 x float>> [#uses=1]
  %mul9 = fmul <4 x float> %neg, %tmp8            ; <<4 x float>> [#uses=1]
  %tmp10 = load <4 x float>* %two                 ; <<4 x float>> [#uses=3]
  %cmp11 = fcmp oeq <4 x float> zeroinitializer, %tmp10 ; <<4 x i1>> [#uses=1]
  %sel12 = select <4 x i1> %cmp11, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp10 ; <<4 x float>> [#uses=0]
  %div13 = fdiv <4 x float> %mul9, %tmp10         ; <<4 x float>> [#uses=1]
  %call14 = call <4 x float> @_Z3expDv4_f(<4 x float> %div13) ; <<4 x float>> [#uses=1]
  %mul15 = fmul <4 x float> %tmp6, %call14        ; <<4 x float>> [#uses=1]
  %tmp16 = load <4 x float>* %t                   ; <<4 x float>> [#uses=1]
  %mul17 = fmul <4 x float> %mul15, %tmp16        ; <<4 x float>> [#uses=1]
  %tmp18 = load <4 x float>* %c1                  ; <<4 x float>> [#uses=1]
  %tmp19 = load <4 x float>* %t                   ; <<4 x float>> [#uses=1]
  %tmp20 = load <4 x float>* %c2                  ; <<4 x float>> [#uses=1]
  %tmp21 = load <4 x float>* %t                   ; <<4 x float>> [#uses=1]
  %tmp22 = load <4 x float>* %c3                  ; <<4 x float>> [#uses=1]
  %tmp23 = load <4 x float>* %t                   ; <<4 x float>> [#uses=1]
  %tmp24 = load <4 x float>* %c4                  ; <<4 x float>> [#uses=1]
  %tmp25 = load <4 x float>* %t                   ; <<4 x float>> [#uses=1]
  %tmp26 = load <4 x float>* %c5                  ; <<4 x float>> [#uses=1]
  %mul27 = fmul <4 x float> %tmp25, %tmp26        ; <<4 x float>> [#uses=1]
  %add28 = fadd <4 x float> %tmp24, %mul27        ; <<4 x float>> [#uses=1]
  %mul29 = fmul <4 x float> %tmp23, %add28        ; <<4 x float>> [#uses=1]
  %add30 = fadd <4 x float> %tmp22, %mul29        ; <<4 x float>> [#uses=1]
  %mul31 = fmul <4 x float> %tmp21, %add30        ; <<4 x float>> [#uses=1]
  %add32 = fadd <4 x float> %tmp20, %mul31        ; <<4 x float>> [#uses=1]
  %mul33 = fmul <4 x float> %tmp19, %add32        ; <<4 x float>> [#uses=1]
  %add34 = fadd <4 x float> %tmp18, %mul33        ; <<4 x float>> [#uses=1]
  %mul35 = fmul <4 x float> %mul17, %add34        ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp5, %mul35           ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %y
  %tmp36 = load <4 x float>* %X.addr              ; <<4 x float>> [#uses=1]
  %tmp37 = extractelement <4 x float> %tmp36, i32 0 ; <float> [#uses=1]
  %tmp38 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  %tmp39 = extractelement <4 x float> %tmp38, i32 0 ; <float> [#uses=1]
  %cmp40 = fcmp olt float %tmp37, %tmp39          ; <i1> [#uses=1]
  br i1 %cmp40, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp41 = load <4 x float>* %one                 ; <<4 x float>> [#uses=1]
  %tmp42 = extractelement <4 x float> %tmp41, i32 0 ; <float> [#uses=1]
  %tmp43 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp44 = extractelement <4 x float> %tmp43, i32 0 ; <float> [#uses=1]
  %sub45 = fsub float %tmp42, %tmp44              ; <float> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp46 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp47 = extractelement <4 x float> %tmp46, i32 0 ; <float> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi float [ %sub45, %cond.true ], [ %tmp47, %cond.false ] ; <float> [#uses=1]
  %tmp48 = load <4 x float>* %result              ; <<4 x float>> [#uses=1]
  %tmp49 = insertelement <4 x float> %tmp48, float %cond, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp49, <4 x float>* %result
  %tmp50 = load <4 x float>* %X.addr              ; <<4 x float>> [#uses=1]
  %tmp51 = extractelement <4 x float> %tmp50, i32 1 ; <float> [#uses=1]
  %tmp52 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  %tmp53 = extractelement <4 x float> %tmp52, i32 1 ; <float> [#uses=1]
  %cmp54 = fcmp olt float %tmp51, %tmp53          ; <i1> [#uses=1]
  br i1 %cmp54, label %cond.true55, label %cond.false61

cond.true55:                                      ; preds = %cond.end
  %tmp56 = load <4 x float>* %one                 ; <<4 x float>> [#uses=1]
  %tmp57 = extractelement <4 x float> %tmp56, i32 1 ; <float> [#uses=1]
  %tmp58 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp59 = extractelement <4 x float> %tmp58, i32 1 ; <float> [#uses=1]
  %sub60 = fsub float %tmp57, %tmp59              ; <float> [#uses=1]
  br label %cond.end64

cond.false61:                                     ; preds = %cond.end
  %tmp62 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp63 = extractelement <4 x float> %tmp62, i32 1 ; <float> [#uses=1]
  br label %cond.end64

cond.end64:                                       ; preds = %cond.false61, %cond.true55
  %cond65 = phi float [ %sub60, %cond.true55 ], [ %tmp63, %cond.false61 ] ; <float> [#uses=1]
  %tmp66 = load <4 x float>* %result              ; <<4 x float>> [#uses=1]
  %tmp67 = insertelement <4 x float> %tmp66, float %cond65, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp67, <4 x float>* %result
  %tmp68 = load <4 x float>* %X.addr              ; <<4 x float>> [#uses=1]
  %tmp69 = extractelement <4 x float> %tmp68, i32 2 ; <float> [#uses=1]
  %tmp70 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  %tmp71 = extractelement <4 x float> %tmp70, i32 2 ; <float> [#uses=1]
  %cmp72 = fcmp olt float %tmp69, %tmp71          ; <i1> [#uses=1]
  br i1 %cmp72, label %cond.true73, label %cond.false79

cond.true73:                                      ; preds = %cond.end64
  %tmp74 = load <4 x float>* %one                 ; <<4 x float>> [#uses=1]
  %tmp75 = extractelement <4 x float> %tmp74, i32 2 ; <float> [#uses=1]
  %tmp76 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp77 = extractelement <4 x float> %tmp76, i32 2 ; <float> [#uses=1]
  %sub78 = fsub float %tmp75, %tmp77              ; <float> [#uses=1]
  br label %cond.end82

cond.false79:                                     ; preds = %cond.end64
  %tmp80 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp81 = extractelement <4 x float> %tmp80, i32 2 ; <float> [#uses=1]
  br label %cond.end82

cond.end82:                                       ; preds = %cond.false79, %cond.true73
  %cond83 = phi float [ %sub78, %cond.true73 ], [ %tmp81, %cond.false79 ] ; <float> [#uses=1]
  %tmp84 = load <4 x float>* %result              ; <<4 x float>> [#uses=1]
  %tmp85 = insertelement <4 x float> %tmp84, float %cond83, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp85, <4 x float>* %result
  %tmp86 = load <4 x float>* %X.addr              ; <<4 x float>> [#uses=1]
  %tmp87 = extractelement <4 x float> %tmp86, i32 3 ; <float> [#uses=1]
  %tmp88 = load <4 x float>* %zero                ; <<4 x float>> [#uses=1]
  %tmp89 = extractelement <4 x float> %tmp88, i32 3 ; <float> [#uses=1]
  %cmp90 = fcmp olt float %tmp87, %tmp89          ; <i1> [#uses=1]
  br i1 %cmp90, label %cond.true91, label %cond.false97

cond.true91:                                      ; preds = %cond.end82
  %tmp92 = load <4 x float>* %one                 ; <<4 x float>> [#uses=1]
  %tmp93 = extractelement <4 x float> %tmp92, i32 3 ; <float> [#uses=1]
  %tmp94 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp95 = extractelement <4 x float> %tmp94, i32 3 ; <float> [#uses=1]
  %sub96 = fsub float %tmp93, %tmp95              ; <float> [#uses=1]
  br label %cond.end100

cond.false97:                                     ; preds = %cond.end82
  %tmp98 = load <4 x float>* %y                   ; <<4 x float>> [#uses=1]
  %tmp99 = extractelement <4 x float> %tmp98, i32 3 ; <float> [#uses=1]
  br label %cond.end100

cond.end100:                                      ; preds = %cond.false97, %cond.true91
  %cond101 = phi float [ %sub96, %cond.true91 ], [ %tmp99, %cond.false97 ] ; <float> [#uses=1]
  %tmp102 = load <4 x float>* %result             ; <<4 x float>> [#uses=1]
  %tmp103 = insertelement <4 x float> %tmp102, float %cond101, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp103, <4 x float>* %result
  %tmp104 = load <4 x float>* %result             ; <<4 x float>> [#uses=1]
  %tmp105 = load <4 x float>** %phi.addr          ; <<4 x float>*> [#uses=1]
  store <4 x float> %tmp104, <4 x float>* %tmp105
  ret void
}

declare <4 x float> @_Z4fabsDv4_f(<4 x float>)

declare <4 x float> @_Z3expDv4_f(<4 x float>)

; CHECK: ret
define void @blackScholes(<4 x float> addrspace(1)* %randArray, i32 %width, <4 x float> addrspace(1)* %call, <4 x float> addrspace(1)* %put) nounwind {
entry:
  %randArray.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %call.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %put.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %d1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %d2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=3]
  %phiD1 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %phiD2 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %sigmaSqrtT = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %KexpMinusRT = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %xPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %yPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %two = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %inRand = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=11]
  %S = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %K = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %T = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %R = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=3]
  %sigmaVal = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=4]
  store <4 x float> addrspace(1)* %randArray, <4 x float> addrspace(1)** %randArray.addr
  store i32 %width, i32* %width.addr
  store <4 x float> addrspace(1)* %call, <4 x float> addrspace(1)** %call.addr
  store <4 x float> addrspace(1)* %put, <4 x float> addrspace(1)** %put.addr
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %xPos
  %call2 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call2, i32* %yPos
  store <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, <4 x float>* %two
  %tmp = load i32* %yPos                          ; <i32> [#uses=1]
  %tmp3 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp3                      ; <i32> [#uses=1]
  %tmp4 = load i32* %xPos                         ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp4                      ; <i32> [#uses=1]
  %tmp5 = load <4 x float> addrspace(1)** %randArray.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %add ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp6 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp6, <4 x float>* %inRand
  %tmp8 = load <4 x float>* %inRand               ; <<4 x float>> [#uses=1]
  %mul9 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %tmp8 ; <<4 x float>> [#uses=1]
  %tmp10 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp10 ; <<4 x float>> [#uses=1]
  %mul11 = fmul <4 x float> <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002>, %sub ; <<4 x float>> [#uses=1]
  %add12 = fadd <4 x float> %mul9, %mul11         ; <<4 x float>> [#uses=1]
  store <4 x float> %add12, <4 x float>* %S
  %tmp14 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul15 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %tmp14 ; <<4 x float>> [#uses=1]
  %tmp16 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub17 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp16 ; <<4 x float>> [#uses=1]
  %mul18 = fmul <4 x float> <float 1.000000e+002, float 1.000000e+002, float 1.000000e+002, float 1.000000e+002>, %sub17 ; <<4 x float>> [#uses=1]
  %add19 = fadd <4 x float> %mul15, %mul18        ; <<4 x float>> [#uses=1]
  store <4 x float> %add19, <4 x float>* %K
  %tmp21 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul22 = fmul <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp21 ; <<4 x float>> [#uses=1]
  %tmp23 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub24 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp23 ; <<4 x float>> [#uses=1]
  %mul25 = fmul <4 x float> <float 1.000000e+001, float 1.000000e+001, float 1.000000e+001, float 1.000000e+001>, %sub24 ; <<4 x float>> [#uses=1]
  %add26 = fadd <4 x float> %mul22, %mul25        ; <<4 x float>> [#uses=1]
  store <4 x float> %add26, <4 x float>* %T
  %tmp28 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul29 = fmul <4 x float> <float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000>, %tmp28 ; <<4 x float>> [#uses=1]
  %tmp30 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub31 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp30 ; <<4 x float>> [#uses=1]
  %mul32 = fmul <4 x float> <float 0x3FA99999A0000000, float 0x3FA99999A0000000, float 0x3FA99999A0000000, float 0x3FA99999A0000000>, %sub31 ; <<4 x float>> [#uses=1]
  %add33 = fadd <4 x float> %mul29, %mul32        ; <<4 x float>> [#uses=1]
  store <4 x float> %add33, <4 x float>* %R
  %tmp35 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %mul36 = fmul <4 x float> <float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000, float 0x3F847AE140000000>, %tmp35 ; <<4 x float>> [#uses=1]
  %tmp37 = load <4 x float>* %inRand              ; <<4 x float>> [#uses=1]
  %sub38 = fsub <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %tmp37 ; <<4 x float>> [#uses=1]
  %mul39 = fmul <4 x float> <float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000>, %sub38 ; <<4 x float>> [#uses=1]
  %add40 = fadd <4 x float> %mul36, %mul39        ; <<4 x float>> [#uses=1]
  store <4 x float> %add40, <4 x float>* %sigmaVal
  %tmp41 = load <4 x float>* %sigmaVal            ; <<4 x float>> [#uses=1]
  %tmp42 = load <4 x float>* %T                   ; <<4 x float>> [#uses=1]
  %call43 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %tmp42) ; <<4 x float>> [#uses=1]
  %mul44 = fmul <4 x float> %tmp41, %call43       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul44, <4 x float>* %sigmaSqrtT
  %tmp45 = load <4 x float>* %S                   ; <<4 x float>> [#uses=1]
  %tmp46 = load <4 x float>* %K                   ; <<4 x float>> [#uses=3]
  %cmp = fcmp oeq <4 x float> zeroinitializer, %tmp46 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp46 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp45, %tmp46          ; <<4 x float>> [#uses=1]
  %call47 = call <4 x float> @_Z3logDv4_f(<4 x float> %div) ; <<4 x float>> [#uses=1]
  %tmp48 = load <4 x float>* %R                   ; <<4 x float>> [#uses=1]
  %tmp49 = load <4 x float>* %sigmaVal            ; <<4 x float>> [#uses=1]
  %tmp50 = load <4 x float>* %sigmaVal            ; <<4 x float>> [#uses=1]
  %mul51 = fmul <4 x float> %tmp49, %tmp50        ; <<4 x float>> [#uses=1]
  %tmp52 = load <4 x float>* %two                 ; <<4 x float>> [#uses=3]
  %cmp53 = fcmp oeq <4 x float> zeroinitializer, %tmp52 ; <<4 x i1>> [#uses=1]
  %sel54 = select <4 x i1> %cmp53, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp52 ; <<4 x float>> [#uses=0]
  %div55 = fdiv <4 x float> %mul51, %tmp52        ; <<4 x float>> [#uses=1]
  %add56 = fadd <4 x float> %tmp48, %div55        ; <<4 x float>> [#uses=1]
  %tmp57 = load <4 x float>* %T                   ; <<4 x float>> [#uses=1]
  %mul58 = fmul <4 x float> %add56, %tmp57        ; <<4 x float>> [#uses=1]
  %add59 = fadd <4 x float> %call47, %mul58       ; <<4 x float>> [#uses=1]
  %tmp60 = load <4 x float>* %sigmaSqrtT          ; <<4 x float>> [#uses=3]
  %cmp61 = fcmp oeq <4 x float> zeroinitializer, %tmp60 ; <<4 x i1>> [#uses=1]
  %sel62 = select <4 x i1> %cmp61, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %tmp60 ; <<4 x float>> [#uses=0]
  %div63 = fdiv <4 x float> %add59, %tmp60        ; <<4 x float>> [#uses=1]
  store <4 x float> %div63, <4 x float>* %d1
  %tmp64 = load <4 x float>* %d1                  ; <<4 x float>> [#uses=1]
  %tmp65 = load <4 x float>* %sigmaSqrtT          ; <<4 x float>> [#uses=1]
  %sub66 = fsub <4 x float> %tmp64, %tmp65        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub66, <4 x float>* %d2
  %tmp67 = load <4 x float>* %K                   ; <<4 x float>> [#uses=1]
  %tmp68 = load <4 x float>* %R                   ; <<4 x float>> [#uses=1]
  %neg = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp68 ; <<4 x float>> [#uses=1]
  %tmp69 = load <4 x float>* %T                   ; <<4 x float>> [#uses=1]
  %mul70 = fmul <4 x float> %neg, %tmp69          ; <<4 x float>> [#uses=1]
  %call71 = call <4 x float> @_Z3expDv4_f(<4 x float> %mul70) ; <<4 x float>> [#uses=1]
  %mul72 = fmul <4 x float> %tmp67, %call71       ; <<4 x float>> [#uses=1]
  store <4 x float> %mul72, <4 x float>* %KexpMinusRT
  %tmp73 = load <4 x float>* %d1                  ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %tmp73, <4 x float>* %phiD1)
  %tmp74 = load <4 x float>* %d2                  ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %tmp74, <4 x float>* %phiD2)
  %tmp75 = load <4 x float>* %S                   ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %phiD1               ; <<4 x float>> [#uses=1]
  %mul77 = fmul <4 x float> %tmp75, %tmp76        ; <<4 x float>> [#uses=1]
  %tmp78 = load <4 x float>* %KexpMinusRT         ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>* %phiD2               ; <<4 x float>> [#uses=1]
  %mul80 = fmul <4 x float> %tmp78, %tmp79        ; <<4 x float>> [#uses=1]
  %sub81 = fsub <4 x float> %mul77, %mul80        ; <<4 x float>> [#uses=1]
  %tmp82 = load i32* %yPos                        ; <i32> [#uses=1]
  %tmp83 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul84 = mul i32 %tmp82, %tmp83                 ; <i32> [#uses=1]
  %tmp85 = load i32* %xPos                        ; <i32> [#uses=1]
  %add86 = add i32 %mul84, %tmp85                 ; <i32> [#uses=1]
  %tmp87 = load <4 x float> addrspace(1)** %call.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds <4 x float> addrspace(1)* %tmp87, i32 %add86 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub81, <4 x float> addrspace(1)* %arrayidx88
  %tmp89 = load <4 x float>* %d1                  ; <<4 x float>> [#uses=1]
  %neg90 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp89 ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %neg90, <4 x float>* %phiD1)
  %tmp91 = load <4 x float>* %d2                  ; <<4 x float>> [#uses=1]
  %neg92 = fsub <4 x float> <float -0.000000e+000, float -0.000000e+000, float -0.000000e+000, float -0.000000e+000>, %tmp91 ; <<4 x float>> [#uses=1]
  call void @phi(<4 x float> %neg92, <4 x float>* %phiD2)
  %tmp93 = load <4 x float>* %KexpMinusRT         ; <<4 x float>> [#uses=1]
  %tmp94 = load <4 x float>* %phiD2               ; <<4 x float>> [#uses=1]
  %mul95 = fmul <4 x float> %tmp93, %tmp94        ; <<4 x float>> [#uses=1]
  %tmp96 = load <4 x float>* %S                   ; <<4 x float>> [#uses=1]
  %tmp97 = load <4 x float>* %phiD1               ; <<4 x float>> [#uses=1]
  %mul98 = fmul <4 x float> %tmp96, %tmp97        ; <<4 x float>> [#uses=1]
  %sub99 = fsub <4 x float> %mul95, %mul98        ; <<4 x float>> [#uses=1]
  %tmp100 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp101 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul102 = mul i32 %tmp100, %tmp101              ; <i32> [#uses=1]
  %tmp103 = load i32* %xPos                       ; <i32> [#uses=1]
  %add104 = add i32 %mul102, %tmp103              ; <i32> [#uses=1]
  %tmp105 = load <4 x float> addrspace(1)** %put.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx106 = getelementptr inbounds <4 x float> addrspace(1)* %tmp105, i32 %add104 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %sub99, <4 x float> addrspace(1)* %arrayidx106
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z4sqrtDv4_f(<4 x float>)

declare <4 x float> @_Z3logDv4_f(<4 x float>)

; CHECK: ret
define void @phi_scalar(float %X, float* %phi) nounwind {
entry:
  %X.addr = alloca float, align 4                 ; <float*> [#uses=5]
  %phi.addr = alloca float*, align 4              ; <float**> [#uses=2]
  %y = alloca float, align 4                      ; <float*> [#uses=3]
  %absX = alloca float, align 4                   ; <float*> [#uses=2]
  %t = alloca float, align 4                      ; <float*> [#uses=6]
  %result = alloca float, align 4                 ; <float*> [#uses=0]
  %c1 = alloca float, align 4                     ; <float*> [#uses=2]
  %c2 = alloca float, align 4                     ; <float*> [#uses=2]
  %c3 = alloca float, align 4                     ; <float*> [#uses=2]
  %c4 = alloca float, align 4                     ; <float*> [#uses=2]
  %c5 = alloca float, align 4                     ; <float*> [#uses=2]
  %temp4 = alloca float, align 4                  ; <float*> [#uses=2]
  %oneBySqrt2pi = alloca float, align 4           ; <float*> [#uses=2]
  store float %X, float* %X.addr
  store float* %phi, float** %phi.addr
  store float 0x3FD470BF40000000, float* %c1
  store float 0xBFD6D1F0E0000000, float* %c2
  store float 0x3FFC80EF00000000, float* %c3
  store float 0xBFFD23DD40000000, float* %c4
  store float 0x3FF548CDE0000000, float* %c5
  store float 0x3FCDA67120000000, float* %temp4
  store float 0x3FD9884540000000, float* %oneBySqrt2pi
  %tmp = load float* %X.addr                      ; <float> [#uses=1]
  %call = call float @_Z4fabsf(float %tmp)        ; <float> [#uses=1]
  store float %call, float* %absX
  %tmp1 = load float* %temp4                      ; <float> [#uses=1]
  %tmp2 = load float* %absX                       ; <float> [#uses=1]
  %mul = fmul float %tmp1, %tmp2                  ; <float> [#uses=1]
  %add = fadd float 1.000000e+000, %mul           ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %add       ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %add ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %add           ; <float> [#uses=1]
  store float %div, float* %t
  %tmp3 = load float* %oneBySqrt2pi               ; <float> [#uses=1]
  %tmp4 = load float* %X.addr                     ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp4         ; <float> [#uses=1]
  %tmp5 = load float* %X.addr                     ; <float> [#uses=1]
  %mul6 = fmul float %neg, %tmp5                  ; <float> [#uses=1]
  %div7 = fdiv float %mul6, 2.000000e+000         ; <float> [#uses=1]
  %call8 = call float @_Z3expf(float %div7)       ; <float> [#uses=1]
  %mul9 = fmul float %tmp3, %call8                ; <float> [#uses=1]
  %tmp10 = load float* %t                         ; <float> [#uses=1]
  %mul11 = fmul float %mul9, %tmp10               ; <float> [#uses=1]
  %tmp12 = load float* %c1                        ; <float> [#uses=1]
  %tmp13 = load float* %t                         ; <float> [#uses=1]
  %tmp14 = load float* %c2                        ; <float> [#uses=1]
  %tmp15 = load float* %t                         ; <float> [#uses=1]
  %tmp16 = load float* %c3                        ; <float> [#uses=1]
  %tmp17 = load float* %t                         ; <float> [#uses=1]
  %tmp18 = load float* %c4                        ; <float> [#uses=1]
  %tmp19 = load float* %t                         ; <float> [#uses=1]
  %tmp20 = load float* %c5                        ; <float> [#uses=1]
  %mul21 = fmul float %tmp19, %tmp20              ; <float> [#uses=1]
  %add22 = fadd float %tmp18, %mul21              ; <float> [#uses=1]
  %mul23 = fmul float %tmp17, %add22              ; <float> [#uses=1]
  %add24 = fadd float %tmp16, %mul23              ; <float> [#uses=1]
  %mul25 = fmul float %tmp15, %add24              ; <float> [#uses=1]
  %add26 = fadd float %tmp14, %mul25              ; <float> [#uses=1]
  %mul27 = fmul float %tmp13, %add26              ; <float> [#uses=1]
  %add28 = fadd float %tmp12, %mul27              ; <float> [#uses=1]
  %mul29 = fmul float %mul11, %add28              ; <float> [#uses=1]
  %sub = fsub float 1.000000e+000, %mul29         ; <float> [#uses=1]
  store float %sub, float* %y
  %tmp30 = load float* %X.addr                    ; <float> [#uses=1]
  %cmp31 = fcmp olt float %tmp30, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp31, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp32 = load float* %y                         ; <float> [#uses=1]
  %sub33 = fsub float 1.000000e+000, %tmp32       ; <float> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp34 = load float* %y                         ; <float> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi float [ %sub33, %cond.true ], [ %tmp34, %cond.false ] ; <float> [#uses=1]
  %tmp35 = load float** %phi.addr                 ; <float*> [#uses=1]
  store float %cond, float* %tmp35
  ret void
}

declare float @_Z4fabsf(float)

declare float @_Z3expf(float)

; CHECK: ret
define void @blackScholes_scalar(float addrspace(1)* %randArray, i32 %width, float addrspace(1)* %call, float addrspace(1)* %put) nounwind {
entry:
  %randArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %call.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %put.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %d1 = alloca float, align 4                     ; <float*> [#uses=4]
  %d2 = alloca float, align 4                     ; <float*> [#uses=3]
  %phiD1 = alloca float, align 4                  ; <float*> [#uses=4]
  %phiD2 = alloca float, align 4                  ; <float*> [#uses=4]
  %sigmaSqrtT = alloca float, align 4             ; <float*> [#uses=3]
  %KexpMinusRT = alloca float, align 4            ; <float*> [#uses=3]
  %xPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %yPos = alloca i32, align 4                     ; <i32*> [#uses=4]
  %two = alloca float, align 4                    ; <float*> [#uses=2]
  %inRand = alloca float, align 4                 ; <float*> [#uses=11]
  %S = alloca float, align 4                      ; <float*> [#uses=4]
  %K = alloca float, align 4                      ; <float*> [#uses=3]
  %T = alloca float, align 4                      ; <float*> [#uses=4]
  %R = alloca float, align 4                      ; <float*> [#uses=3]
  %sigmaVal = alloca float, align 4               ; <float*> [#uses=4]
  store float addrspace(1)* %randArray, float addrspace(1)** %randArray.addr
  store i32 %width, i32* %width.addr
  store float addrspace(1)* %call, float addrspace(1)** %call.addr
  store float addrspace(1)* %put, float addrspace(1)** %put.addr
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %xPos
  %call2 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call2, i32* %yPos
  store float 2.000000e+000, float* %two
  %tmp = load i32* %yPos                          ; <i32> [#uses=1]
  %tmp3 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp3                      ; <i32> [#uses=1]
  %tmp4 = load i32* %xPos                         ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp4                      ; <i32> [#uses=1]
  %tmp5 = load float addrspace(1)** %randArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp5, i32 %add ; <float addrspace(1)*> [#uses=1]
  %tmp6 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  store float %tmp6, float* %inRand
  %tmp8 = load float* %inRand                     ; <float> [#uses=1]
  %mul9 = fmul float 1.000000e+001, %tmp8         ; <float> [#uses=1]
  %tmp10 = load float* %inRand                    ; <float> [#uses=1]
  %sub = fsub float 1.000000e+000, %tmp10         ; <float> [#uses=1]
  %mul11 = fmul float 1.000000e+002, %sub         ; <float> [#uses=1]
  %add12 = fadd float %mul9, %mul11               ; <float> [#uses=1]
  store float %add12, float* %S
  %tmp14 = load float* %inRand                    ; <float> [#uses=1]
  %mul15 = fmul float 1.000000e+001, %tmp14       ; <float> [#uses=1]
  %tmp16 = load float* %inRand                    ; <float> [#uses=1]
  %sub17 = fsub float 1.000000e+000, %tmp16       ; <float> [#uses=1]
  %mul18 = fmul float 1.000000e+002, %sub17       ; <float> [#uses=1]
  %add19 = fadd float %mul15, %mul18              ; <float> [#uses=1]
  store float %add19, float* %K
  %tmp21 = load float* %inRand                    ; <float> [#uses=1]
  %mul22 = fmul float 1.000000e+000, %tmp21       ; <float> [#uses=1]
  %tmp23 = load float* %inRand                    ; <float> [#uses=1]
  %sub24 = fsub float 1.000000e+000, %tmp23       ; <float> [#uses=1]
  %mul25 = fmul float 1.000000e+001, %sub24       ; <float> [#uses=1]
  %add26 = fadd float %mul22, %mul25              ; <float> [#uses=1]
  store float %add26, float* %T
  %tmp28 = load float* %inRand                    ; <float> [#uses=1]
  %mul29 = fmul float 0x3F847AE140000000, %tmp28  ; <float> [#uses=1]
  %tmp30 = load float* %inRand                    ; <float> [#uses=1]
  %sub31 = fsub float 1.000000e+000, %tmp30       ; <float> [#uses=1]
  %mul32 = fmul float 0x3FA99999A0000000, %sub31  ; <float> [#uses=1]
  %add33 = fadd float %mul29, %mul32              ; <float> [#uses=1]
  store float %add33, float* %R
  %tmp35 = load float* %inRand                    ; <float> [#uses=1]
  %mul36 = fmul float 0x3F847AE140000000, %tmp35  ; <float> [#uses=1]
  %tmp37 = load float* %inRand                    ; <float> [#uses=1]
  %sub38 = fsub float 1.000000e+000, %tmp37       ; <float> [#uses=1]
  %mul39 = fmul float 0x3FB99999A0000000, %sub38  ; <float> [#uses=1]
  %add40 = fadd float %mul36, %mul39              ; <float> [#uses=1]
  store float %add40, float* %sigmaVal
  %tmp41 = load float* %sigmaVal                  ; <float> [#uses=1]
  %tmp42 = load float* %T                         ; <float> [#uses=1]
  %call43 = call float @_Z4sqrtf(float %tmp42)    ; <float> [#uses=1]
  %mul44 = fmul float %tmp41, %call43             ; <float> [#uses=1]
  store float %mul44, float* %sigmaSqrtT
  %tmp45 = load float* %S                         ; <float> [#uses=1]
  %tmp46 = load float* %K                         ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %tmp46     ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %tmp46 ; <float> [#uses=0]
  %div = fdiv float %tmp45, %tmp46                ; <float> [#uses=1]
  %call47 = call float @_Z3logf(float %div)       ; <float> [#uses=1]
  %tmp48 = load float* %R                         ; <float> [#uses=1]
  %tmp49 = load float* %sigmaVal                  ; <float> [#uses=1]
  %tmp50 = load float* %sigmaVal                  ; <float> [#uses=1]
  %mul51 = fmul float %tmp49, %tmp50              ; <float> [#uses=1]
  %tmp52 = load float* %two                       ; <float> [#uses=3]
  %cmp53 = fcmp oeq float 0.000000e+000, %tmp52   ; <i1> [#uses=1]
  %sel54 = select i1 %cmp53, float 1.000000e+000, float %tmp52 ; <float> [#uses=0]
  %div55 = fdiv float %mul51, %tmp52              ; <float> [#uses=1]
  %add56 = fadd float %tmp48, %div55              ; <float> [#uses=1]
  %tmp57 = load float* %T                         ; <float> [#uses=1]
  %mul58 = fmul float %add56, %tmp57              ; <float> [#uses=1]
  %add59 = fadd float %call47, %mul58             ; <float> [#uses=1]
  %tmp60 = load float* %sigmaSqrtT                ; <float> [#uses=3]
  %cmp61 = fcmp oeq float 0.000000e+000, %tmp60   ; <i1> [#uses=1]
  %sel62 = select i1 %cmp61, float 1.000000e+000, float %tmp60 ; <float> [#uses=0]
  %div63 = fdiv float %add59, %tmp60              ; <float> [#uses=1]
  store float %div63, float* %d1
  %tmp64 = load float* %d1                        ; <float> [#uses=1]
  %tmp65 = load float* %sigmaSqrtT                ; <float> [#uses=1]
  %sub66 = fsub float %tmp64, %tmp65              ; <float> [#uses=1]
  store float %sub66, float* %d2
  %tmp67 = load float* %K                         ; <float> [#uses=1]
  %tmp68 = load float* %R                         ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp68        ; <float> [#uses=1]
  %tmp69 = load float* %T                         ; <float> [#uses=1]
  %mul70 = fmul float %neg, %tmp69                ; <float> [#uses=1]
  %call71 = call float @_Z3expf(float %mul70)     ; <float> [#uses=1]
  %mul72 = fmul float %tmp67, %call71             ; <float> [#uses=1]
  store float %mul72, float* %KexpMinusRT
  %tmp73 = load float* %d1                        ; <float> [#uses=1]
  call void @phi_scalar(float %tmp73, float* %phiD1)
  %tmp74 = load float* %d2                        ; <float> [#uses=1]
  call void @phi_scalar(float %tmp74, float* %phiD2)
  %tmp75 = load float* %S                         ; <float> [#uses=1]
  %tmp76 = load float* %phiD1                     ; <float> [#uses=1]
  %mul77 = fmul float %tmp75, %tmp76              ; <float> [#uses=1]
  %tmp78 = load float* %KexpMinusRT               ; <float> [#uses=1]
  %tmp79 = load float* %phiD2                     ; <float> [#uses=1]
  %mul80 = fmul float %tmp78, %tmp79              ; <float> [#uses=1]
  %sub81 = fsub float %mul77, %mul80              ; <float> [#uses=1]
  %tmp82 = load i32* %yPos                        ; <i32> [#uses=1]
  %tmp83 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul84 = mul i32 %tmp82, %tmp83                 ; <i32> [#uses=1]
  %tmp85 = load i32* %xPos                        ; <i32> [#uses=1]
  %add86 = add i32 %mul84, %tmp85                 ; <i32> [#uses=1]
  %tmp87 = load float addrspace(1)** %call.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds float addrspace(1)* %tmp87, i32 %add86 ; <float addrspace(1)*> [#uses=1]
  store float %sub81, float addrspace(1)* %arrayidx88
  %tmp89 = load float* %d1                        ; <float> [#uses=1]
  %neg90 = fsub float -0.000000e+000, %tmp89      ; <float> [#uses=1]
  call void @phi_scalar(float %neg90, float* %phiD1)
  %tmp91 = load float* %d2                        ; <float> [#uses=1]
  %neg92 = fsub float -0.000000e+000, %tmp91      ; <float> [#uses=1]
  call void @phi_scalar(float %neg92, float* %phiD2)
  %tmp93 = load float* %KexpMinusRT               ; <float> [#uses=1]
  %tmp94 = load float* %phiD2                     ; <float> [#uses=1]
  %mul95 = fmul float %tmp93, %tmp94              ; <float> [#uses=1]
  %tmp96 = load float* %S                         ; <float> [#uses=1]
  %tmp97 = load float* %phiD1                     ; <float> [#uses=1]
  %mul98 = fmul float %tmp96, %tmp97              ; <float> [#uses=1]
  %sub99 = fsub float %mul95, %mul98              ; <float> [#uses=1]
  %tmp100 = load i32* %yPos                       ; <i32> [#uses=1]
  %tmp101 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul102 = mul i32 %tmp100, %tmp101              ; <i32> [#uses=1]
  %tmp103 = load i32* %xPos                       ; <i32> [#uses=1]
  %add104 = add i32 %mul102, %tmp103              ; <i32> [#uses=1]
  %tmp105 = load float addrspace(1)** %put.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx106 = getelementptr inbounds float addrspace(1)* %tmp105, i32 %add104 ; <float addrspace(1)*> [#uses=1]
  store float %sub99, float addrspace(1)* %arrayidx106
  ret void
}

declare float @_Z4sqrtf(float)

declare float @_Z3logf(float)
