; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'file.s'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @BypassCase7
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: footer
; CHECK: ret
define void @BypassCase7(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %0 = sext i32 %call to i64
  %arrayidx = getelementptr inbounds float addrspace(1)* %a, i64 %0
  %tmp2 = load float addrspace(1)* %arrayidx, align 4
  %cmp = fcmp ogt float %tmp2, 3.000000e+000
  br i1 %cmp, label %if.then, label %if.then3

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  br label %for.header

if.then3:                                         ; preds = %entry
  %add = fadd float %tmp2, 1.000000e+001
  br label %for.header
  
for.header:															  		    ; preds = %if.then3, %if.then, %for.body 
	%temp = phi float [ %add, %if.then3 ], [ %mul, %if.then ], [ %temp.0, %for.body ]
	%inc = phi i32 [ 0, %if.then3 ], [ 0, %if.then ], [ %inc60, %for.body ]
  %add6996 = add nsw i32 %arg2, %arg1
  %cmp7097 = icmp sgt i32 %add6996, %inc
  br i1 %cmp7097, label %for.body, label %if.loopexit

for.body:                                         ; preds = %for.header
  %temp.0 = fadd float %temp, 1.000000e+000
  %inc60 = add nsw i32 %inc, 1
  br label %for.header

if.loopexit:                                     ; preds = %for.header
  %arrayidx22 = getelementptr inbounds float addrspace(1)* %b, i64 %0
  store float %temp, float addrspace(1)* %arrayidx22, align 4
  ret void
}

declare i32 @get_global_id(i32) readnone
