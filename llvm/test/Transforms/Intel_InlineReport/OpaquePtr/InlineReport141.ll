; RUN: opt -opaque-pointers -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that a call to llvm.memset is deleted because it is zero length.

; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: DELETE: llvm.memset.p0.i64 {{.*}}Zero length memory function

define void @foo(ptr nonnull %arg) {
bb:
  call void @llvm.memset.p0.i64(ptr nonnull align 1 %arg, i8 0, i64 0, i1 false)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nocallback nofree nounwind willreturn writeonly }
