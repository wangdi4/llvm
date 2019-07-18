// Driver for test case to support ELF LTO
#include <iostream>

int foo();
int bar();
int baz();

int main () {
  std::cout << "FOO: " << foo() << "\n";
  return 0;
}
