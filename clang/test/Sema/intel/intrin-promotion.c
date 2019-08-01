// RUN: %clang_cc1 %s -verify -emit-llvm -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DTEST1
// RUN: %clang_cc1 %s -verify -emit-llvm -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DTEST2
// RUN: %clang_cc1 %s -verify -emit-llvm -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DTEST3
// RUN: %clang_cc1 %s -verify -emit-llvm -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DTEST4

#include <intrin-promotion.h>

void uses_builtin_directly(__m128d A){
  // expected-warning@-1{{Function target features +avx512f added due to call to an intrinsic}}
  // expected-note@+1{{Intrinsic called here}}
  (void)__builtin_ia32_vcvtsd2si32(A, 4);
}

void uses_intrin_function(){
  // expected-warning@-1{{Function target features +avx512f added due to call to an intrinsic}}
  // expected-note@+1{{Intrinsic called here}}
  avx512f_intrin();
}

// Macros required because error happens in CodeGen which immediately terminates compilation.
#ifdef TEST1
// Should not be promoted, only functions that use definitions.
void uses_fwd_decl() {
  fwd_decl_intrin(); // expected-error{{always_inline function 'fwd_decl_intrin' requires target feature 'avx512f'}}
}
#endif // TEST1

#ifdef TEST2
__attribute__((always_inline, target("avx512f")))
inline void not_in_header(){}

// Should not be promoted, only uses one not in header.
void uses_not_in_header() {
  not_in_header();// expected-error{{always_inline function 'not_in_header' requires target feature 'avx512f'}}
}

#endif // TEST2

#ifdef TEST3
// should not be promoted, is always inline.
__attribute__((always_inline))
void has_always_inline(){
  avx512f_intrin(); // expected-error{{always_inline function 'avx512f_intrin' requires target feature 'avx512f'}}
}
#endif // TEST3

#ifdef TEST4
// should not be promoted, has target attr.
__attribute__((target("avx")))
void has_target_attr(){
  avx512f_intrin();// expected-error{{always_inline function 'avx512f_intrin' requires target feature 'avx512f'}}
}
#endif //TEST4
