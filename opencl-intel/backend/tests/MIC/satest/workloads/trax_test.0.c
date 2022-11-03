#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void kernel_multiple(__global const double *tracksArr,
                              __global const double *radLens,
                              __global double *covMatrix) {
  const unsigned int gID = get_global_id(0);
  const unsigned int gSize = get_global_size(0);
  const unsigned int pos_x = gID;
  const unsigned int pos_y = gID + gSize;
  const unsigned int pos_z = gID + 2 * gSize;
  double d_x = tracksArr[pos_x];
  double d_y = tracksArr[pos_y];
  double d_z = tracksArr[pos_z];
  double radiationLength = radLens[pos_x];
  double d_x2 = d_x * d_x;
  double d_y2 = d_y * d_y;
  double d_z2 = d_z * d_z;
  double p2 = d_x2 + d_y2 + d_z2;
  double one_over_p = 1.0 / sqrt(p2);
  d_x *= one_over_p;
  d_y *= one_over_p;
  d_z *= one_over_p;
  double mass = 105.;
  const double amscon = 1.8496e-4;
  const double m2 = mass * mass;
  double e2 = p2 + m2;
  double beta2 = p2 / e2;
  double eff_radLen = radiationLength / fabs(d_z);
  double fact = 1. + 0.038 * log(eff_radLen);
  fact *= fact;
  double a = fact / (beta2 * p2);
  double sigt2 = amscon * eff_radLen * a;
  double perp2 = d_x2 + d_y2;
  double sl2 = perp2;
  double cl2 = d_z2;
  double cf2 = d_x2 / sl2;
  double sf2 = d_y2 / sl2;
  double den_times_sigt2 = sigt2 / (cl2 * cl2);
  double theDeltaCov_1_1 = den_times_sigt2 * (sf2 * cl2 + cf2);
  double theDeltaCov_1_2 = den_times_sigt2 * (d_x * d_y);
  double theDeltaCov_2_2 = den_times_sigt2 * (cf2 * cl2 + sf2);
  covMatrix[pos_x] = theDeltaCov_1_1;
  covMatrix[pos_y] = theDeltaCov_1_2;
  covMatrix[pos_z] = theDeltaCov_2_2;
}
