// INTEL_FEATURE_SW_DTRANS
// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility-enable=IntelMempoolCtorDtor \
// RUN:  -triple x86_64-unknown-linux -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-LINUX --check-prefix=CHECK-BOTH %s

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility-enable=IntelMempoolCtorDtor \
// RUN:  -triple x86_64-pc-windows-msvc -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-MS --check-prefix=CHECK-BOTH %s

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility \
// RUN:  -fintel-compatibility-disable=IntelMempoolCtorDtor \
// RUN:  -triple x86_64-unknown-linux -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-DISABLE-LINUX \
// RUN:  --check-prefix=CHECK-DISABLE-BOTH %s

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility \
// RUN:  -fintel-compatibility-disable=IntelMempoolCtorDtor \
// RUN:  -triple x86_64-pc-windows-msvc -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-DISABLE-MS \
// RUN:  --check-prefix=CHECK-DISABLE-BOTH %s

// This test verifies that intel-mempool-constructor and
// intel-mempool-destructor attributes are generated correctly.
//
// CHECK-LINUX: define {{.*}} @_ZN1AC2Ev({{.*}}) {{.*}} [[ATTR0:#[0-9]+]]
// CHECK-LINUX: define {{.*}} @_ZN1AD2Ev({{.*}}) {{.*}} [[ATTR1:#[0-9]+]]

// CHECK-MS: define {{.*}} @"??0A@@QEAA@XZ"({{.*}})  {{.*}} [[ATTR0:#[0-9]+]]
// CHECK-MS: define {{.*}} @"??1A@@QEAA@XZ"({{.*}}) {{.*}} [[ATTR1:#[0-9]+]]

// CHECK-DISABLE-LINUX: define {{.*}} @_ZN1AC2Ev({{.*}}) {{.*}} [[ATTR0:#[0-9]+]]
// CHECK-DISABLE-LINUX: define {{.*}} @_ZN1AD2Ev({{.*}}) {{.*}} [[ATTR1:#[0-9]+]]

// CHECK-DISABLE-MS: define {{.*}} @"??0A@@QEAA@XZ"({{.*}})  {{.*}} [[ATTR0:#[0-9]+]]
// CHECK-DISABLE-MS: define {{.*}} @"??1A@@QEAA@XZ"({{.*}}) {{.*}} [[ATTR1:#[0-9]+]]

struct A {
  double d;
  A() { d = 20; }
  ~A() { d = 0;}
};

A a;

// CHECK-BOTH: attributes [[ATTR0]] = { {{.*}}"intel-mempool-constructor"{{.*}} }
// CHECK-BOTH: attributes [[ATTR1]] = { {{.*}}"intel-mempool-destructor"{{.*}} }

// CHECK-DISABLE-BOTH-NOT: attributes [[ATTR0]] = { {{.*}}"intel-mempool-constructor"
// CHECK-DISABLE-BOTH-NOT: attributes [[ATTR1]] = { {{.*}}"intel-mempool-destructor"
// end INTEL_FEATURE_SW_DTRANS
