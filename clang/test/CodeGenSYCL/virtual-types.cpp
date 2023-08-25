<<<<<<< HEAD
// RUN: %clang_cc1 -fsycl-is-device -triple spir64-unknown-linux -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s
=======
// if INTEL_COLLAB
// We don't emit type info in xmain
// RUN: %clang_cc1 -fsycl-is-device -triple spir64-unknown-linux -disable-llvm-passes -opaque-pointers -emit-llvm %s -o - | FileCheck %s --implicit-check-not @_ZTI6Struct
// end INTEL_COLLAB
>>>>>>> dbfad6389a96f70ba2c7070375a883a87de3f595
template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(const Func &kernelFunc) {
  kernelFunc();
}

struct Struct {
// if INTEL_COLLAB
  [[intel::device_indirectly_callable]] virtual void foo() {}
// end INTEL_COLLAB
  void bar() {}
};

int main() {
  kernel_single_task<class kernel_function>([]() {
                                            Struct S;
                                            S.bar(); });
  return 0;
}


// Struct layout big enough for vtable.
// CHECK: %struct.Struct = type { ptr addrspace(4) }
// VTable:
// if INTEL_COLLAB
// CHECK: @_ZTV6Struct = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @_ZN6Struct3fooEv to ptr addrspace(4))] }, comdat, align 8
// CHECK-disabled: @[[TYPEINFO:.+]] = external addrspace(1) global ptr addrspace(4)
// TypeInfo Name:
// CHECK-disabled: @_ZTS6Struct = linkonce_odr constant [8 x i8] c"6Struct\00", comdat, align 1
// TypeInfo:
// CHECK-disabled: @_ZTI6Struct = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (ptr a ddrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i64 2), ptr addrspace(4) addrspacecast (ptr @_ZTS6Struct to ptr addrspace(4)) }, comdat, align 8
// end INTEL_COLLAB
