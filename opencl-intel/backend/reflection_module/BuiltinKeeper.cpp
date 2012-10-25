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

  File Name: BuiltinKeeper.cpp
\****************************************************************************/

#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"
#include <cctype>
#include <sstream>
#include  "llvm/Support/MutexGuard.h"
#include  "llvm/ADT/ArrayRef.h"

namespace reflection{

//
//Utilities
//

//Purpose: indicates whether the two given function descriptors are compatible
//to each other.
//Definition: C : FD X FD is a binary relation of function descriptors.
//Let fd1 and fd2 be two function descriptors. We say that fd1 C fd2 IFF
//a) fd1 and fd2 have the same number of parameters.
//b) for i : {1...n} (n being the number of arguments) the i'th parameter of
//fd1 has the same primitive type as the i'th parameter of fd2.
static bool
compatible(const FunctionDescriptor& l, const FunctionDescriptor& r){
  typedef std::vector<Type*>::const_iterator TypeIter;
  if (l.parameters.size() != r.parameters.size())
    return false;
  TypeIter lit = l.parameters.begin(), lend = l.parameters.end(),
  rit = r.parameters.begin();
  while(lit != lend){
    if ((*lit)->getPrimitive() != (*rit)->getPrimitive())
      return false;
    ++lit;
    ++rit;
  }
  return true;
}

//Purpose: creates a function descriptor from the given order quartet
////(name:String, vp:Primitive, w:Width, s:Primitive) in the following way:
//FunctionDescriptor (name, vwp0, p1), name being the (stripped)
//name of the function, with two parameters: the first, a vector with 'w'
//elements of type 'p0', and the second is a scalar, also of type 'p1',
//where 'p0' is the second element it arg, and p1 is the second argument passed
//to the function.
//Note: the parameter are allocated on the heap, and their ownership is
//transfered to the caller of this function.
static FunctionDescriptor createDescriptorVP_P (
  const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>& arg,
  primitives::Primitive PTy)
{
  FunctionDescriptor fd;
  fd.name = arg.first.first.str();
  Type pPrimitiveTy0(arg.first.second);
  Type* pVectorTy;
  if ( width::SCALAR == arg.second)
    pVectorTy = pPrimitiveTy0.clone();
  else
    pVectorTy = new Vector(&pPrimitiveTy0, static_cast<int>(arg.second));
  Type* pPrimitiveTy1 = new Type(PTy);
  fd.parameters.push_back(pVectorTy);
  fd.parameters.push_back(pPrimitiveTy1);
  return fd;
}

//similar to createDescriptorVP_P, only the vector parameter is the second one,
//and the scalar is the first.
static FunctionDescriptor createDescriptorP_VP (
  const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>& arg,
  primitives::Primitive PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  std::iter_swap(fd.parameters.begin(), fd.parameters.begin()+1);
  return fd;
}

//creates a function descriptor with three parameters in a similar manner to
//the two parameter version  (createDescriptorVP_P), with a third scalar
//parameter
static FunctionDescriptor createDescriptorVP_P_P(
const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>& arg,
primitives::Primitive PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  fd.parameters.push_back(fd.parameters[1]);
  return fd;
}

//creates a function descriptor with three parameters, in the following format:
//(vtype, vtype, stype)
static FunctionDescriptor createDescriptorVP_VP_P(
const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>& arg,
primitives::Primitive PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  //duplicating the first (vector) parameter
  Type* pVector = *(fd.parameters.begin());
  fd.parameters.insert(fd.parameters.begin(), pVector);
  return fd;
}

//creates a function descriptor with three parameters in a similar manner to
//the three parameter version  (createDescriptorVP_P_P), only that the vector
//parameter is third, not first: (type, type, vtype).
static FunctionDescriptor createDescriptorP_P_VP(
const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>& arg,
primitives::Primitive PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P_P(arg, PTy);
  std::iter_swap(fd.parameters.begin(), fd.parameters.begin()+2);
  return fd;
}

//
//BuiltinKeeper
//

void BuiltinKeeper::addConversionGroup (const StringArray& names,
  const PrimitiveArray& types, FDFactory fdFactory){
  for (unsigned i=0 ; i<types.size() ;++i){
    PrimitiveArray singleton = const_cast<PrimitiveArray&>(types).slice(i,1U);
    addConversionGroup(names, singleton, types[i], fdFactory);
  }
}

llvm::sys::Mutex mutex;

BuiltinKeeper* BuiltinKeeper::Instance = NULL;

BuiltinKeeper::BuiltinKeeper(){
  populateReturnTyMap(); 
  m_soaStrategy.setTypeMap(&m_fdToRetTy);
  initNullStrategyEntries();
  initSoaStrategyEntries();
  initHardCodeStrategy();
  //the rest of the functions are versioned by the 'default' strategy, which is
  //to follow the tblgen generated tables
}

//
//strategies initialization
//

void BuiltinKeeper::initSoaStrategyEntries(){
  ReturnTypeMap::const_iterator it=m_fdToRetTy.begin(), e=m_fdToRetTy.end();
  while(it != e){
    addTransposGroup(it->first);
    ++it;
  }
}

void BuiltinKeeper::initNullStrategyEntries(){
  width::V vwidths[] = {width::SCALAR, width::TWO, width::THREE, width::FOUR,
  width::EIGHT, width::SIXTEEN};
  primitives::Primitive primitivesTy[] = {
    primitives::CHAR,    //1
    primitives::SHORT,   //2
    primitives::INT,     //3
    primitives::LONG,    //4
    primitives::UCHAR,   //5
    primitives::USHORT,  //6
    primitives::UINT,    //7
    primitives::ULONG,   //8
    primitives::FLOAT,   //9
    primitives::DOUBLE}; //10
  //arrays of types
  PrimitiveArray arrPrimitives(primitivesTy);
  PrimitiveArray arrReals = arrPrimitives.slice(8, 2);
  //arrays of vector width
  VWidthArray arrNonScalars(vwidths+1, (sizeof(vwidths)/sizeof(width::V))-1);
  VWidthArray arrAllWidth(vwidths);
  //adding the 'ambiguous scalar' builtin functions
  //
  //fmin/fmax
  //
  {
  llvm::StringRef names[] = {"fmin", "fmax"};
  StringArray arrNames (names);
  reflection::primitives::Primitive singleFloat[] = {primitives::FLOAT};
  reflection::primitives::Primitive singleDouble[] = {primitives::DOUBLE};
  PrimitiveArray arrFloat(singleFloat);
  addConversionGroup(arrNames, arrFloat, createDescriptorVP_P);
  PrimitiveArray arrDouble(singleDouble);
  addConversionGroup(arrNames, arrDouble, createDescriptorVP_P);
  }
  //
  //min/max
  //
  {
  llvm::StringRef names[] = {"min", "max"};
  StringArray arrNames (names);
  addConversionGroup(arrNames, arrPrimitives, createDescriptorVP_P);
  }
  //
  //ldexp
  //
  StringArray arrLdexp("ldexp");
  addConversionGroup(arrLdexp, arrReals, primitives::INT, createDescriptorVP_P);
  //
  //clamp
  //
  {
  llvm::StringRef names[] = {"clamp"};
  StringArray arrNames (names);
  addConversionGroup(arrNames, arrPrimitives, createDescriptorVP_P_P);
  }
  //
  //mix
  //
  {
  llvm::StringRef names[] = {"mix"};
  StringArray arrNames (names);
  addConversionGroup(arrNames, arrReals, createDescriptorVP_VP_P);
  }
  //
  //step
  //
  {
  llvm::StringRef names[] = {"step"};
  StringArray arrNames (names);
  addConversionGroup(arrNames, arrReals, createDescriptorP_VP);
  }
  //
  //smooth step
  //
  {
  llvm::StringRef names[] = {"smoothstep"};
  StringArray arrNames (names);
  addConversionGroup(arrNames, arrReals, createDescriptorP_P_VP);
  }
  //
  //Scalar versions of SOA functions
  //
  ReturnTypeMap::const_iterator it=m_fdToRetTy.begin(), e=m_fdToRetTy.end();
  while(it != e){
    //We do not allow soa versiones to be scalarized
    PairSW exceptionsKey (std::make_pair(mangle(it->first), width::SCALAR));
    m_exceptionsMap[exceptionsKey] = &m_nullStrategy;
    ++it;
  }
  //
  //This function cluster cannot be scalarized nor packetized due to OUT return
  //values
  //
  {
  llvm::StringRef names[] = {"_Z5fract*", "_Z5frexp*", "_Z8lgamma_r*", "_Z4modf*",
  "_Z6remquo*", "_Z6sincos*"};
  StringArray pointeredBuiltins(names);
  VWidthArray allWidths(vwidths);
  Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> pairs(pointeredBuiltins,
    allWidths);
  do{
    PairSW key(pairs.get());
    m_exceptionsMap.insert(std::make_pair(key, &m_nullStrategy));
  }while(pairs.next());
  }
  //this function cluster cannot be versioned due the relationals difference in
  //prototype between the scalar versions and the vectorized ones.
  {
    llvm::StringRef names[] = {"_Z7signbit*", "_Z7isequal*", "_Z9isgreater*",
    "_Z10isnotequal*", "_Z14isgreaterequal*", "_Z6isless*", "_Z11islessequal*",
    "_Z13islessgreater*", "_Z8isfinite*", "_Z5isinf*", "_Z5isnan*",
    "_Z8isnormal*", "_Z9isordered*", "_Z11isunordered*"};
    StringArray relationals(names);
    VWidthArray allWidths(vwidths);
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> pairs(relationals,
      allWidths);
    do{
      PairSW key(pairs.get());
      m_exceptionsMap.insert(std::make_pair(key, &m_nullStrategy));
    }while(pairs.next());
  }
}

static void convertToRef(const char* from[width::OCL_VERSIONS],
  llvm::StringRef to[width::OCL_VERSIONS]){
  for (unsigned i=0 ; i<width::OCL_VERSIONS ; ++i)
    to[i] = llvm::StringRef(from[i]);
}

#include "CustomVersionMaping.h"

void BuiltinKeeper::initHardCodeStrategy(){
  width::V sizes[] = {width::SCALAR, width::TWO, width::THREE, width::FOUR,
    width::EIGHT, width::SIXTEEN};
  llvm::ArrayRef<width::V> arrSizes(sizes);
  size_t SIZE = sizeof(mappings)/(sizeof(mappings[0]));
  for(size_t i=0 ; i<SIZE ; ++i){
    m_hardCodedStrategy.assumeResponsability(mappings[i]);
    llvm::StringRef strraw[width::OCL_VERSIONS];
    convertToRef(mappings[i].names, strraw);
    llvm::ArrayRef<llvm::StringRef> arr(strraw);
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> pairs(arr, arrSizes);
    do{
      PairSW key(pairs.get());
      m_exceptionsMap.insert(std::make_pair(key, &m_hardCodedStrategy));
    } while(pairs.next());
  }
}

void BuiltinKeeper::addConversionGroup (const StringArray& names,
  const PrimitiveArray& types, primitives::Primitive p, FDFactory fdFactory){
  //Data Arrays
  width::V vwidths[] = {width::TWO, width::THREE, width::FOUR,
    width::EIGHT, width::SIXTEEN};
  Cartesian<llvm::ArrayRef,llvm::StringRef,primitives::Primitive> product2ways(
    names, types);
  std::vector<std::pair<llvm::StringRef, primitives::Primitive> > tmpResult;
  do{
    tmpResult.push_back(product2ways.get());
  } while(product2ways.next());
  llvm::ArrayRef<std::pair<llvm::StringRef,primitives::Primitive> >arrTmpResult(
    &tmpResult[0], tmpResult.size());
  //arrays of vector width
  VWidthArray arrNonScalars(vwidths);
  Cartesian<llvm::ArrayRef, std::pair<llvm::StringRef, primitives::Primitive>,
    width::V> threeWayProduct(arrTmpResult, arrNonScalars);
  do{
    FunctionDescriptor fd = fdFactory(
      threeWayProduct.get(), //name, primitive type, vector-width
      p);
    std::string strname = mangle(fd);
    for( size_t i=0 ; i<arrNonScalars.size() ; ++i){
      PairSW exceptionsKey = std::make_pair(strname, arrNonScalars[i]);
      m_exceptionsMap[exceptionsKey] = &m_nullStrategy;
    }
  } while(threeWayProduct.next());
}

void BuiltinKeeper::addTransposGroup(const FunctionDescriptor& aosDescriptor){
  width::V aosWidth[] = {width::SCALAR, width::FOUR, width::EIGHT, width::SIXTEEN};
  std::string strAos = mangle(aosDescriptor);
  for(size_t i=0 ; i<(sizeof(aosWidth)/sizeof(width::V)) ; ++i){
    PairSW exceptionsKey = std::make_pair(strAos, aosWidth[i]);
    m_exceptionsMap[exceptionsKey] = &m_soaStrategy;
  }
}

template<int w>
static std::pair<FunctionDescriptor, Type*>
createBiV_V(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type primitiveTy(p);
  Type* vTy = new Vector(&primitiveTy, w);
  fd.parameters.push_back(vTy);
  fd.parameters.push_back(vTy->clone());
  return std::make_pair(fd, vTy->clone());
}

template<>
std::pair<FunctionDescriptor, Type*>
createBiV_V<1>(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type primitiveTy(p);
  fd.parameters.push_back(primitiveTy.clone());
  fd.parameters.push_back(primitiveTy.clone());
  return std::make_pair(fd, primitiveTy.clone());
}

template<int w>
static std::pair<FunctionDescriptor, Type*>
createBiV_S(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type scalar(p);
  Type* vTy = new Vector(&scalar, w);
  fd.parameters.push_back(vTy);
  fd.parameters.push_back(vTy->clone());
  return std::make_pair(fd, scalar.clone());
}

template<> inline
std::pair<FunctionDescriptor, Type*>
createBiV_S<1>(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type* scalar = new Type(p);
  fd.parameters.push_back(scalar);
  fd.parameters.push_back(scalar->clone());
  return std::make_pair(fd, scalar->clone());
}

template<int w>
std::pair<FunctionDescriptor, Type*>
createUniV_S(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type scalar(p);
  Type* vTy = new Vector(&scalar, w);
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, scalar.clone());
}

