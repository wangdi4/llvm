// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - \
// RUN: %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple \
// RUN: i686-windows -o - %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple \
// RUN: x86_64-windows-msvc -o - %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple \
// RUN: i686-unknown-windows-msvc -o - %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop \
// RUN: -emit-llvm -o - %s | FileCheck %s

// expected-no-diagnostics

void foo(int var) {
  int i, j, a[10], n = 10;
  i = 0;
// CHECK: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-SAME: "QUAL.PRAGMA.LEVEL"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.FACTOR"(i32 0)
// CHECK: region.exit(token [[TOKEN]])
// CHECK-SAME: [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma noblock_loop
  while (i < n) {
    a[i] += 3;
  }
}

template <typename T>
void zoo(T var)
{
// CHECK: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-SAME: "QUAL.PRAGMA.LEVEL"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.FACTOR"(i32 0)
// CHECK: region.exit(token [[TOKEN]])
// CHECK-SAME: [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma noblock_loop
  for (int i = 0; i < 10; ++i)
    var = var + i;
}

int main()
{
  zoo<int>(10);
}
