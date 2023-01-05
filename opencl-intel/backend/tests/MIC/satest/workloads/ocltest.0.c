#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef struct {
  double4 curPos;
  double4 prevPos;
  double4 prevVel;
  double4 curPosLoc;
  double4 prevPosLoc;
  double4 prevVelLoc;
  double4 delta;
} PrevModelData;
// multiply 4x4 matrix by 3-vector, return 3-vector
double3 mulMV(__global double4 *M, double3 V) {
  double3 rtn;
  rtn.x = dot(M[0].xyz, V);
  rtn.y = dot(M[1].xyz, V);
  rtn.z = dot(M[2].xyz, V);
  return rtn;
}
double linear(double a, double b, double t) { return a + (b - a) * t; }
// hairStiff
// hairDamp
// mHairPointCount
// mPreviousModelData
// mInputMcAsArray
// mOutputMcAsArray
/*__kernel
void HairCvSimEvalOpt_evaluate(
    int num_hairs,
    int mMaxPointCount,
    int reset_flag,
    int local_flag,
    double local_blend,
    constant double* hairStiff,
    constant double* hairDamp,
    constant int* mHairPointCount,
    global PrevModelData* mPreviousModelData,
    constant double4* mInputMcAsArray,
    global double4* mOutputMcAsArray,
    int mFlexProfileModel,
    int mStiffnessProfileModel,
    constant double* mSampledFlexProfile,
    constant double* mSampledStiffnessProfile,
    constant double4* local_ref_xfm_inv,
    constant double4* ref_xfm_inv,
    constant double4* local_ref_xfm,
    double mDynablend
)*/
__kernel void HairCvSimEvalOpt_evaluate(
    int num_hairs, int mMaxPointCount, int reset_flag, int local_flag,
    double local_blend, __global double *hairStiff, __global double *hairDamp,
    __global int *mHairPointCount, __global PrevModelData *mPreviousModelData,
    __global double4 *mInputMcAsArray, __global double4 *mOutputMcAsArray,
    int mFlexProfileModel, int mStiffnessProfileModel,
    __global double *mSampledFlexProfile,
    __global double *mSampledStiffnessProfile,
    __global double4 *local_ref_xfm_inv, __global double4 *ref_xfm_inv,
    __global double4 *local_ref_xfm, double mDynablend) {
  int h = get_global_id(0);
  int p = get_global_id(1);
  if (h < num_hairs && p < mMaxPointCount) {
    // from step
    double hair_stiff = hairStiff[h];
    double hair_damp = hairDamp[h];
    int nPoints = mHairPointCount[h];
    int idx = h * nPoints + p;
    // TODO: this indexing will not work if num points per hair is not the same
    // for all hairs
    __global PrevModelData *pt_data = mPreviousModelData;
    double4 in_pt_data = mInputMcAsArray[idx];
    __global double4 *out_pt_data = mOutputMcAsArray;
    // Store current position
    double3 cur_pos = in_pt_data.xyz;
    // Set values in point mat
    pt_data[idx].curPos.xyz = cur_pos;
    if (local_flag) {
      double3 loc_pos = mulMV(local_ref_xfm_inv, cur_pos);
      pt_data[idx].curPosLoc.xyz = loc_pos;
    }
    double3 delta;
    double3 ref_delta;
    // step and save_state skip point 0
    if (p > 0) {
      if (!reset_flag) {
        // from step
        // Normalized param along hair
        double t = (double)p / (double)(nPoints - 1);
        // Evaluate profiles for this point
        double flex = linear(0, 1, t);
        double stiff_mult = 1.0;
        // if (mFlexProfileModel != 0) flex = mSampledFlexProfile[p];
        // if (mStiffnessProfileModel != 0) stiff_mult =
        // mSampledStiffnessProfile[p];
        double cv_blend = mDynablend * flex;
        double3 pos;
        double3 prev_pos;
        double3 loc_delta;
        double3 vel;
        double3 loc_vel;
        // Calculate position delta in world space
        //
        double3 x = pt_data[idx].curPos.xyz;
        double3 x_prev = pt_data[idx].prevPos.xyz;
        double3 x_vel_prev = pt_data[idx].prevVel.xyz;
        double3 dx = stiff_mult * hair_stiff * (x - x_prev);
        double3 dv = hair_damp * x_vel_prev;
        double3 acc = (dx - dv);
        double3 x_vel = x_vel_prev + acc;
        double3 outval = x_prev + x_vel;
        double3 delta_x = cv_blend * (outval - x);
        vel = x_vel;
        delta = delta_x;
        pt_data[idx].prevVel.xyz = vel; // Copy velocity into prev_vel
        pt_data[idx].prevPos.xyz =
            x_prev + delta_x; // Copy position into prev_pos
        // Calculate position delta in local space
        //
        if (local_flag) {
          double3 x_loc = pt_data[idx].curPosLoc.xyz;
          double3 x_loc_prev = pt_data[idx].prevPosLoc.xyz;
          double3 x_loc_vel_prev = pt_data[idx].prevVelLoc.xyz;
          dx = stiff_mult * hair_stiff * (x_loc - x_loc_prev);
          dv = hair_damp * x_loc_vel_prev;
          acc = (dx - dv);
          double3 x_loc_vel = x_loc_vel_prev + acc;
          outval = x_loc_prev + x_loc_vel;
          delta_x = cv_blend * (outval - x_loc);
          loc_vel = x_loc_vel;
          loc_delta = delta_x;
          // Copy local vel into prev local vel
          pt_data[idx].prevVelLoc.xyz = loc_vel;
          // Copy local pos into prev local pos
          pt_data[idx].prevPosLoc.xyz = x_loc + delta_x;
          // Blend world and local deltas together
          delta = (local_blend * loc_delta) + ((1 - local_blend) * delta);
        }
        // Put delta into reference space
        ref_delta = mulMV(ref_xfm_inv, delta);
        pt_data[idx].delta.xyz = ref_delta; // Store delta
      }
      // from save_state
      double3 ws_delta;
      ref_delta = pt_data[idx].delta.xyz;
      ws_delta = mulMV(local_ref_xfm, ref_delta);
      // Copy world pos (plus delta) to prev pos
      pt_data[idx].prevPos.xyz = pt_data[idx].curPos.xyz + ws_delta;
      if (local_flag) {
        // Copy local pos (plus delta) into prev local pos
        pt_data[idx].prevPosLoc.xyz =
            pt_data[idx].curPosLoc.xyz + pt_data[idx].delta.xyz;
      }
    }
    double3 deltaData = pt_data[idx].delta.xyz;
    // Get the cached delta position for each cv
    delta = deltaData;
    // Put delta back into world space
    delta = mulMV(local_ref_xfm, delta);
    // Store delta val in data model (ensures synching)
    pt_data[idx].delta.xyz = delta;
    double3 world_pos = pt_data[idx].curPos.xyz;
    // Add delta to world position
    delta *= mDynablend;
    world_pos += delta;
    out_pt_data[idx] = as_double4(world_pos);
  }
}
