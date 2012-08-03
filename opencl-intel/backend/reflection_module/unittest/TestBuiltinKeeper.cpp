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

  File Name: TestBuiltinKeeper.cpp

\****************************************************************************/

#include <gtest/gtest.h>
#include "BuiltinKeeper.h"
#include "FunctionDescriptor.h"

using namespace reflection;

namespace reflection { namespace tests{

TEST(isBuiltin, positiveFirstInCache){
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clampccc"));
}

TEST(isBuiltin, cacheTest){
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clamphhh"));
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clamphhh"));
}

TEST(isBuiltin, positiveTest){
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clampDv2_cS_S_"));
}

//this bi is among the last, so testing it will indicate for the worst lookup
//time
TEST(isBuiltin, addressSpaceTest){
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin(
    "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh")
  );
}

//cache hits should make this test run almost as fast as the previous one
TEST(isBuiltin, performanceTest){
  for (int i=0 ; i<10 ; ++i)
    ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin(
      "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh")
    );
}

TEST(isBuiltin, negativeTest){
  ASSERT_FALSE(BuiltinKeeper::instance()->isBuiltin("_Z3foof"));
}

TEST(isBuiltin, invalidInput){
  try {
    ASSERT_FALSE(BuiltinKeeper::instance()->isBuiltin(
      "this is not a built-in function")
    );
  }catch(...){
    SUCCEED();
    return;
  }
  FAIL() << "exception was not caught";
}

TEST(getWidth, fromScalar){
  FunctionDescriptor fd = BuiltinKeeper::instance()->getVersion(
    "_Z4fabsf", width::TWO);
  ASSERT_TRUE(fd.getWidth() == width::TWO);
  ASSERT_EQ(std::string("fabs(float2)"), fd.toString());
}

TEST(getWidth, nonBuiltin){
  try{
    FunctionDescriptor fd = BuiltinKeeper::instance()->getVersion(
      "_Z2bof", width::TWO);
  } catch(BuiltinKeeperException){
    SUCCEED();
    return;
  }
  FAIL() << "exception wasn't caught";
}

}}
