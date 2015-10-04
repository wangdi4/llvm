// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -O0 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

extern int i1;
static int i1;

extern int i2 = 1;
static int i2;

int i3;
static int i3;

int i4;
static int i4 = 1;

static int i5;
int i5;

int i6;
static int i6;
extern int i6;
static int i6;

static int i7 = 1;
int i7;
extern int i7;
static int i7;

static int i8;
extern int i8 = 1;
static int i8;

int f1();
static int f1();
int f1() { return 1; }

static int f2();
int f2() { return 2; }

int f3();
static int f3() { return 3; }

extern int f4();

int main(void) {
  static int f4();
  static int f5();
  return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 +
         f1() + f2() + f3() + f4() + f5();
}

int f4() { return 4; }
int f5() { return 5; }

// CHECK-DAG: @i1 = internal global i32 0
// CHECK-DAG: @i2 = global i32 1
// CHECK-DAG: @i3 = common global i32 0
// CHECK-DAG: @i4 = global i32 1
// CHECK-DAG: @i5 = common global i32 0
// CHECK-DAG: @i6 = common global i32 0
// CHECK-DAG: @i7 = global i32 1
// CHECK-DAG: @i8 = internal global i32 1
//
// CHECK-DAG: define internal i32 @f1()
// CHECK-DAG: define internal i32 @f2()
// CHECK-DAG: define internal i32 @f3()
// CHECK-DAG: define internal i32 @f4()
// CHECK-DAG: define internal i32 @f5()
