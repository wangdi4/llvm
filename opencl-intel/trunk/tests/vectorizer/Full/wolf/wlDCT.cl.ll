; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlDCT.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_DCT_local_inter = internal addrspace(3) global [64 x float] zeroinitializer, align 4 ; <[64 x float] addrspace(3)*> [#uses=2]
@opencl_DCT_VECTOR_local_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer, align 32 ; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@opencl_DCT_VECTOR_DOT_local_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer, align 32 ; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@opencl_DCT_locals = appending global [2 x i8*] [i8* bitcast ([64 x float] addrspace(3)* @opencl_DCT_local_inter to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_DCT_parameters = appending global [140 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[140 x i8]*> [#uses=1]
@opencl_DCT_VECTOR_locals = appending global [2 x i8*] [i8* bitcast ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_DCT_VECTOR_parameters = appending global [143 x i8] c"float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[143 x i8]*> [#uses=1]
@opencl_DCT_VECTOR_DOT_locals = appending global [2 x i8*] [i8* bitcast ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter to i8*), i8* null], section "llvm.metadata" ; <[2 x i8*]*> [#uses=1]
@opencl_DCT_VECTOR_DOT_parameters = appending global [143 x i8] c"float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[143 x i8]*> [#uses=1]
@opencl_DCT_CPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_DCT_CPU_parameters = appending global [140 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[140 x i8]*> [#uses=1]
@opencl_DCT_CPU_VECTOR_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_DCT_CPU_VECTOR_parameters = appending global [142 x i8] c"float __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[142 x i8]*> [#uses=1]
@opencl_metadata = appending global [5 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @DCT to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_DCT_locals to i8*), i8* getelementptr inbounds ([140 x i8]* @opencl_DCT_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32)* @DCT_VECTOR to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_DCT_VECTOR_locals to i8*), i8* getelementptr inbounds ([143 x i8]* @opencl_DCT_VECTOR_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32)* @DCT_VECTOR_DOT to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([2 x i8*]* @opencl_DCT_VECTOR_DOT_locals to i8*), i8* getelementptr inbounds ([143 x i8]* @opencl_DCT_VECTOR_DOT_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @DCT_CPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_DCT_CPU_locals to i8*), i8* getelementptr inbounds ([140 x i8]* @opencl_DCT_CPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32)* @DCT_CPU_VECTOR to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_DCT_CPU_VECTOR_locals to i8*), i8* getelementptr inbounds ([142 x i8]* @opencl_DCT_CPU_VECTOR_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[5 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=9]
  %input.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %dct.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=1]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=1]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=3]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %k2 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %n1 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %n2 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=19]
  %index1 = alloca i32, align 4                   ; <i32*> [#uses=7]
  %index2 = alloca i32, align 4                   ; <i32*> [#uses=7]
  %acc = alloca [8 x float], align 4              ; <[8 x float]*> [#uses=20]
  %ind = alloca i32, align 4                      ; <i32*> [#uses=5]
  %ind94 = alloca i32, align 4                    ; <i32*> [#uses=5]
  %ind110 = alloca i32, align 4                   ; <i32*> [#uses=5]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store float addrspace(1)* %input, float addrspace(1)** %input.addr
  store float addrspace(1)* %dct, float addrspace(1)** %dct.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalIdx
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdy
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %groupIdx
  %call3 = call i32 @get_group_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call3, i32* %groupIdy
  %call4 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call4, i32* %i
  store i32 0, i32* %idx
  store i32 0, i32* %index1
  store i32 0, i32* %index2
  %tmp = bitcast [8 x float]* %acc to i8*         ; <i8*> [#uses=1]
  call void @llvm.memset.i32(i8* %tmp, i8 0, i32 32, i32 4)
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  store i32 %tmp5, i32* %k1
  %tmp6 = load i32* %k1                           ; <i32> [#uses=1]
  %mul = mul i32 %tmp6, 8                         ; <i32> [#uses=1]
  store i32 %mul, i32* %index1
  %tmp7 = load i32* %groupIdy                     ; <i32> [#uses=1]
  %mul8 = mul i32 %tmp7, 8                        ; <i32> [#uses=1]
  %tmp9 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul10 = mul i32 %mul8, %tmp9                   ; <i32> [#uses=1]
  %tmp11 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %mul12 = mul i32 %tmp11, 8                      ; <i32> [#uses=1]
  %add = add i32 %mul10, %mul12                   ; <i32> [#uses=1]
  store i32 %add, i32* %index2
  store i32 0, i32* %ind
  br label %for.cond

for.cond:                                         ; preds = %for.inc39, %entry
  %tmp14 = load i32* %ind                         ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp14, 8                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end42

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %n1
  br label %for.cond15

for.cond15:                                       ; preds = %for.inc, %for.body
  %tmp16 = load i32* %n1                          ; <i32> [#uses=1]
  %cmp17 = icmp ult i32 %tmp16, 8                 ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body18, label %for.end

for.body18:                                       ; preds = %for.cond15
  %tmp19 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp20 = load i32* %n1                          ; <i32> [#uses=1]
  %add21 = add i32 %tmp19, %tmp20                 ; <i32> [#uses=1]
  %tmp22 = load float addrspace(1)** %dct.addr    ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp22, i32 %add21 ; <float addrspace(1)*> [#uses=1]
  %tmp23 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %tmp24 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp25 = load i32* %n1                          ; <i32> [#uses=1]
  %add26 = add i32 %tmp24, %tmp25                 ; <i32> [#uses=1]
  %tmp27 = load float addrspace(1)** %input.addr  ; <float addrspace(1)*> [#uses=1]
  %arrayidx28 = getelementptr inbounds float addrspace(1)* %tmp27, i32 %add26 ; <float addrspace(1)*> [#uses=1]
  %tmp29 = load float addrspace(1)* %arrayidx28   ; <float> [#uses=1]
  %mul30 = fmul float %tmp23, %tmp29              ; <float> [#uses=1]
  %tmp31 = load i32* %ind                         ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx32 = getelementptr inbounds float* %arraydecay, i32 %tmp31 ; <float*> [#uses=2]
  %tmp33 = load float* %arrayidx32                ; <float> [#uses=1]
  %add34 = fadd float %tmp33, %mul30              ; <float> [#uses=1]
  store float %add34, float* %arrayidx32
  br label %for.inc

for.inc:                                          ; preds = %for.body18
  %tmp35 = load i32* %n1                          ; <i32> [#uses=1]
  %inc = add i32 %tmp35, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %n1
  br label %for.cond15

for.end:                                          ; preds = %for.cond15
  %tmp36 = load i32* %width.addr                  ; <i32> [#uses=1]
  %tmp37 = load i32* %index2                      ; <i32> [#uses=1]
  %add38 = add i32 %tmp37, %tmp36                 ; <i32> [#uses=1]
  store i32 %add38, i32* %index2
  br label %for.inc39

for.inc39:                                        ; preds = %for.end
  %tmp40 = load i32* %ind                         ; <i32> [#uses=1]
  %inc41 = add nsw i32 %tmp40, 1                  ; <i32> [#uses=1]
  store i32 %inc41, i32* %ind
  br label %for.cond

for.end42:                                        ; preds = %for.cond
  %tmp43 = load i32* %k1                          ; <i32> [#uses=1]
  %mul44 = mul i32 %tmp43, 8                      ; <i32> [#uses=1]
  store i32 %mul44, i32* %idx
  %arraydecay45 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float* %arraydecay45, i32 0 ; <float*> [#uses=1]
  %tmp47 = load float* %arrayidx46                ; <float> [#uses=1]
  %tmp48 = load i32* %idx                         ; <i32> [#uses=1]
  %add49 = add i32 %tmp48, 0                      ; <i32> [#uses=1]
  %arrayidx50 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add49 ; <float addrspace(3)*> [#uses=1]
  store float %tmp47, float addrspace(3)* %arrayidx50
  %arraydecay51 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx52 = getelementptr inbounds float* %arraydecay51, i32 1 ; <float*> [#uses=1]
  %tmp53 = load float* %arrayidx52                ; <float> [#uses=1]
  %tmp54 = load i32* %idx                         ; <i32> [#uses=1]
  %add55 = add i32 %tmp54, 1                      ; <i32> [#uses=1]
  %arrayidx56 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add55 ; <float addrspace(3)*> [#uses=1]
  store float %tmp53, float addrspace(3)* %arrayidx56
  %arraydecay57 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx58 = getelementptr inbounds float* %arraydecay57, i32 2 ; <float*> [#uses=1]
  %tmp59 = load float* %arrayidx58                ; <float> [#uses=1]
  %tmp60 = load i32* %idx                         ; <i32> [#uses=1]
  %add61 = add i32 %tmp60, 2                      ; <i32> [#uses=1]
  %arrayidx62 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add61 ; <float addrspace(3)*> [#uses=1]
  store float %tmp59, float addrspace(3)* %arrayidx62
  %arraydecay63 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx64 = getelementptr inbounds float* %arraydecay63, i32 3 ; <float*> [#uses=1]
  %tmp65 = load float* %arrayidx64                ; <float> [#uses=1]
  %tmp66 = load i32* %idx                         ; <i32> [#uses=1]
  %add67 = add i32 %tmp66, 3                      ; <i32> [#uses=1]
  %arrayidx68 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add67 ; <float addrspace(3)*> [#uses=1]
  store float %tmp65, float addrspace(3)* %arrayidx68
  %arraydecay69 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx70 = getelementptr inbounds float* %arraydecay69, i32 4 ; <float*> [#uses=1]
  %tmp71 = load float* %arrayidx70                ; <float> [#uses=1]
  %tmp72 = load i32* %idx                         ; <i32> [#uses=1]
  %add73 = add i32 %tmp72, 4                      ; <i32> [#uses=1]
  %arrayidx74 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add73 ; <float addrspace(3)*> [#uses=1]
  store float %tmp71, float addrspace(3)* %arrayidx74
  %arraydecay75 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx76 = getelementptr inbounds float* %arraydecay75, i32 5 ; <float*> [#uses=1]
  %tmp77 = load float* %arrayidx76                ; <float> [#uses=1]
  %tmp78 = load i32* %idx                         ; <i32> [#uses=1]
  %add79 = add i32 %tmp78, 5                      ; <i32> [#uses=1]
  %arrayidx80 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add79 ; <float addrspace(3)*> [#uses=1]
  store float %tmp77, float addrspace(3)* %arrayidx80
  %arraydecay81 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx82 = getelementptr inbounds float* %arraydecay81, i32 6 ; <float*> [#uses=1]
  %tmp83 = load float* %arrayidx82                ; <float> [#uses=1]
  %tmp84 = load i32* %idx                         ; <i32> [#uses=1]
  %add85 = add i32 %tmp84, 6                      ; <i32> [#uses=1]
  %arrayidx86 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add85 ; <float addrspace(3)*> [#uses=1]
  store float %tmp83, float addrspace(3)* %arrayidx86
  %arraydecay87 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx88 = getelementptr inbounds float* %arraydecay87, i32 7 ; <float*> [#uses=1]
  %tmp89 = load float* %arrayidx88                ; <float> [#uses=1]
  %tmp90 = load i32* %idx                         ; <i32> [#uses=1]
  %add91 = add i32 %tmp90, 7                      ; <i32> [#uses=1]
  %arrayidx92 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add91 ; <float addrspace(3)*> [#uses=1]
  store float %tmp89, float addrspace(3)* %arrayidx92
  call void @barrier(i32 1)
  store i32 0, i32* %ind94
  br label %for.cond95

for.cond95:                                       ; preds = %for.inc102, %for.end42
  %tmp96 = load i32* %ind94                       ; <i32> [#uses=1]
  %cmp97 = icmp slt i32 %tmp96, 8                 ; <i1> [#uses=1]
  br i1 %cmp97, label %for.body98, label %for.end105

for.body98:                                       ; preds = %for.cond95
  %tmp99 = load i32* %ind94                       ; <i32> [#uses=1]
  %arraydecay100 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx101 = getelementptr inbounds float* %arraydecay100, i32 %tmp99 ; <float*> [#uses=1]
  store float 0.000000e+000, float* %arrayidx101
  br label %for.inc102

for.inc102:                                       ; preds = %for.body98
  %tmp103 = load i32* %ind94                      ; <i32> [#uses=1]
  %inc104 = add nsw i32 %tmp103, 1                ; <i32> [#uses=1]
  store i32 %inc104, i32* %ind94
  br label %for.cond95

for.end105:                                       ; preds = %for.cond95
  %tmp106 = load i32* %i                          ; <i32> [#uses=1]
  store i32 %tmp106, i32* %k2
  store i32 0, i32* %index1
  %tmp107 = load i32* %k2                         ; <i32> [#uses=1]
  %mul108 = mul i32 %tmp107, 8                    ; <i32> [#uses=1]
  store i32 %mul108, i32* %index2
  store i32 0, i32* %ind110
  br label %for.cond111

for.cond111:                                      ; preds = %for.inc142, %for.end105
  %tmp112 = load i32* %ind110                     ; <i32> [#uses=1]
  %cmp113 = icmp slt i32 %tmp112, 8               ; <i1> [#uses=1]
  br i1 %cmp113, label %for.body114, label %for.end145

for.body114:                                      ; preds = %for.cond111
  store i32 0, i32* %n2
  br label %for.cond115

for.cond115:                                      ; preds = %for.inc136, %for.body114
  %tmp116 = load i32* %n2                         ; <i32> [#uses=1]
  %cmp117 = icmp ult i32 %tmp116, 8               ; <i1> [#uses=1]
  br i1 %cmp117, label %for.body118, label %for.end139

for.body118:                                      ; preds = %for.cond115
  %tmp119 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp120 = load i32* %n2                         ; <i32> [#uses=1]
  %add121 = add i32 %tmp119, %tmp120              ; <i32> [#uses=1]
  %arrayidx122 = getelementptr inbounds float addrspace(3)* getelementptr inbounds ([64 x float] addrspace(3)* @opencl_DCT_local_inter, i32 0, i32 0), i32 %add121 ; <float addrspace(3)*> [#uses=1]
  %tmp123 = load float addrspace(3)* %arrayidx122 ; <float> [#uses=1]
  %tmp124 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp125 = load i32* %n2                         ; <i32> [#uses=1]
  %add126 = add i32 %tmp124, %tmp125              ; <i32> [#uses=1]
  %tmp127 = load float addrspace(1)** %dct.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx128 = getelementptr inbounds float addrspace(1)* %tmp127, i32 %add126 ; <float addrspace(1)*> [#uses=1]
  %tmp129 = load float addrspace(1)* %arrayidx128 ; <float> [#uses=1]
  %mul130 = fmul float %tmp123, %tmp129           ; <float> [#uses=1]
  %tmp131 = load i32* %ind110                     ; <i32> [#uses=1]
  %arraydecay132 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx133 = getelementptr inbounds float* %arraydecay132, i32 %tmp131 ; <float*> [#uses=2]
  %tmp134 = load float* %arrayidx133              ; <float> [#uses=1]
  %add135 = fadd float %tmp134, %mul130           ; <float> [#uses=1]
  store float %add135, float* %arrayidx133
  br label %for.inc136

for.inc136:                                       ; preds = %for.body118
  %tmp137 = load i32* %n2                         ; <i32> [#uses=1]
  %inc138 = add i32 %tmp137, 1                    ; <i32> [#uses=1]
  store i32 %inc138, i32* %n2
  br label %for.cond115

for.end139:                                       ; preds = %for.cond115
  %tmp140 = load i32* %index1                     ; <i32> [#uses=1]
  %add141 = add i32 %tmp140, 8                    ; <i32> [#uses=1]
  store i32 %add141, i32* %index1
  br label %for.inc142

for.inc142:                                       ; preds = %for.end139
  %tmp143 = load i32* %ind110                     ; <i32> [#uses=1]
  %inc144 = add nsw i32 %tmp143, 1                ; <i32> [#uses=1]
  store i32 %inc144, i32* %ind110
  br label %for.cond111

for.end145:                                       ; preds = %for.cond111
  %tmp146 = load i32* %groupIdy                   ; <i32> [#uses=1]
  %mul147 = mul i32 %tmp146, 8                    ; <i32> [#uses=1]
  %tmp148 = load i32* %k2                         ; <i32> [#uses=1]
  %add149 = add i32 %mul147, %tmp148              ; <i32> [#uses=1]
  %tmp150 = load i32* %width.addr                 ; <i32> [#uses=1]
  %mul151 = mul i32 %add149, %tmp150              ; <i32> [#uses=1]
  %tmp152 = load i32* %groupIdx                   ; <i32> [#uses=1]
  %mul153 = mul i32 %tmp152, 8                    ; <i32> [#uses=1]
  %add154 = add i32 %mul151, %mul153              ; <i32> [#uses=1]
  store i32 %add154, i32* %idx
  %arraydecay155 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx156 = getelementptr inbounds float* %arraydecay155, i32 0 ; <float*> [#uses=1]
  %tmp157 = load float* %arrayidx156              ; <float> [#uses=1]
  %tmp158 = load i32* %idx                        ; <i32> [#uses=1]
  %add159 = add i32 %tmp158, 0                    ; <i32> [#uses=1]
  %tmp160 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx161 = getelementptr inbounds float addrspace(1)* %tmp160, i32 %add159 ; <float addrspace(1)*> [#uses=1]
  store float %tmp157, float addrspace(1)* %arrayidx161
  %arraydecay162 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx163 = getelementptr inbounds float* %arraydecay162, i32 1 ; <float*> [#uses=1]
  %tmp164 = load float* %arrayidx163              ; <float> [#uses=1]
  %tmp165 = load i32* %idx                        ; <i32> [#uses=1]
  %add166 = add i32 %tmp165, 1                    ; <i32> [#uses=1]
  %tmp167 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx168 = getelementptr inbounds float addrspace(1)* %tmp167, i32 %add166 ; <float addrspace(1)*> [#uses=1]
  store float %tmp164, float addrspace(1)* %arrayidx168
  %arraydecay169 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx170 = getelementptr inbounds float* %arraydecay169, i32 2 ; <float*> [#uses=1]
  %tmp171 = load float* %arrayidx170              ; <float> [#uses=1]
  %tmp172 = load i32* %idx                        ; <i32> [#uses=1]
  %add173 = add i32 %tmp172, 2                    ; <i32> [#uses=1]
  %tmp174 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(1)* %tmp174, i32 %add173 ; <float addrspace(1)*> [#uses=1]
  store float %tmp171, float addrspace(1)* %arrayidx175
  %arraydecay176 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx177 = getelementptr inbounds float* %arraydecay176, i32 3 ; <float*> [#uses=1]
  %tmp178 = load float* %arrayidx177              ; <float> [#uses=1]
  %tmp179 = load i32* %idx                        ; <i32> [#uses=1]
  %add180 = add i32 %tmp179, 3                    ; <i32> [#uses=1]
  %tmp181 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx182 = getelementptr inbounds float addrspace(1)* %tmp181, i32 %add180 ; <float addrspace(1)*> [#uses=1]
  store float %tmp178, float addrspace(1)* %arrayidx182
  %arraydecay183 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx184 = getelementptr inbounds float* %arraydecay183, i32 4 ; <float*> [#uses=1]
  %tmp185 = load float* %arrayidx184              ; <float> [#uses=1]
  %tmp186 = load i32* %idx                        ; <i32> [#uses=1]
  %add187 = add i32 %tmp186, 4                    ; <i32> [#uses=1]
  %tmp188 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx189 = getelementptr inbounds float addrspace(1)* %tmp188, i32 %add187 ; <float addrspace(1)*> [#uses=1]
  store float %tmp185, float addrspace(1)* %arrayidx189
  %arraydecay190 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx191 = getelementptr inbounds float* %arraydecay190, i32 5 ; <float*> [#uses=1]
  %tmp192 = load float* %arrayidx191              ; <float> [#uses=1]
  %tmp193 = load i32* %idx                        ; <i32> [#uses=1]
  %add194 = add i32 %tmp193, 5                    ; <i32> [#uses=1]
  %tmp195 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx196 = getelementptr inbounds float addrspace(1)* %tmp195, i32 %add194 ; <float addrspace(1)*> [#uses=1]
  store float %tmp192, float addrspace(1)* %arrayidx196
  %arraydecay197 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx198 = getelementptr inbounds float* %arraydecay197, i32 6 ; <float*> [#uses=1]
  %tmp199 = load float* %arrayidx198              ; <float> [#uses=1]
  %tmp200 = load i32* %idx                        ; <i32> [#uses=1]
  %add201 = add i32 %tmp200, 6                    ; <i32> [#uses=1]
  %tmp202 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx203 = getelementptr inbounds float addrspace(1)* %tmp202, i32 %add201 ; <float addrspace(1)*> [#uses=1]
  store float %tmp199, float addrspace(1)* %arrayidx203
  %arraydecay204 = getelementptr inbounds [8 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx205 = getelementptr inbounds float* %arraydecay204, i32 7 ; <float*> [#uses=1]
  %tmp206 = load float* %arrayidx205              ; <float> [#uses=1]
  %tmp207 = load i32* %idx                        ; <i32> [#uses=1]
  %add208 = add i32 %tmp207, 7                    ; <i32> [#uses=1]
  %tmp209 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx210 = getelementptr inbounds float addrspace(1)* %tmp209, i32 %add208 ; <float addrspace(1)*> [#uses=1]
  store float %tmp206, float addrspace(1)* %arrayidx210
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @llvm.memset.i32(i8* nocapture, i8, i32, i32) nounwind

declare void @barrier(i32)

; CHECK: ret
define void @DCT_VECTOR(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width) nounwind {
entry:
  %output.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=2]
  %input.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=9]
  %dct.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=17]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=1]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=1]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=3]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %k2 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %n1 = alloca i32, align 4                       ; <i32*> [#uses=0]
  %n2 = alloca i32, align 4                       ; <i32*> [#uses=0]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=5]
  %acc = alloca <8 x float>, align 32             ; <<8 x float>*> [#uses=35]
  %temp = alloca <8 x float>, align 32            ; <<8 x float>*> [#uses=144]
  %step = alloca i32, align 4                     ; <i32*> [#uses=10]
  %index1 = alloca i32, align 4                   ; <i32*> [#uses=9]
  %index2 = alloca i32, align 4                   ; <i32*> [#uses=32]
  store <8 x float> addrspace(1)* %output, <8 x float> addrspace(1)** %output.addr
  store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
  store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalIdx
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdy
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %groupIdx
  %call3 = call i32 @get_group_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call3, i32* %groupIdy
  %call4 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call4, i32* %i
  store i32 0, i32* %idx
  store <8 x float> zeroinitializer, <8 x float>* %acc
  %tmp = load i32* %width.addr                    ; <i32> [#uses=1]
  %div = udiv i32 %tmp, 8                         ; <i32> [#uses=1]
  store i32 %div, i32* %step
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  store i32 %tmp5, i32* %k1
  %tmp7 = load i32* %k1                           ; <i32> [#uses=1]
  store i32 %tmp7, i32* %index1
  %tmp9 = load i32* %groupIdy                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp9, 8                         ; <i32> [#uses=1]
  %tmp10 = load i32* %step                        ; <i32> [#uses=1]
  %mul11 = mul i32 %mul, %tmp10                   ; <i32> [#uses=1]
  %tmp12 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %add = add i32 %mul11, %tmp12                   ; <i32> [#uses=1]
  store i32 %add, i32* %index2
  %tmp13 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp14 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x float> addrspace(1)* %tmp14, i32 %tmp13 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp15 = load <8 x float> addrspace(1)* %arrayidx ; <<8 x float>> [#uses=1]
  %tmp16 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp17 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds <8 x float> addrspace(1)* %tmp17, i32 %tmp16 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp19 = load <8 x float> addrspace(1)* %arrayidx18 ; <<8 x float>> [#uses=1]
  %mul20 = fmul <8 x float> %tmp15, %tmp19        ; <<8 x float>> [#uses=1]
  store <8 x float> %mul20, <8 x float>* %temp
  %tmp21 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp22 = extractelement <8 x float> %tmp21, i32 0 ; <float> [#uses=1]
  %tmp23 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp24 = extractelement <8 x float> %tmp23, i32 1 ; <float> [#uses=1]
  %add25 = fadd float %tmp22, %tmp24              ; <float> [#uses=1]
  %tmp26 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp27 = extractelement <8 x float> %tmp26, i32 2 ; <float> [#uses=1]
  %add28 = fadd float %add25, %tmp27              ; <float> [#uses=1]
  %tmp29 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp30 = extractelement <8 x float> %tmp29, i32 3 ; <float> [#uses=1]
  %add31 = fadd float %add28, %tmp30              ; <float> [#uses=1]
  %tmp32 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp33 = extractelement <8 x float> %tmp32, i32 4 ; <float> [#uses=1]
  %add34 = fadd float %add31, %tmp33              ; <float> [#uses=1]
  %tmp35 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp36 = extractelement <8 x float> %tmp35, i32 5 ; <float> [#uses=1]
  %add37 = fadd float %add34, %tmp36              ; <float> [#uses=1]
  %tmp38 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp39 = extractelement <8 x float> %tmp38, i32 6 ; <float> [#uses=1]
  %add40 = fadd float %add37, %tmp39              ; <float> [#uses=1]
  %tmp41 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp42 = extractelement <8 x float> %tmp41, i32 7 ; <float> [#uses=1]
  %add43 = fadd float %add40, %tmp42              ; <float> [#uses=1]
  %tmp44 = load <8 x float>* %acc                 ; <<8 x float>> [#uses=1]
  %tmp45 = insertelement <8 x float> %tmp44, float %add43, i32 0 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp45, <8 x float>* %acc
  %tmp46 = load i32* %step                        ; <i32> [#uses=1]
  %tmp47 = load i32* %index2                      ; <i32> [#uses=1]
  %add48 = add i32 %tmp47, %tmp46                 ; <i32> [#uses=1]
  store i32 %add48, i32* %index2
  %tmp49 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp50 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx51 = getelementptr inbounds <8 x float> addrspace(1)* %tmp50, i32 %tmp49 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp52 = load <8 x float> addrspace(1)* %arrayidx51 ; <<8 x float>> [#uses=1]
  %tmp53 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp54 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx55 = getelementptr inbounds <8 x float> addrspace(1)* %tmp54, i32 %tmp53 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp56 = load <8 x float> addrspace(1)* %arrayidx55 ; <<8 x float>> [#uses=1]
  %mul57 = fmul <8 x float> %tmp52, %tmp56        ; <<8 x float>> [#uses=1]
  store <8 x float> %mul57, <8 x float>* %temp
  %tmp58 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp59 = extractelement <8 x float> %tmp58, i32 0 ; <float> [#uses=1]
  %tmp60 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp61 = extractelement <8 x float> %tmp60, i32 1 ; <float> [#uses=1]
  %add62 = fadd float %tmp59, %tmp61              ; <float> [#uses=1]
  %tmp63 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp64 = extractelement <8 x float> %tmp63, i32 2 ; <float> [#uses=1]
  %add65 = fadd float %add62, %tmp64              ; <float> [#uses=1]
  %tmp66 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp67 = extractelement <8 x float> %tmp66, i32 3 ; <float> [#uses=1]
  %add68 = fadd float %add65, %tmp67              ; <float> [#uses=1]
  %tmp69 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp70 = extractelement <8 x float> %tmp69, i32 4 ; <float> [#uses=1]
  %add71 = fadd float %add68, %tmp70              ; <float> [#uses=1]
  %tmp72 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp73 = extractelement <8 x float> %tmp72, i32 5 ; <float> [#uses=1]
  %add74 = fadd float %add71, %tmp73              ; <float> [#uses=1]
  %tmp75 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp76 = extractelement <8 x float> %tmp75, i32 6 ; <float> [#uses=1]
  %add77 = fadd float %add74, %tmp76              ; <float> [#uses=1]
  %tmp78 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp79 = extractelement <8 x float> %tmp78, i32 7 ; <float> [#uses=1]
  %add80 = fadd float %add77, %tmp79              ; <float> [#uses=1]
  %tmp81 = load <8 x float>* %acc                 ; <<8 x float>> [#uses=1]
  %tmp82 = insertelement <8 x float> %tmp81, float %add80, i32 1 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp82, <8 x float>* %acc
  %tmp83 = load i32* %step                        ; <i32> [#uses=1]
  %tmp84 = load i32* %index2                      ; <i32> [#uses=1]
  %add85 = add i32 %tmp84, %tmp83                 ; <i32> [#uses=1]
  store i32 %add85, i32* %index2
  %tmp86 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp87 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx88 = getelementptr inbounds <8 x float> addrspace(1)* %tmp87, i32 %tmp86 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp89 = load <8 x float> addrspace(1)* %arrayidx88 ; <<8 x float>> [#uses=1]
  %tmp90 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp91 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx92 = getelementptr inbounds <8 x float> addrspace(1)* %tmp91, i32 %tmp90 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp93 = load <8 x float> addrspace(1)* %arrayidx92 ; <<8 x float>> [#uses=1]
  %mul94 = fmul <8 x float> %tmp89, %tmp93        ; <<8 x float>> [#uses=1]
  store <8 x float> %mul94, <8 x float>* %temp
  %tmp95 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp96 = extractelement <8 x float> %tmp95, i32 0 ; <float> [#uses=1]
  %tmp97 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp98 = extractelement <8 x float> %tmp97, i32 1 ; <float> [#uses=1]
  %add99 = fadd float %tmp96, %tmp98              ; <float> [#uses=1]
  %tmp100 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp101 = extractelement <8 x float> %tmp100, i32 2 ; <float> [#uses=1]
  %add102 = fadd float %add99, %tmp101            ; <float> [#uses=1]
  %tmp103 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp104 = extractelement <8 x float> %tmp103, i32 3 ; <float> [#uses=1]
  %add105 = fadd float %add102, %tmp104           ; <float> [#uses=1]
  %tmp106 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp107 = extractelement <8 x float> %tmp106, i32 4 ; <float> [#uses=1]
  %add108 = fadd float %add105, %tmp107           ; <float> [#uses=1]
  %tmp109 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp110 = extractelement <8 x float> %tmp109, i32 5 ; <float> [#uses=1]
  %add111 = fadd float %add108, %tmp110           ; <float> [#uses=1]
  %tmp112 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp113 = extractelement <8 x float> %tmp112, i32 6 ; <float> [#uses=1]
  %add114 = fadd float %add111, %tmp113           ; <float> [#uses=1]
  %tmp115 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp116 = extractelement <8 x float> %tmp115, i32 7 ; <float> [#uses=1]
  %add117 = fadd float %add114, %tmp116           ; <float> [#uses=1]
  %tmp118 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp119 = insertelement <8 x float> %tmp118, float %add117, i32 2 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp119, <8 x float>* %acc
  %tmp120 = load i32* %step                       ; <i32> [#uses=1]
  %tmp121 = load i32* %index2                     ; <i32> [#uses=1]
  %add122 = add i32 %tmp121, %tmp120              ; <i32> [#uses=1]
  store i32 %add122, i32* %index2
  %tmp123 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp124 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx125 = getelementptr inbounds <8 x float> addrspace(1)* %tmp124, i32 %tmp123 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp126 = load <8 x float> addrspace(1)* %arrayidx125 ; <<8 x float>> [#uses=1]
  %tmp127 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp128 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx129 = getelementptr inbounds <8 x float> addrspace(1)* %tmp128, i32 %tmp127 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp130 = load <8 x float> addrspace(1)* %arrayidx129 ; <<8 x float>> [#uses=1]
  %mul131 = fmul <8 x float> %tmp126, %tmp130     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul131, <8 x float>* %temp
  %tmp132 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp133 = extractelement <8 x float> %tmp132, i32 0 ; <float> [#uses=1]
  %tmp134 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp135 = extractelement <8 x float> %tmp134, i32 1 ; <float> [#uses=1]
  %add136 = fadd float %tmp133, %tmp135           ; <float> [#uses=1]
  %tmp137 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp138 = extractelement <8 x float> %tmp137, i32 2 ; <float> [#uses=1]
  %add139 = fadd float %add136, %tmp138           ; <float> [#uses=1]
  %tmp140 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp141 = extractelement <8 x float> %tmp140, i32 3 ; <float> [#uses=1]
  %add142 = fadd float %add139, %tmp141           ; <float> [#uses=1]
  %tmp143 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp144 = extractelement <8 x float> %tmp143, i32 4 ; <float> [#uses=1]
  %add145 = fadd float %add142, %tmp144           ; <float> [#uses=1]
  %tmp146 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp147 = extractelement <8 x float> %tmp146, i32 5 ; <float> [#uses=1]
  %add148 = fadd float %add145, %tmp147           ; <float> [#uses=1]
  %tmp149 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp150 = extractelement <8 x float> %tmp149, i32 6 ; <float> [#uses=1]
  %add151 = fadd float %add148, %tmp150           ; <float> [#uses=1]
  %tmp152 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp153 = extractelement <8 x float> %tmp152, i32 7 ; <float> [#uses=1]
  %add154 = fadd float %add151, %tmp153           ; <float> [#uses=1]
  %tmp155 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp156 = insertelement <8 x float> %tmp155, float %add154, i32 3 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp156, <8 x float>* %acc
  %tmp157 = load i32* %step                       ; <i32> [#uses=1]
  %tmp158 = load i32* %index2                     ; <i32> [#uses=1]
  %add159 = add i32 %tmp158, %tmp157              ; <i32> [#uses=1]
  store i32 %add159, i32* %index2
  %tmp160 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp161 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx162 = getelementptr inbounds <8 x float> addrspace(1)* %tmp161, i32 %tmp160 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp163 = load <8 x float> addrspace(1)* %arrayidx162 ; <<8 x float>> [#uses=1]
  %tmp164 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp165 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx166 = getelementptr inbounds <8 x float> addrspace(1)* %tmp165, i32 %tmp164 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp167 = load <8 x float> addrspace(1)* %arrayidx166 ; <<8 x float>> [#uses=1]
  %mul168 = fmul <8 x float> %tmp163, %tmp167     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul168, <8 x float>* %temp
  %tmp169 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp170 = extractelement <8 x float> %tmp169, i32 0 ; <float> [#uses=1]
  %tmp171 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp172 = extractelement <8 x float> %tmp171, i32 1 ; <float> [#uses=1]
  %add173 = fadd float %tmp170, %tmp172           ; <float> [#uses=1]
  %tmp174 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp175 = extractelement <8 x float> %tmp174, i32 2 ; <float> [#uses=1]
  %add176 = fadd float %add173, %tmp175           ; <float> [#uses=1]
  %tmp177 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp178 = extractelement <8 x float> %tmp177, i32 3 ; <float> [#uses=1]
  %add179 = fadd float %add176, %tmp178           ; <float> [#uses=1]
  %tmp180 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp181 = extractelement <8 x float> %tmp180, i32 4 ; <float> [#uses=1]
  %add182 = fadd float %add179, %tmp181           ; <float> [#uses=1]
  %tmp183 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp184 = extractelement <8 x float> %tmp183, i32 5 ; <float> [#uses=1]
  %add185 = fadd float %add182, %tmp184           ; <float> [#uses=1]
  %tmp186 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp187 = extractelement <8 x float> %tmp186, i32 6 ; <float> [#uses=1]
  %add188 = fadd float %add185, %tmp187           ; <float> [#uses=1]
  %tmp189 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp190 = extractelement <8 x float> %tmp189, i32 7 ; <float> [#uses=1]
  %add191 = fadd float %add188, %tmp190           ; <float> [#uses=1]
  %tmp192 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp193 = insertelement <8 x float> %tmp192, float %add191, i32 4 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp193, <8 x float>* %acc
  %tmp194 = load i32* %step                       ; <i32> [#uses=1]
  %tmp195 = load i32* %index2                     ; <i32> [#uses=1]
  %add196 = add i32 %tmp195, %tmp194              ; <i32> [#uses=1]
  store i32 %add196, i32* %index2
  %tmp197 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp198 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx199 = getelementptr inbounds <8 x float> addrspace(1)* %tmp198, i32 %tmp197 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp200 = load <8 x float> addrspace(1)* %arrayidx199 ; <<8 x float>> [#uses=1]
  %tmp201 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp202 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx203 = getelementptr inbounds <8 x float> addrspace(1)* %tmp202, i32 %tmp201 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp204 = load <8 x float> addrspace(1)* %arrayidx203 ; <<8 x float>> [#uses=1]
  %mul205 = fmul <8 x float> %tmp200, %tmp204     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul205, <8 x float>* %temp
  %tmp206 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp207 = extractelement <8 x float> %tmp206, i32 0 ; <float> [#uses=1]
  %tmp208 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp209 = extractelement <8 x float> %tmp208, i32 1 ; <float> [#uses=1]
  %add210 = fadd float %tmp207, %tmp209           ; <float> [#uses=1]
  %tmp211 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp212 = extractelement <8 x float> %tmp211, i32 2 ; <float> [#uses=1]
  %add213 = fadd float %add210, %tmp212           ; <float> [#uses=1]
  %tmp214 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp215 = extractelement <8 x float> %tmp214, i32 3 ; <float> [#uses=1]
  %add216 = fadd float %add213, %tmp215           ; <float> [#uses=1]
  %tmp217 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp218 = extractelement <8 x float> %tmp217, i32 4 ; <float> [#uses=1]
  %add219 = fadd float %add216, %tmp218           ; <float> [#uses=1]
  %tmp220 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp221 = extractelement <8 x float> %tmp220, i32 5 ; <float> [#uses=1]
  %add222 = fadd float %add219, %tmp221           ; <float> [#uses=1]
  %tmp223 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp224 = extractelement <8 x float> %tmp223, i32 6 ; <float> [#uses=1]
  %add225 = fadd float %add222, %tmp224           ; <float> [#uses=1]
  %tmp226 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp227 = extractelement <8 x float> %tmp226, i32 7 ; <float> [#uses=1]
  %add228 = fadd float %add225, %tmp227           ; <float> [#uses=1]
  %tmp229 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp230 = insertelement <8 x float> %tmp229, float %add228, i32 5 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp230, <8 x float>* %acc
  %tmp231 = load i32* %step                       ; <i32> [#uses=1]
  %tmp232 = load i32* %index2                     ; <i32> [#uses=1]
  %add233 = add i32 %tmp232, %tmp231              ; <i32> [#uses=1]
  store i32 %add233, i32* %index2
  %tmp234 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp235 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx236 = getelementptr inbounds <8 x float> addrspace(1)* %tmp235, i32 %tmp234 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp237 = load <8 x float> addrspace(1)* %arrayidx236 ; <<8 x float>> [#uses=1]
  %tmp238 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp239 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx240 = getelementptr inbounds <8 x float> addrspace(1)* %tmp239, i32 %tmp238 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp241 = load <8 x float> addrspace(1)* %arrayidx240 ; <<8 x float>> [#uses=1]
  %mul242 = fmul <8 x float> %tmp237, %tmp241     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul242, <8 x float>* %temp
  %tmp243 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp244 = extractelement <8 x float> %tmp243, i32 0 ; <float> [#uses=1]
  %tmp245 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp246 = extractelement <8 x float> %tmp245, i32 1 ; <float> [#uses=1]
  %add247 = fadd float %tmp244, %tmp246           ; <float> [#uses=1]
  %tmp248 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp249 = extractelement <8 x float> %tmp248, i32 2 ; <float> [#uses=1]
  %add250 = fadd float %add247, %tmp249           ; <float> [#uses=1]
  %tmp251 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp252 = extractelement <8 x float> %tmp251, i32 3 ; <float> [#uses=1]
  %add253 = fadd float %add250, %tmp252           ; <float> [#uses=1]
  %tmp254 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp255 = extractelement <8 x float> %tmp254, i32 4 ; <float> [#uses=1]
  %add256 = fadd float %add253, %tmp255           ; <float> [#uses=1]
  %tmp257 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp258 = extractelement <8 x float> %tmp257, i32 5 ; <float> [#uses=1]
  %add259 = fadd float %add256, %tmp258           ; <float> [#uses=1]
  %tmp260 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp261 = extractelement <8 x float> %tmp260, i32 6 ; <float> [#uses=1]
  %add262 = fadd float %add259, %tmp261           ; <float> [#uses=1]
  %tmp263 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp264 = extractelement <8 x float> %tmp263, i32 7 ; <float> [#uses=1]
  %add265 = fadd float %add262, %tmp264           ; <float> [#uses=1]
  %tmp266 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp267 = insertelement <8 x float> %tmp266, float %add265, i32 6 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp267, <8 x float>* %acc
  %tmp268 = load i32* %step                       ; <i32> [#uses=1]
  %tmp269 = load i32* %index2                     ; <i32> [#uses=1]
  %add270 = add i32 %tmp269, %tmp268              ; <i32> [#uses=1]
  store i32 %add270, i32* %index2
  %tmp271 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp272 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx273 = getelementptr inbounds <8 x float> addrspace(1)* %tmp272, i32 %tmp271 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp274 = load <8 x float> addrspace(1)* %arrayidx273 ; <<8 x float>> [#uses=1]
  %tmp275 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp276 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx277 = getelementptr inbounds <8 x float> addrspace(1)* %tmp276, i32 %tmp275 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp278 = load <8 x float> addrspace(1)* %arrayidx277 ; <<8 x float>> [#uses=1]
  %mul279 = fmul <8 x float> %tmp274, %tmp278     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul279, <8 x float>* %temp
  %tmp280 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp281 = extractelement <8 x float> %tmp280, i32 0 ; <float> [#uses=1]
  %tmp282 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp283 = extractelement <8 x float> %tmp282, i32 1 ; <float> [#uses=1]
  %add284 = fadd float %tmp281, %tmp283           ; <float> [#uses=1]
  %tmp285 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp286 = extractelement <8 x float> %tmp285, i32 2 ; <float> [#uses=1]
  %add287 = fadd float %add284, %tmp286           ; <float> [#uses=1]
  %tmp288 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp289 = extractelement <8 x float> %tmp288, i32 3 ; <float> [#uses=1]
  %add290 = fadd float %add287, %tmp289           ; <float> [#uses=1]
  %tmp291 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp292 = extractelement <8 x float> %tmp291, i32 4 ; <float> [#uses=1]
  %add293 = fadd float %add290, %tmp292           ; <float> [#uses=1]
  %tmp294 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp295 = extractelement <8 x float> %tmp294, i32 5 ; <float> [#uses=1]
  %add296 = fadd float %add293, %tmp295           ; <float> [#uses=1]
  %tmp297 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp298 = extractelement <8 x float> %tmp297, i32 6 ; <float> [#uses=1]
  %add299 = fadd float %add296, %tmp298           ; <float> [#uses=1]
  %tmp300 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp301 = extractelement <8 x float> %tmp300, i32 7 ; <float> [#uses=1]
  %add302 = fadd float %add299, %tmp301           ; <float> [#uses=1]
  %tmp303 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp304 = insertelement <8 x float> %tmp303, float %add302, i32 7 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp304, <8 x float>* %acc
  %tmp305 = load i32* %k1                         ; <i32> [#uses=1]
  store i32 %tmp305, i32* %idx
  %tmp306 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp307 = load i32* %idx                        ; <i32> [#uses=1]
  %arrayidx308 = getelementptr inbounds <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 0), i32 %tmp307 ; <<8 x float> addrspace(3)*> [#uses=1]
  store <8 x float> %tmp306, <8 x float> addrspace(3)* %arrayidx308
  call void @barrier(i32 1)
  %tmp309 = load i32* %i                          ; <i32> [#uses=1]
  store i32 %tmp309, i32* %k2
  %tmp310 = load i32* %k2                         ; <i32> [#uses=1]
  store i32 %tmp310, i32* %index2
  %tmp311 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 0) ; <<8 x float>> [#uses=1]
  %tmp312 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp313 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx314 = getelementptr inbounds <8 x float> addrspace(1)* %tmp313, i32 %tmp312 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp315 = load <8 x float> addrspace(1)* %arrayidx314 ; <<8 x float>> [#uses=1]
  %mul316 = fmul <8 x float> %tmp311, %tmp315     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul316, <8 x float>* %temp
  %tmp317 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp318 = extractelement <8 x float> %tmp317, i32 0 ; <float> [#uses=1]
  %tmp319 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp320 = extractelement <8 x float> %tmp319, i32 1 ; <float> [#uses=1]
  %add321 = fadd float %tmp318, %tmp320           ; <float> [#uses=1]
  %tmp322 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp323 = extractelement <8 x float> %tmp322, i32 2 ; <float> [#uses=1]
  %add324 = fadd float %add321, %tmp323           ; <float> [#uses=1]
  %tmp325 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp326 = extractelement <8 x float> %tmp325, i32 3 ; <float> [#uses=1]
  %add327 = fadd float %add324, %tmp326           ; <float> [#uses=1]
  %tmp328 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp329 = extractelement <8 x float> %tmp328, i32 4 ; <float> [#uses=1]
  %add330 = fadd float %add327, %tmp329           ; <float> [#uses=1]
  %tmp331 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp332 = extractelement <8 x float> %tmp331, i32 5 ; <float> [#uses=1]
  %add333 = fadd float %add330, %tmp332           ; <float> [#uses=1]
  %tmp334 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp335 = extractelement <8 x float> %tmp334, i32 6 ; <float> [#uses=1]
  %add336 = fadd float %add333, %tmp335           ; <float> [#uses=1]
  %tmp337 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp338 = extractelement <8 x float> %tmp337, i32 7 ; <float> [#uses=1]
  %add339 = fadd float %add336, %tmp338           ; <float> [#uses=1]
  %tmp340 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp341 = insertelement <8 x float> %tmp340, float %add339, i32 0 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp341, <8 x float>* %acc
  %tmp342 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 1) ; <<8 x float>> [#uses=1]
  %tmp343 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp344 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx345 = getelementptr inbounds <8 x float> addrspace(1)* %tmp344, i32 %tmp343 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp346 = load <8 x float> addrspace(1)* %arrayidx345 ; <<8 x float>> [#uses=1]
  %mul347 = fmul <8 x float> %tmp342, %tmp346     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul347, <8 x float>* %temp
  %tmp348 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp349 = extractelement <8 x float> %tmp348, i32 0 ; <float> [#uses=1]
  %tmp350 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp351 = extractelement <8 x float> %tmp350, i32 1 ; <float> [#uses=1]
  %add352 = fadd float %tmp349, %tmp351           ; <float> [#uses=1]
  %tmp353 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp354 = extractelement <8 x float> %tmp353, i32 2 ; <float> [#uses=1]
  %add355 = fadd float %add352, %tmp354           ; <float> [#uses=1]
  %tmp356 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp357 = extractelement <8 x float> %tmp356, i32 3 ; <float> [#uses=1]
  %add358 = fadd float %add355, %tmp357           ; <float> [#uses=1]
  %tmp359 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp360 = extractelement <8 x float> %tmp359, i32 4 ; <float> [#uses=1]
  %add361 = fadd float %add358, %tmp360           ; <float> [#uses=1]
  %tmp362 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp363 = extractelement <8 x float> %tmp362, i32 5 ; <float> [#uses=1]
  %add364 = fadd float %add361, %tmp363           ; <float> [#uses=1]
  %tmp365 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp366 = extractelement <8 x float> %tmp365, i32 6 ; <float> [#uses=1]
  %add367 = fadd float %add364, %tmp366           ; <float> [#uses=1]
  %tmp368 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp369 = extractelement <8 x float> %tmp368, i32 7 ; <float> [#uses=1]
  %add370 = fadd float %add367, %tmp369           ; <float> [#uses=1]
  %tmp371 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp372 = insertelement <8 x float> %tmp371, float %add370, i32 1 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp372, <8 x float>* %acc
  %tmp373 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 2) ; <<8 x float>> [#uses=1]
  %tmp374 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp375 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx376 = getelementptr inbounds <8 x float> addrspace(1)* %tmp375, i32 %tmp374 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp377 = load <8 x float> addrspace(1)* %arrayidx376 ; <<8 x float>> [#uses=1]
  %mul378 = fmul <8 x float> %tmp373, %tmp377     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul378, <8 x float>* %temp
  %tmp379 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp380 = extractelement <8 x float> %tmp379, i32 0 ; <float> [#uses=1]
  %tmp381 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp382 = extractelement <8 x float> %tmp381, i32 1 ; <float> [#uses=1]
  %add383 = fadd float %tmp380, %tmp382           ; <float> [#uses=1]
  %tmp384 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp385 = extractelement <8 x float> %tmp384, i32 2 ; <float> [#uses=1]
  %add386 = fadd float %add383, %tmp385           ; <float> [#uses=1]
  %tmp387 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp388 = extractelement <8 x float> %tmp387, i32 3 ; <float> [#uses=1]
  %add389 = fadd float %add386, %tmp388           ; <float> [#uses=1]
  %tmp390 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp391 = extractelement <8 x float> %tmp390, i32 4 ; <float> [#uses=1]
  %add392 = fadd float %add389, %tmp391           ; <float> [#uses=1]
  %tmp393 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp394 = extractelement <8 x float> %tmp393, i32 5 ; <float> [#uses=1]
  %add395 = fadd float %add392, %tmp394           ; <float> [#uses=1]
  %tmp396 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp397 = extractelement <8 x float> %tmp396, i32 6 ; <float> [#uses=1]
  %add398 = fadd float %add395, %tmp397           ; <float> [#uses=1]
  %tmp399 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp400 = extractelement <8 x float> %tmp399, i32 7 ; <float> [#uses=1]
  %add401 = fadd float %add398, %tmp400           ; <float> [#uses=1]
  %tmp402 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp403 = insertelement <8 x float> %tmp402, float %add401, i32 2 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp403, <8 x float>* %acc
  %tmp404 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 3) ; <<8 x float>> [#uses=1]
  %tmp405 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp406 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx407 = getelementptr inbounds <8 x float> addrspace(1)* %tmp406, i32 %tmp405 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp408 = load <8 x float> addrspace(1)* %arrayidx407 ; <<8 x float>> [#uses=1]
  %mul409 = fmul <8 x float> %tmp404, %tmp408     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul409, <8 x float>* %temp
  %tmp410 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp411 = extractelement <8 x float> %tmp410, i32 0 ; <float> [#uses=1]
  %tmp412 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp413 = extractelement <8 x float> %tmp412, i32 1 ; <float> [#uses=1]
  %add414 = fadd float %tmp411, %tmp413           ; <float> [#uses=1]
  %tmp415 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp416 = extractelement <8 x float> %tmp415, i32 2 ; <float> [#uses=1]
  %add417 = fadd float %add414, %tmp416           ; <float> [#uses=1]
  %tmp418 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp419 = extractelement <8 x float> %tmp418, i32 3 ; <float> [#uses=1]
  %add420 = fadd float %add417, %tmp419           ; <float> [#uses=1]
  %tmp421 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp422 = extractelement <8 x float> %tmp421, i32 4 ; <float> [#uses=1]
  %add423 = fadd float %add420, %tmp422           ; <float> [#uses=1]
  %tmp424 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp425 = extractelement <8 x float> %tmp424, i32 5 ; <float> [#uses=1]
  %add426 = fadd float %add423, %tmp425           ; <float> [#uses=1]
  %tmp427 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp428 = extractelement <8 x float> %tmp427, i32 6 ; <float> [#uses=1]
  %add429 = fadd float %add426, %tmp428           ; <float> [#uses=1]
  %tmp430 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp431 = extractelement <8 x float> %tmp430, i32 7 ; <float> [#uses=1]
  %add432 = fadd float %add429, %tmp431           ; <float> [#uses=1]
  %tmp433 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp434 = insertelement <8 x float> %tmp433, float %add432, i32 3 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp434, <8 x float>* %acc
  %tmp435 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 4) ; <<8 x float>> [#uses=1]
  %tmp436 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp437 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx438 = getelementptr inbounds <8 x float> addrspace(1)* %tmp437, i32 %tmp436 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp439 = load <8 x float> addrspace(1)* %arrayidx438 ; <<8 x float>> [#uses=1]
  %mul440 = fmul <8 x float> %tmp435, %tmp439     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul440, <8 x float>* %temp
  %tmp441 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp442 = extractelement <8 x float> %tmp441, i32 0 ; <float> [#uses=1]
  %tmp443 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp444 = extractelement <8 x float> %tmp443, i32 1 ; <float> [#uses=1]
  %add445 = fadd float %tmp442, %tmp444           ; <float> [#uses=1]
  %tmp446 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp447 = extractelement <8 x float> %tmp446, i32 2 ; <float> [#uses=1]
  %add448 = fadd float %add445, %tmp447           ; <float> [#uses=1]
  %tmp449 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp450 = extractelement <8 x float> %tmp449, i32 3 ; <float> [#uses=1]
  %add451 = fadd float %add448, %tmp450           ; <float> [#uses=1]
  %tmp452 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp453 = extractelement <8 x float> %tmp452, i32 4 ; <float> [#uses=1]
  %add454 = fadd float %add451, %tmp453           ; <float> [#uses=1]
  %tmp455 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp456 = extractelement <8 x float> %tmp455, i32 5 ; <float> [#uses=1]
  %add457 = fadd float %add454, %tmp456           ; <float> [#uses=1]
  %tmp458 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp459 = extractelement <8 x float> %tmp458, i32 6 ; <float> [#uses=1]
  %add460 = fadd float %add457, %tmp459           ; <float> [#uses=1]
  %tmp461 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp462 = extractelement <8 x float> %tmp461, i32 7 ; <float> [#uses=1]
  %add463 = fadd float %add460, %tmp462           ; <float> [#uses=1]
  %tmp464 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp465 = insertelement <8 x float> %tmp464, float %add463, i32 4 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp465, <8 x float>* %acc
  %tmp466 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 5) ; <<8 x float>> [#uses=1]
  %tmp467 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp468 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx469 = getelementptr inbounds <8 x float> addrspace(1)* %tmp468, i32 %tmp467 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp470 = load <8 x float> addrspace(1)* %arrayidx469 ; <<8 x float>> [#uses=1]
  %mul471 = fmul <8 x float> %tmp466, %tmp470     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul471, <8 x float>* %temp
  %tmp472 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp473 = extractelement <8 x float> %tmp472, i32 0 ; <float> [#uses=1]
  %tmp474 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp475 = extractelement <8 x float> %tmp474, i32 1 ; <float> [#uses=1]
  %add476 = fadd float %tmp473, %tmp475           ; <float> [#uses=1]
  %tmp477 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp478 = extractelement <8 x float> %tmp477, i32 2 ; <float> [#uses=1]
  %add479 = fadd float %add476, %tmp478           ; <float> [#uses=1]
  %tmp480 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp481 = extractelement <8 x float> %tmp480, i32 3 ; <float> [#uses=1]
  %add482 = fadd float %add479, %tmp481           ; <float> [#uses=1]
  %tmp483 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp484 = extractelement <8 x float> %tmp483, i32 4 ; <float> [#uses=1]
  %add485 = fadd float %add482, %tmp484           ; <float> [#uses=1]
  %tmp486 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp487 = extractelement <8 x float> %tmp486, i32 5 ; <float> [#uses=1]
  %add488 = fadd float %add485, %tmp487           ; <float> [#uses=1]
  %tmp489 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp490 = extractelement <8 x float> %tmp489, i32 6 ; <float> [#uses=1]
  %add491 = fadd float %add488, %tmp490           ; <float> [#uses=1]
  %tmp492 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp493 = extractelement <8 x float> %tmp492, i32 7 ; <float> [#uses=1]
  %add494 = fadd float %add491, %tmp493           ; <float> [#uses=1]
  %tmp495 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp496 = insertelement <8 x float> %tmp495, float %add494, i32 5 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp496, <8 x float>* %acc
  %tmp497 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 6) ; <<8 x float>> [#uses=1]
  %tmp498 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp499 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx500 = getelementptr inbounds <8 x float> addrspace(1)* %tmp499, i32 %tmp498 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp501 = load <8 x float> addrspace(1)* %arrayidx500 ; <<8 x float>> [#uses=1]
  %mul502 = fmul <8 x float> %tmp497, %tmp501     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul502, <8 x float>* %temp
  %tmp503 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp504 = extractelement <8 x float> %tmp503, i32 0 ; <float> [#uses=1]
  %tmp505 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp506 = extractelement <8 x float> %tmp505, i32 1 ; <float> [#uses=1]
  %add507 = fadd float %tmp504, %tmp506           ; <float> [#uses=1]
  %tmp508 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp509 = extractelement <8 x float> %tmp508, i32 2 ; <float> [#uses=1]
  %add510 = fadd float %add507, %tmp509           ; <float> [#uses=1]
  %tmp511 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp512 = extractelement <8 x float> %tmp511, i32 3 ; <float> [#uses=1]
  %add513 = fadd float %add510, %tmp512           ; <float> [#uses=1]
  %tmp514 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp515 = extractelement <8 x float> %tmp514, i32 4 ; <float> [#uses=1]
  %add516 = fadd float %add513, %tmp515           ; <float> [#uses=1]
  %tmp517 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp518 = extractelement <8 x float> %tmp517, i32 5 ; <float> [#uses=1]
  %add519 = fadd float %add516, %tmp518           ; <float> [#uses=1]
  %tmp520 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp521 = extractelement <8 x float> %tmp520, i32 6 ; <float> [#uses=1]
  %add522 = fadd float %add519, %tmp521           ; <float> [#uses=1]
  %tmp523 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp524 = extractelement <8 x float> %tmp523, i32 7 ; <float> [#uses=1]
  %add525 = fadd float %add522, %tmp524           ; <float> [#uses=1]
  %tmp526 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp527 = insertelement <8 x float> %tmp526, float %add525, i32 6 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp527, <8 x float>* %acc
  %tmp528 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_local_inter, i32 0, i32 7) ; <<8 x float>> [#uses=1]
  %tmp529 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp530 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx531 = getelementptr inbounds <8 x float> addrspace(1)* %tmp530, i32 %tmp529 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp532 = load <8 x float> addrspace(1)* %arrayidx531 ; <<8 x float>> [#uses=1]
  %mul533 = fmul <8 x float> %tmp528, %tmp532     ; <<8 x float>> [#uses=1]
  store <8 x float> %mul533, <8 x float>* %temp
  %tmp534 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp535 = extractelement <8 x float> %tmp534, i32 0 ; <float> [#uses=1]
  %tmp536 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp537 = extractelement <8 x float> %tmp536, i32 1 ; <float> [#uses=1]
  %add538 = fadd float %tmp535, %tmp537           ; <float> [#uses=1]
  %tmp539 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp540 = extractelement <8 x float> %tmp539, i32 2 ; <float> [#uses=1]
  %add541 = fadd float %add538, %tmp540           ; <float> [#uses=1]
  %tmp542 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp543 = extractelement <8 x float> %tmp542, i32 3 ; <float> [#uses=1]
  %add544 = fadd float %add541, %tmp543           ; <float> [#uses=1]
  %tmp545 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp546 = extractelement <8 x float> %tmp545, i32 4 ; <float> [#uses=1]
  %add547 = fadd float %add544, %tmp546           ; <float> [#uses=1]
  %tmp548 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp549 = extractelement <8 x float> %tmp548, i32 5 ; <float> [#uses=1]
  %add550 = fadd float %add547, %tmp549           ; <float> [#uses=1]
  %tmp551 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp552 = extractelement <8 x float> %tmp551, i32 6 ; <float> [#uses=1]
  %add553 = fadd float %add550, %tmp552           ; <float> [#uses=1]
  %tmp554 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp555 = extractelement <8 x float> %tmp554, i32 7 ; <float> [#uses=1]
  %add556 = fadd float %add553, %tmp555           ; <float> [#uses=1]
  %tmp557 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp558 = insertelement <8 x float> %tmp557, float %add556, i32 7 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp558, <8 x float>* %acc
  %tmp559 = load i32* %groupIdy                   ; <i32> [#uses=1]
  %mul560 = mul i32 %tmp559, 8                    ; <i32> [#uses=1]
  %tmp561 = load i32* %k2                         ; <i32> [#uses=1]
  %add562 = add i32 %mul560, %tmp561              ; <i32> [#uses=1]
  %tmp563 = load i32* %step                       ; <i32> [#uses=1]
  %mul564 = mul i32 %add562, %tmp563              ; <i32> [#uses=1]
  %tmp565 = load i32* %groupIdx                   ; <i32> [#uses=1]
  %add566 = add i32 %mul564, %tmp565              ; <i32> [#uses=1]
  store i32 %add566, i32* %idx
  %tmp567 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp568 = load i32* %idx                        ; <i32> [#uses=1]
  %tmp569 = load <8 x float> addrspace(1)** %output.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx570 = getelementptr inbounds <8 x float> addrspace(1)* %tmp569, i32 %tmp568 ; <<8 x float> addrspace(1)*> [#uses=1]
  store <8 x float> %tmp567, <8 x float> addrspace(1)* %arrayidx570
  ret void
}

; CHECK: ret
define void @DCT_VECTOR_DOT(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width) nounwind {
entry:
  %output.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=2]
  %input.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=17]
  %dct.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=33]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=1]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=1]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=3]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %k2 = alloca i32, align 4                       ; <i32*> [#uses=3]
  %n1 = alloca i32, align 4                       ; <i32*> [#uses=0]
  %n2 = alloca i32, align 4                       ; <i32*> [#uses=0]
  %idx = alloca i32, align 4                      ; <i32*> [#uses=5]
  %acc = alloca <8 x float>, align 32             ; <<8 x float>*> [#uses=35]
  %step = alloca i32, align 4                     ; <i32*> [#uses=10]
  %index1 = alloca i32, align 4                   ; <i32*> [#uses=17]
  %index2 = alloca i32, align 4                   ; <i32*> [#uses=48]
  store <8 x float> addrspace(1)* %output, <8 x float> addrspace(1)** %output.addr
  store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
  store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalIdx
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %globalIdy
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %groupIdx
  %call3 = call i32 @get_group_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call3, i32* %groupIdy
  %call4 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call4, i32* %i
  store i32 0, i32* %idx
  store <8 x float> zeroinitializer, <8 x float>* %acc
  %tmp = load i32* %width.addr                    ; <i32> [#uses=1]
  %div = udiv i32 %tmp, 8                         ; <i32> [#uses=1]
  store i32 %div, i32* %step
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  store i32 %tmp5, i32* %k1
  %tmp7 = load i32* %k1                           ; <i32> [#uses=1]
  store i32 %tmp7, i32* %index1
  %tmp9 = load i32* %groupIdy                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp9, 8                         ; <i32> [#uses=1]
  %tmp10 = load i32* %step                        ; <i32> [#uses=1]
  %mul11 = mul i32 %mul, %tmp10                   ; <i32> [#uses=1]
  %tmp12 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %add = add i32 %mul11, %tmp12                   ; <i32> [#uses=1]
  store i32 %add, i32* %index2
  %tmp13 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp14 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x float> addrspace(1)* %tmp14, i32 %tmp13 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp15 = load <8 x float> addrspace(1)* %arrayidx ; <<8 x float>> [#uses=1]
  %tmp16 = shufflevector <8 x float> %tmp15, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp17 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp18 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds <8 x float> addrspace(1)* %tmp18, i32 %tmp17 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp20 = load <8 x float> addrspace(1)* %arrayidx19 ; <<8 x float>> [#uses=1]
  %tmp21 = shufflevector <8 x float> %tmp20, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call22 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp16, <4 x float> %tmp21) ; <float> [#uses=1]
  %tmp23 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp24 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds <8 x float> addrspace(1)* %tmp24, i32 %tmp23 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp26 = load <8 x float> addrspace(1)* %arrayidx25 ; <<8 x float>> [#uses=1]
  %tmp27 = shufflevector <8 x float> %tmp26, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp28 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp29 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds <8 x float> addrspace(1)* %tmp29, i32 %tmp28 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp31 = load <8 x float> addrspace(1)* %arrayidx30 ; <<8 x float>> [#uses=1]
  %tmp32 = shufflevector <8 x float> %tmp31, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call33 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp27, <4 x float> %tmp32) ; <float> [#uses=1]
  %add34 = fadd float %call22, %call33            ; <float> [#uses=1]
  %tmp35 = load <8 x float>* %acc                 ; <<8 x float>> [#uses=1]
  %tmp36 = insertelement <8 x float> %tmp35, float %add34, i32 0 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp36, <8 x float>* %acc
  %tmp37 = load i32* %step                        ; <i32> [#uses=1]
  %tmp38 = load i32* %index2                      ; <i32> [#uses=1]
  %add39 = add i32 %tmp38, %tmp37                 ; <i32> [#uses=1]
  store i32 %add39, i32* %index2
  %tmp40 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp41 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx42 = getelementptr inbounds <8 x float> addrspace(1)* %tmp41, i32 %tmp40 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp43 = load <8 x float> addrspace(1)* %arrayidx42 ; <<8 x float>> [#uses=1]
  %tmp44 = shufflevector <8 x float> %tmp43, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp45 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp46 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx47 = getelementptr inbounds <8 x float> addrspace(1)* %tmp46, i32 %tmp45 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp48 = load <8 x float> addrspace(1)* %arrayidx47 ; <<8 x float>> [#uses=1]
  %tmp49 = shufflevector <8 x float> %tmp48, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call50 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp44, <4 x float> %tmp49) ; <float> [#uses=1]
  %tmp51 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp52 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx53 = getelementptr inbounds <8 x float> addrspace(1)* %tmp52, i32 %tmp51 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp54 = load <8 x float> addrspace(1)* %arrayidx53 ; <<8 x float>> [#uses=1]
  %tmp55 = shufflevector <8 x float> %tmp54, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp56 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp57 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx58 = getelementptr inbounds <8 x float> addrspace(1)* %tmp57, i32 %tmp56 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp59 = load <8 x float> addrspace(1)* %arrayidx58 ; <<8 x float>> [#uses=1]
  %tmp60 = shufflevector <8 x float> %tmp59, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call61 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp55, <4 x float> %tmp60) ; <float> [#uses=1]
  %add62 = fadd float %call50, %call61            ; <float> [#uses=1]
  %tmp63 = load <8 x float>* %acc                 ; <<8 x float>> [#uses=1]
  %tmp64 = insertelement <8 x float> %tmp63, float %add62, i32 1 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp64, <8 x float>* %acc
  %tmp65 = load i32* %step                        ; <i32> [#uses=1]
  %tmp66 = load i32* %index2                      ; <i32> [#uses=1]
  %add67 = add i32 %tmp66, %tmp65                 ; <i32> [#uses=1]
  store i32 %add67, i32* %index2
  %tmp68 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp69 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds <8 x float> addrspace(1)* %tmp69, i32 %tmp68 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp71 = load <8 x float> addrspace(1)* %arrayidx70 ; <<8 x float>> [#uses=1]
  %tmp72 = shufflevector <8 x float> %tmp71, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp73 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp74 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx75 = getelementptr inbounds <8 x float> addrspace(1)* %tmp74, i32 %tmp73 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp76 = load <8 x float> addrspace(1)* %arrayidx75 ; <<8 x float>> [#uses=1]
  %tmp77 = shufflevector <8 x float> %tmp76, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call78 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp72, <4 x float> %tmp77) ; <float> [#uses=1]
  %tmp79 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp80 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx81 = getelementptr inbounds <8 x float> addrspace(1)* %tmp80, i32 %tmp79 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp82 = load <8 x float> addrspace(1)* %arrayidx81 ; <<8 x float>> [#uses=1]
  %tmp83 = shufflevector <8 x float> %tmp82, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp84 = load i32* %index2                      ; <i32> [#uses=1]
  %tmp85 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx86 = getelementptr inbounds <8 x float> addrspace(1)* %tmp85, i32 %tmp84 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp87 = load <8 x float> addrspace(1)* %arrayidx86 ; <<8 x float>> [#uses=1]
  %tmp88 = shufflevector <8 x float> %tmp87, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call89 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp83, <4 x float> %tmp88) ; <float> [#uses=1]
  %add90 = fadd float %call78, %call89            ; <float> [#uses=1]
  %tmp91 = load <8 x float>* %acc                 ; <<8 x float>> [#uses=1]
  %tmp92 = insertelement <8 x float> %tmp91, float %add90, i32 2 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp92, <8 x float>* %acc
  %tmp93 = load i32* %step                        ; <i32> [#uses=1]
  %tmp94 = load i32* %index2                      ; <i32> [#uses=1]
  %add95 = add i32 %tmp94, %tmp93                 ; <i32> [#uses=1]
  store i32 %add95, i32* %index2
  %tmp96 = load i32* %index1                      ; <i32> [#uses=1]
  %tmp97 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx98 = getelementptr inbounds <8 x float> addrspace(1)* %tmp97, i32 %tmp96 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp99 = load <8 x float> addrspace(1)* %arrayidx98 ; <<8 x float>> [#uses=1]
  %tmp100 = shufflevector <8 x float> %tmp99, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp101 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp102 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds <8 x float> addrspace(1)* %tmp102, i32 %tmp101 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp104 = load <8 x float> addrspace(1)* %arrayidx103 ; <<8 x float>> [#uses=1]
  %tmp105 = shufflevector <8 x float> %tmp104, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call106 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp100, <4 x float> %tmp105) ; <float> [#uses=1]
  %tmp107 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp108 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds <8 x float> addrspace(1)* %tmp108, i32 %tmp107 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp110 = load <8 x float> addrspace(1)* %arrayidx109 ; <<8 x float>> [#uses=1]
  %tmp111 = shufflevector <8 x float> %tmp110, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp112 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp113 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx114 = getelementptr inbounds <8 x float> addrspace(1)* %tmp113, i32 %tmp112 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp115 = load <8 x float> addrspace(1)* %arrayidx114 ; <<8 x float>> [#uses=1]
  %tmp116 = shufflevector <8 x float> %tmp115, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call117 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp111, <4 x float> %tmp116) ; <float> [#uses=1]
  %add118 = fadd float %call106, %call117         ; <float> [#uses=1]
  %tmp119 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp120 = insertelement <8 x float> %tmp119, float %add118, i32 3 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp120, <8 x float>* %acc
  %tmp121 = load i32* %step                       ; <i32> [#uses=1]
  %tmp122 = load i32* %index2                     ; <i32> [#uses=1]
  %add123 = add i32 %tmp122, %tmp121              ; <i32> [#uses=1]
  store i32 %add123, i32* %index2
  %tmp124 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp125 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx126 = getelementptr inbounds <8 x float> addrspace(1)* %tmp125, i32 %tmp124 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp127 = load <8 x float> addrspace(1)* %arrayidx126 ; <<8 x float>> [#uses=1]
  %tmp128 = shufflevector <8 x float> %tmp127, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp129 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp130 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx131 = getelementptr inbounds <8 x float> addrspace(1)* %tmp130, i32 %tmp129 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp132 = load <8 x float> addrspace(1)* %arrayidx131 ; <<8 x float>> [#uses=1]
  %tmp133 = shufflevector <8 x float> %tmp132, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call134 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp128, <4 x float> %tmp133) ; <float> [#uses=1]
  %tmp135 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp136 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx137 = getelementptr inbounds <8 x float> addrspace(1)* %tmp136, i32 %tmp135 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp138 = load <8 x float> addrspace(1)* %arrayidx137 ; <<8 x float>> [#uses=1]
  %tmp139 = shufflevector <8 x float> %tmp138, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp140 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp141 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx142 = getelementptr inbounds <8 x float> addrspace(1)* %tmp141, i32 %tmp140 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp143 = load <8 x float> addrspace(1)* %arrayidx142 ; <<8 x float>> [#uses=1]
  %tmp144 = shufflevector <8 x float> %tmp143, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call145 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp139, <4 x float> %tmp144) ; <float> [#uses=1]
  %add146 = fadd float %call134, %call145         ; <float> [#uses=1]
  %tmp147 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp148 = insertelement <8 x float> %tmp147, float %add146, i32 4 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp148, <8 x float>* %acc
  %tmp149 = load i32* %step                       ; <i32> [#uses=1]
  %tmp150 = load i32* %index2                     ; <i32> [#uses=1]
  %add151 = add i32 %tmp150, %tmp149              ; <i32> [#uses=1]
  store i32 %add151, i32* %index2
  %tmp152 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp153 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx154 = getelementptr inbounds <8 x float> addrspace(1)* %tmp153, i32 %tmp152 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp155 = load <8 x float> addrspace(1)* %arrayidx154 ; <<8 x float>> [#uses=1]
  %tmp156 = shufflevector <8 x float> %tmp155, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp157 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp158 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx159 = getelementptr inbounds <8 x float> addrspace(1)* %tmp158, i32 %tmp157 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp160 = load <8 x float> addrspace(1)* %arrayidx159 ; <<8 x float>> [#uses=1]
  %tmp161 = shufflevector <8 x float> %tmp160, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call162 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp156, <4 x float> %tmp161) ; <float> [#uses=1]
  %tmp163 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp164 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx165 = getelementptr inbounds <8 x float> addrspace(1)* %tmp164, i32 %tmp163 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp166 = load <8 x float> addrspace(1)* %arrayidx165 ; <<8 x float>> [#uses=1]
  %tmp167 = shufflevector <8 x float> %tmp166, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp168 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp169 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx170 = getelementptr inbounds <8 x float> addrspace(1)* %tmp169, i32 %tmp168 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp171 = load <8 x float> addrspace(1)* %arrayidx170 ; <<8 x float>> [#uses=1]
  %tmp172 = shufflevector <8 x float> %tmp171, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call173 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp167, <4 x float> %tmp172) ; <float> [#uses=1]
  %add174 = fadd float %call162, %call173         ; <float> [#uses=1]
  %tmp175 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp176 = insertelement <8 x float> %tmp175, float %add174, i32 5 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp176, <8 x float>* %acc
  %tmp177 = load i32* %step                       ; <i32> [#uses=1]
  %tmp178 = load i32* %index2                     ; <i32> [#uses=1]
  %add179 = add i32 %tmp178, %tmp177              ; <i32> [#uses=1]
  store i32 %add179, i32* %index2
  %tmp180 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp181 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx182 = getelementptr inbounds <8 x float> addrspace(1)* %tmp181, i32 %tmp180 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp183 = load <8 x float> addrspace(1)* %arrayidx182 ; <<8 x float>> [#uses=1]
  %tmp184 = shufflevector <8 x float> %tmp183, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp185 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp186 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx187 = getelementptr inbounds <8 x float> addrspace(1)* %tmp186, i32 %tmp185 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp188 = load <8 x float> addrspace(1)* %arrayidx187 ; <<8 x float>> [#uses=1]
  %tmp189 = shufflevector <8 x float> %tmp188, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call190 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp184, <4 x float> %tmp189) ; <float> [#uses=1]
  %tmp191 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp192 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx193 = getelementptr inbounds <8 x float> addrspace(1)* %tmp192, i32 %tmp191 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp194 = load <8 x float> addrspace(1)* %arrayidx193 ; <<8 x float>> [#uses=1]
  %tmp195 = shufflevector <8 x float> %tmp194, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp196 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp197 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx198 = getelementptr inbounds <8 x float> addrspace(1)* %tmp197, i32 %tmp196 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp199 = load <8 x float> addrspace(1)* %arrayidx198 ; <<8 x float>> [#uses=1]
  %tmp200 = shufflevector <8 x float> %tmp199, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call201 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp195, <4 x float> %tmp200) ; <float> [#uses=1]
  %add202 = fadd float %call190, %call201         ; <float> [#uses=1]
  %tmp203 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp204 = insertelement <8 x float> %tmp203, float %add202, i32 6 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp204, <8 x float>* %acc
  %tmp205 = load i32* %step                       ; <i32> [#uses=1]
  %tmp206 = load i32* %index2                     ; <i32> [#uses=1]
  %add207 = add i32 %tmp206, %tmp205              ; <i32> [#uses=1]
  store i32 %add207, i32* %index2
  %tmp208 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp209 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx210 = getelementptr inbounds <8 x float> addrspace(1)* %tmp209, i32 %tmp208 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp211 = load <8 x float> addrspace(1)* %arrayidx210 ; <<8 x float>> [#uses=1]
  %tmp212 = shufflevector <8 x float> %tmp211, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp213 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp214 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx215 = getelementptr inbounds <8 x float> addrspace(1)* %tmp214, i32 %tmp213 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp216 = load <8 x float> addrspace(1)* %arrayidx215 ; <<8 x float>> [#uses=1]
  %tmp217 = shufflevector <8 x float> %tmp216, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call218 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp212, <4 x float> %tmp217) ; <float> [#uses=1]
  %tmp219 = load i32* %index1                     ; <i32> [#uses=1]
  %tmp220 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx221 = getelementptr inbounds <8 x float> addrspace(1)* %tmp220, i32 %tmp219 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp222 = load <8 x float> addrspace(1)* %arrayidx221 ; <<8 x float>> [#uses=1]
  %tmp223 = shufflevector <8 x float> %tmp222, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp224 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp225 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx226 = getelementptr inbounds <8 x float> addrspace(1)* %tmp225, i32 %tmp224 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp227 = load <8 x float> addrspace(1)* %arrayidx226 ; <<8 x float>> [#uses=1]
  %tmp228 = shufflevector <8 x float> %tmp227, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call229 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp223, <4 x float> %tmp228) ; <float> [#uses=1]
  %add230 = fadd float %call218, %call229         ; <float> [#uses=1]
  %tmp231 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp232 = insertelement <8 x float> %tmp231, float %add230, i32 7 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp232, <8 x float>* %acc
  %tmp233 = load i32* %k1                         ; <i32> [#uses=1]
  store i32 %tmp233, i32* %idx
  %tmp234 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp235 = load i32* %idx                        ; <i32> [#uses=1]
  %arrayidx236 = getelementptr inbounds <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 0), i32 %tmp235 ; <<8 x float> addrspace(3)*> [#uses=1]
  store <8 x float> %tmp234, <8 x float> addrspace(3)* %arrayidx236
  call void @barrier(i32 2)
  %tmp237 = load i32* %i                          ; <i32> [#uses=1]
  store i32 %tmp237, i32* %k2
  %tmp238 = load i32* %k2                         ; <i32> [#uses=1]
  store i32 %tmp238, i32* %index2
  %tmp239 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 0) ; <<8 x float>> [#uses=1]
  %tmp240 = shufflevector <8 x float> %tmp239, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp241 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp242 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx243 = getelementptr inbounds <8 x float> addrspace(1)* %tmp242, i32 %tmp241 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp244 = load <8 x float> addrspace(1)* %arrayidx243 ; <<8 x float>> [#uses=1]
  %tmp245 = shufflevector <8 x float> %tmp244, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call246 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp240, <4 x float> %tmp245) ; <float> [#uses=1]
  %tmp247 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 0) ; <<8 x float>> [#uses=1]
  %tmp248 = shufflevector <8 x float> %tmp247, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp249 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp250 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx251 = getelementptr inbounds <8 x float> addrspace(1)* %tmp250, i32 %tmp249 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp252 = load <8 x float> addrspace(1)* %arrayidx251 ; <<8 x float>> [#uses=1]
  %tmp253 = shufflevector <8 x float> %tmp252, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call254 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp248, <4 x float> %tmp253) ; <float> [#uses=1]
  %add255 = fadd float %call246, %call254         ; <float> [#uses=1]
  %tmp256 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp257 = insertelement <8 x float> %tmp256, float %add255, i32 0 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp257, <8 x float>* %acc
  %tmp258 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 1) ; <<8 x float>> [#uses=1]
  %tmp259 = shufflevector <8 x float> %tmp258, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp260 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp261 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx262 = getelementptr inbounds <8 x float> addrspace(1)* %tmp261, i32 %tmp260 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp263 = load <8 x float> addrspace(1)* %arrayidx262 ; <<8 x float>> [#uses=1]
  %tmp264 = shufflevector <8 x float> %tmp263, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call265 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp259, <4 x float> %tmp264) ; <float> [#uses=1]
  %tmp266 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 1) ; <<8 x float>> [#uses=1]
  %tmp267 = shufflevector <8 x float> %tmp266, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp268 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp269 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx270 = getelementptr inbounds <8 x float> addrspace(1)* %tmp269, i32 %tmp268 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp271 = load <8 x float> addrspace(1)* %arrayidx270 ; <<8 x float>> [#uses=1]
  %tmp272 = shufflevector <8 x float> %tmp271, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call273 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp267, <4 x float> %tmp272) ; <float> [#uses=1]
  %add274 = fadd float %call265, %call273         ; <float> [#uses=1]
  %tmp275 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp276 = insertelement <8 x float> %tmp275, float %add274, i32 1 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp276, <8 x float>* %acc
  %tmp277 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 2) ; <<8 x float>> [#uses=1]
  %tmp278 = shufflevector <8 x float> %tmp277, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp279 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp280 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx281 = getelementptr inbounds <8 x float> addrspace(1)* %tmp280, i32 %tmp279 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp282 = load <8 x float> addrspace(1)* %arrayidx281 ; <<8 x float>> [#uses=1]
  %tmp283 = shufflevector <8 x float> %tmp282, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call284 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp278, <4 x float> %tmp283) ; <float> [#uses=1]
  %tmp285 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 2) ; <<8 x float>> [#uses=1]
  %tmp286 = shufflevector <8 x float> %tmp285, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp287 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp288 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx289 = getelementptr inbounds <8 x float> addrspace(1)* %tmp288, i32 %tmp287 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp290 = load <8 x float> addrspace(1)* %arrayidx289 ; <<8 x float>> [#uses=1]
  %tmp291 = shufflevector <8 x float> %tmp290, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call292 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp286, <4 x float> %tmp291) ; <float> [#uses=1]
  %add293 = fadd float %call284, %call292         ; <float> [#uses=1]
  %tmp294 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp295 = insertelement <8 x float> %tmp294, float %add293, i32 2 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp295, <8 x float>* %acc
  %tmp296 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 3) ; <<8 x float>> [#uses=1]
  %tmp297 = shufflevector <8 x float> %tmp296, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp298 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp299 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx300 = getelementptr inbounds <8 x float> addrspace(1)* %tmp299, i32 %tmp298 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp301 = load <8 x float> addrspace(1)* %arrayidx300 ; <<8 x float>> [#uses=1]
  %tmp302 = shufflevector <8 x float> %tmp301, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call303 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp297, <4 x float> %tmp302) ; <float> [#uses=1]
  %tmp304 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 3) ; <<8 x float>> [#uses=1]
  %tmp305 = shufflevector <8 x float> %tmp304, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp306 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp307 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx308 = getelementptr inbounds <8 x float> addrspace(1)* %tmp307, i32 %tmp306 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp309 = load <8 x float> addrspace(1)* %arrayidx308 ; <<8 x float>> [#uses=1]
  %tmp310 = shufflevector <8 x float> %tmp309, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call311 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp305, <4 x float> %tmp310) ; <float> [#uses=1]
  %add312 = fadd float %call303, %call311         ; <float> [#uses=1]
  %tmp313 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp314 = insertelement <8 x float> %tmp313, float %add312, i32 3 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp314, <8 x float>* %acc
  %tmp315 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 4) ; <<8 x float>> [#uses=1]
  %tmp316 = shufflevector <8 x float> %tmp315, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp317 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp318 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx319 = getelementptr inbounds <8 x float> addrspace(1)* %tmp318, i32 %tmp317 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp320 = load <8 x float> addrspace(1)* %arrayidx319 ; <<8 x float>> [#uses=1]
  %tmp321 = shufflevector <8 x float> %tmp320, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call322 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp316, <4 x float> %tmp321) ; <float> [#uses=1]
  %tmp323 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 4) ; <<8 x float>> [#uses=1]
  %tmp324 = shufflevector <8 x float> %tmp323, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp325 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp326 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx327 = getelementptr inbounds <8 x float> addrspace(1)* %tmp326, i32 %tmp325 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp328 = load <8 x float> addrspace(1)* %arrayidx327 ; <<8 x float>> [#uses=1]
  %tmp329 = shufflevector <8 x float> %tmp328, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call330 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp324, <4 x float> %tmp329) ; <float> [#uses=1]
  %add331 = fadd float %call322, %call330         ; <float> [#uses=1]
  %tmp332 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp333 = insertelement <8 x float> %tmp332, float %add331, i32 4 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp333, <8 x float>* %acc
  %tmp334 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 5) ; <<8 x float>> [#uses=1]
  %tmp335 = shufflevector <8 x float> %tmp334, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp336 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp337 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx338 = getelementptr inbounds <8 x float> addrspace(1)* %tmp337, i32 %tmp336 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp339 = load <8 x float> addrspace(1)* %arrayidx338 ; <<8 x float>> [#uses=1]
  %tmp340 = shufflevector <8 x float> %tmp339, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call341 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp335, <4 x float> %tmp340) ; <float> [#uses=1]
  %tmp342 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 5) ; <<8 x float>> [#uses=1]
  %tmp343 = shufflevector <8 x float> %tmp342, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp344 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp345 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx346 = getelementptr inbounds <8 x float> addrspace(1)* %tmp345, i32 %tmp344 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp347 = load <8 x float> addrspace(1)* %arrayidx346 ; <<8 x float>> [#uses=1]
  %tmp348 = shufflevector <8 x float> %tmp347, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call349 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp343, <4 x float> %tmp348) ; <float> [#uses=1]
  %add350 = fadd float %call341, %call349         ; <float> [#uses=1]
  %tmp351 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp352 = insertelement <8 x float> %tmp351, float %add350, i32 5 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp352, <8 x float>* %acc
  %tmp353 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 6) ; <<8 x float>> [#uses=1]
  %tmp354 = shufflevector <8 x float> %tmp353, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp355 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp356 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx357 = getelementptr inbounds <8 x float> addrspace(1)* %tmp356, i32 %tmp355 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp358 = load <8 x float> addrspace(1)* %arrayidx357 ; <<8 x float>> [#uses=1]
  %tmp359 = shufflevector <8 x float> %tmp358, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call360 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp354, <4 x float> %tmp359) ; <float> [#uses=1]
  %tmp361 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 6) ; <<8 x float>> [#uses=1]
  %tmp362 = shufflevector <8 x float> %tmp361, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp363 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp364 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx365 = getelementptr inbounds <8 x float> addrspace(1)* %tmp364, i32 %tmp363 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp366 = load <8 x float> addrspace(1)* %arrayidx365 ; <<8 x float>> [#uses=1]
  %tmp367 = shufflevector <8 x float> %tmp366, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call368 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp362, <4 x float> %tmp367) ; <float> [#uses=1]
  %add369 = fadd float %call360, %call368         ; <float> [#uses=1]
  %tmp370 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp371 = insertelement <8 x float> %tmp370, float %add369, i32 6 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp371, <8 x float>* %acc
  %tmp372 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 7) ; <<8 x float>> [#uses=1]
  %tmp373 = shufflevector <8 x float> %tmp372, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %tmp374 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp375 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx376 = getelementptr inbounds <8 x float> addrspace(1)* %tmp375, i32 %tmp374 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp377 = load <8 x float> addrspace(1)* %arrayidx376 ; <<8 x float>> [#uses=1]
  %tmp378 = shufflevector <8 x float> %tmp377, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3> ; <<4 x float>> [#uses=1]
  %call379 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp373, <4 x float> %tmp378) ; <float> [#uses=1]
  %tmp380 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @opencl_DCT_VECTOR_DOT_local_inter, i32 0, i32 7) ; <<8 x float>> [#uses=1]
  %tmp381 = shufflevector <8 x float> %tmp380, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %tmp382 = load i32* %index2                     ; <i32> [#uses=1]
  %tmp383 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx384 = getelementptr inbounds <8 x float> addrspace(1)* %tmp383, i32 %tmp382 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp385 = load <8 x float> addrspace(1)* %arrayidx384 ; <<8 x float>> [#uses=1]
  %tmp386 = shufflevector <8 x float> %tmp385, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7> ; <<4 x float>> [#uses=1]
  %call387 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp381, <4 x float> %tmp386) ; <float> [#uses=1]
  %add388 = fadd float %call379, %call387         ; <float> [#uses=1]
  %tmp389 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp390 = insertelement <8 x float> %tmp389, float %add388, i32 7 ; <<8 x float>> [#uses=1]
  store <8 x float> %tmp390, <8 x float>* %acc
  %tmp391 = load i32* %groupIdy                   ; <i32> [#uses=1]
  %mul392 = mul i32 %tmp391, 8                    ; <i32> [#uses=1]
  %tmp393 = load i32* %k2                         ; <i32> [#uses=1]
  %add394 = add i32 %mul392, %tmp393              ; <i32> [#uses=1]
  %tmp395 = load i32* %step                       ; <i32> [#uses=1]
  %mul396 = mul i32 %add394, %tmp395              ; <i32> [#uses=1]
  %tmp397 = load i32* %groupIdx                   ; <i32> [#uses=1]
  %add398 = add i32 %mul396, %tmp397              ; <i32> [#uses=1]
  store i32 %add398, i32* %idx
  %tmp399 = load <8 x float>* %acc                ; <<8 x float>> [#uses=1]
  %tmp400 = load i32* %idx                        ; <i32> [#uses=1]
  %tmp401 = load <8 x float> addrspace(1)** %output.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx402 = getelementptr inbounds <8 x float> addrspace(1)* %tmp401, i32 %tmp400 ; <<8 x float> addrspace(1)*> [#uses=1]
  store <8 x float> %tmp399, <8 x float> addrspace(1)* %arrayidx402
  ret void
}

declare float @_Z3dotU8__vector4fS_(<4 x float>, <4 x float>)

; CHECK: ret
define void @DCT_CPU(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %dct.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=4]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=10]
  %k2 = alloca i32, align 4                       ; <i32*> [#uses=4]
  %n1 = alloca i32, align 4                       ; <i32*> [#uses=6]
  %n2 = alloca i32, align 4                       ; <i32*> [#uses=11]
  %inter = alloca [64 x float], align 4           ; <[64 x float]*> [#uses=3]
  %step = alloca i32, align 4                     ; <i32*> [#uses=3]
  %inputIndex = alloca i32, align 4               ; <i32*> [#uses=5]
  %dctIndex = alloca i32, align 4                 ; <i32*> [#uses=9]
  %interIndex = alloca i32, align 4               ; <i32*> [#uses=9]
  %outputIndex = alloca i32, align 4              ; <i32*> [#uses=6]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store float addrspace(1)* %input, float addrspace(1)** %input.addr
  store float addrspace(1)* %dct, float addrspace(1)** %dct.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_group_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %groupIdx
  %call1 = call i32 @get_group_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call1, i32* %groupIdy
  %tmp = bitcast [64 x float]* %inter to i8*      ; <i8*> [#uses=1]
  call void @llvm.memset.i32(i8* %tmp, i8 0, i32 256, i32 4)
  %tmp3 = load i32* %width.addr                   ; <i32> [#uses=1]
  store i32 %tmp3, i32* %step
  store i32 0, i32* %inputIndex
  store i32 0, i32* %dctIndex
  store i32 0, i32* %interIndex
  store i32 0, i32* %outputIndex
  %tmp8 = load i32* %groupIdy                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp8, 8                         ; <i32> [#uses=1]
  %tmp9 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul10 = mul i32 %mul, %tmp9                    ; <i32> [#uses=1]
  %tmp11 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %mul12 = mul i32 %tmp11, 8                      ; <i32> [#uses=1]
  %add = add i32 %mul10, %mul12                   ; <i32> [#uses=1]
  store i32 %add, i32* %inputIndex
  store i32 0, i32* %n2
  br label %for.cond

for.cond:                                         ; preds = %for.inc52, %entry
  %tmp13 = load i32* %n2                          ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp13, 8                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end55

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %dctIndex
  store i32 0, i32* %interIndex
  store i32 0, i32* %k1
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc45, %for.body
  %tmp15 = load i32* %k1                          ; <i32> [#uses=1]
  %cmp16 = icmp ult i32 %tmp15, 8                 ; <i1> [#uses=1]
  br i1 %cmp16, label %for.body17, label %for.end48

for.body17:                                       ; preds = %for.cond14
  store i32 0, i32* %n1
  br label %for.cond18

for.cond18:                                       ; preds = %for.inc, %for.body17
  %tmp19 = load i32* %n1                          ; <i32> [#uses=1]
  %cmp20 = icmp ult i32 %tmp19, 8                 ; <i1> [#uses=1]
  br i1 %cmp20, label %for.body21, label %for.end

for.body21:                                       ; preds = %for.cond18
  %tmp22 = load i32* %dctIndex                    ; <i32> [#uses=1]
  %tmp23 = load i32* %n1                          ; <i32> [#uses=1]
  %add24 = add i32 %tmp22, %tmp23                 ; <i32> [#uses=1]
  %tmp25 = load float addrspace(1)** %dct.addr    ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp25, i32 %add24 ; <float addrspace(1)*> [#uses=1]
  %tmp26 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %tmp27 = load i32* %inputIndex                  ; <i32> [#uses=1]
  %tmp28 = load i32* %n1                          ; <i32> [#uses=1]
  %add29 = add i32 %tmp27, %tmp28                 ; <i32> [#uses=1]
  %tmp30 = load float addrspace(1)** %input.addr  ; <float addrspace(1)*> [#uses=1]
  %arrayidx31 = getelementptr inbounds float addrspace(1)* %tmp30, i32 %add29 ; <float addrspace(1)*> [#uses=1]
  %tmp32 = load float addrspace(1)* %arrayidx31   ; <float> [#uses=1]
  %mul33 = fmul float %tmp26, %tmp32              ; <float> [#uses=1]
  %tmp34 = load i32* %interIndex                  ; <i32> [#uses=1]
  %tmp35 = load i32* %n2                          ; <i32> [#uses=1]
  %add36 = add i32 %tmp34, %tmp35                 ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [64 x float]* %inter, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx37 = getelementptr inbounds float* %arraydecay, i32 %add36 ; <float*> [#uses=2]
  %tmp38 = load float* %arrayidx37                ; <float> [#uses=1]
  %add39 = fadd float %tmp38, %mul33              ; <float> [#uses=1]
  store float %add39, float* %arrayidx37
  br label %for.inc

for.inc:                                          ; preds = %for.body21
  %tmp40 = load i32* %n1                          ; <i32> [#uses=1]
  %inc = add i32 %tmp40, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %n1
  br label %for.cond18

for.end:                                          ; preds = %for.cond18
  %tmp41 = load i32* %dctIndex                    ; <i32> [#uses=1]
  %add42 = add i32 %tmp41, 8                      ; <i32> [#uses=1]
  store i32 %add42, i32* %dctIndex
  %tmp43 = load i32* %interIndex                  ; <i32> [#uses=1]
  %add44 = add i32 %tmp43, 8                      ; <i32> [#uses=1]
  store i32 %add44, i32* %interIndex
  br label %for.inc45

for.inc45:                                        ; preds = %for.end
  %tmp46 = load i32* %k1                          ; <i32> [#uses=1]
  %inc47 = add i32 %tmp46, 1                      ; <i32> [#uses=1]
  store i32 %inc47, i32* %k1
  br label %for.cond14

for.end48:                                        ; preds = %for.cond14
  %tmp49 = load i32* %step                        ; <i32> [#uses=1]
  %tmp50 = load i32* %inputIndex                  ; <i32> [#uses=1]
  %add51 = add i32 %tmp50, %tmp49                 ; <i32> [#uses=1]
  store i32 %add51, i32* %inputIndex
  br label %for.inc52

for.inc52:                                        ; preds = %for.end48
  %tmp53 = load i32* %n2                          ; <i32> [#uses=1]
  %inc54 = add i32 %tmp53, 1                      ; <i32> [#uses=1]
  store i32 %inc54, i32* %n2
  br label %for.cond

for.end55:                                        ; preds = %for.cond
  store i32 0, i32* %dctIndex
  %tmp56 = load i32* %groupIdy                    ; <i32> [#uses=1]
  %mul57 = mul i32 %tmp56, 8                      ; <i32> [#uses=1]
  %tmp58 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul59 = mul i32 %mul57, %tmp58                 ; <i32> [#uses=1]
  %tmp60 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %mul61 = mul i32 %tmp60, 8                      ; <i32> [#uses=1]
  %add62 = add i32 %mul59, %mul61                 ; <i32> [#uses=1]
  store i32 %add62, i32* %outputIndex
  store i32 0, i32* %k2
  br label %for.cond63

for.cond63:                                       ; preds = %for.inc115, %for.end55
  %tmp64 = load i32* %k2                          ; <i32> [#uses=1]
  %cmp65 = icmp ult i32 %tmp64, 8                 ; <i1> [#uses=1]
  br i1 %cmp65, label %for.body66, label %for.end118

for.body66:                                       ; preds = %for.cond63
  store i32 0, i32* %interIndex
  store i32 0, i32* %k1
  br label %for.cond67

for.cond67:                                       ; preds = %for.inc106, %for.body66
  %tmp68 = load i32* %k1                          ; <i32> [#uses=1]
  %cmp69 = icmp ult i32 %tmp68, 8                 ; <i1> [#uses=1]
  br i1 %cmp69, label %for.body70, label %for.end109

for.body70:                                       ; preds = %for.cond67
  %tmp71 = load i32* %outputIndex                 ; <i32> [#uses=1]
  %tmp72 = load i32* %k1                          ; <i32> [#uses=1]
  %add73 = add i32 %tmp71, %tmp72                 ; <i32> [#uses=1]
  %tmp74 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx75 = getelementptr inbounds float addrspace(1)* %tmp74, i32 %add73 ; <float addrspace(1)*> [#uses=1]
  store float 0.000000e+000, float addrspace(1)* %arrayidx75
  store i32 0, i32* %n2
  br label %for.cond76

for.cond76:                                       ; preds = %for.inc100, %for.body70
  %tmp77 = load i32* %n2                          ; <i32> [#uses=1]
  %cmp78 = icmp ult i32 %tmp77, 8                 ; <i1> [#uses=1]
  br i1 %cmp78, label %for.body79, label %for.end103

for.body79:                                       ; preds = %for.cond76
  %tmp80 = load i32* %dctIndex                    ; <i32> [#uses=1]
  %tmp81 = load i32* %n2                          ; <i32> [#uses=1]
  %add82 = add i32 %tmp80, %tmp81                 ; <i32> [#uses=1]
  %tmp83 = load float addrspace(1)** %dct.addr    ; <float addrspace(1)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %tmp83, i32 %add82 ; <float addrspace(1)*> [#uses=1]
  %tmp85 = load float addrspace(1)* %arrayidx84   ; <float> [#uses=1]
  %tmp86 = load i32* %interIndex                  ; <i32> [#uses=1]
  %tmp87 = load i32* %n2                          ; <i32> [#uses=1]
  %add88 = add i32 %tmp86, %tmp87                 ; <i32> [#uses=1]
  %arraydecay89 = getelementptr inbounds [64 x float]* %inter, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx90 = getelementptr inbounds float* %arraydecay89, i32 %add88 ; <float*> [#uses=1]
  %tmp91 = load float* %arrayidx90                ; <float> [#uses=1]
  %mul92 = fmul float %tmp85, %tmp91              ; <float> [#uses=1]
  %tmp93 = load i32* %outputIndex                 ; <i32> [#uses=1]
  %tmp94 = load i32* %k1                          ; <i32> [#uses=1]
  %add95 = add i32 %tmp93, %tmp94                 ; <i32> [#uses=1]
  %tmp96 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx97 = getelementptr inbounds float addrspace(1)* %tmp96, i32 %add95 ; <float addrspace(1)*> [#uses=2]
  %tmp98 = load float addrspace(1)* %arrayidx97   ; <float> [#uses=1]
  %add99 = fadd float %tmp98, %mul92              ; <float> [#uses=1]
  store float %add99, float addrspace(1)* %arrayidx97
  br label %for.inc100

for.inc100:                                       ; preds = %for.body79
  %tmp101 = load i32* %n2                         ; <i32> [#uses=1]
  %inc102 = add i32 %tmp101, 1                    ; <i32> [#uses=1]
  store i32 %inc102, i32* %n2
  br label %for.cond76

for.end103:                                       ; preds = %for.cond76
  %tmp104 = load i32* %interIndex                 ; <i32> [#uses=1]
  %add105 = add i32 %tmp104, 8                    ; <i32> [#uses=1]
  store i32 %add105, i32* %interIndex
  br label %for.inc106

for.inc106:                                       ; preds = %for.end103
  %tmp107 = load i32* %k1                         ; <i32> [#uses=1]
  %inc108 = add i32 %tmp107, 1                    ; <i32> [#uses=1]
  store i32 %inc108, i32* %k1
  br label %for.cond67

for.end109:                                       ; preds = %for.cond67
  %tmp110 = load i32* %dctIndex                   ; <i32> [#uses=1]
  %add111 = add i32 %tmp110, 8                    ; <i32> [#uses=1]
  store i32 %add111, i32* %dctIndex
  %tmp112 = load i32* %step                       ; <i32> [#uses=1]
  %tmp113 = load i32* %outputIndex                ; <i32> [#uses=1]
  %add114 = add i32 %tmp113, %tmp112              ; <i32> [#uses=1]
  store i32 %add114, i32* %outputIndex
  br label %for.inc115

for.inc115:                                       ; preds = %for.end109
  %tmp116 = load i32* %k2                         ; <i32> [#uses=1]
  %inc117 = add i32 %tmp116, 1                    ; <i32> [#uses=1]
  store i32 %inc117, i32* %k2
  br label %for.cond63

for.end118:                                       ; preds = %for.cond63
  ret void
}

; CHECK: ret
define void @DCT_CPU_VECTOR(float addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=2]
  %dct.addr = alloca <8 x float> addrspace(1)*, align 4 ; <<8 x float> addrspace(1)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=5]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=3]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=3]
  %k1 = alloca i32, align 4                       ; <i32*> [#uses=11]
  %k2 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %n1 = alloca i32, align 4                       ; <i32*> [#uses=0]
  %n2 = alloca i32, align 4                       ; <i32*> [#uses=5]
  %acc = alloca [64 x float], align 4             ; <[64 x float]*> [#uses=2]
  %inter = alloca [8 x <8 x float>], align 32     ; <[8 x <8 x float>]*> [#uses=2]
  %temp = alloca <8 x float>, align 32            ; <<8 x float>*> [#uses=18]
  %step = alloca i32, align 4                     ; <i32*> [#uses=2]
  %inputIndex = alloca i32, align 4               ; <i32*> [#uses=5]
  %dctIndex = alloca i32, align 4                 ; <i32*> [#uses=1]
  %interIndex = alloca i32, align 4               ; <i32*> [#uses=5]
  %outputIndex = alloca i32, align 4              ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)** %input.addr
  store <8 x float> addrspace(1)* %dct, <8 x float> addrspace(1)** %dct.addr
  store i32 %width, i32* %width.addr
  %call = call i32 @get_group_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %groupIdx
  %call1 = call i32 @get_group_id(i32 1)          ; <i32> [#uses=1]
  store i32 %call1, i32* %groupIdy
  %tmp = load i32* %width.addr                    ; <i32> [#uses=1]
  %div = udiv i32 %tmp, 8                         ; <i32> [#uses=1]
  store i32 %div, i32* %step
  store i32 0, i32* %inputIndex
  store i32 0, i32* %dctIndex
  store i32 0, i32* %interIndex
  store i32 0, i32* %outputIndex
  %tmp6 = load i32* %groupIdy                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp6, 8                         ; <i32> [#uses=1]
  %tmp7 = load i32* %width.addr                   ; <i32> [#uses=1]
  %mul8 = mul i32 %mul, %tmp7                     ; <i32> [#uses=1]
  %div9 = udiv i32 %mul8, 8                       ; <i32> [#uses=1]
  %tmp10 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %add = add i32 %div9, %tmp10                    ; <i32> [#uses=1]
  store i32 %add, i32* %inputIndex
  store i32 0, i32* %n2
  br label %for.cond

for.cond:                                         ; preds = %for.inc57, %entry
  %tmp11 = load i32* %n2                          ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp11, 8                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end60

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %interIndex
  store i32 0, i32* %k1
  br label %for.cond12

for.cond12:                                       ; preds = %for.inc, %for.body
  %tmp13 = load i32* %k1                          ; <i32> [#uses=1]
  %cmp14 = icmp ult i32 %tmp13, 8                 ; <i1> [#uses=1]
  br i1 %cmp14, label %for.body15, label %for.end

for.body15:                                       ; preds = %for.cond12
  %tmp16 = load i32* %k1                          ; <i32> [#uses=1]
  %tmp17 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <8 x float> addrspace(1)* %tmp17, i32 %tmp16 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp18 = load <8 x float> addrspace(1)* %arrayidx ; <<8 x float>> [#uses=1]
  %tmp19 = load i32* %inputIndex                  ; <i32> [#uses=1]
  %tmp20 = load <8 x float> addrspace(1)** %input.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx21 = getelementptr inbounds <8 x float> addrspace(1)* %tmp20, i32 %tmp19 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp22 = load <8 x float> addrspace(1)* %arrayidx21 ; <<8 x float>> [#uses=1]
  %mul23 = fmul <8 x float> %tmp18, %tmp22        ; <<8 x float>> [#uses=1]
  store <8 x float> %mul23, <8 x float>* %temp
  %tmp24 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp25 = extractelement <8 x float> %tmp24, i32 0 ; <float> [#uses=1]
  %tmp26 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp27 = extractelement <8 x float> %tmp26, i32 1 ; <float> [#uses=1]
  %add28 = fadd float %tmp25, %tmp27              ; <float> [#uses=1]
  %tmp29 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp30 = extractelement <8 x float> %tmp29, i32 2 ; <float> [#uses=1]
  %add31 = fadd float %add28, %tmp30              ; <float> [#uses=1]
  %tmp32 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp33 = extractelement <8 x float> %tmp32, i32 3 ; <float> [#uses=1]
  %add34 = fadd float %add31, %tmp33              ; <float> [#uses=1]
  %tmp35 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp36 = extractelement <8 x float> %tmp35, i32 4 ; <float> [#uses=1]
  %add37 = fadd float %add34, %tmp36              ; <float> [#uses=1]
  %tmp38 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp39 = extractelement <8 x float> %tmp38, i32 5 ; <float> [#uses=1]
  %add40 = fadd float %add37, %tmp39              ; <float> [#uses=1]
  %tmp41 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp42 = extractelement <8 x float> %tmp41, i32 6 ; <float> [#uses=1]
  %add43 = fadd float %add40, %tmp42              ; <float> [#uses=1]
  %tmp44 = load <8 x float>* %temp                ; <<8 x float>> [#uses=1]
  %tmp45 = extractelement <8 x float> %tmp44, i32 7 ; <float> [#uses=1]
  %add46 = fadd float %add43, %tmp45              ; <float> [#uses=1]
  %tmp47 = load i32* %interIndex                  ; <i32> [#uses=1]
  %tmp48 = load i32* %n2                          ; <i32> [#uses=1]
  %add49 = add i32 %tmp47, %tmp48                 ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [64 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx50 = getelementptr inbounds float* %arraydecay, i32 %add49 ; <float*> [#uses=1]
  store float %add46, float* %arrayidx50
  %tmp51 = load i32* %interIndex                  ; <i32> [#uses=1]
  %add52 = add i32 %tmp51, 8                      ; <i32> [#uses=1]
  store i32 %add52, i32* %interIndex
  br label %for.inc

for.inc:                                          ; preds = %for.body15
  %tmp53 = load i32* %k1                          ; <i32> [#uses=1]
  %inc = add i32 %tmp53, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %k1
  br label %for.cond12

for.end:                                          ; preds = %for.cond12
  %tmp54 = load i32* %step                        ; <i32> [#uses=1]
  %tmp55 = load i32* %inputIndex                  ; <i32> [#uses=1]
  %add56 = add i32 %tmp55, %tmp54                 ; <i32> [#uses=1]
  store i32 %add56, i32* %inputIndex
  br label %for.inc57

for.inc57:                                        ; preds = %for.end
  %tmp58 = load i32* %n2                          ; <i32> [#uses=1]
  %inc59 = add i32 %tmp58, 1                      ; <i32> [#uses=1]
  store i32 %inc59, i32* %n2
  br label %for.cond

for.end60:                                        ; preds = %for.cond
  store i32 0, i32* %i
  br label %for.cond62

for.cond62:                                       ; preds = %for.inc73, %for.end60
  %tmp63 = load i32* %i                           ; <i32> [#uses=1]
  %cmp64 = icmp slt i32 %tmp63, 8                 ; <i1> [#uses=1]
  br i1 %cmp64, label %for.body65, label %for.end76

for.body65:                                       ; preds = %for.cond62
  %tmp66 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay67 = getelementptr inbounds [64 x float]* %acc, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx68 = getelementptr inbounds float* %arraydecay67, i32 0 ; <float*> [#uses=1]
  %call69 = call <8 x float> @_Z6vload8jPKf(i32 %tmp66, float* %arrayidx68) ; <<8 x float>> [#uses=1]
  %tmp70 = load i32* %i                           ; <i32> [#uses=1]
  %arraydecay71 = getelementptr inbounds [8 x <8 x float>]* %inter, i32 0, i32 0 ; <<8 x float>*> [#uses=1]
  %arrayidx72 = getelementptr inbounds <8 x float>* %arraydecay71, i32 %tmp70 ; <<8 x float>*> [#uses=1]
  store <8 x float> %call69, <8 x float>* %arrayidx72
  br label %for.inc73

for.inc73:                                        ; preds = %for.body65
  %tmp74 = load i32* %i                           ; <i32> [#uses=1]
  %inc75 = add nsw i32 %tmp74, 1                  ; <i32> [#uses=1]
  store i32 %inc75, i32* %i
  br label %for.cond62

for.end76:                                        ; preds = %for.cond62
  %tmp77 = load i32* %groupIdy                    ; <i32> [#uses=1]
  %mul78 = mul i32 %tmp77, 8                      ; <i32> [#uses=1]
  %tmp79 = load i32* %width.addr                  ; <i32> [#uses=1]
  %mul80 = mul i32 %mul78, %tmp79                 ; <i32> [#uses=1]
  %tmp81 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %mul82 = mul i32 %tmp81, 8                      ; <i32> [#uses=1]
  %add83 = add i32 %mul80, %mul82                 ; <i32> [#uses=1]
  store i32 %add83, i32* %outputIndex
  store i32 0, i32* %k2
  br label %for.cond84

for.cond84:                                       ; preds = %for.inc136, %for.end76
  %tmp85 = load i32* %k2                          ; <i32> [#uses=1]
  %cmp86 = icmp ult i32 %tmp85, 8                 ; <i1> [#uses=1]
  br i1 %cmp86, label %for.body87, label %for.end139

for.body87:                                       ; preds = %for.cond84
  store i32 0, i32* %k1
  br label %for.cond88

for.cond88:                                       ; preds = %for.inc129, %for.body87
  %tmp89 = load i32* %k1                          ; <i32> [#uses=1]
  %cmp90 = icmp ult i32 %tmp89, 8                 ; <i1> [#uses=1]
  br i1 %cmp90, label %for.body91, label %for.end132

for.body91:                                       ; preds = %for.cond88
  %tmp92 = load i32* %k2                          ; <i32> [#uses=1]
  %tmp93 = load <8 x float> addrspace(1)** %dct.addr ; <<8 x float> addrspace(1)*> [#uses=1]
  %arrayidx94 = getelementptr inbounds <8 x float> addrspace(1)* %tmp93, i32 %tmp92 ; <<8 x float> addrspace(1)*> [#uses=1]
  %tmp95 = load <8 x float> addrspace(1)* %arrayidx94 ; <<8 x float>> [#uses=1]
  %tmp96 = load i32* %k1                          ; <i32> [#uses=1]
  %arraydecay97 = getelementptr inbounds [8 x <8 x float>]* %inter, i32 0, i32 0 ; <<8 x float>*> [#uses=1]
  %arrayidx98 = getelementptr inbounds <8 x float>* %arraydecay97, i32 %tmp96 ; <<8 x float>*> [#uses=1]
  %tmp99 = load <8 x float>* %arrayidx98          ; <<8 x float>> [#uses=1]
  %mul100 = fmul <8 x float> %tmp95, %tmp99       ; <<8 x float>> [#uses=1]
  store <8 x float> %mul100, <8 x float>* %temp
  %tmp101 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp102 = extractelement <8 x float> %tmp101, i32 0 ; <float> [#uses=1]
  %tmp103 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp104 = extractelement <8 x float> %tmp103, i32 1 ; <float> [#uses=1]
  %add105 = fadd float %tmp102, %tmp104           ; <float> [#uses=1]
  %tmp106 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp107 = extractelement <8 x float> %tmp106, i32 2 ; <float> [#uses=1]
  %add108 = fadd float %add105, %tmp107           ; <float> [#uses=1]
  %tmp109 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp110 = extractelement <8 x float> %tmp109, i32 3 ; <float> [#uses=1]
  %add111 = fadd float %add108, %tmp110           ; <float> [#uses=1]
  %tmp112 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp113 = extractelement <8 x float> %tmp112, i32 4 ; <float> [#uses=1]
  %add114 = fadd float %add111, %tmp113           ; <float> [#uses=1]
  %tmp115 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp116 = extractelement <8 x float> %tmp115, i32 5 ; <float> [#uses=1]
  %add117 = fadd float %add114, %tmp116           ; <float> [#uses=1]
  %tmp118 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp119 = extractelement <8 x float> %tmp118, i32 6 ; <float> [#uses=1]
  %add120 = fadd float %add117, %tmp119           ; <float> [#uses=1]
  %tmp121 = load <8 x float>* %temp               ; <<8 x float>> [#uses=1]
  %tmp122 = extractelement <8 x float> %tmp121, i32 7 ; <float> [#uses=1]
  %add123 = fadd float %add120, %tmp122           ; <float> [#uses=1]
  %tmp124 = load i32* %outputIndex                ; <i32> [#uses=1]
  %tmp125 = load i32* %k1                         ; <i32> [#uses=1]
  %add126 = add i32 %tmp124, %tmp125              ; <i32> [#uses=1]
  %tmp127 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx128 = getelementptr inbounds float addrspace(1)* %tmp127, i32 %add126 ; <float addrspace(1)*> [#uses=1]
  store float %add123, float addrspace(1)* %arrayidx128
  br label %for.inc129

for.inc129:                                       ; preds = %for.body91
  %tmp130 = load i32* %k1                         ; <i32> [#uses=1]
  %inc131 = add i32 %tmp130, 1                    ; <i32> [#uses=1]
  store i32 %inc131, i32* %k1
  br label %for.cond88

for.end132:                                       ; preds = %for.cond88
  %tmp133 = load i32* %width.addr                 ; <i32> [#uses=1]
  %tmp134 = load i32* %outputIndex                ; <i32> [#uses=1]
  %add135 = add i32 %tmp134, %tmp133              ; <i32> [#uses=1]
  store i32 %add135, i32* %outputIndex
  br label %for.inc136

for.inc136:                                       ; preds = %for.end132
  %tmp137 = load i32* %k2                         ; <i32> [#uses=1]
  %inc138 = add i32 %tmp137, 1                    ; <i32> [#uses=1]
  store i32 %inc138, i32* %k2
  br label %for.cond84

for.end139:                                       ; preds = %for.cond84
  ret void
}

declare <8 x float> @_Z6vload8jPKf(i32, float*)
