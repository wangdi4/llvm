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

#include "gtest/gtest.h"
#include "NameMangleAPI.h"
#include "antlr/ANTLRException.hpp"
#include "Type.h"
#include "FunctionDescriptor.h"
#include "TypeCast.h"
#include <algorithm>
#include <cctype>

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
      FunctionDescriptor fd = demangle(mname);
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

TEST(NameMangle, FailedOnce){
  const char* s = "_Z5frexpDv2_fPU3AS1Dv2_i";
  FunctionDescriptor fd = demangle(s);
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}
TEST(NameMangle, Failed2){
  const char* s = "_Z9mask_fmaxtDv16_fS_";
  FunctionDescriptor fd = demangle(s);
  std::string mangled = mangle(fd);
  ASSERT_STREQ(mangled.c_str(), s);
}

TEST(NameMangle, SOAFunction){
  FunctionDescriptor soaDescriptor;
  soaDescriptor.name = "soa";
  Type* doubleTy = new Type(primitives::DOUBLE);
  Type* intTy = new Type(primitives::INT);
  Pointer* pintTy = new Pointer(intTy);
  soaDescriptor.parameters.push_back(doubleTy->clone());
  soaDescriptor.parameters.push_back(pintTy->clone());
  soaDescriptor.parameters.push_back(pintTy->clone());
  std::cout << soaDescriptor.toString() << std::endl;
  ASSERT_EQ(std::string("_Z3soadPiS_"), mangle(soaDescriptor));
  delete doubleTy;
  delete intTy;
  delete pintTy;
}

