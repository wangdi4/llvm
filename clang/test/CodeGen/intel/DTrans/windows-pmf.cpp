// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

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

class Base {};
class VirtualWithBase : public Base {
public:
  virtual void virtual_func() const;
  void pointed_to();
};

auto returns_pmf() {
  return &VirtualWithBase::pointed_to;
}

class Virtual {
public:
  virtual void virtual_func() const;
  void pointed_to();
};
auto returns_pmf2() {
  return &Virtual::pointed_to;
}

struct B{};
struct B2{};
struct B3{};
struct B4{};
struct B5{};
struct B6{};
struct B7{};

class Has1Base : public B {};
class Has2Base : public B, public B2 {};
class Has3Base : public B, public B2, public B3 {};
class Has4Base : public B, public B2, public B3, public B4 {};
class Has5Base : public B, public B2, public B3, public B4, public B5 {};
class Has6Base : public B,
                 public B2,
                 public B3,
                 public B4,
                 public B5,
                 public B6 {};
class Has7Base : public B,
                 public B2,
                 public B3,
                 public B4,
                 public B5,
                 public B6,
                 public B7 {};

class PtrToHas1Base {
  typedef void (Has1Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas2Base {
  typedef void (Has2Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas3Base {
  typedef void (Has3Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas4Base {
  typedef void (Has4Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas5Base {
  typedef void (Has5Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas6Base {
  typedef void (Has6Base::*MPFT)();
  MPFT PMF;
};
class PtrToHas7Base {
  typedef void (Has7Base::*MPFT)();
  MPFT PMF;
};

void use() {
  PtrToHas1Base b1;
  PtrToHas2Base b2;
  PtrToHas3Base b3;
  PtrToHas4Base b4;
  PtrToHas5Base b5;
  PtrToHas6Base b6;
  PtrToHas7Base b7;
}

// If these ever change (beyond to opaque-ptr) we need to confirm that the
// dtrans info matches.
// PTR: %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" = type { { i8*, i32, i32, i32 } }
// PTR: %"struct..?AUContainsBoth@@.ContainsBoth" = type 
// PTR-SAME: { %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass", 
// PTR-SAME: %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" }
// PTR: %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" = type { i8* } 
// PTR: %"class..?AVVirtualWithBase@@.VirtualWithBase" = type { i32 (...)** }
// PTR: %"class..?AVVirtual@@.Virtual" = type { i32 (...)** } 
// PTR: %"class..?AVPtrToHas1Base@@.PtrToHas1Base" = type { i8* } 
// PTR: %"class..?AVPtrToHas2Base@@.PtrToHas2Base" = type { { i8*, i32 } } 
// PTR: %"class..?AVPtrToHas3Base@@.PtrToHas3Base" = type { { i8*, i32 } } 
// PTR: %"class..?AVPtrToHas4Base@@.PtrToHas4Base" = type { { i8*, i32 } } 
// PTR: %"class..?AVPtrToHas5Base@@.PtrToHas5Base" = type { { i8*, i32 } } 
// PTR: %"class..?AVPtrToHas6Base@@.PtrToHas6Base" = type { { i8*, i32 } } 
// PTR: %"class..?AVPtrToHas7Base@@.PtrToHas7Base" = type { { i8*, i32 } } 

// Order changes for Opaque Ptr?
// OPQ: %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" = type { ptr } 
// OPQ: %"class..?AVPtrToHas1Base@@.PtrToHas1Base" = type { ptr } 
// OPQ: %"class..?AVPtrToHas2Base@@.PtrToHas2Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVPtrToHas3Base@@.PtrToHas3Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVPtrToHas4Base@@.PtrToHas4Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVPtrToHas5Base@@.PtrToHas5Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVPtrToHas6Base@@.PtrToHas6Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVPtrToHas7Base@@.PtrToHas7Base" = type { { ptr, i32 } } 
// OPQ: %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" = type { { ptr, i32, i32, i32 } }
// OPQ: %"struct..?AUContainsBoth@@.ContainsBoth" = type 
// OPQ-SAME: { %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass", 
// OPQ-SAME: %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" }
// OPQ: %"class..?AVVirtualWithBase@@.VirtualWithBase" = type { ptr }
// OPQ: %"class..?AVVirtual@@.Virtual" = type { ptr } 

// DTrans info for the above classes:
// CHECK: !intel.dtrans.types = !{![[INCOMPLETE:[0-9]+]], ![[BOTH:[0-9]+]], 
// CHECK-SAME: ![[COMPLETE:[0-9]+]], ![[VIRT_W_BASE:[0-9]+]], ![[VIRT:[0-9]+]],
// CHECK-SAME: ![[PTR1BASE:[0-9]+]], ![[PTR2BASE:[0-9]+]], ![[PTR3BASE:[0-9]+]],
// CHECK-SAME: ![[PTR4BASE:[0-9]+]], ![[PTR5BASE:[0-9]+]], ![[PTR6BASE:[0-9]+]],
// CHECK-SAME: ![[PTR7BASE:[0-9]+]]}

// CHECK: ![[INCOMPLETE]] = !{!"S", %"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" zeroinitializer, i32 1, ![[PMF_STRUCT_PTR:[0-9]+]]}
// CHECK: ![[PMF_STRUCT_PTR]] = !{![[PMF_STRUCT:[0-9]+]], i32 0}
// HasPMFToIncompleteClass is represented as a literal struct of { i8*, i32, i32, i32 }
// CHECK: ![[PMF_STRUCT]] = !{!"L", i32 4, ![[I8PTR:[0-9]+]], ![[I32:[0-9]+]], ![[I32]], ![[I32]]}
// CHECK: ![[I8PTR]] = !{i8 0, i32 1}
// CHECK: ![[I32]] = !{i32 0, i32 0}

// ContainsBoth just has an element of the complete, and incomplete types.
// CHECK: ![[BOTH]] = !{!"S", %"struct..?AUContainsBoth@@.ContainsBoth" zeroinitializer, i32 2, ![[COMPLETE_ELEM:[0-9]+]], ![[INCOMPLETE_ELEM:[0-9]+]]}
// CHECK: ![[COMPLETE_ELEM]] = !{%"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" zeroinitializer, i32 0}
// CHECK: ![[INCOMPLETE_ELEM]] = !{%"class..?AVHasPMFToIncompleteClass@@.HasPMFToIncompleteClass" zeroinitializer, i32 0}

// HasPMFToIncompleteClass is represented as just an i8*.
// CHECK: ![[COMPLETE]] = !{!"S", %"class..?AVHasPMFToCompleteClass@@.HasPMFToCompleteClass" zeroinitializer, i32 1, ![[I8PTR]]}

// VirtualWithBase should be an i32 (...)**.
// CHECK: ![[VIRT_W_BASE]] = !{!"S", %"class..?AVVirtualWithBase@@.VirtualWithBase" zeroinitializer, i32 1, ![[FUNC_PTR:[0-9]+]]}
// CHECK: ![[FUNC_PTR]] = !{![[FUNC_TY:[0-9]+]], i32 2}
// CHECK: ![[FUNC_TY]] = !{!"F", i1 true, i32 0, ![[I32]]}

// Virtual is also the function pointer type i32 (...)**.
// CHECK: ![[VIRT]] = !{!"S", %"class..?AVVirtual@@.Virtual" zeroinitializer, i32 1, ![[FUNC_PTR]]}

// PtrToHas1Base is an i8*
// CHECK: ![[PTR1BASE]] = !{!"S", %"class..?AVPtrToHas1Base@@.PtrToHas1Base" zeroinitializer, i32 1, ![[I8PTR]]}

// The rest are all a literal of { i8*, i32 }
// CHECK: ![[PTR2BASE]] = !{!"S", %"class..?AVPtrToHas2Base@@.PtrToHas2Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL:[0-9]+]]}
// CHECK: ![[REF_BASE_LITERAL]] = !{![[BASE_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[BASE_LITERAL]] = !{!"L", i32 2, ![[I8PTR]], ![[I32]]}
// CHECK: ![[PTR3BASE]] = !{!"S", %"class..?AVPtrToHas3Base@@.PtrToHas3Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL]]}
// CHECK: ![[PTR4BASE]] = !{!"S", %"class..?AVPtrToHas4Base@@.PtrToHas4Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL]]}
// CHECK: ![[PTR5BASE]] = !{!"S", %"class..?AVPtrToHas5Base@@.PtrToHas5Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL]]}
// CHECK: ![[PTR6BASE]] = !{!"S", %"class..?AVPtrToHas6Base@@.PtrToHas6Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL]]}
// CHECK: ![[PTR7BASE]] = !{!"S", %"class..?AVPtrToHas7Base@@.PtrToHas7Base" zeroinitializer, i32 1, ![[REF_BASE_LITERAL]]}
