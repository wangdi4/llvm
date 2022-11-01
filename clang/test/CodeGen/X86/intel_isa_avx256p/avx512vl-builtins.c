// REQUIRES: intel_feature_isa_avx256p
// INTEL_FEATURE_ISA_DSPV1
// XFAIL: intel_feature_isa_dspv1
// end INTEL_FEATURE_ISA_DSPV1
// RUN: %clang_cc1 -no-opaque-pointers -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %S/../avx512vl-builtins.c

#include "../avx512vl-builtins.c"

// TODO: Don't forget scalar intrinsics.
