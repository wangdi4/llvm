// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -std=c++11 -fintel-compatibility    %s -o - | FileCheck -check-prefix=CHECK-LIN %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -emit-llvm -std=c++11 -fintel-compatibility %s -o - | FileCheck -check-prefix=CHECK-WIN %s

int _regcall foo(int i);

int main()
{
  int p = 0, _data;
  auto lambda = [&](int parameter) -> int {
    _data = foo(parameter);
    return _data;
  };
  return lambda(p);
}

// CHECK-LIN: call x86_regcallcc {{.+}} @_Z15__regcall3__foo
// CHECK-WIN:  call x86_regcallcc {{.+}}@{{.+}}__regcall3__foo

int __regcall foo (int i){
  return i;
}

// CHECK-LIN: define x86_regcallcc {{.+}}@_Z15__regcall3__foo
// CHECK-WIN: define x86_regcallcc {{.+}}@{{.+}}__regcall3__foo
