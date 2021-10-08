// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

class HasPMFToCompleteClass {
public:
  typedef void (HasPMFToCompleteClass::*MFPT)();
  MFPT fToCall;
};

class Incomplete;
class HasPMFToIncompleteClass {
public:
  typedef void (Incomplete::*MFPT)();
  MFPT fToCall;
};

struct ContainsBoth {
  HasPMFToCompleteClass a;
  HasPMFToIncompleteClass b;
};

void foo(HasPMFToCompleteClass a, HasPMFToIncompleteClass b, ContainsBoth c) {
}

// CHECK: !intel.dtrans.types = !{![[INCOMPLETE:[0-9]+]], ![[BOTH:[0-9]+]], ![[COMPLETE:[0-9]+]]}
// CHECK: ![[INCOMPLETE]] = !{!"S", %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" zeroinitializer, i32 1, ![[PMF_STRUCT_PTR:[0-9]+]]}
// CHECK: ![[PMF_STRUCT_PTR]] = !{![[PMF_STRUCT:[0-9]+]], i32 0}
// CHECK: ![[PMF_STRUCT]] = !{!"L", i32 4, ![[I8PTR:[0-9]+]], ![[I32:[0-9]+]], ![[I32]], ![[I32]]}
// CHECK: ![[I8PTR]] = !{i8 0, i32 1}
// CHECK: ![[I32]] = !{i32 0, i32 0}
// CHECK: ![[BOTH]] = !{!"S", %"struct..?AUContainsBoth@@.ContainsBoth" zeroinitializer, i32 2, ![[COMPLETE_ELEM:[0-9]+]], ![[INCOMPLETE_ELEM:[0-9]+]]}
// CHECK: ![[COMPLETE_ELEM]] = !{%"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" zeroinitializer, i32 0}
// CHECK: ![[INCOMPLETE_ELEM]] = !{%"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" zeroinitializer, i32 0}
// CHECK: ![[COMPLETE]] = !{!"S", %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" zeroinitializer, i32 1, ![[I8PTR]]}
