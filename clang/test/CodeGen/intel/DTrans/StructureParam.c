// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s
typedef unsigned short Quantum;

typedef struct _PixelPacket {
  Quantum
      blue,
      green,
      red,
      opacity;

} PixelPacket;

int test1(PixelPacket colorize) { return 0; }

typedef float SomeFloat;

typedef struct _Floats1 {
  SomeFloat a;
} Floats1;

typedef struct _Floats2 {
  SomeFloat a, b;
} Floats2;

typedef struct _Floats3 {
  SomeFloat a, b, c;
} Floats3;

typedef struct _Floats4 {
  SomeFloat a, b, c, d;
} Floats4;

int test2(Floats1 f1, Floats2 f2, Floats3 f3, Floats4 f4) { return 0; }

typedef double SomeDouble;

typedef struct _Doubles1 {
  SomeDouble a;
} Double1;

typedef struct _Doubles2 {
  SomeDouble a, b;
} Double2;

int test3(Double1 d1, Double2 d2) { return 0; }

// NOTE: This checks to make sure that the type handling for these split/joined
// types are properly handled, but don't result in any output.  But we'll check
// the type signatures anyway.
// CHECK: !intel.dtrans.types = !{![[PIXEL:[0-9]+]], ![[FLOATS1:[0-9]+]], ![[FLOATS2:[0-9]+]], ![[FLOATS3:[0-9]+]], ![[FLOATS4:[0-9]+]], ![[DOUBLES1:[0-9]+]], ![[DOUBLES2:[0-9]+]]}

// CHECK: ![[PIXEL]] = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, ![[SHORT:[0-9]+]], ![[SHORT]], ![[SHORT]], ![[SHORT]]}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[FLOATS1]] = !{!"S", %struct._ZTS8_Floats1._Floats1 zeroinitializer, i32 1, ![[FLOAT:[0-9]+]]}
// CHECK: ![[FLOAT]] = !{float 0.0{{.+}}, i32 0}
// CHECK: ![[FLOATS2]] = !{!"S", %struct._ZTS8_Floats2._Floats2 zeroinitializer, i32 2, ![[FLOAT]], ![[FLOAT]]}
// CHECK: ![[FLOATS3]] = !{!"S", %struct._ZTS8_Floats3._Floats3 zeroinitializer, i32 3, ![[FLOAT]], ![[FLOAT]], ![[FLOAT]]}
// CHECK: ![[FLOATS4]] = !{!"S", %struct._ZTS8_Floats4._Floats4 zeroinitializer, i32 4, ![[FLOAT]], ![[FLOAT]], ![[FLOAT]], ![[FLOAT]]}
// CHECK: ![[DOUBLES1]] = !{!"S", %struct._ZTS9_Doubles1._Doubles1 zeroinitializer, i32 1, ![[DOUBLE:[0-9]+]]}
// CHECK: ![[DOUBLE]] = !{double 0.0{{.+}}, i32 0}
// CHECK: ![[DOUBLES2]] = !{!"S", %struct._ZTS9_Doubles2._Doubles2 zeroinitializer, i32 2, ![[DOUBLE]], ![[DOUBLE]]}
