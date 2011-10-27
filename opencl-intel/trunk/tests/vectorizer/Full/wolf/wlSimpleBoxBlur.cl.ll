; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlSimpleBoxBlur.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@opencl_wlSimpleBoxBlur_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSimpleBoxBlur_GPU_parameters = appending global [123 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[123 x i8]*> [#uses=1]
@opencl_wlSimpleBoxBlur_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSimpleBoxBlur_CPU_parameters = appending global [123 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[123 x i8]*> [#uses=1]
@opencl_wlSimpleBoxBlur_image2d_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSimpleBoxBlur_image2d_parameters = appending global [71 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[71 x i8]*> [#uses=1]
@opencl_wlSimpleBoxBlur_Optimized_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSimpleBoxBlur_Optimized_CPU_parameters = appending global [123 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[123 x i8]*> [#uses=1]
@opencl_metadata = appending global [4 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSimpleBoxBlur_GPU_locals to i8*), i8* getelementptr inbounds ([123 x i8]* @opencl_wlSimpleBoxBlur_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSimpleBoxBlur_CPU_locals to i8*), i8* getelementptr inbounds ([123 x i8]* @opencl_wlSimpleBoxBlur_CPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSimpleBoxBlur_image2d_locals to i8*), i8* getelementptr inbounds ([71 x i8]* @opencl_wlSimpleBoxBlur_image2d_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSimpleBoxBlur_Optimized_CPU_locals to i8*), i8* getelementptr inbounds ([123 x i8]* @opencl_wlSimpleBoxBlur_Optimized_CPU_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[4 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=1]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %dims = alloca i32, align 4                     ; <i32*> [#uses=1]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=10]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=6]
  %localIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %localIdy = alloca i32, align 4                 ; <i32*> [#uses=1]
  %global_szx = alloca i32, align 4               ; <i32*> [#uses=4]
  %global_szy = alloca i32, align 4               ; <i32*> [#uses=2]
  %local_szx = alloca i32, align 4                ; <i32*> [#uses=1]
  %local_szy = alloca i32, align 4                ; <i32*> [#uses=1]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=1]
  %denominator = alloca float, align 4            ; <float*> [#uses=2]
  %colorAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=19]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=5]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=4]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=4]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  %call = call i32 (...)* @get_work_dim()         ; <i32> [#uses=1]
  store i32 %call, i32* %dims
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdx
  %call2 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call2, i32* %globalIdy
  %call3 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call3, i32* %localIdx
  %call4 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call4, i32* %localIdy
  %call5 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call5, i32* %global_szx
  %call6 = call i32 @get_global_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call6, i32* %global_szy
  %call7 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call7, i32* %local_szx
  %call8 = call i32 @get_local_size(i32 1)        ; <i32> [#uses=1]
  store i32 %call8, i32* %local_szy
  %call9 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call9, i32* %groupIdx
  %call10 = call i32 @get_group_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call10, i32* %groupIdy
  store float 9.000000e+000, float* %denominator
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp = load i32* %globalIdy                     ; <i32> [#uses=1]
  %tmp11 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp11                     ; <i32> [#uses=1]
  %tmp12 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp12                     ; <i32> [#uses=1]
  store i32 %add, i32* %sourceIndex
  %tmp14 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %sub = sub i32 %tmp14, 1                        ; <i32> [#uses=1]
  %tmp15 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul16 = mul i32 %sub, %tmp15                   ; <i32> [#uses=1]
  %tmp17 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add18 = add i32 %mul16, %tmp17                 ; <i32> [#uses=1]
  store i32 %add18, i32* %tmpIndex1
  %tmp20 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %add21 = add i32 %tmp20, 1                      ; <i32> [#uses=1]
  %tmp22 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul23 = mul i32 %add21, %tmp22                 ; <i32> [#uses=1]
  %tmp24 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %add25 = add i32 %mul23, %tmp24                 ; <i32> [#uses=1]
  store i32 %add25, i32* %tmpIndex2
  %tmp26 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp27 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp27, i32 %tmp26 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp28 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp28, <4 x float>* %colorAccumulator
  %tmp29 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %cmp = icmp ugt i32 %tmp29, 0                   ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp30 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %sub31 = sub i32 %tmp30, 1                      ; <i32> [#uses=1]
  %tmp32 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %tmp32, i32 %sub31 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp34 = load <4 x float> addrspace(1)* %arrayidx33 ; <<4 x float>> [#uses=1]
  %tmp35 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add36 = fadd <4 x float> %tmp35, %tmp34        ; <<4 x float>> [#uses=1]
  store <4 x float> %add36, <4 x float>* %colorAccumulator
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp37 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %tmp38 = load i32* %global_szx                  ; <i32> [#uses=1]
  %sub39 = sub i32 %tmp38, 1                      ; <i32> [#uses=1]
  %cmp40 = icmp ult i32 %tmp37, %sub39            ; <i1> [#uses=1]
  br i1 %cmp40, label %if.then41, label %if.end49

if.then41:                                        ; preds = %if.end
  %tmp42 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %add43 = add i32 %tmp42, 1                      ; <i32> [#uses=1]
  %tmp44 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx45 = getelementptr inbounds <4 x float> addrspace(1)* %tmp44, i32 %add43 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp46 = load <4 x float> addrspace(1)* %arrayidx45 ; <<4 x float>> [#uses=1]
  %tmp47 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add48 = fadd <4 x float> %tmp47, %tmp46        ; <<4 x float>> [#uses=1]
  store <4 x float> %add48, <4 x float>* %colorAccumulator
  br label %if.end49

if.end49:                                         ; preds = %if.then41, %if.end
  %tmp50 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %cmp51 = icmp ugt i32 %tmp50, 0                 ; <i1> [#uses=1]
  br i1 %cmp51, label %if.then52, label %if.end83

if.then52:                                        ; preds = %if.end49
  %tmp53 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %tmp54 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds <4 x float> addrspace(1)* %tmp54, i32 %tmp53 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp56 = load <4 x float> addrspace(1)* %arrayidx55 ; <<4 x float>> [#uses=1]
  %tmp57 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add58 = fadd <4 x float> %tmp57, %tmp56        ; <<4 x float>> [#uses=1]
  store <4 x float> %add58, <4 x float>* %colorAccumulator
  %tmp59 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %cmp60 = icmp ugt i32 %tmp59, 0                 ; <i1> [#uses=1]
  br i1 %cmp60, label %if.then61, label %if.end69

if.then61:                                        ; preds = %if.then52
  %tmp62 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %sub63 = sub i32 %tmp62, 1                      ; <i32> [#uses=1]
  %tmp64 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx65 = getelementptr inbounds <4 x float> addrspace(1)* %tmp64, i32 %sub63 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp66 = load <4 x float> addrspace(1)* %arrayidx65 ; <<4 x float>> [#uses=1]
  %tmp67 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add68 = fadd <4 x float> %tmp67, %tmp66        ; <<4 x float>> [#uses=1]
  store <4 x float> %add68, <4 x float>* %colorAccumulator
  br label %if.end69

if.end69:                                         ; preds = %if.then61, %if.then52
  %tmp70 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %tmp71 = load i32* %global_szx                  ; <i32> [#uses=1]
  %sub72 = sub i32 %tmp71, 1                      ; <i32> [#uses=1]
  %cmp73 = icmp ult i32 %tmp70, %sub72            ; <i1> [#uses=1]
  br i1 %cmp73, label %if.then74, label %if.end82

if.then74:                                        ; preds = %if.end69
  %tmp75 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %add76 = add i32 %tmp75, 1                      ; <i32> [#uses=1]
  %tmp77 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds <4 x float> addrspace(1)* %tmp77, i32 %add76 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp79 = load <4 x float> addrspace(1)* %arrayidx78 ; <<4 x float>> [#uses=1]
  %tmp80 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add81 = fadd <4 x float> %tmp80, %tmp79        ; <<4 x float>> [#uses=1]
  store <4 x float> %add81, <4 x float>* %colorAccumulator
  br label %if.end82

if.end82:                                         ; preds = %if.then74, %if.end69
  br label %if.end83

if.end83:                                         ; preds = %if.end82, %if.end49
  %tmp84 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %tmp85 = load i32* %global_szy                  ; <i32> [#uses=1]
  %sub86 = sub i32 %tmp85, 1                      ; <i32> [#uses=1]
  %cmp87 = icmp ult i32 %tmp84, %sub86            ; <i1> [#uses=1]
  br i1 %cmp87, label %if.then88, label %if.end119

if.then88:                                        ; preds = %if.end83
  %tmp89 = load i32* %tmpIndex2                   ; <i32> [#uses=1]
  %tmp90 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds <4 x float> addrspace(1)* %tmp90, i32 %tmp89 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp92 = load <4 x float> addrspace(1)* %arrayidx91 ; <<4 x float>> [#uses=1]
  %tmp93 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add94 = fadd <4 x float> %tmp93, %tmp92        ; <<4 x float>> [#uses=1]
  store <4 x float> %add94, <4 x float>* %colorAccumulator
  %tmp95 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %cmp96 = icmp ugt i32 %tmp95, 0                 ; <i1> [#uses=1]
  br i1 %cmp96, label %if.then97, label %if.end105

if.then97:                                        ; preds = %if.then88
  %tmp98 = load i32* %tmpIndex2                   ; <i32> [#uses=1]
  %sub99 = sub i32 %tmp98, 1                      ; <i32> [#uses=1]
  %tmp100 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx101 = getelementptr inbounds <4 x float> addrspace(1)* %tmp100, i32 %sub99 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp102 = load <4 x float> addrspace(1)* %arrayidx101 ; <<4 x float>> [#uses=1]
  %tmp103 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add104 = fadd <4 x float> %tmp103, %tmp102     ; <<4 x float>> [#uses=1]
  store <4 x float> %add104, <4 x float>* %colorAccumulator
  br label %if.end105

if.end105:                                        ; preds = %if.then97, %if.then88
  %tmp106 = load i32* %globalIdx                  ; <i32> [#uses=1]
  %tmp107 = load i32* %global_szx                 ; <i32> [#uses=1]
  %sub108 = sub i32 %tmp107, 1                    ; <i32> [#uses=1]
  %cmp109 = icmp ult i32 %tmp106, %sub108         ; <i1> [#uses=1]
  br i1 %cmp109, label %if.then110, label %if.end118

if.then110:                                       ; preds = %if.end105
  %tmp111 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add112 = add i32 %tmp111, 1                    ; <i32> [#uses=1]
  %tmp113 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds <4 x float> addrspace(1)* %tmp113, i32 %add112 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp115 = load <4 x float> addrspace(1)* %arrayidx114 ; <<4 x float>> [#uses=1]
  %tmp116 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add117 = fadd <4 x float> %tmp116, %tmp115     ; <<4 x float>> [#uses=1]
  store <4 x float> %add117, <4 x float>* %colorAccumulator
  br label %if.end118

if.end118:                                        ; preds = %if.then110, %if.end105
  br label %if.end119

if.end119:                                        ; preds = %if.end118, %if.end83
  %tmp120 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp121 = load float* %denominator              ; <float> [#uses=1]
  %tmp122 = insertelement <4 x float> undef, float %tmp121, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp122, <4 x float> %tmp122, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp123 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp123, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp120, %splat         ; <<4 x float>> [#uses=1]
  %tmp124 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp125 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds <4 x float> addrspace(1)* %tmp125, i32 %tmp124 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx126
  ret void
}

declare i32 @get_work_dim(...)

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare i32 @get_global_size(i32)

declare i32 @get_local_size(i32)

declare i32 @get_group_id(i32)

; CHECK: ret
define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=10]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=9]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=4]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
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
  %denominator = alloca float, align 4            ; <float*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  %j = alloca i32, align 4                        ; <i32*> [#uses=4]
  %index_x42 = alloca i32, align 4                ; <i32*> [#uses=12]
  %colorAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=19]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=5]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=4]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=4]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  %call = call i32 (...)* @get_work_dim()         ; <i32> [#uses=1]
  store i32 %call, i32* %dims
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdx
  %call2 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call2, i32* %globalIdy
  %call3 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call3, i32* %localIdx
  %call4 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call4, i32* %localIdy
  %call5 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call5, i32* %global_szx
  %call6 = call i32 @get_global_size(i32 1)       ; <i32> [#uses=1]
  store i32 %call6, i32* %global_szy
  %call7 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call7, i32* %local_szx
  %call8 = call i32 @get_local_size(i32 1)        ; <i32> [#uses=1]
  store i32 %call8, i32* %local_szy
  %call9 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call9, i32* %groupIdx
  %call10 = call i32 @get_group_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call10, i32* %groupIdy
  %tmp = load i32* %width.addr                    ; <i32> [#uses=1]
  %tmp11 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp11                    ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp11        ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %count_x
  %tmp13 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp14 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp15 = icmp eq i32 0, %tmp14                  ; <i1> [#uses=1]
  %sel16 = select i1 %cmp15, i32 1, i32 %tmp14    ; <i32> [#uses=1]
  %div17 = udiv i32 %tmp13, %sel16                ; <i32> [#uses=1]
  store i32 %div17, i32* %count_y
  %tmp19 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp20 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp19, %tmp20                   ; <i32> [#uses=1]
  %tmp21 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp22 = icmp eq i32 0, %tmp21                  ; <i1> [#uses=1]
  %sel23 = select i1 %cmp22, i32 1, i32 %tmp21    ; <i32> [#uses=1]
  %div24 = udiv i32 %mul, %sel23                  ; <i32> [#uses=1]
  store i32 %div24, i32* %index_x
  %tmp26 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp27 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %mul28 = mul i32 %tmp26, %tmp27                 ; <i32> [#uses=1]
  %tmp29 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp30 = icmp eq i32 0, %tmp29                  ; <i1> [#uses=1]
  %sel31 = select i1 %cmp30, i32 1, i32 %tmp29    ; <i32> [#uses=1]
  %div32 = udiv i32 %mul28, %sel31                ; <i32> [#uses=1]
  store i32 %div32, i32* %index_y
  %tmp34 = load i32* %index_x                     ; <i32> [#uses=1]
  store i32 %tmp34, i32* %index_x_orig
  store float 9.000000e+000, float* %denominator
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc175, %entry
  %tmp37 = load i32* %i                           ; <i32> [#uses=1]
  %tmp38 = load i32* %count_y                     ; <i32> [#uses=1]
  %cmp39 = icmp ult i32 %tmp37, %tmp38            ; <i1> [#uses=1]
  br i1 %cmp39, label %for.body, label %for.end180

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j
  %tmp43 = load i32* %index_x_orig                ; <i32> [#uses=1]
  store i32 %tmp43, i32* %index_x42
  br label %for.cond44

for.cond44:                                       ; preds = %for.inc, %for.body
  %tmp45 = load i32* %j                           ; <i32> [#uses=1]
  %tmp46 = load i32* %count_x                     ; <i32> [#uses=1]
  %cmp47 = icmp ult i32 %tmp45, %tmp46            ; <i1> [#uses=1]
  br i1 %cmp47, label %for.body48, label %for.end

for.body48:                                       ; preds = %for.cond44
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp51 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp52 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul53 = mul i32 %tmp51, %tmp52                 ; <i32> [#uses=1]
  %tmp54 = load i32* %index_x42                   ; <i32> [#uses=1]
  %add = add i32 %mul53, %tmp54                   ; <i32> [#uses=1]
  store i32 %add, i32* %sourceIndex
  %tmp56 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub = sub i32 %tmp56, 1                        ; <i32> [#uses=1]
  %tmp57 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul58 = mul i32 %sub, %tmp57                   ; <i32> [#uses=1]
  %tmp59 = load i32* %index_x42                   ; <i32> [#uses=1]
  %add60 = add i32 %mul58, %tmp59                 ; <i32> [#uses=1]
  store i32 %add60, i32* %tmpIndex1
  %tmp62 = load i32* %index_y                     ; <i32> [#uses=1]
  %add63 = add i32 %tmp62, 1                      ; <i32> [#uses=1]
  %tmp64 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul65 = mul i32 %add63, %tmp64                 ; <i32> [#uses=1]
  %tmp66 = load i32* %index_x42                   ; <i32> [#uses=1]
  %add67 = add i32 %mul65, %tmp66                 ; <i32> [#uses=1]
  store i32 %add67, i32* %tmpIndex2
  %tmp68 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp69 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp69, i32 %tmp68 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp70 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp70, <4 x float>* %colorAccumulator
  %tmp71 = load i32* %index_x42                   ; <i32> [#uses=1]
  %cmp72 = icmp ugt i32 %tmp71, 0                 ; <i1> [#uses=1]
  br i1 %cmp72, label %if.then, label %if.end

if.then:                                          ; preds = %for.body48
  %tmp73 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %sub74 = sub i32 %tmp73, 1                      ; <i32> [#uses=1]
  %tmp75 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx76 = getelementptr inbounds <4 x float> addrspace(1)* %tmp75, i32 %sub74 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp77 = load <4 x float> addrspace(1)* %arrayidx76 ; <<4 x float>> [#uses=1]
  %tmp78 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add79 = fadd <4 x float> %tmp78, %tmp77        ; <<4 x float>> [#uses=1]
  store <4 x float> %add79, <4 x float>* %colorAccumulator
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body48
  %tmp80 = load i32* %index_x42                   ; <i32> [#uses=1]
  %tmp81 = load i32* %width.addr                  ; <i32> [#uses=1]
  %sub82 = sub i32 %tmp81, 1                      ; <i32> [#uses=1]
  %cmp83 = icmp ult i32 %tmp80, %sub82            ; <i1> [#uses=1]
  br i1 %cmp83, label %if.then84, label %if.end92

if.then84:                                        ; preds = %if.end
  %tmp85 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %add86 = add i32 %tmp85, 1                      ; <i32> [#uses=1]
  %tmp87 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds <4 x float> addrspace(1)* %tmp87, i32 %add86 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp89 = load <4 x float> addrspace(1)* %arrayidx88 ; <<4 x float>> [#uses=1]
  %tmp90 = load <4 x float>* %colorAccumulator    ; <<4 x float>> [#uses=1]
  %add91 = fadd <4 x float> %tmp90, %tmp89        ; <<4 x float>> [#uses=1]
  store <4 x float> %add91, <4 x float>* %colorAccumulator
  br label %if.end92

if.end92:                                         ; preds = %if.then84, %if.end
  %tmp93 = load i32* %index_y                     ; <i32> [#uses=1]
  %cmp94 = icmp ugt i32 %tmp93, 0                 ; <i1> [#uses=1]
  br i1 %cmp94, label %if.then95, label %if.end126

if.then95:                                        ; preds = %if.end92
  %tmp96 = load i32* %tmpIndex1                   ; <i32> [#uses=1]
  %tmp97 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx98 = getelementptr inbounds <4 x float> addrspace(1)* %tmp97, i32 %tmp96 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp99 = load <4 x float> addrspace(1)* %arrayidx98 ; <<4 x float>> [#uses=1]
  %tmp100 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add101 = fadd <4 x float> %tmp100, %tmp99      ; <<4 x float>> [#uses=1]
  store <4 x float> %add101, <4 x float>* %colorAccumulator
  %tmp102 = load i32* %index_x42                  ; <i32> [#uses=1]
  %cmp103 = icmp ugt i32 %tmp102, 0               ; <i1> [#uses=1]
  br i1 %cmp103, label %if.then104, label %if.end112

if.then104:                                       ; preds = %if.then95
  %tmp105 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %sub106 = sub i32 %tmp105, 1                    ; <i32> [#uses=1]
  %tmp107 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds <4 x float> addrspace(1)* %tmp107, i32 %sub106 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp109 = load <4 x float> addrspace(1)* %arrayidx108 ; <<4 x float>> [#uses=1]
  %tmp110 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add111 = fadd <4 x float> %tmp110, %tmp109     ; <<4 x float>> [#uses=1]
  store <4 x float> %add111, <4 x float>* %colorAccumulator
  br label %if.end112

if.end112:                                        ; preds = %if.then104, %if.then95
  %tmp113 = load i32* %index_x42                  ; <i32> [#uses=1]
  %tmp114 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub115 = sub i32 %tmp114, 1                    ; <i32> [#uses=1]
  %cmp116 = icmp ult i32 %tmp113, %sub115         ; <i1> [#uses=1]
  br i1 %cmp116, label %if.then117, label %if.end125

if.then117:                                       ; preds = %if.end112
  %tmp118 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add119 = add i32 %tmp118, 1                    ; <i32> [#uses=1]
  %tmp120 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx121 = getelementptr inbounds <4 x float> addrspace(1)* %tmp120, i32 %add119 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp122 = load <4 x float> addrspace(1)* %arrayidx121 ; <<4 x float>> [#uses=1]
  %tmp123 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add124 = fadd <4 x float> %tmp123, %tmp122     ; <<4 x float>> [#uses=1]
  store <4 x float> %add124, <4 x float>* %colorAccumulator
  br label %if.end125

if.end125:                                        ; preds = %if.then117, %if.end112
  br label %if.end126

if.end126:                                        ; preds = %if.end125, %if.end92
  %tmp127 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp128 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub129 = sub i32 %tmp128, 1                    ; <i32> [#uses=1]
  %cmp130 = icmp ult i32 %tmp127, %sub129         ; <i1> [#uses=1]
  br i1 %cmp130, label %if.then131, label %if.end162

if.then131:                                       ; preds = %if.end126
  %tmp132 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp133 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds <4 x float> addrspace(1)* %tmp133, i32 %tmp132 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp135 = load <4 x float> addrspace(1)* %arrayidx134 ; <<4 x float>> [#uses=1]
  %tmp136 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add137 = fadd <4 x float> %tmp136, %tmp135     ; <<4 x float>> [#uses=1]
  store <4 x float> %add137, <4 x float>* %colorAccumulator
  %tmp138 = load i32* %index_x42                  ; <i32> [#uses=1]
  %cmp139 = icmp ugt i32 %tmp138, 0               ; <i1> [#uses=1]
  br i1 %cmp139, label %if.then140, label %if.end148

if.then140:                                       ; preds = %if.then131
  %tmp141 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %sub142 = sub i32 %tmp141, 1                    ; <i32> [#uses=1]
  %tmp143 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx144 = getelementptr inbounds <4 x float> addrspace(1)* %tmp143, i32 %sub142 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp145 = load <4 x float> addrspace(1)* %arrayidx144 ; <<4 x float>> [#uses=1]
  %tmp146 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add147 = fadd <4 x float> %tmp146, %tmp145     ; <<4 x float>> [#uses=1]
  store <4 x float> %add147, <4 x float>* %colorAccumulator
  br label %if.end148

if.end148:                                        ; preds = %if.then140, %if.then131
  %tmp149 = load i32* %index_x42                  ; <i32> [#uses=1]
  %tmp150 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub151 = sub i32 %tmp150, 1                    ; <i32> [#uses=1]
  %cmp152 = icmp ult i32 %tmp149, %sub151         ; <i1> [#uses=1]
  br i1 %cmp152, label %if.then153, label %if.end161

if.then153:                                       ; preds = %if.end148
  %tmp154 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add155 = add i32 %tmp154, 1                    ; <i32> [#uses=1]
  %tmp156 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx157 = getelementptr inbounds <4 x float> addrspace(1)* %tmp156, i32 %add155 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp158 = load <4 x float> addrspace(1)* %arrayidx157 ; <<4 x float>> [#uses=1]
  %tmp159 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add160 = fadd <4 x float> %tmp159, %tmp158     ; <<4 x float>> [#uses=1]
  store <4 x float> %add160, <4 x float>* %colorAccumulator
  br label %if.end161

if.end161:                                        ; preds = %if.then153, %if.end148
  br label %if.end162

if.end162:                                        ; preds = %if.end161, %if.end126
  %tmp163 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp164 = load float* %denominator              ; <float> [#uses=1]
  %tmp165 = insertelement <4 x float> undef, float %tmp164, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp165, <4 x float> %tmp165, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp166 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel167 = select <4 x i1> %cmp166, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div168 = fdiv <4 x float> %tmp163, %splat      ; <<4 x float>> [#uses=1]
  %tmp169 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp170 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx171 = getelementptr inbounds <4 x float> addrspace(1)* %tmp170, i32 %tmp169 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div168, <4 x float> addrspace(1)* %arrayidx171
  br label %for.inc

for.inc:                                          ; preds = %if.end162
  %tmp172 = load i32* %j                          ; <i32> [#uses=1]
  %inc = add i32 %tmp172, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  %tmp173 = load i32* %index_x42                  ; <i32> [#uses=1]
  %inc174 = add i32 %tmp173, 1                    ; <i32> [#uses=1]
  store i32 %inc174, i32* %index_x42
  br label %for.cond44

for.end:                                          ; preds = %for.cond44
  br label %for.inc175

for.inc175:                                       ; preds = %for.end
  %tmp176 = load i32* %i                          ; <i32> [#uses=1]
  %inc177 = add i32 %tmp176, 1                    ; <i32> [#uses=1]
  store i32 %inc177, i32* %i
  %tmp178 = load i32* %index_y                    ; <i32> [#uses=1]
  %inc179 = add i32 %tmp178, 1                    ; <i32> [#uses=1]
  store i32 %inc179, i32* %index_y
  br label %for.cond

for.end180:                                       ; preds = %for.cond
  ret void
}

; CHECK: ret
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %outCrd) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outCrd.addr = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=2]
  %samplerNearest = alloca i32, align 4           ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x i32> %outCrd, <2 x i32>* %outCrd.addr
  store i32 1, i32* %samplerNearest
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load <2 x i32>* %outCrd.addr            ; <<2 x i32>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t* %tmp, i32 1, <2 x i32> %tmp1) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp2 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp2, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t*, i32, <2 x i32>)

; CHECK: ret
define void @wlSimpleBoxBlur_image2d(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=11]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=15]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %denominator = alloca float, align 4            ; <float*> [#uses=2]
  %colorAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=19]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=5]
  %curLeftCrd = alloca <2 x i32>, align 8         ; <<2 x i32>*> [#uses=5]
  %curRightCrd = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=5]
  %upCrd = alloca <2 x i32>, align 8              ; <<2 x i32>*> [#uses=5]
  %upLeftCrd = alloca <2 x i32>, align 8          ; <<2 x i32>*> [#uses=5]
  %upRightCrd = alloca <2 x i32>, align 8         ; <<2 x i32>*> [#uses=5]
  %lowCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=5]
  %lowLeftCrd = alloca <2 x i32>, align 8         ; <<2 x i32>*> [#uses=5]
  %lowRightCrd = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=13]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
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
  store float 9.000000e+000, float* %denominator
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  br label %for.cond

for.cond:                                         ; preds = %for.inc152, %entry
  %tmp27 = load i32* %row                         ; <i32> [#uses=1]
  %tmp28 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp27, %tmp28              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end155

for.body:                                         ; preds = %for.cond
  %tmp29 = load i32* %row                         ; <i32> [#uses=1]
  %tmp30 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp31 = insertelement <2 x i32> %tmp30, i32 %tmp29, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp31, <2 x i32>* %curCrd
  %tmp32 = load i32* %row                         ; <i32> [#uses=1]
  %tmp33 = load <2 x i32>* %curLeftCrd            ; <<2 x i32>> [#uses=1]
  %tmp34 = insertelement <2 x i32> %tmp33, i32 %tmp32, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp34, <2 x i32>* %curLeftCrd
  %tmp35 = load i32* %row                         ; <i32> [#uses=1]
  %tmp36 = load <2 x i32>* %curRightCrd           ; <<2 x i32>> [#uses=1]
  %tmp37 = insertelement <2 x i32> %tmp36, i32 %tmp35, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp37, <2 x i32>* %curRightCrd
  %tmp38 = load i32* %row                         ; <i32> [#uses=1]
  %sub = sub i32 %tmp38, 1                        ; <i32> [#uses=1]
  %tmp39 = load <2 x i32>* %upCrd                 ; <<2 x i32>> [#uses=1]
  %tmp40 = insertelement <2 x i32> %tmp39, i32 %sub, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp40, <2 x i32>* %upCrd
  %tmp41 = load i32* %row                         ; <i32> [#uses=1]
  %sub42 = sub i32 %tmp41, 1                      ; <i32> [#uses=1]
  %tmp43 = load <2 x i32>* %upLeftCrd             ; <<2 x i32>> [#uses=1]
  %tmp44 = insertelement <2 x i32> %tmp43, i32 %sub42, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp44, <2 x i32>* %upLeftCrd
  %tmp45 = load i32* %row                         ; <i32> [#uses=1]
  %sub46 = sub i32 %tmp45, 1                      ; <i32> [#uses=1]
  %tmp47 = load <2 x i32>* %upRightCrd            ; <<2 x i32>> [#uses=1]
  %tmp48 = insertelement <2 x i32> %tmp47, i32 %sub46, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp48, <2 x i32>* %upRightCrd
  %tmp49 = load i32* %row                         ; <i32> [#uses=1]
  %add50 = add nsw i32 %tmp49, 1                  ; <i32> [#uses=1]
  %tmp51 = load <2 x i32>* %lowCrd                ; <<2 x i32>> [#uses=1]
  %tmp52 = insertelement <2 x i32> %tmp51, i32 %add50, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp52, <2 x i32>* %lowCrd
  %tmp53 = load i32* %row                         ; <i32> [#uses=1]
  %add54 = add nsw i32 %tmp53, 1                  ; <i32> [#uses=1]
  %tmp55 = load <2 x i32>* %lowLeftCrd            ; <<2 x i32>> [#uses=1]
  %tmp56 = insertelement <2 x i32> %tmp55, i32 %add54, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp56, <2 x i32>* %lowLeftCrd
  %tmp57 = load i32* %row                         ; <i32> [#uses=1]
  %add58 = add nsw i32 %tmp57, 1                  ; <i32> [#uses=1]
  %tmp59 = load <2 x i32>* %lowRightCrd           ; <<2 x i32>> [#uses=1]
  %tmp60 = insertelement <2 x i32> %tmp59, i32 %add58, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp60, <2 x i32>* %lowRightCrd
  store i32 0, i32* %col
  br label %for.cond62

for.cond62:                                       ; preds = %for.inc, %for.body
  %tmp63 = load i32* %col                         ; <i32> [#uses=1]
  %tmp64 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp65 = extractelement <2 x i32> %tmp64, i32 0 ; <i32> [#uses=1]
  %cmp66 = icmp slt i32 %tmp63, %tmp65            ; <i1> [#uses=1]
  br i1 %cmp66, label %for.body67, label %for.end

for.body67:                                       ; preds = %for.cond62
  %tmp68 = load i32* %col                         ; <i32> [#uses=1]
  %tmp69 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp70 = insertelement <2 x i32> %tmp69, i32 %tmp68, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp70, <2 x i32>* %curCrd
  %tmp71 = load i32* %col                         ; <i32> [#uses=1]
  %sub72 = sub i32 %tmp71, 1                      ; <i32> [#uses=1]
  %tmp73 = load <2 x i32>* %curLeftCrd            ; <<2 x i32>> [#uses=1]
  %tmp74 = insertelement <2 x i32> %tmp73, i32 %sub72, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp74, <2 x i32>* %curLeftCrd
  %tmp75 = load i32* %col                         ; <i32> [#uses=1]
  %add76 = add nsw i32 %tmp75, 1                  ; <i32> [#uses=1]
  %tmp77 = load <2 x i32>* %curRightCrd           ; <<2 x i32>> [#uses=1]
  %tmp78 = insertelement <2 x i32> %tmp77, i32 %add76, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp78, <2 x i32>* %curRightCrd
  %tmp79 = load i32* %col                         ; <i32> [#uses=1]
  %tmp80 = load <2 x i32>* %upCrd                 ; <<2 x i32>> [#uses=1]
  %tmp81 = insertelement <2 x i32> %tmp80, i32 %tmp79, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp81, <2 x i32>* %upCrd
  %tmp82 = load i32* %col                         ; <i32> [#uses=1]
  %sub83 = sub i32 %tmp82, 1                      ; <i32> [#uses=1]
  %tmp84 = load <2 x i32>* %upLeftCrd             ; <<2 x i32>> [#uses=1]
  %tmp85 = insertelement <2 x i32> %tmp84, i32 %sub83, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp85, <2 x i32>* %upLeftCrd
  %tmp86 = load i32* %col                         ; <i32> [#uses=1]
  %add87 = add nsw i32 %tmp86, 1                  ; <i32> [#uses=1]
  %tmp88 = load <2 x i32>* %upRightCrd            ; <<2 x i32>> [#uses=1]
  %tmp89 = insertelement <2 x i32> %tmp88, i32 %add87, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp89, <2 x i32>* %upRightCrd
  %tmp90 = load i32* %col                         ; <i32> [#uses=1]
  %tmp91 = load <2 x i32>* %lowCrd                ; <<2 x i32>> [#uses=1]
  %tmp92 = insertelement <2 x i32> %tmp91, i32 %tmp90, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp92, <2 x i32>* %lowCrd
  %tmp93 = load i32* %col                         ; <i32> [#uses=1]
  %sub94 = sub i32 %tmp93, 1                      ; <i32> [#uses=1]
  %tmp95 = load <2 x i32>* %lowLeftCrd            ; <<2 x i32>> [#uses=1]
  %tmp96 = insertelement <2 x i32> %tmp95, i32 %sub94, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp96, <2 x i32>* %lowLeftCrd
  %tmp97 = load i32* %col                         ; <i32> [#uses=1]
  %add98 = add nsw i32 %tmp97, 1                  ; <i32> [#uses=1]
  %tmp99 = load <2 x i32>* %lowRightCrd           ; <<2 x i32>> [#uses=1]
  %tmp100 = insertelement <2 x i32> %tmp99, i32 %add98, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp100, <2 x i32>* %lowRightCrd
  %tmp101 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp102 = load <2 x i32>* %curCrd               ; <<2 x i32>> [#uses=1]
  %call103 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp101, <2 x i32> %tmp102) ; <<4 x float>> [#uses=1]
  store <4 x float> %call103, <4 x float>* %colorAccumulator
  %tmp104 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp105 = load <2 x i32>* %curLeftCrd           ; <<2 x i32>> [#uses=1]
  %call106 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp104, <2 x i32> %tmp105) ; <<4 x float>> [#uses=1]
  %tmp107 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add108 = fadd <4 x float> %tmp107, %call106    ; <<4 x float>> [#uses=1]
  store <4 x float> %add108, <4 x float>* %colorAccumulator
  %tmp109 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp110 = load <2 x i32>* %curRightCrd          ; <<2 x i32>> [#uses=1]
  %call111 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp109, <2 x i32> %tmp110) ; <<4 x float>> [#uses=1]
  %tmp112 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add113 = fadd <4 x float> %tmp112, %call111    ; <<4 x float>> [#uses=1]
  store <4 x float> %add113, <4 x float>* %colorAccumulator
  %tmp114 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp115 = load <2 x i32>* %upCrd                ; <<2 x i32>> [#uses=1]
  %call116 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp114, <2 x i32> %tmp115) ; <<4 x float>> [#uses=1]
  %tmp117 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add118 = fadd <4 x float> %tmp117, %call116    ; <<4 x float>> [#uses=1]
  store <4 x float> %add118, <4 x float>* %colorAccumulator
  %tmp119 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp120 = load <2 x i32>* %upLeftCrd            ; <<2 x i32>> [#uses=1]
  %call121 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp119, <2 x i32> %tmp120) ; <<4 x float>> [#uses=1]
  %tmp122 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add123 = fadd <4 x float> %tmp122, %call121    ; <<4 x float>> [#uses=1]
  store <4 x float> %add123, <4 x float>* %colorAccumulator
  %tmp124 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp125 = load <2 x i32>* %upRightCrd           ; <<2 x i32>> [#uses=1]
  %call126 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp124, <2 x i32> %tmp125) ; <<4 x float>> [#uses=1]
  %tmp127 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add128 = fadd <4 x float> %tmp127, %call126    ; <<4 x float>> [#uses=1]
  store <4 x float> %add128, <4 x float>* %colorAccumulator
  %tmp129 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp130 = load <2 x i32>* %lowCrd               ; <<2 x i32>> [#uses=1]
  %call131 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp129, <2 x i32> %tmp130) ; <<4 x float>> [#uses=1]
  %tmp132 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add133 = fadd <4 x float> %tmp132, %call131    ; <<4 x float>> [#uses=1]
  store <4 x float> %add133, <4 x float>* %colorAccumulator
  %tmp134 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp135 = load <2 x i32>* %lowLeftCrd           ; <<2 x i32>> [#uses=1]
  %call136 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp134, <2 x i32> %tmp135) ; <<4 x float>> [#uses=1]
  %tmp137 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add138 = fadd <4 x float> %tmp137, %call136    ; <<4 x float>> [#uses=1]
  store <4 x float> %add138, <4 x float>* %colorAccumulator
  %tmp139 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp140 = load <2 x i32>* %lowRightCrd          ; <<2 x i32>> [#uses=1]
  %call141 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp139, <2 x i32> %tmp140) ; <<4 x float>> [#uses=1]
  %tmp142 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add143 = fadd <4 x float> %tmp142, %call141    ; <<4 x float>> [#uses=1]
  store <4 x float> %add143, <4 x float>* %colorAccumulator
  %tmp144 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp145 = load float* %denominator              ; <float> [#uses=1]
  %tmp146 = insertelement <4 x float> undef, float %tmp145, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp146, <4 x float> %tmp146, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp147 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp147, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp144, %splat         ; <<4 x float>> [#uses=1]
  %tmp148 = load i32* %index                      ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp148, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp149 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp149, i32 %tmp148 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body67
  %tmp150 = load i32* %col                        ; <i32> [#uses=1]
  %inc151 = add nsw i32 %tmp150, 1                ; <i32> [#uses=1]
  store i32 %inc151, i32* %col
  br label %for.cond62

for.end:                                          ; preds = %for.cond62
  br label %for.inc152

for.inc152:                                       ; preds = %for.end
  %tmp153 = load i32* %row                        ; <i32> [#uses=1]
  %inc154 = add nsw i32 %tmp153, 1                ; <i32> [#uses=1]
  store i32 %inc154, i32* %row
  br label %for.cond

for.end155:                                       ; preds = %for.cond
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare i32 @_Z3minii(i32, i32)

; CHECK: ret
define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=62]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=13]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=64]
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
  %count_x = alloca i32, align 4                  ; <i32*> [#uses=9]
  %index_x = alloca i32, align 4                  ; <i32*> [#uses=14]
  %index_y = alloca i32, align 4                  ; <i32*> [#uses=14]
  %denominator = alloca float, align 4            ; <float*> [#uses=13]
  %colorAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=125]
  %firstBlockAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=23]
  %leftIndex = alloca i32, align 4                ; <i32*> [#uses=6]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=14]
  %leftColumnIndex = alloca i32, align 4          ; <i32*> [#uses=8]
  %rightColumnIndex = alloca i32, align 4         ; <i32*> [#uses=8]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=24]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=24]
  %topRowIndex = alloca i32, align 4              ; <i32*> [#uses=8]
  %bottomRowIndex = alloca i32, align 4           ; <i32*> [#uses=4]
  %row = alloca i32, align 4                      ; <i32*> [#uses=5]
  %column = alloca i32, align 4                   ; <i32*> [#uses=4]
  %column299 = alloca i32, align 4                ; <i32*> [#uses=4]
  %column416 = alloca i32, align 4                ; <i32*> [#uses=11]
  %row540 = alloca i32, align 4                   ; <i32*> [#uses=11]
  %column683 = alloca i32, align 4                ; <i32*> [#uses=11]
  %row786 = alloca i32, align 4                   ; <i32*> [#uses=11]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
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
  %tmp27 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp28 = load i32* %count_y                     ; <i32> [#uses=1]
  %add = add i32 %tmp27, %tmp28                   ; <i32> [#uses=1]
  %add29 = add i32 %add, 1                        ; <i32> [#uses=1]
  %tmp30 = load i32* %height.addr                 ; <i32> [#uses=1]
  %cmp31 = icmp uge i32 %add29, %tmp30            ; <i1> [#uses=1]
  br i1 %cmp31, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i8 1, i8* %bottomEdge
  %tmp32 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp33 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub = sub i32 %tmp32, %tmp33                   ; <i32> [#uses=1]
  %sub34 = sub i32 %sub, 1                        ; <i32> [#uses=1]
  store i32 %sub34, i32* %count_y
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp35 = load i32* %index_x                     ; <i32> [#uses=1]
  %tmp36 = load i32* %count_x                     ; <i32> [#uses=1]
  %add37 = add i32 %tmp35, %tmp36                 ; <i32> [#uses=1]
  %add38 = add i32 %add37, 1                      ; <i32> [#uses=1]
  %tmp39 = load i32* %width.addr                  ; <i32> [#uses=1]
  %cmp40 = icmp uge i32 %add38, %tmp39            ; <i1> [#uses=1]
  br i1 %cmp40, label %if.then41, label %if.end46

if.then41:                                        ; preds = %if.end
  store i8 1, i8* %rightEdge
  %tmp42 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp43 = load i32* %index_x                     ; <i32> [#uses=1]
  %sub44 = sub i32 %tmp42, %tmp43                 ; <i32> [#uses=1]
  %sub45 = sub i32 %sub44, 1                      ; <i32> [#uses=1]
  store i32 %sub45, i32* %count_x
  br label %if.end46

if.end46:                                         ; preds = %if.then41, %if.end
  %tmp47 = load i32* %index_y                     ; <i32> [#uses=1]
  %cmp48 = icmp ult i32 %tmp47, 1                 ; <i1> [#uses=1]
  br i1 %cmp48, label %if.then49, label %if.end52

if.then49:                                        ; preds = %if.end46
  store i8 1, i8* %topEdge
  store i32 1, i32* %index_y
  %tmp50 = load i32* %count_y                     ; <i32> [#uses=1]
  %sub51 = sub i32 %tmp50, 1                      ; <i32> [#uses=1]
  store i32 %sub51, i32* %count_y
  br label %if.end52

if.end52:                                         ; preds = %if.then49, %if.end46
  %tmp53 = load i32* %index_x                     ; <i32> [#uses=1]
  %cmp54 = icmp ult i32 %tmp53, 1                 ; <i1> [#uses=1]
  br i1 %cmp54, label %if.then55, label %if.end58

if.then55:                                        ; preds = %if.end52
  store i8 1, i8* %leftEdge
  store i32 1, i32* %index_x
  %tmp56 = load i32* %count_x                     ; <i32> [#uses=1]
  %sub57 = sub i32 %tmp56, 1                      ; <i32> [#uses=1]
  store i32 %sub57, i32* %count_x
  br label %if.end58

if.end58:                                         ; preds = %if.then55, %if.end52
  store float 9.000000e+000, float* %denominator
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  store <4 x float> zeroinitializer, <4 x float>* %firstBlockAccumulator
  %tmp63 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub64 = sub i32 %tmp63, 1                      ; <i32> [#uses=1]
  %tmp65 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul66 = mul i32 %sub64, %tmp65                 ; <i32> [#uses=1]
  %tmp67 = load i32* %index_x                     ; <i32> [#uses=1]
  %add68 = add i32 %mul66, %tmp67                 ; <i32> [#uses=1]
  %sub69 = sub i32 %add68, 1                      ; <i32> [#uses=1]
  store i32 %sub69, i32* %leftIndex
  store i32 0, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end58
  %tmp71 = load i32* %y                           ; <i32> [#uses=1]
  %cmp72 = icmp ult i32 %tmp71, 3                 ; <i1> [#uses=1]
  br i1 %cmp72, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp73 = load i32* %leftIndex                   ; <i32> [#uses=1]
  %tmp74 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp74, i32 %tmp73 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp75 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp76 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add77 = fadd <4 x float> %tmp76, %tmp75        ; <<4 x float>> [#uses=1]
  store <4 x float> %add77, <4 x float>* %firstBlockAccumulator
  %tmp78 = load i32* %leftIndex                   ; <i32> [#uses=1]
  %add79 = add i32 %tmp78, 1                      ; <i32> [#uses=1]
  %tmp80 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx81 = getelementptr inbounds <4 x float> addrspace(1)* %tmp80, i32 %add79 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp82 = load <4 x float> addrspace(1)* %arrayidx81 ; <<4 x float>> [#uses=1]
  %tmp83 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add84 = fadd <4 x float> %tmp83, %tmp82        ; <<4 x float>> [#uses=1]
  store <4 x float> %add84, <4 x float>* %firstBlockAccumulator
  %tmp85 = load i32* %leftIndex                   ; <i32> [#uses=1]
  %add86 = add i32 %tmp85, 2                      ; <i32> [#uses=1]
  %tmp87 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds <4 x float> addrspace(1)* %tmp87, i32 %add86 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp89 = load <4 x float> addrspace(1)* %arrayidx88 ; <<4 x float>> [#uses=1]
  %tmp90 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add91 = fadd <4 x float> %tmp90, %tmp89        ; <<4 x float>> [#uses=1]
  store <4 x float> %add91, <4 x float>* %firstBlockAccumulator
  %tmp92 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp93 = load i32* %leftIndex                   ; <i32> [#uses=1]
  %add94 = add i32 %tmp93, %tmp92                 ; <i32> [#uses=1]
  store i32 %add94, i32* %leftIndex
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp95 = load i32* %y                           ; <i32> [#uses=1]
  %inc = add i32 %tmp95, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %y
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp97 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp98 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul99 = mul i32 %tmp97, %tmp98                 ; <i32> [#uses=1]
  %tmp100 = load i32* %index_x                    ; <i32> [#uses=1]
  %add101 = add i32 %mul99, %tmp100               ; <i32> [#uses=1]
  store i32 %add101, i32* %sourceIndex
  %tmp102 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %tmp103 = load float* %denominator              ; <float> [#uses=1]
  %tmp104 = insertelement <4 x float> undef, float %tmp103, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp104, <4 x float> %tmp104, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp105 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel106 = select <4 x i1> %cmp105, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div107 = fdiv <4 x float> %tmp102, %splat      ; <<4 x float>> [#uses=1]
  %tmp108 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp109 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx110 = getelementptr inbounds <4 x float> addrspace(1)* %tmp109, i32 %tmp108 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div107, <4 x float> addrspace(1)* %arrayidx110
  %tmp111 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp111, <4 x float>* %colorAccumulator
  %tmp112 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc113 = add i32 %tmp112, 1                    ; <i32> [#uses=1]
  store i32 %inc113, i32* %sourceIndex
  %tmp119 = load i32* %index_y                    ; <i32> [#uses=1]
  %sub120 = sub i32 %tmp119, 1                    ; <i32> [#uses=1]
  %tmp121 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul122 = mul i32 %sub120, %tmp121              ; <i32> [#uses=1]
  %tmp123 = load i32* %index_x                    ; <i32> [#uses=1]
  %add124 = add i32 %mul122, %tmp123              ; <i32> [#uses=1]
  %sub125 = sub i32 %add124, 1                    ; <i32> [#uses=1]
  store i32 %sub125, i32* %topRowIndex
  %tmp127 = load i32* %index_y                    ; <i32> [#uses=1]
  %add128 = add i32 %tmp127, 1                    ; <i32> [#uses=1]
  %tmp129 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul130 = mul i32 %add128, %tmp129              ; <i32> [#uses=1]
  %tmp131 = load i32* %index_x                    ; <i32> [#uses=1]
  %add132 = add i32 %mul130, %tmp131              ; <i32> [#uses=1]
  %sub133 = sub i32 %add132, 1                    ; <i32> [#uses=1]
  store i32 %sub133, i32* %bottomRowIndex
  store i32 0, i32* %row
  br label %for.cond135

for.cond135:                                      ; preds = %for.end219, %for.end
  %tmp136 = load i32* %row                        ; <i32> [#uses=1]
  %tmp137 = load i32* %count_y                    ; <i32> [#uses=1]
  %sub138 = sub i32 %tmp137, 1                    ; <i32> [#uses=1]
  %cmp139 = icmp ult i32 %tmp136, %sub138         ; <i1> [#uses=1]
  br i1 %cmp139, label %for.body140, label %for.end294

for.body140:                                      ; preds = %for.cond135
  %tmp141 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp141, i32* %leftColumnIndex
  %tmp142 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %add143 = add i32 %tmp142, 2                    ; <i32> [#uses=1]
  store i32 %add143, i32* %rightColumnIndex
  store i32 1, i32* %column
  br label %for.cond145

for.cond145:                                      ; preds = %for.inc216, %for.body140
  %tmp146 = load i32* %column                     ; <i32> [#uses=1]
  %tmp147 = load i32* %count_x                    ; <i32> [#uses=1]
  %cmp148 = icmp ult i32 %tmp146, %tmp147         ; <i1> [#uses=1]
  br i1 %cmp148, label %for.body149, label %for.end219

for.body149:                                      ; preds = %for.cond145
  %tmp150 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  %inc151 = add i32 %tmp150, 1                    ; <i32> [#uses=1]
  store i32 %inc151, i32* %rightColumnIndex
  %tmp152 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  store i32 %tmp152, i32* %tmpIndex1
  %tmp153 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  store i32 %tmp153, i32* %tmpIndex2
  %tmp154 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp155 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx156 = getelementptr inbounds <4 x float> addrspace(1)* %tmp155, i32 %tmp154 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp157 = load <4 x float> addrspace(1)* %arrayidx156 ; <<4 x float>> [#uses=1]
  %tmp158 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub159 = fsub <4 x float> %tmp158, %tmp157     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub159, <4 x float>* %colorAccumulator
  %tmp160 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp161 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add162 = add i32 %tmp161, %tmp160              ; <i32> [#uses=1]
  store i32 %add162, i32* %tmpIndex1
  %tmp163 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp164 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx165 = getelementptr inbounds <4 x float> addrspace(1)* %tmp164, i32 %tmp163 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp166 = load <4 x float> addrspace(1)* %arrayidx165 ; <<4 x float>> [#uses=1]
  %tmp167 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add168 = fadd <4 x float> %tmp167, %tmp166     ; <<4 x float>> [#uses=1]
  store <4 x float> %add168, <4 x float>* %colorAccumulator
  %tmp169 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp170 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add171 = add i32 %tmp170, %tmp169              ; <i32> [#uses=1]
  store i32 %add171, i32* %tmpIndex2
  %tmp172 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp173 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx174 = getelementptr inbounds <4 x float> addrspace(1)* %tmp173, i32 %tmp172 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp175 = load <4 x float> addrspace(1)* %arrayidx174 ; <<4 x float>> [#uses=1]
  %tmp176 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub177 = fsub <4 x float> %tmp176, %tmp175     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub177, <4 x float>* %colorAccumulator
  %tmp178 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp179 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add180 = add i32 %tmp179, %tmp178              ; <i32> [#uses=1]
  store i32 %add180, i32* %tmpIndex1
  %tmp181 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp182 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx183 = getelementptr inbounds <4 x float> addrspace(1)* %tmp182, i32 %tmp181 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp184 = load <4 x float> addrspace(1)* %arrayidx183 ; <<4 x float>> [#uses=1]
  %tmp185 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add186 = fadd <4 x float> %tmp185, %tmp184     ; <<4 x float>> [#uses=1]
  store <4 x float> %add186, <4 x float>* %colorAccumulator
  %tmp187 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp188 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add189 = add i32 %tmp188, %tmp187              ; <i32> [#uses=1]
  store i32 %add189, i32* %tmpIndex2
  %tmp190 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp191 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx192 = getelementptr inbounds <4 x float> addrspace(1)* %tmp191, i32 %tmp190 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp193 = load <4 x float> addrspace(1)* %arrayidx192 ; <<4 x float>> [#uses=1]
  %tmp194 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub195 = fsub <4 x float> %tmp194, %tmp193     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub195, <4 x float>* %colorAccumulator
  %tmp196 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp197 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx198 = getelementptr inbounds <4 x float> addrspace(1)* %tmp197, i32 %tmp196 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp199 = load <4 x float> addrspace(1)* %arrayidx198 ; <<4 x float>> [#uses=1]
  %tmp200 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add201 = fadd <4 x float> %tmp200, %tmp199     ; <<4 x float>> [#uses=1]
  store <4 x float> %add201, <4 x float>* %colorAccumulator
  %tmp202 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp203 = load float* %denominator              ; <float> [#uses=1]
  %tmp204 = insertelement <4 x float> undef, float %tmp203, i32 0 ; <<4 x float>> [#uses=2]
  %splat205 = shufflevector <4 x float> %tmp204, <4 x float> %tmp204, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp206 = fcmp oeq <4 x float> zeroinitializer, %splat205 ; <<4 x i1>> [#uses=1]
  %sel207 = select <4 x i1> %cmp206, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat205 ; <<4 x float>> [#uses=0]
  %div208 = fdiv <4 x float> %tmp202, %splat205   ; <<4 x float>> [#uses=1]
  %tmp209 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp210 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx211 = getelementptr inbounds <4 x float> addrspace(1)* %tmp210, i32 %tmp209 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div208, <4 x float> addrspace(1)* %arrayidx211
  %tmp212 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc213 = add i32 %tmp212, 1                    ; <i32> [#uses=1]
  store i32 %inc213, i32* %sourceIndex
  %tmp214 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  %inc215 = add i32 %tmp214, 1                    ; <i32> [#uses=1]
  store i32 %inc215, i32* %leftColumnIndex
  br label %for.inc216

for.inc216:                                       ; preds = %for.body149
  %tmp217 = load i32* %column                     ; <i32> [#uses=1]
  %inc218 = add i32 %tmp217, 1                    ; <i32> [#uses=1]
  store i32 %inc218, i32* %column
  br label %for.cond145

for.end219:                                       ; preds = %for.cond145
  %tmp220 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp221 = load i32* %bottomRowIndex             ; <i32> [#uses=1]
  %add222 = add i32 %tmp221, %tmp220              ; <i32> [#uses=1]
  store i32 %add222, i32* %bottomRowIndex
  %tmp223 = load i32* %row                        ; <i32> [#uses=1]
  %inc224 = add i32 %tmp223, 1                    ; <i32> [#uses=1]
  store i32 %inc224, i32* %row
  %tmp225 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp225, i32* %tmpIndex1
  %tmp226 = load i32* %bottomRowIndex             ; <i32> [#uses=1]
  store i32 %tmp226, i32* %tmpIndex2
  %tmp227 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp228 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx229 = getelementptr inbounds <4 x float> addrspace(1)* %tmp228, i32 %tmp227 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp230 = load <4 x float> addrspace(1)* %arrayidx229 ; <<4 x float>> [#uses=1]
  %tmp231 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %sub232 = fsub <4 x float> %tmp231, %tmp230     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub232, <4 x float>* %firstBlockAccumulator
  %tmp233 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %inc234 = add i32 %tmp233, 1                    ; <i32> [#uses=1]
  store i32 %inc234, i32* %tmpIndex1
  %tmp235 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp236 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx237 = getelementptr inbounds <4 x float> addrspace(1)* %tmp236, i32 %tmp235 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp238 = load <4 x float> addrspace(1)* %arrayidx237 ; <<4 x float>> [#uses=1]
  %tmp239 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add240 = fadd <4 x float> %tmp239, %tmp238     ; <<4 x float>> [#uses=1]
  store <4 x float> %add240, <4 x float>* %firstBlockAccumulator
  %tmp241 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %inc242 = add i32 %tmp241, 1                    ; <i32> [#uses=1]
  store i32 %inc242, i32* %tmpIndex2
  %tmp243 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp244 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx245 = getelementptr inbounds <4 x float> addrspace(1)* %tmp244, i32 %tmp243 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp246 = load <4 x float> addrspace(1)* %arrayidx245 ; <<4 x float>> [#uses=1]
  %tmp247 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %sub248 = fsub <4 x float> %tmp247, %tmp246     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub248, <4 x float>* %firstBlockAccumulator
  %tmp249 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %inc250 = add i32 %tmp249, 1                    ; <i32> [#uses=1]
  store i32 %inc250, i32* %tmpIndex1
  %tmp251 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp252 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx253 = getelementptr inbounds <4 x float> addrspace(1)* %tmp252, i32 %tmp251 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp254 = load <4 x float> addrspace(1)* %arrayidx253 ; <<4 x float>> [#uses=1]
  %tmp255 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add256 = fadd <4 x float> %tmp255, %tmp254     ; <<4 x float>> [#uses=1]
  store <4 x float> %add256, <4 x float>* %firstBlockAccumulator
  %tmp257 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %inc258 = add i32 %tmp257, 1                    ; <i32> [#uses=1]
  store i32 %inc258, i32* %tmpIndex2
  %tmp259 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp260 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx261 = getelementptr inbounds <4 x float> addrspace(1)* %tmp260, i32 %tmp259 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp262 = load <4 x float> addrspace(1)* %arrayidx261 ; <<4 x float>> [#uses=1]
  %tmp263 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %sub264 = fsub <4 x float> %tmp263, %tmp262     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub264, <4 x float>* %firstBlockAccumulator
  %tmp265 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp266 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx267 = getelementptr inbounds <4 x float> addrspace(1)* %tmp266, i32 %tmp265 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp268 = load <4 x float> addrspace(1)* %arrayidx267 ; <<4 x float>> [#uses=1]
  %tmp269 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add270 = fadd <4 x float> %tmp269, %tmp268     ; <<4 x float>> [#uses=1]
  store <4 x float> %add270, <4 x float>* %firstBlockAccumulator
  %tmp271 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp272 = load i32* %row                        ; <i32> [#uses=1]
  %add273 = add i32 %tmp271, %tmp272              ; <i32> [#uses=1]
  %tmp274 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul275 = mul i32 %add273, %tmp274              ; <i32> [#uses=1]
  %tmp276 = load i32* %index_x                    ; <i32> [#uses=1]
  %add277 = add i32 %mul275, %tmp276              ; <i32> [#uses=1]
  store i32 %add277, i32* %sourceIndex
  %tmp278 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %tmp279 = load float* %denominator              ; <float> [#uses=1]
  %tmp280 = insertelement <4 x float> undef, float %tmp279, i32 0 ; <<4 x float>> [#uses=2]
  %splat281 = shufflevector <4 x float> %tmp280, <4 x float> %tmp280, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp282 = fcmp oeq <4 x float> zeroinitializer, %splat281 ; <<4 x i1>> [#uses=1]
  %sel283 = select <4 x i1> %cmp282, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat281 ; <<4 x float>> [#uses=0]
  %div284 = fdiv <4 x float> %tmp278, %splat281   ; <<4 x float>> [#uses=1]
  %tmp285 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp286 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx287 = getelementptr inbounds <4 x float> addrspace(1)* %tmp286, i32 %tmp285 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div284, <4 x float> addrspace(1)* %arrayidx287
  %tmp288 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp288, <4 x float>* %colorAccumulator
  %tmp289 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc290 = add i32 %tmp289, 1                    ; <i32> [#uses=1]
  store i32 %inc290, i32* %sourceIndex
  %tmp291 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp292 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %add293 = add i32 %tmp292, %tmp291              ; <i32> [#uses=1]
  store i32 %add293, i32* %topRowIndex
  br label %for.cond135

for.end294:                                       ; preds = %for.cond135
  %tmp295 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp295, i32* %leftColumnIndex
  %tmp296 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %add297 = add i32 %tmp296, 2                    ; <i32> [#uses=1]
  store i32 %add297, i32* %rightColumnIndex
  store i32 1, i32* %column299
  br label %for.cond300

for.cond300:                                      ; preds = %for.inc371, %for.end294
  %tmp301 = load i32* %column299                  ; <i32> [#uses=1]
  %tmp302 = load i32* %count_x                    ; <i32> [#uses=1]
  %cmp303 = icmp ult i32 %tmp301, %tmp302         ; <i1> [#uses=1]
  br i1 %cmp303, label %for.body304, label %for.end374

for.body304:                                      ; preds = %for.cond300
  %tmp305 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  %inc306 = add i32 %tmp305, 1                    ; <i32> [#uses=1]
  store i32 %inc306, i32* %rightColumnIndex
  %tmp307 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  store i32 %tmp307, i32* %tmpIndex1
  %tmp308 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  store i32 %tmp308, i32* %tmpIndex2
  %tmp309 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp310 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx311 = getelementptr inbounds <4 x float> addrspace(1)* %tmp310, i32 %tmp309 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp312 = load <4 x float> addrspace(1)* %arrayidx311 ; <<4 x float>> [#uses=1]
  %tmp313 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub314 = fsub <4 x float> %tmp313, %tmp312     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub314, <4 x float>* %colorAccumulator
  %tmp315 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp316 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add317 = add i32 %tmp316, %tmp315              ; <i32> [#uses=1]
  store i32 %add317, i32* %tmpIndex1
  %tmp318 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp319 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx320 = getelementptr inbounds <4 x float> addrspace(1)* %tmp319, i32 %tmp318 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp321 = load <4 x float> addrspace(1)* %arrayidx320 ; <<4 x float>> [#uses=1]
  %tmp322 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add323 = fadd <4 x float> %tmp322, %tmp321     ; <<4 x float>> [#uses=1]
  store <4 x float> %add323, <4 x float>* %colorAccumulator
  %tmp324 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp325 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add326 = add i32 %tmp325, %tmp324              ; <i32> [#uses=1]
  store i32 %add326, i32* %tmpIndex2
  %tmp327 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp328 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx329 = getelementptr inbounds <4 x float> addrspace(1)* %tmp328, i32 %tmp327 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp330 = load <4 x float> addrspace(1)* %arrayidx329 ; <<4 x float>> [#uses=1]
  %tmp331 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub332 = fsub <4 x float> %tmp331, %tmp330     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub332, <4 x float>* %colorAccumulator
  %tmp333 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp334 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add335 = add i32 %tmp334, %tmp333              ; <i32> [#uses=1]
  store i32 %add335, i32* %tmpIndex1
  %tmp336 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp337 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx338 = getelementptr inbounds <4 x float> addrspace(1)* %tmp337, i32 %tmp336 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp339 = load <4 x float> addrspace(1)* %arrayidx338 ; <<4 x float>> [#uses=1]
  %tmp340 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add341 = fadd <4 x float> %tmp340, %tmp339     ; <<4 x float>> [#uses=1]
  store <4 x float> %add341, <4 x float>* %colorAccumulator
  %tmp342 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp343 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add344 = add i32 %tmp343, %tmp342              ; <i32> [#uses=1]
  store i32 %add344, i32* %tmpIndex2
  %tmp345 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp346 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx347 = getelementptr inbounds <4 x float> addrspace(1)* %tmp346, i32 %tmp345 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp348 = load <4 x float> addrspace(1)* %arrayidx347 ; <<4 x float>> [#uses=1]
  %tmp349 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub350 = fsub <4 x float> %tmp349, %tmp348     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub350, <4 x float>* %colorAccumulator
  %tmp351 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp352 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx353 = getelementptr inbounds <4 x float> addrspace(1)* %tmp352, i32 %tmp351 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp354 = load <4 x float> addrspace(1)* %arrayidx353 ; <<4 x float>> [#uses=1]
  %tmp355 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add356 = fadd <4 x float> %tmp355, %tmp354     ; <<4 x float>> [#uses=1]
  store <4 x float> %add356, <4 x float>* %colorAccumulator
  %tmp357 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp358 = load float* %denominator              ; <float> [#uses=1]
  %tmp359 = insertelement <4 x float> undef, float %tmp358, i32 0 ; <<4 x float>> [#uses=2]
  %splat360 = shufflevector <4 x float> %tmp359, <4 x float> %tmp359, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp361 = fcmp oeq <4 x float> zeroinitializer, %splat360 ; <<4 x i1>> [#uses=1]
  %sel362 = select <4 x i1> %cmp361, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat360 ; <<4 x float>> [#uses=0]
  %div363 = fdiv <4 x float> %tmp357, %splat360   ; <<4 x float>> [#uses=1]
  %tmp364 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp365 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx366 = getelementptr inbounds <4 x float> addrspace(1)* %tmp365, i32 %tmp364 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div363, <4 x float> addrspace(1)* %arrayidx366
  %tmp367 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc368 = add i32 %tmp367, 1                    ; <i32> [#uses=1]
  store i32 %inc368, i32* %sourceIndex
  %tmp369 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  %inc370 = add i32 %tmp369, 1                    ; <i32> [#uses=1]
  store i32 %inc370, i32* %leftColumnIndex
  br label %for.inc371

for.inc371:                                       ; preds = %for.body304
  %tmp372 = load i32* %column299                  ; <i32> [#uses=1]
  %inc373 = add i32 %tmp372, 1                    ; <i32> [#uses=1]
  store i32 %inc373, i32* %column299
  br label %for.cond300

for.end374:                                       ; preds = %for.cond300
  %tmp375 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool = trunc i8 %tmp375 to i1                ; <i1> [#uses=1]
  br i1 %tobool, label %land.lhs.true, label %if.end411

land.lhs.true:                                    ; preds = %for.end374
  %tmp376 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool377 = trunc i8 %tmp376 to i1             ; <i1> [#uses=1]
  br i1 %tobool377, label %if.then378, label %if.end411

if.then378:                                       ; preds = %land.lhs.true
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp379 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx380 = getelementptr inbounds <4 x float> addrspace(1)* %tmp379, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp381 = load <4 x float> addrspace(1)* %arrayidx380 ; <<4 x float>> [#uses=1]
  %tmp382 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add383 = fadd <4 x float> %tmp382, %tmp381     ; <<4 x float>> [#uses=1]
  store <4 x float> %add383, <4 x float>* %colorAccumulator
  %tmp384 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx385 = getelementptr inbounds <4 x float> addrspace(1)* %tmp384, i32 1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp386 = load <4 x float> addrspace(1)* %arrayidx385 ; <<4 x float>> [#uses=1]
  %tmp387 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add388 = fadd <4 x float> %tmp387, %tmp386     ; <<4 x float>> [#uses=1]
  store <4 x float> %add388, <4 x float>* %colorAccumulator
  %tmp389 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp390 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx391 = getelementptr inbounds <4 x float> addrspace(1)* %tmp390, i32 %tmp389 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp392 = load <4 x float> addrspace(1)* %arrayidx391 ; <<4 x float>> [#uses=1]
  %tmp393 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add394 = fadd <4 x float> %tmp393, %tmp392     ; <<4 x float>> [#uses=1]
  store <4 x float> %add394, <4 x float>* %colorAccumulator
  %tmp395 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add396 = add i32 %tmp395, 1                    ; <i32> [#uses=1]
  %tmp397 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx398 = getelementptr inbounds <4 x float> addrspace(1)* %tmp397, i32 %add396 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp399 = load <4 x float> addrspace(1)* %arrayidx398 ; <<4 x float>> [#uses=1]
  %tmp400 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add401 = fadd <4 x float> %tmp400, %tmp399     ; <<4 x float>> [#uses=1]
  store <4 x float> %add401, <4 x float>* %colorAccumulator
  %tmp402 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp403 = load float* %denominator              ; <float> [#uses=1]
  %tmp404 = insertelement <4 x float> undef, float %tmp403, i32 0 ; <<4 x float>> [#uses=2]
  %splat405 = shufflevector <4 x float> %tmp404, <4 x float> %tmp404, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp406 = fcmp oeq <4 x float> zeroinitializer, %splat405 ; <<4 x i1>> [#uses=1]
  %sel407 = select <4 x i1> %cmp406, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat405 ; <<4 x float>> [#uses=0]
  %div408 = fdiv <4 x float> %tmp402, %splat405   ; <<4 x float>> [#uses=1]
  %tmp409 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx410 = getelementptr inbounds <4 x float> addrspace(1)* %tmp409, i32 0 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div408, <4 x float> addrspace(1)* %arrayidx410
  br label %if.end411

if.end411:                                        ; preds = %if.then378, %land.lhs.true, %for.end374
  %tmp412 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool413 = trunc i8 %tmp412 to i1             ; <i1> [#uses=1]
  br i1 %tobool413, label %if.then414, label %if.end485

if.then414:                                       ; preds = %if.end411
  %tmp417 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp417, i32* %column416
  br label %for.cond418

for.cond418:                                      ; preds = %for.inc481, %if.then414
  %tmp419 = load i32* %column416                  ; <i32> [#uses=1]
  %tmp420 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp421 = load i32* %count_x                    ; <i32> [#uses=1]
  %add422 = add i32 %tmp420, %tmp421              ; <i32> [#uses=1]
  %cmp423 = icmp ult i32 %tmp419, %add422         ; <i1> [#uses=1]
  br i1 %cmp423, label %for.body424, label %for.end484

for.body424:                                      ; preds = %for.cond418
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp425 = load i32* %column416                  ; <i32> [#uses=1]
  %sub426 = sub i32 %tmp425, 1                    ; <i32> [#uses=1]
  %tmp427 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx428 = getelementptr inbounds <4 x float> addrspace(1)* %tmp427, i32 %sub426 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp429 = load <4 x float> addrspace(1)* %arrayidx428 ; <<4 x float>> [#uses=1]
  %tmp430 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add431 = fadd <4 x float> %tmp430, %tmp429     ; <<4 x float>> [#uses=1]
  store <4 x float> %add431, <4 x float>* %colorAccumulator
  %tmp432 = load i32* %column416                  ; <i32> [#uses=1]
  %tmp433 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx434 = getelementptr inbounds <4 x float> addrspace(1)* %tmp433, i32 %tmp432 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp435 = load <4 x float> addrspace(1)* %arrayidx434 ; <<4 x float>> [#uses=1]
  %tmp436 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add437 = fadd <4 x float> %tmp436, %tmp435     ; <<4 x float>> [#uses=1]
  store <4 x float> %add437, <4 x float>* %colorAccumulator
  %tmp438 = load i32* %column416                  ; <i32> [#uses=1]
  %add439 = add nsw i32 %tmp438, 1                ; <i32> [#uses=1]
  %tmp440 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx441 = getelementptr inbounds <4 x float> addrspace(1)* %tmp440, i32 %add439 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp442 = load <4 x float> addrspace(1)* %arrayidx441 ; <<4 x float>> [#uses=1]
  %tmp443 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add444 = fadd <4 x float> %tmp443, %tmp442     ; <<4 x float>> [#uses=1]
  store <4 x float> %add444, <4 x float>* %colorAccumulator
  %tmp445 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp446 = load i32* %column416                  ; <i32> [#uses=1]
  %add447 = add i32 %tmp445, %tmp446              ; <i32> [#uses=1]
  %sub448 = sub i32 %add447, 1                    ; <i32> [#uses=1]
  %tmp449 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx450 = getelementptr inbounds <4 x float> addrspace(1)* %tmp449, i32 %sub448 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp451 = load <4 x float> addrspace(1)* %arrayidx450 ; <<4 x float>> [#uses=1]
  %tmp452 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add453 = fadd <4 x float> %tmp452, %tmp451     ; <<4 x float>> [#uses=1]
  store <4 x float> %add453, <4 x float>* %colorAccumulator
  %tmp454 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp455 = load i32* %column416                  ; <i32> [#uses=1]
  %add456 = add i32 %tmp454, %tmp455              ; <i32> [#uses=1]
  %tmp457 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx458 = getelementptr inbounds <4 x float> addrspace(1)* %tmp457, i32 %add456 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp459 = load <4 x float> addrspace(1)* %arrayidx458 ; <<4 x float>> [#uses=1]
  %tmp460 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add461 = fadd <4 x float> %tmp460, %tmp459     ; <<4 x float>> [#uses=1]
  store <4 x float> %add461, <4 x float>* %colorAccumulator
  %tmp462 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp463 = load i32* %column416                  ; <i32> [#uses=1]
  %add464 = add i32 %tmp462, %tmp463              ; <i32> [#uses=1]
  %add465 = add i32 %add464, 1                    ; <i32> [#uses=1]
  %tmp466 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx467 = getelementptr inbounds <4 x float> addrspace(1)* %tmp466, i32 %add465 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp468 = load <4 x float> addrspace(1)* %arrayidx467 ; <<4 x float>> [#uses=1]
  %tmp469 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add470 = fadd <4 x float> %tmp469, %tmp468     ; <<4 x float>> [#uses=1]
  store <4 x float> %add470, <4 x float>* %colorAccumulator
  %tmp471 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp472 = load float* %denominator              ; <float> [#uses=1]
  %tmp473 = insertelement <4 x float> undef, float %tmp472, i32 0 ; <<4 x float>> [#uses=2]
  %splat474 = shufflevector <4 x float> %tmp473, <4 x float> %tmp473, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp475 = fcmp oeq <4 x float> zeroinitializer, %splat474 ; <<4 x i1>> [#uses=1]
  %sel476 = select <4 x i1> %cmp475, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat474 ; <<4 x float>> [#uses=0]
  %div477 = fdiv <4 x float> %tmp471, %splat474   ; <<4 x float>> [#uses=1]
  %tmp478 = load i32* %column416                  ; <i32> [#uses=1]
  %tmp479 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx480 = getelementptr inbounds <4 x float> addrspace(1)* %tmp479, i32 %tmp478 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div477, <4 x float> addrspace(1)* %arrayidx480
  br label %for.inc481

for.inc481:                                       ; preds = %for.body424
  %tmp482 = load i32* %column416                  ; <i32> [#uses=1]
  %inc483 = add nsw i32 %tmp482, 1                ; <i32> [#uses=1]
  store i32 %inc483, i32* %column416
  br label %for.cond418

for.end484:                                       ; preds = %for.cond418
  br label %if.end485

if.end485:                                        ; preds = %for.end484, %if.end411
  %tmp486 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool487 = trunc i8 %tmp486 to i1             ; <i1> [#uses=1]
  br i1 %tobool487, label %land.lhs.true488, label %if.end535

land.lhs.true488:                                 ; preds = %if.end485
  %tmp489 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool490 = trunc i8 %tmp489 to i1             ; <i1> [#uses=1]
  br i1 %tobool490, label %if.then491, label %if.end535

if.then491:                                       ; preds = %land.lhs.true488
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp492 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub493 = sub i32 %tmp492, 2                    ; <i32> [#uses=1]
  %tmp494 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx495 = getelementptr inbounds <4 x float> addrspace(1)* %tmp494, i32 %sub493 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp496 = load <4 x float> addrspace(1)* %arrayidx495 ; <<4 x float>> [#uses=1]
  %tmp497 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add498 = fadd <4 x float> %tmp497, %tmp496     ; <<4 x float>> [#uses=1]
  store <4 x float> %add498, <4 x float>* %colorAccumulator
  %tmp499 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub500 = sub i32 %tmp499, 1                    ; <i32> [#uses=1]
  %tmp501 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx502 = getelementptr inbounds <4 x float> addrspace(1)* %tmp501, i32 %sub500 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp503 = load <4 x float> addrspace(1)* %arrayidx502 ; <<4 x float>> [#uses=1]
  %tmp504 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add505 = fadd <4 x float> %tmp504, %tmp503     ; <<4 x float>> [#uses=1]
  store <4 x float> %add505, <4 x float>* %colorAccumulator
  %tmp506 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp507 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add508 = add i32 %tmp506, %tmp507              ; <i32> [#uses=1]
  %sub509 = sub i32 %add508, 2                    ; <i32> [#uses=1]
  %tmp510 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx511 = getelementptr inbounds <4 x float> addrspace(1)* %tmp510, i32 %sub509 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp512 = load <4 x float> addrspace(1)* %arrayidx511 ; <<4 x float>> [#uses=1]
  %tmp513 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add514 = fadd <4 x float> %tmp513, %tmp512     ; <<4 x float>> [#uses=1]
  store <4 x float> %add514, <4 x float>* %colorAccumulator
  %tmp515 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp516 = load i32* %width.addr                 ; <i32> [#uses=1]
  %add517 = add i32 %tmp515, %tmp516              ; <i32> [#uses=1]
  %sub518 = sub i32 %add517, 1                    ; <i32> [#uses=1]
  %tmp519 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx520 = getelementptr inbounds <4 x float> addrspace(1)* %tmp519, i32 %sub518 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp521 = load <4 x float> addrspace(1)* %arrayidx520 ; <<4 x float>> [#uses=1]
  %tmp522 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add523 = fadd <4 x float> %tmp522, %tmp521     ; <<4 x float>> [#uses=1]
  store <4 x float> %add523, <4 x float>* %colorAccumulator
  %tmp524 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp525 = load float* %denominator              ; <float> [#uses=1]
  %tmp526 = insertelement <4 x float> undef, float %tmp525, i32 0 ; <<4 x float>> [#uses=2]
  %splat527 = shufflevector <4 x float> %tmp526, <4 x float> %tmp526, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp528 = fcmp oeq <4 x float> zeroinitializer, %splat527 ; <<4 x i1>> [#uses=1]
  %sel529 = select <4 x i1> %cmp528, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat527 ; <<4 x float>> [#uses=0]
  %div530 = fdiv <4 x float> %tmp524, %splat527   ; <<4 x float>> [#uses=1]
  %tmp531 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub532 = sub i32 %tmp531, 1                    ; <i32> [#uses=1]
  %tmp533 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx534 = getelementptr inbounds <4 x float> addrspace(1)* %tmp533, i32 %sub532 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div530, <4 x float> addrspace(1)* %arrayidx534
  br label %if.end535

if.end535:                                        ; preds = %if.then491, %land.lhs.true488, %if.end485
  %tmp536 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool537 = trunc i8 %tmp536 to i1             ; <i1> [#uses=1]
  br i1 %tobool537, label %if.then538, label %if.end620

if.then538:                                       ; preds = %if.end535
  %tmp541 = load i32* %index_y                    ; <i32> [#uses=1]
  store i32 %tmp541, i32* %row540
  br label %for.cond542

for.cond542:                                      ; preds = %for.inc616, %if.then538
  %tmp543 = load i32* %row540                     ; <i32> [#uses=1]
  %tmp544 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp545 = load i32* %count_y                    ; <i32> [#uses=1]
  %add546 = add i32 %tmp544, %tmp545              ; <i32> [#uses=1]
  %cmp547 = icmp ult i32 %tmp543, %add546         ; <i1> [#uses=1]
  br i1 %cmp547, label %for.body548, label %for.end619

for.body548:                                      ; preds = %for.cond542
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp549 = load i32* %row540                     ; <i32> [#uses=1]
  %sub550 = sub i32 %tmp549, 1                    ; <i32> [#uses=1]
  %tmp551 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul552 = mul i32 %sub550, %tmp551              ; <i32> [#uses=1]
  %tmp553 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx554 = getelementptr inbounds <4 x float> addrspace(1)* %tmp553, i32 %mul552 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp555 = load <4 x float> addrspace(1)* %arrayidx554 ; <<4 x float>> [#uses=1]
  %tmp556 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add557 = fadd <4 x float> %tmp556, %tmp555     ; <<4 x float>> [#uses=1]
  store <4 x float> %add557, <4 x float>* %colorAccumulator
  %tmp558 = load i32* %row540                     ; <i32> [#uses=1]
  %tmp559 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul560 = mul i32 %tmp558, %tmp559              ; <i32> [#uses=1]
  %tmp561 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx562 = getelementptr inbounds <4 x float> addrspace(1)* %tmp561, i32 %mul560 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp563 = load <4 x float> addrspace(1)* %arrayidx562 ; <<4 x float>> [#uses=1]
  %tmp564 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add565 = fadd <4 x float> %tmp564, %tmp563     ; <<4 x float>> [#uses=1]
  store <4 x float> %add565, <4 x float>* %colorAccumulator
  %tmp566 = load i32* %row540                     ; <i32> [#uses=1]
  %add567 = add nsw i32 %tmp566, 1                ; <i32> [#uses=1]
  %tmp568 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul569 = mul i32 %add567, %tmp568              ; <i32> [#uses=1]
  %tmp570 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx571 = getelementptr inbounds <4 x float> addrspace(1)* %tmp570, i32 %mul569 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp572 = load <4 x float> addrspace(1)* %arrayidx571 ; <<4 x float>> [#uses=1]
  %tmp573 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add574 = fadd <4 x float> %tmp573, %tmp572     ; <<4 x float>> [#uses=1]
  store <4 x float> %add574, <4 x float>* %colorAccumulator
  %tmp575 = load i32* %row540                     ; <i32> [#uses=1]
  %sub576 = sub i32 %tmp575, 1                    ; <i32> [#uses=1]
  %tmp577 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul578 = mul i32 %sub576, %tmp577              ; <i32> [#uses=1]
  %add579 = add i32 %mul578, 1                    ; <i32> [#uses=1]
  %tmp580 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx581 = getelementptr inbounds <4 x float> addrspace(1)* %tmp580, i32 %add579 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp582 = load <4 x float> addrspace(1)* %arrayidx581 ; <<4 x float>> [#uses=1]
  %tmp583 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add584 = fadd <4 x float> %tmp583, %tmp582     ; <<4 x float>> [#uses=1]
  store <4 x float> %add584, <4 x float>* %colorAccumulator
  %tmp585 = load i32* %row540                     ; <i32> [#uses=1]
  %tmp586 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul587 = mul i32 %tmp585, %tmp586              ; <i32> [#uses=1]
  %add588 = add i32 %mul587, 1                    ; <i32> [#uses=1]
  %tmp589 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx590 = getelementptr inbounds <4 x float> addrspace(1)* %tmp589, i32 %add588 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp591 = load <4 x float> addrspace(1)* %arrayidx590 ; <<4 x float>> [#uses=1]
  %tmp592 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add593 = fadd <4 x float> %tmp592, %tmp591     ; <<4 x float>> [#uses=1]
  store <4 x float> %add593, <4 x float>* %colorAccumulator
  %tmp594 = load i32* %row540                     ; <i32> [#uses=1]
  %add595 = add nsw i32 %tmp594, 1                ; <i32> [#uses=1]
  %tmp596 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul597 = mul i32 %add595, %tmp596              ; <i32> [#uses=1]
  %add598 = add i32 %mul597, 1                    ; <i32> [#uses=1]
  %tmp599 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx600 = getelementptr inbounds <4 x float> addrspace(1)* %tmp599, i32 %add598 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp601 = load <4 x float> addrspace(1)* %arrayidx600 ; <<4 x float>> [#uses=1]
  %tmp602 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add603 = fadd <4 x float> %tmp602, %tmp601     ; <<4 x float>> [#uses=1]
  store <4 x float> %add603, <4 x float>* %colorAccumulator
  %tmp604 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp605 = load float* %denominator              ; <float> [#uses=1]
  %tmp606 = insertelement <4 x float> undef, float %tmp605, i32 0 ; <<4 x float>> [#uses=2]
  %splat607 = shufflevector <4 x float> %tmp606, <4 x float> %tmp606, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp608 = fcmp oeq <4 x float> zeroinitializer, %splat607 ; <<4 x i1>> [#uses=1]
  %sel609 = select <4 x i1> %cmp608, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat607 ; <<4 x float>> [#uses=0]
  %div610 = fdiv <4 x float> %tmp604, %splat607   ; <<4 x float>> [#uses=1]
  %tmp611 = load i32* %row540                     ; <i32> [#uses=1]
  %tmp612 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul613 = mul i32 %tmp611, %tmp612              ; <i32> [#uses=1]
  %tmp614 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx615 = getelementptr inbounds <4 x float> addrspace(1)* %tmp614, i32 %mul613 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div610, <4 x float> addrspace(1)* %arrayidx615
  br label %for.inc616

for.inc616:                                       ; preds = %for.body548
  %tmp617 = load i32* %row540                     ; <i32> [#uses=1]
  %inc618 = add nsw i32 %tmp617, 1                ; <i32> [#uses=1]
  store i32 %inc618, i32* %row540
  br label %for.cond542

for.end619:                                       ; preds = %for.cond542
  br label %if.end620

if.end620:                                        ; preds = %for.end619, %if.end535
  %tmp621 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool622 = trunc i8 %tmp621 to i1             ; <i1> [#uses=1]
  br i1 %tobool622, label %land.lhs.true623, label %if.end678

land.lhs.true623:                                 ; preds = %if.end620
  %tmp624 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool625 = trunc i8 %tmp624 to i1             ; <i1> [#uses=1]
  br i1 %tobool625, label %if.then626, label %if.end678

if.then626:                                       ; preds = %land.lhs.true623
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp627 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub628 = sub i32 %tmp627, 2                    ; <i32> [#uses=1]
  %tmp629 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul630 = mul i32 %sub628, %tmp629              ; <i32> [#uses=1]
  %tmp631 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx632 = getelementptr inbounds <4 x float> addrspace(1)* %tmp631, i32 %mul630 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp633 = load <4 x float> addrspace(1)* %arrayidx632 ; <<4 x float>> [#uses=1]
  %tmp634 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add635 = fadd <4 x float> %tmp634, %tmp633     ; <<4 x float>> [#uses=1]
  store <4 x float> %add635, <4 x float>* %colorAccumulator
  %tmp636 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub637 = sub i32 %tmp636, 2                    ; <i32> [#uses=1]
  %tmp638 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul639 = mul i32 %sub637, %tmp638              ; <i32> [#uses=1]
  %add640 = add i32 %mul639, 1                    ; <i32> [#uses=1]
  %tmp641 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx642 = getelementptr inbounds <4 x float> addrspace(1)* %tmp641, i32 %add640 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp643 = load <4 x float> addrspace(1)* %arrayidx642 ; <<4 x float>> [#uses=1]
  %tmp644 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add645 = fadd <4 x float> %tmp644, %tmp643     ; <<4 x float>> [#uses=1]
  store <4 x float> %add645, <4 x float>* %colorAccumulator
  %tmp646 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub647 = sub i32 %tmp646, 1                    ; <i32> [#uses=1]
  %tmp648 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul649 = mul i32 %sub647, %tmp648              ; <i32> [#uses=1]
  %tmp650 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx651 = getelementptr inbounds <4 x float> addrspace(1)* %tmp650, i32 %mul649 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp652 = load <4 x float> addrspace(1)* %arrayidx651 ; <<4 x float>> [#uses=1]
  %tmp653 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add654 = fadd <4 x float> %tmp653, %tmp652     ; <<4 x float>> [#uses=1]
  store <4 x float> %add654, <4 x float>* %colorAccumulator
  %tmp655 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub656 = sub i32 %tmp655, 1                    ; <i32> [#uses=1]
  %tmp657 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul658 = mul i32 %sub656, %tmp657              ; <i32> [#uses=1]
  %add659 = add i32 %mul658, 1                    ; <i32> [#uses=1]
  %tmp660 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx661 = getelementptr inbounds <4 x float> addrspace(1)* %tmp660, i32 %add659 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp662 = load <4 x float> addrspace(1)* %arrayidx661 ; <<4 x float>> [#uses=1]
  %tmp663 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add664 = fadd <4 x float> %tmp663, %tmp662     ; <<4 x float>> [#uses=1]
  store <4 x float> %add664, <4 x float>* %colorAccumulator
  %tmp665 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp666 = load float* %denominator              ; <float> [#uses=1]
  %tmp667 = insertelement <4 x float> undef, float %tmp666, i32 0 ; <<4 x float>> [#uses=2]
  %splat668 = shufflevector <4 x float> %tmp667, <4 x float> %tmp667, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp669 = fcmp oeq <4 x float> zeroinitializer, %splat668 ; <<4 x i1>> [#uses=1]
  %sel670 = select <4 x i1> %cmp669, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat668 ; <<4 x float>> [#uses=0]
  %div671 = fdiv <4 x float> %tmp665, %splat668   ; <<4 x float>> [#uses=1]
  %tmp672 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub673 = sub i32 %tmp672, 1                    ; <i32> [#uses=1]
  %tmp674 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul675 = mul i32 %sub673, %tmp674              ; <i32> [#uses=1]
  %tmp676 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx677 = getelementptr inbounds <4 x float> addrspace(1)* %tmp676, i32 %mul675 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div671, <4 x float> addrspace(1)* %arrayidx677
  br label %if.end678

if.end678:                                        ; preds = %if.then626, %land.lhs.true623, %if.end620
  %tmp679 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool680 = trunc i8 %tmp679 to i1             ; <i1> [#uses=1]
  br i1 %tobool680, label %if.then681, label %if.end781

if.then681:                                       ; preds = %if.end678
  %tmp684 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp684, i32* %column683
  br label %for.cond685

for.cond685:                                      ; preds = %for.inc777, %if.then681
  %tmp686 = load i32* %column683                  ; <i32> [#uses=1]
  %tmp687 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp688 = load i32* %count_x                    ; <i32> [#uses=1]
  %add689 = add i32 %tmp687, %tmp688              ; <i32> [#uses=1]
  %cmp690 = icmp ult i32 %tmp686, %add689         ; <i1> [#uses=1]
  br i1 %cmp690, label %for.body691, label %for.end780

for.body691:                                      ; preds = %for.cond685
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp692 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub693 = sub i32 %tmp692, 2                    ; <i32> [#uses=1]
  %tmp694 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul695 = mul i32 %sub693, %tmp694              ; <i32> [#uses=1]
  %tmp696 = load i32* %column683                  ; <i32> [#uses=1]
  %add697 = add i32 %mul695, %tmp696              ; <i32> [#uses=1]
  %sub698 = sub i32 %add697, 1                    ; <i32> [#uses=1]
  %tmp699 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx700 = getelementptr inbounds <4 x float> addrspace(1)* %tmp699, i32 %sub698 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp701 = load <4 x float> addrspace(1)* %arrayidx700 ; <<4 x float>> [#uses=1]
  %tmp702 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add703 = fadd <4 x float> %tmp702, %tmp701     ; <<4 x float>> [#uses=1]
  store <4 x float> %add703, <4 x float>* %colorAccumulator
  %tmp704 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub705 = sub i32 %tmp704, 2                    ; <i32> [#uses=1]
  %tmp706 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul707 = mul i32 %sub705, %tmp706              ; <i32> [#uses=1]
  %tmp708 = load i32* %column683                  ; <i32> [#uses=1]
  %add709 = add i32 %mul707, %tmp708              ; <i32> [#uses=1]
  %tmp710 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx711 = getelementptr inbounds <4 x float> addrspace(1)* %tmp710, i32 %add709 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp712 = load <4 x float> addrspace(1)* %arrayidx711 ; <<4 x float>> [#uses=1]
  %tmp713 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add714 = fadd <4 x float> %tmp713, %tmp712     ; <<4 x float>> [#uses=1]
  store <4 x float> %add714, <4 x float>* %colorAccumulator
  %tmp715 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub716 = sub i32 %tmp715, 2                    ; <i32> [#uses=1]
  %tmp717 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul718 = mul i32 %sub716, %tmp717              ; <i32> [#uses=1]
  %tmp719 = load i32* %column683                  ; <i32> [#uses=1]
  %add720 = add i32 %mul718, %tmp719              ; <i32> [#uses=1]
  %add721 = add i32 %add720, 1                    ; <i32> [#uses=1]
  %tmp722 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx723 = getelementptr inbounds <4 x float> addrspace(1)* %tmp722, i32 %add721 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp724 = load <4 x float> addrspace(1)* %arrayidx723 ; <<4 x float>> [#uses=1]
  %tmp725 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add726 = fadd <4 x float> %tmp725, %tmp724     ; <<4 x float>> [#uses=1]
  store <4 x float> %add726, <4 x float>* %colorAccumulator
  %tmp727 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub728 = sub i32 %tmp727, 1                    ; <i32> [#uses=1]
  %tmp729 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul730 = mul i32 %sub728, %tmp729              ; <i32> [#uses=1]
  %tmp731 = load i32* %column683                  ; <i32> [#uses=1]
  %add732 = add i32 %mul730, %tmp731              ; <i32> [#uses=1]
  %sub733 = sub i32 %add732, 1                    ; <i32> [#uses=1]
  %tmp734 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx735 = getelementptr inbounds <4 x float> addrspace(1)* %tmp734, i32 %sub733 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp736 = load <4 x float> addrspace(1)* %arrayidx735 ; <<4 x float>> [#uses=1]
  %tmp737 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add738 = fadd <4 x float> %tmp737, %tmp736     ; <<4 x float>> [#uses=1]
  store <4 x float> %add738, <4 x float>* %colorAccumulator
  %tmp739 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub740 = sub i32 %tmp739, 1                    ; <i32> [#uses=1]
  %tmp741 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul742 = mul i32 %sub740, %tmp741              ; <i32> [#uses=1]
  %tmp743 = load i32* %column683                  ; <i32> [#uses=1]
  %add744 = add i32 %mul742, %tmp743              ; <i32> [#uses=1]
  %tmp745 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx746 = getelementptr inbounds <4 x float> addrspace(1)* %tmp745, i32 %add744 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp747 = load <4 x float> addrspace(1)* %arrayidx746 ; <<4 x float>> [#uses=1]
  %tmp748 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add749 = fadd <4 x float> %tmp748, %tmp747     ; <<4 x float>> [#uses=1]
  store <4 x float> %add749, <4 x float>* %colorAccumulator
  %tmp750 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub751 = sub i32 %tmp750, 1                    ; <i32> [#uses=1]
  %tmp752 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul753 = mul i32 %sub751, %tmp752              ; <i32> [#uses=1]
  %tmp754 = load i32* %column683                  ; <i32> [#uses=1]
  %add755 = add i32 %mul753, %tmp754              ; <i32> [#uses=1]
  %add756 = add i32 %add755, 1                    ; <i32> [#uses=1]
  %tmp757 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx758 = getelementptr inbounds <4 x float> addrspace(1)* %tmp757, i32 %add756 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp759 = load <4 x float> addrspace(1)* %arrayidx758 ; <<4 x float>> [#uses=1]
  %tmp760 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add761 = fadd <4 x float> %tmp760, %tmp759     ; <<4 x float>> [#uses=1]
  store <4 x float> %add761, <4 x float>* %colorAccumulator
  %tmp762 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp763 = load float* %denominator              ; <float> [#uses=1]
  %tmp764 = insertelement <4 x float> undef, float %tmp763, i32 0 ; <<4 x float>> [#uses=2]
  %splat765 = shufflevector <4 x float> %tmp764, <4 x float> %tmp764, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp766 = fcmp oeq <4 x float> zeroinitializer, %splat765 ; <<4 x i1>> [#uses=1]
  %sel767 = select <4 x i1> %cmp766, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat765 ; <<4 x float>> [#uses=0]
  %div768 = fdiv <4 x float> %tmp762, %splat765   ; <<4 x float>> [#uses=1]
  %tmp769 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub770 = sub i32 %tmp769, 1                    ; <i32> [#uses=1]
  %tmp771 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul772 = mul i32 %sub770, %tmp771              ; <i32> [#uses=1]
  %tmp773 = load i32* %column683                  ; <i32> [#uses=1]
  %add774 = add i32 %mul772, %tmp773              ; <i32> [#uses=1]
  %tmp775 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx776 = getelementptr inbounds <4 x float> addrspace(1)* %tmp775, i32 %add774 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div768, <4 x float> addrspace(1)* %arrayidx776
  br label %for.inc777

for.inc777:                                       ; preds = %for.body691
  %tmp778 = load i32* %column683                  ; <i32> [#uses=1]
  %inc779 = add nsw i32 %tmp778, 1                ; <i32> [#uses=1]
  store i32 %inc779, i32* %column683
  br label %for.cond685

for.end780:                                       ; preds = %for.cond685
  br label %if.end781

if.end781:                                        ; preds = %for.end780, %if.end678
  %tmp782 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool783 = trunc i8 %tmp782 to i1             ; <i1> [#uses=1]
  br i1 %tobool783, label %if.then784, label %if.end871

if.then784:                                       ; preds = %if.end781
  %tmp787 = load i32* %index_y                    ; <i32> [#uses=1]
  store i32 %tmp787, i32* %row786
  br label %for.cond788

for.cond788:                                      ; preds = %for.inc867, %if.then784
  %tmp789 = load i32* %row786                     ; <i32> [#uses=1]
  %tmp790 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp791 = load i32* %count_y                    ; <i32> [#uses=1]
  %add792 = add i32 %tmp790, %tmp791              ; <i32> [#uses=1]
  %cmp793 = icmp ult i32 %tmp789, %add792         ; <i1> [#uses=1]
  br i1 %cmp793, label %for.body794, label %for.end870

for.body794:                                      ; preds = %for.cond788
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp795 = load i32* %row786                     ; <i32> [#uses=1]
  %tmp796 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul797 = mul i32 %tmp795, %tmp796              ; <i32> [#uses=1]
  %sub798 = sub i32 %mul797, 1                    ; <i32> [#uses=1]
  %tmp799 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx800 = getelementptr inbounds <4 x float> addrspace(1)* %tmp799, i32 %sub798 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp801 = load <4 x float> addrspace(1)* %arrayidx800 ; <<4 x float>> [#uses=1]
  %tmp802 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add803 = fadd <4 x float> %tmp802, %tmp801     ; <<4 x float>> [#uses=1]
  store <4 x float> %add803, <4 x float>* %colorAccumulator
  %tmp804 = load i32* %row786                     ; <i32> [#uses=1]
  %add805 = add nsw i32 %tmp804, 1                ; <i32> [#uses=1]
  %tmp806 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul807 = mul i32 %add805, %tmp806              ; <i32> [#uses=1]
  %sub808 = sub i32 %mul807, 1                    ; <i32> [#uses=1]
  %tmp809 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx810 = getelementptr inbounds <4 x float> addrspace(1)* %tmp809, i32 %sub808 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp811 = load <4 x float> addrspace(1)* %arrayidx810 ; <<4 x float>> [#uses=1]
  %tmp812 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add813 = fadd <4 x float> %tmp812, %tmp811     ; <<4 x float>> [#uses=1]
  store <4 x float> %add813, <4 x float>* %colorAccumulator
  %tmp814 = load i32* %row786                     ; <i32> [#uses=1]
  %add815 = add nsw i32 %tmp814, 2                ; <i32> [#uses=1]
  %tmp816 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul817 = mul i32 %add815, %tmp816              ; <i32> [#uses=1]
  %sub818 = sub i32 %mul817, 1                    ; <i32> [#uses=1]
  %tmp819 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx820 = getelementptr inbounds <4 x float> addrspace(1)* %tmp819, i32 %sub818 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp821 = load <4 x float> addrspace(1)* %arrayidx820 ; <<4 x float>> [#uses=1]
  %tmp822 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add823 = fadd <4 x float> %tmp822, %tmp821     ; <<4 x float>> [#uses=1]
  store <4 x float> %add823, <4 x float>* %colorAccumulator
  %tmp824 = load i32* %row786                     ; <i32> [#uses=1]
  %tmp825 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul826 = mul i32 %tmp824, %tmp825              ; <i32> [#uses=1]
  %sub827 = sub i32 %mul826, 2                    ; <i32> [#uses=1]
  %tmp828 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx829 = getelementptr inbounds <4 x float> addrspace(1)* %tmp828, i32 %sub827 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp830 = load <4 x float> addrspace(1)* %arrayidx829 ; <<4 x float>> [#uses=1]
  %tmp831 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add832 = fadd <4 x float> %tmp831, %tmp830     ; <<4 x float>> [#uses=1]
  store <4 x float> %add832, <4 x float>* %colorAccumulator
  %tmp833 = load i32* %row786                     ; <i32> [#uses=1]
  %add834 = add nsw i32 %tmp833, 1                ; <i32> [#uses=1]
  %tmp835 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul836 = mul i32 %add834, %tmp835              ; <i32> [#uses=1]
  %sub837 = sub i32 %mul836, 2                    ; <i32> [#uses=1]
  %tmp838 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx839 = getelementptr inbounds <4 x float> addrspace(1)* %tmp838, i32 %sub837 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp840 = load <4 x float> addrspace(1)* %arrayidx839 ; <<4 x float>> [#uses=1]
  %tmp841 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add842 = fadd <4 x float> %tmp841, %tmp840     ; <<4 x float>> [#uses=1]
  store <4 x float> %add842, <4 x float>* %colorAccumulator
  %tmp843 = load i32* %row786                     ; <i32> [#uses=1]
  %add844 = add nsw i32 %tmp843, 2                ; <i32> [#uses=1]
  %tmp845 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul846 = mul i32 %add844, %tmp845              ; <i32> [#uses=1]
  %sub847 = sub i32 %mul846, 2                    ; <i32> [#uses=1]
  %tmp848 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx849 = getelementptr inbounds <4 x float> addrspace(1)* %tmp848, i32 %sub847 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp850 = load <4 x float> addrspace(1)* %arrayidx849 ; <<4 x float>> [#uses=1]
  %tmp851 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add852 = fadd <4 x float> %tmp851, %tmp850     ; <<4 x float>> [#uses=1]
  store <4 x float> %add852, <4 x float>* %colorAccumulator
  %tmp853 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp854 = load float* %denominator              ; <float> [#uses=1]
  %tmp855 = insertelement <4 x float> undef, float %tmp854, i32 0 ; <<4 x float>> [#uses=2]
  %splat856 = shufflevector <4 x float> %tmp855, <4 x float> %tmp855, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp857 = fcmp oeq <4 x float> zeroinitializer, %splat856 ; <<4 x i1>> [#uses=1]
  %sel858 = select <4 x i1> %cmp857, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat856 ; <<4 x float>> [#uses=0]
  %div859 = fdiv <4 x float> %tmp853, %splat856   ; <<4 x float>> [#uses=1]
  %tmp860 = load i32* %row786                     ; <i32> [#uses=1]
  %add861 = add nsw i32 %tmp860, 1                ; <i32> [#uses=1]
  %tmp862 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul863 = mul i32 %add861, %tmp862              ; <i32> [#uses=1]
  %sub864 = sub i32 %mul863, 1                    ; <i32> [#uses=1]
  %tmp865 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx866 = getelementptr inbounds <4 x float> addrspace(1)* %tmp865, i32 %sub864 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div859, <4 x float> addrspace(1)* %arrayidx866
  br label %for.inc867

for.inc867:                                       ; preds = %for.body794
  %tmp868 = load i32* %row786                     ; <i32> [#uses=1]
  %inc869 = add nsw i32 %tmp868, 1                ; <i32> [#uses=1]
  store i32 %inc869, i32* %row786
  br label %for.cond788

for.end870:                                       ; preds = %for.cond788
  br label %if.end871

if.end871:                                        ; preds = %for.end870, %if.end781
  %tmp872 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool873 = trunc i8 %tmp872 to i1             ; <i1> [#uses=1]
  br i1 %tobool873, label %land.lhs.true874, label %if.end929

land.lhs.true874:                                 ; preds = %if.end871
  %tmp875 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool876 = trunc i8 %tmp875 to i1             ; <i1> [#uses=1]
  br i1 %tobool876, label %if.then877, label %if.end929

if.then877:                                       ; preds = %land.lhs.true874
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator
  %tmp878 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub879 = sub i32 %tmp878, 1                    ; <i32> [#uses=1]
  %tmp880 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul881 = mul i32 %sub879, %tmp880              ; <i32> [#uses=1]
  %sub882 = sub i32 %mul881, 2                    ; <i32> [#uses=1]
  %tmp883 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx884 = getelementptr inbounds <4 x float> addrspace(1)* %tmp883, i32 %sub882 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp885 = load <4 x float> addrspace(1)* %arrayidx884 ; <<4 x float>> [#uses=1]
  %tmp886 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add887 = fadd <4 x float> %tmp886, %tmp885     ; <<4 x float>> [#uses=1]
  store <4 x float> %add887, <4 x float>* %colorAccumulator
  %tmp888 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub889 = sub i32 %tmp888, 1                    ; <i32> [#uses=1]
  %tmp890 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul891 = mul i32 %sub889, %tmp890              ; <i32> [#uses=1]
  %sub892 = sub i32 %mul891, 1                    ; <i32> [#uses=1]
  %tmp893 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx894 = getelementptr inbounds <4 x float> addrspace(1)* %tmp893, i32 %sub892 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp895 = load <4 x float> addrspace(1)* %arrayidx894 ; <<4 x float>> [#uses=1]
  %tmp896 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add897 = fadd <4 x float> %tmp896, %tmp895     ; <<4 x float>> [#uses=1]
  store <4 x float> %add897, <4 x float>* %colorAccumulator
  %tmp898 = load i32* %height.addr                ; <i32> [#uses=1]
  %tmp899 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul900 = mul i32 %tmp898, %tmp899              ; <i32> [#uses=1]
  %sub901 = sub i32 %mul900, 2                    ; <i32> [#uses=1]
  %tmp902 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx903 = getelementptr inbounds <4 x float> addrspace(1)* %tmp902, i32 %sub901 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp904 = load <4 x float> addrspace(1)* %arrayidx903 ; <<4 x float>> [#uses=1]
  %tmp905 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add906 = fadd <4 x float> %tmp905, %tmp904     ; <<4 x float>> [#uses=1]
  store <4 x float> %add906, <4 x float>* %colorAccumulator
  %tmp907 = load i32* %height.addr                ; <i32> [#uses=1]
  %tmp908 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul909 = mul i32 %tmp907, %tmp908              ; <i32> [#uses=1]
  %sub910 = sub i32 %mul909, 1                    ; <i32> [#uses=1]
  %tmp911 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx912 = getelementptr inbounds <4 x float> addrspace(1)* %tmp911, i32 %sub910 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp913 = load <4 x float> addrspace(1)* %arrayidx912 ; <<4 x float>> [#uses=1]
  %tmp914 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add915 = fadd <4 x float> %tmp914, %tmp913     ; <<4 x float>> [#uses=1]
  store <4 x float> %add915, <4 x float>* %colorAccumulator
  %tmp916 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp917 = load float* %denominator              ; <float> [#uses=1]
  %tmp918 = insertelement <4 x float> undef, float %tmp917, i32 0 ; <<4 x float>> [#uses=2]
  %splat919 = shufflevector <4 x float> %tmp918, <4 x float> %tmp918, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp920 = fcmp oeq <4 x float> zeroinitializer, %splat919 ; <<4 x i1>> [#uses=1]
  %sel921 = select <4 x i1> %cmp920, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat919 ; <<4 x float>> [#uses=0]
  %div922 = fdiv <4 x float> %tmp916, %splat919   ; <<4 x float>> [#uses=1]
  %tmp923 = load i32* %height.addr                ; <i32> [#uses=1]
  %tmp924 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul925 = mul i32 %tmp923, %tmp924              ; <i32> [#uses=1]
  %sub926 = sub i32 %mul925, 1                    ; <i32> [#uses=1]
  %tmp927 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx928 = getelementptr inbounds <4 x float> addrspace(1)* %tmp927, i32 %sub926 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div922, <4 x float> addrspace(1)* %arrayidx928
  br label %if.end929

if.end929:                                        ; preds = %if.then877, %land.lhs.true874, %if.end871
  ret void
}
