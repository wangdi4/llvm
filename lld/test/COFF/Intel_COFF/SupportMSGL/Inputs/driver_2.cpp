// Driver for test case to suport MS /GL
#include <iostream>

int foo();
int bar();
int baz();

int main () {
  std::cout << "FOO: " << foo() << "\n";
  return 0;
}
