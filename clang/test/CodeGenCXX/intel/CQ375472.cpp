// RUN: %clang_cc1 -verify -emit-llvm %s -triple x86_64-unknown-linux-gnu -fintel-compatibility -o - | FileCheck %s

struct BadDerived;
struct BadBase { operator BadDerived&(); };
struct BadDerived : BadBase {};

// CHECK-LABEL: test
void test()
{
  // these are ambiguous
  bool i1 = false;
  // CHECK: [[BB:%.+]] = alloca %struct.BadBase,
  // CHECK: [[BD:%.+]] = alloca %struct.BadDerived,
  BadBase bb;
  BadDerived bd;
  // CHECK: [[BOOL:%.+]] = trunc i8 %{{.+}} to i1
  // CHECK: br i1 [[BOOL]],
  // CHECK: [[BB_DER:%.+]] = call dereferenceable(1) %struct.BadDerived* @_ZN7BadBasecvR10BadDerivedEv(%struct.BadBase* [[BB]])
  // CHECK: phi %struct.BadDerived* [ [[BB_DER]], %{{.+}} ], [ [[BD]], %{{.+}} ]
  (void)(i1 ? bb : bd); // expected-warning {{conditional expression is ambiguous; 'BadBase' can be converted to 'BadDerived' and vice versa}}
  // CHECK: [[BOOL:%.+]] = trunc i8 %{{.+}} to i1
  // CHECK: br i1 [[BOOL]],
  // CHECK: [[BD_BASE:%.+]] = bitcast %struct.BadDerived* [[BD]] to %struct.BadBase*
  // CHECK: phi %struct.BadBase* [ [[BD_BASE]], %{{.+}} ], [ [[BB]], %{{.+}} ]
  (void)(i1 ? bd : bb); // expected-warning {{conditional expression is ambiguous; 'BadDerived' can be converted to 'BadBase' and vice versa}}
}

