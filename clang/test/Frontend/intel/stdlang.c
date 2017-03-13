// RUN: %clang_cc1 -x c -fintel-compatibility -std=c++11 %s 2>&1 | FileCheck --check-prefix=CHECK-C %s
// RUN: %clang_cc1 -x c++ -fintel-compatibility -std=c99 %s 2>&1 | FileCheck --check-prefix=CHECK-CPP %s
// CHECK-C: warning: invalid argument '-std=c++11' not allowed with 'C/ObjC'
// CHECK-CPP: warning: invalid argument '-std=c99' not allowed with 'C++/ObjC++'

