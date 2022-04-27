; RUN: opt -vpo-paropt-tpv -mem2reg -simplifycfg -S %s | FileCheck %s

; The compiler is expected to emit the call __kmpc_threadprivate_cached.
; It also checks whether the attribute thread_private is accepted or not.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = thread_private global i32 0, align 4
@b = thread_private global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr {
entry:
  %0 = load i32, i32* @a, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* @b, align 4
  ret void
}

; CHECK:  %{{.*}} = call i8* @__kmpc_threadprivate_cached({{.*}})
