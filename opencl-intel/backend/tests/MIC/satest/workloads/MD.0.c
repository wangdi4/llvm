#ifdef SINGLE_PRECISION
#define POSVECTYPE float4
#define FORCEVECTYPE float4
#define FPTYPE float
#elif K_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define POSVECTYPE double4
#define FORCEVECTYPE double4
#define FPTYPE double
#elif AMD_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define POSVECTYPE double4
#define FORCEVECTYPE double4
#define FPTYPE double
#endif
__kernel void compute_lj_force(__global FORCEVECTYPE *force,
                               __global POSVECTYPE *position,
                               const int neighCount, __global int *neighList,
                               const FPTYPE cutsq, const FPTYPE lj1,
                               const FPTYPE lj2, const int inum) {
  int idx = get_global_id(0);
  POSVECTYPE ipos = position[idx];
  FPTYPE fx = 0.0f;
  FPTYPE fy = 0.0f;
  FPTYPE fz = 0.0f;
  int j = 0;
  while (j < neighCount) {
    if ((j + 1) < neighCount) {
      int jidx_next = neighList[(j + 1) * inum + idx];
      prefetch(((__global FPTYPE *)position) + jidx_next * 4, 1);
    }
    int jidx = neighList[j * inum + idx];
    // Uncoalesced read
    POSVECTYPE jpos = position[jidx];
    // Calculate distance
    FPTYPE delx = ipos.x - jpos.x;
    FPTYPE dely = ipos.y - jpos.y;
    FPTYPE delz = ipos.z - jpos.z;
    FPTYPE r2inv = delx * delx + dely * dely + delz * delz;
    // If distance is less than cutoff, calculate force
    int mask = (r2inv < cutsq);
    r2inv = 1.0f / r2inv;
    FPTYPE r6inv = r2inv * r2inv * r2inv;
    FPTYPE forceC = r2inv * r6inv * (lj1 * r6inv - lj2);
    FPTYPE lfx = fx + (delx * forceC);
    FPTYPE lfy = fy + (dely * forceC);
    FPTYPE lfz = fz + (delz * forceC);
    fx = (mask) ? lfx : fx;
    fy = (mask) ? lfy : fy;
    fz = (mask) ? lfz : fz;
    /* should be replaced with the manual predication above after improving the
       WI analysis if (r2inv < cutsq)
            {
                r2inv = 1.0f/r2inv;
                FPTYPE r6inv = r2inv * r2inv * r2inv;
                FPTYPE forceC = r2inv*r6inv*(lj1*r6inv - lj2);
                fx += delx * forceC;
                fy += dely * forceC;
                fz += delz * forceC;
            }
    */
    j++;
  }
  // store the results
  force[idx].x = fx;
  force[idx].y = fy;
  force[idx].z = fz;
}
