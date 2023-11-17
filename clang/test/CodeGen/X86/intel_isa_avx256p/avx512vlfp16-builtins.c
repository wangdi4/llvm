// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vlfp16-builtins.c

#include "../avx512vlfp16-builtins.c"