template<>
std::pair<FunctionDescriptor, Type*>
createUniV_S<1>(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type* scalar = new Type(p);
  fd.parameters.push_back(scalar);
  return std::make_pair(fd, scalar->clone());
}

template <int w>
std::pair<FunctionDescriptor, Type*>
createUniV_V(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type scalar(p);
  Type* vTy = new Vector(&scalar, w);
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, vTy->clone());
}

template <>
std::pair<FunctionDescriptor, Type*>
createUniV_V<1>(primitives::Primitive p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  Type scalar(p);
  fd.parameters.push_back(scalar.clone());
  return std::make_pair(fd, scalar.clone());
}

void BuiltinKeeper::populateReturnTyMap(){
  //
  //cross
  //
  m_fdToRetTy.insert(createBiV_V<3>(primitives::DOUBLE, "cross"));
  m_fdToRetTy.insert(createBiV_V<3>(primitives::FLOAT, "cross"));
  m_fdToRetTy.insert(createBiV_V<4>(primitives::DOUBLE, "cross"));
  m_fdToRetTy.insert(createBiV_V<4>(primitives::FLOAT, "cross"));
  //
  //dot
  //
  m_fdToRetTy.insert(createBiV_S<1>(primitives::FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<2>(primitives::FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<3>(primitives::FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<4>(primitives::FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<1>(primitives::DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<2>(primitives::DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<3>(primitives::DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<4>(primitives::DOUBLE, "dot"));
  //
  //distance
  //
  m_fdToRetTy.insert(createBiV_S<1>(primitives::FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<2>(primitives::FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<3>(primitives::FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<4>(primitives::FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<1>(primitives::DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<2>(primitives::DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<3>(primitives::DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<4>(primitives::DOUBLE, "distance"));
  //
  //fast_distance
  //
  m_fdToRetTy.insert(createBiV_S<1>(primitives::FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<2>(primitives::FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<3>(primitives::FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<4>(primitives::FLOAT, "fast_distance"));
  //
  //fast length
  //
  m_fdToRetTy.insert(createUniV_S<1>(primitives::FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::FLOAT, "fast_length"));
  //
  //fast_normalize
  //
  m_fdToRetTy.insert(createUniV_V<1>(primitives::FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(primitives::FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(primitives::FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(primitives::FLOAT, "fast_normalize"));
  //
  //length
  //
  m_fdToRetTy.insert(createUniV_S<1>(primitives::FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::DOUBLE, "length"));
  //
  //normalize
  //
  m_fdToRetTy.insert(createUniV_V<1>(primitives::FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(primitives::FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(primitives::FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(primitives::FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<1>(primitives::DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(primitives::DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(primitives::DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(primitives::DOUBLE,"normalize"));
  //
  //any
  //
  m_fdToRetTy.insert(createUniV_S<1>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::INT, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::LONG, "any"));
  //
  //all
  //
  m_fdToRetTy.insert(createUniV_S<1>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::INT, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(primitives::LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(primitives::LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(primitives::LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(primitives::LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(primitives::LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(primitives::LONG, "all"));
}

const BuiltinKeeper* BuiltinKeeper::instance(){
  {
    llvm::MutexGuard gaurd(mutex);
    if (!Instance)
      Instance = new BuiltinKeeper();
  }
  return Instance;
}

//Wrapper around the range struct
struct RangeUtil{
  RangeUtil(BuiltinMap::MapRange& mr): m_range(mr){}

  //indicates whether the given range is empty
  bool isEmpty()const{
    return (m_range.first == m_range.second);
  }

  void increment(){
    assert ((m_range.first != m_range.second) && "range is empty!");
    m_range.first++;
  }

  const FunctionDescriptor& getDescriptor()const{
    return *m_range.first;
  }
private:
  BuiltinMap::MapRange& m_range;
};

static bool isOverloaded(const std::string& name){
  const std::string prefix = "_Z";
  return prefix == name.substr(0, prefix.length());
}

bool BuiltinKeeper::isInExceptionMap(const std::string& name)const{
  reflection::width::V allWidth[] = {width::SCALAR, width::TWO, width::FOUR,
    width::EIGHT, width::SIXTEEN, width::THREE};
  for (unsigned i=0 ; i<width::OCL_VERSIONS; ++i){
    reflection::width::V w = allWidth[i];
    VersionCBMap::const_iterator it =
      m_exceptionsMap.find(std::make_pair(name, w));
    if (m_exceptionsMap.end() != it)
      return true;
  }
  return false;
}

bool BuiltinKeeper::isBuiltin(const std::string& mangledString)const{
  if (mangledString.empty())
    return false;
  bool isBi = isInExceptionMap(mangledString);
  if ( !isOverloaded(mangledString) )
    return isBi;
  //we need to execute isBuiltin(FunctionDescriptor) whenever the function
  //isn't overloaded, since it has a side effect
  return isBuiltin (demangle(mangledString.c_str())) || isBi;
}

#include "BuiltinList.h"

bool BuiltinKeeper::isBuiltin(const FunctionDescriptor& fd)const{
  BuiltinMap::MapRange mr = m_descriptorsMap.equalRange(fd.name);
  RangeUtil range(mr);
  //is cache line present?
  if ( !range.isEmpty() ){
    //Note: the invariant of the cache is, that all the versions of a built-in
    //are either in the cache, or none of them is. (they are all loaded
    //together)
    do{
      if (range.getDescriptor() == fd)
        return true;
      range.increment();
    } while (!range.isEmpty());
    return false;
  }
  assert(range.isEmpty() && "internal bug");
  bool bFound = false, bLineCached = false;
  for (size_t i=0 ; i<(sizeof(mangledNames)/sizeof(char*)) ; ++i){
    llvm::StringRef strippedName = stripName(mangledNames[i]);
    if ( m_descriptorsMap.isInSameCacheLine(fd.name, strippedName) ){
      //cache the builtin we demangle
      FunctionDescriptor candidate = demangle(mangledNames[i]);
      candidate.assignAutomaticWidth();
      m_descriptorsMap.insert(candidate);
      bLineCached = true;
      bFound |= (fd == candidate);
    //if the cache-line was inserted, we can notify the result, since all
    //overloads are grouped together in the mangled names array
    } else if (bLineCached)
        return bFound;
  }
  return false;
}

PairSW BuiltinKeeper::getVersion(const std::string& name, width::V w)const
#if !defined(_WIN32) && !defined(_WIN64)
 //cl compiler doesn't approve that, and issued a warning
throw(BuiltinKeeperException)
#endif
{
  VersionCBMap::const_iterator it = m_exceptionsMap.find(std::make_pair(name, w));
  if (m_exceptionsMap.end() != it){
    VersionStrategy* pCb = it->second;
    return (*pCb)(it->first);
  }
  if (!isBuiltin(name)){
    std::string msg = "'" + name + "'";
    msg += "is not an OpenCL built-in function.";
    throw BuiltinKeeperException(msg);
  }
  //now the entire set of overloads is in the cache
  FunctionDescriptor original = demangle(name.c_str());
  BuiltinMap::MapRange mr = m_descriptorsMap.equalRange(original.name);
  RangeUtil range(mr);
  assert(!range.isEmpty() && "cache error: range is empty.");
  do{
    const FunctionDescriptor candidate = range.getDescriptor();
    //we check that the candidate is in the right width, and that its parameters
    //are compatible (i.e., the same scalar type) as the one of the original
    if (w == candidate.width && compatible(candidate, original))
      return fdToPair(candidate);
    range.increment();
  } while(!range.isEmpty());
  return nullPair();
}

//
//BuiltinKeeperException
//
BuiltinKeeperException::BuiltinKeeperException(const std::string& msg): m_msg(msg){
}

BuiltinKeeperException::~BuiltinKeeperException() throw(){
}

const char* BuiltinKeeperException::what()const throw(){
  return m_msg.c_str();
}

}
