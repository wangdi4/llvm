// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-compatibility -fintel-ms-compatibility -emit-llvm -o - %s | FileCheck %s

struct C3 {
  virtual void foo();
  ~C3();
};

C3::~C3() {
}

// CHECK: define void @"\01??1C3@@QEAA@XZ"(%struct.C3* %this)
// CHECK: store i32 (...)** bitcast (i8** @"\01??_7C3@@6B@" to i32 (...)**), i32 (...)*** %{{.+}}, align 8
