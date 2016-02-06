/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/SetVector.h"
#include <vector>
#include <map>

namespace llvm {
  class CallInst;
  class Function;
  class Module;
  class Value;
}
using namespace llvm;

namespace Intel {

  /// @brief  CompilationUtils class used to provide helper utilies that are
  ///         used by several other classes.
  class CompilationUtils {

  public:

    /// We use a SetVector to ensure determinstic iterations
    typedef SetVector<Function*> FunctionSet;
 
    static const std::string NAME_GET_GID;
    static const std::string NAME_GET_GLOBAL_SIZE;
    static const std::string NAME_GET_GLOBAL_OFFSET;

    static const std::string NAME_GET_BASE_GID;

    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGID();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalSize();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_offset
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalOffset();

    static bool isGetGlobalId(const std::string&);
    static bool isGetGlobalSize(const std::string&);
    static bool isGlobalOffset(const std::string&);
   };

  //
  // Base class for all functors, which supports immutability query.
  //

  class AbstractFunctor{
  protected:
    bool m_isChanged;
  public:
    AbstractFunctor(): m_isChanged(false){}

    virtual ~AbstractFunctor() {}

    bool isChanged()const{
      return m_isChanged;
    }
  };

  class FunctionFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::Function&) = 0;
  };

  class BlockFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::BasicBlock&) = 0;
  };

} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __COMPILATION_UTILS_H__
