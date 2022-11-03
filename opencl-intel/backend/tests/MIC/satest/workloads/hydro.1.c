/*
  A simple 2D hydro code
  (C) Romain Teyssier : CEA/IRFU           -- original F90 code
  (C) Pierre-Francois Lavallee : IDRIS      -- original F90 code
  (C) Guillaume Colin de Verdiere : CEA/DAM -- for the C version
*/
/*
  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software.  You can  use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".
  As a counterpart to the access to the source code and  rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty  and the software's author,  the holder of the
  economic rights,  and the successive licensors  have only  limited
  liability.
  In this respect, the user's attention is drawn to the risks associated
  with loading,  using,  modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean  that it is complicated to manipulate,  and  that  also
  therefore means  that it is reserved for developers  and  experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and,  more generally, to use and operate it in the
  same conditions as regards security.
  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
*/
#if defined(cl_khr_fp64) // Khronos extension available?
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64) // AMD extension available?
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif
// #define AVOID_MANUAL_PREDICATION
#define USE_MIC_PREFETCHES
#ifdef USE_MIC_PREFETCHES
#define PREFETCH(a, b) prefetch(a, b)
#else
#define PREFETCH(a, b)
#endif
#include "oclparam.h"
#define ID (0)
#define IU (1)
#define IV (2)
#define IP (3)
#define ExtraLayer (2)
#define ExtraLayerTot (2 * 2)
inline void idx2d(long *x, long *y, const long nx) {
  long i1d =
      get_global_id(0) +
      (get_global_size(1) - 1) *
          (get_global_id(1) + (get_global_size(2) - 1) * get_global_id(2));
  *y = i1d / nx;
  *x = i1d - *y * nx;
}
inline real_t Square(const real_t x) { return x * x; }
/*
 * Here are defined a couple of placeholders of mathematical functions
 * waiting for AMD to provide them as part of the OpenCL language. The
 * implementation is crude and not efficient. It's only to get a
 * running version of the code in double precision.
 */
#define CPUVERSION 0
// Well AMD hardware has made progress: the following code is useless
// (and kept just in case).
// #ifdef AMDATI
// #define CPUVERSION 1
// #endif
// #ifdef NVIDIA
// #define CPUVERSION 0
// #endif
// #ifdef INTEL
// #define CPUVERSION 0
// #endif
#if CPUVERSION == 0
#define Max fmax
#define Min fmin
#define Fabs fabs
#define Sqrt sqrt
#endif
// #if CPUVERSION == 1
// inline double
// Max(const double a, const double b) {
//   return (a > b) ? a : b;
// }
// inline double
// Min(const double a, const double b) {
//   return (a < b) ? a : b;
// }
// inline double
// Fabs(const double a) {
//   return (a > 0) ? a : -a;
// }
// #endif
// #if CPUVERSION == 1
// inline double
// Sqrt(const double a) {
//   double v = 0;
//   double vn = 0;
//   float x0 = (float) a;
//   double error = (double) 1.e-8;
//   double prec = (double) 1.;
//   // initial value: to speedup the process we take the float approximation
//   x0 = sqrt(x0);
//   vn = (double) x0;
//   prec = Fabs((v - vn) / vn);
//   if (prec > error) {
//     v = (double) 0.5 *(vn + a / vn);
//     prec = Fabs((v - vn) / vn);
//     if (prec > error) {
//       vn = v;
//       v = (double) 0.5 *(vn + a / vn);
//       prec = Fabs((v - vn) / vn);
//       if (prec > error) {
//         vn = v;
//         v = (double) 0.5 *(vn + a / vn);
//         prec = Fabs((v - vn) / vn);
//         if (prec > error) {
//           vn = v;
//           v = (double) 0.5 *(vn + a / vn);
//           prec = Fabs((v - vn) / vn);
//           if (prec > error) {
//             vn = v;
//             v = (double) 0.5 *(vn + a / vn);
//             prec = Fabs((v - vn) / vn);
//           }
//         }
//       }
//     }
//   }
//   return v;
// }
// #endif
/*
 * End math functions
 */
