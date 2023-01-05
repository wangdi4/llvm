__kernel void intel_scale(__global uchar *pS, __global uchar *pD,
                          const int iter) {
  int id = get_global_id(0);
  float t = (float)pS[id] + 1.0f;
  float sum = 0.0f;
  for (int i = 0; i < iter; ++i, t += 1.0f)
    sum += 1.0f / (t * t);
  pD[id] = convert_uchar(sum);
}
