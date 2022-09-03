// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -emit-llvm -opaque-pointers %s -o - | FileCheck %s

// In some situations, a bitfield is added to the layout object as an array of
// ints, so make sure that we handle this case correctly, rather than asserting
// about an unknown array type.

struct NormalBitFields {
  unsigned short a;
  unsigned int b : 16;
  unsigned int c : 16;
  unsigned int d : 8;
};

enum Old{};
struct OldEnumBitFields {
  unsigned short a;
  Old b : 16;
  Old c : 16;
  Old d : 8;
};
enum class New;
struct NewEnumBitFields {
  unsigned short a;
  New b : 16;
  New c : 16;
  New d : 8;
};
struct BoolBitFields {
  unsigned short a;
  bool b : 16;
  bool c : 16;
  bool d : 8;
};

NormalBitFields N;
OldEnumBitFields E1;
NewEnumBitFields E2;
BoolBitFields B;

// CHECK: !intel.dtrans.types = !{![[NORMAL:[0-9]+]], ![[OLD:[0-9]+]], ![[NEW:[0-9]+]], ![[BOOL:[0-9]+]]}
// CHECK: ![[NORMAL]] = !{!"S", %struct._ZTS15NormalBitFields.NormalBitFields zeroinitializer, i32 2, ![[SHORT:[0-9]+]], ![[CHAR5ARR:[0-9]+]]}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
// CHECK: ![[CHAR5ARR]] = !{!"A", i32 5, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[OLD]] = !{!"S", %struct._ZTS16OldEnumBitFields.OldEnumBitFields zeroinitializer, i32 2, ![[SHORT]], ![[CHAR5ARR]]}
// CHECK: ![[NEW]] = !{!"S", %struct._ZTS16NewEnumBitFields.NewEnumBitFields zeroinitializer, i32 2, ![[SHORT]], ![[CHAR5ARR]]}
// CHECK: ![[BOOL]] = !{!"S", %struct._ZTS13BoolBitFields.BoolBitFields zeroinitializer, i32 2, ![[SHORT]], ![[CHAR5ARR]]}