#define one (real_t)1.0
#define two (real_t)2.0
#define demi (real_t)0.5
#define zero (real_t)0.0
// const double one = 1.0;
// const double two = 2.0;
// const double demi = 0.5;
// const double zero = 0.;
// #define IHVW(i,v) ((i) + (v) * Hnxyt)
// #define IHU(i, j, v)  ((i) + Hnxt * ((j) + Hnyt * (v)))
// #define IHV(i, j, v)  ((i) + Hnxt * ((j) + Hnyt * (v)))
inline size_t IHVW(int i, int v, int Hnxyt) { return (i) + (v)*Hnxyt; }
inline size_t IHU(int i, int j, int v, int Hnxt, int Hnyt) {
  return (i) + (Hnxt * ((j) + Hnyt * (v)));
}
inline size_t IHV(const int i, const int j, const int v, const int Hnxt,
                  const int Hnyt) {
  return (i) + (Hnxt * ((j) + Hnyt * (v)));
}
__kernel void Loop1KcuCmpflx(__global real_t *qgdnv, __global real_t *flux,
                             const long narray, const long Hnxyt,
                             const real_t Hgamma, const int slices,
                             const int Hnxystep) {
  real_t entho = 0, ekin = 0, etot = 0;
  int s = get_global_id(1);
  int i = get_global_id(0);
  if (s >= slices)
    return;
  if (i >= narray)
    return;
  size_t idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  PREFETCH(&flux[idxID], 1);
  PREFETCH(&qgdnv[idxID], 1);
  size_t idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  PREFETCH(&flux[idxIU], 1);
  PREFETCH(&qgdnv[idxIU], 1);
  size_t idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  PREFETCH(&flux[idxIP], 1);
  PREFETCH(&qgdnv[idxIP], 1);
  size_t idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  PREFETCH(&flux[idxIV], 1);
  PREFETCH(&qgdnv[idxIV], 1);
  entho = one / (Hgamma - one);
  // Mass density
  flux[idxID] = qgdnv[idxID] * qgdnv[idxIU];
  // Normal momentum
  flux[idxIU] = flux[idxID] * qgdnv[idxIU] + qgdnv[idxIP];
  // Transverse momentum 1
  flux[idxIV] = flux[idxID] * qgdnv[idxIV];
  // Total energy
  ekin = demi * qgdnv[idxID] *
         ((qgdnv[idxIU] * qgdnv[idxIU]) + (qgdnv[idxIV] * qgdnv[idxIV]));
  etot = qgdnv[idxIP] * entho + ekin;
  flux[idxIP] = qgdnv[idxIU] * (etot + qgdnv[idxIP]);
}
__kernel void Loop2KcuCmpflx(__global real_t *qgdnv, __global real_t *flux,
                             const long narray, const long Hnxyt,
                             const long Hnvar, const int slices,
                             const int Hnxystep) {
  long IN, s, i = get_global_id(0);
  idx2d(&i, &s, Hnxyt);
  if (s >= slices)
    return;
  if (i >= narray)
    return;
  for (IN = IP + 1; IN < Hnvar; IN++) {
    size_t idxIN = IHVWS_(i, s, IN, Hnxyt, Hnxystep);
    flux[idxIN] = flux[idxIN] * qgdnv[idxIN];
  }
}
__kernel void LoopKQEforRow(const long j, __global real_t *uold,
                            __global real_t *q, __global real_t *e,
                            const real_t Hsmallr, const long Hnxt,
                            const long Hnyt, const long Hnxyt, const long n,
                            const int slices, const int Hnxystep) {
  real_t eken;
  size_t i, s;
  i = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (i >= n)
    return;
  size_t idxuID = IHV(i + ExtraLayer, j + s, ID, Hnxt, Hnyt);
  size_t idxuIU = IHV(i + ExtraLayer, j + s, IU, Hnxt, Hnyt);
  size_t idxuIV = IHV(i + ExtraLayer, j + s, IV, Hnxt, Hnyt);
  size_t idxuIP = IHV(i + ExtraLayer, j + s, IP, Hnxt, Hnyt);
  size_t idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  size_t idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  size_t idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  size_t idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  size_t is = IHS_(i, s, Hnxyt);
  q[idxID] = Max(uold[idxuID], Hsmallr);
  q[idxIU] = uold[idxuIU] / q[idxID];
  q[idxIV] = uold[idxuIV] / q[idxID];
  eken = demi * (Square(q[idxIU]) + Square(q[idxIV]));
  q[idxIP] = uold[idxuIP] / q[idxID] - eken;
  e[is] = q[idxIP];
}
__kernel void LoopKcourant(__global real_t *q, __global real_t *courant,
                           const real_t Hsmallc, __global const real_t *c,
                           const long Hnxyt, const long n, const int slices,
                           const int Hnxystep) {
  real_t cournox, cournoy, courantl;
  size_t i, s;
  // idx2d(&i, &s, Hnxyt);
  i = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (i >= n)
    return;
  size_t idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  size_t idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  size_t idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  size_t idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  size_t is = IHS_(i, s, Hnxyt);
  cournox = cournoy = 0.;
  cournox = c[is] + Fabs(q[idxIU]);
  cournoy = c[is] + Fabs(q[idxIV]);
  courantl = Max(cournox, Max(cournoy, Hsmallc));
  courant[is] = Max(courant[is], courantl);
}
//////
//  new kernel LoopKComputeDeltat
//  added to merge 3 calls to lightweight kernels LoopKQEforRow, LoopKcourant
//  and LoopEOS into one to reduce kernel calling overhead and enable effective
//  prefetching expected application speedup is 3-5%
//////
__kernel void LoopKComputeDeltat(
    const long j, __global real_t *uold, __global real_t *q, __global real_t *e,
    const long Hnxt, const long Hnyt, const long Hnxyt, const long imax,
    const int slices, const int Hnxystep, const long offsetIP,
    const long offsetID, const real_t Hsmallc, const real_t Hgamma,
    const real_t Hsmallr, __global real_t *c, __global real_t *courant) {
  real_t eken;
  real_t smallp, pis, rhois, eintis;
  real_t cournox, cournoy, courantl;
  size_t i, s;
  i = get_global_id(0);
  s = get_global_id(1);
  if (i >= imax)
    return;
  __global real_t *p = &q[offsetIP];
  __global real_t *rho = &q[offsetID];
  size_t idxuID = IHV(i + ExtraLayer, j + s, ID, Hnxt, Hnyt);
  size_t idxuIU = IHV(i + ExtraLayer, j + s, IU, Hnxt, Hnyt);
  size_t idxuIV = IHV(i + ExtraLayer, j + s, IV, Hnxt, Hnyt);
  size_t idxuIP = IHV(i + ExtraLayer, j + s, IP, Hnxt, Hnyt);
  size_t is = IHS_(i, s, Hnxyt);
  size_t idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  size_t idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  size_t idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  size_t idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  q[idxID] = Max(uold[idxuID], Hsmallr);
  real_t qrec = 1.0 / q[idxID];
  q[idxIU] = uold[idxuIU] * qrec;
  q[idxIV] = uold[idxuIV] * qrec;
  eken = demi * (Square(q[idxIU]) + Square(q[idxIV]));
  q[idxIP] = uold[idxuIP] * qrec - eken;
  e[is] = q[idxIP];
  smallp = Square(Hsmallc) / Hgamma;
  rhois = rho[is];
  eintis = e[is];
  pis = (Hgamma - one) * rhois * eintis;
  pis = Max(pis, (real_t)(rhois * smallp));
  c[is] = Sqrt(Hgamma * pis / rhois);
  p[is] = pis;
  cournox = cournoy = 0.;
  cournox = c[is] + Fabs(q[idxIU]);
  cournoy = c[is] + Fabs(q[idxIV]);
  courantl = Max(cournox, Max(cournoy, Hsmallc));
  courant[is] = Max(courant[is], courantl);
}
__kernel void Loop1KcuGather(__global real_t *uold, __global real_t *u,
                             const long rowcol, const long Hnxt,
                             const long Himin, const long Himax,
                             const long Hnyt, const long Hnxyt,
                             const int slices, const int Hnxystep) {
  size_t i, s;
  i = get_global_id(0);
  if (i < Himin || i >= Himax)
    return;
  for (s = 0; s < slices; s++) {
    int oidID = IHU(i, rowcol + s, ID, Hnxt, Hnyt);
    PREFETCH(&uold[oidID], 1);
    int idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
    PREFETCH(&u[idxID], 1);
    int oidIU = IHU(i, rowcol + s, IU, Hnxt, Hnyt);
    PREFETCH(&uold[oidIU], 1);
    int idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
    PREFETCH(&u[idxIU], 1);
    int oidIV = IHU(i, rowcol + s, IV, Hnxt, Hnyt);
    PREFETCH(&uold[oidIV], 1);
    int idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
    PREFETCH(&u[idxIV], 1);
    int oidIP = IHU(i, rowcol + s, IP, Hnxt, Hnyt);
    PREFETCH(&uold[oidIP], 1);
    int idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
    PREFETCH(&u[idxIP], 1);
    u[idxID] = uold[oidID];
    u[idxIU] = uold[oidIU];
    u[idxIV] = uold[oidIV];
    u[idxIP] = uold[oidIP];
  }
}
__kernel void Loop2KcuGather(__global real_t *uold, __global real_t *u,
                             const long rowcol, const long Hnxt,
                             const long Himin, const long Himax,
                             const long Hnyt, const long Hnxyt,
                             const int slices, const int Hnxystep) {
  size_t s, i = get_global_id(0);
  if (i < Himin || i >= Himax)
    return;
  for (s = 0; s < slices; s++) {
    int oidID = IHU(rowcol + s, i, ID, Hnxt, Hnyt);
    int idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
    int oidIU = IHU(rowcol + s, i, IU, Hnxt, Hnyt);
    int idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
    int oidIV = IHU(rowcol + s, i, IV, Hnxt, Hnyt);
    int idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
    int oidIP = IHU(rowcol + s, i, IP, Hnxt, Hnyt);
    int idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
    u[idxID] = uold[oidID];
    u[idxIV] = uold[oidIU];
    u[idxIU] = uold[oidIV];
    u[idxIP] = uold[oidIP];
  }
}
__kernel void Loop3KcuGather(__global real_t *uold, __global real_t *u,
                             const long rowcol, const long Hnxt,
                             const long Himin, const long Himax,
                             const long Hnyt, const long Hnxyt,
                             const long Hnvar, const int slices,
                             const int Hnxystep) {
  size_t i = get_global_id(0);
  size_t ivar;
  size_t s;
  if (i < Himin)
    return;
  if (i >= Himax)
    return;
  for (ivar = IP + 1; ivar < Hnvar; ivar++) {
    for (s = 0; s < slices; s++) {
      u[IHVWS_(i, s, ivar, Hnxyt, Hnxystep)] =
          uold[IHU(i, rowcol + s, ivar, Hnxt, Hnyt)];
    }
  }
}
__kernel void Loop4KcuGather(__global real_t *uold, __global real_t *u,
                             const long rowcol, const long Hnxt,
                             const long Himin, const long Himax,
                             const long Hnyt, const long Hnxyt,
                             const long Hnvar, const int slices,
                             const int Hnxystep) {
  size_t i = get_global_id(0);
  size_t ivar;
  if (i < Himin)
    return;
  if (i >= Himax)
    return;
  int s;
  for (ivar = IP + 1; ivar < Hnvar; ivar++) {
    for (s = 0; s < slices; s++) {
      u[IHVWS_(i, s, ivar, Hnxyt, Hnxystep)] =
          uold[IHU(rowcol + s, i, ivar, Hnxt, Hnyt)];
    }
  }
}
__kernel void Loop1KcuUpdate(const long rowcol, const real_t dtdx,
                             __global real_t *uold, __global real_t *u,
                             __global real_t *flux, const long Himin,
                             const long Himax, const long Hnxt, const long Hnyt,
                             const long Hnxyt, const int slices,
                             const int Hnxystep) {
  size_t i, s;
  i = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (i < (Himin + ExtraLayer))
    return;
  if (i >= (Himax - ExtraLayer))
    return;
  int idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  int idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  int idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  int idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  int oidID = IHU(i, rowcol + s, ID, Hnxt, Hnyt);
  int oidIU = IHU(i, rowcol + s, IU, Hnxt, Hnyt);
  int oidIV = IHU(i, rowcol + s, IV, Hnxt, Hnyt);
  int oidIP = IHU(i, rowcol + s, IP, Hnxt, Hnyt);
  uold[oidID] = u[idxID] + (flux[idxID - 2] - flux[idxID - 1]) * dtdx;
  uold[oidIU] = u[idxIU] + (flux[idxIU - 2] - flux[idxIU - 1]) * dtdx;
  uold[oidIV] = u[idxIV] + (flux[idxIV - 2] - flux[idxIV - 1]) * dtdx;
  uold[oidIP] = u[idxIP] + (flux[idxIP - 2] - flux[idxIP - 1]) * dtdx;
}
__kernel void Loop2KcuUpdate(const long rowcol, const real_t dtdx,
                             __global real_t *uold, __global real_t *u,
                             __global real_t *flux, const long Himin,
                             const long Himax, const long Hnvar,
                             const long Hnxt, const long Hnyt, const long Hnxyt,
                             const int slices, const int Hnxystep) {
  long ivar;
  long i, s;
  idx2d(&i, &s, Hnxyt);
  if (s >= slices)
    return;
  if (i < (Himin + ExtraLayer))
    return;
  if (i >= (Himax - ExtraLayer))
    return;
  for (ivar = IP + 1; ivar < Hnvar; ivar++) {
    uold[IHU(i, rowcol + s, ivar, Hnxt, Hnyt)] =
        u[IHVWS_(i, s, ivar, Hnxyt, Hnxystep)] +
        (flux[IHVWS_(i - 2, s, ivar, Hnxyt, Hnxystep)] -
         flux[IHVWS_(i - 1, s, ivar, Hnxyt, Hnxystep)]) *
            dtdx;
  }
}
__kernel void Loop3KcuUpdate(const long rowcol, const real_t dtdx,
                             __global real_t *uold, __global real_t *u,
                             __global real_t *flux, const long Hjmin,
                             const long Hjmax, const long Hnxt, const long Hnyt,
                             const long Hnxyt, const int slices,
                             const int Hnxystep) {
  int ivar;
  size_t s, j;
  j = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (j < (Hjmin + ExtraLayer))
    return;
  if (j >= (Hjmax - ExtraLayer))
    return;
  int idxID = IHVWS_(j, s, ID, Hnxyt, Hnxystep);
  int idxIP = IHVWS_(j, s, IP, Hnxyt, Hnxystep);
  int idxIU = IHVWS_(j, s, IU, Hnxyt, Hnxystep);
  int idxIV = IHVWS_(j, s, IV, Hnxyt, Hnxystep);
  int oidID = IHU(rowcol + s, j, ID, Hnxt, Hnyt);
  int oidIP = IHU(rowcol + s, j, IP, Hnxt, Hnyt);
  int oidIV = IHU(rowcol + s, j, IV, Hnxt, Hnyt);
  int oidIU = IHU(rowcol + s, j, IU, Hnxt, Hnyt);
  uold[oidID] = u[idxID] + (flux[idxID - 2] - flux[idxID - 1]) * dtdx;
  uold[oidIP] = u[idxIP] + (flux[idxIP - 2] - flux[idxIP - 1]) * dtdx;
  uold[oidIV] = u[idxIU] + (flux[idxIU - 2] - flux[idxIU - 1]) * dtdx;
  uold[oidIU] = u[idxIV] + (flux[idxIV - 2] - flux[idxIV - 1]) * dtdx;
}
__kernel void Loop4KcuUpdate(const long rowcol, const real_t dtdx,
                             __global real_t *uold, __global real_t *u,
                             __global real_t *flux, const long Hjmin,
                             const long Hjmax, const long Hnvar,
                             const long Hnxt, const long Hnyt, const long Hnxyt,
                             const int slices, const int Hnxystep) {
  long ivar;
  long s, j;
  idx2d(&j, &s, Hnxyt);
  if (s >= slices)
    return;
  if (j < (Hjmin + ExtraLayer))
    return;
  if (j >= (Hjmax - ExtraLayer))
    return;
  for (ivar = IP + 1; ivar < Hnvar; ivar++) {
    uold[IHU(rowcol + s, j, ivar, Hnxt, Hnyt)] =
        u[IHVWS_(j, s, ivar, Hnxyt, Hnxystep)] +
        (flux[IHVWS_(j - 2, s, ivar, Hnxyt, Hnxystep)] -
         flux[IHVWS_(j - 1, s, ivar, Hnxyt, Hnxystep)]) *
            dtdx;
  }
}
__kernel void Loop1KcuConstoprim(const long n, __global real_t *u,
                                 __global real_t *q, __global real_t *e,
                                 const long Hnxyt, const real_t Hsmallr,
                                 const int slices, const int Hnxystep,
                                 const long Hnvar, __global real_t *c,
                                 const long offsetIP, const long offsetID,
                                 const real_t Hsmallc, const real_t Hgamma) {
  size_t i, s;
  i = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (i >= n)
    return;
  long IN;
  __global real_t *p = &q[offsetIP];
  __global real_t *rho = &q[offsetID];
  int idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  int idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  int idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  int idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  int is = IHS_(i, s, Hnxyt);
  real_t smallp = Square(Hsmallc) / Hgamma;
  real_t qID = Max(u[idxID], Hsmallr);
  real_t qrec = 1. / qID;
  q[idxIU] = u[idxIU] * qrec;
  q[idxIV] = u[idxIV] * qrec;
  real_t eken = demi * (Square(q[idxIU]) + Square(q[idxIV]));
  q[idxIP] = u[idxIP] * qrec - eken;
  q[idxID] = qID;
  e[is] = q[idxIP];
  for (IN = IP + 1; IN < Hnvar; IN++) {
    int idxIN = IHVWS_(i, s, IN, Hnxyt, Hnxystep);
    q[idxIN] = u[idxIN] / q[idxIN];
  }
  p[is] = (Hgamma - one) * rho[is] * e[is];
  p[is] = Max(p[is], (real_t)(rho[is] * smallp));
  c[is] = Sqrt(Hgamma * p[is] / rho[is]);
}
__kernel void Loop2KcuConstoprim(const long n, __global real_t *u,
                                 __global real_t *q, const long Hnxyt,
                                 const long Hnvar, const int slices,
                                 const int Hnxystep) {
  int IN;
  size_t i, s, idx = get_global_id(0);
  s = idx / Hnxyt;
  i = idx % Hnxyt;
  if (s >= slices)
    return;
  if (i >= n)
    return;
  for (IN = IP + 1; IN < Hnvar; IN++) {
    size_t idxIN = IHVWS_(i, s, IN, Hnxyt, Hnxystep);
    q[idxIN] = u[idxIN] / q[idxIN];
  }
}
__kernel void LoopEOS(__global real_t *q, __global real_t *eint,
                      __global real_t *c, const long offsetIP,
                      const long offsetID, const long imin, const long imax,
                      const real_t Hsmallc, const real_t Hgamma,
                      const int slices, const int Hnxyt) {
  real_t smallp, pis, rhois, eintis;
  size_t s, k;
  k = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (k < imin)
    return;
  if (k >= imax)
    return;
  int is = IHS_(k, s, Hnxyt);
  __global real_t *p = &q[offsetIP];
  __global real_t *rho = &q[offsetID];
  smallp = Square(Hsmallc) / Hgamma;
  rhois = rho[is];
  eintis = eint[is];
  pis = (Hgamma - one) * rhois * eintis;
  pis = Max(pis, (real_t)(rhois * smallp));
  c[is] = Sqrt(Hgamma * pis / rhois);
  p[is] = pis;
}
__kernel void Loop1KcuMakeBoundary(const int i, const int i0, const real_t sign,
                                   const long Hjmin, const long n,
                                   const long Hnxt, const long Hnyt,
                                   const long Hnvar, __global real_t *uold) {
  long j, ivar;
  j = get_global_id(0);
  ivar = get_global_id(1);
  if (j >= n)
    return;
  if (ivar >= Hnvar)
    return;
  j += (Hjmin + ExtraLayer);
  int newId = IHV(i, j, ivar, Hnxt, Hnyt);
  int oldId = IHV(i0, j, ivar, Hnxt, Hnyt);
  real_t vsign = (ivar == IU) ? -1.0 : sign;
  uold[newId] = uold[oldId] * vsign;
}
__kernel void Loop2KcuMakeBoundary(const int j, const int j0, const real_t sign,
                                   const long Himin, const long n,
                                   const long Hnxt, const long Hnyt,
                                   const long Hnvar, __global real_t *uold) {
  long i, ivar;
  i = get_global_id(0);
  ivar = get_global_id(1);
  if (i >= n)
    return;
  if (ivar >= Hnvar)
    return;
  i += (Himin + ExtraLayer);
  int newId = IHV(i, j, ivar, Hnxt, Hnyt);
  int oldId = IHV(i, j0, ivar, Hnxt, Hnyt);
  real_t vsign = (ivar == IV) ? -1.0 : sign;
  uold[newId] = uold[oldId] * vsign;
}
__kernel void Loop1KcuQleftright(const long bmax, const long Hnvar,
                                 const long Hnxyt, const int slices,
                                 const int Hstep, __global real_t *qxm,
                                 __global real_t *qxp, __global real_t *qleft,
                                 __global real_t *qright) {
  size_t i, s, nvar;
  i = get_global_id(0);
  s = get_global_id(1);
  nvar = get_global_id(2);
  if (s >= slices)
    return;
  if (i >= bmax)
    return;
  int idx_out = IHVWS_(i, s, nvar, Hnxyt, Hstep);
  qleft[idx_out] = qxm[IHVWS_(i + 1, s, nvar, Hnxyt, Hstep)];
  qright[idx_out] = qxp[IHVWS_(i + 2, s, nvar, Hnxyt, Hstep)];
}
__kernel void LoopKcuSlope(__global real_t *q, __global real_t *dq,
                           const long Hnvar, const long Hnxyt,
                           const real_t slope_type, const long ijmin,
                           const long ijmax, const int slices,
                           const int Hnxystep) {
  real_t dlft, drgt, dcen, dsgn, slop, dlim;
  int ihvwin, ihvwimn, ihvwipn;
  size_t i, s, n;
  i = get_global_id(0);
  s = get_global_id(1);
  n = get_global_id(2);
  // changed this to enable early exit catch on MIC
  if (i >= ijmax - ijmin - 2)
    return;
  i += ijmin + 1;
  ihvwin = IHVWS_(i, s, n, Hnxyt, Hnxystep);
  ihvwimn = ihvwin - 1; // IHVWS_(i - 1, s, n, Hnxyt, Hnxystep);
  ihvwipn = ihvwin + 1; // IHVWS_(i + 1, s, n, Hnxyt, Hnxystep);
  dlft = slope_type * (q[ihvwin] - q[ihvwimn]);
  drgt = slope_type * (q[ihvwipn] - q[ihvwin]);
  dcen = demi * (dlft + drgt) / slope_type;
  dsgn = (dcen > 0) ? (real_t)1.0 : (real_t)-1.0; // sign(one, dcen);
  slop = (real_t)Min(Fabs(dlft), Fabs(drgt));
  dlim = ((dlft * drgt) <= zero) ? zero : slop;
  dq[ihvwin] = dsgn * (real_t)Min(dlim, Fabs(dcen));
}
__kernel void Loop1KcuTrace(__global real_t *q, __global real_t *dq,
                            __global real_t *c, __global real_t *qxm,
                            __global real_t *qxp, const real_t dtdx,
                            const long Hnxyt, const long imin, const long imax,
                            const real_t zeror, const real_t zerol,
                            const real_t project, const int slices,
                            const int Hnxystep) {
  size_t i, s;
  i = get_global_id(0);
  s = get_global_id(1);
  if (s >= slices)
    return;
  if (i < imin || i >= imax)
    return;
  int is = IHS_(i, s, Hnxyt);
  int idxID = IHVWS_(i, s, ID, Hnxyt, Hnxystep);
  int idxIP = IHVWS_(i, s, IP, Hnxyt, Hnxystep);
  int idxIU = IHVWS_(i, s, IU, Hnxyt, Hnxystep);
  int idxIV = IHVWS_(i, s, IV, Hnxyt, Hnxystep);
  real_t cc = c[is];
  real_t csq = Square(cc);
  real_t r = q[idxID];
  real_t cc__over_r = cc / r;
  real_t half_r_over_cc = demi * r / cc;
  real_t dp = dq[idxIP];
  real_t dp_r_cc_factor = dp * half_r_over_cc / (r * cc);
  real_t du = dq[idxIU];
  real_t alpham = demi * (du * half_r_over_cc - dp_r_cc_factor);
  real_t alphap = -demi * (dp_r_cc_factor + du * half_r_over_cc);
  real_t alpha0r = demi * (dp / csq - dq[idxID]);
  real_t alpha0v = -demi * dq[idxIV];
  // Right state
  real_t u = q[idxIU];
#ifdef AVOID_MANUAL_PREDICATION
  real_t spminus = (u - cc) * dtdx + one;
  real_t spplus = (u + cc) * dtdx + one;
  real_t spzero = u * dtdx + one;
  if ((u - cc) >= zeror) {
    spminus = project;
  }
  if ((u + cc) >= zeror) {
    spplus = project;
  }
  if (u >= zeror) {
    spzero = project;
  }
#else
  // attempt to reduce masking overhead on MIC
  real_t spminus = ((u - cc) >= zeror) ? project : (u - cc) * dtdx + one;
  real_t spplus = ((u + cc) >= zeror) ? project : (u + cc) * dtdx + one;
  real_t spzero = (u >= zeror) ? project : u * dtdx + one;
#endif
  real_t apright = spplus * alphap;
  real_t amright = spminus * alpham;
  real_t azrright = spzero * alpha0r;
  real_t v = q[idxIV];
  real_t p = q[idxIP];
  qxp[idxID] = r + (apright + amright + azrright);
  qxp[idxIU] = u + (apright - amright) * cc__over_r;
  qxp[idxIV] = v + spzero * alpha0v;
  qxp[idxIP] = p + (apright + amright) * csq;
  // Left state
#ifdef AVOID_MANUAL_PREDICATION
  spminus = (u - cc) * dtdx - one;
  spplus = (u + cc) * dtdx - one;
  spzero = u * dtdx - one;
  if ((u - cc) <= zerol) {
    spminus = -project;
  }
  if ((u + cc) <= zerol) {
    spplus = -project;
  }
  if (u <= zerol) {
    spzero = -project;
  }
#else
  // attempt to reduce masking overhead on MIC
  spminus = ((u - cc) <= zerol) ? -project : (u - cc) * dtdx - one;
  spplus = ((u + cc) <= zerol) ? -project : (u + cc) * dtdx - one;
  spzero = (u <= zerol) ? -project : u * dtdx - one;
#endif
  real_t apleft = spplus * alphap;
  real_t amleft = spminus * alpham;
  real_t azrleft = spzero * alpha0r;
  qxm[idxID] = r + (apleft + amleft + azrleft);
  qxm[idxIU] = u + (apleft - amleft) * cc__over_r;
  qxm[idxIV] = v + spzero * alpha0v;
  qxm[idxIP] = p + (apleft + amleft) * csq;
}
__kernel void Loop2KcuTrace(__global real_t *q, __global real_t *dq,
                            __global real_t *qxm, __global real_t *qxp,
                            const real_t dtdx, const long Hnvar,
                            const long Hnxyt, const long imin, const long imax,
                            const real_t zeror, const real_t zerol,
                            const real_t project) {
  long IN;
  real_t u, a;
  real_t da;
  real_t spzero;
  real_t acmpright;
  real_t acmpleft;
  size_t i = get_global_id(0);
  if (i < imin || i >= imax)
    return;
  int idxIU = IHVW(i, IU, Hnxyt);
  int idxIV = IHVW(i, IV, Hnxyt);
  int idxIP = IHVW(i, IP, Hnxyt);
  int idxID = IHVW(i, ID, Hnxyt);
  for (IN = IP + 1; IN < Hnvar; IN++) {
    int idxIN = IHVW(i, IN, Hnxyt);
    u = q[idxIU];
    a = q[idxIN];
    da = dq[idxIN];
    // Right state
    spzero = u * dtdx + one;
    if (u >= zeror) {
      spzero = project;
    }
    acmpright = -demi * spzero * da;
    qxp[idxIN] = a + acmpright;
    // Left state
    spzero = u * dtdx - one;
    if (u <= zerol) {
      spzero = -project;
    }
    acmpleft = -demi * spzero * da;
    qxm[idxIN] = a + acmpleft;
  }
}
__kernel void KernelMemset(__global real_t *a, real_t v, long lobj) {
  size_t gid = get_global_id(0);
  if (gid >= lobj)
    return;
  real_t dv = v;
  a[gid] = dv;
}
__kernel void KernelMemsetV4(__global int4 *a, int v, long lobj) {
  size_t gid = get_global_id(0) * 4;
  if (gid >= lobj)
    return;
  a[gid] = (int4)v;
}
__kernel void Loop1KcuRiemann(__global real_t *qleft, __global real_t *qright,
                              __global real_t *sgnm, __global real_t *qgdnv,
                              long Hnxyt, long Knarray, real_t Hsmallc,
                              real_t Hgamma, real_t Hsmallr,
                              long Hniter_riemann, const int slices,
                              const int HStep) {
  size_t s, i, j;
  i = get_global_id(0);
  s = get_global_id(1);
  if (i >= Knarray)
    return;
  int idxID = IHVWS_(i, s, ID, Hnxyt, HStep);
  int idxIU = IHVWS_(i, s, IU, Hnxyt, HStep);
  int idxIP = IHVWS_(i, s, IP, Hnxyt, HStep);
  int idxIV = IHVWS_(i, s, IV, Hnxyt, HStep);
  int is = IHS_(i, s, Hnxyt);
  real_t smallp = Square(Hsmallc) / Hgamma;
  real_t gamma6 = (Hgamma + one) / (two * Hgamma);
  real_t Krli = Max(qleft[idxID], Hsmallr);
  real_t Kuli = qleft[idxIU];
  // opencl explose au dela de cette ligne si le code n'est pas en commentaire
  real_t Kpli = Max(qleft[idxIP], (real_t)(Krli * smallp));
  real_t Krri = Max(qright[idxID], Hsmallr);
  real_t Kuri = qright[idxIU];
  real_t Kpri = Max(qright[idxIP], (real_t)(Krri * smallp));
  // Lagrangian sound speed
  real_t Kcli = Hgamma * Kpli * Krli;
  real_t Kcri = Hgamma * Kpri * Krri;
  // First guess
  real_t Kwli = Sqrt(Kcli);
  real_t Kwri = Sqrt(Kcri);
  real_t poldS =
      Max(((Kwri * Kpli + Kwli * Kpri) + Kwli * Kwri * (Kuli - Kuri)) /
              (Kwli + Kwri),
          zero);
  for (int iter = 0; iter < Hniter_riemann; iter++) {
    real_t wwl = Sqrt(Kcli * (one + gamma6 * (poldS - Kpli) / Kpli));
    real_t wwr = Sqrt(Kcri * (one + gamma6 * (poldS - Kpri) / Kpri));
    real_t ql = two * wwl * Square(wwl) / (Square(wwl) + Kcli);
    real_t qr = two * wwr * Square(wwr) / (Square(wwr) + Kcri);
    real_t usl = Kuli - (poldS - Kpli) / wwl;
    real_t usr = Kuri + (poldS - Kpri) / wwr;
    real_t t1 = qr * ql / (qr + ql) * (usl - usr);
    real_t t2 = -poldS;
    real_t delpS = Max(t1, t2);
    poldS = poldS + delpS;
    if (Fabs(delpS / (poldS + Hsmallr * smallp)) <= 1.e-6)
      break;
  }
  Kwli = Sqrt(Kcli + Kcli * gamma6 * (poldS - Kpli) / Kpli);
  Kwri = Sqrt(Kcri + Kcri * gamma6 * (poldS - Kpri) / Kpri);
  real_t Kustari =
      demi * (Kuli + (Kpli - poldS) / Kwli + Kuri - (Kpri - poldS) / Kwri);
  real_t sgnm_is = (Kustari > 0) ? 1 : -1;
  sgnm[is] = sgnm_is;
#ifdef AVOID_MANUAL_PREDICATION
  real_t Kroi;
  real_t Kuoi;
  real_t Kpoi;
  real_t Kwoi;
  if (sgnm_is == 1) {
    Kroi = Krli;
    Kuoi = Kuli;
    Kpoi = Kpli;
    Kwoi = Kwli;
  } else {
    Kroi = Krri;
    Kuoi = Kuri;
    Kpoi = Kpri;
    Kwoi = Kwri;
  }
#else
  real_t Kroi = (sgnm_is == 1) ? Krli : Krri;
  real_t Kuoi = (sgnm_is == 1) ? Kuli : Kuri;
  real_t Kpoi = (sgnm_is == 1) ? Kpli : Kpri;
  real_t Kwoi = (sgnm_is == 1) ? Kwli : Kwri;
#endif
  real_t invKroi = 1.0 / Kroi;
  real_t Kcoi = Max(Hsmallc, Sqrt(Fabs(Hgamma * Kpoi * invKroi)));
  real_t Krstari = Kroi / (one + Kroi * (Kpoi - poldS) / Square(Kwoi));
  Krstari = Max(Krstari, Hsmallr);
  real_t Kcstari = Max(Hsmallc, Sqrt(Fabs(Hgamma * poldS / Krstari)));
  real_t Kushocki = Kwoi * invKroi - sgnm_is * Kuoi;
#ifdef AVOID_MANUAL_PREDICATION
  real_t Kspouti = Kcoi - sgnm[is] * Kuoi;
  real_t Kspini = Kcstari - sgnm[is] * Kustari;
  if (poldS >= Kpoi) {
    Kspini = Kushocki;
    Kspouti = Kushocki;
  }
#else
  real_t Kspini = (poldS >= Kpoi) ? Kushocki : Kcstari - sgnm_is * Kustari;
  real_t Kspouti = (poldS >= Kpoi) ? Kushocki : Kcoi - sgnm_is * Kuoi;
#endif
  real_t Kscri = Max((real_t)(Kspouti - Kspini),
                     (real_t)(Hsmallc + Fabs(Kspouti + Kspini)));
  real_t Kfraci = (one + (Kspouti + Kspini) / Kscri) * demi;
  Kfraci = Max(zero, (real_t)(Min(one, Kfraci)));
#ifdef AVOID_MANUAL_PREDICATION
  qgdnv[idxID] = Kfraci * Krstari - (Kfraci * Kroi - Kroi);
  qgdnv[idxIU] = Kfraci * Kustari - (Kfraci * Kuoi - Kuoi);
  qgdnv[idxIP] = Kfraci * poldS - (Kfraci * Kpoi - Kpoi);
  if (Kspouti < zero) {
    qgdnv[idxID] = Kroi;
    qgdnv[idxIU] = Kuoi;
    qgdnv[idxIP] = Kpoi;
  }
  if (Kspini > zero) {
    qgdnv[idxID] = Krstari;
    qgdnv[idxIU] = Kustari;
    qgdnv[idxIP] = poldS;
  }
  if (sgnm_is == 1) {
    qgdnv[idxIV] = qleft[idxIV];
  } else {
    qgdnv[idxIV] = qright[idxIV];
  }
#else
  real_t qgdnv_idxID =
      (Kspouti < zero) ? Kroi : Kfraci * Krstari - (Kfraci * Kroi - Kroi);
  real_t qgdnv_idxIU =
      (Kspouti < zero) ? Kuoi : Kfraci * Kustari - (Kfraci * Kuoi - Kuoi);
  real_t qgdnv_idxIP =
      (Kspouti < zero) ? Kpoi : Kfraci * poldS - (Kfraci * Kpoi - Kpoi);
  qgdnv_idxID = (Kspini > zero) ? Krstari : qgdnv_idxID;
  qgdnv_idxIU = (Kspini > zero) ? Kustari : qgdnv_idxIU;
  qgdnv_idxIP = (Kspini > zero) ? poldS : qgdnv_idxIP;
  qgdnv[idxID] = qgdnv_idxID;
  qgdnv[idxIU] = qgdnv_idxIU;
  qgdnv[idxIP] = qgdnv_idxIP;
  qgdnv[idxIV] = (sgnm_is == 1) ? qleft[idxIV] : qright[idxIV];
#endif
}
__kernel void Loop10KcuRiemann(__global real_t *qleft, __global real_t *qright,
                               __global real_t *sgnm, __global real_t *qgdnv,
                               long Knarray, long Knvar, long KHnxyt,
                               const int slices, const int Hstep) {
  long invar;
  long s, i = get_global_id(0);
  long Hnxyt = KHnxyt;
  idx2d(&i, &s, Hnxyt);
  if (s >= slices)
    return;
  if (i >= Knarray)
    return;
  size_t is = IHS_(i, s, Hnxyt);
  for (invar = IP + 1; invar < Knvar; invar++) {
    size_t idxIN = IHVWS_(i, s, invar, Hnxyt, Hstep);
    if (sgnm[is] == 1) {
      qgdnv[idxIN] = qleft[idxIN];
    } else {
      qgdnv[idxIN] = qright[idxIN];
    }
  }
}
// kernel to pack/unpack arrays used in MPI exchanges
// #define IHv(i,j,v) ((i) + (Hnxt * (Hnyt * (v) + (j))))
#define IHv2v(i, j, v) ((i) + (ExtraLayer * (Hnyt * (v) + (j))))
#define IHv2h(i, j, v) ((i) + (Hnxt * (ExtraLayer * (v) + (j))))
__kernel void kpack_arrayv(const int xmin, const long Hnxt, const long Hnyt,
                           const long Hnvar, __global real_t *buffer,
                           __global real_t *uold) {
  int ivar, i;
  int j = get_global_id(0);
  if (j >= Hnyt)
    return;
  for (ivar = 0; ivar < Hnvar; ivar++) {
    for (i = xmin; i < xmin + ExtraLayer; i++) {
      buffer[IHv2v(i - xmin, j, ivar)] = uold[IHV(i, j, ivar, Hnxt, Hnyt)];
    }
  }
}
__kernel void kunpack_arrayv(const int xmin, const long Hnxt, const long Hnyt,
                             const long Hnvar, __global real_t *buffer,
                             __global real_t *uold) {
  int ivar, i;
  int j = get_global_id(0);
  if (j >= Hnyt)
    return;
  for (ivar = 0; ivar < Hnvar; ivar++) {
    for (i = xmin; i < xmin + ExtraLayer; i++) {
      uold[IHV(i, j, ivar, Hnxt, Hnyt)] = buffer[IHv2v(i - xmin, j, ivar)];
    }
  }
}
__kernel void kpack_arrayh(const int ymin, const long Hnxt, const long Hnyt,
                           const long Hnvar, __global real_t *buffer,
                           __global real_t *uold) {
  int ivar, j;
  int i = get_global_id(0);
  if (i >= Hnxt)
    return;
  for (ivar = 0; ivar < Hnvar; ivar++) {
    for (j = ymin; j < ymin + ExtraLayer; j++) {
      buffer[IHv2h(i, j - ymin, ivar)] = uold[IHV(i, j, ivar, Hnxt, Hnyt)];
    }
  }
}
__kernel void kunpack_arrayh(const int ymin, const long Hnxt, const long Hnyt,
                             const long Hnvar, __global real_t *buffer,
                             __global real_t *uold) {
  int ivar, j;
  int i = get_global_id(0);
  if (i >= Hnxt)
    return;
  for (ivar = 0; ivar < Hnvar; ivar++) {
    for (j = ymin; j < ymin + ExtraLayer; j++) {
      uold[IHV(i, j, ivar, Hnxt, Hnyt)] = buffer[IHv2h(i, j - ymin, ivar)];
    }
  }
}
#define REDUCALGO 1
#if REDUCALGO == 2
__kernel void reduceMaxReal(__global real_t *buffer, __const long length,
                            __global real_t *result, __local real_t *scratch) {
  int global_index = get_global_id(0);
  int i;
  real_t lmaxCourant;
  if (global_index == 0) {
    lmaxCourant = 0.;
    for (i = 0; i < length; i++) {
      lmaxCourant = fmax(lmaxCourant, buffer[i]);
    }
    result[get_group_id(0)] = lmaxCourant;
  }
}
#endif
#if REDUCALGO == 1
__kernel void reduceMaxReal(__global real_t *buffer, __const long length,
                            __global real_t *result, __local real_t *scratch) {
  int global_index = get_global_id(0);
  int local_index = get_local_id(0);
  real_t accumulator = -DBL_MAX;
  // Pass 1
  // Loop sequentially over chunks of input vector
  while (global_index < length) {
    real_t element = buffer[global_index];
    accumulator = fmax(accumulator, element);
    global_index += get_local_size(0); // to favor coalescing
  }
  // Pass 2
  // Perform parallel reduction
  scratch[local_index] = accumulator;
  barrier(CLK_LOCAL_MEM_FENCE);
  for (int offset = get_local_size(0) / 2; offset > 0; offset = offset / 2) {
    if (local_index < offset) {
      real_t other = scratch[local_index + offset];
      real_t mine = scratch[local_index];
      scratch[local_index] = fmax(mine, other);
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    result[get_group_id(0)] = scratch[0];
  }
}
#endif
// EOF
#ifdef NOTDEF
#endif // NOTDEF
