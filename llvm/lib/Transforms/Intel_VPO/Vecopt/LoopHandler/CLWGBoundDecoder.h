/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __EE_DECODER_H__
#define __EE_DECODER_H__

#include <string>

namespace intel {
class CLWGBoundDecoder{
public:
  /// @brief C'tor
  CLWGBoundDecoder(){}
  /// @brief D'tor
  ~CLWGBoundDecoder(){}
 
  ///@brief returns the initial global id index for dimension dim in the
  ///       work group boundaries array.
  ///@param dim - dimension to query.
  ///@retruns as above.
  static unsigned getIndexOfInitGIDAtDim(unsigned dim);

  ///@brief returns the initial loop size index for dimension dim in the
  ///       work group boundaries array.
  ///@param dim - dimension to query.
  ///@retruns as above.
  static unsigned getIndexOfSizeAtDim(unsigned dim);

  ///@brief returns the unoform early exit index for dimension dim in the
  ///       work group boundaries array.
  ///@param dim - dimension to query.
  ///@retruns as above.
  static unsigned getUniformIndex();

  ///@brief returns the number of entries in the work group boundaries array.
  ///@param numDim - number of dimensions.
  ///@retruns as above.
  static unsigned getNumWGBoundArrayEntries(unsigned numDim);

  ///@brief returns the work group boundaries function name for the given
  ///       function name.
  ///@parma funcName - name of kernel to get work group boundaries function 
  ///       name for.
  ///@retruns as above.
  static std::string encodeWGBound(std::string &funcName);

  ///@brief returns true if this is an work group boundaries function name.
  ///@parma name - name to query.
  ///@retruns as above.
  static bool isWGBoundFunction(std::string& name);

private:

  ///@brief prefix of early exit functions.
  static const std::string WGBoundPrefix;
  
};
} // namespace


#endif //__EE_DECODER_H__
