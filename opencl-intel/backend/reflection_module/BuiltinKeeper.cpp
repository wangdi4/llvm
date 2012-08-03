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

#include "NameMangleAPI.h"
#include "BuiltinKeeper.h"
#include "Type.h"
#include  "llvm/Support/MutexGuard.h"
#include <cctype>
#include <sstream>

namespace reflection{

//
//BuiltinKeeper
//

typedef std::multimap<std::string, FunctionDescriptor>::const_iterator MapIT;
typedef std::pair<MapIT, MapIT> MapRange;

llvm::sys::Mutex mutex;

BuiltinKeeper* BuiltinKeeper::Instance = NULL;

BuiltinKeeper::BuiltinKeeper(){
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
  RangeUtil(MapRange& mr): m_range(mr){}

  //indicates whether the given range is empty
  bool isEmpty()const{
    return (m_range.first == m_range.second);
  }

  void increment(){
    assert ((m_range.first != m_range.second) && "range is empty!");
    m_range.first++;
  }

  const FunctionDescriptor& getDescriptor()const{
    return m_range.first->second;
  }
private:
  MapRange& m_range;
};

bool BuiltinKeeper::isBuiltin(const std::string& mangledString)const{
  #include "BuiltinList.h"
  FunctionDescriptor fd = demangle(mangledString.c_str());
  MapRange mr = m_descriptorsMap.equal_range(fd.name);
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
    std::string stripped = stripName(mangledNames[i]);
    if (fd.name == stripped){
      //cache the builtin we demangle
      FunctionDescriptor candidate = demangle(mangledNames[i]);
      m_descriptorsMap.insert(
        std::pair<std::string,FunctionDescriptor>(candidate.name, candidate)
      );
      bLineCached = true;
      bFound |= (fd == candidate);
    } else {
      //if the cache-line was inserted, we can notify the result, since all
      //overloads are grouped together in the mangled names array
      if (bLineCached)
        return bFound;
    }
  }
  //the mangled string could not be found anywhere...
  return false;
}

bool BuiltinKeeper::isBuiltin(const FunctionDescriptor& fd)const{
  return isBuiltin(mangle(fd));
}

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

//TODO: Remove
std::ostream& operator << (std::ostream& os, const FunctionDescriptor& f){
  os << f.toString();
  return os;
}

FunctionDescriptor
BuiltinKeeper::getVersion(const std::string& name, width::V w)const
 #if !defined(_WIN32) && !defined(_WIN64)
 //cl compiler doesn't approve that, and issued a warning
throw(BuiltinKeeperException)
#endif
{
  if (!isBuiltin(name)){
    std::string msg = "'" + name + "'";
    msg += "is not an OpenCL built-in function.";
    throw BuiltinKeeperException(msg);
  }
  //now the entire set of overloads is in the cache
  FunctionDescriptor original = demangle(name.c_str());
  MapRange mr = m_descriptorsMap.equal_range(original.name);
  RangeUtil range(mr);
  assert(!range.isEmpty() && "cache error: range is empty.");
  do{
    const FunctionDescriptor candidate = range.getDescriptor();
    //we check that the canidate is in the right width, and that its parameters
    //are compatible (i.e., the same scalar type) as the one of the original
    if (w == candidate.getWidth() && compatible(candidate, original))
      return candidate;
    range.increment();
  } while(!range.isEmpty());
  std::stringstream msg(name);
  msg << " does not have a " << w << " vector width version";
  throw BuiltinKeeperException(msg.str());
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
