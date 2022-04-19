// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

struct has_padding {
  unsigned int a;
  char c;
  // Padding here.
  unsigned long long bf : 16;
  // Padding here as well.
  int i;
  // Also padding here.
};

struct has_padding2 {
  unsigned int a;
  char c;
  // Padding here.
  unsigned long long bf : 16;
  // Padding here as well.
};

struct has_padding3 {
  unsigned int a;
  char c;
  // Padding here.
  unsigned long long bf : 16;
  // Padding here as well.
  char not_padding[6];
  int i;
  // And more padding.
};

static const struct has_padding array[] =
    {{}};
// CHECK: @array = internal constant [1 x { i32, i8, [3 x i8], i8, i8, [6 x i8], i32, [4 x i8] }]{{.+}} !intel_dtrans_type ![[HP1_ARR:[0-9]+]]
static const struct has_padding2 array2[] =
    {{}};
// CHECK: @array2 = internal constant [1 x { i32, i8, [3 x i8], i8, i8, [6 x i8] }]{{.+}} !intel_dtrans_type ![[HP2_ARR:[0-9]+]]
static const struct has_padding3 array3[] =
    {{}};
// CHECK: @array3 = internal constant [1 x { i32, i8, [3 x i8], i8, i8, [6 x i8], [6 x i8], i32, [4 x i8] }]{{.+}} !intel_dtrans_type ![[HP3_ARR:[0-9]+]]

void EmitArray() {
  (void)array;
  (void)array2;
  (void)array3;
}

// CHECK: ![[HP1_ARR]] = !{!"A", i32 1, ![[HP1_REF:[0-9]+]]}
// CHECK: ![[HP1_REF]] = !{![[HP1:[0-9]+]], i32 0}
// Fields:                          'a'              'c'             Padding           'bf[0]'  'bf[1]'  Padding           'i'       'tail padding'
// CHECK: ![[HP1]] = !{!"L", i32 8, ![[I32:[0-9]+]], ![[I8:[0-9]+]], ![[I8_3:[0-9]+]], ![[I8]], ![[I8]], ![[I8_6:[0-9]+]], ![[I32]], ![[I8_4:[0-9]+]]}
// CHECK: ![[HP2_ARR]] = !{!"A", i32 1, ![[HP2_REF:[0-9]+]]}
// CHECK: ![[HP2_REF]] = !{![[HP2:[0-9]+]], i32 0}
// Fields:                          'a'       'c'      Padding    'bf[0]'  'bf[1]'  Padding
// CHECK: ![[HP2]] = !{!"L", i32 6, ![[I32]], ![[I8]], ![[I8_3]], ![[I8]], ![[I8]], ![[I8_6]]}
// CHECK: ![[HP3_ARR]] = !{!"A", i32 1, ![[HP3_REF:[0-9]+]]}
// CHECK: ![[HP3_REF]] = !{![[HP3:[0-9]+]], i32 0}
// Fields:                          'a'       'c'      Padding    'bf[0]'  'bf[1]'  Padding   not_padd   i     tail padding
// CHECK: ![[HP3]] = !{!"L", i32 9, ![[I32]], ![[I8]], ![[I8_3]], ![[I8]], ![[I8]], ![[I8_6]], ![[I8_6]], ![[I32]], ![[I8_4]]}
