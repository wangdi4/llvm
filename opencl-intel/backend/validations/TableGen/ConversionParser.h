/******************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OclBuiltinsHeaderGen.h
\*****************************************************************************/

#include <assert.h>
#include <string>
#include <assert.h>

namespace reflection{

enum State{PREFIX, TYPENAME, TYPELEN, POSTFIX, STATES_NUM};

class ParserState;

////////////////////////////////////////////////////////////////////////////////
//Purpose: Parses the name of a given conversion function to it components:
//conversion primitive type, conversion type width, and conversion suffix.
//This is useful for cases in which there is a need to sort conversion
//functions, so we could keep a conversion family grouped together.
////////////////////////////////////////////////////////////////////////////////
class ConversionDescriptor{
public:
  ConversionDescriptor(const std::string& s);

  //////////////////////////////////////////////////////////////////////////////
  //Purpose: compares the receiver object of this method to the given object.
  //Return: -1 if the receiver is a predecessor the given object, 1 if it is a
  //successor, or zero if they are equal.
  //////////////////////////////////////////////////////////////////////////////
  int compare(const ConversionDescriptor&)const;

private:
  void parse(const std::string& s);

  std::string m_conversionTy;
  std::string m_postfix;
  size_t m_width;
};//End ConversionDescriptor

}//end reflection
