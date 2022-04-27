// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -opaque-pointers -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t

// RUN: %clang_cc1 -opaque-pointers -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
// RUN:   | FileCheck %s

// Verify subdevice clause accepted with target directives

#ifndef HEADER
#define HEADER

template<typename tx, typename ty>
struct TT{
  tx X;
  ty Y;
};

int global;
extern int global;

enum EnumVals {ONE=1, TWO, THREE};

extern int func(int i);

class S {
  int bar;
  S();
public:
  S(int foo) : bar(foo) {}
};

int test(int n) {
  int level, start, length, stride;
  int a = 0;
  short aa = 0;
  char c = 'a';
  TT<long, char> d;
  TT<int *, int *> d2;
  static long *plocal;
  S e(5);
  int vec[10];

  // subdevice(global:local)
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%a[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(global:a)
  {}

  // subdevice(<short>:<char>)
  //
  // CHECK: [[LD:%[0-9]+]] = load i16, ptr %aa
  // CHECK-NEXT: store i16 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i8, ptr %c
  // CHECK-NEXT: store i8 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i16, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i8, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i16 [[LD1]], i8 [[LD2]], i32 1) ]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(aa:c)
  {}

  // subdevice(<struct field>) - only one argument
  //
  // CHECK: %Y = getelementptr inbounds %struct.TT, ptr %d, i32 0, i32 1
  // CHECK: [[LD:%[0-9]+]] = load i8, ptr %Y
  // CHECK-NEXT: store i8 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i8, ptr [[CE1]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i8 [[LD]], i32 1, i32 1) ]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(d.Y)
  {}

  // subdevice(<enum>) - only one argument
  //
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 1, i32 1, i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(ONE)
  {}

  // subdevice(<function call>:maxint)
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %a
  // CHECK-NEXT: [[CALL:%call[0-9]*]] = call noundef i32 @_Z4funci(i32 noundef [[LD]])
  // CHECK-NEXT: store i32 [[CALL]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD]], i32 2147483647, i32 1) ]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(func(a):2147483647)
  {}

  // subdevice(<ptr to int>:<ptr to int>:<variable>)
  //
  // CHECK: %X = getelementptr inbounds %struct.TT.0, ptr %d2, i32 0, i32 0
  // CHECK-NEXT: [[LD:%[0-9]+]] = load ptr, ptr %X
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[LD]]
  // CHECK-NEXT: store i32 [[LD1]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[Y:%Y[0-9]*]] = getelementptr inbounds %struct.TT.0, ptr %d2, i32 0, i32 1
  // CHECK: [[LD:%[0-9]+]] = load ptr, ptr [[Y]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[LD]]
  // CHECK-NEXT: store i32 [[LD1]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %stride
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE3:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32, ptr [[CE3]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 [[LD3]]) ]
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(*(d2.X):*(d2.Y):stride)
  {}

  // subdevice with other target directives
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: [[IDX:%idxprom[0-9]*]] = sext i32 [[LD]] to i64
  // CHECK-NEXT: [[AINDEX:%arrayidx[0-9]*]] = getelementptr inbounds [10 x i32], ptr %vec, i64 0, i64 [[IDX]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, ptr [[AINDEX]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET.ENTER.DATA"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 10, i32 20, i32 [[LD]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET.ENTER.DATA"()
  #pragma omp target enter data device(a) map(alloc: d.X) \
                subdevice(10:20:vec[start])

  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET.EXIT.DATA"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 30, i32 40, i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET.EXIT.DATA"()
  #pragma omp target exit data device(a) map(release: d.X) subdevice(30:40)

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK: [[LD1:%[0-9]+]] = load i32, ptr %length
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET.DATA"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD]], i32 [[LD1]], i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET.DATA"()
  #pragma omp target data use_device_addr(a) subdevice(start:length)
  {}

  // subdevice(<int param>:<int global>)
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %n.addr
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, ptr %n.addr
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE3:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32, ptr [[CE3]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET.UPDATE"()
  // CHECK-SAME: "QUAL.OMP.DEVICE"(i32 [[LD1]])
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD2]], i32 [[LD3]], i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET.UPDATE"()
  #pragma omp target update device(n) subdevice(n:global) from(a)

  // subdevice(<int param>:<int global>)
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target simd subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, ptr %a
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.PARALLEL"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target parallel subdevice(global:a)
  {}

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %a
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target parallel for subdevice(global:a)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %a
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target parallel for simd subdevice(global:a)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr @global
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %a
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.PARALLEL"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.GENERICLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.GENERICLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target parallel loop subdevice(global:a)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.DISTRIBUTE"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.DISTRIBUTE"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute simd subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.GENERICLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.GENERICLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams loop subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE.PARLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute parallel for subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %length
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 [[LD2]], i32 1)
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TEAMS"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK: region.entry{{.*}} [ "DIR.OMP.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE.PARLOOP"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TEAMS"()
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target teams distribute parallel for simd subdevice(start:length)
  for(int i=0; i < 10; ++i)
    ++vec[i];

  // subdevice - verify uses of optional level parameter
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%start[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 1, i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(0,start)
  {}

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%start[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%length[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE3:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32, ptr [[CE3]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 1, i32 [[LD2]], i32 [[LD3]], i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(1,start:length)
  {}
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%start[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE2:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%length[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE3:%.capture_expr.[0-9]*]]
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr [[A:%stride[0-9]*]]
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE4:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32, ptr [[CE2]]
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32, ptr [[CE3]]
  // CHECK-NEXT: [[LD4:%[0-9]+]] = load i32, ptr [[CE4]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 1, i32 [[LD2]], i32 [[LD3]], i32 [[LD4]])
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(1,start:length:stride)
  {}

  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 1, i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(0,start)
  {}

  // Verify out of range value for level is set to zero.
  //
  // CHECK: [[LD:%[0-9]+]] = load i32, ptr %start
  // CHECK-NEXT: store i32 [[LD]], ptr [[CE1:%.capture_expr.[0-9]*]]
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32, ptr [[CE1]]
  // CHECK: region.entry{{.*}} [ "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.SUBDEVICE"(i32 0, i32 [[LD1]], i32 1, i32 1)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.TARGET"()
  #pragma omp target subdevice(2147483647,start)
  {}
  return 0;
}
#endif // HEADER
// end INTEL_COLLAB
