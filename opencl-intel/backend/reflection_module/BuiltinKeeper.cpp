/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"
#include "llvm/Support/MutexGuard.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ArrayRef.h"
#include <cctype>
#include <sstream>

namespace reflection{

  struct PrimitiveVisitor : TypeVisitor {
    void visit(const PrimitiveType* t) {
      m_primitivType = t->getPrimitive();
    }

    void visit(const VectorType* v) {
      v->getScalarType()->accept(this);
    }

    void visit(const PointerType* p) {
      p->getPointee()->accept(this);
    }

    void visit(const AtomicType* a) {
      a->getBaseType()->accept(this);
    }

    void visit(const BlockType*) {
      m_primitivType = PRIMITIVE_NONE;
    }

    void visit(const UserDefinedType*) {
      m_primitivType = PRIMITIVE_NONE;
    }

    TypePrimitiveEnum getPrimitiveType() const {
      return m_primitivType;
    }
  private:
    TypePrimitiveEnum m_primitivType;
  };
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
  typedef TypeVector::const_iterator TypeIter;
  if (l.parameters.size() != r.parameters.size())
    return false;
  TypeIter lit = l.parameters.begin(), lend = l.parameters.end(),
  rit = r.parameters.begin();
  while(lit != lend){
    PrimitiveVisitor lv, rv;
    (*lit)->accept(&lv);
    (*rit)->accept(&rv);
    if (lv.getPrimitiveType() != rv.getPrimitiveType()) {
      return false;
    }
    if (lv.getPrimitiveType() == PRIMITIVE_NONE) {
      if (!(*lit)->equals(*rit)) return false;
    }
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
  const std::pair<std::pair<llvm::StringRef,TypePrimitiveEnum>,width::V>& arg,
  TypePrimitiveEnum PTy)
{
  FunctionDescriptor fd;
  fd.name = arg.first.first.str();
  RefParamType pPrimitiveTy(new PrimitiveType(arg.first.second));
  RefParamType pVectorTy = (width::SCALAR == arg.second) ? pPrimitiveTy :
    (new VectorType(pPrimitiveTy, (int)(arg.second)));
  fd.parameters.push_back(pVectorTy);
  fd.parameters.push_back(new PrimitiveType(PTy));
  return fd;
}

//similar to createDescriptorVP_P, only the vector parameter is the second one,
//and the scalar is the first.
static FunctionDescriptor createDescriptorP_VP (
  const std::pair<std::pair<llvm::StringRef,TypePrimitiveEnum>,width::V>& arg,
  TypePrimitiveEnum PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  std::iter_swap(fd.parameters.begin(), fd.parameters.begin()+1);
  return fd;
}

//creates a function descriptor with three parameters in a similar manner to
//the two parameter version  (createDescriptorVP_P), with a third scalar
//parameter
static FunctionDescriptor createDescriptorVP_P_P(
const std::pair<std::pair<llvm::StringRef,TypePrimitiveEnum>,width::V>& arg,
TypePrimitiveEnum PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  fd.parameters.push_back(fd.parameters[1]);
  return fd;
}

//creates a function descriptor with three parameters, in the following format:
//(vtype, vtype, stype)
static FunctionDescriptor createDescriptorVP_VP_P(
const std::pair<std::pair<llvm::StringRef,TypePrimitiveEnum>,width::V>& arg,
TypePrimitiveEnum PTy)
{
  FunctionDescriptor fd = createDescriptorVP_P(arg, PTy);
  //duplicating the first (vector) parameter
  RefParamType firstType = fd.parameters[0];
  fd.parameters.insert(fd.parameters.begin(), firstType);
  return fd;
}

//creates a function descriptor with three parameters in a similar manner to
//the three parameter version  (createDescriptorVP_P_P), only that the vector
//parameter is third, not first: (type, type, vtype).
static FunctionDescriptor createDescriptorP_P_VP(
const std::pair<std::pair<llvm::StringRef,TypePrimitiveEnum>,width::V>& arg,
TypePrimitiveEnum PTy)
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
  TypePrimitiveEnum primitivesTy[] = {
    PRIMITIVE_CHAR,    //1
    PRIMITIVE_SHORT,   //2
    PRIMITIVE_INT,     //3
    PRIMITIVE_LONG,    //4
    PRIMITIVE_UCHAR,   //5
    PRIMITIVE_USHORT,  //6
    PRIMITIVE_UINT,    //7
    PRIMITIVE_ULONG,   //8
    PRIMITIVE_FLOAT,   //9
    PRIMITIVE_DOUBLE}; //10
  //arrays of types
  PrimitiveArray arrPrimitives(primitivesTy);
  PrimitiveArray arrReals = arrPrimitives.slice(8, 2);
  //arrays of vector width
  VWidthArray arrNonScalars(vwidths+1, (sizeof(vwidths)/sizeof(width::V))-1);
  VWidthArray arrAllWidth(vwidths);
  //
  // Adding WI and memory functions which accept a single unit.
  //
  {
    llvm::StringRef names[] = {
      "get_global_id", "get_global_size", "get_local_size", "get_enqueued_local_size",
      "get_local_id", "get_num_groups", "get_group_id", "get_global_offset", "barrier",
      "mem_fence", "read_mem_fence", "write_mem_fence"
    };
    addExceptionToWIFunctions(StringArray(names), PRIMITIVE_UINT);
  }
  {
    reflection::FunctionDescriptor fdWorkDim;
    fdWorkDim.name = "get_work_dim";
    PairSW key = std::make_pair(mangle(fdWorkDim), width::SCALAR);
    m_exceptionsMap.insert(std::make_pair(key, &m_nullStrategy));
  }
  //adding the 'ambiguous scalar' builtin functions
  //
  //fmin/fmax
  //
  {
    llvm::StringRef names[] = {"fmin", "fmax"};
    StringArray arrNames (names);
    reflection::TypePrimitiveEnum singleFloat[] = {PRIMITIVE_FLOAT};
    reflection::TypePrimitiveEnum singleDouble[] = {PRIMITIVE_DOUBLE};
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
  {
    llvm::StringRef names[] = {"ldexp"};
    StringArray arrLdexp(names);
    addConversionGroup(arrLdexp, arrReals, PRIMITIVE_INT, createDescriptorVP_P);
  }
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
    llvm::StringRef names[] = {"_Z5fract*", "_Z5frexp*", "_Z8lgamma_r*",
      "_Z4modf*", "_Z6remquo*", "_Z6sincos*"};
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
    llvm::StringRef names[] = {"_Z7signbit*", "_Z8isfinite*","_Z5isinf*",
      "_Z5isnan*","_Z8isnormal*","_Z9isordered*", "_Z11isunordered*"};
    StringArray relationals(names);
    VWidthArray allWidths(vwidths);
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> pairs(relationals,
      allWidths);
    do{
      PairSW key(pairs.get());
      m_exceptionsMap.insert(std::make_pair(key, &m_nullStrategy));
    }while(pairs.next());
  }
  //this function cluster cannot be versioned due its OpenCL definition.
  {
    llvm::StringRef names[] = {
      "_Z21async_work_group_copy*", "_Z29async_work_group_strided_copy*",
      "_Z34__async_work_group_stream_to_image*","_Z36__async_work_group_stream_from_image*",
      "_Z41__async_work_group_stream_to_image_direct*", "_Z43__async_work_group_stream_from_image_direct*",
      "__work_group_reserve_write_pipe", "__work_group_commit_write_pipe",
      "__work_group_reserve_read_pipe", "__work_group_commit_read_pipe"
    };
    StringArray uniform_work_group_builtins(names);
    VWidthArray allWidths(vwidths);
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> pairs(uniform_work_group_builtins, allWidths);
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
    m_hardCodedStrategy.assumeResponsability(mappings+i);
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

void BuiltinKeeper::addExceptionToWIFunctions (const StringArray& names,
                                          TypePrimitiveEnum ty) {
  width::V vwidths[] = {width::TWO, width::THREE, width::FOUR, width::EIGHT,
                        width::SIXTEEN};
  VWidthArray arrVWidth(vwidths);
  VWidthArray arrScalarWidth(width::SCALAR);
  Cartesian<llvm::ArrayRef, llvm::StringRef, TypePrimitiveEnum> namesXTy(
    names, PrimitiveArray(ty));

  do {
    // Building a function descriptor, to acquire the mangled name.
    reflection::FunctionDescriptor fd;
    std::pair<llvm::StringRef,TypePrimitiveEnum> current = namesXTy.get();
    fd.name = current.first.str();
    fd.parameters.push_back(RefParamType(new PrimitiveType(current.second)));
    std::string mangledName = mangle(fd);
    llvm::StringRef refMangledName(mangledName);

    // Assigning null strategy, for all non-scalar width.
    StringArray arrMangledNames(refMangledName);
    {
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> keys(arrMangledNames,
      arrVWidth);
    do {
      PairSW key(keys.get());
      m_exceptionsMap.insert(std::make_pair(key, &m_nullStrategy));
    } while(keys.next());
    }
    // Assiging identity stratefy for the scalar width.
    {
    Cartesian<llvm::ArrayRef,llvm::StringRef,width::V> keys(arrMangledNames,
      arrScalarWidth);
    do {
      PairSW key(keys.get());
      m_exceptionsMap.insert(std::make_pair(key, &m_indentityStrategy));
    } while(keys.next());
    }
  } while(namesXTy.next());
}

void BuiltinKeeper::addConversionGroup (const StringArray& names,
  const PrimitiveArray& types, TypePrimitiveEnum p, FDFactory fdFactory){
  //Data Arrays
  width::V vwidths[] = {width::TWO, width::THREE, width::FOUR,
    width::EIGHT, width::SIXTEEN};
  Cartesian<llvm::ArrayRef, llvm::StringRef, TypePrimitiveEnum> product2ways(
    names, types);
  std::vector<std::pair<llvm::StringRef, TypePrimitiveEnum> > tmpResult;
  do{
    tmpResult.push_back(product2ways.get());
  } while(product2ways.next());
  llvm::ArrayRef<std::pair<llvm::StringRef, TypePrimitiveEnum> >arrTmpResult(
    &tmpResult[0], tmpResult.size());
  //arrays of vector width
  VWidthArray arrNonScalars(vwidths);
  Cartesian<llvm::ArrayRef, std::pair<llvm::StringRef, TypePrimitiveEnum>,
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
static std::pair<FunctionDescriptor, RefParamType>
createBiV_V(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType primitiveTy(new PrimitiveType(p));
  RefParamType vTy(new VectorType(primitiveTy, w));
  fd.parameters.push_back(vTy);
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, vTy);
}

template<>
std::pair<FunctionDescriptor, RefParamType>
createBiV_V<1>(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType primitiveTy(new PrimitiveType(p));
  fd.parameters.push_back(primitiveTy);
  fd.parameters.push_back(primitiveTy);
  return std::make_pair(fd, primitiveTy);
}

template<int w>
static std::pair<FunctionDescriptor, RefParamType>
createBiV_S(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  RefParamType vTy(new VectorType(scalar, w));
  fd.parameters.push_back(vTy);
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, scalar);
}

template<> inline
std::pair<FunctionDescriptor, RefParamType>
createBiV_S<1>(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  fd.parameters.push_back(scalar);
  fd.parameters.push_back(scalar);
  return std::make_pair(fd, scalar);
}

template<int w>
std::pair<FunctionDescriptor, RefParamType>
createUniV_S(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  RefParamType vTy(new VectorType(scalar, w));
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, scalar);
}

