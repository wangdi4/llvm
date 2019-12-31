// RUN: %clang_cc1 -ffreestanding -triple i686-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=17.00 -triple i686-windows-msvc -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=17.00 -triple x86_64-windows-msvc -emit-llvm %s -o - | FileCheck %s

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif

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

unsigned long long test__readmsr(unsigned int x) {
// CHECK-LABEL: test__readmsr(
// CHECK: call { i32, i32 } asm "rdmsr", "={dx},={ax},{cx},~{dirflag},~{fpsr},~{flags}"(i32 %{{.*}})
  return __readmsr(x);
}

void test__writemsr(unsigned int x, unsigned long long y) {
// CHECK-LABEL: test__writemsr(
// CHECK: call void asm sideeffect "wrmsr", "{dx},{ax},{cx},~{dirflag},~{fpsr},~{flags}"(i32 %{{.*}}, i32 %{{.*}}, i32 {{.*}})
  __writemsr(x, y);
}

unsigned long long test_readmsr(unsigned int x) {
// CHECK-LABEL: test_readmsr(
// CHECK: call { i32, i32 } asm "rdmsr", "={dx},={ax},{cx},~{dirflag},~{fpsr},~{flags}"(i32 %{{.*}})
  return _readmsr(x);
}

void test_writemsr(unsigned int x, unsigned long long y) {
// CHECK-LABEL: test_writemsr(
// CHECK: call void asm sideeffect "wrmsr", "{dx},{ax},{cx},~{dirflag},~{fpsr},~{flags}"(i32 %{{.*}}, i32 %{{.*}}, i32 {{.*}})
  _writemsr(x, y);
}
