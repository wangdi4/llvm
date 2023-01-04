// REQUIRES: intel_feature_isa_avx256p
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vlbitalg-builtins.c

#include "../avx512vlbitalg-builtins.c"
