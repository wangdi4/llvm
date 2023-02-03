// This test checks that the FE generates global variables corresponding to the
// virtual table in the global address space (addrspace(1)) when
// -fsycl-allow-virtual-functions is passed.

// RUN: %clang_cc1 -triple spir64 -fsycl-allow-virtual-functions -fsycl-is-device -emit-llvm %s -o - | FileCheck %s  --check-prefixes CHECK,CHECK-PTR
#if INTEL_CUSTOMIZATION
// FIXME: there are issues with relative vtables layout in xmain: we emit more
// pointers in generic address space, but then we can't use @llvm.load.relative
// intrinsic, because it only accepts pointers in default (0) address space.
//
// Since this is some experimental option, temporary disabling the check for now
// until we emit proper address spaces (and casts) everywhere.
// RUNx: %clang_cc1 -triple spir64 -fsycl-allow-virtual-functions -fsycl-is-device -fexperimental-relative-c++-abi-vtables -emit-llvm %s -o - | FileCheck %s --check-prefixes CHECK,CHECK-REL

// We emit slightly different address spaces in xmain comparing to intel/llvm
// CHECK: @_ZTVN10__cxxabiv120__si_class_type_infoE = external addrspace(1) global ptr addrspace(4)
// CHECK: @_ZTVN10__cxxabiv117__class_type_infoE = external addrspace(1) global ptr addrspace(4)
// CHECK-PTR: @_ZTI4Base = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i64 2)
// CHECK-PTR: @_ZTI8Derived1 = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv120__si_class_type_infoE to ptr addrspace(4)), i64 2)
// CHECK-REL: @_ZTI4Base = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (i8, ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i32 8)
// CHECK-REL: @_ZTI8Derived1 = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } { ptr addrspace(4) getelementptr inbounds (i8, ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv120__si_class_type_infoE to ptr addrspace(4)), i32 8)
#endif // INTEL_CUSTOMIZATION

SYCL_EXTERNAL bool rand();

class Base {
   public:
    virtual void display() {}
};

class Derived1 : public Base {
   public:
    void display() {}
};

SYCL_EXTERNAL void test() {
  Derived1 d1;
  Base *b = nullptr;
  if (rand())
    b = &d1;
  b->display();
}
