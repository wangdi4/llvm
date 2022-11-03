#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  // Typedef
  size_t gid = get_global_id(0);

  // Basic types
  bool bb = true;
  int ii = 45;
  long ll = -29393939;
  short ss = -4040;
  uint tdui = 50000;
  unsigned short uss = 990;
  unsigned int uii = 1010;
  unsigned long ull = 99181839;
  float ff;
  double dd;

  struct SomeStruct {
    int a;
    unsigned b;
  };
  struct SomeStruct somestruct;

  struct {
    bool b;
    bool c;
  } anonstruct;

  enum SomeEnum { RED = 2, BLUE = 3 };
  enum SomeEnum someenum;

  float4 ff4 = (float4)(1.25f, 2.75f, 3.0625f, 4.5f);
  double2 dd2;
  char3 cc3 = (char3)('a', 'b', 'c');
  short16 ss16;

  uint **ppuii;
  uint uuaa[3][4];
  uint auarr[4] = {90, 10, 30, 40};

  buf_out[0] = 0;
}
