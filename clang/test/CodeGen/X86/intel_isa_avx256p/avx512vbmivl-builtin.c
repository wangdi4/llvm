// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -target-feature +avx512bw -emit-llvm -o - -Wall -Werror | FileCheck %S/../avx512vbmivl-builtin.c


#include "../avx512vbmivl-builtin.c"
