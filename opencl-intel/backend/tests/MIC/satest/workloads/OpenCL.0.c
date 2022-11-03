#define SEED 7777777
// #define __DO_FLOAT__
#define PIPE_SIZE 4
#ifdef __DO_FLOAT__
typedef float tfloat;
#else
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef double tfloat;
#endif
#define NBONDS 2000 /* Number of convertible bonds to price */
#define _N 100      /* number of decision-making dates */
#define _M 5        /* number of coupon payments dates (per year) */
#define _MGRID 50   /* grid dimension within coupon payments dates */
#define _JGRID 200  /* grid dimension underlying asset prices */
#define EPSILON 10e-16
// ERROR DEFINITIONS
#define CB_MSCO_OK 0
#define CB_MSCO_MALLOC_FAILURE -1
#define CB_MSCO_NULL_POINTER -2
#define CB_MSCO_IRREG_INPUT -3
#define CB_MSCO_MKL_ERR -4
#ifdef __DO_FLOAT__
#define _SIG2 0.09f    /* volatility of underlying asset */
#define _D 0.01f       /* continious dividend rate */
#define _RF 0.0253f    /* risk-free interest rate */
#define _MINV 0.01f    /* lowest underlying asset price */
#define _MAXV 10000.0f /* highest underlying asset price */
#define _T_LO 1.0f     /* maturity of convertible bond in years */
#define _T_HI 5.0f     /* maturity of convertible bond in years */
#define _K_LO 50.0f    /* par value of CB */
#define _K_HI 150.0f   /* par value of CB */
#define _IC_LO 3.0f    /* conversion price */
#define _IC_HI 10.0f   /* conversion price */
#define _RK_LO 0.01f   /* coupon payments percent */
#define _RK_HI 0.02f   /* coupon payments percent */
#define EXP exp
#define LOG log
#define VEXP vsExp
#define FABS fabs
#define ZERO 0.0f
#define HALF 0.5f
#define ONE 1.0f
#define TWO 2.0f
#define FOUR 4.0f
#define HUGE 1.0e10f
#else
#define _SIG2 0.09    /* volatility of underlying asset */
#define _D 0.01       /* continious dividend rate */
#define _RF 0.0253    /* risk-free interest rate */
#define _MINV 0.01    /* lowest underlying asset price */
#define _MAXV 10000.0 /* highest underlying asset price */
#define _T_LO 1.0     /* maturity of convertible bond in years */
#define _T_HI 5.0     /* maturity of convertible bond in years */
#define _K_LO 50.0    /* par value of CB */
#define _K_HI 150.0   /* par value of CB */
#define _IC_LO 3.0    /* conversion price */
#define _IC_HI 10.0   /* conversion price */
#define _RK_LO 0.01   /* coupon payments percent */
#define _RK_HI 0.02   /* coupon payments percent */
#define EXP exp
#define LOG log
#define VEXP vdExp
#define FABS fabs
#define ZERO 0.0
#define HALF 0.5
#define ONE 1.0
#define TWO 2.0
#define FOUR 4.0
#define HUGE 1.0e10
#endif
// Structures for initialization of each bond compute
typedef struct {
  tfloat t;  // life-time of convertible bond in years
  tfloat k;  // par value of CB
  tfloat Ic; // conversion price
  tfloat rk; // coupon payments percent
} cb_params_struct;
typedef struct {
  tfloat *r;
  tfloat *cb_next;
  tfloat *buff_n;
  tfloat *buff_j_1;
  tfloat *buff_j_2;
} cb_buffs_struct;
typedef struct {
  tfloat min_z, max_z;
  tfloat dz;
  tfloat *cb;   //[_J + 1];
  tfloat *Vopt; //[_n];
} cb_result_struct;
// #define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define MAX(a, b) max(a, b) // OpenCL specific
void VEXP(int n, tfloat a[], tfloat r[]) {
  int i;
  for (i = 0; i < n; i++) {
    r[i] = EXP(a[i]);
  }
}
void evalBoundaryCondTop(tfloat K, tfloat Ic, tfloat _maxz, tfloat D, tfloat dT,
                         tfloat dt, int m, tfloat *res) {
  *res = (K / Ic) * EXP(_maxz - D * (dT - ((tfloat)m) * dt));
}
void evalBoundaryCondBottom(tfloat K, tfloat rf, tfloat dT, tfloat dt,
                            tfloat *r, int n, int k, int m, tfloat *res,
                            tfloat *nexp) {
  tfloat sum = 0;
  tfloat rf_dT;
  int l;
  rf_dT = -rf * dT;
  for (l = 0; l <= n - k - 1; l++) {
    nexp[l] = rf_dT * (tfloat)l;
  }
  VEXP(n - k, nexp, nexp);
  for (l = 0; l <= n - k - 1; l++) {
    sum += r[k + l + 1] * nexp[l];
  }
  *res = K * EXP(-rf * (dT - (tfloat)m * dt)) *
         (sum + EXP(-rf * ((tfloat)n - (tfloat)k - 1) * dT));
}
void evalTermCondLastStep(tfloat *CB, tfloat K, tfloat Ic, tfloat rn,
                          tfloat minz, tfloat dz, int J, tfloat *jexp) {
  int j;
  for (j = 0; j <= J; j++) {
    jexp[j] = minz + (tfloat)j * dz;
  }
  VEXP(J + 1, jexp, jexp);
  for (j = 0; j <= J; j++) {
    CB[j] = MAX(K * (ONE + rn), (K / Ic) * jexp[j]);
  }
}
void evalTermCond(tfloat *CB, tfloat *CB_previous, tfloat K, tfloat Ic,
                  tfloat rk, tfloat minz, tfloat dz, int J, tfloat *jexp) {
  int j;
  for (j = 0; j <= J; j++) {
    jexp[j] = minz + ((tfloat)j) * dz;
  }
  VEXP(J + 1, jexp, jexp);
  for (j = 0; j <= J; j++) {
    CB[j] = MAX(K * rk + CB_previous[j], (K / Ic) * jexp[j]);
  }
}
void tridiag_mvMultiply(int n, tfloat *B, tfloat *c, tfloat *res) {
  int i;
  for (i = 0; i < n - 2; i++) {
    res[i] = B[0] * c[i] + B[1] * c[i + 1] + B[2] * c[i + 2];
  }
}
void Sweep(int n, tfloat *v, tfloat *b, tfloat *alpha, tfloat *beta,
           tfloat *x) {
  int i;
  alpha[1] = -v[2] / v[1];
  beta[1] = b[0] / v[1];
  for (i = 1; i < n - 1; i++) {
    alpha[i + 1] = -v[2] / (v[1] + v[0] * alpha[i]);
    beta[i + 1] = (-v[0] * beta[i] + b[i]) / (v[1] + v[0] * alpha[i]);
  }
  x[n - 1] = (-v[0] * beta[n - 1] + b[n - 1]) / (v[1] + v[0] * alpha[n - 1]);
  for (i = n - 2; i >= 0; i--) {
    x[i] = alpha[i + 1] * x[i + 1] + beta[i + 1];
  }
}
void evalOptimalConversionPrice(tfloat *CB, tfloat K, tfloat Ic, tfloat rk,
                                tfloat minz, tfloat dz, int J, tfloat *res,
                                tfloat *buff) {
  int j, __j;
  tfloat minv = HUGE, temp;
  for (j = 0; j < J; j++) {
    buff[j] = minz + ((tfloat)j) * dz;
  }
  VEXP(J, buff, buff);
  for (j = 0; j < J; j++) {
    temp = FABS(K * rk + CB[j] - (K / Ic) * buff[j]);
    if (temp < minv) {
      minv = temp;
      __j = j;
    }
  }
  *res = buff[__j];
}
//  __attribute__((vec_type_hint(double4)))
__kernel void evalConvertibleBond(__global cb_params_struct *iparams,
                                  __global tfloat *omin_z,
                                  __global tfloat *omax_z, __global tfloat *odz,
                                  __global tfloat *ocb,
                                  __global tfloat *oVopt) {
  int i, j, vi;
  int k, p;
  tfloat dt, dz, dT;
  tfloat dz2;
  tfloat fabs_reslt;
  tfloat min_z, max_z;
  tfloat a[3], b[3]; // vectors set up tridiagonal matrixes Ab and B
  tfloat c;
  tfloat tmp_buff[(_N + 1) * 2 + (_JGRID + 1) * 3];
  tfloat tmp_rslt[_N + (_JGRID + 1)];
  cb_buffs_struct vbuffs;
  cb_result_struct vres;
  cb_buffs_struct *buffs = &vbuffs;
  cb_result_struct *res = &vres;
  int id = get_global_id(0);
  __global cb_params_struct *params = &iparams[id];
  buffs->r = tmp_buff;
  buffs->cb_next = tmp_buff + (_N + 1);
  buffs->buff_n = tmp_buff + (_N + 1) + (_JGRID + 1);
  buffs->buff_j_1 = tmp_buff + (_N + 1) * 2 + (_JGRID + 1);
  buffs->buff_j_2 = tmp_buff + (_N + 1) * 2 + (_JGRID + 1) * 2;
  res->cb = tmp_rslt;
  res->Vopt = tmp_rslt + (_JGRID + 1);
  dT = params->t / (tfloat)_N;
  dt = dT / (tfloat)_MGRID;
  // Mapping assets prices domain by formula: V' = ln(V)
  min_z = LOG(_MINV);
  max_z = LOG(_MAXV);
  // Estimating coupon payments dates
  for (i = 0; i <= _N; i++)
    buffs->r[i] = 0;
  for (i = 0; i <= _N; i++) {
    for (j = 1; j <= _M; j++) {
      fabs_reslt = FABS((tfloat)i * dT - (tfloat)j);
      if (fabs_reslt < EPSILON)
        buffs->r[i] = params->rk;
    }
  }
  dz = (max_z - min_z) / (tfloat)_JGRID;
  dz2 = dz * dz;
  a[0] =
      _SIG2 * dt / (FOUR * dz2) - (_RF - _D - HALF * _SIG2) * dt / (FOUR * dz);
  a[1] = -_SIG2 * dt / (TWO * dz2) - ONE - _RF * dt * HALF;
  a[2] =
      _SIG2 * dt / (FOUR * dz2) + (_RF - _D - HALF * _SIG2) * dt / (FOUR * dz);
  b[0] =
      -_SIG2 * dt / (FOUR * dz2) + (_RF - _D - HALF * _SIG2) * dt / (FOUR * dz);
  b[1] = _SIG2 * dt / (TWO * dz2) - ONE + _RF * dt * HALF;
  b[2] =
      -_SIG2 * dt / (FOUR * dz2) - (_RF - _D - HALF * _SIG2) * dt / (FOUR * dz);
  evalTermCondLastStep(res->cb, params->k, params->Ic, buffs->r[_N], min_z, dz,
                       _JGRID, buffs->buff_j_1);
  for (k = _N - 1; k >= 0; k--) {
    for (p = _MGRID; p >= 0; p--) {
      tridiag_mvMultiply(_JGRID + 1, b, res->cb, buffs->cb_next);
      evalBoundaryCondBottom(params->k, _RF, dT, dt, buffs->r, _N, k, p, &c,
                             buffs->buff_n);
      buffs->cb_next[0] -= a[0] * c;
      evalBoundaryCondTop(params->k, params->Ic, max_z, _D, dT, dt, p, &c);
      buffs->cb_next[_JGRID - 2] -= a[2] * c;
      Sweep(_JGRID - 1, a, buffs->cb_next, buffs->buff_j_1, buffs->buff_j_2,
            res->cb + 1);
    }
    evalOptimalConversionPrice(res->cb, params->k, params->Ic, buffs->r[k],
                               min_z, dz, _JGRID, &c, buffs->buff_j_1);
    res->Vopt[k] = c;
    evalTermCond(res->cb, res->cb, params->k, params->Ic, buffs->r[k], min_z,
                 dz, _JGRID, buffs->buff_j_1);
  }
  for (i = 0; i < (_JGRID + 1); i++) {
    ocb[id * (_JGRID + 1) + i] = res->cb[i];
  }
  for (i = 0; i < _N; i++) {
    oVopt[id * _N + i] = res->Vopt[i];
  }
  *omin_z = (tfloat)min_z;
  *omax_z = (tfloat)max_z;
  *odz = (tfloat)dz;
}
