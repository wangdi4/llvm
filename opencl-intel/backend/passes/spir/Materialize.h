/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __MATERIALIZE_H__
#define __MATERIALIZE_H__

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace intel {

// @breif Supplies 'null' value to value types.
template <typename T>
struct Maybe {
  static Maybe<T> null() { return Maybe<T>(); }

  // @brief Returns true if the object represents a 'null' value.
  bool isNull() const { return m_isNull; }

  // @brief Returns the value assigned to this object.
  const T& value() const { return m_value; }

  // @brief Returns the value assigned to this object.
  T& value() { return m_value; }

  // @brief Constructs a valid (data holding) object.
  Maybe(const T& val) : m_value(val), m_isNull(false) {}
private:
  // @brief Constructs a 'null-representing' object.
  Maybe() : m_isNull(true) {}

  // @brief the value assigned to the object
  T m_value;
  // @brief signals whether this object is a 'null' object of not.
  bool m_isNull;
};

///////////////////////////////////////////////////////////////////////////////
// @name  SpirMaterializer
// @brief Adjusts the given module to be processed by the BE.
// (More concretely, replaces SPIR artifacts with Intel-implementation
// specific stuff.
///////////////////////////////////////////////////////////////////////////////
class SpirMaterializer : public llvm::ModulePass{

public:
  static char ID;

  SpirMaterializer();

  const char* getPassName()const;

  bool runOnModule(llvm::Module&);
  
  // @breif returns a 'maybe' string with a valid value if the given module
  // is cross-compiled, and a 'null' value to indicate the module is compiled
  // for the host machine.
  static Maybe<std::string> getCrossTriple(llvm::Module&);
};

}

#endif//__MATERIALIZE_H__
