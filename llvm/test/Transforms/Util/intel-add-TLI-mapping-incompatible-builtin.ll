; Test to check that InjectTLIMappings bails out for calls that are incompatible redeclaration
; of builtin library functions.

; RUN: opt -vector-library=SVML       -inject-tli-mappings        -S < %s | FileCheck %s
; RUN: opt -vector-library=MASSV      -inject-tli-mappings        -S < %s | FileCheck %s

; CHECK-NOT: @llvm.compiler.used
; CHECK: %call = tail call signext i8 (...) @pow() 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  %call = tail call signext i8 (...) @pow()
  ret void
}

declare dso_local signext i8 @pow(...) local_unnamed_addr
