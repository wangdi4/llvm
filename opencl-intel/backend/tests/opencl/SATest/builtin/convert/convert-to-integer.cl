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

  dst[gid] = CONVERT(uc) + CONVERT(us) + CONVERT(ui) + CONVERT(ul) +
             CONVERT(c) + CONVERT(s) + CONVERT(i) + CONVERT(l) + CONVERT(h) +
             CONVERT(f) + CONVERT(d) +

             CONVERT_rte(h) + CONVERT_rte(f) + CONVERT_rte(d) + CONVERT_rtp(h) +
             CONVERT_rtp(f) + CONVERT_rtp(d) + CONVERT_rtn(h) + CONVERT_rtn(f) +
             CONVERT_rtn(d) + CONVERT_rtz(h) + CONVERT_rtz(f) + CONVERT_rtz(d);
}
