/*
 * intel_MonteCarloKernel
 * WRITE YOUR KERNEL DESCRIPTIION HERE
 * @param output output buffer
 * @param input  input buffer
 * @param buffer_size  buffer size
 */
#define VLEN 1
#define LOG log
#define EXP exp
#define SQRT sqrt
typedef struct {
  float S;
  float X;
  float T;
  float R;
  float V;
} TOptionData;
typedef struct {
  float Expected;
  float Confidence;
} TOptionValue;
__kernel void intel_MonteCarloKernel(__global TOptionValue *pOptionValue,
                                     __global TOptionData *pOptionData,
                                     const int pathN) {
  const int optionid = get_global_id(0);
  float T = pOptionData[optionid].T;
  float R = pOptionData[optionid].R;
  float S = pOptionData[optionid].S;
  float X = pOptionData[optionid].X;
  float V = pOptionData[optionid].V;
  float MuByT = (R - 0.5f * V * V) * T;
  float VBySqrtT = V * SQRT(T);
  float sum = 0.0f;
  float sum2 = 0.0f;
  int boundary12 = VLEN * (((int)(0.08f * pathN)) / VLEN);
  int boundary23 = VLEN * (((int)(0.92f * pathN)) / VLEN);
  int pos, n;
  float pathNinv = 1.0f / (float)(pathN + 1);
  {
    //
    // Region 1 of Moro polynomial approximation to inverse CND
    // (0 < p <= 0.08)
    //
    int index = 0;
    const float c1 = 0.337475482272615f;
    const float c2 = 0.976169019091719f;
    const float c3 = 0.160797971491821f;
    const float c4 = 2.76438810333863E-02f;
    const float c5 = 3.8405729373609E-03f;
    const float c6 = 3.951896511919E-04f;
    const float c7 = 3.21767881768E-05f;
    const float c8 = 2.888167364E-07f;
    const float c9 = 3.960315187E-07f;
    for (pos = 0; pos < boundary12; pos++) {
      float p = (pos + 1) * pathNinv;
      float z = LOG(-LOG(p));
#ifdef USE_MAD
      float sample =
          mad(mad(mad(mad(mad(mad(mad(mad(c9, z, c8), z, c7), z, c6), z, c5), z,
                          c4),
                      z, c3),
                  z, c2),
              z, c1);
#else
      float sample =
          c1 +
          z * (c2 +
               z * (c3 +
                    z * (c4 +
                         z * (c5 + z * (c6 + z * (c7 + z * (c8 + z * c9)))))));
#endif
      sample = -sample;
      float callValue = S * EXP(sample * VBySqrtT + MuByT) - X;
      callValue = max(callValue, (float)0.0);
      sum += callValue;
      sum2 += callValue * callValue;
    }
  }
  {
    // if(optionid == 1)    printf("sum: %f sum2: %f\n",
    // (sum.x+sum.y+sum.z+sum.w), (sum2.x + sum2.y + sum2.z + sum2.w));
    //
    //  Region 2 of Moro polynomial approximation to inverse CND
    //  (0.08 < p < 0.92)
    //
    const float a1 = 2.50662823884f;
    const float a2 = -18.61500062529f;
    const float a3 = 41.39119773534f;
    const float a4 = -25.44106049637f;
    const float b1 = -8.4735109309f;
    const float b2 = 23.08336743743f;
    const float b3 = -21.06224101826f;
    const float b4 = 3.13082909833f;
    for (pos = boundary12; pos < boundary23; pos++) {
      float p = (pos + 1) * pathNinv;
      float y = p - 0.5f;
      float z = y * y;
#ifdef USE_MAD
      float sample = y * mad(mad(mad(a4, z, a3), z, a2), z, a1) /
                     mad(mad(mad(mad(b4, z, b3), z, b2), z, b1), z, 1.0f);
#else
      float sample = y * (((a4 * z + a3) * z + a2) * z + a1) /
                     ((((b4 * z + b3) * z + b2) * z + b1) * z + 1.0f);
#endif
      float callValue = S * EXP(sample * VBySqrtT + MuByT) - X;
      callValue = max((float)callValue, (float)0.0);
      sum += callValue;
      sum2 += callValue * callValue;
    }
  }
  {
    //
    // Region 3 of Moro polynomial approximation to inverse CND
    // (0.92 <= p < 1.0)
    //
    int index = 0;
    const float c1 = 0.337475482272615f;
    const float c2 = 0.976169019091719f;
    const float c3 = 0.160797971491821f;
    const float c4 = 2.76438810333863E-02f;
    const float c5 = 3.8405729373609E-03f;
    const float c6 = 3.951896511919E-04f;
    const float c7 = 3.21767881768E-05f;
    const float c8 = 2.888167364E-07f;
    const float c9 = 3.960315187E-07f;
    for (pos = boundary23; pos < pathN; pos++) {
      float p = (pos + 1) * pathNinv;
      float z = LOG(-LOG(1.0f - p));
#ifdef USE_MAD
      float sample =
          mad(mad(mad(mad(mad(mad(mad(mad(c9, z, c8), z, c7), z, c6), z, c5), z,
                          c4),
                      z, c3),
                  z, c2),
              z, c1);
#else
      float sample =
          c1 +
          z * (c2 +
               z * (c3 +
                    z * (c4 +
                         z * (c5 + z * (c6 + z * (c7 + z * (c8 + z * c9)))))));
#endif
      float callValue = S * EXP(sample * VBySqrtT + MuByT) - X;
      callValue = max(callValue, (float)0.0);
      sum += callValue;
      sum2 += callValue * callValue;
    }
  }
  {
    float totalsum = sum;
    float totalsum2 = sum2;
    // printf("%f %f %f %f %f\n",sum.x,sum.y,sum.z,sum.w,totalsum );
    float ExpRT = EXP(-R * T);
    // Derive average from the total sum and discount by riskfree rate
    pOptionValue[optionid].Expected = (float)(ExpRT * totalsum / (float)pathN);
    // Standard deviation
    float stdDev = SQRT(((float)pathN * totalsum2 - totalsum * totalsum) /
                        ((float)pathN * (float)(pathN - 1)));
    // Confidence width; in 95% of all cases theoretical value lies within these
    // borders
    pOptionValue[optionid].Confidence =
        (float)(ExpRT * 1.96f * stdDev / SQRT((float)pathN));
    // printf("Count=%d\n",count);
  }
}
__kernel void
intel_MonteCarloKernel_RandomSamples(__global TOptionValue *pOptionValue,
                                     __global TOptionData *pOptionData,
                                     const int pathN) {
  const int optionid = get_global_id(0);
  float T = pOptionData[optionid].T;
  float R = pOptionData[optionid].R;
  float S = pOptionData[optionid].S;
  float X = pOptionData[optionid].X;
  float V = pOptionData[optionid].V;
  float MuByT = (R - 0.5f * V * V) * T;
  float VBySqrtT = V * SQRT(T);
  float sum = 0.0f;
  float sum2 = 0.0f;
  float boundary12 = 0.08f;
  float boundary23 = 0.92f;
  float pathNinv = 1.0f / (float)(pathN + 1);
  int i;
#define RND_N 624
#define RND_M 397
#define RND_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define RND_LOWER_MASK 0x7fffffffUL /* least significant r bits */
  unsigned int mt[RND_N]; // state array for Mersenne Twister random generator
  int rndi = 0;
  // init states array
  mt[0] = (optionid + 1) & 0xffffffffUL;
  for (i = 1; i < RND_N; i++)
    mt[i] =
        ((1812433253UL * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i)) & 0xffffffffUL;
  for (i = 0; i < pathN; ++i) {
    // 1. generate random value 0..1
    float p = (i + 1) * pathNinv;
    float sample = 0.0f;
    int rndi_1 = (rndi + 1) % RND_N;
    int rndi_M = (rndi + RND_M) % RND_N;
    unsigned int y;
    y = (mt[rndi] & RND_UPPER_MASK) | (mt[rndi_1] & RND_LOWER_MASK);
    mt[rndi] = mt[rndi_M] ^ (y >> 1);
    mt[rndi] ^= (y & 1) * 0x9908b0dfUL;
    y = mt[rndi];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    rndi = rndi_1;
    // convert 0..(2^32-1) into 0..1.0f
    p = ((float)y + 256.0f) * (1.0f / (4294967295.0f + 512.0f));
    if (p < 0.0f)
      p = 0.0f;
    if (p > 1.0f)
      p = 1.0f;
    // printf("=%f\n",p);
    //  2. convert to gaussian distribution
    if (p < boundary12) { //
      // Region 1 of Moro polynomial approximation to inverse CND
      // (0 < p <= 0.08)
      const float c1 = 0.337475482272615f;
      const float c2 = 0.976169019091719f;
      const float c3 = 0.160797971491821f;
      const float c4 = 2.76438810333863E-02f;
      const float c5 = 3.8405729373609E-03f;
      const float c6 = 3.951896511919E-04f;
      const float c7 = 3.21767881768E-05f;
      const float c8 = 2.888167364E-07f;
      const float c9 = 3.960315187E-07f;
      float z;
      z = LOG(-LOG(p));
      z = c1 +
          z * (c2 +
               z * (c3 +
                    z * (c4 +
                         z * (c5 + z * (c6 + z * (c7 + z * (c8 + z * c9)))))));
      sample = -z;
    } else if (p < boundary23) {
      // Region 2 of Moro polynomial approximation to inverse CND
      // (0.08 < p < 0.92)
      const float a1 = 2.50662823884f;
      const float a2 = -18.61500062529f;
      const float a3 = 41.39119773534f;
      const float a4 = -25.44106049637f;
      const float b1 = -8.4735109309f;
      const float b2 = 23.08336743743f;
      const float b3 = -21.06224101826f;
      const float b4 = 3.13082909833f;
      float y = p - 0.5f;
      float z = y * y;
      sample = y * (((a4 * z + a3) * z + a2) * z + a1) /
               ((((b4 * z + b3) * z + b2) * z + b1) * z + 1.0f);
    } else {
      // Region 3 of Moro polynomial approximation to inverse CND
      // (0.92 <= p < 1.0)
      //
      const float c1 = 0.337475482272615f;
      const float c2 = 0.976169019091719f;
      const float c3 = 0.160797971491821f;
      const float c4 = 2.76438810333863E-02f;
      const float c5 = 3.8405729373609E-03f;
      const float c6 = 3.951896511919E-04f;
      const float c7 = 3.21767881768E-05f;
      const float c8 = 2.888167364E-07f;
      const float c9 = 3.960315187E-07f;
      float z = LOG(-LOG(1.0f - p));
      sample =
          c1 +
          z * (c2 +
               z * (c3 +
                    z * (c4 +
                         z * (c5 + z * (c6 + z * (c7 + z * (c8 + z * c9)))))));
    }
    // 3. updates sum (make integration step)
    float callValue = S * EXP(sample * VBySqrtT + MuByT) - X;
    callValue = max(callValue, (float)0.0);
    sum += callValue;
    sum2 += callValue * callValue;
  }
  {
    // float totalsum = sum;
    // float totalsum2 = sum2;
    // printf("%f %f %f %f %f\n",sum.x,sum.y,sum.z,sum.w,totalsum );
    float ExpRT = EXP(-R * T);
    // Derive average from the total sum and discount by riskfree rate
    pOptionValue[optionid].Expected = (float)(ExpRT * sum / (float)pathN);
    // Standard deviation
    float stdDev = SQRT(((float)pathN * sum2 - sum * sum) /
                        ((float)pathN * (float)(pathN - 1)));
    // Confidence width; in 95% of all cases theoretical value lies within these
    // borders
    pOptionValue[optionid].Confidence =
        (float)(ExpRT * 1.96f * stdDev / SQRT((float)pathN));
    // printf("Count=%d\n",count);
  }
}
__kernel void intel_MonteCarloKernel_PrecalculatedSamples(
    __global TOptionValue *pOptionValue, __global TOptionData *pOptionData,
    __global float *pSamples, const int pathN) {
  const int optionid = get_global_id(0);
  float T = pOptionData[optionid].T;
  float R = pOptionData[optionid].R;
  float S = pOptionData[optionid].S;
  float X = pOptionData[optionid].X;
  float V = pOptionData[optionid].V;
  float MuByT = (R - 0.5f * V * V) * T;
  float VBySqrtT = V * SQRT(T);
  float sum = 0.0f;
  float sum2 = 0.0f;
  int pos;
  for (pos = 0; pos < pathN; pos++) {
    float expVal = EXP(pSamples[pos] * VBySqrtT + MuByT);
    float callValue = S * expVal - X;
    callValue = max(callValue, (float)0.0);
    sum += callValue;
    sum2 += callValue * callValue;
  }
  {
    float totalsum = sum;
    float totalsum2 = sum2;
    float ExpRT = EXP(-R * T);
    pOptionValue[optionid].Expected = (float)(ExpRT * totalsum / (float)pathN);
    // Standard deviation
    float stdDev = SQRT(((float)pathN * totalsum2 - totalsum * totalsum) /
                        ((float)pathN * (float)(pathN - 1)));
    // Confidence width; in 95% of all cases theoretical value lies within these
    // borders
    pOptionValue[optionid].Confidence =
        (float)(ExpRT * 1.96f * stdDev / SQRT((float)pathN));
    //    printf("Count=%d\n",count);
  }
}
