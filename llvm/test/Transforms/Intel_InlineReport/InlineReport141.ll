; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that a call to llvm.memset is deleted because it is zero length.

; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: DELETE: llvm.memset.p0i8.i64 {{.*}}Zero length memory function

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

define void @foo(i8* nonnull %0) {
  call void @llvm.memset.p0i8.i64(i8* nonnull align 1 %0, i8 0, i64 0, i1 false)
  ret void
}