static bool testDemangle(const char* mname){
  try{
    printf( "%s\n", demangle(mname).toString().c_str());
    return true;
  } catch (antlr::ANTLRException antlrEx){
    return false;
  }
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

//
//This function might be mistakingly considered as an image function
//
TEST(DemangleTest, imageAmbiguity){
  const char* name = "_Z11mask_vstoretDv16_imPU3AS1i";
  ::testing::AssertionSuccess() << "demangling " << name; // << std::endl;
  FunctionDescriptor fd = demangle(name);
  ASSERT_EQ("mask_vstore", fd.name);
  ASSERT_EQ(4U, fd.parameters.size());
  ASSERT_EQ(std::string("ushort"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int16"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("ulong"), getParameterString(fd, 2));
  ASSERT_EQ(std::string("__global int *"), getParameterString(fd, 3));
}

TEST(DemangleTest, imageBuiltin){
  const char* name = "_Z11read_imagefP10_image2d_tjDv2_f";
  FunctionDescriptor fd = demangle(name);
  ASSERT_EQ("read_imagef", fd.name);
  ASSERT_EQ(3U, fd.parameters.size());
  ASSERT_EQ(std::string("_image2d_t *"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("uint"), getParameterString(fd, 1));
  ASSERT_EQ(std::string("float2"), getParameterString(fd, 2));
}

TEST(DemangleTest, duplicatedVector){
  const char* name = "_Z3maxDv2_iS_";
  FunctionDescriptor fd = demangle(name);
  ASSERT_EQ("max", fd.name);
  ASSERT_EQ(2U, fd.parameters.size());
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("int2"), getParameterString(fd, 1));
}

TEST(DemangleTest, voidptr){
  FunctionDescriptor fd = demangle("_Z34trans_coord_int_NONE_FALSE_NEARESTPvDv4_i");
  ASSERT_STREQ("trans_coord_int_NONE_FALSE_NEAREST", fd.name.c_str());
  ASSERT_EQ(2U, fd.parameters.size());
}

TEST(DemangleTest, userDefinedTy1){
  FunctionDescriptor fd = demangle("_Z6myfunc4myTy");
  ASSERT_STREQ("myfunc", fd.name.c_str());
  ASSERT_EQ(std::string("myTy"), getParameterString(fd, 0));
  ASSERT_EQ(1U, fd.parameters.size());
}

//tests user defined types, one after the other
TEST(DemangleTest, userDefinedTy2){
  FunctionDescriptor fd = demangle("_Z6myfunc5myTy16myTy21");
  ASSERT_EQ(std::string("myTy1"), getParameterString(fd, 0));
  ASSERT_EQ(std::string("myTy21"), getParameterString(fd, 1));
}

//Address space tests

TEST(DemangleTest, addressSpace){
  ASSERT_TRUE( testDemangle( "_Z17vstore_half16_rtzDv16_fmPU3AS1Dh"));
}

TEST(DemangleTest, addressSpace1){
  FunctionDescriptor fd = demangle("_Z3myfPU3AS1i");
  Pointer* p = cast<Pointer>(fd.parameters[0]);
  ASSERT_TRUE(p);
  std::vector<std::string>::const_iterator it = p->beginAttributes();
  ASSERT_EQ(std::string("__global"), *it);
}

TEST(DemangleTest, addressSpaceAndUserDefTy){
  FunctionDescriptor fd = demangle("_Z3myfPU3AS23mta");
  Pointer* p = cast<Pointer>(fd.parameters[0]);
  ASSERT_TRUE(p);
  ASSERT_EQ(std::string("__constant"), *p->beginAttributes());
  ASSERT_EQ(std::string("mta"), p->getPointee()->toString());
}

TEST(DemangleTest, appleImageMangle){
  const char*const strImagefunction = "_Z11read_imageiPU3AS110_image2d_tuSamplerDv2_i";
  FunctionDescriptor fd = demangle(strImagefunction);
  ASSERT_FALSE(fd.isNull());
  ASSERT_TRUE(reflection::cast<Pointer>(fd.parameters[0]));
  std::string strMangled = mangle(fd);
  ASSERT_STREQ(strMangled.c_str(), strImagefunction);
}
//
//Manlge
//

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

TEST(MangleTest, doubleDup){
  FunctionDescriptor fd;
  Type* primitiveFloat = new Type(primitives::FLOAT);
  Vector* vectorFloat = new Vector(primitiveFloat, 4);
  Pointer*  ptrFloat = new Pointer(vectorFloat);

  fd.name = "stam";
  fd.parameters.push_back(vectorFloat);
  fd.parameters.push_back(vectorFloat->clone());
  fd.parameters.push_back(ptrFloat);
  fd.parameters.push_back(ptrFloat->clone());

  std::string mangled = mangle(fd);
  ASSERT_STREQ("_Z4stamDv4_fS_PS_S0_", mangled.c_str());
  const char* soaFunc = "_Z10soa_cross3Dv4_fS_S_S_S_S_PS_S0_S0_" ;
  fd = demangle(soaFunc);
  ASSERT_STREQ(soaFunc, mangle(fd).c_str());
  delete primitiveFloat;
}

TEST(MangleBasic, scalarfloat){
  Type* primitiveFloat = new Type(primitives::FLOAT);
  FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(primitiveFloat->clone());
  ASSERT_STREQ("_Z3foof", mangle(fd).c_str());
  delete primitiveFloat;
}

TEST(MangleBasic, scalardouble){
  Type* primitiveDouble = new Type(primitives::DOUBLE);
  FunctionDescriptor fd;
  fd.name = "foo";
  fd.parameters.push_back(primitiveDouble->clone());
  ASSERT_STREQ("_Z3food", mangle(fd).c_str());
  delete primitiveDouble;
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

    void visit(const Type* p){
      Type primitive(p->getPrimitive());
      std::cout << "primitive " << primitive.toString() << std::endl;
    }

    void visit(const Vector* p){
      std::cout << "vector with length " << p->getLen() << " with ";
      visit((Type*)p);
    }

    void visit(const Pointer* p){
      std::cout << "pointer to ";
      p->getPointee()->accept(this);
    }

    void visit(const UserDefinedTy*){
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
  Type primitiveInt(primitives::INT);
  Vector vectorInt(&primitiveInt, 4);
  ASSERT_EQ( NULL, cast<Vector>(&primitiveInt));
  ASSERT_EQ( &vectorInt, cast<Vector>(&vectorInt));
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
  EXPECT_TRUE(reflection::cast<UserDefinedTy>(fd.parameters[0]));
  std::cout << fd.parameters[0]->toString() << std::endl;
}

}//end namespace test
}//end namespace namemangling

int main(int argc, char** argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
