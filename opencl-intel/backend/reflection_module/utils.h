/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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
