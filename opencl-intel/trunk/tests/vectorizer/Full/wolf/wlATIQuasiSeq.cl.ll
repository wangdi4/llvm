; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIQuasiSeq.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_QuasiRandomSequence_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_QuasiRandomSequence_parameters = appending global [126 x i8] c"float __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[126 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(3)*)* @QuasiRandomSequence to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_QuasiRandomSequence_locals to i8*), i8* getelementptr inbounds ([126 x i8]* @opencl_QuasiRandomSequence_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @QuasiRandomSequence(float addrspace(1)* %output, i32 addrspace(1)* %input, i32 addrspace(3)* %shared) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %input.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %shared.addr = alloca i32 addrspace(3)*, align 4 ; <i32 addrspace(3)**> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=4]
  %local_id = alloca i32, align 4                 ; <i32*> [#uses=5]
  %group_id = alloca i32, align 4                 ; <i32*> [#uses=2]
  %temp = alloca i32, align 4                     ; <i32*> [#uses=4]
  %k = alloca i32, align 4                        ; <i32*> [#uses=7]
  %mask = alloca i32, align 4                     ; <i32*> [#uses=2]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr
  store i32 addrspace(3)* %shared, i32 addrspace(3)** %shared.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %call1 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %local_id
  %call2 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %group_id
  %tmp = load i32* %local_id                      ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp, 32                    ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %tmp3 = load i32* %group_id                     ; <i32> [#uses=1]
  %mul = mul i32 %tmp3, 32                        ; <i32> [#uses=1]
  %tmp4 = load i32* %local_id                     ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp4                      ; <i32> [#uses=1]
  %tmp5 = load i32 addrspace(1)** %input.addr     ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp5, i32 %add ; <i32 addrspace(1)*> [#uses=1]
  %tmp6 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  %tmp7 = load i32* %local_id                     ; <i32> [#uses=1]
  %tmp8 = load i32 addrspace(3)** %shared.addr    ; <i32 addrspace(3)*> [#uses=1]
  %arrayidx9 = getelementptr inbounds i32 addrspace(3)* %tmp8, i32 %tmp7 ; <i32 addrspace(3)*> [#uses=1]
  store i32 %tmp6, i32 addrspace(3)* %arrayidx9
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @barrier(i32 1)
  store i32 0, i32* %temp
  store i32 0, i32* %k
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %tmp12 = load i32* %k                           ; <i32> [#uses=1]
  %cmp13 = icmp slt i32 %tmp12, 32                ; <i1> [#uses=1]
  br i1 %cmp13, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp15 = load i32* %k                           ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp15 to float              ; <float> [#uses=1]
  %call16 = call float @_Z3powff(float 2.000000e+000, float %conv) ; <float> [#uses=1]
  %conv17 = fptosi float %call16 to i32           ; <i32> [#uses=1]
  store i32 %conv17, i32* %mask
  %tmp18 = load i32* %local_id                    ; <i32> [#uses=1]
  %tmp19 = load i32* %mask                        ; <i32> [#uses=1]
  %and = and i32 %tmp18, %tmp19                   ; <i32> [#uses=1]
  %tmp20 = load i32* %k                           ; <i32> [#uses=1]
  %shr = lshr i32 %and, %tmp20                    ; <i32> [#uses=1]
  %tmp21 = load i32* %k                           ; <i32> [#uses=1]
  %tmp22 = load i32 addrspace(3)** %shared.addr   ; <i32 addrspace(3)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds i32 addrspace(3)* %tmp22, i32 %tmp21 ; <i32 addrspace(3)*> [#uses=1]
  %tmp24 = load i32 addrspace(3)* %arrayidx23     ; <i32> [#uses=1]
  %mul25 = mul i32 %shr, %tmp24                   ; <i32> [#uses=1]
  %tmp26 = load i32* %temp                        ; <i32> [#uses=1]
  %xor = xor i32 %tmp26, %mul25                   ; <i32> [#uses=1]
  store i32 %xor, i32* %temp
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp27 = load i32* %k                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp27, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %k
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp28 = load i32* %global_id                   ; <i32> [#uses=1]
  %cmp29 = icmp eq i32 %tmp28, 0                  ; <i1> [#uses=1]
  br i1 %cmp29, label %if.then31, label %if.else

if.then31:                                        ; preds = %for.end
  %tmp32 = load i32* %global_id                   ; <i32> [#uses=1]
  %tmp33 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 %tmp32 ; <float addrspace(1)*> [#uses=1]
  store float 0.000000e+000, float addrspace(1)* %arrayidx34
  br label %if.end42

if.else:                                          ; preds = %for.end
  %tmp35 = load i32* %temp                        ; <i32> [#uses=1]
  %conv36 = uitofp i32 %tmp35 to float            ; <float> [#uses=1]
  %call37 = call float @_Z3powff(float 2.000000e+000, float 3.200000e+001) ; <float> [#uses=3]
  %cmp38 = fcmp oeq float 0.000000e+000, %call37  ; <i1> [#uses=1]
  %sel = select i1 %cmp38, float 1.000000e+000, float %call37 ; <float> [#uses=0]
  %div = fdiv float %conv36, %call37              ; <float> [#uses=1]
  %tmp39 = load i32* %global_id                   ; <i32> [#uses=1]
  %tmp40 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx41 = getelementptr inbounds float addrspace(1)* %tmp40, i32 %tmp39 ; <float addrspace(1)*> [#uses=1]
  store float %div, float addrspace(1)* %arrayidx41
  br label %if.end42

if.end42:                                         ; preds = %if.else, %if.then31
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)

declare float @_Z3powff(float, float)
