// CQ#364712
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -x c++ -std=c++11 -fintel-ms-compatibility -emit-llvm %s -o - | FileCheck %s

int main(int argc, char **argv) {

  struct C {
    int &x;
    int (&y)[2];
  };

  struct D {
    int i;
    D(int ii) : i(ii) {}
  };
  // CHECK: %{{.+}} = alloca %struct.C
  // CHECK: %{{.+}} = alloca i32
  // CHECK: %{{.+}} = alloca [2 x i32]
  // CHECK: %{{.+}} = alloca %struct.D*
  // CHECK: %{{.+}} = alloca %struct.D
  // CHECK: %{{.+}} = getelementptr inbounds %struct.C, %struct.C* %{{.+}}, i32 0, i32 0
  // CHECK: store i32 1, i32* %{{.+}}
  // CHECK: store i32* %{{.+}}, i32** %{{.+}}
  // CHECK: %{{.+}} = getelementptr inbounds %struct.C, %struct.C* %{{.+}}, i32 0, i32 1
  // CHECK: %{{.+}} = getelementptr inbounds [2 x i32], [2 x i32]* %{{.+}}, i64 0, i64 0
  // CHECK: store i32 2, i32* %{{.+}}
  // CHECK: %{{.+}} = getelementptr inbounds i32, i32* %{{.+}}, i64 1
  // CHECK: store i32 3, i32* %{{.+}}
  // CHECK: store [2 x i32]* %{{.+}}, [2 x i32]** %{{.+}}
  // CHECK: call void @{{.+}}(%struct.D* %{{.+}}, i32 10)
  // CHECK: store %struct.D* %{{.+}}, %struct.D** %{{.+}}
  C c{ 1, { 2, 3 } };
  D &d = D(10);

  return 0;
}
