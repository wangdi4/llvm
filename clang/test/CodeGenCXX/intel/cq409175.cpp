// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s --check-prefixes CHECK,LIN
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -o - %s | FileCheck %s --check-prefixes CHECK,WIN

namespace NS {
  struct S {int x,y;};
}

struct NS::S;
NS::S s = {1,2};
// CHECK: [[S:%.+]] = type { i32, i32 }
// LIN: @{{.+}} = global [[S]] { i32 1, i32 2 }
// WIN: @{{.+}} = dso_local global [[S]] { i32 1, i32 2 }
