
#define local_barrier() barrier(CLK_LOCAL_MEM_FENCE);

#define WITHIN_KERNEL /* empty */
#define KERNEL __kernel
#define GLOBAL_MEM __global
#define LOCAL_MEM __local
#define LOCAL_MEM_ARG __local
#define REQD_WG_SIZE(X, Y, psc_Z)                                              \
  __attribute__((reqd_work_group_size(X, Y, psc_Z)))

#define psc_LID_0 get_local_id(0)
#define psc_LID_1 get_local_id(1)
#define psc_LID_2 get_local_id(2)

#define psc_GID_0 get_group_id(0)
#define psc_GID_1 get_group_id(1)
#define psc_GID_2 get_group_id(2)

#define psc_LDIM_0 get_local_size(0)
#define psc_LDIM_1 get_local_size(1)
#define psc_LDIM_2 get_local_size(2)

#define psc_GDIM_0 get_num_groups(0)
#define psc_GDIM_1 get_num_groups(1)
#define psc_GDIM_2 get_num_groups(2)

#if __OPENCL_C_VERSION__ < 120
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
// CL//
#define psc_WG_SIZE 4

#define psc_SCAN_EXPR(a, b, across_seg_boundary)                               \
  scan_t_add(a, b, across_seg_boundary)
#define psc_INPUT_EXPR(i)                                                      \
  (scan_t_from_particle(i, level, &bbox, morton_nrs, user_srcntgt_ids, x, y,   \
                        srcntgt_radii))
#define psc_IS_SEG_START(i, a) (box_start_flags[i])

typedef struct {
  double min_x;
  double max_x;
  double min_y;
  double max_y;
} boxtree_bbox_2d_f8_t;

typedef struct {
  int nonchild_srcntgts;
  int pcnt00;
  int pcnt01;
  int pcnt10;
  int pcnt11;
} boxtree_morton_bin_count_2d_pi4_ext_t;

// CL//
typedef boxtree_morton_bin_count_2d_pi4_ext_t morton_counts_t;
typedef morton_counts_t scan_t;
typedef boxtree_bbox_2d_f8_t bbox_t;
typedef double coord_t;
typedef double2 coord_vec_t;
typedef int box_id_t;
typedef int particle_id_t;

// morton_nr == -1 is defined to mean that the srcntgt is
// remaining at the present level and will not be sorted
// into a child box.
typedef signed char morton_nr_t;
// CL//
#define STICK_OUT_FACTOR ((coord_t)0.25)

// Use this as dbg_printf(("oh snap: %d\n", stuff)); Note the double
// parentheses.
//
// Watch out: 64-bit values on Intel CL must be printed with %ld, or
// subsequent values will print as 0. Things may crash. And you'll be very
// confused.

#define dbg_printf(ARGS) /* */

#define dbg_assert(CONDITION) ((void)0)

// CL//

// {{{ neutral element

scan_t scan_t_neutral() {
  scan_t result;
  result.nonchild_srcntgts = 0;
  result.pcnt00 = 0;
  result.pcnt01 = 0;
  result.pcnt10 = 0;
  result.pcnt11 = 0;
  return result;
}

// }}}

// {{{ scan 'add' operation
scan_t scan_t_add(scan_t a, scan_t b, bool across_seg_boundary) {
  if (!across_seg_boundary) {
    b.nonchild_srcntgts += a.nonchild_srcntgts;

    b.pcnt00 = a.pcnt00 + b.pcnt00;

    b.pcnt01 = a.pcnt01 + b.pcnt01;

    b.pcnt10 = a.pcnt10 + b.pcnt10;

    b.pcnt11 = a.pcnt11 + b.pcnt11;
  }

  return b;
}

// }}}

// {{{ scan data type init from particle

