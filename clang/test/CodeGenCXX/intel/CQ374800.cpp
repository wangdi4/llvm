// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -std=c++03 -emit-llvm -o - | FileCheck %s -check-prefix=CHECK-GNU
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 -fms-compatibility %s -std=c++03 -emit-llvm -o - | FileCheck %s -check-prefix=CHECK-MS

namespace std {
  // CHECK-GNU: ret %"class.std::type_info"* null
  // CHECK-MS: ret %class.type_info* null
  const type_info* f() { return 0; }
}

