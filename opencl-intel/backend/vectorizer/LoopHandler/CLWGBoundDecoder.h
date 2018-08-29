// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