scan_t
scan_t_from_particle(const int i, const int level, bbox_t const *bbox,
                     global morton_nr_t *morton_nrs, // output/side effect
                     global particle_id_t *user_srcntgt_ids,
                     global const coord_t *x, global const coord_t *y,
                     global const coord_t *srcntgt_radii) {
  particle_id_t user_srcntgt_id = user_srcntgt_ids[i];

  // Recall that 'level' is the level currently being built, e.g. 1 at
  // the root.  This should be 0.5 at level 1. (Level 0 is the root.)
  coord_t next_level_box_size_factor = ((coord_t)1) / ((coord_t)(1U << level));

  bool stop_srcntgt_descent = false;
  coord_t srcntgt_radius = srcntgt_radii[user_srcntgt_id];

  const coord_t one_half = ((coord_t)1) / 2;
  const coord_t box_radius_factor =
      // AMD CPU seems to like to miscompile this--change with care.
      // (last seen on 13.4-2)
      (1. + STICK_OUT_FACTOR) * one_half; // convert diameter to radius

  // Most FMMs are isotropic, i.e. global_extent_{x,y,z} are all the same.
  // Nonetheless, the gain from exploiting this assumption seems so
  // minimal that doing so here didn't seem worthwhile.

  coord_t global_min_x = bbox->min_x;
  coord_t global_extent_x = bbox->max_x - global_min_x;
  coord_t srcntgt_x = x[user_srcntgt_id];

  // Note that the upper bound of the global bounding box is computed
  // to be slightly larger than the highest found coordinate, so that
  // 1.0 is never reached as a scaled coordinate at the highest
  // level, and it isn't either by the fact that boxes are
  // [)-half-open in subsequent levels.

  // So (1 << level) is 2 when building level 1.  Because the
  // floating point factor is strictly less than 1, 2 is never
  // reached, so when building level 1, the result is either 0 or 1.
  // After that, we just add one (less significant) bit per level.

  unsigned x_bits = (unsigned)(((srcntgt_x - global_min_x) / global_extent_x) *
                               (1U << level));

  // Need to compute center to compare excess with STICK_OUT_FACTOR.
  coord_t next_level_box_center_x =
      global_min_x +
      global_extent_x * (x_bits + one_half) * next_level_box_size_factor;

  coord_t next_level_box_stick_out_radius_x =
      box_radius_factor * global_extent_x * next_level_box_size_factor;

  stop_srcntgt_descent =
      stop_srcntgt_descent ||
      (srcntgt_x + srcntgt_radius >=
       next_level_box_center_x + next_level_box_stick_out_radius_x);
  stop_srcntgt_descent =
      stop_srcntgt_descent ||
      (srcntgt_x - srcntgt_radius <
       next_level_box_center_x - next_level_box_stick_out_radius_x);
  // Most FMMs are isotropic, i.e. global_extent_{x,y,z} are all the same.
  // Nonetheless, the gain from exploiting this assumption seems so
  // minimal that doing so here didn't seem worthwhile.

  coord_t global_min_y = bbox->min_y;
  coord_t global_extent_y = bbox->max_y - global_min_y;
  coord_t srcntgt_y = y[user_srcntgt_id];

  // Note that the upper bound of the global bounding box is computed
  // to be slightly larger than the highest found coordinate, so that
  // 1.0 is never reached as a scaled coordinate at the highest
  // level, and it isn't either by the fact that boxes are
  // [)-half-open in subsequent levels.

  // So (1 << level) is 2 when building level 1.  Because the
  // floating point factor is strictly less than 1, 2 is never
  // reached, so when building level 1, the result is either 0 or 1.
  // After that, we just add one (less significant) bit per level.

  unsigned y_bits = (unsigned)(((srcntgt_y - global_min_y) / global_extent_y) *
                               (1U << level));

  // Need to compute center to compare excess with STICK_OUT_FACTOR.
  coord_t next_level_box_center_y =
      global_min_y +
      global_extent_y * (y_bits + one_half) * next_level_box_size_factor;

  coord_t next_level_box_stick_out_radius_y =
      box_radius_factor * global_extent_y * next_level_box_size_factor;

  stop_srcntgt_descent =
      stop_srcntgt_descent ||
      (srcntgt_y + srcntgt_radius >=
       next_level_box_center_y + next_level_box_stick_out_radius_y);
  stop_srcntgt_descent =
      stop_srcntgt_descent ||
      (srcntgt_y - srcntgt_radius <
       next_level_box_center_y - next_level_box_stick_out_radius_y);

  // Pick off the lowest-order bit for each axis, put it in its place.
  int level_morton_number = 0 | (x_bits & 1U) << (1) | (y_bits & 1U) << (0);

  if (stop_srcntgt_descent) {
    level_morton_number = -1;
  }

  scan_t result;
  result.nonchild_srcntgts = (level_morton_number == -1);

  result.pcnt00 = (level_morton_number == 0);

  result.pcnt01 = (level_morton_number == 1);

  result.pcnt10 = (level_morton_number == 2);

  result.pcnt11 = (level_morton_number == 3);
  morton_nrs[i] = level_morton_number;

  return result;
}

