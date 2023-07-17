; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,function(require<sycl-kernel-weighted-inst-count-analysis>)' %s -S | FileCheck %s

; FIXME move to a transform pass.

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]

define void @__pipe_global_ctor() {
entry:
  ret void
}

; CHECK: define void @__pipe_global_ctor() {
; CHECK-NOT: recommended_vector_length
