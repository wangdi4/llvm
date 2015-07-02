; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc  -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll | FileCheck %s

; ModuleID = 'add_rnd_cons.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: simpleBranch
; CHECK: WI-RunOnFunction 4   %x = phi i32 [ %x2.x0, %true ], [ %x1, %false ]

define void @simpleBranch(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out, i32 %un1, i32 %un2) nounwind {
  %id = tail call i32 @_Z13get_global_idj(i32 0) nounwind

  %in1 = getelementptr inbounds i32 addrspace(1)* %in, i32 %id
  %in2 = load i32 addrspace(1)* %in1, align 4

  %cmp = icmp slt i32 %id, %in2 
  br i1 %cmp, label %true, label %false

true:
  %x0 = add i32 %un1, 1
  %cmp1 = icmp slt i32 %x0, %un2 
  br i1 %cmp1, label %next, label %next1
next:
  %x2 = sub  i32 %un1, 1
  br label %next1
next1:
%x3 = phi i32 [ %x0, %true ], [ %x2, %next ]
  br label %end
false:
  %x1 = add i32 %un2, 1
  br label %end
end:
  %x = phi i32 [ %x3, %next1 ], [ %x1, %false ]
  %out1 = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 %x, i32 addrspace(1)* %out1, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

