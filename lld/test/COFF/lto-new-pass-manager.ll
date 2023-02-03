; REQUIRES: x86
; RUN: llvm-as %s -o %t.obj

<<<<<<< HEAD
; RUN: lld-link %t.obj -entry:main -opt:ltonewpassmanager -opt:ltodebugpassmanager -mllvm:-opaque-pointers 2>&1 | FileCheck %s --check-prefix=ENABLED
=======
; RUN: lld-link %t.obj -entry:main -opt:ltodebugpassmanager 2>&1 | FileCheck %s --check-prefix=ENABLED
>>>>>>> ae5efe97618355e7552b0cb3c6790f3d8e8f8554
; ENABLED: Running pass: InstCombinePass

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.11.0"

define dso_local i32 @main(i32 %argc, ptr nocapture readnone %0) local_unnamed_addr {
entry:
  ret i32 %argc
}
