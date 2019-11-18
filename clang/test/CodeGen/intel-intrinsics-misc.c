// RUN: %clang_cc1 -ffreestanding -triple i686-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=17.00 -triple i686-windows-msvc -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=17.00 -triple x86_64-windows-msvc -emit-llvm %s -o - | FileCheck %s

#include <immintrin.h>

void test_clac() {
// CHECK-LABEL: test_clac(
// CHECK: call void asm sideeffect "clac", "~{memory},~{dirflag},~{fpsr},~{flags}"()
  _clac();
}

void test_stac() {
// CHECK-LABEL: test_stac(
// CHECK: call void asm sideeffect "stac", "~{memory},~{dirflag},~{fpsr},~{flags}"()
  _stac();
}

void test_lgdt(void *ptr) {
// CHECK-LABEL: test_lgdt(
// CHECK: call void asm sideeffect "lgdt $0", "*m,~{memory},~{dirflag},~{fpsr},~{flags}"
  _lgdt(ptr);
}

void test_sgdt(void *ptr) {
// CHECK-LABEL: test_sgdt(
// CHECK: call void asm sideeffect "sgdt $0", "=*m,~{memory},~{dirflag},~{fpsr},~{flags}"
  _sgdt(ptr);
}
