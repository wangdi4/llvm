; RUN: opt -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S %s | FileCheck %s

; Verify that vpo-paropt-tpv works with bisect limit 0:
; RUN: opt -vpo-paropt-tpv -S -opt-bisect-limit=0 %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S -opt-bisect-limit=0 %s | FileCheck %s

; The compiler is expected to emit the call __kmpc_threadprivate_cached.
; The compiler isn't expected to emit the call __kmpc_global_thread_num.
; It also checks whether the attribute thread_private as well as mtfunc
; are accepted or not.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%ident_t = type { i32, i32, i32, i32, i8* }

@a = thread_private global i32 0, align 4
@.str.1 = private unnamed_addr constant [20 x i8] c"*** TEST FAILED ***\00", align 1
@.str.2 = private unnamed_addr constant [20 x i8] c"a = %d (must be 5)\0A\00", align 1

declare i32 @puts(i8* nocapture readonly) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare void @exit(i32) local_unnamed_addr 

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* %.global_tid.) local_unnamed_addr #0 {
entry:
  %call = tail call i32 @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.1, i64 0, i64 0))
  %0 = load i32, i32* @a, align 4
  %call1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.2, i64 0, i64 0), i32 %0)
  ret void
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr 

attributes #0 = { "mt-func"="true" }

; CHECK:  %{{.*}} = call i8* @__kmpc_threadprivate_cached({{.*}})
; CHECK-NOT:  %{{.*}} = tail call i32 @__kmpc_global_thread_num({{.*}})

