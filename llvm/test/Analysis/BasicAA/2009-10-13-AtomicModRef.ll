; RUN: opt -aa-pipeline=basic-aa -passes=gvn,instcombine -S < %s | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -gvn -instcombine -S | FileCheck --check-prefix=CHECK-SUBS %s
target datalayout = "E-p:64:64:64-a0:0:8-f32:32:32-f64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-v64:64:64-v128:128:128"

define i8 @foo(i8* %ptr) {
  %P = getelementptr i8, i8* %ptr, i32 0
  %Q = getelementptr i8, i8* %ptr, i32 1
; CHECK: getelementptr
; INTEL
; CHECK-SUBS: llvm.intel.subscript
  %X = load i8, i8* %P
  %Y = atomicrmw add i8* %Q, i8 1 monotonic
  %Z = load i8, i8* %P
; CHECK-NOT: = load
; INTEL
; CHECK-SUBS-NOT: = load
  %A = sub i8 %X, %Z
  ret i8 %A
; CHECK: ret i8 0
}
