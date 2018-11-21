// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu \
// RUN:  -std=c++03 -emit-llvm -o - %s | FileCheck %s -check-prefix=CHECK-GNU

// RUN: %clang_cc1 -fintel-compatibility-enable=PredeclareTypeInfo \
// RUN:  -triple x86_64-unknown-linux-gnu -std=c++03 -emit-llvm -o - %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-GNU

// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility \
// RUN:  -triple x86_64-unknown-linux-gnu -std=c++03 \
// RUN:  -fintel-compatibility-disable=PredeclareTypeInfo -verify %s -DERROR

// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 \
// RUN:  -fms-compatibility -std=c++03 -emit-llvm -o -  %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-MS

namespace std {
  // CHECK-GNU: ret %"class.std::type_info"* null
  // CHECK-MS: ret %class.type_info* null
  const type_info* f() { return 0; }
#ifdef ERROR
  // expected-error@-2 {{unknown type name 'type_info'}}
#endif
}

