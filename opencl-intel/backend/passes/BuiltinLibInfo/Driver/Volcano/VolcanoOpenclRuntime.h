/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

#ifndef __VolcanoOpenclRuntime_H_
#define __VolcanoOpenclRuntime_H_

#include "OpenclRuntime.h"

namespace intel {
  
  extern const char *volacanoScalarSelect[];
  
  class VolcanoOpenclRuntime : public OpenclRuntime {
  public:
    /// @brief Constructor
    VolcanoOpenclRuntime(const Module *runtimeModule);
    
    /// @brief Destructor
    ~VolcanoOpenclRuntime() {}
    
    VolcanoOpenclRuntime(); // Do not implement
    
    /// @brief returns true the function needs to be replaced with fake function
    ///   used by OCLBuiltinPreVectorizationPass - currently disabled in Volcano
    /// @param funcName Function name to check
    virtual bool needPreVectorizationFakeFunction(const std::string &funcName) const;
    
    /// @brief returns true the function is writeImage which need scalarizing of
    ///  input used by OCLBuiltinPreVectorizationPass - currently disabled in
    ///  Volcano
    /// @param funcName Function name to check
    virtual bool isWriteImage(const std::string &funcName) const;
    
    /// @brief returns true the function is fake writeImage which produced in
    /// Pre-Scalarization used AppleWiDepPrePacketizationPass
    /// @param funcName Function name to check
    virtual bool isFakeWriteImage(const std::string &funcName) const;
    
    /// @brief returns true iff this is name of transposed_read_image.
    /// @param funcName Function name to check
    virtual bool isTransposedReadImg(const std::string &func_name) const;
    
    /// @brief returns true iff this is name of transposed_write_image.
    /// @param funcName Function name to check
    virtual bool isTransposedWriteImg(const std::string &func_name) const;
    
    /// @brief returns the read stream function from the runtime module.
    /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
    virtual Function *getReadStream(bool isPointer64Bit) const;
    
    /// @brief returns the write stream function from the runtime module.
    /// @param isPointer64Bit true if pointer size is 64bit, false otherwise.
    virtual Function *getWriteStream(bool isPointer64Bit) const;
    
    // @brief return true if this name of stream built-in.
    virtual bool isStreamFunc(const std::string &funcName) const;
  };
  
}

#endif