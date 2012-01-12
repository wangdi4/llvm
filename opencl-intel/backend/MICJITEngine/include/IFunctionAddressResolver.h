//===-- IFunctionAddressResolver.h - Contains for module JIT ----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file defines interface for resolving the functions address which
// already loaded on the device
//
//===----------------------------------------------------------------------===//

#ifndef __IFUNCTION_ADDRESS_RESOLVER_H__
#define __IFUNCTION_ADDRESS_RESOLVER_H__

#include <string>

namespace llvm {

//===--------------------------------------------------------------------===//
// IFunctionAddressResolver - interface for resolving the function addresses 
//  which loaded on the device
//
class IFunctionAddressResolver {
public:
  /////////////////////////////////////////
  //Return: the address of the specified function on the device, 0 in error
  /////////////////////////////////////////
  virtual unsigned long long int getFunctionAddress(const std::string& func) 
    const = 0;
};

}

#endif //__IFUNCTION_ADDRESS_RESOLVER_H__
