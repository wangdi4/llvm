// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: vla_test
int vla_test() {
  auto array_size = 100;
  int *C[array_size];
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* %array_size, align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = zext i32 [[L1]]
  // CHECK: [[VLA_ADDR:%.+]] = alloca i32*,
  // CHECK: [[SIZE:%[0-9]+]] = mul nuw i64 %1, 8
  // CHECK-NEXT: [[ARRIDX:%.+]] = getelementptr inbounds i32*, i32** [[VLA_ADDR]], i64 0
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(i32** [[VLA_ADDR]], i32** [[ARRIDX]], i64 [[SIZE]])
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(from: C)
  for (int i = 0; i < array_size; ++i)
    C[i][1] = 2;
  return 0;
}

// CHECK-LABEL: vla_test1
int vla_test1() {
  auto array_size = 100;
  auto aaa = 10;
  int C[array_size][aaa];
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* %array_size, align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = zext i32 [[L1]]
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* %aaa, align 4
  // CHECK-NEXT: [[L4:%[0-9]+]] = zext i32 [[L3]]
  // CHECK: [[VLA_ADDR:%.+]] = alloca i32,
  // CHECK: [[L6:%[0-9]+]] = mul nuw i64 [[L2]], [[L4]]
  // CHECK-NEXT: [[SIZE:%[0-9]+]] = mul nuw i64 [[L6]], 4
  // CHECK-NEXT: [[ARRIDX:%.+]] = getelementptr inbounds i32, i32* [[VLA_ADDR]], i64 0
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(i32* [[VLA_ADDR]], i32* [[ARRIDX]], i64 [[SIZE]])
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(from: C)
  for (int i = 0; i < array_size; ++i)
    C[i][1] = 2;
  return 0;
}

// CHECK-LABEL: vla_test2
int vla_test2() {
  auto array_size = 100;
  auto aaa = 10;
  int C[array_size][aaa];
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* %array_size, align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = zext i32 [[L1]]
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* %aaa, align 4
  // CHECK-NEXT: [[L4:%[0-9]+]] = zext i32 [[L3]]
  // CHECK: [[VLA_ADDR:%.+]] = alloca i32,
  // CHECK: [[L5:%[0-9]+]] = mul nsw i64 0, [[L4]]
  // CHECK: [[ARRIDX:%.+]] = getelementptr inbounds i32, i32* %vla, i64 %6
  // CHECK: [[L6:%[0-9]+]] = mul nuw i64 [[L2]], [[L4]]
  // CHECK-NEXT: [[SIZE:%[0-9]+]] = mul nuw i64 [[L6]], 4
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(i32* [[VLA_ADDR]], i32* [[ARRIDX]], i64 [[SIZE]])
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(from: C[:])
  for (int i = 0; i < array_size; ++i)
    C[i][1] = 2;
  return 0;
}

// CHECK-LABEL: vla_test3
int vla_test3() {
  auto array_size = 100;
  auto aaa = 10;
  int C[array_size][aaa];
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* %aaa, align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = zext i32 [[L1]]
  // CHECK: [[VLA_ADDR:%.+]] = alloca i32,
  // CHECK: [[SIZE:%[0-9]+]] = mul nuw i64 [[L2]], 4
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(i32* [[VLA_ADDR]], i32* %arrayidx2, i64 [[SIZE]])
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(from: C[1])
  for (int i = 0; i < array_size; ++i)
    C[i][1] = 2;
  return 0;
}

// CHECK-LABEL: vla_test4
int vla_test4() {
  auto array_size = 100;
  auto aaa = 10;
  int C[array_size][aaa];
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(i32* %vla, i32* %arrayidx2, i64 4)
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(from: C[1][1])
  for (int i = 0; i < array_size; ++i)
    C[i][1] = 2;
  return 0;
}
// end INTEL_COLLAB
