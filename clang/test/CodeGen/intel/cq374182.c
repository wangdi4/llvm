// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -std=c99 -o - | FileCheck %s --check-prefix LIN
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -std=c99 -o - %s | FileCheck %s --check-prefix WIN

typedef int *ipa[2];
restrict ipa yyyyy;

// LIN: @yyyyy = common global
// WIN: @yyyyy = common dso_local global

