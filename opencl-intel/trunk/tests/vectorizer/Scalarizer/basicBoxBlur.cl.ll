; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\basicBoxBlur.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_basicBoxBlur_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_basicBoxBlur_CPU_parameters = appending global [124 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, float const\00", section "llvm.metadata" ; <[124 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, float)* @basicBoxBlur_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_basicBoxBlur_CPU_locals to i8*), i8* getelementptr inbounds ([124 x i8]* @opencl_basicBoxBlur_CPU_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @basicBoxBlur_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, float %radius) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=16]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=13]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=39]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=11]
  %radius.addr = alloca float, align 4            ; <float*> [#uses=2]
  %dims = alloca i32, align 4                     ; <i32*> [#uses=1]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  %global_szx = alloca i32, align 4               ; <i32*> [#uses=3]
  %global_szy = alloca i32, align 4               ; <i32*> [#uses=3]
  %local_radius = alloca i32, align 4             ; <i32*> [#uses=70]
  %count_y = alloca i32, align 4                  ; <i32*> [#uses=14]
  %count_x = alloca i32, align 4                  ; <i32*> [#uses=15]
  %index_x = alloca i32, align 4                  ; <i32*> [#uses=20]
  %index_y = alloca i32, align 4                  ; <i32*> [#uses=20]
  %topEdge = alloca i8, align 1                   ; <i8*> [#uses=5]
  %bottomEdge = alloca i8, align 1                ; <i8*> [#uses=5]
  %leftEdge = alloca i8, align 1                  ; <i8*> [#uses=5]
  %rightEdge = alloca i8, align 1                 ; <i8*> [#uses=5]
  %denominator = alloca float, align 4            ; <float*> [#uses=13]
  %colorAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=45]
  %firstBlockAccumulator = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=11]
  %leftIndex = alloca i32, align 4                ; <i32*> [#uses=4]
  %y = alloca i32, align 4                        ; <i32*> [#uses=4]
  %x = alloca i32, align 4                        ; <i32*> [#uses=5]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=14]
  %leftColumnIndex = alloca i32, align 4          ; <i32*> [#uses=8]
  %rightColumnIndex = alloca i32, align 4         ; <i32*> [#uses=8]
  %tmpIndex1 = alloca i32, align 4                ; <i32*> [#uses=12]
  %tmpIndex2 = alloca i32, align 4                ; <i32*> [#uses=12]
  %topRowIndex = alloca i32, align 4              ; <i32*> [#uses=8]
  %bottomRowIndex = alloca i32, align 4           ; <i32*> [#uses=4]
  %row = alloca i32, align 4                      ; <i32*> [#uses=5]
  %column = alloca i32, align 4                   ; <i32*> [#uses=4]
  %c = alloca i32, align 4                        ; <i32*> [#uses=4]
  %r = alloca i32, align 4                        ; <i32*> [#uses=4]
  %column321 = alloca i32, align 4                ; <i32*> [#uses=4]
  %c333 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %row389 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column397 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  %j = alloca i32, align 4                        ; <i32*> [#uses=5]
  %h = alloca i32, align 4                        ; <i32*> [#uses=4]
  %w = alloca i32, align 4                        ; <i32*> [#uses=4]
  %row488 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column496 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i511 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %j521 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %h531 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %w536 = alloca i32, align 4                     ; <i32*> [#uses=2]
  %row595 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column603 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i622 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %j632 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %h642 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %w647 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %row709 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column720 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i732 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %j742 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %h752 = alloca i32, align 4                     ; <i32*> [#uses=2]
  %w757 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %row816 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column831 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i843 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %j853 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %h863 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %w868 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %row930 = alloca i32, align 4                   ; <i32*> [#uses=6]
  %column945 = alloca i32, align 4                ; <i32*> [#uses=6]
  %i960 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %j970 = alloca i32, align 4                     ; <i32*> [#uses=5]
  %h980 = alloca i32, align 4                     ; <i32*> [#uses=4]
  %w985 = alloca i32, align 4                     ; <i32*> [#uses=2]
  %row1042 = alloca i32, align 4                  ; <i32*> [#uses=6]
  %column1053 = alloca i32, align 4               ; <i32*> [#uses=6]
  %i1072 = alloca i32, align 4                    ; <i32*> [#uses=5]
  %j1082 = alloca i32, align 4                    ; <i32*> [#uses=5]
  %h1092 = alloca i32, align 4                    ; <i32*> [#uses=2]
  %w1097 = alloca i32, align 4                    ; <i32*> [#uses=4]
  %row1159 = alloca i32, align 4                  ; <i32*> [#uses=6]
  %column1174 = alloca i32, align 4               ; <i32*> [#uses=6]
  %i1193 = alloca i32, align 4                    ; <i32*> [#uses=5]
  %j1203 = alloca i32, align 4                    ; <i32*> [#uses=5]
  %h1213 = alloca i32, align 4                    ; <i32*> [#uses=4]
  %w1218 = alloca i32, align 4                    ; <i32*> [#uses=4]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %width, i32* %width.addr
  store i32 %height, i32* %height.addr
  store float %radius, float* %radius.addr
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
  %tmp = load float* %radius.addr                 ; <float> [#uses=1]
  %call5 = call float @_Z4ceilf(float %tmp)       ; <float> [#uses=1]
  %conv = fptosi float %call5 to i32              ; <i32> [#uses=1]
  store i32 %conv, i32* %local_radius
  %tmp7 = load i32* %height.addr                  ; <i32> [#uses=1]
  %tmp8 = load i32* %global_szy                   ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp8                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp8         ; <i32> [#uses=1]
  %div = udiv i32 %tmp7, %sel                     ; <i32> [#uses=1]
  store i32 %div, i32* %count_y
  %tmp10 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp11 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp12 = icmp eq i32 0, %tmp11                  ; <i1> [#uses=1]
  %sel13 = select i1 %cmp12, i32 1, i32 %tmp11    ; <i32> [#uses=1]
  %div14 = udiv i32 %tmp10, %sel13                ; <i32> [#uses=1]
  store i32 %div14, i32* %count_x
  %tmp16 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp17 = load i32* %globalIdx                   ; <i32> [#uses=1]
  %mul = mul i32 %tmp16, %tmp17                   ; <i32> [#uses=1]
  %tmp18 = load i32* %global_szx                  ; <i32> [#uses=2]
  %cmp19 = icmp eq i32 0, %tmp18                  ; <i1> [#uses=1]
  %sel20 = select i1 %cmp19, i32 1, i32 %tmp18    ; <i32> [#uses=1]
  %div21 = udiv i32 %mul, %sel20                  ; <i32> [#uses=1]
  store i32 %div21, i32* %index_x
  %tmp23 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp24 = load i32* %globalIdy                   ; <i32> [#uses=1]
  %mul25 = mul i32 %tmp23, %tmp24                 ; <i32> [#uses=1]
  %tmp26 = load i32* %global_szy                  ; <i32> [#uses=2]
  %cmp27 = icmp eq i32 0, %tmp26                  ; <i1> [#uses=1]
  %sel28 = select i1 %cmp27, i32 1, i32 %tmp26    ; <i32> [#uses=1]
  %div29 = udiv i32 %mul25, %sel28                ; <i32> [#uses=1]
  store i32 %div29, i32* %index_y
  store i8 0, i8* %topEdge
  store i8 0, i8* %bottomEdge
  store i8 0, i8* %leftEdge
  store i8 0, i8* %rightEdge
  %tmp34 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp35 = load i32* %count_y                     ; <i32> [#uses=1]
  %add = add i32 %tmp34, %tmp35                   ; <i32> [#uses=1]
  %tmp36 = load i32* %local_radius                ; <i32> [#uses=1]
  %add37 = add i32 %add, %tmp36                   ; <i32> [#uses=1]
  %tmp38 = load i32* %height.addr                 ; <i32> [#uses=1]
  %cmp39 = icmp uge i32 %add37, %tmp38            ; <i1> [#uses=1]
  br i1 %cmp39, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i8 1, i8* %bottomEdge
  %tmp41 = load i32* %height.addr                 ; <i32> [#uses=1]
  %tmp42 = load i32* %index_y                     ; <i32> [#uses=1]
  %sub = sub i32 %tmp41, %tmp42                   ; <i32> [#uses=1]
  %tmp43 = load i32* %local_radius                ; <i32> [#uses=1]
  %sub44 = sub i32 %sub, %tmp43                   ; <i32> [#uses=1]
  store i32 %sub44, i32* %count_y
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %tmp45 = load i32* %index_x                     ; <i32> [#uses=1]
  %tmp46 = load i32* %count_x                     ; <i32> [#uses=1]
  %add47 = add i32 %tmp45, %tmp46                 ; <i32> [#uses=1]
  %tmp48 = load i32* %local_radius                ; <i32> [#uses=1]
  %add49 = add i32 %add47, %tmp48                 ; <i32> [#uses=1]
  %tmp50 = load i32* %width.addr                  ; <i32> [#uses=1]
  %cmp51 = icmp uge i32 %add49, %tmp50            ; <i1> [#uses=1]
  br i1 %cmp51, label %if.then53, label %if.end59

if.then53:                                        ; preds = %if.end
  store i8 1, i8* %rightEdge
  %tmp54 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp55 = load i32* %index_x                     ; <i32> [#uses=1]
  %sub56 = sub i32 %tmp54, %tmp55                 ; <i32> [#uses=1]
  %tmp57 = load i32* %local_radius                ; <i32> [#uses=1]
  %sub58 = sub i32 %sub56, %tmp57                 ; <i32> [#uses=1]
  store i32 %sub58, i32* %count_x
  br label %if.end59

if.end59:                                         ; preds = %if.then53, %if.end
  %tmp60 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp61 = load i32* %local_radius                ; <i32> [#uses=1]
  %cmp62 = icmp ult i32 %tmp60, %tmp61            ; <i1> [#uses=1]
  br i1 %cmp62, label %if.then64, label %if.end69

if.then64:                                        ; preds = %if.end59
  store i8 1, i8* %topEdge
  %tmp65 = load i32* %local_radius                ; <i32> [#uses=1]
  store i32 %tmp65, i32* %index_y
  %tmp66 = load i32* %local_radius                ; <i32> [#uses=1]
  %tmp67 = load i32* %count_y                     ; <i32> [#uses=1]
  %sub68 = sub i32 %tmp67, %tmp66                 ; <i32> [#uses=1]
  store i32 %sub68, i32* %count_y
  br label %if.end69

if.end69:                                         ; preds = %if.then64, %if.end59
  %tmp70 = load i32* %index_x                     ; <i32> [#uses=1]
  %tmp71 = load i32* %local_radius                ; <i32> [#uses=1]
  %cmp72 = icmp ult i32 %tmp70, %tmp71            ; <i1> [#uses=1]
  br i1 %cmp72, label %if.then74, label %if.end79

if.then74:                                        ; preds = %if.end69
  store i8 1, i8* %leftEdge
  %tmp75 = load i32* %local_radius                ; <i32> [#uses=1]
  store i32 %tmp75, i32* %index_x
  %tmp76 = load i32* %local_radius                ; <i32> [#uses=1]
  %tmp77 = load i32* %count_x                     ; <i32> [#uses=1]
  %sub78 = sub i32 %tmp77, %tmp76                 ; <i32> [#uses=1]
  store i32 %sub78, i32* %count_x
  br label %if.end79

if.end79:                                         ; preds = %if.then74, %if.end69
  %tmp81 = load i32* %local_radius                ; <i32> [#uses=1]
  %mul82 = mul i32 2, %tmp81                      ; <i32> [#uses=1]
  %add83 = add nsw i32 %mul82, 1                  ; <i32> [#uses=1]
  %tmp84 = load i32* %local_radius                ; <i32> [#uses=1]
  %mul85 = mul i32 2, %tmp84                      ; <i32> [#uses=1]
  %add86 = add nsw i32 %mul85, 1                  ; <i32> [#uses=1]
  %mul87 = mul i32 %add83, %add86                 ; <i32> [#uses=1]
  %conv88 = sitofp i32 %mul87 to float            ; <float> [#uses=1]
  store float %conv88, float* %denominator
  %call90 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv91 = sitofp i32 %call90 to float           ; <float> [#uses=1]
  %tmp92 = insertelement <4 x float> undef, float %conv91, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp92, <4 x float> %tmp92, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat, <4 x float>* %colorAccumulator
  %call94 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv95 = sitofp i32 %call94 to float           ; <float> [#uses=1]
  %tmp96 = insertelement <4 x float> undef, float %conv95, i32 0 ; <<4 x float>> [#uses=2]
  %splat97 = shufflevector <4 x float> %tmp96, <4 x float> %tmp96, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat97, <4 x float>* %firstBlockAccumulator
  %tmp99 = load i32* %index_y                     ; <i32> [#uses=1]
  %tmp100 = load i32* %local_radius               ; <i32> [#uses=1]
  %sub101 = sub i32 %tmp99, %tmp100               ; <i32> [#uses=1]
  %tmp102 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul103 = mul i32 %sub101, %tmp102              ; <i32> [#uses=1]
  %tmp104 = load i32* %index_x                    ; <i32> [#uses=1]
  %add105 = add i32 %mul103, %tmp104              ; <i32> [#uses=1]
  %tmp106 = load i32* %local_radius               ; <i32> [#uses=1]
  %sub107 = sub i32 %add105, %tmp106              ; <i32> [#uses=1]
  store i32 %sub107, i32* %leftIndex
  store i32 0, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc135, %if.end79
  %tmp109 = load i32* %y                          ; <i32> [#uses=1]
  %tmp110 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul111 = mul i32 2, %tmp110                    ; <i32> [#uses=1]
  %add112 = add nsw i32 %mul111, 1                ; <i32> [#uses=1]
  %cmp113 = icmp ult i32 %tmp109, %add112         ; <i1> [#uses=1]
  br i1 %cmp113, label %for.body, label %for.end138

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %x
  br label %for.cond116

for.cond116:                                      ; preds = %for.inc, %for.body
  %tmp117 = load i32* %x                          ; <i32> [#uses=1]
  %tmp118 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul119 = mul i32 2, %tmp118                    ; <i32> [#uses=1]
  %add120 = add nsw i32 %mul119, 1                ; <i32> [#uses=1]
  %cmp121 = icmp ult i32 %tmp117, %add120         ; <i1> [#uses=1]
  br i1 %cmp121, label %for.body123, label %for.end

for.body123:                                      ; preds = %for.cond116
  %tmp124 = load i32* %leftIndex                  ; <i32> [#uses=1]
  %tmp125 = load i32* %x                          ; <i32> [#uses=1]
  %add126 = add i32 %tmp124, %tmp125              ; <i32> [#uses=1]
  %tmp127 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp127, i32 %add126 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp128 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp129 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add130 = fadd <4 x float> %tmp129, %tmp128     ; <<4 x float>> [#uses=1]
  store <4 x float> %add130, <4 x float>* %firstBlockAccumulator
  br label %for.inc

for.inc:                                          ; preds = %for.body123
  %tmp131 = load i32* %x                          ; <i32> [#uses=1]
  %inc = add i32 %tmp131, 1                       ; <i32> [#uses=1]
  store i32 %inc, i32* %x
  br label %for.cond116

for.end:                                          ; preds = %for.cond116
  %tmp132 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp133 = load i32* %leftIndex                  ; <i32> [#uses=1]
  %add134 = add i32 %tmp133, %tmp132              ; <i32> [#uses=1]
  store i32 %add134, i32* %leftIndex
  br label %for.inc135

for.inc135:                                       ; preds = %for.end
  %tmp136 = load i32* %y                          ; <i32> [#uses=1]
  %inc137 = add i32 %tmp136, 1                    ; <i32> [#uses=1]
  store i32 %inc137, i32* %y
  br label %for.cond

for.end138:                                       ; preds = %for.cond
  %tmp140 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp141 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul142 = mul i32 %tmp140, %tmp141              ; <i32> [#uses=1]
  %tmp143 = load i32* %index_x                    ; <i32> [#uses=1]
  %add144 = add i32 %mul142, %tmp143              ; <i32> [#uses=1]
  store i32 %add144, i32* %sourceIndex
  %tmp145 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %tmp146 = load float* %denominator              ; <float> [#uses=1]
  %tmp147 = insertelement <4 x float> undef, float %tmp146, i32 0 ; <<4 x float>> [#uses=2]
  %splat148 = shufflevector <4 x float> %tmp147, <4 x float> %tmp147, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp149 = fcmp oeq <4 x float> zeroinitializer, %splat148 ; <<4 x i1>> [#uses=1]
  %sel150 = select <4 x i1> %cmp149, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat148 ; <<4 x float>> [#uses=0]
  %div151 = fdiv <4 x float> %tmp145, %splat148   ; <<4 x float>> [#uses=1]
  %tmp152 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp153 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx154 = getelementptr inbounds <4 x float> addrspace(1)* %tmp153, i32 %tmp152 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div151, <4 x float> addrspace(1)* %arrayidx154
  %tmp155 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp155, <4 x float>* %colorAccumulator
  %tmp156 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc157 = add i32 %tmp156, 1                    ; <i32> [#uses=1]
  store i32 %inc157, i32* %sourceIndex
  %tmp163 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp164 = load i32* %local_radius               ; <i32> [#uses=1]
  %sub165 = sub i32 %tmp163, %tmp164              ; <i32> [#uses=1]
  %tmp166 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul167 = mul i32 %sub165, %tmp166              ; <i32> [#uses=1]
  %tmp168 = load i32* %index_x                    ; <i32> [#uses=1]
  %add169 = add i32 %mul167, %tmp168              ; <i32> [#uses=1]
  %tmp170 = load i32* %local_radius               ; <i32> [#uses=1]
  %sub171 = sub i32 %add169, %tmp170              ; <i32> [#uses=1]
  store i32 %sub171, i32* %topRowIndex
  %tmp173 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp174 = load i32* %local_radius               ; <i32> [#uses=1]
  %add175 = add i32 %tmp173, %tmp174              ; <i32> [#uses=1]
  %tmp176 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul177 = mul i32 %add175, %tmp176              ; <i32> [#uses=1]
  %tmp178 = load i32* %index_x                    ; <i32> [#uses=1]
  %add179 = add i32 %mul177, %tmp178              ; <i32> [#uses=1]
  %tmp180 = load i32* %local_radius               ; <i32> [#uses=1]
  %sub181 = sub i32 %add179, %tmp180              ; <i32> [#uses=1]
  store i32 %sub181, i32* %bottomRowIndex
  store i32 0, i32* %row
  br label %for.cond183

for.cond183:                                      ; preds = %for.end290, %for.end138
  %tmp184 = load i32* %row                        ; <i32> [#uses=1]
  %tmp185 = load i32* %count_y                    ; <i32> [#uses=1]
  %sub186 = sub i32 %tmp185, 1                    ; <i32> [#uses=1]
  %cmp187 = icmp ult i32 %tmp184, %sub186         ; <i1> [#uses=1]
  br i1 %cmp187, label %for.body189, label %for.end314

for.body189:                                      ; preds = %for.cond183
  %tmp190 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp190, i32* %leftColumnIndex
  %tmp191 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %tmp192 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul193 = mul i32 2, %tmp192                    ; <i32> [#uses=1]
  %add194 = add i32 %tmp191, %mul193              ; <i32> [#uses=1]
  store i32 %add194, i32* %rightColumnIndex
  store i32 1, i32* %column
  br label %for.cond196

for.cond196:                                      ; preds = %for.inc251, %for.body189
  %tmp197 = load i32* %column                     ; <i32> [#uses=1]
  %tmp198 = load i32* %count_x                    ; <i32> [#uses=1]
  %cmp199 = icmp ult i32 %tmp197, %tmp198         ; <i1> [#uses=1]
  br i1 %cmp199, label %for.body201, label %for.end254

for.body201:                                      ; preds = %for.cond196
  %tmp202 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  %inc203 = add i32 %tmp202, 1                    ; <i32> [#uses=1]
  store i32 %inc203, i32* %rightColumnIndex
  %tmp204 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  store i32 %tmp204, i32* %tmpIndex1
  %tmp205 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  store i32 %tmp205, i32* %tmpIndex2
  store i32 0, i32* %c
  br label %for.cond207

for.cond207:                                      ; preds = %for.inc233, %for.body201
  %tmp208 = load i32* %c                          ; <i32> [#uses=1]
  %tmp209 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul210 = mul i32 2, %tmp209                    ; <i32> [#uses=1]
  %add211 = add nsw i32 %mul210, 1                ; <i32> [#uses=1]
  %cmp212 = icmp ult i32 %tmp208, %add211         ; <i1> [#uses=1]
  br i1 %cmp212, label %for.body214, label %for.end236

for.body214:                                      ; preds = %for.cond207
  %tmp215 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp216 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx217 = getelementptr inbounds <4 x float> addrspace(1)* %tmp216, i32 %tmp215 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp218 = load <4 x float> addrspace(1)* %arrayidx217 ; <<4 x float>> [#uses=1]
  %tmp219 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub220 = fsub <4 x float> %tmp219, %tmp218     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub220, <4 x float>* %colorAccumulator
  %tmp221 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp222 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add223 = add i32 %tmp222, %tmp221              ; <i32> [#uses=1]
  store i32 %add223, i32* %tmpIndex1
  %tmp224 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp225 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx226 = getelementptr inbounds <4 x float> addrspace(1)* %tmp225, i32 %tmp224 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp227 = load <4 x float> addrspace(1)* %arrayidx226 ; <<4 x float>> [#uses=1]
  %tmp228 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add229 = fadd <4 x float> %tmp228, %tmp227     ; <<4 x float>> [#uses=1]
  store <4 x float> %add229, <4 x float>* %colorAccumulator
  %tmp230 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp231 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add232 = add i32 %tmp231, %tmp230              ; <i32> [#uses=1]
  store i32 %add232, i32* %tmpIndex2
  br label %for.inc233

for.inc233:                                       ; preds = %for.body214
  %tmp234 = load i32* %c                          ; <i32> [#uses=1]
  %inc235 = add i32 %tmp234, 1                    ; <i32> [#uses=1]
  store i32 %inc235, i32* %c
  br label %for.cond207

for.end236:                                       ; preds = %for.cond207
  %tmp237 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp238 = load float* %denominator              ; <float> [#uses=1]
  %tmp239 = insertelement <4 x float> undef, float %tmp238, i32 0 ; <<4 x float>> [#uses=2]
  %splat240 = shufflevector <4 x float> %tmp239, <4 x float> %tmp239, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp241 = fcmp oeq <4 x float> zeroinitializer, %splat240 ; <<4 x i1>> [#uses=1]
  %sel242 = select <4 x i1> %cmp241, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat240 ; <<4 x float>> [#uses=0]
  %div243 = fdiv <4 x float> %tmp237, %splat240   ; <<4 x float>> [#uses=1]
  %tmp244 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp245 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx246 = getelementptr inbounds <4 x float> addrspace(1)* %tmp245, i32 %tmp244 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div243, <4 x float> addrspace(1)* %arrayidx246
  %tmp247 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc248 = add i32 %tmp247, 1                    ; <i32> [#uses=1]
  store i32 %inc248, i32* %sourceIndex
  %tmp249 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  %inc250 = add i32 %tmp249, 1                    ; <i32> [#uses=1]
  store i32 %inc250, i32* %leftColumnIndex
  br label %for.inc251

for.inc251:                                       ; preds = %for.end236
  %tmp252 = load i32* %column                     ; <i32> [#uses=1]
  %inc253 = add i32 %tmp252, 1                    ; <i32> [#uses=1]
  store i32 %inc253, i32* %column
  br label %for.cond196

for.end254:                                       ; preds = %for.cond196
  %tmp255 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp256 = load i32* %bottomRowIndex             ; <i32> [#uses=1]
  %add257 = add i32 %tmp256, %tmp255              ; <i32> [#uses=1]
  store i32 %add257, i32* %bottomRowIndex
  %tmp258 = load i32* %row                        ; <i32> [#uses=1]
  %inc259 = add i32 %tmp258, 1                    ; <i32> [#uses=1]
  store i32 %inc259, i32* %row
  %tmp260 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp260, i32* %tmpIndex1
  %tmp261 = load i32* %bottomRowIndex             ; <i32> [#uses=1]
  store i32 %tmp261, i32* %tmpIndex2
  store i32 0, i32* %r
  br label %for.cond263

for.cond263:                                      ; preds = %for.inc287, %for.end254
  %tmp264 = load i32* %r                          ; <i32> [#uses=1]
  %tmp265 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul266 = mul i32 2, %tmp265                    ; <i32> [#uses=1]
  %add267 = add nsw i32 %mul266, 1                ; <i32> [#uses=1]
  %cmp268 = icmp ult i32 %tmp264, %add267         ; <i1> [#uses=1]
  br i1 %cmp268, label %for.body270, label %for.end290

for.body270:                                      ; preds = %for.cond263
  %tmp271 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp272 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx273 = getelementptr inbounds <4 x float> addrspace(1)* %tmp272, i32 %tmp271 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp274 = load <4 x float> addrspace(1)* %arrayidx273 ; <<4 x float>> [#uses=1]
  %tmp275 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %sub276 = fsub <4 x float> %tmp275, %tmp274     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub276, <4 x float>* %firstBlockAccumulator
  %tmp277 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %inc278 = add i32 %tmp277, 1                    ; <i32> [#uses=1]
  store i32 %inc278, i32* %tmpIndex1
  %tmp279 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp280 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx281 = getelementptr inbounds <4 x float> addrspace(1)* %tmp280, i32 %tmp279 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp282 = load <4 x float> addrspace(1)* %arrayidx281 ; <<4 x float>> [#uses=1]
  %tmp283 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %add284 = fadd <4 x float> %tmp283, %tmp282     ; <<4 x float>> [#uses=1]
  store <4 x float> %add284, <4 x float>* %firstBlockAccumulator
  %tmp285 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %inc286 = add i32 %tmp285, 1                    ; <i32> [#uses=1]
  store i32 %inc286, i32* %tmpIndex2
  br label %for.inc287

for.inc287:                                       ; preds = %for.body270
  %tmp288 = load i32* %r                          ; <i32> [#uses=1]
  %inc289 = add i32 %tmp288, 1                    ; <i32> [#uses=1]
  store i32 %inc289, i32* %r
  br label %for.cond263

for.end290:                                       ; preds = %for.cond263
  %tmp291 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp292 = load i32* %row                        ; <i32> [#uses=1]
  %add293 = add i32 %tmp291, %tmp292              ; <i32> [#uses=1]
  %tmp294 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul295 = mul i32 %add293, %tmp294              ; <i32> [#uses=1]
  %tmp296 = load i32* %index_x                    ; <i32> [#uses=1]
  %add297 = add i32 %mul295, %tmp296              ; <i32> [#uses=1]
  store i32 %add297, i32* %sourceIndex
  %tmp298 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  %tmp299 = load float* %denominator              ; <float> [#uses=1]
  %tmp300 = insertelement <4 x float> undef, float %tmp299, i32 0 ; <<4 x float>> [#uses=2]
  %splat301 = shufflevector <4 x float> %tmp300, <4 x float> %tmp300, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp302 = fcmp oeq <4 x float> zeroinitializer, %splat301 ; <<4 x i1>> [#uses=1]
  %sel303 = select <4 x i1> %cmp302, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat301 ; <<4 x float>> [#uses=0]
  %div304 = fdiv <4 x float> %tmp298, %splat301   ; <<4 x float>> [#uses=1]
  %tmp305 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp306 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx307 = getelementptr inbounds <4 x float> addrspace(1)* %tmp306, i32 %tmp305 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div304, <4 x float> addrspace(1)* %arrayidx307
  %tmp308 = load <4 x float>* %firstBlockAccumulator ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp308, <4 x float>* %colorAccumulator
  %tmp309 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc310 = add i32 %tmp309, 1                    ; <i32> [#uses=1]
  store i32 %inc310, i32* %sourceIndex
  %tmp311 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp312 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %add313 = add i32 %tmp312, %tmp311              ; <i32> [#uses=1]
  store i32 %add313, i32* %topRowIndex
  br label %for.cond183

for.end314:                                       ; preds = %for.cond183
  %tmp315 = load i32* %topRowIndex                ; <i32> [#uses=1]
  store i32 %tmp315, i32* %leftColumnIndex
  %tmp316 = load i32* %topRowIndex                ; <i32> [#uses=1]
  %tmp317 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul318 = mul i32 2, %tmp317                    ; <i32> [#uses=1]
  %add319 = add i32 %tmp316, %mul318              ; <i32> [#uses=1]
  store i32 %add319, i32* %rightColumnIndex
  store i32 1, i32* %column321
  br label %for.cond322

for.cond322:                                      ; preds = %for.inc378, %for.end314
  %tmp323 = load i32* %column321                  ; <i32> [#uses=1]
  %tmp324 = load i32* %count_x                    ; <i32> [#uses=1]
  %cmp325 = icmp ult i32 %tmp323, %tmp324         ; <i1> [#uses=1]
  br i1 %cmp325, label %for.body327, label %for.end381

for.body327:                                      ; preds = %for.cond322
  %tmp328 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  %inc329 = add i32 %tmp328, 1                    ; <i32> [#uses=1]
  store i32 %inc329, i32* %rightColumnIndex
  %tmp330 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  store i32 %tmp330, i32* %tmpIndex1
  %tmp331 = load i32* %rightColumnIndex           ; <i32> [#uses=1]
  store i32 %tmp331, i32* %tmpIndex2
  store i32 0, i32* %c333
  br label %for.cond334

for.cond334:                                      ; preds = %for.inc360, %for.body327
  %tmp335 = load i32* %c333                       ; <i32> [#uses=1]
  %tmp336 = load i32* %local_radius               ; <i32> [#uses=1]
  %mul337 = mul i32 2, %tmp336                    ; <i32> [#uses=1]
  %add338 = add nsw i32 %mul337, 1                ; <i32> [#uses=1]
  %cmp339 = icmp ult i32 %tmp335, %add338         ; <i1> [#uses=1]
  br i1 %cmp339, label %for.body341, label %for.end363

for.body341:                                      ; preds = %for.cond334
  %tmp342 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %tmp343 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx344 = getelementptr inbounds <4 x float> addrspace(1)* %tmp343, i32 %tmp342 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp345 = load <4 x float> addrspace(1)* %arrayidx344 ; <<4 x float>> [#uses=1]
  %tmp346 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %sub347 = fsub <4 x float> %tmp346, %tmp345     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub347, <4 x float>* %colorAccumulator
  %tmp348 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp349 = load i32* %tmpIndex1                  ; <i32> [#uses=1]
  %add350 = add i32 %tmp349, %tmp348              ; <i32> [#uses=1]
  store i32 %add350, i32* %tmpIndex1
  %tmp351 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %tmp352 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx353 = getelementptr inbounds <4 x float> addrspace(1)* %tmp352, i32 %tmp351 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp354 = load <4 x float> addrspace(1)* %arrayidx353 ; <<4 x float>> [#uses=1]
  %tmp355 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add356 = fadd <4 x float> %tmp355, %tmp354     ; <<4 x float>> [#uses=1]
  store <4 x float> %add356, <4 x float>* %colorAccumulator
  %tmp357 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp358 = load i32* %tmpIndex2                  ; <i32> [#uses=1]
  %add359 = add i32 %tmp358, %tmp357              ; <i32> [#uses=1]
  store i32 %add359, i32* %tmpIndex2
  br label %for.inc360

for.inc360:                                       ; preds = %for.body341
  %tmp361 = load i32* %c333                       ; <i32> [#uses=1]
  %inc362 = add i32 %tmp361, 1                    ; <i32> [#uses=1]
  store i32 %inc362, i32* %c333
  br label %for.cond334

for.end363:                                       ; preds = %for.cond334
  %tmp364 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp365 = load float* %denominator              ; <float> [#uses=1]
  %tmp366 = insertelement <4 x float> undef, float %tmp365, i32 0 ; <<4 x float>> [#uses=2]
  %splat367 = shufflevector <4 x float> %tmp366, <4 x float> %tmp366, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp368 = fcmp oeq <4 x float> zeroinitializer, %splat367 ; <<4 x i1>> [#uses=1]
  %sel369 = select <4 x i1> %cmp368, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat367 ; <<4 x float>> [#uses=0]
  %div370 = fdiv <4 x float> %tmp364, %splat367   ; <<4 x float>> [#uses=1]
  %tmp371 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %tmp372 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx373 = getelementptr inbounds <4 x float> addrspace(1)* %tmp372, i32 %tmp371 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div370, <4 x float> addrspace(1)* %arrayidx373
  %tmp374 = load i32* %sourceIndex                ; <i32> [#uses=1]
  %inc375 = add i32 %tmp374, 1                    ; <i32> [#uses=1]
  store i32 %inc375, i32* %sourceIndex
  %tmp376 = load i32* %leftColumnIndex            ; <i32> [#uses=1]
  %inc377 = add i32 %tmp376, 1                    ; <i32> [#uses=1]
  store i32 %inc377, i32* %leftColumnIndex
  br label %for.inc378

for.inc378:                                       ; preds = %for.end363
  %tmp379 = load i32* %column321                  ; <i32> [#uses=1]
  %inc380 = add i32 %tmp379, 1                    ; <i32> [#uses=1]
  store i32 %inc380, i32* %column321
  br label %for.cond322

for.end381:                                       ; preds = %for.cond322
  %tmp382 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool = trunc i8 %tmp382 to i1                ; <i1> [#uses=1]
  br i1 %tobool, label %land.lhs.true, label %if.end483

land.lhs.true:                                    ; preds = %for.end381
  %tmp384 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool385 = trunc i8 %tmp384 to i1             ; <i1> [#uses=1]
  br i1 %tobool385, label %if.then387, label %if.end483

if.then387:                                       ; preds = %land.lhs.true
  store i32 0, i32* %row389
  br label %for.cond390

for.cond390:                                      ; preds = %for.inc479, %if.then387
  %tmp391 = load i32* %row389                     ; <i32> [#uses=1]
  %tmp392 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp393 = icmp slt i32 %tmp391, %tmp392         ; <i1> [#uses=1]
  br i1 %cmp393, label %for.body395, label %for.end482

for.body395:                                      ; preds = %for.cond390
  store i32 0, i32* %column397
  br label %for.cond398

for.cond398:                                      ; preds = %for.inc475, %for.body395
  %tmp399 = load i32* %column397                  ; <i32> [#uses=1]
  %tmp400 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp401 = icmp slt i32 %tmp399, %tmp400         ; <i1> [#uses=1]
  br i1 %cmp401, label %for.body403, label %for.end478

for.body403:                                      ; preds = %for.cond398
  %call404 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv405 = sitofp i32 %call404 to float         ; <float> [#uses=1]
  %tmp406 = insertelement <4 x float> undef, float %conv405, i32 0 ; <<4 x float>> [#uses=2]
  %splat407 = shufflevector <4 x float> %tmp406, <4 x float> %tmp406, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat407, <4 x float>* %colorAccumulator
  %tmp409 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg = sub i32 0, %tmp409                       ; <i32> [#uses=1]
  store i32 %neg, i32* %i
  br label %for.cond410

for.cond410:                                      ; preds = %for.inc457, %for.body403
  %tmp411 = load i32* %i                          ; <i32> [#uses=1]
  %tmp412 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp413 = icmp sle i32 %tmp411, %tmp412         ; <i1> [#uses=1]
  br i1 %cmp413, label %for.body415, label %for.end460

for.body415:                                      ; preds = %for.cond410
  %tmp417 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg418 = sub i32 0, %tmp417                    ; <i32> [#uses=1]
  store i32 %neg418, i32* %j
  br label %for.cond419

for.cond419:                                      ; preds = %for.inc453, %for.body415
  %tmp420 = load i32* %j                          ; <i32> [#uses=1]
  %tmp421 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp422 = icmp sle i32 %tmp420, %tmp421         ; <i1> [#uses=1]
  br i1 %cmp422, label %for.body424, label %for.end456

for.body424:                                      ; preds = %for.cond419
  %tmp426 = load i32* %row389                     ; <i32> [#uses=1]
  %tmp427 = load i32* %i                          ; <i32> [#uses=1]
  %add428 = add nsw i32 %tmp426, %tmp427          ; <i32> [#uses=1]
  store i32 %add428, i32* %h
  %tmp430 = load i32* %column397                  ; <i32> [#uses=1]
  %tmp431 = load i32* %j                          ; <i32> [#uses=1]
  %add432 = add nsw i32 %tmp430, %tmp431          ; <i32> [#uses=1]
  store i32 %add432, i32* %w
  %tmp433 = load i32* %h                          ; <i32> [#uses=1]
  %cmp434 = icmp slt i32 %tmp433, 0               ; <i1> [#uses=1]
  br i1 %cmp434, label %if.then436, label %if.end437

if.then436:                                       ; preds = %for.body424
  store i32 0, i32* %h
  br label %if.end437

if.end437:                                        ; preds = %if.then436, %for.body424
  %tmp438 = load i32* %w                          ; <i32> [#uses=1]
  %cmp439 = icmp slt i32 %tmp438, 0               ; <i1> [#uses=1]
  br i1 %cmp439, label %if.then441, label %if.end442

if.then441:                                       ; preds = %if.end437
  store i32 0, i32* %w
  br label %if.end442

if.end442:                                        ; preds = %if.then441, %if.end437
  %tmp443 = load i32* %h                          ; <i32> [#uses=1]
  %tmp444 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul445 = mul i32 %tmp443, %tmp444              ; <i32> [#uses=1]
  %tmp446 = load i32* %w                          ; <i32> [#uses=1]
  %add447 = add i32 %mul445, %tmp446              ; <i32> [#uses=1]
  %tmp448 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx449 = getelementptr inbounds <4 x float> addrspace(1)* %tmp448, i32 %add447 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp450 = load <4 x float> addrspace(1)* %arrayidx449 ; <<4 x float>> [#uses=1]
  %tmp451 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add452 = fadd <4 x float> %tmp451, %tmp450     ; <<4 x float>> [#uses=1]
  store <4 x float> %add452, <4 x float>* %colorAccumulator
  br label %for.inc453

for.inc453:                                       ; preds = %if.end442
  %tmp454 = load i32* %j                          ; <i32> [#uses=1]
  %inc455 = add nsw i32 %tmp454, 1                ; <i32> [#uses=1]
  store i32 %inc455, i32* %j
  br label %for.cond419

for.end456:                                       ; preds = %for.cond419
  br label %for.inc457

for.inc457:                                       ; preds = %for.end456
  %tmp458 = load i32* %i                          ; <i32> [#uses=1]
  %inc459 = add nsw i32 %tmp458, 1                ; <i32> [#uses=1]
  store i32 %inc459, i32* %i
  br label %for.cond410

for.end460:                                       ; preds = %for.cond410
  %tmp461 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp462 = load float* %denominator              ; <float> [#uses=1]
  %tmp463 = insertelement <4 x float> undef, float %tmp462, i32 0 ; <<4 x float>> [#uses=2]
  %splat464 = shufflevector <4 x float> %tmp463, <4 x float> %tmp463, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp465 = fcmp oeq <4 x float> zeroinitializer, %splat464 ; <<4 x i1>> [#uses=1]
  %sel466 = select <4 x i1> %cmp465, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat464 ; <<4 x float>> [#uses=0]
  %div467 = fdiv <4 x float> %tmp461, %splat464   ; <<4 x float>> [#uses=1]
  %tmp468 = load i32* %row389                     ; <i32> [#uses=1]
  %tmp469 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul470 = mul i32 %tmp468, %tmp469              ; <i32> [#uses=1]
  %tmp471 = load i32* %column397                  ; <i32> [#uses=1]
  %add472 = add i32 %mul470, %tmp471              ; <i32> [#uses=1]
  %tmp473 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx474 = getelementptr inbounds <4 x float> addrspace(1)* %tmp473, i32 %add472 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div467, <4 x float> addrspace(1)* %arrayidx474
  br label %for.inc475

for.inc475:                                       ; preds = %for.end460
  %tmp476 = load i32* %column397                  ; <i32> [#uses=1]
  %inc477 = add nsw i32 %tmp476, 1                ; <i32> [#uses=1]
  store i32 %inc477, i32* %column397
  br label %for.cond398

for.end478:                                       ; preds = %for.cond398
  br label %for.inc479

for.inc479:                                       ; preds = %for.end478
  %tmp480 = load i32* %row389                     ; <i32> [#uses=1]
  %inc481 = add nsw i32 %tmp480, 1                ; <i32> [#uses=1]
  store i32 %inc481, i32* %row389
  br label %for.cond390

for.end482:                                       ; preds = %for.cond390
  br label %if.end483

if.end483:                                        ; preds = %for.end482, %land.lhs.true, %for.end381
  %tmp484 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool485 = trunc i8 %tmp484 to i1             ; <i1> [#uses=1]
  br i1 %tobool485, label %if.then486, label %if.end585

if.then486:                                       ; preds = %if.end483
  store i32 0, i32* %row488
  br label %for.cond489

for.cond489:                                      ; preds = %for.inc581, %if.then486
  %tmp490 = load i32* %row488                     ; <i32> [#uses=1]
  %tmp491 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp492 = icmp slt i32 %tmp490, %tmp491         ; <i1> [#uses=1]
  br i1 %cmp492, label %for.body494, label %for.end584

for.body494:                                      ; preds = %for.cond489
  %tmp497 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp497, i32* %column496
  br label %for.cond498

for.cond498:                                      ; preds = %for.inc577, %for.body494
  %tmp499 = load i32* %column496                  ; <i32> [#uses=1]
  %tmp500 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp501 = load i32* %count_x                    ; <i32> [#uses=1]
  %add502 = add i32 %tmp500, %tmp501              ; <i32> [#uses=1]
  %cmp503 = icmp ult i32 %tmp499, %add502         ; <i1> [#uses=1]
  br i1 %cmp503, label %for.body505, label %for.end580

for.body505:                                      ; preds = %for.cond498
  %call506 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv507 = sitofp i32 %call506 to float         ; <float> [#uses=1]
  %tmp508 = insertelement <4 x float> undef, float %conv507, i32 0 ; <<4 x float>> [#uses=2]
  %splat509 = shufflevector <4 x float> %tmp508, <4 x float> %tmp508, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat509, <4 x float>* %colorAccumulator
  %tmp512 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg513 = sub i32 0, %tmp512                    ; <i32> [#uses=1]
  store i32 %neg513, i32* %i511
  br label %for.cond514

for.cond514:                                      ; preds = %for.inc559, %for.body505
  %tmp515 = load i32* %i511                       ; <i32> [#uses=1]
  %tmp516 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp517 = icmp sle i32 %tmp515, %tmp516         ; <i1> [#uses=1]
  br i1 %cmp517, label %for.body519, label %for.end562

for.body519:                                      ; preds = %for.cond514
  %tmp522 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg523 = sub i32 0, %tmp522                    ; <i32> [#uses=1]
  store i32 %neg523, i32* %j521
  br label %for.cond524

for.cond524:                                      ; preds = %for.inc555, %for.body519
  %tmp525 = load i32* %j521                       ; <i32> [#uses=1]
  %tmp526 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp527 = icmp sle i32 %tmp525, %tmp526         ; <i1> [#uses=1]
  br i1 %cmp527, label %for.body529, label %for.end558

for.body529:                                      ; preds = %for.cond524
  %tmp532 = load i32* %row488                     ; <i32> [#uses=1]
  %tmp533 = load i32* %i511                       ; <i32> [#uses=1]
  %add534 = add nsw i32 %tmp532, %tmp533          ; <i32> [#uses=1]
  store i32 %add534, i32* %h531
  %tmp537 = load i32* %column496                  ; <i32> [#uses=1]
  %tmp538 = load i32* %j521                       ; <i32> [#uses=1]
  %add539 = add nsw i32 %tmp537, %tmp538          ; <i32> [#uses=1]
  store i32 %add539, i32* %w536
  %tmp540 = load i32* %h531                       ; <i32> [#uses=1]
  %cmp541 = icmp slt i32 %tmp540, 0               ; <i1> [#uses=1]
  br i1 %cmp541, label %if.then543, label %if.end544

if.then543:                                       ; preds = %for.body529
  store i32 0, i32* %h531
  br label %if.end544

if.end544:                                        ; preds = %if.then543, %for.body529
  %tmp545 = load i32* %h531                       ; <i32> [#uses=1]
  %tmp546 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul547 = mul i32 %tmp545, %tmp546              ; <i32> [#uses=1]
  %tmp548 = load i32* %w536                       ; <i32> [#uses=1]
  %add549 = add i32 %mul547, %tmp548              ; <i32> [#uses=1]
  %tmp550 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx551 = getelementptr inbounds <4 x float> addrspace(1)* %tmp550, i32 %add549 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp552 = load <4 x float> addrspace(1)* %arrayidx551 ; <<4 x float>> [#uses=1]
  %tmp553 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add554 = fadd <4 x float> %tmp553, %tmp552     ; <<4 x float>> [#uses=1]
  store <4 x float> %add554, <4 x float>* %colorAccumulator
  br label %for.inc555

for.inc555:                                       ; preds = %if.end544
  %tmp556 = load i32* %j521                       ; <i32> [#uses=1]
  %inc557 = add nsw i32 %tmp556, 1                ; <i32> [#uses=1]
  store i32 %inc557, i32* %j521
  br label %for.cond524

for.end558:                                       ; preds = %for.cond524
  br label %for.inc559

for.inc559:                                       ; preds = %for.end558
  %tmp560 = load i32* %i511                       ; <i32> [#uses=1]
  %inc561 = add nsw i32 %tmp560, 1                ; <i32> [#uses=1]
  store i32 %inc561, i32* %i511
  br label %for.cond514

for.end562:                                       ; preds = %for.cond514
  %tmp563 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp564 = load float* %denominator              ; <float> [#uses=1]
  %tmp565 = insertelement <4 x float> undef, float %tmp564, i32 0 ; <<4 x float>> [#uses=2]
  %splat566 = shufflevector <4 x float> %tmp565, <4 x float> %tmp565, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp567 = fcmp oeq <4 x float> zeroinitializer, %splat566 ; <<4 x i1>> [#uses=1]
  %sel568 = select <4 x i1> %cmp567, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat566 ; <<4 x float>> [#uses=0]
  %div569 = fdiv <4 x float> %tmp563, %splat566   ; <<4 x float>> [#uses=1]
  %tmp570 = load i32* %row488                     ; <i32> [#uses=1]
  %tmp571 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul572 = mul i32 %tmp570, %tmp571              ; <i32> [#uses=1]
  %tmp573 = load i32* %column496                  ; <i32> [#uses=1]
  %add574 = add i32 %mul572, %tmp573              ; <i32> [#uses=1]
  %tmp575 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx576 = getelementptr inbounds <4 x float> addrspace(1)* %tmp575, i32 %add574 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div569, <4 x float> addrspace(1)* %arrayidx576
  br label %for.inc577

for.inc577:                                       ; preds = %for.end562
  %tmp578 = load i32* %column496                  ; <i32> [#uses=1]
  %inc579 = add nsw i32 %tmp578, 1                ; <i32> [#uses=1]
  store i32 %inc579, i32* %column496
  br label %for.cond498

for.end580:                                       ; preds = %for.cond498
  br label %for.inc581

for.inc581:                                       ; preds = %for.end580
  %tmp582 = load i32* %row488                     ; <i32> [#uses=1]
  %inc583 = add nsw i32 %tmp582, 1                ; <i32> [#uses=1]
  store i32 %inc583, i32* %row488
  br label %for.cond489

for.end584:                                       ; preds = %for.cond489
  br label %if.end585

if.end585:                                        ; preds = %for.end584, %if.end483
  %tmp586 = load i8* %topEdge                     ; <i8> [#uses=1]
  %tobool587 = trunc i8 %tmp586 to i1             ; <i1> [#uses=1]
  br i1 %tobool587, label %land.lhs.true589, label %if.end704

land.lhs.true589:                                 ; preds = %if.end585
  %tmp590 = load i8* %rightEdge                   ; <i8> [#uses=1]
  %tobool591 = trunc i8 %tmp590 to i1             ; <i1> [#uses=1]
  br i1 %tobool591, label %if.then593, label %if.end704

if.then593:                                       ; preds = %land.lhs.true589
  store i32 0, i32* %row595
  br label %for.cond596

for.cond596:                                      ; preds = %for.inc700, %if.then593
  %tmp597 = load i32* %row595                     ; <i32> [#uses=1]
  %tmp598 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp599 = icmp slt i32 %tmp597, %tmp598         ; <i1> [#uses=1]
  br i1 %cmp599, label %for.body601, label %for.end703

for.body601:                                      ; preds = %for.cond596
  %tmp604 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp605 = load i32* %count_x                    ; <i32> [#uses=1]
  %add606 = add i32 %tmp604, %tmp605              ; <i32> [#uses=1]
  store i32 %add606, i32* %column603
  br label %for.cond607

for.cond607:                                      ; preds = %for.inc696, %for.body601
  %tmp608 = load i32* %column603                  ; <i32> [#uses=1]
  %tmp609 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp610 = load i32* %count_x                    ; <i32> [#uses=1]
  %add611 = add i32 %tmp609, %tmp610              ; <i32> [#uses=1]
  %tmp612 = load i32* %local_radius               ; <i32> [#uses=1]
  %add613 = add i32 %add611, %tmp612              ; <i32> [#uses=1]
  %cmp614 = icmp ult i32 %tmp608, %add613         ; <i1> [#uses=1]
  br i1 %cmp614, label %for.body616, label %for.end699

for.body616:                                      ; preds = %for.cond607
  %call617 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv618 = sitofp i32 %call617 to float         ; <float> [#uses=1]
  %tmp619 = insertelement <4 x float> undef, float %conv618, i32 0 ; <<4 x float>> [#uses=2]
  %splat620 = shufflevector <4 x float> %tmp619, <4 x float> %tmp619, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat620, <4 x float>* %colorAccumulator
  %tmp623 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg624 = sub i32 0, %tmp623                    ; <i32> [#uses=1]
  store i32 %neg624, i32* %i622
  br label %for.cond625

for.cond625:                                      ; preds = %for.inc678, %for.body616
  %tmp626 = load i32* %i622                       ; <i32> [#uses=1]
  %tmp627 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp628 = icmp sle i32 %tmp626, %tmp627         ; <i1> [#uses=1]
  br i1 %cmp628, label %for.body630, label %for.end681

for.body630:                                      ; preds = %for.cond625
  %tmp633 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg634 = sub i32 0, %tmp633                    ; <i32> [#uses=1]
  store i32 %neg634, i32* %j632
  br label %for.cond635

for.cond635:                                      ; preds = %for.inc674, %for.body630
  %tmp636 = load i32* %j632                       ; <i32> [#uses=1]
  %tmp637 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp638 = icmp sle i32 %tmp636, %tmp637         ; <i1> [#uses=1]
  br i1 %cmp638, label %for.body640, label %for.end677

for.body640:                                      ; preds = %for.cond635
  %tmp643 = load i32* %row595                     ; <i32> [#uses=1]
  %tmp644 = load i32* %i622                       ; <i32> [#uses=1]
  %add645 = add nsw i32 %tmp643, %tmp644          ; <i32> [#uses=1]
  store i32 %add645, i32* %h642
  %tmp648 = load i32* %column603                  ; <i32> [#uses=1]
  %tmp649 = load i32* %j632                       ; <i32> [#uses=1]
  %add650 = add nsw i32 %tmp648, %tmp649          ; <i32> [#uses=1]
  store i32 %add650, i32* %w647
  %tmp651 = load i32* %h642                       ; <i32> [#uses=1]
  %cmp652 = icmp slt i32 %tmp651, 0               ; <i1> [#uses=1]
  br i1 %cmp652, label %if.then654, label %if.end655

if.then654:                                       ; preds = %for.body640
  store i32 0, i32* %h642
  br label %if.end655

if.end655:                                        ; preds = %if.then654, %for.body640
  %tmp656 = load i32* %w647                       ; <i32> [#uses=1]
  %tmp657 = load i32* %width.addr                 ; <i32> [#uses=1]
  %cmp658 = icmp uge i32 %tmp656, %tmp657         ; <i1> [#uses=1]
  br i1 %cmp658, label %if.then660, label %if.end663

if.then660:                                       ; preds = %if.end655
  %tmp661 = load i32* %width.addr                 ; <i32> [#uses=1]
  %sub662 = sub i32 %tmp661, 1                    ; <i32> [#uses=1]
  store i32 %sub662, i32* %w647
  br label %if.end663

if.end663:                                        ; preds = %if.then660, %if.end655
  %tmp664 = load i32* %h642                       ; <i32> [#uses=1]
  %tmp665 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul666 = mul i32 %tmp664, %tmp665              ; <i32> [#uses=1]
  %tmp667 = load i32* %w647                       ; <i32> [#uses=1]
  %add668 = add i32 %mul666, %tmp667              ; <i32> [#uses=1]
  %tmp669 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx670 = getelementptr inbounds <4 x float> addrspace(1)* %tmp669, i32 %add668 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp671 = load <4 x float> addrspace(1)* %arrayidx670 ; <<4 x float>> [#uses=1]
  %tmp672 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add673 = fadd <4 x float> %tmp672, %tmp671     ; <<4 x float>> [#uses=1]
  store <4 x float> %add673, <4 x float>* %colorAccumulator
  br label %for.inc674

for.inc674:                                       ; preds = %if.end663
  %tmp675 = load i32* %j632                       ; <i32> [#uses=1]
  %inc676 = add nsw i32 %tmp675, 1                ; <i32> [#uses=1]
  store i32 %inc676, i32* %j632
  br label %for.cond635

for.end677:                                       ; preds = %for.cond635
  br label %for.inc678

for.inc678:                                       ; preds = %for.end677
  %tmp679 = load i32* %i622                       ; <i32> [#uses=1]
  %inc680 = add nsw i32 %tmp679, 1                ; <i32> [#uses=1]
  store i32 %inc680, i32* %i622
  br label %for.cond625

for.end681:                                       ; preds = %for.cond625
  %tmp682 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp683 = load float* %denominator              ; <float> [#uses=1]
  %tmp684 = insertelement <4 x float> undef, float %tmp683, i32 0 ; <<4 x float>> [#uses=2]
  %splat685 = shufflevector <4 x float> %tmp684, <4 x float> %tmp684, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp686 = fcmp oeq <4 x float> zeroinitializer, %splat685 ; <<4 x i1>> [#uses=1]
  %sel687 = select <4 x i1> %cmp686, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat685 ; <<4 x float>> [#uses=0]
  %div688 = fdiv <4 x float> %tmp682, %splat685   ; <<4 x float>> [#uses=1]
  %tmp689 = load i32* %row595                     ; <i32> [#uses=1]
  %tmp690 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul691 = mul i32 %tmp689, %tmp690              ; <i32> [#uses=1]
  %tmp692 = load i32* %column603                  ; <i32> [#uses=1]
  %add693 = add i32 %mul691, %tmp692              ; <i32> [#uses=1]
  %tmp694 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx695 = getelementptr inbounds <4 x float> addrspace(1)* %tmp694, i32 %add693 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div688, <4 x float> addrspace(1)* %arrayidx695
  br label %for.inc696

for.inc696:                                       ; preds = %for.end681
  %tmp697 = load i32* %column603                  ; <i32> [#uses=1]
  %inc698 = add nsw i32 %tmp697, 1                ; <i32> [#uses=1]
  store i32 %inc698, i32* %column603
  br label %for.cond607

for.end699:                                       ; preds = %for.cond607
  br label %for.inc700

for.inc700:                                       ; preds = %for.end699
  %tmp701 = load i32* %row595                     ; <i32> [#uses=1]
  %inc702 = add nsw i32 %tmp701, 1                ; <i32> [#uses=1]
  store i32 %inc702, i32* %row595
  br label %for.cond596

for.end703:                                       ; preds = %for.cond596
  br label %if.end704

if.end704:                                        ; preds = %for.end703, %land.lhs.true589, %if.end585
  %tmp705 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool706 = trunc i8 %tmp705 to i1             ; <i1> [#uses=1]
  br i1 %tobool706, label %if.then707, label %if.end806

if.then707:                                       ; preds = %if.end704
  %tmp710 = load i32* %index_y                    ; <i32> [#uses=1]
  store i32 %tmp710, i32* %row709
  br label %for.cond711

for.cond711:                                      ; preds = %for.inc802, %if.then707
  %tmp712 = load i32* %row709                     ; <i32> [#uses=1]
  %tmp713 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp714 = load i32* %count_y                    ; <i32> [#uses=1]
  %add715 = add i32 %tmp713, %tmp714              ; <i32> [#uses=1]
  %cmp716 = icmp ult i32 %tmp712, %add715         ; <i1> [#uses=1]
  br i1 %cmp716, label %for.body718, label %for.end805

for.body718:                                      ; preds = %for.cond711
  store i32 0, i32* %column720
  br label %for.cond721

for.cond721:                                      ; preds = %for.inc798, %for.body718
  %tmp722 = load i32* %column720                  ; <i32> [#uses=1]
  %tmp723 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp724 = icmp slt i32 %tmp722, %tmp723         ; <i1> [#uses=1]
  br i1 %cmp724, label %for.body726, label %for.end801

for.body726:                                      ; preds = %for.cond721
  %call727 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv728 = sitofp i32 %call727 to float         ; <float> [#uses=1]
  %tmp729 = insertelement <4 x float> undef, float %conv728, i32 0 ; <<4 x float>> [#uses=2]
  %splat730 = shufflevector <4 x float> %tmp729, <4 x float> %tmp729, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat730, <4 x float>* %colorAccumulator
  %tmp733 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg734 = sub i32 0, %tmp733                    ; <i32> [#uses=1]
  store i32 %neg734, i32* %i732
  br label %for.cond735

for.cond735:                                      ; preds = %for.inc780, %for.body726
  %tmp736 = load i32* %i732                       ; <i32> [#uses=1]
  %tmp737 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp738 = icmp sle i32 %tmp736, %tmp737         ; <i1> [#uses=1]
  br i1 %cmp738, label %for.body740, label %for.end783

for.body740:                                      ; preds = %for.cond735
  %tmp743 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg744 = sub i32 0, %tmp743                    ; <i32> [#uses=1]
  store i32 %neg744, i32* %j742
  br label %for.cond745

for.cond745:                                      ; preds = %for.inc776, %for.body740
  %tmp746 = load i32* %j742                       ; <i32> [#uses=1]
  %tmp747 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp748 = icmp sle i32 %tmp746, %tmp747         ; <i1> [#uses=1]
  br i1 %cmp748, label %for.body750, label %for.end779

for.body750:                                      ; preds = %for.cond745
  %tmp753 = load i32* %row709                     ; <i32> [#uses=1]
  %tmp754 = load i32* %i732                       ; <i32> [#uses=1]
  %add755 = add nsw i32 %tmp753, %tmp754          ; <i32> [#uses=1]
  store i32 %add755, i32* %h752
  %tmp758 = load i32* %column720                  ; <i32> [#uses=1]
  %tmp759 = load i32* %j742                       ; <i32> [#uses=1]
  %add760 = add nsw i32 %tmp758, %tmp759          ; <i32> [#uses=1]
  store i32 %add760, i32* %w757
  %tmp761 = load i32* %w757                       ; <i32> [#uses=1]
  %cmp762 = icmp slt i32 %tmp761, 0               ; <i1> [#uses=1]
  br i1 %cmp762, label %if.then764, label %if.end765

if.then764:                                       ; preds = %for.body750
  store i32 0, i32* %w757
  br label %if.end765

if.end765:                                        ; preds = %if.then764, %for.body750
  %tmp766 = load i32* %h752                       ; <i32> [#uses=1]
  %tmp767 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul768 = mul i32 %tmp766, %tmp767              ; <i32> [#uses=1]
  %tmp769 = load i32* %w757                       ; <i32> [#uses=1]
  %add770 = add i32 %mul768, %tmp769              ; <i32> [#uses=1]
  %tmp771 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx772 = getelementptr inbounds <4 x float> addrspace(1)* %tmp771, i32 %add770 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp773 = load <4 x float> addrspace(1)* %arrayidx772 ; <<4 x float>> [#uses=1]
  %tmp774 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add775 = fadd <4 x float> %tmp774, %tmp773     ; <<4 x float>> [#uses=1]
  store <4 x float> %add775, <4 x float>* %colorAccumulator
  br label %for.inc776

for.inc776:                                       ; preds = %if.end765
  %tmp777 = load i32* %j742                       ; <i32> [#uses=1]
  %inc778 = add nsw i32 %tmp777, 1                ; <i32> [#uses=1]
  store i32 %inc778, i32* %j742
  br label %for.cond745

for.end779:                                       ; preds = %for.cond745
  br label %for.inc780

for.inc780:                                       ; preds = %for.end779
  %tmp781 = load i32* %i732                       ; <i32> [#uses=1]
  %inc782 = add nsw i32 %tmp781, 1                ; <i32> [#uses=1]
  store i32 %inc782, i32* %i732
  br label %for.cond735

for.end783:                                       ; preds = %for.cond735
  %tmp784 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp785 = load float* %denominator              ; <float> [#uses=1]
  %tmp786 = insertelement <4 x float> undef, float %tmp785, i32 0 ; <<4 x float>> [#uses=2]
  %splat787 = shufflevector <4 x float> %tmp786, <4 x float> %tmp786, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp788 = fcmp oeq <4 x float> zeroinitializer, %splat787 ; <<4 x i1>> [#uses=1]
  %sel789 = select <4 x i1> %cmp788, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat787 ; <<4 x float>> [#uses=0]
  %div790 = fdiv <4 x float> %tmp784, %splat787   ; <<4 x float>> [#uses=1]
  %tmp791 = load i32* %row709                     ; <i32> [#uses=1]
  %tmp792 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul793 = mul i32 %tmp791, %tmp792              ; <i32> [#uses=1]
  %tmp794 = load i32* %column720                  ; <i32> [#uses=1]
  %add795 = add i32 %mul793, %tmp794              ; <i32> [#uses=1]
  %tmp796 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx797 = getelementptr inbounds <4 x float> addrspace(1)* %tmp796, i32 %add795 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div790, <4 x float> addrspace(1)* %arrayidx797
  br label %for.inc798

for.inc798:                                       ; preds = %for.end783
  %tmp799 = load i32* %column720                  ; <i32> [#uses=1]
  %inc800 = add nsw i32 %tmp799, 1                ; <i32> [#uses=1]
  store i32 %inc800, i32* %column720
  br label %for.cond721

for.end801:                                       ; preds = %for.cond721
  br label %for.inc802

for.inc802:                                       ; preds = %for.end801
  %tmp803 = load i32* %row709                     ; <i32> [#uses=1]
  %inc804 = add nsw i32 %tmp803, 1                ; <i32> [#uses=1]
  store i32 %inc804, i32* %row709
  br label %for.cond711

for.end805:                                       ; preds = %for.cond711
  br label %if.end806

if.end806:                                        ; preds = %for.end805, %if.end704
  %tmp807 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool808 = trunc i8 %tmp807 to i1             ; <i1> [#uses=1]
  br i1 %tobool808, label %land.lhs.true810, label %if.end925

land.lhs.true810:                                 ; preds = %if.end806
  %tmp811 = load i8* %leftEdge                    ; <i8> [#uses=1]
  %tobool812 = trunc i8 %tmp811 to i1             ; <i1> [#uses=1]
  br i1 %tobool812, label %if.then814, label %if.end925

if.then814:                                       ; preds = %land.lhs.true810
  %tmp817 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp818 = load i32* %count_y                    ; <i32> [#uses=1]
  %add819 = add i32 %tmp817, %tmp818              ; <i32> [#uses=1]
  store i32 %add819, i32* %row816
  br label %for.cond820

for.cond820:                                      ; preds = %for.inc921, %if.then814
  %tmp821 = load i32* %row816                     ; <i32> [#uses=1]
  %tmp822 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp823 = load i32* %count_y                    ; <i32> [#uses=1]
  %add824 = add i32 %tmp822, %tmp823              ; <i32> [#uses=1]
  %tmp825 = load i32* %local_radius               ; <i32> [#uses=1]
  %add826 = add i32 %add824, %tmp825              ; <i32> [#uses=1]
  %cmp827 = icmp ult i32 %tmp821, %add826         ; <i1> [#uses=1]
  br i1 %cmp827, label %for.body829, label %for.end924

for.body829:                                      ; preds = %for.cond820
  store i32 0, i32* %column831
  br label %for.cond832

for.cond832:                                      ; preds = %for.inc917, %for.body829
  %tmp833 = load i32* %column831                  ; <i32> [#uses=1]
  %tmp834 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp835 = icmp slt i32 %tmp833, %tmp834         ; <i1> [#uses=1]
  br i1 %cmp835, label %for.body837, label %for.end920

for.body837:                                      ; preds = %for.cond832
  %call838 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv839 = sitofp i32 %call838 to float         ; <float> [#uses=1]
  %tmp840 = insertelement <4 x float> undef, float %conv839, i32 0 ; <<4 x float>> [#uses=2]
  %splat841 = shufflevector <4 x float> %tmp840, <4 x float> %tmp840, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat841, <4 x float>* %colorAccumulator
  %tmp844 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg845 = sub i32 0, %tmp844                    ; <i32> [#uses=1]
  store i32 %neg845, i32* %i843
  br label %for.cond846

for.cond846:                                      ; preds = %for.inc899, %for.body837
  %tmp847 = load i32* %i843                       ; <i32> [#uses=1]
  %tmp848 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp849 = icmp sle i32 %tmp847, %tmp848         ; <i1> [#uses=1]
  br i1 %cmp849, label %for.body851, label %for.end902

for.body851:                                      ; preds = %for.cond846
  %tmp854 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg855 = sub i32 0, %tmp854                    ; <i32> [#uses=1]
  store i32 %neg855, i32* %j853
  br label %for.cond856

for.cond856:                                      ; preds = %for.inc895, %for.body851
  %tmp857 = load i32* %j853                       ; <i32> [#uses=1]
  %tmp858 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp859 = icmp sle i32 %tmp857, %tmp858         ; <i1> [#uses=1]
  br i1 %cmp859, label %for.body861, label %for.end898

for.body861:                                      ; preds = %for.cond856
  %tmp864 = load i32* %row816                     ; <i32> [#uses=1]
  %tmp865 = load i32* %i843                       ; <i32> [#uses=1]
  %add866 = add nsw i32 %tmp864, %tmp865          ; <i32> [#uses=1]
  store i32 %add866, i32* %h863
  %tmp869 = load i32* %column831                  ; <i32> [#uses=1]
  %tmp870 = load i32* %j853                       ; <i32> [#uses=1]
  %add871 = add nsw i32 %tmp869, %tmp870          ; <i32> [#uses=1]
  store i32 %add871, i32* %w868
  %tmp872 = load i32* %h863                       ; <i32> [#uses=1]
  %tmp873 = load i32* %height.addr                ; <i32> [#uses=1]
  %cmp874 = icmp uge i32 %tmp872, %tmp873         ; <i1> [#uses=1]
  br i1 %cmp874, label %if.then876, label %if.end879

if.then876:                                       ; preds = %for.body861
  %tmp877 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub878 = sub i32 %tmp877, 1                    ; <i32> [#uses=1]
  store i32 %sub878, i32* %h863
  br label %if.end879

if.end879:                                        ; preds = %if.then876, %for.body861
  %tmp880 = load i32* %w868                       ; <i32> [#uses=1]
  %cmp881 = icmp slt i32 %tmp880, 0               ; <i1> [#uses=1]
  br i1 %cmp881, label %if.then883, label %if.end884

if.then883:                                       ; preds = %if.end879
  store i32 0, i32* %w868
  br label %if.end884

if.end884:                                        ; preds = %if.then883, %if.end879
  %tmp885 = load i32* %h863                       ; <i32> [#uses=1]
  %tmp886 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul887 = mul i32 %tmp885, %tmp886              ; <i32> [#uses=1]
  %tmp888 = load i32* %w868                       ; <i32> [#uses=1]
  %add889 = add i32 %mul887, %tmp888              ; <i32> [#uses=1]
  %tmp890 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx891 = getelementptr inbounds <4 x float> addrspace(1)* %tmp890, i32 %add889 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp892 = load <4 x float> addrspace(1)* %arrayidx891 ; <<4 x float>> [#uses=1]
  %tmp893 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %add894 = fadd <4 x float> %tmp893, %tmp892     ; <<4 x float>> [#uses=1]
  store <4 x float> %add894, <4 x float>* %colorAccumulator
  br label %for.inc895

for.inc895:                                       ; preds = %if.end884
  %tmp896 = load i32* %j853                       ; <i32> [#uses=1]
  %inc897 = add nsw i32 %tmp896, 1                ; <i32> [#uses=1]
  store i32 %inc897, i32* %j853
  br label %for.cond856

for.end898:                                       ; preds = %for.cond856
  br label %for.inc899

for.inc899:                                       ; preds = %for.end898
  %tmp900 = load i32* %i843                       ; <i32> [#uses=1]
  %inc901 = add nsw i32 %tmp900, 1                ; <i32> [#uses=1]
  store i32 %inc901, i32* %i843
  br label %for.cond846

for.end902:                                       ; preds = %for.cond846
  %tmp903 = load <4 x float>* %colorAccumulator   ; <<4 x float>> [#uses=1]
  %tmp904 = load float* %denominator              ; <float> [#uses=1]
  %tmp905 = insertelement <4 x float> undef, float %tmp904, i32 0 ; <<4 x float>> [#uses=2]
  %splat906 = shufflevector <4 x float> %tmp905, <4 x float> %tmp905, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp907 = fcmp oeq <4 x float> zeroinitializer, %splat906 ; <<4 x i1>> [#uses=1]
  %sel908 = select <4 x i1> %cmp907, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat906 ; <<4 x float>> [#uses=0]
  %div909 = fdiv <4 x float> %tmp903, %splat906   ; <<4 x float>> [#uses=1]
  %tmp910 = load i32* %row816                     ; <i32> [#uses=1]
  %tmp911 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul912 = mul i32 %tmp910, %tmp911              ; <i32> [#uses=1]
  %tmp913 = load i32* %column831                  ; <i32> [#uses=1]
  %add914 = add i32 %mul912, %tmp913              ; <i32> [#uses=1]
  %tmp915 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx916 = getelementptr inbounds <4 x float> addrspace(1)* %tmp915, i32 %add914 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div909, <4 x float> addrspace(1)* %arrayidx916
  br label %for.inc917

for.inc917:                                       ; preds = %for.end902
  %tmp918 = load i32* %column831                  ; <i32> [#uses=1]
  %inc919 = add nsw i32 %tmp918, 1                ; <i32> [#uses=1]
  store i32 %inc919, i32* %column831
  br label %for.cond832

for.end920:                                       ; preds = %for.cond832
  br label %for.inc921

for.inc921:                                       ; preds = %for.end920
  %tmp922 = load i32* %row816                     ; <i32> [#uses=1]
  %inc923 = add nsw i32 %tmp922, 1                ; <i32> [#uses=1]
  store i32 %inc923, i32* %row816
  br label %for.cond820

for.end924:                                       ; preds = %for.cond820
  br label %if.end925

if.end925:                                        ; preds = %for.end924, %land.lhs.true810, %if.end806
  %tmp926 = load i8* %bottomEdge                  ; <i8> [#uses=1]
  %tobool927 = trunc i8 %tmp926 to i1             ; <i1> [#uses=1]
  br i1 %tobool927, label %if.then928, label %if.end1037

if.then928:                                       ; preds = %if.end925
  %tmp931 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp932 = load i32* %count_y                    ; <i32> [#uses=1]
  %add933 = add i32 %tmp931, %tmp932              ; <i32> [#uses=1]
  store i32 %add933, i32* %row930
  br label %for.cond934

for.cond934:                                      ; preds = %for.inc1033, %if.then928
  %tmp935 = load i32* %row930                     ; <i32> [#uses=1]
  %tmp936 = load i32* %index_y                    ; <i32> [#uses=1]
  %tmp937 = load i32* %count_y                    ; <i32> [#uses=1]
  %add938 = add i32 %tmp936, %tmp937              ; <i32> [#uses=1]
  %tmp939 = load i32* %local_radius               ; <i32> [#uses=1]
  %add940 = add i32 %add938, %tmp939              ; <i32> [#uses=1]
  %cmp941 = icmp ult i32 %tmp935, %add940         ; <i1> [#uses=1]
  br i1 %cmp941, label %for.body943, label %for.end1036

for.body943:                                      ; preds = %for.cond934
  %tmp946 = load i32* %index_x                    ; <i32> [#uses=1]
  store i32 %tmp946, i32* %column945
  br label %for.cond947

for.cond947:                                      ; preds = %for.inc1029, %for.body943
  %tmp948 = load i32* %column945                  ; <i32> [#uses=1]
  %tmp949 = load i32* %index_x                    ; <i32> [#uses=1]
  %tmp950 = load i32* %count_x                    ; <i32> [#uses=1]
  %add951 = add i32 %tmp949, %tmp950              ; <i32> [#uses=1]
  %cmp952 = icmp ult i32 %tmp948, %add951         ; <i1> [#uses=1]
  br i1 %cmp952, label %for.body954, label %for.end1032

for.body954:                                      ; preds = %for.cond947
  %call955 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv956 = sitofp i32 %call955 to float         ; <float> [#uses=1]
  %tmp957 = insertelement <4 x float> undef, float %conv956, i32 0 ; <<4 x float>> [#uses=2]
  %splat958 = shufflevector <4 x float> %tmp957, <4 x float> %tmp957, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat958, <4 x float>* %colorAccumulator
  %tmp961 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg962 = sub i32 0, %tmp961                    ; <i32> [#uses=1]
  store i32 %neg962, i32* %i960
  br label %for.cond963

for.cond963:                                      ; preds = %for.inc1011, %for.body954
  %tmp964 = load i32* %i960                       ; <i32> [#uses=1]
  %tmp965 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp966 = icmp sle i32 %tmp964, %tmp965         ; <i1> [#uses=1]
  br i1 %cmp966, label %for.body968, label %for.end1014

for.body968:                                      ; preds = %for.cond963
  %tmp971 = load i32* %local_radius               ; <i32> [#uses=1]
  %neg972 = sub i32 0, %tmp971                    ; <i32> [#uses=1]
  store i32 %neg972, i32* %j970
  br label %for.cond973

for.cond973:                                      ; preds = %for.inc1007, %for.body968
  %tmp974 = load i32* %j970                       ; <i32> [#uses=1]
  %tmp975 = load i32* %local_radius               ; <i32> [#uses=1]
  %cmp976 = icmp sle i32 %tmp974, %tmp975         ; <i1> [#uses=1]
  br i1 %cmp976, label %for.body978, label %for.end1010

for.body978:                                      ; preds = %for.cond973
  %tmp981 = load i32* %row930                     ; <i32> [#uses=1]
  %tmp982 = load i32* %i960                       ; <i32> [#uses=1]
  %add983 = add nsw i32 %tmp981, %tmp982          ; <i32> [#uses=1]
  store i32 %add983, i32* %h980
  %tmp986 = load i32* %column945                  ; <i32> [#uses=1]
  %tmp987 = load i32* %j970                       ; <i32> [#uses=1]
  %add988 = add nsw i32 %tmp986, %tmp987          ; <i32> [#uses=1]
  store i32 %add988, i32* %w985
  %tmp989 = load i32* %h980                       ; <i32> [#uses=1]
  %tmp990 = load i32* %height.addr                ; <i32> [#uses=1]
  %cmp991 = icmp uge i32 %tmp989, %tmp990         ; <i1> [#uses=1]
  br i1 %cmp991, label %if.then993, label %if.end996

if.then993:                                       ; preds = %for.body978
  %tmp994 = load i32* %height.addr                ; <i32> [#uses=1]
  %sub995 = sub i32 %tmp994, 1                    ; <i32> [#uses=1]
  store i32 %sub995, i32* %h980
  br label %if.end996

if.end996:                                        ; preds = %if.then993, %for.body978
  %tmp997 = load i32* %h980                       ; <i32> [#uses=1]
  %tmp998 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul999 = mul i32 %tmp997, %tmp998              ; <i32> [#uses=1]
  %tmp1000 = load i32* %w985                      ; <i32> [#uses=1]
  %add1001 = add i32 %mul999, %tmp1000            ; <i32> [#uses=1]
  %tmp1002 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1003 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1002, i32 %add1001 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1004 = load <4 x float> addrspace(1)* %arrayidx1003 ; <<4 x float>> [#uses=1]
  %tmp1005 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %add1006 = fadd <4 x float> %tmp1005, %tmp1004  ; <<4 x float>> [#uses=1]
  store <4 x float> %add1006, <4 x float>* %colorAccumulator
  br label %for.inc1007

for.inc1007:                                      ; preds = %if.end996
  %tmp1008 = load i32* %j970                      ; <i32> [#uses=1]
  %inc1009 = add nsw i32 %tmp1008, 1              ; <i32> [#uses=1]
  store i32 %inc1009, i32* %j970
  br label %for.cond973

for.end1010:                                      ; preds = %for.cond973
  br label %for.inc1011

for.inc1011:                                      ; preds = %for.end1010
  %tmp1012 = load i32* %i960                      ; <i32> [#uses=1]
  %inc1013 = add nsw i32 %tmp1012, 1              ; <i32> [#uses=1]
  store i32 %inc1013, i32* %i960
  br label %for.cond963

for.end1014:                                      ; preds = %for.cond963
  %tmp1015 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %tmp1016 = load float* %denominator             ; <float> [#uses=1]
  %tmp1017 = insertelement <4 x float> undef, float %tmp1016, i32 0 ; <<4 x float>> [#uses=2]
  %splat1018 = shufflevector <4 x float> %tmp1017, <4 x float> %tmp1017, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp1019 = fcmp oeq <4 x float> zeroinitializer, %splat1018 ; <<4 x i1>> [#uses=1]
  %sel1020 = select <4 x i1> %cmp1019, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat1018 ; <<4 x float>> [#uses=0]
  %div1021 = fdiv <4 x float> %tmp1015, %splat1018 ; <<4 x float>> [#uses=1]
  %tmp1022 = load i32* %row930                    ; <i32> [#uses=1]
  %tmp1023 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1024 = mul i32 %tmp1022, %tmp1023           ; <i32> [#uses=1]
  %tmp1025 = load i32* %column945                 ; <i32> [#uses=1]
  %add1026 = add i32 %mul1024, %tmp1025           ; <i32> [#uses=1]
  %tmp1027 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1028 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1027, i32 %add1026 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div1021, <4 x float> addrspace(1)* %arrayidx1028
  br label %for.inc1029

for.inc1029:                                      ; preds = %for.end1014
  %tmp1030 = load i32* %column945                 ; <i32> [#uses=1]
  %inc1031 = add nsw i32 %tmp1030, 1              ; <i32> [#uses=1]
  store i32 %inc1031, i32* %column945
  br label %for.cond947

for.end1032:                                      ; preds = %for.cond947
  br label %for.inc1033

for.inc1033:                                      ; preds = %for.end1032
  %tmp1034 = load i32* %row930                    ; <i32> [#uses=1]
  %inc1035 = add nsw i32 %tmp1034, 1              ; <i32> [#uses=1]
  store i32 %inc1035, i32* %row930
  br label %for.cond934

for.end1036:                                      ; preds = %for.cond934
  br label %if.end1037

if.end1037:                                       ; preds = %for.end1036, %if.end925
  %tmp1038 = load i8* %rightEdge                  ; <i8> [#uses=1]
  %tobool1039 = trunc i8 %tmp1038 to i1           ; <i1> [#uses=1]
  br i1 %tobool1039, label %if.then1040, label %if.end1149

if.then1040:                                      ; preds = %if.end1037
  %tmp1043 = load i32* %index_y                   ; <i32> [#uses=1]
  store i32 %tmp1043, i32* %row1042
  br label %for.cond1044

for.cond1044:                                     ; preds = %for.inc1145, %if.then1040
  %tmp1045 = load i32* %row1042                   ; <i32> [#uses=1]
  %tmp1046 = load i32* %index_y                   ; <i32> [#uses=1]
  %tmp1047 = load i32* %count_y                   ; <i32> [#uses=1]
  %add1048 = add i32 %tmp1046, %tmp1047           ; <i32> [#uses=1]
  %cmp1049 = icmp ult i32 %tmp1045, %add1048      ; <i1> [#uses=1]
  br i1 %cmp1049, label %for.body1051, label %for.end1148

for.body1051:                                     ; preds = %for.cond1044
  %tmp1054 = load i32* %index_x                   ; <i32> [#uses=1]
  %tmp1055 = load i32* %count_x                   ; <i32> [#uses=1]
  %add1056 = add i32 %tmp1054, %tmp1055           ; <i32> [#uses=1]
  store i32 %add1056, i32* %column1053
  br label %for.cond1057

for.cond1057:                                     ; preds = %for.inc1141, %for.body1051
  %tmp1058 = load i32* %column1053                ; <i32> [#uses=1]
  %tmp1059 = load i32* %index_x                   ; <i32> [#uses=1]
  %tmp1060 = load i32* %count_x                   ; <i32> [#uses=1]
  %add1061 = add i32 %tmp1059, %tmp1060           ; <i32> [#uses=1]
  %tmp1062 = load i32* %local_radius              ; <i32> [#uses=1]
  %add1063 = add i32 %add1061, %tmp1062           ; <i32> [#uses=1]
  %cmp1064 = icmp ult i32 %tmp1058, %add1063      ; <i1> [#uses=1]
  br i1 %cmp1064, label %for.body1066, label %for.end1144

for.body1066:                                     ; preds = %for.cond1057
  %call1067 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv1068 = sitofp i32 %call1067 to float       ; <float> [#uses=1]
  %tmp1069 = insertelement <4 x float> undef, float %conv1068, i32 0 ; <<4 x float>> [#uses=2]
  %splat1070 = shufflevector <4 x float> %tmp1069, <4 x float> %tmp1069, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat1070, <4 x float>* %colorAccumulator
  %tmp1073 = load i32* %local_radius              ; <i32> [#uses=1]
  %neg1074 = sub i32 0, %tmp1073                  ; <i32> [#uses=1]
  store i32 %neg1074, i32* %i1072
  br label %for.cond1075

for.cond1075:                                     ; preds = %for.inc1123, %for.body1066
  %tmp1076 = load i32* %i1072                     ; <i32> [#uses=1]
  %tmp1077 = load i32* %local_radius              ; <i32> [#uses=1]
  %cmp1078 = icmp sle i32 %tmp1076, %tmp1077      ; <i1> [#uses=1]
  br i1 %cmp1078, label %for.body1080, label %for.end1126

for.body1080:                                     ; preds = %for.cond1075
  %tmp1083 = load i32* %local_radius              ; <i32> [#uses=1]
  %neg1084 = sub i32 0, %tmp1083                  ; <i32> [#uses=1]
  store i32 %neg1084, i32* %j1082
  br label %for.cond1085

for.cond1085:                                     ; preds = %for.inc1119, %for.body1080
  %tmp1086 = load i32* %j1082                     ; <i32> [#uses=1]
  %tmp1087 = load i32* %local_radius              ; <i32> [#uses=1]
  %cmp1088 = icmp sle i32 %tmp1086, %tmp1087      ; <i1> [#uses=1]
  br i1 %cmp1088, label %for.body1090, label %for.end1122

for.body1090:                                     ; preds = %for.cond1085
  %tmp1093 = load i32* %row1042                   ; <i32> [#uses=1]
  %tmp1094 = load i32* %i1072                     ; <i32> [#uses=1]
  %add1095 = add nsw i32 %tmp1093, %tmp1094       ; <i32> [#uses=1]
  store i32 %add1095, i32* %h1092
  %tmp1098 = load i32* %column1053                ; <i32> [#uses=1]
  %tmp1099 = load i32* %j1082                     ; <i32> [#uses=1]
  %add1100 = add nsw i32 %tmp1098, %tmp1099       ; <i32> [#uses=1]
  store i32 %add1100, i32* %w1097
  %tmp1101 = load i32* %w1097                     ; <i32> [#uses=1]
  %tmp1102 = load i32* %width.addr                ; <i32> [#uses=1]
  %cmp1103 = icmp uge i32 %tmp1101, %tmp1102      ; <i1> [#uses=1]
  br i1 %cmp1103, label %if.then1105, label %if.end1108

if.then1105:                                      ; preds = %for.body1090
  %tmp1106 = load i32* %width.addr                ; <i32> [#uses=1]
  %sub1107 = sub i32 %tmp1106, 1                  ; <i32> [#uses=1]
  store i32 %sub1107, i32* %w1097
  br label %if.end1108

if.end1108:                                       ; preds = %if.then1105, %for.body1090
  %tmp1109 = load i32* %h1092                     ; <i32> [#uses=1]
  %tmp1110 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1111 = mul i32 %tmp1109, %tmp1110           ; <i32> [#uses=1]
  %tmp1112 = load i32* %w1097                     ; <i32> [#uses=1]
  %add1113 = add i32 %mul1111, %tmp1112           ; <i32> [#uses=1]
  %tmp1114 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1115 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1114, i32 %add1113 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1116 = load <4 x float> addrspace(1)* %arrayidx1115 ; <<4 x float>> [#uses=1]
  %tmp1117 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %add1118 = fadd <4 x float> %tmp1117, %tmp1116  ; <<4 x float>> [#uses=1]
  store <4 x float> %add1118, <4 x float>* %colorAccumulator
  br label %for.inc1119

for.inc1119:                                      ; preds = %if.end1108
  %tmp1120 = load i32* %j1082                     ; <i32> [#uses=1]
  %inc1121 = add nsw i32 %tmp1120, 1              ; <i32> [#uses=1]
  store i32 %inc1121, i32* %j1082
  br label %for.cond1085

for.end1122:                                      ; preds = %for.cond1085
  br label %for.inc1123

for.inc1123:                                      ; preds = %for.end1122
  %tmp1124 = load i32* %i1072                     ; <i32> [#uses=1]
  %inc1125 = add nsw i32 %tmp1124, 1              ; <i32> [#uses=1]
  store i32 %inc1125, i32* %i1072
  br label %for.cond1075

for.end1126:                                      ; preds = %for.cond1075
  %tmp1127 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %tmp1128 = load float* %denominator             ; <float> [#uses=1]
  %tmp1129 = insertelement <4 x float> undef, float %tmp1128, i32 0 ; <<4 x float>> [#uses=2]
  %splat1130 = shufflevector <4 x float> %tmp1129, <4 x float> %tmp1129, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp1131 = fcmp oeq <4 x float> zeroinitializer, %splat1130 ; <<4 x i1>> [#uses=1]
  %sel1132 = select <4 x i1> %cmp1131, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat1130 ; <<4 x float>> [#uses=0]
  %div1133 = fdiv <4 x float> %tmp1127, %splat1130 ; <<4 x float>> [#uses=1]
  %tmp1134 = load i32* %row1042                   ; <i32> [#uses=1]
  %tmp1135 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1136 = mul i32 %tmp1134, %tmp1135           ; <i32> [#uses=1]
  %tmp1137 = load i32* %column1053                ; <i32> [#uses=1]
  %add1138 = add i32 %mul1136, %tmp1137           ; <i32> [#uses=1]
  %tmp1139 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1140 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1139, i32 %add1138 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div1133, <4 x float> addrspace(1)* %arrayidx1140
  br label %for.inc1141

for.inc1141:                                      ; preds = %for.end1126
  %tmp1142 = load i32* %column1053                ; <i32> [#uses=1]
  %inc1143 = add nsw i32 %tmp1142, 1              ; <i32> [#uses=1]
  store i32 %inc1143, i32* %column1053
  br label %for.cond1057

for.end1144:                                      ; preds = %for.cond1057
  br label %for.inc1145

for.inc1145:                                      ; preds = %for.end1144
  %tmp1146 = load i32* %row1042                   ; <i32> [#uses=1]
  %inc1147 = add nsw i32 %tmp1146, 1              ; <i32> [#uses=1]
  store i32 %inc1147, i32* %row1042
  br label %for.cond1044

for.end1148:                                      ; preds = %for.cond1044
  br label %if.end1149

if.end1149:                                       ; preds = %for.end1148, %if.end1037
  %tmp1150 = load i8* %bottomEdge                 ; <i8> [#uses=1]
  %tobool1151 = trunc i8 %tmp1150 to i1           ; <i1> [#uses=1]
  br i1 %tobool1151, label %land.lhs.true1153, label %if.end1278

land.lhs.true1153:                                ; preds = %if.end1149
  %tmp1154 = load i8* %rightEdge                  ; <i8> [#uses=1]
  %tobool1155 = trunc i8 %tmp1154 to i1           ; <i1> [#uses=1]
  br i1 %tobool1155, label %if.then1157, label %if.end1278

if.then1157:                                      ; preds = %land.lhs.true1153
  %tmp1160 = load i32* %index_y                   ; <i32> [#uses=1]
  %tmp1161 = load i32* %count_y                   ; <i32> [#uses=1]
  %add1162 = add i32 %tmp1160, %tmp1161           ; <i32> [#uses=1]
  store i32 %add1162, i32* %row1159
  br label %for.cond1163

for.cond1163:                                     ; preds = %for.inc1274, %if.then1157
  %tmp1164 = load i32* %row1159                   ; <i32> [#uses=1]
  %tmp1165 = load i32* %index_y                   ; <i32> [#uses=1]
  %tmp1166 = load i32* %count_y                   ; <i32> [#uses=1]
  %add1167 = add i32 %tmp1165, %tmp1166           ; <i32> [#uses=1]
  %tmp1168 = load i32* %local_radius              ; <i32> [#uses=1]
  %add1169 = add i32 %add1167, %tmp1168           ; <i32> [#uses=1]
  %cmp1170 = icmp ult i32 %tmp1164, %add1169      ; <i1> [#uses=1]
  br i1 %cmp1170, label %for.body1172, label %for.end1277

for.body1172:                                     ; preds = %for.cond1163
  %tmp1175 = load i32* %index_x                   ; <i32> [#uses=1]
  %tmp1176 = load i32* %count_x                   ; <i32> [#uses=1]
  %add1177 = add i32 %tmp1175, %tmp1176           ; <i32> [#uses=1]
  store i32 %add1177, i32* %column1174
  br label %for.cond1178

for.cond1178:                                     ; preds = %for.inc1270, %for.body1172
  %tmp1179 = load i32* %column1174                ; <i32> [#uses=1]
  %tmp1180 = load i32* %index_x                   ; <i32> [#uses=1]
  %tmp1181 = load i32* %count_x                   ; <i32> [#uses=1]
  %add1182 = add i32 %tmp1180, %tmp1181           ; <i32> [#uses=1]
  %tmp1183 = load i32* %local_radius              ; <i32> [#uses=1]
  %add1184 = add i32 %add1182, %tmp1183           ; <i32> [#uses=1]
  %cmp1185 = icmp ult i32 %tmp1179, %add1184      ; <i1> [#uses=1]
  br i1 %cmp1185, label %for.body1187, label %for.end1273

for.body1187:                                     ; preds = %for.cond1178
  %call1188 = call i32 (...)* @make_float4(double 0.000000e+000, double 0.000000e+000, double 0.000000e+000, double 0.000000e+000) ; <i32> [#uses=1]
  %conv1189 = sitofp i32 %call1188 to float       ; <float> [#uses=1]
  %tmp1190 = insertelement <4 x float> undef, float %conv1189, i32 0 ; <<4 x float>> [#uses=2]
  %splat1191 = shufflevector <4 x float> %tmp1190, <4 x float> %tmp1190, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat1191, <4 x float>* %colorAccumulator
  %tmp1194 = load i32* %local_radius              ; <i32> [#uses=1]
  %neg1195 = sub i32 0, %tmp1194                  ; <i32> [#uses=1]
  store i32 %neg1195, i32* %i1193
  br label %for.cond1196

for.cond1196:                                     ; preds = %for.inc1252, %for.body1187
  %tmp1197 = load i32* %i1193                     ; <i32> [#uses=1]
  %tmp1198 = load i32* %local_radius              ; <i32> [#uses=1]
  %cmp1199 = icmp sle i32 %tmp1197, %tmp1198      ; <i1> [#uses=1]
  br i1 %cmp1199, label %for.body1201, label %for.end1255

for.body1201:                                     ; preds = %for.cond1196
  %tmp1204 = load i32* %local_radius              ; <i32> [#uses=1]
  %neg1205 = sub i32 0, %tmp1204                  ; <i32> [#uses=1]
  store i32 %neg1205, i32* %j1203
  br label %for.cond1206

for.cond1206:                                     ; preds = %for.inc1248, %for.body1201
  %tmp1207 = load i32* %j1203                     ; <i32> [#uses=1]
  %tmp1208 = load i32* %local_radius              ; <i32> [#uses=1]
  %cmp1209 = icmp sle i32 %tmp1207, %tmp1208      ; <i1> [#uses=1]
  br i1 %cmp1209, label %for.body1211, label %for.end1251

for.body1211:                                     ; preds = %for.cond1206
  %tmp1214 = load i32* %row1159                   ; <i32> [#uses=1]
  %tmp1215 = load i32* %i1193                     ; <i32> [#uses=1]
  %add1216 = add nsw i32 %tmp1214, %tmp1215       ; <i32> [#uses=1]
  store i32 %add1216, i32* %h1213
  %tmp1219 = load i32* %column1174                ; <i32> [#uses=1]
  %tmp1220 = load i32* %j1203                     ; <i32> [#uses=1]
  %add1221 = add nsw i32 %tmp1219, %tmp1220       ; <i32> [#uses=1]
  store i32 %add1221, i32* %w1218
  %tmp1222 = load i32* %h1213                     ; <i32> [#uses=1]
  %tmp1223 = load i32* %height.addr               ; <i32> [#uses=1]
  %cmp1224 = icmp uge i32 %tmp1222, %tmp1223      ; <i1> [#uses=1]
  br i1 %cmp1224, label %if.then1226, label %if.end1229

if.then1226:                                      ; preds = %for.body1211
  %tmp1227 = load i32* %height.addr               ; <i32> [#uses=1]
  %sub1228 = sub i32 %tmp1227, 1                  ; <i32> [#uses=1]
  store i32 %sub1228, i32* %h1213
  br label %if.end1229

if.end1229:                                       ; preds = %if.then1226, %for.body1211
  %tmp1230 = load i32* %w1218                     ; <i32> [#uses=1]
  %tmp1231 = load i32* %width.addr                ; <i32> [#uses=1]
  %cmp1232 = icmp uge i32 %tmp1230, %tmp1231      ; <i1> [#uses=1]
  br i1 %cmp1232, label %if.then1234, label %if.end1237

if.then1234:                                      ; preds = %if.end1229
  %tmp1235 = load i32* %width.addr                ; <i32> [#uses=1]
  %sub1236 = sub i32 %tmp1235, 1                  ; <i32> [#uses=1]
  store i32 %sub1236, i32* %w1218
  br label %if.end1237

if.end1237:                                       ; preds = %if.then1234, %if.end1229
  %tmp1238 = load i32* %h1213                     ; <i32> [#uses=1]
  %tmp1239 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1240 = mul i32 %tmp1238, %tmp1239           ; <i32> [#uses=1]
  %tmp1241 = load i32* %w1218                     ; <i32> [#uses=1]
  %add1242 = add i32 %mul1240, %tmp1241           ; <i32> [#uses=1]
  %tmp1243 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1244 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1243, i32 %add1242 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp1245 = load <4 x float> addrspace(1)* %arrayidx1244 ; <<4 x float>> [#uses=1]
  %tmp1246 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %add1247 = fadd <4 x float> %tmp1246, %tmp1245  ; <<4 x float>> [#uses=1]
  store <4 x float> %add1247, <4 x float>* %colorAccumulator
  br label %for.inc1248

for.inc1248:                                      ; preds = %if.end1237
  %tmp1249 = load i32* %j1203                     ; <i32> [#uses=1]
  %inc1250 = add nsw i32 %tmp1249, 1              ; <i32> [#uses=1]
  store i32 %inc1250, i32* %j1203
  br label %for.cond1206

for.end1251:                                      ; preds = %for.cond1206
  br label %for.inc1252

for.inc1252:                                      ; preds = %for.end1251
  %tmp1253 = load i32* %i1193                     ; <i32> [#uses=1]
  %inc1254 = add nsw i32 %tmp1253, 1              ; <i32> [#uses=1]
  store i32 %inc1254, i32* %i1193
  br label %for.cond1196

for.end1255:                                      ; preds = %for.cond1196
  %tmp1256 = load <4 x float>* %colorAccumulator  ; <<4 x float>> [#uses=1]
  %tmp1257 = load float* %denominator             ; <float> [#uses=1]
  %tmp1258 = insertelement <4 x float> undef, float %tmp1257, i32 0 ; <<4 x float>> [#uses=2]
  %splat1259 = shufflevector <4 x float> %tmp1258, <4 x float> %tmp1258, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp1260 = fcmp oeq <4 x float> zeroinitializer, %splat1259 ; <<4 x i1>> [#uses=1]
  %sel1261 = select <4 x i1> %cmp1260, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat1259 ; <<4 x float>> [#uses=0]
  %div1262 = fdiv <4 x float> %tmp1256, %splat1259 ; <<4 x float>> [#uses=1]
  %tmp1263 = load i32* %row1159                   ; <i32> [#uses=1]
  %tmp1264 = load i32* %width.addr                ; <i32> [#uses=1]
  %mul1265 = mul i32 %tmp1263, %tmp1264           ; <i32> [#uses=1]
  %tmp1266 = load i32* %column1174                ; <i32> [#uses=1]
  %add1267 = add i32 %mul1265, %tmp1266           ; <i32> [#uses=1]
  %tmp1268 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx1269 = getelementptr inbounds <4 x float> addrspace(1)* %tmp1268, i32 %add1267 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %div1262, <4 x float> addrspace(1)* %arrayidx1269
  br label %for.inc1270

for.inc1270:                                      ; preds = %for.end1255
  %tmp1271 = load i32* %column1174                ; <i32> [#uses=1]
  %inc1272 = add nsw i32 %tmp1271, 1              ; <i32> [#uses=1]
  store i32 %inc1272, i32* %column1174
  br label %for.cond1178

for.end1273:                                      ; preds = %for.cond1178
  br label %for.inc1274

for.inc1274:                                      ; preds = %for.end1273
  %tmp1275 = load i32* %row1159                   ; <i32> [#uses=1]
  %inc1276 = add nsw i32 %tmp1275, 1              ; <i32> [#uses=1]
  store i32 %inc1276, i32* %row1159
  br label %for.cond1163

for.end1277:                                      ; preds = %for.cond1163
  br label %if.end1278

if.end1278:                                       ; preds = %for.end1277, %land.lhs.true1153, %if.end1149
  ret void
}

declare i32 @get_work_dim(...)

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare float @_Z4ceilf(float)

declare i32 @make_float4(...)
