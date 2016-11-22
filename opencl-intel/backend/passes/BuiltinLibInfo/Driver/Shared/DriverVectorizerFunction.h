/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __DRIVER_VECTORIZER_FUNCTION_H__
#define __DRIVER_VECTORIZER_FUNCTION_H__

#include "VectorizerFunction.h"

namespace intel {

class DriverVectorizerFunction : public VectorizerFunction {
public:
  DriverVectorizerFunction(const std::string& s);

  ~DriverVectorizerFunction();

  unsigned getWidth() const;

  bool isPacketizable() const;

  bool isScalarizable() const;

  std::string getVersion(unsigned index) const;

  bool isNull() const;

private:
  bool isMangled() const;

  const std::string m_name;
};

} // namespace intel

#endif // __DRIVER_VECTORIZER_FUNCTION_H__
