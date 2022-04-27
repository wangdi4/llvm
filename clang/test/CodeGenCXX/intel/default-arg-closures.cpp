// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-extensions -fintel-compatibility -fintel-compatibility-enable=InstantiateDefaultArgs -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility -fintel-compatibility-disable=InstantiateDefaultArgs -verify %s

// The MS ABI requires instantiating of default arguments.
// expected-no-diagnostics

template <typename>
class __declspec(dllexport) foo {
  foo(int x = 0);
};
template <>
foo<int>::foo(int a) {}

// CHECK: define weak_odr dso_local dllexport noundef nonnull align 1 dereferenceable(1) ptr @"??4?$foo@H@@QEAAAEAV0@AEBV0@@Z"
// CHECK: define dso_local dllexport noundef ptr @"??0?$foo@H@@AEAA@H@Z"
// CHECK: define weak_odr dso_local dllexport void @"??_F?$foo@H@@AEAAXXZ"

template <typename>
class __declspec(dllexport) goo {
  goo(const goo& F, int x = 0);
};
template<>
goo<int>::goo(const goo&, int) {};


// CHECK: define weak_odr dso_local dllexport noundef nonnull align 1 dereferenceable(1) ptr @"??4?$goo@H@@QEAAAEAV0@AEBV0@@Z"
// CHECK: define dso_local dllexport noundef ptr @"??0?$goo@H@@AEAA@AEBV0@H@Z"
