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

#include "BuiltinKeeper.h"
#include "VolcanoOpenclRuntime.h"
#include "gtest_wrapper.h"
#include "utils.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include <utility>

using namespace llvm::reflection;
using namespace llvm::NameMangleAPI;
using namespace Reflection;

namespace Reflection {
namespace tests {
static int toInt(const std::pair<int, int> &p) {
  return 2 * p.first + p.second;
}

TEST(CartesianProduc, twoWays) {
  typedef llvm::ArrayRef<int> ivector;
  int a1[] = {0, 1};
  ivector v1(a1);
  ivector v2(a1);
  Cartesian<llvm::ArrayRef, int, int> cartesian(v1, v2);
  std::vector<std::pair<int, int>> res;
  int i = 0;
  do {
    std::pair<int, int> p = cartesian.get();
    ASSERT_EQ(i++, toInt(p));
  } while (cartesian.next());
}

TEST(isBuiltin, positiveFirstInCache) {
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clampccc"));
}

TEST(isBuiltin, cacheTest) {
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clamphhh"));
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clamphhh"));
}

TEST(isBuiltin, positiveTest) {
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin("_Z5clampDv2_cS_S_"));
}

#if defined(_M_X64) || defined(__LP64__)
const char *const vstoreHalf16 = "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh";
#else
const char *const vstoreHalf16 = "_Z17vstore_half16_rtzDv16_fjPU3AS1Dh";
#endif

// this bi is among the last, so testing it will indicate for the worst lookup
// time
TEST(isBuiltin, addressSpaceTest) {
  ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin(vstoreHalf16));
}

// cache hits should make this test run almost as fast as the previous one
TEST(isBuiltin, performanceTest) {
  for (int i = 0; i < 10; ++i)
    ASSERT_TRUE(BuiltinKeeper::instance()->isBuiltin(vstoreHalf16));
}

TEST(isBuiltin, negativeTest) {
  ASSERT_FALSE(BuiltinKeeper::instance()->isBuiltin("_Z3foof"));
}

TEST(isBuiltin, invalidInput) {
  ASSERT_FALSE(
      BuiltinKeeper::instance()->isBuiltin("this is not a built-in function"));
}

TEST(isBuiltin, unmangledBuiltins) {
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  ASSERT_TRUE(pKeeper->isBuiltin("_Z13get_global_idj"));
  ASSERT_TRUE(pKeeper->isBuiltin("_Z15get_global_sizej"));
}

TEST(getWidth, fromScalar) {
  PairSW v2 = BuiltinKeeper::instance()->getVersion("_Z4fabsf", width::TWO);
  ASSERT_TRUE(v2.second == width::TWO);
}

TEST(getWidth, nonBuiltin) {
  PairSW sw = BuiltinKeeper::instance()->getVersion("_Z2bof", width::TWO);
  ASSERT_TRUE(isNullPair(sw));
}

static width::V indexToWidth(size_t index) {
  width::V allWidth[] = {width::SCALAR, width::TWO,     width::FOUR,
                         width::EIGHT,  width::SIXTEEN, width::THREE};
  assert(index < width::OCL_VERSIONS);
  return allWidth[index];
}

// This is an array of functions, with unique semantics.
// Each one of these builtin functions has two overloaded forms (let the name
// of the function denoted by 'f', the primitive function for the overload
// denoted by 't', and the vector-with (>1) denoted by 'd').
// The first overloaded form is: f(vd_t, t), while the second is: f(vd_t, vd_t).
// The expected behavior is as follows:
// 1. Both function forms are scalarizable.
// 2. The scalar version of 'f' (which the same for both formats) should be
// packetized to the second form (i.e., to f(vd_t, vd_t)).
llvm::StringRef ambigNames[] = {"fmax", "fmin",  "fmod", "ldexp", "min",
                                "max",  "clamp", "mix",  "step",  "smoothstep"};
llvm::ArrayRef<llvm::StringRef> arrAmbiguous(ambigNames);

