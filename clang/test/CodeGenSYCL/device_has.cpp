// RUN: %clang_cc1 -fsycl-is-device -internal-isystem %S/Inputs -triple spir64-unknown-unknown -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

// Tests for IR of device_has(aspect, ...) attribute
#include "sycl.hpp"

using namespace sycl;
queue q;

// CHECK: define dso_local spir_kernel void @{{.*}}kernel_name_1{{.*}} !sycl_declared_aspects ![[ASPECTS1:[0-9]+]]

// CHECK: define dso_local spir_func void @{{.*}}func1{{.*}} !sycl_declared_aspects ![[ASPECTS1]] {
[[sycl::device_has(sycl::aspect::cpu)]] void func1() {}

// CHECK: define dso_local spir_func void @{{.*}}func2{{.*}} !sycl_declared_aspects ![[ASPECTS2:[0-9]+]] {
[[sycl::device_has(sycl::aspect::fp16, sycl::aspect::gpu)]] void func2() {}

// CHECK: define dso_local spir_func void @{{.*}}func3{{.*}} !sycl_declared_aspects ![[EMPTYASPECTS:[0-9]+]] {
[[sycl::device_has()]] void func3() {}

// CHECK: define linkonce_odr spir_func void @{{.*}}func4{{.*}} !sycl_declared_aspects ![[ASPECTS3:[0-9]+]] {
template <sycl::aspect Aspect>
[[sycl::device_has(Aspect)]] void func4() {}

// CHECK: define dso_local spir_func void @{{.*}}func5{{.*}} !sycl_declared_aspects ![[ASPECTS1]] {
[[sycl::device_has(sycl::aspect::cpu)]] void func5();
void func5() {}

constexpr sycl::aspect getAspect() { return sycl::aspect::cpu; }
// CHECK: define dso_local spir_func void @{{.*}}func6{{.*}} !sycl_declared_aspects ![[ASPECTS1]] {
[[sycl::device_has(getAspect())]] void func6() {}

class KernelFunctor {
public:
  [[sycl::device_has(sycl::aspect::cpu)]] void operator()() const {
    func1();
    func2();
    func3();
    func4<sycl::aspect::host>();
    func5();
    func6();
  }
};

void foo() {
  q.submit([&](handler &h) {
    KernelFunctor f1;
    h.single_task<class kernel_name_1>(f1);
    // CHECK: define dso_local spir_kernel void @{{.*}}kernel_name_2{{.*}} !sycl_declared_aspects ![[ASPECTS4:[0-9]+]]
    h.single_task<class kernel_name_2>([]() [[sycl::device_has(sycl::aspect::gpu)]] {});
  });
}

// CHECK: [[ASPECTS1]] = !{i32 1}
// CHECK: [[EMPTYASPECTS]] = !{}
// CHECK: [[ASPECTS2]] = !{i32 5, i32 2}
// CHECK: [[ASPECTS3]] = !{i32 0}
// CHECK: [[ASPECTS4]] = !{i32 2}
