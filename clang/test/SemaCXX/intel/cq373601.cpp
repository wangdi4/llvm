// RUN: %clang_cc1 -fsyntax-only -std=c++11 -triple x86_64-unknown-linux-gnu -fintel-compatibility -verify -Wno-gnu-alignof-expression %s
// expected-no-diagnostics

int main() {
  typedef char layout_type1[sizeof(double)][[gnu::aligned(alignof(double))]];
  typedef char layout_type2[sizeof(double)];
  typedef char layout_type3[[gnu::aligned(alignof(double))]];
  typedef char layout_type4;

  layout_type1 data1;
  layout_type2 data2[[gnu::aligned(alignof(double))]];
  layout_type3 data3;
  layout_type4 data4[[gnu::aligned(alignof(double))]];
  int arr1[alignof(data1) == 8 ? 1 : -1];
  int arr2[alignof(data2) == 8 ? 1 : -1];
  int arr3[alignof(data3) == 8 ? 1 : -1];
  int arr4[alignof(data4) == 8 ? 1 : -1];
  return 0;
}

