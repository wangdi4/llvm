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

  File Name: VersionStrategy.cpp
\****************************************************************************/

#include "VersionStrategy.h"
#include "NameMangleAPI.h"
#include "TypeCast.h"
#include <sstream>

namespace reflection{

//
//PairSW
//

PairSW::PairSW(const std::pair<std::string, width::V>& sw):
std::pair<std::string, width::V>(sw){
}

bool PairSW::operator < (const PairSW& that)const{
  if (second != that.second )
    return second < that.second;
  //We need to find which object (if at all) contains wild-cards
  if (compareWild(first, that.first) || compareWild(that.first, first))
    return false;
  return first.compare(that.first) < 0;
}

bool PairSW::compareWild(const std::string& w , const std::string& s)const{
  if (w.find('*') == std::string::npos)
    return w == s;
  std::string::const_iterator wb = w.begin(), sb = s.begin();
  while (sb != s.end() && wb != w.end()){
    if ('*' != *wb){
      if (*wb != *sb)
        return false;
      ++wb;
      ++sb;
    } else{
      if ( w.end() != (wb+1) && *(wb+1) == *sb )
        ++wb;
      else
        ++sb;
    }
  }
  if (s.end() != sb)
    return false;
  if (w.end() != wb && '*' == *wb)
    ++wb;
  return w.end() == wb;
}

VersionStrategy::~VersionStrategy(){}

//
//NullDescriptorStrategy
//

PairSW NullDescriptorStrategy::operator()(const PairSW&)const{
  return nullPair();
}

NullDescriptorStrategy::~NullDescriptorStrategy(){}

//
//SoaDescriptorStrategy
//

SoaDescriptorStrategy::SoaDescriptorStrategy(): m_pTypeMap(NULL){
}

void SoaDescriptorStrategy::setTypeMap(const ReturnTypeMap* pMap){
  m_pTypeMap = pMap;
}

SoaDescriptorStrategy::~SoaDescriptorStrategy(){
}

void SoaDescriptorStrategy::visit(const Type*){
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const UserDefinedTy*){
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const Vector*){
  m_transposeStrategy = &SoaDescriptorStrategy::vectorReturnTranspose;
}

void SoaDescriptorStrategy::visit(const Pointer*){
  assert(false && "unreachable code");
}

PairSW SoaDescriptorStrategy::operator()(const PairSW& sw)const{
  assert(m_pTypeMap && "NULL type map! (not assigned?)");
  const std::string name = sw.first;
  width::V transposeWidth = sw.second;
  //If the given function is already the scalar version, we return it write back,
  //Otherwise we disallow the scalarization 
  if (transposeWidth == width::SCALAR){
    const std::string soa_prefix("soa_");
    if (soa_prefix == name.substr(0, soa_prefix.length()))
      return nullPair();
    FunctionDescriptor identity = demangle(name.c_str());
    identity.width = width::SCALAR;
    return fdToPair(identity);
  }
  TypeVisitor* pVisitor = const_cast<SoaDescriptorStrategy*>(this);
  const FunctionDescriptor fdOrig = demangle(sw.first.c_str());
  ReturnTypeMap::const_iterator it = m_pTypeMap->find(fdOrig);
  assert (m_pTypeMap->end() != it &&
  "this type cannot be transposed (forgot to add it to transpose map?");
  Type* retTy = it->second;
  assert (retTy && "return type is NULL");
  //activating the double dispatach, so the transpose stategy will be applied
  retTy->accept(pVisitor);
  FunctionDescriptor transposed = (this->*m_transposeStrategy)(sw);
  transposed.width = sw.second;
  return fdToPair(transposed);
}

//Note! all the parematers of the created function descriptors are allocated on
//the heap. Since there life span is that of the BuiltKeeper object that contains
//them, and that object is a singleton, they are never released.
FunctionDescriptor
SoaDescriptorStrategy::scalarReturnTranspose(const PairSW& sw)const{
  const FunctionDescriptor& orig = demangle(sw.first.c_str());
  width::V transposeWidth = sw.second;
  FunctionDescriptor fd;
  for(size_t i=0 ; i<orig.parameters.size() ; ++i){
    const Vector* pVector = reflection::cast<Vector>(orig.parameters[i]);
    width::V paramWidth = (pVector) ?
      static_cast<width::V>(pVector->getLen()):
      width::SCALAR;
    primitives::Primitive p = orig.parameters[i]->getPrimitive();
    for (unsigned i=0 ; i<static_cast<unsigned>(paramWidth) ; ++i){
      Type scalar(p);
      Type* transposedParam =
        new Vector(&scalar, static_cast<int>(transposeWidth));
      fd.parameters.push_back(transposedParam);
      std::stringstream nameBuilder;
      const Vector* pVec = reflection::cast<Vector>(orig.parameters[0]);
      int nameWidth = pVec ? pVec->getLen() : 1;
      nameBuilder << "soa_" << orig.name << nameWidth;
      fd.name = nameBuilder.str();
    }
  }
  return fd;
}

FunctionDescriptor
SoaDescriptorStrategy::vectorReturnTranspose(const PairSW& sw)const{
  const std::string name = sw.first;
  FunctionDescriptor fdOrig = demangle(name.c_str());
  //we now add the parameter for the return type, one vector pointer for each
  //transpose-size
  FunctionDescriptor fd = scalarReturnTranspose(sw);
  //we can safely downcast here, since the visitor's double-dispatch ensures us
  //that Vector is indeed the dynamic type of m_returnTy
  ReturnTypeMap::const_iterator it = m_pTypeMap->find(fdOrig);
  assert (m_pTypeMap->end() != it &&
  "this type cannot be transposed (forgot to add it to transpose map?");
  const Vector* retTy = (const Vector*)it->second;
  assert(retTy && "NULL return type");
  //Calculating the OUT parameter type
  Vector vOut(retTy, sw.second);
  Pointer ptr = Pointer(&vOut);
  for(int i=0 ; i<retTy->getLen() ; ++i)
    fd.parameters.push_back(ptr.clone());
  return fd;
}

//
//HardCodedVersionStrategy
//

void HardCodedVersionStrategy::assumeResponsability(const TableRow& tableRow){
  size_t index = m_table.size();
  m_table.push_back(tableRow);
  for (size_t i=0 ; i<width::OCL_VERSIONS ; ++i)
    m_rowIndex[tableRow.names[i]] = index;
}

PairSW HardCodedVersionStrategy::operator()(const PairSW& p)const{
  llvm::StringMap<int>::const_iterator it = m_rowIndex.find(p.first);
  if (it == m_rowIndex.end())
    return nullPair();
  int tindex;
  width::V w = p.second;
  switch (w){
    case width::SCALAR:
      tindex = 0;
      break;
    case width::TWO:
      tindex = 1;
      break;
    case width::FOUR:
      tindex = 2;
      break;
    case width::EIGHT:
      tindex = 3;
      break;
    case width::SIXTEEN:
      tindex = 4;
      break;
    case width::THREE:
      tindex = 5;
      break;
    default:
      assert( false && "unreachable code");
      tindex = 0;
  }
  TableRow versions = m_table[it->second];
  llvm::StringRef strVersion = versions.names[tindex];
  if (width::SCALAR == w && p.first != strVersion.str() && !versions.isScalarizable)
    return nullPair();
  if (width::SCALAR != w && !versions.isPacketizable)
    return nullPair();
  return PairSW(std::make_pair(std::string(versions.names[tindex]), w));
}

//
//Pair conversions
//

std::pair<std::string,width::V> fdToPair(const FunctionDescriptor& fd){
  return std::make_pair(mangle(fd), fd.width);
}

std::pair<std::string,width::V> nullPair(){
  return std::make_pair(FunctionDescriptor::nullString(), width::NONE);
}

bool isNullPair(const std::pair<std::string,width::V>& sw){
  return sw.first == FunctionDescriptor::nullString();
}
}
