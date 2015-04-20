// CQ364709
// RUN: %clang_cc1 -triple=x86_64-apple-darwin10 -fintel-ms-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple=x86_64-apple-darwin10 -fintel-ms-compatibility -O2 -emit-llvm %s -o - | FileCheck --check-prefix CHECK-O2 %s

int ii;

struct X {
  X() { ii = 10; }
  X(const X &other) { ii = 20; }
  ~X() { ii = 40; }
};

struct Y {
  struct {
    X ax;
  };

  Y() { ii = 15; }
  Y(const Y &other): ax(other.ax) { ii = 25; }
  ~Y() { ii = 45; }
};

int main() {
  ii = 0;
  {
    // CHECK: %{{.+}} = alloca %struct.Y
    // CHECK: %{{.+}} = alloca %struct.Y
    // CHECK: call void @_ZN1YC1Ev(%struct.Y* %{{.+}})
    // CHECK: call void @_ZN1YC1ERKS_(%struct.Y* %{{.+}}, %struct.Y* {{.*}} %{{.+}})
    // CHECK: call void @_ZN1YD1Ev(%struct.Y* %{{.+}})
    // CHECK: call void @_ZN1YD1Ev(%struct.Y* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YC1Ev(%struct.Y* %{{.+}})
    // CHECK: call void @_ZN1YC2Ev(%struct.Y* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YC1ERKS_(%struct.Y* %{{.+}}, %struct.Y* {{.*}} %{{.+}})
    // CHECK: call void @_ZN1YC2ERKS_(%struct.Y* %{{.+}}, %struct.Y* {{.*}} %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YD1Ev(%struct.Y* %{{.+}})
    // CHECK: call void @_ZN1YD2Ev(%struct.Y* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YC2Ev(%struct.Y* %{{.+}})
    // CHECK: call void @_ZN1XC1Ev(%struct.X* %{{.+}})
    // CHECK: store i32 15, i32* @{{.+}}
    //
    // CHECK: define {{.*}} void @_ZN1XC1Ev(%struct.X* %{{.+}})
    // CHECK: call void @_ZN1XC2Ev(%struct.X* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1XC2Ev(%struct.X* %{{.+}})
    // CHECK: store i32 10, i32* @{{.+}}
    //
    // CHECK: define {{.*}} void @_ZN1YC2ERKS_(%struct.Y* %{{.+}}, %struct.Y* {{.*}} %{{.+}})
    // CHECK: call void @_ZN1XC1ERKS_(%struct.X* %{{.+}}, %struct.X* {{.*}} %{{.+}})
    // CHECK: store i32 25, i32* @{{.+}}
    //
    // CHECK: define {{.*}} void @_ZN1XC1ERKS_(%struct.X* %{{.+}}, %struct.X* {{.*}} %{{.+}})
    // CHECK: call void @_ZN1XC2ERKS_(%struct.X* %{{.+}}, %struct.X* {{.*}} %{{.+}})
    // CHECK: store i32 20, i32* @{{.+}}
    //
    // CHECK: define {{.*}} void @_ZN1YD2Ev(%struct.Y* %{{.+}})
    // CHECK: store i32 45, i32* @{{.+}}
    // CHECK: call void @_ZN1YUt_D1Ev(%struct.anon* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YUt_D1Ev(%struct.anon* %{{.+}})
    // CHECK: call void @_ZN1YUt_D2Ev(%struct.anon* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1YUt_D2Ev(%struct.anon* %{{.+}})
    // CHECK: call void @_ZN1XD1Ev(%struct.X* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1XD1Ev(%struct.X* %{{.+}})
    // CHECK: call void @_ZN1XD2Ev(%struct.X* %{{.+}})
    //
    // CHECK: define {{.*}} void @_ZN1XD2Ev(%struct.X* %{{.+}})
    // CHECK: store i32 40, i32* @{{.+}}
    Y y;
    Y y2 = y;
  }

  // CHECK-O2: ret i32 40
  return ii;
}
