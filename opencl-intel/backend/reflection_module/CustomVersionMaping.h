#ifndef __CUSTOM_VERSION_MAPPING_H__
#define __CUSTOM_VERSION_MAPPING_H__

#include "FunctionDescriptor.h"
#include "utils.h"


TableRow mappings[] = {
  // {{scalar_version, width2_version, ..., width16_version, width3_version}, isScalarizable, isPacketizable}
  { {"allOne","allOne_v2","allOne_v4","allOne_v8","allOne_v16",INVALID_ENTRY}, false, true},
  { {"allZero","allZero_v2","allZero_v4","allZero_v8","allZero_v16",INVALID_ENTRY}, false, true},
  { {"get_global_id", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"get_global_size", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z18mask_write_imageuii9image2d_tDv2_iDv4_j", INVALID_ENTRY, "_Z23mask_soa4_write_imageuiDv4_i9image2d_tS_S_Dv4_jS0_S0_S0_", "_Z23mask_soa8_write_imageuiDv8_i9image2d_tS_S_Dv8_jS0_S0_S0_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  { {"_Z16mask_read_imageii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imageii9image2d_tDv2_iDv4_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imagefi9image2d_tDv2_iDv4_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  // Image properties functions
  {{"_Z16get_image_height9image2d_t", INVALID_ENTRY, "_Z21soa4_get_image_height9image2d_t", "_Z21soa8_get_image_height9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image3d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image3d_t", "_Z20soa8_get_image_width9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image3d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image3d_t", "_Z28soa8_get_image_channel_order9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width15image2d_array_t", INVALID_ENTRY, "_Z20soa4_get_image_width15image2d_array_t", "_Z20soa8_get_image_width15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image2d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image2d_t", "_Z20soa8_get_image_width9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type15image2d_array_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type15image2d_array_t", "_Z32soa8_get_image_channel_data_type15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image3d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image3d_t", "_Z32soa8_get_image_channel_data_type9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image1d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image1d_t", "_Z32soa8_get_image_channel_data_type9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z16get_image_height9image3d_t", INVALID_ENTRY, "_Z21soa4_get_image_height9image3d_t", "_Z21soa8_get_image_height9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_depth9image3d_t", INVALID_ENTRY, "_Z20soa4_get_image_depth9image3d_t", "_Z20soa8_get_image_depth9image3d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width9image1d_t", INVALID_ENTRY, "_Z20soa4_get_image_width9image1d_t", "_Z20soa8_get_image_width9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order16image1d_buffer_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order16image1d_buffer_t", "_Z28soa8_get_image_channel_order16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type9image2d_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type9image2d_t", "_Z32soa8_get_image_channel_data_type9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type15image1d_array_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type15image1d_array_t", "_Z32soa8_get_image_channel_data_type15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image2d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image2d_t", "_Z28soa8_get_image_channel_order9image2d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width15image1d_array_t", INVALID_ENTRY, "_Z20soa4_get_image_width15image1d_array_t", "_Z20soa8_get_image_width15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order9image1d_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order9image1d_t", "_Z28soa8_get_image_channel_order9image1d_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z15get_image_width16image1d_buffer_t", INVALID_ENTRY, "_Z20soa4_get_image_width16image1d_buffer_t", "_Z20soa8_get_image_width16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order15image1d_array_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order15image1d_array_t", "_Z28soa8_get_image_channel_order15image1d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z16get_image_height15image2d_array_t", INVALID_ENTRY, "_Z21soa4_get_image_height15image2d_array_t", "_Z21soa8_get_image_height15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z23get_image_channel_order15image2d_array_t", INVALID_ENTRY, "_Z28soa4_get_image_channel_order15image2d_array_t", "_Z28soa8_get_image_channel_order15image2d_array_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  {{"_Z27get_image_channel_data_type16image1d_buffer_t", INVALID_ENTRY, "_Z32soa4_get_image_channel_data_type16image1d_buffer_t", "_Z32soa8_get_image_channel_data_type16image1d_buffer_t", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  //// Image reading builtins
  {{"_Z12read_imageui9image2d_tjDv2_i", INVALID_ENTRY, "_Z17soa4_read_imageui9image2d_tjDv4_iS_PDv4_jS1_S1_S1_", "_Z17soa8_read_imageui9image2d_tjDv8_iS_PDv8_jS1_S1_S1_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
  //// Image writing builtins
  {{"_Z13write_imageui9image2d_tDv2_iDv4_j", INVALID_ENTRY, "_Z18soa4_write_imageui9image2d_tDv4_iS_Dv4_jS0_S0_S0_", "_Z18soa8_write_imageui9image2d_tDv8_iS_Dv8_jS0_S0_S0_", INVALID_ENTRY, INVALID_ENTRY}, false, true},
//this file is automatically gnerated by tblgen. It is strongly recmommended to use this mechanism, not to write string hardcoded
#include "CustomMappings.gen"
};
#endif//__CUSTOM_VERSION_MAPPING_H__
