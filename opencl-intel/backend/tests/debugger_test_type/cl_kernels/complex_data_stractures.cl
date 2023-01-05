#define ARRAY_SIZE 2

typedef struct {
  float r;
  float i;
} Complex;

typedef struct {
  uint r;
  uint i;
} NaturalComplex;

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  Complex z;
  z.r = 3;
  z.i = -1;
  Complex *pZ = (Complex *)39;
  Complex **ppZ = (Complex **)49;
  ulong8 ulongArr[ARRAY_SIZE];
  Complex complexArr[ARRAY_SIZE];
  int dim4Arr[2][3][4] = {
      {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}},
      {{21, 22, 23, 24}, {25, 26, 27, 28}, {29, 30, 31, 32}}};
  int i;
  for (i = 0; i < ARRAY_SIZE; i++) {
    ulongArr[i] = (ulong8)(8 * i, 8 * i + 1, 8 * i + 2, 8 * i + 3, 8 * i + 4,
                           8 * i + 5, 8 * i + 6, 8 * i + 7);
    complexArr[i].r = i;
    complexArr[i].i = -i;
  }
  i++;
}
