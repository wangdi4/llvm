// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin \
// RUN:            -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vlbf16-builtins.c

#include "../avx512vlbf16-builtins.c"

