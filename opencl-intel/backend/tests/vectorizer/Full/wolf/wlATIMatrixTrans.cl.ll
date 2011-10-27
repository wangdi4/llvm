; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIMatrixTrans.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_matrixTranspose_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_matrixTranspose_parameters = appending global [164 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(3))) *, uint const, uint const, uint const\00", section "llvm.metadata" ; <[164 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i32, i32)* @matrixTranspose to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_matrixTranspose_locals to i8*), i8* getelementptr inbounds ([164 x i8]* @opencl_matrixTranspose_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @matrixTranspose(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(3)* %block, i32 %width, i32 %height, i32 %blockSize) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %input.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %block.addr = alloca float addrspace(3)*, align 4 ; <float addrspace(3)**> [#uses=3]
  %width.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %height.addr = alloca i32, align 4              ; <i32*> [#uses=2]
  %blockSize.addr = alloca i32, align 4           ; <i32*> [#uses=5]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %globalIdy = alloca i32, align 4                ; <i32*> [#uses=2]
  %localIdx = alloca i32, align 4                 ; <i32*> [#uses=4]
  %localIdy = alloca i32, align 4                 ; <i32*> [#uses=4]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=2]
  %groupIdy = alloca i32, align 4                 ; <i32*> [#uses=2]
  %targetGlobalIdx = alloca i32, align 4          ; <i32*> [#uses=2]
  %targetGlobalIdy = alloca i32, align 4          ; <i32*> [#uses=2]
  %targetIndex = alloca i32, align 4              ; <i32*> [#uses=2]
  %sourceIndex = alloca i32, align 4              ; <i32*> [#uses=2]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store float addrspace(1)* %input, float addrspace(1)** %input.addr
  store float addrspace(3)* %block, float addrspace(3)** %block.addr
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
  %tmp6 = load float addrspace(1)** %input.addr   ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp6, i32 %add ; <float addrspace(1)*> [#uses=1]
  %tmp7 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  %tmp8 = load i32* %localIdy                     ; <i32> [#uses=1]
  %tmp9 = load i32* %blockSize.addr               ; <i32> [#uses=1]
  %mul10 = mul i32 %tmp8, %tmp9                   ; <i32> [#uses=1]
  %tmp11 = load i32* %localIdx                    ; <i32> [#uses=1]
  %add12 = add i32 %mul10, %tmp11                 ; <i32> [#uses=1]
  %tmp13 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx14 = getelementptr inbounds float addrspace(3)* %tmp13, i32 %add12 ; <float addrspace(3)*> [#uses=1]
  store float %tmp7, float addrspace(3)* %arrayidx14
  call void @barrier(i32 1)
  %call16 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call16, i32* %groupIdx
  %call18 = call i32 @get_group_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call18, i32* %groupIdy
  %tmp20 = load i32* %groupIdy                    ; <i32> [#uses=1]
  %tmp21 = load i32* %blockSize.addr              ; <i32> [#uses=1]
  %mul22 = mul i32 %tmp20, %tmp21                 ; <i32> [#uses=1]
  %tmp23 = load i32* %localIdy                    ; <i32> [#uses=1]
  %add24 = add i32 %mul22, %tmp23                 ; <i32> [#uses=1]
  store i32 %add24, i32* %targetGlobalIdx
  %tmp26 = load i32* %groupIdx                    ; <i32> [#uses=1]
  %tmp27 = load i32* %blockSize.addr              ; <i32> [#uses=1]
  %mul28 = mul i32 %tmp26, %tmp27                 ; <i32> [#uses=1]
  %tmp29 = load i32* %localIdx                    ; <i32> [#uses=1]
  %add30 = add i32 %mul28, %tmp29                 ; <i32> [#uses=1]
  store i32 %add30, i32* %targetGlobalIdy
  %tmp32 = load i32* %targetGlobalIdy             ; <i32> [#uses=1]
  %tmp33 = load i32* %height.addr                 ; <i32> [#uses=1]
  %mul34 = mul i32 %tmp32, %tmp33                 ; <i32> [#uses=1]
  %tmp35 = load i32* %targetGlobalIdx             ; <i32> [#uses=1]
  %add36 = add i32 %mul34, %tmp35                 ; <i32> [#uses=1]
  store i32 %add36, i32* %targetIndex
  %tmp38 = load i32* %localIdy                    ; <i32> [#uses=1]
  %tmp39 = load i32* %blockSize.addr              ; <i32> [#uses=1]
  %mul40 = mul i32 %tmp38, %tmp39                 ; <i32> [#uses=1]
  %tmp41 = load i32* %localIdx                    ; <i32> [#uses=1]
  %add42 = add i32 %mul40, %tmp41                 ; <i32> [#uses=1]
  store i32 %add42, i32* %sourceIndex
  %tmp43 = load i32* %sourceIndex                 ; <i32> [#uses=1]
  %tmp44 = load float addrspace(3)** %block.addr  ; <float addrspace(3)*> [#uses=1]
  %arrayidx45 = getelementptr inbounds float addrspace(3)* %tmp44, i32 %tmp43 ; <float addrspace(3)*> [#uses=1]
  %tmp46 = load float addrspace(3)* %arrayidx45   ; <float> [#uses=1]
  %tmp47 = load i32* %targetIndex                 ; <i32> [#uses=1]
  %tmp48 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx49 = getelementptr inbounds float addrspace(1)* %tmp48, i32 %tmp47 ; <float addrspace(1)*> [#uses=1]
  store float %tmp46, float addrspace(1)* %arrayidx49
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

declare i32 @get_group_id(i32)
