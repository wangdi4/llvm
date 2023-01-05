//  nBodyVecKernel - evaluates NBody algorithm over overall
// body_count=body_count_per_item*global size bodies   Each item evaluates
// velocity and position after time_delta units of time for its
// szElementsPerItem bodies within the input   input_position_x - input array
// with current x axis position of the element   input_position_y - input array
// with current y axis position of the element   input_position_z - input array
// with current z axis position of the element   mass - mass of the element
// input_velocity_x - input array with current x axis velocity of the element
// input_velocity_y - input array with current y axis velocity of the element
// input_velocity_z - input array with current z axis velocity of the element
// output_position_x - output array with current x axis position of the element
// output_position_y - output array with current y axis position of the element
// output_position_z - output array with current z axis position of the element
// output_velocity_x - output array with current x axis velocity of the element
// output_velocity_y - output array with current y axis velocity of the element
// output_velocity_z - output array with current z axis velocity of the element
// softening_squared - represents epsilon noise added to the distance evaluation
// between each pair of elements
__kernel void nBodyVecKernel(
    const __global float4 *input_position_x,
    const __global float4 *input_position_y,
    const __global float4 *input_position_z, const __global float4 *mass,
    const __global float *input_velocity_x,
    const __global float *input_velocity_y,
    const __global float *input_velocity_z, __global float *output_position_x,
    __global float *output_position_y, __global float *output_position_z,
    __global float *output_velocity_x, __global float *output_velocity_y,
    __global float *output_velocity_z, int body_count, int body_count_per_item,
    float softening_squared, float time_delta) {
  int index = get_global_id(0);
  const __global float *in_position_x =
      (const __global float *)input_position_x;
  const __global float *in_position_y =
      (const __global float *)input_position_y;
  const __global float *in_position_z =
      (const __global float *)input_position_z;
  float4 position_x, position_y, position_z, m;
  float4 current_x1, current_y1, current_z1, current_mass1;
  float4 current_x2, current_y2, current_z2, current_mass2;
  float4 velocity_x, velocity_y, velocity_z;
  float4 zero = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  int i, j, k, l;
  int inner_loop_count = ((body_count >> 2) / 3) * 3;
  int outer_loop_count = body_count_per_item;
  int start = index * outer_loop_count;
  int finish = start + outer_loop_count;
  for (k = start; k < finish; k++) {
    position_x = (float4)(in_position_x[k]);
    position_y = (float4)(in_position_y[k]);
    position_z = (float4)(in_position_z[k]);
    float4 acceleration_x1 = zero;
    float4 acceleration_x2 = zero;
    float4 acceleration_x3 = zero;
    float4 acceleration_y1 = zero;
    float4 acceleration_y2 = zero;
    float4 acceleration_y3 = zero;
    float4 acceleration_z1 = zero;
    float4 acceleration_z2 = zero;
    float4 acceleration_z3 = zero;
    for (i = 0; i < inner_loop_count; i += 3) {
      float4 dx1 = input_position_x[i] - position_x;
      float4 dx2 = input_position_x[i + 1] - position_x;
      float4 dx3 = input_position_x[i + 2] - position_x;
      float4 dy1 = input_position_y[i] - position_y;
      float4 dy2 = input_position_y[i + 1] - position_y;
      float4 dy3 = input_position_y[i + 2] - position_y;
      float4 dz1 = input_position_z[i] - position_z;
      float4 dz2 = input_position_z[i + 1] - position_z;
      float4 dz3 = input_position_z[i + 2] - position_z;
      float4 distance_squared1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;
      float4 distance_squared2 = dx2 * dx2 + dy2 * dy2 + dz2 * dz2;
      float4 distance_squared3 = dx3 * dx3 + dy3 * dy3 + dz3 * dz3;
      distance_squared1 += softening_squared;
      distance_squared2 += softening_squared;
      distance_squared3 += softening_squared;
      float4 inverse_distance1 = rsqrt(distance_squared1);
      float4 inverse_distance2 = rsqrt(distance_squared2);
      float4 inverse_distance3 = rsqrt(distance_squared3);
      float4 mi1 = mass[i];
      float4 mi2 = mass[i + 1];
      float4 mi3 = mass[i + 2];
      float4 s1 =
          (mi1 * inverse_distance1) * (inverse_distance1 * inverse_distance1);
      float4 s2 =
          (mi2 * inverse_distance2) * (inverse_distance2 * inverse_distance2);
      float4 s3 =
          (mi3 * inverse_distance3) * (inverse_distance3 * inverse_distance3);
      acceleration_x1 += dx1 * s1;
      acceleration_x2 += dx2 * s2;
      acceleration_x3 += dx3 * s3;
      acceleration_y1 += dy1 * s1;
      acceleration_y2 += dy2 * s2;
      acceleration_y3 += dy3 * s3;
      acceleration_z1 += dz1 * s1;
      acceleration_z2 += dz2 * s2;
      acceleration_z3 += dz3 * s3;
    }
    for (; i < (body_count >> 2); i++) {
      float4 dx1 = input_position_x[i] - position_x;
      float4 dy1 = input_position_y[i] - position_y;
      float4 dz1 = input_position_z[i] - position_z;
      float4 distance_squared1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;
      distance_squared1 += softening_squared;
      // float4 inverse_distance1 = __native_rsqrtf4(distance_squared1);
      float4 inverse_distance1 = rsqrt(distance_squared1);
      float4 mi1 = mass[i];
      float4 s1 =
          (mi1 * inverse_distance1) * (inverse_distance1 * inverse_distance1);
      acceleration_x1 += dx1 * s1;
      acceleration_y1 += dy1 * s1;
      acceleration_z1 += dz1 * s1;
    }
    acceleration_x1 = acceleration_x1 + acceleration_x2 + acceleration_x3;
    acceleration_y1 = acceleration_y1 + acceleration_y2 + acceleration_y3;
    acceleration_z1 = acceleration_z1 + acceleration_z2 + acceleration_z3;
    float acc_x = acceleration_x1.x + acceleration_x1.y + acceleration_x1.z +
                  acceleration_x1.w;
    float acc_y = acceleration_y1.x + acceleration_y1.y + acceleration_y1.z +
                  acceleration_y1.w;
    float acc_z = acceleration_z1.x + acceleration_z1.y + acceleration_z1.z +
                  acceleration_z1.w;
    output_velocity_x[k] = input_velocity_x[k] + acc_x * time_delta;
    output_velocity_y[k] = input_velocity_y[k] + acc_y * time_delta;
    output_velocity_z[k] = input_velocity_z[k] + acc_z * time_delta;
    output_position_x[k] = in_position_x[k] + input_velocity_x[k] * time_delta +
                           acc_x * time_delta * time_delta / 2;
    output_position_y[k] = in_position_y[k] + input_velocity_y[k] * time_delta +
                           acc_y * time_delta * time_delta / 2;
    output_position_z[k] = in_position_z[k] + input_velocity_z[k] * time_delta +
                           acc_z * time_delta * time_delta / 2;
  }
}
__kernel void nBodyVec8Kernel(
    const __global float8 *input_position_x,
    const __global float8 *input_position_y,
    const __global float8 *input_position_z, const __global float8 *mass,
    const __global float *input_velocity_x,
    const __global float *input_velocity_y,
    const __global float *input_velocity_z, __global float *output_position_x,
    __global float *output_position_y, __global float *output_position_z,
    __global float *output_velocity_x, __global float *output_velocity_y,
    __global float *output_velocity_z, int body_count, int body_count_per_item,
    float softening_squared, float time_delta) {
  int index = get_global_id(0);
  const __global float *in_position_x =
      (const __global float *)input_position_x;
  const __global float *in_position_y =
      (const __global float *)input_position_y;
  const __global float *in_position_z =
      (const __global float *)input_position_z;
  float8 position_x, position_y, position_z, m;
  float8 current_x1, current_y1, current_z1, current_mass1;
  float8 current_x2, current_y2, current_z2, current_mass2;
  float8 velocity_x, velocity_y, velocity_z;
  float8 zero = (float8)(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i, j, k, l;
  int inner_loop_count = ((body_count >> 3) / 3) * 3;
  int outer_loop_count = body_count_per_item;
  int start = index * outer_loop_count;
  int finish = start + outer_loop_count;
  for (k = start; k < finish; k++) {
    position_x = (float8)(in_position_x[k]);
    position_y = (float8)(in_position_y[k]);
    position_z = (float8)(in_position_z[k]);
    float8 acceleration_x1 = zero;
    float8 acceleration_x2 = zero;
    float8 acceleration_x3 = zero;
    float8 acceleration_y1 = zero;
    float8 acceleration_y2 = zero;
    float8 acceleration_y3 = zero;
    float8 acceleration_z1 = zero;
    float8 acceleration_z2 = zero;
    float8 acceleration_z3 = zero;
    for (i = 0; i < inner_loop_count; i += 3) {
      float8 dx1 = input_position_x[i] - position_x;
      float8 dx2 = input_position_x[i + 1] - position_x;
      float8 dx3 = input_position_x[i + 2] - position_x;
      float8 dy1 = input_position_y[i] - position_y;
      float8 dy2 = input_position_y[i + 1] - position_y;
      float8 dy3 = input_position_y[i + 2] - position_y;
      float8 dz1 = input_position_z[i] - position_z;
      float8 dz2 = input_position_z[i + 1] - position_z;
      float8 dz3 = input_position_z[i + 2] - position_z;
      float8 distance_squared1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;
      float8 distance_squared2 = dx2 * dx2 + dy2 * dy2 + dz2 * dz2;
      float8 distance_squared3 = dx3 * dx3 + dy3 * dy3 + dz3 * dz3;
      distance_squared1 += softening_squared;
      distance_squared2 += softening_squared;
      distance_squared3 += softening_squared;
      float8 inverse_distance1 = rsqrt(distance_squared1);
      float8 inverse_distance2 = rsqrt(distance_squared2);
      float8 inverse_distance3 = rsqrt(distance_squared3);
      float8 mi1 = mass[i];
      float8 mi2 = mass[i + 1];
      float8 mi3 = mass[i + 2];
      float8 s1 =
          (mi1 * inverse_distance1) * (inverse_distance1 * inverse_distance1);
      float8 s2 =
          (mi2 * inverse_distance2) * (inverse_distance2 * inverse_distance2);
      float8 s3 =
          (mi3 * inverse_distance3) * (inverse_distance3 * inverse_distance3);
      acceleration_x1 += dx1 * s1;
      acceleration_x2 += dx2 * s2;
      acceleration_x3 += dx3 * s3;
      acceleration_y1 += dy1 * s1;
      acceleration_y2 += dy2 * s2;
      acceleration_y3 += dy3 * s3;
      acceleration_z1 += dz1 * s1;
      acceleration_z2 += dz2 * s2;
      acceleration_z3 += dz3 * s3;
    }
    for (; i < (body_count >> 3); i++) {
      float8 dx1 = input_position_x[i] - position_x;
      float8 dy1 = input_position_y[i] - position_y;
      float8 dz1 = input_position_z[i] - position_z;
      float8 distance_squared1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;
      distance_squared1 += softening_squared;
      // float8 inverse_distance1 = __native_rsqrtf8(distance_squared1);
      float8 inverse_distance1 = rsqrt(distance_squared1);
      float8 mi1 = mass[i];
      float8 s1 =
          (mi1 * inverse_distance1) * (inverse_distance1 * inverse_distance1);
      acceleration_x1 += dx1 * s1;
      acceleration_y1 += dy1 * s1;
      acceleration_z1 += dz1 * s1;
    }
    acceleration_x1 = acceleration_x1 + acceleration_x2 + acceleration_x3;
    acceleration_y1 = acceleration_y1 + acceleration_y2 + acceleration_y3;
    acceleration_z1 = acceleration_z1 + acceleration_z2 + acceleration_z3;
    float acc_x = acceleration_x1.s0 + acceleration_x1.s1 + acceleration_x1.s2 +
                  acceleration_x1.s3 + acceleration_x1.s4 + acceleration_x1.s5 +
                  acceleration_x1.s6 + acceleration_x1.s7;
    float acc_y = acceleration_y1.s0 + acceleration_y1.s1 + acceleration_y1.s2 +
                  acceleration_y1.s3 + acceleration_y1.s4 + acceleration_y1.s5 +
                  acceleration_y1.s6 + acceleration_y1.s7;
    float acc_z = acceleration_z1.s0 + acceleration_z1.s1 + acceleration_z1.s2 +
                  acceleration_z1.s3 + acceleration_z1.s4 + acceleration_z1.s5 +
                  acceleration_z1.s6 + acceleration_z1.s7;
    output_velocity_x[k] = input_velocity_x[k] + acc_x * time_delta;
    output_velocity_y[k] = input_velocity_y[k] + acc_y * time_delta;
    output_velocity_z[k] = input_velocity_z[k] + acc_z * time_delta;
    output_position_x[k] = in_position_x[k] + input_velocity_x[k] * time_delta +
                           acc_x * time_delta * time_delta / 2;
    output_position_y[k] = in_position_y[k] + input_velocity_y[k] * time_delta +
                           acc_y * time_delta * time_delta / 2;
    output_position_z[k] = in_position_z[k] + input_velocity_z[k] * time_delta +
                           acc_z * time_delta * time_delta / 2;
  }
}
//  nBodyScalarKernel - evaluates NBody algorithm over overall
// body_count=body_count_per_item*global size bodies   Each item evaluates
// velocity and position after time_delta units of time for its
// szElementsPerItem bodies within the input   input_position_x - input array
// with current x axis position of the element   input_position_y - input array
// with current y axis position of the element   input_position_z - input array
// with current z axis position of the element   mass - mass of the element
// input_velocity_x - input array with current x axis velocity of the element
// input_velocity_y - input array with current y axis velocity of the element
// input_velocity_z - input array with current z axis velocity of the element
// output_position_x - output array with current x axis position of the element
// output_position_y - output array with current y axis position of the element
// output_position_z - output array with current z axis position of the element
// output_velocity_x - output array with current x axis velocity of the element
// output_velocity_y - output array with current y axis velocity of the element
// output_velocity_z - output array with current z axis velocity of the element
// softening_squared - represents epsilon noise added to the distance evaluation
// between each pair of elements
__kernel void nBodyScalarKernel(
    const __global float *input_position_x,
    const __global float *input_position_y,
    const __global float *input_position_z, const __global float *mass,
    const __global float *input_velocity_x,
    const __global float *input_velocity_y,
    const __global float *input_velocity_z, __global float *output_position_x,
    __global float *output_position_y, __global float *output_position_z,
    __global float *output_velocity_x, __global float *output_velocity_y,
    __global float *output_velocity_z, int body_count, int body_count_per_item,
    float softening_squared, float time_delta) {
  int index = get_global_id(0);
  const __global float *in_position_x =
      (const __global float *)input_position_x;
  const __global float *in_position_y =
      (const __global float *)input_position_y;
  const __global float *in_position_z =
      (const __global float *)input_position_z;
  int i;
  int inner_loop_count = body_count;
  int start = index;
  //!!!!! This simplified (to help vectorizer) version assumes that
  //! body_count_per_item ==1 !!!!!!!!!!!!!!!
  //!!!!! So overall (global) number of work-items should be equal to the nuber
  //! of bodies being proccesed !!!!!!!!!!!!!!!
  //!!!!! i.e. szGlobalWork == szBodies in your cfg file !!!!!!!!!!!!!!!
  // int start = index * body_count_per_item;
  // int outer_loop_count = body_count_per_item;
  // int finish = start + outer_loop_count;
  // for (k = start; k < finish; k++)
  int k = start;
  {
    float position_x = (float)(in_position_x[k]);
    float position_y = (float)(in_position_y[k]);
    float position_z = (float)(in_position_z[k]);
    float acc_x = 0;
    float acc_y = 0;
    float acc_z = 0;
    for (i = 0; i < inner_loop_count; i++) {
      float dx = input_position_x[i] - position_x;
      float dy = input_position_y[i] - position_y;
      float dz = input_position_z[i] - position_z;
      float distance_squared = dx * dx + dy * dy + dz * dz + softening_squared;
      float inverse_distance = rsqrt(distance_squared);
      float mi = mass[i];
      float s = (mi * inverse_distance) * (inverse_distance * inverse_distance);
      acc_x += dx * s;
      acc_y += dy * s;
      acc_z += dz * s;
    }
    output_velocity_x[k] = input_velocity_x[k] + acc_x * time_delta;
    output_velocity_y[k] = input_velocity_y[k] + acc_y * time_delta;
    output_velocity_z[k] = input_velocity_z[k] + acc_z * time_delta;
    output_position_x[k] = in_position_x[k] + input_velocity_x[k] * time_delta +
                           acc_x * time_delta * time_delta / 2;
    output_position_y[k] = in_position_y[k] + input_velocity_y[k] * time_delta +
                           acc_y * time_delta * time_delta / 2;
    output_position_z[k] = in_position_z[k] + input_velocity_z[k] * time_delta +
                           acc_z * time_delta * time_delta / 2;
  }
}
