/****************************************************************************
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

  File Name: MangleTest.cpp

\****************************************************************************/

#include <gtest/gtest.h>
#include "NameMangleAPI.h"
#include "antlr/ANTLRException.hpp"
#include "Type.h"
#include "FunctionDescriptor.h"
#include <algorithm>
#include <cctype>

namespace namemangling { namespace tests{

struct Range{
  //the beginning, and the end of the range (inclusively)
  size_t begin, end;
  Range(int b, int e): begin(b), end(e){}
  size_t length()const {return (1+end)-begin;}
};

//tokenize str, with respect to the given delimiters
std::vector<std::string> tokenize(const std::string& str,
  const std::string& delimiters){
  std::vector<std::string> tokens;
 // skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // find first "non-delimiter".
  std::string::size_type pos = str.find_first_of(delimiters, lastPos);
  while (std::string::npos != pos || std::string::npos != lastPos) {
    // found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
  return tokens;
}

//
//Finds the (first) range in which the given strings differ
//
static Range findRange(const std::string& l, const std::string& r){
  assert(l != r && "strings are the same");
  size_t b = 0, length = std::min(l.length(), r.length());
  size_t e = length;
  while (l[b] == r[b] && b < length-1)
    ++b;
  while (l[e] == r[e] && e > 0)
    --e;
  //advance to the first index in which the two strings has a whitespace
  while((!isspace(l[e]) || !isspace(r[e]) ) && e<length)
    ++e;
  assert (e >= b && "the string are suppose to be different shouldn't they?");
  return Range(b, e);
}

struct AttributeDetector{
  bool operator ()(const std::string& s)const{
    if ( "__local" == s )
      return true;
    if ( "__private" == s )
      return true;
    if ( "__global" == s )
      return true;
    if ( "__constant" == s )
      return true;
    if ( "volatile" == s )
      return true;
    if ( "const" == s )
      return true;
    if ( "restrict" == s )
      return true;
    return false;
  }
};

std::ostream& operator << (std::ostream& o, const std::vector<std::string>& v){
  size_t len = v.size();
  for(size_t i=0 ; i<len ; i++)
    o << v[i] << std::endl;
  return o;
}

//string decoration function
template <int size>
struct SizeT{
  static const char* nativeStr(){return "ulong";}
};

template <>
struct SizeT<32>{
  static const char* nativeStr(){return "uint";}
};

//
//deletes the _private attribute from the given string if it appears in it
//
static void deletePrivate(std::string& s){
  const std::string strPrivate = "__private ";
  size_t pos = s.find(strPrivate);
  while (std::string::npos !=  pos){
    s.erase(pos, strPrivate.length());
    pos = s.find(strPrivate);
  }
}

static void replaceSizeT(std::string& s){
  const std::string strSizet = "size_t";
  size_t pos = s.find(strSizet);
  while (std::string::npos !=  pos){
    s.erase(pos, strSizet.length());
    const std::string strSizeT(SizeT<sizeof(size_t)>::nativeStr());
    s.insert(pos, strSizeT);
    pos = s.find(strSizet);
  }
}

//returns true, if the following function prototypes are semantically the same
//(the itanium standard allows attributes to be order insensitive)
static bool isSematicallyEqual(const std::string& l, const std::string& r){
  if (l == r)
    return true;
  //we remove __private attribute, since it is the default address space,
  //therefore doesn't mangled to the string.
  std::string left(l), right(r);
  deletePrivate(left);
  deletePrivate(right);
  //replacing the size_t argument, since its a typedef, not a real type, and
  //clang treats it that way.
  replaceSizeT(left);
  replaceSizeT(right);
  //if they have different length at this point, they can't be semantically the same
  if (left.length() != right.length())
    return false;
  //after removing/replacing semantically equivalences, the strings might be
  //the same
  if (left == right)
    return true;
  //since the order of appearance of attribute isn't significant, we try to
  //rearrange them.
  Range range = findRange(left, right);
  std::string diff(left, range.begin, range.length());
  std::vector<std::string> tokens = tokenize(diff, " ");
  //if the entire diff string is attributes, they are semantically the same
  AttributeDetector isAttribute;
  size_t attributeCount = std::count_if(tokens.begin(), tokens.end(), isAttribute);
  return (attributeCount == tokens.size());
}

TEST(NameMangle, demangleTostrightAndBack){
  #include "MangledNames.h"
  for( unsigned int i = 0 ; i < sizeof(mangledNames)/sizeof(char*) ; i++){
    try{
      const char* mname = mangledNames[i];
      reflection::FunctionDescriptor fd = demangle(mname);
      std::string expected(mname);
      std::string actual = mangle(fd);
      //checking that the mangle demangle cycle returns to the same string
      ASSERT_EQ(expected, actual);
      //checking that the demangle string is as we expect it to be
      //(semantically the same)
      if (!isSematicallyEqual(fd.toString(), prototypes[i]))
        FAIL() << "\'" << fd.toString() << "\'" << " semantically differs from "
        << "\'" << prototypes[i] << "\'";
    } catch (std::exception e){
      std::cerr << e.what();
    } catch (antlr::ANTLRException antlrEx){
      std::cerr << "in: " << mangledNames[i] << std::endl;
      std::cerr << antlrEx.toString() << std::endl;
    }
  }
}

static bool testDemangle(const char* mname){
  try{
    printf( "%s\n", demangle(mname).toString().c_str());
    return true;
  } catch (antlr::ANTLRException antlrEx){
    return false;
  }
}

TEST(DemangleTest, addressSpace){
  ASSERT_TRUE( testDemangle( "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh"));
}


TEST(DemangleTest, pointerAttributes){
  const char* name = "_Z10mask_vloadtmPKU3AS2c";
  ASSERT_TRUE( testDemangle(name) );
}

TEST(DemangleTest, duplicateParam){
  const char* name = "_Z8abs_diffDv2_cS_";
  ASSERT_TRUE( testDemangle(name) );
}

TEST(DemangleTest, duplicateParam0){
  const char* name = "_Z10soa_cross3Dv16_fS_S_S_S_S_PS_S0_S0_";
  ASSERT_TRUE( testDemangle(name) );
}

TEST(MangleTest, _Z17convert_float_satc){
  std::string orig = "_Z17convert_float_satc";
  std::string syntesized = mangle( demangle( orig.c_str() ) );
  ASSERT_EQ(orig, syntesized);
}

//A special case, in which the param duplicate operator works to duplicate a
//type suffix, not the enture type. The first parameter is v2f, the second is v2f*.
TEST(MangleTest, semidup){
  std::string orig = "_Z5fractDv2_fPU3AS1S_";
  std::string actual = mangle( demangle( orig.c_str() ) );
  ASSERT_EQ(orig, actual);
}

TEST(MangleBasic, scalarfloat){
  reflection::Type primitiveFloat(reflection::primitives::FLOAT);
  reflection::FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(&primitiveFloat);
  ASSERT_STREQ("_Z3foof", mangle(fd).c_str());
}

TEST(MangleBasic, scalardouble){
  reflection::Type primitiveDouble(reflection::primitives::DOUBLE);
  reflection::FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(&primitiveDouble);
  ASSERT_STREQ("_Z3food", mangle(fd).c_str());
}

}
}//end namespace
