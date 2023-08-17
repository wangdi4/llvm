;; Have the original:
;;    - 'weak' linkage
;;    - 'protected' visibility
;; transferred over.

; RUN: opt -passes=vec-clone -S < %s | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define weak protected fastcc void @_ZGVeN16vvv_foo(

; Function Attrs: convergent noinline norecurse nounwind
define weak protected fastcc void @foo(i32 %c, i32 %a, ptr addrspace(1) noalias %b) unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %c, %a
  br i1 %cmp, label %if.then, label %if.else
if.then:
  br label %if.end
if.else:
  br label %if.end
if.end:
  %0 = phi i32 [%a, %if.then], [%c, %if.else]
  store i32 %0, ptr addrspace(1) %b, align 4
  ret void
}

attributes #0 = { "vector-variants"="_ZGVeN16vvv_foo" }
