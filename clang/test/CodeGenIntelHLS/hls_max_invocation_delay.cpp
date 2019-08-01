//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

__attribute__((hls_max_invocation_delay(8))) void foo1() {}
// CHECK: @_Z4foo1v{{.*}}!max_invocation_delay [[CFOO:![0-9]+]]

__attribute__((ihc_component))
__attribute__((hls_max_invocation_delay(8))) void
foo2() {}
// CHECK: @_Z4foo2v{{.*}}!max_invocation_delay [[CFOO:![0-9]+]]

template <int N>
__attribute__((hls_max_invocation_delay(N))) void tfoo() {}

void call_it() {
  tfoo<512>();
  //CHECK: @_Z4tfooILi512EEvv{{.*}}!max_invocation_delay [[CTFOO:![0-9]+]]
}

//CHECK: [[CFOO]] = !{i32 8}
//CHECK: [[CTFOO]] = !{i32 512}
