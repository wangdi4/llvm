// CQ#364427
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// RUN: not %clang_cc1 -fsyntax-only -fintel-compatibility -fdiagnostics-parseable-fixits %s 2>&1 | FileCheck %s

// expected-note@+1 5{{previous definition is here}}
int main() {
  return 0;
}

// expected-error@+3 {{functions that differ only in their return type cannot be overloaded}}
// expected-warning@+2 {{return type of 'main' is not 'int'}}
// expected-note@+1 {{change return type to 'int'}}
void main() {
// CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:1-[[@LINE-1]]:5}:"int"
}

// expected-error@+3 {{functions that differ only in their return type cannot be overloaded}}
// expected-warning@+2 {{return type of 'main' is not 'int'}}
// expected-note@+1 {{change return type to 'int'}}
double main() {
// CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:1-[[@LINE-1]]:7}:"int"
  return 0.0;
}

// expected-error@+3 {{functions that differ only in their return type cannot be overloaded}}
// expected-warning@+2 {{return type of 'main' is not 'int'}}
// expected-note@+1 {{change return type to 'int'}}
const float main() {
// CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:7-[[@LINE-1]]:12}:"int"
  return 0.0f;
}

typedef void *(*fptr)(int a);

// expected-error@+3 {{functions that differ only in their return type cannot be overloaded}}
// expected-warning@+2 {{return type of 'main' is not 'int'}}
// expected-note@+1 {{change return type to 'int'}}
fptr main() {
// CHECK: fix-it:"{{.*}}":{[[@LINE-1]]:1-[[@LINE-1]]:5}:"int"
  return (fptr) 0;
}

// expected-error@+2 {{functions that differ only in their return type cannot be overloaded}}
// expected-warning@+1 {{return type of 'main' is not 'int'}}
void *(*main())(int a) {
  return (fptr) 0;
}
