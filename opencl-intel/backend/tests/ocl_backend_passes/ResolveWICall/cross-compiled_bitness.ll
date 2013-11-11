; RUN: oclopt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i686-pc-linux"


declare i32 @printf(i8 addrspace(2)*, ...)

@str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define void @test_printf() {
  ; this function crashes opt if the pass tries to generate 64-bit code
  ; CHECK: @opencl_printf(i8 addrspace(2)*, i8*, i32*)
  tail call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* bitcast ([4 x i8]* @str to i8 addrspace(2)*), i32 3) nounwind
  ret void
}
