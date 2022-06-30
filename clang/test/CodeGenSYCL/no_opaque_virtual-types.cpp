// RUN: %clang_cc1 -fsycl-is-device -triple spir64-unknown-linux -disable-llvm-passes -no-opaque-pointers -emit-llvm %s -o - | FileCheck %s
template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(const Func &kernelFunc) {
  kernelFunc();
}

struct Struct {
  virtual void foo() {}
  void bar() {}
};

int main() {
  kernel_single_task<class kernel_function>([]() {
                                            Struct S;
                                            S.bar(); });
  return 0;
}


// Struct layout big enough for vtable.
// CHECK: %struct.Struct = type { i32 (...)* addrspace(4)* }
// VTable:
// CHECK: @_ZTV6Struct = linkonce_odr unnamed_addr constant { [3 x i8 addrspace(4)*] } { [3 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI6Struct to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.Struct addrspace(4)*)* @_ZN6Struct3fooEv to i8*) to i8 addrspace(4)*)] }, comdat, align 8
// CHECK: @[[TYPEINFO:.+]] = external addrspace(1) global i8 addrspace(4)*
// TypeInfo Name:
// CHECK: @_ZTS6Struct = linkonce_odr constant [8 x i8] c"6Struct\00", comdat, align 1
// TypeInfo:
// CHECK @_ZTI6Struct = linkonce_odr constant { i8 addrspace(4)*, i8 addrspace(4)* } { i8 addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds (i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspacecast (i8 addrspace(4)* addrspace(1)* @_ZTVN10__cxxabiv117__class_type_infoE to i8 addrspace(4)* addrspace(4)*), i64 2) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZTS6Struct, i32 0, i32 0) to i8 addrspace(4)*) }, comdat, align 8
