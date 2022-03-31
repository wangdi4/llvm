// RUN: %clang_cc1 %s -verify=bmi -emit-llvm -std=c++17 -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DISAVX512=false
// RUN: %clang_cc1 %s -verify=avx512 -emit-llvm -std=c++17 -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs -DISAVX512=true

#include <intrin-promotion.h>

constexpr bool isAVX512 = ISAVX512;

template<typename T>
decltype(auto) func() { // #FUNC
  if constexpr (isAVX512)
      return avx512f_intrin(); // #AVXCALL
  else
      return bmi_intrin(); // #BMICALL
}

__attribute__((always_inline, target("avx512f")))
inline void not_in_header_avx512(){}
__attribute__((always_inline, target("bmi")))
inline void not_in_header_bmi(){}

template<typename T>
void func2() { // #FUNC2
  if constexpr (isAVX512)
      return not_in_header_avx512(); // #AVXCALL2
  else
      return not_in_header_bmi(); // #BMICALL2
}

void use() {
  func<int>();
  func2<int>();
}

// bmi-warning@#FUNC{{Function target features +bmi added due to call to an intrinsic}}
// bmi-note@#BMICALL{{Intrinsic called here}}
// bmi-error@#BMICALL2{{always_inline function 'not_in_header_bmi' requires target feature 'bmi'}}
// avx512-warning@#FUNC{{Function target features +avx512f added due to call to an intrinsic}}
// avx512-note@#AVXCALL{{Intrinsic called here}}
// avx512-error@#AVXCALL2{{always_inline function 'not_in_header_avx512' requires target feature 'avx512f'}}
