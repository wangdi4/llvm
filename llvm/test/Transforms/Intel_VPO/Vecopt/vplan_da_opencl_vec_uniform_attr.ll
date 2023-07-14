; RUN: opt -passes=vplan-vec -vplan-dump-da -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; This test checks that VPlan divergence analysis is able to find
; opencl-vec-uniform-return attributes in either the call attributes
; or function attributes.

; Four calls in order are:
; - attribute not present       -> Divergent
; - attribute on call site only -> Uniform
; - attribute on function only  -> Uniform
; - attribute on both           -> Uniform
define dso_local void @foo(i32 %a) {
; CHECK: Divergent: [Shape: Random] i32 [[VP1:%.*]] = call i32 %a ptr @without_attr
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[VP2:%.*]] = call i32 %a ptr @without_attr
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[VP3:%.*]] = call i32 %a ptr @with_attr
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[VP4:%.*]] = call i32 %a ptr @with_attr
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body
body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %body ]
  %no_attr_anywhere = call i32 @without_attr(i32 %a)
  %attr_on_call = call i32 @without_attr(i32 %a) #0
  %attr_on_func = call i32 @with_attr(i32 %a)
  %attr_on_both = call i32 @with_attr(i32 %a) #0
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 10
  br i1 %exitcond, label %exit, label %body
exit:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare i32 @with_attr(i32) #0
declare i32 @without_attr(i32)

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { "opencl-vec-uniform-return" }