template<>
std::pair<FunctionDescriptor, RefParamType>
createUniV_S<1>(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  fd.parameters.push_back(scalar);
  return std::make_pair(fd, scalar);
}

template <int w>
std::pair<FunctionDescriptor, RefParamType>
createUniV_V(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  RefParamType vTy(new VectorType(scalar, w));
  fd.parameters.push_back(vTy);
  return std::make_pair(fd, vTy);
}

template <>
std::pair<FunctionDescriptor, RefParamType>
createUniV_V<1>(TypePrimitiveEnum p, const std::string& n){
  FunctionDescriptor fd;
  fd.name = n;
  RefParamType scalar(new PrimitiveType(p));
  fd.parameters.push_back(scalar);
  return std::make_pair(fd, scalar);
}

void BuiltinKeeper::populateReturnTyMap(){
  //
  //cross
  //
  m_fdToRetTy.insert(createBiV_V<3>(PRIMITIVE_DOUBLE, "cross"));
  m_fdToRetTy.insert(createBiV_V<3>(PRIMITIVE_FLOAT, "cross"));
  m_fdToRetTy.insert(createBiV_V<4>(PRIMITIVE_DOUBLE, "cross"));
  m_fdToRetTy.insert(createBiV_V<4>(PRIMITIVE_FLOAT, "cross"));
  //
  //dot
  //
  m_fdToRetTy.insert(createBiV_S<1>(PRIMITIVE_FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<2>(PRIMITIVE_FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<3>(PRIMITIVE_FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<4>(PRIMITIVE_FLOAT, "dot"));
  m_fdToRetTy.insert(createBiV_S<1>(PRIMITIVE_DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<2>(PRIMITIVE_DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<3>(PRIMITIVE_DOUBLE, "dot"));
  m_fdToRetTy.insert(createBiV_S<4>(PRIMITIVE_DOUBLE, "dot"));
  //
  //distance
  //
  m_fdToRetTy.insert(createBiV_S<1>(PRIMITIVE_FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<2>(PRIMITIVE_FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<3>(PRIMITIVE_FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<4>(PRIMITIVE_FLOAT, "distance"));
  m_fdToRetTy.insert(createBiV_S<1>(PRIMITIVE_DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<2>(PRIMITIVE_DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<3>(PRIMITIVE_DOUBLE, "distance"));
  m_fdToRetTy.insert(createBiV_S<4>(PRIMITIVE_DOUBLE, "distance"));
  //
  //fast_distance
  //
  m_fdToRetTy.insert(createBiV_S<1>(PRIMITIVE_FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<2>(PRIMITIVE_FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<3>(PRIMITIVE_FLOAT, "fast_distance"));
  m_fdToRetTy.insert(createBiV_S<4>(PRIMITIVE_FLOAT, "fast_distance"));
  //
  //fast length
  //
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_FLOAT, "fast_length"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_FLOAT, "fast_length"));
  //
  //fast_normalize
  //
  m_fdToRetTy.insert(createUniV_V<1>(PRIMITIVE_FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(PRIMITIVE_FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(PRIMITIVE_FLOAT, "fast_normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(PRIMITIVE_FLOAT, "fast_normalize"));
  //
  //length
  //
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_FLOAT, "length"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_DOUBLE, "length"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_DOUBLE, "length"));
  //
  //normalize
  //
  m_fdToRetTy.insert(createUniV_V<1>(PRIMITIVE_FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(PRIMITIVE_FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(PRIMITIVE_FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(PRIMITIVE_FLOAT, "normalize"));
  m_fdToRetTy.insert(createUniV_V<1>(PRIMITIVE_DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<2>(PRIMITIVE_DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<3>(PRIMITIVE_DOUBLE,"normalize"));
  m_fdToRetTy.insert(createUniV_V<4>(PRIMITIVE_DOUBLE,"normalize"));
  //
  //any
  //
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_CHAR, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_SHORT, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_INT, "any"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_LONG, "any"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_LONG, "any"));
  //
  //all
  //
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_CHAR, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_SHORT, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_INT, "all"));
  m_fdToRetTy.insert(createUniV_S<1>(PRIMITIVE_LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<2>(PRIMITIVE_LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<3>(PRIMITIVE_LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<4>(PRIMITIVE_LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<8>(PRIMITIVE_LONG, "all"));
  m_fdToRetTy.insert(createUniV_S<16>(PRIMITIVE_LONG, "all"));
}

const BuiltinKeeper* BuiltinKeeper::instance(){
  llvm::MutexGuard gaurd(mutex);
  static BuiltinKeeper Instance;
  return &Instance;
}

//Wrapper around the range struct
struct RangeUtil{
  RangeUtil(BuiltinMap::MapRange& mr): m_range(mr){}

  void reset(BuiltinMap::MapRange& mr){
    m_range = mr;
  }

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

bool BuiltinKeeper::isInExceptionMap(const std::string& name)const{
  reflection::width::V allWidth[] = {width::SCALAR, width::TWO, width::FOUR,
    width::EIGHT, width::SIXTEEN, width::THREE};
  for (unsigned i=0 ; i<width::OCL_VERSIONS; ++i) {
    reflection::width::V w = allWidth[i];
    VersionCBMap::const_iterator it =
      m_exceptionsMap.find(std::make_pair(name, w));

    if (m_exceptionsMap.end() != it)
      return true;
  }
  return false;
}

#include "BuiltinList.h"

bool BuiltinKeeper::searchAndCacheUpdate(const FunctionDescriptor& fd) const {
  bool bFound = false, bLineCached = false;

  // Iterated over the mangledNames array (included in BuiltinList.h), and
  // search the name. As we go along, we populate the cache.
  for (size_t i=0 ; i<(sizeof(mangledNames)/sizeof(char*)) ; ++i){
    llvm::StringRef strippedName = stripName(mangledNames[i]);
    if (m_descriptorsMap.isInSameCacheLine(fd.name, strippedName)) {
      // Cache the builtin we demangle.
      FunctionDescriptor candidate = demangle(mangledNames[i]);
      candidate.assignAutomaticWidth();
      m_descriptorsMap.insert(candidate);
      bLineCached = true;
      bFound |= (fd == candidate);
    // If the cache-line was inserted, we can notify the result, since all
    // overloads are grouped together in the mangled names array.
    } else if (bLineCached)
        return bFound;
  }

  // The mangled name was no where to be found.
  return false;
}

bool BuiltinKeeper::isBuiltin(const std::string& mangledString)const{

  if (mangledString.empty())
    return false;

  if (isInExceptionMap(mangledString))
    return true;

  // We need to execute isBuiltin(FunctionDescriptor) whenever the function
  // isn't overloaded, since it has a side effect.
  return isBuiltin (demangle(mangledString.c_str()));
}

bool BuiltinKeeper::isBuiltin(const FunctionDescriptor& fd)const{
  if (fd.isNull())
    return false;

  BuiltinMap::MapRange mr = m_descriptorsMap.equalRange(fd.name);
  RangeUtil range(mr);
  // Is cache line resides in cache?
  if (!range.isEmpty()){
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
  return searchAndCacheUpdate(fd);
}

PairSW BuiltinKeeper::getVersion(const std::string& name, width::V w) const {
  VersionCBMap::const_iterator it = m_exceptionsMap.find(std::make_pair(name, w));
  if (m_exceptionsMap.end() != it){
    VersionStrategy* pCb = it->second;
    return (*pCb)(it->first);
  }

  // If it is not a built-in function, it cannot be versioned.
  if (!isBuiltin(name))
    return nullPair();

  // Now there are two alternatives:
  // 1. The entire overload set is in the cache.
  // 2. The given string is in the exception map, but associated to a different
  //    width.
  FunctionDescriptor original = demangle(name.c_str());
  BuiltinMap::MapRange mr = m_descriptorsMap.equalRange(original.name);
  RangeUtil range(mr);

  // Option (2)... (the string is in the exception map, but associated to a
  // different width).
  if (range.isEmpty() && !searchAndCacheUpdate(original))
    return nullPair();

  // Reasgin the range, since the cache has been updated.
  // The function descriptor resides in the cache, get it from there.
  mr = m_descriptorsMap.equalRange(original.name);
  range.reset(mr);
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

}
