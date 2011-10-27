; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIFastWalsh.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_fastWalshTransform_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_fastWalshTransform_parameters = appending global [53 x i8] c"float __attribute__((address_space(1))) *, int const\00", section "llvm.metadata" ; <[53 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32)* @fastWalshTransform to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_fastWalshTransform_locals to i8*), i8* getelementptr inbounds ([53 x i8]* @opencl_fastWalshTransform_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @fastWalshTransform(float addrspace(1)* %tArray, i32 %step) nounwind {
entry:
  %tArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=5]
  %step.addr = alloca i32, align 4                ; <i32*> [#uses=5]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %group = alloca i32, align 4                    ; <i32*> [#uses=2]
  %pair = alloca i32, align 4                     ; <i32*> [#uses=4]
  %match = alloca i32, align 4                    ; <i32*> [#uses=3]
  %T1 = alloca float, align 4                     ; <float*> [#uses=3]
  %T2 = alloca float, align 4                     ; <float*> [#uses=3]
  store float addrspace(1)* %tArray, float addrspace(1)** %tArray.addr
  store i32 %step, i32* %step.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %tmp = load i32* %tid                           ; <i32> [#uses=1]
  %tmp1 = load i32* %step.addr                    ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp1                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp1         ; <i32> [#uses=1]
  %rem = urem i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %rem, i32* %group
  %tmp3 = load i32* %step.addr                    ; <i32> [#uses=1]
  %mul = mul i32 2, %tmp3                         ; <i32> [#uses=1]
  %tmp4 = load i32* %tid                          ; <i32> [#uses=1]
  %tmp5 = load i32* %step.addr                    ; <i32> [#uses=2]
  %cmp6 = icmp eq i32 0, %tmp5                    ; <i1> [#uses=1]
  %sel7 = select i1 %cmp6, i32 1, i32 %tmp5       ; <i32> [#uses=1]
  %div = udiv i32 %tmp4, %sel7                    ; <i32> [#uses=1]
  %mul8 = mul i32 %mul, %div                      ; <i32> [#uses=1]
  %tmp9 = load i32* %group                        ; <i32> [#uses=1]
  %add = add i32 %mul8, %tmp9                     ; <i32> [#uses=1]
  store i32 %add, i32* %pair
  %tmp11 = load i32* %pair                        ; <i32> [#uses=1]
  %tmp12 = load i32* %step.addr                   ; <i32> [#uses=1]
  %add13 = add i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  store i32 %add13, i32* %match
  %tmp15 = load i32* %pair                        ; <i32> [#uses=1]
  %tmp16 = load float addrspace(1)** %tArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp16, i32 %tmp15 ; <float addrspace(1)*> [#uses=1]
  %tmp17 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  store float %tmp17, float* %T1
  %tmp19 = load i32* %match                       ; <i32> [#uses=1]
  %tmp20 = load float addrspace(1)** %tArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx21 = getelementptr inbounds float addrspace(1)* %tmp20, i32 %tmp19 ; <float addrspace(1)*> [#uses=1]
  %tmp22 = load float addrspace(1)* %arrayidx21   ; <float> [#uses=1]
  store float %tmp22, float* %T2
  %tmp23 = load float* %T1                        ; <float> [#uses=1]
  %tmp24 = load float* %T2                        ; <float> [#uses=1]
  %add25 = fadd float %tmp23, %tmp24              ; <float> [#uses=1]
  %tmp26 = load i32* %pair                        ; <i32> [#uses=1]
  %tmp27 = load float addrspace(1)** %tArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx28 = getelementptr inbounds float addrspace(1)* %tmp27, i32 %tmp26 ; <float addrspace(1)*> [#uses=1]
  store float %add25, float addrspace(1)* %arrayidx28
  %tmp29 = load float* %T1                        ; <float> [#uses=1]
  %tmp30 = load float* %T2                        ; <float> [#uses=1]
  %sub = fsub float %tmp29, %tmp30                ; <float> [#uses=1]
  %tmp31 = load i32* %match                       ; <i32> [#uses=1]
  %tmp32 = load float addrspace(1)** %tArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx33 = getelementptr inbounds float addrspace(1)* %tmp32, i32 %tmp31 ; <float addrspace(1)*> [#uses=1]
  store float %sub, float addrspace(1)* %arrayidx33
  ret void
}

declare i32 @get_global_id(i32)
