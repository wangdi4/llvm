; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlSum.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_wlSumKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlSumKernel_parameters = appending global [93 x i8] c"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @wlSumKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlSumKernel_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_wlSumKernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @wlSumKernel(i32 addrspace(1)* %input, i32 addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %output.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr
  store i32 addrspace(1)* %output, i32 addrspace(1)** %output.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %tid
  %tmp = load i32* %tid                           ; <i32> [#uses=1]
  %tmp1 = load i32 addrspace(1)** %input.addr     ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp1, i32 %tmp ; <i32 addrspace(1)*> [#uses=1]
  %tmp2 = load i32 addrspace(1)* %arrayidx        ; <i32> [#uses=1]
  %add = add nsw i32 %tmp2, 1                     ; <i32> [#uses=1]
  %tmp3 = load i32* %tid                          ; <i32> [#uses=1]
  %tmp4 = load i32 addrspace(1)** %output.addr    ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx5 = getelementptr inbounds i32 addrspace(1)* %tmp4, i32 %tmp3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %add, i32 addrspace(1)* %arrayidx5
  ret void
}

declare i32 @get_global_id(i32)
