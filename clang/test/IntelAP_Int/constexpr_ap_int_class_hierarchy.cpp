// Ensure we can declare a class hierarch with constexpr constructors
// where the base class has an ap_int member.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL

typedef int int65_tt __attribute__((__ap_int(65)));

class Base {
  int65_tt an_int;
public:
  constexpr Base(int init) : an_int(init) {}
};

class Derived : public Base {
public:
  constexpr Derived(int init) : Base(init) {}
};

//HLS: %class.Derived = type { %class.Base }
//SYCL: %class.{{.*}}.Derived = type { %class.{{.*}}.Base }
//HLS: %class.Base = type { i65 }
//SYCL: %class.{{.*}}.Base = type { i65 }

constexpr Derived a_derived = 17;
//HLS: @[[VAR:[a-zA-Z0-9_]+]] = internal constant { i65 } { i65 17 }, align 8
//SYCL: @[[VAR:[a-zA-Z0-9_]+]] = internal addrspace(1) constant { i65 } { i65 17 }, align 8

#ifdef SYCL_EXTERNAL
SYCL_EXTERNAL
#endif
Derived foo() {
  return a_derived;
}
