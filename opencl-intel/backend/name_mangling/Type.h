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

  File Name: Type.h

\****************************************************************************/

#include <string>
#include <vector>

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

//The Type class hierarchy models the different types in OCL.

namespace reflection{

namespace primitives{
// the enume doesn't start from one, since it matches the token type
// vocabulary exported by ANTLR. (see the vocabulary in the generted directory,
// with .txt ending.
enum Primitive{
  BOOL=4,
  UCHAR,
  CHAR,
  USHOR,
  SHORT,
  UINT,
  INT,
  ULONG,
  LONG,
  HALF,
  FLOAT,
  DOUBLE
};
}

//total number of primitive types
const int NUM_TYPES = 12;

struct TypeVisitor;

//Represents a primitive type (e.g. i32, i16,...)
struct Type{
  Type(primitives::Primitive);
  //
  //Purpose: returns a string representation of the underlying type.
  //
  virtual std::string toString()const;
  virtual ~Type();
  //
  //Purpose: deep cloning of the type object
  //
  virtual Type* clone()const;
  //
  //Purpose: appends an attribute to the attribute list of the type
  //
  void addAttribute(const std::string&);
  //attributes iterators
  std::vector<std::string>::const_iterator beginAttributes()const;
  std::vector<std::string>::const_iterator endAttributes()const;
  std::vector<std::string>::const_reverse_iterator rbeginAttributes()const;
  std::vector<std::string>::const_reverse_iterator rendAttributes()const;
  //
  //Purpose: 'deep' object equality symetric equality; meaning, a.equals(b) if
  //and only if, both 'a' and 'b' agrees they are equal. (for more details see
  //eq(const Type*)
  //
  bool equals(const Type*)const;
  //
  //Purpose: returns the primitive enumeration of the type
  //
  primitives::Primitive getPrimitive()const {return m_primitive;}
  //
  //Purpose: visitor service method. (see TypeVisitor for more details).
  //When overridden in subclasses, preform a 'double dispatch' to the
  //appropriate visit method in the given visitor
  //
  virtual void accept(TypeVisitor*)const;
protected:
  //
  //returns true if the given type is a subclass, or the same type as that of
  //the receiver, and they both have the same primitive enumeration.
  //
  virtual bool eq(const Type*)const;
  //
  //members
  //

  //an enumeration to identify the primitive type
  primitives::Primitive m_primitive;
  //attributes attached to this type
  std::vector<std::string> m_attributes;
};

struct Pointer: Type{
  //'meta class' related methods (constructions, destruction, cloning)
  //
  //Constructs a Pointer to the given type.
  //t: the type pointed by this Pointer.
  //
  //Remark: t is being cloned by the receiver, thus does not gain ownership
  //for t.
  Pointer(const Type* t);
  ~Pointer();
  Pointer* clone()const;
  //getters/equalizers
  std::string toString()const;
  bool equals(const Type*)const;
  //return the type the pointer is pointing at
  const Type* getPoitee()const;
  void accept(TypeVisitor*)const;
protected:
  bool eq(const Type*)const;
private:
  //the type this pointer is pointing at
  const Type* m_pType;
};

struct Vector: Type{
  //
  //Parameters:
  //  Type*: the type of each scalar element in the vector.
  //  int  : the length of the vector
  Vector(const Type*, int);
  std::string toString()const;
  int getLen()const;
  Vector* clone()const;
  bool equals(Type*)const;
  void accept(TypeVisitor*)const;
protected:
  bool eq(const Type*)const;
private:
  Vector(const Vector&);
  //the length of the vector
  int m_len;
};

//
//parameter discovery callback type system
//

struct TypeCallback{
  virtual void operator() (Type*) = 0;
};

//A delegate object, to be invoked by the name-demangler each time it parses a
//function parameter.
template <typename Receiver>
struct TypeDelegate: TypeCallback{

typedef void (Receiver::*TypeCB)(Type*);

  TypeCB m_cb;
  Receiver* m_pReceiver; //pointer to the receiver of the callback

  void operator() (Type* ptype){
    (m_pReceiver->*m_cb)(ptype);
  }

};

//
//Description:
//  TypeVisitor can be overridden so an object of static type Type* will
//  dispatch the correct visit method according to its dynamic type.
//
struct TypeVisitor{
  virtual void visit(const Type*)   = 0;
  virtual void visit(const Vector*) = 0;
  virtual void visit(const Pointer*)= 0;
};

}//namespace mangle
#endif
