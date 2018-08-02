// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s -check-prefix=CHECK-REGION
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple i686-windows -o - %s | FileCheck %s -check-prefix=CHECK-REGION
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple x86_64-windows-msvc -o - %s | FileCheck %s -check-prefix=CHECK-REGION
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple i686-unknown-windows-msvc -o - %s | FileCheck %s -check-prefix=CHECK-REGION
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop -fintel-compatibility-enable=PragmaBlockLoop -emit-llvm -o - %s | FileCheck %s -check-prefix=CHECK-REGION
// expected-no-diagnostics

void foo(int var) {
  int i, j, a[10], n = 10;
  i = 0;
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token{{.*}}DIR.PRAGMA.BLOCK_LOOP{{.*}}QUAL.PRAGMA.LEVEL{{.*}}QUAL.PRAGMA.FACTOR{{.*}}
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop
  while (i < n) {
    a[i] += 3;
  }
  i = 0;
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token{{.*}}DIR.PRAGMA.BLOCK_LOOP{{.*}}QUAL.PRAGMA.LEVEL{{.*}}QUAL.PRAGMA.FACTOR{{.*}}
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop level(1:1) factor(32)
  do {
    a[i] += 4;
  } while (i < n);
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token{{.*}}DIR.PRAGMA.BLOCK_LOOP{{.*}}QUAL.PRAGMA.LEVEL{{.*}}QUAL.PRAGMA.FACTOR{{.*}}"QUAL.PRAGMA.LEVEL"{{.*}}QUAL.PRAGMA.FACTOR{{.*}}
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop  level(1:2) factor(24)
  for (i = 0; i < n; ++i) {
    for (j = 0; i < n; ++i)
      a[i] += 5;
  }
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token{{.*}}DIR.PRAGMA.BLOCK_LOOP{{.*}}QUAL.PRAGMA.LEVEL{{.*}}QUAL.PRAGMA.FACTOR{{.*}}"QUAL.PRAGMA.LEVEL"{{.*}}QUAL.PRAGMA.FACTOR{{.*}}
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop level(5:6)
  for (auto j: a) {
    for (auto i: a)
      for (auto k: a)
        for (auto l: a)
          for (auto m: a)
            for (auto n: a)
              for (auto o: a)
                for (auto p: a)
                  j += 6;
  }
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 16)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 2)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 16)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 3)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 16)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 4)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 32)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 5)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 32)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 6)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 64)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 7)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 64)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 8)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 64)
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop level(1:3) factor(16)
  #pragma block_loop level(4:5) factor(32)
  #pragma block_loop level(6:8) factor(64)
  for (auto j: a) {
    for (auto i: a)
      for (auto k: a)
        for (auto l: a)
          for (auto m: a)
            for (auto n: a)
              for (auto o: a)
                for (auto p: a)
                  j += 6;
  }
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 3)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 -1)
// CHECK-REGION: region.exit(token [[TOKEN]]) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  #pragma block_loop level(3)
  for (auto j : a) {
    for (auto i: a)
      for (auto k: a)
        j += 6;
  }
 int var1=10;
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 %cond)
 #pragma block_loop level(1) factor(var ? 1:var1)
  for (i = 0; i < 10; ++i)
    var1 = var1 + i;

// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.PRIVATE"(i32*  %var.addr)
// CHECK-REGION-SAME: "QUAL.PRAGMA.PRIVATE"(i32*  %var1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 -1)
 #pragma block_loop level(1) private(var, var1)
  for (i = 0; i < 10; ++i)
    var1 = var1 + i;
}

template <typename T>
void zoo(T var)
{
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.PRIVATE"(i32*  %var.addr)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 -1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 16)
  #pragma block_loop private(var) factor(16)
  for (int i = 0; i < 10; ++i)
    var = var + i;
}

template <int size>
void nontypeargument(int var)
{
  int i;
// CHECK-REGION: [[TOKEN:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(),
// CHECK-REGION-SAME: "QUAL.PRAGMA.PRIVATE"(i32*  %var.addr)
// CHECK-REGION-SAME: "QUAL.PRAGMA.LEVEL"(i32 -1)
// CHECK-REGION-SAME: "QUAL.PRAGMA.FACTOR"(i32 100)
  #pragma block_loop private(var) factor(size)
  for (i = 0; i < 10; ++i)
    var = var + i;
}
int bar()
{
  zoo<int>(10);
  nontypeargument<100>(10);
}
