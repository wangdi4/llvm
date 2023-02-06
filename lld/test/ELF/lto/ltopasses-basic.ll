; REQUIRES: x86
; RUN: llvm-as %s -o %t.o
; INTEL RUN: ld.lld -mllvm -opaque-pointers %t.o -o %t.so -save-temps -mllvm -debug-pass=Arguments -shared 2>&1 | FileCheck %s --check-prefix=MLLVM
; INTEL_CUSTOMIZATION
; RUN: llvm-dis -opaque-pointers %t.so.0.4.opt.bc -o - | FileCheck %s
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @ctor, ptr null }]
define void @ctor() {
  ret void
}

; `@ctor` doesn't do anything and so the optimizer should kill it, leaving no ctors
; CHECK: @llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer

; MLLVM: Pass Arguments:
