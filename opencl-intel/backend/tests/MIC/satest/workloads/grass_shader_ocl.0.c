// #define M_PI 3.1415926  //Arik
typedef float Tuple2d[2];
typedef float Tuple3d[3];
typedef float Tuple4d[4];
typedef float Matrix3d[3][3];
typedef struct _GrassAttribute {
  float mLengthMin;
  float mLengthMax;
  float mLengthRandomness;
  float mLengthBias;
  float mLengthGain;
  float mWidthBase;
  float mWidthTip;
  float mWidthRandomness;
  float mWidthBias;
  float mWindPercent;
  Tuple3d mWindDirection;
  float mWindBase;
  float mWindTip;
  float mWindRandomness;
  float mWindBias;
  float mDirectionPercent;
  Tuple2d mDirection;
  float mDirectionRandomness;
  float mFlatnessBase;
  float mFlatnessTip;
  float mFlatnessRandomness;
  float mFlatnessBias;
  int mNCv;
  int mNGrass;
  int mDirectionOn;
  int mWindOn;
  int mPreserveLength;
} GrassAttribute;
// #define M_PI 3.14151
#define C_Tuple3d_ADD_SMUL(a, sb, b, result)                                   \
  result[0] = a[0] + sb * b[0];                                                \
  result[1] = a[1] + sb * b[1];                                                \
  result[2] = a[2] + sb * b[2];
#define C_Tuple3d_NORMALIZE(v)                                                 \
  {                                                                            \
    float m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);                   \
    v[0] /= m;                                                                 \
    v[1] /= m;                                                                 \
    v[2] /= m;                                                                 \
  }
#define C_Tuple3d_SMUL(s, v, result)                                           \
  result[0] = s * v[0];                                                        \
  result[1] = s * v[1];                                                        \
  result[2] = s * v[2];
#define C_Tuple3d_ADD(a, b, result)                                            \
  result[0] = a[0] + b[0];                                                     \
  result[1] = a[1] + b[1];                                                     \
  result[2] = a[2] + b[2];
#define C_Tuple3d_SUB(a, b, result)                                            \
  result[0] = a[0] - b[0];                                                     \
  result[1] = a[1] - b[1];                                                     \
  result[2] = a[2] - b[2];
#define C_Tuple3d_COPY(a, b)                                                   \
  b[0] = a[0];                                                                 \
  b[1] = a[1];                                                                 \
  b[2] = a[2];
#define C_Tuple3d_COMB(t1, v1, t2, v2, v)                                      \
  v[0] = t1 * v1[0] + t2 * v2[0];                                              \
  v[1] = t1 * v1[1] + t2 * v2[1];                                              \
  v[2] = t1 * v1[2] + t2 * v2[2];
#define C_Tuple3d_CROSS(a, b, cross)                                           \
  cross[0] = a[1] * b[2] - a[2] * b[1];                                        \
  cross[1] = a[2] * b[0] - a[0] * b[2];                                        \
  cross[2] = a[0] * b[1] - a[1] * b[0];
#define C_Tuple3d_SDIV(s, d, result)                                           \
  result[0] = d[0] / s;                                                        \
  result[1] = d[1] / s;                                                        \
  result[2] = d[2] / s;
#define C_Tuple3d_LEN(v) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
#define C_shading_LERP(min, max, t) ((min) + (t) * ((max) - (min)))
#define C_shading_LERP3(min, max, t, result)                                   \
  result[0] = min[0] + t * (max[0] - min[0]);                                  \
  result[1] = min[1] + t * (max[1] - min[1]);                                  \
  result[2] = min[2] + t * (max[2] - min[2]);
#define C_MX3F_ident(m)                                                        \
  m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0;             \
  m[0][0] = m[1][1] = m[2][2] = 1.0;
#define C_MX3F_arbrotmat(axis, angle, m)                                       \
  {                                                                            \
    float l = C_Tuple3d_LEN(axis);                                             \
    if (l == (float)0.0) {                                                     \
      C_MX3F_ident(m);                                                         \
    } else {                                                                   \
      Tuple3d u;                                                               \
      C_Tuple3d_SDIV(l, axis, u);                                              \
                                                                               \
      float c = cos(angle);                                                    \
      float t = 1.0 - c;                                                       \
      float s = sin(angle);                                                    \
                                                                               \
      m[0][0] = u[0] * u[0] * t + c;                                           \
      m[1][1] = u[1] * u[1] * t + c;                                           \
      m[2][2] = u[2] * u[2] * t + c;                                           \
                                                                               \
      float txy = u[0] * u[1] * t;                                             \
      float sz = u[2] * s;                                                     \
      float txz = u[0] * u[2] * t;                                             \
      float sy = u[1] * s;                                                     \
      float tyz = u[1] * u[2] * t;                                             \
      float sx = u[0] * s;                                                     \
                                                                               \
      m[0][1] = txy + sz;                                                      \
      m[1][0] = txy - sz;                                                      \
      m[0][2] = txz - sy;                                                      \
      m[2][0] = txz + sy;                                                      \
      m[1][2] = tyz + sx;                                                      \
      m[2][1] = tyz - sx;                                                      \
    }                                                                          \
  }
#define C_MX3F_trans(a, p, q)                                                  \
  q[0] = p[0] * a[0][0] + p[1] * a[1][0] + p[2] * a[2][0];                     \
  q[1] = p[0] * a[0][1] + p[1] * a[1][1] + p[2] * a[2][1];                     \
  q[2] = p[0] * a[0][2] + p[1] * a[1][2] + p[2] * a[2][2];
