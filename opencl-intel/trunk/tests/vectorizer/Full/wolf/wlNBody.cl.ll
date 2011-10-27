; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlNBody.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_nBodyVecKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_nBodyVecKernel_parameters = appending global [628 x i8] c"float4 const __attribute__((address_space(1))) *, float4 const __attribute__((address_space(1))) *, float4 const __attribute__((address_space(1))) *, float4 const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, float, float\00", section "llvm.metadata" ; <[628 x i8]*> [#uses=1]
@opencl_nBodyScalarKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_nBodyScalarKernel_parameters = appending global [624 x i8] c"float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, float, float\00", section "llvm.metadata" ; <[624 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, float, float)* @nBodyVecKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_nBodyVecKernel_locals to i8*), i8* getelementptr inbounds ([628 x i8]* @opencl_nBodyVecKernel_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, float, float)* @nBodyScalarKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_nBodyScalarKernel_locals to i8*), i8* getelementptr inbounds ([624 x i8]* @opencl_nBodyScalarKernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @nBodyVecKernel(<4 x float> addrspace(1)* %input_position_x, <4 x float> addrspace(1)* %input_position_y, <4 x float> addrspace(1)* %input_position_z, <4 x float> addrspace(1)* %mass, float addrspace(1)* %input_velocity_x, float addrspace(1)* %input_velocity_y, float addrspace(1)* %input_velocity_z, float addrspace(1)* %output_position_x, float addrspace(1)* %output_position_y, float addrspace(1)* %output_position_z, float addrspace(1)* %output_velocity_x, float addrspace(1)* %output_velocity_y, float addrspace(1)* %output_velocity_z, i32 %body_count, i32 %body_count_per_item, float %softening_squared, float %time_delta) nounwind {
entry:
  %input_position_x.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %input_position_y.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %input_position_z.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=6]
  %mass.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=5]
  %input_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %output_position_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %body_count.addr = alloca i32, align 4          ; <i32*> [#uses=3]
  %body_count_per_item.addr = alloca i32, align 4 ; <i32*> [#uses=2]
  %softening_squared.addr = alloca float, align 4 ; <float*> [#uses=5]
  %time_delta.addr = alloca float, align 4        ; <float*> [#uses=13]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %in_position_x = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=6]
  %in_position_y = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=6]
  %in_position_z = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=6]
  %position_x = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=5]
  %position_y = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=5]
  %position_z = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=5]
  %m = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=0]
  %current_x1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_y1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_z1 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_mass1 = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=0]
  %current_x2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_y2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_z2 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %current_mass2 = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=0]
  %velocity_x = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %velocity_y = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %velocity_z = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=0]
  %zero = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=10]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=23]
  %j = alloca i32, align 4                        ; <i32*> [#uses=0]
  %k = alloca i32, align 4                        ; <i32*> [#uses=31]
  %l = alloca i32, align 4                        ; <i32*> [#uses=0]
  %inner_loop_count = alloca i32, align 4         ; <i32*> [#uses=2]
  %outer_loop_count = alloca i32, align 4         ; <i32*> [#uses=3]
  %start = alloca i32, align 4                    ; <i32*> [#uses=3]
  %finish = alloca i32, align 4                   ; <i32*> [#uses=2]
  %.compoundliteral40 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral60 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral82 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %acceleration_x1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=11]
  %acceleration_x2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %acceleration_x3 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %acceleration_y1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=11]
  %acceleration_y2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %acceleration_y3 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %acceleration_z1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=11]
  %acceleration_z2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %acceleration_z3 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %dx1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dx2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dx3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dy1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dy2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dy3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dz1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dz2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %dz3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %distance_squared1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %distance_squared2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %distance_squared3 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %inverse_distance1 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %inverse_distance2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %inverse_distance3 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %mi1 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %mi2 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %mi3 = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %s1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %s2 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %s3 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=4]
  %dx1349 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  %dy1357 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  %dz1365 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  %distance_squared1373 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %inverse_distance1391 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=4]
  %mi1395 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %s1401 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=4]
  %acc_x = alloca float, align 4                  ; <float*> [#uses=3]
  %acc_y = alloca float, align 4                  ; <float*> [#uses=3]
  %acc_z = alloca float, align 4                  ; <float*> [#uses=3]
  store <4 x float> addrspace(1)* %input_position_x, <4 x float> addrspace(1)** %input_position_x.addr
  store <4 x float> addrspace(1)* %input_position_y, <4 x float> addrspace(1)** %input_position_y.addr
  store <4 x float> addrspace(1)* %input_position_z, <4 x float> addrspace(1)** %input_position_z.addr
  store <4 x float> addrspace(1)* %mass, <4 x float> addrspace(1)** %mass.addr
  store float addrspace(1)* %input_velocity_x, float addrspace(1)** %input_velocity_x.addr
  store float addrspace(1)* %input_velocity_y, float addrspace(1)** %input_velocity_y.addr
  store float addrspace(1)* %input_velocity_z, float addrspace(1)** %input_velocity_z.addr
  store float addrspace(1)* %output_position_x, float addrspace(1)** %output_position_x.addr
  store float addrspace(1)* %output_position_y, float addrspace(1)** %output_position_y.addr
  store float addrspace(1)* %output_position_z, float addrspace(1)** %output_position_z.addr
  store float addrspace(1)* %output_velocity_x, float addrspace(1)** %output_velocity_x.addr
  store float addrspace(1)* %output_velocity_y, float addrspace(1)** %output_velocity_y.addr
  store float addrspace(1)* %output_velocity_z, float addrspace(1)** %output_velocity_z.addr
  store i32 %body_count, i32* %body_count.addr
  store i32 %body_count_per_item, i32* %body_count_per_item.addr
  store float %softening_squared, float* %softening_squared.addr
  store float %time_delta, float* %time_delta.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %index
  %tmp = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %0 = bitcast <4 x float> addrspace(1)* %tmp to float addrspace(1)* ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %0, float addrspace(1)** %in_position_x
  %tmp2 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %1 = bitcast <4 x float> addrspace(1)* %tmp2 to float addrspace(1)* ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %1, float addrspace(1)** %in_position_y
  %tmp4 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %2 = bitcast <4 x float> addrspace(1)* %tmp4 to float addrspace(1)* ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %2, float addrspace(1)** %in_position_z
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp21 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp21, <4 x float>* %zero
  %tmp27 = load i32* %body_count.addr             ; <i32> [#uses=1]
  %shr = ashr i32 %tmp27, 2                       ; <i32> [#uses=1]
  %div = sdiv i32 %shr, 3                         ; <i32> [#uses=1]
  %mul = mul i32 %div, 3                          ; <i32> [#uses=1]
  store i32 %mul, i32* %inner_loop_count
  %tmp29 = load i32* %body_count_per_item.addr    ; <i32> [#uses=1]
  store i32 %tmp29, i32* %outer_loop_count
  %tmp31 = load i32* %index                       ; <i32> [#uses=1]
  %tmp32 = load i32* %outer_loop_count            ; <i32> [#uses=1]
  %mul33 = mul i32 %tmp31, %tmp32                 ; <i32> [#uses=1]
  store i32 %mul33, i32* %start
  %tmp35 = load i32* %start                       ; <i32> [#uses=1]
  %tmp36 = load i32* %outer_loop_count            ; <i32> [#uses=1]
  %add = add nsw i32 %tmp35, %tmp36               ; <i32> [#uses=1]
  store i32 %add, i32* %finish
  %tmp37 = load i32* %start                       ; <i32> [#uses=1]
  store i32 %tmp37, i32* %k
  br label %for.cond

for.cond:                                         ; preds = %for.inc574, %entry
  %tmp38 = load i32* %k                           ; <i32> [#uses=1]
  %tmp39 = load i32* %finish                      ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp38, %tmp39              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end577

for.body:                                         ; preds = %for.cond
  %tmp41 = load i32* %k                           ; <i32> [#uses=1]
  %tmp42 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp42, i32 %tmp41 ; <float addrspace(1)*> [#uses=1]
  %tmp43 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %tmp43, i32 0 ; <<4 x float>> [#uses=1]
  %tmp44 = load i32* %k                           ; <i32> [#uses=1]
  %tmp45 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float addrspace(1)* %tmp45, i32 %tmp44 ; <float addrspace(1)*> [#uses=1]
  %tmp47 = load float addrspace(1)* %arrayidx46   ; <float> [#uses=1]
  %vecinit48 = insertelement <4 x float> %vecinit, float %tmp47, i32 1 ; <<4 x float>> [#uses=1]
  %tmp49 = load i32* %k                           ; <i32> [#uses=1]
  %tmp50 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds float addrspace(1)* %tmp50, i32 %tmp49 ; <float addrspace(1)*> [#uses=1]
  %tmp52 = load float addrspace(1)* %arrayidx51   ; <float> [#uses=1]
  %vecinit53 = insertelement <4 x float> %vecinit48, float %tmp52, i32 2 ; <<4 x float>> [#uses=1]
  %tmp54 = load i32* %k                           ; <i32> [#uses=1]
  %tmp55 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(1)* %tmp55, i32 %tmp54 ; <float addrspace(1)*> [#uses=1]
  %tmp57 = load float addrspace(1)* %arrayidx56   ; <float> [#uses=1]
  %vecinit58 = insertelement <4 x float> %vecinit53, float %tmp57, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit58, <4 x float>* %.compoundliteral40
  %tmp59 = load <4 x float>* %.compoundliteral40  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp59, <4 x float>* %position_x
  %tmp61 = load i32* %k                           ; <i32> [#uses=1]
  %tmp62 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx63 = getelementptr inbounds float addrspace(1)* %tmp62, i32 %tmp61 ; <float addrspace(1)*> [#uses=1]
  %tmp64 = load float addrspace(1)* %arrayidx63   ; <float> [#uses=1]
  %vecinit65 = insertelement <4 x float> undef, float %tmp64, i32 0 ; <<4 x float>> [#uses=1]
  %tmp66 = load i32* %k                           ; <i32> [#uses=1]
  %tmp67 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx68 = getelementptr inbounds float addrspace(1)* %tmp67, i32 %tmp66 ; <float addrspace(1)*> [#uses=1]
  %tmp69 = load float addrspace(1)* %arrayidx68   ; <float> [#uses=1]
  %vecinit70 = insertelement <4 x float> %vecinit65, float %tmp69, i32 1 ; <<4 x float>> [#uses=1]
  %tmp71 = load i32* %k                           ; <i32> [#uses=1]
  %tmp72 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx73 = getelementptr inbounds float addrspace(1)* %tmp72, i32 %tmp71 ; <float addrspace(1)*> [#uses=1]
  %tmp74 = load float addrspace(1)* %arrayidx73   ; <float> [#uses=1]
  %vecinit75 = insertelement <4 x float> %vecinit70, float %tmp74, i32 2 ; <<4 x float>> [#uses=1]
  %tmp76 = load i32* %k                           ; <i32> [#uses=1]
  %tmp77 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(1)* %tmp77, i32 %tmp76 ; <float addrspace(1)*> [#uses=1]
  %tmp79 = load float addrspace(1)* %arrayidx78   ; <float> [#uses=1]
  %vecinit80 = insertelement <4 x float> %vecinit75, float %tmp79, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit80, <4 x float>* %.compoundliteral60
  %tmp81 = load <4 x float>* %.compoundliteral60  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81, <4 x float>* %position_y
  %tmp83 = load i32* %k                           ; <i32> [#uses=1]
  %tmp84 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx85 = getelementptr inbounds float addrspace(1)* %tmp84, i32 %tmp83 ; <float addrspace(1)*> [#uses=1]
  %tmp86 = load float addrspace(1)* %arrayidx85   ; <float> [#uses=1]
  %vecinit87 = insertelement <4 x float> undef, float %tmp86, i32 0 ; <<4 x float>> [#uses=1]
  %tmp88 = load i32* %k                           ; <i32> [#uses=1]
  %tmp89 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx90 = getelementptr inbounds float addrspace(1)* %tmp89, i32 %tmp88 ; <float addrspace(1)*> [#uses=1]
  %tmp91 = load float addrspace(1)* %arrayidx90   ; <float> [#uses=1]
  %vecinit92 = insertelement <4 x float> %vecinit87, float %tmp91, i32 1 ; <<4 x float>> [#uses=1]
  %tmp93 = load i32* %k                           ; <i32> [#uses=1]
  %tmp94 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds float addrspace(1)* %tmp94, i32 %tmp93 ; <float addrspace(1)*> [#uses=1]
  %tmp96 = load float addrspace(1)* %arrayidx95   ; <float> [#uses=1]
  %vecinit97 = insertelement <4 x float> %vecinit92, float %tmp96, i32 2 ; <<4 x float>> [#uses=1]
  %tmp98 = load i32* %k                           ; <i32> [#uses=1]
  %tmp99 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx100 = getelementptr inbounds float addrspace(1)* %tmp99, i32 %tmp98 ; <float addrspace(1)*> [#uses=1]
  %tmp101 = load float addrspace(1)* %arrayidx100 ; <float> [#uses=1]
  %vecinit102 = insertelement <4 x float> %vecinit97, float %tmp101, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit102, <4 x float>* %.compoundliteral82
  %tmp103 = load <4 x float>* %.compoundliteral82 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp103, <4 x float>* %position_z
  %tmp105 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp105, <4 x float>* %acceleration_x1
  %tmp107 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp107, <4 x float>* %acceleration_x2
  %tmp109 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp109, <4 x float>* %acceleration_x3
  %tmp111 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp111, <4 x float>* %acceleration_y1
  %tmp113 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp113, <4 x float>* %acceleration_y2
  %tmp115 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp115, <4 x float>* %acceleration_y3
  %tmp117 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp117, <4 x float>* %acceleration_z1
  %tmp119 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp119, <4 x float>* %acceleration_z2
  %tmp121 = load <4 x float>* %zero               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp121, <4 x float>* %acceleration_z3
  store i32 0, i32* %i
  br label %for.cond122

for.cond122:                                      ; preds = %for.inc, %for.body
  %tmp123 = load i32* %i                          ; <i32> [#uses=1]
  %tmp124 = load i32* %inner_loop_count           ; <i32> [#uses=1]
  %cmp125 = icmp slt i32 %tmp123, %tmp124         ; <i1> [#uses=1]
  br i1 %cmp125, label %for.body126, label %for.end

for.body126:                                      ; preds = %for.cond122
  %tmp128 = load i32* %i                          ; <i32> [#uses=1]
  %tmp129 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds <4 x float> addrspace(1)* %tmp129, i32 %tmp128 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp131 = load <4 x float> addrspace(1)* %arrayidx130 ; <<4 x float>> [#uses=1]
  %tmp132 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp131, %tmp132        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %dx1
  %tmp134 = load i32* %i                          ; <i32> [#uses=1]
  %add135 = add nsw i32 %tmp134, 1                ; <i32> [#uses=1]
  %tmp136 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx137 = getelementptr inbounds <4 x float> addrspace(1)* %tmp136, i32 %add135 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp138 = load <4 x float> addrspace(1)* %arrayidx137 ; <<4 x float>> [#uses=1]
  %tmp139 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %sub140 = fsub <4 x float> %tmp138, %tmp139     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub140, <4 x float>* %dx2
  %tmp142 = load i32* %i                          ; <i32> [#uses=1]
  %add143 = add nsw i32 %tmp142, 2                ; <i32> [#uses=1]
  %tmp144 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx145 = getelementptr inbounds <4 x float> addrspace(1)* %tmp144, i32 %add143 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp146 = load <4 x float> addrspace(1)* %arrayidx145 ; <<4 x float>> [#uses=1]
  %tmp147 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %sub148 = fsub <4 x float> %tmp146, %tmp147     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub148, <4 x float>* %dx3
  %tmp150 = load i32* %i                          ; <i32> [#uses=1]
  %tmp151 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx152 = getelementptr inbounds <4 x float> addrspace(1)* %tmp151, i32 %tmp150 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp153 = load <4 x float> addrspace(1)* %arrayidx152 ; <<4 x float>> [#uses=1]
  %tmp154 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %sub155 = fsub <4 x float> %tmp153, %tmp154     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub155, <4 x float>* %dy1
  %tmp157 = load i32* %i                          ; <i32> [#uses=1]
  %add158 = add nsw i32 %tmp157, 1                ; <i32> [#uses=1]
  %tmp159 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx160 = getelementptr inbounds <4 x float> addrspace(1)* %tmp159, i32 %add158 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp161 = load <4 x float> addrspace(1)* %arrayidx160 ; <<4 x float>> [#uses=1]
  %tmp162 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %sub163 = fsub <4 x float> %tmp161, %tmp162     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub163, <4 x float>* %dy2
  %tmp165 = load i32* %i                          ; <i32> [#uses=1]
  %add166 = add nsw i32 %tmp165, 2                ; <i32> [#uses=1]
  %tmp167 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx168 = getelementptr inbounds <4 x float> addrspace(1)* %tmp167, i32 %add166 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp169 = load <4 x float> addrspace(1)* %arrayidx168 ; <<4 x float>> [#uses=1]
  %tmp170 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %sub171 = fsub <4 x float> %tmp169, %tmp170     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub171, <4 x float>* %dy3
  %tmp173 = load i32* %i                          ; <i32> [#uses=1]
  %tmp174 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds <4 x float> addrspace(1)* %tmp174, i32 %tmp173 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp176 = load <4 x float> addrspace(1)* %arrayidx175 ; <<4 x float>> [#uses=1]
  %tmp177 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %sub178 = fsub <4 x float> %tmp176, %tmp177     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub178, <4 x float>* %dz1
  %tmp180 = load i32* %i                          ; <i32> [#uses=1]
  %add181 = add nsw i32 %tmp180, 1                ; <i32> [#uses=1]
  %tmp182 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx183 = getelementptr inbounds <4 x float> addrspace(1)* %tmp182, i32 %add181 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp184 = load <4 x float> addrspace(1)* %arrayidx183 ; <<4 x float>> [#uses=1]
  %tmp185 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %sub186 = fsub <4 x float> %tmp184, %tmp185     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub186, <4 x float>* %dz2
  %tmp188 = load i32* %i                          ; <i32> [#uses=1]
  %add189 = add nsw i32 %tmp188, 2                ; <i32> [#uses=1]
  %tmp190 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx191 = getelementptr inbounds <4 x float> addrspace(1)* %tmp190, i32 %add189 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp192 = load <4 x float> addrspace(1)* %arrayidx191 ; <<4 x float>> [#uses=1]
  %tmp193 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %sub194 = fsub <4 x float> %tmp192, %tmp193     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub194, <4 x float>* %dz3
  %tmp196 = load <4 x float>* %dx1                ; <<4 x float>> [#uses=1]
  %tmp197 = load <4 x float>* %dx1                ; <<4 x float>> [#uses=1]
  %mul198 = fmul <4 x float> %tmp196, %tmp197     ; <<4 x float>> [#uses=1]
  %tmp199 = load <4 x float>* %dy1                ; <<4 x float>> [#uses=1]
  %tmp200 = load <4 x float>* %dy1                ; <<4 x float>> [#uses=1]
  %mul201 = fmul <4 x float> %tmp199, %tmp200     ; <<4 x float>> [#uses=1]
  %add202 = fadd <4 x float> %mul198, %mul201     ; <<4 x float>> [#uses=1]
  %tmp203 = load <4 x float>* %dz1                ; <<4 x float>> [#uses=1]
  %tmp204 = load <4 x float>* %dz1                ; <<4 x float>> [#uses=1]
  %mul205 = fmul <4 x float> %tmp203, %tmp204     ; <<4 x float>> [#uses=1]
  %add206 = fadd <4 x float> %add202, %mul205     ; <<4 x float>> [#uses=1]
  store <4 x float> %add206, <4 x float>* %distance_squared1
  %tmp208 = load <4 x float>* %dx2                ; <<4 x float>> [#uses=1]
  %tmp209 = load <4 x float>* %dx2                ; <<4 x float>> [#uses=1]
  %mul210 = fmul <4 x float> %tmp208, %tmp209     ; <<4 x float>> [#uses=1]
  %tmp211 = load <4 x float>* %dy2                ; <<4 x float>> [#uses=1]
  %tmp212 = load <4 x float>* %dy2                ; <<4 x float>> [#uses=1]
  %mul213 = fmul <4 x float> %tmp211, %tmp212     ; <<4 x float>> [#uses=1]
  %add214 = fadd <4 x float> %mul210, %mul213     ; <<4 x float>> [#uses=1]
  %tmp215 = load <4 x float>* %dz2                ; <<4 x float>> [#uses=1]
  %tmp216 = load <4 x float>* %dz2                ; <<4 x float>> [#uses=1]
  %mul217 = fmul <4 x float> %tmp215, %tmp216     ; <<4 x float>> [#uses=1]
  %add218 = fadd <4 x float> %add214, %mul217     ; <<4 x float>> [#uses=1]
  store <4 x float> %add218, <4 x float>* %distance_squared2
  %tmp220 = load <4 x float>* %dx3                ; <<4 x float>> [#uses=1]
  %tmp221 = load <4 x float>* %dx3                ; <<4 x float>> [#uses=1]
  %mul222 = fmul <4 x float> %tmp220, %tmp221     ; <<4 x float>> [#uses=1]
  %tmp223 = load <4 x float>* %dy3                ; <<4 x float>> [#uses=1]
  %tmp224 = load <4 x float>* %dy3                ; <<4 x float>> [#uses=1]
  %mul225 = fmul <4 x float> %tmp223, %tmp224     ; <<4 x float>> [#uses=1]
  %add226 = fadd <4 x float> %mul222, %mul225     ; <<4 x float>> [#uses=1]
  %tmp227 = load <4 x float>* %dz3                ; <<4 x float>> [#uses=1]
  %tmp228 = load <4 x float>* %dz3                ; <<4 x float>> [#uses=1]
  %mul229 = fmul <4 x float> %tmp227, %tmp228     ; <<4 x float>> [#uses=1]
  %add230 = fadd <4 x float> %add226, %mul229     ; <<4 x float>> [#uses=1]
  store <4 x float> %add230, <4 x float>* %distance_squared3
  %tmp231 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp232 = insertelement <4 x float> undef, float %tmp231, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp232, <4 x float> %tmp232, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp233 = load <4 x float>* %distance_squared1  ; <<4 x float>> [#uses=1]
  %add234 = fadd <4 x float> %tmp233, %splat      ; <<4 x float>> [#uses=1]
  store <4 x float> %add234, <4 x float>* %distance_squared1
  %tmp235 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp236 = insertelement <4 x float> undef, float %tmp235, i32 0 ; <<4 x float>> [#uses=2]
  %splat237 = shufflevector <4 x float> %tmp236, <4 x float> %tmp236, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp238 = load <4 x float>* %distance_squared2  ; <<4 x float>> [#uses=1]
  %add239 = fadd <4 x float> %tmp238, %splat237   ; <<4 x float>> [#uses=1]
  store <4 x float> %add239, <4 x float>* %distance_squared2
  %tmp240 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp241 = insertelement <4 x float> undef, float %tmp240, i32 0 ; <<4 x float>> [#uses=2]
  %splat242 = shufflevector <4 x float> %tmp241, <4 x float> %tmp241, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp243 = load <4 x float>* %distance_squared3  ; <<4 x float>> [#uses=1]
  %add244 = fadd <4 x float> %tmp243, %splat242   ; <<4 x float>> [#uses=1]
  store <4 x float> %add244, <4 x float>* %distance_squared3
  %tmp246 = load <4 x float>* %distance_squared1  ; <<4 x float>> [#uses=1]
  %call247 = call <4 x float> @_Z5rsqrtU8__vector4f(<4 x float> %tmp246) ; <<4 x float>> [#uses=1]
  store <4 x float> %call247, <4 x float>* %inverse_distance1
  %tmp249 = load <4 x float>* %distance_squared2  ; <<4 x float>> [#uses=1]
  %call250 = call <4 x float> @_Z5rsqrtU8__vector4f(<4 x float> %tmp249) ; <<4 x float>> [#uses=1]
  store <4 x float> %call250, <4 x float>* %inverse_distance2
  %tmp252 = load <4 x float>* %distance_squared3  ; <<4 x float>> [#uses=1]
  %call253 = call <4 x float> @_Z5rsqrtU8__vector4f(<4 x float> %tmp252) ; <<4 x float>> [#uses=1]
  store <4 x float> %call253, <4 x float>* %inverse_distance3
  %tmp255 = load i32* %i                          ; <i32> [#uses=1]
  %tmp256 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx257 = getelementptr inbounds <4 x float> addrspace(1)* %tmp256, i32 %tmp255 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp258 = load <4 x float> addrspace(1)* %arrayidx257 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp258, <4 x float>* %mi1
  %tmp260 = load i32* %i                          ; <i32> [#uses=1]
  %add261 = add nsw i32 %tmp260, 1                ; <i32> [#uses=1]
  %tmp262 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx263 = getelementptr inbounds <4 x float> addrspace(1)* %tmp262, i32 %add261 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp264 = load <4 x float> addrspace(1)* %arrayidx263 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp264, <4 x float>* %mi2
  %tmp266 = load i32* %i                          ; <i32> [#uses=1]
  %add267 = add nsw i32 %tmp266, 2                ; <i32> [#uses=1]
  %tmp268 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx269 = getelementptr inbounds <4 x float> addrspace(1)* %tmp268, i32 %add267 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp270 = load <4 x float> addrspace(1)* %arrayidx269 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp270, <4 x float>* %mi3
  %tmp272 = load <4 x float>* %mi1                ; <<4 x float>> [#uses=1]
  %tmp273 = load <4 x float>* %inverse_distance1  ; <<4 x float>> [#uses=1]
  %mul274 = fmul <4 x float> %tmp272, %tmp273     ; <<4 x float>> [#uses=1]
  %tmp275 = load <4 x float>* %inverse_distance1  ; <<4 x float>> [#uses=1]
  %tmp276 = load <4 x float>* %inverse_distance1  ; <<4 x float>> [#uses=1]
  %mul277 = fmul <4 x float> %tmp275, %tmp276     ; <<4 x float>> [#uses=1]
  %mul278 = fmul <4 x float> %mul274, %mul277     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul278, <4 x float>* %s1
  %tmp280 = load <4 x float>* %mi2                ; <<4 x float>> [#uses=1]
  %tmp281 = load <4 x float>* %inverse_distance2  ; <<4 x float>> [#uses=1]
  %mul282 = fmul <4 x float> %tmp280, %tmp281     ; <<4 x float>> [#uses=1]
  %tmp283 = load <4 x float>* %inverse_distance2  ; <<4 x float>> [#uses=1]
  %tmp284 = load <4 x float>* %inverse_distance2  ; <<4 x float>> [#uses=1]
  %mul285 = fmul <4 x float> %tmp283, %tmp284     ; <<4 x float>> [#uses=1]
  %mul286 = fmul <4 x float> %mul282, %mul285     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul286, <4 x float>* %s2
  %tmp288 = load <4 x float>* %mi3                ; <<4 x float>> [#uses=1]
  %tmp289 = load <4 x float>* %inverse_distance3  ; <<4 x float>> [#uses=1]
  %mul290 = fmul <4 x float> %tmp288, %tmp289     ; <<4 x float>> [#uses=1]
  %tmp291 = load <4 x float>* %inverse_distance3  ; <<4 x float>> [#uses=1]
  %tmp292 = load <4 x float>* %inverse_distance3  ; <<4 x float>> [#uses=1]
  %mul293 = fmul <4 x float> %tmp291, %tmp292     ; <<4 x float>> [#uses=1]
  %mul294 = fmul <4 x float> %mul290, %mul293     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul294, <4 x float>* %s3
  %tmp295 = load <4 x float>* %dx1                ; <<4 x float>> [#uses=1]
  %tmp296 = load <4 x float>* %s1                 ; <<4 x float>> [#uses=1]
  %mul297 = fmul <4 x float> %tmp295, %tmp296     ; <<4 x float>> [#uses=1]
  %tmp298 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %add299 = fadd <4 x float> %tmp298, %mul297     ; <<4 x float>> [#uses=1]
  store <4 x float> %add299, <4 x float>* %acceleration_x1
  %tmp300 = load <4 x float>* %dx2                ; <<4 x float>> [#uses=1]
  %tmp301 = load <4 x float>* %s2                 ; <<4 x float>> [#uses=1]
  %mul302 = fmul <4 x float> %tmp300, %tmp301     ; <<4 x float>> [#uses=1]
  %tmp303 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %add304 = fadd <4 x float> %tmp303, %mul302     ; <<4 x float>> [#uses=1]
  store <4 x float> %add304, <4 x float>* %acceleration_x2
  %tmp305 = load <4 x float>* %dx3                ; <<4 x float>> [#uses=1]
  %tmp306 = load <4 x float>* %s3                 ; <<4 x float>> [#uses=1]
  %mul307 = fmul <4 x float> %tmp305, %tmp306     ; <<4 x float>> [#uses=1]
  %tmp308 = load <4 x float>* %acceleration_x3    ; <<4 x float>> [#uses=1]
  %add309 = fadd <4 x float> %tmp308, %mul307     ; <<4 x float>> [#uses=1]
  store <4 x float> %add309, <4 x float>* %acceleration_x3
  %tmp310 = load <4 x float>* %dy1                ; <<4 x float>> [#uses=1]
  %tmp311 = load <4 x float>* %s1                 ; <<4 x float>> [#uses=1]
  %mul312 = fmul <4 x float> %tmp310, %tmp311     ; <<4 x float>> [#uses=1]
  %tmp313 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %add314 = fadd <4 x float> %tmp313, %mul312     ; <<4 x float>> [#uses=1]
  store <4 x float> %add314, <4 x float>* %acceleration_y1
  %tmp315 = load <4 x float>* %dy2                ; <<4 x float>> [#uses=1]
  %tmp316 = load <4 x float>* %s2                 ; <<4 x float>> [#uses=1]
  %mul317 = fmul <4 x float> %tmp315, %tmp316     ; <<4 x float>> [#uses=1]
  %tmp318 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %add319 = fadd <4 x float> %tmp318, %mul317     ; <<4 x float>> [#uses=1]
  store <4 x float> %add319, <4 x float>* %acceleration_y2
  %tmp320 = load <4 x float>* %dy3                ; <<4 x float>> [#uses=1]
  %tmp321 = load <4 x float>* %s3                 ; <<4 x float>> [#uses=1]
  %mul322 = fmul <4 x float> %tmp320, %tmp321     ; <<4 x float>> [#uses=1]
  %tmp323 = load <4 x float>* %acceleration_y3    ; <<4 x float>> [#uses=1]
  %add324 = fadd <4 x float> %tmp323, %mul322     ; <<4 x float>> [#uses=1]
  store <4 x float> %add324, <4 x float>* %acceleration_y3
  %tmp325 = load <4 x float>* %dz1                ; <<4 x float>> [#uses=1]
  %tmp326 = load <4 x float>* %s1                 ; <<4 x float>> [#uses=1]
  %mul327 = fmul <4 x float> %tmp325, %tmp326     ; <<4 x float>> [#uses=1]
  %tmp328 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %add329 = fadd <4 x float> %tmp328, %mul327     ; <<4 x float>> [#uses=1]
  store <4 x float> %add329, <4 x float>* %acceleration_z1
  %tmp330 = load <4 x float>* %dz2                ; <<4 x float>> [#uses=1]
  %tmp331 = load <4 x float>* %s2                 ; <<4 x float>> [#uses=1]
  %mul332 = fmul <4 x float> %tmp330, %tmp331     ; <<4 x float>> [#uses=1]
  %tmp333 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %add334 = fadd <4 x float> %tmp333, %mul332     ; <<4 x float>> [#uses=1]
  store <4 x float> %add334, <4 x float>* %acceleration_z2
  %tmp335 = load <4 x float>* %dz3                ; <<4 x float>> [#uses=1]
  %tmp336 = load <4 x float>* %s3                 ; <<4 x float>> [#uses=1]
  %mul337 = fmul <4 x float> %tmp335, %tmp336     ; <<4 x float>> [#uses=1]
  %tmp338 = load <4 x float>* %acceleration_z3    ; <<4 x float>> [#uses=1]
  %add339 = fadd <4 x float> %tmp338, %mul337     ; <<4 x float>> [#uses=1]
  store <4 x float> %add339, <4 x float>* %acceleration_z3
  br label %for.inc

for.inc:                                          ; preds = %for.body126
  %tmp340 = load i32* %i                          ; <i32> [#uses=1]
  %add341 = add nsw i32 %tmp340, 3                ; <i32> [#uses=1]
  store i32 %add341, i32* %i
  br label %for.cond122

for.end:                                          ; preds = %for.cond122
  br label %for.cond342

for.cond342:                                      ; preds = %for.inc424, %for.end
  %tmp343 = load i32* %i                          ; <i32> [#uses=1]
  %tmp344 = load i32* %body_count.addr            ; <i32> [#uses=1]
  %shr345 = ashr i32 %tmp344, 2                   ; <i32> [#uses=1]
  %cmp346 = icmp slt i32 %tmp343, %shr345         ; <i1> [#uses=1]
  br i1 %cmp346, label %for.body347, label %for.end426

for.body347:                                      ; preds = %for.cond342
  %tmp350 = load i32* %i                          ; <i32> [#uses=1]
  %tmp351 = load <4 x float> addrspace(1)** %input_position_x.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx352 = getelementptr inbounds <4 x float> addrspace(1)* %tmp351, i32 %tmp350 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp353 = load <4 x float> addrspace(1)* %arrayidx352 ; <<4 x float>> [#uses=1]
  %tmp354 = load <4 x float>* %position_x         ; <<4 x float>> [#uses=1]
  %sub355 = fsub <4 x float> %tmp353, %tmp354     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub355, <4 x float>* %dx1349
  %tmp358 = load i32* %i                          ; <i32> [#uses=1]
  %tmp359 = load <4 x float> addrspace(1)** %input_position_y.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx360 = getelementptr inbounds <4 x float> addrspace(1)* %tmp359, i32 %tmp358 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp361 = load <4 x float> addrspace(1)* %arrayidx360 ; <<4 x float>> [#uses=1]
  %tmp362 = load <4 x float>* %position_y         ; <<4 x float>> [#uses=1]
  %sub363 = fsub <4 x float> %tmp361, %tmp362     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub363, <4 x float>* %dy1357
  %tmp366 = load i32* %i                          ; <i32> [#uses=1]
  %tmp367 = load <4 x float> addrspace(1)** %input_position_z.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx368 = getelementptr inbounds <4 x float> addrspace(1)* %tmp367, i32 %tmp366 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp369 = load <4 x float> addrspace(1)* %arrayidx368 ; <<4 x float>> [#uses=1]
  %tmp370 = load <4 x float>* %position_z         ; <<4 x float>> [#uses=1]
  %sub371 = fsub <4 x float> %tmp369, %tmp370     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub371, <4 x float>* %dz1365
  %tmp374 = load <4 x float>* %dx1349             ; <<4 x float>> [#uses=1]
  %tmp375 = load <4 x float>* %dx1349             ; <<4 x float>> [#uses=1]
  %mul376 = fmul <4 x float> %tmp374, %tmp375     ; <<4 x float>> [#uses=1]
  %tmp377 = load <4 x float>* %dy1357             ; <<4 x float>> [#uses=1]
  %tmp378 = load <4 x float>* %dy1357             ; <<4 x float>> [#uses=1]
  %mul379 = fmul <4 x float> %tmp377, %tmp378     ; <<4 x float>> [#uses=1]
  %add380 = fadd <4 x float> %mul376, %mul379     ; <<4 x float>> [#uses=1]
  %tmp381 = load <4 x float>* %dz1365             ; <<4 x float>> [#uses=1]
  %tmp382 = load <4 x float>* %dz1365             ; <<4 x float>> [#uses=1]
  %mul383 = fmul <4 x float> %tmp381, %tmp382     ; <<4 x float>> [#uses=1]
  %add384 = fadd <4 x float> %add380, %mul383     ; <<4 x float>> [#uses=1]
  store <4 x float> %add384, <4 x float>* %distance_squared1373
  %tmp385 = load float* %softening_squared.addr   ; <float> [#uses=1]
  %tmp386 = insertelement <4 x float> undef, float %tmp385, i32 0 ; <<4 x float>> [#uses=2]
  %splat387 = shufflevector <4 x float> %tmp386, <4 x float> %tmp386, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp388 = load <4 x float>* %distance_squared1373 ; <<4 x float>> [#uses=1]
  %add389 = fadd <4 x float> %tmp388, %splat387   ; <<4 x float>> [#uses=1]
  store <4 x float> %add389, <4 x float>* %distance_squared1373
  %tmp392 = load <4 x float>* %distance_squared1373 ; <<4 x float>> [#uses=1]
  %call393 = call <4 x float> @_Z5rsqrtU8__vector4f(<4 x float> %tmp392) ; <<4 x float>> [#uses=1]
  store <4 x float> %call393, <4 x float>* %inverse_distance1391
  %tmp396 = load i32* %i                          ; <i32> [#uses=1]
  %tmp397 = load <4 x float> addrspace(1)** %mass.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx398 = getelementptr inbounds <4 x float> addrspace(1)* %tmp397, i32 %tmp396 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp399 = load <4 x float> addrspace(1)* %arrayidx398 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp399, <4 x float>* %mi1395
  %tmp402 = load <4 x float>* %mi1395             ; <<4 x float>> [#uses=1]
  %tmp403 = load <4 x float>* %inverse_distance1391 ; <<4 x float>> [#uses=1]
  %mul404 = fmul <4 x float> %tmp402, %tmp403     ; <<4 x float>> [#uses=1]
  %tmp405 = load <4 x float>* %inverse_distance1391 ; <<4 x float>> [#uses=1]
  %tmp406 = load <4 x float>* %inverse_distance1391 ; <<4 x float>> [#uses=1]
  %mul407 = fmul <4 x float> %tmp405, %tmp406     ; <<4 x float>> [#uses=1]
  %mul408 = fmul <4 x float> %mul404, %mul407     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul408, <4 x float>* %s1401
  %tmp409 = load <4 x float>* %dx1349             ; <<4 x float>> [#uses=1]
  %tmp410 = load <4 x float>* %s1401              ; <<4 x float>> [#uses=1]
  %mul411 = fmul <4 x float> %tmp409, %tmp410     ; <<4 x float>> [#uses=1]
  %tmp412 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %add413 = fadd <4 x float> %tmp412, %mul411     ; <<4 x float>> [#uses=1]
  store <4 x float> %add413, <4 x float>* %acceleration_x1
  %tmp414 = load <4 x float>* %dy1357             ; <<4 x float>> [#uses=1]
  %tmp415 = load <4 x float>* %s1401              ; <<4 x float>> [#uses=1]
  %mul416 = fmul <4 x float> %tmp414, %tmp415     ; <<4 x float>> [#uses=1]
  %tmp417 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %add418 = fadd <4 x float> %tmp417, %mul416     ; <<4 x float>> [#uses=1]
  store <4 x float> %add418, <4 x float>* %acceleration_y1
  %tmp419 = load <4 x float>* %dz1365             ; <<4 x float>> [#uses=1]
  %tmp420 = load <4 x float>* %s1401              ; <<4 x float>> [#uses=1]
  %mul421 = fmul <4 x float> %tmp419, %tmp420     ; <<4 x float>> [#uses=1]
  %tmp422 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %add423 = fadd <4 x float> %tmp422, %mul421     ; <<4 x float>> [#uses=1]
  store <4 x float> %add423, <4 x float>* %acceleration_z1
  br label %for.inc424

for.inc424:                                       ; preds = %for.body347
  %tmp425 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp425, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond342

for.end426:                                       ; preds = %for.cond342
  %tmp427 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp428 = load <4 x float>* %acceleration_x2    ; <<4 x float>> [#uses=1]
  %add429 = fadd <4 x float> %tmp427, %tmp428     ; <<4 x float>> [#uses=1]
  %tmp430 = load <4 x float>* %acceleration_x3    ; <<4 x float>> [#uses=1]
  %add431 = fadd <4 x float> %add429, %tmp430     ; <<4 x float>> [#uses=1]
  store <4 x float> %add431, <4 x float>* %acceleration_x1
  %tmp432 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp433 = load <4 x float>* %acceleration_y2    ; <<4 x float>> [#uses=1]
  %add434 = fadd <4 x float> %tmp432, %tmp433     ; <<4 x float>> [#uses=1]
  %tmp435 = load <4 x float>* %acceleration_y3    ; <<4 x float>> [#uses=1]
  %add436 = fadd <4 x float> %add434, %tmp435     ; <<4 x float>> [#uses=1]
  store <4 x float> %add436, <4 x float>* %acceleration_y1
  %tmp437 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp438 = load <4 x float>* %acceleration_z2    ; <<4 x float>> [#uses=1]
  %add439 = fadd <4 x float> %tmp437, %tmp438     ; <<4 x float>> [#uses=1]
  %tmp440 = load <4 x float>* %acceleration_z3    ; <<4 x float>> [#uses=1]
  %add441 = fadd <4 x float> %add439, %tmp440     ; <<4 x float>> [#uses=1]
  store <4 x float> %add441, <4 x float>* %acceleration_z1
  %tmp443 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp444 = extractelement <4 x float> %tmp443, i32 0 ; <float> [#uses=1]
  %tmp445 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp446 = extractelement <4 x float> %tmp445, i32 1 ; <float> [#uses=1]
  %add447 = fadd float %tmp444, %tmp446           ; <float> [#uses=1]
  %tmp448 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp449 = extractelement <4 x float> %tmp448, i32 2 ; <float> [#uses=1]
  %add450 = fadd float %add447, %tmp449           ; <float> [#uses=1]
  %tmp451 = load <4 x float>* %acceleration_x1    ; <<4 x float>> [#uses=1]
  %tmp452 = extractelement <4 x float> %tmp451, i32 3 ; <float> [#uses=1]
  %add453 = fadd float %add450, %tmp452           ; <float> [#uses=1]
  store float %add453, float* %acc_x
  %tmp455 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp456 = extractelement <4 x float> %tmp455, i32 0 ; <float> [#uses=1]
  %tmp457 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp458 = extractelement <4 x float> %tmp457, i32 1 ; <float> [#uses=1]
  %add459 = fadd float %tmp456, %tmp458           ; <float> [#uses=1]
  %tmp460 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp461 = extractelement <4 x float> %tmp460, i32 2 ; <float> [#uses=1]
  %add462 = fadd float %add459, %tmp461           ; <float> [#uses=1]
  %tmp463 = load <4 x float>* %acceleration_y1    ; <<4 x float>> [#uses=1]
  %tmp464 = extractelement <4 x float> %tmp463, i32 3 ; <float> [#uses=1]
  %add465 = fadd float %add462, %tmp464           ; <float> [#uses=1]
  store float %add465, float* %acc_y
  %tmp467 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp468 = extractelement <4 x float> %tmp467, i32 0 ; <float> [#uses=1]
  %tmp469 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp470 = extractelement <4 x float> %tmp469, i32 1 ; <float> [#uses=1]
  %add471 = fadd float %tmp468, %tmp470           ; <float> [#uses=1]
  %tmp472 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp473 = extractelement <4 x float> %tmp472, i32 2 ; <float> [#uses=1]
  %add474 = fadd float %add471, %tmp473           ; <float> [#uses=1]
  %tmp475 = load <4 x float>* %acceleration_z1    ; <<4 x float>> [#uses=1]
  %tmp476 = extractelement <4 x float> %tmp475, i32 3 ; <float> [#uses=1]
  %add477 = fadd float %add474, %tmp476           ; <float> [#uses=1]
  store float %add477, float* %acc_z
  %tmp478 = load i32* %k                          ; <i32> [#uses=1]
  %tmp479 = load float addrspace(1)** %input_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx480 = getelementptr inbounds float addrspace(1)* %tmp479, i32 %tmp478 ; <float addrspace(1)*> [#uses=1]
  %tmp481 = load float addrspace(1)* %arrayidx480 ; <float> [#uses=1]
  %tmp482 = load float* %acc_x                    ; <float> [#uses=1]
  %tmp483 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul484 = fmul float %tmp482, %tmp483           ; <float> [#uses=1]
  %add485 = fadd float %tmp481, %mul484           ; <float> [#uses=1]
  %tmp486 = load i32* %k                          ; <i32> [#uses=1]
  %tmp487 = load float addrspace(1)** %output_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx488 = getelementptr inbounds float addrspace(1)* %tmp487, i32 %tmp486 ; <float addrspace(1)*> [#uses=1]
  store float %add485, float addrspace(1)* %arrayidx488
  %tmp489 = load i32* %k                          ; <i32> [#uses=1]
  %tmp490 = load float addrspace(1)** %input_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx491 = getelementptr inbounds float addrspace(1)* %tmp490, i32 %tmp489 ; <float addrspace(1)*> [#uses=1]
  %tmp492 = load float addrspace(1)* %arrayidx491 ; <float> [#uses=1]
  %tmp493 = load float* %acc_y                    ; <float> [#uses=1]
  %tmp494 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul495 = fmul float %tmp493, %tmp494           ; <float> [#uses=1]
  %add496 = fadd float %tmp492, %mul495           ; <float> [#uses=1]
  %tmp497 = load i32* %k                          ; <i32> [#uses=1]
  %tmp498 = load float addrspace(1)** %output_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx499 = getelementptr inbounds float addrspace(1)* %tmp498, i32 %tmp497 ; <float addrspace(1)*> [#uses=1]
  store float %add496, float addrspace(1)* %arrayidx499
  %tmp500 = load i32* %k                          ; <i32> [#uses=1]
  %tmp501 = load float addrspace(1)** %input_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx502 = getelementptr inbounds float addrspace(1)* %tmp501, i32 %tmp500 ; <float addrspace(1)*> [#uses=1]
  %tmp503 = load float addrspace(1)* %arrayidx502 ; <float> [#uses=1]
  %tmp504 = load float* %acc_z                    ; <float> [#uses=1]
  %tmp505 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul506 = fmul float %tmp504, %tmp505           ; <float> [#uses=1]
  %add507 = fadd float %tmp503, %mul506           ; <float> [#uses=1]
  %tmp508 = load i32* %k                          ; <i32> [#uses=1]
  %tmp509 = load float addrspace(1)** %output_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx510 = getelementptr inbounds float addrspace(1)* %tmp509, i32 %tmp508 ; <float addrspace(1)*> [#uses=1]
  store float %add507, float addrspace(1)* %arrayidx510
  %tmp511 = load i32* %k                          ; <i32> [#uses=1]
  %tmp512 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx513 = getelementptr inbounds float addrspace(1)* %tmp512, i32 %tmp511 ; <float addrspace(1)*> [#uses=1]
  %tmp514 = load float addrspace(1)* %arrayidx513 ; <float> [#uses=1]
  %tmp515 = load i32* %k                          ; <i32> [#uses=1]
  %tmp516 = load float addrspace(1)** %input_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx517 = getelementptr inbounds float addrspace(1)* %tmp516, i32 %tmp515 ; <float addrspace(1)*> [#uses=1]
  %tmp518 = load float addrspace(1)* %arrayidx517 ; <float> [#uses=1]
  %tmp519 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul520 = fmul float %tmp518, %tmp519           ; <float> [#uses=1]
  %add521 = fadd float %tmp514, %mul520           ; <float> [#uses=1]
  %tmp522 = load float* %acc_x                    ; <float> [#uses=1]
  %tmp523 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul524 = fmul float %tmp522, %tmp523           ; <float> [#uses=1]
  %tmp525 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul526 = fmul float %mul524, %tmp525           ; <float> [#uses=1]
  %div527 = fdiv float %mul526, 2.000000e+000     ; <float> [#uses=1]
  %add528 = fadd float %add521, %div527           ; <float> [#uses=1]
  %tmp529 = load i32* %k                          ; <i32> [#uses=1]
  %tmp530 = load float addrspace(1)** %output_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx531 = getelementptr inbounds float addrspace(1)* %tmp530, i32 %tmp529 ; <float addrspace(1)*> [#uses=1]
  store float %add528, float addrspace(1)* %arrayidx531
  %tmp532 = load i32* %k                          ; <i32> [#uses=1]
  %tmp533 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx534 = getelementptr inbounds float addrspace(1)* %tmp533, i32 %tmp532 ; <float addrspace(1)*> [#uses=1]
  %tmp535 = load float addrspace(1)* %arrayidx534 ; <float> [#uses=1]
  %tmp536 = load i32* %k                          ; <i32> [#uses=1]
  %tmp537 = load float addrspace(1)** %input_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx538 = getelementptr inbounds float addrspace(1)* %tmp537, i32 %tmp536 ; <float addrspace(1)*> [#uses=1]
  %tmp539 = load float addrspace(1)* %arrayidx538 ; <float> [#uses=1]
  %tmp540 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul541 = fmul float %tmp539, %tmp540           ; <float> [#uses=1]
  %add542 = fadd float %tmp535, %mul541           ; <float> [#uses=1]
  %tmp543 = load float* %acc_y                    ; <float> [#uses=1]
  %tmp544 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul545 = fmul float %tmp543, %tmp544           ; <float> [#uses=1]
  %tmp546 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul547 = fmul float %mul545, %tmp546           ; <float> [#uses=1]
  %div548 = fdiv float %mul547, 2.000000e+000     ; <float> [#uses=1]
  %add549 = fadd float %add542, %div548           ; <float> [#uses=1]
  %tmp550 = load i32* %k                          ; <i32> [#uses=1]
  %tmp551 = load float addrspace(1)** %output_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx552 = getelementptr inbounds float addrspace(1)* %tmp551, i32 %tmp550 ; <float addrspace(1)*> [#uses=1]
  store float %add549, float addrspace(1)* %arrayidx552
  %tmp553 = load i32* %k                          ; <i32> [#uses=1]
  %tmp554 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx555 = getelementptr inbounds float addrspace(1)* %tmp554, i32 %tmp553 ; <float addrspace(1)*> [#uses=1]
  %tmp556 = load float addrspace(1)* %arrayidx555 ; <float> [#uses=1]
  %tmp557 = load i32* %k                          ; <i32> [#uses=1]
  %tmp558 = load float addrspace(1)** %input_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx559 = getelementptr inbounds float addrspace(1)* %tmp558, i32 %tmp557 ; <float addrspace(1)*> [#uses=1]
  %tmp560 = load float addrspace(1)* %arrayidx559 ; <float> [#uses=1]
  %tmp561 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul562 = fmul float %tmp560, %tmp561           ; <float> [#uses=1]
  %add563 = fadd float %tmp556, %mul562           ; <float> [#uses=1]
  %tmp564 = load float* %acc_z                    ; <float> [#uses=1]
  %tmp565 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul566 = fmul float %tmp564, %tmp565           ; <float> [#uses=1]
  %tmp567 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul568 = fmul float %mul566, %tmp567           ; <float> [#uses=1]
  %div569 = fdiv float %mul568, 2.000000e+000     ; <float> [#uses=1]
  %add570 = fadd float %add563, %div569           ; <float> [#uses=1]
  %tmp571 = load i32* %k                          ; <i32> [#uses=1]
  %tmp572 = load float addrspace(1)** %output_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx573 = getelementptr inbounds float addrspace(1)* %tmp572, i32 %tmp571 ; <float addrspace(1)*> [#uses=1]
  store float %add570, float addrspace(1)* %arrayidx573
  br label %for.inc574

for.inc574:                                       ; preds = %for.end426
  %tmp575 = load i32* %k                          ; <i32> [#uses=1]
  %inc576 = add nsw i32 %tmp575, 1                ; <i32> [#uses=1]
  store i32 %inc576, i32* %k
  br label %for.cond

for.end577:                                       ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z5rsqrtU8__vector4f(<4 x float>)

; CHECK: ret
define void @nBodyScalarKernel(float addrspace(1)* %input_position_x, float addrspace(1)* %input_position_y, float addrspace(1)* %input_position_z, float addrspace(1)* %mass, float addrspace(1)* %input_velocity_x, float addrspace(1)* %input_velocity_y, float addrspace(1)* %input_velocity_z, float addrspace(1)* %output_position_x, float addrspace(1)* %output_position_y, float addrspace(1)* %output_position_z, float addrspace(1)* %output_velocity_x, float addrspace(1)* %output_velocity_y, float addrspace(1)* %output_velocity_z, i32 %body_count, i32 %body_count_per_item, float %softening_squared, float %time_delta) nounwind {
entry:
  %input_position_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_position_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_position_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %mass.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %output_position_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_position_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_x.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_y.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %output_velocity_z.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %body_count.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %body_count_per_item.addr = alloca i32, align 4 ; <i32*> [#uses=1]
  %softening_squared.addr = alloca float, align 4 ; <float*> [#uses=2]
  %time_delta.addr = alloca float, align 4        ; <float*> [#uses=13]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %in_position_x = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %in_position_y = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %in_position_z = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=8]
  %inner_loop_count = alloca i32, align 4         ; <i32*> [#uses=2]
  %start = alloca i32, align 4                    ; <i32*> [#uses=2]
  %k = alloca i32, align 4                        ; <i32*> [#uses=19]
  %position_x = alloca float, align 4             ; <float*> [#uses=2]
  %position_y = alloca float, align 4             ; <float*> [#uses=2]
  %position_z = alloca float, align 4             ; <float*> [#uses=2]
  %acc_x = alloca float, align 4                  ; <float*> [#uses=5]
  %acc_y = alloca float, align 4                  ; <float*> [#uses=5]
  %acc_z = alloca float, align 4                  ; <float*> [#uses=5]
  %dx = alloca float, align 4                     ; <float*> [#uses=4]
  %dy = alloca float, align 4                     ; <float*> [#uses=4]
  %dz = alloca float, align 4                     ; <float*> [#uses=4]
  %distance_squared = alloca float, align 4       ; <float*> [#uses=2]
  %inverse_distance = alloca float, align 4       ; <float*> [#uses=4]
  %mi = alloca float, align 4                     ; <float*> [#uses=2]
  %s = alloca float, align 4                      ; <float*> [#uses=4]
  store float addrspace(1)* %input_position_x, float addrspace(1)** %input_position_x.addr
  store float addrspace(1)* %input_position_y, float addrspace(1)** %input_position_y.addr
  store float addrspace(1)* %input_position_z, float addrspace(1)** %input_position_z.addr
  store float addrspace(1)* %mass, float addrspace(1)** %mass.addr
  store float addrspace(1)* %input_velocity_x, float addrspace(1)** %input_velocity_x.addr
  store float addrspace(1)* %input_velocity_y, float addrspace(1)** %input_velocity_y.addr
  store float addrspace(1)* %input_velocity_z, float addrspace(1)** %input_velocity_z.addr
  store float addrspace(1)* %output_position_x, float addrspace(1)** %output_position_x.addr
  store float addrspace(1)* %output_position_y, float addrspace(1)** %output_position_y.addr
  store float addrspace(1)* %output_position_z, float addrspace(1)** %output_position_z.addr
  store float addrspace(1)* %output_velocity_x, float addrspace(1)** %output_velocity_x.addr
  store float addrspace(1)* %output_velocity_y, float addrspace(1)** %output_velocity_y.addr
  store float addrspace(1)* %output_velocity_z, float addrspace(1)** %output_velocity_z.addr
  store i32 %body_count, i32* %body_count.addr
  store i32 %body_count_per_item, i32* %body_count_per_item.addr
  store float %softening_squared, float* %softening_squared.addr
  store float %time_delta, float* %time_delta.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %index
  %tmp = load float addrspace(1)** %input_position_x.addr ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %tmp, float addrspace(1)** %in_position_x
  %tmp2 = load float addrspace(1)** %input_position_y.addr ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %tmp2, float addrspace(1)** %in_position_y
  %tmp4 = load float addrspace(1)** %input_position_z.addr ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %tmp4, float addrspace(1)** %in_position_z
  %tmp7 = load i32* %body_count.addr              ; <i32> [#uses=1]
  store i32 %tmp7, i32* %inner_loop_count
  %tmp9 = load i32* %index                        ; <i32> [#uses=1]
  store i32 %tmp9, i32* %start
  %tmp11 = load i32* %start                       ; <i32> [#uses=1]
  store i32 %tmp11, i32* %k
  %tmp13 = load i32* %k                           ; <i32> [#uses=1]
  %tmp14 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp14, i32 %tmp13 ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  store float %tmp15, float* %position_x
  %tmp17 = load i32* %k                           ; <i32> [#uses=1]
  %tmp18 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds float addrspace(1)* %tmp18, i32 %tmp17 ; <float addrspace(1)*> [#uses=1]
  %tmp20 = load float addrspace(1)* %arrayidx19   ; <float> [#uses=1]
  store float %tmp20, float* %position_y
  %tmp22 = load i32* %k                           ; <i32> [#uses=1]
  %tmp23 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds float addrspace(1)* %tmp23, i32 %tmp22 ; <float addrspace(1)*> [#uses=1]
  %tmp25 = load float addrspace(1)* %arrayidx24   ; <float> [#uses=1]
  store float %tmp25, float* %position_z
  store float 0.000000e+000, float* %acc_x
  store float 0.000000e+000, float* %acc_y
  store float 0.000000e+000, float* %acc_z
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp29 = load i32* %i                           ; <i32> [#uses=1]
  %tmp30 = load i32* %inner_loop_count            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp29, %tmp30              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp32 = load i32* %i                           ; <i32> [#uses=1]
  %tmp33 = load float addrspace(1)** %input_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 %tmp32 ; <float addrspace(1)*> [#uses=1]
  %tmp35 = load float addrspace(1)* %arrayidx34   ; <float> [#uses=1]
  %tmp36 = load float* %position_x                ; <float> [#uses=1]
  %sub = fsub float %tmp35, %tmp36                ; <float> [#uses=1]
  store float %sub, float* %dx
  %tmp38 = load i32* %i                           ; <i32> [#uses=1]
  %tmp39 = load float addrspace(1)** %input_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx40 = getelementptr inbounds float addrspace(1)* %tmp39, i32 %tmp38 ; <float addrspace(1)*> [#uses=1]
  %tmp41 = load float addrspace(1)* %arrayidx40   ; <float> [#uses=1]
  %tmp42 = load float* %position_y                ; <float> [#uses=1]
  %sub43 = fsub float %tmp41, %tmp42              ; <float> [#uses=1]
  store float %sub43, float* %dy
  %tmp45 = load i32* %i                           ; <i32> [#uses=1]
  %tmp46 = load float addrspace(1)** %input_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx47 = getelementptr inbounds float addrspace(1)* %tmp46, i32 %tmp45 ; <float addrspace(1)*> [#uses=1]
  %tmp48 = load float addrspace(1)* %arrayidx47   ; <float> [#uses=1]
  %tmp49 = load float* %position_z                ; <float> [#uses=1]
  %sub50 = fsub float %tmp48, %tmp49              ; <float> [#uses=1]
  store float %sub50, float* %dz
  %tmp52 = load float* %dx                        ; <float> [#uses=1]
  %tmp53 = load float* %dx                        ; <float> [#uses=1]
  %mul = fmul float %tmp52, %tmp53                ; <float> [#uses=1]
  %tmp54 = load float* %dy                        ; <float> [#uses=1]
  %tmp55 = load float* %dy                        ; <float> [#uses=1]
  %mul56 = fmul float %tmp54, %tmp55              ; <float> [#uses=1]
  %add = fadd float %mul, %mul56                  ; <float> [#uses=1]
  %tmp57 = load float* %dz                        ; <float> [#uses=1]
  %tmp58 = load float* %dz                        ; <float> [#uses=1]
  %mul59 = fmul float %tmp57, %tmp58              ; <float> [#uses=1]
  %add60 = fadd float %add, %mul59                ; <float> [#uses=1]
  %tmp61 = load float* %softening_squared.addr    ; <float> [#uses=1]
  %add62 = fadd float %add60, %tmp61              ; <float> [#uses=1]
  store float %add62, float* %distance_squared
  %tmp64 = load float* %distance_squared          ; <float> [#uses=1]
  %call65 = call float @_Z5rsqrtf(float %tmp64)   ; <float> [#uses=1]
  store float %call65, float* %inverse_distance
  %tmp67 = load i32* %i                           ; <i32> [#uses=1]
  %tmp68 = load float addrspace(1)** %mass.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx69 = getelementptr inbounds float addrspace(1)* %tmp68, i32 %tmp67 ; <float addrspace(1)*> [#uses=1]
  %tmp70 = load float addrspace(1)* %arrayidx69   ; <float> [#uses=1]
  store float %tmp70, float* %mi
  %tmp72 = load float* %mi                        ; <float> [#uses=1]
  %tmp73 = load float* %inverse_distance          ; <float> [#uses=1]
  %mul74 = fmul float %tmp72, %tmp73              ; <float> [#uses=1]
  %tmp75 = load float* %inverse_distance          ; <float> [#uses=1]
  %tmp76 = load float* %inverse_distance          ; <float> [#uses=1]
  %mul77 = fmul float %tmp75, %tmp76              ; <float> [#uses=1]
  %mul78 = fmul float %mul74, %mul77              ; <float> [#uses=1]
  store float %mul78, float* %s
  %tmp79 = load float* %dx                        ; <float> [#uses=1]
  %tmp80 = load float* %s                         ; <float> [#uses=1]
  %mul81 = fmul float %tmp79, %tmp80              ; <float> [#uses=1]
  %tmp82 = load float* %acc_x                     ; <float> [#uses=1]
  %add83 = fadd float %tmp82, %mul81              ; <float> [#uses=1]
  store float %add83, float* %acc_x
  %tmp84 = load float* %dy                        ; <float> [#uses=1]
  %tmp85 = load float* %s                         ; <float> [#uses=1]
  %mul86 = fmul float %tmp84, %tmp85              ; <float> [#uses=1]
  %tmp87 = load float* %acc_y                     ; <float> [#uses=1]
  %add88 = fadd float %tmp87, %mul86              ; <float> [#uses=1]
  store float %add88, float* %acc_y
  %tmp89 = load float* %dz                        ; <float> [#uses=1]
  %tmp90 = load float* %s                         ; <float> [#uses=1]
  %mul91 = fmul float %tmp89, %tmp90              ; <float> [#uses=1]
  %tmp92 = load float* %acc_z                     ; <float> [#uses=1]
  %add93 = fadd float %tmp92, %mul91              ; <float> [#uses=1]
  store float %add93, float* %acc_z
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp94 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp94, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp95 = load i32* %k                           ; <i32> [#uses=1]
  %tmp96 = load float addrspace(1)** %input_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx97 = getelementptr inbounds float addrspace(1)* %tmp96, i32 %tmp95 ; <float addrspace(1)*> [#uses=1]
  %tmp98 = load float addrspace(1)* %arrayidx97   ; <float> [#uses=1]
  %tmp99 = load float* %acc_x                     ; <float> [#uses=1]
  %tmp100 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul101 = fmul float %tmp99, %tmp100            ; <float> [#uses=1]
  %add102 = fadd float %tmp98, %mul101            ; <float> [#uses=1]
  %tmp103 = load i32* %k                          ; <i32> [#uses=1]
  %tmp104 = load float addrspace(1)** %output_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx105 = getelementptr inbounds float addrspace(1)* %tmp104, i32 %tmp103 ; <float addrspace(1)*> [#uses=1]
  store float %add102, float addrspace(1)* %arrayidx105
  %tmp106 = load i32* %k                          ; <i32> [#uses=1]
  %tmp107 = load float addrspace(1)** %input_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds float addrspace(1)* %tmp107, i32 %tmp106 ; <float addrspace(1)*> [#uses=1]
  %tmp109 = load float addrspace(1)* %arrayidx108 ; <float> [#uses=1]
  %tmp110 = load float* %acc_y                    ; <float> [#uses=1]
  %tmp111 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul112 = fmul float %tmp110, %tmp111           ; <float> [#uses=1]
  %add113 = fadd float %tmp109, %mul112           ; <float> [#uses=1]
  %tmp114 = load i32* %k                          ; <i32> [#uses=1]
  %tmp115 = load float addrspace(1)** %output_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx116 = getelementptr inbounds float addrspace(1)* %tmp115, i32 %tmp114 ; <float addrspace(1)*> [#uses=1]
  store float %add113, float addrspace(1)* %arrayidx116
  %tmp117 = load i32* %k                          ; <i32> [#uses=1]
  %tmp118 = load float addrspace(1)** %input_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx119 = getelementptr inbounds float addrspace(1)* %tmp118, i32 %tmp117 ; <float addrspace(1)*> [#uses=1]
  %tmp120 = load float addrspace(1)* %arrayidx119 ; <float> [#uses=1]
  %tmp121 = load float* %acc_z                    ; <float> [#uses=1]
  %tmp122 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul123 = fmul float %tmp121, %tmp122           ; <float> [#uses=1]
  %add124 = fadd float %tmp120, %mul123           ; <float> [#uses=1]
  %tmp125 = load i32* %k                          ; <i32> [#uses=1]
  %tmp126 = load float addrspace(1)** %output_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx127 = getelementptr inbounds float addrspace(1)* %tmp126, i32 %tmp125 ; <float addrspace(1)*> [#uses=1]
  store float %add124, float addrspace(1)* %arrayidx127
  %tmp128 = load i32* %k                          ; <i32> [#uses=1]
  %tmp129 = load float addrspace(1)** %in_position_x ; <float addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds float addrspace(1)* %tmp129, i32 %tmp128 ; <float addrspace(1)*> [#uses=1]
  %tmp131 = load float addrspace(1)* %arrayidx130 ; <float> [#uses=1]
  %tmp132 = load i32* %k                          ; <i32> [#uses=1]
  %tmp133 = load float addrspace(1)** %input_velocity_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds float addrspace(1)* %tmp133, i32 %tmp132 ; <float addrspace(1)*> [#uses=1]
  %tmp135 = load float addrspace(1)* %arrayidx134 ; <float> [#uses=1]
  %tmp136 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul137 = fmul float %tmp135, %tmp136           ; <float> [#uses=1]
  %add138 = fadd float %tmp131, %mul137           ; <float> [#uses=1]
  %tmp139 = load float* %acc_x                    ; <float> [#uses=1]
  %tmp140 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul141 = fmul float %tmp139, %tmp140           ; <float> [#uses=1]
  %tmp142 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul143 = fmul float %mul141, %tmp142           ; <float> [#uses=1]
  %div = fdiv float %mul143, 2.000000e+000        ; <float> [#uses=1]
  %add144 = fadd float %add138, %div              ; <float> [#uses=1]
  %tmp145 = load i32* %k                          ; <i32> [#uses=1]
  %tmp146 = load float addrspace(1)** %output_position_x.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx147 = getelementptr inbounds float addrspace(1)* %tmp146, i32 %tmp145 ; <float addrspace(1)*> [#uses=1]
  store float %add144, float addrspace(1)* %arrayidx147
  %tmp148 = load i32* %k                          ; <i32> [#uses=1]
  %tmp149 = load float addrspace(1)** %in_position_y ; <float addrspace(1)*> [#uses=1]
  %arrayidx150 = getelementptr inbounds float addrspace(1)* %tmp149, i32 %tmp148 ; <float addrspace(1)*> [#uses=1]
  %tmp151 = load float addrspace(1)* %arrayidx150 ; <float> [#uses=1]
  %tmp152 = load i32* %k                          ; <i32> [#uses=1]
  %tmp153 = load float addrspace(1)** %input_velocity_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx154 = getelementptr inbounds float addrspace(1)* %tmp153, i32 %tmp152 ; <float addrspace(1)*> [#uses=1]
  %tmp155 = load float addrspace(1)* %arrayidx154 ; <float> [#uses=1]
  %tmp156 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul157 = fmul float %tmp155, %tmp156           ; <float> [#uses=1]
  %add158 = fadd float %tmp151, %mul157           ; <float> [#uses=1]
  %tmp159 = load float* %acc_y                    ; <float> [#uses=1]
  %tmp160 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul161 = fmul float %tmp159, %tmp160           ; <float> [#uses=1]
  %tmp162 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul163 = fmul float %mul161, %tmp162           ; <float> [#uses=1]
  %div164 = fdiv float %mul163, 2.000000e+000     ; <float> [#uses=1]
  %add165 = fadd float %add158, %div164           ; <float> [#uses=1]
  %tmp166 = load i32* %k                          ; <i32> [#uses=1]
  %tmp167 = load float addrspace(1)** %output_position_y.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx168 = getelementptr inbounds float addrspace(1)* %tmp167, i32 %tmp166 ; <float addrspace(1)*> [#uses=1]
  store float %add165, float addrspace(1)* %arrayidx168
  %tmp169 = load i32* %k                          ; <i32> [#uses=1]
  %tmp170 = load float addrspace(1)** %in_position_z ; <float addrspace(1)*> [#uses=1]
  %arrayidx171 = getelementptr inbounds float addrspace(1)* %tmp170, i32 %tmp169 ; <float addrspace(1)*> [#uses=1]
  %tmp172 = load float addrspace(1)* %arrayidx171 ; <float> [#uses=1]
  %tmp173 = load i32* %k                          ; <i32> [#uses=1]
  %tmp174 = load float addrspace(1)** %input_velocity_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(1)* %tmp174, i32 %tmp173 ; <float addrspace(1)*> [#uses=1]
  %tmp176 = load float addrspace(1)* %arrayidx175 ; <float> [#uses=1]
  %tmp177 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul178 = fmul float %tmp176, %tmp177           ; <float> [#uses=1]
  %add179 = fadd float %tmp172, %mul178           ; <float> [#uses=1]
  %tmp180 = load float* %acc_z                    ; <float> [#uses=1]
  %tmp181 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul182 = fmul float %tmp180, %tmp181           ; <float> [#uses=1]
  %tmp183 = load float* %time_delta.addr          ; <float> [#uses=1]
  %mul184 = fmul float %mul182, %tmp183           ; <float> [#uses=1]
  %div185 = fdiv float %mul184, 2.000000e+000     ; <float> [#uses=1]
  %add186 = fadd float %add179, %div185           ; <float> [#uses=1]
  %tmp187 = load i32* %k                          ; <i32> [#uses=1]
  %tmp188 = load float addrspace(1)** %output_position_z.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx189 = getelementptr inbounds float addrspace(1)* %tmp188, i32 %tmp187 ; <float addrspace(1)*> [#uses=1]
  store float %add186, float addrspace(1)* %arrayidx189
  ret void
}

declare float @_Z5rsqrtf(float)
