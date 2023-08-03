; CMPLRLLVM-8626: This test verifies that va_arg instruction is supported
; by Andersens Analysis.
; RUN: opt < %s -passes='require<anders-aa>'  -print-anders-points-to -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; foo:%p and foo:%q basically point to bar:%call1.
;
; CHECK: foo:p --> ({{[0-9a-f]+}}): bar:call1
; CHECK: foo:q     --> same as ({{[0-9a-f]+}}) foo:p

define dso_local void @foo(i32 %num, ...) local_unnamed_addr #0 {
entry:
  %arguments = alloca ptr, align 8
  %copy = alloca ptr, align 8
  %0 = bitcast ptr %arguments to ptr
  call void @llvm.va_start(ptr nonnull %0)

  %p = va_arg ptr %arguments, ptr
  store i8 0, ptr %p, align 4

  %1 = bitcast ptr %copy to ptr
  call void @llvm.va_copy(ptr %1, ptr %0)

  %q = va_arg ptr %copy, ptr
  store i8 1, ptr %q, align 4

  call void @llvm.va_end(ptr nonnull %0)
  ret void
}

define dso_local i32 @bar() local_unnamed_addr {
entry:
  %call1 = tail call ptr @malloc(i64 16)
  %add1 = getelementptr i8, ptr %call1, i64 8
  tail call void (i32, ...) @foo(i32 undef, ptr %call1, ptr %add1)
  ret i32 undef
}

declare dso_local noalias ptr @malloc(i64)
declare void @llvm.va_start(ptr)
declare void @llvm.va_end(ptr)
declare void @llvm.va_copy(ptr, ptr)
