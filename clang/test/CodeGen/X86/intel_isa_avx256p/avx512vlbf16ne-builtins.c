// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -ffreestanding -flax-vector-conversions=none %s -triple=x86_64-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../intel_isa_avx512_bf16_ne/avx512vlbf16ne-builtins.c

#include "../intel_isa_avx512_bf16_ne/avx512vlbf16ne-builtins.c"
