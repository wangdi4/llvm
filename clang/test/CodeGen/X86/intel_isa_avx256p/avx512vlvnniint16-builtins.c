// REQUIRES: intel_feature_isa_avx256p
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %S/../intel_isa_avx512_vnni_int16/avx512vlvnniint16-builtins.c

#include "../intel_isa_avx512_vnni_int16/avx512vlvnniint16-builtins.c"
