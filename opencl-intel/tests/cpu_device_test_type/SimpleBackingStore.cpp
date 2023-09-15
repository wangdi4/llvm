// Copyright (c) 2006 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "SimpleBackingStore.h"
#include "cl_sys_defines.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
/// SimpleBackingStore
//////////////////////////////////////////////////////////////////////////
SimpleBackingStore::SimpleBackingStore(const cl_image_format *pclImageFormat,
                                       unsigned int dim_count,
                                       const size_t *dimensions,
                                       const size_t *pitches)
    : m_ptr(NULL), m_dim_count(dim_count), m_raw_data_size(0),
      m_refCount(0) // force autodelete on first release
{
  if (pclImageFormat) {
    m_format = *pclImageFormat;
  } else {
    memset(&m_format, 0, sizeof(m_format));
  }

  m_element_size = get_element_size(pclImageFormat);

  // caclulate pitches and dimensions
  assert(NULL != dimensions);
  calculate_pitches_and_dimentions(m_element_size, dim_count, dimensions,
                                   pitches, m_dimensions, m_pitches);

  // calc raw data size
  m_raw_data_size = calculate_size(m_element_size, (cl_uint)m_dim_count,
                                   m_dimensions, m_pitches);
  m_ptr = ALIGNED_MALLOC(m_raw_data_size, 128);
}

SimpleBackingStore::~SimpleBackingStore() {
  if (m_ptr) {
    ALIGNED_FREE(m_ptr);
  }
}

int SimpleBackingStore::AddPendency() { return m_refCount++; }

int SimpleBackingStore::RemovePendency() {
  long prevVal = m_refCount--;
  if (1 == prevVal) {
    delete this;
  }
  return prevVal;
}

size_t SimpleBackingStore::get_element_size(const cl_image_format *format) {
  if (NULL == format) {
    return 1;
  }

  size_t stChannels = 0;
  size_t stChSize = 0;
  switch (format->image_channel_order) {
  case CL_R:
  case CL_A:
  case CL_LUMINANCE:
  case CL_INTENSITY:
  case CL_RGB: // Special case, must be used only with specific data type
    stChannels = 1;
    break;
  case CL_RG:
  case CL_RA:
    stChannels = 2;
    break;
  case CL_RGBA:
  case CL_ARGB:
  case CL_BGRA:
    stChannels = 4;
    break;
  default:
    assert(0);
  }
  switch (format->image_channel_data_type) {
  case (CL_SNORM_INT8):
  case (CL_UNORM_INT8):
  case (CL_SIGNED_INT8):
  case (CL_UNSIGNED_INT8):
    stChSize = 1;
    break;
  case (CL_SNORM_INT16):
  case (CL_UNORM_INT16):
  case (CL_UNSIGNED_INT16):
  case (CL_SIGNED_INT16):
  case (CL_HALF_FLOAT):
  case CL_UNORM_SHORT_555:
  case CL_UNORM_SHORT_565:
    stChSize = 2;
    break;
  case (CL_SIGNED_INT32):
  case (CL_UNSIGNED_INT32):
  case (CL_FLOAT):
  case CL_UNORM_INT_101010:
    stChSize = 4;
    break;
  default:
    assert(0);
  }

  return stChannels * stChSize;
}

size_t SimpleBackingStore::calculate_offset(size_t elem_size,
                                            unsigned int dim_count,
                                            const size_t origin[],
                                            const size_t pitches[]) {
  size_t offset = elem_size * origin[0];
  for (size_t i = 1; i < dim_count; ++i) {
    offset += pitches[i - 1] * origin[i];
  }
  return offset;
}

void SimpleBackingStore::calculate_pitches_and_dimentions(
    size_t elem_size, unsigned int dim_count, const size_t user_dims[],
    const size_t user_pitches[], size_t dimensions[], size_t pitches[]) {
  assert(NULL != user_dims);

  memset(dimensions, 0, sizeof(dimensions[0]) * MAX_WORK_DIM);
  memset(pitches, 0, sizeof(pitches[0]) * (MAX_WORK_DIM - 1));

  dimensions[0] = user_dims[0];
  size_t prev_pitch = elem_size;

  for (cl_uint i = 1; i < dim_count; ++i) {
    dimensions[i] = user_dims[i];
    size_t next_pitch = dimensions[i - 1] * prev_pitch;

    if ((NULL != user_pitches) && (user_pitches[i - 1] > next_pitch)) {
      next_pitch = user_pitches[i - 1];
    }

    pitches[i - 1] = next_pitch;
    prev_pitch = next_pitch;
  }
}

size_t SimpleBackingStore::calculate_size(size_t elem_size,
                                          unsigned int dim_count,
                                          const size_t dimensions[],
                                          const size_t pitches[]) {
  size_t tmp_dims[MAX_WORK_DIM];
  size_t tmp_pitches[MAX_WORK_DIM - 1];

  const size_t *working_dims = dimensions;
  const size_t *working_pitches = pitches;

  if ((NULL == pitches) || ((dim_count > 1) && (0 == pitches[0]))) {
    calculate_pitches_and_dimentions(elem_size, dim_count, dimensions, pitches,
                                     tmp_dims, tmp_pitches);
    working_dims = tmp_dims;
    working_pitches = tmp_pitches;
  }

  return (1 == dim_count)
             ? working_dims[0] * elem_size
             : working_pitches[dim_count - 2] * working_dims[dim_count - 1];
}

size_t SimpleBackingStore::GetRawDataOffset(const size_t *origin) const {
  return calculate_offset(m_element_size, (cl_uint)m_dim_count, origin,
                          m_pitches);
}
