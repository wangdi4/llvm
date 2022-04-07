; RUN: %oclopt -winstcounter %s -S 2>&1 | FileCheck %s

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]

define void @__pipe_global_ctor() {
entry:
  ret void
}

; CHECK: define void @__pipe_global_ctor() {
; CHECK-NOT: recommended_vector_length
