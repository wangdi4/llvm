#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void convert(global uchar *ucData, global ushort *usData,
                    global uint *uiData, global ulong *ulData,
                    global char *cData, global short *sData, global int *iData,
                    global long *lData, global half *hData, global float *fData,
                    global double *dData, global DTYPE *dst) {
  size_t gid = get_global_id(0);
  uchar uc = ucData[gid];
  ushort us = usData[gid];
  uint ui = uiData[gid];
  ulong ul = ulData[gid];
  char c = cData[gid];
  short s = sData[gid];
  int i = iData[gid];
  long l = lData[gid];
  half h = hData[gid];
  float f = fData[gid];
  double d = dData[gid];

  dst[gid] =
      CONVERT(uc) + CONVERT(us) + CONVERT(ui) + CONVERT(ul) + CONVERT(c) +
      CONVERT(s) + CONVERT(i) + CONVERT(l) + CONVERT(h) + CONVERT(f) +
      CONVERT(d) +

      CONVERT_rte(uc) + CONVERT_rte(us) + CONVERT_rte(ui) + CONVERT_rte(ul) +
      CONVERT_rte(c) + CONVERT_rte(s) + CONVERT_rte(i) + CONVERT_rte(l) +
      CONVERT_rte(h) + CONVERT_rte(f) + CONVERT_rte(d) +

      CONVERT_rtp(uc) + CONVERT_rtp(us) + CONVERT_rtp(ui) + CONVERT_rtp(ul) +
      CONVERT_rtp(c) + CONVERT_rtp(s) + CONVERT_rtp(i) + CONVERT_rtp(l) +
      CONVERT_rtp(h) + CONVERT_rtp(f) + CONVERT_rtp(d) +

      CONVERT_rtn(uc) + CONVERT_rtn(us) + CONVERT_rtn(ui) + CONVERT_rtn(ul) +
      CONVERT_rtn(c) + CONVERT_rtn(s) + CONVERT_rtn(i) + CONVERT_rtn(l) +
      CONVERT_rtn(h) + CONVERT_rtn(f) + CONVERT_rtn(d) +

      CONVERT_rtz(uc) + CONVERT_rtz(us) + CONVERT_rtz(ui) + CONVERT_rtz(ul) +
      CONVERT_rtz(c) + CONVERT_rtz(s) + CONVERT_rtz(i) + CONVERT_rtz(l) +
      CONVERT_rtz(h) + CONVERT_rtz(f) + CONVERT_rtz(d);
}
