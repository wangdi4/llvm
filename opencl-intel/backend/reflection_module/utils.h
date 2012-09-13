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

  File Name: utils.h

\****************************************************************************/

#ifndef __REFLECTION_MOD_UTILS_H__
#define __REFLECTION_MOD_UTILS_H__

namespace reflection{
///////////////////////////////////////////////////////////////////////////////
//Purpose: preforms a cartesian product between two given collections
//Template Parameters:
//  Container: the type of the container. (should receive one template
//  parameter, and have a standard iterator interface.
//  T: the element type of the first container.
//  U: the element type of the second container.
///////////////////////////////////////////////////////////////////////////////
template <template<typename> class Container, typename T, typename U>
class Cartesian{
  typename Container<T>::const_iterator lit, le;
  typename Container<U>::const_iterator rit, re, rb;
public:
  Cartesian(const Container<T>& left, const Container<U>& right){
    assert (!left.empty() && "left container is empty");
    assert (!right.empty() && "right container is empty");
    lit = left.begin();
    rb = rit = right.begin();
    le = left.end();
    re = right.end();    
  }
  
  /////////////////////////////////////////////////////////////////////////////
  //Purpose: advances the cartesian product to the next pair.
  //Return: true if there are more pairs in the product, false otherwise
  /////////////////////////////////////////////////////////////////////////////
  bool next(){
    assert (lit != le && "iteration is finished");
    assert (rit != re && "internal bug, I suck");
    if (++rit == re){
      rit = rb;
      ++lit;
    }
    return lit != le;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  //Purpose: gets the current pair 
  /////////////////////////////////////////////////////////////////////////////
  std::pair<T,U> get()const{
    return std::make_pair(*lit, *rit);
  }
};

//
//TableEntry
//
struct TableRow{
  const char* names[6];
  bool isScalarizable;
  bool isPacketizable; 
};


}//end namespace
#endif//__REFLECTION_MOD_UTILS_H__
