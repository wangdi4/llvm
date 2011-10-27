; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


; CHECK: ret
define void @test(i32 addrspace(1)* nocapture %pOutputs) nounwind {
  %1 = tail call i32 @get_local_id(i32 0) nounwind
  %2 = getelementptr inbounds i32 addrspace(1)* %pOutputs, i32 %1
  %3 = load i32 addrspace(1)* %2, align 4
  %4 = tail call i32 @get_global_size(i32 %3) nounwind
  store i32 %4, i32 addrspace(1)* %pOutputs, align 4
  ret void
}

declare i32 @get_global_size(i32)

declare i32 @get_local_id(i32)