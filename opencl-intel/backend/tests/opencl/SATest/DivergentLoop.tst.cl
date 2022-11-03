typedef struct {
  bool complete;
} TRACKED_OBJECT_T;

kernel void
my_cc_cc_analysis(const unsigned short offset_y,
                  const unsigned int table_tile_stride,
                  const unsigned int fv_tile_stride,
                  __global unsigned short *restrict active_line_objects_count) {
  int line_labels_1[3];
  size_t thread_id = get_global_id(0);

  __global int *restrict eq_table = (__global int *restrict)table_tile_stride;
  __global TRACKED_OBJECT_T *restrict fv_table =
      (__global TRACKED_OBJECT_T *restrict)fv_tile_stride;

  unsigned short line_labels_1_count =
      (offset_y == 0) ? 0 : active_line_objects_count[thread_id];
  for (unsigned int l = 0; l < line_labels_1_count; ++l) {
    line_labels_1[l] = 0;
  }

  for (unsigned int l = 0; l < line_labels_1_count; ++l) {
    int obj_label = eq_table[line_labels_1[l]];
    if (!fv_table[obj_label].complete) {
      fv_table[obj_label].complete = true;
    }
  }
}