// }}}

typedef boxtree_morton_bin_count_2d_pi4_ext_t psc_scan_type;
typedef int psc_index_type;

// NO_SEG_BOUNDARY is the largest representable integer in psc_index_type.
// This assumption is used in code below.
#define NO_SEG_BOUNDARY 2147483647
// CL//

#define psc_K 256

// #define psc_DEBUG
#ifdef psc_DEBUG
#define pycl_printf(psc_ARGS) printf psc_ARGS
#else
#define pycl_printf(psc_ARGS) /* */
#endif

KERNEL
REQD_WG_SIZE(psc_WG_SIZE, 1, 1)
void scan_scan_intervals_lev1(
    __global boxtree_morton_bin_count_2d_pi4_ext_t *morton_bin_counts,
    __global signed char *morton_nrs, __global unsigned char *box_start_flags,
    __global int *srcntgt_box_ids, __global int *split_box_ids,
    __global boxtree_morton_bin_count_2d_pi4_ext_t *box_morton_bin_counts,
    __global int *box_srcntgt_starts, __global int *box_srcntgt_counts_cumul,
    __global int *box_parent_ids, __global signed char *box_morton_nrs,
    __global int *nboxes, int level, int max_particles_in_box,
    boxtree_bbox_2d_f8_t bbox, __global int *user_srcntgt_ids,
    __global double *x, __global double *y, __global double *srcntgt_radii,
    GLOBAL_MEM psc_scan_type *restrict psc_partial_scan_buffer,
    const psc_index_type N, const psc_index_type psc_interval_size,
    GLOBAL_MEM psc_scan_type *restrict psc_interval_results
    // NO_SEG_BOUNDARY if no segment boundary in interval.
    ,
    GLOBAL_MEM psc_index_type *restrict psc_g_first_segment_start_in_interval) {
  // index psc_K in first dimension used for psc_carry storage
  struct psc_wrapped_scan_type {
    psc_scan_type psc_value;
  };

  // padded in psc_WG_SIZE to avoid bank conflicts
  LOCAL_MEM struct psc_wrapped_scan_type psc_ldata[psc_K + 1][psc_WG_SIZE];

  LOCAL_MEM char psc_l_segment_start_flags[psc_K][psc_WG_SIZE];
  LOCAL_MEM psc_index_type psc_l_first_segment_start_in_subtree[psc_WG_SIZE];

  // only relevant/populated for local id 0
  psc_index_type psc_first_segment_start_in_interval = NO_SEG_BOUNDARY;

  psc_index_type psc_first_segment_start_in_k_group,
      psc_first_segment_start_in_subtree;

  // {{{ declare local data for input_fetch_exprs if any of them are stenciled

  // }}}

  const psc_index_type psc_interval_begin = psc_interval_size * psc_GID_0;
  const psc_index_type psc_interval_end =
      min(psc_interval_begin + psc_interval_size, N);

  const psc_index_type psc_unit_size = psc_K * psc_WG_SIZE;

  psc_index_type psc_unit_base = psc_interval_begin;

  for (; psc_unit_base + psc_unit_size <= psc_interval_end;
       psc_unit_base += psc_unit_size)

  {

    // {{{ psc_carry out input_fetch_exprs
    // (if there are ones that need to be fetched into local)

    pycl_printf(("after input_fetch_exprs\n"));

    // }}}

    // {{{ read a unit's worth of data from psc_global

    for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
      const psc_index_type psc_offset = psc_k * psc_WG_SIZE + psc_LID_0;
      const psc_index_type psc_read_i = psc_unit_base + psc_offset;

      {

        psc_scan_type psc_scan_value = psc_INPUT_EXPR(psc_read_i);

        const psc_index_type psc_o_mod_k = psc_offset % psc_K;
        const psc_index_type psc_o_div_k = psc_offset / psc_K;
        psc_ldata[psc_o_mod_k][psc_o_div_k].psc_value = psc_scan_value;

        bool psc_is_seg_start = psc_IS_SEG_START(psc_read_i, psc_scan_value);
        psc_l_segment_start_flags[psc_o_mod_k][psc_o_div_k] = psc_is_seg_start;
      }
    }

    pycl_printf(("after read from psc_global\n"));

    // }}}

    // {{{ psc_carry in from previous unit, if applicable

    local_barrier();

    psc_first_segment_start_in_k_group = NO_SEG_BOUNDARY;
    if (psc_l_segment_start_flags[0][psc_LID_0])
      psc_first_segment_start_in_k_group = psc_unit_base + psc_K * psc_LID_0;

    if (psc_LID_0 == 0 && psc_unit_base != psc_interval_begin) {
      psc_ldata[0][0].psc_value = psc_SCAN_EXPR(
          psc_ldata[psc_K][psc_WG_SIZE - 1].psc_value,
          psc_ldata[0][0].psc_value, (psc_l_segment_start_flags[0][0]));
    }

    pycl_printf(("after psc_carry-in\n"));

    // }}}

    local_barrier();

    // {{{ scan along psc_k (sequentially in each work item)

    psc_scan_type psc_sum = psc_ldata[0][psc_LID_0].psc_value;

    for (psc_index_type psc_k = 1; psc_k < psc_K; psc_k++) {
      {
        psc_scan_type psc_tmp = psc_ldata[psc_k][psc_LID_0].psc_value;
        psc_index_type psc_seq_i = psc_unit_base + psc_K * psc_LID_0 + psc_k;

        if (psc_l_segment_start_flags[psc_k][psc_LID_0]) {
          psc_first_segment_start_in_k_group =
              min(psc_first_segment_start_in_k_group, psc_seq_i);
        }

        psc_sum = psc_SCAN_EXPR(psc_sum, psc_tmp,
                                (psc_l_segment_start_flags[psc_k][psc_LID_0]));

        psc_ldata[psc_k][psc_LID_0].psc_value = psc_sum;
      }
    }

    pycl_printf(("after scan along psc_k\n"));

    // }}}

    // store psc_carry in out-of-bounds (padding) array entry (index psc_K) in
    // the psc_K direction
    psc_ldata[psc_K][psc_LID_0].psc_value = psc_sum;

    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_k_group;

    local_barrier();

    // {{{ tree-based local parallel scan

    // This tree-based scan works as follows:
    // - Each work item adds the previous item to its current state
    // - barrier
    // - Each work item adds in the item from two positions to the left
    // - barrier
    // - Each work item adds in the item from four positions to the left
    // ...
    // At the end, each item has summed all prior items.

    // across psc_k groups, along local id
    // (uses out-of-bounds psc_k=psc_K array entry for storage)

    psc_scan_type psc_val = psc_ldata[psc_K][psc_LID_0].psc_value;

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 1) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 1].psc_value;
      {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 1]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 2) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 2].psc_value;
      {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 2]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 4) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 4].psc_value;
      {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 4]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    pycl_printf(("after tree scan\n"));

    // }}}

    // {{{ update local values

    if (psc_LID_0 > 0) {
      psc_sum = psc_ldata[psc_K][psc_LID_0 - 1].psc_value;

      for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
        {
          psc_scan_type psc_tmp = psc_ldata[psc_k][psc_LID_0].psc_value;
          psc_ldata[psc_k][psc_LID_0].psc_value =
              psc_SCAN_EXPR(psc_sum, psc_tmp,
                            (psc_unit_base + psc_K * psc_LID_0 + psc_k >=
                             psc_first_segment_start_in_k_group));
        }
      }
    }

    if (psc_LID_0 == 0) {
      // update interval-wide first-seg variable from current unit
      psc_first_segment_start_in_interval =
          min(psc_first_segment_start_in_interval,
              psc_l_first_segment_start_in_subtree[psc_WG_SIZE - 1]);
    }

    pycl_printf(("after local update\n"));

    // }}}

    local_barrier();

    // {{{ write data

    for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
      const psc_index_type psc_offset = psc_k * psc_WG_SIZE + psc_LID_0;

      {
        pycl_printf(("write: %d\n", psc_unit_base + psc_offset));
        psc_partial_scan_buffer[psc_unit_base + psc_offset] =
            psc_ldata[psc_offset % psc_K][psc_offset / psc_K].psc_value;
      }
    }

    pycl_printf(("after write\n"));

    // }}}

    local_barrier();
  }

  if (psc_unit_base < psc_interval_end)

  {

    // {{{ psc_carry out input_fetch_exprs
    // (if there are ones that need to be fetched into local)

    pycl_printf(("after input_fetch_exprs\n"));

    // }}}

    // {{{ read a unit's worth of data from psc_global

    for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
      const psc_index_type psc_offset = psc_k * psc_WG_SIZE + psc_LID_0;
      const psc_index_type psc_read_i = psc_unit_base + psc_offset;

      if (psc_read_i < psc_interval_end) {

        psc_scan_type psc_scan_value = psc_INPUT_EXPR(psc_read_i);

        const psc_index_type psc_o_mod_k = psc_offset % psc_K;
        const psc_index_type psc_o_div_k = psc_offset / psc_K;
        psc_ldata[psc_o_mod_k][psc_o_div_k].psc_value = psc_scan_value;

        bool psc_is_seg_start = psc_IS_SEG_START(psc_read_i, psc_scan_value);
        psc_l_segment_start_flags[psc_o_mod_k][psc_o_div_k] = psc_is_seg_start;
      }
    }

    pycl_printf(("after read from psc_global\n"));

    // }}}

    // {{{ psc_carry in from previous unit, if applicable

    local_barrier();

    psc_first_segment_start_in_k_group = NO_SEG_BOUNDARY;
    if (psc_l_segment_start_flags[0][psc_LID_0])
      psc_first_segment_start_in_k_group = psc_unit_base + psc_K * psc_LID_0;

    if (psc_LID_0 == 0 && psc_unit_base != psc_interval_begin) {
      psc_ldata[0][0].psc_value = psc_SCAN_EXPR(
          psc_ldata[psc_K][psc_WG_SIZE - 1].psc_value,
          psc_ldata[0][0].psc_value, (psc_l_segment_start_flags[0][0]));
    }

    pycl_printf(("after psc_carry-in\n"));

    // }}}

    local_barrier();

    // {{{ scan along psc_k (sequentially in each work item)

    psc_scan_type psc_sum = psc_ldata[0][psc_LID_0].psc_value;

    const psc_index_type psc_offset_end = psc_interval_end - psc_unit_base;

    for (psc_index_type psc_k = 1; psc_k < psc_K; psc_k++) {
      if (psc_K * psc_LID_0 + psc_k < psc_offset_end) {
        psc_scan_type psc_tmp = psc_ldata[psc_k][psc_LID_0].psc_value;
        psc_index_type psc_seq_i = psc_unit_base + psc_K * psc_LID_0 + psc_k;

        if (psc_l_segment_start_flags[psc_k][psc_LID_0]) {
          psc_first_segment_start_in_k_group =
              min(psc_first_segment_start_in_k_group, psc_seq_i);
        }

        psc_sum = psc_SCAN_EXPR(psc_sum, psc_tmp,
                                (psc_l_segment_start_flags[psc_k][psc_LID_0]));

        psc_ldata[psc_k][psc_LID_0].psc_value = psc_sum;
      }
    }

    pycl_printf(("after scan along psc_k\n"));

    // }}}

    // store psc_carry in out-of-bounds (padding) array entry (index psc_K) in
    // the psc_K direction
    psc_ldata[psc_K][psc_LID_0].psc_value = psc_sum;

    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_k_group;

    local_barrier();

    // {{{ tree-based local parallel scan

    // This tree-based scan works as follows:
    // - Each work item adds the previous item to its current state
    // - barrier
    // - Each work item adds in the item from two positions to the left
    // - barrier
    // - Each work item adds in the item from four positions to the left
    // ...
    // At the end, each item has summed all prior items.

    // across psc_k groups, along local id
    // (uses out-of-bounds psc_k=psc_K array entry for storage)

    psc_scan_type psc_val = psc_ldata[psc_K][psc_LID_0].psc_value;

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 1) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 1].psc_value;
      if (psc_K * psc_LID_0 < psc_offset_end) {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 1]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 2) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 2].psc_value;
      if (psc_K * psc_LID_0 < psc_offset_end) {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 2]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    // {{{ reads from local allowed, writes to local not allowed

    if (psc_LID_0 >= 4) {
      psc_scan_type psc_tmp = psc_ldata[psc_K][psc_LID_0 - 4].psc_value;
      if (psc_K * psc_LID_0 < psc_offset_end) {
        psc_val =
            psc_SCAN_EXPR(psc_tmp, psc_val,
                          (psc_l_first_segment_start_in_subtree[psc_LID_0] !=
                           NO_SEG_BOUNDARY));
      }

      // Prepare for psc_l_first_segment_start_in_subtree, below.

      // Note that this update must take place *even* if we're
      // out of bounds.

      psc_first_segment_start_in_subtree =
          min(psc_l_first_segment_start_in_subtree[psc_LID_0],
              psc_l_first_segment_start_in_subtree[psc_LID_0 - 4]);
    } else {
      psc_first_segment_start_in_subtree =
          psc_l_first_segment_start_in_subtree[psc_LID_0];
    }

    // }}}

    local_barrier();

    // {{{ writes to local allowed, reads from local not allowed

    psc_ldata[psc_K][psc_LID_0].psc_value = psc_val;
    psc_l_first_segment_start_in_subtree[psc_LID_0] =
        psc_first_segment_start_in_subtree;

    // }}}

    local_barrier();

    pycl_printf(("after tree scan\n"));

    // }}}

    // {{{ update local values

    if (psc_LID_0 > 0) {
      psc_sum = psc_ldata[psc_K][psc_LID_0 - 1].psc_value;

      for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
        if (psc_K * psc_LID_0 + psc_k < psc_offset_end) {
          psc_scan_type psc_tmp = psc_ldata[psc_k][psc_LID_0].psc_value;
          psc_ldata[psc_k][psc_LID_0].psc_value =
              psc_SCAN_EXPR(psc_sum, psc_tmp,
                            (psc_unit_base + psc_K * psc_LID_0 + psc_k >=
                             psc_first_segment_start_in_k_group));
        }
      }
    }

    if (psc_LID_0 == 0) {
      // update interval-wide first-seg variable from current unit
      psc_first_segment_start_in_interval =
          min(psc_first_segment_start_in_interval,
              psc_l_first_segment_start_in_subtree[psc_WG_SIZE - 1]);
    }

    pycl_printf(("after local update\n"));

    // }}}

    local_barrier();

    // {{{ write data

    for (psc_index_type psc_k = 0; psc_k < psc_K; psc_k++) {
      const psc_index_type psc_offset = psc_k * psc_WG_SIZE + psc_LID_0;

      if (psc_unit_base + psc_offset < psc_interval_end) {
        pycl_printf(("write: %d\n", psc_unit_base + psc_offset));
        psc_partial_scan_buffer[psc_unit_base + psc_offset] =
            psc_ldata[psc_offset % psc_K][psc_offset / psc_K].psc_value;
      }
    }

    pycl_printf(("after write\n"));

    // }}}

    local_barrier();
  }

  // write interval psc_sum
  if (psc_LID_0 == 0) {
    psc_interval_results[psc_GID_0] =
        psc_partial_scan_buffer[psc_interval_end - 1];
    psc_g_first_segment_start_in_interval[psc_GID_0] =
        psc_first_segment_start_in_interval;
  }
}
