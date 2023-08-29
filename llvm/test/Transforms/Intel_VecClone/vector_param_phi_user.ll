;Check the phi user of vector parameter is correctly updated.
;RUN: opt -passes=vec-clone -S < %s | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;CHECK-LABEL: @_ZGVeN16vvv_foo
;CHECK:       if.then:
;CHECK-NEXT:    %vec.a.gep = getelementptr i32, ptr %vec.a, i32 %index
;CHECK-NEXT:    %vec.a.elem = load i32, ptr %vec.a.gep, align 4
;CHECK-NEXT:    br label %if.end
;CHECK:       if.else:
;CHECK-NEXT:    %vec.c.gep = getelementptr i32, ptr %vec.c, i32 %index
;CHECK-NEXT:    %vec.c.elem = load i32, ptr %vec.c.gep, align 4
;CHECK-NEXT:    br label %if.end
;CHECK:       if.end:
;CHECK-NEXT:    %[[#]] = phi i32 [ %vec.a.elem, %if.then ], [ %vec.c.elem, %if.else ]

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc void @foo(i32 %c, i32 %a, ptr addrspace(1) noalias %b) unnamed_addr #0 {
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
