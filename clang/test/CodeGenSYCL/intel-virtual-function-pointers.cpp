// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-function-pointers -O0 -no-opaque-pointers \
// RUN: -fenable-variant-virtual-calls \
// RUN:  -triple spir64-unknown-linux %s | FileCheck %s

// CHECK: @"_ZN1C3zooEv$SIMDTable" = weak global [1 x void (%struct.C addrspace(4)*)*] [void (%struct.C addrspace(4)*)* @_ZN1C3zooEv], align 8
// CHECK: @_ZTV4Base = linkonce_odr unnamed_addr constant { [3 x i8 addrspace(4)*] } { [3 x i8 addrspace(4)*] [{{.+}} ({{.+}} ({{.+}} @_ZTI4Base to i8*) {{.+}}), {{.+}} ({{.+}}({{.+}} @"_ZN4Base1fEi$SIMDTable" to i8*) {{.+}})] }
// CHECK: @"_ZN4Base1fEi$SIMDTable" =  weak global [1 x void (%struct.Base addrspace(4)*, i32)*] [void (%struct.Base addrspace(4)*, i32)* @_ZN4Base1fEi]
template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

struct Base {
  int res = 0;
  virtual void f(int x) {    res = x;  }
};

void aoo(Base pb) {
  pb.f(10);
}

void bar(Base &pbr) {
  pbr.f(20);
}

void zoo(Base *pbp) {
  pbp->f(30);
}
struct  A {
  virtual void foo() {};
};

struct B {
  virtual void foo() {};
};

struct C {
  virtual void foo() {};
  virtual void bar() {};
  void zoo() {}
};

void test_1(C *p) {
  p->bar();
  p->zoo();
}

typedef  void (C::*PMFn)();
void test_2(C *p, void (C::*q)(void)) {
  (p->*q)();
}
int main() {
  kernel_single_task<class kernel_function>([]()
                                 __attribute__((sycl_explicit_simd)) {
                                   Base b;
                                   Base &br = b;
                                   Base *bp = &b;
                                   b.f(10000);
                                   br.f(2);
                                   bp->f(3);
                                   aoo(b);
                                   bar(br);
                                   zoo(bp);
                                   C c;
                                   C *cp = &c;
                                   test_1(cp);
                                   PMFn pf = &C::foo;
                                   test_2(cp, pf);
                                   pf = &C::zoo;
                                   test_2(cp, pf);
                                 });
}

// CHECK: define internal spir_func {{.+}}main{{.+}}
// CHECK: call spir_func void @_ZN4Base1fEi({{.+}}, i32 noundef 10000)
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 2)
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 3)
// CHECK: store { i64, i64 } { i64 ptrtoint ([1 x void (%struct.C addrspace(4)*)*]* @"_ZN1C3zooEv$SIMDTable" to i64), i64 0 }, { i64, i64 } addrspace(4)* %pf.ascast, align 8
// CHECK: ret void

// CHECK: define dso_local spir_func void{{.+}}aoo{{.+}}
// CHECK: call spir_func void @_ZN4Base1fEi({{.+}}, i32 noundef 10)
// CHECK: ret void

// CHECK: define dso_local spir_func void {{.+}}bar{{.+}}
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 20)
// CHECK: ret void

// CHECK: define dso_local spir_func void {{.+}}zoo{{.+}}
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 30)
// CHECK: ret void

// CHECK: define dso_local spir_func void {{.+}}test_1{{.+}}
// CHECK: call void @__intel_indirect_call_1(
// CHECK: ret void

// CHECK: define dso_local spir_func void {{.+}}test_2{{.+}}
// CHECK: [[L10:%.*]] = phi void (%struct.C addrspace(4)*)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
// CHECK: [[L11:%.*]] = addrspacecast void (%struct.C addrspace(4)*)* [[L10]] to void (%struct.C addrspace(4)*)* addrspace(4)*
// CHECk: call void @__intel_indirect_call_1({{.+}}[[L11]], %struct.C addrspace(4)* %this.adjusted)
// CHECK: ret void