#define LOGHALF -0.693147f
float C_shading_bias(float value, float bias) {
  float ret = 0;
  if (bias == (float)0.5 || value <= (float)0.0 || value >= (float)1.0) {
    ret = value;
  } else {
    if (bias <= (float)0.0) {
      ret = 0;
    } else {
      ret = pow(value, log(bias) / LOGHALF);
    }
  }
  return ret;
}
float C_shading_gain(float value, float gain) {
  float result = (float)0.0;
  if (gain == (float)0.5) {
    result = value;
  } else {
    if (value < (float)0.5) {
      result = C_shading_bias(2 * value, 1 - gain) / 2;
    } else {
      result = 1 - C_shading_bias(2 - 2 * value, 1 - gain) / 2;
    }
  }
  return result;
}
#define INVERSE_M ((float)4.656612875e-10) /* (1.0/(float)M) */
int RNGnext(int seed_) {
  return 255;
  uint L = 16807 * (seed_ & 0xffff);
  uint H = 16807 * (seed_ >> 16);
  seed_ = ((H & 0x7fff) << 16) + L;
  seed_ -= 0x7fffffff;
  seed_ += H >> 15;
  if (seed_ <= 0)
    seed_ += 0x7fffffff;
  return seed_;
}
#define SMALLVALUE 1.0e-5
__kernel void make_grass_kernel(__constant GrassAttribute *pAttributes,
                                __global Tuple3d *P, __global Tuple3d *N,
                                __global Tuple3d *du, __global Tuple3d *dv,
                                __global Tuple4d *grassData) {
  int nGrass = pAttributes->mNGrass;
  int nCv = pAttributes->mNCv;
  const int i = get_global_id(0);
  uint rnd = RNGnext(i + 1);
  float r = 1.0 + pAttributes->mWidthRandomness * ((float)rnd * INVERSE_M);
  float widthBase = r * pAttributes->mWidthBase;
  float widthTip = r * pAttributes->mWidthTip;
  // compute length
  rnd = RNGnext(rnd);
  float length = ((float)rnd * INVERSE_M);
  length = C_shading_gain(C_shading_bias(length, pAttributes->mLengthBias),
                          pAttributes->mLengthGain);
  length = pAttributes->mLengthMin +
           length * (pAttributes->mLengthMax - pAttributes->mLengthMin);
  rnd = RNGnext(rnd);
  r = 1.0 + pAttributes->mLengthRandomness * ((float)rnd * INVERSE_M);
  length = length * r;
  float dt = 1.0 / (pAttributes->mNCv - 1);
  float t = 0.0;
  int offset = i * nCv;
  for (int j = 0; j < nCv; j++) {
    float s = t * length;
    float x = P[i][0] + s * N[i][0];
    float y = P[i][1] + s * N[i][1];
    float z = P[i][2] + s * N[i][2];
    grassData[offset + j][0] = x;
    grassData[offset + j][1] = y;
    grassData[offset + j][2] = z;
    float biasT = C_shading_bias(t, pAttributes->mWidthBias);
    float width = C_shading_LERP(widthBase, widthTip, biasT);
    grassData[offset + j][3] = width;
    t += dt;
  }
  // check if wind in on
  rnd = RNGnext(rnd);
  bool windOn = false;
  if (pAttributes->mWindOn &&
      pAttributes->mWindPercent >= ((float)rnd * INVERSE_M))
    windOn = true;
  float windTip, windBase;
  if (windOn) {
    rnd = RNGnext(rnd);
    float r = ((float)rnd * INVERSE_M) * pAttributes->mWindRandomness;
    windTip = (1 + r) * pAttributes->mWindTip;
    windBase = (1 + r) * pAttributes->mWindBase;
  }
  // prepare direction
  Tuple3d axis;
  float flatnessBase, flatnessTip;
  if (pAttributes->mDirectionOn) {
    // Compute direction on surface
    Tuple3d dir;
    C_Tuple3d_COMB(pAttributes->mDirection[0], du[i],
                   pAttributes->mDirection[1], dv[i], dir);
    C_Tuple3d_NORMALIZE(dir);
    rnd = RNGnext(rnd);
    dir[0] += pAttributes->mDirectionRandomness * ((float)rnd * INVERSE_M);
    rnd = RNGnext(rnd);
    dir[1] += pAttributes->mDirectionRandomness * ((float)rnd * INVERSE_M);
    rnd = RNGnext(rnd);
    dir[2] += pAttributes->mDirectionRandomness * ((float)rnd * INVERSE_M);
    C_Tuple3d_NORMALIZE(dir);
    // Compute rotation axis
    C_Tuple3d_CROSS(dir, N[i], axis);
    float axisLength = C_Tuple3d_LEN(axis);
    // if (axisLength <= SMALLVALUE)
    //  return;
    C_Tuple3d_SDIV(axisLength, axis, axis);
    // Flatness control
    rnd = RNGnext(rnd);
    float r = ((float)rnd * INVERSE_M) * pAttributes->mFlatnessRandomness;
    flatnessBase = (r + 1.0) * pAttributes->mFlatnessBase;
    flatnessTip = (r + 1.0) * pAttributes->mFlatnessTip;
  }
  Tuple3d prevRefP, refDp, finalP;
  float refDplen;
  C_Tuple3d_COPY(grassData[i * pAttributes->mNCv], prevRefP);
  C_Tuple3d_COPY(prevRefP, finalP);
  t = dt;
  for (int j = 1; j < nCv; j++) {
    int offset = (i * nCv + j);
    C_Tuple3d_SUB(grassData[offset], prevRefP, refDp);
    C_Tuple3d_COPY(grassData[offset], prevRefP);
    if (pAttributes->mPreserveLength) {
      refDplen = C_Tuple3d_LEN(refDp);
    }
    // Flatness
    if (pAttributes->mDirectionOn) {
      float biasT = C_shading_bias(t, pAttributes->mFlatnessBias);
      float flatness = C_shading_LERP(flatnessBase, flatnessTip, biasT);
      // Bending xform
      Matrix3d mx;
      Tuple3d tmp;
      C_MX3F_arbrotmat(axis, -flatness * M_PI / 2, mx);
      C_MX3F_trans(mx, refDp, tmp);
      C_Tuple3d_COPY(tmp, refDp);
    }
    // Add in the transformed offset
    if (pAttributes->mPreserveLength) {
      C_Tuple3d_NORMALIZE(refDp);
      C_Tuple3d_SMUL(refDplen, refDp, refDp);
    }
    // compute wind
    if (windOn) {
      Tuple3d wind;
      float biasT = C_shading_bias(t, pAttributes->mWindBias);
      float windFactor = C_shading_LERP(windBase, windTip, biasT);
      C_Tuple3d_SMUL(windFactor, pAttributes->mWindDirection, wind);
      C_Tuple3d_ADD(refDp, wind, refDp);
      if (pAttributes->mPreserveLength) {
        C_Tuple3d_NORMALIZE(refDp);
        C_Tuple3d_SMUL(refDplen, refDp, refDp);
      }
    }
    C_Tuple3d_ADD(finalP, refDp, grassData[offset]);
    C_Tuple3d_COPY(grassData[offset], finalP);
    // printf("%d, %f, %f, %f", i, finalP[0], finalP[1], finalP[2]);
    t += dt;
  }
}
