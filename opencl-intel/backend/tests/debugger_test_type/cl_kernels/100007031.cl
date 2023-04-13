__constant int myglob = 333;
__constant float hisglob = 1.f;

void blehLLLLLLLLLLLLLLLLLLLLLLLLLLLLL(__global uchar *buf, int i) {
  int ggggggggggggggggg = get_global_id(i);

  if (i == 0)
    return;
  else {
    uchar t = buf[i];
    buf[i] = buf[i - 1];
    buf[i - 1] = t;
  }
}

void mybarBBBBBBBBBBBBBBBBBBBBBBBB() {
  int myglob = 333;
  printf("in mybar 1\n!");
  printf("in mybar 2\n!");
}

void myfooMMMMMMMMMMMMMMMMMMMMMMMM(__global uchar *buf) {
  printf("In myfoo!\n");
  barrier(CLK_LOCAL_MEM_FENCE);
  int bb = 2435, df;

  mybarBBBBBBBBBBBBBBBBBBBBBBBB();
  if (buf[1] != 0)
    return;

  df = bb * 2;

  int dimension = buf[1] == 0 ? 0 : 1;
  blehLLLLLLLLLLLLLLLLLLLLLLLLLLLLL(buf, get_global_id(dimension));
}

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  int myarr[4];
  int i, u1, u2;
  int gid0 = 2;
  uint b;
  bool boo = true;
  size_t gigid1 = get_global_id(1);
  int kwa = 1;
  kwa = 2;
  printf("GIDS %u %u %u\n", get_global_id(0), get_global_id(1),
         get_global_id(2));
  float4 fl4 = (float4)(1.1f, 2.2f, 3.3f, 4.4f);
  char4 cc4 = (char4)(17, -18, 19, 20);
  short4 ss4 = (short4)(0xAC, 0xBC, 0xCC, 0xDC);
  int3 ii3 = (int3)(1, 2, 3);
  int2 ii2 = (int2)(gid0, 9);
  int4 ii4 = (int4)(0xAC, 0xBC, 0xCC, 0xDC);
  b = i;
  b = ii2.x + ii2.y;
  i = ii2.x + b * 20;
  printf("Int vector: %v4d\n", ii4);
  printf("Short vector: %v4d\n", ss4);
  printf("Dah float vectorz: %v4f\n", fl4);
  printf("Before myfoo!\n");
  barrier(CLK_LOCAL_MEM_FENCE);
  myfooMMMMMMMMMMMMMMMMMMMMMMMM(buf_in);
  printf("After myfoo!\n");
  for (b = 0; b < 1 && boo; ++b) {
    int kwade = b;
    i = ii2.y + kwade;
  }
  for (i = 0; i < 3 && boo; ++i) {
    int dwik = i * 2;
    ii2.x = dwik;
    ii2.y = i * (ii2.x + 7);

    blehLLLLLLLLLLLLLLLLLLLLLLLLLLLLL(buf_in, i);
  }
  i = ii2.x + b * 20;
}
