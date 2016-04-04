// RUN: %clang_cc1 -triple x86_64-unknown-linux -fintel-compatibility %s -emit-llvm -verify -o - | FileCheck %s
// expected-no-diagnostics

__declspec(thread) int i;
// CHECK: thread_local
