// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -verify 

typedef struct S { } S;
int main()
{
  char byte_1, byte_2, byte_3;
  float f;
  double d;
  void* v;
  S s_1;
  // validate too many params results in an error for intel specific calls
  __atomic_store_explicit(&byte_1, 5, __ATOMIC_RELAXED, 0); // expected-error {{too many arguments to function call, expected 3, have 4}}
  __atomic_store_explicit_1(&byte_1, 5, __ATOMIC_RELAXED, 0); // expected-error {{too many arguments to function call, expected 3, have 4}}
  __atomic_store_explicit_16(&byte_1, 5, __ATOMIC_RELAXED, 0); // expected-error {{too many arguments to function call, expected 3, have 4}}

  __atomic_load_explicit(&byte_1, &byte_2, __ATOMIC_RELAXED); // expected-error {{too many arguments to function call, expected 2, have 3}}
  __atomic_exchange_explicit(&byte_1, &byte_2, &byte_3, __ATOMIC_RELAXED); // expected-error {{too many arguments to function call, expected 3, have 4}}
  __atomic_compare_exchange_weak_explicit(&byte_1, &byte_2, 8, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED, 0); // expected-error {{too many arguments to function call, expected 5, have 6}}

  // It is an error to specify a float, void, or struct ptr to a atomic operation
  __atomic_store_explicit(&f, 5, __ATOMIC_RELAXED); //expected-error {{address argument to atomic operation must be a pointer to integer or pointer ('float *' invalid)}}
  __atomic_store_explicit(&d, 5, __ATOMIC_RELAXED); //expected-error {{address argument to atomic operation must be a pointer to integer or pointer ('double *' invalid)}}
  __atomic_store_explicit(v, 5, __ATOMIC_RELAXED); //expected-error {{address argument to atomic operation must be a pointer to integer or pointer ('void *' invalid)}}
  __atomic_store_explicit(&s_1, 5, __ATOMIC_RELAXED); //expected-error {{address argument to atomic operation must be a pointer to integer or pointer ('S *' (aka 'struct S *') invalid)}}

  // Most type errors disappear when size is specified, except for missing a ptr when necessary
  __atomic_compare_exchange_weak_explicit_1(&f, d, f, 0, 0); // expected-error {{address argument to atomic builtin must be a pointer ('double' invalid)}}  
  __atomic_compare_exchange_weak_explicit_1(f, &d, f, 0, 0); // expected-error {{address argument to atomic builtin must be a pointer ('float' invalid)}}
}
