// RUN: %clang_cc1  -triple x86_64-linux-gnu -fintel-compatibility %s -emit-llvm -o - -verify | FileCheck %s
// CQ#376225: allow the user to define its own operator new/delete if header
// file 'new' is not included
// expected-no-diagnostics

extern void *operator new(unsigned long req) throw();

bool flag = true;
int *val;

void *operator new(unsigned long req) throw() {
  if (flag) {
    return 0;
  } else {
    return val;
  }
}

int main() {
  operator new(3);
  return 0;
}

// CHECK-LABEL: {{.+}}main{{.+}}

