; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlATIReduction.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_reduce_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_reduce_parameters = appending global [128 x i8] c"uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(3))) *\00", section "llvm.metadata" ; <[128 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, <4 x i32> addrspace(3)*)* @reduce to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_reduce_locals to i8*), i8* getelementptr inbounds ([128 x i8]* @opencl_reduce_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @reduce(<4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)* %output, <4 x i32> addrspace(3)* %sdata) nounwind {
entry:
  %input.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x i32> addrspace(1)*, align 4 ; <<4 x i32> addrspace(1)**> [#uses=2]
  %sdata.addr = alloca <4 x i32> addrspace(3)*, align 4 ; <<4 x i32> addrspace(3)**> [#uses=5]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=6]
  %bid = alloca i32, align 4                      ; <i32*> [#uses=2]
  %gid = alloca i32, align 4                      ; <i32*> [#uses=2]
  %localSize = alloca i32, align 4                ; <i32*> [#uses=2]
  %s = alloca i32, align 4                        ; <i32*> [#uses=6]
  store <4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)** %input.addr
  store <4 x i32> addrspace(1)* %output, <4 x i32> addrspace(1)** %output.addr
  store <4 x i32> addrspace(3)* %sdata, <4 x i32> addrspace(3)** %sdata.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %call1 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %bid
  %call2 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call2, i32* %gid
  %call3 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call3, i32* %localSize
  %tmp = load i32* %gid                           ; <i32> [#uses=1]
  %tmp4 = load <4 x i32> addrspace(1)** %input.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i32> addrspace(1)* %tmp4, i32 %tmp ; <<4 x i32> addrspace(1)*> [#uses=1]
  %tmp5 = load <4 x i32> addrspace(1)* %arrayidx  ; <<4 x i32>> [#uses=1]
  %tmp6 = load i32* %tid                          ; <i32> [#uses=1]
  %tmp7 = load <4 x i32> addrspace(3)** %sdata.addr ; <<4 x i32> addrspace(3)*> [#uses=1]
  %arrayidx8 = getelementptr inbounds <4 x i32> addrspace(3)* %tmp7, i32 %tmp6 ; <<4 x i32> addrspace(3)*> [#uses=1]
  store <4 x i32> %tmp5, <4 x i32> addrspace(3)* %arrayidx8
  call void @barrier(i32 1)
  %tmp10 = load i32* %localSize                   ; <i32> [#uses=1]
  %div = udiv i32 %tmp10, 2                       ; <i32> [#uses=1]
  store i32 %div, i32* %s
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp11 = load i32* %s                           ; <i32> [#uses=1]
  %cmp = icmp ugt i32 %tmp11, 0                   ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp12 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp13 = load i32* %s                           ; <i32> [#uses=1]
  %cmp14 = icmp ult i32 %tmp12, %tmp13            ; <i1> [#uses=1]
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp15 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp16 = load i32* %s                           ; <i32> [#uses=1]
  %add = add i32 %tmp15, %tmp16                   ; <i32> [#uses=1]
  %tmp17 = load <4 x i32> addrspace(3)** %sdata.addr ; <<4 x i32> addrspace(3)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds <4 x i32> addrspace(3)* %tmp17, i32 %add ; <<4 x i32> addrspace(3)*> [#uses=1]
  %tmp19 = load <4 x i32> addrspace(3)* %arrayidx18 ; <<4 x i32>> [#uses=1]
  %tmp20 = load i32* %tid                         ; <i32> [#uses=1]
  %tmp21 = load <4 x i32> addrspace(3)** %sdata.addr ; <<4 x i32> addrspace(3)*> [#uses=1]
  %arrayidx22 = getelementptr inbounds <4 x i32> addrspace(3)* %tmp21, i32 %tmp20 ; <<4 x i32> addrspace(3)*> [#uses=2]
  %tmp23 = load <4 x i32> addrspace(3)* %arrayidx22 ; <<4 x i32>> [#uses=1]
  %add24 = add <4 x i32> %tmp23, %tmp19           ; <<4 x i32>> [#uses=1]
  store <4 x i32> %add24, <4 x i32> addrspace(3)* %arrayidx22
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  call void @barrier(i32 1)
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp25 = load i32* %s                           ; <i32> [#uses=1]
  %shr = lshr i32 %tmp25, 1                       ; <i32> [#uses=1]
  store i32 %shr, i32* %s
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp26 = load i32* %tid                         ; <i32> [#uses=1]
  %cmp27 = icmp eq i32 %tmp26, 0                  ; <i1> [#uses=1]
  br i1 %cmp27, label %if.then28, label %if.end35

if.then28:                                        ; preds = %for.end
  %tmp29 = load <4 x i32> addrspace(3)** %sdata.addr ; <<4 x i32> addrspace(3)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds <4 x i32> addrspace(3)* %tmp29, i32 0 ; <<4 x i32> addrspace(3)*> [#uses=1]
  %tmp31 = load <4 x i32> addrspace(3)* %arrayidx30 ; <<4 x i32>> [#uses=1]
  %tmp32 = load i32* %bid                         ; <i32> [#uses=1]
  %tmp33 = load <4 x i32> addrspace(1)** %output.addr ; <<4 x i32> addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds <4 x i32> addrspace(1)* %tmp33, i32 %tmp32 ; <<4 x i32> addrspace(1)*> [#uses=1]
  store <4 x i32> %tmp31, <4 x i32> addrspace(1)* %arrayidx34
  br label %if.end35

if.end35:                                         ; preds = %if.then28, %for.end
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)
