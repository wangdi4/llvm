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
  %arguments = alloca i8*, align 8
  %copy = alloca i8*, align 8
  %0 = bitcast i8** %arguments to i8*
  call void @llvm.va_start(i8* nonnull %0)

  %p = va_arg i8** %arguments, i8*
  store i8 0, i8* %p, align 4

  %1 = bitcast i8** %copy to i8*
  call void @llvm.va_copy(i8* %1, i8* %0)

  %q = va_arg i8** %copy, i8*
  store i8 1, i8* %q, align 4

  call void @llvm.va_end(i8* nonnull %0)
  ret void
}

define dso_local i32 @bar() local_unnamed_addr {
entry:
  %call1 = tail call i8* @malloc(i64 16)
  %add1 = getelementptr i8, i8* %call1, i64 8
  tail call void (i32, ...) @foo(i32 undef, i8* %call1, i8* %add1)
  ret i32 undef
}

declare dso_local noalias i8* @malloc(i64)
declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)
declare void @llvm.va_copy(i8*, i8*)
