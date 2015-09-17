// CQ#364712
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -x c++ -std=c++11 -fintel-ms-compatibility -emit-llvm %s -o - | FileCheck %s

int main() {

  struct C {
    int i;
    C(int ii) : i(ii) {}
  };
  // CHECK: %{{.+}} = alloca %struct.C*
  // CHECK: %{{.+}} = alloca %struct.C
  // CHECK: call void @_ZZ4mainEN1CC1Ei(%struct.C* %{{.+}}, i32 10)
  // CHECK: store %struct.C* %{{.+}}, %struct.C** %{{.+}}
  C &c = C(10);

  return 0;
}
