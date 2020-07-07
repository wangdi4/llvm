// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fintel-compatibility \
// RUN:  -debug-info-kind=limited -dwarf-version=4 \
// RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

// RUN: %clang_cc1 -triple spir64 \
// RUN:  -emit-llvm -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fintel-compatibility \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -verify -Wsource-uses-openmp -o - %s \
// RUN:  -debug-info-kind=limited -dwarf-version=4 \
// RUN:  | FileCheck %s

// expected-no-diagnostics

void foo(int a) {
  const int b = 100;
  int matrix[a][b];
#pragma omp target map(alloc: matrix[a][0:b])
  {
    for (int i = 0; i < b; ++i)
      matrix[a][i] += 1;
  }
}

// Variable length arrays are not currently supported by the LLVM/SPIRV
// translator. Instead of emitting a variable bound for the array, emit an
// unbounded array subrange (count: -1).

// CHECK: [[FOO:![0-9]+]] = distinct !DISubprogram(name: "foo"
// CHECK: [[INT:![0-9]+]] = !DIBasicType(name: "int"
// CHECK: [[MATRIX:![0-9]+]] = !DILocalVariable(name: "matrix"
// CHECK-SAME: scope: [[FOO]]
// CHECK-SAME: type: [[ARRAY:![0-9]+]]
// CHECK: [[ARRAY]] = !DICompositeType(tag: DW_TAG_array_type
// CHECK-SAME: baseType: [[INT]]
// CHECK-SAME: elements: [[ELEMENTS:![0-9]+]]
// CHECK: [[ELEMENTS]] = !{[[SUBRANGE1:![0-9]+]], [[SUBRANGE2:![0-9]+]]}
// CHECK: [[SUBRANGE1]] = !DISubrange(count: -1)
// CHECK: [[SUBRANGE2]] = !DISubrange(count: 100)

