; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt-tpv -mem2reg -simplifycfg -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes="vpo-paropt-tpv,mem2reg,function(simplifycfg)" -S %s | FileCheck %s

; The compiler is expected to emit the call __kmpc_threadprivate_cached.
; It also checks whether the attribute thread_private is accepted or not.

; CHECK: @__tpv_ptr_a = internal global i8** null, align 64
; CHECK: @__tpv_ptr_b = internal global i8** null, align 64
; CHECK:  %{{.*}} = call i8* @__kmpc_threadprivate_cached({{.*}}@__tpv_ptr_a)
; CHECK:  %{{.*}} = call i8* @__kmpc_threadprivate_cached({{.*}}@__tpv_ptr_b)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = thread_private global i32 0, align 4
@b = thread_private global i32 0, align 4

define void @foo() local_unnamed_addr {
entry:
  %0 = load i32, i32* @a, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* @b, align 4
  ret void
}
