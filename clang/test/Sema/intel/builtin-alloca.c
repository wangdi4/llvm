// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// expected-no-diagnostics

void foo(void) {
  // Builtin _alloca should be always available in ICC compatibility mode.
  void* ptr = _alloca(sizeof(int));
}
