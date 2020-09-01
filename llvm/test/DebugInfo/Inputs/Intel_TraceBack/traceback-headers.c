#include "traceback-header1.h"
#include "traceback-header2.h"

int main() {
  int x = header1_f1();
  x += header2_f1();
  x -= header1_f2();
  return x;
}