// Uses the old (hard-coded) table of the vectorizer, to test that the
// properties of all the function descriptors are maintained. This test should
// validate the code after the BuiltinKeeper integration with the vectorizer.
TEST(VectorizerReference, OldTable) {
#include "VectorizerReference.h"
  const size_t numFunctionEntries = sizeof(functionEntries) /
                                    (width::OCL_VERSIONS * sizeof(char *)),
               numPropertiesEntries =
                   sizeof(tableProperties) / (NUM_PROPERTIES * sizeof(bool));
  ASSERT_EQ(numFunctionEntries, numPropertiesEntries);
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  for (size_t rowindex = 0; rowindex < numFunctionEntries; ++rowindex) {
    for (size_t colindex = 0; colindex < width::OCL_VERSIONS; ++colindex) {
      const char *biName = functionEntries[rowindex][colindex];
      if (!biName) // not interested in processing NULL entries.
        continue;
      if (!pKeeper->isBuiltin(biName)) {
        // we skip those, since they are not first class citizens (they are not
        // built-in functions), thus may not be demangled
        continue;
      }
      // Do the two versions agree on the scalarization property?
      width::V expectedWidth = indexToWidth(colindex);
      PairSW scalarVersion = pKeeper->getVersion(biName, width::SCALAR);
      if (expectedWidth != width::SCALAR) {
        ASSERT_EQ(0 == tableProperties[rowindex][0], isNullPair(scalarVersion))
            << "in built-in " << biName;
      }

      // Do the two versions agree on the packetizer property?
      // We arbitrary picked four as the destination vector width.
      bool isNull = isNullPair(pKeeper->getVersion(biName, width::FOUR));

      // The scalar versions of some builtin functions have ambiguity on that
      // property, and was solved by the vectorizer with hard-coded ifs.
      PairSW versionedFd = pKeeper->getVersion(biName, expectedWidth);
      width::V actualWidth = versionedFd.second;
      FunctionDescriptor fd = demangle(biName);
      bool isAmbiguous = std::find(arrAmbiguous.begin(), arrAmbiguous.end(),
                                   fd.Name) != arrAmbiguous.end();
      if (width::SCALAR == expectedWidth && isAmbiguous) {
        // Ambiguous scalar built-ins should always be scalarized.
        ASSERT_FALSE(isNull) << "in built-in: " << biName;
        continue;
      }
      ASSERT_EQ(tableProperties[rowindex][1], !isNull)
          << "Packetization property failure: mangled name " << biName;
      //
      // Width check.
      //
      if (!isNullPair(versionedFd))
        ASSERT_EQ(expectedWidth, actualWidth) << "in builtin " << biName;
      else
        ASSERT_TRUE(isAmbiguous)
            << " NULL function Descriptor in non ambiguous built-in " << biName
            << " width expectedWidth=" << expectedWidth;
    }
  }
} // end TEST

TEST(Functionality, VShapeBuiltiins) {
  // Mangled names of the functions, in the flavor in which the second arg. is
  // scalar.
  const char *scalarFlavor[] = {
      "_Z3maxDv8_ff",   "_Z3minDv8_ff",   "_Z4fmaxDv8_ff",
      "_Z4fminDv8_ff",  "_Z5ldexpDv8_fi", "_Z5clampDv8_fff",
      "_Z3mixDv8_fS_f", "_Z4stepfDv8_f",  "_Z10smoothstepffDv8_f"};
  // Mangled names of the scalar functions.
  const char *scalarFunctions[] = {
      "_Z3maxff",    "_Z3minff",  "_Z4fmaxff", "_Z4fminff",        "_Z5ldexpfi",
      "_Z5clampfff", "_Z3mixfff", "_Z4stepff", "_Z10smoothstepfff"};
  // Mangled names of the functions, in the symmetric flavor (both args are
  // vectors).
  const char *vecFlavor[] = {
      "_Z3maxDv8_fS_",   "_Z3minDv8_fS_",      "_Z4fmaxDv8_fS_",
      "_Z4fminDv8_fS_",  "_Z5ldexpDv8_fDv8_i", "_Z5clampDv8_fS_S_",
      "_Z3mixDv8_fS_S_", "_Z4stepDv8_fS_",     "_Z10smoothstepDv8_fS_S_"};
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  // Testing the function can be scalarized.
  for (size_t i = 0; i < sizeof(scalarFlavor) / sizeof(const char *); ++i) {
    PairSW scalarVersion = pKeeper->getVersion(scalarFlavor[i], width::SCALAR);
    ASSERT_FALSE(isNullPair(scalarVersion));
    PairSW vectorVersion =
        pKeeper->getVersion(scalarFunctions[i], width::EIGHT);
    ASSERT_STREQ(vectorVersion.first.c_str(), vecFlavor[i]);
  }
}

