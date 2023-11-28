// Copyright (C) 2012 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "gtest_wrapper.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"
#include <algorithm>
#include <map>
#include <stdint.h>

using namespace llvm::reflection;
using namespace llvm::NameMangleAPI;

namespace namemangling {
namespace tests {

static std::string getParameterString(const FunctionDescriptor &fd, int index) {
  return fd.Parameters[index]->toString();
}

struct Range {
  // the beginning, and the end of the range (inclusively)
  size_t begin, end;
  Range(int b, int e) : begin(b), end(e) {}
  size_t length() const { return (1 + end) - begin; }
};

// tokenize str, with respect to the given delimiters
std::vector<std::string> tokenize(const std::string &str,
                                  const std::string &delimiters) {
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
// Finds the (first) range in which the given strings differ
//
static Range findRange(const std::string &l, const std::string &r) {
  assert(l != r && "strings are the same");
  size_t b = 0, length = std::min(l.length(), r.length());
  size_t e = length;
  while (l[b] == r[b] && b < length - 1)
    ++b;
  while (l[e] == r[e] && e > 0)
    --e;
  // advance to the first index in which the two strings has a whitespace
  while ((!isspace(l[e]) || !isspace(r[e])) && e < length)
    ++e;
  assert(e >= b && "the string are suppose to be different shouldn't they?");
  return Range(b, e);
}

struct AttributeDetector {
  bool operator()(const std::string &s) const {
    if ("__local" == s)
      return true;
    if ("__private" == s)
      return true;
    if ("__global" == s)
      return true;
    if ("__constant" == s)
      return true;
    if ("__generic" == s)
      return true;
    if ("volatile" == s)
      return true;
    if ("const" == s)
      return true;
    if ("restrict" == s)
      return true;
    return false;
  }
};

std::ostream &operator<<(std::ostream &o, const std::vector<std::string> &v) {
  size_t len = v.size();
  for (size_t i = 0; i < len; i++)
    o << v[i] << std::endl;
  return o;
}

// string decoration function
template <typename T> struct IntTypedef {
  static std::string nativeStr() {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Unsupported type!");
    if (sizeof(T) == 4)
      return std::is_signed<T>() ? "int" : "uint";
    else
      return std::is_signed<T>() ? "long" : "ulong";
  }
};

static void replaceAll(std::string &src, const std::string &from,
                       const std::string &to) {
  size_t pos = src.find(from);
  while (std::string::npos != pos) {
    src.erase(pos, from.length());
    src.insert(pos, to);
    pos = src.find(from);
  }
}

static void replaceIntTypedef(std::string &s) {
  replaceAll(s, "size_t", IntTypedef<size_t>::nativeStr());
  replaceAll(s, "uintptr_t", IntTypedef<uintptr_t>::nativeStr());
  replaceAll(s, "intptr_t", IntTypedef<intptr_t>::nativeStr());
  replaceAll(s, "ptrdiff_t", IntTypedef<ptrdiff_t>::nativeStr());
}

static void replaceClMemFenceFlags(std::string &s) {
  // cl_mem_fence_flags is typedef to uint
  replaceAll(s, "cl_mem_fence_flags", "uint");
}

static void replaceCL20AtomicTypes(std::string &s) {
  // atomic_flag is typedef to atomic_int
  replaceAll(s, "atomic_flag", "atomic_int");
  // memory_scope is typedef to uint
  replaceAll(s, "memory_scope", "int");
  // memory_order is typedef to uint
  replaceAll(s, "memory_order", "int");
}

static void fixImageTypeNames(std::string &s) {
  // drop ocl_ prefix
  replaceAll(s, "ocl_image", "image");
  // drop access qualifier
  replaceAll(s, "_ro", "");
  replaceAll(s, "_wo", "");
  replaceAll(s, "_rw", "");

  replaceAll(s, "__read_only ", "");
  replaceAll(s, "__write_only ", "");
  replaceAll(s, "__read_write ", "");
}

static void fixGenericAddressSpaceQualifier(std::string &s) {
  // All pointers are __generic, unless stated otherwise
  replaceAll(s, "__generic ", "");
}

// returns true, if the following function prototypes are semantically the same
//(the itanium standard allows attributes to be order insensitive)
static bool isSematicallyEqual(const std::string &l, const std::string &r) {
  if (l == r)
    return true;
  std::string left(l), right(r);
  // cl_mem_fence_flags are typedef's for int under SPIR SPEC.
  replaceClMemFenceFlags(left);
  replaceClMemFenceFlags(right);
  // replacing the size_t/uintptr_t/intptr_t/ptrdiff_t argument, since its a
  // typedef, not a real type, and clang treats it that way.
  replaceIntTypedef(left);
  replaceIntTypedef(right);
  // replacing CL2.0 types with the expected types as clang mangle them
  // e.g. "memory_scope" -> "int", "memory_order" -> "int", "atomic_flag" ->
  // "atomic_int"
  replaceCL20AtomicTypes(left);
  replaceCL20AtomicTypes(right);

  fixImageTypeNames(left);
  fixImageTypeNames(right);

  fixGenericAddressSpaceQualifier(left);
  fixGenericAddressSpaceQualifier(right);

  // if they have different length at this point, they can't be semantically the
  // same
  if (left.length() != right.length())
    return false;
  // after removing/replacing semantically equivalences, the strings might be
  // the same
  if (left == right)
    return true;
  // since the order of appearance of attribute isn't significant, we try to
  // rearrange them.
  Range range = findRange(left, right);
  std::string diff(left, range.begin, range.length());
  std::vector<std::string> tokens = tokenize(diff, " ");
  // if the entire diff string is attributes, they are semantically the same
  AttributeDetector isAttribute;
  size_t attributeCount =
      std::count_if(tokens.begin(), tokens.end(), isAttribute);
  return (attributeCount == tokens.size());
}

static bool isVector(const ParamType *T) {
  return NULL != llvm::dyn_cast<VectorType>(T);
}

static bool isPointer(const ParamType *T) {
  return NULL != llvm::dyn_cast<PointerType>(T);
}

//
// Tests
//

TEST(NameMangle, demangleTostrightAndBack) {
#include "BuiltinList.inc"
  for (unsigned int i = 0; i < sizeof(mangledNames) / sizeof(char *); i++) {
    const char *mname = mangledNames[i];

    FunctionDescriptor fd = demangle(mname);
    ASSERT_FALSE(fd.isNull());

    std::string expected(mname);
    std::string actual = mangle(fd);
    // checking that the mangle demangle cycle returns to the same string
    ASSERT_EQ(expected, actual);
    // checking that the demangle string is as we expect it to be
    //(semantically the same)
    if (!isSematicallyEqual(fd.toString(), prototypes[i]))
      FAIL() << "\'" << fd.toString() << "\'"
             << " semantically differs from "
             << "\'" << prototypes[i] << "\' at entry " << i;
  }
}

const char *const strRetByPtr = "_Z4FuncfPfS0_";

TEST(DemangleTest, retByPtr) {
  FunctionDescriptor fd = demangle(strRetByPtr);
  ASSERT_FALSE(fd.isNull());

  const PrimitiveType *pSclTy =
      llvm::dyn_cast<PrimitiveType>(fd.Parameters[0].get());
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());

  ASSERT_TRUE(isPointer(fd.Parameters[1].get()));
  ASSERT_TRUE(isPointer(fd.Parameters[2].get()));
}

TEST(MangleTest, retByPtr) {
  FunctionDescriptor fd;
  fd.Name = "Func";
  RefParamType F(new PrimitiveType(PRIMITIVE_FLOAT));
  RefParamType PF(new PointerType(F, {ATTR_PRIVATE}));
  fd.Parameters.push_back(F);
  fd.Parameters.push_back(PF);
  fd.Parameters.push_back(PF);
  ASSERT_EQ(std::string(strRetByPtr), mangle(fd));
}

TEST(DemangleTest, AsyncGropuCpy) {
  FunctionDescriptor fd =
      demangle("_Z21async_work_group_copyPU3AS3Dv2_cPU3AS1KS_mm");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("async_work_group_copy(__local char2 *, const __global "
                        "char2 *, ulong, ulong)"),
            fd.toString());
}

TEST(MangleTest, clk_event) {
  const std::string s = "_Z14enqueue_kernelPK12ocl_clkeventPS_";
  FunctionDescriptor fd = demangle(s.c_str());
  ASSERT_EQ(s, mangle(fd));
}

TEST(DemangleTest, block) {
  char const *names[] = {"_Z14enqueue_kernelU13block_pointerFvvE",
                         "_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_"
                         "clkeventPU3AS4S1_U13block_pointerFvPU3AS3vzEjz",
                         "_Z14enqueue_kernel9ocl_queuei9ndrange_tjPK12ocl_"
                         "clkeventPS1_U13block_pointerFvPU3AS3vzEjz"};

  for (char const *name : names) {
    FunctionDescriptor fd1 = demangle(name);
    ASSERT_FALSE(fd1.isNull());
    ASSERT_EQ(mangle(fd1), std::string(name));
  }
}

TEST(NameMangle, FailedOnce) {
  const char *s = "_Z5frexpDv2_fPU3AS1Dv2_i";
  FunctionDescriptor fd = demangle(s);
  ASSERT_FALSE(fd.isNull());
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}

TEST(NameMangle, Failed2) {
  const char *s = "_Z9mask_fmaxtDv16_fS_";
  FunctionDescriptor fd = demangle(s);
  ASSERT_FALSE(fd.isNull());
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}

TEST(NameMangle, SOAFunction) {
  FunctionDescriptor soaDescriptor;
  soaDescriptor.Name = "soa";
  RefParamType doubleTy(new PrimitiveType(PRIMITIVE_DOUBLE));
  RefParamType intTy(new PrimitiveType(PRIMITIVE_INT));
  RefParamType pintTy(new PointerType(intTy, {ATTR_PRIVATE}));
  soaDescriptor.Parameters.push_back(doubleTy);
  soaDescriptor.Parameters.push_back(pintTy);
  soaDescriptor.Parameters.push_back(pintTy);
  std::cout << soaDescriptor.toString() << std::endl;
  ASSERT_EQ(std::string("_Z3soadPiS0_"), mangle(soaDescriptor));
}

static bool testDemangle(const char *mname) {
  FunctionDescriptor fd = demangle(mname);
  if (fd.isNull())
    return false;
  printf("%s\n", fd.toString().c_str());
  return true;
}

TEST(DemangleTest, OclTypesSubstSPIR12Compatibility) {
  const char *name = "_Z12write_imagei16ocl_image2darrayDv4_iS_";
  FunctionDescriptor fd = demangle(name, /*isSpir12Name=*/true);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("image2d_array_t"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int4"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("int4"), getParameterString(fd, 2));
}

TEST(DemangleTest, OclTypesSubst) {
  const char *name = "_Z12write_imagei16ocl_image2darrayDv4_iS_";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("image2d_array_t"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int4"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("image2d_array_t"), getParameterString(fd, 2));
}

TEST(DemangleTest, OclTypesSubst2) {
  const char *name = "_Z12write_imagei20ocl_image2d_array_woDv4_iS0_";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("image2d_array_wo_t"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int4"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("int4"), getParameterString(fd, 2));
}

TEST(DemangleTest, pointerAttributes) {
  const char *name = "_Z10mask_vloadtmPU3AS2Kc";
  ASSERT_TRUE(testDemangle(name));
}

TEST(DemangleTest, duplicateParam) {
  const char *name = "_Z8abs_diffDv2_cS_";
  ASSERT_TRUE(testDemangle(name));
}

TEST(DemangleTest, duplicateParam0) {
  const char *name = "_Z10soa_cross3Dv16_fS_S_S_S_S_PS_S0_S0_";
  ASSERT_TRUE(testDemangle(name));
}

//
// This function might be mistakingly considered as an image function
//
TEST(DemangleTest, imageAmbiguity) {
  const char *name = "_Z11mask_vstoretDv16_imPU3AS1i";
  ::testing::AssertionSuccess() << "demangling " << name; // << std::endl;
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("mask_vstore", fd.Name);
  ASSERT_EQ(4U, fd.Parameters.size());
  ASSERT_EQ(std::string("ushort"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int16"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("ulong"), getParameterString(fd, 2));
  ASSERT_EQ(std::string("__global int *"), getParameterString(fd, 3));
}

TEST(DemangleTest, imageBuiltin) {
  const char *name = "_Z11read_imagef11ocl_image2d11ocl_samplerDv2_f";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("read_imagef", fd.Name);
  ASSERT_EQ(3U, fd.Parameters.size());
  ASSERT_EQ(std::string("image2d_t"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("sampler_t"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("float2"), getParameterString(fd, 2));
}

TEST(DemangleTest, duplicatedVector) {
  const char *name = "_Z3maxDv2_iS_";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("max", fd.Name);
  ASSERT_EQ(2U, fd.Parameters.size());
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 1));
}

TEST(DemangleTest, voidptr) {
  FunctionDescriptor fd =
      demangle("_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i");
  ASSERT_FALSE(fd.isNull());
  ASSERT_STREQ("trans_coord_int_NONE_FALSE_NEAREST", fd.Name.c_str());
  ASSERT_EQ(2U, fd.Parameters.size());
}

TEST(DemangleTest, userDefinedTy1) {
  FunctionDescriptor fd = demangle("_Z6myfunc4myTy");
  ASSERT_FALSE(fd.isNull());
  ASSERT_STREQ("myfunc", fd.Name.c_str());
  ASSERT_EQ(std::string("myTy"), getParameterString(fd, 0));
  ASSERT_EQ(1U, fd.Parameters.size());
}

// tests user defined types, one after the other
TEST(DemangleTest, userDefinedTy2) {
  FunctionDescriptor fd = demangle("_Z6myfunc5myTy16myTy21");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("myTy1"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("myTy21"), getParameterString(fd, 1));
}

// Address space tests

TEST(DemangleTest, addressSpace) {
  ASSERT_TRUE(testDemangle("_Z17vstore_half16_rtzDv16_fmPU3AS1Dh"));
}

TEST(DemangleTest, addressSpace1) {
  FunctionDescriptor fd = demangle("_Z3myfPU3AS1i");
  ASSERT_FALSE(fd.isNull());
  PointerType *p = llvm::dyn_cast<PointerType>(fd.Parameters[0].get());
  ASSERT_TRUE(p);
  ASSERT_EQ(ATTR_GLOBAL, *p->getAttributes().begin());
}

TEST(DemangleTest, addressSpaceAndUserDefTy) {
  FunctionDescriptor fd = demangle("_Z3myfPU3AS23mta");
  ASSERT_FALSE(fd.isNull());
  PointerType *p = llvm::dyn_cast<PointerType>(fd.Parameters[0].get());
  ASSERT_TRUE(p);
  ASSERT_EQ(ATTR_CONSTANT, *p->getAttributes().begin());
  ASSERT_EQ(std::string("mta"), p->getPointee()->toString());
}

//
// Mangle
//

TEST(MangleTest, _Z17convert_float_satc) {
  std::string orig = "_Z17convert_float_satc";
  FunctionDescriptor fd = demangle(orig.c_str());
  ASSERT_FALSE(fd.isNull());
  std::string syntesized = mangle(fd);
  ASSERT_EQ(orig, syntesized);
}

// A special case, in which the param duplicate operator works to duplicate a
// type suffix, not the enture type. The first parameter is v2f, the second is
// v2f*.
TEST(MangleTest, semidup) {
  std::string orig = "_Z5fractDv2_fPU3AS1S_";
  FunctionDescriptor fd = demangle(orig.c_str());
  ASSERT_FALSE(fd.isNull());
  std::string actual = mangle(fd);
  ASSERT_EQ(orig, actual);
}

const char *strDoubleDup = "_Z4stamDv4_fS_PS_S1_";

TEST(MangleTest, doubleDup) {
  FunctionDescriptor fd;
  RefParamType primitiveFloat(new PrimitiveType(PRIMITIVE_FLOAT));
  RefParamType vectorFloat(new VectorType(primitiveFloat, 4));
  RefParamType ptrFloat(new PointerType(vectorFloat, {ATTR_PRIVATE}));

  fd.Name = "stam";
  fd.Parameters.push_back(vectorFloat);
  fd.Parameters.push_back(vectorFloat);
  fd.Parameters.push_back(ptrFloat);
  fd.Parameters.push_back(ptrFloat);

  std::string mangled = mangle(fd);
  ASSERT_STREQ(strDoubleDup, mangled.c_str());
}

TEST(DemangleTest, doubleDup1) {
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.Parameters[0];
  ASSERT_TRUE(isVector(T.get()));
  const VectorType *pVecTy = llvm::dyn_cast<VectorType>(T.get());
  const PrimitiveType *pSclTy =
      llvm::dyn_cast<PrimitiveType>(pVecTy->getScalarType().get());
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());
}

TEST(DemangleTest, doubleDup2) {
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.Parameters[1];
  ASSERT_TRUE(isVector(T.get()));
  const VectorType *pVecTy = llvm::dyn_cast<VectorType>(T.get());
  const PrimitiveType *pSclTy =
      llvm::dyn_cast<PrimitiveType>(pVecTy->getScalarType().get());
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());
}

TEST(DemangleTest, doubleDup3) {
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  ASSERT_TRUE(isPointer(fd.Parameters[2].get()));
}

TEST(DemangleTest, doubleDup4) {
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.Parameters[2];
  ASSERT_TRUE(isPointer(T.get()));
  const PointerType *P = llvm::dyn_cast<PointerType>(T.get());
  ASSERT_TRUE(isVector(P->getPointee().get()));
}

TEST(DemangleTest, doubleDup5) {
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.Parameters[3];
  ASSERT_TRUE(isPointer(T.get()));
  const PointerType *P = llvm::dyn_cast<PointerType>(T.get());
  ASSERT_TRUE(isVector(P->getPointee().get()));
}

TEST(DemangleTest, doubleDup6) {
  FunctionDescriptor fd =
      demangle("_Z3fooDv2_iDv2_jDv2_cDv2_hDv4_fDv8_dS_S0_S1_S2_S3_S4_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("foo(int2, uint2, char2, uchar2, float4, double8, "
                        "int2, uint2, char2, uchar2, float4, double8)"),
            fd.toString());
}

TEST(DemangleTest, doubleDup7) {
  FunctionDescriptor fd = demangle("_Z3fooPDv4_fS_S1_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("foo(__private float4 *, float4, __private float4 *)"),
            fd.toString());
}

TEST(DemangleTest, doubleDup8) {
  FunctionDescriptor fd =
      demangle("_Z3fooPiPjPcPhPfPdS0_S2_S4_S6_S8_SA_Dv4_iDv4_jDv4_cDv4_hDv4_"
               "fDv4_dSB_SC_SD_SE_SF_SG_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(
      std::string("foo(__private int *, __private uint *, __private char *, "
                  "__private uchar *, __private float *, __private double *, "
                  "__private int *, __private uint *, __private char *, "
                  "__private uchar *, __private float *, __private double *, "
                  "int4, uint4, char4, uchar4, float4, double4, "
                  "int4, uint4, char4, uchar4, float4, double4)"),
      fd.toString());
}

TEST(DemangleTest, doubleDup9) {
  FunctionDescriptor fd = demangle("_Z3fooP4sFooS1_S_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("foo(__private sFoo *, __private sFoo *, sFoo)"),
            fd.toString());
}

// Check that all (more than one) CVR-qualifiers is demangled correctly
TEST(DemangleTest, CVRQualifiers) {
  const char *orig = "_Z37atomic_compare_exchange_weak_explicitPU3AS3rVKU7_"
                     "AtomiciPii12memory_orderS3_";
  FunctionDescriptor fd = demangle(orig);
  ASSERT_FALSE(fd.isNull());
  ASSERT_TRUE(fd.toString().find("const volatile restrict") !=
              std::string::npos);
}

// Check that demangler can parse SPIR1.2 where CVR-qualifiers are before
// address space qualifiers
TEST(DemangleTest, SPIR12Pointers) {
  const char *origPointer = "_Z6vload2mPKU3AS1l";
  FunctionDescriptor fdPointer = demangle(origPointer);
  ASSERT_FALSE(fdPointer.isNull());
  ASSERT_TRUE(fdPointer.toString().find("const") != std::string::npos);

  const char *origAtomic = "_Z37atomic_compare_exchange_weak_explicitPVU3AS3U7_"
                           "AtomiciPii12memory_orderS3_";
  FunctionDescriptor fdAtomic = demangle(origAtomic);
  ASSERT_FALSE(fdAtomic.isNull());
  ASSERT_TRUE(fdAtomic.toString().find("volatile") != std::string::npos);
}

TEST(MangleBasic, scalarfloat) {
  RefParamType primitiveFloat(new PrimitiveType(PRIMITIVE_FLOAT));
  FunctionDescriptor fd;
  fd.Name = "foo";
  fd.Parameters.push_back(primitiveFloat);
  ASSERT_STREQ("_Z3foof", mangle(fd).c_str());
}

TEST(MangleBasic, scalardouble) {
  RefParamType primitiveDouble(new PrimitiveType(PRIMITIVE_DOUBLE));
  FunctionDescriptor fd;
  fd.Name = "foo";
  fd.Parameters.push_back(primitiveDouble);
  ASSERT_STREQ("_Z3food", mangle(fd).c_str());
}

//
// MangleAPI
//

TEST(MangleAPI, visitorExample) {
  const char *soaFunc = "_Z5dummyiDv4_fPS_";
  FunctionDescriptor fd = demangle(soaFunc);
  struct PrintTypeVisitor : TypeVisitor {
    int ordinal;

    std::string ordinalStr() {
      std::stringstream ss;
      ss << "parameter " << ++ordinal << ":\t";
      return ss.str();
    }

    PrintTypeVisitor() : ordinal(0) {}

    void visit(const PrimitiveType *p) {
      std::cout << "primitive " << p->toString() << std::endl;
    }

    void visit(const VectorType *p) {
      std::cout << "vector with length " << p->getLength() << " with ";
      p->getScalarType()->accept(this);
    }

    void visit(const PointerType *p) {
      std::cout << "pointer to ";
      p->getPointee()->accept(this);
    }

    void visit(const AtomicType *p) {
      std::cout << "atomic " << p->toString() << std::endl;
    }

    void visit(const BlockType *p) {
      std::cout << "block ( ";
      for (unsigned int i = 0; i < p->getNumOfParams(); ++i) {
        p->getParam(i)->accept(this);
      }
      std::cout << ")";
    }

    void visit(const UserDefinedType *) {
      std::cout << ordinalStr();
      std::cout << "user defined type. in OCL, its images..." << std::endl;
    }
  };
  TypeVector::iterator bi = fd.Parameters.begin(), e = fd.Parameters.end();
  PrintTypeVisitor printVisitor;
  while (bi != e) {
    std::cout << printVisitor.ordinalStr();
    (*bi)->accept(&printVisitor);
    ++bi;
  }
}

TEST(Type, TypeCast) {
  RefParamType primitiveInt(new PrimitiveType(PRIMITIVE_INT));
  VectorType vectorInt(primitiveInt, 4);
  ASSERT_TRUE(nullptr == llvm::dyn_cast<VectorType>(primitiveInt.get()));
  ASSERT_EQ(&vectorInt, llvm::dyn_cast<VectorType>(&vectorInt));
}

TEST(MemoryLeaks, Vector) {
  const char *s = "_Z5frexpDv2_i";
  demangle(s);
}

TEST(MemoryLeaks, PointerToPrimitive) {
  const char *s = "_Z5frexpPi";
  demangle(s);
}

TEST(MemoryLeaks, PointerToVector) {
  const char *s = "_Z5frexpPDv2_i";
  demangle(s);
}

TEST(MemoryLeaks, PointerAttribToVector) {
  const char *s = "_Z5frexpPU3AS1Dv2_i";
  demangle(s);
}

TEST(MemoryLeaks, StructTy) {
  FunctionDescriptor fd = demangle("_Z1f2xx");
  ASSERT_FALSE(fd.isNull());
  EXPECT_TRUE(llvm::dyn_cast<UserDefinedType>(fd.Parameters[0].get()));
}

} // namespace tests
} // end namespace namemangling

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
