// RUN: %clang_cc1 -stack-protector 2 -Rpass-missed=inline -O2 -verify %s -emit-llvm-only

void side_effect(void);

void foo(void) {
  side_effect();
}

// INTEL_CUSTOMIZATION
// expected-remark@+5 {{foo not inlined into bar because it should never be inlined (cost=never): no alwaysinline attribute}}
// END INTEL_CUSTOMIZATION
// expected-remark@+3 {{foo will not be inlined into bar: stack protected callee but caller requested no stack protector}}
__attribute__((no_stack_protector))
void bar(void) {
  foo();
}

// INTEL_CUSTOMIZATION
// expected-remark@+4 {{bar not inlined into baz because it should never be inlined (cost=never): no alwaysinline attribute}}
// END INTEL_CUSTOMIZATION
// expected-remark@+2 {{bar will not be inlined into baz: stack protected caller but callee requested no stack protector}}
void baz(void) {
  bar();
}

void ssp_callee(void);

// No issue; matching stack protections.
void ssp_caller(void) {
  ssp_callee();
}

__attribute__((no_stack_protector))
void nossp_callee(void);

// No issue; matching stack protections.
__attribute__((no_stack_protector))
void nossp_caller(void) {
  nossp_callee();
}
