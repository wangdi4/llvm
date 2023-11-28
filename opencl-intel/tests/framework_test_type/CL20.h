// INTEL CONFIDENTIAL
//
// Copyright 2015 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __CL20_
#define __CL20_

#include "CL_BASE.h"

class CL20 : public ::CL_base {
protected:
  virtual void SetUp() override {
    CL_base::SetUp();
    ASSERT_LE(OPENCL_VERSION::OPENCL_VERSION_2_0, m_version)
        << "Test required OpenCL2.0 version at least";
  }
  virtual void TearDown() override { CL_base::TearDown(); }
};

#endif /*__CL20_*/
