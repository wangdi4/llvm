// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

class Incomplete;

class Base {
public:
  int a;
  virtual const Incomplete *f1() { return 0; }
};

template <class T>
class HasInstantiation : public Base {
public:
  virtual Incomplete *f1() { return 0; }
};

// CHECK: [[BASE_TY:.+]] = type <{ i32 (...)**, i32, [4 x i8] }>
// CHECK: [[DER_TY:.+]] = type { [[BASE_BASE_TY:%.+]], [4 x i8] }
// CHECK: [[BASE_BASE_TY]] = type <{ i32 (...)**, i32 }>
// CHECK: [[INCOMPL_TY:.+]] = type opaque

int main() {
  Base *obj = new HasInstantiation<int>();

  // CHECK: [[OBJ:%.+]] = alloca [[BASE_TY]]*
  // CHECK: call void @_ZN16HasInstantiationIiEC1Ev([[DER_TY]]* %{{.+}})
  // CHECK: [[PTR:%.+]] = bitcast [[DER_TY]]* %{{.+}} to [[BASE_TY]]*
  // CHECK: store [[BASE_TY]]* [[PTR]], [[BASE_TY]]** [[OBJ]]
  // CHECK: %{{.+}} = load [[BASE_TY]]*, [[BASE_TY]]** [[OBJ]]
  // CHECK: %{{.+}} = bitcast [[BASE_TY]]* %{{.+}} to [[INCOMPL_TY]]* ([[BASE_TY]]*)***

  obj->f1();

  // CHECK: [[VFTABLE:%.+]] = load [[INCOMPL_TY]]* ([[BASE_TY]]*)**, [[INCOMPL_TY]]* ([[BASE_TY]]*)*** %{{.+}}
  // CHECK: [[VFN:%.+]] = getelementptr inbounds [[INCOMPL_TY]]* ([[BASE_TY]]*)*, [[INCOMPL_TY]]* ([[BASE_TY]]*)** [[VFTABLE]], i64 0
  // CHECK: [[F1:%.+]] = load [[INCOMPL_TY]]* ([[BASE_TY]]*)*, [[INCOMPL_TY]]* ([[BASE_TY]]*)** [[VFN]]
  // CHECK: {{.+}} = call [[INCOMPL_TY]]* [[F1]]([[BASE_TY]]* %{{.+}})

  return 0;
}
