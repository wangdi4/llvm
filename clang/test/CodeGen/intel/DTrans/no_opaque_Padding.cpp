// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s

class FNCode {
  int *data;
  int x;
  int y;
  int z;
};

class FNCode2 {
  int *data;
  int x;
  int y;
  int z;
  short a;
  char b;
};

int main() {
  FNCode code;
  FNCode2 code2;
  return 0;
}

// CHECK: intel.dtrans.types = !{![[FNCODE:[0-9]+]], ![[FNCODE2:[0-9]+]]}
// CHECK: ![[FNCODE]] = !{!"S", %class._ZTS6FNCode.FNCode zeroinitializer, i32 5, ![[INT_PTR:[0-9]+]], ![[INT:[0-9]+]], ![[INT]], ![[INT]], ![[ARRAY_PAD:[0-9]+]]}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[ARRAY_PAD]] = !{!"A", i32 4, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[FNCODE2]] = !{!"S", %class._ZTS7FNCode2.FNCode2 zeroinitializer, i32 7, ![[INT_PTR]], ![[INT]], ![[INT]], ![[INT]], ![[SHORT:[0-9]+]], ![[CHAR]], ![[CHAR]]}
// CHECK: ![[SHORT]] = !{i16 0, i32 0}
