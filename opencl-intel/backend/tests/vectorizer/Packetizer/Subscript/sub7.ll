; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -subscript  -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; This testcase checks that we do not fail nor generate scatters for v4i8 types if -subscript-v4i8 is not specified.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK-NOT: scatter
;CHECK: ret void


define void @kernel(i8 addrspace(1)* nocapture %src, i32 %j) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = mul i64 %call, 7
  %mul = add i64 %0, 56
  %idxprom = and i64 %mul, 4294967295
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %idxprom
  store i8 zeroinitializer, i8 addrspace(1)* %arrayidx, align 4
  ret void
}

;CHECK: kernel2
;CHECK-NOT: scatter
;CHECK: ret void

define void @kernel2(i8 addrspace(1)* nocapture %src, i32 %j) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = mul i64 %call, 7
  %mul = add i64 %0, 56
  %idxprom = and i64 %mul, 12345
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %idxprom
  store i8 zeroinitializer, i8 addrspace(1)* %arrayidx, align 4
  ret void
}

;CHECK: kernel_int
;CHECK-NOT: scatter
;CHECK: ret void

define void @kernel_int(i8 addrspace(1)* nocapture %src, i32 %j) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %mul = mul i64 %call, 30064771072
  %sext = add i64 %mul, 240518168576
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %src, i64 %idxprom
  store i8 zeroinitializer, i8 addrspace(1)* %arrayidx, align 4
  ret void
}

declare i64 @_Z13get_global_idj(i32 )
