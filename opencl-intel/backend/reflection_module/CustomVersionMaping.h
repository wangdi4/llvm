#ifndef __CUSTOM_VERSION_MAPPING_H__
#define __CUSTOM_VERSION_MAPPING_H__

#include "FunctionDescriptor.h"
#include "utils.h"

#define INVALID_ENTRY "<invalid>"

TableRow mappings[] = {
  // {{scalar_version, width2_version, ..., width16_version, width3_version}, isScalarizable, isPacketizable}
  { {"allOne","allOne_v2","allOne_v4","allOne_v8","allOne_v16",INVALID_ENTRY}, false, true},
  { {"allZero","allZero_v2","allZero_v4","allZero_v8","allZero_v16",INVALID_ENTRY}, false, true},
  { {"get_global_id", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"get_global_size", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z18mask_write_imageuii9image2d_tDv2_iDv4_j", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imageii9image2d_tDv2_iDv4_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tjDv2_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_write_imagefi9image2d_tDv2_iDv4_f", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imagefi9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z16mask_read_imageii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
  { {"_Z17mask_read_imageuii9image2d_tDv2_i", INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY, INVALID_ENTRY}, false, false},
};
#endif//__CUSTOM_VERSION_MAPPING_H__
