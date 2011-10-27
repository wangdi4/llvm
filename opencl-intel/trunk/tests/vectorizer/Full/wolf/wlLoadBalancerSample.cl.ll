; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlLoadBalancerSample.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_initMatrix_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_initMatrix_parameters = appending global [50 x i8] c"int, int, int __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[50 x i8]*> [#uses=1]
@opencl_initMatrixMult_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_initMatrixMult_parameters = appending global [50 x i8] c"int, int, int __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[50 x i8]*> [#uses=1]
@opencl_copyMatrix_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_copyMatrix_parameters = appending global [86 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int\00", section "llvm.metadata" ; <[86 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32, i32, i32 addrspace(1)*)* @initMatrix to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_initMatrix_locals to i8*), i8* getelementptr inbounds ([50 x i8]* @opencl_initMatrix_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, i32, i32 addrspace(1)*)* @initMatrixMult to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_initMatrixMult_locals to i8*), i8* getelementptr inbounds ([50 x i8]* @opencl_initMatrixMult_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @copyMatrix to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_copyMatrix_locals to i8*), i8* getelementptr inbounds ([86 x i8]* @opencl_copyMatrix_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @initMatrix(i32 %value, i32 %arraySize, i32 addrspace(1)* %outArray) nounwind {
entry:
  %value.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %arraySize.addr = alloca i32, align 4           ; <i32*> [#uses=4]
  %outArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=4]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=4]
  %numElements = alloca i32, align 4              ; <i32*> [#uses=5]
  %prevElements = alloca i32, align 4             ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  store i32 %value, i32* %value.addr
  store i32 %arraySize, i32* %arraySize.addr
  store i32 addrspace(1)* %outArray, i32 addrspace(1)** %outArray.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %tmp1 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %cmp = icmp sge i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end

if.end:                                           ; preds = %entry
  %call3 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call3, i32* %globalSize
  %tmp5 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %tmp6 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp7 = icmp eq i32 0, %tmp6                    ; <i1> [#uses=1]
  %sel = select i1 %cmp7, i32 1, i32 %tmp6        ; <i32> [#uses=1]
  %div = sdiv i32 %tmp5, %sel                     ; <i32> [#uses=1]
  store i32 %div, i32* %numElements
  %tmp9 = load i32* %numElements                  ; <i32> [#uses=1]
  store i32 %tmp9, i32* %prevElements
  %tmp10 = load i32* %globalSize                  ; <i32> [#uses=1]
  %sub = sub i32 %tmp10, 1                        ; <i32> [#uses=1]
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %cmp12 = icmp eq i32 %sub, %tmp11               ; <i1> [#uses=1]
  br i1 %cmp12, label %if.then13, label %if.end19

if.then13:                                        ; preds = %if.end
  %tmp14 = load i32* %arraySize.addr              ; <i32> [#uses=1]
  %tmp15 = load i32* %globalSize                  ; <i32> [#uses=2]
  %cmp16 = icmp eq i32 0, %tmp15                  ; <i1> [#uses=1]
  %sel17 = select i1 %cmp16, i32 1, i32 %tmp15    ; <i32> [#uses=1]
  %rem = srem i32 %tmp14, %sel17                  ; <i32> [#uses=1]
  %tmp18 = load i32* %numElements                 ; <i32> [#uses=1]
  %add = add nsw i32 %tmp18, %rem                 ; <i32> [#uses=1]
  store i32 %add, i32* %numElements
  br label %if.end19

if.end19:                                         ; preds = %if.then13, %if.end
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end19
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %tmp22 = load i32* %numElements                 ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp21, %tmp22            ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp24 = load i32* %value.addr                  ; <i32> [#uses=1]
  %tmp25 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp26 = load i32* %prevElements                ; <i32> [#uses=1]
  %mul = mul i32 %tmp25, %tmp26                   ; <i32> [#uses=1]
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %add28 = add nsw i32 %mul, %tmp27               ; <i32> [#uses=1]
  %tmp29 = load i32 addrspace(1)** %outArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp29, i32 %add28 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp24, i32 addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp30 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp30, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %if.then, %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

; CHECK: ret
define void @initMatrixMult(i32 %value, i32 %arraySize, i32 addrspace(1)* %outArray) nounwind {
entry:
  %value.addr = alloca i32, align 4               ; <i32*> [#uses=2]
  %arraySize.addr = alloca i32, align 4           ; <i32*> [#uses=4]
  %outArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=4]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=4]
  %numElements = alloca i32, align 4              ; <i32*> [#uses=5]
  %prevElements = alloca i32, align 4             ; <i32*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=5]
  store i32 %value, i32* %value.addr
  store i32 %arraySize, i32* %arraySize.addr
  store i32 addrspace(1)* %outArray, i32 addrspace(1)** %outArray.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %tmp1 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %cmp = icmp sge i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end

if.end:                                           ; preds = %entry
  %call3 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call3, i32* %globalSize
  %tmp5 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %tmp6 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp7 = icmp eq i32 0, %tmp6                    ; <i1> [#uses=1]
  %sel = select i1 %cmp7, i32 1, i32 %tmp6        ; <i32> [#uses=1]
  %div = sdiv i32 %tmp5, %sel                     ; <i32> [#uses=1]
  store i32 %div, i32* %numElements
  %tmp9 = load i32* %numElements                  ; <i32> [#uses=1]
  store i32 %tmp9, i32* %prevElements
  %tmp10 = load i32* %globalSize                  ; <i32> [#uses=1]
  %sub = sub i32 %tmp10, 1                        ; <i32> [#uses=1]
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %cmp12 = icmp eq i32 %sub, %tmp11               ; <i1> [#uses=1]
  br i1 %cmp12, label %if.then13, label %if.end19

if.then13:                                        ; preds = %if.end
  %tmp14 = load i32* %arraySize.addr              ; <i32> [#uses=1]
  %tmp15 = load i32* %globalSize                  ; <i32> [#uses=2]
  %cmp16 = icmp eq i32 0, %tmp15                  ; <i1> [#uses=1]
  %sel17 = select i1 %cmp16, i32 1, i32 %tmp15    ; <i32> [#uses=1]
  %rem = srem i32 %tmp14, %sel17                  ; <i32> [#uses=1]
  %tmp18 = load i32* %numElements                 ; <i32> [#uses=1]
  %add = add nsw i32 %tmp18, %rem                 ; <i32> [#uses=1]
  store i32 %add, i32* %numElements
  br label %if.end19

if.end19:                                         ; preds = %if.then13, %if.end
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end19
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %tmp22 = load i32* %numElements                 ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp21, %tmp22            ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp24 = load i32* %value.addr                  ; <i32> [#uses=1]
  %tmp25 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp26 = load i32* %prevElements                ; <i32> [#uses=1]
  %mul = mul i32 %tmp25, %tmp26                   ; <i32> [#uses=1]
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %add28 = add nsw i32 %mul, %tmp27               ; <i32> [#uses=1]
  %tmp29 = load i32 addrspace(1)** %outArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp29, i32 %add28 ; <i32 addrspace(1)*> [#uses=2]
  %tmp30 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %mul31 = mul i32 %tmp30, %tmp24                 ; <i32> [#uses=1]
  store i32 %mul31, i32 addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp32 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp32, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %if.then, %for.cond
  ret void
}

; CHECK: ret
define void @copyMatrix(i32 addrspace(1)* %inArray, i32 addrspace(1)* %outArray, i32 %arraySize) nounwind {
entry:
  %inArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %outArray.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %arraySize.addr = alloca i32, align 4           ; <i32*> [#uses=4]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=5]
  %globalSize = alloca i32, align 4               ; <i32*> [#uses=4]
  %numElements = alloca i32, align 4              ; <i32*> [#uses=5]
  %prevElements = alloca i32, align 4             ; <i32*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  store i32 addrspace(1)* %inArray, i32 addrspace(1)** %inArray.addr
  store i32 addrspace(1)* %outArray, i32 addrspace(1)** %outArray.addr
  store i32 %arraySize, i32* %arraySize.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %tmp1 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %cmp = icmp sge i32 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end

if.end:                                           ; preds = %entry
  %call3 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  store i32 %call3, i32* %globalSize
  %tmp5 = load i32* %arraySize.addr               ; <i32> [#uses=1]
  %tmp6 = load i32* %globalSize                   ; <i32> [#uses=2]
  %cmp7 = icmp eq i32 0, %tmp6                    ; <i1> [#uses=1]
  %sel = select i1 %cmp7, i32 1, i32 %tmp6        ; <i32> [#uses=1]
  %div = sdiv i32 %tmp5, %sel                     ; <i32> [#uses=1]
  store i32 %div, i32* %numElements
  %tmp9 = load i32* %numElements                  ; <i32> [#uses=1]
  store i32 %tmp9, i32* %prevElements
  %tmp10 = load i32* %globalSize                  ; <i32> [#uses=1]
  %sub = sub i32 %tmp10, 1                        ; <i32> [#uses=1]
  %tmp11 = load i32* %gid                         ; <i32> [#uses=1]
  %cmp12 = icmp eq i32 %sub, %tmp11               ; <i1> [#uses=1]
  br i1 %cmp12, label %if.then13, label %if.end19

if.then13:                                        ; preds = %if.end
  %tmp14 = load i32* %arraySize.addr              ; <i32> [#uses=1]
  %tmp15 = load i32* %globalSize                  ; <i32> [#uses=2]
  %cmp16 = icmp eq i32 0, %tmp15                  ; <i1> [#uses=1]
  %sel17 = select i1 %cmp16, i32 1, i32 %tmp15    ; <i32> [#uses=1]
  %rem = srem i32 %tmp14, %sel17                  ; <i32> [#uses=1]
  %tmp18 = load i32* %numElements                 ; <i32> [#uses=1]
  %add = add nsw i32 %tmp18, %rem                 ; <i32> [#uses=1]
  store i32 %add, i32* %numElements
  br label %if.end19

if.end19:                                         ; preds = %if.then13, %if.end
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end19
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %tmp22 = load i32* %numElements                 ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp21, %tmp22            ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp24 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp25 = load i32* %prevElements                ; <i32> [#uses=1]
  %mul = mul i32 %tmp24, %tmp25                   ; <i32> [#uses=1]
  %tmp26 = load i32* %i                           ; <i32> [#uses=1]
  %add27 = add nsw i32 %mul, %tmp26               ; <i32> [#uses=1]
  %tmp28 = load i32 addrspace(1)** %inArray.addr  ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp28, i32 %add27 ; <i32 addrspace(1)*> [#uses=1]
  %tmp29 = load i32 addrspace(1)* %arrayidx       ; <i32> [#uses=1]
  %tmp30 = load i32* %gid                         ; <i32> [#uses=1]
  %tmp31 = load i32* %prevElements                ; <i32> [#uses=1]
  %mul32 = mul i32 %tmp30, %tmp31                 ; <i32> [#uses=1]
  %tmp33 = load i32* %i                           ; <i32> [#uses=1]
  %add34 = add nsw i32 %mul32, %tmp33             ; <i32> [#uses=1]
  %tmp35 = load i32 addrspace(1)** %outArray.addr ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i32 addrspace(1)* %tmp35, i32 %add34 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp29, i32 addrspace(1)* %arrayidx36
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp37 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp37, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %if.then, %for.cond
  ret void
}
