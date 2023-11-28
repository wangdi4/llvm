// Test verifying that RTTI information is not emitted during SYCL device compilation.

// INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -triple spir64 -fsycl-allow-virtual-functions -fsycl-is-device -emit-llvm %s -o - | FileCheck %s --implicit-check-not _ZTI4Base --implicit-check-not _ZTI8Derived1 -check-prefix VTABLE-INTEL
// RUN-INTEL: %clang_cc1 -triple spir64 -fsycl-allow-virtual-functions -fsycl-is-device -fexperimental-relative-c++-abi-vtables -emit-llvm %s -o - | FileCheck %s --implicit-check-not _ZTI4Base --implicit-check-not _ZTI8Derived1

// Intel vtable implementation are different from sycl branch,eg: we use addrspace(4). Use VTABLE-INTEL instead.

// Since experimental-relative-c++-abi-vtables is some experimental option, temporary disabling the check for now
// until we emit proper address spaces (and casts) everywhere.

// VTABLE-INTEL: @_ZTV8Derived1 = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @_ZN8Derived17displayEv to ptr addrspace(4))] }, comdat, align 8
// VTABLE-INTEL: @_ZTV4Base = linkonce_odr unnamed_addr constant { [3 x ptr addrspace(4)] } { [3 x ptr addrspace(4)] [ptr addrspace(4) null, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @_ZN4Base7displayEv to ptr addrspace(4))] }, comdat, align 8
// end INTEL_CUSTOMIZATION

// VTABLE: @_ZTV8Derived1 = linkonce_odr unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN8Derived17displayEv] }, comdat, align 8
// VTABLE: @_ZTV4Base = linkonce_odr unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN4Base7displayEv] }, comdat, align 8

SYCL_EXTERNAL bool rand();

class Base {
   public:
// INTEL_CUSTOMIZATION
    [[intel::device_indirectly_callable]] virtual void display() {}
// end INTEL_CUSTOMIZATION
};

class Derived1 : public Base {
   public:
// INTEL_CUSTOMIZATION
    [[intel::device_indirectly_callable]] void display() {}
// end INTEL_CUSTOMIZATION
};

SYCL_EXTERNAL void test() {
  Derived1 d1;
  Base *b = nullptr;
  if (rand())
    b = &d1;
  b->display();
}
