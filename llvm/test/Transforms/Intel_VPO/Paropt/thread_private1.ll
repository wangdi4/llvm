; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S %s | FileCheck %s

; Verify that vpo-paropt-tpv works with bisect limit 0:
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-tpv -S -opt-bisect-limit=0 %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S -opt-bisect-limit=0 %s | FileCheck %s

; The compiler is expected to emit the call __kmpc_threadprivate_cached.
; The compiler isn't expected to emit the call __kmpc_global_thread_num.
; It also checks whether the attribute thread_private as well as mtfunc
; are accepted or not.

; CHECK: @__tpv_ptr_a = internal global ptr null, align 64
; CHECK: %{{.*}} = call ptr @__kmpc_threadprivate_cached({{.*}}@__tpv_ptr_a)
; CHECK-NOT:  %{{.*}} = tail call i32 @__kmpc_global_thread_num({{.*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%ident_t = type { i32, i32, i32, i32, ptr }

@a = thread_private global i32 0, align 4
@.str.1 = private unnamed_addr constant [20 x i8] c"*** TEST FAILED ***\00", align 1
@.str.2 = private unnamed_addr constant [20 x i8] c"a = %d (must be 5)\0A\00", align 1

declare i32 @puts(ptr nocapture readonly) local_unnamed_addr
declare void @exit(i32) local_unnamed_addr

define void @foo(ptr %.global_tid.) local_unnamed_addr #0 {
entry:
  %call = tail call i32 @puts(ptr @.str.1)
  %0 = load i32, ptr @a, align 4
  %call1 = tail call i32 (ptr, ...) @printf(ptr @.str.2, i32 %0)
  ret void
}

declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr
attributes #0 = { "mt-func"="true" }