// BuiltinKepper functionality tests
TEST(Functionality, duplicatedScalarEntry) {
  const char *const sincosToscalarize = "_Z19__retbyarray_sincosDv4_d";
  const char *const sincosScalarVersion = "_Z20__retbyvector_sincosd";
  const char *const sincosExpectedVectorized = "_Z14sincos_ret2ptrDv4_dPS_S0_";
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  PairSW scalarVersion = pKeeper->getVersion(sincosToscalarize, width::SCALAR);
  ASSERT_STREQ(scalarVersion.first.c_str(), sincosScalarVersion);
  PairSW v4Version = pKeeper->getVersion(sincosScalarVersion, width::FOUR);
  ASSERT_STREQ(sincosExpectedVectorized, v4Version.first.c_str());
}

//
// Bi Transpose test
//
TEST(FDTranspose, vectorReturnTyFunctionality) {
  FunctionDescriptor cross4 = demangle("_Z5crossDv4_dS_");
  RefParamType doubleTy(new PrimitiveType(PRIMITIVE_DOUBLE));
  RefParamType double4Ty(new llvm::reflection::VectorType(doubleTy, 4));
  ReturnTypeMap retMap;
  retMap[cross4] = double4Ty;

  SoaDescriptorStrategy soaStrategy;
  soaStrategy.setTypeMap(&retMap);
  PairSW pair =
      soaStrategy(PairSW(std::make_pair("_Z5crossDv4_dS_", width::FOUR)));
  ASSERT_STREQ("_Z10soa_cross4Dv4_dS_S_S_S_S_S_S_PS_S0_S0_S0_",
               pair.first.c_str());
  ASSERT_EQ(width::FOUR, pair.second);
}

TEST(FDTranspose, scalarReturnTyFunctionality) {
  RefParamType doubleTy(new PrimitiveType(PRIMITIVE_DOUBLE));
  FunctionDescriptor dot = demangle("_Z3dotdd");
  ReturnTypeMap retMap;
  retMap[dot] = doubleTy;
  SoaDescriptorStrategy soaStrategy;
  soaStrategy.setTypeMap(&retMap);

  PairSW pair(std::make_pair("_Z3dotdd", width::FOUR));
  pair = soaStrategy(pair);
  ASSERT_STREQ("_Z8soa_dot1Dv4_dS_", pair.first.c_str());
}

//
// Tests the glue code between one of the dirver of builtin-keeper (in this
// case the OpenCL vectorizer) to the Builtin-keeper
//
TEST(DriverUse, oclVectorizer) {
  //
  // parse the test ir file into a module
  //
  llvm::LLVMContext context;
  llvm::SMDiagnostic errDiagnostic;
  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  llvm::SmallVector<llvm::Module *, 2> runtimeModuleList;
  runtimeModuleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(runtimeModuleList);
  std::string biname = "_Z3mixddd";
  std::unique_ptr<intel::VectorizerFunction> pFunction =
      pRuntime->findBuiltinFunction(biname);
  ASSERT_EQ(1U, pFunction->getWidth());
  ASSERT_STREQ("_Z3mixDv4_dS_S_", pFunction->getVersion(2).c_str());
  ASSERT_TRUE(pFunction->isPacketizable());
  ASSERT_TRUE(pFunction->isScalarizable());
  delete pRuntime;
}

