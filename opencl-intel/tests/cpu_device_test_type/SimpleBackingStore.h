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

#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_memory_object.h
//  Implementation of the Class MemoryObject
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_device_api.h"
class SimpleBackingStore : public IOCLDevBackingStore {
public:
  SimpleBackingStore(const cl_image_format *pclImageFormat,
                     unsigned int dim_count, const size_t *dimension,
                     const size_t *pitches);

  void *GetRawData() const { return m_ptr; }
  size_t GetRawDataSize() const { return m_raw_data_size; }
  size_t GetRawDataOffset(const size_t *origin) const;

  size_t GetDimCount() const { return m_dim_count; }
  const size_t *GetDimentions() const { return m_dimensions; }
  bool IsDataValid() const { return true; }
  void SetDataValid(bool value) {}
  const size_t *GetPitch() const { return m_pitches; }
  const cl_image_format &GetFormat() const { return m_format; }
  size_t GetElementSize() const { return m_element_size; }
  void *GetUserProvidedHostMapPtr() const { return m_ptr; }

  int AddPendency();
  int RemovePendency();

  // helper function to calculate offsets
  static size_t calculate_offset(size_t elem_size, unsigned int dim_count,
                                 const size_t origin[], const size_t pitches[]);
  static size_t get_element_size(const cl_image_format *format);
  static size_t calculate_size(size_t elem_size, unsigned int dim_count,
                               const size_t dimensions[],
                               const size_t pitches[]);

private:
  virtual ~SimpleBackingStore();

  static void calculate_pitches_and_dimentions(
      size_t elem_size, unsigned int dim_count, const size_t user_dims[],
      const size_t user_pitches[], size_t dimensions[], size_t pitches[]);

  void *m_ptr;

  size_t m_dim_count;
  size_t m_dimensions[MAX_WORK_DIM];
  size_t m_pitches[MAX_WORK_DIM - 1];
  cl_image_format m_format;
  size_t m_element_size;

  size_t m_raw_data_size;

  unsigned int m_refCount;
};
