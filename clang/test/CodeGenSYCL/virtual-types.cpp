// if INTEL_COLLAB
// We don't emit type info in xmain
// RUN: %clang_cc1 -fsycl-is-device -triple spir64-unknown-linux -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s --implicit-check-not @_ZTI6Struct
// end INTEL_COLLAB
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
<<<<<<< HEAD
// if INTEL_COLLAB
// CHECK: @_ZTV6Struct = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @_ZN6Struct3fooEv to ptr addrspace(4))] }, comdat, align 8
// CHECK-disabled: @[[TYPEINFO:.+]] = external addrspace(1) global ptr addrspace(4)
=======
// CHECK: @_ZTV6Struct = linkonce_odr unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI6Struct, ptr @_ZN6Struct3fooEv] }, comdat, align 8
// CHECK: @[[TYPEINFO:.+]] = external addrspace(1) global [0 x ptr addrspace(4)]
>>>>>>> 0989c8a4884e8627310b63dde31572ff9af58d6b
// TypeInfo Name:
// CHECK-disabled: @_ZTS6Struct = linkonce_odr constant [8 x i8] c"6Struct\00", comdat, align 1
// TypeInfo:
// CHECK-disabled: @_ZTI6Struct = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (ptr a ddrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i64 2), ptr addrspace(4) addrspacecast (ptr @_ZTS6Struct to ptr addrspace(4)) }, comdat, align 8
// end INTEL_COLLAB
