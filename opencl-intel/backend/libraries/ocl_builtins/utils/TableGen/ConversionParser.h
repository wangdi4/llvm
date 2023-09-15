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

#include <assert.h>
#include <string>

namespace reflection {

enum State { PREFIX, TYPENAME, TYPELEN, POSTFIX, STATES_NUM };

class ParserState;

////////////////////////////////////////////////////////////////////////////////
// Purpose: Parses the name of a given conversion function to it components:
// conversion primitive type, conversion type width, and conversion suffix.
// This is useful for cases in which there is a need to sort conversion
// functions, so we could keep a conversion family grouped together.
////////////////////////////////////////////////////////////////////////////////
class ConversionDescriptor {
public:
  ConversionDescriptor(const std::string &s);

  //////////////////////////////////////////////////////////////////////////////
  // Purpose: compares the receiver object of this method to the given object.
  // Return: -1 if the receiver is a predecessor the given object, 1 if it is a
  // successor, or zero if they are equal.
  //////////////////////////////////////////////////////////////////////////////
  int compare(const ConversionDescriptor &) const;

private:
  void parse(const std::string &s);

  std::string m_conversionTy;
  std::string m_postfix;
  size_t m_width;
}; // End ConversionDescriptor

} // namespace reflection
