; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlSobel.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@oneEighth = addrspace(1) constant <4 x float> <float 1.250000e-001, float 1.250000e-001, float 1.250000e-001, float 1.250000e-001>, align 16 ; <<4 x float> addrspace(1)*> [#uses=1]
@opencl_wlSobel_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_GPU_parameters = appending global [162 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const, uint const, uint const\00", section "llvm.metadata" ; <[162 x i8]*> [#uses=1]
@opencl_wlSobel_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_CPU_parameters = appending global [162 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const, uint const, uint const\00", section "llvm.metadata" ; <[162 x i8]*> [#uses=1]
@opencl_wlSobel_image2d_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_image2d_parameters = appending global [110 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const\00", section "llvm.metadata" ; <[110 x i8]*> [#uses=1]
@opencl_wlSobel_image2dNoFloat_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_image2dNoFloat_parameters = appending global [110 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const\00", section "llvm.metadata" ; <[110 x i8]*> [#uses=1]
@opencl_wlSobel_image2d_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_image2d_GPU_parameters = appending global [98 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, float const, float const, float const\00", section "llvm.metadata" ; <[98 x i8]*> [#uses=1]
@opencl_wlSobel_image2dNoEdge_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_image2dNoEdge_parameters = appending global [110 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const\00", section "llvm.metadata" ; <[110 x i8]*> [#uses=1]
@opencl_wlSobel_image2dUnrolled_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_image2dUnrolled_parameters = appending global [110 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const\00", section "llvm.metadata" ; <[110 x i8]*> [#uses=1]
@opencl_wlSobel_Optimized_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSobel_Optimized_CPU_parameters = appending global [162 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float const, float const, float const, uint const, uint const, uint const\00", section "llvm.metadata" ; <[162 x i8]*> [#uses=1]
@opencl_metadata = appending global [8 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, float, float, i32, i32, i32)* @wlSobel_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_GPU_locals to i8*), i8* getelementptr inbounds ([162 x i8]* @opencl_wlSobel_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, float, float, i32, i32, i32)* @wlSobel_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_CPU_locals to i8*), i8* getelementptr inbounds ([162 x i8]* @opencl_wlSobel_CPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, float, float, float, i32)* @wlSobel_image2d to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_image2d_locals to i8*), i8* getelementptr inbounds ([110 x i8]* @opencl_wlSobel_image2d_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, float, float, float, i32)* @wlSobel_image2dNoFloat to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_image2dNoFloat_locals to i8*), i8* getelementptr inbounds ([110 x i8]* @opencl_wlSobel_image2dNoFloat_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, float, float, float)* @wlSobel_image2d_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_image2d_GPU_locals to i8*), i8* getelementptr inbounds ([98 x i8]* @opencl_wlSobel_image2d_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, float, float, float, i32)* @wlSobel_image2dNoEdge to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_image2dNoEdge_locals to i8*), i8* getelementptr inbounds ([110 x i8]* @opencl_wlSobel_image2dNoEdge_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, float, float, float, i32)* @wlSobel_image2dUnrolled to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_image2dUnrolled_locals to i8*), i8* getelementptr inbounds ([110 x i8]* @opencl_wlSobel_image2dUnrolled_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, float, float, i32, i32, i32)* @wlSobel_Optimized_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSobel_Optimized_CPU_locals to i8*), i8* getelementptr inbounds ([162 x i8]* @opencl_wlSobel_Optimized_CPU_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[8 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @wlSobel_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=1]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %floatZero = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=9]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %dims = alloca i32, align 4                     ; <i32*> [#uses=1]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=10]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=6]
  %global_szx = alloca i32, align 4               ; <i32*> [#uses=4]
  %global_szy = alloca i32, align 4               ; <i32*> [#uses=2]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=1]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=5]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=4]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=4]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=2]
  %.compoundliteral127 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral182 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %floatZero
  %call = call i32 (...)* @get_work_dim()         ; <i32> [#uses=1]
  store i32 %call, i32* %dims
  %call3 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call3, i32* %globalIdx
  %call5 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call5, i32* %globalIdy
  %call7 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call7, i32* %global_szx
  %call9 = call i32 @get_global_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call9, i32* %global_szy
  %call11 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call11, i32* %groupIdx
  %call13 = call i32 @get_group_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call13, i32* %groupIdy
  %tmp15 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %tmp16 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp15, %tmp16                   ; <i32> [#uses=1]
  %tmp17 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp17                     ; <i32> [#uses=1]
  store i32 %add, i32* %sourceIndex
  %tmp19 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %sub = sub i32 %tmp19, 1                        ; <i32> [#uses=1]
  %tmp20 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul21 = mul i32 %sub, %tmp20                   ; <i32> [#uses=1]
  %tmp22 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add23 = add i32 %mul21, %tmp22                 ; <i32> [#uses=1]
  store i32 %add23, i32* %tmpIndex1
  %tmp25 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %add26 = add i32 %tmp25, 1                      ; <i32> [#uses=1]
  %tmp27 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul28 = mul i32 %add26, %tmp27                 ; <i32> [#uses=1]
  %tmp29 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add30 = add i32 %mul28, %tmp29                 ; <i32> [#uses=1]
  store i32 %add30, i32* %tmpIndex2
  %tmp32 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp33 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp33, i32 %tmp32 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp34 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp34, <4 x float>* %current
  %tmp36 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp36, <4 x float>* %m00
  %tmp38 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp38, <4 x float>* %m01
  %tmp40 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp40, <4 x float>* %m02
  %tmp42 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp42, <4 x float>* %m10
  %tmp44 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp44, <4 x float>* %m12
  %tmp46 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46, <4 x float>* %m20
  %tmp48 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp48, <4 x float>* %m21
  %tmp50 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp50, <4 x float>* %m22
  %tmp51 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %cmp = icmp ugt i32 %tmp51, 0                   ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp52 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %sub53 = sub i32 %tmp52, 1                      ; <i32> [#uses=1]
  %tmp54 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds <4 x float> addrspace(1)* %tmp54, i32 %sub53 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp56 = load <4 x float> addrspace(1)* %arrayidx55 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %m10
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp57 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %tmp58 = load i32* %global_szx                  ; <i32> [#uses=1]
  %sub59 = sub i32 %tmp58, 1                      ; <i32> [#uses=1]
  %cmp60 = icmp ult i32 %tmp57, %sub59            ; <i1> [#uses=1]
  br i1 %cmp60, label %if.then61, label %if.end67

if.then61:                                        ; preds = %if.end
  %tmp62 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %add63 = add i32 %tmp62, 1                      ; <i32> [#uses=1]
  %tmp64 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds <4 x float> addrspace(1)* %tmp64, i32 %add63 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp66 = load <4 x float> addrspace(1)* %arrayidx65 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp66, <4 x float>* %m12
  br label %if.end67

if.end67:                                         ; preds = %if.then61, %if.end
  %tmp68 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %cmp69 = icmp ugt i32 %tmp68, 0                 ; <i1> [#uses=1]
  br i1 %cmp69, label %if.then70, label %if.end95

if.then70:                                        ; preds = %if.end67
  %tmp71 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %tmp72 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds <4 x float> addrspace(1)* %tmp72, i32 %tmp71 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp74 = load <4 x float> addrspace(1)* %arrayidx73 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp74, <4 x float>* %m01
  %tmp75 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %cmp76 = icmp ugt i32 %tmp75, 0                 ; <i1> [#uses=1]
  br i1 %cmp76, label %if.then77, label %if.end83

if.then77:                                        ; preds = %if.then70
  %tmp78 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %sub79 = sub i32 %tmp78, 1                      ; <i32> [#uses=1]
  %tmp80 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx81 = getelementptr inbounds <4 x float> addrspace(1)* %tmp80, i32 %sub79 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp82 = load <4 x float> addrspace(1)* %arrayidx81 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp82, <4 x float>* %m00
  br label %if.end83

if.end83:                                         ; preds = %if.then77, %if.then70
  %tmp84 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %tmp85 = load i32* %global_szx                  ; <i32> [#uses=1]
  %sub86 = sub i32 %tmp85, 1                      ; <i32> [#uses=1]
  %cmp87 = icmp ult i32 %tmp84, %sub86            ; <i1> [#uses=1]
  br i1 %cmp87, label %if.then88, label %if.end94

if.then88:                                        ; preds = %if.end83
  %tmp89 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %add90 = add i32 %tmp89, 1                      ; <i32> [#uses=1]
  %tmp91 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx92 = getelementptr inbounds <4 x float> addrspace(1)* %tmp91, i32 %add90 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp93 = load <4 x float> addrspace(1)* %arrayidx92 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp93, <4 x float>* %m02
  br label %if.end94

if.end94:                                         ; preds = %if.then88, %if.end83
  br label %if.end95

if.end95:                                         ; preds = %if.end94, %if.end67
  %tmp96 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %tmp97 = load i32* %global_szy                  ; <i32> [#uses=1]
  %sub98 = sub i32 %tmp97, 1                      ; <i32> [#uses=1]
  %cmp99 = icmp ult i32 %tmp96, %sub98            ; <i1> [#uses=1]
  br i1 %cmp99, label %if.then100, label %if.end125

if.then100:                                       ; preds = %if.end95
  %tmp101 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp102 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds <4 x float> addrspace(1)* %tmp102, i32 %tmp101 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp104 = load <4 x float> addrspace(1)* %arrayidx103 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp104, <4 x float>* %m21
  %tmp105 = load i32* %globalIdx                  ; <i32> [#uses=1]
  %cmp106 = icmp ugt i32 %tmp105, 0               ; <i1> [#uses=1]
  br i1 %cmp106, label %if.then107, label %if.end113

if.then107:                                       ; preds = %if.then100
  %tmp108 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %sub109 = sub i32 %tmp108, 1                    ; <i32> [#uses=1]
  %tmp110 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx111 = getelementptr inbounds <4 x float> addrspace(1)* %tmp110, i32 %sub109 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp112 = load <4 x float> addrspace(1)* %arrayidx111 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp112, <4 x float>* %m20
  br label %if.end113

if.end113:                                        ; preds = %if.then107, %if.then100
  %tmp114 = load i32* %globalIdx                  ; <i32> [#uses=1]
  %tmp115 = load i32* %global_szx                 ; <i32> [#uses=1]
  %sub116 = sub i32 %tmp115, 1                    ; <i32> [#uses=1]
  %cmp117 = icmp ult i32 %tmp114, %sub116         ; <i1> [#uses=1]
  br i1 %cmp117, label %if.then118, label %if.end124

if.then118:                                       ; preds = %if.end113
  %tmp119 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add120 = add i32 %tmp119, 1                    ; <i32> [#uses=1]
  %tmp121 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx122 = getelementptr inbounds <4 x float> addrspace(1)* %tmp121, i32 %add120 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp123 = load <4 x float> addrspace(1)* %arrayidx122 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp123, <4 x float>* %m22
  br label %if.end124

if.end124:                                        ; preds = %if.then118, %if.end113
  br label %if.end125

if.end125:                                        ; preds = %if.end124, %if.end95
  %tmp128 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp129 = extractelement <4 x float> %tmp128, i32 0 ; <float> [#uses=1]
  %tmp130 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp131 = extractelement <4 x float> %tmp130, i32 0 ; <float> [#uses=1]
  %mul132 = fmul float 2.000000e+000, %tmp131     ; <float> [#uses=1]
  %add133 = fadd float %tmp129, %mul132           ; <float> [#uses=1]
  %tmp134 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp135 = extractelement <4 x float> %tmp134, i32 0 ; <float> [#uses=1]
  %add136 = fadd float %add133, %tmp135           ; <float> [#uses=1]
  %tmp137 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp138 = extractelement <4 x float> %tmp137, i32 0 ; <float> [#uses=1]
  %tmp139 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp140 = extractelement <4 x float> %tmp139, i32 0 ; <float> [#uses=1]
  %mul141 = fmul float 2.000000e+000, %tmp140     ; <float> [#uses=1]
  %add142 = fadd float %tmp138, %mul141           ; <float> [#uses=1]
  %tmp143 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp144 = extractelement <4 x float> %tmp143, i32 0 ; <float> [#uses=1]
  %add145 = fadd float %add142, %tmp144           ; <float> [#uses=1]
  %sub146 = fsub float %add136, %add145           ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %sub146, i32 0 ; <<2 x float>> [#uses=1]
  %tmp147 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp148 = extractelement <4 x float> %tmp147, i32 0 ; <float> [#uses=1]
  %tmp149 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp150 = extractelement <4 x float> %tmp149, i32 0 ; <float> [#uses=1]
  %mul151 = fmul float 2.000000e+000, %tmp150     ; <float> [#uses=1]
  %add152 = fadd float %tmp148, %mul151           ; <float> [#uses=1]
  %tmp153 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp154 = extractelement <4 x float> %tmp153, i32 0 ; <float> [#uses=1]
  %add155 = fadd float %add152, %tmp154           ; <float> [#uses=1]
  %tmp156 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp157 = extractelement <4 x float> %tmp156, i32 0 ; <float> [#uses=1]
  %tmp158 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp159 = extractelement <4 x float> %tmp158, i32 0 ; <float> [#uses=1]
  %mul160 = fmul float 2.000000e+000, %tmp159     ; <float> [#uses=1]
  %add161 = fadd float %tmp157, %mul160           ; <float> [#uses=1]
  %tmp162 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp163 = extractelement <4 x float> %tmp162, i32 0 ; <float> [#uses=1]
  %add164 = fadd float %add161, %tmp163           ; <float> [#uses=1]
  %sub165 = fsub float %add155, %add164           ; <float> [#uses=1]
  %vecinit166 = insertelement <2 x float> %vecinit, float %sub165, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit166, <2 x float>* %.compoundliteral127
  %tmp167 = load <2 x float>* %.compoundliteral127 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp167, <2 x float>* %sobelXY_interim
  %tmp169 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %mul170 = fmul <2 x float> %tmp169, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul170, <2 x float>* %sobelXY
  %tmp172 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call173 = call float @_Z6lengthDv2_f(<2 x float> %tmp172) ; <float> [#uses=1]
  store float %call173, float* %len
  %tmp175 = load float* %len                      ; <float> [#uses=1]
  %tmp176 = load float* %minVal.addr              ; <float> [#uses=1]
  %call177 = call float @_Z3maxff(float %tmp175, float %tmp176) ; <float> [#uses=1]
  store float %call177, float* %mag
  %tmp178 = load float* %mag                      ; <float> [#uses=1]
  %tmp179 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call180 = call float @_Z3minff(float %tmp178, float %tmp179) ; <float> [#uses=1]
  store float %call180, float* %mag
  %tmp183 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp184 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp185 = extractelement <2 x float> %tmp184, i32 0 ; <float> [#uses=1]
  %mul186 = fmul float %tmp183, %tmp185           ; <float> [#uses=1]
  %vecinit187 = insertelement <4 x float> undef, float %mul186, i32 0 ; <<4 x float>> [#uses=1]
  %tmp188 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp189 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp190 = extractelement <2 x float> %tmp189, i32 1 ; <float> [#uses=1]
  %mul191 = fmul float %tmp188, %tmp190           ; <float> [#uses=1]
  %vecinit192 = insertelement <4 x float> %vecinit187, float %mul191, i32 1 ; <<4 x float>> [#uses=1]
  %tmp193 = load float* %mag                      ; <float> [#uses=1]
  %vecinit194 = insertelement <4 x float> %vecinit192, float %tmp193, i32 2 ; <<4 x float>> [#uses=1]
  %tmp195 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp196 = extractelement <4 x float> %tmp195, i32 3 ; <float> [#uses=1]
  %vecinit197 = insertelement <4 x float> %vecinit194, float %tmp196, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit197, <4 x float>* %.compoundliteral182
  %tmp198 = load <4 x float>* %.compoundliteral182 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp198, <4 x float>* %outTmp
  %tmp199 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp200 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp201 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx202 = getelementptr inbounds <4 x float> addrspace(1)* %tmp201, i32 %tmp200 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp199, <4 x float> addrspace(1)* %arrayidx202
  ret void
}

declare i32 @get_work_dim(...)

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare i32 @get_group_id(i32)

declare float @_Z6lengthDv2_f(<2 x float>)

declare float @_Z3maxff(float, float)

declare float @_Z3minff(float, float)

; CHECK: ret
define void @wlSobel_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=9]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=4]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %floatZero = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=9]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %dims = alloca i32, align 4                     ; <i32*> [#uses=1]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  %localIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %localIdy = alloca i32, align 4                 ; <i32*> [#uses=1]
  %global_szx = alloca i32, align 4               ; <i32*> [#uses=3]
  %global_szy = alloca i32, align 4               ; <i32*> [#uses=3]
  %local_szx = alloca i32, align 4                ; <i32*> [#uses=1]
  %local_szy = alloca i32, align 4                ; <i32*> [#uses=1]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=1]
  %count_x = alloca i32, align 4                  ; <i32*> [#uses=2]
  %count_y = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index_x = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index_y = alloca i32, align 4                  ; <i32*> [#uses=8]
  %index_x_orig = alloca i32, align 4             ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  %j = alloca i32, align 4                        ; <i32*> [#uses=4]
  %index_x54 = alloca i32, align 4                ; <i32*> [#uses=12]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=5]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=4]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=4]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=3]
  %.compoundliteral176 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %.compoundliteral231 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral256 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %floatZero
  %call = call i32 (...)* @get_work_dim()         ; <i32> [#uses=1]
  store i32 %call, i32* %dims
  %call3 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call3, i32* %globalIdx
  %call5 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call5, i32* %globalIdy
  %call7 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call7, i32* %localIdx
  %call9 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call9, i32* %localIdy
  %call11 = call i32 @get_global_size(i32 0)      ; <i32> [#uses=1]
  store i32 %call11, i32* %global_szx
  %call13 = call i32 @get_global_size(i32 1)      ; <i32> [#uses=1]
  store i32 %call13, i32* %global_szy
  %call15 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call15, i32* %local_szx
  %call17 = call i32 @get_local_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call17, i32* %local_szy
  %call19 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call19, i32* %groupIdx
  %call21 = call i32 @get_group_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call21, i32* %groupIdy
  %tmp23 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp24 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp24                    ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp24        ; <i32> [#uses=1]
  %div = udiv i32 %tmp23, %sel                    ; <i32> [#uses=1]
  store i32 %div, i32* %count_x
  %tmp26 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp27 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp28 = icmp eq i32 0, %tmp27                  ; <i1> [#uses=1]
  %sel29 = select i1 %cmp28, i32 1, i32 %tmp27    ; <i32> [#uses=1]
  %div30 = udiv i32 %tmp26, %sel29                ; <i32> [#uses=1]
  store i32 %div30, i32* %count_y
  %tmp32 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp33 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp32, %tmp33                   ; <i32> [#uses=1]
  %tmp34 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp35 = icmp eq i32 0, %tmp34                  ; <i1> [#uses=1]
  %sel36 = select i1 %cmp35, i32 1, i32 %tmp34    ; <i32> [#uses=1]
  %div37 = udiv i32 %mul, %sel36                  ; <i32> [#uses=1]
  store i32 %div37, i32* %index_x
  %tmp39 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp40 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %mul41 = mul i32 %tmp39, %tmp40                 ; <i32> [#uses=1]
  %tmp42 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp43 = icmp eq i32 0, %tmp42                  ; <i1> [#uses=1]
  %sel44 = select i1 %cmp43, i32 1, i32 %tmp42    ; <i32> [#uses=1]
  %div45 = udiv i32 %mul41, %sel44                ; <i32> [#uses=1]
  store i32 %div45, i32* %index_y
  %tmp47 = load i32* %index_x                     ; <i32> [#uses=1]
  store i32 %tmp47, i32* %index_x_orig
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc280, %entry
  %tmp49 = load i32* %i                           ; <i32> [#uses=1]
  %tmp50 = load i32* %count_y                     ; <i32> [#uses=1]
  %cmp51 = icmp ult i32 %tmp49, %tmp50            ; <i1> [#uses=1]
  br i1 %cmp51, label %for.body, label %for.end285

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j
  %tmp55 = load i32* %index_x_orig                ; <i32> [#uses=1]
  store i32 %tmp55, i32* %index_x54
  br label %for.cond56

for.cond56:                                       ; preds = %for.inc, %for.body
  %tmp57 = load i32* %j                           ; <i32> [#uses=1]
  %tmp58 = load i32* %count_x                     ; <i32> [#uses=1]
  %cmp59 = icmp ult i32 %tmp57, %tmp58            ; <i1> [#uses=1]
  br i1 %cmp59, label %for.body60, label %for.end

for.body60:                                       ; preds = %for.cond56
  %tmp62 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp63 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul64 = mul i32 %tmp62, %tmp63                 ; <i32> [#uses=1]
  %tmp65 = load i32* %index_x54                   ; <i32> [#uses=1]
  %add = add i32 %mul64, %tmp65                   ; <i32> [#uses=1]
  store i32 %add, i32* %sourceIndex
  %tmp67 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub = sub i32 %tmp67, 1                        ; <i32> [#uses=1]
  %tmp68 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul69 = mul i32 %sub, %tmp68                   ; <i32> [#uses=1]
  %tmp70 = load i32* %index_x54                   ; <i32> [#uses=1]
  %add71 = add i32 %mul69, %tmp70                 ; <i32> [#uses=1]
  store i32 %add71, i32* %tmpIndex1
  %tmp73 = load i32* %index_y                     ; <i32> [#uses=1]
  %add74 = add i32 %tmp73, 1                      ; <i32> [#uses=1]
  %tmp75 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul76 = mul i32 %add74, %tmp75                 ; <i32> [#uses=1]
  %tmp77 = load i32* %index_x54                   ; <i32> [#uses=1]
  %add78 = add i32 %mul76, %tmp77                 ; <i32> [#uses=1]
  store i32 %add78, i32* %tmpIndex2
  %tmp80 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp81 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp81, i32 %tmp80 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp82 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp82, <4 x float>* %current
  %tmp84 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp84, <4 x float>* %m00
  %tmp86 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp86, <4 x float>* %m01
  %tmp88 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp88, <4 x float>* %m02
  %tmp90 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp90, <4 x float>* %m10
  %tmp92 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp92, <4 x float>* %m12
  %tmp94 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp94, <4 x float>* %m20
  %tmp96 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp96, <4 x float>* %m21
  %tmp98 = load <4 x float>* %floatZero           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp98, <4 x float>* %m22
  %tmp99 = load i32* %index_x54                   ; <i32> [#uses=1]
  %cmp100 = icmp ugt i32 %tmp99, 0                ; <i1> [#uses=1]
  br i1 %cmp100, label %if.then, label %if.end

if.then:                                          ; preds = %for.body60
  %tmp101 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %sub102 = sub i32 %tmp101, 1                    ; <i32> [#uses=1]
  %tmp103 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx104 = getelementptr inbounds <4 x float> addrspace(1)* %tmp103, i32 %sub102 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp105 = load <4 x float> addrspace(1)* %arrayidx104 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp105, <4 x float>* %m10
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body60
  %tmp106 = load i32* %index_x54                  ; <i32> [#uses=1]
  %tmp107 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub108 = sub i32 %tmp107, 1                    ; <i32> [#uses=1]
  %cmp109 = icmp ult i32 %tmp106, %sub108         ; <i1> [#uses=1]
  br i1 %cmp109, label %if.then110, label %if.end116

if.then110:                                       ; preds = %if.end
  %tmp111 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %add112 = add i32 %tmp111, 1                    ; <i32> [#uses=1]
  %tmp113 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds <4 x float> addrspace(1)* %tmp113, i32 %add112 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp115 = load <4 x float> addrspace(1)* %arrayidx114 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp115, <4 x float>* %m12
  br label %if.end116

if.end116:                                        ; preds = %if.then110, %if.end
  %tmp117 = load i32* %index_y                    ; <i32> [#uses=1]
  %cmp118 = icmp ugt i32 %tmp117, 0               ; <i1> [#uses=1]
  br i1 %cmp118, label %if.then119, label %if.end144

if.then119:                                       ; preds = %if.end116
  %tmp120 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp121 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx122 = getelementptr inbounds <4 x float> addrspace(1)* %tmp121, i32 %tmp120 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp123 = load <4 x float> addrspace(1)* %arrayidx122 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp123, <4 x float>* %m01
  %tmp124 = load i32* %index_x54                  ; <i32> [#uses=1]
  %cmp125 = icmp ugt i32 %tmp124, 0               ; <i1> [#uses=1]
  br i1 %cmp125, label %if.then126, label %if.end132

if.then126:                                       ; preds = %if.then119
  %tmp127 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %sub128 = sub i32 %tmp127, 1                    ; <i32> [#uses=1]
  %tmp129 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds <4 x float> addrspace(1)* %tmp129, i32 %sub128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp131 = load <4 x float> addrspace(1)* %arrayidx130 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp131, <4 x float>* %m00
  br label %if.end132

if.end132:                                        ; preds = %if.then126, %if.then119
  %tmp133 = load i32* %index_x54                  ; <i32> [#uses=1]
  %tmp134 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub135 = sub i32 %tmp134, 1                    ; <i32> [#uses=1]
  %cmp136 = icmp ult i32 %tmp133, %sub135         ; <i1> [#uses=1]
  br i1 %cmp136, label %if.then137, label %if.end143

if.then137:                                       ; preds = %if.end132
  %tmp138 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add139 = add i32 %tmp138, 1                    ; <i32> [#uses=1]
  %tmp140 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx141 = getelementptr inbounds <4 x float> addrspace(1)* %tmp140, i32 %add139 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp142 = load <4 x float> addrspace(1)* %arrayidx141 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp142, <4 x float>* %m02
  br label %if.end143

if.end143:                                        ; preds = %if.then137, %if.end132
  br label %if.end144

if.end144:                                        ; preds = %if.end143, %if.end116
  %tmp145 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp146 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub147 = sub i32 %tmp146, 1                    ; <i32> [#uses=1]
  %cmp148 = icmp ult i32 %tmp145, %sub147         ; <i1> [#uses=1]
  br i1 %cmp148, label %if.then149, label %if.end174

if.then149:                                       ; preds = %if.end144
  %tmp150 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp151 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx152 = getelementptr inbounds <4 x float> addrspace(1)* %tmp151, i32 %tmp150 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp153 = load <4 x float> addrspace(1)* %arrayidx152 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp153, <4 x float>* %m21
  %tmp154 = load i32* %index_x54                  ; <i32> [#uses=1]
  %cmp155 = icmp ugt i32 %tmp154, 0               ; <i1> [#uses=1]
  br i1 %cmp155, label %if.then156, label %if.end162

if.then156:                                       ; preds = %if.then149
  %tmp157 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %sub158 = sub i32 %tmp157, 1                    ; <i32> [#uses=1]
  %tmp159 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx160 = getelementptr inbounds <4 x float> addrspace(1)* %tmp159, i32 %sub158 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp161 = load <4 x float> addrspace(1)* %arrayidx160 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp161, <4 x float>* %m20
  br label %if.end162

if.end162:                                        ; preds = %if.then156, %if.then149
  %tmp163 = load i32* %index_x54                  ; <i32> [#uses=1]
  %tmp164 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub165 = sub i32 %tmp164, 1                    ; <i32> [#uses=1]
  %cmp166 = icmp ult i32 %tmp163, %sub165         ; <i1> [#uses=1]
  br i1 %cmp166, label %if.then167, label %if.end173

if.then167:                                       ; preds = %if.end162
  %tmp168 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add169 = add i32 %tmp168, 1                    ; <i32> [#uses=1]
  %tmp170 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx171 = getelementptr inbounds <4 x float> addrspace(1)* %tmp170, i32 %add169 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp172 = load <4 x float> addrspace(1)* %arrayidx171 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp172, <4 x float>* %m22
  br label %if.end173

if.end173:                                        ; preds = %if.then167, %if.end162
  br label %if.end174

if.end174:                                        ; preds = %if.end173, %if.end144
  %tmp177 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp178 = extractelement <4 x float> %tmp177, i32 0 ; <float> [#uses=1]
  %conv = fpext float %tmp178 to double           ; <double> [#uses=1]
  %tmp179 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp180 = extractelement <4 x float> %tmp179, i32 0 ; <float> [#uses=1]
  %conv181 = fpext float %tmp180 to double        ; <double> [#uses=1]
  %mul182 = fmul double 2.000000e+000, %conv181   ; <double> [#uses=1]
  %add183 = fadd double %conv, %mul182            ; <double> [#uses=1]
  %tmp184 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp185 = extractelement <4 x float> %tmp184, i32 0 ; <float> [#uses=1]
  %conv186 = fpext float %tmp185 to double        ; <double> [#uses=1]
  %add187 = fadd double %add183, %conv186         ; <double> [#uses=1]
  %tmp188 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp189 = extractelement <4 x float> %tmp188, i32 0 ; <float> [#uses=1]
  %conv190 = fpext float %tmp189 to double        ; <double> [#uses=1]
  %tmp191 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp192 = extractelement <4 x float> %tmp191, i32 0 ; <float> [#uses=1]
  %conv193 = fpext float %tmp192 to double        ; <double> [#uses=1]
  %mul194 = fmul double 2.000000e+000, %conv193   ; <double> [#uses=1]
  %add195 = fadd double %conv190, %mul194         ; <double> [#uses=1]
  %tmp196 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp197 = extractelement <4 x float> %tmp196, i32 0 ; <float> [#uses=1]
  %conv198 = fpext float %tmp197 to double        ; <double> [#uses=1]
  %add199 = fadd double %add195, %conv198         ; <double> [#uses=1]
  %sub200 = fsub double %add187, %add199          ; <double> [#uses=1]
  %conv201 = fptrunc double %sub200 to float      ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv201, i32 0 ; <<2 x float>> [#uses=1]
  %tmp202 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp203 = extractelement <4 x float> %tmp202, i32 0 ; <float> [#uses=1]
  %conv204 = fpext float %tmp203 to double        ; <double> [#uses=1]
  %tmp205 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp206 = extractelement <4 x float> %tmp205, i32 0 ; <float> [#uses=1]
  %conv207 = fpext float %tmp206 to double        ; <double> [#uses=1]
  %mul208 = fmul double 2.000000e+000, %conv207   ; <double> [#uses=1]
  %add209 = fadd double %conv204, %mul208         ; <double> [#uses=1]
  %tmp210 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp211 = extractelement <4 x float> %tmp210, i32 0 ; <float> [#uses=1]
  %conv212 = fpext float %tmp211 to double        ; <double> [#uses=1]
  %add213 = fadd double %add209, %conv212         ; <double> [#uses=1]
  %tmp214 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp215 = extractelement <4 x float> %tmp214, i32 0 ; <float> [#uses=1]
  %conv216 = fpext float %tmp215 to double        ; <double> [#uses=1]
  %tmp217 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp218 = extractelement <4 x float> %tmp217, i32 0 ; <float> [#uses=1]
  %conv219 = fpext float %tmp218 to double        ; <double> [#uses=1]
  %mul220 = fmul double 2.000000e+000, %conv219   ; <double> [#uses=1]
  %add221 = fadd double %conv216, %mul220         ; <double> [#uses=1]
  %tmp222 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp223 = extractelement <4 x float> %tmp222, i32 0 ; <float> [#uses=1]
  %conv224 = fpext float %tmp223 to double        ; <double> [#uses=1]
  %add225 = fadd double %add221, %conv224         ; <double> [#uses=1]
  %sub226 = fsub double %add213, %add225          ; <double> [#uses=1]
  %conv227 = fptrunc double %sub226 to float      ; <float> [#uses=1]
  %vecinit228 = insertelement <2 x float> %vecinit, float %conv227, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit228, <2 x float>* %.compoundliteral176
  %tmp229 = load <2 x float>* %.compoundliteral176 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp229, <2 x float>* %sobelXY_interim
  %tmp232 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp233 = extractelement <2 x float> %tmp232, i32 0 ; <float> [#uses=1]
  %conv234 = fpext float %tmp233 to double        ; <double> [#uses=1]
  %mul235 = fmul double %conv234, 1.250000e-001   ; <double> [#uses=1]
  %conv236 = fptrunc double %mul235 to float      ; <float> [#uses=1]
  %vecinit237 = insertelement <2 x float> undef, float %conv236, i32 0 ; <<2 x float>> [#uses=1]
  %tmp238 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp239 = extractelement <2 x float> %tmp238, i32 1 ; <float> [#uses=1]
  %conv240 = fpext float %tmp239 to double        ; <double> [#uses=1]
  %mul241 = fmul double %conv240, 1.250000e-001   ; <double> [#uses=1]
  %conv242 = fptrunc double %mul241 to float      ; <float> [#uses=1]
  %vecinit243 = insertelement <2 x float> %vecinit237, float %conv242, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit243, <2 x float>* %.compoundliteral231
  %tmp244 = load <2 x float>* %.compoundliteral231 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp244, <2 x float>* %sobelXY
  %tmp246 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call247 = call float @_Z6lengthDv2_f(<2 x float> %tmp246) ; <float> [#uses=1]
  store float %call247, float* %len
  %tmp249 = load float* %len                      ; <float> [#uses=1]
  %tmp250 = load float* %minVal.addr              ; <float> [#uses=1]
  %call251 = call float @_Z3maxff(float %tmp249, float %tmp250) ; <float> [#uses=1]
  store float %call251, float* %mag
  %tmp252 = load float* %mag                      ; <float> [#uses=1]
  %tmp253 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call254 = call float @_Z3minff(float %tmp252, float %tmp253) ; <float> [#uses=1]
  store float %call254, float* %mag
  %tmp257 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp258 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp259 = extractelement <2 x float> %tmp258, i32 0 ; <float> [#uses=1]
  %mul260 = fmul float %tmp257, %tmp259           ; <float> [#uses=1]
  %vecinit261 = insertelement <4 x float> undef, float %mul260, i32 0 ; <<4 x float>> [#uses=1]
  %tmp262 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp263 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp264 = extractelement <2 x float> %tmp263, i32 1 ; <float> [#uses=1]
  %mul265 = fmul float %tmp262, %tmp264           ; <float> [#uses=1]
  %vecinit266 = insertelement <4 x float> %vecinit261, float %mul265, i32 1 ; <<4 x float>> [#uses=1]
  %tmp267 = load float* %mag                      ; <float> [#uses=1]
  %vecinit268 = insertelement <4 x float> %vecinit266, float %tmp267, i32 2 ; <<4 x float>> [#uses=1]
  %tmp269 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp270 = extractelement <4 x float> %tmp269, i32 3 ; <float> [#uses=1]
  %vecinit271 = insertelement <4 x float> %vecinit268, float %tmp270, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit271, <4 x float>* %.compoundliteral256
  %tmp272 = load <4 x float>* %.compoundliteral256 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp272, <4 x float>* %outTmp
  %tmp273 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp274 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp275 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx276 = getelementptr inbounds <4 x float> addrspace(1)* %tmp275, i32 %tmp274 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp273, <4 x float> addrspace(1)* %arrayidx276
  br label %for.inc

for.inc:                                          ; preds = %if.end174
  %tmp277 = load i32* %j                          ; <i32> [#uses=1]
  %inc = add i32 %tmp277, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  %tmp278 = load i32* %index_x54                  ; <i32> [#uses=1]
  %inc279 = add i32 %tmp278, 1                    ; <i32> [#uses=1]
  store i32 %inc279, i32* %index_x54
  br label %for.cond56

for.end:                                          ; preds = %for.cond56
  br label %for.inc280

for.inc280:                                       ; preds = %for.end
  %tmp281 = load i32* %i                          ; <i32> [#uses=1]
  %inc282 = add i32 %tmp281, 1                    ; <i32> [#uses=1]
  store i32 %inc282, i32* %i
  %tmp283 = load i32* %index_y                    ; <i32> [#uses=1]
  %inc284 = add i32 %tmp283, 1                    ; <i32> [#uses=1]
  store i32 %inc284, i32* %index_y
  br label %for.cond

for.end285:                                       ; preds = %for.cond
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

; CHECK: ret
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store i32 1, i32* %samplerLinear
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load <2 x float>* %outCrd.addr          ; <<2 x float>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %tmp, i32 1, <2 x float> %tmp1) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp2 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp2, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t*, i32, <2 x float>)

; CHECK: ret
define void @wlSobel_image2d(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=11]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=15]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %current_Id = alloca <2 x float>, align 8       ; <<2 x float>*> [#uses=5]
  %m10_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m12_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m01_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m00_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m02_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m21_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m20_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %m22_Id = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %col = alloca i32, align 4                      ; <i32*> [#uses=13]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=3]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %.compoundliteral221 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral246 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call4 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp3) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call4, <2 x i32>* %imgSize
  %tmp6 = load i32* %row                          ; <i32> [#uses=1]
  %tmp7 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add nsw i32 %tmp6, %tmp7                 ; <i32> [#uses=1]
  %tmp8 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %call10 = call i32 @_Z3minii(i32 %add, i32 %tmp9) ; <i32> [#uses=1]
  store i32 %call10, i32* %lastRow
  %tmp12 = load i32* %row                         ; <i32> [#uses=1]
  %tmp13 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp14 = extractelement <2 x i32> %tmp13, i32 0 ; <i32> [#uses=1]
  %mul15 = mul i32 %tmp12, %tmp14                 ; <i32> [#uses=1]
  store i32 %mul15, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc268, %entry
  %tmp34 = load i32* %row                         ; <i32> [#uses=1]
  %tmp35 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp34, %tmp35              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end271

for.body:                                         ; preds = %for.cond
  %tmp36 = load i32* %row                         ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp36 to float              ; <float> [#uses=1]
  %tmp37 = load <2 x float>* %current_Id          ; <<2 x float>> [#uses=1]
  %tmp38 = insertelement <2 x float> %tmp37, float %conv, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp38, <2 x float>* %current_Id
  %tmp39 = load i32* %row                         ; <i32> [#uses=1]
  %conv40 = sitofp i32 %tmp39 to float            ; <float> [#uses=1]
  %tmp41 = load <2 x float>* %m10_Id              ; <<2 x float>> [#uses=1]
  %tmp42 = insertelement <2 x float> %tmp41, float %conv40, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp42, <2 x float>* %m10_Id
  %tmp43 = load i32* %row                         ; <i32> [#uses=1]
  %conv44 = sitofp i32 %tmp43 to float            ; <float> [#uses=1]
  %tmp45 = load <2 x float>* %m12_Id              ; <<2 x float>> [#uses=1]
  %tmp46 = insertelement <2 x float> %tmp45, float %conv44, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp46, <2 x float>* %m12_Id
  %tmp47 = load i32* %row                         ; <i32> [#uses=1]
  %conv48 = sitofp i32 %tmp47 to double           ; <double> [#uses=1]
  %sub = fsub double %conv48, 1.000000e+000       ; <double> [#uses=1]
  %conv49 = fptrunc double %sub to float          ; <float> [#uses=1]
  %tmp50 = load <2 x float>* %m01_Id              ; <<2 x float>> [#uses=1]
  %tmp51 = insertelement <2 x float> %tmp50, float %conv49, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp51, <2 x float>* %m01_Id
  %tmp52 = load i32* %row                         ; <i32> [#uses=1]
  %conv53 = sitofp i32 %tmp52 to double           ; <double> [#uses=1]
  %sub54 = fsub double %conv53, 1.000000e+000     ; <double> [#uses=1]
  %conv55 = fptrunc double %sub54 to float        ; <float> [#uses=1]
  %tmp56 = load <2 x float>* %m00_Id              ; <<2 x float>> [#uses=1]
  %tmp57 = insertelement <2 x float> %tmp56, float %conv55, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp57, <2 x float>* %m00_Id
  %tmp58 = load i32* %row                         ; <i32> [#uses=1]
  %conv59 = sitofp i32 %tmp58 to double           ; <double> [#uses=1]
  %sub60 = fsub double %conv59, 1.000000e+000     ; <double> [#uses=1]
  %conv61 = fptrunc double %sub60 to float        ; <float> [#uses=1]
  %tmp62 = load <2 x float>* %m02_Id              ; <<2 x float>> [#uses=1]
  %tmp63 = insertelement <2 x float> %tmp62, float %conv61, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp63, <2 x float>* %m02_Id
  %tmp64 = load i32* %row                         ; <i32> [#uses=1]
  %conv65 = sitofp i32 %tmp64 to double           ; <double> [#uses=1]
  %add66 = fadd double %conv65, 1.000000e+000     ; <double> [#uses=1]
  %conv67 = fptrunc double %add66 to float        ; <float> [#uses=1]
  %tmp68 = load <2 x float>* %m21_Id              ; <<2 x float>> [#uses=1]
  %tmp69 = insertelement <2 x float> %tmp68, float %conv67, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp69, <2 x float>* %m21_Id
  %tmp70 = load i32* %row                         ; <i32> [#uses=1]
  %conv71 = sitofp i32 %tmp70 to double           ; <double> [#uses=1]
  %add72 = fadd double %conv71, 1.000000e+000     ; <double> [#uses=1]
  %conv73 = fptrunc double %add72 to float        ; <float> [#uses=1]
  %tmp74 = load <2 x float>* %m20_Id              ; <<2 x float>> [#uses=1]
  %tmp75 = insertelement <2 x float> %tmp74, float %conv73, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp75, <2 x float>* %m20_Id
  %tmp76 = load i32* %row                         ; <i32> [#uses=1]
  %conv77 = sitofp i32 %tmp76 to double           ; <double> [#uses=1]
  %add78 = fadd double %conv77, 1.000000e+000     ; <double> [#uses=1]
  %conv79 = fptrunc double %add78 to float        ; <float> [#uses=1]
  %tmp80 = load <2 x float>* %m22_Id              ; <<2 x float>> [#uses=1]
  %tmp81 = insertelement <2 x float> %tmp80, float %conv79, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp81, <2 x float>* %m22_Id
  store i32 0, i32* %col
  br label %for.cond83

for.cond83:                                       ; preds = %for.inc, %for.body
  %tmp84 = load i32* %col                         ; <i32> [#uses=1]
  %tmp85 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp86 = extractelement <2 x i32> %tmp85, i32 0 ; <i32> [#uses=1]
  %cmp87 = icmp slt i32 %tmp84, %tmp86            ; <i1> [#uses=1]
  br i1 %cmp87, label %for.body89, label %for.end

for.body89:                                       ; preds = %for.cond83
  %tmp90 = load i32* %col                         ; <i32> [#uses=1]
  %conv91 = sitofp i32 %tmp90 to float            ; <float> [#uses=1]
  %tmp92 = load <2 x float>* %current_Id          ; <<2 x float>> [#uses=1]
  %tmp93 = insertelement <2 x float> %tmp92, float %conv91, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp93, <2 x float>* %current_Id
  %tmp94 = load i32* %col                         ; <i32> [#uses=1]
  %conv95 = sitofp i32 %tmp94 to double           ; <double> [#uses=1]
  %sub96 = fsub double %conv95, 1.000000e+000     ; <double> [#uses=1]
  %conv97 = fptrunc double %sub96 to float        ; <float> [#uses=1]
  %tmp98 = load <2 x float>* %m10_Id              ; <<2 x float>> [#uses=1]
  %tmp99 = insertelement <2 x float> %tmp98, float %conv97, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp99, <2 x float>* %m10_Id
  %tmp100 = load i32* %col                        ; <i32> [#uses=1]
  %conv101 = sitofp i32 %tmp100 to double         ; <double> [#uses=1]
  %add102 = fadd double %conv101, 1.000000e+000   ; <double> [#uses=1]
  %conv103 = fptrunc double %add102 to float      ; <float> [#uses=1]
  %tmp104 = load <2 x float>* %m12_Id             ; <<2 x float>> [#uses=1]
  %tmp105 = insertelement <2 x float> %tmp104, float %conv103, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp105, <2 x float>* %m12_Id
  %tmp106 = load i32* %col                        ; <i32> [#uses=1]
  %conv107 = sitofp i32 %tmp106 to float          ; <float> [#uses=1]
  %tmp108 = load <2 x float>* %m01_Id             ; <<2 x float>> [#uses=1]
  %tmp109 = insertelement <2 x float> %tmp108, float %conv107, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp109, <2 x float>* %m01_Id
  %tmp110 = load i32* %col                        ; <i32> [#uses=1]
  %conv111 = sitofp i32 %tmp110 to double         ; <double> [#uses=1]
  %sub112 = fsub double %conv111, 1.000000e+000   ; <double> [#uses=1]
  %conv113 = fptrunc double %sub112 to float      ; <float> [#uses=1]
  %tmp114 = load <2 x float>* %m00_Id             ; <<2 x float>> [#uses=1]
  %tmp115 = insertelement <2 x float> %tmp114, float %conv113, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp115, <2 x float>* %m00_Id
  %tmp116 = load i32* %col                        ; <i32> [#uses=1]
  %conv117 = sitofp i32 %tmp116 to double         ; <double> [#uses=1]
  %add118 = fadd double %conv117, 1.000000e+000   ; <double> [#uses=1]
  %conv119 = fptrunc double %add118 to float      ; <float> [#uses=1]
  %tmp120 = load <2 x float>* %m02_Id             ; <<2 x float>> [#uses=1]
  %tmp121 = insertelement <2 x float> %tmp120, float %conv119, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp121, <2 x float>* %m02_Id
  %tmp122 = load i32* %col                        ; <i32> [#uses=1]
  %conv123 = sitofp i32 %tmp122 to float          ; <float> [#uses=1]
  %tmp124 = load <2 x float>* %m21_Id             ; <<2 x float>> [#uses=1]
  %tmp125 = insertelement <2 x float> %tmp124, float %conv123, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp125, <2 x float>* %m21_Id
  %tmp126 = load i32* %col                        ; <i32> [#uses=1]
  %conv127 = sitofp i32 %tmp126 to double         ; <double> [#uses=1]
  %sub128 = fsub double %conv127, 1.000000e+000   ; <double> [#uses=1]
  %conv129 = fptrunc double %sub128 to float      ; <float> [#uses=1]
  %tmp130 = load <2 x float>* %m20_Id             ; <<2 x float>> [#uses=1]
  %tmp131 = insertelement <2 x float> %tmp130, float %conv129, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp131, <2 x float>* %m20_Id
  %tmp132 = load i32* %col                        ; <i32> [#uses=1]
  %conv133 = sitofp i32 %tmp132 to double         ; <double> [#uses=1]
  %add134 = fadd double %conv133, 1.000000e+000   ; <double> [#uses=1]
  %conv135 = fptrunc double %add134 to float      ; <float> [#uses=1]
  %tmp136 = load <2 x float>* %m22_Id             ; <<2 x float>> [#uses=1]
  %tmp137 = insertelement <2 x float> %tmp136, float %conv135, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp137, <2 x float>* %m22_Id
  %tmp138 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp139 = load <2 x float>* %current_Id         ; <<2 x float>> [#uses=1]
  %call140 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp138, <2 x float> %tmp139) ; <<4 x float>> [#uses=1]
  store <4 x float> %call140, <4 x float>* %current
  %tmp141 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp142 = load <2 x float>* %m10_Id             ; <<2 x float>> [#uses=1]
  %call143 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp141, <2 x float> %tmp142) ; <<4 x float>> [#uses=1]
  store <4 x float> %call143, <4 x float>* %m10
  %tmp144 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp145 = load <2 x float>* %m12_Id             ; <<2 x float>> [#uses=1]
  %call146 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp144, <2 x float> %tmp145) ; <<4 x float>> [#uses=1]
  store <4 x float> %call146, <4 x float>* %m12
  %tmp147 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp148 = load <2 x float>* %m01_Id             ; <<2 x float>> [#uses=1]
  %call149 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp147, <2 x float> %tmp148) ; <<4 x float>> [#uses=1]
  store <4 x float> %call149, <4 x float>* %m01
  %tmp150 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp151 = load <2 x float>* %m00_Id             ; <<2 x float>> [#uses=1]
  %call152 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp150, <2 x float> %tmp151) ; <<4 x float>> [#uses=1]
  store <4 x float> %call152, <4 x float>* %m00
  %tmp153 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp154 = load <2 x float>* %m02_Id             ; <<2 x float>> [#uses=1]
  %call155 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp153, <2 x float> %tmp154) ; <<4 x float>> [#uses=1]
  store <4 x float> %call155, <4 x float>* %m02
  %tmp156 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp157 = load <2 x float>* %m21_Id             ; <<2 x float>> [#uses=1]
  %call158 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp156, <2 x float> %tmp157) ; <<4 x float>> [#uses=1]
  store <4 x float> %call158, <4 x float>* %m21
  %tmp159 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp160 = load <2 x float>* %m20_Id             ; <<2 x float>> [#uses=1]
  %call161 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp159, <2 x float> %tmp160) ; <<4 x float>> [#uses=1]
  store <4 x float> %call161, <4 x float>* %m20
  %tmp162 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp163 = load <2 x float>* %m22_Id             ; <<2 x float>> [#uses=1]
  %call164 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp162, <2 x float> %tmp163) ; <<4 x float>> [#uses=1]
  store <4 x float> %call164, <4 x float>* %m22
  %tmp166 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp167 = extractelement <4 x float> %tmp166, i32 0 ; <float> [#uses=1]
  %conv168 = fpext float %tmp167 to double        ; <double> [#uses=1]
  %tmp169 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp170 = extractelement <4 x float> %tmp169, i32 0 ; <float> [#uses=1]
  %conv171 = fpext float %tmp170 to double        ; <double> [#uses=1]
  %mul172 = fmul double 2.000000e+000, %conv171   ; <double> [#uses=1]
  %add173 = fadd double %conv168, %mul172         ; <double> [#uses=1]
  %tmp174 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp175 = extractelement <4 x float> %tmp174, i32 0 ; <float> [#uses=1]
  %conv176 = fpext float %tmp175 to double        ; <double> [#uses=1]
  %add177 = fadd double %add173, %conv176         ; <double> [#uses=1]
  %tmp178 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp179 = extractelement <4 x float> %tmp178, i32 0 ; <float> [#uses=1]
  %conv180 = fpext float %tmp179 to double        ; <double> [#uses=1]
  %tmp181 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp182 = extractelement <4 x float> %tmp181, i32 0 ; <float> [#uses=1]
  %conv183 = fpext float %tmp182 to double        ; <double> [#uses=1]
  %mul184 = fmul double 2.000000e+000, %conv183   ; <double> [#uses=1]
  %add185 = fadd double %conv180, %mul184         ; <double> [#uses=1]
  %tmp186 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp187 = extractelement <4 x float> %tmp186, i32 0 ; <float> [#uses=1]
  %conv188 = fpext float %tmp187 to double        ; <double> [#uses=1]
  %add189 = fadd double %add185, %conv188         ; <double> [#uses=1]
  %sub190 = fsub double %add177, %add189          ; <double> [#uses=1]
  %conv191 = fptrunc double %sub190 to float      ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv191, i32 0 ; <<2 x float>> [#uses=1]
  %tmp192 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp193 = extractelement <4 x float> %tmp192, i32 0 ; <float> [#uses=1]
  %conv194 = fpext float %tmp193 to double        ; <double> [#uses=1]
  %tmp195 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp196 = extractelement <4 x float> %tmp195, i32 0 ; <float> [#uses=1]
  %conv197 = fpext float %tmp196 to double        ; <double> [#uses=1]
  %mul198 = fmul double 2.000000e+000, %conv197   ; <double> [#uses=1]
  %add199 = fadd double %conv194, %mul198         ; <double> [#uses=1]
  %tmp200 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp201 = extractelement <4 x float> %tmp200, i32 0 ; <float> [#uses=1]
  %conv202 = fpext float %tmp201 to double        ; <double> [#uses=1]
  %add203 = fadd double %add199, %conv202         ; <double> [#uses=1]
  %tmp204 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp205 = extractelement <4 x float> %tmp204, i32 0 ; <float> [#uses=1]
  %conv206 = fpext float %tmp205 to double        ; <double> [#uses=1]
  %tmp207 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp208 = extractelement <4 x float> %tmp207, i32 0 ; <float> [#uses=1]
  %conv209 = fpext float %tmp208 to double        ; <double> [#uses=1]
  %mul210 = fmul double 2.000000e+000, %conv209   ; <double> [#uses=1]
  %add211 = fadd double %conv206, %mul210         ; <double> [#uses=1]
  %tmp212 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp213 = extractelement <4 x float> %tmp212, i32 0 ; <float> [#uses=1]
  %conv214 = fpext float %tmp213 to double        ; <double> [#uses=1]
  %add215 = fadd double %add211, %conv214         ; <double> [#uses=1]
  %sub216 = fsub double %add203, %add215          ; <double> [#uses=1]
  %conv217 = fptrunc double %sub216 to float      ; <float> [#uses=1]
  %vecinit218 = insertelement <2 x float> %vecinit, float %conv217, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit218, <2 x float>* %.compoundliteral
  %tmp219 = load <2 x float>* %.compoundliteral   ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp219, <2 x float>* %sobelXY_interim
  %tmp222 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp223 = extractelement <2 x float> %tmp222, i32 0 ; <float> [#uses=1]
  %conv224 = fpext float %tmp223 to double        ; <double> [#uses=1]
  %mul225 = fmul double %conv224, 1.250000e-001   ; <double> [#uses=1]
  %conv226 = fptrunc double %mul225 to float      ; <float> [#uses=1]
  %vecinit227 = insertelement <2 x float> undef, float %conv226, i32 0 ; <<2 x float>> [#uses=1]
  %tmp228 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp229 = extractelement <2 x float> %tmp228, i32 1 ; <float> [#uses=1]
  %conv230 = fpext float %tmp229 to double        ; <double> [#uses=1]
  %mul231 = fmul double %conv230, 1.250000e-001   ; <double> [#uses=1]
  %conv232 = fptrunc double %mul231 to float      ; <float> [#uses=1]
  %vecinit233 = insertelement <2 x float> %vecinit227, float %conv232, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit233, <2 x float>* %.compoundliteral221
  %tmp234 = load <2 x float>* %.compoundliteral221 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp234, <2 x float>* %sobelXY
  %tmp236 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call237 = call float @_Z6lengthDv2_f(<2 x float> %tmp236) ; <float> [#uses=1]
  store float %call237, float* %len
  %tmp239 = load float* %len                      ; <float> [#uses=1]
  %tmp240 = load float* %minVal.addr              ; <float> [#uses=1]
  %call241 = call float @_Z3maxff(float %tmp239, float %tmp240) ; <float> [#uses=1]
  store float %call241, float* %mag
  %tmp242 = load float* %mag                      ; <float> [#uses=1]
  %tmp243 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call244 = call float @_Z3minff(float %tmp242, float %tmp243) ; <float> [#uses=1]
  store float %call244, float* %mag
  %tmp247 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp248 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp249 = extractelement <2 x float> %tmp248, i32 0 ; <float> [#uses=1]
  %mul250 = fmul float %tmp247, %tmp249           ; <float> [#uses=1]
  %vecinit251 = insertelement <4 x float> undef, float %mul250, i32 0 ; <<4 x float>> [#uses=1]
  %tmp252 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp253 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp254 = extractelement <2 x float> %tmp253, i32 1 ; <float> [#uses=1]
  %mul255 = fmul float %tmp252, %tmp254           ; <float> [#uses=1]
  %vecinit256 = insertelement <4 x float> %vecinit251, float %mul255, i32 1 ; <<4 x float>> [#uses=1]
  %tmp257 = load float* %mag                      ; <float> [#uses=1]
  %vecinit258 = insertelement <4 x float> %vecinit256, float %tmp257, i32 2 ; <<4 x float>> [#uses=1]
  %tmp259 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp260 = extractelement <4 x float> %tmp259, i32 3 ; <float> [#uses=1]
  %vecinit261 = insertelement <4 x float> %vecinit258, float %tmp260, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit261, <4 x float>* %.compoundliteral246
  %tmp262 = load <4 x float>* %.compoundliteral246 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp262, <4 x float>* %outTmp
  %tmp263 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp264 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp264, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp265 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp265, i32 %tmp264 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp263, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body89
  %tmp266 = load i32* %col                        ; <i32> [#uses=1]
  %inc267 = add nsw i32 %tmp266, 1                ; <i32> [#uses=1]
  store i32 %inc267, i32* %col
  br label %for.cond83

for.end:                                          ; preds = %for.cond83
  br label %for.inc268

for.inc268:                                       ; preds = %for.end
  %tmp269 = load i32* %row                        ; <i32> [#uses=1]
  %inc270 = add nsw i32 %tmp269, 1                ; <i32> [#uses=1]
  store i32 %inc270, i32* %row
  br label %for.cond

for.end271:                                       ; preds = %for.cond
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare i32 @_Z3minii(i32, i32)

; CHECK: ret
define <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %inputImage, i32 %x, i32 %y) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %x.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %y.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <2 x i32>, align 8   ; <<2 x i32>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store i32 %x, i32* %x.addr
  store i32 %y, i32* %y.addr
  store i32 1, i32* %samplerLinear
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load i32* %x.addr                       ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp1, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp2 = load i32* %y.addr                       ; <i32> [#uses=1]
  %vecinit3 = insertelement <2 x i32> %vecinit, i32 %tmp2, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit3, <2 x i32>* %.compoundliteral
  %tmp4 = load <2 x i32>* %.compoundliteral       ; <<2 x i32>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %tmp, i32 1, <2 x i32> %tmp4) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp5 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp5, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t*, i32, <2 x i32>)

; CHECK: ret
define void @wlSobel_image2dNoFloat(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=11]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=15]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %col = alloca i32, align 4                      ; <i32*> [#uses=13]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=2]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral136 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call4 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp3) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call4, <2 x i32>* %imgSize
  %tmp6 = load i32* %row                          ; <i32> [#uses=1]
  %tmp7 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add nsw i32 %tmp6, %tmp7                 ; <i32> [#uses=1]
  %tmp8 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %call10 = call i32 @_Z3minii(i32 %add, i32 %tmp9) ; <i32> [#uses=1]
  store i32 %call10, i32* %lastRow
  %tmp12 = load i32* %row                         ; <i32> [#uses=1]
  %tmp13 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp14 = extractelement <2 x i32> %tmp13, i32 0 ; <i32> [#uses=1]
  %mul15 = mul i32 %tmp12, %tmp14                 ; <i32> [#uses=1]
  store i32 %mul15, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc158, %entry
  %tmp25 = load i32* %row                         ; <i32> [#uses=1]
  %tmp26 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp25, %tmp26              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end161

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %col
  br label %for.cond28

for.cond28:                                       ; preds = %for.inc, %for.body
  %tmp29 = load i32* %col                         ; <i32> [#uses=1]
  %tmp30 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp31 = extractelement <2 x i32> %tmp30, i32 0 ; <i32> [#uses=1]
  %cmp32 = icmp slt i32 %tmp29, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end

for.body33:                                       ; preds = %for.cond28
  %tmp34 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp35 = load i32* %col                         ; <i32> [#uses=1]
  %tmp36 = load i32* %row                         ; <i32> [#uses=1]
  %call37 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp34, i32 %tmp35, i32 %tmp36) ; <<4 x float>> [#uses=1]
  store <4 x float> %call37, <4 x float>* %current
  %tmp38 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp39 = load i32* %col                         ; <i32> [#uses=1]
  %sub = sub i32 %tmp39, 1                        ; <i32> [#uses=1]
  %tmp40 = load i32* %row                         ; <i32> [#uses=1]
  %call41 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp38, i32 %sub, i32 %tmp40) ; <<4 x float>> [#uses=1]
  store <4 x float> %call41, <4 x float>* %m10
  %tmp42 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp43 = load i32* %col                         ; <i32> [#uses=1]
  %add44 = add nsw i32 %tmp43, 1                  ; <i32> [#uses=1]
  %tmp45 = load i32* %row                         ; <i32> [#uses=1]
  %call46 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp42, i32 %add44, i32 %tmp45) ; <<4 x float>> [#uses=1]
  store <4 x float> %call46, <4 x float>* %m12
  %tmp47 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp48 = load i32* %col                         ; <i32> [#uses=1]
  %tmp49 = load i32* %row                         ; <i32> [#uses=1]
  %sub50 = sub i32 %tmp49, 1                      ; <i32> [#uses=1]
  %call51 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp47, i32 %tmp48, i32 %sub50) ; <<4 x float>> [#uses=1]
  store <4 x float> %call51, <4 x float>* %m01
  %tmp52 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp53 = load i32* %col                         ; <i32> [#uses=1]
  %sub54 = sub i32 %tmp53, 1                      ; <i32> [#uses=1]
  %tmp55 = load i32* %row                         ; <i32> [#uses=1]
  %sub56 = sub i32 %tmp55, 1                      ; <i32> [#uses=1]
  %call57 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp52, i32 %sub54, i32 %sub56) ; <<4 x float>> [#uses=1]
  store <4 x float> %call57, <4 x float>* %m00
  %tmp58 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp59 = load i32* %col                         ; <i32> [#uses=1]
  %add60 = add nsw i32 %tmp59, 1                  ; <i32> [#uses=1]
  %tmp61 = load i32* %row                         ; <i32> [#uses=1]
  %sub62 = sub i32 %tmp61, 1                      ; <i32> [#uses=1]
  %call63 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp58, i32 %add60, i32 %sub62) ; <<4 x float>> [#uses=1]
  store <4 x float> %call63, <4 x float>* %m02
  %tmp64 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp65 = load i32* %col                         ; <i32> [#uses=1]
  %tmp66 = load i32* %row                         ; <i32> [#uses=1]
  %add67 = add nsw i32 %tmp66, 1                  ; <i32> [#uses=1]
  %call68 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp64, i32 %tmp65, i32 %add67) ; <<4 x float>> [#uses=1]
  store <4 x float> %call68, <4 x float>* %m21
  %tmp69 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp70 = load i32* %col                         ; <i32> [#uses=1]
  %sub71 = sub i32 %tmp70, 1                      ; <i32> [#uses=1]
  %tmp72 = load i32* %row                         ; <i32> [#uses=1]
  %add73 = add nsw i32 %tmp72, 1                  ; <i32> [#uses=1]
  %call74 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp69, i32 %sub71, i32 %add73) ; <<4 x float>> [#uses=1]
  store <4 x float> %call74, <4 x float>* %m20
  %tmp75 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp76 = load i32* %col                         ; <i32> [#uses=1]
  %add77 = add nsw i32 %tmp76, 1                  ; <i32> [#uses=1]
  %tmp78 = load i32* %row                         ; <i32> [#uses=1]
  %add79 = add nsw i32 %tmp78, 1                  ; <i32> [#uses=1]
  %call80 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp75, i32 %add77, i32 %add79) ; <<4 x float>> [#uses=1]
  store <4 x float> %call80, <4 x float>* %m22
  %tmp82 = load <4 x float>* %m02                 ; <<4 x float>> [#uses=1]
  %tmp83 = extractelement <4 x float> %tmp82, i32 0 ; <float> [#uses=1]
  %tmp84 = load <4 x float>* %m12                 ; <<4 x float>> [#uses=1]
  %tmp85 = extractelement <4 x float> %tmp84, i32 0 ; <float> [#uses=1]
  %mul86 = fmul float 2.000000e+000, %tmp85       ; <float> [#uses=1]
  %add87 = fadd float %tmp83, %mul86              ; <float> [#uses=1]
  %tmp88 = load <4 x float>* %m22                 ; <<4 x float>> [#uses=1]
  %tmp89 = extractelement <4 x float> %tmp88, i32 0 ; <float> [#uses=1]
  %add90 = fadd float %add87, %tmp89              ; <float> [#uses=1]
  %tmp91 = load <4 x float>* %m00                 ; <<4 x float>> [#uses=1]
  %tmp92 = extractelement <4 x float> %tmp91, i32 0 ; <float> [#uses=1]
  %tmp93 = load <4 x float>* %m10                 ; <<4 x float>> [#uses=1]
  %tmp94 = extractelement <4 x float> %tmp93, i32 0 ; <float> [#uses=1]
  %mul95 = fmul float 2.000000e+000, %tmp94       ; <float> [#uses=1]
  %add96 = fadd float %tmp92, %mul95              ; <float> [#uses=1]
  %tmp97 = load <4 x float>* %m20                 ; <<4 x float>> [#uses=1]
  %tmp98 = extractelement <4 x float> %tmp97, i32 0 ; <float> [#uses=1]
  %add99 = fadd float %add96, %tmp98              ; <float> [#uses=1]
  %sub100 = fsub float %add90, %add99             ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %sub100, i32 0 ; <<2 x float>> [#uses=1]
  %tmp101 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp102 = extractelement <4 x float> %tmp101, i32 0 ; <float> [#uses=1]
  %tmp103 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp104 = extractelement <4 x float> %tmp103, i32 0 ; <float> [#uses=1]
  %mul105 = fmul float 2.000000e+000, %tmp104     ; <float> [#uses=1]
  %add106 = fadd float %tmp102, %mul105           ; <float> [#uses=1]
  %tmp107 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp108 = extractelement <4 x float> %tmp107, i32 0 ; <float> [#uses=1]
  %add109 = fadd float %add106, %tmp108           ; <float> [#uses=1]
  %tmp110 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp111 = extractelement <4 x float> %tmp110, i32 0 ; <float> [#uses=1]
  %tmp112 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp113 = extractelement <4 x float> %tmp112, i32 0 ; <float> [#uses=1]
  %mul114 = fmul float 2.000000e+000, %tmp113     ; <float> [#uses=1]
  %add115 = fadd float %tmp111, %mul114           ; <float> [#uses=1]
  %tmp116 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp117 = extractelement <4 x float> %tmp116, i32 0 ; <float> [#uses=1]
  %add118 = fadd float %add115, %tmp117           ; <float> [#uses=1]
  %sub119 = fsub float %add109, %add118           ; <float> [#uses=1]
  %vecinit120 = insertelement <2 x float> %vecinit, float %sub119, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit120, <2 x float>* %.compoundliteral
  %tmp121 = load <2 x float>* %.compoundliteral   ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp121, <2 x float>* %sobelXY_interim
  %tmp123 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %mul124 = fmul <2 x float> %tmp123, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul124, <2 x float>* %sobelXY
  %tmp126 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call127 = call float @_Z6lengthDv2_f(<2 x float> %tmp126) ; <float> [#uses=1]
  store float %call127, float* %len
  %tmp129 = load float* %len                      ; <float> [#uses=1]
  %tmp130 = load float* %minVal.addr              ; <float> [#uses=1]
  %call131 = call float @_Z3maxff(float %tmp129, float %tmp130) ; <float> [#uses=1]
  store float %call131, float* %mag
  %tmp132 = load float* %mag                      ; <float> [#uses=1]
  %tmp133 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call134 = call float @_Z3minff(float %tmp132, float %tmp133) ; <float> [#uses=1]
  store float %call134, float* %mag
  %tmp137 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp138 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp139 = extractelement <2 x float> %tmp138, i32 0 ; <float> [#uses=1]
  %mul140 = fmul float %tmp137, %tmp139           ; <float> [#uses=1]
  %vecinit141 = insertelement <4 x float> undef, float %mul140, i32 0 ; <<4 x float>> [#uses=1]
  %tmp142 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp143 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp144 = extractelement <2 x float> %tmp143, i32 1 ; <float> [#uses=1]
  %mul145 = fmul float %tmp142, %tmp144           ; <float> [#uses=1]
  %vecinit146 = insertelement <4 x float> %vecinit141, float %mul145, i32 1 ; <<4 x float>> [#uses=1]
  %tmp147 = load float* %mag                      ; <float> [#uses=1]
  %vecinit148 = insertelement <4 x float> %vecinit146, float %tmp147, i32 2 ; <<4 x float>> [#uses=1]
  %tmp149 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp150 = extractelement <4 x float> %tmp149, i32 3 ; <float> [#uses=1]
  %vecinit151 = insertelement <4 x float> %vecinit148, float %tmp150, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit151, <4 x float>* %.compoundliteral136
  %tmp152 = load <4 x float>* %.compoundliteral136 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp152, <4 x float>* %outTmp
  %tmp153 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp154 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp154, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp155 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp155, i32 %tmp154 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp153, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body33
  %tmp156 = load i32* %col                        ; <i32> [#uses=1]
  %inc157 = add nsw i32 %tmp156, 1                ; <i32> [#uses=1]
  store i32 %inc157, i32* %col
  br label %for.cond28

for.end:                                          ; preds = %for.cond28
  br label %for.inc158

for.inc158:                                       ; preds = %for.end
  %tmp159 = load i32* %row                        ; <i32> [#uses=1]
  %inc160 = add nsw i32 %tmp159, 1                ; <i32> [#uses=1]
  store i32 %inc160, i32* %row
  br label %for.cond

for.end161:                                       ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @wlSobel_image2d_GPU(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=11]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=2]
  %col = alloca i32, align 4                      ; <i32*> [#uses=11]
  %row = alloca i32, align 4                      ; <i32*> [#uses=11]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=2]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral121 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call, <2 x i32>* %imgSize
  %call2 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call2, i32* %col
  %call4 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call4, i32* %row
  %tmp6 = load i32* %row                          ; <i32> [#uses=1]
  %tmp7 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp8 = extractelement <2 x i32> %tmp7, i32 0   ; <i32> [#uses=1]
  %mul = mul i32 %tmp6, %tmp8                     ; <i32> [#uses=1]
  %tmp9 = load i32* %col                          ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp9                  ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp19 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp20 = load i32* %col                         ; <i32> [#uses=1]
  %tmp21 = load i32* %row                         ; <i32> [#uses=1]
  %call22 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp19, i32 %tmp20, i32 %tmp21) ; <<4 x float>> [#uses=1]
  store <4 x float> %call22, <4 x float>* %current
  %tmp23 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp24 = load i32* %col                         ; <i32> [#uses=1]
  %sub = sub i32 %tmp24, 1                        ; <i32> [#uses=1]
  %tmp25 = load i32* %row                         ; <i32> [#uses=1]
  %call26 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp23, i32 %sub, i32 %tmp25) ; <<4 x float>> [#uses=1]
  store <4 x float> %call26, <4 x float>* %m10
  %tmp27 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp28 = load i32* %col                         ; <i32> [#uses=1]
  %add29 = add nsw i32 %tmp28, 1                  ; <i32> [#uses=1]
  %tmp30 = load i32* %row                         ; <i32> [#uses=1]
  %call31 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp27, i32 %add29, i32 %tmp30) ; <<4 x float>> [#uses=1]
  store <4 x float> %call31, <4 x float>* %m12
  %tmp32 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp33 = load i32* %col                         ; <i32> [#uses=1]
  %tmp34 = load i32* %row                         ; <i32> [#uses=1]
  %sub35 = sub i32 %tmp34, 1                      ; <i32> [#uses=1]
  %call36 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp32, i32 %tmp33, i32 %sub35) ; <<4 x float>> [#uses=1]
  store <4 x float> %call36, <4 x float>* %m01
  %tmp37 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp38 = load i32* %col                         ; <i32> [#uses=1]
  %sub39 = sub i32 %tmp38, 1                      ; <i32> [#uses=1]
  %tmp40 = load i32* %row                         ; <i32> [#uses=1]
  %sub41 = sub i32 %tmp40, 1                      ; <i32> [#uses=1]
  %call42 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp37, i32 %sub39, i32 %sub41) ; <<4 x float>> [#uses=1]
  store <4 x float> %call42, <4 x float>* %m00
  %tmp43 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp44 = load i32* %col                         ; <i32> [#uses=1]
  %add45 = add nsw i32 %tmp44, 1                  ; <i32> [#uses=1]
  %tmp46 = load i32* %row                         ; <i32> [#uses=1]
  %sub47 = sub i32 %tmp46, 1                      ; <i32> [#uses=1]
  %call48 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp43, i32 %add45, i32 %sub47) ; <<4 x float>> [#uses=1]
  store <4 x float> %call48, <4 x float>* %m02
  %tmp49 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp50 = load i32* %col                         ; <i32> [#uses=1]
  %tmp51 = load i32* %row                         ; <i32> [#uses=1]
  %add52 = add nsw i32 %tmp51, 1                  ; <i32> [#uses=1]
  %call53 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp49, i32 %tmp50, i32 %add52) ; <<4 x float>> [#uses=1]
  store <4 x float> %call53, <4 x float>* %m21
  %tmp54 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp55 = load i32* %col                         ; <i32> [#uses=1]
  %sub56 = sub i32 %tmp55, 1                      ; <i32> [#uses=1]
  %tmp57 = load i32* %row                         ; <i32> [#uses=1]
  %add58 = add nsw i32 %tmp57, 1                  ; <i32> [#uses=1]
  %call59 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp54, i32 %sub56, i32 %add58) ; <<4 x float>> [#uses=1]
  store <4 x float> %call59, <4 x float>* %m20
  %tmp60 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp61 = load i32* %col                         ; <i32> [#uses=1]
  %add62 = add nsw i32 %tmp61, 1                  ; <i32> [#uses=1]
  %tmp63 = load i32* %row                         ; <i32> [#uses=1]
  %add64 = add nsw i32 %tmp63, 1                  ; <i32> [#uses=1]
  %call65 = call <4 x float> @evaluatePixelNoFloat(%struct._image2d_t* %tmp60, i32 %add62, i32 %add64) ; <<4 x float>> [#uses=1]
  store <4 x float> %call65, <4 x float>* %m22
  %tmp67 = load <4 x float>* %m02                 ; <<4 x float>> [#uses=1]
  %tmp68 = extractelement <4 x float> %tmp67, i32 0 ; <float> [#uses=1]
  %tmp69 = load <4 x float>* %m12                 ; <<4 x float>> [#uses=1]
  %tmp70 = extractelement <4 x float> %tmp69, i32 0 ; <float> [#uses=1]
  %mul71 = fmul float 2.000000e+000, %tmp70       ; <float> [#uses=1]
  %add72 = fadd float %tmp68, %mul71              ; <float> [#uses=1]
  %tmp73 = load <4 x float>* %m22                 ; <<4 x float>> [#uses=1]
  %tmp74 = extractelement <4 x float> %tmp73, i32 0 ; <float> [#uses=1]
  %add75 = fadd float %add72, %tmp74              ; <float> [#uses=1]
  %tmp76 = load <4 x float>* %m00                 ; <<4 x float>> [#uses=1]
  %tmp77 = extractelement <4 x float> %tmp76, i32 0 ; <float> [#uses=1]
  %tmp78 = load <4 x float>* %m10                 ; <<4 x float>> [#uses=1]
  %tmp79 = extractelement <4 x float> %tmp78, i32 0 ; <float> [#uses=1]
  %mul80 = fmul float 2.000000e+000, %tmp79       ; <float> [#uses=1]
  %add81 = fadd float %tmp77, %mul80              ; <float> [#uses=1]
  %tmp82 = load <4 x float>* %m20                 ; <<4 x float>> [#uses=1]
  %tmp83 = extractelement <4 x float> %tmp82, i32 0 ; <float> [#uses=1]
  %add84 = fadd float %add81, %tmp83              ; <float> [#uses=1]
  %sub85 = fsub float %add75, %add84              ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %sub85, i32 0 ; <<2 x float>> [#uses=1]
  %tmp86 = load <4 x float>* %m20                 ; <<4 x float>> [#uses=1]
  %tmp87 = extractelement <4 x float> %tmp86, i32 0 ; <float> [#uses=1]
  %tmp88 = load <4 x float>* %m21                 ; <<4 x float>> [#uses=1]
  %tmp89 = extractelement <4 x float> %tmp88, i32 0 ; <float> [#uses=1]
  %mul90 = fmul float 2.000000e+000, %tmp89       ; <float> [#uses=1]
  %add91 = fadd float %tmp87, %mul90              ; <float> [#uses=1]
  %tmp92 = load <4 x float>* %m22                 ; <<4 x float>> [#uses=1]
  %tmp93 = extractelement <4 x float> %tmp92, i32 0 ; <float> [#uses=1]
  %add94 = fadd float %add91, %tmp93              ; <float> [#uses=1]
  %tmp95 = load <4 x float>* %m00                 ; <<4 x float>> [#uses=1]
  %tmp96 = extractelement <4 x float> %tmp95, i32 0 ; <float> [#uses=1]
  %tmp97 = load <4 x float>* %m01                 ; <<4 x float>> [#uses=1]
  %tmp98 = extractelement <4 x float> %tmp97, i32 0 ; <float> [#uses=1]
  %mul99 = fmul float 2.000000e+000, %tmp98       ; <float> [#uses=1]
  %add100 = fadd float %tmp96, %mul99             ; <float> [#uses=1]
  %tmp101 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp102 = extractelement <4 x float> %tmp101, i32 0 ; <float> [#uses=1]
  %add103 = fadd float %add100, %tmp102           ; <float> [#uses=1]
  %sub104 = fsub float %add94, %add103            ; <float> [#uses=1]
  %vecinit105 = insertelement <2 x float> %vecinit, float %sub104, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit105, <2 x float>* %.compoundliteral
  %tmp106 = load <2 x float>* %.compoundliteral   ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp106, <2 x float>* %sobelXY_interim
  %tmp108 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %mul109 = fmul <2 x float> %tmp108, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul109, <2 x float>* %sobelXY
  %tmp111 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call112 = call float @_Z6lengthDv2_f(<2 x float> %tmp111) ; <float> [#uses=1]
  store float %call112, float* %len
  %tmp114 = load float* %len                      ; <float> [#uses=1]
  %tmp115 = load float* %minVal.addr              ; <float> [#uses=1]
  %call116 = call float @_Z3maxff(float %tmp114, float %tmp115) ; <float> [#uses=1]
  store float %call116, float* %mag
  %tmp117 = load float* %mag                      ; <float> [#uses=1]
  %tmp118 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call119 = call float @_Z3minff(float %tmp117, float %tmp118) ; <float> [#uses=1]
  store float %call119, float* %mag
  %tmp122 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp123 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp124 = extractelement <2 x float> %tmp123, i32 0 ; <float> [#uses=1]
  %mul125 = fmul float %tmp122, %tmp124           ; <float> [#uses=1]
  %vecinit126 = insertelement <4 x float> undef, float %mul125, i32 0 ; <<4 x float>> [#uses=1]
  %tmp127 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp128 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp129 = extractelement <2 x float> %tmp128, i32 1 ; <float> [#uses=1]
  %mul130 = fmul float %tmp127, %tmp129           ; <float> [#uses=1]
  %vecinit131 = insertelement <4 x float> %vecinit126, float %mul130, i32 1 ; <<4 x float>> [#uses=1]
  %tmp132 = load float* %mag                      ; <float> [#uses=1]
  %vecinit133 = insertelement <4 x float> %vecinit131, float %tmp132, i32 2 ; <<4 x float>> [#uses=1]
  %tmp134 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp135 = extractelement <4 x float> %tmp134, i32 3 ; <float> [#uses=1]
  %vecinit136 = insertelement <4 x float> %vecinit133, float %tmp135, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit136, <4 x float>* %.compoundliteral121
  %tmp137 = load <4 x float>* %.compoundliteral121 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp137, <4 x float>* %outTmp
  %tmp138 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp139 = load i32* %index                      ; <i32> [#uses=1]
  %tmp140 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp140, i32 %tmp139 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp138, <4 x float> addrspace(1)* %arrayidx
  ret void
}

; CHECK: ret
define <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %inputImage, i32 %x, i32 %y) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %x.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %y.addr = alloca i32, align 4                   ; <i32*> [#uses=2]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <2 x i32>, align 8   ; <<2 x i32>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store i32 %x, i32* %x.addr
  store i32 %y, i32* %y.addr
  store i32 0, i32* %samplerLinear
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load i32* %x.addr                       ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp1, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp2 = load i32* %y.addr                       ; <i32> [#uses=1]
  %vecinit3 = insertelement <2 x i32> %vecinit, i32 %tmp2, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit3, <2 x i32>* %.compoundliteral
  %tmp4 = load <2 x i32>* %.compoundliteral       ; <<2 x i32>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %tmp, i32 0, <2 x i32> %tmp4) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp5 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp5, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define void @wlSobel_image2dNoEdge(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=11]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=3]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=15]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %col = alloca i32, align 4                      ; <i32*> [#uses=13]
  %sobelXY_interim = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=3]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY = alloca <2 x float>, align 8          ; <<2 x float>*> [#uses=4]
  %.compoundliteral139 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %len = alloca float, align 4                    ; <float*> [#uses=2]
  %mag = alloca float, align 4                    ; <float*> [#uses=4]
  %outTmp = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral164 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  %add = add i32 %mul, 1                          ; <i32> [#uses=1]
  store i32 %add, i32* %row
  %tmp3 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call4 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp3) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call4, <2 x i32>* %imgSize
  %tmp6 = load i32* %row                          ; <i32> [#uses=1]
  %tmp7 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add8 = add nsw i32 %tmp6, %tmp7                ; <i32> [#uses=1]
  %tmp9 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp10 = extractelement <2 x i32> %tmp9, i32 1  ; <i32> [#uses=1]
  %sub = sub i32 %tmp10, 1                        ; <i32> [#uses=1]
  %call11 = call i32 @_Z3minii(i32 %add8, i32 %sub) ; <i32> [#uses=1]
  store i32 %call11, i32* %lastRow
  %tmp13 = load i32* %row                         ; <i32> [#uses=1]
  %tmp14 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 0 ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp13, %tmp15                 ; <i32> [#uses=1]
  store i32 %mul16, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc186, %entry
  %tmp26 = load i32* %row                         ; <i32> [#uses=1]
  %tmp27 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp26, %tmp27              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end189

for.body:                                         ; preds = %for.cond
  store i32 1, i32* %col
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc, %for.body
  %tmp30 = load i32* %col                         ; <i32> [#uses=1]
  %tmp31 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp32 = extractelement <2 x i32> %tmp31, i32 0 ; <i32> [#uses=1]
  %sub33 = sub i32 %tmp32, 2                      ; <i32> [#uses=1]
  %cmp34 = icmp slt i32 %tmp30, %sub33            ; <i1> [#uses=1]
  br i1 %cmp34, label %for.body35, label %for.end

for.body35:                                       ; preds = %for.cond29
  %tmp36 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp37 = load i32* %col                         ; <i32> [#uses=1]
  %tmp38 = load i32* %row                         ; <i32> [#uses=1]
  %call39 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp36, i32 %tmp37, i32 %tmp38) ; <<4 x float>> [#uses=1]
  store <4 x float> %call39, <4 x float>* %current
  %tmp40 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp41 = load i32* %col                         ; <i32> [#uses=1]
  %sub42 = sub i32 %tmp41, 1                      ; <i32> [#uses=1]
  %tmp43 = load i32* %row                         ; <i32> [#uses=1]
  %call44 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp40, i32 %sub42, i32 %tmp43) ; <<4 x float>> [#uses=1]
  store <4 x float> %call44, <4 x float>* %m10
  %tmp45 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp46 = load i32* %col                         ; <i32> [#uses=1]
  %add47 = add nsw i32 %tmp46, 1                  ; <i32> [#uses=1]
  %tmp48 = load i32* %row                         ; <i32> [#uses=1]
  %call49 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp45, i32 %add47, i32 %tmp48) ; <<4 x float>> [#uses=1]
  store <4 x float> %call49, <4 x float>* %m12
  %tmp50 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp51 = load i32* %col                         ; <i32> [#uses=1]
  %tmp52 = load i32* %row                         ; <i32> [#uses=1]
  %sub53 = sub i32 %tmp52, 1                      ; <i32> [#uses=1]
  %call54 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp50, i32 %tmp51, i32 %sub53) ; <<4 x float>> [#uses=1]
  store <4 x float> %call54, <4 x float>* %m01
  %tmp55 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp56 = load i32* %col                         ; <i32> [#uses=1]
  %sub57 = sub i32 %tmp56, 1                      ; <i32> [#uses=1]
  %tmp58 = load i32* %row                         ; <i32> [#uses=1]
  %sub59 = sub i32 %tmp58, 1                      ; <i32> [#uses=1]
  %call60 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp55, i32 %sub57, i32 %sub59) ; <<4 x float>> [#uses=1]
  store <4 x float> %call60, <4 x float>* %m00
  %tmp61 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp62 = load i32* %col                         ; <i32> [#uses=1]
  %add63 = add nsw i32 %tmp62, 1                  ; <i32> [#uses=1]
  %tmp64 = load i32* %row                         ; <i32> [#uses=1]
  %sub65 = sub i32 %tmp64, 1                      ; <i32> [#uses=1]
  %call66 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp61, i32 %add63, i32 %sub65) ; <<4 x float>> [#uses=1]
  store <4 x float> %call66, <4 x float>* %m02
  %tmp67 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp68 = load i32* %col                         ; <i32> [#uses=1]
  %tmp69 = load i32* %row                         ; <i32> [#uses=1]
  %add70 = add nsw i32 %tmp69, 1                  ; <i32> [#uses=1]
  %call71 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp67, i32 %tmp68, i32 %add70) ; <<4 x float>> [#uses=1]
  store <4 x float> %call71, <4 x float>* %m21
  %tmp72 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp73 = load i32* %col                         ; <i32> [#uses=1]
  %sub74 = sub i32 %tmp73, 1                      ; <i32> [#uses=1]
  %tmp75 = load i32* %row                         ; <i32> [#uses=1]
  %add76 = add nsw i32 %tmp75, 1                  ; <i32> [#uses=1]
  %call77 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp72, i32 %sub74, i32 %add76) ; <<4 x float>> [#uses=1]
  store <4 x float> %call77, <4 x float>* %m20
  %tmp78 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp79 = load i32* %col                         ; <i32> [#uses=1]
  %add80 = add nsw i32 %tmp79, 1                  ; <i32> [#uses=1]
  %tmp81 = load i32* %row                         ; <i32> [#uses=1]
  %add82 = add nsw i32 %tmp81, 1                  ; <i32> [#uses=1]
  %call83 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp78, i32 %add80, i32 %add82) ; <<4 x float>> [#uses=1]
  store <4 x float> %call83, <4 x float>* %m22
  %tmp85 = load <4 x float>* %m02                 ; <<4 x float>> [#uses=1]
  %tmp86 = extractelement <4 x float> %tmp85, i32 0 ; <float> [#uses=1]
  %conv = fpext float %tmp86 to double            ; <double> [#uses=1]
  %tmp87 = load <4 x float>* %m12                 ; <<4 x float>> [#uses=1]
  %tmp88 = extractelement <4 x float> %tmp87, i32 0 ; <float> [#uses=1]
  %conv89 = fpext float %tmp88 to double          ; <double> [#uses=1]
  %mul90 = fmul double 2.000000e+000, %conv89     ; <double> [#uses=1]
  %add91 = fadd double %conv, %mul90              ; <double> [#uses=1]
  %tmp92 = load <4 x float>* %m22                 ; <<4 x float>> [#uses=1]
  %tmp93 = extractelement <4 x float> %tmp92, i32 0 ; <float> [#uses=1]
  %conv94 = fpext float %tmp93 to double          ; <double> [#uses=1]
  %add95 = fadd double %add91, %conv94            ; <double> [#uses=1]
  %tmp96 = load <4 x float>* %m00                 ; <<4 x float>> [#uses=1]
  %tmp97 = extractelement <4 x float> %tmp96, i32 0 ; <float> [#uses=1]
  %conv98 = fpext float %tmp97 to double          ; <double> [#uses=1]
  %tmp99 = load <4 x float>* %m10                 ; <<4 x float>> [#uses=1]
  %tmp100 = extractelement <4 x float> %tmp99, i32 0 ; <float> [#uses=1]
  %conv101 = fpext float %tmp100 to double        ; <double> [#uses=1]
  %mul102 = fmul double 2.000000e+000, %conv101   ; <double> [#uses=1]
  %add103 = fadd double %conv98, %mul102          ; <double> [#uses=1]
  %tmp104 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp105 = extractelement <4 x float> %tmp104, i32 0 ; <float> [#uses=1]
  %conv106 = fpext float %tmp105 to double        ; <double> [#uses=1]
  %add107 = fadd double %add103, %conv106         ; <double> [#uses=1]
  %sub108 = fsub double %add95, %add107           ; <double> [#uses=1]
  %conv109 = fptrunc double %sub108 to float      ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv109, i32 0 ; <<2 x float>> [#uses=1]
  %tmp110 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp111 = extractelement <4 x float> %tmp110, i32 0 ; <float> [#uses=1]
  %conv112 = fpext float %tmp111 to double        ; <double> [#uses=1]
  %tmp113 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp114 = extractelement <4 x float> %tmp113, i32 0 ; <float> [#uses=1]
  %conv115 = fpext float %tmp114 to double        ; <double> [#uses=1]
  %mul116 = fmul double 2.000000e+000, %conv115   ; <double> [#uses=1]
  %add117 = fadd double %conv112, %mul116         ; <double> [#uses=1]
  %tmp118 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp119 = extractelement <4 x float> %tmp118, i32 0 ; <float> [#uses=1]
  %conv120 = fpext float %tmp119 to double        ; <double> [#uses=1]
  %add121 = fadd double %add117, %conv120         ; <double> [#uses=1]
  %tmp122 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp123 = extractelement <4 x float> %tmp122, i32 0 ; <float> [#uses=1]
  %conv124 = fpext float %tmp123 to double        ; <double> [#uses=1]
  %tmp125 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp126 = extractelement <4 x float> %tmp125, i32 0 ; <float> [#uses=1]
  %conv127 = fpext float %tmp126 to double        ; <double> [#uses=1]
  %mul128 = fmul double 2.000000e+000, %conv127   ; <double> [#uses=1]
  %add129 = fadd double %conv124, %mul128         ; <double> [#uses=1]
  %tmp130 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp131 = extractelement <4 x float> %tmp130, i32 0 ; <float> [#uses=1]
  %conv132 = fpext float %tmp131 to double        ; <double> [#uses=1]
  %add133 = fadd double %add129, %conv132         ; <double> [#uses=1]
  %sub134 = fsub double %add121, %add133          ; <double> [#uses=1]
  %conv135 = fptrunc double %sub134 to float      ; <float> [#uses=1]
  %vecinit136 = insertelement <2 x float> %vecinit, float %conv135, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit136, <2 x float>* %.compoundliteral
  %tmp137 = load <2 x float>* %.compoundliteral   ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp137, <2 x float>* %sobelXY_interim
  %tmp140 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp141 = extractelement <2 x float> %tmp140, i32 0 ; <float> [#uses=1]
  %conv142 = fpext float %tmp141 to double        ; <double> [#uses=1]
  %mul143 = fmul double %conv142, 1.250000e-001   ; <double> [#uses=1]
  %conv144 = fptrunc double %mul143 to float      ; <float> [#uses=1]
  %vecinit145 = insertelement <2 x float> undef, float %conv144, i32 0 ; <<2 x float>> [#uses=1]
  %tmp146 = load <2 x float>* %sobelXY_interim    ; <<2 x float>> [#uses=1]
  %tmp147 = extractelement <2 x float> %tmp146, i32 1 ; <float> [#uses=1]
  %conv148 = fpext float %tmp147 to double        ; <double> [#uses=1]
  %mul149 = fmul double %conv148, 1.250000e-001   ; <double> [#uses=1]
  %conv150 = fptrunc double %mul149 to float      ; <float> [#uses=1]
  %vecinit151 = insertelement <2 x float> %vecinit145, float %conv150, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit151, <2 x float>* %.compoundliteral139
  %tmp152 = load <2 x float>* %.compoundliteral139 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp152, <2 x float>* %sobelXY
  %tmp154 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %call155 = call float @_Z6lengthDv2_f(<2 x float> %tmp154) ; <float> [#uses=1]
  store float %call155, float* %len
  %tmp157 = load float* %len                      ; <float> [#uses=1]
  %tmp158 = load float* %minVal.addr              ; <float> [#uses=1]
  %call159 = call float @_Z3maxff(float %tmp157, float %tmp158) ; <float> [#uses=1]
  store float %call159, float* %mag
  %tmp160 = load float* %mag                      ; <float> [#uses=1]
  %tmp161 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call162 = call float @_Z3minff(float %tmp160, float %tmp161) ; <float> [#uses=1]
  store float %call162, float* %mag
  %tmp165 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp166 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp167 = extractelement <2 x float> %tmp166, i32 0 ; <float> [#uses=1]
  %mul168 = fmul float %tmp165, %tmp167           ; <float> [#uses=1]
  %vecinit169 = insertelement <4 x float> undef, float %mul168, i32 0 ; <<4 x float>> [#uses=1]
  %tmp170 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp171 = load <2 x float>* %sobelXY            ; <<2 x float>> [#uses=1]
  %tmp172 = extractelement <2 x float> %tmp171, i32 1 ; <float> [#uses=1]
  %mul173 = fmul float %tmp170, %tmp172           ; <float> [#uses=1]
  %vecinit174 = insertelement <4 x float> %vecinit169, float %mul173, i32 1 ; <<4 x float>> [#uses=1]
  %tmp175 = load float* %mag                      ; <float> [#uses=1]
  %vecinit176 = insertelement <4 x float> %vecinit174, float %tmp175, i32 2 ; <<4 x float>> [#uses=1]
  %tmp177 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp178 = extractelement <4 x float> %tmp177, i32 3 ; <float> [#uses=1]
  %vecinit179 = insertelement <4 x float> %vecinit176, float %tmp178, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit179, <4 x float>* %.compoundliteral164
  %tmp180 = load <4 x float>* %.compoundliteral164 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp180, <4 x float>* %outTmp
  %tmp181 = load <4 x float>* %outTmp             ; <<4 x float>> [#uses=1]
  %tmp182 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp182, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp183 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp183, i32 %tmp182 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp181, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body35
  %tmp184 = load i32* %col                        ; <i32> [#uses=1]
  %inc185 = add nsw i32 %tmp184, 1                ; <i32> [#uses=1]
  store i32 %inc185, i32* %col
  br label %for.cond29

for.end:                                          ; preds = %for.cond29
  br label %for.inc186

for.inc186:                                       ; preds = %for.end
  %tmp187 = load i32* %row                        ; <i32> [#uses=1]
  %inc188 = add nsw i32 %tmp187, 1                ; <i32> [#uses=1]
  store i32 %inc188, i32* %row
  br label %for.cond

for.end189:                                       ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @wlSobel_image2dUnrolled(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=14]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=3]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=5]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=3]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=3]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %sobelScaleVec = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=18]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=5]
  %current = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %m03 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %m13 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %m23 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=3]
  %col = alloca i32, align 4                      ; <i32*> [#uses=16]
  %sobelXY_interim1 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=3]
  %.compoundliteral115 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %sobelXY_interim2 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=3]
  %.compoundliteral171 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %len1 = alloca float, align 4                   ; <float*> [#uses=2]
  %len2 = alloca float, align 4                   ; <float*> [#uses=2]
  %sobelXY = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=7]
  %.compoundliteral236 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %mag1 = alloca float, align 4                   ; <float*> [#uses=4]
  %mag2 = alloca float, align 4                   ; <float*> [#uses=4]
  %outTmp1 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %.compoundliteral263 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %outTmp2 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %.compoundliteral275 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  %tmp = load float* %sobelScale.addr             ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %tmp, i32 0 ; <<4 x float>> [#uses=1]
  %tmp1 = load float* %sobelScale.addr            ; <float> [#uses=1]
  %vecinit2 = insertelement <4 x float> %vecinit, float %tmp1, i32 1 ; <<4 x float>> [#uses=1]
  %tmp3 = load float* %sobelScale.addr            ; <float> [#uses=1]
  %vecinit4 = insertelement <4 x float> %vecinit2, float %tmp3, i32 2 ; <<4 x float>> [#uses=1]
  %tmp5 = load float* %sobelScale.addr            ; <float> [#uses=1]
  %vecinit6 = insertelement <4 x float> %vecinit4, float %tmp5, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit6, <4 x float>* %.compoundliteral
  %tmp7 = load <4 x float>* %.compoundliteral     ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp7, <4 x float>* %sobelScaleVec
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp10 = load i32* %rowCountPerGlobalID.addr    ; <i32> [#uses=1]
  %tmp11 = load i32* %global_id                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp10, %tmp11                   ; <i32> [#uses=1]
  %add = add i32 %mul, 1                          ; <i32> [#uses=1]
  store i32 %add, i32* %row
  %tmp13 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call14 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp13) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call14, <2 x i32>* %imgSize
  %tmp16 = load i32* %row                         ; <i32> [#uses=1]
  %tmp17 = load i32* %rowCountPerGlobalID.addr    ; <i32> [#uses=1]
  %add18 = add nsw i32 %tmp16, %tmp17             ; <i32> [#uses=1]
  %tmp19 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp20 = extractelement <2 x i32> %tmp19, i32 1 ; <i32> [#uses=1]
  %sub = sub i32 %tmp20, 1                        ; <i32> [#uses=1]
  %call21 = call i32 @_Z3minii(i32 %add18, i32 %sub) ; <i32> [#uses=1]
  store i32 %call21, i32* %lastRow
  %tmp23 = load i32* %row                         ; <i32> [#uses=1]
  %tmp24 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp25 = extractelement <2 x i32> %tmp24, i32 0 ; <i32> [#uses=1]
  %mul26 = mul i32 %tmp23, %tmp25                 ; <i32> [#uses=1]
  store i32 %mul26, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc296, %entry
  %tmp39 = load i32* %row                         ; <i32> [#uses=1]
  %tmp40 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp39, %tmp40              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end299

for.body:                                         ; preds = %for.cond
  store i32 1, i32* %col
  br label %for.cond42

for.cond42:                                       ; preds = %for.inc, %for.body
  %tmp43 = load i32* %col                         ; <i32> [#uses=1]
  %tmp44 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp45 = extractelement <2 x i32> %tmp44, i32 0 ; <i32> [#uses=1]
  %sub46 = sub i32 %tmp45, 3                      ; <i32> [#uses=1]
  %cmp47 = icmp slt i32 %tmp43, %sub46            ; <i1> [#uses=1]
  br i1 %cmp47, label %for.body48, label %for.end

for.body48:                                       ; preds = %for.cond42
  %tmp49 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp50 = load i32* %col                         ; <i32> [#uses=1]
  %tmp51 = load i32* %row                         ; <i32> [#uses=1]
  %call52 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp49, i32 %tmp50, i32 %tmp51) ; <<4 x float>> [#uses=1]
  store <4 x float> %call52, <4 x float>* %current
  %tmp53 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp54 = load i32* %col                         ; <i32> [#uses=1]
  %sub55 = sub i32 %tmp54, 1                      ; <i32> [#uses=1]
  %tmp56 = load i32* %row                         ; <i32> [#uses=1]
  %call57 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp53, i32 %sub55, i32 %tmp56) ; <<4 x float>> [#uses=1]
  store <4 x float> %call57, <4 x float>* %m10
  %tmp58 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp59 = load i32* %col                         ; <i32> [#uses=1]
  %add60 = add nsw i32 %tmp59, 1                  ; <i32> [#uses=1]
  %tmp61 = load i32* %row                         ; <i32> [#uses=1]
  %call62 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp58, i32 %add60, i32 %tmp61) ; <<4 x float>> [#uses=1]
  store <4 x float> %call62, <4 x float>* %m12
  %tmp63 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp64 = load i32* %col                         ; <i32> [#uses=1]
  %tmp65 = load i32* %row                         ; <i32> [#uses=1]
  %sub66 = sub i32 %tmp65, 1                      ; <i32> [#uses=1]
  %call67 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp63, i32 %tmp64, i32 %sub66) ; <<4 x float>> [#uses=1]
  store <4 x float> %call67, <4 x float>* %m01
  %tmp68 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp69 = load i32* %col                         ; <i32> [#uses=1]
  %sub70 = sub i32 %tmp69, 1                      ; <i32> [#uses=1]
  %tmp71 = load i32* %row                         ; <i32> [#uses=1]
  %sub72 = sub i32 %tmp71, 1                      ; <i32> [#uses=1]
  %call73 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp68, i32 %sub70, i32 %sub72) ; <<4 x float>> [#uses=1]
  store <4 x float> %call73, <4 x float>* %m00
  %tmp74 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp75 = load i32* %col                         ; <i32> [#uses=1]
  %add76 = add nsw i32 %tmp75, 1                  ; <i32> [#uses=1]
  %tmp77 = load i32* %row                         ; <i32> [#uses=1]
  %sub78 = sub i32 %tmp77, 1                      ; <i32> [#uses=1]
  %call79 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp74, i32 %add76, i32 %sub78) ; <<4 x float>> [#uses=1]
  store <4 x float> %call79, <4 x float>* %m02
  %tmp80 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp81 = load i32* %col                         ; <i32> [#uses=1]
  %tmp82 = load i32* %row                         ; <i32> [#uses=1]
  %add83 = add nsw i32 %tmp82, 1                  ; <i32> [#uses=1]
  %call84 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp80, i32 %tmp81, i32 %add83) ; <<4 x float>> [#uses=1]
  store <4 x float> %call84, <4 x float>* %m21
  %tmp85 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp86 = load i32* %col                         ; <i32> [#uses=1]
  %sub87 = sub i32 %tmp86, 1                      ; <i32> [#uses=1]
  %tmp88 = load i32* %row                         ; <i32> [#uses=1]
  %add89 = add nsw i32 %tmp88, 1                  ; <i32> [#uses=1]
  %call90 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp85, i32 %sub87, i32 %add89) ; <<4 x float>> [#uses=1]
  store <4 x float> %call90, <4 x float>* %m20
  %tmp91 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp92 = load i32* %col                         ; <i32> [#uses=1]
  %add93 = add nsw i32 %tmp92, 1                  ; <i32> [#uses=1]
  %tmp94 = load i32* %row                         ; <i32> [#uses=1]
  %add95 = add nsw i32 %tmp94, 1                  ; <i32> [#uses=1]
  %call96 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp91, i32 %add93, i32 %add95) ; <<4 x float>> [#uses=1]
  store <4 x float> %call96, <4 x float>* %m22
  %tmp97 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp98 = load i32* %col                         ; <i32> [#uses=1]
  %add99 = add nsw i32 %tmp98, 2                  ; <i32> [#uses=1]
  %tmp100 = load i32* %row                        ; <i32> [#uses=1]
  %sub101 = sub i32 %tmp100, 1                    ; <i32> [#uses=1]
  %call102 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp97, i32 %add99, i32 %sub101) ; <<4 x float>> [#uses=1]
  store <4 x float> %call102, <4 x float>* %m03
  %tmp103 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp104 = load i32* %col                        ; <i32> [#uses=1]
  %add105 = add nsw i32 %tmp104, 2                ; <i32> [#uses=1]
  %tmp106 = load i32* %row                        ; <i32> [#uses=1]
  %call107 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp103, i32 %add105, i32 %tmp106) ; <<4 x float>> [#uses=1]
  store <4 x float> %call107, <4 x float>* %m13
  %tmp108 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp109 = load i32* %col                        ; <i32> [#uses=1]
  %add110 = add nsw i32 %tmp109, 2                ; <i32> [#uses=1]
  %tmp111 = load i32* %row                        ; <i32> [#uses=1]
  %add112 = add nsw i32 %tmp111, 1                ; <i32> [#uses=1]
  %call113 = call <4 x float> @evaluatePixelNoEdge(%struct._image2d_t* %tmp108, i32 %add110, i32 %add112) ; <<4 x float>> [#uses=1]
  store <4 x float> %call113, <4 x float>* %m23
  %tmp116 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp117 = extractelement <4 x float> %tmp116, i32 0 ; <float> [#uses=1]
  %conv = fpext float %tmp117 to double           ; <double> [#uses=1]
  %tmp118 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp119 = extractelement <4 x float> %tmp118, i32 0 ; <float> [#uses=1]
  %conv120 = fpext float %tmp119 to double        ; <double> [#uses=1]
  %mul121 = fmul double 2.000000e+000, %conv120   ; <double> [#uses=1]
  %add122 = fadd double %conv, %mul121            ; <double> [#uses=1]
  %tmp123 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp124 = extractelement <4 x float> %tmp123, i32 0 ; <float> [#uses=1]
  %conv125 = fpext float %tmp124 to double        ; <double> [#uses=1]
  %add126 = fadd double %add122, %conv125         ; <double> [#uses=1]
  %tmp127 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp128 = extractelement <4 x float> %tmp127, i32 0 ; <float> [#uses=1]
  %conv129 = fpext float %tmp128 to double        ; <double> [#uses=1]
  %tmp130 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp131 = extractelement <4 x float> %tmp130, i32 0 ; <float> [#uses=1]
  %conv132 = fpext float %tmp131 to double        ; <double> [#uses=1]
  %mul133 = fmul double 2.000000e+000, %conv132   ; <double> [#uses=1]
  %add134 = fadd double %conv129, %mul133         ; <double> [#uses=1]
  %tmp135 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp136 = extractelement <4 x float> %tmp135, i32 0 ; <float> [#uses=1]
  %conv137 = fpext float %tmp136 to double        ; <double> [#uses=1]
  %add138 = fadd double %add134, %conv137         ; <double> [#uses=1]
  %sub139 = fsub double %add126, %add138          ; <double> [#uses=1]
  %conv140 = fptrunc double %sub139 to float      ; <float> [#uses=1]
  %vecinit141 = insertelement <2 x float> undef, float %conv140, i32 0 ; <<2 x float>> [#uses=1]
  %tmp142 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp143 = extractelement <4 x float> %tmp142, i32 0 ; <float> [#uses=1]
  %conv144 = fpext float %tmp143 to double        ; <double> [#uses=1]
  %tmp145 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp146 = extractelement <4 x float> %tmp145, i32 0 ; <float> [#uses=1]
  %conv147 = fpext float %tmp146 to double        ; <double> [#uses=1]
  %mul148 = fmul double 2.000000e+000, %conv147   ; <double> [#uses=1]
  %add149 = fadd double %conv144, %mul148         ; <double> [#uses=1]
  %tmp150 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp151 = extractelement <4 x float> %tmp150, i32 0 ; <float> [#uses=1]
  %conv152 = fpext float %tmp151 to double        ; <double> [#uses=1]
  %add153 = fadd double %add149, %conv152         ; <double> [#uses=1]
  %tmp154 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp155 = extractelement <4 x float> %tmp154, i32 0 ; <float> [#uses=1]
  %conv156 = fpext float %tmp155 to double        ; <double> [#uses=1]
  %tmp157 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp158 = extractelement <4 x float> %tmp157, i32 0 ; <float> [#uses=1]
  %conv159 = fpext float %tmp158 to double        ; <double> [#uses=1]
  %mul160 = fmul double 2.000000e+000, %conv159   ; <double> [#uses=1]
  %add161 = fadd double %conv156, %mul160         ; <double> [#uses=1]
  %tmp162 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp163 = extractelement <4 x float> %tmp162, i32 0 ; <float> [#uses=1]
  %conv164 = fpext float %tmp163 to double        ; <double> [#uses=1]
  %add165 = fadd double %add161, %conv164         ; <double> [#uses=1]
  %sub166 = fsub double %add153, %add165          ; <double> [#uses=1]
  %conv167 = fptrunc double %sub166 to float      ; <float> [#uses=1]
  %vecinit168 = insertelement <2 x float> %vecinit141, float %conv167, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit168, <2 x float>* %.compoundliteral115
  %tmp169 = load <2 x float>* %.compoundliteral115 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp169, <2 x float>* %sobelXY_interim1
  %tmp172 = load <4 x float>* %m03                ; <<4 x float>> [#uses=1]
  %tmp173 = extractelement <4 x float> %tmp172, i32 0 ; <float> [#uses=1]
  %conv174 = fpext float %tmp173 to double        ; <double> [#uses=1]
  %tmp175 = load <4 x float>* %m13                ; <<4 x float>> [#uses=1]
  %tmp176 = extractelement <4 x float> %tmp175, i32 0 ; <float> [#uses=1]
  %conv177 = fpext float %tmp176 to double        ; <double> [#uses=1]
  %mul178 = fmul double 2.000000e+000, %conv177   ; <double> [#uses=1]
  %add179 = fadd double %conv174, %mul178         ; <double> [#uses=1]
  %tmp180 = load <4 x float>* %m23                ; <<4 x float>> [#uses=1]
  %tmp181 = extractelement <4 x float> %tmp180, i32 0 ; <float> [#uses=1]
  %conv182 = fpext float %tmp181 to double        ; <double> [#uses=1]
  %add183 = fadd double %add179, %conv182         ; <double> [#uses=1]
  %tmp184 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp185 = extractelement <4 x float> %tmp184, i32 0 ; <float> [#uses=1]
  %conv186 = fpext float %tmp185 to double        ; <double> [#uses=1]
  %tmp187 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp188 = extractelement <4 x float> %tmp187, i32 0 ; <float> [#uses=1]
  %conv189 = fpext float %tmp188 to double        ; <double> [#uses=1]
  %mul190 = fmul double 2.000000e+000, %conv189   ; <double> [#uses=1]
  %add191 = fadd double %conv186, %mul190         ; <double> [#uses=1]
  %tmp192 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp193 = extractelement <4 x float> %tmp192, i32 0 ; <float> [#uses=1]
  %conv194 = fpext float %tmp193 to double        ; <double> [#uses=1]
  %add195 = fadd double %add191, %conv194         ; <double> [#uses=1]
  %sub196 = fsub double %add183, %add195          ; <double> [#uses=1]
  %conv197 = fptrunc double %sub196 to float      ; <float> [#uses=1]
  %vecinit198 = insertelement <2 x float> undef, float %conv197, i32 0 ; <<2 x float>> [#uses=1]
  %tmp199 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp200 = extractelement <4 x float> %tmp199, i32 0 ; <float> [#uses=1]
  %conv201 = fpext float %tmp200 to double        ; <double> [#uses=1]
  %tmp202 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp203 = extractelement <4 x float> %tmp202, i32 0 ; <float> [#uses=1]
  %conv204 = fpext float %tmp203 to double        ; <double> [#uses=1]
  %mul205 = fmul double 2.000000e+000, %conv204   ; <double> [#uses=1]
  %add206 = fadd double %conv201, %mul205         ; <double> [#uses=1]
  %tmp207 = load <4 x float>* %m23                ; <<4 x float>> [#uses=1]
  %tmp208 = extractelement <4 x float> %tmp207, i32 0 ; <float> [#uses=1]
  %conv209 = fpext float %tmp208 to double        ; <double> [#uses=1]
  %add210 = fadd double %add206, %conv209         ; <double> [#uses=1]
  %tmp211 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp212 = extractelement <4 x float> %tmp211, i32 0 ; <float> [#uses=1]
  %conv213 = fpext float %tmp212 to double        ; <double> [#uses=1]
  %tmp214 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp215 = extractelement <4 x float> %tmp214, i32 0 ; <float> [#uses=1]
  %conv216 = fpext float %tmp215 to double        ; <double> [#uses=1]
  %mul217 = fmul double 2.000000e+000, %conv216   ; <double> [#uses=1]
  %add218 = fadd double %conv213, %mul217         ; <double> [#uses=1]
  %tmp219 = load <4 x float>* %m03                ; <<4 x float>> [#uses=1]
  %tmp220 = extractelement <4 x float> %tmp219, i32 0 ; <float> [#uses=1]
  %conv221 = fpext float %tmp220 to double        ; <double> [#uses=1]
  %add222 = fadd double %add218, %conv221         ; <double> [#uses=1]
  %sub223 = fsub double %add210, %add222          ; <double> [#uses=1]
  %conv224 = fptrunc double %sub223 to float      ; <float> [#uses=1]
  %vecinit225 = insertelement <2 x float> %vecinit198, float %conv224, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit225, <2 x float>* %.compoundliteral171
  %tmp226 = load <2 x float>* %.compoundliteral171 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp226, <2 x float>* %sobelXY_interim2
  %tmp228 = load <2 x float>* %sobelXY_interim1   ; <<2 x float>> [#uses=1]
  %call229 = call float @_Z6lengthDv2_f(<2 x float> %tmp228) ; <float> [#uses=1]
  %mul230 = fmul float %call229, 1.250000e-001    ; <float> [#uses=1]
  store float %mul230, float* %len1
  %tmp232 = load <2 x float>* %sobelXY_interim2   ; <<2 x float>> [#uses=1]
  %call233 = call float @_Z6lengthDv2_f(<2 x float> %tmp232) ; <float> [#uses=1]
  %mul234 = fmul float %call233, 1.250000e-001    ; <float> [#uses=1]
  store float %mul234, float* %len2
  %tmp237 = load <2 x float>* %sobelXY_interim1   ; <<2 x float>> [#uses=1]
  %vext = shufflevector <2 x float> %tmp237, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %vecinit238 = shufflevector <4 x float> %vext, <4 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp239 = load <2 x float>* %sobelXY_interim2   ; <<2 x float>> [#uses=1]
  %vext240 = shufflevector <2 x float> %tmp239, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %vecinit241 = shufflevector <4 x float> %vecinit238, <4 x float> %vext240, <4 x i32> <i32 0, i32 1, i32 4, i32 5> ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit241, <4 x float>* %.compoundliteral236
  %tmp242 = load <4 x float>* %.compoundliteral236 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp242, <4 x float>* %sobelXY
  %tmp243 = load <4 x float>* %sobelScaleVec      ; <<4 x float>> [#uses=1]
  %tmp244 = load <4 x float> addrspace(1)* @oneEighth ; <<4 x float>> [#uses=1]
  %mul245 = fmul <4 x float> %tmp243, %tmp244     ; <<4 x float>> [#uses=1]
  %tmp246 = load <4 x float>* %sobelXY            ; <<4 x float>> [#uses=1]
  %mul247 = fmul <4 x float> %tmp246, %mul245     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul247, <4 x float>* %sobelXY
  %tmp249 = load float* %len1                     ; <float> [#uses=1]
  %tmp250 = load float* %minVal.addr              ; <float> [#uses=1]
  %call251 = call float @_Z3maxff(float %tmp249, float %tmp250) ; <float> [#uses=1]
  store float %call251, float* %mag1
  %tmp253 = load float* %len2                     ; <float> [#uses=1]
  %tmp254 = load float* %minVal.addr              ; <float> [#uses=1]
  %call255 = call float @_Z3maxff(float %tmp253, float %tmp254) ; <float> [#uses=1]
  store float %call255, float* %mag2
  %tmp256 = load float* %mag1                     ; <float> [#uses=1]
  %tmp257 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call258 = call float @_Z3minff(float %tmp256, float %tmp257) ; <float> [#uses=1]
  store float %call258, float* %mag1
  %tmp259 = load float* %mag2                     ; <float> [#uses=1]
  %tmp260 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call261 = call float @_Z3minff(float %tmp259, float %tmp260) ; <float> [#uses=1]
  store float %call261, float* %mag2
  %tmp264 = load <4 x float>* %sobelXY            ; <<4 x float>> [#uses=3]
  %tmp265 = extractelement <4 x float> %tmp264, i32 0 ; <float> [#uses=0]
  %0 = shufflevector <4 x float> %tmp264, <4 x float> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp266 = load <4 x float>* %sobelXY            ; <<4 x float>> [#uses=2]
  %tmp267 = extractelement <4 x float> %tmp266, i32 1 ; <float> [#uses=0]
  %1 = shufflevector <4 x float> %tmp264, <4 x float> %tmp266, <4 x i32> <i32 0, i32 5, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp268 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit269 = insertelement <4 x float> %1, float %tmp268, i32 2 ; <<4 x float>> [#uses=1]
  %tmp270 = load <4 x float>* %current            ; <<4 x float>> [#uses=1]
  %tmp271 = extractelement <4 x float> %tmp270, i32 3 ; <float> [#uses=1]
  %vecinit272 = insertelement <4 x float> %vecinit269, float %tmp271, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit272, <4 x float>* %.compoundliteral263
  %tmp273 = load <4 x float>* %.compoundliteral263 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp273, <4 x float>* %outTmp1
  %tmp276 = load <4 x float>* %sobelXY            ; <<4 x float>> [#uses=3]
  %tmp277 = extractelement <4 x float> %tmp276, i32 2 ; <float> [#uses=0]
  %2 = shufflevector <4 x float> %tmp276, <4 x float> undef, <4 x i32> <i32 2, i32 undef, i32 undef, i32 undef> ; <<4 x float>> [#uses=0]
  %tmp278 = load <4 x float>* %sobelXY            ; <<4 x float>> [#uses=2]
  %tmp279 = extractelement <4 x float> %tmp278, i32 3 ; <float> [#uses=0]
  %3 = shufflevector <4 x float> %tmp276, <4 x float> %tmp278, <4 x i32> <i32 2, i32 7, i32 undef, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp280 = load float* %mag2                     ; <float> [#uses=1]
  %vecinit281 = insertelement <4 x float> %3, float %tmp280, i32 2 ; <<4 x float>> [#uses=1]
  %tmp282 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp283 = extractelement <4 x float> %tmp282, i32 3 ; <float> [#uses=1]
  %vecinit284 = insertelement <4 x float> %vecinit281, float %tmp283, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit284, <4 x float>* %.compoundliteral275
  %tmp285 = load <4 x float>* %.compoundliteral275 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp285, <4 x float>* %outTmp2
  %tmp286 = load <4 x float>* %outTmp1            ; <<4 x float>> [#uses=1]
  %tmp287 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp287, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp288 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp288, i32 %tmp287 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp286, <4 x float> addrspace(1)* %arrayidx
  %tmp289 = load <4 x float>* %outTmp2            ; <<4 x float>> [#uses=1]
  %tmp290 = load i32* %index                      ; <i32> [#uses=2]
  %inc291 = add nsw i32 %tmp290, 1                ; <i32> [#uses=1]
  store i32 %inc291, i32* %index
  %tmp292 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx293 = getelementptr inbounds <4 x float> addrspace(1)* %tmp292, i32 %tmp290 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp289, <4 x float> addrspace(1)* %arrayidx293
  br label %for.inc

for.inc:                                          ; preds = %for.body48
  %tmp294 = load i32* %col                        ; <i32> [#uses=1]
  %add295 = add nsw i32 %tmp294, 2                ; <i32> [#uses=1]
  store i32 %add295, i32* %col
  br label %for.cond42

for.end:                                          ; preds = %for.cond42
  br label %for.inc296

for.inc296:                                       ; preds = %for.end
  %tmp297 = load i32* %row                        ; <i32> [#uses=1]
  %inc298 = add nsw i32 %tmp297, 1                ; <i32> [#uses=1]
  store i32 %inc298, i32* %row
  br label %for.cond

for.end299:                                       ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @wlSobel_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, float %sobelScale, float %minVal, float %maxVal, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=50]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %sobelScale.addr = alloca float, align 4        ; <float*> [#uses=19]
  %minVal.addr = alloca float, align 4            ; <float*> [#uses=10]
  %maxVal.addr = alloca float, align 4            ; <float*> [#uses=10]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=56]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=22]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %topEdge = alloca i8, align 1                   ; <i8*> [#uses=5]
  %bottomEdge = alloca i8, align 1                ; <i8*> [#uses=5]
  %leftEdge = alloca i8, align 1                  ; <i8*> [#uses=5]
  %rightEdge = alloca i8, align 1                 ; <i8*> [#uses=5]
  %dims = alloca i32, align 4                     ; <i32*> [#uses=1]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  %global_szx = alloca i32, align 4               ; <i32*> [#uses=3]
  %global_szy = alloca i32, align 4               ; <i32*> [#uses=3]
  %count_y = alloca i32, align 4                  ; <i32*> [#uses=8]
  %count_x = alloca i32, align 4                  ; <i32*> [#uses=8]
  %index_x = alloca i32, align 4                  ; <i32*> [#uses=10]
  %index_y = alloca i32, align 4                  ; <i32*> [#uses=10]
  %m00 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m10 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m20 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m01 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m11 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=18]
  %m21 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m02 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m12 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %m22 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=12]
  %sobelXY1 = alloca <2 x float>, align 8         ; <<2 x float>*> [#uses=55]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %len1 = alloca float, align 4                   ; <float*> [#uses=19]
  %mag1 = alloca float, align 4                   ; <float*> [#uses=37]
  %tmpIndex = alloca i32, align 4                 ; <i32*> [#uses=13]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=4]
  %row = alloca i32, align 4                      ; <i32*> [#uses=4]
  %column = alloca i32, align 4                   ; <i32*> [#uses=4]
  %.compoundliteral143 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral207 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral258 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral290 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %column314 = alloca i32, align 4                ; <i32*> [#uses=11]
  %.compoundliteral358 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral402 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral459 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral492 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %row518 = alloca i32, align 4                   ; <i32*> [#uses=11]
  %.compoundliteral571 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral615 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral680 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral713 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %column741 = alloca i32, align 4                ; <i32*> [#uses=11]
  %.compoundliteral809 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral855 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %row889 = alloca i32, align 4                   ; <i32*> [#uses=11]
  %.compoundliteral945 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral991 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral1058 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral1092 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %sobelScale, float* %sobelScale.addr
  store float %minVal, float* %minVal.addr
  store float %maxVal, float* %maxVal.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  store i8 0, i8* %topEdge
  store i8 0, i8* %bottomEdge
  store i8 0, i8* %leftEdge
  store i8 0, i8* %rightEdge
  %call = call i32 (...)* @get_work_dim()         ; <i32> [#uses=1]
  store i32 %call, i32* %dims
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdx
  %call2 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call2, i32* %globalIdy
  %call3 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call3, i32* %global_szx
  %call4 = call i32 @get_global_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call4, i32* %global_szy
  %tmp = load i32* %height.addr                   ; <i32> [#uses=1]
  %tmp5 = load i32* %global_szy                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp5                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp5         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %count_y
  %tmp7 = load i32* %width.addr                   ; <i32> [#uses=1]
  %tmp8 = load i32* %global_szx                   ; <i32> [#uses=2]
  %cmp9 = icmp eq i32 0, %tmp8                    ; <i1> [#uses=1]
  %sel10 = select i1 %cmp9, i32 1, i32 %tmp8      ; <i32> [#uses=1]
  %div11 = udiv i32 %tmp7, %sel10                 ; <i32> [#uses=1]
  store i32 %div11, i32* %count_x
  %tmp13 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp14 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp13, %tmp14                   ; <i32> [#uses=1]
  %tmp15 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp16 = icmp eq i32 0, %tmp15                  ; <i1> [#uses=1]
  %sel17 = select i1 %cmp16, i32 1, i32 %tmp15    ; <i32> [#uses=1]
  %div18 = udiv i32 %mul, %sel17                  ; <i32> [#uses=1]
  store i32 %div18, i32* %index_x
  %tmp20 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp21 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %mul22 = mul i32 %tmp20, %tmp21                 ; <i32> [#uses=1]
  %tmp23 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp24 = icmp eq i32 0, %tmp23                  ; <i1> [#uses=1]
  %sel25 = select i1 %cmp24, i32 1, i32 %tmp23    ; <i32> [#uses=1]
  %div26 = udiv i32 %mul22, %sel25                ; <i32> [#uses=1]
  store i32 %div26, i32* %index_y
  store <2 x float> zeroinitializer, <2 x float>* %.compoundliteral
  %tmp37 = load <2 x float>* %.compoundliteral    ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp37, <2 x float>* %sobelXY1
  store float 0.000000e+000, float* %len1
  store float 0.000000e+000, float* %mag1
  %tmp40 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp41 = load i32* %count_y                     ; <i32> [#uses=1]
  %add = add i32 %tmp40, %tmp41                   ; <i32> [#uses=1]
  %add42 = add i32 %add, 1                        ; <i32> [#uses=1]
  %tmp43 = load i32* %height.addr                 ; <i32> [#uses=1]
  %cmp44 = icmp uge i32 %add42, %tmp43            ; <i1> [#uses=1]
  br i1 %cmp44, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i8 1, i8* %bottomEdge
  %tmp45 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp46 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub = sub i32 %tmp45, %tmp46                   ; <i32> [#uses=1]
  %sub47 = sub i32 %sub, 1                        ; <i32> [#uses=1]
  store i32 %sub47, i32* %count_y
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp48 = load i32* %index_x                     ; <i32> [#uses=1]
  %tmp49 = load i32* %count_x                     ; <i32> [#uses=1]
  %add50 = add i32 %tmp48, %tmp49                 ; <i32> [#uses=1]
  %add51 = add i32 %add50, 1                      ; <i32> [#uses=1]
  %tmp52 = load i32* %width.addr                  ; <i32> [#uses=1]
  %cmp53 = icmp uge i32 %add51, %tmp52            ; <i1> [#uses=1]
  br i1 %cmp53, label %if.then54, label %if.end59

if.then54:                                        ; preds = %if.end
  store i8 1, i8* %rightEdge
  %tmp55 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp56 = load i32* %index_x                     ; <i32> [#uses=1]
  %sub57 = sub i32 %tmp55, %tmp56                 ; <i32> [#uses=1]
  %sub58 = sub i32 %sub57, 1                      ; <i32> [#uses=1]
  store i32 %sub58, i32* %count_x
  br label %if.end59

if.end59:                                         ; preds = %if.then54, %if.end
  %tmp60 = load i32* %index_y                     ; <i32> [#uses=1]
  %cmp61 = icmp ult i32 %tmp60, 1                 ; <i1> [#uses=1]
  br i1 %cmp61, label %if.then62, label %if.end65

if.then62:                                        ; preds = %if.end59
  store i8 1, i8* %topEdge
  store i32 1, i32* %index_y
  %tmp63 = load i32* %count_y                     ; <i32> [#uses=1]
  %sub64 = sub i32 %tmp63, 1                      ; <i32> [#uses=1]
  store i32 %sub64, i32* %count_y
  br label %if.end65

if.end65:                                         ; preds = %if.then62, %if.end59
  %tmp66 = load i32* %index_x                     ; <i32> [#uses=1]
  %cmp67 = icmp ult i32 %tmp66, 1                 ; <i1> [#uses=1]
  br i1 %cmp67, label %if.then68, label %if.end71

if.then68:                                        ; preds = %if.end65
  store i8 1, i8* %leftEdge
  store i32 1, i32* %index_x
  %tmp69 = load i32* %count_x                     ; <i32> [#uses=1]
  %sub70 = sub i32 %tmp69, 1                      ; <i32> [#uses=1]
  store i32 %sub70, i32* %count_x
  br label %if.end71

if.end71:                                         ; preds = %if.then68, %if.end65
  %tmp74 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp75 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul76 = mul i32 %tmp74, %tmp75                 ; <i32> [#uses=1]
  %tmp77 = load i32* %index_x                     ; <i32> [#uses=1]
  %add78 = add i32 %mul76, %tmp77                 ; <i32> [#uses=1]
  store i32 %add78, i32* %sourceIndex
  store i32 0, i32* %row
  br label %for.cond

for.cond:                                         ; preds = %for.inc233, %if.end71
  %tmp80 = load i32* %row                         ; <i32> [#uses=1]
  %tmp81 = load i32* %count_y                     ; <i32> [#uses=1]
  %cmp82 = icmp ult i32 %tmp80, %tmp81            ; <i1> [#uses=1]
  br i1 %cmp82, label %for.body, label %for.end236

for.body:                                         ; preds = %for.cond
  %tmp83 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  store i32 %tmp83, i32* %tmpIndex
  store i32 0, i32* %column
  br label %for.cond85

for.cond85:                                       ; preds = %for.inc, %for.body
  %tmp86 = load i32* %column                      ; <i32> [#uses=1]
  %tmp87 = load i32* %count_x                     ; <i32> [#uses=1]
  %cmp88 = icmp ult i32 %tmp86, %tmp87            ; <i1> [#uses=1]
  br i1 %cmp88, label %for.body89, label %for.end

for.body89:                                       ; preds = %for.cond85
  %tmp90 = load i32* %tmpIndex                    ; <i32> [#uses=1]
  %tmp91 = load i32* %width.addr                  ; <i32> [#uses=1]
  %sub92 = sub i32 %tmp90, %tmp91                 ; <i32> [#uses=1]
  %sub93 = sub i32 %sub92, 1                      ; <i32> [#uses=1]
  %tmp94 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp94, i32 %sub93 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp95 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp95, <4 x float>* %m00
  %tmp96 = load i32* %tmpIndex                    ; <i32> [#uses=1]
  %tmp97 = load i32* %width.addr                  ; <i32> [#uses=1]
  %sub98 = sub i32 %tmp96, %tmp97                 ; <i32> [#uses=1]
  %tmp99 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx100 = getelementptr inbounds <4 x float> addrspace(1)* %tmp99, i32 %sub98 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp101 = load <4 x float> addrspace(1)* %arrayidx100 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp101, <4 x float>* %m01
  %tmp102 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp103 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub104 = sub i32 %tmp102, %tmp103              ; <i32> [#uses=1]
  %add105 = add i32 %sub104, 1                    ; <i32> [#uses=1]
  %tmp106 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx107 = getelementptr inbounds <4 x float> addrspace(1)* %tmp106, i32 %add105 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp108 = load <4 x float> addrspace(1)* %arrayidx107 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp108, <4 x float>* %m02
  %tmp109 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %sub110 = sub i32 %tmp109, 1                    ; <i32> [#uses=1]
  %tmp111 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx112 = getelementptr inbounds <4 x float> addrspace(1)* %tmp111, i32 %sub110 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp113 = load <4 x float> addrspace(1)* %arrayidx112 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp113, <4 x float>* %m10
  %tmp114 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp115 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx116 = getelementptr inbounds <4 x float> addrspace(1)* %tmp115, i32 %tmp114 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp117 = load <4 x float> addrspace(1)* %arrayidx116 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117, <4 x float>* %m11
  %tmp118 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %add119 = add i32 %tmp118, 1                    ; <i32> [#uses=1]
  %tmp120 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds <4 x float> addrspace(1)* %tmp120, i32 %add119 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp122 = load <4 x float> addrspace(1)* %arrayidx121 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp122, <4 x float>* %m12
  %tmp123 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp124 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add125 = add i32 %tmp123, %tmp124              ; <i32> [#uses=1]
  %sub126 = sub i32 %add125, 1                    ; <i32> [#uses=1]
  %tmp127 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx128 = getelementptr inbounds <4 x float> addrspace(1)* %tmp127, i32 %sub126 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp129 = load <4 x float> addrspace(1)* %arrayidx128 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp129, <4 x float>* %m20
  %tmp130 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp131 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add132 = add i32 %tmp130, %tmp131              ; <i32> [#uses=1]
  %tmp133 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds <4 x float> addrspace(1)* %tmp133, i32 %add132 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp135 = load <4 x float> addrspace(1)* %arrayidx134 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp135, <4 x float>* %m21
  %tmp136 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp137 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add138 = add i32 %tmp136, %tmp137              ; <i32> [#uses=1]
  %add139 = add i32 %add138, 1                    ; <i32> [#uses=1]
  %tmp140 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx141 = getelementptr inbounds <4 x float> addrspace(1)* %tmp140, i32 %add139 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp142 = load <4 x float> addrspace(1)* %arrayidx141 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp142, <4 x float>* %m22
  %tmp144 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp145 = extractelement <4 x float> %tmp144, i32 0 ; <float> [#uses=1]
  %conv = fpext float %tmp145 to double           ; <double> [#uses=1]
  %tmp146 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp147 = extractelement <4 x float> %tmp146, i32 0 ; <float> [#uses=1]
  %conv148 = fpext float %tmp147 to double        ; <double> [#uses=1]
  %mul149 = fmul double 2.000000e+000, %conv148   ; <double> [#uses=1]
  %add150 = fadd double %conv, %mul149            ; <double> [#uses=1]
  %tmp151 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp152 = extractelement <4 x float> %tmp151, i32 0 ; <float> [#uses=1]
  %conv153 = fpext float %tmp152 to double        ; <double> [#uses=1]
  %add154 = fadd double %add150, %conv153         ; <double> [#uses=1]
  %tmp155 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp156 = extractelement <4 x float> %tmp155, i32 0 ; <float> [#uses=1]
  %conv157 = fpext float %tmp156 to double        ; <double> [#uses=1]
  %tmp158 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp159 = extractelement <4 x float> %tmp158, i32 0 ; <float> [#uses=1]
  %conv160 = fpext float %tmp159 to double        ; <double> [#uses=1]
  %mul161 = fmul double 2.000000e+000, %conv160   ; <double> [#uses=1]
  %add162 = fadd double %conv157, %mul161         ; <double> [#uses=1]
  %tmp163 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp164 = extractelement <4 x float> %tmp163, i32 0 ; <float> [#uses=1]
  %conv165 = fpext float %tmp164 to double        ; <double> [#uses=1]
  %add166 = fadd double %add162, %conv165         ; <double> [#uses=1]
  %sub167 = fsub double %add154, %add166          ; <double> [#uses=1]
  %conv168 = fptrunc double %sub167 to float      ; <float> [#uses=1]
  %vecinit = insertelement <2 x float> undef, float %conv168, i32 0 ; <<2 x float>> [#uses=1]
  %tmp169 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp170 = extractelement <4 x float> %tmp169, i32 0 ; <float> [#uses=1]
  %conv171 = fpext float %tmp170 to double        ; <double> [#uses=1]
  %tmp172 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp173 = extractelement <4 x float> %tmp172, i32 0 ; <float> [#uses=1]
  %conv174 = fpext float %tmp173 to double        ; <double> [#uses=1]
  %mul175 = fmul double 2.000000e+000, %conv174   ; <double> [#uses=1]
  %add176 = fadd double %conv171, %mul175         ; <double> [#uses=1]
  %tmp177 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp178 = extractelement <4 x float> %tmp177, i32 0 ; <float> [#uses=1]
  %conv179 = fpext float %tmp178 to double        ; <double> [#uses=1]
  %add180 = fadd double %add176, %conv179         ; <double> [#uses=1]
  %tmp181 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp182 = extractelement <4 x float> %tmp181, i32 0 ; <float> [#uses=1]
  %conv183 = fpext float %tmp182 to double        ; <double> [#uses=1]
  %tmp184 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp185 = extractelement <4 x float> %tmp184, i32 0 ; <float> [#uses=1]
  %conv186 = fpext float %tmp185 to double        ; <double> [#uses=1]
  %mul187 = fmul double 2.000000e+000, %conv186   ; <double> [#uses=1]
  %add188 = fadd double %conv183, %mul187         ; <double> [#uses=1]
  %tmp189 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp190 = extractelement <4 x float> %tmp189, i32 0 ; <float> [#uses=1]
  %conv191 = fpext float %tmp190 to double        ; <double> [#uses=1]
  %add192 = fadd double %add188, %conv191         ; <double> [#uses=1]
  %sub193 = fsub double %add180, %add192          ; <double> [#uses=1]
  %conv194 = fptrunc double %sub193 to float      ; <float> [#uses=1]
  %vecinit195 = insertelement <2 x float> %vecinit, float %conv194, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit195, <2 x float>* %.compoundliteral143
  %tmp196 = load <2 x float>* %.compoundliteral143 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp196, <2 x float>* %sobelXY1
  %tmp197 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul198 = fmul <2 x float> %tmp197, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul198, <2 x float>* %sobelXY1
  %tmp199 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call200 = call float @_Z6lengthDv2_f(<2 x float> %tmp199) ; <float> [#uses=1]
  store float %call200, float* %len1
  %tmp201 = load float* %len1                     ; <float> [#uses=1]
  %tmp202 = load float* %minVal.addr              ; <float> [#uses=1]
  %call203 = call float @_Z3maxff(float %tmp201, float %tmp202) ; <float> [#uses=1]
  store float %call203, float* %mag1
  %tmp204 = load float* %mag1                     ; <float> [#uses=1]
  %tmp205 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call206 = call float @_Z3minff(float %tmp204, float %tmp205) ; <float> [#uses=1]
  store float %call206, float* %mag1
  %tmp208 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp209 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp210 = extractelement <2 x float> %tmp209, i32 0 ; <float> [#uses=1]
  %mul211 = fmul float %tmp208, %tmp210           ; <float> [#uses=1]
  %vecinit212 = insertelement <4 x float> undef, float %mul211, i32 0 ; <<4 x float>> [#uses=1]
  %tmp213 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp214 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp215 = extractelement <2 x float> %tmp214, i32 1 ; <float> [#uses=1]
  %mul216 = fmul float %tmp213, %tmp215           ; <float> [#uses=1]
  %vecinit217 = insertelement <4 x float> %vecinit212, float %mul216, i32 1 ; <<4 x float>> [#uses=1]
  %tmp218 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit219 = insertelement <4 x float> %vecinit217, float %tmp218, i32 2 ; <<4 x float>> [#uses=1]
  %tmp220 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp221 = extractelement <4 x float> %tmp220, i32 3 ; <float> [#uses=1]
  %vecinit222 = insertelement <4 x float> %vecinit219, float %tmp221, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit222, <4 x float>* %.compoundliteral207
  %tmp223 = load <4 x float>* %.compoundliteral207 ; <<4 x float>> [#uses=1]
  %tmp224 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %tmp225 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx226 = getelementptr inbounds <4 x float> addrspace(1)* %tmp225, i32 %tmp224 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp223, <4 x float> addrspace(1)* %arrayidx226
  %tmp227 = load i32* %tmpIndex                   ; <i32> [#uses=1]
  %inc = add i32 %tmp227, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %tmpIndex
  br label %for.inc

for.inc:                                          ; preds = %for.body89
  %tmp228 = load i32* %column                     ; <i32> [#uses=1]
  %inc229 = add i32 %tmp228, 1                    ; <i32> [#uses=1]
  store i32 %inc229, i32* %column
  br label %for.cond85

for.end:                                          ; preds = %for.cond85
  %tmp230 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp231 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %add232 = add i32 %tmp231, %tmp230              ; <i32> [#uses=1]
  store i32 %add232, i32* %sourceIndex
  br label %for.inc233

for.inc233:                                       ; preds = %for.end
  %tmp234 = load i32* %row                        ; <i32> [#uses=1]
  %inc235 = add i32 %tmp234, 1                    ; <i32> [#uses=1]
  store i32 %inc235, i32* %row
  br label %for.cond

for.end236:                                       ; preds = %for.cond
  %tmp237 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool = trunc i8 %tmp237 to i1                ; <i1> [#uses=1]
  br i1 %tobool, label %land.lhs.true, label %if.end309

land.lhs.true:                                    ; preds = %for.end236
  %tmp239 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool240 = trunc i8 %tmp239 to i1             ; <i1> [#uses=1]
  br i1 %tobool240, label %if.then242, label %if.end309

if.then242:                                       ; preds = %land.lhs.true
  %tmp243 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx244 = getelementptr inbounds <4 x float> addrspace(1)* %tmp243, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp245 = load <4 x float> addrspace(1)* %arrayidx244 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp245, <4 x float>* %m11
  %tmp246 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx247 = getelementptr inbounds <4 x float> addrspace(1)* %tmp246, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp248 = load <4 x float> addrspace(1)* %arrayidx247 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp248, <4 x float>* %m12
  %tmp249 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp250 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx251 = getelementptr inbounds <4 x float> addrspace(1)* %tmp250, i32 %tmp249 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp252 = load <4 x float> addrspace(1)* %arrayidx251 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp252, <4 x float>* %m21
  %tmp253 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add254 = add i32 %tmp253, 1                    ; <i32> [#uses=1]
  %tmp255 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx256 = getelementptr inbounds <4 x float> addrspace(1)* %tmp255, i32 %add254 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp257 = load <4 x float> addrspace(1)* %arrayidx256 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp257, <4 x float>* %m22
  %tmp259 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp260 = extractelement <4 x float> %tmp259, i32 0 ; <float> [#uses=1]
  %conv261 = fpext float %tmp260 to double        ; <double> [#uses=1]
  %mul262 = fmul double 2.000000e+000, %conv261   ; <double> [#uses=1]
  %tmp263 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp264 = extractelement <4 x float> %tmp263, i32 0 ; <float> [#uses=1]
  %conv265 = fpext float %tmp264 to double        ; <double> [#uses=1]
  %add266 = fadd double %mul262, %conv265         ; <double> [#uses=1]
  %conv267 = fptrunc double %add266 to float      ; <float> [#uses=1]
  %vecinit268 = insertelement <2 x float> undef, float %conv267, i32 0 ; <<2 x float>> [#uses=1]
  %tmp269 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp270 = extractelement <4 x float> %tmp269, i32 0 ; <float> [#uses=1]
  %conv271 = fpext float %tmp270 to double        ; <double> [#uses=1]
  %mul272 = fmul double 2.000000e+000, %conv271   ; <double> [#uses=1]
  %tmp273 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp274 = extractelement <4 x float> %tmp273, i32 0 ; <float> [#uses=1]
  %conv275 = fpext float %tmp274 to double        ; <double> [#uses=1]
  %add276 = fadd double %mul272, %conv275         ; <double> [#uses=1]
  %conv277 = fptrunc double %add276 to float      ; <float> [#uses=1]
  %vecinit278 = insertelement <2 x float> %vecinit268, float %conv277, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit278, <2 x float>* %.compoundliteral258
  %tmp279 = load <2 x float>* %.compoundliteral258 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp279, <2 x float>* %sobelXY1
  %tmp280 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul281 = fmul <2 x float> %tmp280, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul281, <2 x float>* %sobelXY1
  %tmp282 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call283 = call float @_Z6lengthDv2_f(<2 x float> %tmp282) ; <float> [#uses=1]
  store float %call283, float* %len1
  %tmp284 = load float* %len1                     ; <float> [#uses=1]
  %tmp285 = load float* %minVal.addr              ; <float> [#uses=1]
  %call286 = call float @_Z3maxff(float %tmp284, float %tmp285) ; <float> [#uses=1]
  store float %call286, float* %mag1
  %tmp287 = load float* %mag1                     ; <float> [#uses=1]
  %tmp288 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call289 = call float @_Z3minff(float %tmp287, float %tmp288) ; <float> [#uses=1]
  store float %call289, float* %mag1
  %tmp291 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp292 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp293 = extractelement <2 x float> %tmp292, i32 0 ; <float> [#uses=1]
  %mul294 = fmul float %tmp291, %tmp293           ; <float> [#uses=1]
  %vecinit295 = insertelement <4 x float> undef, float %mul294, i32 0 ; <<4 x float>> [#uses=1]
  %tmp296 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp297 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp298 = extractelement <2 x float> %tmp297, i32 1 ; <float> [#uses=1]
  %mul299 = fmul float %tmp296, %tmp298           ; <float> [#uses=1]
  %vecinit300 = insertelement <4 x float> %vecinit295, float %mul299, i32 1 ; <<4 x float>> [#uses=1]
  %tmp301 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit302 = insertelement <4 x float> %vecinit300, float %tmp301, i32 2 ; <<4 x float>> [#uses=1]
  %tmp303 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp304 = extractelement <4 x float> %tmp303, i32 3 ; <float> [#uses=1]
  %vecinit305 = insertelement <4 x float> %vecinit302, float %tmp304, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit305, <4 x float>* %.compoundliteral290
  %tmp306 = load <4 x float>* %.compoundliteral290 ; <<4 x float>> [#uses=1]
  %tmp307 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx308 = getelementptr inbounds <4 x float> addrspace(1)* %tmp307, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp306, <4 x float> addrspace(1)* %arrayidx308
  br label %if.end309

if.end309:                                        ; preds = %if.then242, %land.lhs.true, %for.end236
  %tmp310 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool311 = trunc i8 %tmp310 to i1             ; <i1> [#uses=1]
  br i1 %tobool311, label %if.then312, label %if.end426

if.then312:                                       ; preds = %if.end309
  %tmp315 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp315, i32* %column314
  br label %for.cond316

for.cond316:                                      ; preds = %for.inc422, %if.then312
  %tmp317 = load i32* %column314                  ; <i32> [#uses=1]
  %tmp318 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp319 = load i32* %count_x                    ; <i32> [#uses=1]
  %add320 = add i32 %tmp318, %tmp319              ; <i32> [#uses=1]
  %cmp321 = icmp ult i32 %tmp317, %add320         ; <i1> [#uses=1]
  br i1 %cmp321, label %for.body323, label %for.end425

for.body323:                                      ; preds = %for.cond316
  %tmp324 = load i32* %column314                  ; <i32> [#uses=1]
  %sub325 = sub i32 %tmp324, 1                    ; <i32> [#uses=1]
  %tmp326 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx327 = getelementptr inbounds <4 x float> addrspace(1)* %tmp326, i32 %sub325 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp328 = load <4 x float> addrspace(1)* %arrayidx327 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp328, <4 x float>* %m10
  %tmp329 = load i32* %column314                  ; <i32> [#uses=1]
  %tmp330 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx331 = getelementptr inbounds <4 x float> addrspace(1)* %tmp330, i32 %tmp329 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp332 = load <4 x float> addrspace(1)* %arrayidx331 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp332, <4 x float>* %m11
  %tmp333 = load i32* %column314                  ; <i32> [#uses=1]
  %add334 = add nsw i32 %tmp333, 1                ; <i32> [#uses=1]
  %tmp335 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx336 = getelementptr inbounds <4 x float> addrspace(1)* %tmp335, i32 %add334 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp337 = load <4 x float> addrspace(1)* %arrayidx336 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp337, <4 x float>* %m12
  %tmp338 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp339 = load i32* %column314                  ; <i32> [#uses=1]
  %add340 = add i32 %tmp338, %tmp339              ; <i32> [#uses=1]
  %sub341 = sub i32 %add340, 1                    ; <i32> [#uses=1]
  %tmp342 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx343 = getelementptr inbounds <4 x float> addrspace(1)* %tmp342, i32 %sub341 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp344 = load <4 x float> addrspace(1)* %arrayidx343 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp344, <4 x float>* %m20
  %tmp345 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp346 = load i32* %column314                  ; <i32> [#uses=1]
  %add347 = add i32 %tmp345, %tmp346              ; <i32> [#uses=1]
  %tmp348 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx349 = getelementptr inbounds <4 x float> addrspace(1)* %tmp348, i32 %add347 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp350 = load <4 x float> addrspace(1)* %arrayidx349 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp350, <4 x float>* %m21
  %tmp351 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp352 = load i32* %column314                  ; <i32> [#uses=1]
  %add353 = add i32 %tmp351, %tmp352              ; <i32> [#uses=1]
  %add354 = add i32 %add353, 1                    ; <i32> [#uses=1]
  %tmp355 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx356 = getelementptr inbounds <4 x float> addrspace(1)* %tmp355, i32 %add354 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp357 = load <4 x float> addrspace(1)* %arrayidx356 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp357, <4 x float>* %m22
  %tmp359 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp360 = extractelement <4 x float> %tmp359, i32 0 ; <float> [#uses=1]
  %conv361 = fpext float %tmp360 to double        ; <double> [#uses=1]
  %mul362 = fmul double 2.000000e+000, %conv361   ; <double> [#uses=1]
  %tmp363 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp364 = extractelement <4 x float> %tmp363, i32 0 ; <float> [#uses=1]
  %conv365 = fpext float %tmp364 to double        ; <double> [#uses=1]
  %add366 = fadd double %mul362, %conv365         ; <double> [#uses=1]
  %tmp367 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp368 = extractelement <4 x float> %tmp367, i32 0 ; <float> [#uses=1]
  %mul369 = fmul float 2.000000e+000, %tmp368     ; <float> [#uses=1]
  %tmp370 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp371 = extractelement <4 x float> %tmp370, i32 0 ; <float> [#uses=1]
  %add372 = fadd float %mul369, %tmp371           ; <float> [#uses=1]
  %conv373 = fpext float %add372 to double        ; <double> [#uses=1]
  %sub374 = fsub double %add366, %conv373         ; <double> [#uses=1]
  %conv375 = fptrunc double %sub374 to float      ; <float> [#uses=1]
  %vecinit376 = insertelement <2 x float> undef, float %conv375, i32 0 ; <<2 x float>> [#uses=1]
  %tmp377 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp378 = extractelement <4 x float> %tmp377, i32 0 ; <float> [#uses=1]
  %conv379 = fpext float %tmp378 to double        ; <double> [#uses=1]
  %tmp380 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp381 = extractelement <4 x float> %tmp380, i32 0 ; <float> [#uses=1]
  %conv382 = fpext float %tmp381 to double        ; <double> [#uses=1]
  %mul383 = fmul double 2.000000e+000, %conv382   ; <double> [#uses=1]
  %add384 = fadd double %conv379, %mul383         ; <double> [#uses=1]
  %tmp385 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp386 = extractelement <4 x float> %tmp385, i32 0 ; <float> [#uses=1]
  %conv387 = fpext float %tmp386 to double        ; <double> [#uses=1]
  %add388 = fadd double %add384, %conv387         ; <double> [#uses=1]
  %conv389 = fptrunc double %add388 to float      ; <float> [#uses=1]
  %vecinit390 = insertelement <2 x float> %vecinit376, float %conv389, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit390, <2 x float>* %.compoundliteral358
  %tmp391 = load <2 x float>* %.compoundliteral358 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp391, <2 x float>* %sobelXY1
  %tmp392 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul393 = fmul <2 x float> %tmp392, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul393, <2 x float>* %sobelXY1
  %tmp394 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call395 = call float @_Z6lengthDv2_f(<2 x float> %tmp394) ; <float> [#uses=1]
  store float %call395, float* %len1
  %tmp396 = load float* %len1                     ; <float> [#uses=1]
  %tmp397 = load float* %minVal.addr              ; <float> [#uses=1]
  %call398 = call float @_Z3maxff(float %tmp396, float %tmp397) ; <float> [#uses=1]
  store float %call398, float* %mag1
  %tmp399 = load float* %mag1                     ; <float> [#uses=1]
  %tmp400 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call401 = call float @_Z3minff(float %tmp399, float %tmp400) ; <float> [#uses=1]
  store float %call401, float* %mag1
  %tmp403 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp404 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp405 = extractelement <2 x float> %tmp404, i32 0 ; <float> [#uses=1]
  %mul406 = fmul float %tmp403, %tmp405           ; <float> [#uses=1]
  %vecinit407 = insertelement <4 x float> undef, float %mul406, i32 0 ; <<4 x float>> [#uses=1]
  %tmp408 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp409 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp410 = extractelement <2 x float> %tmp409, i32 1 ; <float> [#uses=1]
  %mul411 = fmul float %tmp408, %tmp410           ; <float> [#uses=1]
  %vecinit412 = insertelement <4 x float> %vecinit407, float %mul411, i32 1 ; <<4 x float>> [#uses=1]
  %tmp413 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit414 = insertelement <4 x float> %vecinit412, float %tmp413, i32 2 ; <<4 x float>> [#uses=1]
  %tmp415 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp416 = extractelement <4 x float> %tmp415, i32 3 ; <float> [#uses=1]
  %vecinit417 = insertelement <4 x float> %vecinit414, float %tmp416, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit417, <4 x float>* %.compoundliteral402
  %tmp418 = load <4 x float>* %.compoundliteral402 ; <<4 x float>> [#uses=1]
  %tmp419 = load i32* %column314                  ; <i32> [#uses=1]
  %tmp420 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx421 = getelementptr inbounds <4 x float> addrspace(1)* %tmp420, i32 %tmp419 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp418, <4 x float> addrspace(1)* %arrayidx421
  br label %for.inc422

for.inc422:                                       ; preds = %for.body323
  %tmp423 = load i32* %column314                  ; <i32> [#uses=1]
  %inc424 = add nsw i32 %tmp423, 1                ; <i32> [#uses=1]
  store i32 %inc424, i32* %column314
  br label %for.cond316

for.end425:                                       ; preds = %for.cond316
  br label %if.end426

if.end426:                                        ; preds = %for.end425, %if.end309
  %tmp427 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool428 = trunc i8 %tmp427 to i1             ; <i1> [#uses=1]
  br i1 %tobool428, label %land.lhs.true430, label %if.end513

land.lhs.true430:                                 ; preds = %if.end426
  %tmp431 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool432 = trunc i8 %tmp431 to i1             ; <i1> [#uses=1]
  br i1 %tobool432, label %if.then434, label %if.end513

if.then434:                                       ; preds = %land.lhs.true430
  %tmp435 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub436 = sub i32 %tmp435, 2                    ; <i32> [#uses=1]
  %tmp437 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx438 = getelementptr inbounds <4 x float> addrspace(1)* %tmp437, i32 %sub436 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp439 = load <4 x float> addrspace(1)* %arrayidx438 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp439, <4 x float>* %m10
  %tmp440 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub441 = sub i32 %tmp440, 1                    ; <i32> [#uses=1]
  %tmp442 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx443 = getelementptr inbounds <4 x float> addrspace(1)* %tmp442, i32 %sub441 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp444 = load <4 x float> addrspace(1)* %arrayidx443 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp444, <4 x float>* %m11
  %tmp445 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp446 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add447 = add i32 %tmp445, %tmp446              ; <i32> [#uses=1]
  %sub448 = sub i32 %add447, 2                    ; <i32> [#uses=1]
  %tmp449 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx450 = getelementptr inbounds <4 x float> addrspace(1)* %tmp449, i32 %sub448 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp451 = load <4 x float> addrspace(1)* %arrayidx450 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp451, <4 x float>* %m20
  %tmp452 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp453 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add454 = add i32 %tmp452, %tmp453              ; <i32> [#uses=1]
  %sub455 = sub i32 %add454, 1                    ; <i32> [#uses=1]
  %tmp456 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx457 = getelementptr inbounds <4 x float> addrspace(1)* %tmp456, i32 %sub455 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp458 = load <4 x float> addrspace(1)* %arrayidx457 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp458, <4 x float>* %m21
  %tmp460 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp461 = extractelement <4 x float> %tmp460, i32 0 ; <float> [#uses=1]
  %conv462 = fpext float %tmp461 to double        ; <double> [#uses=1]
  %mul463 = fmul double 2.000000e+000, %conv462   ; <double> [#uses=1]
  %tmp464 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp465 = extractelement <4 x float> %tmp464, i32 0 ; <float> [#uses=1]
  %conv466 = fpext float %tmp465 to double        ; <double> [#uses=1]
  %add467 = fadd double %mul463, %conv466         ; <double> [#uses=1]
  %sub468 = fsub double 0.000000e+000, %add467    ; <double> [#uses=1]
  %conv469 = fptrunc double %sub468 to float      ; <float> [#uses=1]
  %vecinit470 = insertelement <2 x float> undef, float %conv469, i32 0 ; <<2 x float>> [#uses=1]
  %tmp471 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp472 = extractelement <4 x float> %tmp471, i32 0 ; <float> [#uses=1]
  %conv473 = fpext float %tmp472 to double        ; <double> [#uses=1]
  %tmp474 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp475 = extractelement <4 x float> %tmp474, i32 0 ; <float> [#uses=1]
  %conv476 = fpext float %tmp475 to double        ; <double> [#uses=1]
  %mul477 = fmul double 2.000000e+000, %conv476   ; <double> [#uses=1]
  %add478 = fadd double %conv473, %mul477         ; <double> [#uses=1]
  %conv479 = fptrunc double %add478 to float      ; <float> [#uses=1]
  %vecinit480 = insertelement <2 x float> %vecinit470, float %conv479, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit480, <2 x float>* %.compoundliteral459
  %tmp481 = load <2 x float>* %.compoundliteral459 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp481, <2 x float>* %sobelXY1
  %tmp482 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul483 = fmul <2 x float> %tmp482, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul483, <2 x float>* %sobelXY1
  %tmp484 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call485 = call float @_Z6lengthDv2_f(<2 x float> %tmp484) ; <float> [#uses=1]
  store float %call485, float* %len1
  %tmp486 = load float* %len1                     ; <float> [#uses=1]
  %tmp487 = load float* %minVal.addr              ; <float> [#uses=1]
  %call488 = call float @_Z3maxff(float %tmp486, float %tmp487) ; <float> [#uses=1]
  store float %call488, float* %mag1
  %tmp489 = load float* %mag1                     ; <float> [#uses=1]
  %tmp490 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call491 = call float @_Z3minff(float %tmp489, float %tmp490) ; <float> [#uses=1]
  store float %call491, float* %mag1
  %tmp493 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp494 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp495 = extractelement <2 x float> %tmp494, i32 0 ; <float> [#uses=1]
  %mul496 = fmul float %tmp493, %tmp495           ; <float> [#uses=1]
  %vecinit497 = insertelement <4 x float> undef, float %mul496, i32 0 ; <<4 x float>> [#uses=1]
  %tmp498 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp499 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp500 = extractelement <2 x float> %tmp499, i32 1 ; <float> [#uses=1]
  %mul501 = fmul float %tmp498, %tmp500           ; <float> [#uses=1]
  %vecinit502 = insertelement <4 x float> %vecinit497, float %mul501, i32 1 ; <<4 x float>> [#uses=1]
  %tmp503 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit504 = insertelement <4 x float> %vecinit502, float %tmp503, i32 2 ; <<4 x float>> [#uses=1]
  %tmp505 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp506 = extractelement <4 x float> %tmp505, i32 3 ; <float> [#uses=1]
  %vecinit507 = insertelement <4 x float> %vecinit504, float %tmp506, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit507, <4 x float>* %.compoundliteral492
  %tmp508 = load <4 x float>* %.compoundliteral492 ; <<4 x float>> [#uses=1]
  %tmp509 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub510 = sub i32 %tmp509, 1                    ; <i32> [#uses=1]
  %tmp511 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx512 = getelementptr inbounds <4 x float> addrspace(1)* %tmp511, i32 %sub510 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp508, <4 x float> addrspace(1)* %arrayidx512
  br label %if.end513

if.end513:                                        ; preds = %if.then434, %land.lhs.true430, %if.end426
  %tmp514 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool515 = trunc i8 %tmp514 to i1             ; <i1> [#uses=1]
  br i1 %tobool515, label %if.then516, label %if.end641

if.then516:                                       ; preds = %if.end513
  %tmp519 = load i32* %index_y                    ; <i32> [#uses=1]
  store i32 %tmp519, i32* %row518
  br label %for.cond520

for.cond520:                                      ; preds = %for.inc637, %if.then516
  %tmp521 = load i32* %row518                     ; <i32> [#uses=1]
  %tmp522 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp523 = load i32* %count_y                    ; <i32> [#uses=1]
  %add524 = add i32 %tmp522, %tmp523              ; <i32> [#uses=1]
  %cmp525 = icmp ult i32 %tmp521, %add524         ; <i1> [#uses=1]
  br i1 %cmp525, label %for.body527, label %for.end640

for.body527:                                      ; preds = %for.cond520
  %tmp528 = load i32* %row518                     ; <i32> [#uses=1]
  %sub529 = sub i32 %tmp528, 1                    ; <i32> [#uses=1]
  %tmp530 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul531 = mul i32 %sub529, %tmp530              ; <i32> [#uses=1]
  %tmp532 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx533 = getelementptr inbounds <4 x float> addrspace(1)* %tmp532, i32 %mul531 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp534 = load <4 x float> addrspace(1)* %arrayidx533 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp534, <4 x float>* %m01
  %tmp535 = load i32* %row518                     ; <i32> [#uses=1]
  %tmp536 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul537 = mul i32 %tmp535, %tmp536              ; <i32> [#uses=1]
  %tmp538 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx539 = getelementptr inbounds <4 x float> addrspace(1)* %tmp538, i32 %mul537 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp540 = load <4 x float> addrspace(1)* %arrayidx539 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp540, <4 x float>* %m11
  %tmp541 = load i32* %row518                     ; <i32> [#uses=1]
  %add542 = add nsw i32 %tmp541, 1                ; <i32> [#uses=1]
  %tmp543 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul544 = mul i32 %add542, %tmp543              ; <i32> [#uses=1]
  %tmp545 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx546 = getelementptr inbounds <4 x float> addrspace(1)* %tmp545, i32 %mul544 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp547 = load <4 x float> addrspace(1)* %arrayidx546 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp547, <4 x float>* %m21
  %tmp548 = load i32* %row518                     ; <i32> [#uses=1]
  %sub549 = sub i32 %tmp548, 1                    ; <i32> [#uses=1]
  %tmp550 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul551 = mul i32 %sub549, %tmp550              ; <i32> [#uses=1]
  %add552 = add i32 %mul551, 1                    ; <i32> [#uses=1]
  %tmp553 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx554 = getelementptr inbounds <4 x float> addrspace(1)* %tmp553, i32 %add552 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp555 = load <4 x float> addrspace(1)* %arrayidx554 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp555, <4 x float>* %m02
  %tmp556 = load i32* %row518                     ; <i32> [#uses=1]
  %tmp557 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul558 = mul i32 %tmp556, %tmp557              ; <i32> [#uses=1]
  %add559 = add i32 %mul558, 1                    ; <i32> [#uses=1]
  %tmp560 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx561 = getelementptr inbounds <4 x float> addrspace(1)* %tmp560, i32 %add559 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp562 = load <4 x float> addrspace(1)* %arrayidx561 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp562, <4 x float>* %m12
  %tmp563 = load i32* %row518                     ; <i32> [#uses=1]
  %add564 = add nsw i32 %tmp563, 1                ; <i32> [#uses=1]
  %tmp565 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul566 = mul i32 %add564, %tmp565              ; <i32> [#uses=1]
  %add567 = add i32 %mul566, 1                    ; <i32> [#uses=1]
  %tmp568 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx569 = getelementptr inbounds <4 x float> addrspace(1)* %tmp568, i32 %add567 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp570 = load <4 x float> addrspace(1)* %arrayidx569 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp570, <4 x float>* %m22
  %tmp572 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp573 = extractelement <4 x float> %tmp572, i32 0 ; <float> [#uses=1]
  %conv574 = fpext float %tmp573 to double        ; <double> [#uses=1]
  %tmp575 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp576 = extractelement <4 x float> %tmp575, i32 0 ; <float> [#uses=1]
  %conv577 = fpext float %tmp576 to double        ; <double> [#uses=1]
  %mul578 = fmul double 2.000000e+000, %conv577   ; <double> [#uses=1]
  %add579 = fadd double %conv574, %mul578         ; <double> [#uses=1]
  %tmp580 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp581 = extractelement <4 x float> %tmp580, i32 0 ; <float> [#uses=1]
  %conv582 = fpext float %tmp581 to double        ; <double> [#uses=1]
  %add583 = fadd double %add579, %conv582         ; <double> [#uses=1]
  %conv584 = fptrunc double %add583 to float      ; <float> [#uses=1]
  %vecinit585 = insertelement <2 x float> undef, float %conv584, i32 0 ; <<2 x float>> [#uses=1]
  %tmp586 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp587 = extractelement <4 x float> %tmp586, i32 0 ; <float> [#uses=1]
  %mul588 = fmul float 2.000000e+000, %tmp587     ; <float> [#uses=1]
  %tmp589 = load <4 x float>* %m22                ; <<4 x float>> [#uses=1]
  %tmp590 = extractelement <4 x float> %tmp589, i32 0 ; <float> [#uses=1]
  %add591 = fadd float %mul588, %tmp590           ; <float> [#uses=1]
  %conv592 = fpext float %add591 to double        ; <double> [#uses=1]
  %tmp593 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp594 = extractelement <4 x float> %tmp593, i32 0 ; <float> [#uses=1]
  %conv595 = fpext float %tmp594 to double        ; <double> [#uses=1]
  %mul596 = fmul double 2.000000e+000, %conv595   ; <double> [#uses=1]
  %tmp597 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp598 = extractelement <4 x float> %tmp597, i32 0 ; <float> [#uses=1]
  %conv599 = fpext float %tmp598 to double        ; <double> [#uses=1]
  %add600 = fadd double %mul596, %conv599         ; <double> [#uses=1]
  %sub601 = fsub double %conv592, %add600         ; <double> [#uses=1]
  %conv602 = fptrunc double %sub601 to float      ; <float> [#uses=1]
  %vecinit603 = insertelement <2 x float> %vecinit585, float %conv602, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit603, <2 x float>* %.compoundliteral571
  %tmp604 = load <2 x float>* %.compoundliteral571 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp604, <2 x float>* %sobelXY1
  %tmp605 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul606 = fmul <2 x float> %tmp605, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul606, <2 x float>* %sobelXY1
  %tmp607 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call608 = call float @_Z6lengthDv2_f(<2 x float> %tmp607) ; <float> [#uses=1]
  store float %call608, float* %len1
  %tmp609 = load float* %len1                     ; <float> [#uses=1]
  %tmp610 = load float* %minVal.addr              ; <float> [#uses=1]
  %call611 = call float @_Z3maxff(float %tmp609, float %tmp610) ; <float> [#uses=1]
  store float %call611, float* %mag1
  %tmp612 = load float* %mag1                     ; <float> [#uses=1]
  %tmp613 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call614 = call float @_Z3minff(float %tmp612, float %tmp613) ; <float> [#uses=1]
  store float %call614, float* %mag1
  %tmp616 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp617 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp618 = extractelement <2 x float> %tmp617, i32 0 ; <float> [#uses=1]
  %mul619 = fmul float %tmp616, %tmp618           ; <float> [#uses=1]
  %vecinit620 = insertelement <4 x float> undef, float %mul619, i32 0 ; <<4 x float>> [#uses=1]
  %tmp621 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp622 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp623 = extractelement <2 x float> %tmp622, i32 1 ; <float> [#uses=1]
  %mul624 = fmul float %tmp621, %tmp623           ; <float> [#uses=1]
  %vecinit625 = insertelement <4 x float> %vecinit620, float %mul624, i32 1 ; <<4 x float>> [#uses=1]
  %tmp626 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit627 = insertelement <4 x float> %vecinit625, float %tmp626, i32 2 ; <<4 x float>> [#uses=1]
  %tmp628 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp629 = extractelement <4 x float> %tmp628, i32 3 ; <float> [#uses=1]
  %vecinit630 = insertelement <4 x float> %vecinit627, float %tmp629, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit630, <4 x float>* %.compoundliteral615
  %tmp631 = load <4 x float>* %.compoundliteral615 ; <<4 x float>> [#uses=1]
  %tmp632 = load i32* %row518                     ; <i32> [#uses=1]
  %tmp633 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul634 = mul i32 %tmp632, %tmp633              ; <i32> [#uses=1]
  %tmp635 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx636 = getelementptr inbounds <4 x float> addrspace(1)* %tmp635, i32 %mul634 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp631, <4 x float> addrspace(1)* %arrayidx636
  br label %for.inc637

for.inc637:                                       ; preds = %for.body527
  %tmp638 = load i32* %row518                     ; <i32> [#uses=1]
  %inc639 = add nsw i32 %tmp638, 1                ; <i32> [#uses=1]
  store i32 %inc639, i32* %row518
  br label %for.cond520

for.end640:                                       ; preds = %for.cond520
  br label %if.end641

if.end641:                                        ; preds = %for.end640, %if.end513
  %tmp642 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool643 = trunc i8 %tmp642 to i1             ; <i1> [#uses=1]
  br i1 %tobool643, label %land.lhs.true645, label %if.end736

land.lhs.true645:                                 ; preds = %if.end641
  %tmp646 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool647 = trunc i8 %tmp646 to i1             ; <i1> [#uses=1]
  br i1 %tobool647, label %if.then649, label %if.end736

if.then649:                                       ; preds = %land.lhs.true645
  %tmp650 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub651 = sub i32 %tmp650, 2                    ; <i32> [#uses=1]
  %tmp652 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul653 = mul i32 %sub651, %tmp652              ; <i32> [#uses=1]
  %tmp654 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx655 = getelementptr inbounds <4 x float> addrspace(1)* %tmp654, i32 %mul653 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp656 = load <4 x float> addrspace(1)* %arrayidx655 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp656, <4 x float>* %m01
  %tmp657 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub658 = sub i32 %tmp657, 2                    ; <i32> [#uses=1]
  %tmp659 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul660 = mul i32 %sub658, %tmp659              ; <i32> [#uses=1]
  %add661 = add i32 %mul660, 1                    ; <i32> [#uses=1]
  %tmp662 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx663 = getelementptr inbounds <4 x float> addrspace(1)* %tmp662, i32 %add661 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp664 = load <4 x float> addrspace(1)* %arrayidx663 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp664, <4 x float>* %m02
  %tmp665 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub666 = sub i32 %tmp665, 1                    ; <i32> [#uses=1]
  %tmp667 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul668 = mul i32 %sub666, %tmp667              ; <i32> [#uses=1]
  %tmp669 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx670 = getelementptr inbounds <4 x float> addrspace(1)* %tmp669, i32 %mul668 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp671 = load <4 x float> addrspace(1)* %arrayidx670 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp671, <4 x float>* %m11
  %tmp672 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub673 = sub i32 %tmp672, 1                    ; <i32> [#uses=1]
  %tmp674 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul675 = mul i32 %sub673, %tmp674              ; <i32> [#uses=1]
  %add676 = add i32 %mul675, 1                    ; <i32> [#uses=1]
  %tmp677 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx678 = getelementptr inbounds <4 x float> addrspace(1)* %tmp677, i32 %add676 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp679 = load <4 x float> addrspace(1)* %arrayidx678 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp679, <4 x float>* %m12
  %tmp681 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp682 = extractelement <4 x float> %tmp681, i32 0 ; <float> [#uses=1]
  %conv683 = fpext float %tmp682 to double        ; <double> [#uses=1]
  %tmp684 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp685 = extractelement <4 x float> %tmp684, i32 0 ; <float> [#uses=1]
  %conv686 = fpext float %tmp685 to double        ; <double> [#uses=1]
  %mul687 = fmul double 2.000000e+000, %conv686   ; <double> [#uses=1]
  %add688 = fadd double %conv683, %mul687         ; <double> [#uses=1]
  %conv689 = fptrunc double %add688 to float      ; <float> [#uses=1]
  %vecinit690 = insertelement <2 x float> undef, float %conv689, i32 0 ; <<2 x float>> [#uses=1]
  %tmp691 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp692 = extractelement <4 x float> %tmp691, i32 0 ; <float> [#uses=1]
  %conv693 = fpext float %tmp692 to double        ; <double> [#uses=1]
  %mul694 = fmul double 2.000000e+000, %conv693   ; <double> [#uses=1]
  %tmp695 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp696 = extractelement <4 x float> %tmp695, i32 0 ; <float> [#uses=1]
  %conv697 = fpext float %tmp696 to double        ; <double> [#uses=1]
  %add698 = fadd double %mul694, %conv697         ; <double> [#uses=1]
  %sub699 = fsub double 0.000000e+000, %add698    ; <double> [#uses=1]
  %conv700 = fptrunc double %sub699 to float      ; <float> [#uses=1]
  %vecinit701 = insertelement <2 x float> %vecinit690, float %conv700, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit701, <2 x float>* %.compoundliteral680
  %tmp702 = load <2 x float>* %.compoundliteral680 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp702, <2 x float>* %sobelXY1
  %tmp703 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul704 = fmul <2 x float> %tmp703, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul704, <2 x float>* %sobelXY1
  %tmp705 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call706 = call float @_Z6lengthDv2_f(<2 x float> %tmp705) ; <float> [#uses=1]
  store float %call706, float* %len1
  %tmp707 = load float* %len1                     ; <float> [#uses=1]
  %tmp708 = load float* %minVal.addr              ; <float> [#uses=1]
  %call709 = call float @_Z3maxff(float %tmp707, float %tmp708) ; <float> [#uses=1]
  store float %call709, float* %mag1
  %tmp710 = load float* %mag1                     ; <float> [#uses=1]
  %tmp711 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call712 = call float @_Z3minff(float %tmp710, float %tmp711) ; <float> [#uses=1]
  store float %call712, float* %mag1
  %tmp714 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp715 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp716 = extractelement <2 x float> %tmp715, i32 0 ; <float> [#uses=1]
  %mul717 = fmul float %tmp714, %tmp716           ; <float> [#uses=1]
  %vecinit718 = insertelement <4 x float> undef, float %mul717, i32 0 ; <<4 x float>> [#uses=1]
  %tmp719 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp720 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp721 = extractelement <2 x float> %tmp720, i32 1 ; <float> [#uses=1]
  %mul722 = fmul float %tmp719, %tmp721           ; <float> [#uses=1]
  %vecinit723 = insertelement <4 x float> %vecinit718, float %mul722, i32 1 ; <<4 x float>> [#uses=1]
  %tmp724 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit725 = insertelement <4 x float> %vecinit723, float %tmp724, i32 2 ; <<4 x float>> [#uses=1]
  %tmp726 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp727 = extractelement <4 x float> %tmp726, i32 3 ; <float> [#uses=1]
  %vecinit728 = insertelement <4 x float> %vecinit725, float %tmp727, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit728, <4 x float>* %.compoundliteral713
  %tmp729 = load <4 x float>* %.compoundliteral713 ; <<4 x float>> [#uses=1]
  %tmp730 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub731 = sub i32 %tmp730, 1                    ; <i32> [#uses=1]
  %tmp732 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul733 = mul i32 %sub731, %tmp732              ; <i32> [#uses=1]
  %tmp734 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx735 = getelementptr inbounds <4 x float> addrspace(1)* %tmp734, i32 %mul733 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp729, <4 x float> addrspace(1)* %arrayidx735
  br label %if.end736

if.end736:                                        ; preds = %if.then649, %land.lhs.true645, %if.end641
  %tmp737 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool738 = trunc i8 %tmp737 to i1             ; <i1> [#uses=1]
  br i1 %tobool738, label %if.then739, label %if.end884

if.then739:                                       ; preds = %if.end736
  %tmp742 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp742, i32* %column741
  br label %for.cond743

for.cond743:                                      ; preds = %for.inc880, %if.then739
  %tmp744 = load i32* %column741                  ; <i32> [#uses=1]
  %tmp745 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp746 = load i32* %count_x                    ; <i32> [#uses=1]
  %add747 = add i32 %tmp745, %tmp746              ; <i32> [#uses=1]
  %cmp748 = icmp ult i32 %tmp744, %add747         ; <i1> [#uses=1]
  br i1 %cmp748, label %for.body750, label %for.end883

for.body750:                                      ; preds = %for.cond743
  %tmp751 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub752 = sub i32 %tmp751, 2                    ; <i32> [#uses=1]
  %tmp753 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul754 = mul i32 %sub752, %tmp753              ; <i32> [#uses=1]
  %tmp755 = load i32* %column741                  ; <i32> [#uses=1]
  %add756 = add i32 %mul754, %tmp755              ; <i32> [#uses=1]
  %sub757 = sub i32 %add756, 1                    ; <i32> [#uses=1]
  %tmp758 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx759 = getelementptr inbounds <4 x float> addrspace(1)* %tmp758, i32 %sub757 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp760 = load <4 x float> addrspace(1)* %arrayidx759 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp760, <4 x float>* %m00
  %tmp761 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub762 = sub i32 %tmp761, 2                    ; <i32> [#uses=1]
  %tmp763 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul764 = mul i32 %sub762, %tmp763              ; <i32> [#uses=1]
  %tmp765 = load i32* %column741                  ; <i32> [#uses=1]
  %add766 = add i32 %mul764, %tmp765              ; <i32> [#uses=1]
  %tmp767 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx768 = getelementptr inbounds <4 x float> addrspace(1)* %tmp767, i32 %add766 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp769 = load <4 x float> addrspace(1)* %arrayidx768 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp769, <4 x float>* %m01
  %tmp770 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub771 = sub i32 %tmp770, 2                    ; <i32> [#uses=1]
  %tmp772 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul773 = mul i32 %sub771, %tmp772              ; <i32> [#uses=1]
  %tmp774 = load i32* %column741                  ; <i32> [#uses=1]
  %add775 = add i32 %mul773, %tmp774              ; <i32> [#uses=1]
  %add776 = add i32 %add775, 1                    ; <i32> [#uses=1]
  %tmp777 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx778 = getelementptr inbounds <4 x float> addrspace(1)* %tmp777, i32 %add776 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp779 = load <4 x float> addrspace(1)* %arrayidx778 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp779, <4 x float>* %m02
  %tmp780 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub781 = sub i32 %tmp780, 1                    ; <i32> [#uses=1]
  %tmp782 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul783 = mul i32 %sub781, %tmp782              ; <i32> [#uses=1]
  %tmp784 = load i32* %column741                  ; <i32> [#uses=1]
  %add785 = add i32 %mul783, %tmp784              ; <i32> [#uses=1]
  %sub786 = sub i32 %add785, 1                    ; <i32> [#uses=1]
  %tmp787 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx788 = getelementptr inbounds <4 x float> addrspace(1)* %tmp787, i32 %sub786 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp789 = load <4 x float> addrspace(1)* %arrayidx788 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp789, <4 x float>* %m10
  %tmp790 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub791 = sub i32 %tmp790, 1                    ; <i32> [#uses=1]
  %tmp792 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul793 = mul i32 %sub791, %tmp792              ; <i32> [#uses=1]
  %tmp794 = load i32* %column741                  ; <i32> [#uses=1]
  %add795 = add i32 %mul793, %tmp794              ; <i32> [#uses=1]
  %tmp796 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx797 = getelementptr inbounds <4 x float> addrspace(1)* %tmp796, i32 %add795 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp798 = load <4 x float> addrspace(1)* %arrayidx797 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp798, <4 x float>* %m11
  %tmp799 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub800 = sub i32 %tmp799, 1                    ; <i32> [#uses=1]
  %tmp801 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul802 = mul i32 %sub800, %tmp801              ; <i32> [#uses=1]
  %tmp803 = load i32* %column741                  ; <i32> [#uses=1]
  %add804 = add i32 %mul802, %tmp803              ; <i32> [#uses=1]
  %add805 = add i32 %add804, 1                    ; <i32> [#uses=1]
  %tmp806 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx807 = getelementptr inbounds <4 x float> addrspace(1)* %tmp806, i32 %add805 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp808 = load <4 x float> addrspace(1)* %arrayidx807 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp808, <4 x float>* %m12
  %tmp810 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp811 = extractelement <4 x float> %tmp810, i32 0 ; <float> [#uses=1]
  %conv812 = fpext float %tmp811 to double        ; <double> [#uses=1]
  %tmp813 = load <4 x float>* %m12                ; <<4 x float>> [#uses=1]
  %tmp814 = extractelement <4 x float> %tmp813, i32 0 ; <float> [#uses=1]
  %conv815 = fpext float %tmp814 to double        ; <double> [#uses=1]
  %mul816 = fmul double 2.000000e+000, %conv815   ; <double> [#uses=1]
  %add817 = fadd double %conv812, %mul816         ; <double> [#uses=1]
  %tmp818 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp819 = extractelement <4 x float> %tmp818, i32 0 ; <float> [#uses=1]
  %conv820 = fpext float %tmp819 to double        ; <double> [#uses=1]
  %tmp821 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp822 = extractelement <4 x float> %tmp821, i32 0 ; <float> [#uses=1]
  %conv823 = fpext float %tmp822 to double        ; <double> [#uses=1]
  %mul824 = fmul double 2.000000e+000, %conv823   ; <double> [#uses=1]
  %add825 = fadd double %conv820, %mul824         ; <double> [#uses=1]
  %sub826 = fsub double %add817, %add825          ; <double> [#uses=1]
  %conv827 = fptrunc double %sub826 to float      ; <float> [#uses=1]
  %vecinit828 = insertelement <2 x float> undef, float %conv827, i32 0 ; <<2 x float>> [#uses=1]
  %tmp829 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp830 = extractelement <4 x float> %tmp829, i32 0 ; <float> [#uses=1]
  %conv831 = fpext float %tmp830 to double        ; <double> [#uses=1]
  %tmp832 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp833 = extractelement <4 x float> %tmp832, i32 0 ; <float> [#uses=1]
  %conv834 = fpext float %tmp833 to double        ; <double> [#uses=1]
  %mul835 = fmul double 2.000000e+000, %conv834   ; <double> [#uses=1]
  %add836 = fadd double %conv831, %mul835         ; <double> [#uses=1]
  %tmp837 = load <4 x float>* %m02                ; <<4 x float>> [#uses=1]
  %tmp838 = extractelement <4 x float> %tmp837, i32 0 ; <float> [#uses=1]
  %conv839 = fpext float %tmp838 to double        ; <double> [#uses=1]
  %add840 = fadd double %add836, %conv839         ; <double> [#uses=1]
  %sub841 = fsub double 0.000000e+000, %add840    ; <double> [#uses=1]
  %conv842 = fptrunc double %sub841 to float      ; <float> [#uses=1]
  %vecinit843 = insertelement <2 x float> %vecinit828, float %conv842, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit843, <2 x float>* %.compoundliteral809
  %tmp844 = load <2 x float>* %.compoundliteral809 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp844, <2 x float>* %sobelXY1
  %tmp845 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul846 = fmul <2 x float> %tmp845, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul846, <2 x float>* %sobelXY1
  %tmp847 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call848 = call float @_Z6lengthDv2_f(<2 x float> %tmp847) ; <float> [#uses=1]
  store float %call848, float* %len1
  %tmp849 = load float* %len1                     ; <float> [#uses=1]
  %tmp850 = load float* %minVal.addr              ; <float> [#uses=1]
  %call851 = call float @_Z3maxff(float %tmp849, float %tmp850) ; <float> [#uses=1]
  store float %call851, float* %mag1
  %tmp852 = load float* %mag1                     ; <float> [#uses=1]
  %tmp853 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call854 = call float @_Z3minff(float %tmp852, float %tmp853) ; <float> [#uses=1]
  store float %call854, float* %mag1
  %tmp856 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp857 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp858 = extractelement <2 x float> %tmp857, i32 0 ; <float> [#uses=1]
  %mul859 = fmul float %tmp856, %tmp858           ; <float> [#uses=1]
  %vecinit860 = insertelement <4 x float> undef, float %mul859, i32 0 ; <<4 x float>> [#uses=1]
  %tmp861 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp862 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp863 = extractelement <2 x float> %tmp862, i32 1 ; <float> [#uses=1]
  %mul864 = fmul float %tmp861, %tmp863           ; <float> [#uses=1]
  %vecinit865 = insertelement <4 x float> %vecinit860, float %mul864, i32 1 ; <<4 x float>> [#uses=1]
  %tmp866 = load float* %mag1                     ; <float> [#uses=1]
  %vecinit867 = insertelement <4 x float> %vecinit865, float %tmp866, i32 2 ; <<4 x float>> [#uses=1]
  %tmp868 = load <4 x float>* %m11                ; <<4 x float>> [#uses=1]
  %tmp869 = extractelement <4 x float> %tmp868, i32 3 ; <float> [#uses=1]
  %vecinit870 = insertelement <4 x float> %vecinit867, float %tmp869, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit870, <4 x float>* %.compoundliteral855
  %tmp871 = load <4 x float>* %.compoundliteral855 ; <<4 x float>> [#uses=1]
  %tmp872 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub873 = sub i32 %tmp872, 1                    ; <i32> [#uses=1]
  %tmp874 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul875 = mul i32 %sub873, %tmp874              ; <i32> [#uses=1]
  %tmp876 = load i32* %column741                  ; <i32> [#uses=1]
  %add877 = add i32 %mul875, %tmp876              ; <i32> [#uses=1]
  %tmp878 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx879 = getelementptr inbounds <4 x float> addrspace(1)* %tmp878, i32 %add877 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp871, <4 x float> addrspace(1)* %arrayidx879
  br label %for.inc880

for.inc880:                                       ; preds = %for.body750
  %tmp881 = load i32* %column741                  ; <i32> [#uses=1]
  %inc882 = add nsw i32 %tmp881, 1                ; <i32> [#uses=1]
  store i32 %inc882, i32* %column741
  br label %for.cond743

for.end883:                                       ; preds = %for.cond743
  br label %if.end884

if.end884:                                        ; preds = %for.end883, %if.end736
  %tmp885 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool886 = trunc i8 %tmp885 to i1             ; <i1> [#uses=1]
  br i1 %tobool886, label %if.then887, label %if.end1019

if.then887:                                       ; preds = %if.end884
  %tmp890 = load i32* %index_y                    ; <i32> [#uses=1]
  store i32 %tmp890, i32* %row889
  br label %for.cond891

for.cond891:                                      ; preds = %for.inc1015, %if.then887
  %tmp892 = load i32* %row889                     ; <i32> [#uses=1]
  %tmp893 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp894 = load i32* %count_y                    ; <i32> [#uses=1]
  %add895 = add i32 %tmp893, %tmp894              ; <i32> [#uses=1]
  %cmp896 = icmp ult i32 %tmp892, %add895         ; <i1> [#uses=1]
  br i1 %cmp896, label %for.body898, label %for.end1018

for.body898:                                      ; preds = %for.cond891
  %tmp899 = load i32* %row889                     ; <i32> [#uses=1]
  %tmp900 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul901 = mul i32 %tmp899, %tmp900              ; <i32> [#uses=1]
  %sub902 = sub i32 %mul901, 1                    ; <i32> [#uses=1]
  %tmp903 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx904 = getelementptr inbounds <4 x float> addrspace(1)* %tmp903, i32 %sub902 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp905 = load <4 x float> addrspace(1)* %arrayidx904 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp905, <4 x float>* %m01
  %tmp906 = load i32* %row889                     ; <i32> [#uses=1]
  %add907 = add nsw i32 %tmp906, 1                ; <i32> [#uses=1]
  %tmp908 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul909 = mul i32 %add907, %tmp908              ; <i32> [#uses=1]
  %sub910 = sub i32 %mul909, 1                    ; <i32> [#uses=1]
  %tmp911 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx912 = getelementptr inbounds <4 x float> addrspace(1)* %tmp911, i32 %sub910 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp913 = load <4 x float> addrspace(1)* %arrayidx912 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp913, <4 x float>* %m11
  %tmp914 = load i32* %row889                     ; <i32> [#uses=1]
  %add915 = add nsw i32 %tmp914, 2                ; <i32> [#uses=1]
  %tmp916 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul917 = mul i32 %add915, %tmp916              ; <i32> [#uses=1]
  %sub918 = sub i32 %mul917, 1                    ; <i32> [#uses=1]
  %tmp919 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx920 = getelementptr inbounds <4 x float> addrspace(1)* %tmp919, i32 %sub918 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp921 = load <4 x float> addrspace(1)* %arrayidx920 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp921, <4 x float>* %m21
  %tmp922 = load i32* %row889                     ; <i32> [#uses=1]
  %tmp923 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul924 = mul i32 %tmp922, %tmp923              ; <i32> [#uses=1]
  %sub925 = sub i32 %mul924, 2                    ; <i32> [#uses=1]
  %tmp926 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx927 = getelementptr inbounds <4 x float> addrspace(1)* %tmp926, i32 %sub925 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp928 = load <4 x float> addrspace(1)* %arrayidx927 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp928, <4 x float>* %m00
  %tmp929 = load i32* %row889                     ; <i32> [#uses=1]
  %add930 = add nsw i32 %tmp929, 1                ; <i32> [#uses=1]
  %tmp931 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul932 = mul i32 %add930, %tmp931              ; <i32> [#uses=1]
  %sub933 = sub i32 %mul932, 2                    ; <i32> [#uses=1]
  %tmp934 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx935 = getelementptr inbounds <4 x float> addrspace(1)* %tmp934, i32 %sub933 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp936 = load <4 x float> addrspace(1)* %arrayidx935 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp936, <4 x float>* %m10
  %tmp937 = load i32* %row889                     ; <i32> [#uses=1]
  %add938 = add nsw i32 %tmp937, 2                ; <i32> [#uses=1]
  %tmp939 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul940 = mul i32 %add938, %tmp939              ; <i32> [#uses=1]
  %sub941 = sub i32 %mul940, 2                    ; <i32> [#uses=1]
  %tmp942 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx943 = getelementptr inbounds <4 x float> addrspace(1)* %tmp942, i32 %sub941 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp944 = load <4 x float> addrspace(1)* %arrayidx943 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp944, <4 x float>* %m20
  %tmp946 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp947 = extractelement <4 x float> %tmp946, i32 0 ; <float> [#uses=1]
  %conv948 = fpext float %tmp947 to double        ; <double> [#uses=1]
  %tmp949 = load <4 x float>* %m10                ; <<4 x float>> [#uses=1]
  %tmp950 = extractelement <4 x float> %tmp949, i32 0 ; <float> [#uses=1]
  %conv951 = fpext float %tmp950 to double        ; <double> [#uses=1]
  %mul952 = fmul double 2.000000e+000, %conv951   ; <double> [#uses=1]
  %add953 = fadd double %conv948, %mul952         ; <double> [#uses=1]
  %tmp954 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp955 = extractelement <4 x float> %tmp954, i32 0 ; <float> [#uses=1]
  %conv956 = fpext float %tmp955 to double        ; <double> [#uses=1]
  %add957 = fadd double %add953, %conv956         ; <double> [#uses=1]
  %sub958 = fsub double 0.000000e+000, %add957    ; <double> [#uses=1]
  %conv959 = fptrunc double %sub958 to float      ; <float> [#uses=1]
  %vecinit960 = insertelement <2 x float> undef, float %conv959, i32 0 ; <<2 x float>> [#uses=1]
  %tmp961 = load <4 x float>* %m20                ; <<4 x float>> [#uses=1]
  %tmp962 = extractelement <4 x float> %tmp961, i32 0 ; <float> [#uses=1]
  %conv963 = fpext float %tmp962 to double        ; <double> [#uses=1]
  %tmp964 = load <4 x float>* %m21                ; <<4 x float>> [#uses=1]
  %tmp965 = extractelement <4 x float> %tmp964, i32 0 ; <float> [#uses=1]
  %conv966 = fpext float %tmp965 to double        ; <double> [#uses=1]
  %mul967 = fmul double 2.000000e+000, %conv966   ; <double> [#uses=1]
  %add968 = fadd double %conv963, %mul967         ; <double> [#uses=1]
  %tmp969 = load <4 x float>* %m00                ; <<4 x float>> [#uses=1]
  %tmp970 = extractelement <4 x float> %tmp969, i32 0 ; <float> [#uses=1]
  %conv971 = fpext float %tmp970 to double        ; <double> [#uses=1]
  %tmp972 = load <4 x float>* %m01                ; <<4 x float>> [#uses=1]
  %tmp973 = extractelement <4 x float> %tmp972, i32 0 ; <float> [#uses=1]
  %conv974 = fpext float %tmp973 to double        ; <double> [#uses=1]
  %mul975 = fmul double 2.000000e+000, %conv974   ; <double> [#uses=1]
  %add976 = fadd double %conv971, %mul975         ; <double> [#uses=1]
  %sub977 = fsub double %add968, %add976          ; <double> [#uses=1]
  %conv978 = fptrunc double %sub977 to float      ; <float> [#uses=1]
  %vecinit979 = insertelement <2 x float> %vecinit960, float %conv978, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit979, <2 x float>* %.compoundliteral945
  %tmp980 = load <2 x float>* %.compoundliteral945 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp980, <2 x float>* %sobelXY1
  %tmp981 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %mul982 = fmul <2 x float> %tmp981, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul982, <2 x float>* %sobelXY1
  %tmp983 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %call984 = call float @_Z6lengthDv2_f(<2 x float> %tmp983) ; <float> [#uses=1]
  store float %call984, float* %len1
  %tmp985 = load float* %len1                     ; <float> [#uses=1]
  %tmp986 = load float* %minVal.addr              ; <float> [#uses=1]
  %call987 = call float @_Z3maxff(float %tmp985, float %tmp986) ; <float> [#uses=1]
  store float %call987, float* %mag1
  %tmp988 = load float* %mag1                     ; <float> [#uses=1]
  %tmp989 = load float* %maxVal.addr              ; <float> [#uses=1]
  %call990 = call float @_Z3minff(float %tmp988, float %tmp989) ; <float> [#uses=1]
  store float %call990, float* %mag1
  %tmp992 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp993 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp994 = extractelement <2 x float> %tmp993, i32 0 ; <float> [#uses=1]
  %mul995 = fmul float %tmp992, %tmp994           ; <float> [#uses=1]
  %vecinit996 = insertelement <4 x float> undef, float %mul995, i32 0 ; <<4 x float>> [#uses=1]
  %tmp997 = load float* %sobelScale.addr          ; <float> [#uses=1]
  %tmp998 = load <2 x float>* %sobelXY1           ; <<2 x float>> [#uses=1]
  %tmp999 = extractelement <2 x float> %tmp998, i32 1 ; <float> [#uses=1]
  %mul1000 = fmul float %tmp997, %tmp999          ; <float> [#uses=1]
  %vecinit1001 = insertelement <4 x float> %vecinit996, float %mul1000, i32 1 ; <<4 x float>> [#uses=1]
  %tmp1002 = load float* %mag1                    ; <float> [#uses=1]
  %vecinit1003 = insertelement <4 x float> %vecinit1001, float %tmp1002, i32 2 ; <<4 x float>> [#uses=1]
  %tmp1004 = load <4 x float>* %m11               ; <<4 x float>> [#uses=1]
  %tmp1005 = extractelement <4 x float> %tmp1004, i32 3 ; <float> [#uses=1]
  %vecinit1006 = insertelement <4 x float> %vecinit1003, float %tmp1005, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit1006, <4 x float>* %.compoundliteral991
  %tmp1007 = load <4 x float>* %.compoundliteral991 ; <<4 x float>> [#uses=1]
  %tmp1008 = load i32* %row889                    ; <i32> [#uses=1]
  %add1009 = add nsw i32 %tmp1008, 1              ; <i32> [#uses=1]
  %tmp1010 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1011 = mul i32 %add1009, %tmp1010           ; <i32> [#uses=1]
  %sub1012 = sub i32 %mul1011, 1                  ; <i32> [#uses=1]
  %tmp1013 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1014 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1013, i32 %sub1012 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp1007, <4 x float> addrspace(1)* %arrayidx1014
  br label %for.inc1015

for.inc1015:                                      ; preds = %for.body898
  %tmp1016 = load i32* %row889                    ; <i32> [#uses=1]
  %inc1017 = add nsw i32 %tmp1016, 1              ; <i32> [#uses=1]
  store i32 %inc1017, i32* %row889
  br label %for.cond891

for.end1018:                                      ; preds = %for.cond891
  br label %if.end1019

if.end1019:                                       ; preds = %for.end1018, %if.end884
  %tmp1020 = load i8* %bottomEdge                 ; <i8> [#uses=1]
  %tobool1021 = trunc i8 %tmp1020 to i1           ; <i1> [#uses=1]
  br i1 %tobool1021, label %land.lhs.true1023, label %if.end1115

land.lhs.true1023:                                ; preds = %if.end1019
  %tmp1024 = load i8* %rightEdge                  ; <i8> [#uses=1]
  %tobool1025 = trunc i8 %tmp1024 to i1           ; <i1> [#uses=1]
  br i1 %tobool1025, label %if.then1027, label %if.end1115

if.then1027:                                      ; preds = %land.lhs.true1023
  %tmp1028 = load i32* %height.addr               ; <i32> [#uses=1]
  %sub1029 = sub i32 %tmp1028, 1                  ; <i32> [#uses=1]
  %tmp1030 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1031 = mul i32 %sub1029, %tmp1030           ; <i32> [#uses=1]
  %sub1032 = sub i32 %mul1031, 2                  ; <i32> [#uses=1]
  %tmp1033 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1034 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1033, i32 %sub1032 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1035 = load <4 x float> addrspace(1)* %arrayidx1034 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1035, <4 x float>* %m00
  %tmp1036 = load i32* %height.addr               ; <i32> [#uses=1]
  %sub1037 = sub i32 %tmp1036, 1                  ; <i32> [#uses=1]
  %tmp1038 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1039 = mul i32 %sub1037, %tmp1038           ; <i32> [#uses=1]
  %sub1040 = sub i32 %mul1039, 1                  ; <i32> [#uses=1]
  %tmp1041 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1042 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1041, i32 %sub1040 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1043 = load <4 x float> addrspace(1)* %arrayidx1042 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1043, <4 x float>* %m01
  %tmp1044 = load i32* %height.addr               ; <i32> [#uses=1]
  %tmp1045 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1046 = mul i32 %tmp1044, %tmp1045           ; <i32> [#uses=1]
  %sub1047 = sub i32 %mul1046, 2                  ; <i32> [#uses=1]
  %tmp1048 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1049 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1048, i32 %sub1047 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1050 = load <4 x float> addrspace(1)* %arrayidx1049 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1050, <4 x float>* %m10
  %tmp1051 = load i32* %height.addr               ; <i32> [#uses=1]
  %tmp1052 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1053 = mul i32 %tmp1051, %tmp1052           ; <i32> [#uses=1]
  %sub1054 = sub i32 %mul1053, 1                  ; <i32> [#uses=1]
  %tmp1055 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1056 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1055, i32 %sub1054 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1057 = load <4 x float> addrspace(1)* %arrayidx1056 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1057, <4 x float>* %m11
  %tmp1059 = load <4 x float>* %m00               ; <<4 x float>> [#uses=1]
  %tmp1060 = extractelement <4 x float> %tmp1059, i32 0 ; <float> [#uses=1]
  %conv1061 = fpext float %tmp1060 to double      ; <double> [#uses=1]
  %tmp1062 = load <4 x float>* %m10               ; <<4 x float>> [#uses=1]
  %tmp1063 = extractelement <4 x float> %tmp1062, i32 0 ; <float> [#uses=1]
  %conv1064 = fpext float %tmp1063 to double      ; <double> [#uses=1]
  %mul1065 = fmul double 2.000000e+000, %conv1064 ; <double> [#uses=1]
  %add1066 = fadd double %conv1061, %mul1065      ; <double> [#uses=1]
  %sub1067 = fsub double 0.000000e+000, %add1066  ; <double> [#uses=1]
  %conv1068 = fptrunc double %sub1067 to float    ; <float> [#uses=1]
  %vecinit1069 = insertelement <2 x float> undef, float %conv1068, i32 0 ; <<2 x float>> [#uses=1]
  %tmp1070 = load <4 x float>* %m00               ; <<4 x float>> [#uses=1]
  %tmp1071 = extractelement <4 x float> %tmp1070, i32 0 ; <float> [#uses=1]
  %conv1072 = fpext float %tmp1071 to double      ; <double> [#uses=1]
  %tmp1073 = load <4 x float>* %m01               ; <<4 x float>> [#uses=1]
  %tmp1074 = extractelement <4 x float> %tmp1073, i32 0 ; <float> [#uses=1]
  %conv1075 = fpext float %tmp1074 to double      ; <double> [#uses=1]
  %mul1076 = fmul double 2.000000e+000, %conv1075 ; <double> [#uses=1]
  %add1077 = fadd double %conv1072, %mul1076      ; <double> [#uses=1]
  %sub1078 = fsub double 0.000000e+000, %add1077  ; <double> [#uses=1]
  %conv1079 = fptrunc double %sub1078 to float    ; <float> [#uses=1]
  %vecinit1080 = insertelement <2 x float> %vecinit1069, float %conv1079, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit1080, <2 x float>* %.compoundliteral1058
  %tmp1081 = load <2 x float>* %.compoundliteral1058 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp1081, <2 x float>* %sobelXY1
  %tmp1082 = load <2 x float>* %sobelXY1          ; <<2 x float>> [#uses=1]
  %mul1083 = fmul <2 x float> %tmp1082, <float 1.250000e-001, float 1.250000e-001> ; <<2 x float>> [#uses=1]
  store <2 x float> %mul1083, <2 x float>* %sobelXY1
  %tmp1084 = load <2 x float>* %sobelXY1          ; <<2 x float>> [#uses=1]
  %call1085 = call float @_Z6lengthDv2_f(<2 x float> %tmp1084) ; <float> [#uses=1]
  store float %call1085, float* %len1
  %tmp1086 = load float* %len1                    ; <float> [#uses=1]
  %tmp1087 = load float* %minVal.addr             ; <float> [#uses=1]
  %call1088 = call float @_Z3maxff(float %tmp1086, float %tmp1087) ; <float> [#uses=1]
  store float %call1088, float* %mag1
  %tmp1089 = load float* %mag1                    ; <float> [#uses=1]
  %tmp1090 = load float* %maxVal.addr             ; <float> [#uses=1]
  %call1091 = call float @_Z3minff(float %tmp1089, float %tmp1090) ; <float> [#uses=1]
  store float %call1091, float* %mag1
  %tmp1093 = load float* %sobelScale.addr         ; <float> [#uses=1]
  %tmp1094 = load <2 x float>* %sobelXY1          ; <<2 x float>> [#uses=1]
  %tmp1095 = extractelement <2 x float> %tmp1094, i32 0 ; <float> [#uses=1]
  %mul1096 = fmul float %tmp1093, %tmp1095        ; <float> [#uses=1]
  %vecinit1097 = insertelement <4 x float> undef, float %mul1096, i32 0 ; <<4 x float>> [#uses=1]
  %tmp1098 = load float* %sobelScale.addr         ; <float> [#uses=1]
  %tmp1099 = load <2 x float>* %sobelXY1          ; <<2 x float>> [#uses=1]
  %tmp1100 = extractelement <2 x float> %tmp1099, i32 1 ; <float> [#uses=1]
  %mul1101 = fmul float %tmp1098, %tmp1100        ; <float> [#uses=1]
  %vecinit1102 = insertelement <4 x float> %vecinit1097, float %mul1101, i32 1 ; <<4 x float>> [#uses=1]
  %tmp1103 = load float* %mag1                    ; <float> [#uses=1]
  %vecinit1104 = insertelement <4 x float> %vecinit1102, float %tmp1103, i32 2 ; <<4 x float>> [#uses=1]
  %tmp1105 = load <4 x float>* %m11               ; <<4 x float>> [#uses=1]
  %tmp1106 = extractelement <4 x float> %tmp1105, i32 3 ; <float> [#uses=1]
  %vecinit1107 = insertelement <4 x float> %vecinit1104, float %tmp1106, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit1107, <4 x float>* %.compoundliteral1092
  %tmp1108 = load <4 x float>* %.compoundliteral1092 ; <<4 x float>> [#uses=1]
  %tmp1109 = load i32* %height.addr               ; <i32> [#uses=1]
  %tmp1110 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1111 = mul i32 %tmp1109, %tmp1110           ; <i32> [#uses=1]
  %sub1112 = sub i32 %mul1111, 1                  ; <i32> [#uses=1]
  %tmp1113 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1114 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1113, i32 %sub1112 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %tmp1108, <4 x float> addrspace(1)* %arrayidx1114
  br label %if.end1115

if.end1115:                                       ; preds = %if.then1027, %land.lhs.true1023, %if.end1019
  ret void
}
