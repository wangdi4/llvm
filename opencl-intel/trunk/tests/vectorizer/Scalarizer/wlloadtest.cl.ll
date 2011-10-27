; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlloadtest.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_wlloadtestKernel_a_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlloadtestKernel_a_parameters = appending global [93 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_wlloadtestKernel_b_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlloadtestKernel_b_parameters = appending global [93 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @wlloadtestKernel_a to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlloadtestKernel_a_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_wlloadtestKernel_a_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @wlloadtestKernel_b to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlloadtestKernel_b_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_wlloadtestKernel_b_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @wlloadtestKernel_a(i32 addrspace(1)* %input, i32 addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %output.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=4]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %a = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %in = alloca i32, align 4                       ; <i32*> [#uses=1]
  %out = alloca i32, align 4                      ; <i32*> [#uses=1]
  %j = alloca i32, align 4                        ; <i32*> [#uses=4]
  store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr
  store i32 addrspace(1)* %output, i32 addrspace(1)** %output.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  store i32 5, i32* %a
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %i
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %tmp1 = load i32 addrspace(1)** %input.addr     ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp1, i32 %tmp ; <i32 addrspace(1)*> [#uses=1]
  %tmp2 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  store i32 %tmp2, i32* %in
  %tmp4 = load i32* %i                            ; <i32> [#uses=1]
  %tmp5 = load i32 addrspace(1)** %output.addr    ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx6 = getelementptr inbounds i32 addrspace(1)* %tmp5, i32 %tmp4 ; <i32 addrspace(1)*> [#uses=1]
  %tmp7 = load i32 addrspace(1)* %arrayidx6       ; <i32> [#uses=1]
  store i32 %tmp7, i32* %out
  store i32 0, i32* %j
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp9 = load i32* %j                            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp9, 20000                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp10 = load i32* %a                           ; <i32> [#uses=1]
  %tmp11 = load i32* %a                           ; <i32> [#uses=1]
  %add = add nsw i32 %tmp11, %tmp10               ; <i32> [#uses=1]
  store i32 %add, i32* %a
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp12 = load i32* %j                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp12, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp13 = load i32* %a                           ; <i32> [#uses=1]
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load i32 addrspace(1)** %input.addr    ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx16 = getelementptr inbounds i32 addrspace(1)* %tmp15, i32 %tmp14 ; <i32 addrspace(1)*> [#uses=1]
  %tmp17 = load i32 addrspace(1)* %arrayidx16     ; <i32> [#uses=1]
  %mul = mul i32 %tmp13, %tmp17                   ; <i32> [#uses=1]
  %tmp18 = load i32* %i                           ; <i32> [#uses=1]
  %tmp19 = load i32 addrspace(1)** %output.addr   ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds i32 addrspace(1)* %tmp19, i32 %tmp18 ; <i32 addrspace(1)*> [#uses=1]
  %tmp21 = load i32 addrspace(1)* %arrayidx20     ; <i32> [#uses=1]
  %add22 = add nsw i32 %mul, %tmp21               ; <i32> [#uses=1]
  %tmp23 = load i32* %i                           ; <i32> [#uses=1]
  %tmp24 = load i32 addrspace(1)** %output.addr   ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i32 addrspace(1)* %tmp24, i32 %tmp23 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add22, i32 addrspace(1)* %arrayidx25
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @wlloadtestKernel_b(i32 addrspace(1)* %input, i32 addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=3]
  %output.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=4]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %a = alloca i32, align 4                        ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %in = alloca i32, align 4                       ; <i32*> [#uses=1]
  %out = alloca i32, align 4                      ; <i32*> [#uses=1]
  %j = alloca i32, align 4                        ; <i32*> [#uses=4]
  store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr
  store i32 addrspace(1)* %output, i32 addrspace(1)** %output.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  store i32 5, i32* %a
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %i
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %tmp1 = load i32 addrspace(1)** %input.addr     ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp1, i32 %tmp ; <i32 addrspace(1)*> [#uses=1]
  %tmp2 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  store i32 %tmp2, i32* %in
  %tmp4 = load i32* %i                            ; <i32> [#uses=1]
  %tmp5 = load i32 addrspace(1)** %output.addr    ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx6 = getelementptr inbounds i32 addrspace(1)* %tmp5, i32 %tmp4 ; <i32 addrspace(1)*> [#uses=1]
  %tmp7 = load i32 addrspace(1)* %arrayidx6       ; <i32> [#uses=1]
  store i32 %tmp7, i32* %out
  store i32 0, i32* %j
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp9 = load i32* %j                            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp9, 20000                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp10 = load i32* %a                           ; <i32> [#uses=1]
  %tmp11 = load i32* %a                           ; <i32> [#uses=1]
  %add = add nsw i32 %tmp11, %tmp10               ; <i32> [#uses=1]
  store i32 %add, i32* %a
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp12 = load i32* %j                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp12, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %j
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp13 = load i32* %a                           ; <i32> [#uses=1]
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %tmp15 = load i32 addrspace(1)** %input.addr    ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx16 = getelementptr inbounds i32 addrspace(1)* %tmp15, i32 %tmp14 ; <i32 addrspace(1)*> [#uses=1]
  %tmp17 = load i32 addrspace(1)* %arrayidx16     ; <i32> [#uses=1]
  %mul = mul i32 %tmp13, %tmp17                   ; <i32> [#uses=1]
  %tmp18 = load i32* %i                           ; <i32> [#uses=1]
  %tmp19 = load i32 addrspace(1)** %output.addr   ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds i32 addrspace(1)* %tmp19, i32 %tmp18 ; <i32 addrspace(1)*> [#uses=1]
  %tmp21 = load i32 addrspace(1)* %arrayidx20     ; <i32> [#uses=1]
  %add22 = add nsw i32 %mul, %tmp21               ; <i32> [#uses=1]
  %tmp23 = load i32* %i                           ; <i32> [#uses=1]
  %tmp24 = load i32 addrspace(1)** %output.addr   ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i32 addrspace(1)* %tmp24, i32 %tmp23 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add22, i32 addrspace(1)* %arrayidx25
  ret void
}