//
// The width of function which have SOA conversion is tricky, not the usual
// algorithm
//
TEST(DriverUse, soaDescriptorsWidth) {
  llvm::LLVMContext context;
  llvm::SmallVector<llvm::Module *, 2> moduleList;
  llvm::SMDiagnostic errDiagnostic;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);
  std::string biname = "_Z6lengthDv2_f";
  std::unique_ptr<intel::VectorizerFunction> pFunction =
      pRuntime->findBuiltinFunction(biname);
  ASSERT_EQ(1U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv4_fS_";
  pFunction = pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(4U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv8_fS_";
  pFunction = pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(8U, pFunction->getWidth());

  biname = "_Z11soa_length2Dv16_fS_";
  pFunction = pRuntime->findBuiltinFunction(biname);
  ASSERT_FALSE(pFunction->isScalarizable());
  ASSERT_EQ(16U, pFunction->getWidth());

  delete pRuntime;
}

TEST(DriverUse, nonVersioned) {
  llvm::LLVMContext context;
  llvm::SmallVector<llvm::Module *, 2> moduleList;
  llvm::SMDiagnostic errDiagnostic;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);

  std::string fract = "_Z5fractDv16_fPU3AS1S_";
  std::unique_ptr<intel::VectorizerFunction> pFunction =
      pRuntime->findBuiltinFunction(fract);
  ASSERT_FALSE(pFunction->isPacketizable());
  ASSERT_FALSE(pFunction->isScalarizable());
}

TEST(DriverUse, soaVersion3) {
  std::string scalarFunction = "_Z5crossDv3_fS_";
  std::string vectorFunction = "_Z10soa_cross3Dv4_fS_S_S_S_S_PS_S0_S0_";
  llvm::LLVMContext context;
  llvm::SmallVector<llvm::Module *, 2> moduleList;
  llvm::SMDiagnostic errDiagnostic;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);
  const std::unique_ptr<intel::VectorizerFunction> foundFunction =
      pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(DriverUse, soaVersion4) {
  std::string scalarFunction = "_Z5crossDv4_dS_";
  std::string vectorFunction = "_Z10soa_cross4Dv4_dS_S_S_S_S_S_S_PS_S0_S0_S0_";
  llvm::LLVMContext context;
  llvm::SmallVector<llvm::Module *, 2> moduleList;
  llvm::SMDiagnostic errDiagnostic;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);
  const std::unique_ptr<intel::VectorizerFunction> foundFunction =
      pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(DriverUse, normalize) {
  std::string scalarFunction = "_Z9normalizeDv4_f";
  std::string vectorFunction = "_Z14soa_normalize4Dv4_fS_S_S_PS_S0_S0_S0_";
  llvm::LLVMContext context;
  llvm::SmallVector<llvm::Module *, 2> moduleList;
  llvm::SMDiagnostic errDiagnostic;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);
  const std::unique_ptr<intel::VectorizerFunction> foundFunction =
      pRuntime->findBuiltinFunction(scalarFunction);
  std::string actual = foundFunction->getVersion(2);
  ASSERT_EQ(vectorFunction, actual);
  delete pRuntime;
}

TEST(GenTest, soaGenTest) {
  std::string scalarVersion = "_Z7isequalff";
  std::string strExpected = "_Z7isequalDv4_fS_";
  llvm::LLVMContext context;
  llvm::SMDiagnostic errDiagnostic;
  llvm::SmallVector<llvm::Module *, 2> moduleList;

  std::unique_ptr<llvm::Module> pModule =
      llvm::parseIRFile("mybi.ll", errDiagnostic, context);
  moduleList.push_back(pModule.get());
  intel::RuntimeServices *pRuntime =
      new intel::VolcanoOpenclRuntime(moduleList);
  const std::unique_ptr<intel::VectorizerFunction> pFunc =
      pRuntime->findBuiltinFunction(scalarVersion);
  std::string strActual = pFunc->getVersion(2);
  ASSERT_EQ(strActual, strActual);
  delete pRuntime;
}

} // namespace tests
} // namespace Reflection

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
