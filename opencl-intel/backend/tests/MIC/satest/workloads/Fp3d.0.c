/********************************************************************************/
/*                                                                              */
/*    Filename: RaySum_8_1.cl */
/*                                                                              */
/*    Copyright (c) 2003-2009, GE Healthcare. All rights reserved */
/*                                                                              */
/*    For use only by GE Healthcare. */
/*    This document contains proprietary information. */
/*    It must not be reproduced or disclosed to others without prior */
/*    written approval. */
/*                                                                              */
/*    Description: */
/*    This file is an OpenCL kernel used in ray-driven forward projection */
/*                                                                              */
/*    Inputs: */
/*      -image volume to be forward projected */
/*                                                                              */
/*    Outputs: */
/*      -float32 value representing the line integral of the attenuation */
/*       of a ray as it passes through the volume */
/*                                                                              */
/*    Notes: */
/*      -Implementation tested on the following OpenCL SDKs */
/*        -AMD, v2.2 RC2 */
/*        -nVidia, 3.1 */
/*      -more info at GEHC wiki "Ray-Driven Forward Projection" */
/*                                                                              */
/*    Original Creation Date:  11 August 2010 */
/*    Original Author:         David Coccarelli */
/*                                                                              */
/********************************************************************************/
__kernel void raySum81(__global short *inVolume /* Input Image Volume */
                       ,
                       __global float *outSino /* Ouput Sinogram */) {
  /* Calculate which ray is being processed
   * ****************************************/
  const int id = get_global_id(0);        // Pull the global id of the work item
  const int view = id / NUMCHANXNUMCHROW; // Calculate the view being processed
  const int chrow = (id - view * NUMCHANXNUMCHROW) /
                    NUMCHAN; // Calculate the row being processed
  const int chan = id - view * NUMCHANXNUMCHROW -
                   chrow * NUMCHAN; // Calculate the channel being processed
  /* Calculate the geometrical parameters describing the ray
   * ************************/
  const float beta =
      BETAKNAUGHT +
      view * BETAINC; // Calculate the angle of the gantry  position
  const float beta_adj = (DETANGLERAD * (CENTCHAN - chan)) +
                         beta; // Calculate the angle of the given ray
  /* Calculate the position of the source and detector */
  const float4 source =
      (float4)(NS2ISO * cos(beta - PIDIV2), NS2ISO * sin(beta - PIDIV2),
               ZSOURCE_AX +
                   HELICAL * (VIEWINCZ * (view - 73.0f) - CENTCHROWXNDETZ),
               0);
  const float4 det =
      (float4)(sin(beta_adj) * NS2ISO + source.x,
               -cos(beta_adj) * NS2ISO + source.y,
               source.z + TABLEDIR * /*NDETZ*/ (-CENTCHROW + chrow), 0);
  const float4 r =
      det - source; // The difference vector between the detector and radius
  /* Precalculate the stepping for the coordinates used in the summation */
  const float xval_incy_x1 =
      r.y / r.x; // coordinate stepping for the x value for one branch
  const float xval_incx_x1 =
      -TABLEDIR * r.x /
      r.y;           // coordinate stepping for the x value for the other branch
  float zval_inc_x1; // coordinate stepping for the z value
  /* Declare and initialize variables */
  float4 weightx_0; // weight for pixel interpolation in the x dimension for
                    // pixel_0
  float4 weightx_1; // weight for pixel interpolation in the x dimension for
                    // pixel_1
  float4
      weight_z; // weight for pixel interpolation in the z dimension for pixel_0
  float4 xval;  // x value for interpolation
  float4 zval;  // z value for interpolation
  float4 interposum = 0.0f; // summation for ray integral
  float m = 0.0f;           // linear slope multiplier for 3D line
  float l = 0.0f;           // weighting factor
  int plane;                // the plane constrained for interpolation
  int4 x1; // vector containing the x coordates to index into the input volume
  int4 z1; // vector containing the z coordates to index into the input volume
  int4 a;  // vector containing one-dimensional indicies to the input volume
  int4 b;  // vector containing one-dimensional indicies to the input volume
  int4 planevol; // vector containing one-dimensional indicies to the input
                 // volume
                 /* Summation of the Ray's attenuation
                  * *********************************************/
  /* First branching option */
  if ((xval_incy_x1 < -1.0f) ||
      (xval_incy_x1 >
       1.0f)) { // check to see how the volume should be stepped through
    l = fabs(sqrt((r.x * r.x + r.y * r.y) * (r.z * r.z + r.y * r.y)) /
             (r.y * r.y)); // calculate the ray weighting based on ray distance
                           // through each pixel
    m = (YVALINIT - source.y) / r.y;  // calculate the slope parameter
    zval_inc_x1 = r.z / r.y;          // coordinate stepping for the z value
    zval = source.z + m * r.z + 1.0f; // initialize z coordinates
    zval -= (float4)(zval_inc_x1, zval_inc_x1, zval_inc_x1,
                     zval_inc_x1); // calculate first z value stepping
    zval -= (float4)(zval_inc_x1, zval_inc_x1, zval_inc_x1, 0.0f);
    zval -= (float4)(zval_inc_x1, zval_inc_x1, 0.0f, 0.0f);
    zval -= (float4)(zval_inc_x1, 0.0f, 0.0f, 0.0f);
    xval =
        -TABLEDIR * (source.x + m * r.x) + VOLWDIV2; // initialize x coordinates
    xval -= (float4)(xval_incx_x1, xval_incx_x1, xval_incx_x1,
                     xval_incx_x1); // calculate first x value stepping
    xval -= (float4)(xval_incx_x1, xval_incx_x1, xval_incx_x1, 0.0f);
    xval -= (float4)(xval_incx_x1, xval_incx_x1, 0.0f, 0.0f);
    xval -= (float4)(xval_incx_x1, 0.0f, 0.0f, 0.0f);
    for (plane = 0; plane < VOLW;
         plane += 4) {      // for loop to step through all y values
      xval += xval_incx_x1; // increment x coordinate
      xval += xval_incx_x1;
      xval += xval_incx_x1;
      xval += xval_incx_x1;
      zval += zval_inc_x1; // increment z coordinate
      zval += zval_inc_x1;
      zval += zval_inc_x1;
      zval += zval_inc_x1;
      if (((xval.x >= 0.5f) && (xval.x < (VOLW - 1)) && (zval.x >= .5f) &&
           (zval.x < (VOLD + 1.5f))) ||
          ((xval.w >= 0.5f) && (xval.w < (VOLW - 1)) && (zval.w >= .5f) &&
           (zval.w < (VOLD + 1.5f)))) {
        weightx_0 = convert_float4(convert_int4(xval + 0.5f)) - xval +
                    0.5f; // calculate x dimension weighting
        weightx_1 = 1.0f - weightx_0;
        weight_z = convert_float4(convert_int4(zval + 0.5f)) - zval +
                   0.5f;                // calculate z dimension weighting
        x1 = convert_int4(xval - 0.5f); // convert float coordinate to integer
                                        // for indexing, x dimension
        z1 = convert_int4(zval - 0.5f); // convert float coordinate to integer
                                        // for indexing, z dimension
        planevol = VOLW * (int4)(plane, plane + 1, plane + 2, plane + 3) +
                   x1;               // continue to calculate indices
        a = z1 * VOLAREA + planevol; // continue to calculate indices
        a &= (xval >= 0.5f) & (xval < (VOLW - 1)) & (zval >= .5f) &
             (zval <
              (VOLD +
               1.5f));   // mask the indices to avoid indexing out of the volume
        b = a + VOLAREA; // calculate this addition ahead of time
        /*  interpolation and summation */
        interposum +=
            weight_z *
                (weightx_0 * (float4)(inVolume[a.x], inVolume[a.y],
                                      inVolume[a.z], inVolume[a.w]) +
                 (weightx_1) * (float4)(inVolume[a.x + 1], inVolume[a.y + 1],
                                        inVolume[a.z + 1], inVolume[a.w + 1])) +
            (1.0f - weight_z) *
                (weightx_0 * (float4)(inVolume[b.x], inVolume[b.y],
                                      inVolume[b.z], inVolume[b.w]) +
                 (weightx_1) * (float4)(inVolume[b.x + 1], inVolume[b.y + 1],
                                        inVolume[b.z + 1], inVolume[b.w + 1]));
      }
    }
  }
  /* Second branching option */
  else {
    l = fabs(sqrt((r.x * r.x + r.y * r.y) * (r.z * r.z + r.x * r.x)) /
             (r.x * r.x)); // calculate the ray weighting based on ray distance
                           // through each pixel
    m = (YVALINIT - source.x) / r.x;  // calculate the slope parameter
    zval_inc_x1 = r.z / r.x;          // coordinate stepping for the z value
    zval = source.z + m * r.z + 1.0f; // initialize z coordinates
    zval -= (float4)(zval_inc_x1, zval_inc_x1, zval_inc_x1,
                     zval_inc_x1); // calculate first z value stepping
    zval -= (float4)(zval_inc_x1, zval_inc_x1, zval_inc_x1, 0.0f);
    zval -= (float4)(zval_inc_x1, zval_inc_x1, 0.0f, 0.0f);
    zval -= (float4)(zval_inc_x1, 0.0f, 0.0f, 0.0f);
    xval = source.y + m * r.y + VOLWDIV2; // initialize x coordinates
    xval -= (float4)(xval_incy_x1, xval_incy_x1, xval_incy_x1,
                     xval_incy_x1); // calculate first x value stepping
    xval -= (float4)(xval_incy_x1, xval_incy_x1, xval_incy_x1, 0.0f);
    xval -= (float4)(xval_incy_x1, xval_incy_x1, 0.0f, 0.0f);
    xval -= (float4)(xval_incy_x1, 0.0f, 0.0f, 0.0f);
    int4 plane_vec;
    if (TABLEDIR == 1) {
      plane_vec = (int4)(VOLW, VOLW - 1, VOLW - 2, VOLW - 3);
      // lane_start = VOLW;
      //  plane_end   = 0;
    } else {
      plane_vec = (int4)(0, 1, 2, 3);
      //  plane_start = 0;
      //  plane_end   = VOLW;
    }
    plane_vec += TABLEDIR * 4;
    for (plane = 0; plane < VOLW;
         plane += 4) {      // for loop to step through all y values
      xval += xval_incy_x1; // increment x coordinate
      xval += xval_incy_x1;
      xval += xval_incy_x1;
      xval += xval_incy_x1;
      zval += zval_inc_x1; // increment z coordinate
      zval += zval_inc_x1;
      zval += zval_inc_x1;
      zval += zval_inc_x1;
      plane_vec += -TABLEDIR * 4;
      if (((xval.x >= 0.5f) && (xval.x < (VOLW - 1)) && (zval.x >= .5f) &&
           (zval.x < (VOLD + 1.5f))) ||
          ((xval.w >= 0.5f) && (xval.w < (VOLW - 1)) && (zval.w >= .5f) &&
           (zval.w < (VOLD + 1.5f)))) {
        weightx_0 = convert_float4(convert_int4(xval + 0.5f)) - xval +
                    0.5f; // calculate x dimension weighting
        weightx_1 = 1.0f - weightx_0;
        weight_z = convert_float4(convert_int4(zval + 0.5f)) - zval +
                   0.5f;                // calculate z dimension weighting
        x1 = convert_int4(xval - 0.5f); // convert float coordinate to integer
                                        // for indexing, x dimension
        z1 = convert_int4(zval - 0.5f); // convert float coordinate to integer
                                        // for indexing, z dimension
        a = z1 * VOLAREA + plane_vec /* + -TABLEDIR*(int4)(0,1,2,3)*/ +
            x1 * VOLW; // continue to calculate indices
        a &= (xval >= 0.5f) & (xval < (VOLW - 1)) & (zval >= .5f) &
             (zval <
              (VOLD +
               1.5f)); // mask the indices to avoid indexing out of the volume
        /*  interpolation and summation */
        interposum +=
            weight_z *
                (weightx_0 *
                     convert_float4((short4)(inVolume[a.x], inVolume[a.y],
                                             inVolume[a.z], inVolume[a.w])) +
                 (weightx_1)*convert_float4(
                     (short4)(inVolume[a.x + VOLW], inVolume[a.y + VOLW],
                              inVolume[a.z + VOLW], inVolume[a.w + VOLW]))) +
            (1.0f - weight_z) *
                (weightx_0 * convert_float4((short4)(inVolume[a.x + VOLAREA],
                                                     inVolume[a.y + VOLAREA],
                                                     inVolume[a.z + VOLAREA],
                                                     inVolume[a.w + VOLAREA])) +
                 (weightx_1)*convert_float4(
                     (short4)(inVolume[a.x + VOLVOL], inVolume[a.y + VOLVOL],
                              inVolume[a.z + VOLVOL], inVolume[a.w + VOLVOL])));
      }
    }
  }
  /* Write output
   * ***************************************************************/
  outSino[id] = l * (interposum.x + interposum.y + interposum.z + interposum.w);
}
