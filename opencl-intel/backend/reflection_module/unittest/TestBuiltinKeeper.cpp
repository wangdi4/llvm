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
#include <utility>
#include "utils.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
#include "BuiltinKeeper.h"
#include "FunctionDescriptor.h"
#include "NameMangleAPI.h"
#include "OpenclRuntime.h"

using namespace reflection;

namespace reflection { namespace tests{
static int toInt(const std::pair<int,int>& p){
  return 2*p.first + p.second;
}

TEST(CartesianProduc, twoWays){
  typedef llvm::ArrayRef<int> ivector;
  int a1[] = {0,1};
  ivector v1(a1);
  ivector v2(a1);
  Cartesian<llvm::ArrayRef, int, int> cartesian(v1, v2);
  std::vector<std::pair<int,int> > res;
  int i = 0;
  do{
    std::pair<int,int> p = cartesian.get();
    ASSERT_EQ(i++, toInt(p));
  }while (cartesian.next());
}

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
  ASSERT_FALSE(BuiltinKeeper::instance()->isBuiltin(
    "this is not a built-in function"));
}

TEST(isBuiltin, unmangledBuiltins){
  const char* allZero = "__ocl_allZero";
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  ASSERT_TRUE(pKeeper->isBuiltin(allZero));
  PairSW sw = pKeeper->getVersion(allZero, width::TWO);
  ASSERT_EQ(std::string("__ocl_allZero_v2"), sw.first);
  ASSERT_EQ(width::TWO, sw.second);
  ASSERT_TRUE(pKeeper->isBuiltin("get_global_id"));
  ASSERT_TRUE(pKeeper->isBuiltin("get_global_size"));
}

TEST(getWidth, fromScalar){
  PairSW v2 = BuiltinKeeper::instance()->getVersion("_Z4fabsf", width::TWO);
  ASSERT_TRUE(v2.second == width::TWO);
}

TEST(getWidth, nonBuiltin){
  try{
    BuiltinKeeper::instance()->getVersion("_Z2bof", width::TWO);
  } catch(BuiltinKeeperException){
    SUCCEED();
    return;
  }
  FAIL() << "exception wasn't caught";
}

static width::V indexToWidth(size_t index){
  width::V allWidth[] = {width::SCALAR, width::TWO, width::FOUR, width::EIGHT,
  width::SIXTEEN, width::THREE};
  assert(index < width::OCL_VERSIONS);
  return allWidth[index];
}

//an array of function names, that can be packetized, but not scalarized
llvm::StringRef ambigNames[] = {"fmax", "fmin", "fmod", "ldexp", "min", "max",
"clamp", "mix", "step", "smoothstep"};
llvm::ArrayRef<llvm::StringRef> arrAmbiguous( ambigNames );

//Uses the old (hard-coded) table of the vectorizer, to test that the properties
//of all the function descriptors are maintained.
//This test should validate the code after the BuiltinKeeper integration with
//the vectorizer.
TEST(VectorizerReference, OldTable){
#include "VectorizerReference.h"
  const size_t numFunctionEntries =
    sizeof(functionEntries)/(width::OCL_VERSIONS*sizeof(char*)),
  numPropertiesEntries = sizeof(tableProperties)/(NUM_PROPERTIES*sizeof(bool));
  ASSERT_EQ(numFunctionEntries, numPropertiesEntries);
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  for (size_t rowindex=0 ; rowindex<numFunctionEntries ; ++rowindex){
    for (size_t colindex=0 ; colindex<width::OCL_VERSIONS ; ++colindex){
      const char* biName = functionEntries[rowindex][colindex];
      if (biName && !pKeeper->isBuiltin(biName)){
        //we skip those, since they are not first class citizens (they are not
        //built-in functions), thus may not be demangled
        continue;
      }
      if (!biName) //not interested in processing NULL entries.
        continue;
      //Do the two versions agree on the scalarization property?
      width::V expectedWidth = indexToWidth(colindex);
      PairSW scalarVersion = pKeeper->getVersion(biName, width::SCALAR);
      if (expectedWidth != width::SCALAR)
        ASSERT_EQ(0==tableProperties[rowindex][0], isNullPair(scalarVersion)) <<
        "in built-in " << biName;
      //Do the two versions agree on the packetizer property?
      //We arbitrary picked four as the destination vector width.
      bool isNull = isNullPair(pKeeper->getVersion(biName, width::FOUR));
      //the scalar versions of some builtin functions have ambiguity on that
      //property, and was solved by the vectorizer with hard-coded ifs.
      PairSW versionedFd = pKeeper->getVersion(biName, expectedWidth);
      width::V actualWidth = versionedFd.second;
      FunctionDescriptor fd = demangle(biName);
      bool isAmbiguous =
      std::find(arrAmbiguous.begin(), arrAmbiguous.end(), fd.name) != arrAmbiguous.end();
      if (width::SCALAR == expectedWidth && isAmbiguous){
        //ambiguous scalar built-ins should always be vectorized
        ASSERT_FALSE(isNull) << "in built-in: " << biName;
        continue;
      }
      ASSERT_TRUE (tableProperties[rowindex][1] == !isNull) <<
      "packetization property failure: mangled name " << biName;
      //
      //Width check
      //
      if ( !isNullPair(versionedFd) )
        ASSERT_EQ(expectedWidth, actualWidth) << "in builtin " << biName;
      else
        ASSERT_TRUE(isAmbiguous) <<
        " NULL function Descriptor in non ambiguous built-in " << biName
        << " width expectedWidth=" << expectedWidth;
    }
  }
}//end TEST

TEST(VectorizerReference, syntesizedFunctions){
  const BuiltinKeeper* pKepper = BuiltinKeeper::instance();
  PairSW allonev = pKepper->getVersion("__ocl_allOne", width::TWO);
  ASSERT_EQ("__ocl_allOne_v2", allonev.first);
  allonev = pKepper->getVersion("__ocl_allOne", width::FOUR);
  ASSERT_EQ("__ocl_allOne_v4", allonev.first);
  allonev = pKepper->getVersion("__ocl_allOne", width::EIGHT);
  ASSERT_EQ("__ocl_allOne_v8", allonev.first);
  allonev = pKepper->getVersion("__ocl_allOne", width::SIXTEEN);
  ASSERT_EQ("__ocl_allOne_v16", allonev.first);
  allonev = pKepper->getVersion("__ocl_allOne", width::THREE);
  ASSERT_TRUE(isNullPair(allonev));

  PairSW allzerov = pKepper->getVersion("__ocl_allZero", width::TWO);
  ASSERT_EQ("__ocl_allZero_v2", allzerov.first);
  allzerov = pKepper->getVersion("__ocl_allZero", width::FOUR);
  ASSERT_EQ("__ocl_allZero_v4", allzerov.first);
  allzerov = pKepper->getVersion("__ocl_allZero", width::EIGHT);
  ASSERT_EQ("__ocl_allZero_v8", allzerov.first);
  allzerov = pKepper->getVersion("__ocl_allZero", width::SIXTEEN);
  ASSERT_EQ("__ocl_allZero_v16", allzerov.first);
}

//BuiltinKepper functionality tests
TEST(Functionality, duplicatedScalarEntry){
  const char*const sincosToscalarize = "_Z18_retbyarray_sincosDv4_d";
  const char*const sincosScalarVersion = "_Z17sincos_scalarizedd";
  const char*const sincosExpectedVectorized = "_Z14sincos_ret2ptrDv4_dPS_S0_";
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  PairSW scalarVersion = pKeeper->getVersion(sincosToscalarize, width::SCALAR);
  ASSERT_STREQ(scalarVersion.first.c_str(), sincosScalarVersion);
  PairSW v4Version = pKeeper->getVersion(sincosScalarVersion, width::FOUR);
  ASSERT_STREQ(sincosExpectedVectorized, v4Version.first.c_str());
}


//
//Bi Transpose test
//
TEST(FDTranspose, vectorReturnTyFunctionality){
  FunctionDescriptor cross4 = demangle("_Z5crossDv4_dS_");
  Type doubleTy(primitives::DOUBLE);
  Vector double4Ty(&doubleTy, 4);
  ReturnTypeMap retMap;
  retMap[cross4] = &double4Ty;

  SoaDescriptorStrategy soaStrategy;
  soaStrategy.setTypeMap(&retMap);
  PairSW pair = soaStrategy(PairSW(std::make_pair("_Z5crossDv4_dS_", width::FOUR)));
  ASSERT_STREQ("_Z10soa_cross4Dv4_dS_S_S_S_S_S_S_PS_S0_S0_S0_", pair.first.c_str());
  ASSERT_EQ(width::FOUR, pair.second);
}

TEST(FDTranspose, scalarReturnTyFunctionality){
  Type doubleTy(primitives::DOUBLE);
  FunctionDescriptor dot = demangle("_Z3dotdd");
  ReturnTypeMap retMap;
  retMap[dot] = &doubleTy;
  SoaDescriptorStrategy soaStrategy;
  soaStrategy.setTypeMap(&retMap);

  PairSW pair(std::make_pair("_Z3dotdd", width::FOUR));
  pair = soaStrategy(pair);
  ASSERT_STREQ("_Z8soa_dot1Dv4_dS_", pair.first.c_str());
}

//
//Tests the glue code between one of the dirver of builtin-keeper (in this
//case the OpenCL vectorizer) to the Builtin-keeper
//
TEST(DriverUse, oclVectorizer){
  //
  //parse the test ir file into a module
  //
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  std::string biname = "_Z3mixddd";
  std::auto_ptr<intel::VectorizerFunction> pFunction =
    pRuntime->findBuiltinFunction(biname);
  ASSERT_EQ(1U, pFunction->getWidth());
  ASSERT_STREQ("_Z3mixDv4_dS_S_", pFunction->getVersion(2).c_str());
  ASSERT_TRUE(pFunction->isPacketizable());
  ASSERT_TRUE(pFunction->isScalarizable());
  delete pRuntime;
}

//
//The width of function which have SOA conversion is tricky, not the usual
//algorithm
//
TEST(DriverUse, soaDescriptorsWidth){
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  std::string biname = "_Z6lengthDv2_f";
  std::auto_ptr<intel::VectorizerFunction> pFunction =
    pRuntime->findBuiltinFunction(biname);
  ASSERT_EQ(1U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv4_fS_";
  pFunction =  pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(4U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv8_fS_";
  pFunction =  pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(8U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv16_fS_";
  pFunction =  pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(16U, pFunction->getWidth());

  delete pRuntime;
}

TEST(DriverUse, nonVersioned){
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);

  std::string fract = "_Z5fractDv16_fPU3AS1S_";
  std::auto_ptr<intel::VectorizerFunction> pFunction =
    pRuntime->findBuiltinFunction(fract);
  ASSERT_FALSE(pFunction->isPacketizable());
  ASSERT_FALSE(pFunction->isScalarizable());
}

TEST(DriverUse, soaVersion3){
  std::string scalarFunction = "_Z5crossDv3_fS_";
  std::string vectorFunction = "_Z10soa_cross3Dv4_fS_S_S_S_S_PS_S0_S0_";
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  const std::auto_ptr<intel::VectorizerFunction> foundFunction =
    pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(DriverUse, soaVersion4){
  std::string scalarFunction = "_Z5crossDv4_dS_";
  std::string vectorFunction = "_Z10soa_cross4Dv4_dS_S_S_S_S_S_S_PS_S0_S0_S0_";
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  const std::auto_ptr<intel::VectorizerFunction> foundFunction =
    pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(DriverUse, normalize){
  std::string scalarFunction = "_Z9normalizeDv4_f";
  std::string vectorFunction = "_Z14soa_normalize4Dv4_fS_S_S_PS_S0_S0_S0_";
  llvm::LLVMContext context;
  llvm::Module* pModule = NULL;
  llvm::SMDiagnostic errDiagnostic;
  pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  const std::auto_ptr<intel::VectorizerFunction> foundFunction =
    pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(GenTest, soaGenTest){
  std::string scalarVersion = "_Z7isequalff";
  std::string strExpected   = "_Z7isequalDv4_fS_";
  llvm::LLVMContext context;
  llvm::SMDiagnostic errDiagnostic;
  llvm::Module* pModule = llvm::ParseIRFile("mybi.ll", errDiagnostic, context);
  intel::RuntimeServices* pRuntime = new intel::OpenclRuntime(pModule);
  const std::auto_ptr<intel::VectorizerFunction> pFunc =
    pRuntime->findBuiltinFunction(scalarVersion);
  std::string strActual = pFunc->getVersion(2);
  ASSERT_EQ(strActual, strActual);
  delete pRuntime;
}

}}

int main(int argc, char** argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

