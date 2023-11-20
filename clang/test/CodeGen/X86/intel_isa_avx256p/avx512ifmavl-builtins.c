// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512ifmavl-builtins.c

#include "../avx512ifmavl-builtins.c"
