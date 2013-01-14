/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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
