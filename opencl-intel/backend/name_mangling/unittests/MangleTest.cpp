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
//#include <iostream>
#include "antlr/ANTLRException.hpp"

namespace namemangling { namespace tests{

TEST(NameMangle, demangleTostrightAndBack){
  #include "MangledNames.h"
  for( unsigned int i = 0 ; i < sizeof(mangledNames)/sizeof(char*) ; i++)
    try{
      const char* mname = mangledNames[i];
      reflection::FunctionDescriptor fd = demangle(mname);
      std::string expected(mname);
      std::string actual = mangle(fd);
      ASSERT_EQ(expected, actual);
    } catch (std::exception e){
      std::cerr << e.what();
    } catch (antlr::ANTLRException antlrEx){
      std::cerr << "in: " << mangledNames[i] << std::endl;
      std::cerr << antlrEx.toString() << std::endl;
    }
}

static bool testDemangle(const char* mname){
  try{
    //std::cout << "demangling " << mname << std::endl; 
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

}
}//end namespace
