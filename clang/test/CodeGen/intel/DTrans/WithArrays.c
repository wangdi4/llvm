// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s

typedef struct test_array_member {
  char inputfile[200];
  char clustfile[200];
  long n;
  long n_trips[50];
} test_array_member_t;

int main() {
  struct test_array_member instance;

  instance.inputfile[0] = '\0';
  instance.clustfile[0] = '\0';
  instance.n = 0;
  instance.n_trips[0] = 0;

  return 0;
}

// CHECK: !intel.dtrans.types = !{![[TAM:[0-9]+]]}
// CHECK: [[TAM]] = !{!"S",  %struct._ZTS17test_array_member.test_array_member zeroinitializer, i32 4, ![[CHAR_200_ARRAY:[0-9]+]], ![[CHAR_200_ARRAY]], ![[LONG:[0-9]+]], ![[LONG_50_ARRAY:[0-9]+]]}
// CHECK: ![[CHAR_200_ARRAY]]  = !{!"A", i32 200, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[LONG]] = !{i64 0, i32 0}
// CHECK: ![[LONG_50_ARRAY]] = !{!"A", i32 50, ![[LONG]]}
