// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __DRIVER_VECTORIZER_FUNCTION_H__
#define __DRIVER_VECTORIZER_FUNCTION_H__

#include "VectorizerFunction.h"

namespace intel {

class DriverVectorizerFunction : public VectorizerFunction {
public:
  DriverVectorizerFunction(const std::string &s);

  ~DriverVectorizerFunction();

  unsigned getWidth() const override;

  bool isPacketizable() const override;

  bool isScalarizable() const override;

  std::string getVersion(unsigned index) const override;

  bool isNull() const override;

private:
  bool isMangled() const;

  const std::string m_name;
};

} // namespace intel

#endif // __DRIVER_VECTORIZER_FUNCTION_H__
