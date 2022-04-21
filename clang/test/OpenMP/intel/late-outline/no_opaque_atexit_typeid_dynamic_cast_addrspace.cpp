// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

namespace std
{
  class type_info
  {
  public:
    virtual ~type_info();

    const char* name() const noexcept {return __name;}
    const char *__name;

    explicit type_info(const char *__n): __name(__n) { }


    type_info& operator=(const type_info&);
    type_info(const type_info&);
  };
}
extern "C" int printf(const char*,...);

struct Base {
    void foo(int) {}
    virtual ~Base() {}
};
struct Derived: virtual Base {
    void foo() {}
    virtual void name() {}
};
#pragma omp declare target
void test() {
  Derived obj;
  printf("%s\n",typeid(obj).name());
}
#pragma omp end declare target

void test1() {
  Base* b2 = new Derived;
#pragma omp target
  {
    if (Derived *d = dynamic_cast<Derived *>(b2)) {
      printf("downcast from b2 to d successful\n");
      d->name(); // safe to call
      b2->foo(1);
    }
  }
  delete b2;
}

Derived obj;
//CHECK: call spir_func i8 addrspace(4)* @__dynamic_cast(i8 addrspace(4)* {{.*}}, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI4Base to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i8 addrspace(4)*, i64 }* @_ZTI7Derived to i8*) to i8 addrspace(4)*), i64 -1)
//CHECK: call spir_func noundef i8 addrspace(4)* @_ZNKSt9type_info4nameEv(%"class.std::type_info" addrspace(4)* noundef align 8 dereferenceable_or_null(16) addrspacecast (%"class.std::type_info"* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i8 addrspace(4)*, i64 }* @_ZTI7Derived to %"class.std::type_info"*) to %"class.std::type_info" addrspace(4)*))
//CHECK: define linkonce_odr spir_func void @_ZTv0_n24_N7DerivedD0Ev(%struct.Derived addrspace(4)* noundef %this)
//CHECK-NOT: call i32 @__cxa_atexit(
// end INTEL_COLLAB
