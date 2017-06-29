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

#include "NameMangleAPI.h"
#include "FunctionDescriptor.h"
#include "ParameterType.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <stdint.h>
#include <map>

using namespace reflection;

namespace namemangling { namespace tests{

static std::string getParameterString(const FunctionDescriptor& fd,
  int index){
  return fd.parameters[index]->toString();
}

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
    if ( "__generic" == s)
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
struct SizeT<4>{
  static const char* nativeStr(){return "uint";}
};

static void replaceAll(std::string& src, const std::string& from,
  const std::string& to) {
  size_t pos = src.find(from);
  while (std::string::npos !=  pos){
    src.erase(pos, from.length());
    src.insert(pos, to);
    pos = src.find(from);
  }
}

//
//deletes the __private attribute from the given string if it appears in it
//
static void deletePrivate(std::string& s){
  replaceAll(s, "__private ", "");
  replaceAll(s, "private ", "");
}

static void replaceSizeT(std::string& s){
  const std::string strSizeT(SizeT<sizeof(size_t)>::nativeStr());
  replaceAll(s, "size_t", strSizeT);
}

static void replaceClMemFenceFlags(std::string& s){
  // cl_mem_fence_flags is typedef to uint
  replaceAll(s, "cl_mem_fence_flags", "uint");
}

static void replaceCL20AtomicTypes(std::string& s){
  // atomic_flag is typedef to atomic_int
  replaceAll(s, "atomic_flag",   "atomic_int");
  // memory_scope is typedef to uint
  replaceAll(s, "memory_scope",  "int");
  // memory_order is typedef to uint
  replaceAll(s, "memory_order",  "int");
}

static void fixImageTypeNames(std::string& s){
  // drop ocl_ prefix
  replaceAll(s, "ocl_image",   "image");
  // drop access qualifier
  replaceAll(s, "_ro",  "");
  replaceAll(s, "_wo",  "");
  replaceAll(s, "_rw",  "");

  replaceAll(s, "__read_only ",  "");
  replaceAll(s, "__write_only ",  "");
  replaceAll(s, "__read_write ",  "");
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
  //cl_mem_fence_flags are typedef's for int under SPIR SPEC.
  replaceClMemFenceFlags(left);
  replaceClMemFenceFlags(right);
  //replacing the size_t argument, since its a typedef, not a real type, and
  //clang treats it that way.
  replaceSizeT(left);
  replaceSizeT(right);
  //replacing CL2.0 types with the expected types as clang mangle them
  //e.g. "memory_scope" -> "int", "memory_order" -> "int", "atomic_flag" -> "atomic_int"
  replaceCL20AtomicTypes(left);
  replaceCL20AtomicTypes(right);

  fixImageTypeNames(left);
  fixImageTypeNames(right);

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

static bool isVector(const ParamType *T){
  return NULL != reflection::dyn_cast<VectorType>(T);
}

static bool isPointer(const ParamType *T){
  return NULL != reflection::dyn_cast<PointerType>(T);
}

//
//Tests
//

TEST(NameMangle, demangleTostrightAndBack){
  #include "MangledNames.h"
  for( unsigned int i = 0 ; i < sizeof(mangledNames)/sizeof(char*) ; i++){
      const char* mname = mangledNames[i];

      FunctionDescriptor fd = demangle(mname);
      ASSERT_FALSE(fd.isNull());

      std::string expected(mname);
      std::string actual = mangle(fd);
      //checking that the mangle demangle cycle returns to the same string
      ASSERT_EQ(expected, actual);
      //checking that the demangle string is as we expect it to be
      //(semantically the same)
      if (!isSematicallyEqual(fd.toString(), prototypes[i]))
        FAIL() << "\'" << fd.toString() << "\'" << " semantically differs from "
        << "\'" << prototypes[i] << "\'";
  }
}

const char*const strRetByPtr = "_Z4FuncfPfS_";

TEST(DemangleTest, retByPtr){
  FunctionDescriptor fd = demangle(strRetByPtr);
  ASSERT_FALSE(fd.isNull());

  const PrimitiveType *pSclTy = reflection::dyn_cast<PrimitiveType>(fd.parameters[0]);
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());

  ASSERT_TRUE(isPointer(fd.parameters[1]));
  ASSERT_TRUE(isPointer(fd.parameters[2]));
}

TEST(MangleTest, retByPtr){
  FunctionDescriptor fd;
  fd.name = "Func";
  RefParamType F(new PrimitiveType(PRIMITIVE_FLOAT));
  RefParamType PF(new PointerType(F));
  fd.parameters.push_back(F);
  fd.parameters.push_back(PF);
  fd.parameters.push_back(PF);
  ASSERT_EQ(std::string(strRetByPtr), mangle(fd));
}

TEST(DemangleTest, AsyncGropuCpy){
  FunctionDescriptor fd =
    demangle("_Z21async_work_group_copyPU3AS3Dv2_cPU3AS1KS_mm");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(
    std::string("async_work_group_copy(__local char2 *, const __global char2 *, ulong, ulong)")
    , fd.toString()
  );
}


TEST(MangleTest, clk_event){
  const std::string s = "_Z14enqueue_kernelPK12ocl_clkeventS0_";
  FunctionDescriptor fd = demangle(s.c_str());
  ASSERT_EQ(s, mangle(fd));
}

TEST(DemangleTest, block){
  char const* names[] = {
    "_Z14enqueue_kernelU13block_pointerFvvE",
    "_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventS2_U13block_pointerFvPU3AS3vzEjz",
    "_Z14enqueue_kernel9ocl_queuei9ndrange_tjPK12ocl_clkeventS2_U13block_pointerFvPU3AS3vzEjz"
  };

  for(char const* name : names) {
    FunctionDescriptor fd1 = demangle(name);
    ASSERT_FALSE(fd1.isNull());
    ASSERT_EQ(mangle(fd1), std::string(name));
  }

}

TEST(NameMangle, FailedOnce){
  const char* s = "_Z5frexpDv2_fPU3AS1Dv2_i";
  FunctionDescriptor fd = demangle(s);
  ASSERT_FALSE(fd.isNull());
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}

TEST(NameMangle, Failed2){
  const char* s = "_Z9mask_fmaxtDv16_fS_";
  FunctionDescriptor fd = demangle(s);
  ASSERT_FALSE(fd.isNull());
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}

TEST(NameMangle, SOAFunction){
  FunctionDescriptor soaDescriptor;
  soaDescriptor.name = "soa";
  RefParamType doubleTy(new PrimitiveType(PRIMITIVE_DOUBLE));
  RefParamType intTy(new PrimitiveType(PRIMITIVE_INT));
  RefParamType pintTy(new PointerType(intTy));
  soaDescriptor.parameters.push_back(doubleTy);
  soaDescriptor.parameters.push_back(pintTy);
  soaDescriptor.parameters.push_back(pintTy);
  std::cout << soaDescriptor.toString() << std::endl;
  ASSERT_EQ(std::string("_Z3soadPiS_"), mangle(soaDescriptor));
}

static bool testDemangle(const char* mname){
    FunctionDescriptor fd = demangle(mname);
    if(fd.isNull()) return false;
    printf( "%s\n", fd.toString().c_str());
    return true;
}

TEST(DemangleTest, pointerAttributes){
  const char* name = "_Z10mask_vloadtmPU3AS2Kc";
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

//
//This function might be mistakingly considered as an image function
//
TEST(DemangleTest, imageAmbiguity){
  const char* name = "_Z11mask_vstoretDv16_imPU3AS1i";
  ::testing::AssertionSuccess() << "demangling " << name; // << std::endl;
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("mask_vstore", fd.name);
  ASSERT_EQ(4U, fd.parameters.size());
  ASSERT_EQ(std::string("ushort"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int16"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("ulong"), getParameterString(fd, 2));
  ASSERT_EQ(std::string("__global int *"), getParameterString(fd, 3));
}

TEST(DemangleTest, imageBuiltin){
  const char* name = "_Z11read_imagef11ocl_image2d11ocl_samplerDv2_f";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("read_imagef", fd.name);
  ASSERT_EQ(3U, fd.parameters.size());
  ASSERT_EQ(std::string("image2d_t"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("sampler_t"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("float2"), getParameterString(fd, 2));
}

TEST(DemangleTest, duplicatedVector){
  const char* name = "_Z3maxDv2_iS_";
  FunctionDescriptor fd = demangle(name);
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ("max", fd.name);
  ASSERT_EQ(2U, fd.parameters.size());
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 1));
}

TEST(DemangleTest, voidptr){
  FunctionDescriptor fd = demangle("_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i");
  ASSERT_FALSE(fd.isNull());
  ASSERT_STREQ("trans_coord_int_NONE_FALSE_NEAREST", fd.name.c_str());
  ASSERT_EQ(2U, fd.parameters.size());
}

TEST(DemangleTest, userDefinedTy1){
  FunctionDescriptor fd = demangle("_Z6myfunc4myTy");
  ASSERT_FALSE(fd.isNull());
  ASSERT_STREQ("myfunc", fd.name.c_str());
  ASSERT_EQ(std::string("myTy"), getParameterString(fd, 0));
  ASSERT_EQ(1U, fd.parameters.size());
}

//tests user defined types, one after the other
TEST(DemangleTest, userDefinedTy2){
  FunctionDescriptor fd = demangle("_Z6myfunc5myTy16myTy21");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("myTy1"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("myTy21"), getParameterString(fd, 1));
}

//Address space tests

TEST(DemangleTest, addressSpace){
  ASSERT_TRUE( testDemangle( "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh"));
}

TEST(DemangleTest, addressSpace1){
  FunctionDescriptor fd = demangle("_Z3myfPU3AS1i");
  ASSERT_FALSE(fd.isNull());
  PointerType* p = reflection::dyn_cast<PointerType>(fd.parameters[0]);
  ASSERT_TRUE(p);
  ASSERT_EQ(ATTR_GLOBAL, *p->getAttributes().begin());
}

TEST(DemangleTest, addressSpaceAndUserDefTy){
  FunctionDescriptor fd = demangle("_Z3myfPU3AS23mta");
  ASSERT_FALSE(fd.isNull());
  PointerType* p = reflection::dyn_cast<PointerType>(fd.parameters[0]);
  ASSERT_TRUE(p);
  ASSERT_EQ(ATTR_CONSTANT, *p->getAttributes().begin());
  ASSERT_EQ(std::string("mta"), p->getPointee()->toString());
}

// Test for Apple (deprecated)
//TEST(DemangleTest, appleImageMangle){
//  const char*const strImagefunction = "_Z11read_imageiPU3AS110_image2d_t11ocl_samplerDv2_i";
//  FunctionDescriptor fd = demangle(strImagefunction);
//  ASSERT_FALSE(fd.isNull());
//  ASSERT_TRUE(reflection::dyn_cast<PointerType>(fd.parameters[0]));
//  std::string strMangled = mangle(fd);
//  ASSERT_STREQ(strMangled.c_str(), strImagefunction);
//}
//
//Manlge
//

TEST(MangleTest, _Z17convert_float_satc){
  std::string orig = "_Z17convert_float_satc";
  FunctionDescriptor fd = demangle( orig.c_str() );
  ASSERT_FALSE(fd.isNull());
  std::string syntesized = mangle( fd );
  ASSERT_EQ(orig, syntesized);
}

TEST(MangleTest, ignorePrivateAddrSpaceQual){
  // Built-in declaration:
  //
  // bool atomic_compare_exchange_weak_explicit(
  //           __local volatile atomic_int *object,
  //           __private int *expected,  <--- private address space
  //           int desired,                   qualifier is not mangled
  //           memory_order success,
  //           memory_order failure);

  std::string orig = "_Z37atomic_compare_exchange_weak_explicitPU3AS3VU7_AtomiciPii12memory_orderS3_";
  FunctionDescriptor fd = demangle( orig.c_str() );
  ASSERT_FALSE(fd.isNull());
  std::string syntesized = mangle( fd );
  ASSERT_EQ(orig, syntesized);

  // Check that __private has no affect on mangling
  PointerType *Ty = reinterpret_cast<PointerType *>((ParamType *)fd.parameters[1]);
  Ty->addAttribute(reflection::ATTR_PRIVATE);
  std::string syntesizedWithPrivate = mangle( fd );
  ASSERT_EQ(syntesized, syntesizedWithPrivate);
}


//A special case, in which the param duplicate operator works to duplicate a
//type suffix, not the enture type. The first parameter is v2f, the second is v2f*.
TEST(MangleTest, semidup){
  std::string orig = "_Z5fractDv2_fPU3AS1S_";
  FunctionDescriptor fd = demangle( orig.c_str() );
  ASSERT_FALSE(fd.isNull());
  std::string actual = mangle( fd );
  ASSERT_EQ(orig, actual);
}

const char* strDoubleDup = "_Z4stamDv4_fS_PS_S0_" ;

TEST(MangleTest, doubleDup){
  FunctionDescriptor fd;
  RefParamType primitiveFloat(new PrimitiveType(PRIMITIVE_FLOAT));
  RefParamType vectorFloat(new VectorType(primitiveFloat, 4));
  RefParamType ptrFloat(new PointerType(vectorFloat));

  fd.name = "stam";
  fd.parameters.push_back(vectorFloat);
  fd.parameters.push_back(vectorFloat);
  fd.parameters.push_back(ptrFloat);
  fd.parameters.push_back(ptrFloat);

  std::string mangled = mangle(fd);
  ASSERT_STREQ(strDoubleDup, mangled.c_str());
}

TEST(DemangleTest, doubleDup1){
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.parameters[0];
  ASSERT_TRUE(isVector(T));
  const VectorType *pVecTy = reflection::dyn_cast<VectorType>(T);
  const PrimitiveType *pSclTy = reflection::dyn_cast<PrimitiveType>(pVecTy->getScalarType());
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());
}

TEST(DemangleTest, doubleDup2){
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.parameters[1];
  ASSERT_TRUE(isVector(T));
  const VectorType *pVecTy = reflection::dyn_cast<VectorType>(T);
  const PrimitiveType *pSclTy = reflection::dyn_cast<PrimitiveType>(pVecTy->getScalarType());
  ASSERT_TRUE(pSclTy != NULL);
  ASSERT_EQ(PRIMITIVE_FLOAT, pSclTy->getPrimitive());
}

TEST(DemangleTest, doubleDup3){
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  ASSERT_TRUE(isPointer(fd.parameters[2]));
}

TEST(DemangleTest, doubleDup4){
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.parameters[2];
  ASSERT_TRUE(isPointer(T));
  const PointerType *P = reflection::dyn_cast<PointerType>(T);
  ASSERT_TRUE(isVector(P->getPointee()));
}

TEST(DemangleTest, doubleDup5){
  FunctionDescriptor fd = demangle(strDoubleDup);
  ASSERT_FALSE(fd.isNull());
  RefParamType T = fd.parameters[3];
  ASSERT_TRUE(isPointer(T));
  const PointerType *P = reflection::dyn_cast<PointerType>(T);
  ASSERT_TRUE(isVector(P->getPointee()));
}

TEST(DemangleTest, doubleDup6){
  FunctionDescriptor fd =
    demangle("_Z3fooDv2_iDv2_jDv2_cDv2_hDv4_fDv8_dS_S0_S1_S2_S3_S4_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(
    std::string("foo(int2, uint2, char2, uchar2, float4, double8, int2, uint2, char2, uchar2, float4, double8)")
    , fd.toString()
  );
}

TEST(DemangleTest, doubleDup7){
  FunctionDescriptor fd =
    demangle("_Z3fooPDv4_fS_S0_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(
    std::string("foo(float4 *, float4, float4 *)")
    , fd.toString()
  );
}

TEST(DemangleTest, doubleDup8){
  FunctionDescriptor fd =
    demangle("_Z3fooPiPjPcPhPfPdS_S0_S1_S2_S3_S4_Dv4_iDv4_jDv4_cDv4_hDv4_fDv4_dS5_S6_S7_S8_S9_SA_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(
    std::string("foo(int *, uint *, char *, uchar *, float *, "
    "double *, int *, uint *, char *, uchar *, float *, "
    "double *, int4, uint4, char4, uchar4, float4, double4, int4, uint4, char4, uchar4, float4, double4)")
    , fd.toString()
  );
}

TEST(DemangleTest, doubleDup9){
  FunctionDescriptor fd = demangle("_Z3fooP4sFooS0_S_");
  ASSERT_FALSE(fd.isNull());
  ASSERT_EQ(std::string("foo(sFoo *, sFoo *, sFoo)"), fd.toString());
}


TEST(MangleBasic, scalarfloat){
  RefParamType primitiveFloat(new PrimitiveType(PRIMITIVE_FLOAT));
  FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(primitiveFloat);
  ASSERT_STREQ("_Z3foof", mangle(fd).c_str());
}

TEST(MangleBasic, scalardouble){
  RefParamType primitiveDouble(new PrimitiveType(PRIMITIVE_DOUBLE));
  FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(primitiveDouble);
  ASSERT_STREQ("_Z3food", mangle(fd).c_str());
}

//
//MangleAPI
//

TEST(MangleAPI, visitorExample){
  const char* soaFunc = "_Z5dummyiDv4_fPS_" ;
  FunctionDescriptor fd = demangle(soaFunc);
  struct PrintTypeVisitor: TypeVisitor{
    int ordinal;

    std::string ordinalStr(){
      std::stringstream ss;
      ss << "parameter " << ++ordinal << ":\t";
      return ss.str();
    }

    PrintTypeVisitor(): ordinal(0){}

    void visit(const PrimitiveType* p){
      std::cout << "primitive " << p->toString() << std::endl;
    }

    void visit(const VectorType* p){
      std::cout << "vector with length " << p->getLength() << " with ";
      p->getScalarType()->accept(this);
    }

    void visit(const PointerType* p){
      std::cout << "pointer to ";
      p->getPointee()->accept(this);
    }

    void visit(const AtomicType* p){
      std::cout << "atomic " << p->toString() << std::endl;
    }

    void visit(const BlockType* p){
      std::cout << "block ( ";
      for (unsigned int i=0; i<p->getNumOfParams(); ++i) {
        p->getParam(i)->accept(this);
      }
      std::cout << ")";
    }

    void visit(const UserDefinedType*){
      std::cout << ordinalStr();
      std::cout << "user defined type. in OCL, its images..." << std::endl;
    }
  };
  TypeVector::iterator bi = fd.parameters.begin(),
    e = fd.parameters.end();
  PrintTypeVisitor printVisitor;
  while (bi != e){
    std::cout << printVisitor.ordinalStr();
    (*bi)->accept(&printVisitor);
    ++bi;
  }
}

TEST(Type, TypeCast){
  RefParamType primitiveInt(new PrimitiveType(PRIMITIVE_INT));
  VectorType vectorInt(primitiveInt, 4);
  ASSERT_TRUE(NULL == reflection::dyn_cast<VectorType>(primitiveInt));
  ASSERT_EQ(&vectorInt, reflection::dyn_cast<VectorType>(&vectorInt));
}

TEST(MemoryLeaks, Vector){
  const char* s = "_Z5frexpDv2_i";
  demangle(s);
}

TEST(MemoryLeaks, PointerToPrimitive){
  const char* s = "_Z5frexpPi";
  demangle(s);
}

TEST(MemoryLeaks, PointerToVector){
  const char* s = "_Z5frexpPDv2_i";
  demangle(s);
}

TEST(MemoryLeaks, PointerAttribToVector){
  const char* s = "_Z5frexpPU3AS1Dv2_i";
  demangle(s);
}

TEST(MemoryLeaks, StructTy){
  FunctionDescriptor fd = demangle("_Z1f2xx");
  ASSERT_FALSE(fd.isNull());
  EXPECT_TRUE(reflection::dyn_cast<UserDefinedType>(fd.parameters[0]));
}

}//end namespace test
}//end namespace namemangling

int main(int argc, char** argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
