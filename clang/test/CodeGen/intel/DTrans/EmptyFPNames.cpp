// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct ContainsFPType;
using FPType = int(*)(ContainsFPType*);

struct ContainsFPType {
  FPType F;
};

int func1(ContainsFPType*) {
  return 1;
}

struct ContainsFPType2;
using FPType2 = int(*)(ContainsFPType2*);

struct ContainsFPType2 {
  FPType2 F;
};

int func2(ContainsFPType2*) {
  return 1;
}

// CHECK: %struct._ZTS14ContainsFPType.ContainsFPType = type { %"__Intel$Empty$Struct"* }
// CHECK: %struct._ZTS15ContainsFPType2.ContainsFPType2 = type { %"__Intel$Empty$Struct.0"* }
