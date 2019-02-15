// RUN: %clang_cc1 -o - %s -DT1 -emit-llvm -debug-info-kind=limited \
// RUN:            -triple x86_64-linux-gnu | FileCheck --check-prefix=T1 %s
// RUN: %clang_cc1 -o - %s -DT2 -emit-llvm -debug-info-kind=limited \
// RUN:            -triple x86_64-linux-gnu | FileCheck --check-prefix=T2 %s
//
// Test T1
// None of the classes are defined externally and all of them need definitions.
//
// Test T2
// Class "D" contains a key function which is defined externally, so a
// declaration is emitted for D and C is omitted.  B is a vtable holder for E
// and gets a declaration, and A is omitted.
//

class A {};

class B : public A {
public:
  virtual int FB() = 0; // pure virtual method
};

class C : public B {
};

class D : public C {
public:
  virtual int FD() = 0;
#ifdef T2
  virtual int KD(); // externally defined key 
#endif
};

class E : public D {
  virtual int FB() { return 1; }
  virtual int FD() { return 2; }
};

E e;

int main() {
  B *b = &e;
  b->FB();
  return 0;
}

// T1:      !DICompositeType(tag: DW_TAG_class_type, name: "E"
// T1-NOT:  DIFlagFwdDecl
// T1-SAME: elements: !{{[0-9]+}}
// T1-SAME: ){{$}}
// T1:      !DICompositeType(tag: DW_TAG_class_type, name: "D"
// T1-NOT      DIFlagFwdDecl
// T1-SAME:    elements: !{{[0-9]+}}
// T1-SAME: ){{$}}
// T1:      !DICompositeType(tag: DW_TAG_class_type, name: "C"
// T1-NOT   DIFlagFwdDecl
// T1-SAME: elements: !{{[0-9]+}}
// T1-SAME: ){{$}}
// T1:      !DICompositeType(tag: DW_TAG_class_type, name: "B"
// T1-NOT   DIFlagFwdDecl
// T1-SAME: elements: !{{[0-9]+}}
// T1-SAME: ){{$}}
// T1:      !DICompositeType(tag: DW_TAG_class_type, name: "A"
// T1-NOT   DIFlagFwdDecl
// T1-SAME: elements: !{{[0-9]+}}
// T1-SAME: ){{$}}


// T2:      !DICompositeType(tag: DW_TAG_class_type, name: "E"
// T2-NOT:  DIFlagFwdDecl
// T2-SAME: elements: !{{[0-9]+}}
// T2-SAME: ){{$}}
// T2:      !DICompositeType(tag: DW_TAG_class_type, name: "D"
// T2-SAME: DIFlagFwdDecl
// T2-NOT:  elements: !{{[0-9]+}}
// T2-SAME: ){{$}}
// T2-NOT:  !DICompositeType(tag: DW_TAG_class_type, name: "C"
// T2:      !DICompositeType(tag: DW_TAG_class_type, name: "B"
// T2-SAME: DIFlagFwdDecl
// T2-NOT:  elements: !{{[0-9]+}}
// T2-SAME: ){{$}}
// T1-NOT:  !DICompositeType(tag: DW_TAG_class_type, name: "A"

