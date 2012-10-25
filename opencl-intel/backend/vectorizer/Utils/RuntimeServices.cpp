/*********************************************************************************************
 * Copyright � 2010-2012, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

#include "RuntimeServices.h"

namespace intel {

void RuntimeServices::set(RuntimeServices * obj) {
  m_singleton = obj;
}

RuntimeServices * RuntimeServices::get() {
  return m_singleton;
}

bool RuntimeServices::isFakedFunction(StringRef)const{
  return false;
}

RuntimeServices * RuntimeServices::m_singleton = NULL;

} // namespace
