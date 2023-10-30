// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -fsycl-is-device \
// RUN: -fenable-variant-function-pointers -O0 \
// RUN: -fenable-variant-virtual-calls \
// RUN:  -triple spir64-unknown-linux %s | FileCheck %s

// CHECK: @"_ZN1C3zooEv$SIMDTable" = weak global [1 x ptr] [ptr @_ZN1C3zooEv], align 8
// CHECK: @_ZTV4Base = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [{{.+}} null, {{.+}} ({{.+}} @"_ZN4Base1fEi$SIMDTable" to ptr addrspace(4))] }
// CHECK: @"_ZN4Base1fEi$SIMDTable" =  weak global [1 x ptr] [ptr @_ZN4Base1fEi]
template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

struct Base {
  int res = 0;
  [[intel::device_indirectly_callable]] virtual void f(int x) {    res = x;  }
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
  [[intel::device_indirectly_callable]] virtual void foo() {};
};

struct B {
  [[intel::device_indirectly_callable]] virtual void foo() {};
};

struct C {
  [[intel::device_indirectly_callable]] virtual void foo() {};
  [[intel::device_indirectly_callable]] virtual void bar() {};
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
  kernel_single_task<class kernel_function>([]() {
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

// CHECK: call spir_func void @_ZN4Base1fEi({{.+}}, i32 noundef 10000)
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 2)
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 3)
// CHECK: store { i64, i64 } { i64 ptrtoint (ptr @"_ZN1C3zooEv$SIMDTable" to i64), i64 0 }, ptr addrspace(4) %pf.ascast.i, align 8
// CHECK: ret void

// CHECK: define linkonce_odr spir_func void{{.+}}aoo{{.+}}
// CHECK: call spir_func void @_ZN4Base1fEi({{.+}}, i32 noundef 10)
// CHECK: ret void

// CHECK: define linkonce_odr spir_func void {{.+}}bar{{.+}}
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 20)
// CHECK: ret void

// CHECK: define linkonce_odr spir_func void {{.+}}zoo{{.+}}
// CHECK: call void @__intel_indirect_call_0({{.+}}, i32 30)
// CHECK: ret void

// CHECK: define linkonce_odr spir_func void {{.+}}test_1{{.+}}
// CHECK: call void @__intel_indirect_call_1(
// CHECK: ret void

// CHECK: define linkonce_odr spir_func void {{.+}}test_2{{.+}}
// CHECK: [[L10:%.*]] = phi ptr [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
// CHECK: [[L11:%.*]] = addrspacecast ptr [[L10]] to ptr addrspace(4)
// CHECK: call void @__intel_indirect_call_1({{.+}}[[L11]], ptr addrspace(4) [[GEP:%.*]])
// CHECK: ret void
