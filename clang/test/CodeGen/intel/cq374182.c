// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -std=c99 -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -std=c99 -o - %s | FileCheck %s

typedef int *ipa[2];
restrict ipa yyyyy;

// CHECK: @yyyyy = common global

