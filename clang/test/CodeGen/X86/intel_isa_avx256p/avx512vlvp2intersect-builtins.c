// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vlvp2intersect-builtins.c
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=i386-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vlvp2intersect-builtins.c

#include "../avx512vlvp2intersect-builtins.c"
