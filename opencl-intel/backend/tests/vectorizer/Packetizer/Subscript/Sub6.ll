; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -subscript  -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;CHECK: kernel
;CHECK: @scatter_I8
;CHECK: ret void

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"


define void @kernel(i8 addrspace(1)* nocapture %src, i32 %j) nounwind {
entry:
  %call = tail call i64 @get_global_id(i32 0) nounwind readnone
  %0 = mul i64 %call, 7
  %mul = add i64 %0, 56
  %idxprom = and i64 %mul, 4294967295
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i64 %idxprom
  store i8 zeroinitializer, i8 addrspace(1)* %arrayidx, align 4
  ret void
}

;CHECK: kernel2
;CHECK-NOT: @scatter_I8
;CHECK: ret void

define void @kernel2(i8 addrspace(1)* nocapture %src, i32 %j) nounwind {
entry:
  %call = tail call i64 @get_global_id(i32 0) nounwind readnone
  %0 = mul i64 %call, 7
  %mul = add i64 %0, 56
  %idxprom = and i64 %mul, 12345
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i64 %idxprom
  store i8 zeroinitializer, i8 addrspace(1)* %arrayidx, align 4
  ret void
}


declare i64 @get_global_id(i32 )
