; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,function(require<dpcpp-kernel-weighted-inst-count-analysis>)' %s -S | FileCheck %s

; FIXME move to a transform pass.

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]

define void @__pipe_global_ctor() {
entry:
  ret void
}

; CHECK: define void @__pipe_global_ctor() {
; CHECK-NOT: recommended_vector_length
