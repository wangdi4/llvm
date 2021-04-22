// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -triple x86_64-unknown-linux-gnu -verify %s -emit-llvm -o - | FileCheck %s
//
// We should emit a warning, not an error, if attribute 'tls_model' is
// applied to the non-tls variable. CQ#376466.
//

int non_tls_var  __attribute__((tls_model("local-dynamic"))); // expected-warning {{'tls_model' attribute only applies to thread-local variables}}
__thread int tls_var   __attribute__((tls_model("local-dynamic")));

// CHECK-NOT: @non_tls_var = {{.*}}thread_local
// CHECK: @tls_var = {{.*}}thread_local

int main(void)
{
  return 0;
}

